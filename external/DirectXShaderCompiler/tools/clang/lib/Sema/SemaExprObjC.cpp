//===--- SemaExprObjC.cpp - Semantic Analysis for ObjC Expressions --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for Objective-C expressions.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ExprObjC.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Analysis/DomainSpecific/CocoaConventions.h"
#include "clang/Edit/Commit.h"
#include "clang/Edit/Rewriters.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ScopeInfo.h"
#include "llvm/ADT/SmallString.h"

using namespace clang;
using namespace sema;
using llvm::makeArrayRef;

// HLSL Change Starts
// No ObjC parse/sema support, so simply skip all of this compilation.
// Here are enough stubs to link the current targets.
#if 1
ExprResult Sema::ParseObjCStringLiteral(SourceLocation *AtLocs,
  Expr **strings,
  unsigned NumStrings) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildObjCStringLiteral(SourceLocation AtLoc, StringLiteral *S) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildObjCNumericLiteral(SourceLocation AtLoc, Expr *Number) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::ActOnObjCBoolLiteral(SourceLocation AtLoc,
  SourceLocation ValueLoc,
  bool Value) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildObjCBoxedExpr(SourceRange SR, Expr *ValueExpr) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildObjCSubscriptExpression(SourceLocation RB, Expr *BaseExpr,
  Expr *IndexExpr,
  ObjCMethodDecl *getterMethod,
  ObjCMethodDecl *setterMethod) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildObjCArrayLiteral(SourceRange SR, MultiExprArg Elements) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildObjCDictionaryLiteral(SourceRange SR,
  ObjCDictionaryElement *Elements,
  unsigned NumElements) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildObjCEncodeExpression(SourceLocation AtLoc,
  TypeSourceInfo *EncodedTypeInfo,
  SourceLocation RParenLoc) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::ParseObjCEncodeExpression(SourceLocation AtLoc,
  SourceLocation EncodeLoc,
  SourceLocation LParenLoc,
  ParsedType ty,
  SourceLocation RParenLoc) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::ParseObjCSelectorExpression(Selector Sel,
  SourceLocation AtLoc,
  SourceLocation SelLoc,
  SourceLocation LParenLoc,
  SourceLocation RParenLoc,
  bool WarnMultipleSelectors) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::ParseObjCProtocolExpression(IdentifierInfo *ProtocolId,
  SourceLocation AtLoc,
  SourceLocation ProtoLoc,
  SourceLocation LParenLoc,
  SourceLocation ProtoIdLoc,
  SourceLocation RParenLoc) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return ExprError();
}

/// Try to capture an implicit reference to 'self'.
ObjCMethodDecl *Sema::tryCaptureObjCSelf(SourceLocation Loc) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return nullptr;
}

QualType Sema::getMessageSendResultType(QualType ReceiverType,
  ObjCMethodDecl *Method,
  bool isClassMessage,
  bool isSuperMessage) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return QualType();
}

void Sema::EmitRelatedResultTypeNoteForReturn(QualType destType) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::EmitRelatedResultTypeNote(const Expr *E) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

bool Sema::CheckMessageArgumentTypes(QualType ReceiverType,
  MultiExprArg Args,
  Selector Sel,
  ArrayRef<SourceLocation> SelectorLocs,
  ObjCMethodDecl *Method,
  bool isClassMessage, bool isSuperMessage,
  SourceLocation lbrac, SourceLocation rbrac,
  SourceRange RecRange,
  QualType &ReturnType, ExprValueKind &VK) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return false;
}

bool Sema::isSelfExpr(Expr *RExpr) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return false;
}

bool Sema::isSelfExpr(Expr *receiver, const ObjCMethodDecl *method) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return false;
}

ObjCMethodDecl *Sema::LookupMethodInObjectType(Selector sel, QualType type,
  bool isInstance) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return nullptr;
}

ObjCMethodDecl *Sema::LookupMethodInQualifiedType(Selector Sel,
  const ObjCObjectPointerType *OPT,
  bool Instance)
{
  llvm_unreachable("HLSL does not support ObjC constructs");
  return nullptr;
}

ExprResult Sema::
HandleExprPropertyRefExpr(const ObjCObjectPointerType *OPT,
  Expr *BaseExpr, SourceLocation OpLoc,
  DeclarationName MemberName,
  SourceLocation MemberLoc,
  SourceLocation SuperLoc, QualType SuperType,
  bool Super) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return ExprError();
}

ExprResult Sema::
ActOnClassPropertyRefExpr(IdentifierInfo &receiverName,
  IdentifierInfo &propertyName,
  SourceLocation receiverNameLoc,
  SourceLocation propertyNameLoc) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return ExprError();
}

Sema::ObjCMessageKind Sema::getObjCMessageKind(Scope *S,
  IdentifierInfo *Name,
  SourceLocation NameLoc,
  bool IsSuper,
  bool HasTrailingDot,
  ParsedType &ReceiverType) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::ActOnSuperMessage(Scope *S,
  SourceLocation SuperLoc,
  Selector Sel,
  SourceLocation LBracLoc,
  ArrayRef<SourceLocation> SelectorLocs,
  SourceLocation RBracLoc,
  MultiExprArg Args) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return ExprError();
}


ExprResult Sema::BuildClassMessageImplicit(QualType ReceiverType,
  bool isSuperReceiver,
  SourceLocation Loc,
  Selector Sel,
  ObjCMethodDecl *Method,
  MultiExprArg Args) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return ExprError();
}

ExprResult Sema::BuildClassMessage(TypeSourceInfo *ReceiverTypeInfo,
  QualType ReceiverType,
  SourceLocation SuperLoc,
  Selector Sel,
  ObjCMethodDecl *Method,
  SourceLocation LBracLoc,
  ArrayRef<SourceLocation> SelectorLocs,
  SourceLocation RBracLoc,
  MultiExprArg ArgsIn,
  bool isImplicit) {
  llvm_unreachable("HLSL does not support ObjC constructs");
  return ExprError();
}

ExprResult Sema::ActOnClassMessage(Scope *S,
  ParsedType Receiver,
  Selector Sel,
  SourceLocation LBracLoc,
  ArrayRef<SourceLocation> SelectorLocs,
  SourceLocation RBracLoc,
  MultiExprArg Args) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildInstanceMessageImplicit(Expr *Receiver,
  QualType ReceiverType,
  SourceLocation Loc,
  Selector Sel,
  ObjCMethodDecl *Method,
  MultiExprArg Args) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildInstanceMessage(Expr *Receiver,
  QualType ReceiverType,
  SourceLocation SuperLoc,
  Selector Sel,
  ObjCMethodDecl *Method,
  SourceLocation LBracLoc,
  ArrayRef<SourceLocation> SelectorLocs,
  SourceLocation RBracLoc,
  MultiExprArg ArgsIn,
  bool isImplicit) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::ActOnInstanceMessage(Scope *S,
  Expr *Receiver,
  Selector Sel,
  SourceLocation LBracLoc,
  ArrayRef<SourceLocation> SelectorLocs,
  SourceLocation RBracLoc,
  MultiExprArg Args) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

bool Sema::isKnownName(StringRef name) {
  if (name.empty())
    return false;
  LookupResult R(*this, &Context.Idents.get(name), SourceLocation(),
    Sema::LookupOrdinaryName);
  return LookupName(R, TUScope, false);
}

void Sema::CheckTollFreeBridgeCast(QualType castType, Expr *castExpr) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::CheckObjCBridgeRelatedCast(QualType castType, Expr *castExpr) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

bool Sema::CheckTollFreeBridgeStaticCast(QualType castType, Expr *castExpr,
  CastKind &Kind) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

bool Sema::checkObjCBridgeRelatedComponents(SourceLocation Loc,
  QualType DestType, QualType SrcType,
  ObjCInterfaceDecl *&RelatedClass,
  ObjCMethodDecl *&ClassMethod,
  ObjCMethodDecl *&InstanceMethod,
  TypedefNameDecl *&TDNDecl,
  bool CfToNs) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

bool
Sema::CheckObjCBridgeRelatedConversions(SourceLocation Loc,
  QualType DestType, QualType SrcType,
  Expr *&SrcExpr) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

Sema::ARCConversionResult
Sema::CheckObjCARCConversion(SourceRange castRange, QualType castType,
  Expr *&castExpr, CheckedConversionKind CCK,
  bool DiagnoseCFAudited,
  BinaryOperatorKind Opc) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

/// Given that we saw an expression with the ARCUnbridgedCastTy
/// placeholder type, complain bitterly.
void Sema::diagnoseARCUnbridgedCast(Expr *e) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

/// stripARCUnbridgedCast - Given an expression of ARCUnbridgedCast
/// type, remove the placeholder cast.
Expr *Sema::stripARCUnbridgedCast(Expr *e) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

bool Sema::CheckObjCARCUnavailableWeakConversion(QualType castType,
  QualType exprType) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::BuildObjCBridgedCast(SourceLocation LParenLoc,
  ObjCBridgeCastKind Kind,
  SourceLocation BridgeKeywordLoc,
  TypeSourceInfo *TSInfo,
  Expr *SubExpr) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ExprResult Sema::ActOnObjCBridgedCast(Scope *S,
  SourceLocation LParenLoc,
  ObjCBridgeCastKind Kind,
  SourceLocation BridgeKeywordLoc,
  ParsedType Type,
  SourceLocation RParenLoc,
  Expr *SubExpr) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

#else
// HLSL Change Ends

ExprResult Sema::ParseObjCStringLiteral(SourceLocation *AtLocs,
                                        Expr **strings,
                                        unsigned NumStrings) {
  StringLiteral **Strings = reinterpret_cast<StringLiteral**>(strings);

  // Most ObjC strings are formed out of a single piece.  However, we *can*
  // have strings formed out of multiple @ strings with multiple pptokens in
  // each one, e.g. @"foo" "bar" @"baz" "qux"   which need to be turned into one
  // StringLiteral for ObjCStringLiteral to hold onto.
  StringLiteral *S = Strings[0];

  // If we have a multi-part string, merge it all together.
  if (NumStrings != 1) {
    // Concatenate objc strings.
    SmallString<128> StrBuf;
    SmallVector<SourceLocation, 8> StrLocs;

    for (unsigned i = 0; i != NumStrings; ++i) {
      S = Strings[i];

      // ObjC strings can't be wide or UTF.
      if (!S->isAscii()) {
        Diag(S->getLocStart(), diag::err_cfstring_literal_not_string_constant)
          << S->getSourceRange();
        return true;
      }

      // Append the string.
      StrBuf += S->getString();

      // Get the locations of the string tokens.
      StrLocs.append(S->tokloc_begin(), S->tokloc_end());
    }

    // Create the aggregate string with the appropriate content and location
    // information.
    const ConstantArrayType *CAT = Context.getAsConstantArrayType(S->getType());
    assert(CAT && "String literal not of constant array type!");
    QualType StrTy = Context.getConstantArrayType(
        CAT->getElementType(), llvm::APInt(32, StrBuf.size() + 1),
        CAT->getSizeModifier(), CAT->getIndexTypeCVRQualifiers());
    S = StringLiteral::Create(Context, StrBuf, StringLiteral::Ascii,
                              /*Pascal=*/false, StrTy, &StrLocs[0],
                              StrLocs.size());
  }
  
  return BuildObjCStringLiteral(AtLocs[0], S);
}

ExprResult Sema::BuildObjCStringLiteral(SourceLocation AtLoc, StringLiteral *S){
  // Verify that this composite string is acceptable for ObjC strings.
  if (CheckObjCString(S))
    return true;

  // Initialize the constant string interface lazily. This assumes
  // the NSString interface is seen in this translation unit. Note: We
  // don't use NSConstantString, since the runtime team considers this
  // interface private (even though it appears in the header files).
  QualType Ty = Context.getObjCConstantStringInterface();
  if (!Ty.isNull()) {
    Ty = Context.getObjCObjectPointerType(Ty);
  } else if (getLangOpts().NoConstantCFStrings) {
    IdentifierInfo *NSIdent=nullptr;
    std::string StringClass(getLangOpts().ObjCConstantStringClass);
    
    if (StringClass.empty())
      NSIdent = &Context.Idents.get("NSConstantString");
    else
      NSIdent = &Context.Idents.get(StringClass);
    
    NamedDecl *IF = LookupSingleName(TUScope, NSIdent, AtLoc,
                                     LookupOrdinaryName);
    if (ObjCInterfaceDecl *StrIF = dyn_cast_or_null<ObjCInterfaceDecl>(IF)) {
      Context.setObjCConstantStringInterface(StrIF);
      Ty = Context.getObjCConstantStringInterface();
      Ty = Context.getObjCObjectPointerType(Ty);
    } else {
      // If there is no NSConstantString interface defined then treat this
      // as error and recover from it.
      Diag(S->getLocStart(), diag::err_no_nsconstant_string_class) << NSIdent
        << S->getSourceRange();
      Ty = Context.getObjCIdType();
    }
  } else {
    IdentifierInfo *NSIdent = NSAPIObj->getNSClassId(NSAPI::ClassId_NSString);
    NamedDecl *IF = LookupSingleName(TUScope, NSIdent, AtLoc,
                                     LookupOrdinaryName);
    if (ObjCInterfaceDecl *StrIF = dyn_cast_or_null<ObjCInterfaceDecl>(IF)) {
      Context.setObjCConstantStringInterface(StrIF);
      Ty = Context.getObjCConstantStringInterface();
      Ty = Context.getObjCObjectPointerType(Ty);
    } else {
      // If there is no NSString interface defined, implicitly declare
      // a @class NSString; and use that instead. This is to make sure
      // type of an NSString literal is represented correctly, instead of
      // being an 'id' type.
      Ty = Context.getObjCNSStringType();
      if (Ty.isNull()) {
        ObjCInterfaceDecl *NSStringIDecl = 
          ObjCInterfaceDecl::Create (Context, 
                                     Context.getTranslationUnitDecl(), 
                                     SourceLocation(), NSIdent, 
                                     nullptr, nullptr, SourceLocation());
        Ty = Context.getObjCInterfaceType(NSStringIDecl);
        Context.setObjCNSStringType(Ty);
      }
      Ty = Context.getObjCObjectPointerType(Ty);
    }
  }

  return new (Context) ObjCStringLiteral(S, Ty, AtLoc);
}

/// \brief Emits an error if the given method does not exist, or if the return
/// type is not an Objective-C object.
static bool validateBoxingMethod(Sema &S, SourceLocation Loc,
                                 const ObjCInterfaceDecl *Class,
                                 Selector Sel, const ObjCMethodDecl *Method) {
  if (!Method) {
    // FIXME: Is there a better way to avoid quotes than using getName()?
    S.Diag(Loc, diag::err_undeclared_boxing_method) << Sel << Class->getName();
    return false;
  }

  // Make sure the return type is reasonable.
  QualType ReturnType = Method->getReturnType();
  if (!ReturnType->isObjCObjectPointerType()) {
    S.Diag(Loc, diag::err_objc_literal_method_sig)
      << Sel;
    S.Diag(Method->getLocation(), diag::note_objc_literal_method_return)
      << ReturnType;
    return false;
  }

  return true;
}

/// \brief Retrieve the NSNumber factory method that should be used to create
/// an Objective-C literal for the given type.
static ObjCMethodDecl *getNSNumberFactoryMethod(Sema &S, SourceLocation Loc,
                                                QualType NumberType,
                                                bool isLiteral = false,
                                                SourceRange R = SourceRange()) {
  Optional<NSAPI::NSNumberLiteralMethodKind> Kind =
      S.NSAPIObj->getNSNumberFactoryMethodKind(NumberType);

  if (!Kind) {
    if (isLiteral) {
      S.Diag(Loc, diag::err_invalid_nsnumber_type)
        << NumberType << R;
    }
    return nullptr;
  }
  
  // If we already looked up this method, we're done.
  if (S.NSNumberLiteralMethods[*Kind])
    return S.NSNumberLiteralMethods[*Kind];
  
  Selector Sel = S.NSAPIObj->getNSNumberLiteralSelector(*Kind,
                                                        /*Instance=*/false);
  
  ASTContext &CX = S.Context;
  
  // Look up the NSNumber class, if we haven't done so already. It's cached
  // in the Sema instance.
  if (!S.NSNumberDecl) {
    IdentifierInfo *NSNumberId =
      S.NSAPIObj->getNSClassId(NSAPI::ClassId_NSNumber);
    NamedDecl *IF = S.LookupSingleName(S.TUScope, NSNumberId,
                                       Loc, Sema::LookupOrdinaryName);
    S.NSNumberDecl = dyn_cast_or_null<ObjCInterfaceDecl>(IF);
    if (!S.NSNumberDecl) {
      if (S.getLangOpts().DebuggerObjCLiteral) {
        // Create a stub definition of NSNumber.
        S.NSNumberDecl = ObjCInterfaceDecl::Create(CX,
                                                   CX.getTranslationUnitDecl(),
                                                   SourceLocation(), NSNumberId,
                                                   nullptr, nullptr,
                                                   SourceLocation());
      } else {
        // Otherwise, require a declaration of NSNumber.
        S.Diag(Loc, diag::err_undeclared_nsnumber);
        return nullptr;
      }
    } else if (!S.NSNumberDecl->hasDefinition()) {
      S.Diag(Loc, diag::err_undeclared_nsnumber);
      return nullptr;
    }
  }

  if (S.NSNumberPointer.isNull()) {
    // generate the pointer to NSNumber type.
    QualType NSNumberObject = CX.getObjCInterfaceType(S.NSNumberDecl);
    S.NSNumberPointer = CX.getObjCObjectPointerType(NSNumberObject);
  }
  
  // Look for the appropriate method within NSNumber.
  ObjCMethodDecl *Method = S.NSNumberDecl->lookupClassMethod(Sel);
  if (!Method && S.getLangOpts().DebuggerObjCLiteral) {
    // create a stub definition this NSNumber factory method.
    TypeSourceInfo *ReturnTInfo = nullptr;
    Method =
        ObjCMethodDecl::Create(CX, SourceLocation(), SourceLocation(), Sel,
                               S.NSNumberPointer, ReturnTInfo, S.NSNumberDecl,
                               /*isInstance=*/false, /*isVariadic=*/false,
                               /*isPropertyAccessor=*/false,
                               /*isImplicitlyDeclared=*/true,
                               /*isDefined=*/false, ObjCMethodDecl::Required,
                               /*HasRelatedResultType=*/false);
    ParmVarDecl *value = ParmVarDecl::Create(S.Context, Method,
                                             SourceLocation(), SourceLocation(),
                                             &CX.Idents.get("value"),
                                             NumberType, /*TInfo=*/nullptr,
                                             SC_None, nullptr);
    Method->setMethodParams(S.Context, value, None);
  }

  if (!validateBoxingMethod(S, Loc, S.NSNumberDecl, Sel, Method))
    return nullptr;

  // Note: if the parameter type is out-of-line, we'll catch it later in the
  // implicit conversion.
  
  S.NSNumberLiteralMethods[*Kind] = Method;
  return Method;
}

/// BuildObjCNumericLiteral - builds an ObjCBoxedExpr AST node for the
/// numeric literal expression. Type of the expression will be "NSNumber *".
ExprResult Sema::BuildObjCNumericLiteral(SourceLocation AtLoc, Expr *Number) {
  // Determine the type of the literal.
  QualType NumberType = Number->getType();
  if (CharacterLiteral *Char = dyn_cast<CharacterLiteral>(Number)) {
    // In C, character literals have type 'int'. That's not the type we want
    // to use to determine the Objective-c literal kind.
    switch (Char->getKind()) {
    case CharacterLiteral::Ascii:
      NumberType = Context.CharTy;
      break;
      
    case CharacterLiteral::Wide:
      NumberType = Context.getWideCharType();
      break;
      
    case CharacterLiteral::UTF16:
      NumberType = Context.Char16Ty;
      break;
      
    case CharacterLiteral::UTF32:
      NumberType = Context.Char32Ty;
      break;
    }
  }
  
  // Look for the appropriate method within NSNumber.
  // Construct the literal.
  SourceRange NR(Number->getSourceRange());
  ObjCMethodDecl *Method = getNSNumberFactoryMethod(*this, AtLoc, NumberType,
                                                    true, NR);
  if (!Method)
    return ExprError();

  // Convert the number to the type that the parameter expects.
  ParmVarDecl *ParamDecl = Method->parameters()[0];
  InitializedEntity Entity = InitializedEntity::InitializeParameter(Context,
                                                                    ParamDecl);
  ExprResult ConvertedNumber = PerformCopyInitialization(Entity,
                                                         SourceLocation(),
                                                         Number);
  if (ConvertedNumber.isInvalid())
    return ExprError();
  Number = ConvertedNumber.get();
  
  // Use the effective source range of the literal, including the leading '@'.
  return MaybeBindToTemporary(
           new (Context) ObjCBoxedExpr(Number, NSNumberPointer, Method,
                                       SourceRange(AtLoc, NR.getEnd())));
}

ExprResult Sema::ActOnObjCBoolLiteral(SourceLocation AtLoc, 
                                      SourceLocation ValueLoc,
                                      bool Value) {
  ExprResult Inner;
  if (getLangOpts().CPlusPlus) {
    Inner = ActOnCXXBoolLiteral(ValueLoc, Value? tok::kw_true : tok::kw_false);
  } else {
    // C doesn't actually have a way to represent literal values of type 
    // _Bool. So, we'll use 0/1 and implicit cast to _Bool.
    Inner = ActOnIntegerConstant(ValueLoc, Value? 1 : 0);
    Inner = ImpCastExprToType(Inner.get(), Context.BoolTy, 
                              CK_IntegralToBoolean);
  }
  
  return BuildObjCNumericLiteral(AtLoc, Inner.get());
}

/// \brief Check that the given expression is a valid element of an Objective-C
/// collection literal.
static ExprResult CheckObjCCollectionLiteralElement(Sema &S, Expr *Element, 
                                                    QualType T,
                                                    bool ArrayLiteral = false) {
  // If the expression is type-dependent, there's nothing for us to do.
  if (Element->isTypeDependent())
    return Element;

  ExprResult Result = S.CheckPlaceholderExpr(Element);
  if (Result.isInvalid())
    return ExprError();
  Element = Result.get();

  // In C++, check for an implicit conversion to an Objective-C object pointer 
  // type.
  if (S.getLangOpts().CPlusPlus && Element->getType()->isRecordType()) {
    InitializedEntity Entity
      = InitializedEntity::InitializeParameter(S.Context, T,
                                               /*Consumed=*/false);
    InitializationKind Kind
      = InitializationKind::CreateCopy(Element->getLocStart(),
                                       SourceLocation());
    InitializationSequence Seq(S, Entity, Kind, Element);
    if (!Seq.Failed())
      return Seq.Perform(S, Entity, Kind, Element);
  }

  Expr *OrigElement = Element;

  // Perform lvalue-to-rvalue conversion.
  Result = S.DefaultLvalueConversion(Element);
  if (Result.isInvalid())
    return ExprError();
  Element = Result.get();  

  // Make sure that we have an Objective-C pointer type or block.
  if (!Element->getType()->isObjCObjectPointerType() &&
      !Element->getType()->isBlockPointerType()) {
    bool Recovered = false;
    
    // If this is potentially an Objective-C numeric literal, add the '@'.
    if (isa<IntegerLiteral>(OrigElement) || 
        isa<CharacterLiteral>(OrigElement) ||
        isa<FloatingLiteral>(OrigElement) ||
        isa<ObjCBoolLiteralExpr>(OrigElement) ||
        isa<CXXBoolLiteralExpr>(OrigElement)) {
      if (S.NSAPIObj->getNSNumberFactoryMethodKind(OrigElement->getType())) {
        int Which = isa<CharacterLiteral>(OrigElement) ? 1
                  : (isa<CXXBoolLiteralExpr>(OrigElement) ||
                     isa<ObjCBoolLiteralExpr>(OrigElement)) ? 2
                  : 3;
        
        S.Diag(OrigElement->getLocStart(), diag::err_box_literal_collection)
          << Which << OrigElement->getSourceRange()
          << FixItHint::CreateInsertion(OrigElement->getLocStart(), "@");
        
        Result = S.BuildObjCNumericLiteral(OrigElement->getLocStart(),
                                           OrigElement);
        if (Result.isInvalid())
          return ExprError();
        
        Element = Result.get();
        Recovered = true;
      }
    }
    // If this is potentially an Objective-C string literal, add the '@'.
    else if (StringLiteral *String = dyn_cast<StringLiteral>(OrigElement)) {
      if (String->isAscii()) {
        S.Diag(OrigElement->getLocStart(), diag::err_box_literal_collection)
          << 0 << OrigElement->getSourceRange()
          << FixItHint::CreateInsertion(OrigElement->getLocStart(), "@");

        Result = S.BuildObjCStringLiteral(OrigElement->getLocStart(), String);
        if (Result.isInvalid())
          return ExprError();
        
        Element = Result.get();
        Recovered = true;
      }
    }
  
    if (!Recovered) {
      S.Diag(Element->getLocStart(), diag::err_invalid_collection_element)
        << Element->getType();
      return ExprError();
    }
  }
  if (ArrayLiteral)
    if (ObjCStringLiteral *getString =
          dyn_cast<ObjCStringLiteral>(OrigElement)) {
      if (StringLiteral *SL = getString->getString()) {
        unsigned numConcat = SL->getNumConcatenated();
        if (numConcat > 1) {
          // Only warn if the concatenated string doesn't come from a macro.
          bool hasMacro = false;
          for (unsigned i = 0; i < numConcat ; ++i)
            if (SL->getStrTokenLoc(i).isMacroID()) {
              hasMacro = true;
              break;
            }
          if (!hasMacro)
            S.Diag(Element->getLocStart(),
                   diag::warn_concatenated_nsarray_literal)
              << Element->getType();
        }
      }
    }

  // Make sure that the element has the type that the container factory 
  // function expects. 
  return S.PerformCopyInitialization(
           InitializedEntity::InitializeParameter(S.Context, T, 
                                                  /*Consumed=*/false),
           Element->getLocStart(), Element);
}

ExprResult Sema::BuildObjCBoxedExpr(SourceRange SR, Expr *ValueExpr) {
  if (ValueExpr->isTypeDependent()) {
    ObjCBoxedExpr *BoxedExpr = 
      new (Context) ObjCBoxedExpr(ValueExpr, Context.DependentTy, nullptr, SR);
    return BoxedExpr;
  }
  ObjCMethodDecl *BoxingMethod = nullptr;
  QualType BoxedType;
  // Convert the expression to an RValue, so we can check for pointer types...
  ExprResult RValue = DefaultFunctionArrayLvalueConversion(ValueExpr);
  if (RValue.isInvalid()) {
    return ExprError();
  }
  ValueExpr = RValue.get();
  QualType ValueType(ValueExpr->getType());
  if (const PointerType *PT = ValueType->getAs<PointerType>()) {
    QualType PointeeType = PT->getPointeeType();
    if (Context.hasSameUnqualifiedType(PointeeType, Context.CharTy)) {

      if (!NSStringDecl) {
        IdentifierInfo *NSStringId =
          NSAPIObj->getNSClassId(NSAPI::ClassId_NSString);
        NamedDecl *Decl = LookupSingleName(TUScope, NSStringId,
                                           SR.getBegin(), LookupOrdinaryName);
        NSStringDecl = dyn_cast_or_null<ObjCInterfaceDecl>(Decl);
        if (!NSStringDecl) {
          if (getLangOpts().DebuggerObjCLiteral) {
            // Support boxed expressions in the debugger w/o NSString declaration.
            DeclContext *TU = Context.getTranslationUnitDecl();
            NSStringDecl = ObjCInterfaceDecl::Create(Context, TU,
                                                     SourceLocation(),
                                                     NSStringId,
                                                     nullptr, nullptr,
                                                     SourceLocation());
          } else {
            Diag(SR.getBegin(), diag::err_undeclared_nsstring);
            return ExprError();
          }
        } else if (!NSStringDecl->hasDefinition()) {
          Diag(SR.getBegin(), diag::err_undeclared_nsstring);
          return ExprError();
        }
        assert(NSStringDecl && "NSStringDecl should not be NULL");
        QualType NSStringObject = Context.getObjCInterfaceType(NSStringDecl);
        NSStringPointer = Context.getObjCObjectPointerType(NSStringObject);
      }
      
      if (!StringWithUTF8StringMethod) {
        IdentifierInfo *II = &Context.Idents.get("stringWithUTF8String");
        Selector stringWithUTF8String = Context.Selectors.getUnarySelector(II);

        // Look for the appropriate method within NSString.
        BoxingMethod = NSStringDecl->lookupClassMethod(stringWithUTF8String);
        if (!BoxingMethod && getLangOpts().DebuggerObjCLiteral) {
          // Debugger needs to work even if NSString hasn't been defined.
          TypeSourceInfo *ReturnTInfo = nullptr;
          ObjCMethodDecl *M = ObjCMethodDecl::Create(
              Context, SourceLocation(), SourceLocation(), stringWithUTF8String,
              NSStringPointer, ReturnTInfo, NSStringDecl,
              /*isInstance=*/false, /*isVariadic=*/false,
              /*isPropertyAccessor=*/false,
              /*isImplicitlyDeclared=*/true,
              /*isDefined=*/false, ObjCMethodDecl::Required,
              /*HasRelatedResultType=*/false);
          QualType ConstCharType = Context.CharTy.withConst();
          ParmVarDecl *value =
            ParmVarDecl::Create(Context, M,
                                SourceLocation(), SourceLocation(),
                                &Context.Idents.get("value"),
                                Context.getPointerType(ConstCharType),
                                /*TInfo=*/nullptr,
                                SC_None, nullptr);
          M->setMethodParams(Context, value, None);
          BoxingMethod = M;
        }

        if (!validateBoxingMethod(*this, SR.getBegin(), NSStringDecl,
                                  stringWithUTF8String, BoxingMethod))
           return ExprError();

        StringWithUTF8StringMethod = BoxingMethod;
      }
      
      BoxingMethod = StringWithUTF8StringMethod;
      BoxedType = NSStringPointer;
    }
  } else if (ValueType->isBuiltinType()) {
    // The other types we support are numeric, char and BOOL/bool. We could also
    // provide limited support for structure types, such as NSRange, NSRect, and
    // NSSize. See NSValue (NSValueGeometryExtensions) in <Foundation/NSGeometry.h>
    // for more details.

    // Check for a top-level character literal.
    if (const CharacterLiteral *Char =
        dyn_cast<CharacterLiteral>(ValueExpr->IgnoreParens())) {
      // In C, character literals have type 'int'. That's not the type we want
      // to use to determine the Objective-c literal kind.
      switch (Char->getKind()) {
      case CharacterLiteral::Ascii:
        ValueType = Context.CharTy;
        break;
        
      case CharacterLiteral::Wide:
        ValueType = Context.getWideCharType();
        break;
        
      case CharacterLiteral::UTF16:
        ValueType = Context.Char16Ty;
        break;
        
      case CharacterLiteral::UTF32:
        ValueType = Context.Char32Ty;
        break;
      }
    }
    CheckForIntOverflow(ValueExpr);
    // FIXME:  Do I need to do anything special with BoolTy expressions?
    
    // Look for the appropriate method within NSNumber.
    BoxingMethod = getNSNumberFactoryMethod(*this, SR.getBegin(), ValueType);
    BoxedType = NSNumberPointer;
  } else if (const EnumType *ET = ValueType->getAs<EnumType>()) {
    if (!ET->getDecl()->isComplete()) {
      Diag(SR.getBegin(), diag::err_objc_incomplete_boxed_expression_type)
        << ValueType << ValueExpr->getSourceRange();
      return ExprError();
    }

    BoxingMethod = getNSNumberFactoryMethod(*this, SR.getBegin(),
                                            ET->getDecl()->getIntegerType());
    BoxedType = NSNumberPointer;
  } else if (ValueType->isObjCBoxableRecordType()) {
    // Support for structure types, that marked as objc_boxable
    // struct __attribute__((objc_boxable)) s { ... };
    
    // Look up the NSValue class, if we haven't done so already. It's cached
    // in the Sema instance.
    if (!NSValueDecl) {
      IdentifierInfo *NSValueId =
        NSAPIObj->getNSClassId(NSAPI::ClassId_NSValue);
      NamedDecl *IF = LookupSingleName(TUScope, NSValueId,
                                       SR.getBegin(), Sema::LookupOrdinaryName);
      NSValueDecl = dyn_cast_or_null<ObjCInterfaceDecl>(IF);
      if (!NSValueDecl) {
        if (getLangOpts().DebuggerObjCLiteral) {
          // Create a stub definition of NSValue.
          DeclContext *TU = Context.getTranslationUnitDecl();
          NSValueDecl = ObjCInterfaceDecl::Create(Context, TU,
                                                  SourceLocation(), NSValueId,
                                                  nullptr, nullptr,
                                                  SourceLocation());
        } else {
          // Otherwise, require a declaration of NSValue.
          Diag(SR.getBegin(), diag::err_undeclared_nsvalue);
          return ExprError();
        }
      } else if (!NSValueDecl->hasDefinition()) {
        Diag(SR.getBegin(), diag::err_undeclared_nsvalue);
        return ExprError();
      }
      
      // generate the pointer to NSValue type.
      QualType NSValueObject = Context.getObjCInterfaceType(NSValueDecl);
      NSValuePointer = Context.getObjCObjectPointerType(NSValueObject);
    }
    
    if (!ValueWithBytesObjCTypeMethod) {
      IdentifierInfo *II[] = {
        &Context.Idents.get("valueWithBytes"),
        &Context.Idents.get("objCType")
      };
      Selector ValueWithBytesObjCType = Context.Selectors.getSelector(2, II);
      
      // Look for the appropriate method within NSValue.
      BoxingMethod = NSValueDecl->lookupClassMethod(ValueWithBytesObjCType);
      if (!BoxingMethod && getLangOpts().DebuggerObjCLiteral) {
        // Debugger needs to work even if NSValue hasn't been defined.
        TypeSourceInfo *ReturnTInfo = nullptr;
        ObjCMethodDecl *M = ObjCMethodDecl::Create(
                                               Context,
                                               SourceLocation(),
                                               SourceLocation(),
                                               ValueWithBytesObjCType,
                                               NSValuePointer,
                                               ReturnTInfo,
                                               NSValueDecl,
                                               /*isInstance=*/false,
                                               /*isVariadic=*/false,
                                               /*isPropertyAccessor=*/false,
                                               /*isImplicitlyDeclared=*/true,
                                               /*isDefined=*/false,
                                               ObjCMethodDecl::Required,
                                               /*HasRelatedResultType=*/false);
        
        SmallVector<ParmVarDecl *, 2> Params;
        
        ParmVarDecl *bytes =
        ParmVarDecl::Create(Context, M,
                            SourceLocation(), SourceLocation(),
                            &Context.Idents.get("bytes"),
                            Context.VoidPtrTy.withConst(),
                            /*TInfo=*/nullptr,
                            SC_None, nullptr);
        Params.push_back(bytes);
        
        QualType ConstCharType = Context.CharTy.withConst();
        ParmVarDecl *type =
        ParmVarDecl::Create(Context, M,
                            SourceLocation(), SourceLocation(),
                            &Context.Idents.get("type"),
                            Context.getPointerType(ConstCharType),
                            /*TInfo=*/nullptr,
                            SC_None, nullptr);
        Params.push_back(type);
        
        M->setMethodParams(Context, Params, None);
        BoxingMethod = M;
      }
      
      if (!validateBoxingMethod(*this, SR.getBegin(), NSValueDecl,
                                ValueWithBytesObjCType, BoxingMethod))
        return ExprError();
      
      ValueWithBytesObjCTypeMethod = BoxingMethod;
    }
    
    if (!ValueType.isTriviallyCopyableType(Context)) {
      Diag(SR.getBegin(), 
           diag::err_objc_non_trivially_copyable_boxed_expression_type)
        << ValueType << ValueExpr->getSourceRange();
      return ExprError();
    }

    BoxingMethod = ValueWithBytesObjCTypeMethod;
    BoxedType = NSValuePointer;
  }

  if (!BoxingMethod) {
    Diag(SR.getBegin(), diag::err_objc_illegal_boxed_expression_type)
      << ValueType << ValueExpr->getSourceRange();
    return ExprError();
  }
  
  DiagnoseUseOfDecl(BoxingMethod, SR.getBegin());

  ExprResult ConvertedValueExpr;
  if (ValueType->isObjCBoxableRecordType()) {
    InitializedEntity IE = InitializedEntity::InitializeTemporary(ValueType);
    ConvertedValueExpr = PerformCopyInitialization(IE, ValueExpr->getExprLoc(), 
                                                   ValueExpr);
  } else {
    // Convert the expression to the type that the parameter requires.
    ParmVarDecl *ParamDecl = BoxingMethod->parameters()[0];
    InitializedEntity IE = InitializedEntity::InitializeParameter(Context,
                                                                  ParamDecl);
    ConvertedValueExpr = PerformCopyInitialization(IE, SourceLocation(),
                                                   ValueExpr);
  }
  
  if (ConvertedValueExpr.isInvalid())
    return ExprError();
  ValueExpr = ConvertedValueExpr.get();
  
  ObjCBoxedExpr *BoxedExpr = 
    new (Context) ObjCBoxedExpr(ValueExpr, BoxedType,
                                      BoxingMethod, SR);
  return MaybeBindToTemporary(BoxedExpr);
}

/// Build an ObjC subscript pseudo-object expression, given that
/// that's supported by the runtime.
ExprResult Sema::BuildObjCSubscriptExpression(SourceLocation RB, Expr *BaseExpr,
                                        Expr *IndexExpr,
                                        ObjCMethodDecl *getterMethod,
                                        ObjCMethodDecl *setterMethod) {
  assert(!LangOpts.isSubscriptPointerArithmetic());

  // We can't get dependent types here; our callers should have
  // filtered them out.
  assert((!BaseExpr->isTypeDependent() && !IndexExpr->isTypeDependent()) &&
         "base or index cannot have dependent type here");

  // Filter out placeholders in the index.  In theory, overloads could
  // be preserved here, although that might not actually work correctly.
  ExprResult Result = CheckPlaceholderExpr(IndexExpr);
  if (Result.isInvalid())
    return ExprError();
  IndexExpr = Result.get();
  
  // Perform lvalue-to-rvalue conversion on the base.
  Result = DefaultLvalueConversion(BaseExpr);
  if (Result.isInvalid())
    return ExprError();
  BaseExpr = Result.get();

  // Build the pseudo-object expression.
  return ObjCSubscriptRefExpr::Create(Context, BaseExpr, IndexExpr,
                                      Context.PseudoObjectTy, getterMethod,
                                      setterMethod, RB);
}

ExprResult Sema::BuildObjCArrayLiteral(SourceRange SR, MultiExprArg Elements) {
  // Look up the NSArray class, if we haven't done so already.
  if (!NSArrayDecl) {
    NamedDecl *IF = LookupSingleName(TUScope,
                                 NSAPIObj->getNSClassId(NSAPI::ClassId_NSArray),
                                 SR.getBegin(),
                                 LookupOrdinaryName);
    NSArrayDecl = dyn_cast_or_null<ObjCInterfaceDecl>(IF);
    if (!NSArrayDecl && getLangOpts().DebuggerObjCLiteral)
      NSArrayDecl =  ObjCInterfaceDecl::Create (Context,
                            Context.getTranslationUnitDecl(),
                            SourceLocation(),
                            NSAPIObj->getNSClassId(NSAPI::ClassId_NSArray),
                            nullptr, nullptr, SourceLocation());

    if (!NSArrayDecl) {
      Diag(SR.getBegin(), diag::err_undeclared_nsarray);
      return ExprError();
    }
  }
  
  // Find the arrayWithObjects:count: method, if we haven't done so already.
  QualType IdT = Context.getObjCIdType();
  if (!ArrayWithObjectsMethod) {
    Selector
      Sel = NSAPIObj->getNSArraySelector(NSAPI::NSArr_arrayWithObjectsCount);
    ObjCMethodDecl *Method = NSArrayDecl->lookupClassMethod(Sel);
    if (!Method && getLangOpts().DebuggerObjCLiteral) {
      TypeSourceInfo *ReturnTInfo = nullptr;
      Method = ObjCMethodDecl::Create(
          Context, SourceLocation(), SourceLocation(), Sel, IdT, ReturnTInfo,
          Context.getTranslationUnitDecl(), false /*Instance*/,
          false /*isVariadic*/,
          /*isPropertyAccessor=*/false,
          /*isImplicitlyDeclared=*/true, /*isDefined=*/false,
          ObjCMethodDecl::Required, false);
      SmallVector<ParmVarDecl *, 2> Params;
      ParmVarDecl *objects = ParmVarDecl::Create(Context, Method,
                                                 SourceLocation(),
                                                 SourceLocation(),
                                                 &Context.Idents.get("objects"),
                                                 Context.getPointerType(IdT),
                                                 /*TInfo=*/nullptr,
                                                 SC_None, nullptr);
      Params.push_back(objects);
      ParmVarDecl *cnt = ParmVarDecl::Create(Context, Method,
                                             SourceLocation(),
                                             SourceLocation(),
                                             &Context.Idents.get("cnt"),
                                             Context.UnsignedLongTy,
                                             /*TInfo=*/nullptr, SC_None,
                                             nullptr);
      Params.push_back(cnt);
      Method->setMethodParams(Context, Params, None);
    }

    if (!validateBoxingMethod(*this, SR.getBegin(), NSArrayDecl, Sel, Method))
      return ExprError();

    // Dig out the type that all elements should be converted to.
    QualType T = Method->parameters()[0]->getType();
    const PointerType *PtrT = T->getAs<PointerType>();
    if (!PtrT || 
        !Context.hasSameUnqualifiedType(PtrT->getPointeeType(), IdT)) {
      Diag(SR.getBegin(), diag::err_objc_literal_method_sig)
        << Sel;
      Diag(Method->parameters()[0]->getLocation(),
           diag::note_objc_literal_method_param)
        << 0 << T 
        << Context.getPointerType(IdT.withConst());
      return ExprError();
    }
  
    // Check that the 'count' parameter is integral.
    if (!Method->parameters()[1]->getType()->isIntegerType()) {
      Diag(SR.getBegin(), diag::err_objc_literal_method_sig)
        << Sel;
      Diag(Method->parameters()[1]->getLocation(),
           diag::note_objc_literal_method_param)
        << 1 
        << Method->parameters()[1]->getType()
        << "integral";
      return ExprError();
    }

    // We've found a good +arrayWithObjects:count: method. Save it!
    ArrayWithObjectsMethod = Method;
  }

  QualType ObjectsType = ArrayWithObjectsMethod->parameters()[0]->getType();
  QualType RequiredType = ObjectsType->castAs<PointerType>()->getPointeeType();

  // Check that each of the elements provided is valid in a collection literal,
  // performing conversions as necessary.
  Expr **ElementsBuffer = Elements.data();
  for (unsigned I = 0, N = Elements.size(); I != N; ++I) {
    ExprResult Converted = CheckObjCCollectionLiteralElement(*this,
                                                             ElementsBuffer[I],
                                                             RequiredType, true);
    if (Converted.isInvalid())
      return ExprError();
    
    ElementsBuffer[I] = Converted.get();
  }
    
  QualType Ty 
    = Context.getObjCObjectPointerType(
                                    Context.getObjCInterfaceType(NSArrayDecl));

  return MaybeBindToTemporary(
           ObjCArrayLiteral::Create(Context, Elements, Ty,
                                    ArrayWithObjectsMethod, SR));
}

ExprResult Sema::BuildObjCDictionaryLiteral(SourceRange SR, 
                                            ObjCDictionaryElement *Elements,
                                            unsigned NumElements) {
  // Look up the NSDictionary class, if we haven't done so already.
  if (!NSDictionaryDecl) {
    NamedDecl *IF = LookupSingleName(TUScope,
                            NSAPIObj->getNSClassId(NSAPI::ClassId_NSDictionary),
                            SR.getBegin(), LookupOrdinaryName);
    NSDictionaryDecl = dyn_cast_or_null<ObjCInterfaceDecl>(IF);
    if (!NSDictionaryDecl && getLangOpts().DebuggerObjCLiteral)
      NSDictionaryDecl =  ObjCInterfaceDecl::Create (Context,
                            Context.getTranslationUnitDecl(),
                            SourceLocation(),
                            NSAPIObj->getNSClassId(NSAPI::ClassId_NSDictionary),
                            nullptr, nullptr, SourceLocation());

    if (!NSDictionaryDecl) {
      Diag(SR.getBegin(), diag::err_undeclared_nsdictionary);
      return ExprError();    
    }
  }
  
  // Find the dictionaryWithObjects:forKeys:count: method, if we haven't done
  // so already.
  QualType IdT = Context.getObjCIdType();
  if (!DictionaryWithObjectsMethod) {
    Selector Sel = NSAPIObj->getNSDictionarySelector(
                               NSAPI::NSDict_dictionaryWithObjectsForKeysCount);
    ObjCMethodDecl *Method = NSDictionaryDecl->lookupClassMethod(Sel);
    if (!Method && getLangOpts().DebuggerObjCLiteral) {
      Method = ObjCMethodDecl::Create(Context,  
                           SourceLocation(), SourceLocation(), Sel,
                           IdT,
                           nullptr /*TypeSourceInfo */,
                           Context.getTranslationUnitDecl(),
                           false /*Instance*/, false/*isVariadic*/,
                           /*isPropertyAccessor=*/false,
                           /*isImplicitlyDeclared=*/true, /*isDefined=*/false,
                           ObjCMethodDecl::Required,
                           false);
      SmallVector<ParmVarDecl *, 3> Params;
      ParmVarDecl *objects = ParmVarDecl::Create(Context, Method,
                                                 SourceLocation(),
                                                 SourceLocation(),
                                                 &Context.Idents.get("objects"),
                                                 Context.getPointerType(IdT),
                                                 /*TInfo=*/nullptr, SC_None,
                                                 nullptr);
      Params.push_back(objects);
      ParmVarDecl *keys = ParmVarDecl::Create(Context, Method,
                                              SourceLocation(),
                                              SourceLocation(),
                                              &Context.Idents.get("keys"),
                                              Context.getPointerType(IdT),
                                              /*TInfo=*/nullptr, SC_None,
                                              nullptr);
      Params.push_back(keys);
      ParmVarDecl *cnt = ParmVarDecl::Create(Context, Method,
                                             SourceLocation(),
                                             SourceLocation(),
                                             &Context.Idents.get("cnt"),
                                             Context.UnsignedLongTy,
                                             /*TInfo=*/nullptr, SC_None,
                                             nullptr);
      Params.push_back(cnt);
      Method->setMethodParams(Context, Params, None);
    }

    if (!validateBoxingMethod(*this, SR.getBegin(), NSDictionaryDecl, Sel,
                              Method))
       return ExprError();

    // Dig out the type that all values should be converted to.
    QualType ValueT = Method->parameters()[0]->getType();
    const PointerType *PtrValue = ValueT->getAs<PointerType>();
    if (!PtrValue || 
        !Context.hasSameUnqualifiedType(PtrValue->getPointeeType(), IdT)) {
      Diag(SR.getBegin(), diag::err_objc_literal_method_sig)
        << Sel;
      Diag(Method->parameters()[0]->getLocation(),
           diag::note_objc_literal_method_param)
        << 0 << ValueT
        << Context.getPointerType(IdT.withConst());
      return ExprError();
    }

    // Dig out the type that all keys should be converted to.
    QualType KeyT = Method->parameters()[1]->getType();
    const PointerType *PtrKey = KeyT->getAs<PointerType>();
    if (!PtrKey || 
        !Context.hasSameUnqualifiedType(PtrKey->getPointeeType(),
                                        IdT)) {
      bool err = true;
      if (PtrKey) {
        if (QIDNSCopying.isNull()) {
          // key argument of selector is id<NSCopying>?
          if (ObjCProtocolDecl *NSCopyingPDecl =
              LookupProtocol(&Context.Idents.get("NSCopying"), SR.getBegin())) {
            ObjCProtocolDecl *PQ[] = {NSCopyingPDecl};
            QIDNSCopying = 
              Context.getObjCObjectType(Context.ObjCBuiltinIdTy, { },
                                        llvm::makeArrayRef(
                                          (ObjCProtocolDecl**) PQ,
                                          1),
                                        false);
            QIDNSCopying = Context.getObjCObjectPointerType(QIDNSCopying);
          }
        }
        if (!QIDNSCopying.isNull())
          err = !Context.hasSameUnqualifiedType(PtrKey->getPointeeType(),
                                                QIDNSCopying);
      }
    
      if (err) {
        Diag(SR.getBegin(), diag::err_objc_literal_method_sig)
          << Sel;
        Diag(Method->parameters()[1]->getLocation(),
             diag::note_objc_literal_method_param)
          << 1 << KeyT
          << Context.getPointerType(IdT.withConst());
        return ExprError();
      }
    }

    // Check that the 'count' parameter is integral.
    QualType CountType = Method->parameters()[2]->getType();
    if (!CountType->isIntegerType()) {
      Diag(SR.getBegin(), diag::err_objc_literal_method_sig)
        << Sel;
      Diag(Method->parameters()[2]->getLocation(),
           diag::note_objc_literal_method_param)
        << 2 << CountType
        << "integral";
      return ExprError();
    }

    // We've found a good +dictionaryWithObjects:keys:count: method; save it!
    DictionaryWithObjectsMethod = Method;
  }

  QualType ValuesT = DictionaryWithObjectsMethod->parameters()[0]->getType();
  QualType ValueT = ValuesT->castAs<PointerType>()->getPointeeType();
  QualType KeysT = DictionaryWithObjectsMethod->parameters()[1]->getType();
  QualType KeyT = KeysT->castAs<PointerType>()->getPointeeType();

  // Check that each of the keys and values provided is valid in a collection 
  // literal, performing conversions as necessary.
  bool HasPackExpansions = false;
  for (unsigned I = 0, N = NumElements; I != N; ++I) {
    // Check the key.
    ExprResult Key = CheckObjCCollectionLiteralElement(*this, Elements[I].Key, 
                                                       KeyT);
    if (Key.isInvalid())
      return ExprError();
    
    // Check the value.
    ExprResult Value
      = CheckObjCCollectionLiteralElement(*this, Elements[I].Value, ValueT);
    if (Value.isInvalid())
      return ExprError();
    
    Elements[I].Key = Key.get();
    Elements[I].Value = Value.get();
    
    if (Elements[I].EllipsisLoc.isInvalid())
      continue;
    
    if (!Elements[I].Key->containsUnexpandedParameterPack() &&
        !Elements[I].Value->containsUnexpandedParameterPack()) {
      Diag(Elements[I].EllipsisLoc, 
           diag::err_pack_expansion_without_parameter_packs)
        << SourceRange(Elements[I].Key->getLocStart(),
                       Elements[I].Value->getLocEnd());
      return ExprError();
    }
    
    HasPackExpansions = true;
  }

  
  QualType Ty
    = Context.getObjCObjectPointerType(
                                Context.getObjCInterfaceType(NSDictionaryDecl));
  return MaybeBindToTemporary(ObjCDictionaryLiteral::Create(
      Context, makeArrayRef(Elements, NumElements), HasPackExpansions, Ty,
      DictionaryWithObjectsMethod, SR));
}

ExprResult Sema::BuildObjCEncodeExpression(SourceLocation AtLoc,
                                      TypeSourceInfo *EncodedTypeInfo,
                                      SourceLocation RParenLoc) {
  QualType EncodedType = EncodedTypeInfo->getType();
  QualType StrTy;
  if (EncodedType->isDependentType())
    StrTy = Context.DependentTy;
  else {
    if (!EncodedType->getAsArrayTypeUnsafe() && //// Incomplete array is handled.
        !EncodedType->isVoidType()) // void is handled too.
      if (RequireCompleteType(AtLoc, EncodedType,
                              diag::err_incomplete_type_objc_at_encode,
                              EncodedTypeInfo->getTypeLoc()))
        return ExprError();

    std::string Str;
    QualType NotEncodedT;
    Context.getObjCEncodingForType(EncodedType, Str, nullptr, &NotEncodedT);
    if (!NotEncodedT.isNull())
      Diag(AtLoc, diag::warn_incomplete_encoded_type)
        << EncodedType << NotEncodedT;

    // The type of @encode is the same as the type of the corresponding string,
    // which is an array type.
    StrTy = Context.CharTy;
    // A C++ string literal has a const-qualified element type (C++ 2.13.4p1).
    if (getLangOpts().CPlusPlus || getLangOpts().ConstStrings)
      StrTy.addConst();
    StrTy = Context.getConstantArrayType(StrTy, llvm::APInt(32, Str.size()+1),
                                         ArrayType::Normal, 0);
  }

  return new (Context) ObjCEncodeExpr(StrTy, EncodedTypeInfo, AtLoc, RParenLoc);
}

ExprResult Sema::ParseObjCEncodeExpression(SourceLocation AtLoc,
                                           SourceLocation EncodeLoc,
                                           SourceLocation LParenLoc,
                                           ParsedType ty,
                                           SourceLocation RParenLoc) {
  // FIXME: Preserve type source info ?
  TypeSourceInfo *TInfo;
  QualType EncodedType = GetTypeFromParser(ty, &TInfo);
  if (!TInfo)
    TInfo = Context.getTrivialTypeSourceInfo(EncodedType,
                                             PP.getLocForEndOfToken(LParenLoc));

  return BuildObjCEncodeExpression(AtLoc, TInfo, RParenLoc);
}

static bool HelperToDiagnoseMismatchedMethodsInGlobalPool(Sema &S,
                                               SourceLocation AtLoc,
                                               SourceLocation LParenLoc,
                                               SourceLocation RParenLoc,
                                               ObjCMethodDecl *Method,
                                               ObjCMethodList &MethList) {
  ObjCMethodList *M = &MethList;
  bool Warned = false;
  for (M = M->getNext(); M; M=M->getNext()) {
    ObjCMethodDecl *MatchingMethodDecl = M->getMethod();
    if (MatchingMethodDecl == Method ||
        isa<ObjCImplDecl>(MatchingMethodDecl->getDeclContext()) ||
        MatchingMethodDecl->getSelector() != Method->getSelector())
      continue;
    if (!S.MatchTwoMethodDeclarations(Method,
                                      MatchingMethodDecl, Sema::MMS_loose)) {
      if (!Warned) {
        Warned = true;
        S.Diag(AtLoc, diag::warning_multiple_selectors)
          << Method->getSelector() << FixItHint::CreateInsertion(LParenLoc, "(")
          << FixItHint::CreateInsertion(RParenLoc, ")");
        S.Diag(Method->getLocation(), diag::note_method_declared_at)
          << Method->getDeclName();
      }
      S.Diag(MatchingMethodDecl->getLocation(), diag::note_method_declared_at)
        << MatchingMethodDecl->getDeclName();
    }
  }
  return Warned;
}

static void DiagnoseMismatchedSelectors(Sema &S, SourceLocation AtLoc,
                                        ObjCMethodDecl *Method,
                                        SourceLocation LParenLoc,
                                        SourceLocation RParenLoc,
                                        bool WarnMultipleSelectors) {
  if (!WarnMultipleSelectors ||
      S.Diags.isIgnored(diag::warning_multiple_selectors, SourceLocation()))
    return;
  bool Warned = false;
  for (Sema::GlobalMethodPool::iterator b = S.MethodPool.begin(),
       e = S.MethodPool.end(); b != e; b++) {
    // first, instance methods
    ObjCMethodList &InstMethList = b->second.first;
    if (HelperToDiagnoseMismatchedMethodsInGlobalPool(S, AtLoc, LParenLoc, RParenLoc,
                                                      Method, InstMethList))
      Warned = true;
        
    // second, class methods
    ObjCMethodList &ClsMethList = b->second.second;
    if (HelperToDiagnoseMismatchedMethodsInGlobalPool(S, AtLoc, LParenLoc, RParenLoc,
                                                      Method, ClsMethList) || Warned)
      return;
  }
}

ExprResult Sema::ParseObjCSelectorExpression(Selector Sel,
                                             SourceLocation AtLoc,
                                             SourceLocation SelLoc,
                                             SourceLocation LParenLoc,
                                             SourceLocation RParenLoc,
                                             bool WarnMultipleSelectors) {
  ObjCMethodDecl *Method = LookupInstanceMethodInGlobalPool(Sel,
                             SourceRange(LParenLoc, RParenLoc));
  if (!Method)
    Method = LookupFactoryMethodInGlobalPool(Sel,
                                          SourceRange(LParenLoc, RParenLoc));
  if (!Method) {
    if (const ObjCMethodDecl *OM = SelectorsForTypoCorrection(Sel)) {
      Selector MatchedSel = OM->getSelector();
      SourceRange SelectorRange(LParenLoc.getLocWithOffset(1),
                                RParenLoc.getLocWithOffset(-1));
      Diag(SelLoc, diag::warn_undeclared_selector_with_typo)
        << Sel << MatchedSel
        << FixItHint::CreateReplacement(SelectorRange, MatchedSel.getAsString());
      
    } else
        Diag(SelLoc, diag::warn_undeclared_selector) << Sel;
  } else
    DiagnoseMismatchedSelectors(*this, AtLoc, Method, LParenLoc, RParenLoc,
                                WarnMultipleSelectors);

  if (Method &&
      Method->getImplementationControl() != ObjCMethodDecl::Optional &&
      !getSourceManager().isInSystemHeader(Method->getLocation()))
    ReferencedSelectors.insert(std::make_pair(Sel, AtLoc));

  // In ARC, forbid the user from using @selector for 
  // retain/release/autorelease/dealloc/retainCount.
  if (getLangOpts().ObjCAutoRefCount) {
    switch (Sel.getMethodFamily()) {
    case OMF_retain:
    case OMF_release:
    case OMF_autorelease:
    case OMF_retainCount:
    case OMF_dealloc:
      Diag(AtLoc, diag::err_arc_illegal_selector) << 
        Sel << SourceRange(LParenLoc, RParenLoc);
      break;

    case OMF_None:
    case OMF_alloc:
    case OMF_copy:
    case OMF_finalize:
    case OMF_init:
    case OMF_mutableCopy:
    case OMF_new:
    case OMF_self:
    case OMF_initialize:
    case OMF_performSelector:
      break;
    }
  }
  QualType Ty = Context.getObjCSelType();
  return new (Context) ObjCSelectorExpr(Ty, Sel, AtLoc, RParenLoc);
}

ExprResult Sema::ParseObjCProtocolExpression(IdentifierInfo *ProtocolId,
                                             SourceLocation AtLoc,
                                             SourceLocation ProtoLoc,
                                             SourceLocation LParenLoc,
                                             SourceLocation ProtoIdLoc,
                                             SourceLocation RParenLoc) {
  ObjCProtocolDecl* PDecl = LookupProtocol(ProtocolId, ProtoIdLoc);
  if (!PDecl) {
    Diag(ProtoLoc, diag::err_undeclared_protocol) << ProtocolId;
    return true;
  }
  if (PDecl->hasDefinition())
    PDecl = PDecl->getDefinition();

  QualType Ty = Context.getObjCProtoType();
  if (Ty.isNull())
    return true;
  Ty = Context.getObjCObjectPointerType(Ty);
  return new (Context) ObjCProtocolExpr(Ty, PDecl, AtLoc, ProtoIdLoc, RParenLoc);
}

/// Try to capture an implicit reference to 'self'.
ObjCMethodDecl *Sema::tryCaptureObjCSelf(SourceLocation Loc) {
  DeclContext *DC = getFunctionLevelDeclContext();

  // If we're not in an ObjC method, error out.  Note that, unlike the
  // C++ case, we don't require an instance method --- class methods
  // still have a 'self', and we really do still need to capture it!
  ObjCMethodDecl *method = dyn_cast<ObjCMethodDecl>(DC);
  if (!method)
    return nullptr;

  tryCaptureVariable(method->getSelfDecl(), Loc);

  return method;
}

static QualType stripObjCInstanceType(ASTContext &Context, QualType T) {
  QualType origType = T;
  if (auto nullability = AttributedType::stripOuterNullability(T)) {
    if (T == Context.getObjCInstanceType()) {
      return Context.getAttributedType(
               AttributedType::getNullabilityAttrKind(*nullability),
               Context.getObjCIdType(),
               Context.getObjCIdType());
    }

    return origType;
  }

  if (T == Context.getObjCInstanceType())
    return Context.getObjCIdType();
  
  return origType;
}

/// Determine the result type of a message send based on the receiver type,
/// method, and the kind of message send.
///
/// This is the "base" result type, which will still need to be adjusted
/// to account for nullability.
static QualType getBaseMessageSendResultType(Sema &S,
                                             QualType ReceiverType,
                                             ObjCMethodDecl *Method,
                                             bool isClassMessage,
                                             bool isSuperMessage) {
  assert(Method && "Must have a method");
  if (!Method->hasRelatedResultType())
    return Method->getSendResultType(ReceiverType);

  ASTContext &Context = S.Context;

  // Local function that transfers the nullability of the method's
  // result type to the returned result.
  auto transferNullability = [&](QualType type) -> QualType {
    // If the method's result type has nullability, extract it.
    if (auto nullability = Method->getSendResultType(ReceiverType)
                             ->getNullability(Context)){
      // Strip off any outer nullability sugar from the provided type.
      (void)AttributedType::stripOuterNullability(type);

      // Form a new attributed type using the method result type's nullability.
      return Context.getAttributedType(
               AttributedType::getNullabilityAttrKind(*nullability),
               type,
               type);
    }

    return type;
  };

  // If a method has a related return type:
  //   - if the method found is an instance method, but the message send
  //     was a class message send, T is the declared return type of the method
  //     found
  if (Method->isInstanceMethod() && isClassMessage)
    return stripObjCInstanceType(Context, 
                                 Method->getSendResultType(ReceiverType));

  //   - if the receiver is super, T is a pointer to the class of the
  //     enclosing method definition
  if (isSuperMessage) {
    if (ObjCMethodDecl *CurMethod = S.getCurMethodDecl())
      if (ObjCInterfaceDecl *Class = CurMethod->getClassInterface()) {
        return transferNullability(
                 Context.getObjCObjectPointerType(
                   Context.getObjCInterfaceType(Class)));
      }
  }

  //   - if the receiver is the name of a class U, T is a pointer to U
  if (ReceiverType->getAsObjCInterfaceType())
    return transferNullability(Context.getObjCObjectPointerType(ReceiverType));
  //   - if the receiver is of type Class or qualified Class type,
  //     T is the declared return type of the method.
  if (ReceiverType->isObjCClassType() ||
      ReceiverType->isObjCQualifiedClassType())
    return stripObjCInstanceType(Context, 
                                 Method->getSendResultType(ReceiverType));

  //   - if the receiver is id, qualified id, Class, or qualified Class, T
  //     is the receiver type, otherwise
  //   - T is the type of the receiver expression.
  return transferNullability(ReceiverType);
}

QualType Sema::getMessageSendResultType(QualType ReceiverType,
                                        ObjCMethodDecl *Method,
                                        bool isClassMessage,
                                        bool isSuperMessage) {
  // Produce the result type.
  QualType resultType = getBaseMessageSendResultType(*this, ReceiverType,
                                                     Method,
                                                     isClassMessage,
                                                     isSuperMessage);

  // If this is a class message, ignore the nullability of the receiver.
  if (isClassMessage)
    return resultType;

  // Map the nullability of the result into a table index.
  unsigned receiverNullabilityIdx = 0;
  if (auto nullability = ReceiverType->getNullability(Context))
    receiverNullabilityIdx = 1 + static_cast<unsigned>(*nullability);

  unsigned resultNullabilityIdx = 0;
  if (auto nullability = resultType->getNullability(Context))
    resultNullabilityIdx = 1 + static_cast<unsigned>(*nullability);

  // The table of nullability mappings, indexed by the receiver's nullability
  // and then the result type's nullability.
  static const uint8_t None = 0;
  static const uint8_t NonNull = 1;
  static const uint8_t Nullable = 2;
  static const uint8_t Unspecified = 3;
  static const uint8_t nullabilityMap[4][4] = {
    //                  None        NonNull       Nullable    Unspecified
    /* None */        { None,       None,         Nullable,   None },
    /* NonNull */     { None,       NonNull,      Nullable,   Unspecified },
    /* Nullable */    { Nullable,   Nullable,     Nullable,   Nullable },
    /* Unspecified */ { None,       Unspecified,  Nullable,   Unspecified }
  };

  unsigned newResultNullabilityIdx
    = nullabilityMap[receiverNullabilityIdx][resultNullabilityIdx];
  if (newResultNullabilityIdx == resultNullabilityIdx)
    return resultType;

  // Strip off the existing nullability. This removes as little type sugar as
  // possible.
  do {
    if (auto attributed = dyn_cast<AttributedType>(resultType.getTypePtr())) {
      resultType = attributed->getModifiedType();
    } else {
      resultType = resultType.getDesugaredType(Context);
    }
  } while (resultType->getNullability(Context));

  // Add nullability back if needed.
  if (newResultNullabilityIdx > 0) {
    auto newNullability
      = static_cast<NullabilityKind>(newResultNullabilityIdx-1);
    return Context.getAttributedType(
             AttributedType::getNullabilityAttrKind(newNullability),
             resultType, resultType);
  }

  return resultType;
}

/// Look for an ObjC method whose result type exactly matches the given type.
static const ObjCMethodDecl *
findExplicitInstancetypeDeclarer(const ObjCMethodDecl *MD,
                                 QualType instancetype) {
  if (MD->getReturnType() == instancetype)
    return MD;

  // For these purposes, a method in an @implementation overrides a
  // declaration in the @interface.
  if (const ObjCImplDecl *impl =
        dyn_cast<ObjCImplDecl>(MD->getDeclContext())) {
    const ObjCContainerDecl *iface;
    if (const ObjCCategoryImplDecl *catImpl = 
          dyn_cast<ObjCCategoryImplDecl>(impl)) {
      iface = catImpl->getCategoryDecl();
    } else {
      iface = impl->getClassInterface();
    }

    const ObjCMethodDecl *ifaceMD = 
      iface->getMethod(MD->getSelector(), MD->isInstanceMethod());
    if (ifaceMD) return findExplicitInstancetypeDeclarer(ifaceMD, instancetype);
  }

  SmallVector<const ObjCMethodDecl *, 4> overrides;
  MD->getOverriddenMethods(overrides);
  for (unsigned i = 0, e = overrides.size(); i != e; ++i) {
    if (const ObjCMethodDecl *result =
          findExplicitInstancetypeDeclarer(overrides[i], instancetype))
      return result;
  }

  return nullptr;
}

void Sema::EmitRelatedResultTypeNoteForReturn(QualType destType) {
  // Only complain if we're in an ObjC method and the required return
  // type doesn't match the method's declared return type.
  ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(CurContext);
  if (!MD || !MD->hasRelatedResultType() ||
      Context.hasSameUnqualifiedType(destType, MD->getReturnType()))
    return;

  // Look for a method overridden by this method which explicitly uses
  // 'instancetype'.
  if (const ObjCMethodDecl *overridden =
        findExplicitInstancetypeDeclarer(MD, Context.getObjCInstanceType())) {
    SourceRange range = overridden->getReturnTypeSourceRange();
    SourceLocation loc = range.getBegin();
    if (loc.isInvalid())
      loc = overridden->getLocation();
    Diag(loc, diag::note_related_result_type_explicit)
      << /*current method*/ 1 << range;
    return;
  }

  // Otherwise, if we have an interesting method family, note that.
  // This should always trigger if the above didn't.
  if (ObjCMethodFamily family = MD->getMethodFamily())
    Diag(MD->getLocation(), diag::note_related_result_type_family)
      << /*current method*/ 1
      << family;
}

void Sema::EmitRelatedResultTypeNote(const Expr *E) {
  E = E->IgnoreParenImpCasts();
  const ObjCMessageExpr *MsgSend = dyn_cast<ObjCMessageExpr>(E);
  if (!MsgSend)
    return;
  
  const ObjCMethodDecl *Method = MsgSend->getMethodDecl();
  if (!Method)
    return;
  
  if (!Method->hasRelatedResultType())
    return;

  if (Context.hasSameUnqualifiedType(
          Method->getReturnType().getNonReferenceType(), MsgSend->getType()))
    return;

  if (!Context.hasSameUnqualifiedType(Method->getReturnType(),
                                      Context.getObjCInstanceType()))
    return;
  
  Diag(Method->getLocation(), diag::note_related_result_type_inferred)
    << Method->isInstanceMethod() << Method->getSelector()
    << MsgSend->getType();
}

bool Sema::CheckMessageArgumentTypes(QualType ReceiverType,
                                     MultiExprArg Args,
                                     Selector Sel,
                                     ArrayRef<SourceLocation> SelectorLocs,
                                     ObjCMethodDecl *Method,
                                     bool isClassMessage, bool isSuperMessage,
                                     SourceLocation lbrac, SourceLocation rbrac,
                                     SourceRange RecRange,
                                     QualType &ReturnType, ExprValueKind &VK) {
  SourceLocation SelLoc;
  if (!SelectorLocs.empty() && SelectorLocs.front().isValid())
    SelLoc = SelectorLocs.front();
  else
    SelLoc = lbrac;

  if (!Method) {
    // Apply default argument promotion as for (C99 6.5.2.2p6).
    for (unsigned i = 0, e = Args.size(); i != e; i++) {
      if (Args[i]->isTypeDependent())
        continue;

      ExprResult result;
      if (getLangOpts().DebuggerSupport) {
        QualType paramTy; // ignored
        result = checkUnknownAnyArg(SelLoc, Args[i], paramTy);
      } else {
        result = DefaultArgumentPromotion(Args[i]);
      }
      if (result.isInvalid())
        return true;
      Args[i] = result.get();
    }

    unsigned DiagID;
    if (getLangOpts().ObjCAutoRefCount)
      DiagID = diag::err_arc_method_not_found;
    else
      DiagID = isClassMessage ? diag::warn_class_method_not_found
                              : diag::warn_inst_method_not_found;
    if (!getLangOpts().DebuggerSupport) {
      const ObjCMethodDecl *OMD = SelectorsForTypoCorrection(Sel, ReceiverType);
      if (OMD && !OMD->isInvalidDecl()) {
        if (getLangOpts().ObjCAutoRefCount)
          DiagID = diag::error_method_not_found_with_typo;
        else
          DiagID = isClassMessage ? diag::warn_class_method_not_found_with_typo
                                  : diag::warn_instance_method_not_found_with_typo;
        Selector MatchedSel = OMD->getSelector();
        SourceRange SelectorRange(SelectorLocs.front(), SelectorLocs.back());
        if (MatchedSel.isUnarySelector())
          Diag(SelLoc, DiagID)
            << Sel<< isClassMessage << MatchedSel
            << FixItHint::CreateReplacement(SelectorRange, MatchedSel.getAsString());
        else
          Diag(SelLoc, DiagID) << Sel<< isClassMessage << MatchedSel;
      }
      else
        Diag(SelLoc, DiagID)
          << Sel << isClassMessage << SourceRange(SelectorLocs.front(), 
                                                SelectorLocs.back());
      // Find the class to which we are sending this message.
      if (ReceiverType->isObjCObjectPointerType()) {
        if (ObjCInterfaceDecl *ThisClass =
            ReceiverType->getAs<ObjCObjectPointerType>()->getInterfaceDecl()) {
          Diag(ThisClass->getLocation(), diag::note_receiver_class_declared);
          if (!RecRange.isInvalid())
            if (ThisClass->lookupClassMethod(Sel))
              Diag(RecRange.getBegin(),diag::note_receiver_expr_here)
                << FixItHint::CreateReplacement(RecRange,
                                                ThisClass->getNameAsString());
        }
      }
    }

    // In debuggers, we want to use __unknown_anytype for these
    // results so that clients can cast them.
    if (getLangOpts().DebuggerSupport) {
      ReturnType = Context.UnknownAnyTy;
    } else {
      ReturnType = Context.getObjCIdType();
    }
    VK = VK_RValue;
    return false;
  }

  ReturnType = getMessageSendResultType(ReceiverType, Method, isClassMessage, 
                                        isSuperMessage);
  VK = Expr::getValueKindForType(Method->getReturnType());

  unsigned NumNamedArgs = Sel.getNumArgs();
  // Method might have more arguments than selector indicates. This is due
  // to addition of c-style arguments in method.
  if (Method->param_size() > Sel.getNumArgs())
    NumNamedArgs = Method->param_size();
  // FIXME. This need be cleaned up.
  if (Args.size() < NumNamedArgs) {
    Diag(SelLoc, diag::err_typecheck_call_too_few_args)
      << 2 << NumNamedArgs << static_cast<unsigned>(Args.size());
    return false;
  }

  // Compute the set of type arguments to be substituted into each parameter
  // type.
  Optional<ArrayRef<QualType>> typeArgs
    = ReceiverType->getObjCSubstitutions(Method->getDeclContext());
  bool IsError = false;
  for (unsigned i = 0; i < NumNamedArgs; i++) {
    // We can't do any type-checking on a type-dependent argument.
    if (Args[i]->isTypeDependent())
      continue;

    Expr *argExpr = Args[i];

    ParmVarDecl *param = Method->parameters()[i];
    assert(argExpr && "CheckMessageArgumentTypes(): missing expression");

    // Strip the unbridged-cast placeholder expression off unless it's
    // a consumed argument.
    if (argExpr->hasPlaceholderType(BuiltinType::ARCUnbridgedCast) &&
        !param->hasAttr<CFConsumedAttr>())
      argExpr = stripARCUnbridgedCast(argExpr);

    // If the parameter is __unknown_anytype, infer its type
    // from the argument.
    if (param->getType() == Context.UnknownAnyTy) {
      QualType paramType;
      ExprResult argE = checkUnknownAnyArg(SelLoc, argExpr, paramType);
      if (argE.isInvalid()) {
        IsError = true;
      } else {
        Args[i] = argE.get();

        // Update the parameter type in-place.
        param->setType(paramType);
      }
      continue;
    }

    QualType origParamType = param->getType();
    QualType paramType = param->getType();
    if (typeArgs)
      paramType = paramType.substObjCTypeArgs(
                    Context,
                    *typeArgs,
                    ObjCSubstitutionContext::Parameter);

    if (RequireCompleteType(argExpr->getSourceRange().getBegin(),
                            paramType,
                            diag::err_call_incomplete_argument, argExpr))
      return true;

    InitializedEntity Entity
      = InitializedEntity::InitializeParameter(Context, param, paramType);
    ExprResult ArgE = PerformCopyInitialization(Entity, SourceLocation(), argExpr);
    if (ArgE.isInvalid())
      IsError = true;
    else {
      Args[i] = ArgE.getAs<Expr>();

      // If we are type-erasing a block to a block-compatible
      // Objective-C pointer type, we may need to extend the lifetime
      // of the block object.
      if (typeArgs && Args[i]->isRValue() && paramType->isBlockPointerType() &&
          origParamType->isBlockCompatibleObjCPointerType(Context)) {
        ExprResult arg = Args[i];
        maybeExtendBlockObject(arg);
        Args[i] = arg.get();
      }
    }
  }

  // Promote additional arguments to variadic methods.
  if (Method->isVariadic()) {
    for (unsigned i = NumNamedArgs, e = Args.size(); i < e; ++i) {
      if (Args[i]->isTypeDependent())
        continue;

      ExprResult Arg = DefaultVariadicArgumentPromotion(Args[i], VariadicMethod,
                                                        nullptr);
      IsError |= Arg.isInvalid();
      Args[i] = Arg.get();
    }
  } else {
    // Check for extra arguments to non-variadic methods.
    if (Args.size() != NumNamedArgs) {
      Diag(Args[NumNamedArgs]->getLocStart(),
           diag::err_typecheck_call_too_many_args)
        << 2 /*method*/ << NumNamedArgs << static_cast<unsigned>(Args.size())
        << Method->getSourceRange()
        << SourceRange(Args[NumNamedArgs]->getLocStart(),
                       Args.back()->getLocEnd());
    }
  }

  DiagnoseSentinelCalls(Method, SelLoc, Args);

  // Do additional checkings on method.
  IsError |= CheckObjCMethodCall(
      Method, SelLoc, makeArrayRef(Args.data(), Args.size()));

  return IsError;
}

bool Sema::isSelfExpr(Expr *RExpr) {
  // 'self' is objc 'self' in an objc method only.
  ObjCMethodDecl *Method =
      dyn_cast_or_null<ObjCMethodDecl>(CurContext->getNonClosureAncestor());
  return isSelfExpr(RExpr, Method);
}

bool Sema::isSelfExpr(Expr *receiver, const ObjCMethodDecl *method) {
  if (!method) return false;

  receiver = receiver->IgnoreParenLValueCasts();
  if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(receiver))
    if (DRE->getDecl() == method->getSelfDecl())
      return true;
  return false;
}

/// LookupMethodInType - Look up a method in an ObjCObjectType.
ObjCMethodDecl *Sema::LookupMethodInObjectType(Selector sel, QualType type,
                                               bool isInstance) {
  const ObjCObjectType *objType = type->castAs<ObjCObjectType>();
  if (ObjCInterfaceDecl *iface = objType->getInterface()) {
    // Look it up in the main interface (and categories, etc.)
    if (ObjCMethodDecl *method = iface->lookupMethod(sel, isInstance))
      return method;

    // Okay, look for "private" methods declared in any
    // @implementations we've seen.
    if (ObjCMethodDecl *method = iface->lookupPrivateMethod(sel, isInstance))
      return method;
  }

  // Check qualifiers.
  for (const auto *I : objType->quals())
    if (ObjCMethodDecl *method = I->lookupMethod(sel, isInstance))
      return method;

  return nullptr;
}

/// LookupMethodInQualifiedType - Lookups up a method in protocol qualifier 
/// list of a qualified objective pointer type.
ObjCMethodDecl *Sema::LookupMethodInQualifiedType(Selector Sel,
                                              const ObjCObjectPointerType *OPT,
                                              bool Instance)
{
  ObjCMethodDecl *MD = nullptr;
  for (const auto *PROTO : OPT->quals()) {
    if ((MD = PROTO->lookupMethod(Sel, Instance))) {
      return MD;
    }
  }
  return nullptr;
}

/// HandleExprPropertyRefExpr - Handle foo.bar where foo is a pointer to an
/// objective C interface.  This is a property reference expression.
ExprResult Sema::
HandleExprPropertyRefExpr(const ObjCObjectPointerType *OPT,
                          Expr *BaseExpr, SourceLocation OpLoc,
                          DeclarationName MemberName,
                          SourceLocation MemberLoc,
                          SourceLocation SuperLoc, QualType SuperType,
                          bool Super) {
  const ObjCInterfaceType *IFaceT = OPT->getInterfaceType();
  ObjCInterfaceDecl *IFace = IFaceT->getDecl();

  if (!MemberName.isIdentifier()) {
    Diag(MemberLoc, diag::err_invalid_property_name)
      << MemberName << QualType(OPT, 0);
    return ExprError();
  }

  IdentifierInfo *Member = MemberName.getAsIdentifierInfo();
  
  SourceRange BaseRange = Super? SourceRange(SuperLoc)
                               : BaseExpr->getSourceRange();
  if (RequireCompleteType(MemberLoc, OPT->getPointeeType(), 
                          diag::err_property_not_found_forward_class,
                          MemberName, BaseRange))
    return ExprError();
  
  // Search for a declared property first.
  if (ObjCPropertyDecl *PD = IFace->FindPropertyDeclaration(Member)) {
    // Check whether we can reference this property.
    if (DiagnoseUseOfDecl(PD, MemberLoc))
      return ExprError();
    if (Super)
      return new (Context)
          ObjCPropertyRefExpr(PD, Context.PseudoObjectTy, VK_LValue,
                              OK_ObjCProperty, MemberLoc, SuperLoc, SuperType);
    else
      return new (Context)
          ObjCPropertyRefExpr(PD, Context.PseudoObjectTy, VK_LValue,
                              OK_ObjCProperty, MemberLoc, BaseExpr);
  }
  // Check protocols on qualified interfaces.
  for (const auto *I : OPT->quals())
    if (ObjCPropertyDecl *PD = I->FindPropertyDeclaration(Member)) {
      // Check whether we can reference this property.
      if (DiagnoseUseOfDecl(PD, MemberLoc))
        return ExprError();

      if (Super)
        return new (Context) ObjCPropertyRefExpr(
            PD, Context.PseudoObjectTy, VK_LValue, OK_ObjCProperty, MemberLoc,
            SuperLoc, SuperType);
      else
        return new (Context)
            ObjCPropertyRefExpr(PD, Context.PseudoObjectTy, VK_LValue,
                                OK_ObjCProperty, MemberLoc, BaseExpr);
    }
  // If that failed, look for an "implicit" property by seeing if the nullary
  // selector is implemented.

  // FIXME: The logic for looking up nullary and unary selectors should be
  // shared with the code in ActOnInstanceMessage.

  Selector Sel = PP.getSelectorTable().getNullarySelector(Member);
  ObjCMethodDecl *Getter = IFace->lookupInstanceMethod(Sel);
  
  // May be founf in property's qualified list.
  if (!Getter)
    Getter = LookupMethodInQualifiedType(Sel, OPT, true);

  // If this reference is in an @implementation, check for 'private' methods.
  if (!Getter)
    Getter = IFace->lookupPrivateMethod(Sel);

  if (Getter) {
    // Check if we can reference this property.
    if (DiagnoseUseOfDecl(Getter, MemberLoc))
      return ExprError();
  }
  // If we found a getter then this may be a valid dot-reference, we
  // will look for the matching setter, in case it is needed.
  Selector SetterSel =
    SelectorTable::constructSetterSelector(PP.getIdentifierTable(),
                                           PP.getSelectorTable(), Member);
  ObjCMethodDecl *Setter = IFace->lookupInstanceMethod(SetterSel);
      
  // May be founf in property's qualified list.
  if (!Setter)
    Setter = LookupMethodInQualifiedType(SetterSel, OPT, true);
  
  if (!Setter) {
    // If this reference is in an @implementation, also check for 'private'
    // methods.
    Setter = IFace->lookupPrivateMethod(SetterSel);
  }
    
  if (Setter && DiagnoseUseOfDecl(Setter, MemberLoc))
    return ExprError();

  // Special warning if member name used in a property-dot for a setter accessor
  // does not use a property with same name; e.g. obj.X = ... for a property with
  // name 'x'.
  if (Setter && Setter->isImplicit() && Setter->isPropertyAccessor()
      && !IFace->FindPropertyDeclaration(Member)) {
      if (const ObjCPropertyDecl *PDecl = Setter->findPropertyDecl()) {
        // Do not warn if user is using property-dot syntax to make call to
        // user named setter.
        if (!(PDecl->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_setter))
          Diag(MemberLoc,
               diag::warn_property_access_suggest)
          << MemberName << QualType(OPT, 0) << PDecl->getName()
          << FixItHint::CreateReplacement(MemberLoc, PDecl->getName());
      }
  }

  if (Getter || Setter) {
    if (Super)
      return new (Context)
          ObjCPropertyRefExpr(Getter, Setter, Context.PseudoObjectTy, VK_LValue,
                              OK_ObjCProperty, MemberLoc, SuperLoc, SuperType);
    else
      return new (Context)
          ObjCPropertyRefExpr(Getter, Setter, Context.PseudoObjectTy, VK_LValue,
                              OK_ObjCProperty, MemberLoc, BaseExpr);

  }

  // Attempt to correct for typos in property names.
  if (TypoCorrection Corrected =
          CorrectTypo(DeclarationNameInfo(MemberName, MemberLoc),
                      LookupOrdinaryName, nullptr, nullptr,
                      llvm::make_unique<DeclFilterCCC<ObjCPropertyDecl>>(),
                      CTK_ErrorRecovery, IFace, false, OPT)) {
    diagnoseTypo(Corrected, PDiag(diag::err_property_not_found_suggest)
                              << MemberName << QualType(OPT, 0));
    DeclarationName TypoResult = Corrected.getCorrection();
    return HandleExprPropertyRefExpr(OPT, BaseExpr, OpLoc,
                                     TypoResult, MemberLoc,
                                     SuperLoc, SuperType, Super);
  }
  ObjCInterfaceDecl *ClassDeclared;
  if (ObjCIvarDecl *Ivar = 
      IFace->lookupInstanceVariable(Member, ClassDeclared)) {
    QualType T = Ivar->getType();
    if (const ObjCObjectPointerType * OBJPT = 
        T->getAsObjCInterfacePointerType()) {
      if (RequireCompleteType(MemberLoc, OBJPT->getPointeeType(), 
                              diag::err_property_not_as_forward_class,
                              MemberName, BaseExpr))
        return ExprError();
    }
    Diag(MemberLoc, 
         diag::err_ivar_access_using_property_syntax_suggest)
    << MemberName << QualType(OPT, 0) << Ivar->getDeclName()
    << FixItHint::CreateReplacement(OpLoc, "->");
    return ExprError();
  }
  
  Diag(MemberLoc, diag::err_property_not_found)
    << MemberName << QualType(OPT, 0);
  if (Setter)
    Diag(Setter->getLocation(), diag::note_getter_unavailable)
          << MemberName << BaseExpr->getSourceRange();
  return ExprError();
}



ExprResult Sema::
ActOnClassPropertyRefExpr(IdentifierInfo &receiverName,
                          IdentifierInfo &propertyName,
                          SourceLocation receiverNameLoc,
                          SourceLocation propertyNameLoc) {

  IdentifierInfo *receiverNamePtr = &receiverName;
  ObjCInterfaceDecl *IFace = getObjCInterfaceDecl(receiverNamePtr,
                                                  receiverNameLoc);

  QualType SuperType;
  if (!IFace) {
    // If the "receiver" is 'super' in a method, handle it as an expression-like
    // property reference.
    if (receiverNamePtr->isStr("super")) {
      if (ObjCMethodDecl *CurMethod = tryCaptureObjCSelf(receiverNameLoc)) {
        if (auto classDecl = CurMethod->getClassInterface()) {
          SuperType = QualType(classDecl->getSuperClassType(), 0);
          if (CurMethod->isInstanceMethod()) {
            if (SuperType.isNull()) {
              // The current class does not have a superclass.
              Diag(receiverNameLoc, diag::error_root_class_cannot_use_super)
                << CurMethod->getClassInterface()->getIdentifier();
              return ExprError();
            }
            QualType T = Context.getObjCObjectPointerType(SuperType);

            return HandleExprPropertyRefExpr(T->castAs<ObjCObjectPointerType>(),
                                             /*BaseExpr*/nullptr,
                                             SourceLocation()/*OpLoc*/,
                                             &propertyName,
                                             propertyNameLoc,
                                             receiverNameLoc, T, true);
          }

          // Otherwise, if this is a class method, try dispatching to our
          // superclass.
          IFace = CurMethod->getClassInterface()->getSuperClass();
        }
      }
    }

    if (!IFace) {
      Diag(receiverNameLoc, diag::err_expected_either) << tok::identifier
                                                       << tok::l_paren;
      return ExprError();
    }
  }

  // Search for a declared property first.
  Selector Sel = PP.getSelectorTable().getNullarySelector(&propertyName);
  ObjCMethodDecl *Getter = IFace->lookupClassMethod(Sel);

  // If this reference is in an @implementation, check for 'private' methods.
  if (!Getter)
    Getter = IFace->lookupPrivateClassMethod(Sel);

  if (Getter) {
    // FIXME: refactor/share with ActOnMemberReference().
    // Check if we can reference this property.
    if (DiagnoseUseOfDecl(Getter, propertyNameLoc))
      return ExprError();
  }

  // Look for the matching setter, in case it is needed.
  Selector SetterSel =
    SelectorTable::constructSetterSelector(PP.getIdentifierTable(),
                                            PP.getSelectorTable(),
                                           &propertyName);

  ObjCMethodDecl *Setter = IFace->lookupClassMethod(SetterSel);
  if (!Setter) {
    // If this reference is in an @implementation, also check for 'private'
    // methods.
    Setter = IFace->lookupPrivateClassMethod(SetterSel);
  }
  // Look through local category implementations associated with the class.
  if (!Setter)
    Setter = IFace->getCategoryClassMethod(SetterSel);

  if (Setter && DiagnoseUseOfDecl(Setter, propertyNameLoc))
    return ExprError();

  if (Getter || Setter) {
    if (!SuperType.isNull())
      return new (Context)
          ObjCPropertyRefExpr(Getter, Setter, Context.PseudoObjectTy, VK_LValue,
                              OK_ObjCProperty, propertyNameLoc, receiverNameLoc,
                              SuperType);

    return new (Context) ObjCPropertyRefExpr(
        Getter, Setter, Context.PseudoObjectTy, VK_LValue, OK_ObjCProperty,
        propertyNameLoc, receiverNameLoc, IFace);
  }
  return ExprError(Diag(propertyNameLoc, diag::err_property_not_found)
                     << &propertyName << Context.getObjCInterfaceType(IFace));
}

namespace {

class ObjCInterfaceOrSuperCCC : public CorrectionCandidateCallback {
 public:
  ObjCInterfaceOrSuperCCC(ObjCMethodDecl *Method) {
    // Determine whether "super" is acceptable in the current context.
    if (Method && Method->getClassInterface())
      WantObjCSuper = Method->getClassInterface()->getSuperClass();
  }

  bool ValidateCandidate(const TypoCorrection &candidate) override {
    return candidate.getCorrectionDeclAs<ObjCInterfaceDecl>() ||
        candidate.isKeyword("super");
  }
};

}

Sema::ObjCMessageKind Sema::getObjCMessageKind(Scope *S,
                                               IdentifierInfo *Name,
                                               SourceLocation NameLoc,
                                               bool IsSuper,
                                               bool HasTrailingDot,
                                               ParsedType &ReceiverType) {
  ReceiverType = ParsedType();

  // If the identifier is "super" and there is no trailing dot, we're
  // messaging super. If the identifier is "super" and there is a
  // trailing dot, it's an instance message.
  if (IsSuper && S->isInObjcMethodScope())
    return HasTrailingDot? ObjCInstanceMessage : ObjCSuperMessage;
  
  LookupResult Result(*this, Name, NameLoc, LookupOrdinaryName);
  LookupName(Result, S);
  
  switch (Result.getResultKind()) {
  case LookupResult::NotFound:
    // Normal name lookup didn't find anything. If we're in an
    // Objective-C method, look for ivars. If we find one, we're done!
    // FIXME: This is a hack. Ivar lookup should be part of normal
    // lookup.
    if (ObjCMethodDecl *Method = getCurMethodDecl()) {
      if (!Method->getClassInterface()) {
        // Fall back: let the parser try to parse it as an instance message.
        return ObjCInstanceMessage;
      }

      ObjCInterfaceDecl *ClassDeclared;
      if (Method->getClassInterface()->lookupInstanceVariable(Name, 
                                                              ClassDeclared))
        return ObjCInstanceMessage;
    }
  
    // Break out; we'll perform typo correction below.
    break;

  case LookupResult::NotFoundInCurrentInstantiation:
  case LookupResult::FoundOverloaded:
  case LookupResult::FoundUnresolvedValue:
  case LookupResult::Ambiguous:
    Result.suppressDiagnostics();
    return ObjCInstanceMessage;

  case LookupResult::Found: {
    // If the identifier is a class or not, and there is a trailing dot,
    // it's an instance message.
    if (HasTrailingDot)
      return ObjCInstanceMessage;
    // We found something. If it's a type, then we have a class
    // message. Otherwise, it's an instance message.
    NamedDecl *ND = Result.getFoundDecl();
    QualType T;
    if (ObjCInterfaceDecl *Class = dyn_cast<ObjCInterfaceDecl>(ND))
      T = Context.getObjCInterfaceType(Class);
    else if (TypeDecl *Type = dyn_cast<TypeDecl>(ND)) {
      T = Context.getTypeDeclType(Type);
      DiagnoseUseOfDecl(Type, NameLoc);
    }
    else
      return ObjCInstanceMessage;

    //  We have a class message, and T is the type we're
    //  messaging. Build source-location information for it.
    TypeSourceInfo *TSInfo = Context.getTrivialTypeSourceInfo(T, NameLoc);
    ReceiverType = CreateParsedType(T, TSInfo);
    return ObjCClassMessage;
  }
  }

  if (TypoCorrection Corrected = CorrectTypo(
          Result.getLookupNameInfo(), Result.getLookupKind(), S, nullptr,
          llvm::make_unique<ObjCInterfaceOrSuperCCC>(getCurMethodDecl()),
          CTK_ErrorRecovery, nullptr, false, nullptr, false)) {
    if (Corrected.isKeyword()) {
      // If we've found the keyword "super" (the only keyword that would be
      // returned by CorrectTypo), this is a send to super.
      diagnoseTypo(Corrected,
                   PDiag(diag::err_unknown_receiver_suggest) << Name);
      return ObjCSuperMessage;
    } else if (ObjCInterfaceDecl *Class =
                   Corrected.getCorrectionDeclAs<ObjCInterfaceDecl>()) {
      // If we found a declaration, correct when it refers to an Objective-C
      // class.
      diagnoseTypo(Corrected,
                   PDiag(diag::err_unknown_receiver_suggest) << Name);
      QualType T = Context.getObjCInterfaceType(Class);
      TypeSourceInfo *TSInfo = Context.getTrivialTypeSourceInfo(T, NameLoc);
      ReceiverType = CreateParsedType(T, TSInfo);
      return ObjCClassMessage;
    }
  }

  // Fall back: let the parser try to parse it as an instance message.
  return ObjCInstanceMessage;
}

ExprResult Sema::ActOnSuperMessage(Scope *S, 
                                   SourceLocation SuperLoc,
                                   Selector Sel,
                                   SourceLocation LBracLoc,
                                   ArrayRef<SourceLocation> SelectorLocs,
                                   SourceLocation RBracLoc,
                                   MultiExprArg Args) {
  // Determine whether we are inside a method or not.
  ObjCMethodDecl *Method = tryCaptureObjCSelf(SuperLoc);
  if (!Method) {
    Diag(SuperLoc, diag::err_invalid_receiver_to_message_super);
    return ExprError();
  }

  ObjCInterfaceDecl *Class = Method->getClassInterface();
  if (!Class) {
    Diag(SuperLoc, diag::error_no_super_class_message)
      << Method->getDeclName();
    return ExprError();
  }

  QualType SuperTy(Class->getSuperClassType(), 0);
  if (SuperTy.isNull()) {
    // The current class does not have a superclass.
    Diag(SuperLoc, diag::error_root_class_cannot_use_super)
      << Class->getIdentifier();
    return ExprError();
  }

  // We are in a method whose class has a superclass, so 'super'
  // is acting as a keyword.
  if (Method->getSelector() == Sel)
    getCurFunction()->ObjCShouldCallSuper = false;

  if (Method->isInstanceMethod()) {
    // Since we are in an instance method, this is an instance
    // message to the superclass instance.
    SuperTy = Context.getObjCObjectPointerType(SuperTy);
    return BuildInstanceMessage(nullptr, SuperTy, SuperLoc,
                                Sel, /*Method=*/nullptr,
                                LBracLoc, SelectorLocs, RBracLoc, Args);
  }
  
  // Since we are in a class method, this is a class message to
  // the superclass.
  return BuildClassMessage(/*ReceiverTypeInfo=*/nullptr,
                           SuperTy,
                           SuperLoc, Sel, /*Method=*/nullptr,
                           LBracLoc, SelectorLocs, RBracLoc, Args);
}


ExprResult Sema::BuildClassMessageImplicit(QualType ReceiverType,
                                           bool isSuperReceiver,
                                           SourceLocation Loc,
                                           Selector Sel,
                                           ObjCMethodDecl *Method,
                                           MultiExprArg Args) {
  TypeSourceInfo *receiverTypeInfo = nullptr;
  if (!ReceiverType.isNull())
    receiverTypeInfo = Context.getTrivialTypeSourceInfo(ReceiverType);

  return BuildClassMessage(receiverTypeInfo, ReceiverType,
                          /*SuperLoc=*/isSuperReceiver ? Loc : SourceLocation(),
                           Sel, Method, Loc, Loc, Loc, Args,
                           /*isImplicit=*/true);

}

static void applyCocoaAPICheck(Sema &S, const ObjCMessageExpr *Msg,
                               unsigned DiagID,
                               bool (*refactor)(const ObjCMessageExpr *,
                                              const NSAPI &, edit::Commit &)) {
  SourceLocation MsgLoc = Msg->getExprLoc();
  if (S.Diags.isIgnored(DiagID, MsgLoc))
    return;

  SourceManager &SM = S.SourceMgr;
  edit::Commit ECommit(SM, S.LangOpts);
  if (refactor(Msg,*S.NSAPIObj, ECommit)) {
    DiagnosticBuilder Builder = S.Diag(MsgLoc, DiagID)
                        << Msg->getSelector() << Msg->getSourceRange();
    // FIXME: Don't emit diagnostic at all if fixits are non-commitable.
    if (!ECommit.isCommitable())
      return;
    for (edit::Commit::edit_iterator
           I = ECommit.edit_begin(), E = ECommit.edit_end(); I != E; ++I) {
      const edit::Commit::Edit &Edit = *I;
      switch (Edit.Kind) {
      case edit::Commit::Act_Insert:
        Builder.AddFixItHint(FixItHint::CreateInsertion(Edit.OrigLoc,
                                                        Edit.Text,
                                                        Edit.BeforePrev));
        break;
      case edit::Commit::Act_InsertFromRange:
        Builder.AddFixItHint(
            FixItHint::CreateInsertionFromRange(Edit.OrigLoc,
                                                Edit.getInsertFromRange(SM),
                                                Edit.BeforePrev));
        break;
      case edit::Commit::Act_Remove:
        Builder.AddFixItHint(FixItHint::CreateRemoval(Edit.getFileRange(SM)));
        break;
      }
    }
  }
}

static void checkCocoaAPI(Sema &S, const ObjCMessageExpr *Msg) {
  applyCocoaAPICheck(S, Msg, diag::warn_objc_redundant_literal_use,
                     edit::rewriteObjCRedundantCallWithLiteral);
}

/// \brief Diagnose use of %s directive in an NSString which is being passed
/// as formatting string to formatting method.
static void
DiagnoseCStringFormatDirectiveInObjCAPI(Sema &S,
                                        ObjCMethodDecl *Method,
                                        Selector Sel,
                                        Expr **Args, unsigned NumArgs) {
  unsigned Idx = 0;
  bool Format = false;
  ObjCStringFormatFamily SFFamily = Sel.getStringFormatFamily();
  if (SFFamily == ObjCStringFormatFamily::SFF_NSString) {
    Idx = 0;
    Format = true;
  }
  else if (Method) {
    for (const auto *I : Method->specific_attrs<FormatAttr>()) {
      if (S.GetFormatNSStringIdx(I, Idx)) {
        Format = true;
        break;
      }
    }
  }
  if (!Format || NumArgs <= Idx)
    return;
  
  Expr *FormatExpr = Args[Idx];
  if (ObjCStringLiteral *OSL =
      dyn_cast<ObjCStringLiteral>(FormatExpr->IgnoreParenImpCasts())) {
    StringLiteral *FormatString = OSL->getString();
    if (S.FormatStringHasSArg(FormatString)) {
      S.Diag(FormatExpr->getExprLoc(), diag::warn_objc_cdirective_format_string)
        << "%s" << 0 << 0;
      if (Method)
        S.Diag(Method->getLocation(), diag::note_method_declared_at)
          << Method->getDeclName();
    }
  }
}

/// \brief Build an Objective-C class message expression.
///
/// This routine takes care of both normal class messages and
/// class messages to the superclass.
///
/// \param ReceiverTypeInfo Type source information that describes the
/// receiver of this message. This may be NULL, in which case we are
/// sending to the superclass and \p SuperLoc must be a valid source
/// location.

/// \param ReceiverType The type of the object receiving the
/// message. When \p ReceiverTypeInfo is non-NULL, this is the same
/// type as that refers to. For a superclass send, this is the type of
/// the superclass.
///
/// \param SuperLoc The location of the "super" keyword in a
/// superclass message.
///
/// \param Sel The selector to which the message is being sent.
///
/// \param Method The method that this class message is invoking, if
/// already known.
///
/// \param LBracLoc The location of the opening square bracket ']'.
///
/// \param RBracLoc The location of the closing square bracket ']'.
///
/// \param ArgsIn The message arguments.
ExprResult Sema::BuildClassMessage(TypeSourceInfo *ReceiverTypeInfo,
                                   QualType ReceiverType,
                                   SourceLocation SuperLoc,
                                   Selector Sel,
                                   ObjCMethodDecl *Method,
                                   SourceLocation LBracLoc, 
                                   ArrayRef<SourceLocation> SelectorLocs,
                                   SourceLocation RBracLoc,
                                   MultiExprArg ArgsIn,
                                   bool isImplicit) {
  SourceLocation Loc = SuperLoc.isValid()? SuperLoc
    : ReceiverTypeInfo->getTypeLoc().getSourceRange().getBegin();
  if (LBracLoc.isInvalid()) {
    Diag(Loc, diag::err_missing_open_square_message_send)
      << FixItHint::CreateInsertion(Loc, "[");
    LBracLoc = Loc;
  }
  SourceLocation SelLoc;
  if (!SelectorLocs.empty() && SelectorLocs.front().isValid())
    SelLoc = SelectorLocs.front();
  else
    SelLoc = Loc;

  if (ReceiverType->isDependentType()) {
    // If the receiver type is dependent, we can't type-check anything
    // at this point. Build a dependent expression.
    unsigned NumArgs = ArgsIn.size();
    Expr **Args = ArgsIn.data();
    assert(SuperLoc.isInvalid() && "Message to super with dependent type");
    return ObjCMessageExpr::Create(
        Context, ReceiverType, VK_RValue, LBracLoc, ReceiverTypeInfo, Sel,
        SelectorLocs, /*Method=*/nullptr, makeArrayRef(Args, NumArgs), RBracLoc,
        isImplicit);
  }
  
  // Find the class to which we are sending this message.
  ObjCInterfaceDecl *Class = nullptr;
  const ObjCObjectType *ClassType = ReceiverType->getAs<ObjCObjectType>();
  if (!ClassType || !(Class = ClassType->getInterface())) {
    Diag(Loc, diag::err_invalid_receiver_class_message)
      << ReceiverType;
    return ExprError();
  }
  assert(Class && "We don't know which class we're messaging?");
  // objc++ diagnoses during typename annotation.
  if (!getLangOpts().CPlusPlus)
    (void)DiagnoseUseOfDecl(Class, SelLoc);
  // Find the method we are messaging.
  if (!Method) {
    SourceRange TypeRange 
      = SuperLoc.isValid()? SourceRange(SuperLoc)
                          : ReceiverTypeInfo->getTypeLoc().getSourceRange();
    if (RequireCompleteType(Loc, Context.getObjCInterfaceType(Class),
                            (getLangOpts().ObjCAutoRefCount
                               ? diag::err_arc_receiver_forward_class
                               : diag::warn_receiver_forward_class),
                            TypeRange)) {
      // A forward class used in messaging is treated as a 'Class'
      Method = LookupFactoryMethodInGlobalPool(Sel, 
                                               SourceRange(LBracLoc, RBracLoc));
      if (Method && !getLangOpts().ObjCAutoRefCount)
        Diag(Method->getLocation(), diag::note_method_sent_forward_class)
          << Method->getDeclName();
    }
    if (!Method)
      Method = Class->lookupClassMethod(Sel);

    // If we have an implementation in scope, check "private" methods.
    if (!Method)
      Method = Class->lookupPrivateClassMethod(Sel);

    if (Method && DiagnoseUseOfDecl(Method, SelLoc))
      return ExprError();
  }

  // Check the argument types and determine the result type.
  QualType ReturnType;
  ExprValueKind VK = VK_RValue;

  unsigned NumArgs = ArgsIn.size();
  Expr **Args = ArgsIn.data();
  if (CheckMessageArgumentTypes(ReceiverType, MultiExprArg(Args, NumArgs),
                                Sel, SelectorLocs,
                                Method, true,
                                SuperLoc.isValid(), LBracLoc, RBracLoc,
                                SourceRange(),
                                ReturnType, VK))
    return ExprError();

  if (Method && !Method->getReturnType()->isVoidType() &&
      RequireCompleteType(LBracLoc, Method->getReturnType(),
                          diag::err_illegal_message_expr_incomplete_type))
    return ExprError();
  
  // Warn about explicit call of +initialize on its own class. But not on 'super'.
  if (Method && Method->getMethodFamily() == OMF_initialize) {
    if (!SuperLoc.isValid()) {
      const ObjCInterfaceDecl *ID =
        dyn_cast<ObjCInterfaceDecl>(Method->getDeclContext());
      if (ID == Class) {
        Diag(Loc, diag::warn_direct_initialize_call);
        Diag(Method->getLocation(), diag::note_method_declared_at)
          << Method->getDeclName();
      }
    }
    else if (ObjCMethodDecl *CurMeth = getCurMethodDecl()) {
      // [super initialize] is allowed only within an +initialize implementation
      if (CurMeth->getMethodFamily() != OMF_initialize) {
        Diag(Loc, diag::warn_direct_super_initialize_call);
        Diag(Method->getLocation(), diag::note_method_declared_at)
          << Method->getDeclName();
        Diag(CurMeth->getLocation(), diag::note_method_declared_at)
        << CurMeth->getDeclName();
      }
    }
  }
  
  DiagnoseCStringFormatDirectiveInObjCAPI(*this, Method, Sel, Args, NumArgs);
  
  // Construct the appropriate ObjCMessageExpr.
  ObjCMessageExpr *Result;
  if (SuperLoc.isValid())
    Result = ObjCMessageExpr::Create(Context, ReturnType, VK, LBracLoc, 
                                     SuperLoc, /*IsInstanceSuper=*/false, 
                                     ReceiverType, Sel, SelectorLocs,
                                     Method, makeArrayRef(Args, NumArgs),
                                     RBracLoc, isImplicit);
  else {
    Result = ObjCMessageExpr::Create(Context, ReturnType, VK, LBracLoc, 
                                     ReceiverTypeInfo, Sel, SelectorLocs,
                                     Method, makeArrayRef(Args, NumArgs),
                                     RBracLoc, isImplicit);
    if (!isImplicit)
      checkCocoaAPI(*this, Result);
  }
  return MaybeBindToTemporary(Result);
}

// ActOnClassMessage - used for both unary and keyword messages.
// ArgExprs is optional - if it is present, the number of expressions
// is obtained from Sel.getNumArgs().
ExprResult Sema::ActOnClassMessage(Scope *S, 
                                   ParsedType Receiver,
                                   Selector Sel,
                                   SourceLocation LBracLoc,
                                   ArrayRef<SourceLocation> SelectorLocs,
                                   SourceLocation RBracLoc,
                                   MultiExprArg Args) {
  TypeSourceInfo *ReceiverTypeInfo;
  QualType ReceiverType = GetTypeFromParser(Receiver, &ReceiverTypeInfo);
  if (ReceiverType.isNull())
    return ExprError();


  if (!ReceiverTypeInfo)
    ReceiverTypeInfo = Context.getTrivialTypeSourceInfo(ReceiverType, LBracLoc);

  return BuildClassMessage(ReceiverTypeInfo, ReceiverType, 
                           /*SuperLoc=*/SourceLocation(), Sel,
                           /*Method=*/nullptr, LBracLoc, SelectorLocs, RBracLoc,
                           Args);
}

ExprResult Sema::BuildInstanceMessageImplicit(Expr *Receiver,
                                              QualType ReceiverType,
                                              SourceLocation Loc,
                                              Selector Sel,
                                              ObjCMethodDecl *Method,
                                              MultiExprArg Args) {
  return BuildInstanceMessage(Receiver, ReceiverType,
                              /*SuperLoc=*/!Receiver ? Loc : SourceLocation(),
                              Sel, Method, Loc, Loc, Loc, Args,
                              /*isImplicit=*/true);
}

/// \brief Build an Objective-C instance message expression.
///
/// This routine takes care of both normal instance messages and
/// instance messages to the superclass instance.
///
/// \param Receiver The expression that computes the object that will
/// receive this message. This may be empty, in which case we are
/// sending to the superclass instance and \p SuperLoc must be a valid
/// source location.
///
/// \param ReceiverType The (static) type of the object receiving the
/// message. When a \p Receiver expression is provided, this is the
/// same type as that expression. For a superclass instance send, this
/// is a pointer to the type of the superclass.
///
/// \param SuperLoc The location of the "super" keyword in a
/// superclass instance message.
///
/// \param Sel The selector to which the message is being sent.
///
/// \param Method The method that this instance message is invoking, if
/// already known.
///
/// \param LBracLoc The location of the opening square bracket ']'.
///
/// \param RBracLoc The location of the closing square bracket ']'.
///
/// \param ArgsIn The message arguments.
ExprResult Sema::BuildInstanceMessage(Expr *Receiver,
                                      QualType ReceiverType,
                                      SourceLocation SuperLoc,
                                      Selector Sel,
                                      ObjCMethodDecl *Method,
                                      SourceLocation LBracLoc, 
                                      ArrayRef<SourceLocation> SelectorLocs,
                                      SourceLocation RBracLoc,
                                      MultiExprArg ArgsIn,
                                      bool isImplicit) {
  // The location of the receiver.
  SourceLocation Loc = SuperLoc.isValid()? SuperLoc : Receiver->getLocStart();
  SourceRange RecRange =
      SuperLoc.isValid()? SuperLoc : Receiver->getSourceRange();
  SourceLocation SelLoc;
  if (!SelectorLocs.empty() && SelectorLocs.front().isValid())
    SelLoc = SelectorLocs.front();
  else
    SelLoc = Loc;

  if (LBracLoc.isInvalid()) {
    Diag(Loc, diag::err_missing_open_square_message_send)
      << FixItHint::CreateInsertion(Loc, "[");
    LBracLoc = Loc;
  }

  // If we have a receiver expression, perform appropriate promotions
  // and determine receiver type.
  if (Receiver) {
    if (Receiver->hasPlaceholderType()) {
      ExprResult Result;
      if (Receiver->getType() == Context.UnknownAnyTy)
        Result = forceUnknownAnyToType(Receiver, Context.getObjCIdType());
      else
        Result = CheckPlaceholderExpr(Receiver);
      if (Result.isInvalid()) return ExprError();
      Receiver = Result.get();
    }

    if (Receiver->isTypeDependent()) {
      // If the receiver is type-dependent, we can't type-check anything
      // at this point. Build a dependent expression.
      unsigned NumArgs = ArgsIn.size();
      Expr **Args = ArgsIn.data();
      assert(SuperLoc.isInvalid() && "Message to super with dependent type");
      return ObjCMessageExpr::Create(
          Context, Context.DependentTy, VK_RValue, LBracLoc, Receiver, Sel,
          SelectorLocs, /*Method=*/nullptr, makeArrayRef(Args, NumArgs),
          RBracLoc, isImplicit);
    }

    // If necessary, apply function/array conversion to the receiver.
    // C99 6.7.5.3p[7,8].
    ExprResult Result = DefaultFunctionArrayLvalueConversion(Receiver);
    if (Result.isInvalid())
      return ExprError();
    Receiver = Result.get();
    ReceiverType = Receiver->getType();

    // If the receiver is an ObjC pointer, a block pointer, or an
    // __attribute__((NSObject)) pointer, we don't need to do any
    // special conversion in order to look up a receiver.
    if (ReceiverType->isObjCRetainableType()) {
      // do nothing
    } else if (!getLangOpts().ObjCAutoRefCount &&
               !Context.getObjCIdType().isNull() &&
               (ReceiverType->isPointerType() || 
                ReceiverType->isIntegerType())) {
      // Implicitly convert integers and pointers to 'id' but emit a warning.
      // But not in ARC.
      Diag(Loc, diag::warn_bad_receiver_type)
        << ReceiverType 
        << Receiver->getSourceRange();
      if (ReceiverType->isPointerType()) {
        Receiver = ImpCastExprToType(Receiver, Context.getObjCIdType(), 
                                     CK_CPointerToObjCPointerCast).get();
      } else {
        // TODO: specialized warning on null receivers?
        bool IsNull = Receiver->isNullPointerConstant(Context,
                                              Expr::NPC_ValueDependentIsNull);
        CastKind Kind = IsNull ? CK_NullToPointer : CK_IntegralToPointer;
        Receiver = ImpCastExprToType(Receiver, Context.getObjCIdType(),
                                     Kind).get();
      }
      ReceiverType = Receiver->getType();
    } else if (getLangOpts().CPlusPlus) {
      // The receiver must be a complete type.
      if (RequireCompleteType(Loc, Receiver->getType(),
                              diag::err_incomplete_receiver_type))
        return ExprError();

      ExprResult result = PerformContextuallyConvertToObjCPointer(Receiver);
      if (result.isUsable()) {
        Receiver = result.get();
        ReceiverType = Receiver->getType();
      }
    }
  }

  // There's a somewhat weird interaction here where we assume that we
  // won't actually have a method unless we also don't need to do some
  // of the more detailed type-checking on the receiver.

  if (!Method) {
    // Handle messages to id and __kindof types (where we use the
    // global method pool).
    // FIXME: The type bound is currently ignored by lookup in the
    // global pool.
    const ObjCObjectType *typeBound = nullptr;
    bool receiverIsIdLike = ReceiverType->isObjCIdOrObjectKindOfType(Context,
                                                                     typeBound);
    if (receiverIsIdLike || ReceiverType->isBlockPointerType() ||
        (Receiver && Context.isObjCNSObjectType(Receiver->getType()))) {
      Method = LookupInstanceMethodInGlobalPool(Sel, 
                                                SourceRange(LBracLoc, RBracLoc),
                                                receiverIsIdLike);
      if (!Method)
        Method = LookupFactoryMethodInGlobalPool(Sel, 
                                                 SourceRange(LBracLoc,RBracLoc),
                                                 receiverIsIdLike);
      if (Method) {
        if (ObjCMethodDecl *BestMethod =
              SelectBestMethod(Sel, ArgsIn, Method->isInstanceMethod()))
          Method = BestMethod;
        if (!AreMultipleMethodsInGlobalPool(Sel, Method,
                                            SourceRange(LBracLoc, RBracLoc),
                                            receiverIsIdLike)) {
          DiagnoseUseOfDecl(Method, SelLoc);
        }
      }
    } else if (ReceiverType->isObjCClassOrClassKindOfType() ||
               ReceiverType->isObjCQualifiedClassType()) {
      // Handle messages to Class.
      // We allow sending a message to a qualified Class ("Class<foo>"), which
      // is ok as long as one of the protocols implements the selector (if not,
      // warn).
      if (!ReceiverType->isObjCClassOrClassKindOfType()) {
        const ObjCObjectPointerType *QClassTy
          = ReceiverType->getAsObjCQualifiedClassType();
        // Search protocols for class methods.
        Method = LookupMethodInQualifiedType(Sel, QClassTy, false);
        if (!Method) {
          Method = LookupMethodInQualifiedType(Sel, QClassTy, true);
          // warn if instance method found for a Class message.
          if (Method) {
            Diag(SelLoc, diag::warn_instance_method_on_class_found)
              << Method->getSelector() << Sel;
            Diag(Method->getLocation(), diag::note_method_declared_at)
              << Method->getDeclName();
          }
        }
      } else {
        if (ObjCMethodDecl *CurMeth = getCurMethodDecl()) {
          if (ObjCInterfaceDecl *ClassDecl = CurMeth->getClassInterface()) {
            // First check the public methods in the class interface.
            Method = ClassDecl->lookupClassMethod(Sel);

            if (!Method)
              Method = ClassDecl->lookupPrivateClassMethod(Sel);
          }
          if (Method && DiagnoseUseOfDecl(Method, SelLoc))
            return ExprError();
        }
        if (!Method) {
          // If not messaging 'self', look for any factory method named 'Sel'.
          if (!Receiver || !isSelfExpr(Receiver)) {
            Method = LookupFactoryMethodInGlobalPool(Sel, 
                                                SourceRange(LBracLoc, RBracLoc));
            if (!Method) {
              // If no class (factory) method was found, check if an _instance_
              // method of the same name exists in the root class only.
              Method = LookupInstanceMethodInGlobalPool(Sel,
                                               SourceRange(LBracLoc, RBracLoc));
              if (Method)
                  if (const ObjCInterfaceDecl *ID =
                      dyn_cast<ObjCInterfaceDecl>(Method->getDeclContext())) {
                    if (ID->getSuperClass())
                      Diag(SelLoc, diag::warn_root_inst_method_not_found)
                      << Sel << SourceRange(LBracLoc, RBracLoc);
                  }
            }
            if (Method)
              if (ObjCMethodDecl *BestMethod =
                  SelectBestMethod(Sel, ArgsIn, Method->isInstanceMethod()))
                Method = BestMethod;
          }
        }
      }
    } else {
      ObjCInterfaceDecl *ClassDecl = nullptr;

      // We allow sending a message to a qualified ID ("id<foo>"), which is ok as
      // long as one of the protocols implements the selector (if not, warn).
      // And as long as message is not deprecated/unavailable (warn if it is).
      if (const ObjCObjectPointerType *QIdTy 
                                   = ReceiverType->getAsObjCQualifiedIdType()) {
        // Search protocols for instance methods.
        Method = LookupMethodInQualifiedType(Sel, QIdTy, true);
        if (!Method)
          Method = LookupMethodInQualifiedType(Sel, QIdTy, false);
        if (Method && DiagnoseUseOfDecl(Method, SelLoc))
          return ExprError();
      } else if (const ObjCObjectPointerType *OCIType
                   = ReceiverType->getAsObjCInterfacePointerType()) {
        // We allow sending a message to a pointer to an interface (an object).
        ClassDecl = OCIType->getInterfaceDecl();

        // Try to complete the type. Under ARC, this is a hard error from which
        // we don't try to recover.
        const ObjCInterfaceDecl *forwardClass = nullptr;
        if (RequireCompleteType(Loc, OCIType->getPointeeType(),
              getLangOpts().ObjCAutoRefCount
                ? diag::err_arc_receiver_forward_instance
                : diag::warn_receiver_forward_instance,
                                Receiver? Receiver->getSourceRange()
                                        : SourceRange(SuperLoc))) {
          if (getLangOpts().ObjCAutoRefCount)
            return ExprError();
          
          forwardClass = OCIType->getInterfaceDecl();
          Diag(Receiver ? Receiver->getLocStart() 
                        : SuperLoc, diag::note_receiver_is_id);
          Method = nullptr;
        } else {
          Method = ClassDecl->lookupInstanceMethod(Sel);
        }

        if (!Method)
          // Search protocol qualifiers.
          Method = LookupMethodInQualifiedType(Sel, OCIType, true);
        
        if (!Method) {
          // If we have implementations in scope, check "private" methods.
          Method = ClassDecl->lookupPrivateMethod(Sel);

          if (!Method && getLangOpts().ObjCAutoRefCount) {
            Diag(SelLoc, diag::err_arc_may_not_respond)
              << OCIType->getPointeeType() << Sel << RecRange
              << SourceRange(SelectorLocs.front(), SelectorLocs.back());
            return ExprError();
          }

          if (!Method && (!Receiver || !isSelfExpr(Receiver))) {
            // If we still haven't found a method, look in the global pool. This
            // behavior isn't very desirable, however we need it for GCC
            // compatibility. FIXME: should we deviate??
            if (OCIType->qual_empty()) {
              Method = LookupInstanceMethodInGlobalPool(Sel,
                                              SourceRange(LBracLoc, RBracLoc));
              if (Method) {
                if (auto BestMethod =
                      SelectBestMethod(Sel, ArgsIn, Method->isInstanceMethod()))
                  Method = BestMethod;
                AreMultipleMethodsInGlobalPool(Sel, Method,
                                               SourceRange(LBracLoc, RBracLoc),
                                               true);
              }
              if (Method && !forwardClass)
                Diag(SelLoc, diag::warn_maynot_respond)
                  << OCIType->getInterfaceDecl()->getIdentifier()
                  << Sel << RecRange;
            }
          }
        }
        if (Method && DiagnoseUseOfDecl(Method, SelLoc, forwardClass))
          return ExprError();
      } else {
        // Reject other random receiver types (e.g. structs).
        Diag(Loc, diag::err_bad_receiver_type)
          << ReceiverType << Receiver->getSourceRange();
        return ExprError();
      }
    }
  }

  FunctionScopeInfo *DIFunctionScopeInfo =
    (Method && Method->getMethodFamily() == OMF_init)
      ? getEnclosingFunction() : nullptr;

  if (DIFunctionScopeInfo &&
      DIFunctionScopeInfo->ObjCIsDesignatedInit &&
      (SuperLoc.isValid() || isSelfExpr(Receiver))) {
    bool isDesignatedInitChain = false;
    if (SuperLoc.isValid()) {
      if (const ObjCObjectPointerType *
            OCIType = ReceiverType->getAsObjCInterfacePointerType()) {
        if (const ObjCInterfaceDecl *ID = OCIType->getInterfaceDecl()) {
          // Either we know this is a designated initializer or we
          // conservatively assume it because we don't know for sure.
          if (!ID->declaresOrInheritsDesignatedInitializers() ||
              ID->isDesignatedInitializer(Sel)) {
            isDesignatedInitChain = true;
            DIFunctionScopeInfo->ObjCWarnForNoDesignatedInitChain = false;
          }
        }
      }
    }
    if (!isDesignatedInitChain) {
      const ObjCMethodDecl *InitMethod = nullptr;
      bool isDesignated =
        getCurMethodDecl()->isDesignatedInitializerForTheInterface(&InitMethod);
      assert(isDesignated && InitMethod);
      (void)isDesignated;
      Diag(SelLoc, SuperLoc.isValid() ?
             diag::warn_objc_designated_init_non_designated_init_call :
             diag::warn_objc_designated_init_non_super_designated_init_call);
      Diag(InitMethod->getLocation(),
           diag::note_objc_designated_init_marked_here);
    }
  }

  if (DIFunctionScopeInfo &&
      DIFunctionScopeInfo->ObjCIsSecondaryInit &&
      (SuperLoc.isValid() || isSelfExpr(Receiver))) {
    if (SuperLoc.isValid()) {
      Diag(SelLoc, diag::warn_objc_secondary_init_super_init_call);
    } else {
      DIFunctionScopeInfo->ObjCWarnForNoInitDelegation = false;
    }
  }

  // Check the message arguments.
  unsigned NumArgs = ArgsIn.size();
  Expr **Args = ArgsIn.data();
  QualType ReturnType;
  ExprValueKind VK = VK_RValue;
  bool ClassMessage = (ReceiverType->isObjCClassType() ||
                       ReceiverType->isObjCQualifiedClassType());
  if (CheckMessageArgumentTypes(ReceiverType, MultiExprArg(Args, NumArgs),
                                Sel, SelectorLocs, Method,
                                ClassMessage, SuperLoc.isValid(), 
                                LBracLoc, RBracLoc, RecRange, ReturnType, VK))
    return ExprError();

  if (Method && !Method->getReturnType()->isVoidType() &&
      RequireCompleteType(LBracLoc, Method->getReturnType(),
                          diag::err_illegal_message_expr_incomplete_type))
    return ExprError();

  // In ARC, forbid the user from sending messages to 
  // retain/release/autorelease/dealloc/retainCount explicitly.
  if (getLangOpts().ObjCAutoRefCount) {
    ObjCMethodFamily family =
      (Method ? Method->getMethodFamily() : Sel.getMethodFamily());
    switch (family) {
    case OMF_init:
      if (Method)
        checkInitMethod(Method, ReceiverType);

    case OMF_None:
    case OMF_alloc:
    case OMF_copy:
    case OMF_finalize:
    case OMF_mutableCopy:
    case OMF_new:
    case OMF_self:
    case OMF_initialize:
      break;

    case OMF_dealloc:
    case OMF_retain:
    case OMF_release:
    case OMF_autorelease:
    case OMF_retainCount:
      Diag(SelLoc, diag::err_arc_illegal_explicit_message)
        << Sel << RecRange;
      break;
    
    case OMF_performSelector:
      if (Method && NumArgs >= 1) {
        if (ObjCSelectorExpr *SelExp = dyn_cast<ObjCSelectorExpr>(Args[0])) {
          Selector ArgSel = SelExp->getSelector();
          ObjCMethodDecl *SelMethod = 
            LookupInstanceMethodInGlobalPool(ArgSel,
                                             SelExp->getSourceRange());
          if (!SelMethod)
            SelMethod =
              LookupFactoryMethodInGlobalPool(ArgSel,
                                              SelExp->getSourceRange());
          if (SelMethod) {
            ObjCMethodFamily SelFamily = SelMethod->getMethodFamily();
            switch (SelFamily) {
              case OMF_alloc:
              case OMF_copy:
              case OMF_mutableCopy:
              case OMF_new:
              case OMF_self:
              case OMF_init:
                // Issue error, unless ns_returns_not_retained.
                if (!SelMethod->hasAttr<NSReturnsNotRetainedAttr>()) {
                  // selector names a +1 method 
                  Diag(SelLoc, 
                       diag::err_arc_perform_selector_retains);
                  Diag(SelMethod->getLocation(), diag::note_method_declared_at)
                    << SelMethod->getDeclName();
                }
                break;
              default:
                // +0 call. OK. unless ns_returns_retained.
                if (SelMethod->hasAttr<NSReturnsRetainedAttr>()) {
                  // selector names a +1 method
                  Diag(SelLoc, 
                       diag::err_arc_perform_selector_retains);
                  Diag(SelMethod->getLocation(), diag::note_method_declared_at)
                    << SelMethod->getDeclName();
                }
                break;
            }
          }
        } else {
          // error (may leak).
          Diag(SelLoc, diag::warn_arc_perform_selector_leaks);
          Diag(Args[0]->getExprLoc(), diag::note_used_here);
        }
      }
      break;
    }
  }

  DiagnoseCStringFormatDirectiveInObjCAPI(*this, Method, Sel, Args, NumArgs);
  
  // Construct the appropriate ObjCMessageExpr instance.
  ObjCMessageExpr *Result;
  if (SuperLoc.isValid())
    Result = ObjCMessageExpr::Create(Context, ReturnType, VK, LBracLoc,
                                     SuperLoc,  /*IsInstanceSuper=*/true,
                                     ReceiverType, Sel, SelectorLocs, Method, 
                                     makeArrayRef(Args, NumArgs), RBracLoc,
                                     isImplicit);
  else {
    Result = ObjCMessageExpr::Create(Context, ReturnType, VK, LBracLoc,
                                     Receiver, Sel, SelectorLocs, Method,
                                     makeArrayRef(Args, NumArgs), RBracLoc,
                                     isImplicit);
    if (!isImplicit)
      checkCocoaAPI(*this, Result);
  }

  if (getLangOpts().ObjCAutoRefCount) {
    // In ARC, annotate delegate init calls.
    if (Result->getMethodFamily() == OMF_init &&
        (SuperLoc.isValid() || isSelfExpr(Receiver))) {
      // Only consider init calls *directly* in init implementations,
      // not within blocks.
      ObjCMethodDecl *method = dyn_cast<ObjCMethodDecl>(CurContext);
      if (method && method->getMethodFamily() == OMF_init) {
        // The implicit assignment to self means we also don't want to
        // consume the result.
        Result->setDelegateInitCall(true);
        return Result;
      }
    }

    // In ARC, check for message sends which are likely to introduce
    // retain cycles.
    checkRetainCycles(Result);

    if (!isImplicit && Method) {
      if (const ObjCPropertyDecl *Prop = Method->findPropertyDecl()) {
        bool IsWeak =
          Prop->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_weak;
        if (!IsWeak && Sel.isUnarySelector())
          IsWeak = ReturnType.getObjCLifetime() & Qualifiers::OCL_Weak;
        if (IsWeak &&
            !Diags.isIgnored(diag::warn_arc_repeated_use_of_weak, LBracLoc))
          getCurFunction()->recordUseOfWeak(Result, Prop);
      }
    }
  }

  CheckObjCCircularContainer(Result);

  return MaybeBindToTemporary(Result);
}

static void RemoveSelectorFromWarningCache(Sema &S, Expr* Arg) {
  if (ObjCSelectorExpr *OSE =
      dyn_cast<ObjCSelectorExpr>(Arg->IgnoreParenCasts())) {
    Selector Sel = OSE->getSelector();
    SourceLocation Loc = OSE->getAtLoc();
    auto Pos = S.ReferencedSelectors.find(Sel);
    if (Pos != S.ReferencedSelectors.end() && Pos->second == Loc)
      S.ReferencedSelectors.erase(Pos);
  }
}

// ActOnInstanceMessage - used for both unary and keyword messages.
// ArgExprs is optional - if it is present, the number of expressions
// is obtained from Sel.getNumArgs().
ExprResult Sema::ActOnInstanceMessage(Scope *S,
                                      Expr *Receiver, 
                                      Selector Sel,
                                      SourceLocation LBracLoc,
                                      ArrayRef<SourceLocation> SelectorLocs,
                                      SourceLocation RBracLoc,
                                      MultiExprArg Args) {
  if (!Receiver)
    return ExprError();

  // A ParenListExpr can show up while doing error recovery with invalid code.
  if (isa<ParenListExpr>(Receiver)) {
    ExprResult Result = MaybeConvertParenListExprToParenExpr(S, Receiver);
    if (Result.isInvalid()) return ExprError();
    Receiver = Result.get();
  }
  
  if (RespondsToSelectorSel.isNull()) {
    IdentifierInfo *SelectorId = &Context.Idents.get("respondsToSelector");
    RespondsToSelectorSel = Context.Selectors.getUnarySelector(SelectorId);
  }
  if (Sel == RespondsToSelectorSel)
    RemoveSelectorFromWarningCache(*this, Args[0]);

  return BuildInstanceMessage(Receiver, Receiver->getType(),
                              /*SuperLoc=*/SourceLocation(), Sel,
                              /*Method=*/nullptr, LBracLoc, SelectorLocs,
                              RBracLoc, Args);
}

enum ARCConversionTypeClass {
  /// int, void, struct A
  ACTC_none,

  /// id, void (^)()
  ACTC_retainable,

  /// id*, id***, void (^*)(),
  ACTC_indirectRetainable,

  /// void* might be a normal C type, or it might a CF type.
  ACTC_voidPtr,

  /// struct A*
  ACTC_coreFoundation
};
static bool isAnyRetainable(ARCConversionTypeClass ACTC) {
  return (ACTC == ACTC_retainable ||
          ACTC == ACTC_coreFoundation ||
          ACTC == ACTC_voidPtr);
}
static bool isAnyCLike(ARCConversionTypeClass ACTC) {
  return ACTC == ACTC_none ||
         ACTC == ACTC_voidPtr ||
         ACTC == ACTC_coreFoundation;
}

static ARCConversionTypeClass classifyTypeForARCConversion(QualType type) {
  bool isIndirect = false;
  
  // Ignore an outermost reference type.
  if (const ReferenceType *ref = type->getAs<ReferenceType>()) {
    type = ref->getPointeeType();
    isIndirect = true;
  }
  
  // Drill through pointers and arrays recursively.
  while (true) {
    if (const PointerType *ptr = type->getAs<PointerType>()) {
      type = ptr->getPointeeType();

      // The first level of pointer may be the innermost pointer on a CF type.
      if (!isIndirect) {
        if (type->isVoidType()) return ACTC_voidPtr;
        if (type->isRecordType()) return ACTC_coreFoundation;
      }
    } else if (const ArrayType *array = type->getAsArrayTypeUnsafe()) {
      type = QualType(array->getElementType()->getBaseElementTypeUnsafe(), 0);
    } else {
      break;
    }
    isIndirect = true;
  }
  
  if (isIndirect) {
    if (type->isObjCARCBridgableType())
      return ACTC_indirectRetainable;
    return ACTC_none;
  }

  if (type->isObjCARCBridgableType())
    return ACTC_retainable;

  return ACTC_none;
}

namespace {
  /// A result from the cast checker.
  enum ACCResult {
    /// Cannot be casted.
    ACC_invalid,

    /// Can be safely retained or not retained.
    ACC_bottom,

    /// Can be casted at +0.
    ACC_plusZero,

    /// Can be casted at +1.
    ACC_plusOne
  };
  ACCResult merge(ACCResult left, ACCResult right) {
    if (left == right) return left;
    if (left == ACC_bottom) return right;
    if (right == ACC_bottom) return left;
    return ACC_invalid;
  }

  /// A checker which white-lists certain expressions whose conversion
  /// to or from retainable type would otherwise be forbidden in ARC.
  class ARCCastChecker : public StmtVisitor<ARCCastChecker, ACCResult> {
    typedef StmtVisitor<ARCCastChecker, ACCResult> super;

    ASTContext &Context;
    ARCConversionTypeClass SourceClass;
    ARCConversionTypeClass TargetClass;
    bool Diagnose;

    static bool isCFType(QualType type) {
      // Someday this can use ns_bridged.  For now, it has to do this.
      return type->isCARCBridgableType();
    }

  public:
    ARCCastChecker(ASTContext &Context, ARCConversionTypeClass source,
                   ARCConversionTypeClass target, bool diagnose)
      : Context(Context), SourceClass(source), TargetClass(target),
        Diagnose(diagnose) {}

    using super::Visit;
    ACCResult Visit(Expr *e) {
      return super::Visit(e->IgnoreParens());
    }

    ACCResult VisitStmt(Stmt *s) {
      return ACC_invalid;
    }

    /// Null pointer constants can be casted however you please.
    ACCResult VisitExpr(Expr *e) {
      if (e->isNullPointerConstant(Context, Expr::NPC_ValueDependentIsNotNull))
        return ACC_bottom;
      return ACC_invalid;
    }

    /// Objective-C string literals can be safely casted.
    ACCResult VisitObjCStringLiteral(ObjCStringLiteral *e) {
      // If we're casting to any retainable type, go ahead.  Global
      // strings are immune to retains, so this is bottom.
      if (isAnyRetainable(TargetClass)) return ACC_bottom;

      return ACC_invalid;
    }
    
    /// Look through certain implicit and explicit casts.
    ACCResult VisitCastExpr(CastExpr *e) {
      switch (e->getCastKind()) {
        case CK_NullToPointer:
          return ACC_bottom;

        case CK_NoOp:
        case CK_LValueToRValue:
        case CK_BitCast:
        case CK_CPointerToObjCPointerCast:
        case CK_BlockPointerToObjCPointerCast:
        case CK_AnyPointerToBlockPointerCast:
          return Visit(e->getSubExpr());

        default:
          return ACC_invalid;
      }
    }

    /// Look through unary extension.
    ACCResult VisitUnaryExtension(UnaryOperator *e) {
      return Visit(e->getSubExpr());
    }

    /// Ignore the LHS of a comma operator.
    ACCResult VisitBinComma(BinaryOperator *e) {
      return Visit(e->getRHS());
    }

    /// Conditional operators are okay if both sides are okay.
    ACCResult VisitConditionalOperator(ConditionalOperator *e) {
      ACCResult left = Visit(e->getTrueExpr());
      if (left == ACC_invalid) return ACC_invalid;
      return merge(left, Visit(e->getFalseExpr()));
    }

    /// Look through pseudo-objects.
    ACCResult VisitPseudoObjectExpr(PseudoObjectExpr *e) {
      // If we're getting here, we should always have a result.
      return Visit(e->getResultExpr());
    }

    /// Statement expressions are okay if their result expression is okay.
    ACCResult VisitStmtExpr(StmtExpr *e) {
      return Visit(e->getSubStmt()->body_back());
    }

    /// Some declaration references are okay.
    ACCResult VisitDeclRefExpr(DeclRefExpr *e) {
      VarDecl *var = dyn_cast<VarDecl>(e->getDecl());
      // References to global constants are okay.
      if (isAnyRetainable(TargetClass) &&
          isAnyRetainable(SourceClass) &&
          var &&
          var->getStorageClass() == SC_Extern &&
          var->getType().isConstQualified()) {

        // In system headers, they can also be assumed to be immune to retains.
        // These are things like 'kCFStringTransformToLatin'.
        if (Context.getSourceManager().isInSystemHeader(var->getLocation()))
          return ACC_bottom;

        return ACC_plusZero;
      }

      // Nothing else.
      return ACC_invalid;
    }

    /// Some calls are okay.
    ACCResult VisitCallExpr(CallExpr *e) {
      if (FunctionDecl *fn = e->getDirectCallee())
        if (ACCResult result = checkCallToFunction(fn))
          return result;

      return super::VisitCallExpr(e);
    }

    ACCResult checkCallToFunction(FunctionDecl *fn) {
      // Require a CF*Ref return type.
      if (!isCFType(fn->getReturnType()))
        return ACC_invalid;

      if (!isAnyRetainable(TargetClass))
        return ACC_invalid;

      // Honor an explicit 'not retained' attribute.
      if (fn->hasAttr<CFReturnsNotRetainedAttr>())
        return ACC_plusZero;

      // Honor an explicit 'retained' attribute, except that for
      // now we're not going to permit implicit handling of +1 results,
      // because it's a bit frightening.
      if (fn->hasAttr<CFReturnsRetainedAttr>())
        return Diagnose ? ACC_plusOne
                        : ACC_invalid; // ACC_plusOne if we start accepting this

      // Recognize this specific builtin function, which is used by CFSTR.
      unsigned builtinID = fn->getBuiltinID();
      if (builtinID == Builtin::BI__builtin___CFStringMakeConstantString)
        return ACC_bottom;

      // Otherwise, don't do anything implicit with an unaudited function.
      if (!fn->hasAttr<CFAuditedTransferAttr>())
        return ACC_invalid;
      
      // Otherwise, it's +0 unless it follows the create convention.
      if (ento::coreFoundation::followsCreateRule(fn))
        return Diagnose ? ACC_plusOne 
                        : ACC_invalid; // ACC_plusOne if we start accepting this

      return ACC_plusZero;
    }

    ACCResult VisitObjCMessageExpr(ObjCMessageExpr *e) {
      return checkCallToMethod(e->getMethodDecl());
    }

    ACCResult VisitObjCPropertyRefExpr(ObjCPropertyRefExpr *e) {
      ObjCMethodDecl *method;
      if (e->isExplicitProperty())
        method = e->getExplicitProperty()->getGetterMethodDecl();
      else
        method = e->getImplicitPropertyGetter();
      return checkCallToMethod(method);
    }

    ACCResult checkCallToMethod(ObjCMethodDecl *method) {
      if (!method) return ACC_invalid;

      // Check for message sends to functions returning CF types.  We
      // just obey the Cocoa conventions with these, even though the
      // return type is CF.
      if (!isAnyRetainable(TargetClass) || !isCFType(method->getReturnType()))
        return ACC_invalid;
      
      // If the method is explicitly marked not-retained, it's +0.
      if (method->hasAttr<CFReturnsNotRetainedAttr>())
        return ACC_plusZero;

      // If the method is explicitly marked as returning retained, or its
      // selector follows a +1 Cocoa convention, treat it as +1.
      if (method->hasAttr<CFReturnsRetainedAttr>())
        return ACC_plusOne;

      switch (method->getSelector().getMethodFamily()) {
      case OMF_alloc:
      case OMF_copy:
      case OMF_mutableCopy:
      case OMF_new:
        return ACC_plusOne;

      default:
        // Otherwise, treat it as +0.
        return ACC_plusZero;
      }
    }
  };
}

bool Sema::isKnownName(StringRef name) {
  if (name.empty())
    return false;
  LookupResult R(*this, &Context.Idents.get(name), SourceLocation(),
                 Sema::LookupOrdinaryName);
  return LookupName(R, TUScope, false);
}

#if 0 // HLSL Change Start - No ObjC support
static void addFixitForObjCARCConversion(Sema &S,
                                         DiagnosticBuilder &DiagB,
                                         Sema::CheckedConversionKind CCK,
                                         SourceLocation afterLParen,
                                         QualType castType,
                                         Expr *castExpr,
                                         Expr *realCast,
                                         const char *bridgeKeyword,
                                         const char *CFBridgeName) {
  // We handle C-style and implicit casts here.
  switch (CCK) {
  case Sema::CCK_ImplicitConversion:
  case Sema::CCK_CStyleCast:
  case Sema::CCK_OtherCast:
    break;
  case Sema::CCK_FunctionalCast:
    return;
  }

  if (CFBridgeName) {
    if (CCK == Sema::CCK_OtherCast) {
      if (const CXXNamedCastExpr *NCE = dyn_cast<CXXNamedCastExpr>(realCast)) {
        SourceRange range(NCE->getOperatorLoc(),
                          NCE->getAngleBrackets().getEnd());
        SmallString<32> BridgeCall;
        
        SourceManager &SM = S.getSourceManager();
        char PrevChar = *SM.getCharacterData(range.getBegin().getLocWithOffset(-1));
        if (Lexer::isIdentifierBodyChar(PrevChar, S.getLangOpts()))
          BridgeCall += ' ';
        
        BridgeCall += CFBridgeName;
        DiagB.AddFixItHint(FixItHint::CreateReplacement(range, BridgeCall));
      }
      return;
    }
    Expr *castedE = castExpr;
    if (CStyleCastExpr *CCE = dyn_cast<CStyleCastExpr>(castedE))
      castedE = CCE->getSubExpr();
    castedE = castedE->IgnoreImpCasts();
    SourceRange range = castedE->getSourceRange();

    SmallString<32> BridgeCall;

    SourceManager &SM = S.getSourceManager();
    char PrevChar = *SM.getCharacterData(range.getBegin().getLocWithOffset(-1));
    if (Lexer::isIdentifierBodyChar(PrevChar, S.getLangOpts()))
      BridgeCall += ' ';

    BridgeCall += CFBridgeName;

    if (isa<ParenExpr>(castedE)) {
      DiagB.AddFixItHint(FixItHint::CreateInsertion(range.getBegin(),
                         BridgeCall));
    } else {
      BridgeCall += '(';
      DiagB.AddFixItHint(FixItHint::CreateInsertion(range.getBegin(),
                                                    BridgeCall));
      DiagB.AddFixItHint(FixItHint::CreateInsertion(
                                       S.PP.getLocForEndOfToken(range.getEnd()),
                                       ")"));
    }
    return;
  }

  if (CCK == Sema::CCK_CStyleCast) {
    DiagB.AddFixItHint(FixItHint::CreateInsertion(afterLParen, bridgeKeyword));
  } else if (CCK == Sema::CCK_OtherCast) {
    if (const CXXNamedCastExpr *NCE = dyn_cast<CXXNamedCastExpr>(realCast)) {
      std::string castCode = "(";
      castCode += bridgeKeyword;
      castCode += castType.getAsString();
      castCode += ")";
      SourceRange Range(NCE->getOperatorLoc(),
                        NCE->getAngleBrackets().getEnd());
      DiagB.AddFixItHint(FixItHint::CreateReplacement(Range, castCode));
    }
  } else {
    std::string castCode = "(";
    castCode += bridgeKeyword;
    castCode += castType.getAsString();
    castCode += ")";
    Expr *castedE = castExpr->IgnoreImpCasts();
    SourceRange range = castedE->getSourceRange();
    if (isa<ParenExpr>(castedE)) {
      DiagB.AddFixItHint(FixItHint::CreateInsertion(range.getBegin(),
                         castCode));
    } else {
      castCode += "(";
      DiagB.AddFixItHint(FixItHint::CreateInsertion(range.getBegin(),
                                                    castCode));
      DiagB.AddFixItHint(FixItHint::CreateInsertion(
                                       S.PP.getLocForEndOfToken(range.getEnd()),
                                       ")"));
    }
  }
}
#endif // HLSL change End - No ObjC support

template <typename T>
static inline T *getObjCBridgeAttr(const TypedefType *TD) {
  TypedefNameDecl *TDNDecl = TD->getDecl();
  QualType QT = TDNDecl->getUnderlyingType();
  if (QT->isPointerType()) {
    QT = QT->getPointeeType();
    if (const RecordType *RT = QT->getAs<RecordType>())
      if (RecordDecl *RD = RT->getDecl()->getMostRecentDecl())
        return RD->getAttr<T>();
  }
  return nullptr;
}

static ObjCBridgeRelatedAttr *ObjCBridgeRelatedAttrFromType(QualType T,
                                                            TypedefNameDecl *&TDNDecl) {
  while (const TypedefType *TD = dyn_cast<TypedefType>(T.getTypePtr())) {
    TDNDecl = TD->getDecl();
    if (ObjCBridgeRelatedAttr *ObjCBAttr =
        getObjCBridgeAttr<ObjCBridgeRelatedAttr>(TD))
      return ObjCBAttr;
    T = TDNDecl->getUnderlyingType();
  }
  return nullptr;
}

static void
diagnoseObjCARCConversion(Sema &S, SourceRange castRange,
                          QualType castType, ARCConversionTypeClass castACTC,
                          Expr *castExpr, Expr *realCast,
                          ARCConversionTypeClass exprACTC,
                          Sema::CheckedConversionKind CCK) {
  SourceLocation loc =
    (castRange.isValid() ? castRange.getBegin() : castExpr->getExprLoc());
  
  if (S.makeUnavailableInSystemHeader(loc,
                "converts between Objective-C and C pointers in -fobjc-arc"))
    return;

  QualType castExprType = castExpr->getType();
  TypedefNameDecl *TDNDecl = nullptr;
  if ((castACTC == ACTC_coreFoundation &&  exprACTC == ACTC_retainable &&
       ObjCBridgeRelatedAttrFromType(castType, TDNDecl)) ||
      (exprACTC == ACTC_coreFoundation && castACTC == ACTC_retainable &&
       ObjCBridgeRelatedAttrFromType(castExprType, TDNDecl)))
    return;
  
  unsigned srcKind = 0;
  switch (exprACTC) {
  case ACTC_none:
  case ACTC_coreFoundation:
  case ACTC_voidPtr:
    srcKind = (castExprType->isPointerType() ? 1 : 0);
    break;
  case ACTC_retainable:
    srcKind = (castExprType->isBlockPointerType() ? 2 : 3);
    break;
  case ACTC_indirectRetainable:
    srcKind = 4;
    break;
  }
  
  // Check whether this could be fixed with a bridge cast.
  SourceLocation afterLParen = S.PP.getLocForEndOfToken(castRange.getBegin());
  SourceLocation noteLoc = afterLParen.isValid() ? afterLParen : loc;

  // Bridge from an ARC type to a CF type.
  if (castACTC == ACTC_retainable && isAnyRetainable(exprACTC)) {

    S.Diag(loc, diag::err_arc_cast_requires_bridge)
      << unsigned(CCK == Sema::CCK_ImplicitConversion) // cast|implicit
      << 2 // of C pointer type
      << castExprType
      << unsigned(castType->isBlockPointerType()) // to ObjC|block type
      << castType
      << castRange
      << castExpr->getSourceRange();
    bool br = S.isKnownName("CFBridgingRelease");
    ACCResult CreateRule = 
      ARCCastChecker(S.Context, exprACTC, castACTC, true).Visit(castExpr);
    assert(CreateRule != ACC_bottom && "This cast should already be accepted.");
    if (CreateRule != ACC_plusOne)
    {
      DiagnosticBuilder DiagB = 
        (CCK != Sema::CCK_OtherCast) ? S.Diag(noteLoc, diag::note_arc_bridge)
                              : S.Diag(noteLoc, diag::note_arc_cstyle_bridge);

      addFixitForObjCARCConversion(S, DiagB, CCK, afterLParen,
                                   castType, castExpr, realCast, "__bridge ",
                                   nullptr);
    }
    if (CreateRule != ACC_plusZero)
    {
      DiagnosticBuilder DiagB =
        (CCK == Sema::CCK_OtherCast && !br) ?
          S.Diag(noteLoc, diag::note_arc_cstyle_bridge_transfer) << castExprType :
          S.Diag(br ? castExpr->getExprLoc() : noteLoc,
                 diag::note_arc_bridge_transfer)
            << castExprType << br;

      addFixitForObjCARCConversion(S, DiagB, CCK, afterLParen,
                                   castType, castExpr, realCast, "__bridge_transfer ",
                                   br ? "CFBridgingRelease" : nullptr);
    }

    return;
  }
  
  // Bridge from a CF type to an ARC type.
  if (exprACTC == ACTC_retainable && isAnyRetainable(castACTC)) {
    bool br = S.isKnownName("CFBridgingRetain");
    S.Diag(loc, diag::err_arc_cast_requires_bridge)
      << unsigned(CCK == Sema::CCK_ImplicitConversion) // cast|implicit
      << unsigned(castExprType->isBlockPointerType()) // of ObjC|block type
      << castExprType
      << 2 // to C pointer type
      << castType
      << castRange
      << castExpr->getSourceRange();
    ACCResult CreateRule = 
      ARCCastChecker(S.Context, exprACTC, castACTC, true).Visit(castExpr);
    assert(CreateRule != ACC_bottom && "This cast should already be accepted.");
    if (CreateRule != ACC_plusOne)
    {
      DiagnosticBuilder DiagB =
      (CCK != Sema::CCK_OtherCast) ? S.Diag(noteLoc, diag::note_arc_bridge)
                               : S.Diag(noteLoc, diag::note_arc_cstyle_bridge);
      addFixitForObjCARCConversion(S, DiagB, CCK, afterLParen,
                                   castType, castExpr, realCast, "__bridge ",
                                   nullptr);
    }
    if (CreateRule != ACC_plusZero)
    {
      DiagnosticBuilder DiagB =
        (CCK == Sema::CCK_OtherCast && !br) ?
          S.Diag(noteLoc, diag::note_arc_cstyle_bridge_retained) << castType :
          S.Diag(br ? castExpr->getExprLoc() : noteLoc,
                 diag::note_arc_bridge_retained)
            << castType << br;

      addFixitForObjCARCConversion(S, DiagB, CCK, afterLParen,
                                   castType, castExpr, realCast, "__bridge_retained ",
                                   br ? "CFBridgingRetain" : nullptr);
    }

    return;
  }
  
  S.Diag(loc, diag::err_arc_mismatched_cast)
    << (CCK != Sema::CCK_ImplicitConversion)
    << srcKind << castExprType << castType
    << castRange << castExpr->getSourceRange();
}

template <typename TB>
static bool CheckObjCBridgeNSCast(Sema &S, QualType castType, Expr *castExpr,
                                  bool &HadTheAttribute, bool warn) {
  QualType T = castExpr->getType();
  HadTheAttribute = false;
  while (const TypedefType *TD = dyn_cast<TypedefType>(T.getTypePtr())) {
    TypedefNameDecl *TDNDecl = TD->getDecl();
    if (TB *ObjCBAttr = getObjCBridgeAttr<TB>(TD)) {
      if (IdentifierInfo *Parm = ObjCBAttr->getBridgedType()) {
        HadTheAttribute = true;
        if (Parm->isStr("id"))
          return true;
        
        NamedDecl *Target = nullptr;
        // Check for an existing type with this name.
        LookupResult R(S, DeclarationName(Parm), SourceLocation(),
                       Sema::LookupOrdinaryName);
        if (S.LookupName(R, S.TUScope)) {
          Target = R.getFoundDecl();
          if (Target && isa<ObjCInterfaceDecl>(Target)) {
            ObjCInterfaceDecl *ExprClass = cast<ObjCInterfaceDecl>(Target);
            if (const ObjCObjectPointerType *InterfacePointerType =
                  castType->getAsObjCInterfacePointerType()) {
              ObjCInterfaceDecl *CastClass
                = InterfacePointerType->getObjectType()->getInterface();
              if ((CastClass == ExprClass) ||
                  (CastClass && CastClass->isSuperClassOf(ExprClass)))
                return true;
              if (warn)
                S.Diag(castExpr->getLocStart(), diag::warn_objc_invalid_bridge)
                  << T << Target->getName() << castType->getPointeeType();
              return false;
            } else if (castType->isObjCIdType() ||
                       (S.Context.ObjCObjectAdoptsQTypeProtocols(
                          castType, ExprClass)))
              // ok to cast to 'id'.
              // casting to id<p-list> is ok if bridge type adopts all of
              // p-list protocols.
              return true;
            else {
              if (warn) {
                S.Diag(castExpr->getLocStart(), diag::warn_objc_invalid_bridge)
                  << T << Target->getName() << castType;
                S.Diag(TDNDecl->getLocStart(), diag::note_declared_at);
                S.Diag(Target->getLocStart(), diag::note_declared_at);
              }
              return false;
           }
          }
        } else if (!castType->isObjCIdType()) {
          S.Diag(castExpr->getLocStart(), diag::err_objc_cf_bridged_not_interface)
            << castExpr->getType() << Parm;
          S.Diag(TDNDecl->getLocStart(), diag::note_declared_at);
          if (Target)
            S.Diag(Target->getLocStart(), diag::note_declared_at);
        }
        return true;
      }
      return false;
    }
    T = TDNDecl->getUnderlyingType();
  }
  return true;
}

template <typename TB>
static bool CheckObjCBridgeCFCast(Sema &S, QualType castType, Expr *castExpr,
                                  bool &HadTheAttribute, bool warn) {
  QualType T = castType;
  HadTheAttribute = false;
  while (const TypedefType *TD = dyn_cast<TypedefType>(T.getTypePtr())) {
    TypedefNameDecl *TDNDecl = TD->getDecl();
    if (TB *ObjCBAttr = getObjCBridgeAttr<TB>(TD)) {
      if (IdentifierInfo *Parm = ObjCBAttr->getBridgedType()) {
        HadTheAttribute = true;
        if (Parm->isStr("id"))
          return true;

        NamedDecl *Target = nullptr;
        // Check for an existing type with this name.
        LookupResult R(S, DeclarationName(Parm), SourceLocation(),
                       Sema::LookupOrdinaryName);
        if (S.LookupName(R, S.TUScope)) {
          Target = R.getFoundDecl();
          if (Target && isa<ObjCInterfaceDecl>(Target)) {
            ObjCInterfaceDecl *CastClass = cast<ObjCInterfaceDecl>(Target);
            if (const ObjCObjectPointerType *InterfacePointerType =
                  castExpr->getType()->getAsObjCInterfacePointerType()) {
              ObjCInterfaceDecl *ExprClass
                = InterfacePointerType->getObjectType()->getInterface();
              if ((CastClass == ExprClass) ||
                  (ExprClass && CastClass->isSuperClassOf(ExprClass)))
                return true;
              if (warn) {
                S.Diag(castExpr->getLocStart(), diag::warn_objc_invalid_bridge_to_cf)
                  << castExpr->getType()->getPointeeType() << T;
                S.Diag(TDNDecl->getLocStart(), diag::note_declared_at);
              }
              return false;
            } else if (castExpr->getType()->isObjCIdType() ||
                       (S.Context.QIdProtocolsAdoptObjCObjectProtocols(
                          castExpr->getType(), CastClass)))
              // ok to cast an 'id' expression to a CFtype.
              // ok to cast an 'id<plist>' expression to CFtype provided plist
              // adopts all of CFtype's ObjetiveC's class plist.
              return true;
            else {
              if (warn) {
                S.Diag(castExpr->getLocStart(), diag::warn_objc_invalid_bridge_to_cf)
                  << castExpr->getType() << castType;
                S.Diag(TDNDecl->getLocStart(), diag::note_declared_at);
                S.Diag(Target->getLocStart(), diag::note_declared_at);
              }
              return false;
            }
          }
        }
        S.Diag(castExpr->getLocStart(), diag::err_objc_ns_bridged_invalid_cfobject)
        << castExpr->getType() << castType;
        S.Diag(TDNDecl->getLocStart(), diag::note_declared_at);
        if (Target)
          S.Diag(Target->getLocStart(), diag::note_declared_at);
        return true;
      }
      return false;
    }
    T = TDNDecl->getUnderlyingType();
  }
  return true;
}

void Sema::CheckTollFreeBridgeCast(QualType castType, Expr *castExpr) {
  if (!getLangOpts().ObjC1)
    return;
  // warn in presence of __bridge casting to or from a toll free bridge cast.
  ARCConversionTypeClass exprACTC = classifyTypeForARCConversion(castExpr->getType());
  ARCConversionTypeClass castACTC = classifyTypeForARCConversion(castType);
  if (castACTC == ACTC_retainable && exprACTC == ACTC_coreFoundation) {
    bool HasObjCBridgeAttr;
    bool ObjCBridgeAttrWillNotWarn =
      CheckObjCBridgeNSCast<ObjCBridgeAttr>(*this, castType, castExpr, HasObjCBridgeAttr,
                                            false);
    if (ObjCBridgeAttrWillNotWarn && HasObjCBridgeAttr)
      return;
    bool HasObjCBridgeMutableAttr;
    bool ObjCBridgeMutableAttrWillNotWarn =
      CheckObjCBridgeNSCast<ObjCBridgeMutableAttr>(*this, castType, castExpr,
                                                   HasObjCBridgeMutableAttr, false);
    if (ObjCBridgeMutableAttrWillNotWarn && HasObjCBridgeMutableAttr)
      return;
    
    if (HasObjCBridgeAttr)
      CheckObjCBridgeNSCast<ObjCBridgeAttr>(*this, castType, castExpr, HasObjCBridgeAttr,
                                            true);
    else if (HasObjCBridgeMutableAttr)
      CheckObjCBridgeNSCast<ObjCBridgeMutableAttr>(*this, castType, castExpr,
                                                   HasObjCBridgeMutableAttr, true);
  }
  else if (castACTC == ACTC_coreFoundation && exprACTC == ACTC_retainable) {
    bool HasObjCBridgeAttr;
    bool ObjCBridgeAttrWillNotWarn =
      CheckObjCBridgeCFCast<ObjCBridgeAttr>(*this, castType, castExpr, HasObjCBridgeAttr,
                                            false);
    if (ObjCBridgeAttrWillNotWarn && HasObjCBridgeAttr)
      return;
    bool HasObjCBridgeMutableAttr;
    bool ObjCBridgeMutableAttrWillNotWarn =
      CheckObjCBridgeCFCast<ObjCBridgeMutableAttr>(*this, castType, castExpr,
                                                   HasObjCBridgeMutableAttr, false);
    if (ObjCBridgeMutableAttrWillNotWarn && HasObjCBridgeMutableAttr)
      return;
    
    if (HasObjCBridgeAttr)
      CheckObjCBridgeCFCast<ObjCBridgeAttr>(*this, castType, castExpr, HasObjCBridgeAttr,
                                            true);
    else if (HasObjCBridgeMutableAttr)
      CheckObjCBridgeCFCast<ObjCBridgeMutableAttr>(*this, castType, castExpr,
                                                   HasObjCBridgeMutableAttr, true);
  }
}

void Sema::CheckObjCBridgeRelatedCast(QualType castType, Expr *castExpr) {
  QualType SrcType = castExpr->getType();
  if (ObjCPropertyRefExpr *PRE = dyn_cast<ObjCPropertyRefExpr>(castExpr)) {
    if (PRE->isExplicitProperty()) {
      if (ObjCPropertyDecl *PDecl = PRE->getExplicitProperty())
        SrcType = PDecl->getType();
    }
    else if (PRE->isImplicitProperty()) {
      if (ObjCMethodDecl *Getter = PRE->getImplicitPropertyGetter())
        SrcType = Getter->getReturnType();
      
    }
  }
  
  ARCConversionTypeClass srcExprACTC = classifyTypeForARCConversion(SrcType);
  ARCConversionTypeClass castExprACTC = classifyTypeForARCConversion(castType);
  if (srcExprACTC != ACTC_retainable || castExprACTC != ACTC_coreFoundation)
    return;
  CheckObjCBridgeRelatedConversions(castExpr->getLocStart(),
                                    castType, SrcType, castExpr);
  return;
}

bool Sema::CheckTollFreeBridgeStaticCast(QualType castType, Expr *castExpr,
                                         CastKind &Kind) {
  if (!getLangOpts().ObjC1)
    return false;
  ARCConversionTypeClass exprACTC =
    classifyTypeForARCConversion(castExpr->getType());
  ARCConversionTypeClass castACTC = classifyTypeForARCConversion(castType);
  if ((castACTC == ACTC_retainable && exprACTC == ACTC_coreFoundation) ||
      (castACTC == ACTC_coreFoundation && exprACTC == ACTC_retainable)) {
    CheckTollFreeBridgeCast(castType, castExpr);
    Kind = (castACTC == ACTC_coreFoundation) ? CK_BitCast
                                             : CK_CPointerToObjCPointerCast;
    return true;
  }
  return false;
}

bool Sema::checkObjCBridgeRelatedComponents(SourceLocation Loc,
                                            QualType DestType, QualType SrcType,
                                            ObjCInterfaceDecl *&RelatedClass,
                                            ObjCMethodDecl *&ClassMethod,
                                            ObjCMethodDecl *&InstanceMethod,
                                            TypedefNameDecl *&TDNDecl,
                                            bool CfToNs) {
  QualType T = CfToNs ? SrcType : DestType;
  ObjCBridgeRelatedAttr *ObjCBAttr = ObjCBridgeRelatedAttrFromType(T, TDNDecl);
  if (!ObjCBAttr)
    return false;
  
  IdentifierInfo *RCId = ObjCBAttr->getRelatedClass();
  IdentifierInfo *CMId = ObjCBAttr->getClassMethod();
  IdentifierInfo *IMId = ObjCBAttr->getInstanceMethod();
  if (!RCId)
    return false;
  NamedDecl *Target = nullptr;
  // Check for an existing type with this name.
  LookupResult R(*this, DeclarationName(RCId), SourceLocation(),
                 Sema::LookupOrdinaryName);
  if (!LookupName(R, TUScope)) {
    Diag(Loc, diag::err_objc_bridged_related_invalid_class) << RCId
          << SrcType << DestType;
    Diag(TDNDecl->getLocStart(), diag::note_declared_at);
    return false;
  }
  Target = R.getFoundDecl();
  if (Target && isa<ObjCInterfaceDecl>(Target))
    RelatedClass = cast<ObjCInterfaceDecl>(Target);
  else {
    Diag(Loc, diag::err_objc_bridged_related_invalid_class_name) << RCId
          << SrcType << DestType;
    Diag(TDNDecl->getLocStart(), diag::note_declared_at);
    if (Target)
      Diag(Target->getLocStart(), diag::note_declared_at);
    return false;
  }
      
  // Check for an existing class method with the given selector name.
  if (CfToNs && CMId) {
    Selector Sel = Context.Selectors.getUnarySelector(CMId);
    ClassMethod = RelatedClass->lookupMethod(Sel, false);
    if (!ClassMethod) {
      Diag(Loc, diag::err_objc_bridged_related_known_method)
            << SrcType << DestType << Sel << false;
      Diag(TDNDecl->getLocStart(), diag::note_declared_at);
      return false;
    }
  }
      
  // Check for an existing instance method with the given selector name.
  if (!CfToNs && IMId) {
    Selector Sel = Context.Selectors.getNullarySelector(IMId);
    InstanceMethod = RelatedClass->lookupMethod(Sel, true);
    if (!InstanceMethod) {
      Diag(Loc, diag::err_objc_bridged_related_known_method)
            << SrcType << DestType << Sel << true;
      Diag(TDNDecl->getLocStart(), diag::note_declared_at);
      return false;
    }
  }
  return true;
}

bool
Sema::CheckObjCBridgeRelatedConversions(SourceLocation Loc,
                                        QualType DestType, QualType SrcType,
                                        Expr *&SrcExpr) {
  ARCConversionTypeClass rhsExprACTC = classifyTypeForARCConversion(SrcType);
  ARCConversionTypeClass lhsExprACTC = classifyTypeForARCConversion(DestType);
  bool CfToNs = (rhsExprACTC == ACTC_coreFoundation && lhsExprACTC == ACTC_retainable);
  bool NsToCf = (rhsExprACTC == ACTC_retainable && lhsExprACTC == ACTC_coreFoundation);
  if (!CfToNs && !NsToCf)
    return false;
  
  ObjCInterfaceDecl *RelatedClass;
  ObjCMethodDecl *ClassMethod = nullptr;
  ObjCMethodDecl *InstanceMethod = nullptr;
  TypedefNameDecl *TDNDecl = nullptr;
  if (!checkObjCBridgeRelatedComponents(Loc, DestType, SrcType, RelatedClass,
                                        ClassMethod, InstanceMethod, TDNDecl, CfToNs))
    return false;
  
  if (CfToNs) {
    // Implicit conversion from CF to ObjC object is needed.
    if (ClassMethod) {
      std::string ExpressionString = "[";
      ExpressionString += RelatedClass->getNameAsString();
      ExpressionString += " ";
      ExpressionString += ClassMethod->getSelector().getAsString();
      SourceLocation SrcExprEndLoc = PP.getLocForEndOfToken(SrcExpr->getLocEnd());
      // Provide a fixit: [RelatedClass ClassMethod SrcExpr]
      Diag(Loc, diag::err_objc_bridged_related_known_method)
        << SrcType << DestType << ClassMethod->getSelector() << false
        << FixItHint::CreateInsertion(SrcExpr->getLocStart(), ExpressionString)
        << FixItHint::CreateInsertion(SrcExprEndLoc, "]");
      Diag(RelatedClass->getLocStart(), diag::note_declared_at);
      Diag(TDNDecl->getLocStart(), diag::note_declared_at);
      
      QualType receiverType =
        Context.getObjCInterfaceType(RelatedClass);
      // Argument.
      Expr *args[] = { SrcExpr };
      ExprResult msg = BuildClassMessageImplicit(receiverType, false,
                                      ClassMethod->getLocation(),
                                      ClassMethod->getSelector(), ClassMethod,
                                      MultiExprArg(args, 1));
      SrcExpr = msg.get();
      return true;
    }
  }
  else {
    // Implicit conversion from ObjC type to CF object is needed.
    if (InstanceMethod) {
      std::string ExpressionString;
      SourceLocation SrcExprEndLoc = PP.getLocForEndOfToken(SrcExpr->getLocEnd());
      if (InstanceMethod->isPropertyAccessor())
        if (const ObjCPropertyDecl *PDecl = InstanceMethod->findPropertyDecl()) {
          // fixit: ObjectExpr.propertyname when it is  aproperty accessor.
          ExpressionString = ".";
          ExpressionString += PDecl->getNameAsString();
          Diag(Loc, diag::err_objc_bridged_related_known_method)
          << SrcType << DestType << InstanceMethod->getSelector() << true
          << FixItHint::CreateInsertion(SrcExprEndLoc, ExpressionString);
        }
      if (ExpressionString.empty()) {
        // Provide a fixit: [ObjectExpr InstanceMethod]
        ExpressionString = " ";
        ExpressionString += InstanceMethod->getSelector().getAsString();
        ExpressionString += "]";
      
        Diag(Loc, diag::err_objc_bridged_related_known_method)
        << SrcType << DestType << InstanceMethod->getSelector() << true
        << FixItHint::CreateInsertion(SrcExpr->getLocStart(), "[")
        << FixItHint::CreateInsertion(SrcExprEndLoc, ExpressionString);
      }
      Diag(RelatedClass->getLocStart(), diag::note_declared_at);
      Diag(TDNDecl->getLocStart(), diag::note_declared_at);
      
      ExprResult msg =
        BuildInstanceMessageImplicit(SrcExpr, SrcType,
                                     InstanceMethod->getLocation(),
                                     InstanceMethod->getSelector(),
                                     InstanceMethod, None);
      SrcExpr = msg.get();
      return true;
    }
  }
  return false;
}

Sema::ARCConversionResult
Sema::CheckObjCARCConversion(SourceRange castRange, QualType castType,
                             Expr *&castExpr, CheckedConversionKind CCK,
                             bool DiagnoseCFAudited,
                             BinaryOperatorKind Opc) {
  QualType castExprType = castExpr->getType();

  // For the purposes of the classification, we assume reference types
  // will bind to temporaries.
  QualType effCastType = castType;
  if (const ReferenceType *ref = castType->getAs<ReferenceType>())
    effCastType = ref->getPointeeType();
  
  ARCConversionTypeClass exprACTC = classifyTypeForARCConversion(castExprType);
  ARCConversionTypeClass castACTC = classifyTypeForARCConversion(effCastType);
  if (exprACTC == castACTC) {
    // check for viablity and report error if casting an rvalue to a
    // life-time qualifier.
    if ((castACTC == ACTC_retainable) &&
        (CCK == CCK_CStyleCast || CCK == CCK_OtherCast) &&
        (castType != castExprType)) {
      const Type *DT = castType.getTypePtr();
      QualType QDT = castType;
      // We desugar some types but not others. We ignore those
      // that cannot happen in a cast; i.e. auto, and those which
      // should not be de-sugared; i.e typedef.
      if (const ParenType *PT = dyn_cast<ParenType>(DT))
        QDT = PT->desugar();
      else if (const TypeOfType *TP = dyn_cast<TypeOfType>(DT))
        QDT = TP->desugar();
      else if (const AttributedType *AT = dyn_cast<AttributedType>(DT))
        QDT = AT->desugar();
      if (QDT != castType &&
          QDT.getObjCLifetime() !=  Qualifiers::OCL_None) {
        SourceLocation loc =
          (castRange.isValid() ? castRange.getBegin() 
                              : castExpr->getExprLoc());
        Diag(loc, diag::err_arc_nolifetime_behavior);
      }
    }
    return ACR_okay;
  }
  
  if (isAnyCLike(exprACTC) && isAnyCLike(castACTC)) return ACR_okay;

  // Allow all of these types to be cast to integer types (but not
  // vice-versa).
  if (castACTC == ACTC_none && castType->isIntegralType(Context))
    return ACR_okay;
  
  // Allow casts between pointers to lifetime types (e.g., __strong id*)
  // and pointers to void (e.g., cv void *). Casting from void* to lifetime*
  // must be explicit.
  if (exprACTC == ACTC_indirectRetainable && castACTC == ACTC_voidPtr)
    return ACR_okay;
  if (castACTC == ACTC_indirectRetainable && exprACTC == ACTC_voidPtr &&
      CCK != CCK_ImplicitConversion)
    return ACR_okay;

  switch (ARCCastChecker(Context, exprACTC, castACTC, false).Visit(castExpr)) {
  // For invalid casts, fall through.
  case ACC_invalid:
    break;

  // Do nothing for both bottom and +0.
  case ACC_bottom:
  case ACC_plusZero:
    return ACR_okay;

  // If the result is +1, consume it here.
  case ACC_plusOne:
    castExpr = ImplicitCastExpr::Create(Context, castExpr->getType(),
                                        CK_ARCConsumeObject, castExpr,
                                        nullptr, VK_RValue);
    ExprNeedsCleanups = true;
    return ACR_okay;
  }

  // If this is a non-implicit cast from id or block type to a
  // CoreFoundation type, delay complaining in case the cast is used
  // in an acceptable context.
  if (exprACTC == ACTC_retainable && isAnyRetainable(castACTC) &&
      CCK != CCK_ImplicitConversion)
    return ACR_unbridged;

  // Do not issue bridge cast" diagnostic when implicit casting a cstring
  // to 'NSString *'. Let caller issue a normal mismatched diagnostic with
  // suitable fix-it.
  if (castACTC == ACTC_retainable && exprACTC == ACTC_none &&
      ConversionToObjCStringLiteralCheck(castType, castExpr))
    return ACR_okay;
  
  // Do not issue "bridge cast" diagnostic when implicit casting
  // a retainable object to a CF type parameter belonging to an audited
  // CF API function. Let caller issue a normal type mismatched diagnostic
  // instead.
  if (!DiagnoseCFAudited || exprACTC != ACTC_retainable ||
      castACTC != ACTC_coreFoundation)
    if (!(exprACTC == ACTC_voidPtr && castACTC == ACTC_retainable &&
          (Opc == BO_NE || Opc == BO_EQ)))
      diagnoseObjCARCConversion(*this, castRange, castType, castACTC,
                                castExpr, castExpr, exprACTC, CCK);
  return ACR_okay;
}

/// Given that we saw an expression with the ARCUnbridgedCastTy
/// placeholder type, complain bitterly.
void Sema::diagnoseARCUnbridgedCast(Expr *e) {
  // We expect the spurious ImplicitCastExpr to already have been stripped.
  assert(!e->hasPlaceholderType(BuiltinType::ARCUnbridgedCast));
  CastExpr *realCast = cast<CastExpr>(e->IgnoreParens());

  SourceRange castRange;
  QualType castType;
  CheckedConversionKind CCK;

  if (CStyleCastExpr *cast = dyn_cast<CStyleCastExpr>(realCast)) {
    castRange = SourceRange(cast->getLParenLoc(), cast->getRParenLoc());
    castType = cast->getTypeAsWritten();
    CCK = CCK_CStyleCast;
  } else if (ExplicitCastExpr *cast = dyn_cast<ExplicitCastExpr>(realCast)) {
    castRange = cast->getTypeInfoAsWritten()->getTypeLoc().getSourceRange();
    castType = cast->getTypeAsWritten();
    CCK = CCK_OtherCast;
  } else {
    castType = cast->getType();
    CCK = CCK_ImplicitConversion;
  }

  ARCConversionTypeClass castACTC =
    classifyTypeForARCConversion(castType.getNonReferenceType());

  Expr *castExpr = realCast->getSubExpr();
  assert(classifyTypeForARCConversion(castExpr->getType()) == ACTC_retainable);

  diagnoseObjCARCConversion(*this, castRange, castType, castACTC,
                            castExpr, realCast, ACTC_retainable, CCK);
}

/// stripARCUnbridgedCast - Given an expression of ARCUnbridgedCast
/// type, remove the placeholder cast.
Expr *Sema::stripARCUnbridgedCast(Expr *e) {
  assert(e->hasPlaceholderType(BuiltinType::ARCUnbridgedCast));

  if (ParenExpr *pe = dyn_cast<ParenExpr>(e)) {
    Expr *sub = stripARCUnbridgedCast(pe->getSubExpr());
    return new (Context) ParenExpr(pe->getLParen(), pe->getRParen(), sub);
  } else if (UnaryOperator *uo = dyn_cast<UnaryOperator>(e)) {
    assert(uo->getOpcode() == UO_Extension);
    Expr *sub = stripARCUnbridgedCast(uo->getSubExpr());
    return new (Context) UnaryOperator(sub, UO_Extension, sub->getType(),
                                   sub->getValueKind(), sub->getObjectKind(),
                                       uo->getOperatorLoc());
  } else if (GenericSelectionExpr *gse = dyn_cast<GenericSelectionExpr>(e)) {
    assert(!gse->isResultDependent());

    unsigned n = gse->getNumAssocs();
    SmallVector<Expr*, 4> subExprs(n);
    SmallVector<TypeSourceInfo*, 4> subTypes(n);
    for (unsigned i = 0; i != n; ++i) {
      subTypes[i] = gse->getAssocTypeSourceInfo(i);
      Expr *sub = gse->getAssocExpr(i);
      if (i == gse->getResultIndex())
        sub = stripARCUnbridgedCast(sub);
      subExprs[i] = sub;
    }

    return new (Context) GenericSelectionExpr(Context, gse->getGenericLoc(),
                                              gse->getControllingExpr(),
                                              subTypes, subExprs,
                                              gse->getDefaultLoc(),
                                              gse->getRParenLoc(),
                                       gse->containsUnexpandedParameterPack(),
                                              gse->getResultIndex());
  } else {
    assert(isa<ImplicitCastExpr>(e) && "bad form of unbridged cast!");
    return cast<ImplicitCastExpr>(e)->getSubExpr();
  }
}

bool Sema::CheckObjCARCUnavailableWeakConversion(QualType castType,
                                                 QualType exprType) {
  QualType canCastType = 
    Context.getCanonicalType(castType).getUnqualifiedType();
  QualType canExprType = 
    Context.getCanonicalType(exprType).getUnqualifiedType();
  if (isa<ObjCObjectPointerType>(canCastType) &&
      castType.getObjCLifetime() == Qualifiers::OCL_Weak &&
      canExprType->isObjCObjectPointerType()) {
    if (const ObjCObjectPointerType *ObjT =
        canExprType->getAs<ObjCObjectPointerType>())
      if (const ObjCInterfaceDecl *ObjI = ObjT->getInterfaceDecl())
        return !ObjI->isArcWeakrefUnavailable();
  }
  return true;
}

/// Look for an ObjCReclaimReturnedObject cast and destroy it.
static Expr *maybeUndoReclaimObject(Expr *e) {
  // For now, we just undo operands that are *immediately* reclaim
  // expressions, which prevents the vast majority of potential
  // problems here.  To catch them all, we'd need to rebuild arbitrary
  // value-propagating subexpressions --- we can't reliably rebuild
  // in-place because of expression sharing.
  if (ImplicitCastExpr *ice = dyn_cast<ImplicitCastExpr>(e))
    if (ice->getCastKind() == CK_ARCReclaimReturnedObject)
      return ice->getSubExpr();

  return e;
}

ExprResult Sema::BuildObjCBridgedCast(SourceLocation LParenLoc,
                                      ObjCBridgeCastKind Kind,
                                      SourceLocation BridgeKeywordLoc,
                                      TypeSourceInfo *TSInfo,
                                      Expr *SubExpr) {
  ExprResult SubResult = UsualUnaryConversions(SubExpr);
  if (SubResult.isInvalid()) return ExprError();
  SubExpr = SubResult.get();

  QualType T = TSInfo->getType();
  QualType FromType = SubExpr->getType();

  CastKind CK;

  bool MustConsume = false;
  if (T->isDependentType() || SubExpr->isTypeDependent()) {
    // Okay: we'll build a dependent expression type.
    CK = CK_Dependent;
  } else if (T->isObjCARCBridgableType() && FromType->isCARCBridgableType()) {
    // Casting CF -> id
    CK = (T->isBlockPointerType() ? CK_AnyPointerToBlockPointerCast
                                  : CK_CPointerToObjCPointerCast);
    switch (Kind) {
    case OBC_Bridge:
      break;
      
    case OBC_BridgeRetained: {
      bool br = isKnownName("CFBridgingRelease");
      Diag(BridgeKeywordLoc, diag::err_arc_bridge_cast_wrong_kind)
        << 2
        << FromType
        << (T->isBlockPointerType()? 1 : 0)
        << T
        << SubExpr->getSourceRange()
        << Kind;
      Diag(BridgeKeywordLoc, diag::note_arc_bridge)
        << FixItHint::CreateReplacement(BridgeKeywordLoc, "__bridge");
      Diag(BridgeKeywordLoc, diag::note_arc_bridge_transfer)
        << FromType << br
        << FixItHint::CreateReplacement(BridgeKeywordLoc, 
                                        br ? "CFBridgingRelease " 
                                           : "__bridge_transfer ");

      Kind = OBC_Bridge;
      break;
    }
      
    case OBC_BridgeTransfer:
      // We must consume the Objective-C object produced by the cast.
      MustConsume = true;
      break;
    }
  } else if (T->isCARCBridgableType() && FromType->isObjCARCBridgableType()) {
    // Okay: id -> CF
    CK = CK_BitCast;
    switch (Kind) {
    case OBC_Bridge:
      // Reclaiming a value that's going to be __bridge-casted to CF
      // is very dangerous, so we don't do it.
      SubExpr = maybeUndoReclaimObject(SubExpr);
      break;
      
    case OBC_BridgeRetained:        
      // Produce the object before casting it.
      SubExpr = ImplicitCastExpr::Create(Context, FromType,
                                         CK_ARCProduceObject,
                                         SubExpr, nullptr, VK_RValue);
      break;
      
    case OBC_BridgeTransfer: {
      bool br = isKnownName("CFBridgingRetain");
      Diag(BridgeKeywordLoc, diag::err_arc_bridge_cast_wrong_kind)
        << (FromType->isBlockPointerType()? 1 : 0)
        << FromType
        << 2
        << T
        << SubExpr->getSourceRange()
        << Kind;
        
      Diag(BridgeKeywordLoc, diag::note_arc_bridge)
        << FixItHint::CreateReplacement(BridgeKeywordLoc, "__bridge ");
      Diag(BridgeKeywordLoc, diag::note_arc_bridge_retained)
        << T << br
        << FixItHint::CreateReplacement(BridgeKeywordLoc, 
                          br ? "CFBridgingRetain " : "__bridge_retained");
        
      Kind = OBC_Bridge;
      break;
    }
    }
  } else {
    Diag(LParenLoc, diag::err_arc_bridge_cast_incompatible)
      << FromType << T << Kind
      << SubExpr->getSourceRange()
      << TSInfo->getTypeLoc().getSourceRange();
    return ExprError();
  }

  Expr *Result = new (Context) ObjCBridgedCastExpr(LParenLoc, Kind, CK,
                                                   BridgeKeywordLoc,
                                                   TSInfo, SubExpr);
  
  if (MustConsume) {
    ExprNeedsCleanups = true;
    Result = ImplicitCastExpr::Create(Context, T, CK_ARCConsumeObject, Result, 
                                      nullptr, VK_RValue);
  }
  
  return Result;
}

ExprResult Sema::ActOnObjCBridgedCast(Scope *S,
                                      SourceLocation LParenLoc,
                                      ObjCBridgeCastKind Kind,
                                      SourceLocation BridgeKeywordLoc,
                                      ParsedType Type,
                                      SourceLocation RParenLoc,
                                      Expr *SubExpr) {
  TypeSourceInfo *TSInfo = nullptr;
  QualType T = GetTypeFromParser(Type, &TSInfo);
  if (Kind == OBC_Bridge)
    CheckTollFreeBridgeCast(T, SubExpr);
  if (!TSInfo)
    TSInfo = Context.getTrivialTypeSourceInfo(T, LParenLoc);
  return BuildObjCBridgedCast(LParenLoc, Kind, BridgeKeywordLoc, TSInfo, 
                              SubExpr);
}

#endif // HLSL Change
