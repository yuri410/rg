//===-- CXLoadedDiagnostic.cpp - Handling of persisent diags ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements handling of persisent diagnostics.
//
//===----------------------------------------------------------------------===//

#include "CXLoadedDiagnostic.h"
#include "CXString.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LLVM.h"
#include "clang/Frontend/SerializedDiagnosticReader.h"
#include "clang/Frontend/SerializedDiagnostics.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Bitcode/BitstreamReader.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MemoryBuffer.h"
using namespace clang;

//===----------------------------------------------------------------------===//
// Extend CXDiagnosticSetImpl which contains strings for diagnostics.
//===----------------------------------------------------------------------===//

typedef llvm::DenseMap<unsigned, const char *> Strings;

namespace {
class CXLoadedDiagnosticSetImpl : public CXDiagnosticSetImpl {
public:
  CXLoadedDiagnosticSetImpl() : CXDiagnosticSetImpl(true), FakeFiles(FO) {}
  ~CXLoadedDiagnosticSetImpl() override {}

  llvm::BumpPtrAllocator Alloc;
  Strings Categories;
  Strings WarningFlags;
  Strings FileNames;
  
  FileSystemOptions FO;
  FileManager FakeFiles;
  llvm::DenseMap<unsigned, const FileEntry *> Files;

  /// \brief Copy the string into our own allocator.
  const char *copyString(StringRef Blob) {
    char *mem = Alloc.Allocate<char>(Blob.size() + 1);
    memcpy(mem, Blob.data(), Blob.size());
    mem[Blob.size()] = '\0';
    return mem;
  }
};
}

//===----------------------------------------------------------------------===//
// Cleanup.
//===----------------------------------------------------------------------===//

CXLoadedDiagnostic::~CXLoadedDiagnostic() {}

//===----------------------------------------------------------------------===//
// Public CXLoadedDiagnostic methods.
//===----------------------------------------------------------------------===//

CXDiagnosticSeverity CXLoadedDiagnostic::getSeverity() const {
  // FIXME: Fail more softly if the diagnostic level is unknown?
  auto severityAsLevel = static_cast<serialized_diags::Level>(severity);
  assert(severity == static_cast<unsigned>(severityAsLevel) &&
         "unknown serialized diagnostic level");

  switch (severityAsLevel) {
#define CASE(X) case serialized_diags::X: return CXDiagnostic_##X;
  CASE(Ignored)
  CASE(Note)
  CASE(Warning)
  CASE(Error)
  CASE(Fatal)
#undef CASE
  // The 'Remark' level isn't represented in the stable API.
  case serialized_diags::Remark: return CXDiagnostic_Warning;
  }
  
  llvm_unreachable("Invalid diagnostic level");
}

static CXSourceLocation makeLocation(const CXLoadedDiagnostic::Location *DLoc) {
  // The lowest bit of ptr_data[0] is always set to 1 to indicate this
  // is a persistent diagnostic.
  uintptr_t V = (uintptr_t) DLoc;
  V |= 0x1;
  CXSourceLocation Loc = { {  (void*) V, nullptr }, 0 };
  return Loc;
}  

CXSourceLocation CXLoadedDiagnostic::getLocation() const {
  // The lowest bit of ptr_data[0] is always set to 1 to indicate this
  // is a persistent diagnostic.
  return makeLocation(&DiagLoc);
}

CXString CXLoadedDiagnostic::getSpelling() const {
  return cxstring::createRef(Spelling);
}

CXString CXLoadedDiagnostic::getDiagnosticOption(CXString *Disable) const {
  if (DiagOption.empty())
    return cxstring::createEmpty();

  // FIXME: possibly refactor with logic in CXStoredDiagnostic.
  if (Disable)
    *Disable = cxstring::createDup((Twine("-Wno-") + DiagOption).str());
  return cxstring::createDup((Twine("-W") + DiagOption).str());
}

unsigned CXLoadedDiagnostic::getCategory() const {
  return category;
}

CXString CXLoadedDiagnostic::getCategoryText() const {
  return cxstring::createDup(CategoryText);
}

unsigned CXLoadedDiagnostic::getNumRanges() const {
  return Ranges.size();
}

CXSourceRange CXLoadedDiagnostic::getRange(unsigned Range) const {
  assert(Range < Ranges.size());
  return Ranges[Range];
}

unsigned CXLoadedDiagnostic::getNumFixIts() const {
  return FixIts.size();
}

CXString CXLoadedDiagnostic::getFixIt(unsigned FixIt,
                                      CXSourceRange *ReplacementRange) const {
  assert(FixIt < FixIts.size());
  if (ReplacementRange)
    *ReplacementRange = FixIts[FixIt].first;
  return cxstring::createRef(FixIts[FixIt].second);
}

void CXLoadedDiagnostic::decodeLocation(CXSourceLocation location,
                                        CXFile *file,
                                        unsigned int *line,
                                        unsigned int *column,
                                        unsigned int *offset) {
  
  
  // CXSourceLocation consists of the following fields:
  //
  //   void *ptr_data[2];
  //   unsigned int_data;
  //
  // The lowest bit of ptr_data[0] is always set to 1 to indicate this
  // is a persistent diagnostic.
  //
  // For now, do the unoptimized approach and store the data in a side
  // data structure.  We can optimize this case later.
  
  uintptr_t V = (uintptr_t) location.ptr_data[0];
  assert((V & 0x1) == 1);
  V &= ~(uintptr_t)1;
  
  const Location &Loc = *((Location*)V);
  
  if (file)
    *file = Loc.file;  
  if (line)
    *line = Loc.line;
  if (column)
    *column = Loc.column;
  if (offset)
    *offset = Loc.offset;
}

//===----------------------------------------------------------------------===//
// Deserialize diagnostics.
//===----------------------------------------------------------------------===//

namespace {
class DiagLoader : serialized_diags::SerializedDiagnosticReader {
  enum CXLoadDiag_Error *error;
  CXString *errorString;
  std::unique_ptr<CXLoadedDiagnosticSetImpl> TopDiags;
  SmallVector<std::unique_ptr<CXLoadedDiagnostic>, 8> CurrentDiags;

  std::error_code reportBad(enum CXLoadDiag_Error code, llvm::StringRef err) {
    if (error)
      *error = code;
    if (errorString)
      *errorString = cxstring::createDup(err);
    return serialized_diags::SDError::HandlerFailed;
  }
  
  std::error_code reportInvalidFile(llvm::StringRef err) {
    return reportBad(CXLoadDiag_InvalidFile, err);
  }

  std::error_code readRange(const serialized_diags::Location &SDStart,
                            const serialized_diags::Location &SDEnd,
                            CXSourceRange &SR);

  std::error_code readLocation(const serialized_diags::Location &SDLoc,
                               CXLoadedDiagnostic::Location &LoadedLoc);

protected:
  std::error_code visitStartOfDiagnostic() override;
  std::error_code visitEndOfDiagnostic() override;

  std::error_code visitCategoryRecord(unsigned ID, StringRef Name) override;

  std::error_code visitDiagFlagRecord(unsigned ID, StringRef Name) override;

  std::error_code visitDiagnosticRecord(
      unsigned Severity, const serialized_diags::Location &Location,
      unsigned Category, unsigned Flag, StringRef Message) override;

  std::error_code visitFilenameRecord(unsigned ID, unsigned Size,
                                      unsigned Timestamp,
                                      StringRef Name) override;

  std::error_code visitFixitRecord(const serialized_diags::Location &Start,
                                   const serialized_diags::Location &End,
                                   StringRef CodeToInsert) override;

  std::error_code
  visitSourceRangeRecord(const serialized_diags::Location &Start,
                         const serialized_diags::Location &End) override;

public:
  DiagLoader(enum CXLoadDiag_Error *e, CXString *es)
      : SerializedDiagnosticReader(), error(e), errorString(es) {
    if (error)
      *error = CXLoadDiag_None;
    if (errorString)
      *errorString = cxstring::createEmpty();
  }

  CXDiagnosticSet load(const char *file);
};
}

CXDiagnosticSet DiagLoader::load(const char *file) {
  TopDiags = llvm::make_unique<CXLoadedDiagnosticSetImpl>();

  std::error_code EC = readDiagnostics(file);
  if (EC) {
    switch (EC.value()) {
    case static_cast<int>(serialized_diags::SDError::HandlerFailed):
      // We've already reported the problem.
      break;
    case static_cast<int>(serialized_diags::SDError::CouldNotLoad):
      reportBad(CXLoadDiag_CannotLoad, EC.message());
      break;
    default:
      reportInvalidFile(EC.message());
      break;
    }
    return 0;
  }

  return (CXDiagnosticSet)TopDiags.release();
}

std::error_code
DiagLoader::readLocation(const serialized_diags::Location &SDLoc,
                         CXLoadedDiagnostic::Location &LoadedLoc) {
  unsigned FileID = SDLoc.FileID;
  if (FileID == 0)
    LoadedLoc.file = nullptr;
  else {
    LoadedLoc.file = const_cast<FileEntry *>(TopDiags->Files[FileID]);
    if (!LoadedLoc.file)
      return reportInvalidFile("Corrupted file entry in source location");
  }
  LoadedLoc.line = SDLoc.Line;
  LoadedLoc.column = SDLoc.Col;
  LoadedLoc.offset = SDLoc.Offset;
  return std::error_code();
}

std::error_code
DiagLoader::readRange(const serialized_diags::Location &SDStart,
                      const serialized_diags::Location &SDEnd,
                      CXSourceRange &SR) {
  CXLoadedDiagnostic::Location *Start, *End;
  Start = TopDiags->Alloc.Allocate<CXLoadedDiagnostic::Location>();
  End = TopDiags->Alloc.Allocate<CXLoadedDiagnostic::Location>();

  std::error_code EC;
  if ((EC = readLocation(SDStart, *Start)))
    return EC;
  if ((EC = readLocation(SDEnd, *End)))
    return EC;
  
  CXSourceLocation startLoc = makeLocation(Start);
  CXSourceLocation endLoc = makeLocation(End);
  SR = clang_getRange(startLoc, endLoc);
  return std::error_code();
}

std::error_code DiagLoader::visitStartOfDiagnostic() {
  CurrentDiags.push_back(llvm::make_unique<CXLoadedDiagnostic>());
  return std::error_code();
}

std::error_code DiagLoader::visitEndOfDiagnostic() {
  auto D = CurrentDiags.pop_back_val();
  if (CurrentDiags.empty())
    TopDiags->appendDiagnostic(std::move(D));
  else
    CurrentDiags.back()->getChildDiagnostics().appendDiagnostic(std::move(D));
  return std::error_code();
}

std::error_code DiagLoader::visitCategoryRecord(unsigned ID, StringRef Name) {
  // FIXME: Why do we care about long strings?
  if (Name.size() > 65536)
    return reportInvalidFile("Out-of-bounds string in category");
  TopDiags->Categories[ID] = TopDiags->copyString(Name);
  return std::error_code();
}

std::error_code DiagLoader::visitDiagFlagRecord(unsigned ID, StringRef Name) {
  // FIXME: Why do we care about long strings?
  if (Name.size() > 65536)
    return reportInvalidFile("Out-of-bounds string in warning flag");
  TopDiags->WarningFlags[ID] = TopDiags->copyString(Name);
  return std::error_code();
}

std::error_code DiagLoader::visitFilenameRecord(unsigned ID, unsigned Size,
                                                unsigned Timestamp,
                                                StringRef Name) {
  // FIXME: Why do we care about long strings?
  if (Name.size() > 65536)
    return reportInvalidFile("Out-of-bounds string in filename");
  TopDiags->FileNames[ID] = TopDiags->copyString(Name);
  TopDiags->Files[ID] =
      TopDiags->FakeFiles.getVirtualFile(Name, Size, Timestamp);
  return std::error_code();
}

std::error_code
DiagLoader::visitSourceRangeRecord(const serialized_diags::Location &Start,
                                   const serialized_diags::Location &End) {
  CXSourceRange SR;
  if (std::error_code EC = readRange(Start, End, SR))
    return EC;
  CurrentDiags.back()->Ranges.push_back(SR);
  return std::error_code();
}

std::error_code
DiagLoader::visitFixitRecord(const serialized_diags::Location &Start,
                             const serialized_diags::Location &End,
                             StringRef CodeToInsert) {
  CXSourceRange SR;
  if (std::error_code EC = readRange(Start, End, SR))
    return EC;
  // FIXME: Why do we care about long strings?
  if (CodeToInsert.size() > 65536)
    return reportInvalidFile("Out-of-bounds string in FIXIT");
  CurrentDiags.back()->FixIts.push_back(
      std::make_pair(SR, TopDiags->copyString(CodeToInsert)));
  return std::error_code();
}

std::error_code DiagLoader::visitDiagnosticRecord(
    unsigned Severity, const serialized_diags::Location &Location,
    unsigned Category, unsigned Flag, StringRef Message) {
  CXLoadedDiagnostic &D = *CurrentDiags.back();
  D.severity = Severity;
  if (std::error_code EC = readLocation(Location, D.DiagLoc))
    return EC;
  D.category = Category;
  D.DiagOption = Flag ? TopDiags->WarningFlags[Flag] : "";
  D.CategoryText = Category ? TopDiags->Categories[Category] : "";
  D.Spelling = TopDiags->copyString(Message);
  return std::error_code();
}

// extern "C" {                  // HLSL Change -Don't use c linkage.
CXDiagnosticSet clang_loadDiagnostics(const char *file,
                                      enum CXLoadDiag_Error *error,
                                      CXString *errorString) {
  DiagLoader L(error, errorString);
  return L.load(file);
}
// } // end extern 'C'.          // HLSL Change -Don't use c linkage.
