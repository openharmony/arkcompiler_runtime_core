/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir/inst.h"
#include "tests/unit_test.h"

#include <cstdlib>

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class EtsAsyncIrBuilderTest : public AsmTest {
public:
    EtsAsyncIrBuilderTest() = default;
    ~EtsAsyncIrBuilderTest() override = default;
    NO_COPY_SEMANTIC(EtsAsyncIrBuilderTest);
    NO_MOVE_SEMANTIC(EtsAsyncIrBuilderTest);
};

static Inst *FindFirstInst(BasicBlock *block, Opcode opcode)
{
    for (auto inst : block->AllInsts()) {
        if (inst->GetOpcode() == opcode) {
            return inst;
        }
    }
    return nullptr;
}

static Inst *FindFirstInst(Graph *graph, Opcode opcode)
{
    for (auto block : graph->GetBlocksRPO()) {
        auto inst = FindFirstInst(block, opcode);
        if (inst != nullptr) {
            return inst;
        }
    }
    return nullptr;
}

static size_t CountInstInBlock(BasicBlock *block, Opcode opcode)
{
    size_t count = 0;
    for (auto inst : block->AllInsts()) {
        if (inst->GetOpcode() == opcode) {
            ++count;
        }
    }
    return count;
}

static size_t CountInstInGraph(Graph *graph, Opcode opcode)
{
    size_t count = 0;
    for (auto block : graph->GetBlocksRPO()) {
        count += CountInstInBlock(block, opcode);
    }
    return count;
}

// CC-OFFNXT(huge_method, G.FUN.01) test, solid logic
TEST_F(EtsAsyncIrBuilderTest, StacklessAsyncFlow)
{
    // Fake bytecode to test Dispatch basic block creation
    auto source = R"(
    .record arkruntime.AsyncContext {
    }
    .record std.core.Promise {
    }
    .record std.core.Object {
    }
    .record std.core.Error {
    }

    .function arkruntime.AsyncContext arkruntime.AsyncContext.current() <static> {
        lda.null
        return.obj
    }

    .function std.core.Promise async_foo(std.core.Promise a0) {
    try_begin:
        call.short arkruntime.AsyncContext.current
        sta.obj v0
        ets.async.dispatch v0
        lda.null
        sta.obj v0
        ets.async.suspend v0
        lda.null
        return.obj
    try_end:
        lda.null
        return.obj

    .catch std.core.Error, try_begin, try_end, try_end
    }
    )";

    ASSERT_TRUE(ParseToGraph(source, "async_foo"));

#ifdef COMPILER_DEBUG_CHECKS
    GetGraph()->SetInliningComplete();
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
#endif

    ASSERT_EQ(CountInstInGraph(GetGraph(), Opcode::Dispatch), 1U);
    ASSERT_EQ(CountInstInGraph(GetGraph(), Opcode::SaveStateSuspend), 1U);

    auto dispatch = FindFirstInst(GetGraph(), Opcode::Dispatch);
    auto saveStateSuspend = FindFirstInst(GetGraph(), Opcode::SaveStateSuspend);
    ASSERT_NE(dispatch, nullptr);
    ASSERT_NE(saveStateSuspend, nullptr);
    auto saveStateSuspendInst = saveStateSuspend->CastToSaveStateSuspend();

    auto dispatchBlock = dispatch->GetBasicBlock();
    ASSERT_EQ(dispatchBlock->GetFirstInst(), dispatch);
    ASSERT_EQ(dispatchBlock->GetLastInst(), dispatch);
    ASSERT_TRUE(dispatchBlock->GetSuccessor(0U)->IsEndBlock());

    auto prologueBlock = dispatchBlock->GetPredecessor(0U);
    ASSERT_EQ(prologueBlock, GetGraph()->GetStartBlock()->GetSuccessor(0U)->GetSuccessor(0U));  // skip try begin
    ASSERT_EQ(prologueBlock->GetTrueSuccessor(), dispatchBlock);
    ASSERT_EQ(prologueBlock->GetLastInst()->GetOpcode(), Opcode::IfImm);
    ASSERT_TRUE(prologueBlock->GetLastInst()->CastToIfImm()->IsUnlikely());
    ASSERT_EQ(CountInstInBlock(prologueBlock, Opcode::CallStatic), 1U);
    ASSERT_EQ(dispatch->GetInput(0U).GetInst(), FindFirstInst(prologueBlock, Opcode::CallStatic));

    auto continuationBlock = prologueBlock->GetFalseSuccessor();
    ASSERT_EQ(saveStateSuspend->GetBasicBlock(), continuationBlock);
    ASSERT_EQ(continuationBlock->GetLastInst()->GetOpcode(), Opcode::Return);
    ASSERT_NE(saveStateSuspendInst->GetAsyncContext(), nullptr);
    ASSERT_EQ(saveStateSuspendInst->GetVirtualRegister(0U).Value(), 0U);
}

}  // namespace ark::compiler
