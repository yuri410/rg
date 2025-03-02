//===- DxilLoopUnroll.cpp - Special Unroll for Constant Values ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//
// Special loop unroll routine for creating mandatory constant values and
// loops that have exits.
//
// Overview of algorithm:
// 
// 1. Identify a set of blocks to unroll.
//
//    LLVM's concept of loop excludes exit blocks, which are blocks that no
//    longer have a path to the loop latch. However, some exit blocks in HLSL
//    also need to be unrolled. For example:
//
//        [unroll]
//        for (uint i = 0; i < 4; i++)
//        {
//          if (...)
//          {
//            // This block here is an exit block, since it's.
//            // guaranteed to exit the loop.
//            ...
//            a[i] = ...; // Indexing requires unroll.
//            return;
//          }
//        }
//
//
// 2. Create LCSSA based on the new loop boundary.
//
//    See LCSSA.cpp for more details. It creates trivial PHI nodes for any
//    outgoing values of the loop at the exit blocks, so when the loop body
//    gets cloned, the outgoing values can be added to those PHI nodes easily.
//
//    We are using a modified LCSSA routine here because we are including some
//    of the original exit blocks in the unroll.
//
//
// 3. Unroll the loop until we succeed.
//
//    Unlike LLVM, we do not try to find a loop count before unrolling.
//    Instead, we unroll to find a constant terminal condition. Give up when we
//    fail to do so.
//
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/PredIteratorCache.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "dxc/DXIL/DxilUtil.h"
#include "dxc/HLSL/HLModule.h"
#include "llvm/Analysis/DxilValueCache.h"

using namespace llvm;
using namespace hlsl;

// Copied over from LoopUnroll.cpp - RemapInstruction()
static inline void RemapInstruction(Instruction *I,
                                    ValueToValueMapTy &VMap) {
  for (unsigned op = 0, E = I->getNumOperands(); op != E; ++op) {
    Value *Op = I->getOperand(op);
    ValueToValueMapTy::iterator It = VMap.find(Op);
    if (It != VMap.end())
      I->setOperand(op, It->second);
  }

  if (PHINode *PN = dyn_cast<PHINode>(I)) {
    for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
      ValueToValueMapTy::iterator It = VMap.find(PN->getIncomingBlock(i));
      if (It != VMap.end())
        PN->setIncomingBlock(i, cast<BasicBlock>(It->second));
    }
  }
}


namespace {

class DxilLoopUnroll : public LoopPass {
public:
  static char ID;

  std::unordered_set<Function *> CleanedUpAlloca;
  const unsigned MaxIterationAttempt;

  DxilLoopUnroll(unsigned MaxIterationAttempt = 1024) :
    LoopPass(ID),
    MaxIterationAttempt(MaxIterationAttempt)
  {
    initializeDxilLoopUnrollPass(*PassRegistry::getPassRegistry());
  }
  const char *getPassName() const override { return "Dxil Loop Unroll"; }
  bool runOnLoop(Loop *L, LPPassManager &LPM) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<DxilValueCache>();
    AU.addRequiredID(LoopSimplifyID);
  }
};

char DxilLoopUnroll::ID;

static void FailLoopUnroll(bool WarnOnly, LLVMContext &Ctx, DebugLoc DL, const Twine &Message) {
  if (WarnOnly) {
    if (DL)
      Ctx.emitWarning(hlsl::dxilutil::FormatMessageAtLocation(DL, Message));
    else
      Ctx.emitWarning(hlsl::dxilutil::FormatMessageWithoutLocation(Message));
  }
  else {
    if (DL)
      Ctx.emitError(hlsl::dxilutil::FormatMessageAtLocation(DL, Message));
    else
      Ctx.emitError(hlsl::dxilutil::FormatMessageWithoutLocation(Message));
  }
}

struct LoopIteration {
  SmallVector<BasicBlock *, 16> Body;
  BasicBlock *Latch = nullptr;
  BasicBlock *Header = nullptr;
  ValueToValueMapTy VarMap;
  SetVector<BasicBlock *> Extended; // Blocks that are included in the clone that are not in the core loop body.
  LoopIteration() {}
};

static bool GetConstantI1(Value *V, bool *Val=nullptr) {
  if (ConstantInt *C = dyn_cast<ConstantInt>(V)) {
    if (V->getType()->isIntegerTy(1)) {
      if (Val)
        *Val = (bool)C->getLimitedValue();
      return true;
    }
  }
  return false;
}

static bool IsMarkedFullUnroll(Loop *L) {
  if (MDNode *LoopID = L->getLoopID())
    return GetUnrollMetadata(LoopID, "llvm.loop.unroll.full");
  return false;
}

static bool IsMarkedUnrollCount(Loop *L, int *OutCount) {
  if (MDNode *LoopID = L->getLoopID()) {
    if (MDNode *MD = GetUnrollMetadata(LoopID, "llvm.loop.unroll.count")) {
      assert(MD->getNumOperands() == 2 &&
             "Unroll count hint metadata should have two operands.");
      ConstantInt *Val = mdconst::extract<ConstantInt>(MD->getOperand(1));
      int Count = Val->getZExtValue();
      *OutCount = Count;
      return true;
    }
  }
  return false;
}

static bool HasSuccessorsInLoop(BasicBlock *BB, Loop *L) {
  for (BasicBlock *Succ : successors(BB)) {
    if (L->contains(Succ)) {
      return true;
    }
  }
  return false;
}

static void DetachFromSuccessors(BasicBlock *BB) {
  SmallVector<BasicBlock *, 16> Successors(succ_begin(BB), succ_end(BB));
  for (BasicBlock *Succ : Successors) {
    Succ->removePredecessor(BB);
  }
}

/// Return true if the specified block is in the list.
static bool isExitBlock(BasicBlock *BB,
                        const SmallVectorImpl<BasicBlock *> &ExitBlocks) {
  for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i)
    if (ExitBlocks[i] == BB)
      return true;
  return false;
}

// Copied and modified from LCSSA.cpp
static bool processInstruction(SetVector<BasicBlock *> &Body, Loop &L, Instruction &Inst, DominatorTree &DT, // HLSL Change
                               const SmallVectorImpl<BasicBlock *> &ExitBlocks,
                               PredIteratorCache &PredCache, LoopInfo *LI) {

  SmallVector<Use *, 16> UsesToRewrite;

  BasicBlock *InstBB = Inst.getParent();

  for (Use &U : Inst.uses()) {
    Instruction *User = cast<Instruction>(U.getUser());
    BasicBlock *UserBB = User->getParent();
    if (PHINode *PN = dyn_cast<PHINode>(User))
      UserBB = PN->getIncomingBlock(U);

    if (InstBB != UserBB && /*!L.contains(UserBB)*/!Body.count(UserBB)) // HLSL Change
      UsesToRewrite.push_back(&U);
  }

  // If there are no uses outside the loop, exit with no change.
  if (UsesToRewrite.empty())
    return false;
#if 0 // HLSL Change
  ++NumLCSSA; // We are applying the transformation
#endif // HLSL Change
  // Invoke instructions are special in that their result value is not available
  // along their unwind edge. The code below tests to see whether DomBB
  // dominates
  // the value, so adjust DomBB to the normal destination block, which is
  // effectively where the value is first usable.
  BasicBlock *DomBB = Inst.getParent();
  if (InvokeInst *Inv = dyn_cast<InvokeInst>(&Inst))
    DomBB = Inv->getNormalDest();

  DomTreeNode *DomNode = DT.getNode(DomBB);

  SmallVector<PHINode *, 16> AddedPHIs;
  SmallVector<PHINode *, 8> PostProcessPHIs;

  SSAUpdater SSAUpdate;
  SSAUpdate.Initialize(Inst.getType(), Inst.getName());

  // Insert the LCSSA phi's into all of the exit blocks dominated by the
  // value, and add them to the Phi's map.
  for (SmallVectorImpl<BasicBlock *>::const_iterator BBI = ExitBlocks.begin(),
                                                     BBE = ExitBlocks.end();
       BBI != BBE; ++BBI) {
    BasicBlock *ExitBB = *BBI;
    if (!DT.dominates(DomNode, DT.getNode(ExitBB)))
      continue;

    // If we already inserted something for this BB, don't reprocess it.
    if (SSAUpdate.HasValueForBlock(ExitBB))
      continue;

    PHINode *PN = PHINode::Create(Inst.getType(), PredCache.size(ExitBB),
                                  Inst.getName() + ".lcssa", ExitBB->begin());

    // Add inputs from inside the loop for this PHI.
    for (BasicBlock *Pred : PredCache.get(ExitBB)) {
      PN->addIncoming(&Inst, Pred);

      // If the exit block has a predecessor not within the loop, arrange for
      // the incoming value use corresponding to that predecessor to be
      // rewritten in terms of a different LCSSA PHI.
      if (/*!L.contains(Pred)*/ !Body.count(Pred)) // HLSL Change
        UsesToRewrite.push_back(
            &PN->getOperandUse(PN->getOperandNumForIncomingValue(
                 PN->getNumIncomingValues() - 1)));
    }

    AddedPHIs.push_back(PN);

    // Remember that this phi makes the value alive in this block.
    SSAUpdate.AddAvailableValue(ExitBB, PN);

    // LoopSimplify might fail to simplify some loops (e.g. when indirect
    // branches are involved). In such situations, it might happen that an exit
    // for Loop L1 is the header of a disjoint Loop L2. Thus, when we create
    // PHIs in such an exit block, we are also inserting PHIs into L2's header.
    // This could break LCSSA form for L2 because these inserted PHIs can also
    // have uses outside of L2. Remember all PHIs in such situation as to
    // revisit than later on. FIXME: Remove this if indirectbr support into
    // LoopSimplify gets improved.
    if (auto *OtherLoop = LI->getLoopFor(ExitBB))
      if (!L.contains(OtherLoop))
        PostProcessPHIs.push_back(PN);
  }

  // Rewrite all uses outside the loop in terms of the new PHIs we just
  // inserted.
  for (unsigned i = 0, e = UsesToRewrite.size(); i != e; ++i) {
    // If this use is in an exit block, rewrite to use the newly inserted PHI.
    // This is required for correctness because SSAUpdate doesn't handle uses in
    // the same block.  It assumes the PHI we inserted is at the end of the
    // block.
    Instruction *User = cast<Instruction>(UsesToRewrite[i]->getUser());
    BasicBlock *UserBB = User->getParent();
    if (PHINode *PN = dyn_cast<PHINode>(User))
      UserBB = PN->getIncomingBlock(*UsesToRewrite[i]);

    if (isa<PHINode>(UserBB->begin()) && isExitBlock(UserBB, ExitBlocks)) {
      // Tell the VHs that the uses changed. This updates SCEV's caches.
      if (UsesToRewrite[i]->get()->hasValueHandle())
        ValueHandleBase::ValueIsRAUWd(*UsesToRewrite[i], UserBB->begin());
      UsesToRewrite[i]->set(UserBB->begin());
      continue;
    }

    // Otherwise, do full PHI insertion.
    SSAUpdate.RewriteUse(*UsesToRewrite[i]);
  }

  // Post process PHI instructions that were inserted into another disjoint loop
  // and update their exits properly.
  for (auto *I : PostProcessPHIs) {
    if (I->use_empty())
      continue;

    BasicBlock *PHIBB = I->getParent();
    Loop *OtherLoop = LI->getLoopFor(PHIBB);
    SmallVector<BasicBlock *, 8> EBs;
    OtherLoop->getExitBlocks(EBs);
    if (EBs.empty())
      continue;

    // Recurse and re-process each PHI instruction. FIXME: we should really
    // convert this entire thing to a worklist approach where we process a
    // vector of instructions...
    SetVector<BasicBlock *> OtherLoopBody(OtherLoop->block_begin(), OtherLoop->block_end()); // HLSL Change
    processInstruction(OtherLoopBody, *OtherLoop, *I, DT, EBs, PredCache, LI);
  }

  // Remove PHI nodes that did not have any uses rewritten.
  for (unsigned i = 0, e = AddedPHIs.size(); i != e; ++i) {
    if (AddedPHIs[i]->use_empty())
      AddedPHIs[i]->eraseFromParent();
  }

  return true;

}

// Copied from LCSSA.cpp
static bool blockDominatesAnExit(BasicBlock *BB,
                     DominatorTree &DT,
                     const SmallVectorImpl<BasicBlock *> &ExitBlocks) {
  DomTreeNode *DomNode = DT.getNode(BB);
  for (BasicBlock *Exit : ExitBlocks)
    if (DT.dominates(DomNode, DT.getNode(Exit)))
      return true;
  return false;
};

// Copied from LCSSA.cpp
//
// We need to recreate the LCSSA form since our loop boundary is potentially different from
// the canonical one.
static bool CreateLCSSA(SetVector<BasicBlock *> &Body, const SmallVectorImpl<BasicBlock *> &ExitBlocks, Loop *L, DominatorTree &DT, LoopInfo *LI) {

  PredIteratorCache PredCache;
  bool Changed = false;
  // Look at all the instructions in the loop, checking to see if they have uses
  // outside the loop.  If so, rewrite those uses.
  for (SetVector<BasicBlock *>::iterator BBI = Body.begin(), BBE = Body.end();
       BBI != BBE; ++BBI) {
    BasicBlock *BB = *BBI;

    // For large loops, avoid use-scanning by using dominance information:  In
    // particular, if a block does not dominate any of the loop exits, then none
    // of the values defined in the block could be used outside the loop.
    if (!blockDominatesAnExit(BB, DT, ExitBlocks))
      continue;

    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
      // Reject two common cases fast: instructions with no uses (like stores)
      // and instructions with one use that is in the same block as this.
      if (I->use_empty() ||
          (I->hasOneUse() && I->user_back()->getParent() == BB &&
           !isa<PHINode>(I->user_back())))
        continue;

      Changed |= processInstruction(Body, *L, *I, DT, ExitBlocks, PredCache, LI);
    }
  }

  return Changed;
}

static Value *GetGEPPtrOrigin(GEPOperator *GEP) {
  Value *Ptr = GEP->getPointerOperand();
  while (Ptr) {
    if (AllocaInst *AI = dyn_cast<AllocaInst>(Ptr)) {
      return AI;
    }
    else if (GEPOperator *NewGEP = dyn_cast<GEPOperator>(Ptr)) {
      Ptr = NewGEP->getPointerOperand();
    }
    else if (GlobalVariable *GV = dyn_cast<GlobalVariable>(Ptr)) {
      return GV;
    }
    else {
      break;
    }
  }
  return nullptr;
}

// Find all blocks in the loop with instructions that
// would require an unroll to be correct.
//
// For example:
// for (int i = 0; i < 10; i++) {
//   gep i
// }
//
static void FindProblemBlocks(BasicBlock *Header, const SmallVectorImpl<BasicBlock *> &BlocksInLoop, std::unordered_set<BasicBlock *> &ProblemBlocks, SetVector<AllocaInst *> &ProblemAllocas) {
  SmallVector<Instruction *, 16> WorkList;

  std::unordered_set<BasicBlock *> BlocksInLoopSet(BlocksInLoop.begin(), BlocksInLoop.end());
  std::unordered_set<Instruction *> InstructionsSeen;

  for (Instruction &I : *Header) {
    PHINode *PN = dyn_cast<PHINode>(&I);
    if (!PN)
      break;
    WorkList.push_back(PN);
    InstructionsSeen.insert(PN);
  }

  while (WorkList.size()) {
    Instruction *I = WorkList.pop_back_val();

    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I)) {
      Type *EltType = GEP->getType()->getPointerElementType();

      // NOTE: This is a very convservative in the following conditions:
      // - constant global resource arrays with external linkage (these can be
      //   dynamically accessed)
      // - global resource arrays or alloca resource arrays, as long as all
      //   writes come from the same original resource definition (which can
      //   also be an array).
      //
      // We may want to make this more precise in the future if it becomes a
      // problem.
      //
      if (hlsl::dxilutil::IsHLSLObjectType(EltType)) {
        if (Value *Ptr = GetGEPPtrOrigin(cast<GEPOperator>(GEP))) {
          if (GlobalVariable *GV = dyn_cast<GlobalVariable>(Ptr)) {
            if (!GV->isExternalLinkage(llvm::GlobalValue::ExternalLinkage))
              ProblemBlocks.insert(GEP->getParent());
          }
          else if (AllocaInst *AI = dyn_cast<AllocaInst>(Ptr)) {
            ProblemAllocas.insert(AI);
            ProblemBlocks.insert(GEP->getParent());
          }
        }
        continue; // Stop Propagating
      }
    }

    for (User *U : I->users()) {
      if (Instruction *UserI = dyn_cast<Instruction>(U)) {
        if (!InstructionsSeen.count(UserI) &&
          BlocksInLoopSet.count(UserI->getParent()))
        {
          InstructionsSeen.insert(UserI);
          WorkList.push_back(UserI);
        }
      }
    }
  }
}

// Helper function for getting GEP's const index value
inline static int64_t GetGEPIndex(GEPOperator *GEP, unsigned idx) {
  return cast<ConstantInt>(GEP->getOperand(idx + 1))->getSExtValue();
} 

// Replace allocas with all constant indices with scalar allocas, then promote
// them to values where possible (mem2reg).
//
// Before loop unroll, we did not have constant indices for arrays and SROA was
// unable to break them into scalars. Now that unroll has potentially given
// them constant values, we need to turn them into scalars.
//
// if "AllowOOBIndex" is true, it turns any out of bound index into 0.
// Otherwise it emits an error and fails compilation.
//
template<typename IteratorT>
static bool BreakUpArrayAllocas(bool AllowOOBIndex, IteratorT ItBegin, IteratorT ItEnd, DominatorTree *DT, AssumptionCache *AC, DxilValueCache *DVC) { 
  bool Success = true;

  SmallVector<AllocaInst *, 8> WorkList(ItBegin, ItEnd);

  SmallVector<GEPOperator *, 16> GEPs;
  while (WorkList.size()) {
    AllocaInst *AI = WorkList.pop_back_val();

    Type *AllocaType = AI->getAllocatedType();

    // Only deal with array allocas.
    if (!AllocaType->isArrayTy())
      continue;

    unsigned ArraySize = AI->getAllocatedType()->getArrayNumElements();
    Type *ElementType = AllocaType->getArrayElementType();
    if (!ArraySize)
      continue;

    GEPs.clear(); // Re-use array
    for (User *U : AI->users()) {
      // Try to set all GEP operands to constant
      if (GEPOperator *GEP = dyn_cast<GEPOperator>(U)) {
        if (!GEP->hasAllConstantIndices() && isa<GetElementPtrInst>(GEP)) {
          for (unsigned i = 0; i < GEP->getNumIndices(); i++) {
            Value *IndexOp = GEP->getOperand(i + 1);
            if (isa<Constant>(IndexOp))
              continue;

            if (Constant *C = DVC->GetConstValue(IndexOp))
              GEP->setOperand(i + 1, C);
          }
        }

        if (!GEP->hasAllConstantIndices() || GEP->getNumIndices() < 2 ||
          GetGEPIndex(GEP, 0) != 0)
        {
          GEPs.clear();
          break;
        }
        else {
          GEPs.push_back(GEP);
        }
      }
      else {
        GEPs.clear();
        break;
      }
    }

    if (!GEPs.size())
      continue;

    SmallVector<AllocaInst *, 8> ScalarAllocas;
    ScalarAllocas.resize(ArraySize);

    IRBuilder<> B(AI);
    for (GEPOperator *GEP : GEPs) {
      int64_t idx = GetGEPIndex(GEP, 1);
      GetElementPtrInst *GEPInst = dyn_cast<GetElementPtrInst>(GEP);

      if (idx < 0 || idx >= ArraySize) {
        if (AllowOOBIndex)
          idx = 0;
        else {
          Success = false;
          if (GEPInst)
            hlsl::dxilutil::EmitErrorOnInstruction(GEPInst, "Array access out of bound.");
          continue;
        }
      } 
      AllocaInst *ScalarAlloca = ScalarAllocas[idx];
      if (!ScalarAlloca) {
        ScalarAlloca = B.CreateAlloca(ElementType);
        ScalarAllocas[idx] = ScalarAlloca;
        if (ElementType->isArrayTy()) {
          WorkList.push_back(ScalarAlloca);
        }
      }
      Value *NewPointer = nullptr;
      if (ElementType->isArrayTy()) {
        SmallVector<Value *, 2> Indices;
        Indices.push_back(B.getInt32(0));
        for (unsigned i = 2; i < GEP->getNumIndices(); i++) {
          Indices.push_back(GEP->getOperand(i + 1));
        }
        NewPointer = B.CreateGEP(ScalarAlloca, Indices);
      } else {
        NewPointer = ScalarAlloca;
      }

      GEP->replaceAllUsesWith(NewPointer);
    } 

    if (!ElementType->isArrayTy()) {
      ScalarAllocas.erase(std::remove(ScalarAllocas.begin(), ScalarAllocas.end(), nullptr), ScalarAllocas.end());
      PromoteMemToReg(ScalarAllocas, *DT, nullptr, AC);
    }
  }

  return Success;
}

static bool ContainsFloatingPointType(Type *Ty) {
  if (Ty->isFloatingPointTy()) {
    return true;
  }
  else if (Ty->isArrayTy()) {
    return ContainsFloatingPointType(Ty->getArrayElementType());
  }
  else if (Ty->isVectorTy()) {
    return ContainsFloatingPointType(Ty->getVectorElementType());
  }
  else if (Ty->isStructTy()) {
    for (unsigned i = 0, NumStructElms = Ty->getStructNumElements(); i < NumStructElms; i++) {
      if (ContainsFloatingPointType(Ty->getStructElementType(i)))
        return true;
    }
  }
  return false;
}

static bool Mem2Reg(Function &F, DominatorTree &DT, AssumptionCache &AC) {
  BasicBlock &BB = F.getEntryBlock();  // Get the entry node for the function
  bool Changed  = false;
  std::vector<AllocaInst*> Allocas;
  while (1) {
    Allocas.clear();

    // Find allocas that are safe to promote, by looking at all instructions in
    // the entry node
    for (BasicBlock::iterator I = BB.begin(), E = --BB.end(); I != E; ++I)
      if (AllocaInst *AI = dyn_cast<AllocaInst>(I))       // Is it an alloca?
        if (isAllocaPromotable(AI) &&
          (!HLModule::HasPreciseAttributeWithMetadata(AI) || !ContainsFloatingPointType(AI->getAllocatedType())))
          Allocas.push_back(AI);

    if (Allocas.empty()) break;

    PromoteMemToReg(Allocas, DT, nullptr, &AC);
    Changed = true;
  }

  return Changed;
}

static void RecursivelyRemoveLoopFromQueue(LPPassManager &LPM, Loop *L) {
  // Copy the sub loops into a separate list because
  // the original list may change.
  SmallVector<Loop *, 4> SubLoops(L->getSubLoops().begin(), L->getSubLoops().end());

  // Must remove all child loops first.
  for (Loop *SubL : SubLoops) {
    RecursivelyRemoveLoopFromQueue(LPM, SubL);
  }

  LPM.deleteLoopFromQueue(L);
}

bool DxilLoopUnroll::runOnLoop(Loop *L, LPPassManager &LPM) {

  DebugLoc LoopLoc = L->getStartLoc(); // Debug location for the start of the loop.
  Function *F = L->getHeader()->getParent();
  ScalarEvolution *SE = &getAnalysis<ScalarEvolution>();
  DxilValueCache *DVC = &getAnalysis<DxilValueCache>();

  bool HasExplicitLoopCount = false;
  int ExplicitUnrollCountSigned = 0;

  // If the loop is not marked as [unroll], don't do anything.
  if (IsMarkedUnrollCount(L, &ExplicitUnrollCountSigned)) {
    HasExplicitLoopCount = true;
  }
  else if (!IsMarkedFullUnroll(L)) {
    return false;
  }

  unsigned ExplicitUnrollCount = 0;
  if (HasExplicitLoopCount) {
    if (ExplicitUnrollCountSigned < 1) {
      FailLoopUnroll(false, F->getContext(), LoopLoc, "Could not unroll loop. Invalid unroll count.");
      return false;
    }
    ExplicitUnrollCount = (unsigned)ExplicitUnrollCountSigned;
  }

  if (!L->isSafeToClone())
    return false;

  bool FxcCompatMode = false;
  if (F->getParent()->HasHLModule()) {
    HLModule &HM = F->getParent()->GetHLModule();
    FxcCompatMode = HM.GetHLOptions().bFXCCompatMode;
  }

  unsigned TripCount = 0;

  BasicBlock *ExitingBlock = L->getLoopLatch();
  if (!ExitingBlock || !L->isLoopExiting(ExitingBlock))
    ExitingBlock = L->getExitingBlock();

  if (ExitingBlock) {
    TripCount = SE->getSmallConstantTripCount(L, ExitingBlock);
  }

  // Analysis passes
  DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  AssumptionCache *AC =
    &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(*F);
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  Loop *OuterL = L->getParentLoop();
  BasicBlock *Latch = L->getLoopLatch();
  BasicBlock *Header = L->getHeader();
  BasicBlock *Predecessor = L->getLoopPredecessor();
  const DataLayout &DL = F->getParent()->getDataLayout();

  // Quit if we don't have a single latch block or predecessor
  if (!Latch || !Predecessor) {
    return false;
  }

  // If the loop exit condition is not in the latch, then the loop is not rotated. Give up.
  if (!cast<BranchInst>(Latch->getTerminator())->isConditional()) {
    return false;
  }

  SmallVector<BasicBlock *, 16> ExitBlocks;
  L->getExitBlocks(ExitBlocks);
  std::unordered_set<BasicBlock *> ExitBlockSet(ExitBlocks.begin(), ExitBlocks.end());

  SmallVector<BasicBlock *, 16> BlocksInLoop; // Set of blocks including both body and exits
  BlocksInLoop.append(L->getBlocks().begin(), L->getBlocks().end());
  BlocksInLoop.append(ExitBlocks.begin(), ExitBlocks.end());

  // Heuristically find blocks that likely need to be unrolled
  SetVector<AllocaInst *> ProblemAllocas;
  std::unordered_set<BasicBlock *> ProblemBlocks;
  FindProblemBlocks(L->getHeader(), BlocksInLoop, ProblemBlocks, ProblemAllocas);

  // Keep track of the PHI nodes at the header.
  SmallVector<PHINode *, 16> PHIs;
  for (auto it = Header->begin(); it != Header->end(); it++) {
    if (PHINode *PN = dyn_cast<PHINode>(it)) {
      PHIs.push_back(PN);
    }
    else {
      break;
    }
  }

  // Quick simplification of PHINode incoming values
  for (PHINode *PN : PHIs) {
    for (unsigned i = 0; i < PN->getNumIncomingValues(); i++) {
      Value *OldIncomingV = PN->getIncomingValue(i);
      if (Instruction *IncomingI = dyn_cast<Instruction>(OldIncomingV)) {
        if (Value *NewIncomingV = llvm::SimplifyInstruction(IncomingI, DL)) {
          PN->setIncomingValue(i, NewIncomingV);
        }
      }
    }
  }

  SetVector<BasicBlock *> ToBeCloned; // List of blocks that will be cloned.
  for (BasicBlock *BB : L->getBlocks()) // Include the body right away
    ToBeCloned.insert(BB);

  // Find the exit blocks that also need to be included
  // in the unroll.
  SmallVector<BasicBlock *, 8> NewExits; // New set of exit blocks as boundaries for LCSSA
  SmallVector<BasicBlock *, 8> FakeExits; // Set of blocks created to allow cloning original exit blocks.
  for (BasicBlock *BB : ExitBlocks) {
    bool CloneThisExitBlock = ProblemBlocks.count(BB);

    if (CloneThisExitBlock) {
      ToBeCloned.insert(BB);

      // If we are cloning this basic block, we must create a new exit
      // block for inserting LCSSA PHI nodes.
      BasicBlock *FakeExit = BasicBlock::Create(BB->getContext(), "loop.exit.new");
      F->getBasicBlockList().insert(BB, FakeExit);

      TerminatorInst *OldTerm = BB->getTerminator();
      OldTerm->removeFromParent();
      FakeExit->getInstList().push_back(OldTerm);

      BranchInst::Create(FakeExit, BB);
      for (BasicBlock *Succ : successors(FakeExit)) {
        for (Instruction &I : *Succ) {
          if (PHINode *PN = dyn_cast<PHINode>(&I)) {
            for (unsigned i = 0; i < PN->getNumIncomingValues(); i++) {
              if (PN->getIncomingBlock(i) == BB)
                PN->setIncomingBlock(i, FakeExit);
            }
          }
        }
      }

      NewExits.push_back(FakeExit);
      FakeExits.push_back(FakeExit);

      // Update Dom tree with new exit
      if (!DT->getNode(FakeExit))
        DT->addNewBlock(FakeExit, BB);
    }
    else {
      // If we are not including this exit block in the unroll,
      // use it for LCSSA as normal.
      NewExits.push_back(BB);
    }
  }

  // Simplify the PHI nodes that have single incoming value. The original LCSSA form
  // (if exists) does not necessarily work for our unroll because we may be unrolling
  // from a different boundary.
  for (BasicBlock *BB : BlocksInLoop)
    hlsl::dxilutil::SimplifyTrivialPHIs(BB);

  // Re-establish LCSSA form to get ready for unrolling.
  CreateLCSSA(ToBeCloned, NewExits, L, *DT, LI);

  SmallVector<std::unique_ptr<LoopIteration>, 16> Iterations; // List of cloned iterations
  bool Succeeded = false;

  unsigned MaxAttempt = this->MaxIterationAttempt;
  // If we were able to figure out the definitive trip count,
  // just unroll that many times.
  if (TripCount != 0) {
    MaxAttempt = TripCount;
  }
  else if (HasExplicitLoopCount) {
    MaxAttempt = ExplicitUnrollCount;
  }

  for (unsigned IterationI = 0; IterationI < MaxAttempt; IterationI++) {

    LoopIteration *PrevIteration = nullptr;
    if (Iterations.size())
      PrevIteration = Iterations.back().get();
    Iterations.push_back(llvm::make_unique<LoopIteration>());
    LoopIteration &CurIteration = *Iterations.back().get();

    // Clone the blocks.
    for (BasicBlock *BB : ToBeCloned) {

      BasicBlock *ClonedBB = CloneBasicBlock(BB, CurIteration.VarMap);
      CurIteration.VarMap[BB] = ClonedBB;
      ClonedBB->insertInto(F, Header);

      if (ExitBlockSet.count(BB))
        CurIteration.Extended.insert(ClonedBB);

      CurIteration.Body.push_back(ClonedBB);

      // Identify the special blocks.
      if (BB == Latch) {
        CurIteration.Latch = ClonedBB;
      }
      if (BB == Header) {
        CurIteration.Header = ClonedBB;
      }
    }

    for (BasicBlock *BB : ToBeCloned) {
      BasicBlock *ClonedBB = cast<BasicBlock>(CurIteration.VarMap[BB]);
      // If branching to outside of the loop, need to update the
      // phi nodes there to include new values.
      for (BasicBlock *Succ : successors(ClonedBB)) {
        if (ToBeCloned.count(Succ))
          continue;
        for (Instruction &I : *Succ) {
          PHINode *PN = dyn_cast<PHINode>(&I);
          if (!PN)
            break;

          // Find the incoming value for this new block. If there is an entry
          // for this block in the map, then it was defined in the loop, use it.
          // Otherwise it came from outside the loop.
          Value *OldIncoming = PN->getIncomingValueForBlock(BB);
          Value *NewIncoming = OldIncoming;
          ValueToValueMapTy::iterator Itor = CurIteration.VarMap.find(OldIncoming);
          if (Itor != CurIteration.VarMap.end())
            NewIncoming = Itor->second;
          PN->addIncoming(NewIncoming, ClonedBB);
        }
      }
    }

    // Remap the instructions inside of cloned blocks.
    for (BasicBlock *BB : CurIteration.Body) {
      for (Instruction &I : *BB) {
        ::RemapInstruction(&I, CurIteration.VarMap);
      }
    }

    // If this is the first block
    if (!PrevIteration) {
      // Replace the phi nodes in the clone block with the values coming
      // from outside of the loop
      for (PHINode *PN : PHIs) {
        PHINode *ClonedPN = cast<PHINode>(CurIteration.VarMap[PN]);
        Value *ReplacementVal = ClonedPN->getIncomingValueForBlock(Predecessor);
        ClonedPN->replaceAllUsesWith(ReplacementVal);
        ClonedPN->eraseFromParent();
        CurIteration.VarMap[PN] = ReplacementVal;
      }
    }
    else {
      // Replace the phi nodes with the value defined INSIDE the previous iteration.
      for (PHINode *PN : PHIs) {
        PHINode *ClonedPN = cast<PHINode>(CurIteration.VarMap[PN]);
        Value *ReplacementVal = PN->getIncomingValueForBlock(Latch);
        auto itRep = PrevIteration->VarMap.find(ReplacementVal);
        if (itRep != PrevIteration->VarMap.end())
          ReplacementVal = itRep->second;
        ClonedPN->replaceAllUsesWith(ReplacementVal);
        ClonedPN->eraseFromParent();
        CurIteration.VarMap[PN] = ReplacementVal;
      }

      // Make the latch of the previous iteration branch to the header
      // of this new iteration.
      if (BranchInst *BI = dyn_cast<BranchInst>(PrevIteration->Latch->getTerminator())) {
        for (unsigned i = 0; i < BI->getNumSuccessors(); i++) {
          if (BI->getSuccessor(i) == PrevIteration->Header) {
            BI->setSuccessor(i, CurIteration.Header);
            break;
          }
        }
      }
    }

    // Check exit condition to see if we fully unrolled the loop
    if (BranchInst *BI = dyn_cast<BranchInst>(CurIteration.Latch->getTerminator())) {
      bool Cond = false;

      Value *ConstantCond = BI->getCondition();
      if (Value *C = DVC->GetValue(ConstantCond))
        ConstantCond = C;

      if (GetConstantI1(ConstantCond, &Cond)) {
        if (BI->getSuccessor(Cond ? 1 : 0) == CurIteration.Header) {
          Succeeded = true;
          break;
        }
      }
    }

    // We've reached the N defined in [unroll(N)]
    if ((HasExplicitLoopCount && IterationI+1 >= ExplicitUnrollCount) ||
      (TripCount != 0 && IterationI+1 >= TripCount))
    {
      Succeeded = true;
      BranchInst *BI = cast<BranchInst>(CurIteration.Latch->getTerminator());

      BasicBlock *ExitBlock = nullptr;
      for (unsigned i = 0; i < BI->getNumSuccessors(); i++) {
        BasicBlock *Succ = BI->getSuccessor(i);
        if (Succ != CurIteration.Header) {
          ExitBlock = Succ;
          break;
        }
      }

      BranchInst *NewBI = BranchInst::Create(ExitBlock, BI);
      BI->replaceAllUsesWith(NewBI);
      BI->eraseFromParent();

      break;
    }
  }

  if (Succeeded) {
    // We are going to be cleaning them up later. Maker sure
    // they're in entry block so deleting loop blocks don't 
    // kill them too.
    for (AllocaInst *AI : ProblemAllocas)
      DXASSERT_LOCALVAR(AI, AI->getParent() == &F->getEntryBlock(), "Alloca is not in entry block.");

    LoopIteration &FirstIteration = *Iterations.front().get();
    // Make the predecessor branch to the first new header.
    {
      BranchInst *BI = cast<BranchInst>(Predecessor->getTerminator());
      for (unsigned i = 0, NumSucc = BI->getNumSuccessors(); i < NumSucc; i++) {
        if (BI->getSuccessor(i) == Header) {
          BI->setSuccessor(i, FirstIteration.Header);
        }
      }
    }

    if (OuterL) {

      // Core body blocks need to be added to outer loop
      for (size_t i = 0; i < Iterations.size(); i++) {
        LoopIteration &Iteration = *Iterations[i].get();
        for (BasicBlock *BB : Iteration.Body) {
          if (!Iteration.Extended.count(BB)) {
            OuterL->addBasicBlockToLoop(BB, *LI);
          }
        }
      }

      // Our newly created exit blocks may need to be added to outer loop
      for (BasicBlock *BB : FakeExits) {
        if (HasSuccessorsInLoop(BB, OuterL))
          OuterL->addBasicBlockToLoop(BB, *LI);
      }

      // Cloned exit blocks may need to be added to outer loop
      for (size_t i = 0; i < Iterations.size(); i++) {
        LoopIteration &Iteration = *Iterations[i].get();
        for (BasicBlock *BB : Iteration.Extended) {
          if (HasSuccessorsInLoop(BB, OuterL))
            OuterL->addBasicBlockToLoop(BB, *LI);
        }
      }
    }

    SE->forgetLoop(L);

    // Remove the original blocks that we've cloned from all loops.
    for (BasicBlock *BB : ToBeCloned)
      LI->removeBlock(BB);

    // Remove loop and all child loops from queue.
    RecursivelyRemoveLoopFromQueue(LPM, L);

    // Remove dead blocks.
    for (BasicBlock *BB : ToBeCloned)
      DetachFromSuccessors(BB);
    for (BasicBlock *BB : ToBeCloned)
      BB->dropAllReferences();
    for (BasicBlock *BB : ToBeCloned)
      BB->eraseFromParent();

    // Blocks need to be removed from DomTree. There's no easy way
    // to remove them in the right order, so just make DomTree
    // recalculate.
    DT->recalculate(*F);

    if (OuterL) {
      // This process may have created multiple back edges for the
      // parent loop. Simplify to keep it well-formed.
      simplifyLoop(OuterL, DT, LI, this, nullptr, nullptr, AC);
    }

    // Now that we potentially turned some GEP indices into constants,
    // try to clean up their allocas.
    if (!BreakUpArrayAllocas(FxcCompatMode /* allow oob index */, ProblemAllocas.begin(), ProblemAllocas.end(), DT, AC, DVC)) {
      FailLoopUnroll(false, F->getContext(), LoopLoc, "Could not unroll loop due to out of bound array access.");
    }

    return true;
  }

  // If we were unsuccessful in unrolling the loop
  else {
    const char *Msg =
        "Could not unroll loop. Loop bound could not be deduced at compile time. "
        "Use [unroll(n)] to give an explicit count.";
    if (FxcCompatMode) {
      FailLoopUnroll(true /*warn only*/, F->getContext(), LoopLoc, Msg);
    }
    else {
      FailLoopUnroll(false /*warn only*/, F->getContext(), LoopLoc,
        Twine(Msg) + Twine(" Use '-HV 2016' to treat this as warning."));
    }

    // Remove all the cloned blocks
    for (std::unique_ptr<LoopIteration> &Ptr : Iterations) {
      LoopIteration &Iteration = *Ptr.get();
      for (BasicBlock *BB : Iteration.Body)
        DetachFromSuccessors(BB);
    }
    for (std::unique_ptr<LoopIteration> &Ptr : Iterations) {
      LoopIteration &Iteration = *Ptr.get();
      for (BasicBlock *BB : Iteration.Body)
        BB->dropAllReferences();
    }
    for (std::unique_ptr<LoopIteration> &Ptr : Iterations) {
      LoopIteration &Iteration = *Ptr.get();
      for (BasicBlock *BB : Iteration.Body)
        BB->eraseFromParent();
    }

    return false;
  }
}

}

// Special Mem2Reg pass
//
// In order to figure out loop bounds to unroll, we must first run mem2reg pass
// on the function, but we don't want to run mem2reg on functions that don't
// have to be unrolled when /Od is given. This pass considers all these
// conditions and runs mem2reg on functions only when needed.
//
class DxilConditionalMem2Reg : public FunctionPass {
public:
  static char ID;

  // Function overrides that resolve options when used for DxOpt
  void applyOptions(PassOptions O) override {
    GetPassOptionBool(O, "NoOpt", &NoOpt, false);
  }
  void dumpConfig(raw_ostream &OS) override {
    FunctionPass::dumpConfig(OS);
    OS << ",NoOpt=" << NoOpt;
  }

  bool NoOpt = false;
  explicit DxilConditionalMem2Reg(bool NoOpt=false) : FunctionPass(ID), NoOpt(NoOpt)
  {
    initializeDxilConditionalMem2RegPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<AssumptionCacheTracker>();
    AU.setPreservesCFG();
  }

  // Recursively find loops that are marked with [unroll]
  static bool HasLoopsMarkedUnrollRecursive(Loop *L) {
    int Count = 0;
    if (IsMarkedFullUnroll(L) || IsMarkedUnrollCount(L, &Count)) {
      return true;
    }
    for (Loop *ChildLoop : *L) {
      if (HasLoopsMarkedUnrollRecursive(ChildLoop))
        return true;
    }
    return false;
  }

  // Collect and remove all instructions that use AI, but
  // give up if there are anything other than store, bitcast,
  // memcpy, or GEP.
  static bool TryRemoveUnusedAlloca(AllocaInst *AI) {
    std::vector<Instruction *> WorkList;

    WorkList.push_back(AI);

    for (unsigned i = 0; i < WorkList.size(); i++) {
      Instruction *I = WorkList[i];

      for (User *U : I->users()) {
        Instruction *UI = cast<Instruction>(U);

        unsigned Opcode = UI->getOpcode();
        if (Opcode == Instruction::BitCast ||
          Opcode == Instruction::GetElementPtr ||
          Opcode == Instruction::Store)
        {
          WorkList.push_back(UI);
        }
        else if (MemCpyInst *MC = dyn_cast<MemCpyInst>(UI)) {
          if (MC->getSource() == I) { // MC reads from our alloca
            return false;
          }
          WorkList.push_back(UI);
        }
        else { // Load? PHINode? Assume read.
          return false;
        }
      }
    }

    // Remove all instructions
    for (auto It = WorkList.rbegin(), E = WorkList.rend(); It != E; It++) {
      Instruction *I = *It;
      I->eraseFromParent();
    }

    return true;
  }

  static bool RemoveAllUnusedAllocas(Function &F) {
    std::vector<AllocaInst *> Allocas;
    BasicBlock &EntryBB = *F.begin();
    for (auto It = EntryBB.begin(), E = EntryBB.end(); It != E;) {
      Instruction &I = *(It++);
      if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
        Allocas.push_back(AI);
      }
    }

    bool Changed = false;
    for (AllocaInst *AI : Allocas) {
      Changed |= TryRemoveUnusedAlloca(AI);
    }

    return Changed;
  }

  bool runOnFunction(Function &F) override {


    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    AssumptionCache *AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);

    bool Changed = false;
    
    Changed |= RemoveAllUnusedAllocas(F);
    Changed |= Mem2Reg(F, *DT, *AC);

    return Changed;
  }
};
char DxilConditionalMem2Reg::ID;

Pass *llvm::createDxilConditionalMem2RegPass(bool NoOpt) {
  return new DxilConditionalMem2Reg(NoOpt);
}
Pass *llvm::createDxilLoopUnrollPass(unsigned MaxIterationAttempt) {
  return new DxilLoopUnroll(MaxIterationAttempt);
}

INITIALIZE_PASS_BEGIN(DxilConditionalMem2Reg, "dxil-cond-mem2reg", "Dxil Conditional Mem2Reg", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_END(DxilConditionalMem2Reg, "dxil-cond-mem2reg", "Dxil Conditional Mem2Reg", false, false)

INITIALIZE_PASS_BEGIN(DxilLoopUnroll, "dxil-loop-unroll", "Dxil Unroll loops", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(DxilValueCache)
INITIALIZE_PASS_END(DxilLoopUnroll, "dxil-loop-unroll", "Dxil Unroll loops", false, false)

