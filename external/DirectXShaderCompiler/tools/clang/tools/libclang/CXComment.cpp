//===- CXComment.cpp - libclang APIs for manipulating CXComments ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines all libclang APIs related to walking comment AST.
//
//===----------------------------------------------------------------------===//

#include "clang-c/Index.h"
#include "CXComment.h"
#include "CXCursor.h"
#include "CXString.h"
#include "clang-c/Documentation.h"
#include "clang/AST/Decl.h"
#include "clang/Index/CommentToXML.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"
#include <climits>

using namespace clang;
using namespace clang::comments;
using namespace clang::cxcomment;

//  extern "C" {           // HLSL Change -Don't use c linkage.

CXComment clang_Cursor_getParsedComment(CXCursor C) {
  using namespace clang::cxcursor;

  if (!clang_isDeclaration(C.kind))
    return createCXComment(nullptr, nullptr);

  const Decl *D = getCursorDecl(C);
  const ASTContext &Context = getCursorContext(C);
  const FullComment *FC = Context.getCommentForDecl(D, /*PP=*/nullptr);

  return createCXComment(FC, getCursorTU(C));
}

enum CXCommentKind clang_Comment_getKind(CXComment CXC) {
  const Comment *C = getASTNode(CXC);
  if (!C)
    return CXComment_Null;

  switch (C->getCommentKind()) {
  case Comment::NoCommentKind:
    return CXComment_Null;

  case Comment::TextCommentKind:
    return CXComment_Text;

  case Comment::InlineCommandCommentKind:
    return CXComment_InlineCommand;

  case Comment::HTMLStartTagCommentKind:
    return CXComment_HTMLStartTag;

  case Comment::HTMLEndTagCommentKind:
    return CXComment_HTMLEndTag;

  case Comment::ParagraphCommentKind:
    return CXComment_Paragraph;

  case Comment::BlockCommandCommentKind:
    return CXComment_BlockCommand;

  case Comment::ParamCommandCommentKind:
    return CXComment_ParamCommand;

  case Comment::TParamCommandCommentKind:
    return CXComment_TParamCommand;

  case Comment::VerbatimBlockCommentKind:
    return CXComment_VerbatimBlockCommand;

  case Comment::VerbatimBlockLineCommentKind:
    return CXComment_VerbatimBlockLine;

  case Comment::VerbatimLineCommentKind:
    return CXComment_VerbatimLine;

  case Comment::FullCommentKind:
    return CXComment_FullComment;
  }
  llvm_unreachable("unknown CommentKind");
}

unsigned clang_Comment_getNumChildren(CXComment CXC) {
  const Comment *C = getASTNode(CXC);
  if (!C)
    return 0;

  return C->child_count();
}

CXComment clang_Comment_getChild(CXComment CXC, unsigned ChildIdx) {
  const Comment *C = getASTNode(CXC);
  if (!C || ChildIdx >= C->child_count())
    return createCXComment(nullptr, nullptr);

  return createCXComment(*(C->child_begin() + ChildIdx), CXC.TranslationUnit);
}

unsigned clang_Comment_isWhitespace(CXComment CXC) {
  const Comment *C = getASTNode(CXC);
  if (!C)
    return false;

  if (const TextComment *TC = dyn_cast<TextComment>(C))
    return TC->isWhitespace();

  if (const ParagraphComment *PC = dyn_cast<ParagraphComment>(C))
    return PC->isWhitespace();

  return false;
}

unsigned clang_InlineContentComment_hasTrailingNewline(CXComment CXC) {
  const InlineContentComment *ICC = getASTNodeAs<InlineContentComment>(CXC);
  if (!ICC)
    return false;

  return ICC->hasTrailingNewline();
}

CXString clang_TextComment_getText(CXComment CXC) {
  const TextComment *TC = getASTNodeAs<TextComment>(CXC);
  if (!TC)
    return cxstring::createNull();

  return cxstring::createRef(TC->getText());
}

CXString clang_InlineCommandComment_getCommandName(CXComment CXC) {
  const InlineCommandComment *ICC = getASTNodeAs<InlineCommandComment>(CXC);
  if (!ICC)
    return cxstring::createNull();

  const CommandTraits &Traits = getCommandTraits(CXC);
  return cxstring::createRef(ICC->getCommandName(Traits));
}

enum CXCommentInlineCommandRenderKind
clang_InlineCommandComment_getRenderKind(CXComment CXC) {
  const InlineCommandComment *ICC = getASTNodeAs<InlineCommandComment>(CXC);
  if (!ICC)
    return CXCommentInlineCommandRenderKind_Normal;

  switch (ICC->getRenderKind()) {
  case InlineCommandComment::RenderNormal:
    return CXCommentInlineCommandRenderKind_Normal;

  case InlineCommandComment::RenderBold:
    return CXCommentInlineCommandRenderKind_Bold;

  case InlineCommandComment::RenderMonospaced:
    return CXCommentInlineCommandRenderKind_Monospaced;

  case InlineCommandComment::RenderEmphasized:
    return CXCommentInlineCommandRenderKind_Emphasized;
  }
  llvm_unreachable("unknown InlineCommandComment::RenderKind");
}

unsigned clang_InlineCommandComment_getNumArgs(CXComment CXC) {
  const InlineCommandComment *ICC = getASTNodeAs<InlineCommandComment>(CXC);
  if (!ICC)
    return 0;

  return ICC->getNumArgs();
}

CXString clang_InlineCommandComment_getArgText(CXComment CXC,
                                               unsigned ArgIdx) {
  const InlineCommandComment *ICC = getASTNodeAs<InlineCommandComment>(CXC);
  if (!ICC || ArgIdx >= ICC->getNumArgs())
    return cxstring::createNull();

  return cxstring::createRef(ICC->getArgText(ArgIdx));
}

CXString clang_HTMLTagComment_getTagName(CXComment CXC) {
  const HTMLTagComment *HTC = getASTNodeAs<HTMLTagComment>(CXC);
  if (!HTC)
    return cxstring::createNull();

  return cxstring::createRef(HTC->getTagName());
}

unsigned clang_HTMLStartTagComment_isSelfClosing(CXComment CXC) {
  const HTMLStartTagComment *HST = getASTNodeAs<HTMLStartTagComment>(CXC);
  if (!HST)
    return false;

  return HST->isSelfClosing();
}

unsigned clang_HTMLStartTag_getNumAttrs(CXComment CXC) {
  const HTMLStartTagComment *HST = getASTNodeAs<HTMLStartTagComment>(CXC);
  if (!HST)
    return 0;

  return HST->getNumAttrs();
}

CXString clang_HTMLStartTag_getAttrName(CXComment CXC, unsigned AttrIdx) {
  const HTMLStartTagComment *HST = getASTNodeAs<HTMLStartTagComment>(CXC);
  if (!HST || AttrIdx >= HST->getNumAttrs())
    return cxstring::createNull();

  return cxstring::createRef(HST->getAttr(AttrIdx).Name);
}

CXString clang_HTMLStartTag_getAttrValue(CXComment CXC, unsigned AttrIdx) {
  const HTMLStartTagComment *HST = getASTNodeAs<HTMLStartTagComment>(CXC);
  if (!HST || AttrIdx >= HST->getNumAttrs())
    return cxstring::createNull();

  return cxstring::createRef(HST->getAttr(AttrIdx).Value);
}

CXString clang_BlockCommandComment_getCommandName(CXComment CXC) {
  const BlockCommandComment *BCC = getASTNodeAs<BlockCommandComment>(CXC);
  if (!BCC)
    return cxstring::createNull();

  const CommandTraits &Traits = getCommandTraits(CXC);
  return cxstring::createRef(BCC->getCommandName(Traits));
}

unsigned clang_BlockCommandComment_getNumArgs(CXComment CXC) {
  const BlockCommandComment *BCC = getASTNodeAs<BlockCommandComment>(CXC);
  if (!BCC)
    return 0;

  return BCC->getNumArgs();
}

CXString clang_BlockCommandComment_getArgText(CXComment CXC,
                                              unsigned ArgIdx) {
  const BlockCommandComment *BCC = getASTNodeAs<BlockCommandComment>(CXC);
  if (!BCC || ArgIdx >= BCC->getNumArgs())
    return cxstring::createNull();

  return cxstring::createRef(BCC->getArgText(ArgIdx));
}

CXComment clang_BlockCommandComment_getParagraph(CXComment CXC) {
  const BlockCommandComment *BCC = getASTNodeAs<BlockCommandComment>(CXC);
  if (!BCC)
    return createCXComment(nullptr, nullptr);

  return createCXComment(BCC->getParagraph(), CXC.TranslationUnit);
}

CXString clang_ParamCommandComment_getParamName(CXComment CXC) {
  const ParamCommandComment *PCC = getASTNodeAs<ParamCommandComment>(CXC);
  if (!PCC || !PCC->hasParamName())
    return cxstring::createNull();

  return cxstring::createRef(PCC->getParamNameAsWritten());
}

unsigned clang_ParamCommandComment_isParamIndexValid(CXComment CXC) {
  const ParamCommandComment *PCC = getASTNodeAs<ParamCommandComment>(CXC);
  if (!PCC)
    return false;

  return PCC->isParamIndexValid();
}

unsigned clang_ParamCommandComment_getParamIndex(CXComment CXC) {
  const ParamCommandComment *PCC = getASTNodeAs<ParamCommandComment>(CXC);
  if (!PCC || !PCC->isParamIndexValid() || PCC->isVarArgParam())
    return ParamCommandComment::InvalidParamIndex;

  return PCC->getParamIndex();
}

unsigned clang_ParamCommandComment_isDirectionExplicit(CXComment CXC) {
  const ParamCommandComment *PCC = getASTNodeAs<ParamCommandComment>(CXC);
  if (!PCC)
    return false;

  return PCC->isDirectionExplicit();
}

enum CXCommentParamPassDirection clang_ParamCommandComment_getDirection(
                                                            CXComment CXC) {
  const ParamCommandComment *PCC = getASTNodeAs<ParamCommandComment>(CXC);
  if (!PCC)
    return CXCommentParamPassDirection_In;

  switch (PCC->getDirection()) {
  case ParamCommandComment::In:
    return CXCommentParamPassDirection_In;

  case ParamCommandComment::Out:
    return CXCommentParamPassDirection_Out;

  case ParamCommandComment::InOut:
    return CXCommentParamPassDirection_InOut;
  }
  llvm_unreachable("unknown ParamCommandComment::PassDirection");
}

CXString clang_TParamCommandComment_getParamName(CXComment CXC) {
  const TParamCommandComment *TPCC = getASTNodeAs<TParamCommandComment>(CXC);
  if (!TPCC || !TPCC->hasParamName())
    return cxstring::createNull();

  return cxstring::createRef(TPCC->getParamNameAsWritten());
}

unsigned clang_TParamCommandComment_isParamPositionValid(CXComment CXC) {
  const TParamCommandComment *TPCC = getASTNodeAs<TParamCommandComment>(CXC);
  if (!TPCC)
    return false;

  return TPCC->isPositionValid();
}

unsigned clang_TParamCommandComment_getDepth(CXComment CXC) {
  const TParamCommandComment *TPCC = getASTNodeAs<TParamCommandComment>(CXC);
  if (!TPCC || !TPCC->isPositionValid())
    return 0;

  return TPCC->getDepth();
}

unsigned clang_TParamCommandComment_getIndex(CXComment CXC, unsigned Depth) {
  const TParamCommandComment *TPCC = getASTNodeAs<TParamCommandComment>(CXC);
  if (!TPCC || !TPCC->isPositionValid() || Depth >= TPCC->getDepth())
    return 0;

  return TPCC->getIndex(Depth);
}

CXString clang_VerbatimBlockLineComment_getText(CXComment CXC) {
  const VerbatimBlockLineComment *VBL =
      getASTNodeAs<VerbatimBlockLineComment>(CXC);
  if (!VBL)
    return cxstring::createNull();

  return cxstring::createRef(VBL->getText());
}

CXString clang_VerbatimLineComment_getText(CXComment CXC) {
  const VerbatimLineComment *VLC = getASTNodeAs<VerbatimLineComment>(CXC);
  if (!VLC)
    return cxstring::createNull();

  return cxstring::createRef(VLC->getText());
}

//===----------------------------------------------------------------------===//
// Converting comments to XML.
//===----------------------------------------------------------------------===//

CXString clang_HTMLTagComment_getAsString(CXComment CXC) {
  const HTMLTagComment *HTC = getASTNodeAs<HTMLTagComment>(CXC);
  if (!HTC)
    return cxstring::createNull();

  CXTranslationUnit TU = CXC.TranslationUnit;
  if (!TU->CommentToXML)
    TU->CommentToXML = new clang::index::CommentToXMLConverter();

  SmallString<128> Text;
  TU->CommentToXML->convertHTMLTagNodeToText(
      HTC, Text, cxtu::getASTUnit(TU)->getASTContext());
  return cxstring::createDup(Text.str());
}

CXString clang_FullComment_getAsHTML(CXComment CXC) {
  const FullComment *FC = getASTNodeAs<FullComment>(CXC);
  if (!FC)
    return cxstring::createNull();

  CXTranslationUnit TU = CXC.TranslationUnit;
  if (!TU->CommentToXML)
    TU->CommentToXML = new clang::index::CommentToXMLConverter();

  SmallString<1024> HTML;
  TU->CommentToXML
      ->convertCommentToHTML(FC, HTML, cxtu::getASTUnit(TU)->getASTContext());
  return cxstring::createDup(HTML.str());
}

CXString clang_FullComment_getAsXML(CXComment CXC) {
  const FullComment *FC = getASTNodeAs<FullComment>(CXC);
  if (!FC)
    return cxstring::createNull();

  CXTranslationUnit TU = CXC.TranslationUnit;
  if (!TU->CommentToXML)
    TU->CommentToXML = new clang::index::CommentToXMLConverter();

  SmallString<1024> XML;
  TU->CommentToXML
      ->convertCommentToXML(FC, XML, cxtu::getASTUnit(TU)->getASTContext());
  return cxstring::createDup(XML.str());
}

// } // end extern "C"          // HLSL Change -Don't use c linkage.

