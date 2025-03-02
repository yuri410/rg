//===-- LegalizeTypes.h - DAG Type Legalizer class definition ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the DAGTypeLegalizer class.  This is a private interface
// shared between the code that implements the SelectionDAG::LegalizeTypes
// method.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_SELECTIONDAG_LEGALIZETYPES_H
#define LLVM_LIB_CODEGEN_SELECTIONDAG_LEGALIZETYPES_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
/// DAGTypeLegalizer - This takes an arbitrary SelectionDAG as input and hacks
/// on it until only value types the target machine can handle are left.  This
/// involves promoting small sizes to large sizes or splitting up large values
/// into small values.
///
class LLVM_LIBRARY_VISIBILITY DAGTypeLegalizer {
  const TargetLowering &TLI;
  SelectionDAG &DAG;
public:
  // NodeIdFlags - This pass uses the NodeId on the SDNodes to hold information
  // about the state of the node.  The enum has all the values.
  enum NodeIdFlags {
    /// ReadyToProcess - All operands have been processed, so this node is ready
    /// to be handled.
    ReadyToProcess = 0,

    /// NewNode - This is a new node, not before seen, that was created in the
    /// process of legalizing some other node.
    NewNode = -1,

    /// Unanalyzed - This node's ID needs to be set to the number of its
    /// unprocessed operands.
    Unanalyzed = -2,

    /// Processed - This is a node that has already been processed.
    Processed = -3

    // 1+ - This is a node which has this many unprocessed operands.
  };
private:

  /// ValueTypeActions - This is a bitvector that contains two bits for each
  /// simple value type, where the two bits correspond to the LegalizeAction
  /// enum from TargetLowering.  This can be queried with "getTypeAction(VT)".
  TargetLowering::ValueTypeActionImpl ValueTypeActions;

  /// getTypeAction - Return how we should legalize values of this type.
  TargetLowering::LegalizeTypeAction getTypeAction(EVT VT) const {
    return TLI.getTypeAction(*DAG.getContext(), VT);
  }

  /// isTypeLegal - Return true if this type is legal on this target.
  bool isTypeLegal(EVT VT) const {
    return TLI.getTypeAction(*DAG.getContext(), VT) == TargetLowering::TypeLegal;
  }

  EVT getSetCCResultType(EVT VT) const {
    return TLI.getSetCCResultType(DAG.getDataLayout(), *DAG.getContext(), VT);
  }

  /// IgnoreNodeResults - Pretend all of this node's results are legal.
  bool IgnoreNodeResults(SDNode *N) const {
    return N->getOpcode() == ISD::TargetConstant;
  }

  /// PromotedIntegers - For integer nodes that are below legal width, this map
  /// indicates what promoted value to use.
  SmallDenseMap<SDValue, SDValue, 8> PromotedIntegers;

  /// ExpandedIntegers - For integer nodes that need to be expanded this map
  /// indicates which operands are the expanded version of the input.
  SmallDenseMap<SDValue, std::pair<SDValue, SDValue>, 8> ExpandedIntegers;

  /// SoftenedFloats - For floating point nodes converted to integers of
  /// the same size, this map indicates the converted value to use.
  SmallDenseMap<SDValue, SDValue, 8> SoftenedFloats;

  /// PromotedFloats - For floating point nodes that have a smaller precision
  /// than the smallest supported precision, this map indicates what promoted
  /// value to use.
  SmallDenseMap<SDValue, SDValue, 8> PromotedFloats;

  /// ExpandedFloats - For float nodes that need to be expanded this map
  /// indicates which operands are the expanded version of the input.
  SmallDenseMap<SDValue, std::pair<SDValue, SDValue>, 8> ExpandedFloats;

  /// ScalarizedVectors - For nodes that are <1 x ty>, this map indicates the
  /// scalar value of type 'ty' to use.
  SmallDenseMap<SDValue, SDValue, 8> ScalarizedVectors;

  /// SplitVectors - For nodes that need to be split this map indicates
  /// which operands are the expanded version of the input.
  SmallDenseMap<SDValue, std::pair<SDValue, SDValue>, 8> SplitVectors;

  /// WidenedVectors - For vector nodes that need to be widened, indicates
  /// the widened value to use.
  SmallDenseMap<SDValue, SDValue, 8> WidenedVectors;

  /// ReplacedValues - For values that have been replaced with another,
  /// indicates the replacement value to use.
  SmallDenseMap<SDValue, SDValue, 8> ReplacedValues;

  /// Worklist - This defines a worklist of nodes to process.  In order to be
  /// pushed onto this worklist, all operands of a node must have already been
  /// processed.
  SmallVector<SDNode*, 128> Worklist;

public:
  explicit DAGTypeLegalizer(SelectionDAG &dag)
    : TLI(dag.getTargetLoweringInfo()), DAG(dag),
    ValueTypeActions(TLI.getValueTypeActions()) {
    static_assert(MVT::LAST_VALUETYPE <= MVT::MAX_ALLOWED_VALUETYPE,
                  "Too many value types for ValueTypeActions to hold!");
  }

  /// run - This is the main entry point for the type legalizer.  This does a
  /// top-down traversal of the dag, legalizing types as it goes.  Returns
  /// "true" if it made any changes.
  bool run();

  void NoteDeletion(SDNode *Old, SDNode *New) {
    ExpungeNode(Old);
    ExpungeNode(New);
    for (unsigned i = 0, e = Old->getNumValues(); i != e; ++i)
      ReplacedValues[SDValue(Old, i)] = SDValue(New, i);
  }

  SelectionDAG &getDAG() const { return DAG; }

private:
  SDNode *AnalyzeNewNode(SDNode *N);
  void AnalyzeNewValue(SDValue &Val);
  void ExpungeNode(SDNode *N);
  void PerformExpensiveChecks();
  void RemapValue(SDValue &N);

  // Common routines.
  SDValue BitConvertToInteger(SDValue Op);
  SDValue BitConvertVectorToIntegerVector(SDValue Op);
  SDValue CreateStackStoreLoad(SDValue Op, EVT DestVT);
  bool CustomLowerNode(SDNode *N, EVT VT, bool LegalizeResult);
  bool CustomWidenLowerNode(SDNode *N, EVT VT);

  /// DisintegrateMERGE_VALUES - Replace each result of the given MERGE_VALUES
  /// node with the corresponding input operand, except for the result 'ResNo',
  /// for which the corresponding input operand is returned.
  SDValue DisintegrateMERGE_VALUES(SDNode *N, unsigned ResNo);

  SDValue GetVectorElementPointer(SDValue VecPtr, EVT EltVT, SDValue Index);
  SDValue JoinIntegers(SDValue Lo, SDValue Hi);
  SDValue LibCallify(RTLIB::Libcall LC, SDNode *N, bool isSigned);

  std::pair<SDValue, SDValue> ExpandChainLibCall(RTLIB::Libcall LC,
                                                 SDNode *Node, bool isSigned);
  std::pair<SDValue, SDValue> ExpandAtomic(SDNode *Node);

  SDValue PromoteTargetBoolean(SDValue Bool, EVT ValVT);
  void ReplaceValueWith(SDValue From, SDValue To);
  void SplitInteger(SDValue Op, SDValue &Lo, SDValue &Hi);
  void SplitInteger(SDValue Op, EVT LoVT, EVT HiVT,
                    SDValue &Lo, SDValue &Hi);

  //===--------------------------------------------------------------------===//
  // Integer Promotion Support: LegalizeIntegerTypes.cpp
  //===--------------------------------------------------------------------===//

  /// GetPromotedInteger - Given a processed operand Op which was promoted to a
  /// larger integer type, this returns the promoted value.  The low bits of the
  /// promoted value corresponding to the original type are exactly equal to Op.
  /// The extra bits contain rubbish, so the promoted value may need to be zero-
  /// or sign-extended from the original type before it is usable (the helpers
  /// SExtPromotedInteger and ZExtPromotedInteger can do this for you).
  /// For example, if Op is an i16 and was promoted to an i32, then this method
  /// returns an i32, the lower 16 bits of which coincide with Op, and the upper
  /// 16 bits of which contain rubbish.
  SDValue GetPromotedInteger(SDValue Op) {
    SDValue &PromotedOp = PromotedIntegers[Op];
    RemapValue(PromotedOp);
    assert(PromotedOp.getNode() && "Operand wasn't promoted?");
    return PromotedOp;
  }
  void SetPromotedInteger(SDValue Op, SDValue Result);

  /// SExtPromotedInteger - Get a promoted operand and sign extend it to the
  /// final size.
  SDValue SExtPromotedInteger(SDValue Op) {
    EVT OldVT = Op.getValueType();
    SDLoc dl(Op);
    Op = GetPromotedInteger(Op);
    return DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, Op.getValueType(), Op,
                       DAG.getValueType(OldVT));
  }

  /// ZExtPromotedInteger - Get a promoted operand and zero extend it to the
  /// final size.
  SDValue ZExtPromotedInteger(SDValue Op) {
    EVT OldVT = Op.getValueType();
    SDLoc dl(Op);
    Op = GetPromotedInteger(Op);
    return DAG.getZeroExtendInReg(Op, dl, OldVT.getScalarType());
  }

  // Integer Result Promotion.
  void PromoteIntegerResult(SDNode *N, unsigned ResNo);
  SDValue PromoteIntRes_MERGE_VALUES(SDNode *N, unsigned ResNo);
  SDValue PromoteIntRes_AssertSext(SDNode *N);
  SDValue PromoteIntRes_AssertZext(SDNode *N);
  SDValue PromoteIntRes_Atomic0(AtomicSDNode *N);
  SDValue PromoteIntRes_Atomic1(AtomicSDNode *N);
  SDValue PromoteIntRes_AtomicCmpSwap(AtomicSDNode *N, unsigned ResNo);
  SDValue PromoteIntRes_EXTRACT_SUBVECTOR(SDNode *N);
  SDValue PromoteIntRes_VECTOR_SHUFFLE(SDNode *N);
  SDValue PromoteIntRes_BUILD_VECTOR(SDNode *N);
  SDValue PromoteIntRes_SCALAR_TO_VECTOR(SDNode *N);
  SDValue PromoteIntRes_INSERT_VECTOR_ELT(SDNode *N);
  SDValue PromoteIntRes_CONCAT_VECTORS(SDNode *N);
  SDValue PromoteIntRes_BITCAST(SDNode *N);
  SDValue PromoteIntRes_BSWAP(SDNode *N);
  SDValue PromoteIntRes_BUILD_PAIR(SDNode *N);
  SDValue PromoteIntRes_Constant(SDNode *N);
  SDValue PromoteIntRes_CONVERT_RNDSAT(SDNode *N);
  SDValue PromoteIntRes_CTLZ(SDNode *N);
  SDValue PromoteIntRes_CTPOP(SDNode *N);
  SDValue PromoteIntRes_CTTZ(SDNode *N);
  SDValue PromoteIntRes_EXTRACT_VECTOR_ELT(SDNode *N);
  SDValue PromoteIntRes_FP_TO_XINT(SDNode *N);
  SDValue PromoteIntRes_FP_TO_FP16(SDNode *N);
  SDValue PromoteIntRes_INT_EXTEND(SDNode *N);
  SDValue PromoteIntRes_LOAD(LoadSDNode *N);
  SDValue PromoteIntRes_MLOAD(MaskedLoadSDNode *N);
  SDValue PromoteIntRes_Overflow(SDNode *N);
  SDValue PromoteIntRes_SADDSUBO(SDNode *N, unsigned ResNo);
  SDValue PromoteIntRes_SDIV(SDNode *N);
  SDValue PromoteIntRes_SELECT(SDNode *N);
  SDValue PromoteIntRes_VSELECT(SDNode *N);
  SDValue PromoteIntRes_SELECT_CC(SDNode *N);
  SDValue PromoteIntRes_SETCC(SDNode *N);
  SDValue PromoteIntRes_SHL(SDNode *N);
  SDValue PromoteIntRes_SimpleIntBinOp(SDNode *N);
  SDValue PromoteIntRes_SIGN_EXTEND_INREG(SDNode *N);
  SDValue PromoteIntRes_SRA(SDNode *N);
  SDValue PromoteIntRes_SRL(SDNode *N);
  SDValue PromoteIntRes_TRUNCATE(SDNode *N);
  SDValue PromoteIntRes_UADDSUBO(SDNode *N, unsigned ResNo);
  SDValue PromoteIntRes_UDIV(SDNode *N);
  SDValue PromoteIntRes_UNDEF(SDNode *N);
  SDValue PromoteIntRes_VAARG(SDNode *N);
  SDValue PromoteIntRes_XMULO(SDNode *N, unsigned ResNo);

  // Integer Operand Promotion.
  bool PromoteIntegerOperand(SDNode *N, unsigned OperandNo);
  SDValue PromoteIntOp_ANY_EXTEND(SDNode *N);
  SDValue PromoteIntOp_ATOMIC_STORE(AtomicSDNode *N);
  SDValue PromoteIntOp_BITCAST(SDNode *N);
  SDValue PromoteIntOp_BUILD_PAIR(SDNode *N);
  SDValue PromoteIntOp_BR_CC(SDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_BRCOND(SDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_BUILD_VECTOR(SDNode *N);
  SDValue PromoteIntOp_CONVERT_RNDSAT(SDNode *N);
  SDValue PromoteIntOp_INSERT_VECTOR_ELT(SDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_EXTRACT_ELEMENT(SDNode *N);
  SDValue PromoteIntOp_EXTRACT_VECTOR_ELT(SDNode *N);
  SDValue PromoteIntOp_EXTRACT_SUBVECTOR(SDNode *N);
  SDValue PromoteIntOp_CONCAT_VECTORS(SDNode *N);
  SDValue PromoteIntOp_SCALAR_TO_VECTOR(SDNode *N);
  SDValue PromoteIntOp_SELECT(SDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_SELECT_CC(SDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_SETCC(SDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_VSETCC(SDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_Shift(SDNode *N);
  SDValue PromoteIntOp_SIGN_EXTEND(SDNode *N);
  SDValue PromoteIntOp_SINT_TO_FP(SDNode *N);
  SDValue PromoteIntOp_STORE(StoreSDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_TRUNCATE(SDNode *N);
  SDValue PromoteIntOp_UINT_TO_FP(SDNode *N);
  SDValue PromoteIntOp_ZERO_EXTEND(SDNode *N);
  SDValue PromoteIntOp_MSTORE(MaskedStoreSDNode *N, unsigned OpNo);
  SDValue PromoteIntOp_MLOAD(MaskedLoadSDNode *N, unsigned OpNo);

  void PromoteSetCCOperands(SDValue &LHS,SDValue &RHS, ISD::CondCode Code);

  //===--------------------------------------------------------------------===//
  // Integer Expansion Support: LegalizeIntegerTypes.cpp
  //===--------------------------------------------------------------------===//

  /// GetExpandedInteger - Given a processed operand Op which was expanded into
  /// two integers of half the size, this returns the two halves.  The low bits
  /// of Op are exactly equal to the bits of Lo; the high bits exactly equal Hi.
  /// For example, if Op is an i64 which was expanded into two i32's, then this
  /// method returns the two i32's, with Lo being equal to the lower 32 bits of
  /// Op, and Hi being equal to the upper 32 bits.
  void GetExpandedInteger(SDValue Op, SDValue &Lo, SDValue &Hi);
  void SetExpandedInteger(SDValue Op, SDValue Lo, SDValue Hi);

  // Integer Result Expansion.
  void ExpandIntegerResult(SDNode *N, unsigned ResNo);
  void ExpandIntRes_MERGE_VALUES      (SDNode *N, unsigned ResNo,
                                       SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_ANY_EXTEND        (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_AssertSext        (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_AssertZext        (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_Constant          (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_CTLZ              (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_CTPOP             (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_CTTZ              (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_LOAD          (LoadSDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_SIGN_EXTEND       (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_SIGN_EXTEND_INREG (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_TRUNCATE          (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_ZERO_EXTEND       (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_FP_TO_SINT        (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_FP_TO_UINT        (SDNode *N, SDValue &Lo, SDValue &Hi);

  void ExpandIntRes_Logical           (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_ADDSUB            (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_ADDSUBC           (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_ADDSUBE           (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_BSWAP             (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_MUL               (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_SDIV              (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_SREM              (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_UDIV              (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_UREM              (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_Shift             (SDNode *N, SDValue &Lo, SDValue &Hi);

  void ExpandIntRes_SADDSUBO          (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_UADDSUBO          (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandIntRes_XMULO             (SDNode *N, SDValue &Lo, SDValue &Hi);

  void ExpandIntRes_ATOMIC_LOAD       (SDNode *N, SDValue &Lo, SDValue &Hi);

  void ExpandShiftByConstant(SDNode *N, const APInt &Amt,
                             SDValue &Lo, SDValue &Hi);
  bool ExpandShiftWithKnownAmountBit(SDNode *N, SDValue &Lo, SDValue &Hi);
  bool ExpandShiftWithUnknownAmountBit(SDNode *N, SDValue &Lo, SDValue &Hi);

  // Integer Operand Expansion.
  bool ExpandIntegerOperand(SDNode *N, unsigned OperandNo);
  SDValue ExpandIntOp_BITCAST(SDNode *N);
  SDValue ExpandIntOp_BR_CC(SDNode *N);
  SDValue ExpandIntOp_BUILD_VECTOR(SDNode *N);
  SDValue ExpandIntOp_EXTRACT_ELEMENT(SDNode *N);
  SDValue ExpandIntOp_SELECT_CC(SDNode *N);
  SDValue ExpandIntOp_SETCC(SDNode *N);
  SDValue ExpandIntOp_Shift(SDNode *N);
  SDValue ExpandIntOp_SINT_TO_FP(SDNode *N);
  SDValue ExpandIntOp_STORE(StoreSDNode *N, unsigned OpNo);
  SDValue ExpandIntOp_TRUNCATE(SDNode *N);
  SDValue ExpandIntOp_UINT_TO_FP(SDNode *N);
  SDValue ExpandIntOp_RETURNADDR(SDNode *N);
  SDValue ExpandIntOp_ATOMIC_STORE(SDNode *N);

  void IntegerExpandSetCCOperands(SDValue &NewLHS, SDValue &NewRHS,
                                  ISD::CondCode &CCCode, SDLoc dl);

  //===--------------------------------------------------------------------===//
  // Float to Integer Conversion Support: LegalizeFloatTypes.cpp
  //===--------------------------------------------------------------------===//

  /// GetSoftenedFloat - Given a processed operand Op which was converted to an
  /// integer of the same size, this returns the integer.  The integer contains
  /// exactly the same bits as Op - only the type changed.  For example, if Op
  /// is an f32 which was softened to an i32, then this method returns an i32,
  /// the bits of which coincide with those of Op.
  SDValue GetSoftenedFloat(SDValue Op) {
    SDValue &SoftenedOp = SoftenedFloats[Op];
    RemapValue(SoftenedOp);
    assert(SoftenedOp.getNode() && "Operand wasn't converted to integer?");
    return SoftenedOp;
  }
  void SetSoftenedFloat(SDValue Op, SDValue Result);

  // Result Float to Integer Conversion.
  void SoftenFloatResult(SDNode *N, unsigned OpNo);
  SDValue SoftenFloatRes_MERGE_VALUES(SDNode *N, unsigned ResNo);
  SDValue SoftenFloatRes_BITCAST(SDNode *N);
  SDValue SoftenFloatRes_BUILD_PAIR(SDNode *N);
  SDValue SoftenFloatRes_ConstantFP(ConstantFPSDNode *N);
  SDValue SoftenFloatRes_EXTRACT_VECTOR_ELT(SDNode *N);
  SDValue SoftenFloatRes_FABS(SDNode *N);
  SDValue SoftenFloatRes_FMINNUM(SDNode *N);
  SDValue SoftenFloatRes_FMAXNUM(SDNode *N);
  SDValue SoftenFloatRes_FADD(SDNode *N);
  SDValue SoftenFloatRes_FCEIL(SDNode *N);
  SDValue SoftenFloatRes_FCOPYSIGN(SDNode *N);
  SDValue SoftenFloatRes_FCOS(SDNode *N);
  SDValue SoftenFloatRes_FDIV(SDNode *N);
  SDValue SoftenFloatRes_FEXP(SDNode *N);
  SDValue SoftenFloatRes_FEXP2(SDNode *N);
  SDValue SoftenFloatRes_FFLOOR(SDNode *N);
  SDValue SoftenFloatRes_FLOG(SDNode *N);
  SDValue SoftenFloatRes_FLOG2(SDNode *N);
  SDValue SoftenFloatRes_FLOG10(SDNode *N);
  SDValue SoftenFloatRes_FMA(SDNode *N);
  SDValue SoftenFloatRes_FMUL(SDNode *N);
  SDValue SoftenFloatRes_FNEARBYINT(SDNode *N);
  SDValue SoftenFloatRes_FNEG(SDNode *N);
  SDValue SoftenFloatRes_FP_EXTEND(SDNode *N);
  SDValue SoftenFloatRes_FP16_TO_FP(SDNode *N);
  SDValue SoftenFloatRes_FP_ROUND(SDNode *N);
  SDValue SoftenFloatRes_FPOW(SDNode *N);
  SDValue SoftenFloatRes_FPOWI(SDNode *N);
  SDValue SoftenFloatRes_FREM(SDNode *N);
  SDValue SoftenFloatRes_FRINT(SDNode *N);
  SDValue SoftenFloatRes_FROUND(SDNode *N);
  SDValue SoftenFloatRes_FSIN(SDNode *N);
  SDValue SoftenFloatRes_FSQRT(SDNode *N);
  SDValue SoftenFloatRes_FSUB(SDNode *N);
  SDValue SoftenFloatRes_FTRUNC(SDNode *N);
  SDValue SoftenFloatRes_LOAD(SDNode *N);
  SDValue SoftenFloatRes_SELECT(SDNode *N);
  SDValue SoftenFloatRes_SELECT_CC(SDNode *N);
  SDValue SoftenFloatRes_UNDEF(SDNode *N);
  SDValue SoftenFloatRes_VAARG(SDNode *N);
  SDValue SoftenFloatRes_XINT_TO_FP(SDNode *N);

  // Operand Float to Integer Conversion.
  bool SoftenFloatOperand(SDNode *N, unsigned OpNo);
  SDValue SoftenFloatOp_BITCAST(SDNode *N);
  SDValue SoftenFloatOp_BR_CC(SDNode *N);
  SDValue SoftenFloatOp_FP_EXTEND(SDNode *N);
  SDValue SoftenFloatOp_FP_ROUND(SDNode *N);
  SDValue SoftenFloatOp_FP_TO_SINT(SDNode *N);
  SDValue SoftenFloatOp_FP_TO_UINT(SDNode *N);
  SDValue SoftenFloatOp_SELECT_CC(SDNode *N);
  SDValue SoftenFloatOp_SETCC(SDNode *N);
  SDValue SoftenFloatOp_STORE(SDNode *N, unsigned OpNo);

  //===--------------------------------------------------------------------===//
  // Float Expansion Support: LegalizeFloatTypes.cpp
  //===--------------------------------------------------------------------===//

  /// GetExpandedFloat - Given a processed operand Op which was expanded into
  /// two floating point values of half the size, this returns the two halves.
  /// The low bits of Op are exactly equal to the bits of Lo; the high bits
  /// exactly equal Hi.  For example, if Op is a ppcf128 which was expanded
  /// into two f64's, then this method returns the two f64's, with Lo being
  /// equal to the lower 64 bits of Op, and Hi to the upper 64 bits.
  void GetExpandedFloat(SDValue Op, SDValue &Lo, SDValue &Hi);
  void SetExpandedFloat(SDValue Op, SDValue Lo, SDValue Hi);

  // Float Result Expansion.
  void ExpandFloatResult(SDNode *N, unsigned ResNo);
  void ExpandFloatRes_ConstantFP(SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FABS      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FMINNUM   (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FMAXNUM   (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FADD      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FCEIL     (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FCOPYSIGN (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FCOS      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FDIV      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FEXP      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FEXP2     (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FFLOOR    (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FLOG      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FLOG2     (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FLOG10    (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FMA       (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FMUL      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FNEARBYINT(SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FNEG      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FP_EXTEND (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FPOW      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FPOWI     (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FREM      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FRINT     (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FROUND    (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FSIN      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FSQRT     (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FSUB      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_FTRUNC    (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_LOAD      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandFloatRes_XINT_TO_FP(SDNode *N, SDValue &Lo, SDValue &Hi);

  // Float Operand Expansion.
  bool ExpandFloatOperand(SDNode *N, unsigned OperandNo);
  SDValue ExpandFloatOp_BR_CC(SDNode *N);
  SDValue ExpandFloatOp_FCOPYSIGN(SDNode *N);
  SDValue ExpandFloatOp_FP_ROUND(SDNode *N);
  SDValue ExpandFloatOp_FP_TO_SINT(SDNode *N);
  SDValue ExpandFloatOp_FP_TO_UINT(SDNode *N);
  SDValue ExpandFloatOp_SELECT_CC(SDNode *N);
  SDValue ExpandFloatOp_SETCC(SDNode *N);
  SDValue ExpandFloatOp_STORE(SDNode *N, unsigned OpNo);

  void FloatExpandSetCCOperands(SDValue &NewLHS, SDValue &NewRHS,
                                ISD::CondCode &CCCode, SDLoc dl);


  //===--------------------------------------------------------------------===//
  // Float promotion support: LegalizeFloatTypes.cpp
  //===--------------------------------------------------------------------===//

  SDValue GetPromotedFloat(SDValue Op) {
    SDValue &PromotedOp = PromotedFloats[Op];
    RemapValue(PromotedOp);
    assert(PromotedOp.getNode() && "Operand wasn't promoted?");
    return PromotedOp;
  }
  void SetPromotedFloat(SDValue Op, SDValue Result);

  void PromoteFloatResult(SDNode *N, unsigned ResNo);
  SDValue PromoteFloatRes_BITCAST(SDNode *N);
  SDValue PromoteFloatRes_BinOp(SDNode *N);
  SDValue PromoteFloatRes_ConstantFP(SDNode *N);
  SDValue PromoteFloatRes_EXTRACT_VECTOR_ELT(SDNode *N);
  SDValue PromoteFloatRes_FCOPYSIGN(SDNode *N);
  SDValue PromoteFloatRes_FMAD(SDNode *N);
  SDValue PromoteFloatRes_FPOWI(SDNode *N);
  SDValue PromoteFloatRes_FP_ROUND(SDNode *N);
  SDValue PromoteFloatRes_LOAD(SDNode *N);
  SDValue PromoteFloatRes_SELECT(SDNode *N);
  SDValue PromoteFloatRes_SELECT_CC(SDNode *N);
  SDValue PromoteFloatRes_UnaryOp(SDNode *N);
  SDValue PromoteFloatRes_UNDEF(SDNode *N);
  SDValue PromoteFloatRes_XINT_TO_FP(SDNode *N);

  bool PromoteFloatOperand(SDNode *N, unsigned ResNo);
  SDValue PromoteFloatOp_BITCAST(SDNode *N, unsigned OpNo);
  SDValue PromoteFloatOp_FCOPYSIGN(SDNode *N, unsigned OpNo);
  SDValue PromoteFloatOp_FP_EXTEND(SDNode *N, unsigned OpNo);
  SDValue PromoteFloatOp_FP_TO_XINT(SDNode *N, unsigned OpNo);
  SDValue PromoteFloatOp_STORE(SDNode *N, unsigned OpNo);
  SDValue PromoteFloatOp_SELECT_CC(SDNode *N, unsigned OpNo);
  SDValue PromoteFloatOp_SETCC(SDNode *N, unsigned OpNo);

  //===--------------------------------------------------------------------===//
  // Scalarization Support: LegalizeVectorTypes.cpp
  //===--------------------------------------------------------------------===//

  /// GetScalarizedVector - Given a processed one-element vector Op which was
  /// scalarized to its element type, this returns the element.  For example,
  /// if Op is a v1i32, Op = < i32 val >, this method returns val, an i32.
  SDValue GetScalarizedVector(SDValue Op) {
    SDValue &ScalarizedOp = ScalarizedVectors[Op];
    RemapValue(ScalarizedOp);
    assert(ScalarizedOp.getNode() && "Operand wasn't scalarized?");
    return ScalarizedOp;
  }
  void SetScalarizedVector(SDValue Op, SDValue Result);

  // Vector Result Scalarization: <1 x ty> -> ty.
  void ScalarizeVectorResult(SDNode *N, unsigned OpNo);
  SDValue ScalarizeVecRes_MERGE_VALUES(SDNode *N, unsigned ResNo);
  SDValue ScalarizeVecRes_BinOp(SDNode *N);
  SDValue ScalarizeVecRes_TernaryOp(SDNode *N);
  SDValue ScalarizeVecRes_UnaryOp(SDNode *N);
  SDValue ScalarizeVecRes_InregOp(SDNode *N);

  SDValue ScalarizeVecRes_BITCAST(SDNode *N);
  SDValue ScalarizeVecRes_BUILD_VECTOR(SDNode *N);
  SDValue ScalarizeVecRes_CONVERT_RNDSAT(SDNode *N);
  SDValue ScalarizeVecRes_EXTRACT_SUBVECTOR(SDNode *N);
  SDValue ScalarizeVecRes_FP_ROUND(SDNode *N);
  SDValue ScalarizeVecRes_FPOWI(SDNode *N);
  SDValue ScalarizeVecRes_INSERT_VECTOR_ELT(SDNode *N);
  SDValue ScalarizeVecRes_LOAD(LoadSDNode *N);
  SDValue ScalarizeVecRes_SCALAR_TO_VECTOR(SDNode *N);
  SDValue ScalarizeVecRes_SIGN_EXTEND_INREG(SDNode *N);
  SDValue ScalarizeVecRes_VSELECT(SDNode *N);
  SDValue ScalarizeVecRes_SELECT(SDNode *N);
  SDValue ScalarizeVecRes_SELECT_CC(SDNode *N);
  SDValue ScalarizeVecRes_SETCC(SDNode *N);
  SDValue ScalarizeVecRes_UNDEF(SDNode *N);
  SDValue ScalarizeVecRes_VECTOR_SHUFFLE(SDNode *N);
  SDValue ScalarizeVecRes_VSETCC(SDNode *N);

  // Vector Operand Scalarization: <1 x ty> -> ty.
  bool ScalarizeVectorOperand(SDNode *N, unsigned OpNo);
  SDValue ScalarizeVecOp_BITCAST(SDNode *N);
  SDValue ScalarizeVecOp_UnaryOp(SDNode *N);
  SDValue ScalarizeVecOp_CONCAT_VECTORS(SDNode *N);
  SDValue ScalarizeVecOp_EXTRACT_VECTOR_ELT(SDNode *N);
  SDValue ScalarizeVecOp_VSELECT(SDNode *N);
  SDValue ScalarizeVecOp_STORE(StoreSDNode *N, unsigned OpNo);
  SDValue ScalarizeVecOp_FP_ROUND(SDNode *N, unsigned OpNo);

  //===--------------------------------------------------------------------===//
  // Vector Splitting Support: LegalizeVectorTypes.cpp
  //===--------------------------------------------------------------------===//

  /// GetSplitVector - Given a processed vector Op which was split into vectors
  /// of half the size, this method returns the halves.  The first elements of
  /// Op coincide with the elements of Lo; the remaining elements of Op coincide
  /// with the elements of Hi: Op is what you would get by concatenating Lo and
  /// Hi.  For example, if Op is a v8i32 that was split into two v4i32's, then
  /// this method returns the two v4i32's, with Lo corresponding to the first 4
  /// elements of Op, and Hi to the last 4 elements.
  void GetSplitVector(SDValue Op, SDValue &Lo, SDValue &Hi);
  void SetSplitVector(SDValue Op, SDValue Lo, SDValue Hi);

  // Vector Result Splitting: <128 x ty> -> 2 x <64 x ty>.
  void SplitVectorResult(SDNode *N, unsigned OpNo);
  void SplitVecRes_BinOp(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_TernaryOp(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_UnaryOp(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_ExtendOp(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_InregOp(SDNode *N, SDValue &Lo, SDValue &Hi);

  void SplitVecRes_BITCAST(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_BUILD_PAIR(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_BUILD_VECTOR(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_CONCAT_VECTORS(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_EXTRACT_SUBVECTOR(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_INSERT_SUBVECTOR(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_FPOWI(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_INSERT_VECTOR_ELT(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_LOAD(LoadSDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_MLOAD(MaskedLoadSDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_MGATHER(MaskedGatherSDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_SCALAR_TO_VECTOR(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_SIGN_EXTEND_INREG(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_SETCC(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_UNDEF(SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitVecRes_VECTOR_SHUFFLE(ShuffleVectorSDNode *N, SDValue &Lo,
                                  SDValue &Hi);

  // Vector Operand Splitting: <128 x ty> -> 2 x <64 x ty>.
  bool SplitVectorOperand(SDNode *N, unsigned OpNo);
  SDValue SplitVecOp_VSELECT(SDNode *N, unsigned OpNo);
  SDValue SplitVecOp_UnaryOp(SDNode *N);
  SDValue SplitVecOp_TruncateHelper(SDNode *N);

  SDValue SplitVecOp_BITCAST(SDNode *N);
  SDValue SplitVecOp_EXTRACT_SUBVECTOR(SDNode *N);
  SDValue SplitVecOp_EXTRACT_VECTOR_ELT(SDNode *N);
  SDValue SplitVecOp_STORE(StoreSDNode *N, unsigned OpNo);
  SDValue SplitVecOp_MSTORE(MaskedStoreSDNode *N, unsigned OpNo);
  SDValue SplitVecOp_MSCATTER(MaskedScatterSDNode *N, unsigned OpNo);
  SDValue SplitVecOp_MGATHER(MaskedGatherSDNode *N, unsigned OpNo);
  SDValue SplitVecOp_CONCAT_VECTORS(SDNode *N);
  SDValue SplitVecOp_VSETCC(SDNode *N);
  SDValue SplitVecOp_FP_ROUND(SDNode *N);

  //===--------------------------------------------------------------------===//
  // Vector Widening Support: LegalizeVectorTypes.cpp
  //===--------------------------------------------------------------------===//

  /// GetWidenedVector - Given a processed vector Op which was widened into a
  /// larger vector, this method returns the larger vector.  The elements of
  /// the returned vector consist of the elements of Op followed by elements
  /// containing rubbish.  For example, if Op is a v2i32 that was widened to a
  /// v4i32, then this method returns a v4i32 for which the first two elements
  /// are the same as those of Op, while the last two elements contain rubbish.
  SDValue GetWidenedVector(SDValue Op) {
    SDValue &WidenedOp = WidenedVectors[Op];
    RemapValue(WidenedOp);
    assert(WidenedOp.getNode() && "Operand wasn't widened?");
    return WidenedOp;
  }
  void SetWidenedVector(SDValue Op, SDValue Result);

  // Widen Vector Result Promotion.
  void WidenVectorResult(SDNode *N, unsigned ResNo);
  SDValue WidenVecRes_MERGE_VALUES(SDNode* N, unsigned ResNo);
  SDValue WidenVecRes_BITCAST(SDNode* N);
  SDValue WidenVecRes_BUILD_VECTOR(SDNode* N);
  SDValue WidenVecRes_CONCAT_VECTORS(SDNode* N);
  SDValue WidenVecRes_CONVERT_RNDSAT(SDNode* N);
  SDValue WidenVecRes_EXTRACT_SUBVECTOR(SDNode* N);
  SDValue WidenVecRes_INSERT_VECTOR_ELT(SDNode* N);
  SDValue WidenVecRes_LOAD(SDNode* N);
  SDValue WidenVecRes_MLOAD(MaskedLoadSDNode* N);
  SDValue WidenVecRes_SCALAR_TO_VECTOR(SDNode* N);
  SDValue WidenVecRes_SIGN_EXTEND_INREG(SDNode* N);
  SDValue WidenVecRes_SELECT(SDNode* N);
  SDValue WidenVecRes_SELECT_CC(SDNode* N);
  SDValue WidenVecRes_SETCC(SDNode* N);
  SDValue WidenVecRes_UNDEF(SDNode *N);
  SDValue WidenVecRes_VECTOR_SHUFFLE(ShuffleVectorSDNode *N);
  SDValue WidenVecRes_VSETCC(SDNode* N);

  SDValue WidenVecRes_Ternary(SDNode *N);
  SDValue WidenVecRes_Binary(SDNode *N);
  SDValue WidenVecRes_BinaryCanTrap(SDNode *N);
  SDValue WidenVecRes_Convert(SDNode *N);
  SDValue WidenVecRes_POWI(SDNode *N);
  SDValue WidenVecRes_Shift(SDNode *N);
  SDValue WidenVecRes_Unary(SDNode *N);
  SDValue WidenVecRes_InregOp(SDNode *N);

  // Widen Vector Operand.
  bool WidenVectorOperand(SDNode *N, unsigned OpNo);
  SDValue WidenVecOp_BITCAST(SDNode *N);
  SDValue WidenVecOp_CONCAT_VECTORS(SDNode *N);
  SDValue WidenVecOp_EXTEND(SDNode *N);
  SDValue WidenVecOp_EXTRACT_VECTOR_ELT(SDNode *N);
  SDValue WidenVecOp_EXTRACT_SUBVECTOR(SDNode *N);
  SDValue WidenVecOp_STORE(SDNode* N);
  SDValue WidenVecOp_MSTORE(SDNode* N, unsigned OpNo);
  SDValue WidenVecOp_SETCC(SDNode* N);

  SDValue WidenVecOp_Convert(SDNode *N);

  //===--------------------------------------------------------------------===//
  // Vector Widening Utilities Support: LegalizeVectorTypes.cpp
  //===--------------------------------------------------------------------===//

  /// Helper GenWidenVectorLoads - Helper function to generate a set of
  /// loads to load a vector with a resulting wider type. It takes
  ///   LdChain: list of chains for the load to be generated.
  ///   Ld:      load to widen
  SDValue GenWidenVectorLoads(SmallVectorImpl<SDValue> &LdChain,
                              LoadSDNode *LD);

  /// GenWidenVectorExtLoads - Helper function to generate a set of extension
  /// loads to load a ector with a resulting wider type.  It takes
  ///   LdChain: list of chains for the load to be generated.
  ///   Ld:      load to widen
  ///   ExtType: extension element type
  SDValue GenWidenVectorExtLoads(SmallVectorImpl<SDValue> &LdChain,
                                 LoadSDNode *LD, ISD::LoadExtType ExtType);

  /// Helper genWidenVectorStores - Helper function to generate a set of
  /// stores to store a widen vector into non-widen memory
  ///   StChain: list of chains for the stores we have generated
  ///   ST:      store of a widen value
  void GenWidenVectorStores(SmallVectorImpl<SDValue> &StChain, StoreSDNode *ST);

  /// Helper genWidenVectorTruncStores - Helper function to generate a set of
  /// stores to store a truncate widen vector into non-widen memory
  ///   StChain: list of chains for the stores we have generated
  ///   ST:      store of a widen value
  void GenWidenVectorTruncStores(SmallVectorImpl<SDValue> &StChain,
                                 StoreSDNode *ST);

  /// Modifies a vector input (widen or narrows) to a vector of NVT.  The
  /// input vector must have the same element type as NVT.
  SDValue ModifyToType(SDValue InOp, EVT WidenVT);


  //===--------------------------------------------------------------------===//
  // Generic Splitting: LegalizeTypesGeneric.cpp
  //===--------------------------------------------------------------------===//

  // Legalization methods which only use that the illegal type is split into two
  // not necessarily identical types.  As such they can be used for splitting
  // vectors and expanding integers and floats.

  void GetSplitOp(SDValue Op, SDValue &Lo, SDValue &Hi) {
    if (Op.getValueType().isVector())
      GetSplitVector(Op, Lo, Hi);
    else if (Op.getValueType().isInteger())
      GetExpandedInteger(Op, Lo, Hi);
    else
      GetExpandedFloat(Op, Lo, Hi);
  }

  /// GetPairElements - Use ISD::EXTRACT_ELEMENT nodes to extract the low and
  /// high parts of the given value.
  void GetPairElements(SDValue Pair, SDValue &Lo, SDValue &Hi);

  // Generic Result Splitting.
  void SplitRes_MERGE_VALUES(SDNode *N, unsigned ResNo,
                             SDValue &Lo, SDValue &Hi);
  void SplitRes_SELECT      (SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitRes_SELECT_CC   (SDNode *N, SDValue &Lo, SDValue &Hi);
  void SplitRes_UNDEF       (SDNode *N, SDValue &Lo, SDValue &Hi);

  //===--------------------------------------------------------------------===//
  // Generic Expansion: LegalizeTypesGeneric.cpp
  //===--------------------------------------------------------------------===//

  // Legalization methods which only use that the illegal type is split into two
  // identical types of half the size, and that the Lo/Hi part is stored first
  // in memory on little/big-endian machines, followed by the Hi/Lo part.  As
  // such they can be used for expanding integers and floats.

  void GetExpandedOp(SDValue Op, SDValue &Lo, SDValue &Hi) {
    if (Op.getValueType().isInteger())
      GetExpandedInteger(Op, Lo, Hi);
    else
      GetExpandedFloat(Op, Lo, Hi);
  }


  /// This function will split the integer \p Op into \p NumElements
  /// operations of type \p EltVT and store them in \p Ops.
  void IntegerToVector(SDValue Op, unsigned NumElements,
                       SmallVectorImpl<SDValue> &Ops, EVT EltVT);

  // Generic Result Expansion.
  void ExpandRes_MERGE_VALUES      (SDNode *N, unsigned ResNo,
                                    SDValue &Lo, SDValue &Hi);
  void ExpandRes_BITCAST           (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandRes_BUILD_PAIR        (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandRes_EXTRACT_ELEMENT   (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandRes_EXTRACT_VECTOR_ELT(SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandRes_NormalLoad        (SDNode *N, SDValue &Lo, SDValue &Hi);
  void ExpandRes_VAARG             (SDNode *N, SDValue &Lo, SDValue &Hi);

  // Generic Operand Expansion.
  SDValue ExpandOp_BITCAST          (SDNode *N);
  SDValue ExpandOp_BUILD_VECTOR     (SDNode *N);
  SDValue ExpandOp_EXTRACT_ELEMENT  (SDNode *N);
  SDValue ExpandOp_INSERT_VECTOR_ELT(SDNode *N);
  SDValue ExpandOp_SCALAR_TO_VECTOR (SDNode *N);
  SDValue ExpandOp_NormalStore      (SDNode *N, unsigned OpNo);
};

} // end namespace llvm.

#endif
