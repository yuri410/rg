//===- BuildSystem.cpp - Utilities for use by build systems ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements various utilities for use by build systems.
//
//===----------------------------------------------------------------------===//

#include "clang-c/BuildSystem.h"
#include "CXString.h"
#include "clang/Basic/VirtualFileSystem.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/CBindingWrapping.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TimeValue.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace llvm::sys;

unsigned long long clang_getBuildSessionTimestamp(void) {
  return llvm::sys::TimeValue::now().toEpochTime();
}

DEFINE_SIMPLE_CONVERSION_FUNCTIONS(clang::vfs::YAMLVFSWriter,
                                   CXVirtualFileOverlay)

CXVirtualFileOverlay clang_VirtualFileOverlay_create(unsigned) {
  return wrap(new clang::vfs::YAMLVFSWriter());
}

enum CXErrorCode
clang_VirtualFileOverlay_addFileMapping(CXVirtualFileOverlay VFO,
                                        const char *virtualPath,
                                        const char *realPath) {
  if (!VFO || !virtualPath || !realPath)
    return CXError_InvalidArguments;
  if (!path::is_absolute(virtualPath))
    return CXError_InvalidArguments;
  if (!path::is_absolute(realPath))
    return CXError_InvalidArguments;

  for (path::const_iterator
         PI = path::begin(virtualPath),
         PE = path::end(virtualPath); PI != PE; ++PI) {
    StringRef Comp = *PI;
    if (Comp == "." || Comp == "..")
      return CXError_InvalidArguments;
  }

  unwrap(VFO)->addFileMapping(virtualPath, realPath);
  return CXError_Success;
}

enum CXErrorCode
clang_VirtualFileOverlay_setCaseSensitivity(CXVirtualFileOverlay VFO,
                                            int caseSensitive) {
  if (!VFO)
    return CXError_InvalidArguments;
  unwrap(VFO)->setCaseSensitivity(caseSensitive);
  return CXError_Success;
}

enum CXErrorCode
clang_VirtualFileOverlay_writeToBuffer(CXVirtualFileOverlay VFO, unsigned,
                                       char **out_buffer_ptr,
                                       unsigned *out_buffer_size) {
  if (!VFO || !out_buffer_ptr || !out_buffer_size)
    return CXError_InvalidArguments;

  llvm::SmallString<256> Buf;
  llvm::raw_svector_ostream OS(Buf);
  unwrap(VFO)->write(OS);

  StringRef Data = OS.str();
  *out_buffer_ptr = (char*)malloc(Data.size());
  *out_buffer_size = Data.size();
  memcpy(*out_buffer_ptr, Data.data(), Data.size());
  return CXError_Success;
}

void clang_free(void *buffer) {
  free(buffer);
}

void clang_VirtualFileOverlay_dispose(CXVirtualFileOverlay VFO) {
  delete unwrap(VFO);
}


struct CXModuleMapDescriptorImpl {
  std::string ModuleName;
  std::string UmbrellaHeader;
};

CXModuleMapDescriptor clang_ModuleMapDescriptor_create(unsigned) {
  return new CXModuleMapDescriptorImpl();
}

enum CXErrorCode
clang_ModuleMapDescriptor_setFrameworkModuleName(CXModuleMapDescriptor MMD,
                                                 const char *name) {
  if (!MMD || !name)
    return CXError_InvalidArguments;

  MMD->ModuleName = name;
  return CXError_Success;
}

enum CXErrorCode
clang_ModuleMapDescriptor_setUmbrellaHeader(CXModuleMapDescriptor MMD,
                                            const char *name) {
  if (!MMD || !name)
    return CXError_InvalidArguments;

  MMD->UmbrellaHeader = name;
  return CXError_Success;
}

enum CXErrorCode
clang_ModuleMapDescriptor_writeToBuffer(CXModuleMapDescriptor MMD, unsigned,
                                       char **out_buffer_ptr,
                                       unsigned *out_buffer_size) {
  if (!MMD || !out_buffer_ptr || !out_buffer_size)
    return CXError_InvalidArguments;

  llvm::SmallString<256> Buf;
  llvm::raw_svector_ostream OS(Buf);
  OS << "framework module " << MMD->ModuleName << " {\n";
  OS << "  umbrella header \"";
  OS.write_escaped(MMD->UmbrellaHeader) << "\"\n";
  OS << '\n';
  OS << "  export *\n";
  OS << "  module * { export * }\n";
  OS << "}\n";

  StringRef Data = OS.str();
  *out_buffer_ptr = (char*)malloc(Data.size());
  *out_buffer_size = Data.size();
  memcpy(*out_buffer_ptr, Data.data(), Data.size());
  return CXError_Success;
}

void clang_ModuleMapDescriptor_dispose(CXModuleMapDescriptor MMD) {
  delete MMD;
}
