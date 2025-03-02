//===----- Commit.cpp - A unit of edits -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/Edit/Commit.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Edit/EditedSource.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/PPConditionalDirectiveRecord.h"

using namespace clang;
using namespace edit;

SourceLocation Commit::Edit::getFileLocation(SourceManager &SM) const {
  SourceLocation Loc = SM.getLocForStartOfFile(Offset.getFID());
  Loc = Loc.getLocWithOffset(Offset.getOffset());
  assert(Loc.isFileID());
  return Loc;
}

CharSourceRange Commit::Edit::getFileRange(SourceManager &SM) const {
  SourceLocation Loc = getFileLocation(SM);
  return CharSourceRange::getCharRange(Loc, Loc.getLocWithOffset(Length));
}

CharSourceRange Commit::Edit::getInsertFromRange(SourceManager &SM) const {
  SourceLocation Loc = SM.getLocForStartOfFile(InsertFromRangeOffs.getFID());
  Loc = Loc.getLocWithOffset(InsertFromRangeOffs.getOffset());
  assert(Loc.isFileID());
  return CharSourceRange::getCharRange(Loc, Loc.getLocWithOffset(Length));
}

Commit::Commit(EditedSource &Editor)
  : SourceMgr(Editor.getSourceManager()), LangOpts(Editor.getLangOpts()),
    PPRec(Editor.getPPCondDirectiveRecord()),
    Editor(&Editor), IsCommitable(true) { }

bool Commit::insert(SourceLocation loc, StringRef text,
                    bool afterToken, bool beforePreviousInsertions) {
  if (text.empty())
    return true;

  FileOffset Offs;
  if ((!afterToken && !canInsert(loc, Offs)) ||
      ( afterToken && !canInsertAfterToken(loc, Offs, loc))) {
    IsCommitable = false;
    return false;
  }

  addInsert(loc, Offs, text, beforePreviousInsertions);
  return true;
}

bool Commit::insertFromRange(SourceLocation loc,
                             CharSourceRange range,
                             bool afterToken, bool beforePreviousInsertions) {
  FileOffset RangeOffs;
  unsigned RangeLen;
  if (!canRemoveRange(range, RangeOffs, RangeLen)) {
    IsCommitable = false;
    return false;
  }

  FileOffset Offs;
  if ((!afterToken && !canInsert(loc, Offs)) ||
      ( afterToken && !canInsertAfterToken(loc, Offs, loc))) {
    IsCommitable = false;
    return false;
  }

  if (PPRec &&
      PPRec->areInDifferentConditionalDirectiveRegion(loc, range.getBegin())) {
    IsCommitable = false;
    return false;
  }

  addInsertFromRange(loc, Offs, RangeOffs, RangeLen, beforePreviousInsertions);
  return true;
}

bool Commit::remove(CharSourceRange range) {
  FileOffset Offs;
  unsigned Len;
  if (!canRemoveRange(range, Offs, Len)) {
    IsCommitable = false;
    return false;
  }

  addRemove(range.getBegin(), Offs, Len);
  return true;
}

bool Commit::insertWrap(StringRef before, CharSourceRange range,
                        StringRef after) {
  bool commitableBefore = insert(range.getBegin(), before, /*afterToken=*/false,
                                 /*beforePreviousInsertions=*/true);
  bool commitableAfter;
  if (range.isTokenRange())
    commitableAfter = insertAfterToken(range.getEnd(), after);
  else
    commitableAfter = insert(range.getEnd(), after);

  return commitableBefore && commitableAfter;
}

bool Commit::replace(CharSourceRange range, StringRef text) {
  if (text.empty())
    return remove(range);

  FileOffset Offs;
  unsigned Len;
  if (!canInsert(range.getBegin(), Offs) || !canRemoveRange(range, Offs, Len)) {
    IsCommitable = false;
    return false;
  }

  addRemove(range.getBegin(), Offs, Len);
  addInsert(range.getBegin(), Offs, text, false);
  return true;
}

bool Commit::replaceWithInner(CharSourceRange range,
                              CharSourceRange replacementRange) {
  FileOffset OuterBegin;
  unsigned OuterLen;
  if (!canRemoveRange(range, OuterBegin, OuterLen)) {
    IsCommitable = false;
    return false;
  }

  FileOffset InnerBegin;
  unsigned InnerLen;
  if (!canRemoveRange(replacementRange, InnerBegin, InnerLen)) {
    IsCommitable = false;
    return false;
  }

  FileOffset OuterEnd = OuterBegin.getWithOffset(OuterLen);
  FileOffset InnerEnd = InnerBegin.getWithOffset(InnerLen); 
  if (OuterBegin.getFID() != InnerBegin.getFID() ||
      InnerBegin < OuterBegin ||
      InnerBegin > OuterEnd ||
      InnerEnd > OuterEnd) {
    IsCommitable = false;
    return false;
  }

  addRemove(range.getBegin(),
            OuterBegin, InnerBegin.getOffset() - OuterBegin.getOffset());
  addRemove(replacementRange.getEnd(),
            InnerEnd, OuterEnd.getOffset() - InnerEnd.getOffset());
  return true;
}

bool Commit::replaceText(SourceLocation loc, StringRef text,
                         StringRef replacementText) {
  if (text.empty() || replacementText.empty())
    return true;

  FileOffset Offs;
  unsigned Len;
  if (!canReplaceText(loc, replacementText, Offs, Len)) {
    IsCommitable = false;
    return false;
  }

  addRemove(loc, Offs, Len);
  addInsert(loc, Offs, text, false);
  return true;
}

void Commit::addInsert(SourceLocation OrigLoc, FileOffset Offs, StringRef text,
                       bool beforePreviousInsertions) {
  if (text.empty())
    return;

  Edit data;
  data.Kind = Act_Insert;
  data.OrigLoc = OrigLoc;
  data.Offset = Offs;
  data.Text = copyString(text);
  data.BeforePrev = beforePreviousInsertions;
  CachedEdits.push_back(data);
}

void Commit::addInsertFromRange(SourceLocation OrigLoc, FileOffset Offs,
                                FileOffset RangeOffs, unsigned RangeLen,
                                bool beforePreviousInsertions) {
  if (RangeLen == 0)
    return;

  Edit data;
  data.Kind = Act_InsertFromRange;
  data.OrigLoc = OrigLoc;
  data.Offset = Offs;
  data.InsertFromRangeOffs = RangeOffs;
  data.Length = RangeLen;
  data.BeforePrev = beforePreviousInsertions;
  CachedEdits.push_back(data);
}

void Commit::addRemove(SourceLocation OrigLoc,
                       FileOffset Offs, unsigned Len) {
  if (Len == 0)
    return;

  Edit data;
  data.Kind = Act_Remove;
  data.OrigLoc = OrigLoc;
  data.Offset = Offs;
  data.Length = Len;
  CachedEdits.push_back(data);
}

bool Commit::canInsert(SourceLocation loc, FileOffset &offs) {
  if (loc.isInvalid())
    return false;

  if (loc.isMacroID())
    isAtStartOfMacroExpansion(loc, &loc);

  const SourceManager &SM = SourceMgr;
  while (SM.isMacroArgExpansion(loc))
    loc = SM.getImmediateSpellingLoc(loc);

  if (loc.isMacroID())
    if (!isAtStartOfMacroExpansion(loc, &loc))
      return false;

  if (SM.isInSystemHeader(loc))
    return false;

  std::pair<FileID, unsigned> locInfo = SM.getDecomposedLoc(loc);
  if (locInfo.first.isInvalid())
    return false;
  offs = FileOffset(locInfo.first, locInfo.second);
  return canInsertInOffset(loc, offs);
}

bool Commit::canInsertAfterToken(SourceLocation loc, FileOffset &offs,
                                 SourceLocation &AfterLoc) {
  if (loc.isInvalid())

    return false;

  SourceLocation spellLoc = SourceMgr.getSpellingLoc(loc);
  unsigned tokLen = Lexer::MeasureTokenLength(spellLoc, SourceMgr, LangOpts);
  AfterLoc = loc.getLocWithOffset(tokLen);

  if (loc.isMacroID())
    isAtEndOfMacroExpansion(loc, &loc);

  const SourceManager &SM = SourceMgr;
  while (SM.isMacroArgExpansion(loc))
    loc = SM.getImmediateSpellingLoc(loc);

  if (loc.isMacroID())
    if (!isAtEndOfMacroExpansion(loc, &loc))
      return false;

  if (SM.isInSystemHeader(loc))
    return false;

  loc = Lexer::getLocForEndOfToken(loc, 0, SourceMgr, LangOpts);
  if (loc.isInvalid())
    return false;

  std::pair<FileID, unsigned> locInfo = SM.getDecomposedLoc(loc);
  if (locInfo.first.isInvalid())
    return false;
  offs = FileOffset(locInfo.first, locInfo.second);
  return canInsertInOffset(loc, offs);
}

bool Commit::canInsertInOffset(SourceLocation OrigLoc, FileOffset Offs) {
  for (unsigned i = 0, e = CachedEdits.size(); i != e; ++i) {
    Edit &act = CachedEdits[i];
    if (act.Kind == Act_Remove) {
      if (act.Offset.getFID() == Offs.getFID() &&
          Offs > act.Offset && Offs < act.Offset.getWithOffset(act.Length))
        return false; // position has been removed.
    }
  }

  if (!Editor)
    return true;
  return Editor->canInsertInOffset(OrigLoc, Offs);
}

bool Commit::canRemoveRange(CharSourceRange range,
                            FileOffset &Offs, unsigned &Len) {
  const SourceManager &SM = SourceMgr;
  range = Lexer::makeFileCharRange(range, SM, LangOpts);
  if (range.isInvalid())
    return false;
  
  if (range.getBegin().isMacroID() || range.getEnd().isMacroID())
    return false;
  if (SM.isInSystemHeader(range.getBegin()) ||
      SM.isInSystemHeader(range.getEnd()))
    return false;

  if (PPRec && PPRec->rangeIntersectsConditionalDirective(range.getAsRange()))
    return false;

  std::pair<FileID, unsigned> beginInfo = SM.getDecomposedLoc(range.getBegin());
  std::pair<FileID, unsigned> endInfo = SM.getDecomposedLoc(range.getEnd());
  if (beginInfo.first != endInfo.first ||
      beginInfo.second > endInfo.second)
    return false;

  Offs = FileOffset(beginInfo.first, beginInfo.second);
  Len = endInfo.second - beginInfo.second;
  return true;
}

bool Commit::canReplaceText(SourceLocation loc, StringRef text,
                            FileOffset &Offs, unsigned &Len) {
  assert(!text.empty());

  if (!canInsert(loc, Offs))
    return false;

  // Try to load the file buffer.
  bool invalidTemp = false;
  StringRef file = SourceMgr.getBufferData(Offs.getFID(), &invalidTemp);
  if (invalidTemp)
    return false;

  Len = text.size();
  return file.substr(Offs.getOffset()).startswith(text);
}

bool Commit::isAtStartOfMacroExpansion(SourceLocation loc,
                                       SourceLocation *MacroBegin) const {
  return Lexer::isAtStartOfMacroExpansion(loc, SourceMgr, LangOpts, MacroBegin);
}
bool Commit::isAtEndOfMacroExpansion(SourceLocation loc,
                                     SourceLocation *MacroEnd) const {
  return Lexer::isAtEndOfMacroExpansion(loc, SourceMgr, LangOpts, MacroEnd);
}
