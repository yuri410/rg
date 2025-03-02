//===- SampleProfile.cpp - Incorporate sample profiles into the IR --------===//
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the SampleProfileLoader transformation. This pass
// reads a profile file generated by a sampling profiler (e.g. Linux Perf -
// http://perf.wiki.kernel.org/) and generates IR metadata to reflect the
// profile information in the given profile.
//
// This pass generates branch weight annotations on the IR:
//
// - prof: Represents branch weights. This annotation is added to branches
//      to indicate the weights of each edge coming out of the branch.
//      The weight of each edge is the weight of the target block for
//      that edge. The weight of a block B is computed as the maximum
//      number of samples found in B.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/ProfileData/SampleProfReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <cctype>

using namespace llvm;
using namespace sampleprof;

#define DEBUG_TYPE "sample-profile"

#if 0 // HLSL Change Start
// Command line option to specify the file to read samples from. This is
// mainly used for debugging.
static cl::opt<std::string> SampleProfileFile(
    "sample-profile-file", cl::init(""), cl::value_desc("filename"),
    cl::desc("Profile file loaded by -sample-profile"), cl::Hidden);
static cl::opt<unsigned> SampleProfileMaxPropagateIterations(
    "sample-profile-max-propagate-iterations", cl::init(100),
    cl::desc("Maximum number of iterations to go through when propagating "
             "sample block/edge weights through the CFG."));
#else
static const char SampleProfileFile[] = "";
static const unsigned SampleProfileMaxPropagateIterations = 100;
#endif // HLSL Change Ends

namespace {
typedef DenseMap<BasicBlock *, unsigned> BlockWeightMap;
typedef DenseMap<BasicBlock *, BasicBlock *> EquivalenceClassMap;
typedef std::pair<BasicBlock *, BasicBlock *> Edge;
typedef DenseMap<Edge, unsigned> EdgeWeightMap;
typedef DenseMap<BasicBlock *, SmallVector<BasicBlock *, 8>> BlockEdgeMap;

/// \brief Sample profile pass.
///
/// This pass reads profile data from the file specified by
/// -sample-profile-file and annotates every affected function with the
/// profile information found in that file.
class SampleProfileLoader : public FunctionPass {
public:
  // Class identification, replacement for typeinfo
  static char ID;

  SampleProfileLoader(StringRef Name = SampleProfileFile)
      : FunctionPass(ID), DT(nullptr), PDT(nullptr), LI(nullptr), Ctx(nullptr),
        Reader(), Samples(nullptr), Filename(Name), ProfileIsValid(false) {
    initializeSampleProfileLoaderPass(*PassRegistry::getPassRegistry());
  }

  bool doInitialization(Module &M) override;

  void dump() { Reader->dump(); }

  const char *getPassName() const override { return "Sample profile pass"; }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTree>();
  }

protected:
  unsigned getFunctionLoc(Function &F);
  bool emitAnnotations(Function &F);
  unsigned getInstWeight(Instruction &I);
  unsigned getBlockWeight(BasicBlock *BB);
  void printEdgeWeight(raw_ostream &OS, Edge E);
  void printBlockWeight(raw_ostream &OS, BasicBlock *BB);
  void printBlockEquivalence(raw_ostream &OS, BasicBlock *BB);
  bool computeBlockWeights(Function &F);
  void findEquivalenceClasses(Function &F);
  void findEquivalencesFor(BasicBlock *BB1,
                           SmallVector<BasicBlock *, 8> Descendants,
                           DominatorTreeBase<BasicBlock> *DomTree);
  void propagateWeights(Function &F);
  unsigned visitEdge(Edge E, unsigned *NumUnknownEdges, Edge *UnknownEdge);
  void buildEdges(Function &F);
  bool propagateThroughEdges(Function &F);

  /// \brief Line number for the function header. Used to compute absolute
  /// line numbers from the relative line numbers found in the profile.
  unsigned HeaderLineno;

  /// \brief Map basic blocks to their computed weights.
  ///
  /// The weight of a basic block is defined to be the maximum
  /// of all the instruction weights in that block.
  BlockWeightMap BlockWeights;

  /// \brief Map edges to their computed weights.
  ///
  /// Edge weights are computed by propagating basic block weights in
  /// SampleProfile::propagateWeights.
  EdgeWeightMap EdgeWeights;

  /// \brief Set of visited blocks during propagation.
  SmallPtrSet<BasicBlock *, 128> VisitedBlocks;

  /// \brief Set of visited edges during propagation.
  SmallSet<Edge, 128> VisitedEdges;

  /// \brief Equivalence classes for block weights.
  ///
  /// Two blocks BB1 and BB2 are in the same equivalence class if they
  /// dominate and post-dominate each other, and they are in the same loop
  /// nest. When this happens, the two blocks are guaranteed to execute
  /// the same number of times.
  EquivalenceClassMap EquivalenceClass;

  /// \brief Dominance, post-dominance and loop information.
  DominatorTree *DT;
  PostDominatorTree *PDT;
  LoopInfo *LI;

  /// \brief Predecessors for each basic block in the CFG.
  BlockEdgeMap Predecessors;

  /// \brief Successors for each basic block in the CFG.
  BlockEdgeMap Successors;

  /// \brief LLVM context holding the debug data we need.
  LLVMContext *Ctx;

  /// \brief Profile reader object.
  std::unique_ptr<SampleProfileReader> Reader;

  /// \brief Samples collected for the body of this function.
  FunctionSamples *Samples;

  /// \brief Name of the profile file to load.
  StringRef Filename;

  /// \brief Flag indicating whether the profile input loaded successfully.
  bool ProfileIsValid;
};
}

/// \brief Print the weight of edge \p E on stream \p OS.
///
/// \param OS  Stream to emit the output to.
/// \param E  Edge to print.
void SampleProfileLoader::printEdgeWeight(raw_ostream &OS, Edge E) {
  OS << "weight[" << E.first->getName() << "->" << E.second->getName()
     << "]: " << EdgeWeights[E] << "\n";
}

/// \brief Print the equivalence class of block \p BB on stream \p OS.
///
/// \param OS  Stream to emit the output to.
/// \param BB  Block to print.
void SampleProfileLoader::printBlockEquivalence(raw_ostream &OS,
                                                BasicBlock *BB) {
  BasicBlock *Equiv = EquivalenceClass[BB];
  OS << "equivalence[" << BB->getName()
     << "]: " << ((Equiv) ? EquivalenceClass[BB]->getName() : "NONE") << "\n";
}

/// \brief Print the weight of block \p BB on stream \p OS.
///
/// \param OS  Stream to emit the output to.
/// \param BB  Block to print.
void SampleProfileLoader::printBlockWeight(raw_ostream &OS, BasicBlock *BB) {
  OS << "weight[" << BB->getName() << "]: " << BlockWeights[BB] << "\n";
}

/// \brief Get the weight for an instruction.
///
/// The "weight" of an instruction \p Inst is the number of samples
/// collected on that instruction at runtime. To retrieve it, we
/// need to compute the line number of \p Inst relative to the start of its
/// function. We use HeaderLineno to compute the offset. We then
/// look up the samples collected for \p Inst using BodySamples.
///
/// \param Inst Instruction to query.
///
/// \returns The profiled weight of I.
unsigned SampleProfileLoader::getInstWeight(Instruction &Inst) {
  DebugLoc DLoc = Inst.getDebugLoc();
  if (!DLoc)
    return 0;

  unsigned Lineno = DLoc.getLine();
  if (Lineno < HeaderLineno)
    return 0;

  const DILocation *DIL = DLoc;
  int LOffset = Lineno - HeaderLineno;
  unsigned Discriminator = DIL->getDiscriminator();
  unsigned Weight = Samples->samplesAt(LOffset, Discriminator);
  DEBUG(dbgs() << "    " << Lineno << "." << Discriminator << ":" << Inst
               << " (line offset: " << LOffset << "." << Discriminator
               << " - weight: " << Weight << ")\n");
  return Weight;
}

/// \brief Compute the weight of a basic block.
///
/// The weight of basic block \p BB is the maximum weight of all the
/// instructions in BB. The weight of \p BB is computed and cached in
/// the BlockWeights map.
///
/// \param BB The basic block to query.
///
/// \returns The computed weight of BB.
unsigned SampleProfileLoader::getBlockWeight(BasicBlock *BB) {
  // If we've computed BB's weight before, return it.
  std::pair<BlockWeightMap::iterator, bool> Entry =
      BlockWeights.insert(std::make_pair(BB, 0));
  if (!Entry.second)
    return Entry.first->second;

  // Otherwise, compute and cache BB's weight.
  unsigned Weight = 0;
  for (auto &I : BB->getInstList()) {
    unsigned InstWeight = getInstWeight(I);
    if (InstWeight > Weight)
      Weight = InstWeight;
  }
  Entry.first->second = Weight;
  return Weight;
}

/// \brief Compute and store the weights of every basic block.
///
/// This populates the BlockWeights map by computing
/// the weights of every basic block in the CFG.
///
/// \param F The function to query.
bool SampleProfileLoader::computeBlockWeights(Function &F) {
  bool Changed = false;
  DEBUG(dbgs() << "Block weights\n");
  for (auto &BB : F) {
    unsigned Weight = getBlockWeight(&BB);
    Changed |= (Weight > 0);
    DEBUG(printBlockWeight(dbgs(), &BB));
  }

  return Changed;
}

/// \brief Find equivalence classes for the given block.
///
/// This finds all the blocks that are guaranteed to execute the same
/// number of times as \p BB1. To do this, it traverses all the
/// descendants of \p BB1 in the dominator or post-dominator tree.
///
/// A block BB2 will be in the same equivalence class as \p BB1 if
/// the following holds:
///
/// 1- \p BB1 is a descendant of BB2 in the opposite tree. So, if BB2
///    is a descendant of \p BB1 in the dominator tree, then BB2 should
///    dominate BB1 in the post-dominator tree.
///
/// 2- Both BB2 and \p BB1 must be in the same loop.
///
/// For every block BB2 that meets those two requirements, we set BB2's
/// equivalence class to \p BB1.
///
/// \param BB1  Block to check.
/// \param Descendants  Descendants of \p BB1 in either the dom or pdom tree.
/// \param DomTree  Opposite dominator tree. If \p Descendants is filled
///                 with blocks from \p BB1's dominator tree, then
///                 this is the post-dominator tree, and vice versa.
void SampleProfileLoader::findEquivalencesFor(
    BasicBlock *BB1, SmallVector<BasicBlock *, 8> Descendants,
    DominatorTreeBase<BasicBlock> *DomTree) {
  for (auto *BB2 : Descendants) {
    bool IsDomParent = DomTree->dominates(BB2, BB1);
    bool IsInSameLoop = LI->getLoopFor(BB1) == LI->getLoopFor(BB2);
    if (BB1 != BB2 && VisitedBlocks.insert(BB2).second && IsDomParent &&
        IsInSameLoop) {
      EquivalenceClass[BB2] = BB1;

      // If BB2 is heavier than BB1, make BB2 have the same weight
      // as BB1.
      //
      // Note that we don't worry about the opposite situation here
      // (when BB2 is lighter than BB1). We will deal with this
      // during the propagation phase. Right now, we just want to
      // make sure that BB1 has the largest weight of all the
      // members of its equivalence set.
      unsigned &BB1Weight = BlockWeights[BB1];
      unsigned &BB2Weight = BlockWeights[BB2];
      BB1Weight = std::max(BB1Weight, BB2Weight);
    }
  }
}

/// \brief Find equivalence classes.
///
/// Since samples may be missing from blocks, we can fill in the gaps by setting
/// the weights of all the blocks in the same equivalence class to the same
/// weight. To compute the concept of equivalence, we use dominance and loop
/// information. Two blocks B1 and B2 are in the same equivalence class if B1
/// dominates B2, B2 post-dominates B1 and both are in the same loop.
///
/// \param F The function to query.
void SampleProfileLoader::findEquivalenceClasses(Function &F) {
  SmallVector<BasicBlock *, 8> DominatedBBs;
  DEBUG(dbgs() << "\nBlock equivalence classes\n");
  // Find equivalence sets based on dominance and post-dominance information.
  for (auto &BB : F) {
    BasicBlock *BB1 = &BB;

    // Compute BB1's equivalence class once.
    if (EquivalenceClass.count(BB1)) {
      DEBUG(printBlockEquivalence(dbgs(), BB1));
      continue;
    }

    // By default, blocks are in their own equivalence class.
    EquivalenceClass[BB1] = BB1;

    // Traverse all the blocks dominated by BB1. We are looking for
    // every basic block BB2 such that:
    //
    // 1- BB1 dominates BB2.
    // 2- BB2 post-dominates BB1.
    // 3- BB1 and BB2 are in the same loop nest.
    //
    // If all those conditions hold, it means that BB2 is executed
    // as many times as BB1, so they are placed in the same equivalence
    // class by making BB2's equivalence class be BB1.
    DominatedBBs.clear();
    DT->getDescendants(BB1, DominatedBBs);
    findEquivalencesFor(BB1, DominatedBBs, PDT->DT);

    // Repeat the same logic for all the blocks post-dominated by BB1.
    // We are looking for every basic block BB2 such that:
    //
    // 1- BB1 post-dominates BB2.
    // 2- BB2 dominates BB1.
    // 3- BB1 and BB2 are in the same loop nest.
    //
    // If all those conditions hold, BB2's equivalence class is BB1.
    DominatedBBs.clear();
    PDT->getDescendants(BB1, DominatedBBs);
    findEquivalencesFor(BB1, DominatedBBs, DT);

    DEBUG(printBlockEquivalence(dbgs(), BB1));
  }

  // Assign weights to equivalence classes.
  //
  // All the basic blocks in the same equivalence class will execute
  // the same number of times. Since we know that the head block in
  // each equivalence class has the largest weight, assign that weight
  // to all the blocks in that equivalence class.
  DEBUG(dbgs() << "\nAssign the same weight to all blocks in the same class\n");
  for (auto &BI : F) {
    BasicBlock *BB = &BI;
    BasicBlock *EquivBB = EquivalenceClass[BB];
    if (BB != EquivBB)
      BlockWeights[BB] = BlockWeights[EquivBB];
    DEBUG(printBlockWeight(dbgs(), BB));
  }
}

/// \brief Visit the given edge to decide if it has a valid weight.
///
/// If \p E has not been visited before, we copy to \p UnknownEdge
/// and increment the count of unknown edges.
///
/// \param E  Edge to visit.
/// \param NumUnknownEdges  Current number of unknown edges.
/// \param UnknownEdge  Set if E has not been visited before.
///
/// \returns E's weight, if known. Otherwise, return 0.
unsigned SampleProfileLoader::visitEdge(Edge E, unsigned *NumUnknownEdges,
                                        Edge *UnknownEdge) {
  if (!VisitedEdges.count(E)) {
    (*NumUnknownEdges)++;
    *UnknownEdge = E;
    return 0;
  }

  return EdgeWeights[E];
}

/// \brief Propagate weights through incoming/outgoing edges.
///
/// If the weight of a basic block is known, and there is only one edge
/// with an unknown weight, we can calculate the weight of that edge.
///
/// Similarly, if all the edges have a known count, we can calculate the
/// count of the basic block, if needed.
///
/// \param F  Function to process.
///
/// \returns  True if new weights were assigned to edges or blocks.
bool SampleProfileLoader::propagateThroughEdges(Function &F) {
  bool Changed = false;
  DEBUG(dbgs() << "\nPropagation through edges\n");
  for (auto &BI : F) {
    BasicBlock *BB = &BI;

    // Visit all the predecessor and successor edges to determine
    // which ones have a weight assigned already. Note that it doesn't
    // matter that we only keep track of a single unknown edge. The
    // only case we are interested in handling is when only a single
    // edge is unknown (see setEdgeOrBlockWeight).
    for (unsigned i = 0; i < 2; i++) {
      unsigned TotalWeight = 0;
      unsigned NumUnknownEdges = 0;
      Edge UnknownEdge, SelfReferentialEdge;

      if (i == 0) {
        // First, visit all predecessor edges.
        for (auto *Pred : Predecessors[BB]) {
          Edge E = std::make_pair(Pred, BB);
          TotalWeight += visitEdge(E, &NumUnknownEdges, &UnknownEdge);
          if (E.first == E.second)
            SelfReferentialEdge = E;
        }
      } else {
        // On the second round, visit all successor edges.
        for (auto *Succ : Successors[BB]) {
          Edge E = std::make_pair(BB, Succ);
          TotalWeight += visitEdge(E, &NumUnknownEdges, &UnknownEdge);
        }
      }

      // After visiting all the edges, there are three cases that we
      // can handle immediately:
      //
      // - All the edge weights are known (i.e., NumUnknownEdges == 0).
      //   In this case, we simply check that the sum of all the edges
      //   is the same as BB's weight. If not, we change BB's weight
      //   to match. Additionally, if BB had not been visited before,
      //   we mark it visited.
      //
      // - Only one edge is unknown and BB has already been visited.
      //   In this case, we can compute the weight of the edge by
      //   subtracting the total block weight from all the known
      //   edge weights. If the edges weight more than BB, then the
      //   edge of the last remaining edge is set to zero.
      //
      // - There exists a self-referential edge and the weight of BB is
      //   known. In this case, this edge can be based on BB's weight.
      //   We add up all the other known edges and set the weight on
      //   the self-referential edge as we did in the previous case.
      //
      // In any other case, we must continue iterating. Eventually,
      // all edges will get a weight, or iteration will stop when
      // it reaches SampleProfileMaxPropagateIterations.
      if (NumUnknownEdges <= 1) {
        unsigned &BBWeight = BlockWeights[BB];
        if (NumUnknownEdges == 0) {
          // If we already know the weight of all edges, the weight of the
          // basic block can be computed. It should be no larger than the sum
          // of all edge weights.
          if (TotalWeight > BBWeight) {
            BBWeight = TotalWeight;
            Changed = true;
            DEBUG(dbgs() << "All edge weights for " << BB->getName()
                         << " known. Set weight for block: ";
                  printBlockWeight(dbgs(), BB););
          }
          if (VisitedBlocks.insert(BB).second)
            Changed = true;
        } else if (NumUnknownEdges == 1 && VisitedBlocks.count(BB)) {
          // If there is a single unknown edge and the block has been
          // visited, then we can compute E's weight.
          if (BBWeight >= TotalWeight)
            EdgeWeights[UnknownEdge] = BBWeight - TotalWeight;
          else
            EdgeWeights[UnknownEdge] = 0;
          VisitedEdges.insert(UnknownEdge);
          Changed = true;
          DEBUG(dbgs() << "Set weight for edge: ";
                printEdgeWeight(dbgs(), UnknownEdge));
        }
      } else if (SelfReferentialEdge.first && VisitedBlocks.count(BB)) {
        unsigned &BBWeight = BlockWeights[BB];
        // We have a self-referential edge and the weight of BB is known.
        if (BBWeight >= TotalWeight)
          EdgeWeights[SelfReferentialEdge] = BBWeight - TotalWeight;
        else
          EdgeWeights[SelfReferentialEdge] = 0;
        VisitedEdges.insert(SelfReferentialEdge);
        Changed = true;
        DEBUG(dbgs() << "Set self-referential edge weight to: ";
              printEdgeWeight(dbgs(), SelfReferentialEdge));
      }
    }
  }

  return Changed;
}

/// \brief Build in/out edge lists for each basic block in the CFG.
///
/// We are interested in unique edges. If a block B1 has multiple
/// edges to another block B2, we only add a single B1->B2 edge.
void SampleProfileLoader::buildEdges(Function &F) {
  for (auto &BI : F) {
    BasicBlock *B1 = &BI;

    // Add predecessors for B1.
    SmallPtrSet<BasicBlock *, 16> Visited;
    if (!Predecessors[B1].empty())
      llvm_unreachable("Found a stale predecessors list in a basic block.");
    for (pred_iterator PI = pred_begin(B1), PE = pred_end(B1); PI != PE; ++PI) {
      BasicBlock *B2 = *PI;
      if (Visited.insert(B2).second)
        Predecessors[B1].push_back(B2);
    }

    // Add successors for B1.
    Visited.clear();
    if (!Successors[B1].empty())
      llvm_unreachable("Found a stale successors list in a basic block.");
    for (succ_iterator SI = succ_begin(B1), SE = succ_end(B1); SI != SE; ++SI) {
      BasicBlock *B2 = *SI;
      if (Visited.insert(B2).second)
        Successors[B1].push_back(B2);
    }
  }
}

/// \brief Propagate weights into edges
///
/// The following rules are applied to every block BB in the CFG:
///
/// - If BB has a single predecessor/successor, then the weight
///   of that edge is the weight of the block.
///
/// - If all incoming or outgoing edges are known except one, and the
///   weight of the block is already known, the weight of the unknown
///   edge will be the weight of the block minus the sum of all the known
///   edges. If the sum of all the known edges is larger than BB's weight,
///   we set the unknown edge weight to zero.
///
/// - If there is a self-referential edge, and the weight of the block is
///   known, the weight for that edge is set to the weight of the block
///   minus the weight of the other incoming edges to that block (if
///   known).
void SampleProfileLoader::propagateWeights(Function &F) {
  bool Changed = true;
  unsigned i = 0;

  // Add an entry count to the function using the samples gathered
  // at the function entry.
  F.setEntryCount(Samples->getHeadSamples());

  // Before propagation starts, build, for each block, a list of
  // unique predecessors and successors. This is necessary to handle
  // identical edges in multiway branches. Since we visit all blocks and all
  // edges of the CFG, it is cleaner to build these lists once at the start
  // of the pass.
  buildEdges(F);

  // Propagate until we converge or we go past the iteration limit.
  while (Changed && i++ < SampleProfileMaxPropagateIterations) {
    Changed = propagateThroughEdges(F);
  }

  // Generate MD_prof metadata for every branch instruction using the
  // edge weights computed during propagation.
  DEBUG(dbgs() << "\nPropagation complete. Setting branch weights\n");
  MDBuilder MDB(F.getContext());
  for (auto &BI : F) {
    BasicBlock *BB = &BI;
    TerminatorInst *TI = BB->getTerminator();
    if (TI->getNumSuccessors() == 1)
      continue;
    if (!isa<BranchInst>(TI) && !isa<SwitchInst>(TI))
      continue;

    DEBUG(dbgs() << "\nGetting weights for branch at line "
                 << TI->getDebugLoc().getLine() << ".\n");
    SmallVector<unsigned, 4> Weights;
    bool AllWeightsZero = true;
    for (unsigned I = 0; I < TI->getNumSuccessors(); ++I) {
      BasicBlock *Succ = TI->getSuccessor(I);
      Edge E = std::make_pair(BB, Succ);
      unsigned Weight = EdgeWeights[E];
      DEBUG(dbgs() << "\t"; printEdgeWeight(dbgs(), E));
      Weights.push_back(Weight);
      if (Weight != 0)
        AllWeightsZero = false;
    }

    // Only set weights if there is at least one non-zero weight.
    // In any other case, let the analyzer set weights.
    if (!AllWeightsZero) {
      DEBUG(dbgs() << "SUCCESS. Found non-zero weights.\n");
      TI->setMetadata(llvm::LLVMContext::MD_prof,
                      MDB.createBranchWeights(Weights));
    } else {
      DEBUG(dbgs() << "SKIPPED. All branch weights are zero.\n");
    }
  }
}

/// \brief Get the line number for the function header.
///
/// This looks up function \p F in the current compilation unit and
/// retrieves the line number where the function is defined. This is
/// line 0 for all the samples read from the profile file. Every line
/// number is relative to this line.
///
/// \param F  Function object to query.
///
/// \returns the line number where \p F is defined. If it returns 0,
///          it means that there is no debug information available for \p F.
unsigned SampleProfileLoader::getFunctionLoc(Function &F) {
  if (DISubprogram *S = getDISubprogram(&F))
    return S->getLine();

  // If could not find the start of \p F, emit a diagnostic to inform the user
  // about the missed opportunity.
  F.getContext().diagnose(DiagnosticInfoSampleProfile(
      "No debug information found in function " + F.getName() +
          ": Function profile not used",
      DS_Warning));
  return 0;
}

/// \brief Generate branch weight metadata for all branches in \p F.
///
/// Branch weights are computed out of instruction samples using a
/// propagation heuristic. Propagation proceeds in 3 phases:
///
/// 1- Assignment of block weights. All the basic blocks in the function
///    are initial assigned the same weight as their most frequently
///    executed instruction.
///
/// 2- Creation of equivalence classes. Since samples may be missing from
///    blocks, we can fill in the gaps by setting the weights of all the
///    blocks in the same equivalence class to the same weight. To compute
///    the concept of equivalence, we use dominance and loop information.
///    Two blocks B1 and B2 are in the same equivalence class if B1
///    dominates B2, B2 post-dominates B1 and both are in the same loop.
///
/// 3- Propagation of block weights into edges. This uses a simple
///    propagation heuristic. The following rules are applied to every
///    block BB in the CFG:
///
///    - If BB has a single predecessor/successor, then the weight
///      of that edge is the weight of the block.
///
///    - If all the edges are known except one, and the weight of the
///      block is already known, the weight of the unknown edge will
///      be the weight of the block minus the sum of all the known
///      edges. If the sum of all the known edges is larger than BB's weight,
///      we set the unknown edge weight to zero.
///
///    - If there is a self-referential edge, and the weight of the block is
///      known, the weight for that edge is set to the weight of the block
///      minus the weight of the other incoming edges to that block (if
///      known).
///
/// Since this propagation is not guaranteed to finalize for every CFG, we
/// only allow it to proceed for a limited number of iterations (controlled
/// by -sample-profile-max-propagate-iterations).
///
/// FIXME: Try to replace this propagation heuristic with a scheme
/// that is guaranteed to finalize. A work-list approach similar to
/// the standard value propagation algorithm used by SSA-CCP might
/// work here.
///
/// Once all the branch weights are computed, we emit the MD_prof
/// metadata on BB using the computed values for each of its branches.
///
/// \param F The function to query.
///
/// \returns true if \p F was modified. Returns false, otherwise.
bool SampleProfileLoader::emitAnnotations(Function &F) {
  bool Changed = false;

  // Initialize invariants used during computation and propagation.
  HeaderLineno = getFunctionLoc(F);
  if (HeaderLineno == 0)
    return false;

  DEBUG(dbgs() << "Line number for the first instruction in " << F.getName()
               << ": " << HeaderLineno << "\n");

  // Compute basic block weights.
  Changed |= computeBlockWeights(F);

  if (Changed) {
    // Find equivalence classes.
    findEquivalenceClasses(F);

    // Propagate weights to all edges.
    propagateWeights(F);
  }

  return Changed;
}

char SampleProfileLoader::ID = 0;
INITIALIZE_PASS_BEGIN(SampleProfileLoader, "sample-profile",
                      "Sample Profile loader", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTree)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AddDiscriminators)
INITIALIZE_PASS_END(SampleProfileLoader, "sample-profile",
                    "Sample Profile loader", false, false)

bool SampleProfileLoader::doInitialization(Module &M) {
  auto ReaderOrErr = SampleProfileReader::create(Filename, M.getContext());
  if (std::error_code EC = ReaderOrErr.getError()) {
    std::string Msg = "Could not open profile: " + EC.message();
    M.getContext().diagnose(DiagnosticInfoSampleProfile(Filename.data(), Msg));
    return false;
  }
  Reader = std::move(ReaderOrErr.get());
  ProfileIsValid = (Reader->read() == sampleprof_error::success);
  return true;
}

FunctionPass *llvm::createSampleProfileLoaderPass() {
  return new SampleProfileLoader(SampleProfileFile);
}

FunctionPass *llvm::createSampleProfileLoaderPass(StringRef Name) {
  return new SampleProfileLoader(Name);
}

bool SampleProfileLoader::runOnFunction(Function &F) {
  if (!ProfileIsValid)
    return false;

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTree>();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  Ctx = &F.getParent()->getContext();
  Samples = Reader->getSamplesFor(F);
  if (!Samples->empty())
    return emitAnnotations(F);
  return false;
}
