// Copyright (c) 2018 Google LLC
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

#include "source/reduce/reduction_util.h"

#include "source/opt/ir_context.h"

namespace spvtools {
namespace reduce {

using opt::IRContext;
using opt::Instruction;

const uint32_t kTrueBranchOperandIndex = 1;
const uint32_t kFalseBranchOperandIndex = 2;

uint32_t FindOrCreateGlobalUndef(IRContext* context, uint32_t type_id) {
  for (auto& inst : context->module()->types_values()) {
    if (inst.opcode() != SpvOpUndef) {
      continue;
    }
    if (inst.type_id() == type_id) {
      return inst.result_id();
    }
  }
  // TODO(2182): this is adapted from MemPass::Type2Undef.  In due course it
  // would be good to factor out this duplication.
  const uint32_t undef_id = context->TakeNextId();
  std::unique_ptr<Instruction> undef_inst(
      new Instruction(context, SpvOpUndef, type_id, undef_id, {}));
  assert(undef_id == undef_inst->result_id());
  context->module()->AddGlobalValue(std::move(undef_inst));
  return undef_id;
}

void AdaptPhiInstructionsForRemovedEdge(uint32_t from_id,
                                        opt::BasicBlock* to_block) {
  to_block->ForEachPhiInst([&from_id](Instruction* phi_inst) {
    Instruction::OperandList new_in_operands;
    // Go through the OpPhi's input operands in (variable, parent) pairs.
    for (uint32_t index = 0; index < phi_inst->NumInOperands(); index += 2) {
      // Keep all pairs where the parent is not the block from which the edge
      // is being removed.
      if (phi_inst->GetInOperand(index + 1).words[0] != from_id) {
        new_in_operands.push_back(phi_inst->GetInOperand(index));
        new_in_operands.push_back(phi_inst->GetInOperand(index + 1));
      }
    }
    phi_inst->SetInOperands(std::move(new_in_operands));
  });
}

}  // namespace reduce
}  // namespace spvtools
