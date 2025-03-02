//===- CallGraphSCCPass.cpp - Pass that operates BU on call graph ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CallGraphSCCPass class, which is used for passes
// which are implemented as bottom-up traversals on the call graph.  Because
// there may be cycles in the call graph, passes of this type operate on the
// call-graph in SCC order: that is, they process function bottom-up, except for
// recursive functions, which they process all at once.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManagers.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "cgscc-passmgr"

#if 0 // HLSL Change Starts - option pending
static cl::opt<unsigned> 
MaxIterations("max-cg-scc-iterations", cl::ReallyHidden, cl::init(4));
#else
static const unsigned MaxIterations = 4;
#endif

STATISTIC(MaxSCCIterations, "Maximum CGSCCPassMgr iterations on one SCC");

//===----------------------------------------------------------------------===//
// CGPassManager
//
/// CGPassManager manages FPPassManagers and CallGraphSCCPasses.

namespace {

class CGPassManager : public ModulePass, public PMDataManager {
public:
  static char ID;
  explicit CGPassManager() 
    : ModulePass(ID), PMDataManager() { }

  /// Execute all of the passes scheduled for execution.  Keep track of
  /// whether any of the passes modifies the module, and if so, return true.
  bool runOnModule(Module &M) override;

  using ModulePass::doInitialization;
  using ModulePass::doFinalization;

  bool doInitialization(CallGraph &CG);
  bool doFinalization(CallGraph &CG);

  /// Pass Manager itself does not invalidate any analysis info.
  void getAnalysisUsage(AnalysisUsage &Info) const override {
    // CGPassManager walks SCC and it needs CallGraph.
    Info.addRequired<CallGraphWrapperPass>();
    Info.setPreservesAll();
  }

  const char *getPassName() const override {
    return "CallGraph Pass Manager";
  }

  PMDataManager *getAsPMDataManager() override { return this; }
  Pass *getAsPass() override { return this; }

  // Print passes managed by this manager
  void dumpPassStructure(unsigned Offset) override {
    errs().indent(Offset*2) << "Call Graph SCC Pass Manager\n";
    for (unsigned Index = 0; Index < getNumContainedPasses(); ++Index) {
      Pass *P = getContainedPass(Index);
      P->dumpPassStructure(Offset + 1);
      dumpLastUses(P, Offset+1);
    }
  }

  Pass *getContainedPass(unsigned N) {
    assert(N < PassVector.size() && "Pass number out of range!");
    return static_cast<Pass *>(PassVector[N]);
  }

  PassManagerType getPassManagerType() const override {
    return PMT_CallGraphPassManager; 
  }
  
private:
  bool RunAllPassesOnSCC(CallGraphSCC &CurSCC, CallGraph &CG,
                         bool &DevirtualizedCall);
  
  bool RunPassOnSCC(Pass *P, CallGraphSCC &CurSCC,
                    CallGraph &CG, bool &CallGraphUpToDate,
                    bool &DevirtualizedCall);
  bool RefreshCallGraph(CallGraphSCC &CurSCC, CallGraph &CG,
                        bool IsCheckingMode);
};

} // end anonymous namespace.

char CGPassManager::ID = 0;


bool CGPassManager::RunPassOnSCC(Pass *P, CallGraphSCC &CurSCC,
                                 CallGraph &CG, bool &CallGraphUpToDate,
                                 bool &DevirtualizedCall) {
  bool Changed = false;
  PMDataManager *PM = P->getAsPMDataManager();

  if (!PM) {
    CallGraphSCCPass *CGSP = (CallGraphSCCPass*)P;
    if (!CallGraphUpToDate) {
      DevirtualizedCall |= RefreshCallGraph(CurSCC, CG, false);
      CallGraphUpToDate = true;
    }

    {
      TimeRegion PassTimer(getPassTimer(CGSP));
      Changed = CGSP->runOnSCC(CurSCC);
    }
    
    // After the CGSCCPass is done, when assertions are enabled, use
    // RefreshCallGraph to verify that the callgraph was correctly updated.
#ifndef NDEBUG
    if (Changed)
      RefreshCallGraph(CurSCC, CG, true);
#endif
    
    return Changed;
  }
  
  
  assert(PM->getPassManagerType() == PMT_FunctionPassManager &&
         "Invalid CGPassManager member");
  FPPassManager *FPP = (FPPassManager*)P;
  
  // Run pass P on all functions in the current SCC.
  for (CallGraphNode *CGN : CurSCC) {
    if (Function *F = CGN->getFunction()) {
      dumpPassInfo(P, EXECUTION_MSG, ON_FUNCTION_MSG, F->getName());
      {
        TimeRegion PassTimer(getPassTimer(FPP));
        Changed |= FPP->runOnFunction(*F);
      }
      F->getContext().yield();
    }
  }
  
  // The function pass(es) modified the IR, they may have clobbered the
  // callgraph.
  if (Changed && CallGraphUpToDate) {
    DEBUG(dbgs() << "CGSCCPASSMGR: Pass Dirtied SCC: "
                 << P->getPassName() << '\n');
    CallGraphUpToDate = false;
  }
  return Changed;
}


/// Scan the functions in the specified CFG and resync the
/// callgraph with the call sites found in it.  This is used after
/// FunctionPasses have potentially munged the callgraph, and can be used after
/// CallGraphSCC passes to verify that they correctly updated the callgraph.
///
/// This function returns true if it devirtualized an existing function call,
/// meaning it turned an indirect call into a direct call.  This happens when
/// a function pass like GVN optimizes away stuff feeding the indirect call.
/// This never happens in checking mode.
///
bool CGPassManager::RefreshCallGraph(CallGraphSCC &CurSCC,
                                     CallGraph &CG, bool CheckingMode) {
  DenseMap<Value*, CallGraphNode*> CallSites;
  
  DEBUG(dbgs() << "CGSCCPASSMGR: Refreshing SCC with " << CurSCC.size()
               << " nodes:\n";
        for (CallGraphNode *CGN : CurSCC)
          CGN->dump();
        );

  bool MadeChange = false;
  bool DevirtualizedCall = false;
  
  // Scan all functions in the SCC.
  unsigned FunctionNo = 0;
  for (CallGraphSCC::iterator SCCIdx = CurSCC.begin(), E = CurSCC.end();
       SCCIdx != E; ++SCCIdx, ++FunctionNo) {
    CallGraphNode *CGN = *SCCIdx;
    Function *F = CGN->getFunction();
    if (!F || F->isDeclaration()) continue;
    
    // Walk the function body looking for call sites.  Sync up the call sites in
    // CGN with those actually in the function.

    // Keep track of the number of direct and indirect calls that were
    // invalidated and removed.
    unsigned NumDirectRemoved = 0, NumIndirectRemoved = 0;
    
    // Get the set of call sites currently in the function.
    for (CallGraphNode::iterator I = CGN->begin(), E = CGN->end(); I != E; ) {
      // If this call site is null, then the function pass deleted the call
      // entirely and the WeakVH nulled it out.  
      if (!I->first ||
          // If we've already seen this call site, then the FunctionPass RAUW'd
          // one call with another, which resulted in two "uses" in the edge
          // list of the same call.
          CallSites.count(I->first) ||

          // If the call edge is not from a call or invoke, or it is a
          // instrinsic call, then the function pass RAUW'd a call with 
          // another value. This can happen when constant folding happens
          // of well known functions etc.
          !CallSite(I->first) ||
          (CallSite(I->first).getCalledFunction() &&
           CallSite(I->first).getCalledFunction()->isIntrinsic() &&
           Intrinsic::isLeaf(
               CallSite(I->first).getCalledFunction()->getIntrinsicID()))) {
        assert(!CheckingMode &&
               "CallGraphSCCPass did not update the CallGraph correctly!");
        
        // If this was an indirect call site, count it.
        if (!I->second->getFunction())
          ++NumIndirectRemoved;
        else 
          ++NumDirectRemoved;
        
        // Just remove the edge from the set of callees, keep track of whether
        // I points to the last element of the vector.
        bool WasLast = I + 1 == E;
        CGN->removeCallEdge(I);
        
        // If I pointed to the last element of the vector, we have to bail out:
        // iterator checking rejects comparisons of the resultant pointer with
        // end.
        if (WasLast)
          break;
        E = CGN->end();
        continue;
      }
      
      assert(!CallSites.count(I->first) &&
             "Call site occurs in node multiple times");
      
      CallSite CS(I->first);
      if (CS) {
        Function *Callee = CS.getCalledFunction();
        // Ignore intrinsics because they're not really function calls.
        if (!Callee || !(Callee->isIntrinsic()))
          CallSites.insert(std::make_pair(I->first, I->second));
      }
      ++I;
    }
    
    // Loop over all of the instructions in the function, getting the callsites.
    // Keep track of the number of direct/indirect calls added.
    unsigned NumDirectAdded = 0, NumIndirectAdded = 0;
    
    for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
      for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
        CallSite CS(cast<Value>(I));
        if (!CS) continue;
        Function *Callee = CS.getCalledFunction();
        if (Callee && Callee->isIntrinsic()) continue;
        
        // If this call site already existed in the callgraph, just verify it
        // matches up to expectations and remove it from CallSites.
        DenseMap<Value*, CallGraphNode*>::iterator ExistingIt =
          CallSites.find(CS.getInstruction());
        if (ExistingIt != CallSites.end()) {
          CallGraphNode *ExistingNode = ExistingIt->second;

          // Remove from CallSites since we have now seen it.
          CallSites.erase(ExistingIt);
          
          // Verify that the callee is right.
          if (ExistingNode->getFunction() == CS.getCalledFunction())
            continue;
          
          // If we are in checking mode, we are not allowed to actually mutate
          // the callgraph.  If this is a case where we can infer that the
          // callgraph is less precise than it could be (e.g. an indirect call
          // site could be turned direct), don't reject it in checking mode, and
          // don't tweak it to be more precise.
          if (CheckingMode && CS.getCalledFunction() &&
              ExistingNode->getFunction() == nullptr)
            continue;
          
          assert(!CheckingMode &&
                 "CallGraphSCCPass did not update the CallGraph correctly!");
          
          // If not, we either went from a direct call to indirect, indirect to
          // direct, or direct to different direct.
          CallGraphNode *CalleeNode;
          if (Function *Callee = CS.getCalledFunction()) {
            CalleeNode = CG.getOrInsertFunction(Callee);
            // Keep track of whether we turned an indirect call into a direct
            // one.
            if (!ExistingNode->getFunction()) {
              DevirtualizedCall = true;
              DEBUG(dbgs() << "  CGSCCPASSMGR: Devirtualized call to '"
                           << Callee->getName() << "'\n");
            }
          } else {
            CalleeNode = CG.getCallsExternalNode();
          }

          // Update the edge target in CGN.
          CGN->replaceCallEdge(CS, CS, CalleeNode);
          MadeChange = true;
          continue;
        }
        
        assert(!CheckingMode &&
               "CallGraphSCCPass did not update the CallGraph correctly!");

        // If the call site didn't exist in the CGN yet, add it.
        CallGraphNode *CalleeNode;
        if (Function *Callee = CS.getCalledFunction()) {
          CalleeNode = CG.getOrInsertFunction(Callee);
          ++NumDirectAdded;
        } else {
          CalleeNode = CG.getCallsExternalNode();
          ++NumIndirectAdded;
        }
        
        CGN->addCalledFunction(CS, CalleeNode);
        MadeChange = true;
      }
    
    // We scanned the old callgraph node, removing invalidated call sites and
    // then added back newly found call sites.  One thing that can happen is
    // that an old indirect call site was deleted and replaced with a new direct
    // call.  In this case, we have devirtualized a call, and CGSCCPM would like
    // to iteratively optimize the new code.  Unfortunately, we don't really
    // have a great way to detect when this happens.  As an approximation, we
    // just look at whether the number of indirect calls is reduced and the
    // number of direct calls is increased.  There are tons of ways to fool this
    // (e.g. DCE'ing an indirect call and duplicating an unrelated block with a
    // direct call) but this is close enough.
    if (NumIndirectRemoved > NumIndirectAdded &&
        NumDirectRemoved < NumDirectAdded)
      DevirtualizedCall = true;
    
    // After scanning this function, if we still have entries in callsites, then
    // they are dangling pointers.  WeakVH should save us for this, so abort if
    // this happens.
    assert(CallSites.empty() && "Dangling pointers found in call sites map");
    
    // Periodically do an explicit clear to remove tombstones when processing
    // large scc's.
    if ((FunctionNo & 15) == 15)
      CallSites.clear();
  }

  DEBUG(if (MadeChange) {
          dbgs() << "CGSCCPASSMGR: Refreshed SCC is now:\n";
          for (CallGraphNode *CGN : CurSCC)
            CGN->dump();
          if (DevirtualizedCall)
            dbgs() << "CGSCCPASSMGR: Refresh devirtualized a call!\n";

         } else {
           dbgs() << "CGSCCPASSMGR: SCC Refresh didn't change call graph.\n";
         }
        );
  (void)MadeChange;

  return DevirtualizedCall;
}

/// Execute the body of the entire pass manager on the specified SCC.
/// This keeps track of whether a function pass devirtualizes
/// any calls and returns it in DevirtualizedCall.
bool CGPassManager::RunAllPassesOnSCC(CallGraphSCC &CurSCC, CallGraph &CG,
                                      bool &DevirtualizedCall) {
  bool Changed = false;
  
  // Keep track of whether the callgraph is known to be up-to-date or not.
  // The CGSSC pass manager runs two types of passes:
  // CallGraphSCC Passes and other random function passes.  Because other
  // random function passes are not CallGraph aware, they may clobber the
  // call graph by introducing new calls or deleting other ones.  This flag
  // is set to false when we run a function pass so that we know to clean up
  // the callgraph when we need to run a CGSCCPass again.
  bool CallGraphUpToDate = true;

  // Run all passes on current SCC.
  for (unsigned PassNo = 0, e = getNumContainedPasses();
       PassNo != e; ++PassNo) {
    Pass *P = getContainedPass(PassNo);
    
    // If we're in -debug-pass=Executions mode, construct the SCC node list,
    // otherwise avoid constructing this string as it is expensive.
    if (isPassDebuggingExecutionsOrMore()) {
      std::string Functions;
  #ifndef NDEBUG
      raw_string_ostream OS(Functions);
      for (CallGraphSCC::iterator I = CurSCC.begin(), E = CurSCC.end();
           I != E; ++I) {
        if (I != CurSCC.begin()) OS << ", ";
        (*I)->print(OS);
      }
      OS.flush();
  #endif
      dumpPassInfo(P, EXECUTION_MSG, ON_CG_MSG, Functions);
    }
    dumpRequiredSet(P);
    
    initializeAnalysisImpl(P);
    
    // Actually run this pass on the current SCC.
    Changed |= RunPassOnSCC(P, CurSCC, CG,
                            CallGraphUpToDate, DevirtualizedCall);
    
    if (Changed)
      dumpPassInfo(P, MODIFICATION_MSG, ON_CG_MSG, "");
    dumpPreservedSet(P);
    
    verifyPreservedAnalysis(P);      
    removeNotPreservedAnalysis(P);
    recordAvailableAnalysis(P);
    removeDeadPasses(P, "", ON_CG_MSG);
  }
  
  // If the callgraph was left out of date (because the last pass run was a
  // functionpass), refresh it before we move on to the next SCC.
  if (!CallGraphUpToDate)
    DevirtualizedCall |= RefreshCallGraph(CurSCC, CG, false);
  return Changed;
}

/// Execute all of the passes scheduled for execution.  Keep track of
/// whether any of the passes modifies the module, and if so, return true.
bool CGPassManager::runOnModule(Module &M) {
  CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  bool Changed = doInitialization(CG);
  
  // Walk the callgraph in bottom-up SCC order.
  scc_iterator<CallGraph*> CGI = scc_begin(&CG);

  CallGraphSCC CurSCC(&CGI);
  while (!CGI.isAtEnd()) {
    // Copy the current SCC and increment past it so that the pass can hack
    // on the SCC if it wants to without invalidating our iterator.
    const std::vector<CallGraphNode *> &NodeVec = *CGI;
    CurSCC.initialize(NodeVec.data(), NodeVec.data() + NodeVec.size());
    ++CGI;

    // At the top level, we run all the passes in this pass manager on the
    // functions in this SCC.  However, we support iterative compilation in the
    // case where a function pass devirtualizes a call to a function.  For
    // example, it is very common for a function pass (often GVN or instcombine)
    // to eliminate the addressing that feeds into a call.  With that improved
    // information, we would like the call to be an inline candidate, infer
    // mod-ref information etc.
    //
    // Because of this, we allow iteration up to a specified iteration count.
    // This only happens in the case of a devirtualized call, so we only burn
    // compile time in the case that we're making progress.  We also have a hard
    // iteration count limit in case there is crazy code.
    unsigned Iteration = 0;
    bool DevirtualizedCall = false;
    do {
      DEBUG(if (Iteration)
              dbgs() << "  SCCPASSMGR: Re-visiting SCC, iteration #"
                     << Iteration << '\n');
      DevirtualizedCall = false;
      Changed |= RunAllPassesOnSCC(CurSCC, CG, DevirtualizedCall);
    } while (Iteration++ < MaxIterations && DevirtualizedCall);
    
    if (DevirtualizedCall)
      DEBUG(dbgs() << "  CGSCCPASSMGR: Stopped iteration after " << Iteration
                   << " times, due to -max-cg-scc-iterations\n");
    
    if (Iteration > MaxSCCIterations)
      MaxSCCIterations = Iteration;
    
  }
  Changed |= doFinalization(CG);
  return Changed;
}


/// Initialize CG
bool CGPassManager::doInitialization(CallGraph &CG) {
  bool Changed = false;
  for (unsigned i = 0, e = getNumContainedPasses(); i != e; ++i) {  
    if (PMDataManager *PM = getContainedPass(i)->getAsPMDataManager()) {
      assert(PM->getPassManagerType() == PMT_FunctionPassManager &&
             "Invalid CGPassManager member");
      Changed |= ((FPPassManager*)PM)->doInitialization(CG.getModule());
    } else {
      Changed |= ((CallGraphSCCPass*)getContainedPass(i))->doInitialization(CG);
    }
  }
  return Changed;
}

/// Finalize CG
bool CGPassManager::doFinalization(CallGraph &CG) {
  bool Changed = false;
  for (unsigned i = 0, e = getNumContainedPasses(); i != e; ++i) {  
    if (PMDataManager *PM = getContainedPass(i)->getAsPMDataManager()) {
      assert(PM->getPassManagerType() == PMT_FunctionPassManager &&
             "Invalid CGPassManager member");
      Changed |= ((FPPassManager*)PM)->doFinalization(CG.getModule());
    } else {
      Changed |= ((CallGraphSCCPass*)getContainedPass(i))->doFinalization(CG);
    }
  }
  return Changed;
}

//===----------------------------------------------------------------------===//
// CallGraphSCC Implementation
//===----------------------------------------------------------------------===//

/// This informs the SCC and the pass manager that the specified
/// Old node has been deleted, and New is to be used in its place.
void CallGraphSCC::ReplaceNode(CallGraphNode *Old, CallGraphNode *New) {
  assert(Old != New && "Should not replace node with self");
  for (unsigned i = 0; ; ++i) {
    assert(i != Nodes.size() && "Node not in SCC");
    if (Nodes[i] != Old) continue;
    Nodes[i] = New;
    break;
  }
  
  // Update the active scc_iterator so that it doesn't contain dangling
  // pointers to the old CallGraphNode.
  scc_iterator<CallGraph*> *CGI = (scc_iterator<CallGraph*>*)Context;
  CGI->ReplaceNode(Old, New);
}


//===----------------------------------------------------------------------===//
// CallGraphSCCPass Implementation
//===----------------------------------------------------------------------===//

/// Assign pass manager to manage this pass.
void CallGraphSCCPass::assignPassManager(PMStack &PMS,
                                         PassManagerType PreferredType) {
  std::unique_ptr<CallGraphSCCPass> thisPtr(this); // HLSL Change
  // Find CGPassManager 
  while (!PMS.empty() &&
         PMS.top()->getPassManagerType() > PMT_CallGraphPassManager)
    PMS.pop();

  assert(!PMS.empty() && "Unable to handle Call Graph Pass");
  CGPassManager *CGP;
  
  if (PMS.top()->getPassManagerType() == PMT_CallGraphPassManager)
    CGP = (CGPassManager*)PMS.top();
  else {
    // Create new Call Graph SCC Pass Manager if it does not exist. 
    assert(!PMS.empty() && "Unable to create Call Graph Pass Manager");
    PMDataManager *PMD = PMS.top();

    // [1] Create new Call Graph Pass Manager
    CGP = new CGPassManager();

    // [2] Set up new manager's top level manager
    PMTopLevelManager *TPM = PMD->getTopLevelManager();
    TPM->addIndirectPassManager(CGP);

    // [3] Assign manager to manage this new manager. This may create
    // and push new managers into PMS
    Pass *P = CGP;
    TPM->schedulePass(P);

    // [4] Push new manager into PMS
    PMS.push(CGP);
  }

  thisPtr.release();
  CGP->add(this);
}

/// For this class, we declare that we require and preserve the call graph.
/// If the derived class implements this method, it should
/// always explicitly call the implementation here.
void CallGraphSCCPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<CallGraphWrapperPass>();
  AU.addPreserved<CallGraphWrapperPass>();
}


//===----------------------------------------------------------------------===//
// PrintCallGraphPass Implementation
//===----------------------------------------------------------------------===//

namespace {
  /// PrintCallGraphPass - Print a Module corresponding to a call graph.
  ///
  class PrintCallGraphPass : public CallGraphSCCPass {
    std::string Banner;
    raw_ostream &Out;       // raw_ostream to print on.
    
  public:
    static char ID;
    PrintCallGraphPass(const std::string &B, raw_ostream &o)
      : CallGraphSCCPass(ID), Banner(B), Out(o) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    bool runOnSCC(CallGraphSCC &SCC) override {
      Out << Banner;
      for (CallGraphNode *CGN : SCC) {
        if (CGN->getFunction())
          CGN->getFunction()->print(Out);
        else
          Out << "\nPrinting <null> Function\n";
      }
      return false;
    }
  };
  
} // end anonymous namespace.

char PrintCallGraphPass::ID = 0;

Pass *CallGraphSCCPass::createPrinterPass(raw_ostream &O,
                                          const std::string &Banner) const {
  return new PrintCallGraphPass(Banner, O);
}

