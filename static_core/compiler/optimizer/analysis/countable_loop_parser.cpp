/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "optimizer/analysis/bounds_analysis.h"
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
    if (loop_exit->IsEmpty()) {
        return std::nullopt;
    }
    if (loop_exit != loop_.GetHeader() && loop_exit != loop_.GetBackEdges()[0]) {
        return std::nullopt;
    }
    is_head_loop_exit_ = (loop_exit == loop_.GetHeader() && loop_exit != loop_.GetBackEdges()[0]);
    loop_info_.if_imm = loop_exit->GetLastInst();
    if (loop_info_.if_imm->GetOpcode() != Opcode::IfImm && loop_info_.if_imm->GetOpcode() != Opcode::If) {
        return std::nullopt;
    }
    auto loop_exit_cmp = loop_info_.if_imm->GetInput(0).GetInst();
    if (loop_exit_cmp->GetOpcode() != Opcode::Compare) {
        return std::nullopt;
    }
    if (is_head_loop_exit_ && !loop_exit_cmp->GetInput(0).GetInst()->IsPhi() &&
        !loop_exit_cmp->GetInput(1).GetInst()->IsPhi()) {
        return std::nullopt;
    }
    auto cmp_type = loop_exit_cmp->CastToCompare()->GetOperandsType();
    if (DataType::GetCommonType(cmp_type) != DataType::INT64) {
        return std::nullopt;
    }
    if (!SetUpdateAndTestInputs()) {
        return std::nullopt;
    }

    if (!IsInstIncOrDec(loop_info_.update)) {
        return std::nullopt;
    }
    SetIndexAndConstStep();
    if (loop_info_.index->GetBasicBlock() != loop_.GetHeader()) {
        return std::nullopt;
    }

    if (!TryProcessBackEdge()) {
        return std::nullopt;
    }
    return loop_info_;
}

bool CountableLoopParser::TryProcessBackEdge()
{
    ASSERT(loop_info_.index->IsPhi());
    auto back_edge {loop_.GetBackEdges()[0]};
    auto back_edge_idx {loop_info_.index->CastToPhi()->GetPredBlockIndex(back_edge)};
    if (loop_info_.index->GetInput(back_edge_idx).GetInst() != loop_info_.update) {
        return false;
    }
    ASSERT(loop_info_.index->GetInputsCount() == MAX_SUCCS_NUM);
    loop_info_.init = loop_info_.index->GetInput(1 - back_edge_idx).GetInst();
    SetNormalizedConditionCode();
    return IsConditionCodeAcceptable();
}

bool CountableLoopParser::HasPreHeaderCompare(Loop *loop, const CountableLoopInfo &loop_info)
{
    auto pre_header = loop->GetPreHeader();
    auto back_edge = loop->GetBackEdges()[0];
    if (loop_info.if_imm->GetBasicBlock() != back_edge || pre_header->IsEmpty() ||
        pre_header->GetLastInst()->GetOpcode() != Opcode::IfImm) {
        return false;
    }
    auto pre_header_if_imm = pre_header->GetLastInst();
    ASSERT(pre_header_if_imm->GetOpcode() == Opcode::IfImm);
    auto pre_header_cmp = pre_header_if_imm->GetInput(0).GetInst();
    if (pre_header_cmp->GetOpcode() != Opcode::Compare) {
        return false;
    }
    auto back_edge_cmp = loop_info.if_imm->GetInput(0).GetInst();
    ASSERT(back_edge_cmp->GetOpcode() == Opcode::Compare);

    // Compare condition codes
    if (pre_header_cmp->CastToCompare()->GetCc() != back_edge_cmp->CastToCompare()->GetCc()) {
        return false;
    }

    if (loop_info.if_imm->CastToIfImm()->GetCc() != pre_header_if_imm->CastToIfImm()->GetCc() ||
        loop_info.if_imm->CastToIfImm()->GetImm() != pre_header_if_imm->CastToIfImm()->GetImm()) {
        return false;
    }

    // Compare control-flow
    if (pre_header->GetTrueSuccessor() != back_edge->GetTrueSuccessor() ||
        pre_header->GetFalseSuccessor() != back_edge->GetFalseSuccessor()) {
        return false;
    }

    // Compare test inputs
    auto test_input_idx = 1;
    if (back_edge_cmp->GetInput(0) == loop_info.test) {
        test_input_idx = 0;
    } else {
        ASSERT(back_edge_cmp->GetInput(1) == loop_info.test);
    }

    return pre_header_cmp->GetInput(test_input_idx).GetInst() == loop_info.test &&
           pre_header_cmp->GetInput(1 - test_input_idx).GetInst() == loop_info.init;
}

// Returns exact number of iterations for loop with constant boundaries
// if its index does not overflow
std::optional<uint64_t> CountableLoopParser::GetLoopIterations(const CountableLoopInfo &loop_info)
{
    if (!loop_info.init->IsConst() || !loop_info.test->IsConst() || loop_info.const_step == 0) {
        return std::nullopt;
    }
    uint64_t init_value = loop_info.init->CastToConstant()->GetInt64Value();
    uint64_t test_value = loop_info.test->CastToConstant()->GetInt64Value();
    auto type = loop_info.index->GetType();

    if (loop_info.is_inc) {
        int64_t max_test = BoundsRange::GetMax(type) - static_cast<int64_t>(loop_info.const_step);
        if (loop_info.normalized_cc == CC_LE) {
            max_test--;
        }
        if (static_cast<int64_t>(test_value) > max_test) {
            // index may overflow
            return std::nullopt;
        }
    } else {
        int64_t min_test = BoundsRange::GetMin(type) + static_cast<int64_t>(loop_info.const_step);
        if (loop_info.normalized_cc == CC_GE) {
            min_test++;
        }
        if (static_cast<int64_t>(test_value) < min_test) {
            // index may overflow
            return std::nullopt;
        }
        std::swap(init_value, test_value);
    }
    if (static_cast<int64_t>(init_value) > static_cast<int64_t>(test_value)) {
        return 0;
    }
    uint64_t diff = test_value - init_value;
    uint64_t count = diff + loop_info.const_step;
    if (diff > std::numeric_limits<uint64_t>::max() - loop_info.const_step) {
        // count may overflow
        return std::nullopt;
    }
    if (loop_info.normalized_cc == CC_LT || loop_info.normalized_cc == CC_GT) {
        count--;
    }
    return count / loop_info.const_step;
}

/*
 * Check if instruction is Add or Sub with constant and phi inputs
 */
bool CountableLoopParser::IsInstIncOrDec(Inst *inst)
{
    if (!inst->IsAddSub()) {
        return false;
    }
    ConstantInst *cnst = nullptr;
    if (inst->GetInput(0).GetInst()->IsConst() && inst->GetInput(1).GetInst()->IsPhi()) {
        cnst = inst->GetInput(0).GetInst()->CastToConstant();
    } else if (inst->GetInput(1).GetInst()->IsConst() && inst->GetInput(0).GetInst()->IsPhi()) {
        cnst = inst->GetInput(1).GetInst()->CastToConstant();
    }
    return cnst != nullptr;
}

// NOTE(a.popov) Suppot 'GetLoopExit()' method in the 'Loop' class
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

void CountableLoopParser::SetIndexAndConstStep()
{
    loop_info_.index = loop_info_.update->GetInput(0).GetInst();
    auto const_inst = loop_info_.update->GetInput(1).GetInst();
    if (loop_info_.index->IsConst()) {
        loop_info_.index = loop_info_.update->GetInput(1).GetInst();
        const_inst = loop_info_.update->GetInput(0).GetInst();
    }

    ASSERT(const_inst->GetType() == DataType::INT64);
    auto cnst = const_inst->CastToConstant()->GetIntValue();
    const uint64_t mask = (1ULL << 63U);
    auto is_neg = DataType::IsTypeSigned(loop_info_.update->GetType()) && (cnst & mask) != 0;
    loop_info_.is_inc = loop_info_.update->IsAdd();
    if (is_neg) {
        cnst = ~cnst + 1;
        loop_info_.is_inc = !loop_info_.is_inc;
    }
    loop_info_.const_step = cnst;
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
    if (loop_exit->GetFalseSuccessor()->GetLoop() == loop ||
        loop_exit->GetFalseSuccessor()->GetLoop()->GetOuterLoop() == loop) {
        cc = GetInverseConditionCode(cc);
    } else {
        ASSERT(loop_exit->GetTrueSuccessor()->GetLoop() == loop ||
               loop_exit->GetTrueSuccessor()->GetLoop()->GetOuterLoop() == loop);
    }
    loop_info_.normalized_cc = cc;
}

bool CountableLoopParser::IsConditionCodeAcceptable()
{
    auto cc = loop_info_.normalized_cc;
    // Condition should be: inc <= test | inc < test
    if (loop_info_.is_inc && cc != CC_LE && cc != CC_LT) {
        return false;
    }
    // Condition should be: dec >= test | dec > test
    if (!loop_info_.is_inc && cc != CC_GE && cc != CC_GT) {
        return false;
    }
    return true;
}
}  // namespace panda::compiler
