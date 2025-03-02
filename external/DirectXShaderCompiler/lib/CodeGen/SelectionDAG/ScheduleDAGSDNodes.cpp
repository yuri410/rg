//===--- ScheduleDAGSDNodes.cpp - Implement the ScheduleDAGSDNodes class --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This implements the ScheduleDAG class, which is a base class used by
// scheduling implementation classes.
//
//===----------------------------------------------------------------------===//

#include "ScheduleDAGSDNodes.h"
#include "InstrEmitter.h"
#include "SDNodeDbgValue.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
using namespace llvm;

#define DEBUG_TYPE "pre-RA-sched"

STATISTIC(LoadsClustered, "Number of loads clustered together");

// This allows the latency-based scheduler to notice high latency instructions
// without a target itinerary. The choice of number here has more to do with
// balancing scheduler heuristics than with the actual machine latency.
static cl::opt<int> HighLatencyCycles(
  "sched-high-latency-cycles", cl::Hidden, cl::init(10),
  cl::desc("Roughly estimate the number of cycles that 'long latency'"
           "instructions take for targets with no itinerary"));

ScheduleDAGSDNodes::ScheduleDAGSDNodes(MachineFunction &mf)
    : ScheduleDAG(mf), BB(nullptr), DAG(nullptr),
      InstrItins(mf.getSubtarget().getInstrItineraryData()) {}

/// Run - perform scheduling.
///
void ScheduleDAGSDNodes::Run(SelectionDAG *dag, MachineBasicBlock *bb) {
  BB = bb;
  DAG = dag;

  // Clear the scheduler's SUnit DAG.
  ScheduleDAG::clearDAG();
  Sequence.clear();

  // Invoke the target's selection of scheduler.
  Schedule();
}

/// NewSUnit - Creates a new SUnit and return a ptr to it.
///
SUnit *ScheduleDAGSDNodes::newSUnit(SDNode *N) {
#ifndef NDEBUG
  const SUnit *Addr = nullptr;
  if (!SUnits.empty())
    Addr = &SUnits[0];
#endif
  SUnits.emplace_back(N, (unsigned)SUnits.size());
  assert((Addr == nullptr || Addr == &SUnits[0]) &&
         "SUnits std::vector reallocated on the fly!");
  SUnits.back().OrigNode = &SUnits.back();
  SUnit *SU = &SUnits.back();
  const TargetLowering &TLI = DAG->getTargetLoweringInfo();
  if (!N ||
      (N->isMachineOpcode() &&
       N->getMachineOpcode() == TargetOpcode::IMPLICIT_DEF))
    SU->SchedulingPref = Sched::None;
  else
    SU->SchedulingPref = TLI.getSchedulingPreference(N);
  return SU;
}

SUnit *ScheduleDAGSDNodes::Clone(SUnit *Old) {
  SUnit *SU = newSUnit(Old->getNode());
  SU->OrigNode = Old->OrigNode;
  SU->Latency = Old->Latency;
  SU->isVRegCycle = Old->isVRegCycle;
  SU->isCall = Old->isCall;
  SU->isCallOp = Old->isCallOp;
  SU->isTwoAddress = Old->isTwoAddress;
  SU->isCommutable = Old->isCommutable;
  SU->hasPhysRegDefs = Old->hasPhysRegDefs;
  SU->hasPhysRegClobbers = Old->hasPhysRegClobbers;
  SU->isScheduleHigh = Old->isScheduleHigh;
  SU->isScheduleLow = Old->isScheduleLow;
  SU->SchedulingPref = Old->SchedulingPref;
  Old->isCloned = true;
  return SU;
}

/// CheckForPhysRegDependency - Check if the dependency between def and use of
/// a specified operand is a physical register dependency. If so, returns the
/// register and the cost of copying the register.
static void CheckForPhysRegDependency(SDNode *Def, SDNode *User, unsigned Op,
                                      const TargetRegisterInfo *TRI,
                                      const TargetInstrInfo *TII,
                                      unsigned &PhysReg, int &Cost) {
  if (Op != 2 || User->getOpcode() != ISD::CopyToReg)
    return;

  unsigned Reg = cast<RegisterSDNode>(User->getOperand(1))->getReg();
  if (TargetRegisterInfo::isVirtualRegister(Reg))
    return;

  unsigned ResNo = User->getOperand(2).getResNo();
  if (Def->getOpcode() == ISD::CopyFromReg &&
      cast<RegisterSDNode>(Def->getOperand(1))->getReg() == Reg) {
    PhysReg = Reg;
  } else if (Def->isMachineOpcode()) {
    const MCInstrDesc &II = TII->get(Def->getMachineOpcode());
    if (ResNo >= II.getNumDefs() &&
        II.ImplicitDefs[ResNo - II.getNumDefs()] == Reg)
      PhysReg = Reg;
  }

  if (PhysReg != 0) {
    const TargetRegisterClass *RC =
        TRI->getMinimalPhysRegClass(Reg, Def->getSimpleValueType(ResNo));
    Cost = RC->getCopyCost();
  }
}

// Helper for AddGlue to clone node operands.
static void CloneNodeWithValues(SDNode *N, SelectionDAG *DAG, ArrayRef<EVT> VTs,
                                SDValue ExtraOper = SDValue()) {
  SmallVector<SDValue, 8> Ops(N->op_begin(), N->op_end());
  if (ExtraOper.getNode())
    Ops.push_back(ExtraOper);

  SDVTList VTList = DAG->getVTList(VTs);
  MachineSDNode::mmo_iterator Begin = nullptr, End = nullptr;
  MachineSDNode *MN = dyn_cast<MachineSDNode>(N);

  // Store memory references.
  if (MN) {
    Begin = MN->memoperands_begin();
    End = MN->memoperands_end();
  }

  DAG->MorphNodeTo(N, N->getOpcode(), VTList, Ops);

  // Reset the memory references
  if (MN)
    MN->setMemRefs(Begin, End);
}

static bool AddGlue(SDNode *N, SDValue Glue, bool AddGlue, SelectionDAG *DAG) {
  SDNode *GlueDestNode = Glue.getNode();

  // Don't add glue from a node to itself.
  if (GlueDestNode == N) return false;

  // Don't add a glue operand to something that already uses glue.
  if (GlueDestNode &&
      N->getOperand(N->getNumOperands()-1).getValueType() == MVT::Glue) {
    return false;
  }
  // Don't add glue to something that already has a glue value.
  if (N->getValueType(N->getNumValues() - 1) == MVT::Glue) return false;

  SmallVector<EVT, 4> VTs(N->value_begin(), N->value_end());
  if (AddGlue)
    VTs.push_back(MVT::Glue);

  CloneNodeWithValues(N, DAG, VTs, Glue);

  return true;
}

// Cleanup after unsuccessful AddGlue. Use the standard method of morphing the
// node even though simply shrinking the value list is sufficient.
static void RemoveUnusedGlue(SDNode *N, SelectionDAG *DAG) {
  assert((N->getValueType(N->getNumValues() - 1) == MVT::Glue &&
          !N->hasAnyUseOfValue(N->getNumValues() - 1)) &&
         "expected an unused glue value");

  CloneNodeWithValues(N, DAG,
                      makeArrayRef(N->value_begin(), N->getNumValues() - 1));
}

/// ClusterNeighboringLoads - Force nearby loads together by "gluing" them.
/// This function finds loads of the same base and different offsets. If the
/// offsets are not far apart (target specific), it add MVT::Glue inputs and
/// outputs to ensure they are scheduled together and in order. This
/// optimization may benefit some targets by improving cache locality.
void ScheduleDAGSDNodes::ClusterNeighboringLoads(SDNode *Node) {
  SDNode *Chain = nullptr;
  unsigned NumOps = Node->getNumOperands();
  if (Node->getOperand(NumOps-1).getValueType() == MVT::Other)
    Chain = Node->getOperand(NumOps-1).getNode();
  if (!Chain)
    return;

  // Look for other loads of the same chain. Find loads that are loading from
  // the same base pointer and different offsets.
  SmallPtrSet<SDNode*, 16> Visited;
  SmallVector<int64_t, 4> Offsets;
  DenseMap<long long, SDNode*> O2SMap;  // Map from offset to SDNode.
  bool Cluster = false;
  SDNode *Base = Node;
  // This algorithm requires a reasonably low use count before finding a match
  // to avoid uselessly blowing up compile time in large blocks.
  unsigned UseCount = 0;
  for (SDNode::use_iterator I = Chain->use_begin(), E = Chain->use_end();
       I != E && UseCount < 100; ++I, ++UseCount) {
    SDNode *User = *I;
    if (User == Node || !Visited.insert(User).second)
      continue;
    int64_t Offset1, Offset2;
    if (!TII->areLoadsFromSameBasePtr(Base, User, Offset1, Offset2) ||
        Offset1 == Offset2)
      // FIXME: Should be ok if they addresses are identical. But earlier
      // optimizations really should have eliminated one of the loads.
      continue;
    if (O2SMap.insert(std::make_pair(Offset1, Base)).second)
      Offsets.push_back(Offset1);
    O2SMap.insert(std::make_pair(Offset2, User));
    Offsets.push_back(Offset2);
    if (Offset2 < Offset1)
      Base = User;
    Cluster = true;
    // Reset UseCount to allow more matches.
    UseCount = 0;
  }

  if (!Cluster)
    return;

  // Sort them in increasing order.
  std::sort(Offsets.begin(), Offsets.end());

  // Check if the loads are close enough.
  SmallVector<SDNode*, 4> Loads;
  unsigned NumLoads = 0;
  int64_t BaseOff = Offsets[0];
  SDNode *BaseLoad = O2SMap[BaseOff];
  Loads.push_back(BaseLoad);
  for (unsigned i = 1, e = Offsets.size(); i != e; ++i) {
    int64_t Offset = Offsets[i];
    SDNode *Load = O2SMap[Offset];
    if (!TII->shouldScheduleLoadsNear(BaseLoad, Load, BaseOff, Offset,NumLoads))
      break; // Stop right here. Ignore loads that are further away.
    Loads.push_back(Load);
    ++NumLoads;
  }

  if (NumLoads == 0)
    return;

  // Cluster loads by adding MVT::Glue outputs and inputs. This also
  // ensure they are scheduled in order of increasing addresses.
  SDNode *Lead = Loads[0];
  SDValue InGlue = SDValue(nullptr, 0);
  if (AddGlue(Lead, InGlue, true, DAG))
    InGlue = SDValue(Lead, Lead->getNumValues() - 1);
  for (unsigned I = 1, E = Loads.size(); I != E; ++I) {
    bool OutGlue = I < E - 1;
    SDNode *Load = Loads[I];

    // If AddGlue fails, we could leave an unsused glue value. This should not
    // cause any
    if (AddGlue(Load, InGlue, OutGlue, DAG)) {
      if (OutGlue)
        InGlue = SDValue(Load, Load->getNumValues() - 1);

      ++LoadsClustered;
    }
    else if (!OutGlue && InGlue.getNode())
      RemoveUnusedGlue(InGlue.getNode(), DAG);
  }
}

/// ClusterNodes - Cluster certain nodes which should be scheduled together.
///
void ScheduleDAGSDNodes::ClusterNodes() {
  for (SDNode &NI : DAG->allnodes()) {
    SDNode *Node = &NI;
    if (!Node || !Node->isMachineOpcode())
      continue;

    unsigned Opc = Node->getMachineOpcode();
    const MCInstrDesc &MCID = TII->get(Opc);
    if (MCID.mayLoad())
      // Cluster loads from "near" addresses into combined SUnits.
      ClusterNeighboringLoads(Node);
  }
}

void ScheduleDAGSDNodes::BuildSchedUnits() {
  // During scheduling, the NodeId field of SDNode is used to map SDNodes
  // to their associated SUnits by holding SUnits table indices. A value
  // of -1 means the SDNode does not yet have an associated SUnit.
  unsigned NumNodes = 0;
  for (SDNode &NI : DAG->allnodes()) {
    NI.setNodeId(-1);
    ++NumNodes;
  }

  // Reserve entries in the vector for each of the SUnits we are creating.  This
  // ensure that reallocation of the vector won't happen, so SUnit*'s won't get
  // invalidated.
  // FIXME: Multiply by 2 because we may clone nodes during scheduling.
  // This is a temporary workaround.
  SUnits.reserve(NumNodes * 2);

  // Add all nodes in depth first order.
  SmallVector<SDNode*, 64> Worklist;
  SmallPtrSet<SDNode*, 64> Visited;
  Worklist.push_back(DAG->getRoot().getNode());
  Visited.insert(DAG->getRoot().getNode());

  SmallVector<SUnit*, 8> CallSUnits;
  while (!Worklist.empty()) {
    SDNode *NI = Worklist.pop_back_val();

    // Add all operands to the worklist unless they've already been added.
    for (const SDValue &Op : NI->op_values())
      if (Visited.insert(Op.getNode()).second)
        Worklist.push_back(Op.getNode());

    if (isPassiveNode(NI))  // Leaf node, e.g. a TargetImmediate.
      continue;

    // If this node has already been processed, stop now.
    if (NI->getNodeId() != -1) continue;

    SUnit *NodeSUnit = newSUnit(NI);

    // See if anything is glued to this node, if so, add them to glued
    // nodes.  Nodes can have at most one glue input and one glue output.  Glue
    // is required to be the last operand and result of a node.

    // Scan up to find glued preds.
    SDNode *N = NI;
    while (N->getNumOperands() &&
           N->getOperand(N->getNumOperands()-1).getValueType() == MVT::Glue) {
      N = N->getOperand(N->getNumOperands()-1).getNode();
      assert(N->getNodeId() == -1 && "Node already inserted!");
      N->setNodeId(NodeSUnit->NodeNum);
      if (N->isMachineOpcode() && TII->get(N->getMachineOpcode()).isCall())
        NodeSUnit->isCall = true;
    }

    // Scan down to find any glued succs.
    N = NI;
    while (N->getValueType(N->getNumValues()-1) == MVT::Glue) {
      SDValue GlueVal(N, N->getNumValues()-1);

      // There are either zero or one users of the Glue result.
      bool HasGlueUse = false;
      for (SDNode::use_iterator UI = N->use_begin(), E = N->use_end();
           UI != E; ++UI)
        if (GlueVal.isOperandOf(*UI)) {
          HasGlueUse = true;
          assert(N->getNodeId() == -1 && "Node already inserted!");
          N->setNodeId(NodeSUnit->NodeNum);
          N = *UI;
          if (N->isMachineOpcode() && TII->get(N->getMachineOpcode()).isCall())
            NodeSUnit->isCall = true;
          break;
        }
      if (!HasGlueUse) break;
    }

    if (NodeSUnit->isCall)
      CallSUnits.push_back(NodeSUnit);

    // Schedule zero-latency TokenFactor below any nodes that may increase the
    // schedule height. Otherwise, ancestors of the TokenFactor may appear to
    // have false stalls.
    if (NI->getOpcode() == ISD::TokenFactor)
      NodeSUnit->isScheduleLow = true;

    // If there are glue operands involved, N is now the bottom-most node
    // of the sequence of nodes that are glued together.
    // Update the SUnit.
    NodeSUnit->setNode(N);
    assert(N->getNodeId() == -1 && "Node already inserted!");
    N->setNodeId(NodeSUnit->NodeNum);

    // Compute NumRegDefsLeft. This must be done before AddSchedEdges.
    InitNumRegDefsLeft(NodeSUnit);

    // Assign the Latency field of NodeSUnit using target-provided information.
    computeLatency(NodeSUnit);
  }

  // Find all call operands.
  while (!CallSUnits.empty()) {
    SUnit *SU = CallSUnits.pop_back_val();
    for (const SDNode *SUNode = SU->getNode(); SUNode;
         SUNode = SUNode->getGluedNode()) {
      if (SUNode->getOpcode() != ISD::CopyToReg)
        continue;
      SDNode *SrcN = SUNode->getOperand(2).getNode();
      if (isPassiveNode(SrcN)) continue;   // Not scheduled.
      SUnit *SrcSU = &SUnits[SrcN->getNodeId()];
      SrcSU->isCallOp = true;
    }
  }
}

void ScheduleDAGSDNodes::AddSchedEdges() {
  const TargetSubtargetInfo &ST = MF.getSubtarget();

  // Check to see if the scheduler cares about latencies.
  bool UnitLatencies = forceUnitLatencies();

  // Pass 2: add the preds, succs, etc.
  for (unsigned su = 0, e = SUnits.size(); su != e; ++su) {
    SUnit *SU = &SUnits[su];
    SDNode *MainNode = SU->getNode();

    if (MainNode->isMachineOpcode()) {
      unsigned Opc = MainNode->getMachineOpcode();
      const MCInstrDesc &MCID = TII->get(Opc);
      for (unsigned i = 0; i != MCID.getNumOperands(); ++i) {
        if (MCID.getOperandConstraint(i, MCOI::TIED_TO) != -1) {
          SU->isTwoAddress = true;
          break;
        }
      }
      if (MCID.isCommutable())
        SU->isCommutable = true;
    }

    // Find all predecessors and successors of the group.
    for (SDNode *N = SU->getNode(); N; N = N->getGluedNode()) {
      if (N->isMachineOpcode() &&
          TII->get(N->getMachineOpcode()).getImplicitDefs()) {
        SU->hasPhysRegClobbers = true;
        unsigned NumUsed = InstrEmitter::CountResults(N);
        while (NumUsed != 0 && !N->hasAnyUseOfValue(NumUsed - 1))
          --NumUsed;    // Skip over unused values at the end.
        if (NumUsed > TII->get(N->getMachineOpcode()).getNumDefs())
          SU->hasPhysRegDefs = true;
      }

      for (unsigned i = 0, e = N->getNumOperands(); i != e; ++i) {
        SDNode *OpN = N->getOperand(i).getNode();
        if (isPassiveNode(OpN)) continue;   // Not scheduled.
        SUnit *OpSU = &SUnits[OpN->getNodeId()];
        assert(OpSU && "Node has no SUnit!");
        if (OpSU == SU) continue;           // In the same group.

        EVT OpVT = N->getOperand(i).getValueType();
        assert(OpVT != MVT::Glue && "Glued nodes should be in same sunit!");
        bool isChain = OpVT == MVT::Other;

        unsigned PhysReg = 0;
        int Cost = 1;
        // Determine if this is a physical register dependency.
        CheckForPhysRegDependency(OpN, N, i, TRI, TII, PhysReg, Cost);
        assert((PhysReg == 0 || !isChain) &&
               "Chain dependence via physreg data?");
        // FIXME: See ScheduleDAGSDNodes::EmitCopyFromReg. For now, scheduler
        // emits a copy from the physical register to a virtual register unless
        // it requires a cross class copy (cost < 0). That means we are only
        // treating "expensive to copy" register dependency as physical register
        // dependency. This may change in the future though.
        if (Cost >= 0 && !StressSched)
          PhysReg = 0;

        // If this is a ctrl dep, latency is 1.
        unsigned OpLatency = isChain ? 1 : OpSU->Latency;
        // Special-case TokenFactor chains as zero-latency.
        if(isChain && OpN->getOpcode() == ISD::TokenFactor)
          OpLatency = 0;

        SDep Dep = isChain ? SDep(OpSU, SDep::Barrier)
          : SDep(OpSU, SDep::Data, PhysReg);
        Dep.setLatency(OpLatency);
        if (!isChain && !UnitLatencies) {
          computeOperandLatency(OpN, N, i, Dep);
          ST.adjustSchedDependency(OpSU, SU, Dep);
        }

        if (!SU->addPred(Dep) && !Dep.isCtrl() && OpSU->NumRegDefsLeft > 1) {
          // Multiple register uses are combined in the same SUnit. For example,
          // we could have a set of glued nodes with all their defs consumed by
          // another set of glued nodes. Register pressure tracking sees this as
          // a single use, so to keep pressure balanced we reduce the defs.
          //
          // We can't tell (without more book-keeping) if this results from
          // glued nodes or duplicate operands. As long as we don't reduce
          // NumRegDefsLeft to zero, we handle the common cases well.
          --OpSU->NumRegDefsLeft;
        }
      }
    }
  }
}

/// BuildSchedGraph - Build the SUnit graph from the selection dag that we
/// are input.  This SUnit graph is similar to the SelectionDAG, but
/// excludes nodes that aren't interesting to scheduling, and represents
/// glued together nodes with a single SUnit.
void ScheduleDAGSDNodes::BuildSchedGraph(AliasAnalysis *AA) {
  // Cluster certain nodes which should be scheduled together.
  ClusterNodes();
  // Populate the SUnits array.
  BuildSchedUnits();
  // Compute all the scheduling dependencies between nodes.
  AddSchedEdges();
}

// Initialize NumNodeDefs for the current Node's opcode.
void ScheduleDAGSDNodes::RegDefIter::InitNodeNumDefs() {
  // Check for phys reg copy.
  if (!Node)
    return;

  if (!Node->isMachineOpcode()) {
    if (Node->getOpcode() == ISD::CopyFromReg)
      NodeNumDefs = 1;
    else
      NodeNumDefs = 0;
    return;
  }
  unsigned POpc = Node->getMachineOpcode();
  if (POpc == TargetOpcode::IMPLICIT_DEF) {
    // No register need be allocated for this.
    NodeNumDefs = 0;
    return;
  }
  if (POpc == TargetOpcode::PATCHPOINT &&
      Node->getValueType(0) == MVT::Other) {
    // PATCHPOINT is defined to have one result, but it might really have none
    // if we're not using CallingConv::AnyReg. Don't mistake the chain for a
    // real definition.
    NodeNumDefs = 0;
    return;
  }
  unsigned NRegDefs = SchedDAG->TII->get(Node->getMachineOpcode()).getNumDefs();
  // Some instructions define regs that are not represented in the selection DAG
  // (e.g. unused flags). See tMOVi8. Make sure we don't access past NumValues.
  NodeNumDefs = std::min(Node->getNumValues(), NRegDefs);
  DefIdx = 0;
}

// Construct a RegDefIter for this SUnit and find the first valid value.
ScheduleDAGSDNodes::RegDefIter::RegDefIter(const SUnit *SU,
                                           const ScheduleDAGSDNodes *SD)
  : SchedDAG(SD), Node(SU->getNode()), DefIdx(0), NodeNumDefs(0) {
  InitNodeNumDefs();
  Advance();
}

// Advance to the next valid value defined by the SUnit.
void ScheduleDAGSDNodes::RegDefIter::Advance() {
  for (;Node;) { // Visit all glued nodes.
    for (;DefIdx < NodeNumDefs; ++DefIdx) {
      if (!Node->hasAnyUseOfValue(DefIdx))
        continue;
      ValueType = Node->getSimpleValueType(DefIdx);
      ++DefIdx;
      return; // Found a normal regdef.
    }
    Node = Node->getGluedNode();
    if (!Node) {
      return; // No values left to visit.
    }
    InitNodeNumDefs();
  }
}

void ScheduleDAGSDNodes::InitNumRegDefsLeft(SUnit *SU) {
  assert(SU->NumRegDefsLeft == 0 && "expect a new node");
  for (RegDefIter I(SU, this); I.IsValid(); I.Advance()) {
    assert(SU->NumRegDefsLeft < USHRT_MAX && "overflow is ok but unexpected");
    ++SU->NumRegDefsLeft;
  }
}

void ScheduleDAGSDNodes::computeLatency(SUnit *SU) {
  SDNode *N = SU->getNode();

  // TokenFactor operands are considered zero latency, and some schedulers
  // (e.g. Top-Down list) may rely on the fact that operand latency is nonzero
  // whenever node latency is nonzero.
  if (N && N->getOpcode() == ISD::TokenFactor) {
    SU->Latency = 0;
    return;
  }

  // Check to see if the scheduler cares about latencies.
  if (forceUnitLatencies()) {
    SU->Latency = 1;
    return;
  }

  if (!InstrItins || InstrItins->isEmpty()) {
    if (N && N->isMachineOpcode() &&
        TII->isHighLatencyDef(N->getMachineOpcode()))
      SU->Latency = HighLatencyCycles;
    else
      SU->Latency = 1;
    return;
  }

  // Compute the latency for the node.  We use the sum of the latencies for
  // all nodes glued together into this SUnit.
  SU->Latency = 0;
  for (SDNode *N = SU->getNode(); N; N = N->getGluedNode())
    if (N->isMachineOpcode())
      SU->Latency += TII->getInstrLatency(InstrItins, N);
}

void ScheduleDAGSDNodes::computeOperandLatency(SDNode *Def, SDNode *Use,
                                               unsigned OpIdx, SDep& dep) const{
  // Check to see if the scheduler cares about latencies.
  if (forceUnitLatencies())
    return;

  if (dep.getKind() != SDep::Data)
    return;

  unsigned DefIdx = Use->getOperand(OpIdx).getResNo();
  if (Use->isMachineOpcode())
    // Adjust the use operand index by num of defs.
    OpIdx += TII->get(Use->getMachineOpcode()).getNumDefs();
  int Latency = TII->getOperandLatency(InstrItins, Def, DefIdx, Use, OpIdx);
  if (Latency > 1 && Use->getOpcode() == ISD::CopyToReg &&
      !BB->succ_empty()) {
    unsigned Reg = cast<RegisterSDNode>(Use->getOperand(1))->getReg();
    if (TargetRegisterInfo::isVirtualRegister(Reg))
      // This copy is a liveout value. It is likely coalesced, so reduce the
      // latency so not to penalize the def.
      // FIXME: need target specific adjustment here?
      Latency = (Latency > 1) ? Latency - 1 : 1;
  }
  if (Latency >= 0)
    dep.setLatency(Latency);
}

void ScheduleDAGSDNodes::dumpNode(const SUnit *SU) const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (!SU->getNode()) {
    dbgs() << "PHYS REG COPY\n";
    return;
  }

  SU->getNode()->dump(DAG);
  dbgs() << "\n";
  SmallVector<SDNode *, 4> GluedNodes;
  for (SDNode *N = SU->getNode()->getGluedNode(); N; N = N->getGluedNode())
    GluedNodes.push_back(N);
  while (!GluedNodes.empty()) {
    dbgs() << "    ";
    GluedNodes.back()->dump(DAG);
    dbgs() << "\n";
    GluedNodes.pop_back();
  }
#endif
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ScheduleDAGSDNodes::dumpSchedule() const {
  for (unsigned i = 0, e = Sequence.size(); i != e; i++) {
    if (SUnit *SU = Sequence[i])
      SU->dump(this);
    else
      dbgs() << "**** NOOP ****\n";
  }
}
#endif

#ifndef NDEBUG
/// VerifyScheduledSequence - Verify that all SUnits were scheduled and that
/// their state is consistent with the nodes listed in Sequence.
///
void ScheduleDAGSDNodes::VerifyScheduledSequence(bool isBottomUp) {
  unsigned ScheduledNodes = ScheduleDAG::VerifyScheduledDAG(isBottomUp);
  unsigned Noops = 0;
  for (unsigned i = 0, e = Sequence.size(); i != e; ++i)
    if (!Sequence[i])
      ++Noops;
  assert(Sequence.size() - Noops == ScheduledNodes &&
         "The number of nodes scheduled doesn't match the expected number!");
}
#endif // NDEBUG

/// ProcessSDDbgValues - Process SDDbgValues associated with this node.
static void
ProcessSDDbgValues(SDNode *N, SelectionDAG *DAG, InstrEmitter &Emitter,
                   SmallVectorImpl<std::pair<unsigned, MachineInstr*> > &Orders,
                   DenseMap<SDValue, unsigned> &VRBaseMap, unsigned Order) {
  if (!N->getHasDebugValue())
    return;

  // Opportunistically insert immediate dbg_value uses, i.e. those with source
  // order number right after the N.
  MachineBasicBlock *BB = Emitter.getBlock();
  MachineBasicBlock::iterator InsertPos = Emitter.getInsertPos();
  ArrayRef<SDDbgValue*> DVs = DAG->GetDbgValues(N);
  for (unsigned i = 0, e = DVs.size(); i != e; ++i) {
    if (DVs[i]->isInvalidated())
      continue;
    unsigned DVOrder = DVs[i]->getOrder();
    if (!Order || DVOrder == ++Order) {
      MachineInstr *DbgMI = Emitter.EmitDbgValue(DVs[i], VRBaseMap);
      if (DbgMI) {
        Orders.push_back(std::make_pair(DVOrder, DbgMI));
        BB->insert(InsertPos, DbgMI);
      }
      DVs[i]->setIsInvalidated();
    }
  }
}

// ProcessSourceNode - Process nodes with source order numbers. These are added
// to a vector which EmitSchedule uses to determine how to insert dbg_value
// instructions in the right order.
static void
ProcessSourceNode(SDNode *N, SelectionDAG *DAG, InstrEmitter &Emitter,
                  DenseMap<SDValue, unsigned> &VRBaseMap,
                  SmallVectorImpl<std::pair<unsigned, MachineInstr*> > &Orders,
                  SmallSet<unsigned, 8> &Seen) {
  unsigned Order = N->getIROrder();
  if (!Order || !Seen.insert(Order).second) {
    // Process any valid SDDbgValues even if node does not have any order
    // assigned.
    ProcessSDDbgValues(N, DAG, Emitter, Orders, VRBaseMap, 0);
    return;
  }

  MachineBasicBlock *BB = Emitter.getBlock();
  if (Emitter.getInsertPos() == BB->begin() || BB->back().isPHI() ||
      // Fast-isel may have inserted some instructions, in which case the
      // BB->back().isPHI() test will not fire when we want it to.
      std::prev(Emitter.getInsertPos())->isPHI()) {
    // Did not insert any instruction.
    Orders.push_back(std::make_pair(Order, (MachineInstr*)nullptr));
    return;
  }

  Orders.push_back(std::make_pair(Order, std::prev(Emitter.getInsertPos())));
  ProcessSDDbgValues(N, DAG, Emitter, Orders, VRBaseMap, Order);
}

void ScheduleDAGSDNodes::
EmitPhysRegCopy(SUnit *SU, DenseMap<SUnit*, unsigned> &VRBaseMap,
                MachineBasicBlock::iterator InsertPos) {
  for (SUnit::const_pred_iterator I = SU->Preds.begin(), E = SU->Preds.end();
       I != E; ++I) {
    if (I->isCtrl()) continue;  // ignore chain preds
    if (I->getSUnit()->CopyDstRC) {
      // Copy to physical register.
      DenseMap<SUnit*, unsigned>::iterator VRI = VRBaseMap.find(I->getSUnit());
      assert(VRI != VRBaseMap.end() && "Node emitted out of order - late");
      // Find the destination physical register.
      unsigned Reg = 0;
      for (SUnit::const_succ_iterator II = SU->Succs.begin(),
             EE = SU->Succs.end(); II != EE; ++II) {
        if (II->isCtrl()) continue;  // ignore chain preds
        if (II->getReg()) {
          Reg = II->getReg();
          break;
        }
      }
      BuildMI(*BB, InsertPos, DebugLoc(), TII->get(TargetOpcode::COPY), Reg)
        .addReg(VRI->second);
    } else {
      // Copy from physical register.
      assert(I->getReg() && "Unknown physical register!");
      unsigned VRBase = MRI.createVirtualRegister(SU->CopyDstRC);
      bool isNew = VRBaseMap.insert(std::make_pair(SU, VRBase)).second;
      (void)isNew; // Silence compiler warning.
      assert(isNew && "Node emitted out of order - early");
      BuildMI(*BB, InsertPos, DebugLoc(), TII->get(TargetOpcode::COPY), VRBase)
        .addReg(I->getReg());
    }
    break;
  }
}

/// EmitSchedule - Emit the machine code in scheduled order. Return the new
/// InsertPos and MachineBasicBlock that contains this insertion
/// point. ScheduleDAGSDNodes holds a BB pointer for convenience, but this does
/// not necessarily refer to returned BB. The emitter may split blocks.
MachineBasicBlock *ScheduleDAGSDNodes::
EmitSchedule(MachineBasicBlock::iterator &InsertPos) {
  InstrEmitter Emitter(BB, InsertPos);
  DenseMap<SDValue, unsigned> VRBaseMap;
  DenseMap<SUnit*, unsigned> CopyVRBaseMap;
  SmallVector<std::pair<unsigned, MachineInstr*>, 32> Orders;
  SmallSet<unsigned, 8> Seen;
  bool HasDbg = DAG->hasDebugValues();

  // If this is the first BB, emit byval parameter dbg_value's.
  if (HasDbg && BB->getParent()->begin() == MachineFunction::iterator(BB)) {
    SDDbgInfo::DbgIterator PDI = DAG->ByvalParmDbgBegin();
    SDDbgInfo::DbgIterator PDE = DAG->ByvalParmDbgEnd();
    for (; PDI != PDE; ++PDI) {
      MachineInstr *DbgMI= Emitter.EmitDbgValue(*PDI, VRBaseMap);
      if (DbgMI)
        BB->insert(InsertPos, DbgMI);
    }
  }

  for (unsigned i = 0, e = Sequence.size(); i != e; i++) {
    SUnit *SU = Sequence[i];
    if (!SU) {
      // Null SUnit* is a noop.
      TII->insertNoop(*Emitter.getBlock(), InsertPos);
      continue;
    }

    // For pre-regalloc scheduling, create instructions corresponding to the
    // SDNode and any glued SDNodes and append them to the block.
    if (!SU->getNode()) {
      // Emit a copy.
      EmitPhysRegCopy(SU, CopyVRBaseMap, InsertPos);
      continue;
    }

    SmallVector<SDNode *, 4> GluedNodes;
    for (SDNode *N = SU->getNode()->getGluedNode(); N; N = N->getGluedNode())
      GluedNodes.push_back(N);
    while (!GluedNodes.empty()) {
      SDNode *N = GluedNodes.back();
      Emitter.EmitNode(GluedNodes.back(), SU->OrigNode != SU, SU->isCloned,
                       VRBaseMap);
      // Remember the source order of the inserted instruction.
      if (HasDbg)
        ProcessSourceNode(N, DAG, Emitter, VRBaseMap, Orders, Seen);
      GluedNodes.pop_back();
    }
    Emitter.EmitNode(SU->getNode(), SU->OrigNode != SU, SU->isCloned,
                     VRBaseMap);
    // Remember the source order of the inserted instruction.
    if (HasDbg)
      ProcessSourceNode(SU->getNode(), DAG, Emitter, VRBaseMap, Orders,
                        Seen);
  }

  // Insert all the dbg_values which have not already been inserted in source
  // order sequence.
  if (HasDbg) {
    MachineBasicBlock::iterator BBBegin = BB->getFirstNonPHI();

    // Sort the source order instructions and use the order to insert debug
    // values.
    std::sort(Orders.begin(), Orders.end(), less_first());

    SDDbgInfo::DbgIterator DI = DAG->DbgBegin();
    SDDbgInfo::DbgIterator DE = DAG->DbgEnd();
    // Now emit the rest according to source order.
    unsigned LastOrder = 0;
    for (unsigned i = 0, e = Orders.size(); i != e && DI != DE; ++i) {
      unsigned Order = Orders[i].first;
      MachineInstr *MI = Orders[i].second;
      // Insert all SDDbgValue's whose order(s) are before "Order".
      if (!MI)
        continue;
      for (; DI != DE &&
             (*DI)->getOrder() >= LastOrder && (*DI)->getOrder() < Order; ++DI) {
        if ((*DI)->isInvalidated())
          continue;
        MachineInstr *DbgMI = Emitter.EmitDbgValue(*DI, VRBaseMap);
        if (DbgMI) {
          if (!LastOrder)
            // Insert to start of the BB (after PHIs).
            BB->insert(BBBegin, DbgMI);
          else {
            // Insert at the instruction, which may be in a different
            // block, if the block was split by a custom inserter.
            MachineBasicBlock::iterator Pos = MI;
            MI->getParent()->insert(Pos, DbgMI);
          }
        }
      }
      LastOrder = Order;
    }
    // Add trailing DbgValue's before the terminator. FIXME: May want to add
    // some of them before one or more conditional branches?
    SmallVector<MachineInstr*, 8> DbgMIs;
    while (DI != DE) {
      if (!(*DI)->isInvalidated())
        if (MachineInstr *DbgMI = Emitter.EmitDbgValue(*DI, VRBaseMap))
          DbgMIs.push_back(DbgMI);
      ++DI;
    }

    MachineBasicBlock *InsertBB = Emitter.getBlock();
    MachineBasicBlock::iterator Pos = InsertBB->getFirstTerminator();
    InsertBB->insert(Pos, DbgMIs.begin(), DbgMIs.end());
  }

  InsertPos = Emitter.getInsertPos();
  return Emitter.getBlock();
}

/// Return the basic block label.
std::string ScheduleDAGSDNodes::getDAGName() const {
  return "sunit-dag." + BB->getFullName();
}
