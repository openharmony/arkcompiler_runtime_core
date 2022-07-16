/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"

namespace panda::compiler {
/**
 * Check if loop is countable
 *
 * [Loop]
 * Phi(init, update)
 * ...
 * update(phi, 1)
 * Compare(Add/Sub, test)
 *
 * where `update` is Add or Sub instruction
 */
std::optional<CountableLoopInfo> CountableLoopParser::Parse()
{
    if (loop_.IsIrreducible() || loop_.IsOsrLoop() || loop_.IsTryCatchLoop() || loop_.GetBackEdges().size() != 1 ||
        loop_.IsRoot() || loop_.IsInfinite()) {
        return std::nullopt;
    }
    auto loop_exit = FindLoopExitBlock();
    if (loop_exit != loop_.GetHeader() && loop_exit != loop_.GetBackEdges()[0]) {
        return std::nullopt;
    }
    is_head_loop_exit_ = (loop_exit == loop_.GetHeader() && loop_exit != loop_.GetBackEdges()[0]);
    loop_info_.if_imm = loop_exit->GetLastInst();
    ASSERT(loop_info_.if_imm->GetOpcode() == Opcode::IfImm || loop_info_.if_imm->GetOpcode() == Opcode::If);
    auto loop_exit_cmp = loop_info_.if_imm->GetInput(0).GetInst();
    if (loop_exit_cmp->GetOpcode() != Opcode::Compare) {
        return std::nullopt;
    }
    if (is_head_loop_exit_ && !loop_exit_cmp->GetInput(0).GetInst()->IsPhi() &&
        !loop_exit_cmp->GetInput(1).GetInst()->IsPhi()) {
        return std::nullopt;
    }
    // Compare type should be signed integer
    // TODO (a.popov) Support loops with unsigned index
    auto cmp_type = loop_exit_cmp->CastToCompare()->GetOperandsType();
    if (DataType::GetCommonType(cmp_type) != DataType::INT64 || !DataType::IsTypeSigned(cmp_type)) {
        return std::nullopt;
    }
    if (!SetUpdateAndTestInputs()) {
        return std::nullopt;
    }

    if (!IsInstIncOrDec(loop_info_.update)) {
        return std::nullopt;
    }
    auto const_inst = SetIndexAndRetrunConstInst();
    if (loop_info_.index->GetBasicBlock() != loop_.GetHeader()) {
        return std::nullopt;
    }

    ASSERT(loop_info_.index->IsPhi());
    auto back_edge {loop_.GetBackEdges()[0]};
    auto back_edge_idx {loop_info_.index->CastToPhi()->GetPredBlockIndex(back_edge)};
    if (loop_info_.index->GetInput(back_edge_idx).GetInst() != loop_info_.update) {
        return std::nullopt;
    }
    ASSERT(loop_info_.index->GetInputsCount() == MAX_SUCCS_NUM);
    ASSERT(const_inst->GetType() == DataType::INT64);
    loop_info_.const_step = const_inst->CastToConstant()->GetIntValue();
    loop_info_.init = loop_info_.index->GetInput(1 - back_edge_idx).GetInst();
    SetNormalizedConditionCode();
    if (!IsConditionCodeAcceptable()) {
        return std::nullopt;
    }
    return loop_info_;
}

/*
 * Check if instruction is Add or Sub with constant and phi inputs
 */
bool CountableLoopParser::IsInstIncOrDec(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::Add && inst->GetOpcode() != Opcode::Sub) {
        return false;
    }
    ConstantInst *cnst = nullptr;
    if (inst->GetInput(0).GetInst()->IsConst() && inst->GetInput(1).GetInst()->IsPhi()) {
        cnst = inst->GetInput(0).GetInst()->CastToConstant();
    } else if (inst->GetInput(1).GetInst()->IsConst() && inst->GetInput(0).GetInst()->IsPhi()) {
        cnst = inst->GetInput(1).GetInst()->CastToConstant();
    }
    if (cnst == nullptr) {
        return false;
    }
    const uint64_t MASK = (1ULL << 63U);
    return !(DataType::IsTypeSigned(inst->GetType()) && (cnst->GetIntValue() & MASK) != 0);
}

// TODO(a.popov) Suppot 'GetLoopExit()' method in the 'Loop' class
BasicBlock *CountableLoopParser::FindLoopExitBlock()
{
    auto outer_loop = loop_.GetOuterLoop();
    for (auto block : loop_.GetBlocks()) {
        const auto &succs = block->GetSuccsBlocks();
        auto it = std::find_if(succs.begin(), succs.end(),
                               [&outer_loop](const BasicBlock *bb) { return bb->GetLoop() == outer_loop; });
        if (it != succs.end()) {
            return block;
        }
    }
    UNREACHABLE();
    return nullptr;
}

bool CountableLoopParser::SetUpdateAndTestInputs()
{
    auto loop_exit_cmp = loop_info_.if_imm->GetInput(0).GetInst();
    ASSERT(loop_exit_cmp->GetOpcode() == Opcode::Compare);
    loop_info_.update = loop_exit_cmp->GetInput(0).GetInst();
    loop_info_.test = loop_exit_cmp->GetInput(1).GetInst();
    if (is_head_loop_exit_) {
        if (!loop_info_.update->IsPhi()) {
            std::swap(loop_info_.update, loop_info_.test);
        }
        ASSERT(loop_info_.update->IsPhi());
        if (loop_info_.update->GetBasicBlock() != loop_.GetHeader()) {
            return false;
        }
        auto back_edge {loop_.GetBackEdges()[0]};
        loop_info_.update = loop_info_.update->CastToPhi()->GetPhiInput(back_edge);
    } else {
        if (!IsInstIncOrDec(loop_info_.update)) {
            std::swap(loop_info_.update, loop_info_.test);
        }
    }

    return true;
}

Inst *CountableLoopParser::SetIndexAndRetrunConstInst()
{
    loop_info_.index = loop_info_.update->GetInput(0).GetInst();
    auto const_inst = loop_info_.update->GetInput(1).GetInst();
    if (loop_info_.index->IsConst()) {
        loop_info_.index = loop_info_.update->GetInput(1).GetInst();
        const_inst = loop_info_.update->GetInput(0).GetInst();
    }
    return const_inst;
}

void CountableLoopParser::SetNormalizedConditionCode()
{
    auto loop_exit = loop_info_.if_imm->GetBasicBlock();
    ASSERT(loop_exit != nullptr);
    auto loop_exit_cmp = loop_info_.if_imm->GetInput(0).GetInst();
    ASSERT(loop_exit_cmp->GetOpcode() == Opcode::Compare);
    auto cc = loop_exit_cmp->CastToCompare()->GetCc();
    if (loop_info_.test == loop_exit_cmp->GetInput(0).GetInst()) {
        cc = SwapOperandsConditionCode(cc);
    }
    ASSERT(loop_info_.if_imm->CastToIfImm()->GetImm() == 0);
    if (loop_info_.if_imm->CastToIfImm()->GetCc() == CC_EQ) {
        cc = GetInverseConditionCode(cc);
    } else {
        ASSERT(loop_info_.if_imm->CastToIfImm()->GetCc() == CC_NE);
    }
    auto loop = loop_exit->GetLoop();
    if (loop_exit->GetFalseSuccessor()->GetLoop() == loop) {
        cc = GetInverseConditionCode(cc);
    } else {
        ASSERT(loop_exit->GetTrueSuccessor()->GetLoop() == loop);
    }
    loop_info_.normalized_cc = cc;
}

bool CountableLoopParser::IsConditionCodeAcceptable()
{
    auto cc = loop_info_.normalized_cc;
    bool is_inc = loop_info_.update->GetOpcode() == Opcode::Add;
    // Condition should be: inc <= test | inc < test
    if (is_inc && cc != CC_LE && cc != CC_LT) {
        return false;
    }
    // Condition should be: dec >= test | dec > test
    if (!is_inc && cc != CC_GE && cc != CC_GT) {
        return false;
    }
    return true;
}
}  // namespace panda::compiler
