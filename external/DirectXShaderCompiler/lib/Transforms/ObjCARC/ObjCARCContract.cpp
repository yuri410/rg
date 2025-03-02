//===- ObjCARCContract.cpp - ObjC ARC Optimization ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file defines late ObjC ARC optimizations. ARC stands for Automatic
/// Reference Counting and is a system for managing reference counts for objects
/// in Objective C.
///
/// This specific file mainly deals with ``contracting'' multiple lower level
/// operations into singular higher level operations through pattern matching.
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
///
//===----------------------------------------------------------------------===//

// TODO: ObjCARCContract could insert PHI nodes when uses aren't
// dominated by single calls.

#include "ObjCARC.h"
#include "ARCRuntimeEntryPoints.h"
#include "DependencyAnalysis.h"
#include "ProvenanceAnalysis.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::objcarc;

#define DEBUG_TYPE "objc-arc-contract"

STATISTIC(NumPeeps,       "Number of calls peephole-optimized");
STATISTIC(NumStoreStrongs, "Number objc_storeStrong calls formed");

//===----------------------------------------------------------------------===//
//                                Declarations
//===----------------------------------------------------------------------===//

namespace {
  /// \brief Late ARC optimizations
  ///
  /// These change the IR in a way that makes it difficult to be analyzed by
  /// ObjCARCOpt, so it's run late.
  class ObjCARCContract : public FunctionPass {
    bool Changed;
    AliasAnalysis *AA;
    DominatorTree *DT;
    ProvenanceAnalysis PA;
    ARCRuntimeEntryPoints EP;

    /// A flag indicating whether this optimization pass should run.
    bool Run;

    /// The inline asm string to insert between calls and RetainRV calls to make
    /// the optimization work on targets which need it.
    const MDString *RetainRVMarker;

    /// The set of inserted objc_storeStrong calls. If at the end of walking the
    /// function we have found no alloca instructions, these calls can be marked
    /// "tail".
    SmallPtrSet<CallInst *, 8> StoreStrongCalls;

    /// Returns true if we eliminated Inst.
    bool tryToPeepholeInstruction(Function &F, Instruction *Inst,
                                  inst_iterator &Iter,
                                  SmallPtrSetImpl<Instruction *> &DepInsts,
                                  SmallPtrSetImpl<const BasicBlock *> &Visited,
                                  bool &TailOkForStoreStrong);

    bool optimizeRetainCall(Function &F, Instruction *Retain);

    bool
    contractAutorelease(Function &F, Instruction *Autorelease,
                        ARCInstKind Class,
                        SmallPtrSetImpl<Instruction *> &DependingInstructions,
                        SmallPtrSetImpl<const BasicBlock *> &Visited);

    void tryToContractReleaseIntoStoreStrong(Instruction *Release,
                                             inst_iterator &Iter);

    void getAnalysisUsage(AnalysisUsage &AU) const override;
    bool doInitialization(Module &M) override;
    bool runOnFunction(Function &F) override;

  public:
    static char ID;
    ObjCARCContract() : FunctionPass(ID) {
      initializeObjCARCContractPass(*PassRegistry::getPassRegistry());
    }
  };
}

//===----------------------------------------------------------------------===//
//                               Implementation
//===----------------------------------------------------------------------===//

/// Turn objc_retain into objc_retainAutoreleasedReturnValue if the operand is a
/// return value. We do this late so we do not disrupt the dataflow analysis in
/// ObjCARCOpt.
bool ObjCARCContract::optimizeRetainCall(Function &F, Instruction *Retain) {
  ImmutableCallSite CS(GetArgRCIdentityRoot(Retain));
  const Instruction *Call = CS.getInstruction();
  if (!Call)
    return false;
  if (Call->getParent() != Retain->getParent())
    return false;

  // Check that the call is next to the retain.
  BasicBlock::const_iterator I = Call;
  ++I;
  while (IsNoopInstruction(I)) ++I;
  if (&*I != Retain)
    return false;

  // Turn it to an objc_retainAutoreleasedReturnValue.
  Changed = true;
  ++NumPeeps;

  DEBUG(dbgs() << "Transforming objc_retain => "
                  "objc_retainAutoreleasedReturnValue since the operand is a "
                  "return value.\nOld: "<< *Retain << "\n");

  // We do not have to worry about tail calls/does not throw since
  // retain/retainRV have the same properties.
  Constant *Decl = EP.get(ARCRuntimeEntryPointKind::RetainRV);
  cast<CallInst>(Retain)->setCalledFunction(Decl);

  DEBUG(dbgs() << "New: " << *Retain << "\n");
  return true;
}

/// Merge an autorelease with a retain into a fused call.
bool ObjCARCContract::contractAutorelease(
    Function &F, Instruction *Autorelease, ARCInstKind Class,
    SmallPtrSetImpl<Instruction *> &DependingInstructions,
    SmallPtrSetImpl<const BasicBlock *> &Visited) {
  const Value *Arg = GetArgRCIdentityRoot(Autorelease);

  // Check that there are no instructions between the retain and the autorelease
  // (such as an autorelease_pop) which may change the count.
  CallInst *Retain = nullptr;
  if (Class == ARCInstKind::AutoreleaseRV)
    FindDependencies(RetainAutoreleaseRVDep, Arg,
                     Autorelease->getParent(), Autorelease,
                     DependingInstructions, Visited, PA);
  else
    FindDependencies(RetainAutoreleaseDep, Arg,
                     Autorelease->getParent(), Autorelease,
                     DependingInstructions, Visited, PA);

  Visited.clear();
  if (DependingInstructions.size() != 1) {
    DependingInstructions.clear();
    return false;
  }

  Retain = dyn_cast_or_null<CallInst>(*DependingInstructions.begin());
  DependingInstructions.clear();

  if (!Retain || GetBasicARCInstKind(Retain) != ARCInstKind::Retain ||
      GetArgRCIdentityRoot(Retain) != Arg)
    return false;

  Changed = true;
  ++NumPeeps;

  DEBUG(dbgs() << "    Fusing retain/autorelease!\n"
                  "        Autorelease:" << *Autorelease << "\n"
                  "        Retain: " << *Retain << "\n");

  Constant *Decl = EP.get(Class == ARCInstKind::AutoreleaseRV
                              ? ARCRuntimeEntryPointKind::RetainAutoreleaseRV
                              : ARCRuntimeEntryPointKind::RetainAutorelease);
  Retain->setCalledFunction(Decl);

  DEBUG(dbgs() << "        New RetainAutorelease: " << *Retain << "\n");

  EraseInstruction(Autorelease);
  return true;
}

static StoreInst *findSafeStoreForStoreStrongContraction(LoadInst *Load,
                                                         Instruction *Release,
                                                         ProvenanceAnalysis &PA,
                                                         AliasAnalysis *AA) {
  StoreInst *Store = nullptr;
  bool SawRelease = false;

  // Get the location associated with Load.
  MemoryLocation Loc = MemoryLocation::get(Load);

  // Walk down to find the store and the release, which may be in either order.
  for (auto I = std::next(BasicBlock::iterator(Load)),
            E = Load->getParent()->end();
       I != E; ++I) {
    // If we found the store we were looking for and saw the release,
    // break. There is no more work to be done.
    if (Store && SawRelease)
      break;

    // Now we know that we have not seen either the store or the release. If I
    // is the release, mark that we saw the release and continue.
    Instruction *Inst = &*I;
    if (Inst == Release) {
      SawRelease = true;
      continue;
    }

    // Otherwise, we check if Inst is a "good" store. Grab the instruction class
    // of Inst.
    ARCInstKind Class = GetBasicARCInstKind(Inst);

    // If Inst is an unrelated retain, we don't care about it.
    //
    // TODO: This is one area where the optimization could be made more
    // aggressive.
    if (IsRetain(Class))
      continue;

    // If we have seen the store, but not the release...
    if (Store) {
      // We need to make sure that it is safe to move the release from its
      // current position to the store. This implies proving that any
      // instruction in between Store and the Release conservatively can not use
      // the RCIdentityRoot of Release. If we can prove we can ignore Inst, so
      // continue...
      if (!CanUse(Inst, Load, PA, Class)) {
        continue;
      }

      // Otherwise, be conservative and return nullptr.
      return nullptr;
    }

    // Ok, now we know we have not seen a store yet. See if Inst can write to
    // our load location, if it can not, just ignore the instruction.
    if (!(AA->getModRefInfo(Inst, Loc) & AliasAnalysis::Mod))
      continue;

    Store = dyn_cast<StoreInst>(Inst);

    // If Inst can, then check if Inst is a simple store. If Inst is not a
    // store or a store that is not simple, then we have some we do not
    // understand writing to this memory implying we can not move the load
    // over the write to any subsequent store that we may find.
    if (!Store || !Store->isSimple())
      return nullptr;

    // Then make sure that the pointer we are storing to is Ptr. If so, we
    // found our Store!
    if (Store->getPointerOperand() == Loc.Ptr)
      continue;

    // Otherwise, we have an unknown store to some other ptr that clobbers
    // Loc.Ptr. Bail!
    return nullptr;
  }

  // If we did not find the store or did not see the release, fail.
  if (!Store || !SawRelease)
    return nullptr;

  // We succeeded!
  return Store;
}

static Instruction *
findRetainForStoreStrongContraction(Value *New, StoreInst *Store,
                                    Instruction *Release,
                                    ProvenanceAnalysis &PA) {
  // Walk up from the Store to find the retain.
  BasicBlock::iterator I = Store;
  BasicBlock::iterator Begin = Store->getParent()->begin();
  while (I != Begin && GetBasicARCInstKind(I) != ARCInstKind::Retain) {
    Instruction *Inst = &*I;

    // It is only safe to move the retain to the store if we can prove
    // conservatively that nothing besides the release can decrement reference
    // counts in between the retain and the store.
    if (CanDecrementRefCount(Inst, New, PA) && Inst != Release)
      return nullptr;
    --I;
  }
  Instruction *Retain = I;
  if (GetBasicARCInstKind(Retain) != ARCInstKind::Retain)
    return nullptr;
  if (GetArgRCIdentityRoot(Retain) != New)
    return nullptr;
  return Retain;
}

/// Attempt to merge an objc_release with a store, load, and objc_retain to form
/// an objc_storeStrong. An objc_storeStrong:
///
///   objc_storeStrong(i8** %old_ptr, i8* new_value)
///
/// is equivalent to the following IR sequence:
///
///   ; Load old value.
///   %old_value = load i8** %old_ptr               (1)
///
///   ; Increment the new value and then release the old value. This must occur
///   ; in order in case old_value releases new_value in its destructor causing
///   ; us to potentially have a dangling ptr.
///   tail call i8* @objc_retain(i8* %new_value)    (2)
///   tail call void @objc_release(i8* %old_value)  (3)
///
///   ; Store the new_value into old_ptr
///   store i8* %new_value, i8** %old_ptr           (4)
///
/// The safety of this optimization is based around the following
/// considerations:
///
///  1. We are forming the store strong at the store. Thus to perform this
///     optimization it must be safe to move the retain, load, and release to
///     (4).
///  2. We need to make sure that any re-orderings of (1), (2), (3), (4) are
///     safe.
void ObjCARCContract::tryToContractReleaseIntoStoreStrong(Instruction *Release,
                                                          inst_iterator &Iter) {
  // See if we are releasing something that we just loaded.
  auto *Load = dyn_cast<LoadInst>(GetArgRCIdentityRoot(Release));
  if (!Load || !Load->isSimple())
    return;

  // For now, require everything to be in one basic block.
  BasicBlock *BB = Release->getParent();
  if (Load->getParent() != BB)
    return;

  // First scan down the BB from Load, looking for a store of the RCIdentityRoot
  // of Load's
  StoreInst *Store =
      findSafeStoreForStoreStrongContraction(Load, Release, PA, AA);
  // If we fail, bail.
  if (!Store)
    return;

  // Then find what new_value's RCIdentity Root is.
  Value *New = GetRCIdentityRoot(Store->getValueOperand());

  // Then walk up the BB and look for a retain on New without any intervening
  // instructions which conservatively might decrement ref counts.
  Instruction *Retain =
      findRetainForStoreStrongContraction(New, Store, Release, PA);

  // If we fail, bail.
  if (!Retain)
    return;

  Changed = true;
  ++NumStoreStrongs;

  DEBUG(
      llvm::dbgs() << "    Contracting retain, release into objc_storeStrong.\n"
                   << "        Old:\n"
                   << "            Store:   " << *Store << "\n"
                   << "            Release: " << *Release << "\n"
                   << "            Retain:  " << *Retain << "\n"
                   << "            Load:    " << *Load << "\n");

  LLVMContext &C = Release->getContext();
  Type *I8X = PointerType::getUnqual(Type::getInt8Ty(C));
  Type *I8XX = PointerType::getUnqual(I8X);

  Value *Args[] = { Load->getPointerOperand(), New };
  if (Args[0]->getType() != I8XX)
    Args[0] = new BitCastInst(Args[0], I8XX, "", Store);
  if (Args[1]->getType() != I8X)
    Args[1] = new BitCastInst(Args[1], I8X, "", Store);
  Constant *Decl = EP.get(ARCRuntimeEntryPointKind::StoreStrong);
  CallInst *StoreStrong = CallInst::Create(Decl, Args, "", Store);
  StoreStrong->setDoesNotThrow();
  StoreStrong->setDebugLoc(Store->getDebugLoc());

  // We can't set the tail flag yet, because we haven't yet determined
  // whether there are any escaping allocas. Remember this call, so that
  // we can set the tail flag once we know it's safe.
  StoreStrongCalls.insert(StoreStrong);

  DEBUG(llvm::dbgs() << "        New Store Strong: " << *StoreStrong << "\n");

  if (&*Iter == Store) ++Iter;
  Store->eraseFromParent();
  Release->eraseFromParent();
  EraseInstruction(Retain);
  if (Load->use_empty())
    Load->eraseFromParent();
}

bool ObjCARCContract::tryToPeepholeInstruction(
  Function &F, Instruction *Inst, inst_iterator &Iter,
  SmallPtrSetImpl<Instruction *> &DependingInsts,
  SmallPtrSetImpl<const BasicBlock *> &Visited,
  bool &TailOkForStoreStrongs) {
    // Only these library routines return their argument. In particular,
    // objc_retainBlock does not necessarily return its argument.
  ARCInstKind Class = GetBasicARCInstKind(Inst);
    switch (Class) {
    case ARCInstKind::FusedRetainAutorelease:
    case ARCInstKind::FusedRetainAutoreleaseRV:
      return false;
    case ARCInstKind::Autorelease:
    case ARCInstKind::AutoreleaseRV:
      return contractAutorelease(F, Inst, Class, DependingInsts, Visited);
    case ARCInstKind::Retain:
      // Attempt to convert retains to retainrvs if they are next to function
      // calls.
      if (!optimizeRetainCall(F, Inst))
        return false;
      // If we succeed in our optimization, fall through.
      // FALLTHROUGH
    case ARCInstKind::RetainRV: {
      // If we're compiling for a target which needs a special inline-asm
      // marker to do the retainAutoreleasedReturnValue optimization,
      // insert it now.
      if (!RetainRVMarker)
        return false;
      BasicBlock::iterator BBI = Inst;
      BasicBlock *InstParent = Inst->getParent();

      // Step up to see if the call immediately precedes the RetainRV call.
      // If it's an invoke, we have to cross a block boundary. And we have
      // to carefully dodge no-op instructions.
      do {
        if (&*BBI == InstParent->begin()) {
          BasicBlock *Pred = InstParent->getSinglePredecessor();
          if (!Pred)
            goto decline_rv_optimization;
          BBI = Pred->getTerminator();
          break;
        }
        --BBI;
      } while (IsNoopInstruction(BBI));

      if (&*BBI == GetArgRCIdentityRoot(Inst)) {
        DEBUG(dbgs() << "Adding inline asm marker for "
                        "retainAutoreleasedReturnValue optimization.\n");
        Changed = true;
        InlineAsm *IA =
          InlineAsm::get(FunctionType::get(Type::getVoidTy(Inst->getContext()),
                                           /*isVarArg=*/false),
                         RetainRVMarker->getString(),
                         /*Constraints=*/"", /*hasSideEffects=*/true);
        CallInst::Create(IA, "", Inst);
      }
    decline_rv_optimization:
      return false;
    }
    case ARCInstKind::InitWeak: {
      // objc_initWeak(p, null) => *p = null
      CallInst *CI = cast<CallInst>(Inst);
      if (IsNullOrUndef(CI->getArgOperand(1))) {
        Value *Null =
          ConstantPointerNull::get(cast<PointerType>(CI->getType()));
        Changed = true;
        new StoreInst(Null, CI->getArgOperand(0), CI);

        DEBUG(dbgs() << "OBJCARCContract: Old = " << *CI << "\n"
                     << "                 New = " << *Null << "\n");

        CI->replaceAllUsesWith(Null);
        CI->eraseFromParent();
      }
      return true;
    }
    case ARCInstKind::Release:
      // Try to form an objc store strong from our release. If we fail, there is
      // nothing further to do below, so continue.
      tryToContractReleaseIntoStoreStrong(Inst, Iter);
      return true;
    case ARCInstKind::User:
      // Be conservative if the function has any alloca instructions.
      // Technically we only care about escaping alloca instructions,
      // but this is sufficient to handle some interesting cases.
      if (isa<AllocaInst>(Inst))
        TailOkForStoreStrongs = false;
      return true;
    case ARCInstKind::IntrinsicUser:
      // Remove calls to @clang.arc.use(...).
      Inst->eraseFromParent();
      return true;
    default:
      return true;
    }
}

//===----------------------------------------------------------------------===//
//                              Top Level Driver
//===----------------------------------------------------------------------===//

bool ObjCARCContract::runOnFunction(Function &F) {
  if (!EnableARCOpts)
    return false;

  // If nothing in the Module uses ARC, don't do anything.
  if (!Run)
    return false;

  Changed = false;
  AA = &getAnalysis<AliasAnalysis>();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  PA.setAA(&getAnalysis<AliasAnalysis>());

  DEBUG(llvm::dbgs() << "**** ObjCARC Contract ****\n");

  // Track whether it's ok to mark objc_storeStrong calls with the "tail"
  // keyword. Be conservative if the function has variadic arguments.
  // It seems that functions which "return twice" are also unsafe for the
  // "tail" argument, because they are setjmp, which could need to
  // return to an earlier stack state.
  bool TailOkForStoreStrongs =
      !F.isVarArg() && !F.callsFunctionThatReturnsTwice();

  // For ObjC library calls which return their argument, replace uses of the
  // argument with uses of the call return value, if it dominates the use. This
  // reduces register pressure.
  SmallPtrSet<Instruction *, 4> DependingInstructions;
  SmallPtrSet<const BasicBlock *, 4> Visited;
  for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E;) {
    Instruction *Inst = &*I++;

    DEBUG(dbgs() << "Visiting: " << *Inst << "\n");

    // First try to peephole Inst. If there is nothing further we can do in
    // terms of undoing objc-arc-expand, process the next inst.
    if (tryToPeepholeInstruction(F, Inst, I, DependingInstructions, Visited,
                                 TailOkForStoreStrongs))
      continue;

    // Otherwise, try to undo objc-arc-expand.

    // Don't use GetArgRCIdentityRoot because we don't want to look through bitcasts
    // and such; to do the replacement, the argument must have type i8*.
    Value *Arg = cast<CallInst>(Inst)->getArgOperand(0);

    // TODO: Change this to a do-while.
    for (;;) {
      // If we're compiling bugpointed code, don't get in trouble.
      if (!isa<Instruction>(Arg) && !isa<Argument>(Arg))
        break;
      // Look through the uses of the pointer.
      for (Value::use_iterator UI = Arg->use_begin(), UE = Arg->use_end();
           UI != UE; ) {
        // Increment UI now, because we may unlink its element.
        Use &U = *UI++;
        unsigned OperandNo = U.getOperandNo();

        // If the call's return value dominates a use of the call's argument
        // value, rewrite the use to use the return value. We check for
        // reachability here because an unreachable call is considered to
        // trivially dominate itself, which would lead us to rewriting its
        // argument in terms of its return value, which would lead to
        // infinite loops in GetArgRCIdentityRoot.
        if (DT->isReachableFromEntry(U) && DT->dominates(Inst, U)) {
          Changed = true;
          Instruction *Replacement = Inst;
          Type *UseTy = U.get()->getType();
          if (PHINode *PHI = dyn_cast<PHINode>(U.getUser())) {
            // For PHI nodes, insert the bitcast in the predecessor block.
            unsigned ValNo = PHINode::getIncomingValueNumForOperand(OperandNo);
            BasicBlock *BB = PHI->getIncomingBlock(ValNo);
            if (Replacement->getType() != UseTy)
              Replacement = new BitCastInst(Replacement, UseTy, "",
                                            &BB->back());
            // While we're here, rewrite all edges for this PHI, rather
            // than just one use at a time, to minimize the number of
            // bitcasts we emit.
            for (unsigned i = 0, e = PHI->getNumIncomingValues(); i != e; ++i)
              if (PHI->getIncomingBlock(i) == BB) {
                // Keep the UI iterator valid.
                if (UI != UE &&
                    &PHI->getOperandUse(
                        PHINode::getOperandNumForIncomingValue(i)) == &*UI)
                  ++UI;
                PHI->setIncomingValue(i, Replacement);
              }
          } else {
            if (Replacement->getType() != UseTy)
              Replacement = new BitCastInst(Replacement, UseTy, "",
                                            cast<Instruction>(U.getUser()));
            U.set(Replacement);
          }
        }
      }

      // If Arg is a no-op casted pointer, strip one level of casts and iterate.
      if (const BitCastInst *BI = dyn_cast<BitCastInst>(Arg))
        Arg = BI->getOperand(0);
      else if (isa<GEPOperator>(Arg) &&
               cast<GEPOperator>(Arg)->hasAllZeroIndices())
        Arg = cast<GEPOperator>(Arg)->getPointerOperand();
      else if (isa<GlobalAlias>(Arg) &&
               !cast<GlobalAlias>(Arg)->mayBeOverridden())
        Arg = cast<GlobalAlias>(Arg)->getAliasee();
      else
        break;
    }
  }

  // If this function has no escaping allocas or suspicious vararg usage,
  // objc_storeStrong calls can be marked with the "tail" keyword.
  if (TailOkForStoreStrongs)
    for (CallInst *CI : StoreStrongCalls)
      CI->setTailCall();
  StoreStrongCalls.clear();

  return Changed;
}

//===----------------------------------------------------------------------===//
//                             Misc Pass Manager
//===----------------------------------------------------------------------===//

char ObjCARCContract::ID = 0;
INITIALIZE_PASS_BEGIN(ObjCARCContract, "objc-arc-contract",
                      "ObjC ARC contraction", false, false)
INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(ObjCARCContract, "objc-arc-contract",
                    "ObjC ARC contraction", false, false)

void ObjCARCContract::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<AliasAnalysis>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.setPreservesCFG();
}

Pass *llvm::createObjCARCContractPass() { return new ObjCARCContract(); }

bool ObjCARCContract::doInitialization(Module &M) {
  // If nothing in the Module uses ARC, don't do anything.
  Run = ModuleHasARC(M);
  if (!Run)
    return false;

  EP.init(&M);

  // Initialize RetainRVMarker.
  RetainRVMarker = nullptr;
  if (NamedMDNode *NMD =
          M.getNamedMetadata("clang.arc.retainAutoreleasedReturnValueMarker"))
    if (NMD->getNumOperands() == 1) {
      const MDNode *N = NMD->getOperand(0);
      if (N->getNumOperands() == 1)
        if (const MDString *S = dyn_cast<MDString>(N->getOperand(0)))
          RetainRVMarker = S;
    }

  return false;
}
