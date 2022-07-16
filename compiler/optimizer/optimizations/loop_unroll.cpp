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
#include "compiler_logger.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/optimizations/loop_unroll.h"

namespace panda::compiler {
bool LoopUnroll::RunImpl()
{
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Run " << GetPassName();
    RunLoopsVisitor();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << GetPassName() << " complete";
    return is_applied_;
}

void LoopUnroll::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

/**
 * TODO(a.popov) Create pre-header compare if it doesn't exist
 */
bool LoopUnroll::HasPreHeaderCompare(Loop *loop, const CountableLoopInfo &loop_info)
{
    auto pre_header = loop->GetPreHeader();
    if (pre_header->IsEmpty() || pre_header->GetLastInst()->GetOpcode() != Opcode::IfImm) {
        return false;
    }
    auto back_edge = loop->GetBackEdges()[0];
    ASSERT(loop_info.if_imm->GetBasicBlock() == back_edge);
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
    if (back_edge_cmp->GetInput(0).GetInst() == loop_info.test) {
        test_input_idx = 0;
    } else {
        ASSERT(back_edge_cmp->GetInput(1).GetInst() == loop_info.test);
    }

    return pre_header_cmp->GetInput(test_input_idx).GetInst() == loop_info.test;
}

bool ConditionOverFlow(const CountableLoopInfo &loop_info, uint32_t unroll_factor)
{
    auto type = loop_info.index->GetType();
    ASSERT(DataType::GetCommonType(type) == DataType::INT64 && DataType::IsTypeSigned(type));
    if (!loop_info.test->IsConst()) {
        return false;
    }
    bool is_inc = loop_info.update->GetOpcode() == Opcode::Add;
    int64_t imm_value = (static_cast<int64_t>(unroll_factor) - 1) * loop_info.const_step;
    int64_t test_value = loop_info.test->CastToConstant()->GetIntValue();

    auto is_int32 = DataType::Is32Bits(type, loop_info.test->GetBasicBlock()->GetGraph()->GetArch());
    auto type_min = is_int32 ? std::numeric_limits<int32_t>::min() : std::numeric_limits<int64_t>::min();
    auto type_max = is_int32 ? std::numeric_limits<int32_t>::max() : std::numeric_limits<int64_t>::max();
    if (is_inc) {
        // condition will be updated: test_value - imm_value
        // so if (test_value - imm_value) < type_min, it's overflow
        return (type_min + imm_value) > test_value;
    }
    // condition will be updated: test_value + imm_value
    // so if (test_value + imm_value) > type_max, it's overflow
    return (type_max - imm_value) < test_value;
}

bool LoopUnroll::TransformLoop(Loop *loop)
{
    auto unroll_params = GetUnrollParams(loop);
    if (!options.IsCompilerUnrollLoopWithCalls() && unroll_params.has_call) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Loop isn't unrolled since it contains calls. Loop id = " << loop->GetId();
        return false;
    }

    uint32_t unroll_factor = std::min(unroll_params.unroll_factor, UNROLL_FACTOR);
    if (unroll_factor <= 1) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Loop isn't unrolled due to unroll factor = " << unroll_factor << ". Loop id = " << loop->GetId();
        return false;
    }

    auto graph_cloner = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator());
    auto loop_parser = CountableLoopParser(*loop);
    auto loop_info = loop_parser.Parse();
    if (loop_info.has_value() && !ConditionOverFlow(loop_info.value(), unroll_factor) &&
        HasPreHeaderCompare(loop, loop_info.value())) {
        auto clone_loop = graph_cloner.CloneLoop(loop);
        // Unroll loop without side-exits and fix compare in the pre-header and back-edge
        graph_cloner.UnrollLoopBody<UnrollType::UNROLL_WITHOUT_SIDE_EXITS>(loop, unroll_factor);
        FixCompareInst(loop_info.value(), loop->GetHeader(), unroll_factor);
        // Unroll loop with side-exits for remaining iterations
        graph_cloner.UnrollLoopBody<UnrollType::UNROLL_POST_INCREMENT>(clone_loop, unroll_factor - 1);
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Unrolled without side-exits the loop with unroll factor = " << unroll_factor
            << ". Loop id = " << loop->GetId();
    } else if (options.IsCompilerUnrollWithSideExits()) {
        graph_cloner.UnrollLoopBody<UnrollType::UNROLL_WITH_SIDE_EXITS>(loop, unroll_factor);
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Unrolled with side-exits the loop with unroll factor = " << unroll_factor
            << ". Loop id = " << loop->GetId();
    }
    is_applied_ = true;
    GetGraph()->GetEventWriter().EventLoopUnroll(loop->GetId(), loop->GetHeader()->GetGuestPc(), unroll_factor,
                                                 unroll_params.cloneable_insts);
    return true;
}

/**
 * @return - unroll parameters:
 * - maximum value of unroll factor, depends on INST_LIMIT
 * - number of cloneable instructions
 */
LoopUnroll::UnrollParams LoopUnroll::GetUnrollParams(Loop *loop)
{
    uint32_t base_inst_count = 0;
    uint32_t not_cloneable_count = 0;
    bool has_call = false;
    for (auto block : loop->GetBlocks()) {
        for (auto inst : block->AllInsts()) {
            base_inst_count++;
            if ((block->IsLoopHeader() && inst->IsPhi()) || inst->GetOpcode() == Opcode::SafePoint) {
                not_cloneable_count++;
            }
            has_call |= inst->IsCall();
        }
    }

    UnrollParams params = {1, (base_inst_count - not_cloneable_count), has_call};
    if (base_inst_count >= INST_LIMIT) {
        return params;
    }
    uint32_t can_be_cloned_count = INST_LIMIT - base_inst_count;
    params.unroll_factor = UNROLL_FACTOR;
    if (params.cloneable_insts > 0) {
        params.unroll_factor = (can_be_cloned_count / params.cloneable_insts) + 1;
    }
    return params;
}

/**
 * @return - `if_imm`'s compare input when `if_imm` its single user,
 * otherwise create a new one Compare for this `if_imm` and return it
 */
Inst *GetOrCreateIfImmUniqueCompare(Inst *if_imm)
{
    ASSERT(if_imm->GetOpcode() == Opcode::IfImm);
    auto compare = if_imm->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    if (compare->HasSingleUser()) {
        return compare;
    }
    auto new_cmp = compare->Clone(compare->GetBasicBlock()->GetGraph());
    new_cmp->SetInput(0, compare->GetInput(0).GetInst());
    new_cmp->SetInput(1, compare->GetInput(1).GetInst());
    if_imm->InsertBefore(new_cmp);
    if_imm->SetInput(0, new_cmp);
    return new_cmp;
}

/**
 * Normalize control-flow to the form: `if condition is true goto loop_header`
 */
void NormalizeControlFlow(BasicBlock *edge, const BasicBlock *loop_header)
{
    auto if_imm = edge->GetLastInst()->CastToIfImm();
    ASSERT(if_imm->GetImm() == 0);
    if (if_imm->GetCc() == CC_EQ) {
        if_imm->SetCc(CC_NE);
        edge->SwapTrueFalseSuccessors<true>();
    }
    auto cmp = if_imm->GetInput(0).GetInst()->CastToCompare();
    ASSERT(cmp->GetBasicBlock() == edge);
    if (edge->GetFalseSuccessor() == loop_header) {
        auto inversed_cc = GetInverseConditionCode(cmp->GetCc());
        cmp->SetCc(inversed_cc);
        edge->SwapTrueFalseSuccessors<true>();
    }
}

Inst *LoopUnroll::CreateNewTestInst(const CountableLoopInfo &loop_info, Inst *const_inst, Inst *pre_header_cmp)
{
    Inst *test = nullptr;
    bool is_inc = loop_info.update->GetOpcode() == Opcode::Add;
    if (is_inc) {
        test = GetGraph()->CreateInstSub(pre_header_cmp->CastToCompare()->GetOperandsType(), pre_header_cmp->GetPc());
    } else {
        ASSERT(loop_info.update->GetOpcode() == Opcode::Sub);
        test = GetGraph()->CreateInstAdd(pre_header_cmp->CastToCompare()->GetOperandsType(), pre_header_cmp->GetPc());
    }
    test->SetInput(0, loop_info.test);
    test->SetInput(1, const_inst);
    pre_header_cmp->InsertBefore(test);
    return test;
}

/**
 * Replace `Compare(init, test)` with these instructions:
 *
 * Constant(unroll_factor)
 * Sub/Add(test, Constant)
 * Compare(init, SubI/AddI)
 *
 * And replace condition code if it is `CC_NE`.
 * We use Constant + Sub/Add because low-level instructions (SubI/AddI) may appear only after Lowering pass.
 */
void LoopUnroll::FixCompareInst(const CountableLoopInfo &loop_info, BasicBlock *header, uint32_t unroll_factor)
{
    auto pre_header = header->GetLoop()->GetPreHeader();
    auto back_edge = loop_info.if_imm->GetBasicBlock();
    ASSERT(!pre_header->IsEmpty() && pre_header->GetLastInst()->GetOpcode() == Opcode::IfImm);
    auto pre_header_if = pre_header->GetLastInst()->CastToIfImm();
    auto pre_header_cmp = GetOrCreateIfImmUniqueCompare(pre_header_if);
    auto back_edge_cmp = GetOrCreateIfImmUniqueCompare(loop_info.if_imm);
    NormalizeControlFlow(pre_header, header);
    NormalizeControlFlow(back_edge, header);
    // Create Sub/Add + Const instructions and replace Compare's test inst input
    auto imm_value = (static_cast<uint64_t>(unroll_factor) - 1) * loop_info.const_step;
    auto new_test = CreateNewTestInst(loop_info, GetGraph()->FindOrCreateConstant(imm_value), pre_header_cmp);
    auto test_input_idx = 1;
    if (back_edge_cmp->GetInput(0).GetInst() == loop_info.test) {
        test_input_idx = 0;
    } else {
        ASSERT(back_edge_cmp->GetInput(1).GetInst() == loop_info.test);
    }
    ASSERT(pre_header_cmp->GetInput(test_input_idx).GetInst() == loop_info.test);
    pre_header_cmp->SetInput(test_input_idx, new_test);
    back_edge_cmp->SetInput(test_input_idx, new_test);
    // Replace CC_NE ConditionCode
    if (loop_info.normalized_cc == CC_NE) {
        auto cc = loop_info.update->GetOpcode() == Opcode::Add ? CC_LT : CC_GT;
        if (test_input_idx == 0) {
            cc = SwapOperandsConditionCode(cc);
        }
        pre_header_cmp->CastToCompare()->SetCc(cc);
        back_edge_cmp->CastToCompare()->SetCc(cc);
    }
    // for not constant test-instruction we need to insert `overflow-check`:
    // `test - imm_value` should be less than `test` (incerement loop-index case)
    // `test + imm_value` should be greater than `test` (decrement loop-index case)
    // If overflow-check is failed goto after-loop
    if (!loop_info.test->IsConst()) {
        auto cc = loop_info.update->GetOpcode() == Opcode::Add ? CC_LT : CC_GT;
        // Create overflow_compare
        auto overflow_compare = GetGraph()->CreateInstCompare(compiler::DataType::BOOL, pre_header_cmp->GetPc(), cc);
        overflow_compare->CastToCompare()->SetOperandsType(loop_info.test->GetType());
        overflow_compare->SetInput(0, new_test);
        overflow_compare->SetInput(1, loop_info.test);
        // Create (pre_header_compare AND overflow_compare) inst
        auto and_inst = GetGraph()->CreateInstAnd(DataType::BOOL, pre_header_cmp->GetPc());
        and_inst->SetInput(0, pre_header_cmp);
        and_inst->SetInput(1, overflow_compare);
        pre_header_if->SetInput(0, and_inst);
        pre_header_if->InsertBefore(and_inst);
        and_inst->InsertBefore(overflow_compare);
    }
}
}  // namespace panda::compiler
