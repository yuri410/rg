//===--- UnwrappedLineFormatter.cpp - Format C++ code ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UnwrappedLineFormatter.h"
#include "WhitespaceManager.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "format-formatter"

namespace clang {
namespace format {

namespace {

bool startsExternCBlock(const AnnotatedLine &Line) {
  const FormatToken *Next = Line.First->getNextNonComment();
  const FormatToken *NextNext = Next ? Next->getNextNonComment() : nullptr;
  return Line.startsWith(tok::kw_extern) && Next && Next->isStringLiteral() &&
         NextNext && NextNext->is(tok::l_brace);
}

/// \brief Tracks the indent level of \c AnnotatedLines across levels.
///
/// \c nextLine must be called for each \c AnnotatedLine, after which \c
/// getIndent() will return the indent for the last line \c nextLine was called
/// with.
/// If the line is not formatted (and thus the indent does not change), calling
/// \c adjustToUnmodifiedLine after the call to \c nextLine will cause
/// subsequent lines on the same level to be indented at the same level as the
/// given line.
class LevelIndentTracker {
public:
  LevelIndentTracker(const FormatStyle &Style,
                     const AdditionalKeywords &Keywords, unsigned StartLevel,
                     int AdditionalIndent)
      : Style(Style), Keywords(Keywords), AdditionalIndent(AdditionalIndent) {
    for (unsigned i = 0; i != StartLevel; ++i)
      IndentForLevel.push_back(Style.IndentWidth * i + AdditionalIndent);
  }

  /// \brief Returns the indent for the current line.
  unsigned getIndent() const { return Indent; }

  /// \brief Update the indent state given that \p Line is going to be formatted
  /// next.
  void nextLine(const AnnotatedLine &Line) {
    Offset = getIndentOffset(*Line.First);
    // Update the indent level cache size so that we can rely on it
    // having the right size in adjustToUnmodifiedline.
    while (IndentForLevel.size() <= Line.Level)
      IndentForLevel.push_back(-1);
    if (Line.InPPDirective) {
      Indent = Line.Level * Style.IndentWidth + AdditionalIndent;
    } else {
      IndentForLevel.resize(Line.Level + 1);
      Indent = getIndent(IndentForLevel, Line.Level);
    }
    if (static_cast<int>(Indent) + Offset >= 0)
      Indent += Offset;
  }

  /// \brief Update the level indent to adapt to the given \p Line.
  ///
  /// When a line is not formatted, we move the subsequent lines on the same
  /// level to the same indent.
  /// Note that \c nextLine must have been called before this method.
  void adjustToUnmodifiedLine(const AnnotatedLine &Line) {
    unsigned LevelIndent = Line.First->OriginalColumn;
    if (static_cast<int>(LevelIndent) - Offset >= 0)
      LevelIndent -= Offset;
    if ((!Line.First->is(tok::comment) || IndentForLevel[Line.Level] == -1) &&
        !Line.InPPDirective)
      IndentForLevel[Line.Level] = LevelIndent;
  }

private:
  /// \brief Get the offset of the line relatively to the level.
  ///
  /// For example, 'public:' labels in classes are offset by 1 or 2
  /// characters to the left from their level.
  int getIndentOffset(const FormatToken &RootToken) {
    if (Style.Language == FormatStyle::LK_Java ||
        Style.Language == FormatStyle::LK_JavaScript)
      return 0;
    if (RootToken.isAccessSpecifier(false) ||
        RootToken.isObjCAccessSpecifier() ||
        (RootToken.is(Keywords.kw_signals) && RootToken.Next &&
         RootToken.Next->is(tok::colon)))
      return Style.AccessModifierOffset;
    return 0;
  }

  /// \brief Get the indent of \p Level from \p IndentForLevel.
  ///
  /// \p IndentForLevel must contain the indent for the level \c l
  /// at \p IndentForLevel[l], or a value < 0 if the indent for
  /// that level is unknown.
  unsigned getIndent(ArrayRef<int> IndentForLevel, unsigned Level) {
    if (IndentForLevel[Level] != -1)
      return IndentForLevel[Level];
    if (Level == 0)
      return 0;
    return getIndent(IndentForLevel, Level - 1) + Style.IndentWidth;
  }

  const FormatStyle &Style;
  const AdditionalKeywords &Keywords;
  const unsigned AdditionalIndent;

  /// \brief The indent in characters for each level.
  std::vector<int> IndentForLevel;

  /// \brief Offset of the current line relative to the indent level.
  ///
  /// For example, the 'public' keywords is often indented with a negative
  /// offset.
  int Offset = 0;

  /// \brief The current line's indent.
  unsigned Indent = 0;
};

class LineJoiner {
public:
  LineJoiner(const FormatStyle &Style, const AdditionalKeywords &Keywords,
             const SmallVectorImpl<AnnotatedLine *> &Lines)
      : Style(Style), Keywords(Keywords), End(Lines.end()),
        Next(Lines.begin()) {}

  /// \brief Returns the next line, merging multiple lines into one if possible.
  const AnnotatedLine *getNextMergedLine(bool DryRun,
                                         LevelIndentTracker &IndentTracker) {
    if (Next == End)
      return nullptr;
    const AnnotatedLine *Current = *Next;
    IndentTracker.nextLine(*Current);
    unsigned MergedLines =
        tryFitMultipleLinesInOne(IndentTracker.getIndent(), Next, End);
    if (MergedLines > 0 && Style.ColumnLimit == 0)
      // Disallow line merging if there is a break at the start of one of the
      // input lines.
      for (unsigned i = 0; i < MergedLines; ++i)
        if (Next[i + 1]->First->NewlinesBefore > 0)
          MergedLines = 0;
    if (!DryRun)
      for (unsigned i = 0; i < MergedLines; ++i)
        join(*Next[i], *Next[i + 1]);
    Next = Next + MergedLines + 1;
    return Current;
  }

private:
  /// \brief Calculates how many lines can be merged into 1 starting at \p I.
  unsigned
  tryFitMultipleLinesInOne(unsigned Indent,
                           SmallVectorImpl<AnnotatedLine *>::const_iterator I,
                           SmallVectorImpl<AnnotatedLine *>::const_iterator E) {
    // Can't join the last line with anything.
    if (I + 1 == E)
      return 0;
    // We can never merge stuff if there are trailing line comments.
    const AnnotatedLine *TheLine = *I;
    if (TheLine->Last->is(TT_LineComment))
      return 0;
    if (I[1]->Type == LT_Invalid || I[1]->First->MustBreakBefore)
      return 0;
    if (TheLine->InPPDirective &&
        (!I[1]->InPPDirective || I[1]->First->HasUnescapedNewline))
      return 0;

    if (Style.ColumnLimit > 0 && Indent > Style.ColumnLimit)
      return 0;

    unsigned Limit =
        Style.ColumnLimit == 0 ? UINT_MAX : Style.ColumnLimit - Indent;
    // If we already exceed the column limit, we set 'Limit' to 0. The different
    // tryMerge..() functions can then decide whether to still do merging.
    Limit = TheLine->Last->TotalLength > Limit
                ? 0
                : Limit - TheLine->Last->TotalLength;

    // FIXME: TheLine->Level != 0 might or might not be the right check to do.
    // If necessary, change to something smarter.
    bool MergeShortFunctions =
        Style.AllowShortFunctionsOnASingleLine == FormatStyle::SFS_All ||
        (Style.AllowShortFunctionsOnASingleLine >= FormatStyle::SFS_Empty &&
         I[1]->First->is(tok::r_brace)) ||
        (Style.AllowShortFunctionsOnASingleLine == FormatStyle::SFS_Inline &&
         TheLine->Level != 0);

    if (TheLine->Last->is(TT_FunctionLBrace) &&
        TheLine->First != TheLine->Last) {
      return MergeShortFunctions ? tryMergeSimpleBlock(I, E, Limit) : 0;
    }
    if (TheLine->Last->is(tok::l_brace)) {
      return Style.BreakBeforeBraces == FormatStyle::BS_Attach
                 ? tryMergeSimpleBlock(I, E, Limit)
                 : 0;
    }
    if (I[1]->First->is(TT_FunctionLBrace) &&
        Style.BreakBeforeBraces != FormatStyle::BS_Attach) {
      if (I[1]->Last->is(TT_LineComment))
        return 0;

      // Check for Limit <= 2 to account for the " {".
      if (Limit <= 2 || (Style.ColumnLimit == 0 && containsMustBreak(TheLine)))
        return 0;
      Limit -= 2;

      unsigned MergedLines = 0;
      if (MergeShortFunctions) {
        MergedLines = tryMergeSimpleBlock(I + 1, E, Limit);
        // If we managed to merge the block, count the function header, which is
        // on a separate line.
        if (MergedLines > 0)
          ++MergedLines;
      }
      return MergedLines;
    }
    if (TheLine->First->is(tok::kw_if)) {
      return Style.AllowShortIfStatementsOnASingleLine
                 ? tryMergeSimpleControlStatement(I, E, Limit)
                 : 0;
    }
    if (TheLine->First->isOneOf(tok::kw_for, tok::kw_while)) {
      return Style.AllowShortLoopsOnASingleLine
                 ? tryMergeSimpleControlStatement(I, E, Limit)
                 : 0;
    }
    if (TheLine->First->isOneOf(tok::kw_case, tok::kw_default)) {
      return Style.AllowShortCaseLabelsOnASingleLine
                 ? tryMergeShortCaseLabels(I, E, Limit)
                 : 0;
    }
    if (TheLine->InPPDirective &&
        (TheLine->First->HasUnescapedNewline || TheLine->First->IsFirst)) {
      return tryMergeSimplePPDirective(I, E, Limit);
    }
    return 0;
  }

  unsigned
  tryMergeSimplePPDirective(SmallVectorImpl<AnnotatedLine *>::const_iterator I,
                            SmallVectorImpl<AnnotatedLine *>::const_iterator E,
                            unsigned Limit) {
    if (Limit == 0)
      return 0;
    if (I + 2 != E && I[2]->InPPDirective && !I[2]->First->HasUnescapedNewline)
      return 0;
    if (1 + I[1]->Last->TotalLength > Limit)
      return 0;
    return 1;
  }

  unsigned tryMergeSimpleControlStatement(
      SmallVectorImpl<AnnotatedLine *>::const_iterator I,
      SmallVectorImpl<AnnotatedLine *>::const_iterator E, unsigned Limit) {
    if (Limit == 0)
      return 0;
    if ((Style.BreakBeforeBraces == FormatStyle::BS_Allman ||
         Style.BreakBeforeBraces == FormatStyle::BS_GNU) &&
        (I[1]->First->is(tok::l_brace) && !Style.AllowShortBlocksOnASingleLine))
      return 0;
    if (I[1]->InPPDirective != (*I)->InPPDirective ||
        (I[1]->InPPDirective && I[1]->First->HasUnescapedNewline))
      return 0;
    Limit = limitConsideringMacros(I + 1, E, Limit);
    AnnotatedLine &Line = **I;
    if (Line.Last->isNot(tok::r_paren))
      return 0;
    if (1 + I[1]->Last->TotalLength > Limit)
      return 0;
    if (I[1]->First->isOneOf(tok::semi, tok::kw_if, tok::kw_for, tok::kw_while,
                             TT_LineComment))
      return 0;
    // Only inline simple if's (no nested if or else).
    if (I + 2 != E && Line.startsWith(tok::kw_if) &&
        I[2]->First->is(tok::kw_else))
      return 0;
    return 1;
  }

  unsigned
  tryMergeShortCaseLabels(SmallVectorImpl<AnnotatedLine *>::const_iterator I,
                          SmallVectorImpl<AnnotatedLine *>::const_iterator E,
                          unsigned Limit) {
    if (Limit == 0 || I + 1 == E ||
        I[1]->First->isOneOf(tok::kw_case, tok::kw_default))
      return 0;
    unsigned NumStmts = 0;
    unsigned Length = 0;
    bool InPPDirective = I[0]->InPPDirective;
    for (; NumStmts < 3; ++NumStmts) {
      if (I + 1 + NumStmts == E)
        break;
      const AnnotatedLine *Line = I[1 + NumStmts];
      if (Line->InPPDirective != InPPDirective)
        break;
      if (Line->First->isOneOf(tok::kw_case, tok::kw_default, tok::r_brace))
        break;
      if (Line->First->isOneOf(tok::kw_if, tok::kw_for, tok::kw_switch,
                               tok::kw_while, tok::comment))
        return 0;
      Length += I[1 + NumStmts]->Last->TotalLength + 1; // 1 for the space.
    }
    if (NumStmts == 0 || NumStmts == 3 || Length > Limit)
      return 0;
    return NumStmts;
  }

  unsigned
  tryMergeSimpleBlock(SmallVectorImpl<AnnotatedLine *>::const_iterator I,
                      SmallVectorImpl<AnnotatedLine *>::const_iterator E,
                      unsigned Limit) {
    AnnotatedLine &Line = **I;

    // Don't merge ObjC @ keywords and methods.
    // FIXME: If an option to allow short exception handling clauses on a single
    // line is added, change this to not return for @try and friends.
    if (Style.Language != FormatStyle::LK_Java &&
        Line.First->isOneOf(tok::at, tok::minus, tok::plus))
      return 0;

    // Check that the current line allows merging. This depends on whether we
    // are in a control flow statements as well as several style flags.
    if (Line.First->isOneOf(tok::kw_else, tok::kw_case) ||
        (Line.First->Next && Line.First->Next->is(tok::kw_else)))
      return 0;
    if (Line.First->isOneOf(tok::kw_if, tok::kw_while, tok::kw_do, tok::kw_try,
                            tok::kw___try, tok::kw_catch, tok::kw___finally,
                            tok::kw_for, tok::r_brace, Keywords.kw___except)) {
      if (!Style.AllowShortBlocksOnASingleLine)
        return 0;
      if (!Style.AllowShortIfStatementsOnASingleLine &&
          Line.startsWith(tok::kw_if))
        return 0;
      if (!Style.AllowShortLoopsOnASingleLine &&
          Line.First->isOneOf(tok::kw_while, tok::kw_do, tok::kw_for))
        return 0;
      // FIXME: Consider an option to allow short exception handling clauses on
      // a single line.
      // FIXME: This isn't covered by tests.
      // FIXME: For catch, __except, __finally the first token on the line
      // is '}', so this isn't correct here.
      if (Line.First->isOneOf(tok::kw_try, tok::kw___try, tok::kw_catch,
                              Keywords.kw___except, tok::kw___finally))
        return 0;
    }

    FormatToken *Tok = I[1]->First;
    if (Tok->is(tok::r_brace) && !Tok->MustBreakBefore &&
        (Tok->getNextNonComment() == nullptr ||
         Tok->getNextNonComment()->is(tok::semi))) {
      // We merge empty blocks even if the line exceeds the column limit.
      Tok->SpacesRequiredBefore = 0;
      Tok->CanBreakBefore = true;
      return 1;
    } else if (Limit != 0 && !Line.startsWith(tok::kw_namespace) &&
               !startsExternCBlock(Line)) {
      // We don't merge short records.
      if (Line.First->isOneOf(tok::kw_class, tok::kw_union, tok::kw_struct,
                              Keywords.kw_interface))
        return 0;

      // Check that we still have three lines and they fit into the limit.
      if (I + 2 == E || I[2]->Type == LT_Invalid)
        return 0;
      Limit = limitConsideringMacros(I + 2, E, Limit);

      if (!nextTwoLinesFitInto(I, Limit))
        return 0;

      // Second, check that the next line does not contain any braces - if it
      // does, readability declines when putting it into a single line.
      if (I[1]->Last->is(TT_LineComment))
        return 0;
      do {
        if (Tok->is(tok::l_brace) && Tok->BlockKind != BK_BracedInit)
          return 0;
        Tok = Tok->Next;
      } while (Tok);

      // Last, check that the third line starts with a closing brace.
      Tok = I[2]->First;
      if (Tok->isNot(tok::r_brace))
        return 0;

      // Don't merge "if (a) { .. } else {".
      if (Tok->Next && Tok->Next->is(tok::kw_else))
        return 0;

      return 2;
    }
    return 0;
  }

  /// Returns the modified column limit for \p I if it is inside a macro and
  /// needs a trailing '\'.
  unsigned
  limitConsideringMacros(SmallVectorImpl<AnnotatedLine *>::const_iterator I,
                         SmallVectorImpl<AnnotatedLine *>::const_iterator E,
                         unsigned Limit) {
    if (I[0]->InPPDirective && I + 1 != E &&
        !I[1]->First->HasUnescapedNewline && !I[1]->First->is(tok::eof)) {
      return Limit < 2 ? 0 : Limit - 2;
    }
    return Limit;
  }

  bool nextTwoLinesFitInto(SmallVectorImpl<AnnotatedLine *>::const_iterator I,
                           unsigned Limit) {
    if (I[1]->First->MustBreakBefore || I[2]->First->MustBreakBefore)
      return false;
    return 1 + I[1]->Last->TotalLength + 1 + I[2]->Last->TotalLength <= Limit;
  }

  bool containsMustBreak(const AnnotatedLine *Line) {
    for (const FormatToken *Tok = Line->First; Tok; Tok = Tok->Next) {
      if (Tok->MustBreakBefore)
        return true;
    }
    return false;
  }

  void join(AnnotatedLine &A, const AnnotatedLine &B) {
    assert(!A.Last->Next);
    assert(!B.First->Previous);
    if (B.Affected)
      A.Affected = true;
    A.Last->Next = B.First;
    B.First->Previous = A.Last;
    B.First->CanBreakBefore = true;
    unsigned LengthA = A.Last->TotalLength + B.First->SpacesRequiredBefore;
    for (FormatToken *Tok = B.First; Tok; Tok = Tok->Next) {
      Tok->TotalLength += LengthA;
      A.Last = Tok;
    }
  }

  const FormatStyle &Style;
  const AdditionalKeywords &Keywords;
  const SmallVectorImpl<AnnotatedLine *>::const_iterator End;

  SmallVectorImpl<AnnotatedLine *>::const_iterator Next;
};

static void markFinalized(FormatToken *Tok) {
  for (; Tok; Tok = Tok->Next) {
    Tok->Finalized = true;
    for (AnnotatedLine *Child : Tok->Children)
      markFinalized(Child->First);
  }
}

#ifndef NDEBUG
static void printLineState(const LineState &State) {
  llvm::dbgs() << "State: ";
  for (const ParenState &P : State.Stack) {
    llvm::dbgs() << P.Indent << "|" << P.LastSpace << "|" << P.NestedBlockIndent
                 << " ";
  }
  llvm::dbgs() << State.NextToken->TokenText << "\n";
}
#endif

/// \brief Base class for classes that format one \c AnnotatedLine.
class LineFormatter {
public:
  LineFormatter(ContinuationIndenter *Indenter, WhitespaceManager *Whitespaces,
                const FormatStyle &Style,
                UnwrappedLineFormatter *BlockFormatter)
      : Indenter(Indenter), Whitespaces(Whitespaces), Style(Style),
        BlockFormatter(BlockFormatter) {}
  virtual ~LineFormatter() {}

  /// \brief Formats an \c AnnotatedLine and returns the penalty.
  ///
  /// If \p DryRun is \c false, directly applies the changes.
  virtual unsigned formatLine(const AnnotatedLine &Line, unsigned FirstIndent,
                              bool DryRun) = 0;

protected:
  /// \brief If the \p State's next token is an r_brace closing a nested block,
  /// format the nested block before it.
  ///
  /// Returns \c true if all children could be placed successfully and adapts
  /// \p Penalty as well as \p State. If \p DryRun is false, also directly
  /// creates changes using \c Whitespaces.
  ///
  /// The crucial idea here is that children always get formatted upon
  /// encountering the closing brace right after the nested block. Now, if we
  /// are currently trying to keep the "}" on the same line (i.e. \p NewLine is
  /// \c false), the entire block has to be kept on the same line (which is only
  /// possible if it fits on the line, only contains a single statement, etc.
  ///
  /// If \p NewLine is true, we format the nested block on separate lines, i.e.
  /// break after the "{", format all lines with correct indentation and the put
  /// the closing "}" on yet another new line.
  ///
  /// This enables us to keep the simple structure of the
  /// \c UnwrappedLineFormatter, where we only have two options for each token:
  /// break or don't break.
  bool formatChildren(LineState &State, bool NewLine, bool DryRun,
                      unsigned &Penalty) {
    const FormatToken *LBrace = State.NextToken->getPreviousNonComment();
    FormatToken &Previous = *State.NextToken->Previous;
    if (!LBrace || LBrace->isNot(tok::l_brace) ||
        LBrace->BlockKind != BK_Block || Previous.Children.size() == 0)
      // The previous token does not open a block. Nothing to do. We don't
      // assert so that we can simply call this function for all tokens.
      return true;

    if (NewLine) {
      int AdditionalIndent = State.Stack.back().Indent -
                             Previous.Children[0]->Level * Style.IndentWidth;

      Penalty +=
          BlockFormatter->format(Previous.Children, DryRun, AdditionalIndent,
                                 /*FixBadIndentation=*/true);
      return true;
    }

    if (Previous.Children[0]->First->MustBreakBefore)
      return false;

    // Cannot merge multiple statements into a single line.
    if (Previous.Children.size() > 1)
      return false;

    // Cannot merge into one line if this line ends on a comment.
    if (Previous.is(tok::comment))
      return false;

    // We can't put the closing "}" on a line with a trailing comment.
    if (Previous.Children[0]->Last->isTrailingComment())
      return false;

    // If the child line exceeds the column limit, we wouldn't want to merge it.
    // We add +2 for the trailing " }".
    if (Style.ColumnLimit > 0 &&
        Previous.Children[0]->Last->TotalLength + State.Column + 2 >
            Style.ColumnLimit)
      return false;

    if (!DryRun) {
      Whitespaces->replaceWhitespace(
          *Previous.Children[0]->First,
          /*Newlines=*/0, /*IndentLevel=*/0, /*Spaces=*/1,
          /*StartOfTokenColumn=*/State.Column, State.Line->InPPDirective);
    }
    Penalty += formatLine(*Previous.Children[0], State.Column + 1, DryRun);

    State.Column += 1 + Previous.Children[0]->Last->TotalLength;
    return true;
  }

  ContinuationIndenter *Indenter;

private:
  WhitespaceManager *Whitespaces;
  const FormatStyle &Style;
  UnwrappedLineFormatter *BlockFormatter;
};

/// \brief Formatter that keeps the existing line breaks.
class NoColumnLimitLineFormatter : public LineFormatter {
public:
  NoColumnLimitLineFormatter(ContinuationIndenter *Indenter,
                             WhitespaceManager *Whitespaces,
                             const FormatStyle &Style,
                             UnwrappedLineFormatter *BlockFormatter)
      : LineFormatter(Indenter, Whitespaces, Style, BlockFormatter) {}

  /// \brief Formats the line, simply keeping all of the input's line breaking
  /// decisions.
  unsigned formatLine(const AnnotatedLine &Line, unsigned FirstIndent,
                      bool DryRun) override {
    assert(!DryRun);
    LineState State =
        Indenter->getInitialState(FirstIndent, &Line, /*DryRun=*/false);
    while (State.NextToken) {
      bool Newline =
          Indenter->mustBreak(State) ||
          (Indenter->canBreak(State) && State.NextToken->NewlinesBefore > 0);
      unsigned Penalty = 0;
      formatChildren(State, Newline, /*DryRun=*/false, Penalty);
      Indenter->addTokenToState(State, Newline, /*DryRun=*/false);
    }
    return 0;
  }
};

/// \brief Formatter that puts all tokens into a single line without breaks.
class NoLineBreakFormatter : public LineFormatter {
public:
  NoLineBreakFormatter(ContinuationIndenter *Indenter,
                       WhitespaceManager *Whitespaces, const FormatStyle &Style,
                       UnwrappedLineFormatter *BlockFormatter)
      : LineFormatter(Indenter, Whitespaces, Style, BlockFormatter) {}

  /// \brief Puts all tokens into a single line.
  unsigned formatLine(const AnnotatedLine &Line, unsigned FirstIndent,
                      bool DryRun) {
    unsigned Penalty = 0;
    LineState State = Indenter->getInitialState(FirstIndent, &Line, DryRun);
    while (State.NextToken) {
      formatChildren(State, /*Newline=*/false, DryRun, Penalty);
      Indenter->addTokenToState(State, /*Newline=*/false, DryRun);
    }
    return Penalty;
  }
};

/// \brief Finds the best way to break lines.
class OptimizingLineFormatter : public LineFormatter {
public:
  OptimizingLineFormatter(ContinuationIndenter *Indenter,
                          WhitespaceManager *Whitespaces,
                          const FormatStyle &Style,
                          UnwrappedLineFormatter *BlockFormatter)
      : LineFormatter(Indenter, Whitespaces, Style, BlockFormatter) {}

  /// \brief Formats the line by finding the best line breaks with line lengths
  /// below the column limit.
  unsigned formatLine(const AnnotatedLine &Line, unsigned FirstIndent,
                      bool DryRun) {
    LineState State = Indenter->getInitialState(FirstIndent, &Line, DryRun);

    // If the ObjC method declaration does not fit on a line, we should format
    // it with one arg per line.
    if (State.Line->Type == LT_ObjCMethodDecl)
      State.Stack.back().BreakBeforeParameter = true;

    // Find best solution in solution space.
    return analyzeSolutionSpace(State, DryRun);
  }

private:
  struct CompareLineStatePointers {
    bool operator()(LineState *obj1, LineState *obj2) const {
      return *obj1 < *obj2;
    }
  };

  /// \brief A pair of <penalty, count> that is used to prioritize the BFS on.
  ///
  /// In case of equal penalties, we want to prefer states that were inserted
  /// first. During state generation we make sure that we insert states first
  /// that break the line as late as possible.
  typedef std::pair<unsigned, unsigned> OrderedPenalty;

  /// \brief An edge in the solution space from \c Previous->State to \c State,
  /// inserting a newline dependent on the \c NewLine.
  struct StateNode {
    StateNode(const LineState &State, bool NewLine, StateNode *Previous)
        : State(State), NewLine(NewLine), Previous(Previous) {}
    LineState State;
    bool NewLine;
    StateNode *Previous;
  };

  /// \brief An item in the prioritized BFS search queue. The \c StateNode's
  /// \c State has the given \c OrderedPenalty.
  typedef std::pair<OrderedPenalty, StateNode *> QueueItem;

  /// \brief The BFS queue type.
  typedef std::priority_queue<QueueItem, std::vector<QueueItem>,
                              std::greater<QueueItem>> QueueType;

  /// \brief Analyze the entire solution space starting from \p InitialState.
  ///
  /// This implements a variant of Dijkstra's algorithm on the graph that spans
  /// the solution space (\c LineStates are the nodes). The algorithm tries to
  /// find the shortest path (the one with lowest penalty) from \p InitialState
  /// to a state where all tokens are placed. Returns the penalty.
  ///
  /// If \p DryRun is \c false, directly applies the changes.
  unsigned analyzeSolutionSpace(LineState &InitialState, bool DryRun) {
    std::set<LineState *, CompareLineStatePointers> Seen;

    // Increasing count of \c StateNode items we have created. This is used to
    // create a deterministic order independent of the container.
    unsigned Count = 0;
    QueueType Queue;

    // Insert start element into queue.
    StateNode *Node =
        new (Allocator.Allocate()) StateNode(InitialState, false, nullptr);
    Queue.push(QueueItem(OrderedPenalty(0, Count), Node));
    ++Count;

    unsigned Penalty = 0;

    // While not empty, take first element and follow edges.
    while (!Queue.empty()) {
      Penalty = Queue.top().first.first;
      StateNode *Node = Queue.top().second;
      if (!Node->State.NextToken) {
        DEBUG(llvm::dbgs() << "\n---\nPenalty for line: " << Penalty << "\n");
        break;
      }
      Queue.pop();

      // Cut off the analysis of certain solutions if the analysis gets too
      // complex. See description of IgnoreStackForComparison.
      if (Count > 10000)
        Node->State.IgnoreStackForComparison = true;

      if (!Seen.insert(&Node->State).second)
        // State already examined with lower penalty.
        continue;

      FormatDecision LastFormat = Node->State.NextToken->Decision;
      if (LastFormat == FD_Unformatted || LastFormat == FD_Continue)
        addNextStateToQueue(Penalty, Node, /*NewLine=*/false, &Count, &Queue);
      if (LastFormat == FD_Unformatted || LastFormat == FD_Break)
        addNextStateToQueue(Penalty, Node, /*NewLine=*/true, &Count, &Queue);
    }

    if (Queue.empty()) {
      // We were unable to find a solution, do nothing.
      // FIXME: Add diagnostic?
      DEBUG(llvm::dbgs() << "Could not find a solution.\n");
      return 0;
    }

    // Reconstruct the solution.
    if (!DryRun)
      reconstructPath(InitialState, Queue.top().second);

    DEBUG(llvm::dbgs() << "Total number of analyzed states: " << Count << "\n");
    DEBUG(llvm::dbgs() << "---\n");

    return Penalty;
  }

  /// \brief Add the following state to the analysis queue \c Queue.
  ///
  /// Assume the current state is \p PreviousNode and has been reached with a
  /// penalty of \p Penalty. Insert a line break if \p NewLine is \c true.
  void addNextStateToQueue(unsigned Penalty, StateNode *PreviousNode,
                           bool NewLine, unsigned *Count, QueueType *Queue) {
    if (NewLine && !Indenter->canBreak(PreviousNode->State))
      return;
    if (!NewLine && Indenter->mustBreak(PreviousNode->State))
      return;

    StateNode *Node = new (Allocator.Allocate())
        StateNode(PreviousNode->State, NewLine, PreviousNode);
    if (!formatChildren(Node->State, NewLine, /*DryRun=*/true, Penalty))
      return;

    Penalty += Indenter->addTokenToState(Node->State, NewLine, true);

    Queue->push(QueueItem(OrderedPenalty(Penalty, *Count), Node));
    ++(*Count);
  }

  /// \brief Applies the best formatting by reconstructing the path in the
  /// solution space that leads to \c Best.
  void reconstructPath(LineState &State, StateNode *Best) {
    std::deque<StateNode *> Path;
    // We do not need a break before the initial token.
    while (Best->Previous) {
      Path.push_front(Best);
      Best = Best->Previous;
    }
    for (std::deque<StateNode *>::iterator I = Path.begin(), E = Path.end();
         I != E; ++I) {
      unsigned Penalty = 0;
      formatChildren(State, (*I)->NewLine, /*DryRun=*/false, Penalty);
      Penalty += Indenter->addTokenToState(State, (*I)->NewLine, false);

      DEBUG({
        printLineState((*I)->Previous->State);
        if ((*I)->NewLine) {
          llvm::dbgs() << "Penalty for placing "
                       << (*I)->Previous->State.NextToken->Tok.getName() << ": "
                       << Penalty << "\n";
        }
      });
    }
  }

  llvm::SpecificBumpPtrAllocator<StateNode> Allocator;
};

} // namespace

unsigned
UnwrappedLineFormatter::format(const SmallVectorImpl<AnnotatedLine *> &Lines,
                               bool DryRun, int AdditionalIndent,
                               bool FixBadIndentation) {
  LineJoiner Joiner(Style, Keywords, Lines);

  // Try to look up already computed penalty in DryRun-mode.
  std::pair<const SmallVectorImpl<AnnotatedLine *> *, unsigned> CacheKey(
      &Lines, AdditionalIndent);
  auto CacheIt = PenaltyCache.find(CacheKey);
  if (DryRun && CacheIt != PenaltyCache.end())
    return CacheIt->second;

  assert(!Lines.empty());
  unsigned Penalty = 0;
  LevelIndentTracker IndentTracker(Style, Keywords, Lines[0]->Level,
                                   AdditionalIndent);
  const AnnotatedLine *PreviousLine = nullptr;
  const AnnotatedLine *NextLine = nullptr;
  for (const AnnotatedLine *Line =
           Joiner.getNextMergedLine(DryRun, IndentTracker);
       Line; Line = NextLine) {
    const AnnotatedLine &TheLine = *Line;
    unsigned Indent = IndentTracker.getIndent();
    bool FixIndentation =
        FixBadIndentation && (Indent != TheLine.First->OriginalColumn);
    bool ShouldFormat = TheLine.Affected || FixIndentation;
    // We cannot format this line; if the reason is that the line had a
    // parsing error, remember that.
    if (ShouldFormat && TheLine.Type == LT_Invalid && IncompleteFormat)
      *IncompleteFormat = true;

    if (ShouldFormat && TheLine.Type != LT_Invalid) {
      if (!DryRun)
        formatFirstToken(*TheLine.First, PreviousLine, TheLine.Level, Indent,
                         TheLine.InPPDirective);

      NextLine = Joiner.getNextMergedLine(DryRun, IndentTracker);
      unsigned ColumnLimit = getColumnLimit(TheLine.InPPDirective, NextLine);
      bool FitsIntoOneLine =
          TheLine.Last->TotalLength + Indent <= ColumnLimit ||
          TheLine.Type == LT_ImportStatement;

      if (Style.ColumnLimit == 0)
        NoColumnLimitLineFormatter(Indenter, Whitespaces, Style, this)
            .formatLine(TheLine, Indent, DryRun);
      else if (FitsIntoOneLine)
        Penalty += NoLineBreakFormatter(Indenter, Whitespaces, Style, this)
                       .formatLine(TheLine, Indent, DryRun);
      else
        Penalty += OptimizingLineFormatter(Indenter, Whitespaces, Style, this)
                       .formatLine(TheLine, Indent, DryRun);
    } else {
      // If no token in the current line is affected, we still need to format
      // affected children.
      if (TheLine.ChildrenAffected)
        format(TheLine.Children, DryRun);

      // Adapt following lines on the current indent level to the same level
      // unless the current \c AnnotatedLine is not at the beginning of a line.
      bool StartsNewLine =
          TheLine.First->NewlinesBefore > 0 || TheLine.First->IsFirst;
      if (StartsNewLine)
        IndentTracker.adjustToUnmodifiedLine(TheLine);
      if (!DryRun) {
        bool ReformatLeadingWhitespace =
            StartsNewLine && ((PreviousLine && PreviousLine->Affected) ||
                              TheLine.LeadingEmptyLinesAffected);
        // Format the first token.
        if (ReformatLeadingWhitespace)
          formatFirstToken(*TheLine.First, PreviousLine, TheLine.Level,
                           TheLine.First->OriginalColumn,
                           TheLine.InPPDirective);
        else
          Whitespaces->addUntouchableToken(*TheLine.First,
                                           TheLine.InPPDirective);

        // Notify the WhitespaceManager about the unchanged whitespace.
        for (FormatToken *Tok = TheLine.First->Next; Tok; Tok = Tok->Next)
          Whitespaces->addUntouchableToken(*Tok, TheLine.InPPDirective);
      }
      NextLine = Joiner.getNextMergedLine(DryRun, IndentTracker);
    }
    if (!DryRun)
      markFinalized(TheLine.First);
    PreviousLine = &TheLine;
  }
  PenaltyCache[CacheKey] = Penalty;
  return Penalty;
}

void UnwrappedLineFormatter::formatFirstToken(FormatToken &RootToken,
                                              const AnnotatedLine *PreviousLine,
                                              unsigned IndentLevel,
                                              unsigned Indent,
                                              bool InPPDirective) {
  if (RootToken.is(tok::eof)) {
    unsigned Newlines = std::min(RootToken.NewlinesBefore, 1u);
    Whitespaces->replaceWhitespace(RootToken, Newlines, /*IndentLevel=*/0,
                                   /*Spaces=*/0, /*TargetColumn=*/0);
    return;
  }
  unsigned Newlines =
      std::min(RootToken.NewlinesBefore, Style.MaxEmptyLinesToKeep + 1);
  // Remove empty lines before "}" where applicable.
  if (RootToken.is(tok::r_brace) &&
      (!RootToken.Next ||
       (RootToken.Next->is(tok::semi) && !RootToken.Next->Next)))
    Newlines = std::min(Newlines, 1u);
  if (Newlines == 0 && !RootToken.IsFirst)
    Newlines = 1;
  if (RootToken.IsFirst && !RootToken.HasUnescapedNewline)
    Newlines = 0;

  // Remove empty lines after "{".
  if (!Style.KeepEmptyLinesAtTheStartOfBlocks && PreviousLine &&
      PreviousLine->Last->is(tok::l_brace) &&
      PreviousLine->First->isNot(tok::kw_namespace) &&
      !startsExternCBlock(*PreviousLine))
    Newlines = 1;

  // Insert extra new line before access specifiers.
  if (PreviousLine && PreviousLine->Last->isOneOf(tok::semi, tok::r_brace) &&
      RootToken.isAccessSpecifier() && RootToken.NewlinesBefore == 1)
    ++Newlines;

  // Remove empty lines after access specifiers.
  if (PreviousLine && PreviousLine->First->isAccessSpecifier() &&
      (!PreviousLine->InPPDirective || !RootToken.HasUnescapedNewline))
    Newlines = std::min(1u, Newlines);

  Whitespaces->replaceWhitespace(RootToken, Newlines, IndentLevel, Indent,
                                 Indent, InPPDirective &&
                                             !RootToken.HasUnescapedNewline);
}

unsigned
UnwrappedLineFormatter::getColumnLimit(bool InPPDirective,
                                       const AnnotatedLine *NextLine) const {
  // In preprocessor directives reserve two chars for trailing " \" if the
  // next line continues the preprocessor directive.
  bool ContinuesPPDirective =
      InPPDirective &&
      // If there is no next line, this is likely a child line and the parent
      // continues the preprocessor directive.
      (!NextLine ||
       (NextLine->InPPDirective &&
        // If there is an unescaped newline between this line and the next, the
        // next line starts a new preprocessor directive.
        !NextLine->First->HasUnescapedNewline));
  return Style.ColumnLimit - (ContinuesPPDirective ? 2 : 0);
}

} // namespace format
} // namespace clang
