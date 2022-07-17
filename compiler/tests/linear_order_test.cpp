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

#include "unit_test.h"
#include "jit/profiling_data.h"
#include "compiler/optimizer/analysis/linear_order.h"
#include "compiler/optimizer/optimizations/loop_peeling.h"
#include "compiler/optimizer/optimizations/loop_unroll.h"

namespace panda::compiler {
class LinearOrderTest : public AsmTest {
public:
    LinearOrderTest() : default_threshold_(compiler::options.GetCompilerFreqBasedBranchReorderThreshold())
    {
        compiler::options.SetCompilerFreqBasedBranchReorderThreshold(10);
    }

    ~LinearOrderTest() override
    {
        compiler::options.SetCompilerFreqBasedBranchReorderThreshold(default_threshold_);
    }

    void StartProfiling()
    {
        auto method = reinterpret_cast<Method *>(GetGraph()->GetMethod());
        Locks::mutator_lock->WriteLock();
        method->StartProfiling();
        Locks::mutator_lock->Unlock();
    }

    void UpdateBranchTaken(uint32_t pc, uint32_t count = 1)
    {
        auto method = reinterpret_cast<Method *>(GetGraph()->GetMethod());
        auto profiling_data = method->GetProfilingData();
        for (uint32_t i = 0; i < count; i++) {
            profiling_data->UpdateBranchTaken(pc);
        }
    }

    void UpdateBranchNotTaken(uint32_t pc, uint32_t count = 1)
    {
        auto method = reinterpret_cast<Method *>(GetGraph()->GetMethod());
        auto profiling_data = method->GetProfilingData();
        for (uint32_t i = 0; i < count; i++) {
            profiling_data->UpdateBranchNotTaken(pc);
        }
    }

    void Reset()
    {
        GetGraph()->GetValidAnalysis<LinearOrder>().SetValid(false);
        auto blocks = GetGraph()->GetVectorBlocks();
        std::for_each(std::begin(blocks), std::end(blocks), [](BasicBlock *b) {
            if (b != nullptr) {
                b->SetNeedsJump(false);
            }
        });
    }

    const BasicBlock *GetOrderedBasicBlock(uint32_t id, uint32_t pos)
    {
        const auto &blocks = GetGraph()->GetBlocksLinearOrder();
        return *(std::find_if(std::begin(blocks), std::end(blocks), [id](BasicBlock *b) { return b->GetId() == id; }) +
                 pos);
    }

    bool CheckOrder(uint32_t id, std::initializer_list<uint32_t> expected)
    {
        Reset();
        const auto &blocks = GetGraph()->GetBlocksLinearOrder();
        auto actual_it =
            std::find_if(std::begin(blocks), std::end(blocks), [id](BasicBlock *b) { return b->GetId() == id; }) + 1;
        auto expected_it = std::begin(expected);
        while (actual_it != std::end(blocks) && expected_it != std::end(expected)) {
            if ((*actual_it)->GetId() != *expected_it) {
                return false;
            }
            ++actual_it;
            ++expected_it;
        }
        return expected_it == std::end(expected);
    }

private:
    uint32_t default_threshold_;
};

TEST_F(LinearOrderTest, RareLoopSideExit)
{
    auto source = R"(
    .record Test <> {
            u1 a <static>
    }
    .function i32 foo() <static> {
            movi v0, 0xa
            movi v1, 0x0
            mov v2, v1
            jump_label_2: lda v2
            jge v0, jump_label_0
            ldstatic Test.a
            jeqz jump_label_1
            lda v2
            return
            jump_label_1: lda v1
            addi 0x1
            sta v3
            lda v2
            addi 0x1
            sta v1
            mov v2, v1
            mov v1, v3
            jmp jump_label_2
            jump_label_0: lda v1
            return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "foo"));
    ASSERT_TRUE(CheckOrder(2, {4, 3})) << "Unexpected initial order";

    StartProfiling();
    UpdateBranchTaken(0xF);

    ASSERT_TRUE(CheckOrder(2, {3, 4})) << "Unexpected order";
}

TEST_F(LinearOrderTest, FrequencyThreshold)
{
    auto source = R"(
    .record Test <> {
            u1 f <static>
    }
    .function void foo() <static> {
            return.void
    }

    .function void bar() <static> {
            return.void
    }
    .function void main() <static> {
            movi v0, 0x64
            movi v1, 0x0
            jump_label_3: lda v1
            jge v0, jump_label_0
            ldstatic Test.f
            jeqz jump_label_1
            call.short foo
            jmp jump_label_2
            jump_label_1: call.short bar
            jump_label_2: inci v1, 0x1
            jmp jump_label_3
            jump_label_0: return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(CheckOrder(2, {4, 3, 5})) << "Unexpected initial order";

    StartProfiling();
    UpdateBranchNotTaken(0xD, 90);
    UpdateBranchTaken(0xD, 99);

    ASSERT_TRUE(CheckOrder(2, {4, 3, 5})) << "Unexpected order, threshold was not exceeded";

    UpdateBranchTaken(0xD);
    ASSERT_TRUE(CheckOrder(2, {3, 5, 4})) << "Unexpected order, threshold was exceeded";

    UpdateBranchNotTaken(0xD, 21);
    ASSERT_TRUE(CheckOrder(2, {3, 4, 5})) << "Unexpected order, another branch didn't exceed threshold";

    UpdateBranchNotTaken(0xD);
    ASSERT_TRUE(CheckOrder(2, {4, 5, 3})) << "Unexpected order, another branch exceeded threshold";
}

TEST_F(LinearOrderTest, LoopTransform)
{
    auto source = R"(
    .function i32 foo() <static> {
            movi v0, 0x64
            movi v1, 0x0
            mov v2, v1
            jump_label_3: lda v2
            jge v0, jump_label_0
            lda v2
            modi 0x3
            jnez jump_label_1
            lda v1
            addi 0x2
            sta v3
            mov v1, v3
            jmp jump_label_2
            jump_label_1: lda v1
            addi 0x3
            sta v3
            mov v1, v3
            jump_label_2: inci v2, 0x1
            jmp jump_label_3
            jump_label_0: lda v1
            return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "foo"));
    ASSERT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnroll>(100, 3));

    ASSERT_TRUE(CheckOrder(20, {4, 3, 5, 22, 24, 25, 23, 26, 28, 29, 27, 21, 1})) << "Unexpected initial order";

    StartProfiling();
    UpdateBranchTaken(0x10, 10);
    UpdateBranchNotTaken(0x10);

    ASSERT_TRUE(CheckOrder(20, {3, 5, 22, 25, 23, 26, 29, 27, 21, 28, 24, 4, 1}))
        << "Unexpected order, threshold was exceeded";

    UpdateBranchNotTaken(0x10, 20);

    ASSERT_TRUE(CheckOrder(20, {4, 5, 22, 24, 23, 26, 28, 27, 21, 29, 25, 3, 1}))
        << "Unexpected order, another branch threshold was exceeded";
}
}  // namespace panda::compiler
