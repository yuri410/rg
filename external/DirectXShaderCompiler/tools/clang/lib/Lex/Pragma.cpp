//===--- Pragma.cpp - Pragma registration and handling --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the PragmaHandler/PragmaTable interfaces and implements
// pragma related methods of the Preprocessor class.
//
//===----------------------------------------------------------------------===//

#include "clang/Lex/Pragma.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/LiteralSupport.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/ErrorHandling.h"
#include <algorithm>
using namespace clang;

#include "llvm/Support/raw_ostream.h"

// Out-of-line destructor to provide a home for the class.
PragmaHandler::~PragmaHandler() {
}

//===----------------------------------------------------------------------===//
// EmptyPragmaHandler Implementation.
//===----------------------------------------------------------------------===//

EmptyPragmaHandler::EmptyPragmaHandler() {}

void EmptyPragmaHandler::HandlePragma(Preprocessor &PP, 
                                      PragmaIntroducerKind Introducer,
                                      Token &FirstToken) {}

//===----------------------------------------------------------------------===//
// PragmaNamespace Implementation.
//===----------------------------------------------------------------------===//

PragmaNamespace::~PragmaNamespace() {
  llvm::DeleteContainerSeconds(Handlers);
}

/// FindHandler - Check to see if there is already a handler for the
/// specified name.  If not, return the handler for the null identifier if it
/// exists, otherwise return null.  If IgnoreNull is true (the default) then
/// the null handler isn't returned on failure to match.
PragmaHandler *PragmaNamespace::FindHandler(StringRef Name,
                                            bool IgnoreNull) const {
  if (PragmaHandler *Handler = Handlers.lookup(Name))
    return Handler;
  return IgnoreNull ? nullptr : Handlers.lookup(StringRef());
}

void PragmaNamespace::AddPragma(PragmaHandler *Handler) {
  // HLSL Change Begins: Don't leak on exceptions
  std::unique_ptr<PragmaHandler> HandlerPtr(Handler);
  assert(!Handlers.lookup(Handler->getName()) &&
         "A handler with this name is already registered in this namespace");
  PragmaHandler*& MapHandler = Handlers[Handler->getName()];
  MapHandler = HandlerPtr.release();
  // HLSL Change Ends
}

void PragmaNamespace::RemovePragmaHandler(PragmaHandler *Handler) {
  assert(Handlers.lookup(Handler->getName()) &&
         "Handler not registered in this namespace");
  Handlers.erase(Handler->getName());
}

void PragmaNamespace::HandlePragma(Preprocessor &PP, 
                                   PragmaIntroducerKind Introducer,
                                   Token &Tok) {
  // Read the 'namespace' that the directive is in, e.g. STDC.  Do not macro
  // expand it, the user can have a STDC #define, that should not affect this.
  PP.LexUnexpandedToken(Tok);

  // Get the handler for this token.  If there is no handler, ignore the pragma.
  PragmaHandler *Handler
    = FindHandler(Tok.getIdentifierInfo() ? Tok.getIdentifierInfo()->getName()
                                          : StringRef(),
                  /*IgnoreNull=*/false);
  if (!Handler) {
    PP.Diag(Tok, diag::warn_pragma_ignored);
    return;
  }

  // Otherwise, pass it down.
  Handler->HandlePragma(PP, Introducer, Tok);
}

//===----------------------------------------------------------------------===//
// Preprocessor Pragma Directive Handling.
//===----------------------------------------------------------------------===//

/// HandlePragmaDirective - The "\#pragma" directive has been parsed.  Lex the
/// rest of the pragma, passing it to the registered pragma handlers.
void Preprocessor::HandlePragmaDirective(SourceLocation IntroducerLoc,
                                         PragmaIntroducerKind Introducer) {
  if (Callbacks)
    Callbacks->PragmaDirective(IntroducerLoc, Introducer);

  if (!PragmasEnabled)
    return;

  ++NumPragma;

  // Invoke the first level of pragma handlers which reads the namespace id.
  Token Tok;
  PragmaHandlers->HandlePragma(*this, Introducer, Tok);

  // If the pragma handler didn't read the rest of the line, consume it now.
  if ((CurTokenLexer && CurTokenLexer->isParsingPreprocessorDirective()) 
   || (CurPPLexer && CurPPLexer->ParsingPreprocessorDirective))
    DiscardUntilEndOfDirective();
}

namespace {
/// \brief Helper class for \see Preprocessor::Handle_Pragma.
class LexingFor_PragmaRAII {
  Preprocessor &PP;
  bool InMacroArgPreExpansion;
  bool Failed;
  Token &OutTok;
  Token PragmaTok;

public:
  LexingFor_PragmaRAII(Preprocessor &PP, bool InMacroArgPreExpansion,
                       Token &Tok)
    : PP(PP), InMacroArgPreExpansion(InMacroArgPreExpansion),
      Failed(false), OutTok(Tok) {
    if (InMacroArgPreExpansion) {
      PragmaTok = OutTok;
      PP.EnableBacktrackAtThisPos();
    }
  }

  ~LexingFor_PragmaRAII() {
    if (InMacroArgPreExpansion) {
      if (Failed) {
        PP.CommitBacktrackedTokens();
      } else {
        PP.Backtrack();
        OutTok = PragmaTok;
      }
    }
  }

  void failed() {
    Failed = true;
  }
};
}

/// Handle_Pragma - Read a _Pragma directive, slice it up, process it, then
/// return the first token after the directive.  The _Pragma token has just
/// been read into 'Tok'.
void Preprocessor::Handle_Pragma(Token &Tok) {

  // This works differently if we are pre-expanding a macro argument.
  // In that case we don't actually "activate" the pragma now, we only lex it
  // until we are sure it is lexically correct and then we backtrack so that
  // we activate the pragma whenever we encounter the tokens again in the token
  // stream. This ensures that we will activate it in the correct location
  // or that we will ignore it if it never enters the token stream, e.g:
  //
  //     #define EMPTY(x)
  //     #define INACTIVE(x) EMPTY(x)
  //     INACTIVE(_Pragma("clang diagnostic ignored \"-Wconversion\""))

  LexingFor_PragmaRAII _PragmaLexing(*this, InMacroArgPreExpansion, Tok);

  // Remember the pragma token location.
  SourceLocation PragmaLoc = Tok.getLocation();

  // Read the '('.
  Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    Diag(PragmaLoc, diag::err__Pragma_malformed);
    return _PragmaLexing.failed();
  }

  // Read the '"..."'.
  Lex(Tok);
  if (!tok::isStringLiteral(Tok.getKind())) {
    Diag(PragmaLoc, diag::err__Pragma_malformed);
    // Skip this token, and the ')', if present.
    if (Tok.isNot(tok::r_paren) && Tok.isNot(tok::eof))
      Lex(Tok);
    if (Tok.is(tok::r_paren))
      Lex(Tok);
    return _PragmaLexing.failed();
  }

  if (Tok.hasUDSuffix()) {
    Diag(Tok, diag::err_invalid_string_udl);
    // Skip this token, and the ')', if present.
    Lex(Tok);
    if (Tok.is(tok::r_paren))
      Lex(Tok);
    return _PragmaLexing.failed();
  }

  // Remember the string.
  Token StrTok = Tok;

  // Read the ')'.
  Lex(Tok);
  if (Tok.isNot(tok::r_paren)) {
    Diag(PragmaLoc, diag::err__Pragma_malformed);
    return _PragmaLexing.failed();
  }

  if (InMacroArgPreExpansion)
    return;

  SourceLocation RParenLoc = Tok.getLocation();
  std::string StrVal = getSpelling(StrTok);

  // The _Pragma is lexically sound.  Destringize according to C11 6.10.9.1:
  // "The string literal is destringized by deleting any encoding prefix,
  // deleting the leading and trailing double-quotes, replacing each escape
  // sequence \" by a double-quote, and replacing each escape sequence \\ by a
  // single backslash."
  if (StrVal[0] == 'L' || StrVal[0] == 'U' ||
      (StrVal[0] == 'u' && StrVal[1] != '8'))
    StrVal.erase(StrVal.begin());
  else if (StrVal[0] == 'u')
    StrVal.erase(StrVal.begin(), StrVal.begin() + 2);

  if (StrVal[0] == 'R') {
    // FIXME: C++11 does not specify how to handle raw-string-literals here.
    // We strip off the 'R', the quotes, the d-char-sequences, and the parens.
    assert(StrVal[1] == '"' && StrVal[StrVal.size() - 1] == '"' &&
           "Invalid raw string token!");

    // Measure the length of the d-char-sequence.
    unsigned NumDChars = 0;
    while (StrVal[2 + NumDChars] != '(') {
      assert(NumDChars < (StrVal.size() - 5) / 2 &&
             "Invalid raw string token!");
      ++NumDChars;
    }
    assert(StrVal[StrVal.size() - 2 - NumDChars] == ')');

    // Remove 'R " d-char-sequence' and 'd-char-sequence "'. We'll replace the
    // parens below.
    StrVal.erase(0, 2 + NumDChars);
    StrVal.erase(StrVal.size() - 1 - NumDChars);
  } else {
    assert(StrVal[0] == '"' && StrVal[StrVal.size()-1] == '"' &&
           "Invalid string token!");

    // Remove escaped quotes and escapes.
    unsigned ResultPos = 1;
    for (unsigned i = 1, e = StrVal.size() - 1; i != e; ++i) {
      // Skip escapes.  \\ -> '\' and \" -> '"'.
      if (StrVal[i] == '\\' && i + 1 < e &&
          (StrVal[i + 1] == '\\' || StrVal[i + 1] == '"'))
        ++i;
      StrVal[ResultPos++] = StrVal[i];
    }
    StrVal.erase(StrVal.begin() + ResultPos, StrVal.end() - 1);
  }

  // Remove the front quote, replacing it with a space, so that the pragma
  // contents appear to have a space before them.
  StrVal[0] = ' ';

  // Replace the terminating quote with a \n.
  StrVal[StrVal.size()-1] = '\n';

  // Plop the string (including the newline and trailing null) into a buffer
  // where we can lex it.
  Token TmpTok;
  TmpTok.startToken();
  CreateString(StrVal, TmpTok);
  SourceLocation TokLoc = TmpTok.getLocation();

  // Make and enter a lexer object so that we lex and expand the tokens just
  // like any others.
  Lexer *TL = Lexer::Create_PragmaLexer(TokLoc, PragmaLoc, RParenLoc,
                                        StrVal.size(), *this);

  EnterSourceFileWithLexer(TL, nullptr);

  // With everything set up, lex this as a #pragma directive.
  HandlePragmaDirective(PragmaLoc, PIK__Pragma);

  // Finally, return whatever came after the pragma directive.
  return Lex(Tok);
}

/// HandleMicrosoft__pragma - Like Handle_Pragma except the pragma text
/// is not enclosed within a string literal.
void Preprocessor::HandleMicrosoft__pragma(Token &Tok) {
  // Remember the pragma token location.
  SourceLocation PragmaLoc = Tok.getLocation();

  // Read the '('.
  Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    Diag(PragmaLoc, diag::err__Pragma_malformed);
    return;
  }

  // Get the tokens enclosed within the __pragma(), as well as the final ')'.
  SmallVector<Token, 32> PragmaToks;
  int NumParens = 0;
  Lex(Tok);
  while (Tok.isNot(tok::eof)) {
    PragmaToks.push_back(Tok);
    if (Tok.is(tok::l_paren))
      NumParens++;
    else if (Tok.is(tok::r_paren) && NumParens-- == 0)
      break;
    Lex(Tok);
  }

  if (Tok.is(tok::eof)) {
    Diag(PragmaLoc, diag::err_unterminated___pragma);
    return;
  }

  PragmaToks.front().setFlag(Token::LeadingSpace);

  // Replace the ')' with an EOD to mark the end of the pragma.
  PragmaToks.back().setKind(tok::eod);

  Token *TokArray = new Token[PragmaToks.size()];
  std::copy(PragmaToks.begin(), PragmaToks.end(), TokArray);

  // Push the tokens onto the stack.
  EnterTokenStream(TokArray, PragmaToks.size(), true, true);

  // With everything set up, lex this as a #pragma directive.
  HandlePragmaDirective(PragmaLoc, PIK___pragma);

  // Finally, return whatever came after the pragma directive.
  return Lex(Tok);
}

/// HandlePragmaOnce - Handle \#pragma once.  OnceTok is the 'once'.
///
void Preprocessor::HandlePragmaOnce(Token &OnceTok) {
  if (isInPrimaryFile()) {
    Diag(OnceTok, diag::pp_pragma_once_in_main_file);
    return;
  }

  // Get the current file lexer we're looking at.  Ignore _Pragma 'files' etc.
  // Mark the file as a once-only file now.
  HeaderInfo.MarkFileIncludeOnce(getCurrentFileLexer()->getFileEntry());
}

void Preprocessor::HandlePragmaMark() {
  assert(CurPPLexer && "No current lexer?");
  if (CurLexer)
    CurLexer->ReadToEndOfLine();
  else
    CurPTHLexer->DiscardToEndOfLine();
}


/// HandlePragmaPoison - Handle \#pragma GCC poison.  PoisonTok is the 'poison'.
///
void Preprocessor::HandlePragmaPoison(Token &PoisonTok) {
  Token Tok;

  while (1) {
    // Read the next token to poison.  While doing this, pretend that we are
    // skipping while reading the identifier to poison.
    // This avoids errors on code like:
    //   #pragma GCC poison X
    //   #pragma GCC poison X
    if (CurPPLexer) CurPPLexer->LexingRawMode = true;
    LexUnexpandedToken(Tok);
    if (CurPPLexer) CurPPLexer->LexingRawMode = false;

    // If we reached the end of line, we're done.
    if (Tok.is(tok::eod)) return;

    // Can only poison identifiers.
    if (Tok.isNot(tok::raw_identifier)) {
      Diag(Tok, diag::err_pp_invalid_poison);
      return;
    }

    // Look up the identifier info for the token.  We disabled identifier lookup
    // by saying we're skipping contents, so we need to do this manually.
    IdentifierInfo *II = LookUpIdentifierInfo(Tok);

    // Already poisoned.
    if (II->isPoisoned()) continue;

    // If this is a macro identifier, emit a warning.
    if (isMacroDefined(II))
      Diag(Tok, diag::pp_poisoning_existing_macro);

    // Finally, poison it!
    II->setIsPoisoned();
    if (II->isFromAST())
      II->setChangedSinceDeserialization();
  }
}

/// HandlePragmaSystemHeader - Implement \#pragma GCC system_header.  We know
/// that the whole directive has been parsed.
void Preprocessor::HandlePragmaSystemHeader(Token &SysHeaderTok) {
  if (isInPrimaryFile()) {
    Diag(SysHeaderTok, diag::pp_pragma_sysheader_in_main_file);
    return;
  }

  // Get the current file lexer we're looking at.  Ignore _Pragma 'files' etc.
  PreprocessorLexer *TheLexer = getCurrentFileLexer();

  // Mark the file as a system header.
  HeaderInfo.MarkFileSystemHeader(TheLexer->getFileEntry());


  PresumedLoc PLoc = SourceMgr.getPresumedLoc(SysHeaderTok.getLocation());
  if (PLoc.isInvalid())
    return;
  
  unsigned FilenameID = SourceMgr.getLineTableFilenameID(PLoc.getFilename());

  // Notify the client, if desired, that we are in a new source file.
  if (Callbacks)
    Callbacks->FileChanged(SysHeaderTok.getLocation(),
                           PPCallbacks::SystemHeaderPragma, SrcMgr::C_System);

  // Emit a line marker.  This will change any source locations from this point
  // forward to realize they are in a system header.
  // Create a line note with this information.
  SourceMgr.AddLineNote(SysHeaderTok.getLocation(), PLoc.getLine()+1,
                        FilenameID, /*IsEntry=*/false, /*IsExit=*/false,
                        /*IsSystem=*/true, /*IsExternC=*/false);
}

/// HandlePragmaDependency - Handle \#pragma GCC dependency "foo" blah.
///
void Preprocessor::HandlePragmaDependency(Token &DependencyTok) {
  Token FilenameTok;
  CurPPLexer->LexIncludeFilename(FilenameTok);

  // If the token kind is EOD, the error has already been diagnosed.
  if (FilenameTok.is(tok::eod))
    return;

  // Reserve a buffer to get the spelling.
  SmallString<128> FilenameBuffer;
  bool Invalid = false;
  StringRef Filename = getSpelling(FilenameTok, FilenameBuffer, &Invalid);
  if (Invalid)
    return;

  bool isAngled =
    GetIncludeFilenameSpelling(FilenameTok.getLocation(), Filename);
  // If GetIncludeFilenameSpelling set the start ptr to null, there was an
  // error.
  if (Filename.empty())
    return;

  // Search include directories for this file.
  const DirectoryLookup *CurDir;
  const FileEntry *File =
      LookupFile(FilenameTok.getLocation(), Filename, isAngled, nullptr,
                 nullptr, CurDir, nullptr, nullptr, nullptr);
  if (!File) {
    if (!SuppressIncludeNotFoundError)
      Diag(FilenameTok, diag::err_pp_file_not_found) << Filename;
    return;
  }

  const FileEntry *CurFile = getCurrentFileLexer()->getFileEntry();

  // If this file is older than the file it depends on, emit a diagnostic.
  if (CurFile && CurFile->getModificationTime() < File->getModificationTime()) {
    // Lex tokens at the end of the message and include them in the message.
    std::string Message;
    Lex(DependencyTok);
    while (DependencyTok.isNot(tok::eod)) {
      Message += getSpelling(DependencyTok) + " ";
      Lex(DependencyTok);
    }

    // Remove the trailing ' ' if present.
    if (!Message.empty())
      Message.erase(Message.end()-1);
    Diag(FilenameTok, diag::pp_out_of_date_dependency) << Message;
  }
}

/// ParsePragmaPushOrPopMacro - Handle parsing of pragma push_macro/pop_macro.
/// Return the IdentifierInfo* associated with the macro to push or pop.
IdentifierInfo *Preprocessor::ParsePragmaPushOrPopMacro(Token &Tok) {
  // Remember the pragma token location.
  Token PragmaTok = Tok;

  // Read the '('.
  Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    Diag(PragmaTok.getLocation(), diag::err_pragma_push_pop_macro_malformed)
      << getSpelling(PragmaTok);
    return nullptr;
  }

  // Read the macro name string.
  Lex(Tok);
  if (Tok.isNot(tok::string_literal)) {
    Diag(PragmaTok.getLocation(), diag::err_pragma_push_pop_macro_malformed)
      << getSpelling(PragmaTok);
    return nullptr;
  }

  if (Tok.hasUDSuffix()) {
    Diag(Tok, diag::err_invalid_string_udl);
    return nullptr;
  }

  // Remember the macro string.
  std::string StrVal = getSpelling(Tok);

  // Read the ')'.
  Lex(Tok);
  if (Tok.isNot(tok::r_paren)) {
    Diag(PragmaTok.getLocation(), diag::err_pragma_push_pop_macro_malformed)
      << getSpelling(PragmaTok);
    return nullptr;
  }

  assert(StrVal[0] == '"' && StrVal[StrVal.size()-1] == '"' &&
         "Invalid string token!");

  // Create a Token from the string.
  Token MacroTok;
  MacroTok.startToken();
  MacroTok.setKind(tok::raw_identifier);
  CreateString(StringRef(&StrVal[1], StrVal.size() - 2), MacroTok);

  // Get the IdentifierInfo of MacroToPushTok.
  return LookUpIdentifierInfo(MacroTok);
}

/// \brief Handle \#pragma push_macro.
///
/// The syntax is:
/// \code
///   #pragma push_macro("macro")
/// \endcode
void Preprocessor::HandlePragmaPushMacro(Token &PushMacroTok) {
  // Parse the pragma directive and get the macro IdentifierInfo*.
  IdentifierInfo *IdentInfo = ParsePragmaPushOrPopMacro(PushMacroTok);
  if (!IdentInfo) return;

  // Get the MacroInfo associated with IdentInfo.
  MacroInfo *MI = getMacroInfo(IdentInfo);
 
  if (MI) {
    // Allow the original MacroInfo to be redefined later.
    MI->setIsAllowRedefinitionsWithoutWarning(true);
  }

  // Push the cloned MacroInfo so we can retrieve it later.
  PragmaPushMacroInfo[IdentInfo].push_back(MI);
}

/// \brief Handle \#pragma pop_macro.
///
/// The syntax is:
/// \code
///   #pragma pop_macro("macro")
/// \endcode
void Preprocessor::HandlePragmaPopMacro(Token &PopMacroTok) {
  SourceLocation MessageLoc = PopMacroTok.getLocation();

  // Parse the pragma directive and get the macro IdentifierInfo*.
  IdentifierInfo *IdentInfo = ParsePragmaPushOrPopMacro(PopMacroTok);
  if (!IdentInfo) return;

  // Find the vector<MacroInfo*> associated with the macro.
  llvm::DenseMap<IdentifierInfo*, std::vector<MacroInfo*> >::iterator iter =
    PragmaPushMacroInfo.find(IdentInfo);
  if (iter != PragmaPushMacroInfo.end()) {
    // Forget the MacroInfo currently associated with IdentInfo.
    if (MacroInfo *MI = getMacroInfo(IdentInfo)) {
      if (MI->isWarnIfUnused())
        WarnUnusedMacroLocs.erase(MI->getDefinitionLoc());
      appendMacroDirective(IdentInfo, AllocateUndefMacroDirective(MessageLoc));
    }

    // Get the MacroInfo we want to reinstall.
    MacroInfo *MacroToReInstall = iter->second.back();

    if (MacroToReInstall)
      // Reinstall the previously pushed macro.
      appendDefMacroDirective(IdentInfo, MacroToReInstall, MessageLoc);

    // Pop PragmaPushMacroInfo stack.
    iter->second.pop_back();
    if (iter->second.size() == 0)
      PragmaPushMacroInfo.erase(iter);
  } else {
    Diag(MessageLoc, diag::warn_pragma_pop_macro_no_push)
      << IdentInfo->getName();
  }
}

void Preprocessor::HandlePragmaIncludeAlias(Token &Tok) {
  // We will either get a quoted filename or a bracketed filename, and we 
  // have to track which we got.  The first filename is the source name,
  // and the second name is the mapped filename.  If the first is quoted,
  // the second must be as well (cannot mix and match quotes and brackets).

  // Get the open paren
  Lex(Tok);
  if (Tok.isNot(tok::l_paren)) {
    Diag(Tok, diag::warn_pragma_include_alias_expected) << "(";
    return;
  }

  // We expect either a quoted string literal, or a bracketed name
  Token SourceFilenameTok;
  CurPPLexer->LexIncludeFilename(SourceFilenameTok);
  if (SourceFilenameTok.is(tok::eod)) {
    // The diagnostic has already been handled
    return;
  }

  StringRef SourceFileName;
  SmallString<128> FileNameBuffer;
  if (SourceFilenameTok.is(tok::string_literal) || 
      SourceFilenameTok.is(tok::angle_string_literal)) {
    SourceFileName = getSpelling(SourceFilenameTok, FileNameBuffer);
  } else if (SourceFilenameTok.is(tok::less)) {
    // This could be a path instead of just a name
    FileNameBuffer.push_back('<');
    SourceLocation End;
    if (ConcatenateIncludeName(FileNameBuffer, End))
      return; // Diagnostic already emitted
    SourceFileName = FileNameBuffer;
  } else {
    Diag(Tok, diag::warn_pragma_include_alias_expected_filename);
    return;
  }
  FileNameBuffer.clear();

  // Now we expect a comma, followed by another include name
  Lex(Tok);
  if (Tok.isNot(tok::comma)) {
    Diag(Tok, diag::warn_pragma_include_alias_expected) << ",";
    return;
  }

  Token ReplaceFilenameTok;
  CurPPLexer->LexIncludeFilename(ReplaceFilenameTok);
  if (ReplaceFilenameTok.is(tok::eod)) {
    // The diagnostic has already been handled
    return;
  }

  StringRef ReplaceFileName;
  if (ReplaceFilenameTok.is(tok::string_literal) || 
      ReplaceFilenameTok.is(tok::angle_string_literal)) {
    ReplaceFileName = getSpelling(ReplaceFilenameTok, FileNameBuffer);
  } else if (ReplaceFilenameTok.is(tok::less)) {
    // This could be a path instead of just a name
    FileNameBuffer.push_back('<');
    SourceLocation End;
    if (ConcatenateIncludeName(FileNameBuffer, End))
      return; // Diagnostic already emitted
    ReplaceFileName = FileNameBuffer;
  } else {
    Diag(Tok, diag::warn_pragma_include_alias_expected_filename);
    return;
  }

  // Finally, we expect the closing paren
  Lex(Tok);
  if (Tok.isNot(tok::r_paren)) {
    Diag(Tok, diag::warn_pragma_include_alias_expected) << ")";
    return;
  }

  // Now that we have the source and target filenames, we need to make sure
  // they're both of the same type (angled vs non-angled)
  StringRef OriginalSource = SourceFileName;

  bool SourceIsAngled = 
    GetIncludeFilenameSpelling(SourceFilenameTok.getLocation(), 
                                SourceFileName);
  bool ReplaceIsAngled =
    GetIncludeFilenameSpelling(ReplaceFilenameTok.getLocation(),
                                ReplaceFileName);
  if (!SourceFileName.empty() && !ReplaceFileName.empty() &&
      (SourceIsAngled != ReplaceIsAngled)) {
    unsigned int DiagID;
    if (SourceIsAngled)
      DiagID = diag::warn_pragma_include_alias_mismatch_angle;
    else
      DiagID = diag::warn_pragma_include_alias_mismatch_quote;

    Diag(SourceFilenameTok.getLocation(), DiagID)
      << SourceFileName 
      << ReplaceFileName;

    return;
  }

  // Now we can let the include handler know about this mapping
  getHeaderSearchInfo().AddIncludeAlias(OriginalSource, ReplaceFileName);
}

/// AddPragmaHandler - Add the specified pragma handler to the preprocessor.
/// If 'Namespace' is non-null, then it is a token required to exist on the
/// pragma line before the pragma string starts, e.g. "STDC" or "GCC".
void Preprocessor::AddPragmaHandler(StringRef Namespace,
                                    PragmaHandler *Handler) {
  std::unique_ptr<PragmaHandler> HandlerPtr(Handler); // HLSL Change: Don't leak on exceptions
  PragmaNamespace *InsertNS = PragmaHandlers.get();

  // If this is specified to be in a namespace, step down into it.
  if (!Namespace.empty()) {
    // If there is already a pragma handler with the name of this namespace,
    // we either have an error (directive with the same name as a namespace) or
    // we already have the namespace to insert into.
    if (PragmaHandler *Existing = PragmaHandlers->FindHandler(Namespace)) {
      InsertNS = Existing->getIfNamespace();
      assert(InsertNS != nullptr && "Cannot have a pragma namespace and pragma"
             " handler with the same name!");
    } else {
      // Otherwise, this namespace doesn't exist yet, create and insert the
      // handler for it.
      InsertNS = new PragmaNamespace(Namespace);
      PragmaHandlers->AddPragma(InsertNS);
    }
  }

  // Check to make sure we don't already have a pragma for this identifier.
  assert(!InsertNS->FindHandler(Handler->getName()) &&
         "Pragma handler already exists for this identifier!");
  InsertNS->AddPragma(HandlerPtr.release()); // HLSL Change: Don't leak on exceptions
}

/// RemovePragmaHandler - Remove the specific pragma handler from the
/// preprocessor. If \arg Namespace is non-null, then it should be the
/// namespace that \arg Handler was added to. It is an error to remove
/// a handler that has not been registered.
void Preprocessor::RemovePragmaHandler(StringRef Namespace,
                                       PragmaHandler *Handler) {
  PragmaNamespace *NS = PragmaHandlers.get();

  // If this is specified to be in a namespace, step down into it.
  if (!Namespace.empty()) {
    PragmaHandler *Existing = PragmaHandlers->FindHandler(Namespace);
    assert(Existing && "Namespace containing handler does not exist!");

    NS = Existing->getIfNamespace();
    assert(NS && "Invalid namespace, registered as a regular pragma handler!");
  }

  NS->RemovePragmaHandler(Handler);

  // If this is a non-default namespace and it is now empty, remove it.
  if (NS != PragmaHandlers.get() && NS->IsEmpty()) {
    PragmaHandlers->RemovePragmaHandler(NS);
    delete NS;
  }
}

bool Preprocessor::LexOnOffSwitch(tok::OnOffSwitch &Result) {
  Token Tok;
  LexUnexpandedToken(Tok);

  if (Tok.isNot(tok::identifier)) {
    Diag(Tok, diag::ext_on_off_switch_syntax);
    return true;
  }
  IdentifierInfo *II = Tok.getIdentifierInfo();
  if (II->isStr("ON"))
    Result = tok::OOS_ON;
  else if (II->isStr("OFF"))
    Result = tok::OOS_OFF;
  else if (II->isStr("DEFAULT"))
    Result = tok::OOS_DEFAULT;
  else {
    Diag(Tok, diag::ext_on_off_switch_syntax);
    return true;
  }

  // Verify that this is followed by EOD.
  LexUnexpandedToken(Tok);
  if (Tok.isNot(tok::eod))
    Diag(Tok, diag::ext_pragma_syntax_eod);
  return false;
}

namespace {
/// PragmaOnceHandler - "\#pragma once" marks the file as atomically included.
struct PragmaOnceHandler : public PragmaHandler {
  PragmaOnceHandler() : PragmaHandler("once") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &OnceTok) override {
    PP.CheckEndOfDirective("pragma once");
    PP.HandlePragmaOnce(OnceTok);
  }
};

/// PragmaMarkHandler - "\#pragma mark ..." is ignored by the compiler, and the
/// rest of the line is not lexed.
struct PragmaMarkHandler : public PragmaHandler {
  PragmaMarkHandler() : PragmaHandler("mark") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &MarkTok) override {
    PP.HandlePragmaMark();
  }
};

/// PragmaPoisonHandler - "\#pragma poison x" marks x as not usable.
struct PragmaPoisonHandler : public PragmaHandler {
  PragmaPoisonHandler() : PragmaHandler("poison") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &PoisonTok) override {
    PP.HandlePragmaPoison(PoisonTok);
  }
};

/// PragmaSystemHeaderHandler - "\#pragma system_header" marks the current file
/// as a system header, which silences warnings in it.
struct PragmaSystemHeaderHandler : public PragmaHandler {
  PragmaSystemHeaderHandler() : PragmaHandler("system_header") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &SHToken) override {
    PP.HandlePragmaSystemHeader(SHToken);
    PP.CheckEndOfDirective("pragma");
  }
};
struct PragmaDependencyHandler : public PragmaHandler {
  PragmaDependencyHandler() : PragmaHandler("dependency") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &DepToken) override {
    PP.HandlePragmaDependency(DepToken);
  }
};

struct PragmaDebugHandler : public PragmaHandler {
  PragmaDebugHandler() : PragmaHandler("__debug") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &DepToken) override {
    Token Tok;
    PP.LexUnexpandedToken(Tok);
    if (Tok.isNot(tok::identifier)) {
      PP.Diag(Tok, diag::warn_pragma_diagnostic_invalid);
      return;
    }
    IdentifierInfo *II = Tok.getIdentifierInfo();

    if (II->isStr("assert")) {
      llvm_unreachable("This is an assertion!");
    } else if (II->isStr("crash")) {
      LLVM_BUILTIN_TRAP;
    } else if (II->isStr("parser_crash")) {
      Token Crasher;
      Crasher.startToken();
      Crasher.setKind(tok::annot_pragma_parser_crash);
      Crasher.setAnnotationRange(SourceRange(Tok.getLocation()));
      PP.EnterToken(Crasher);
    } else if (II->isStr("llvm_fatal_error")) {
      llvm::report_fatal_error("#pragma clang __debug llvm_fatal_error");
    } else if (II->isStr("llvm_unreachable")) {
      llvm_unreachable("#pragma clang __debug llvm_unreachable");
    } else if (II->isStr("macro")) {
      Token MacroName;
      PP.LexUnexpandedToken(MacroName);
      auto *MacroII = MacroName.getIdentifierInfo();
      if (MacroII)
        PP.dumpMacroInfo(MacroII);
      else
        PP.Diag(MacroName, diag::warn_pragma_diagnostic_invalid);
    } else if (II->isStr("overflow_stack")) {
      DebugOverflowStack();
    } else if (II->isStr("handle_crash")) {
      llvm::CrashRecoveryContext *CRC =llvm::CrashRecoveryContext::GetCurrent();
      if (CRC)
        CRC->HandleCrash();
    } else if (II->isStr("captured")) {
      HandleCaptured(PP);
    } else {
      PP.Diag(Tok, diag::warn_pragma_debug_unexpected_command)
        << II->getName();
    }

    PPCallbacks *Callbacks = PP.getPPCallbacks();
    if (Callbacks)
      Callbacks->PragmaDebug(Tok.getLocation(), II->getName());
  }

  void HandleCaptured(Preprocessor &PP) {
    // Skip if emitting preprocessed output.
    if (PP.isPreprocessedOutput())
      return;

    Token Tok;
    PP.LexUnexpandedToken(Tok);

    if (Tok.isNot(tok::eod)) {
      PP.Diag(Tok, diag::ext_pp_extra_tokens_at_eol)
        << "pragma clang __debug captured";
      return;
    }

    SourceLocation NameLoc = Tok.getLocation();
    Token *Toks = PP.getPreprocessorAllocator().Allocate<Token>(1);
    Toks->startToken();
    Toks->setKind(tok::annot_pragma_captured);
    Toks->setLocation(NameLoc);

    PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true,
                        /*OwnsTokens=*/false);
  }

// Disable MSVC warning about runtime stack overflow.
#ifdef _MSC_VER
    #pragma warning(disable : 4717)
#endif
  static void DebugOverflowStack() {
    void (*volatile Self)() = DebugOverflowStack;
    Self();
  }
#ifdef _MSC_VER
    #pragma warning(default : 4717)
#endif

};

/// PragmaDiagnosticHandler - e.g. '\#pragma GCC diagnostic ignored "-Wformat"'
struct PragmaDiagnosticHandler : public PragmaHandler {
private:
  const char *Namespace;
public:
  explicit PragmaDiagnosticHandler(const char *NS) :
    PragmaHandler("diagnostic"), Namespace(NS) {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &DiagToken) override {
    SourceLocation DiagLoc = DiagToken.getLocation();
    Token Tok;
    PP.LexUnexpandedToken(Tok);
    if (Tok.isNot(tok::identifier)) {
      PP.Diag(Tok, diag::warn_pragma_diagnostic_invalid);
      return;
    }
    IdentifierInfo *II = Tok.getIdentifierInfo();
    PPCallbacks *Callbacks = PP.getPPCallbacks();

    if (II->isStr("pop")) {
      if (!PP.getDiagnostics().popMappings(DiagLoc))
        PP.Diag(Tok, diag::warn_pragma_diagnostic_cannot_pop);
      else if (Callbacks)
        Callbacks->PragmaDiagnosticPop(DiagLoc, Namespace);
      return;
    } else if (II->isStr("push")) {
      PP.getDiagnostics().pushMappings(DiagLoc);
      if (Callbacks)
        Callbacks->PragmaDiagnosticPush(DiagLoc, Namespace);
      return;
    }

    diag::Severity SV = llvm::StringSwitch<diag::Severity>(II->getName())
                            .Case("ignored", diag::Severity::Ignored)
                            .Case("warning", diag::Severity::Warning)
                            .Case("error", diag::Severity::Error)
                            .Case("fatal", diag::Severity::Fatal)
                            .Default(diag::Severity());

    if (SV == diag::Severity()) {
      PP.Diag(Tok, diag::warn_pragma_diagnostic_invalid);
      return;
    }

    PP.LexUnexpandedToken(Tok);
    SourceLocation StringLoc = Tok.getLocation();

    std::string WarningName;
    if (!PP.FinishLexStringLiteral(Tok, WarningName, "pragma diagnostic",
                                   /*MacroExpansion=*/false))
      return;

    if (Tok.isNot(tok::eod)) {
      PP.Diag(Tok.getLocation(), diag::warn_pragma_diagnostic_invalid_token);
      return;
    }

    if (WarningName.size() < 3 || WarningName[0] != '-' ||
        (WarningName[1] != 'W' && WarningName[1] != 'R')) {
      PP.Diag(StringLoc, diag::warn_pragma_diagnostic_invalid_option);
      return;
    }

    if (PP.getDiagnostics().setSeverityForGroup(
            WarningName[1] == 'W' ? diag::Flavor::WarningOrError
                                  : diag::Flavor::Remark,
            WarningName.substr(2), SV, DiagLoc))
      PP.Diag(StringLoc, diag::warn_pragma_diagnostic_unknown_warning)
        << WarningName;
    else if (Callbacks)
      Callbacks->PragmaDiagnostic(DiagLoc, Namespace, SV, WarningName);
  }
};

/// "\#pragma warning(...)".  MSVC's diagnostics do not map cleanly to clang's
/// diagnostics, so we don't really implement this pragma.  We parse it and
/// ignore it to avoid -Wunknown-pragma warnings.
struct PragmaWarningHandler : public PragmaHandler {
  PragmaWarningHandler() : PragmaHandler("warning") {}

  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &Tok) override {
    // Parse things like:
    // warning(push, 1)
    // warning(pop)
    // warning(disable : 1 2 3 ; error : 4 5 6 ; suppress : 7 8 9)
    SourceLocation DiagLoc = Tok.getLocation();
    PPCallbacks *Callbacks = PP.getPPCallbacks();

    PP.Lex(Tok);
    if (Tok.isNot(tok::l_paren)) {
      PP.Diag(Tok, diag::warn_pragma_warning_expected) << "(";
      return;
    }

    PP.Lex(Tok);
    IdentifierInfo *II = Tok.getIdentifierInfo();

    if (II && II->isStr("push")) {
      // #pragma warning( push[ ,n ] )
      int Level = -1;
      PP.Lex(Tok);
      if (Tok.is(tok::comma)) {
        PP.Lex(Tok);
        uint64_t Value;
        if (Tok.is(tok::numeric_constant) &&
            PP.parseSimpleIntegerLiteral(Tok, Value))
          Level = int(Value);
        if (Level < 0 || Level > 4) {
          PP.Diag(Tok, diag::warn_pragma_warning_push_level);
          return;
        }
      }
      if (Callbacks)
        Callbacks->PragmaWarningPush(DiagLoc, Level);
    } else if (II && II->isStr("pop")) {
      // #pragma warning( pop )
      PP.Lex(Tok);
      if (Callbacks)
        Callbacks->PragmaWarningPop(DiagLoc);
    } else {
      // #pragma warning( warning-specifier : warning-number-list
      //                  [; warning-specifier : warning-number-list...] )
      while (true) {
        II = Tok.getIdentifierInfo();
        if (!II && !Tok.is(tok::numeric_constant)) {
          PP.Diag(Tok, diag::warn_pragma_warning_spec_invalid);
          return;
        }

        // Figure out which warning specifier this is.
        bool SpecifierValid;
        StringRef Specifier;
        llvm::SmallString<1> SpecifierBuf;
        if (II) {
          Specifier = II->getName();
          SpecifierValid = llvm::StringSwitch<bool>(Specifier)
                               .Cases("default", "disable", "error", "once",
                                      "suppress", true)
                               .Default(false);
          // If we read a correct specifier, snatch next token (that should be
          // ":", checked later).
          if (SpecifierValid)
            PP.Lex(Tok);
        } else {
          // Token is a numeric constant. It should be either 1, 2, 3 or 4.
          uint64_t Value;
          Specifier = PP.getSpelling(Tok, SpecifierBuf);
          if (PP.parseSimpleIntegerLiteral(Tok, Value)) {
            SpecifierValid = (Value >= 1) && (Value <= 4);
          } else
            SpecifierValid = false;
          // Next token already snatched by parseSimpleIntegerLiteral.
        }

        if (!SpecifierValid) {
          PP.Diag(Tok, diag::warn_pragma_warning_spec_invalid);
          return;
        }
        if (Tok.isNot(tok::colon)) {
          PP.Diag(Tok, diag::warn_pragma_warning_expected) << ":";
          return;
        }

        // Collect the warning ids.
        SmallVector<int, 4> Ids;
        PP.Lex(Tok);
        while (Tok.is(tok::numeric_constant)) {
          uint64_t Value;
          if (!PP.parseSimpleIntegerLiteral(Tok, Value) || Value == 0 ||
              Value > INT_MAX) {
            PP.Diag(Tok, diag::warn_pragma_warning_expected_number);
            return;
          }
          Ids.push_back(int(Value));
        }
        if (Callbacks)
          Callbacks->PragmaWarning(DiagLoc, Specifier, Ids);

        // Parse the next specifier if there is a semicolon.
        if (Tok.isNot(tok::semi))
          break;
        PP.Lex(Tok);
      }
    }

    if (Tok.isNot(tok::r_paren)) {
      PP.Diag(Tok, diag::warn_pragma_warning_expected) << ")";
      return;
    }

    PP.Lex(Tok);
    if (Tok.isNot(tok::eod))
      PP.Diag(Tok, diag::ext_pp_extra_tokens_at_eol) << "pragma warning";
  }
};

/// PragmaIncludeAliasHandler - "\#pragma include_alias("...")".
struct PragmaIncludeAliasHandler : public PragmaHandler {
  PragmaIncludeAliasHandler() : PragmaHandler("include_alias") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &IncludeAliasTok) override {
    PP.HandlePragmaIncludeAlias(IncludeAliasTok);
  }
};

/// PragmaMessageHandler - Handle the microsoft and gcc \#pragma message
/// extension.  The syntax is:
/// \code
///   #pragma message(string)
/// \endcode
/// OR, in GCC mode:
/// \code
///   #pragma message string
/// \endcode
/// string is a string, which is fully macro expanded, and permits string
/// concatenation, embedded escape characters, etc... See MSDN for more details.
/// Also handles \#pragma GCC warning and \#pragma GCC error which take the same
/// form as \#pragma message.
struct PragmaMessageHandler : public PragmaHandler {
private:
  const PPCallbacks::PragmaMessageKind Kind;
  const StringRef Namespace;

  static const char* PragmaKind(PPCallbacks::PragmaMessageKind Kind,
                                bool PragmaNameOnly = false) {
    switch (Kind) {
      case PPCallbacks::PMK_Message:
        return PragmaNameOnly ? "message" : "pragma message";
      case PPCallbacks::PMK_Warning:
        return PragmaNameOnly ? "warning" : "pragma warning";
      case PPCallbacks::PMK_Error:
        return PragmaNameOnly ? "error" : "pragma error";
    }
    llvm_unreachable("Unknown PragmaMessageKind!");
  }

public:
  PragmaMessageHandler(PPCallbacks::PragmaMessageKind Kind,
                       StringRef Namespace = StringRef())
    : PragmaHandler(PragmaKind(Kind, true)), Kind(Kind), Namespace(Namespace) {}

  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &Tok) override {
    SourceLocation MessageLoc = Tok.getLocation();
    PP.Lex(Tok);
    bool ExpectClosingParen = false;
    switch (Tok.getKind()) {
    case tok::l_paren:
      // We have a MSVC style pragma message.
      ExpectClosingParen = true;
      // Read the string.
      PP.Lex(Tok);
      break;
    case tok::string_literal:
      // We have a GCC style pragma message, and we just read the string.
      break;
    default:
      PP.Diag(MessageLoc, diag::err_pragma_message_malformed) << Kind;
      return;
    }

    std::string MessageString;
    if (!PP.FinishLexStringLiteral(Tok, MessageString, PragmaKind(Kind),
                                   /*MacroExpansion=*/true))
      return;

    if (ExpectClosingParen) {
      if (Tok.isNot(tok::r_paren)) {
        PP.Diag(Tok.getLocation(), diag::err_pragma_message_malformed) << Kind;
        return;
      }
      PP.Lex(Tok);  // eat the r_paren.
    }

    if (Tok.isNot(tok::eod)) {
      PP.Diag(Tok.getLocation(), diag::err_pragma_message_malformed) << Kind;
      return;
    }

    // Output the message.
    PP.Diag(MessageLoc, (Kind == PPCallbacks::PMK_Error)
                          ? diag::err_pragma_message
                          : diag::warn_pragma_message) << MessageString;

    // If the pragma is lexically sound, notify any interested PPCallbacks.
    if (PPCallbacks *Callbacks = PP.getPPCallbacks())
      Callbacks->PragmaMessage(MessageLoc, Namespace, Kind, MessageString);
  }
};

/// PragmaPushMacroHandler - "\#pragma push_macro" saves the value of the
/// macro on the top of the stack.
struct PragmaPushMacroHandler : public PragmaHandler {
  PragmaPushMacroHandler() : PragmaHandler("push_macro") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &PushMacroTok) override {
    PP.HandlePragmaPushMacro(PushMacroTok);
  }
};


/// PragmaPopMacroHandler - "\#pragma pop_macro" sets the value of the
/// macro to the value on the top of the stack.
struct PragmaPopMacroHandler : public PragmaHandler {
  PragmaPopMacroHandler() : PragmaHandler("pop_macro") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &PopMacroTok) override {
    PP.HandlePragmaPopMacro(PopMacroTok);
  }
};

// Pragma STDC implementations.

/// PragmaSTDC_FENV_ACCESSHandler - "\#pragma STDC FENV_ACCESS ...".
struct PragmaSTDC_FENV_ACCESSHandler : public PragmaHandler {
  PragmaSTDC_FENV_ACCESSHandler() : PragmaHandler("FENV_ACCESS") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &Tok) override {
    tok::OnOffSwitch OOS;
    if (PP.LexOnOffSwitch(OOS))
     return;
    if (OOS == tok::OOS_ON)
      PP.Diag(Tok, diag::warn_stdc_fenv_access_not_supported);
  }
};

/// PragmaSTDC_CX_LIMITED_RANGEHandler - "\#pragma STDC CX_LIMITED_RANGE ...".
struct PragmaSTDC_CX_LIMITED_RANGEHandler : public PragmaHandler {
  PragmaSTDC_CX_LIMITED_RANGEHandler()
    : PragmaHandler("CX_LIMITED_RANGE") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &Tok) override {
    tok::OnOffSwitch OOS;
    PP.LexOnOffSwitch(OOS);
  }
};

/// PragmaSTDC_UnknownHandler - "\#pragma STDC ...".
struct PragmaSTDC_UnknownHandler : public PragmaHandler {
  PragmaSTDC_UnknownHandler() {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &UnknownTok) override {
    // C99 6.10.6p2, unknown forms are not allowed.
    PP.Diag(UnknownTok, diag::ext_stdc_pragma_ignored);
  }
};

/// PragmaARCCFCodeAuditedHandler - 
///   \#pragma clang arc_cf_code_audited begin/end
struct PragmaARCCFCodeAuditedHandler : public PragmaHandler {
  PragmaARCCFCodeAuditedHandler() : PragmaHandler("arc_cf_code_audited") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &NameTok) override {
    SourceLocation Loc = NameTok.getLocation();
    bool IsBegin;

    Token Tok;

    // Lex the 'begin' or 'end'.
    PP.LexUnexpandedToken(Tok);
    const IdentifierInfo *BeginEnd = Tok.getIdentifierInfo();
    if (BeginEnd && BeginEnd->isStr("begin")) {
      IsBegin = true;
    } else if (BeginEnd && BeginEnd->isStr("end")) {
      IsBegin = false;
    } else {
      PP.Diag(Tok.getLocation(), diag::err_pp_arc_cf_code_audited_syntax);
      return;
    }

    // Verify that this is followed by EOD.
    PP.LexUnexpandedToken(Tok);
    if (Tok.isNot(tok::eod))
      PP.Diag(Tok, diag::ext_pp_extra_tokens_at_eol) << "pragma";

    // The start location of the active audit.
    SourceLocation BeginLoc = PP.getPragmaARCCFCodeAuditedLoc();

    // The start location we want after processing this.
    SourceLocation NewLoc;

    if (IsBegin) {
      // Complain about attempts to re-enter an audit.
      if (BeginLoc.isValid()) {
        PP.Diag(Loc, diag::err_pp_double_begin_of_arc_cf_code_audited);
        PP.Diag(BeginLoc, diag::note_pragma_entered_here);
      }
      NewLoc = Loc;
    } else {
      // Complain about attempts to leave an audit that doesn't exist.
      if (!BeginLoc.isValid()) {
        PP.Diag(Loc, diag::err_pp_unmatched_end_of_arc_cf_code_audited);
        return;
      }
      NewLoc = SourceLocation();
    }

    PP.setPragmaARCCFCodeAuditedLoc(NewLoc);
  }
};

/// PragmaAssumeNonNullHandler -
///   \#pragma clang assume_nonnull begin/end
struct PragmaAssumeNonNullHandler : public PragmaHandler {
  PragmaAssumeNonNullHandler() : PragmaHandler("assume_nonnull") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &NameTok) override {
    SourceLocation Loc = NameTok.getLocation();
    bool IsBegin;

    Token Tok;

    // Lex the 'begin' or 'end'.
    PP.LexUnexpandedToken(Tok);
    const IdentifierInfo *BeginEnd = Tok.getIdentifierInfo();
    if (BeginEnd && BeginEnd->isStr("begin")) {
      IsBegin = true;
    } else if (BeginEnd && BeginEnd->isStr("end")) {
      IsBegin = false;
    } else {
      PP.Diag(Tok.getLocation(), diag::err_pp_assume_nonnull_syntax);
      return;
    }

    // Verify that this is followed by EOD.
    PP.LexUnexpandedToken(Tok);
    if (Tok.isNot(tok::eod))
      PP.Diag(Tok, diag::ext_pp_extra_tokens_at_eol) << "pragma";

    // The start location of the active audit.
    SourceLocation BeginLoc = PP.getPragmaAssumeNonNullLoc();

    // The start location we want after processing this.
    SourceLocation NewLoc;

    if (IsBegin) {
      // Complain about attempts to re-enter an audit.
      if (BeginLoc.isValid()) {
        PP.Diag(Loc, diag::err_pp_double_begin_of_assume_nonnull);
        PP.Diag(BeginLoc, diag::note_pragma_entered_here);
      }
      NewLoc = Loc;
    } else {
      // Complain about attempts to leave an audit that doesn't exist.
      if (!BeginLoc.isValid()) {
        PP.Diag(Loc, diag::err_pp_unmatched_end_of_assume_nonnull);
        return;
      }
      NewLoc = SourceLocation();
    }

    PP.setPragmaAssumeNonNullLoc(NewLoc);
  }
};

/// \brief Handle "\#pragma region [...]"
///
/// The syntax is
/// \code
///   #pragma region [optional name]
///   #pragma endregion [optional comment]
/// \endcode
///
/// \note This is
/// <a href="http://msdn.microsoft.com/en-us/library/b6xkz944(v=vs.80).aspx">editor-only</a>
/// pragma, just skipped by compiler.
struct PragmaRegionHandler : public PragmaHandler {
  PragmaRegionHandler(const char *pragma) : PragmaHandler(pragma) { }

  void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &NameTok) override {
    // #pragma region: endregion matches can be verified
    // __pragma(region): no sense, but ignored by msvc
    // _Pragma is not valid for MSVC, but there isn't any point
    // to handle a _Pragma differently.
  }
};

}  // end anonymous namespace


/// RegisterBuiltinPragmas - Install the standard preprocessor pragmas:
/// \#pragma GCC poison/system_header/dependency and \#pragma once.
void Preprocessor::RegisterBuiltinPragmas() {
  // HLSL Change Starts - pick only the pragma handlers we care about
  if (LangOpts.HLSL) {
    // TODO: add HLSL-specific pragma handlers
    AddPragmaHandler(new PragmaOnceHandler());
    AddPragmaHandler(new PragmaMarkHandler());
    AddPragmaHandler("dxc", new PragmaDiagnosticHandler("dxc"));
    AddPragmaHandler(new PragmaMessageHandler(PPCallbacks::PMK_Message));
    return;
  }
  // HLSL Change Ends

  AddPragmaHandler(new PragmaOnceHandler());
  AddPragmaHandler(new PragmaMarkHandler());
  AddPragmaHandler(new PragmaPushMacroHandler());
  AddPragmaHandler(new PragmaPopMacroHandler());
  AddPragmaHandler(new PragmaMessageHandler(PPCallbacks::PMK_Message));

  // #pragma GCC ...
  AddPragmaHandler("GCC", new PragmaPoisonHandler());
  AddPragmaHandler("GCC", new PragmaSystemHeaderHandler());
  AddPragmaHandler("GCC", new PragmaDependencyHandler());
  AddPragmaHandler("GCC", new PragmaDiagnosticHandler("GCC"));
  AddPragmaHandler("GCC", new PragmaMessageHandler(PPCallbacks::PMK_Warning,
                                                   "GCC"));
  AddPragmaHandler("GCC", new PragmaMessageHandler(PPCallbacks::PMK_Error,
                                                   "GCC"));
  // #pragma clang ...
  AddPragmaHandler("clang", new PragmaPoisonHandler());
  AddPragmaHandler("clang", new PragmaSystemHeaderHandler());
  AddPragmaHandler("clang", new PragmaDebugHandler());
  AddPragmaHandler("clang", new PragmaDependencyHandler());
  AddPragmaHandler("clang", new PragmaDiagnosticHandler("clang"));
  AddPragmaHandler("clang", new PragmaARCCFCodeAuditedHandler());
  AddPragmaHandler("clang", new PragmaAssumeNonNullHandler());

  AddPragmaHandler("STDC", new PragmaSTDC_FENV_ACCESSHandler());
  AddPragmaHandler("STDC", new PragmaSTDC_CX_LIMITED_RANGEHandler());
  AddPragmaHandler("STDC", new PragmaSTDC_UnknownHandler());

  // MS extensions.
  if (LangOpts.MicrosoftExt) {
    AddPragmaHandler(new PragmaWarningHandler());
    AddPragmaHandler(new PragmaIncludeAliasHandler());
    AddPragmaHandler(new PragmaRegionHandler("region"));
    AddPragmaHandler(new PragmaRegionHandler("endregion"));
  }
}

/// Ignore all pragmas, useful for modes such as -Eonly which would otherwise
/// warn about those pragmas being unknown.
void Preprocessor::IgnorePragmas() {
  AddPragmaHandler(new EmptyPragmaHandler());
  // Also ignore all pragmas in all namespaces created
  // in Preprocessor::RegisterBuiltinPragmas().
  AddPragmaHandler("GCC", new EmptyPragmaHandler());
  AddPragmaHandler("clang", new EmptyPragmaHandler());
  if (PragmaHandler *NS = PragmaHandlers->FindHandler("STDC")) {
    // Preprocessor::RegisterBuiltinPragmas() already registers
    // PragmaSTDC_UnknownHandler as the empty handler, so remove it first,
    // otherwise there will be an assert about a duplicate handler.
    PragmaNamespace *STDCNamespace = NS->getIfNamespace();
    assert(STDCNamespace &&
           "Invalid namespace, registered as a regular pragma handler!");
    if (PragmaHandler *Existing = STDCNamespace->FindHandler("", false)) {
      RemovePragmaHandler("STDC", Existing);
      delete Existing;
    }
  }
  AddPragmaHandler("STDC", new EmptyPragmaHandler());
}
