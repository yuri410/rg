// Copyright (c) 2019 The Khronos Group Inc.
// Copyright (c) 2019 Valve Corporation
// Copyright (c) 2019 LunarG Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "convert_to_half_pass.h"

#include "source/opt/ir_builder.h"

namespace {

// Indices of operands in SPIR-V instructions
static const int kImageSampleDrefIdInIdx = 2;

}  // anonymous namespace

namespace spvtools {
namespace opt {

bool ConvertToHalfPass::IsArithmetic(Instruction* inst) {
  return target_ops_core_.count(inst->opcode()) != 0 ||
         (inst->opcode() == SpvOpExtInst &&
          inst->GetSingleWordInOperand(0) ==
              context()->get_feature_mgr()->GetExtInstImportId_GLSLstd450() &&
          target_ops_450_.count(inst->GetSingleWordInOperand(1)) != 0);
}

bool ConvertToHalfPass::IsFloat(Instruction* inst, uint32_t width) {
  uint32_t ty_id = inst->type_id();
  if (ty_id == 0) return false;
  return Pass::IsFloat(ty_id, width);
}

bool ConvertToHalfPass::IsDecoratedRelaxed(Instruction* inst) {
  uint32_t r_id = inst->result_id();
  for (auto r_inst : get_decoration_mgr()->GetDecorationsFor(r_id, false))
    if (r_inst->opcode() == SpvOpDecorate &&
        r_inst->GetSingleWordInOperand(1) == SpvDecorationRelaxedPrecision)
      return true;
  return false;
}

bool ConvertToHalfPass::IsRelaxed(uint32_t id) {
  return relaxed_ids_set_.count(id) > 0;
}

void ConvertToHalfPass::AddRelaxed(uint32_t id) { relaxed_ids_set_.insert(id); }

analysis::Type* ConvertToHalfPass::FloatScalarType(uint32_t width) {
  analysis::Float float_ty(width);
  return context()->get_type_mgr()->GetRegisteredType(&float_ty);
}

analysis::Type* ConvertToHalfPass::FloatVectorType(uint32_t v_len,
                                                   uint32_t width) {
  analysis::Type* reg_float_ty = FloatScalarType(width);
  analysis::Vector vec_ty(reg_float_ty, v_len);
  return context()->get_type_mgr()->GetRegisteredType(&vec_ty);
}

analysis::Type* ConvertToHalfPass::FloatMatrixType(uint32_t v_cnt,
                                                   uint32_t vty_id,
                                                   uint32_t width) {
  Instruction* vty_inst = get_def_use_mgr()->GetDef(vty_id);
  uint32_t v_len = vty_inst->GetSingleWordInOperand(1);
  analysis::Type* reg_vec_ty = FloatVectorType(v_len, width);
  analysis::Matrix mat_ty(reg_vec_ty, v_cnt);
  return context()->get_type_mgr()->GetRegisteredType(&mat_ty);
}

uint32_t ConvertToHalfPass::EquivFloatTypeId(uint32_t ty_id, uint32_t width) {
  analysis::Type* reg_equiv_ty;
  Instruction* ty_inst = get_def_use_mgr()->GetDef(ty_id);
  if (ty_inst->opcode() == SpvOpTypeMatrix)
    reg_equiv_ty = FloatMatrixType(ty_inst->GetSingleWordInOperand(1),
                                   ty_inst->GetSingleWordInOperand(0), width);
  else if (ty_inst->opcode() == SpvOpTypeVector)
    reg_equiv_ty = FloatVectorType(ty_inst->GetSingleWordInOperand(1), width);
  else  // SpvOpTypeFloat
    reg_equiv_ty = FloatScalarType(width);
  return context()->get_type_mgr()->GetTypeInstruction(reg_equiv_ty);
}

void ConvertToHalfPass::GenConvert(uint32_t* val_idp, uint32_t width,
                                   Instruction* inst) {
  Instruction* val_inst = get_def_use_mgr()->GetDef(*val_idp);
  uint32_t ty_id = val_inst->type_id();
  uint32_t nty_id = EquivFloatTypeId(ty_id, width);
  if (nty_id == ty_id) return;
  Instruction* cvt_inst;
  InstructionBuilder builder(
      context(), inst,
      IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);
  if (val_inst->opcode() == SpvOpUndef)
    cvt_inst = builder.AddNullaryOp(nty_id, SpvOpUndef);
  else
    cvt_inst = builder.AddUnaryOp(nty_id, SpvOpFConvert, *val_idp);
  *val_idp = cvt_inst->result_id();
}

bool ConvertToHalfPass::MatConvertCleanup(Instruction* inst) {
  if (inst->opcode() != SpvOpFConvert) return false;
  uint32_t mty_id = inst->type_id();
  Instruction* mty_inst = get_def_use_mgr()->GetDef(mty_id);
  if (mty_inst->opcode() != SpvOpTypeMatrix) return false;
  uint32_t vty_id = mty_inst->GetSingleWordInOperand(0);
  uint32_t v_cnt = mty_inst->GetSingleWordInOperand(1);
  Instruction* vty_inst = get_def_use_mgr()->GetDef(vty_id);
  uint32_t cty_id = vty_inst->GetSingleWordInOperand(0);
  Instruction* cty_inst = get_def_use_mgr()->GetDef(cty_id);
  InstructionBuilder builder(
      context(), inst,
      IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);
  // Convert each component vector, combine them with OpCompositeConstruct
  // and replace original instruction.
  uint32_t orig_width = (cty_inst->GetSingleWordInOperand(0) == 16) ? 32 : 16;
  uint32_t orig_mat_id = inst->GetSingleWordInOperand(0);
  uint32_t orig_vty_id = EquivFloatTypeId(vty_id, orig_width);
  std::vector<Operand> opnds = {};
  for (uint32_t vidx = 0; vidx < v_cnt; ++vidx) {
    Instruction* ext_inst = builder.AddIdLiteralOp(
        orig_vty_id, SpvOpCompositeExtract, orig_mat_id, vidx);
    Instruction* cvt_inst =
        builder.AddUnaryOp(vty_id, SpvOpFConvert, ext_inst->result_id());
    opnds.push_back({SPV_OPERAND_TYPE_ID, {cvt_inst->result_id()}});
  }
  uint32_t mat_id = TakeNextId();
  std::unique_ptr<Instruction> mat_inst(new Instruction(
      context(), SpvOpCompositeConstruct, mty_id, mat_id, opnds));
  (void)builder.AddInstruction(std::move(mat_inst));
  context()->ReplaceAllUsesWith(inst->result_id(), mat_id);
  // Turn original instruction into copy so it is valid.
  inst->SetOpcode(SpvOpCopyObject);
  inst->SetResultType(EquivFloatTypeId(mty_id, orig_width));
  get_def_use_mgr()->AnalyzeInstUse(inst);
  return true;
}

void ConvertToHalfPass::RemoveRelaxedDecoration(uint32_t id) {
  context()->get_decoration_mgr()->RemoveDecorationsFrom(
      id, [](const Instruction& dec) {
        if (dec.opcode() == SpvOpDecorate &&
            dec.GetSingleWordInOperand(1u) == SpvDecorationRelaxedPrecision)
          return true;
        else
          return false;
      });
}

bool ConvertToHalfPass::GenHalfArith(Instruction* inst) {
  bool modified = false;
  // Convert all float32 based operands to float16 equivalent and change
  // instruction type to float16 equivalent.
  inst->ForEachInId([&inst, &modified, this](uint32_t* idp) {
    Instruction* op_inst = get_def_use_mgr()->GetDef(*idp);
    if (!IsFloat(op_inst, 32)) return;
    GenConvert(idp, 16, inst);
    modified = true;
  });
  if (IsFloat(inst, 32)) {
    inst->SetResultType(EquivFloatTypeId(inst->type_id(), 16));
    converted_ids_.insert(inst->result_id());
    modified = true;
  }
  if (modified) get_def_use_mgr()->AnalyzeInstUse(inst);
  return modified;
}

bool ConvertToHalfPass::ProcessPhi(Instruction* inst) {
  // Add float16 converts of any float32 operands and change type
  // of phi to float16 equivalent. Operand converts need to be added to
  // preceeding blocks.
  uint32_t ocnt = 0;
  uint32_t* prev_idp;
  inst->ForEachInId([&ocnt, &prev_idp, this](uint32_t* idp) {
    if (ocnt % 2 == 0) {
      prev_idp = idp;
    } else {
      Instruction* val_inst = get_def_use_mgr()->GetDef(*prev_idp);
      if (IsFloat(val_inst, 32)) {
        BasicBlock* bp = context()->get_instr_block(*idp);
        auto insert_before = bp->tail();
        if (insert_before != bp->begin()) {
          --insert_before;
          if (insert_before->opcode() != SpvOpSelectionMerge &&
              insert_before->opcode() != SpvOpLoopMerge)
            ++insert_before;
        }
        GenConvert(prev_idp, 16, &*insert_before);
      }
    }
    ++ocnt;
  });
  inst->SetResultType(EquivFloatTypeId(inst->type_id(), 16));
  get_def_use_mgr()->AnalyzeInstUse(inst);
  converted_ids_.insert(inst->result_id());
  return true;
}

bool ConvertToHalfPass::ProcessConvert(Instruction* inst) {
  // If float32 and relaxed, change to float16 convert
  if (IsFloat(inst, 32) && IsRelaxed(inst->result_id())) {
    inst->SetResultType(EquivFloatTypeId(inst->type_id(), 16));
    get_def_use_mgr()->AnalyzeInstUse(inst);
    converted_ids_.insert(inst->result_id());
  }
  // If operand and result types are the same, change FConvert to CopyObject to
  // keep validator happy; simplification and DCE will clean it up
  // One way this can happen is if an FConvert generated during this pass
  // (likely by ProcessPhi) is later encountered here and its operand has been
  // changed to half.
  uint32_t val_id = inst->GetSingleWordInOperand(0);
  Instruction* val_inst = get_def_use_mgr()->GetDef(val_id);
  if (inst->type_id() == val_inst->type_id()) inst->SetOpcode(SpvOpCopyObject);
  return true;  // modified
}

bool ConvertToHalfPass::ProcessImageRef(Instruction* inst) {
  bool modified = false;
  // If image reference, only need to convert dref args back to float32
  if (dref_image_ops_.count(inst->opcode()) != 0) {
    uint32_t dref_id = inst->GetSingleWordInOperand(kImageSampleDrefIdInIdx);
    if (converted_ids_.count(dref_id) > 0) {
      GenConvert(&dref_id, 32, inst);
      inst->SetInOperand(kImageSampleDrefIdInIdx, {dref_id});
      get_def_use_mgr()->AnalyzeInstUse(inst);
      modified = true;
    }
  }
  return modified;
}

bool ConvertToHalfPass::ProcessDefault(Instruction* inst) {
  bool modified = false;
  // If non-relaxed instruction has changed operands, need to convert
  // them back to float32
  inst->ForEachInId([&inst, &modified, this](uint32_t* idp) {
    if (converted_ids_.count(*idp) == 0) return;
    uint32_t old_id = *idp;
    GenConvert(idp, 32, inst);
    if (*idp != old_id) modified = true;
  });
  if (modified) get_def_use_mgr()->AnalyzeInstUse(inst);
  return modified;
}

bool ConvertToHalfPass::GenHalfInst(Instruction* inst) {
  bool modified = false;
  // Remember id for later deletion of RelaxedPrecision decoration
  bool inst_relaxed = IsRelaxed(inst->result_id());
  if (IsArithmetic(inst) && inst_relaxed)
    modified = GenHalfArith(inst);
  else if (inst->opcode() == SpvOpPhi && inst_relaxed)
    modified = ProcessPhi(inst);
  else if (inst->opcode() == SpvOpFConvert)
    modified = ProcessConvert(inst);
  else if (image_ops_.count(inst->opcode()) != 0)
    modified = ProcessImageRef(inst);
  else
    modified = ProcessDefault(inst);
  return modified;
}

bool ConvertToHalfPass::CloseRelaxInst(Instruction* inst) {
  if (inst->result_id() == 0) return false;
  if (IsRelaxed(inst->result_id())) return false;
  if (!IsFloat(inst, 32)) return false;
  if (IsDecoratedRelaxed(inst)) {
    AddRelaxed(inst->result_id());
    return true;
  }
  if (closure_ops_.count(inst->opcode()) == 0) return false;
  // Can relax if all float operands are relaxed
  bool relax = true;
  inst->ForEachInId([&relax, this](uint32_t* idp) {
    Instruction* op_inst = get_def_use_mgr()->GetDef(*idp);
    if (!IsFloat(op_inst, 32)) return;
    if (!IsRelaxed(*idp)) relax = false;
  });
  if (relax) {
    AddRelaxed(inst->result_id());
    return true;
  }
  // Can relax if all uses are relaxed
  relax = true;
  get_def_use_mgr()->ForEachUser(inst, [&relax, this](Instruction* uinst) {
    if (uinst->result_id() == 0 || !IsFloat(uinst, 32) ||
        (!IsDecoratedRelaxed(uinst) && !IsRelaxed(uinst->result_id()))) {
      relax = false;
      return;
    }
  });
  if (relax) {
    AddRelaxed(inst->result_id());
    return true;
  }
  return false;
}

bool ConvertToHalfPass::ProcessFunction(Function* func) {
  // Do a closure of Relaxed on composite and phi instructions
  bool changed = true;
  while (changed) {
    changed = false;
    cfg()->ForEachBlockInReversePostOrder(
        func->entry().get(), [&changed, this](BasicBlock* bb) {
          for (auto ii = bb->begin(); ii != bb->end(); ++ii)
            changed |= CloseRelaxInst(&*ii);
        });
  }
  // Do convert of relaxed instructions to half precision
  bool modified = false;
  cfg()->ForEachBlockInReversePostOrder(
      func->entry().get(), [&modified, this](BasicBlock* bb) {
        for (auto ii = bb->begin(); ii != bb->end(); ++ii)
          modified |= GenHalfInst(&*ii);
      });
  // Replace invalid converts of matrix into equivalent vector extracts,
  // converts and finally a composite construct
  cfg()->ForEachBlockInReversePostOrder(
      func->entry().get(), [&modified, this](BasicBlock* bb) {
        for (auto ii = bb->begin(); ii != bb->end(); ++ii)
          modified |= MatConvertCleanup(&*ii);
      });
  return modified;
}

Pass::Status ConvertToHalfPass::ProcessImpl() {
  Pass::ProcessFunction pfn = [this](Function* fp) {
    return ProcessFunction(fp);
  };
  bool modified = context()->ProcessEntryPointCallTree(pfn);
  // If modified, make sure module has Float16 capability
  if (modified) context()->AddCapability(SpvCapabilityFloat16);
  // Remove all RelaxedPrecision decorations from instructions and globals
  for (auto c_id : relaxed_ids_set_) RemoveRelaxedDecoration(c_id);
  for (auto& val : get_module()->types_values()) {
    uint32_t v_id = val.result_id();
    if (v_id != 0) RemoveRelaxedDecoration(v_id);
  }
  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

Pass::Status ConvertToHalfPass::Process() {
  Initialize();
  return ProcessImpl();
}

void ConvertToHalfPass::Initialize() {
  target_ops_core_ = {
      SpvOpVectorExtractDynamic,
      SpvOpVectorInsertDynamic,
      SpvOpVectorShuffle,
      SpvOpCompositeConstruct,
      SpvOpCompositeInsert,
      SpvOpCompositeExtract,
      SpvOpCopyObject,
      SpvOpTranspose,
      SpvOpConvertSToF,
      SpvOpConvertUToF,
      // SpvOpFConvert,
      // SpvOpQuantizeToF16,
      SpvOpFNegate,
      SpvOpFAdd,
      SpvOpFSub,
      SpvOpFMul,
      SpvOpFDiv,
      SpvOpFMod,
      SpvOpVectorTimesScalar,
      SpvOpMatrixTimesScalar,
      SpvOpVectorTimesMatrix,
      SpvOpMatrixTimesVector,
      SpvOpMatrixTimesMatrix,
      SpvOpOuterProduct,
      SpvOpDot,
      SpvOpSelect,
      SpvOpFOrdEqual,
      SpvOpFUnordEqual,
      SpvOpFOrdNotEqual,
      SpvOpFUnordNotEqual,
      SpvOpFOrdLessThan,
      SpvOpFUnordLessThan,
      SpvOpFOrdGreaterThan,
      SpvOpFUnordGreaterThan,
      SpvOpFOrdLessThanEqual,
      SpvOpFUnordLessThanEqual,
      SpvOpFOrdGreaterThanEqual,
      SpvOpFUnordGreaterThanEqual,
  };
  target_ops_450_ = {
      GLSLstd450Round, GLSLstd450RoundEven, GLSLstd450Trunc, GLSLstd450FAbs,
      GLSLstd450FSign, GLSLstd450Floor, GLSLstd450Ceil, GLSLstd450Fract,
      GLSLstd450Radians, GLSLstd450Degrees, GLSLstd450Sin, GLSLstd450Cos,
      GLSLstd450Tan, GLSLstd450Asin, GLSLstd450Acos, GLSLstd450Atan,
      GLSLstd450Sinh, GLSLstd450Cosh, GLSLstd450Tanh, GLSLstd450Asinh,
      GLSLstd450Acosh, GLSLstd450Atanh, GLSLstd450Atan2, GLSLstd450Pow,
      GLSLstd450Exp, GLSLstd450Log, GLSLstd450Exp2, GLSLstd450Log2,
      GLSLstd450Sqrt, GLSLstd450InverseSqrt, GLSLstd450Determinant,
      GLSLstd450MatrixInverse,
      // TODO(greg-lunarg): GLSLstd450ModfStruct,
      GLSLstd450FMin, GLSLstd450FMax, GLSLstd450FClamp, GLSLstd450FMix,
      GLSLstd450Step, GLSLstd450SmoothStep, GLSLstd450Fma,
      // TODO(greg-lunarg): GLSLstd450FrexpStruct,
      GLSLstd450Ldexp, GLSLstd450Length, GLSLstd450Distance, GLSLstd450Cross,
      GLSLstd450Normalize, GLSLstd450FaceForward, GLSLstd450Reflect,
      GLSLstd450Refract, GLSLstd450NMin, GLSLstd450NMax, GLSLstd450NClamp};
  image_ops_ = {SpvOpImageSampleImplicitLod,
                SpvOpImageSampleExplicitLod,
                SpvOpImageSampleDrefImplicitLod,
                SpvOpImageSampleDrefExplicitLod,
                SpvOpImageSampleProjImplicitLod,
                SpvOpImageSampleProjExplicitLod,
                SpvOpImageSampleProjDrefImplicitLod,
                SpvOpImageSampleProjDrefExplicitLod,
                SpvOpImageFetch,
                SpvOpImageGather,
                SpvOpImageDrefGather,
                SpvOpImageRead,
                SpvOpImageSparseSampleImplicitLod,
                SpvOpImageSparseSampleExplicitLod,
                SpvOpImageSparseSampleDrefImplicitLod,
                SpvOpImageSparseSampleDrefExplicitLod,
                SpvOpImageSparseSampleProjImplicitLod,
                SpvOpImageSparseSampleProjExplicitLod,
                SpvOpImageSparseSampleProjDrefImplicitLod,
                SpvOpImageSparseSampleProjDrefExplicitLod,
                SpvOpImageSparseFetch,
                SpvOpImageSparseGather,
                SpvOpImageSparseDrefGather,
                SpvOpImageSparseTexelsResident,
                SpvOpImageSparseRead};
  dref_image_ops_ = {
      SpvOpImageSampleDrefImplicitLod,
      SpvOpImageSampleDrefExplicitLod,
      SpvOpImageSampleProjDrefImplicitLod,
      SpvOpImageSampleProjDrefExplicitLod,
      SpvOpImageDrefGather,
      SpvOpImageSparseSampleDrefImplicitLod,
      SpvOpImageSparseSampleDrefExplicitLod,
      SpvOpImageSparseSampleProjDrefImplicitLod,
      SpvOpImageSparseSampleProjDrefExplicitLod,
      SpvOpImageSparseDrefGather,
  };
  closure_ops_ = {
      SpvOpVectorExtractDynamic,
      SpvOpVectorInsertDynamic,
      SpvOpVectorShuffle,
      SpvOpCompositeConstruct,
      SpvOpCompositeInsert,
      SpvOpCompositeExtract,
      SpvOpCopyObject,
      SpvOpTranspose,
      SpvOpPhi,
  };
  relaxed_ids_set_.clear();
  converted_ids_.clear();
}

}  // namespace opt
}  // namespace spvtools
