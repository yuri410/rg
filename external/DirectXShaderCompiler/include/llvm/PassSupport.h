//===- llvm/PassSupport.h - Pass Support code -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines stuff that is used to define and "use" Passes.  This file
// is automatically #included by Pass.h, so:
//
//           NO .CPP FILES SHOULD INCLUDE THIS FILE DIRECTLY
//
// Instead, #include Pass.h.
//
// This file defines Pass registration code and classes used for it.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PASSSUPPORT_H
#define LLVM_PASSSUPPORT_H

#include "Pass.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassInfo.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/Atomic.h"
#include "llvm/Support/Valgrind.h"
#include <vector>

namespace llvm {

class TargetMachine;

#define CALL_ONCE_INITIALIZATION(function) \
  static volatile sys::cas_flag initialized = 0; \
  sys::cas_flag old_val = sys::CompareAndSwap(&initialized, 1, 0); \
  if (old_val == 0) { \
    function(Registry); \
    sys::MemoryFence(); \
    TsanIgnoreWritesBegin(); \
    TsanHappensBefore(&initialized); \
    initialized = 2; \
    TsanIgnoreWritesEnd(); \
  } else { \
    sys::cas_flag tmp = initialized; \
    sys::MemoryFence(); \
    while (tmp != 2) { \
      tmp = initialized; \
      sys::MemoryFence(); \
    } \
  } \
  TsanHappensAfter(&initialized);

#define INITIALIZE_PASS(passName, arg, name, cfg, analysis) \
  static void* initialize##passName##PassOnce(PassRegistry &Registry) { \
    PassInfo *PI = new PassInfo(name, arg, & passName ::ID, \
      PassInfo::NormalCtor_t(callDefaultCtor< passName >), cfg, analysis); \
    Registry.registerPass(*PI, true); \
    return PI; \
  } \
  void llvm::initialize##passName##Pass(PassRegistry &Registry) { \
    CALL_ONCE_INITIALIZATION(initialize##passName##PassOnce) \
  }

#define INITIALIZE_PASS_BEGIN(passName, arg, name, cfg, analysis) \
  static void* initialize##passName##PassOnce(PassRegistry &Registry) {

#define INITIALIZE_PASS_DEPENDENCY(depName) \
    initialize##depName##Pass(Registry);
#define INITIALIZE_AG_DEPENDENCY(depName) \
    initialize##depName##AnalysisGroup(Registry);

#define INITIALIZE_PASS_END(passName, arg, name, cfg, analysis) \
    PassInfo *PI = new PassInfo(name, arg, & passName ::ID, \
      PassInfo::NormalCtor_t(callDefaultCtor< passName >), cfg, analysis); \
    Registry.registerPass(*PI, true); \
    return PI; \
  } \
  void llvm::initialize##passName##Pass(PassRegistry &Registry) { \
    CALL_ONCE_INITIALIZATION(initialize##passName##PassOnce) \
  }

#define INITIALIZE_PASS_WITH_OPTIONS(PassName, Arg, Name, Cfg, Analysis) \
  INITIALIZE_PASS_BEGIN(PassName, Arg, Name, Cfg, Analysis) \
  PassName::registerOptions(); \
  INITIALIZE_PASS_END(PassName, Arg, Name, Cfg, Analysis)

#define INITIALIZE_PASS_WITH_OPTIONS_BEGIN(PassName, Arg, Name, Cfg, Analysis) \
  INITIALIZE_PASS_BEGIN(PassName, Arg, Name, Cfg, Analysis) \
  PassName::registerOptions(); \

template<typename PassName>
Pass *callDefaultCtor() { return new PassName(); }

template <typename PassName> Pass *callTargetMachineCtor(TargetMachine *TM) {
  return new PassName(TM);
}

//===---------------------------------------------------------------------------
/// RegisterPass<t> template - This template class is used to notify the system
/// that a Pass is available for use, and registers it into the internal
/// database maintained by the PassManager.  Unless this template is used, opt,
/// for example will not be able to see the pass and attempts to create the pass
/// will fail. This template is used in the follow manner (at global scope, in
/// your .cpp file):
///
/// static RegisterPass<YourPassClassName> tmp("passopt", "My Pass Name");
///
/// This statement will cause your pass to be created by calling the default
/// constructor exposed by the pass.  If you have a different constructor that
/// must be called, create a global constructor function (which takes the
/// arguments you need and returns a Pass*) and register your pass like this:
///
/// static RegisterPass<PassClassName> tmp("passopt", "My Name");
///
template<typename passName>
struct RegisterPass : public PassInfo {

  // Register Pass using default constructor...
  RegisterPass(const char *PassArg, const char *Name, bool CFGOnly = false,
               bool is_analysis = false)
    : PassInfo(Name, PassArg, &passName::ID,
               PassInfo::NormalCtor_t(callDefaultCtor<passName>),
               CFGOnly, is_analysis) {
    PassRegistry::getPassRegistry()->registerPass(*this);
  }
};


/// RegisterAnalysisGroup - Register a Pass as a member of an analysis _group_.
/// Analysis groups are used to define an interface (which need not derive from
/// Pass) that is required by passes to do their job.  Analysis Groups differ
/// from normal analyses because any available implementation of the group will
/// be used if it is available.
///
/// If no analysis implementing the interface is available, a default
/// implementation is created and added.  A pass registers itself as the default
/// implementation by specifying 'true' as the second template argument of this
/// class.
///
/// In addition to registering itself as an analysis group member, a pass must
/// register itself normally as well.  Passes may be members of multiple groups
/// and may still be "required" specifically by name.
///
/// The actual interface may also be registered as well (by not specifying the
/// second template argument).  The interface should be registered to associate
/// a nice name with the interface.
///
class RegisterAGBase : public PassInfo {
public:
  RegisterAGBase(const char *Name,
                 const void *InterfaceID,
                 const void *PassID = nullptr,
                 bool isDefault = false);
};

template<typename Interface, bool Default = false>
struct RegisterAnalysisGroup : public RegisterAGBase {
  explicit RegisterAnalysisGroup(PassInfo &RPB)
    : RegisterAGBase(RPB.getPassName(),
                     &Interface::ID, RPB.getTypeInfo(),
                     Default) {
  }

  explicit RegisterAnalysisGroup(const char *Name)
    : RegisterAGBase(Name, &Interface::ID) {
  }
};

#define INITIALIZE_ANALYSIS_GROUP(agName, name, defaultPass) \
  static void* initialize##agName##AnalysisGroupOnce(PassRegistry &Registry) { \
    initialize##defaultPass##Pass(Registry); \
    PassInfo *AI = new PassInfo(name, & agName :: ID); \
    Registry.registerAnalysisGroup(& agName ::ID, 0, *AI, false, true); \
    return AI; \
  } \
  void llvm::initialize##agName##AnalysisGroup(PassRegistry &Registry) { \
    CALL_ONCE_INITIALIZATION(initialize##agName##AnalysisGroupOnce) \
  }


#define INITIALIZE_AG_PASS(passName, agName, arg, name, cfg, analysis, def) \
  static void* initialize##passName##PassOnce(PassRegistry &Registry) { \
    if (!def) initialize##agName##AnalysisGroup(Registry); \
    PassInfo *PI = new PassInfo(name, arg, & passName ::ID, \
      PassInfo::NormalCtor_t(callDefaultCtor< passName >), cfg, analysis); \
    Registry.registerPass(*PI, true); \
    \
    PassInfo *AI = new PassInfo(name, & agName :: ID); \
    Registry.registerAnalysisGroup(& agName ::ID, & passName ::ID, \
                                   *AI, def, true); \
    return AI; \
  } \
  void llvm::initialize##passName##Pass(PassRegistry &Registry) { \
    CALL_ONCE_INITIALIZATION(initialize##passName##PassOnce) \
  }


#define INITIALIZE_AG_PASS_BEGIN(passName, agName, arg, n, cfg, analysis, def) \
  static void* initialize##passName##PassOnce(PassRegistry &Registry) { \
    if (!def) initialize##agName##AnalysisGroup(Registry);

#define INITIALIZE_AG_PASS_END(passName, agName, arg, n, cfg, analysis, def) \
    PassInfo *PI = new PassInfo(n, arg, & passName ::ID, \
      PassInfo::NormalCtor_t(callDefaultCtor< passName >), cfg, analysis); \
    Registry.registerPass(*PI, true); \
    \
    PassInfo *AI = new PassInfo(n, & agName :: ID); \
    Registry.registerAnalysisGroup(& agName ::ID, & passName ::ID, \
                                   *AI, def, true); \
    return AI; \
  } \
  void llvm::initialize##passName##Pass(PassRegistry &Registry) { \
    CALL_ONCE_INITIALIZATION(initialize##passName##PassOnce) \
  }

//===---------------------------------------------------------------------------
/// PassRegistrationListener class - This class is meant to be derived from by
/// clients that are interested in which passes get registered and unregistered
/// at runtime (which can be because of the RegisterPass constructors being run
/// as the program starts up, or may be because a shared object just got
/// loaded).
///
struct PassRegistrationListener {

  PassRegistrationListener() {}
  virtual ~PassRegistrationListener() {}

  /// Callback functions - These functions are invoked whenever a pass is loaded
  /// or removed from the current executable.
  ///
  virtual void passRegistered(const PassInfo *) {}

  /// enumeratePasses - Iterate over the registered passes, calling the
  /// passEnumerate callback on each PassInfo object.
  ///
  void enumeratePasses();

  /// passEnumerate - Callback function invoked when someone calls
  /// enumeratePasses on this PassRegistrationListener object.
  ///
  virtual void passEnumerate(const PassInfo *) {}
};


} // End llvm namespace

#endif
