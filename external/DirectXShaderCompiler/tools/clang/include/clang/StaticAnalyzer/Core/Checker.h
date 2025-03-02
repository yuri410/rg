//== Checker.h - Registration mechanism for checkers -------------*- C++ -*--=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines Checker, used to create and register checkers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CORE_CHECKER_H
#define LLVM_CLANG_STATICANALYZER_CORE_CHECKER_H

#include "clang/Analysis/ProgramPoint.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "llvm/Support/Casting.h"

namespace clang {
namespace ento {
  class BugReporter;

namespace check {

template <typename DECL>
class ASTDecl {
  template <typename CHECKER>
  static void _checkDecl(void *checker, const Decl *D, AnalysisManager& mgr,
                         BugReporter &BR) {
    ((const CHECKER *)checker)->checkASTDecl(cast<DECL>(D), mgr, BR);
  }

  static bool _handlesDecl(const Decl *D) {
    return isa<DECL>(D);
  }
public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForDecl(CheckerManager::CheckDeclFunc(checker,
                                                       _checkDecl<CHECKER>),
                         _handlesDecl);
  }
};

class ASTCodeBody {
  template <typename CHECKER>
  static void _checkBody(void *checker, const Decl *D, AnalysisManager& mgr,
                         BugReporter &BR) {
    ((const CHECKER *)checker)->checkASTCodeBody(D, mgr, BR);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForBody(CheckerManager::CheckDeclFunc(checker,
                                                       _checkBody<CHECKER>));
  }
};

class EndOfTranslationUnit {
  template <typename CHECKER>
  static void _checkEndOfTranslationUnit(void *checker,
                                         const TranslationUnitDecl *TU, 
                                         AnalysisManager& mgr,
                                         BugReporter &BR) {
    ((const CHECKER *)checker)->checkEndOfTranslationUnit(TU, mgr, BR);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr){
    mgr._registerForEndOfTranslationUnit(
                              CheckerManager::CheckEndOfTranslationUnit(checker,
                                          _checkEndOfTranslationUnit<CHECKER>));
  }
};

template <typename STMT>
class PreStmt {
  template <typename CHECKER>
  static void _checkStmt(void *checker, const Stmt *S, CheckerContext &C) {
    ((const CHECKER *)checker)->checkPreStmt(cast<STMT>(S), C);
  }

  static bool _handlesStmt(const Stmt *S) {
    return isa<STMT>(S);
  }
public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForPreStmt(CheckerManager::CheckStmtFunc(checker,
                                                          _checkStmt<CHECKER>),
                            _handlesStmt);
  }
};

template <typename STMT>
class PostStmt {
  template <typename CHECKER>
  static void _checkStmt(void *checker, const Stmt *S, CheckerContext &C) {
    ((const CHECKER *)checker)->checkPostStmt(cast<STMT>(S), C);
  }

  static bool _handlesStmt(const Stmt *S) {
    return isa<STMT>(S);
  }
public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForPostStmt(CheckerManager::CheckStmtFunc(checker,
                                                           _checkStmt<CHECKER>),
                             _handlesStmt);
  }
};

class PreObjCMessage {
  template <typename CHECKER>
  static void _checkObjCMessage(void *checker, const ObjCMethodCall &msg,
                                CheckerContext &C) {
    ((const CHECKER *)checker)->checkPreObjCMessage(msg, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForPreObjCMessage(
     CheckerManager::CheckObjCMessageFunc(checker, _checkObjCMessage<CHECKER>));
  }
};

class PostObjCMessage {
  template <typename CHECKER>
  static void _checkObjCMessage(void *checker, const ObjCMethodCall &msg,
                                CheckerContext &C) {
    ((const CHECKER *)checker)->checkPostObjCMessage(msg, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForPostObjCMessage(
     CheckerManager::CheckObjCMessageFunc(checker, _checkObjCMessage<CHECKER>));
  }
};

class PreCall {
  template <typename CHECKER>
  static void _checkCall(void *checker, const CallEvent &msg,
                         CheckerContext &C) {
    ((const CHECKER *)checker)->checkPreCall(msg, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForPreCall(
     CheckerManager::CheckCallFunc(checker, _checkCall<CHECKER>));
  }
};

class PostCall {
  template <typename CHECKER>
  static void _checkCall(void *checker, const CallEvent &msg,
                         CheckerContext &C) {
    ((const CHECKER *)checker)->checkPostCall(msg, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForPostCall(
     CheckerManager::CheckCallFunc(checker, _checkCall<CHECKER>));
  }
};

class Location {
  template <typename CHECKER>
  static void _checkLocation(void *checker,
                             const SVal &location, bool isLoad, const Stmt *S,
                             CheckerContext &C) {
    ((const CHECKER *)checker)->checkLocation(location, isLoad, S, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForLocation(
           CheckerManager::CheckLocationFunc(checker, _checkLocation<CHECKER>));
  }
};

class Bind {
  template <typename CHECKER>
  static void _checkBind(void *checker,
                         const SVal &location, const SVal &val, const Stmt *S,
                         CheckerContext &C) {
    ((const CHECKER *)checker)->checkBind(location, val, S, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForBind(
           CheckerManager::CheckBindFunc(checker, _checkBind<CHECKER>));
  }
};

class EndAnalysis {
  template <typename CHECKER>
  static void _checkEndAnalysis(void *checker, ExplodedGraph &G,
                                BugReporter &BR, ExprEngine &Eng) {
    ((const CHECKER *)checker)->checkEndAnalysis(G, BR, Eng);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForEndAnalysis(
     CheckerManager::CheckEndAnalysisFunc(checker, _checkEndAnalysis<CHECKER>));
  }
};

class EndFunction {
  template <typename CHECKER>
  static void _checkEndFunction(void *checker,
                                CheckerContext &C) {
    ((const CHECKER *)checker)->checkEndFunction(C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForEndFunction(
     CheckerManager::CheckEndFunctionFunc(checker, _checkEndFunction<CHECKER>));
  }
};

class BranchCondition {
  template <typename CHECKER>
  static void _checkBranchCondition(void *checker, const Stmt *Condition,
                                    CheckerContext & C) {
    ((const CHECKER *)checker)->checkBranchCondition(Condition, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForBranchCondition(
      CheckerManager::CheckBranchConditionFunc(checker,
                                               _checkBranchCondition<CHECKER>));
  }
};

class LiveSymbols {
  template <typename CHECKER>
  static void _checkLiveSymbols(void *checker, ProgramStateRef state,
                                SymbolReaper &SR) {
    ((const CHECKER *)checker)->checkLiveSymbols(state, SR);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForLiveSymbols(
     CheckerManager::CheckLiveSymbolsFunc(checker, _checkLiveSymbols<CHECKER>));
  }
};

class DeadSymbols {
  template <typename CHECKER>
  static void _checkDeadSymbols(void *checker,
                                SymbolReaper &SR, CheckerContext &C) {
    ((const CHECKER *)checker)->checkDeadSymbols(SR, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForDeadSymbols(
     CheckerManager::CheckDeadSymbolsFunc(checker, _checkDeadSymbols<CHECKER>));
  }
};

class RegionChanges {
  template <typename CHECKER>
  static ProgramStateRef 
  _checkRegionChanges(void *checker,
                      ProgramStateRef state,
                      const InvalidatedSymbols *invalidated,
                      ArrayRef<const MemRegion *> Explicits,
                      ArrayRef<const MemRegion *> Regions,
                      const CallEvent *Call) {
    return ((const CHECKER *)checker)->checkRegionChanges(state, invalidated,
                                                      Explicits, Regions, Call);
  }
  template <typename CHECKER>
  static bool _wantsRegionChangeUpdate(void *checker,
                                       ProgramStateRef state) {
    return ((const CHECKER *)checker)->wantsRegionChangeUpdate(state);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForRegionChanges(
          CheckerManager::CheckRegionChangesFunc(checker,
                                                 _checkRegionChanges<CHECKER>),
          CheckerManager::WantsRegionChangeUpdateFunc(checker,
                                            _wantsRegionChangeUpdate<CHECKER>));
  }
};

class PointerEscape {
  template <typename CHECKER>
  static ProgramStateRef
  _checkPointerEscape(void *Checker,
                     ProgramStateRef State,
                     const InvalidatedSymbols &Escaped,
                     const CallEvent *Call,
                     PointerEscapeKind Kind,
                     RegionAndSymbolInvalidationTraits *ETraits) {

    if (!ETraits)
      return ((const CHECKER *)Checker)->checkPointerEscape(State,
                                                            Escaped,
                                                            Call,
                                                            Kind);

    InvalidatedSymbols RegularEscape;
    for (InvalidatedSymbols::const_iterator I = Escaped.begin(), 
                                            E = Escaped.end(); I != E; ++I)
      if (!ETraits->hasTrait(*I,
              RegionAndSymbolInvalidationTraits::TK_PreserveContents) &&
          !ETraits->hasTrait(*I,
              RegionAndSymbolInvalidationTraits::TK_SuppressEscape))
        RegularEscape.insert(*I);

    if (RegularEscape.empty())
      return State;

    return ((const CHECKER *)Checker)->checkPointerEscape(State,
                                                          RegularEscape,
                                                          Call,
                                                          Kind);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForPointerEscape(
          CheckerManager::CheckPointerEscapeFunc(checker,
                                                _checkPointerEscape<CHECKER>));
  }
};

class ConstPointerEscape {
  template <typename CHECKER>
  static ProgramStateRef
  _checkConstPointerEscape(void *Checker,
                      ProgramStateRef State,
                      const InvalidatedSymbols &Escaped,
                      const CallEvent *Call,
                      PointerEscapeKind Kind,
                      RegionAndSymbolInvalidationTraits *ETraits) {

    if (!ETraits)
      return State;

    InvalidatedSymbols ConstEscape;
    for (InvalidatedSymbols::const_iterator I = Escaped.begin(), 
                                            E = Escaped.end(); I != E; ++I)
      if (ETraits->hasTrait(*I,
              RegionAndSymbolInvalidationTraits::TK_PreserveContents) &&
          !ETraits->hasTrait(*I,
              RegionAndSymbolInvalidationTraits::TK_SuppressEscape))
        ConstEscape.insert(*I);

    if (ConstEscape.empty())
      return State;

    return ((const CHECKER *)Checker)->checkConstPointerEscape(State,
                                                               ConstEscape,
                                                               Call,
                                                               Kind);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForPointerEscape(
      CheckerManager::CheckPointerEscapeFunc(checker,
                                            _checkConstPointerEscape<CHECKER>));
  }
};

  
template <typename EVENT>
class Event {
  template <typename CHECKER>
  static void _checkEvent(void *checker, const void *event) {
    ((const CHECKER *)checker)->checkEvent(*(const EVENT *)event);
  }
public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerListenerForEvent<EVENT>(
                 CheckerManager::CheckEventFunc(checker, _checkEvent<CHECKER>));
  }
};

} // end check namespace

namespace eval {

class Assume {
  template <typename CHECKER>
  static ProgramStateRef _evalAssume(void *checker,
                                         ProgramStateRef state,
                                         const SVal &cond,
                                         bool assumption) {
    return ((const CHECKER *)checker)->evalAssume(state, cond, assumption);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForEvalAssume(
                 CheckerManager::EvalAssumeFunc(checker, _evalAssume<CHECKER>));
  }
};

class Call {
  template <typename CHECKER>
  static bool _evalCall(void *checker, const CallExpr *CE, CheckerContext &C) {
    return ((const CHECKER *)checker)->evalCall(CE, C);
  }

public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerForEvalCall(
                     CheckerManager::EvalCallFunc(checker, _evalCall<CHECKER>));
  }
};

} // end eval namespace

class CheckerBase : public ProgramPointTag {
  CheckName Name;
  friend class ::clang::ento::CheckerManager;

public:
  StringRef getTagDescription() const override;
  CheckName getCheckName() const;

  /// See CheckerManager::runCheckersForPrintState.
  virtual void printState(raw_ostream &Out, ProgramStateRef State,
                          const char *NL, const char *Sep) const { }
};

/// Dump checker name to stream.
raw_ostream& operator<<(raw_ostream &Out, const CheckerBase &Checker);

/// Tag that can use a checker name as a message provider 
/// (see SimpleProgramPointTag).
class CheckerProgramPointTag : public SimpleProgramPointTag {
public:
  CheckerProgramPointTag(StringRef CheckerName, StringRef Msg);
  CheckerProgramPointTag(const CheckerBase *Checker, StringRef Msg);
};

template <typename CHECK1, typename... CHECKs>
class Checker : public CHECK1, public CHECKs..., public CheckerBase {
public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    CHECK1::_register(checker, mgr);
    Checker<CHECKs...>::_register(checker, mgr);
  }
};

template <typename CHECK1>
class Checker<CHECK1> : public CHECK1, public CheckerBase {
public:
  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    CHECK1::_register(checker, mgr);
  }
};

template <typename EVENT>
class EventDispatcher {
  CheckerManager *Mgr;
public:
  EventDispatcher() : Mgr(nullptr) { }

  template <typename CHECKER>
  static void _register(CHECKER *checker, CheckerManager &mgr) {
    mgr._registerDispatcherForEvent<EVENT>();
    static_cast<EventDispatcher<EVENT> *>(checker)->Mgr = &mgr;
  }

  void dispatchEvent(const EVENT &event) const {
    Mgr->_dispatchEvent(event);
  }
};

/// \brief We dereferenced a location that may be null.
struct ImplicitNullDerefEvent {
  SVal Location;
  bool IsLoad;
  ExplodedNode *SinkNode;
  BugReporter *BR;
};

/// \brief A helper class which wraps a boolean value set to false by default.
///
/// This class should behave exactly like 'bool' except that it doesn't need to
/// be explicitly initialized.
struct DefaultBool {
  bool val;
  DefaultBool() : val(false) {}
  /*implicit*/ operator bool&() { return val; }
  /*implicit*/ operator const bool&() const { return val; }
  DefaultBool &operator=(bool b) { val = b; return *this; }
};

} // end ento namespace

} // end clang namespace

#endif
