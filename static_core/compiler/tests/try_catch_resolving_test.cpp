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

#include "macros.h"
#include "unit_test.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/try_catch_resolving.h"
#include "optimizer/optimizations/cleanup.h"

namespace panda::compiler {
class TryCatchResolvingTest : public AsmTest {  // NOLINT(fuchsia-multiple-inheritance)
public:
    TryCatchResolvingTest() : default_compiler_non_optimizing_(OPTIONS.IsCompilerNonOptimizing())
    {
        OPTIONS.SetCompilerNonOptimizing(false);
    }

    ~TryCatchResolvingTest() override
    {
        OPTIONS.SetCompilerNonOptimizing(default_compiler_non_optimizing_);
    }

    NO_COPY_SEMANTIC(TryCatchResolvingTest);
    NO_MOVE_SEMANTIC(TryCatchResolvingTest);

private:
    bool default_compiler_non_optimizing_;
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(TryCatchResolvingTest, ThrowNewObject)
{
    auto source = R"(
.record E1 {}

.function u1 main() {
    newobj v0, E1
try_begin:
    throw v0
    ldai 2
try_end:    
    return


catch_block1_begin:
    ldai 0
    return

catch_block2_begin:
    ldai 10
    return

.catchall try_begin, try_end, catch_block1_begin
.catch E1, try_begin, try_end, catch_block2_begin
}
    )";

    auto graph = GetGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    graph->RunPass<TryCatchResolving>();
    graph->RunPass<Cleanup>();

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        BASIC_BLOCK(2, 3)
        {
            INST(5, Opcode::SaveState).Inputs().SrcVregs({});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::NewObject).ref().Inputs(6, 5);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(8, Opcode::SaveState).Inputs(7).SrcVregs({0});
            INST(9, Opcode::Throw).Inputs(7, 8);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

TEST_F(TryCatchResolvingTest, RemoveAllCatchHandlers)
{
    auto source = R"(
    .record E0 {}
    .record E1 {}
    .record E2 {}
    .record E3 {}

    .function u1 main() {
    try_begin:
        newobj v0, E0
        throw v0
        ldai 3
    try_end:
        return


    catch_block1:
        ldai 0
        return

    catch_block2:
        ldai 1
        return

    catch_block3:
        ldai 2
        return

    .catch E1, try_begin, try_end, catch_block1
    .catch E2, try_begin, try_end, catch_block2
    .catch E3, try_begin, try_end, catch_block3

    }
    )";

    auto graph = GetGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    graph->RunPass<TryCatchResolving>();
    graph->RunPass<Cleanup>();

    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::SaveState).Inputs().SrcVregs({});
            INST(8, Opcode::LoadAndInitClass).ref().Inputs(7);
            INST(9, Opcode::NewObject).ref().Inputs(8, 7);
            INST(10, Opcode::SaveState).Inputs(9).SrcVregs({0});
            INST(11, Opcode::Throw).Inputs(9, 10);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

TEST_F(TryCatchResolvingTest, EmptyTryCatches)
{
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        CONSTANT(5, 2).i32();
        CONSTANT(8, 10).i32();
        BASIC_BLOCK(7, 2) {}
        BASIC_BLOCK(2, 4, 9)
        {
            INST(1, Opcode::Try).CatchTypeIds({0});
        }
        BASIC_BLOCK(4, 3)
        {
            INST(6, Opcode::SaveState).Inputs().SrcVregs({});
            INST(7, Opcode::LoadType).ref().Inputs(6).TypeId(4646690);
        }
        BASIC_BLOCK(3, 5, 9) {}
        BASIC_BLOCK(9, 6)
        {
            INST(2, Opcode::CatchPhi).i32().ThrowableInsts({}).Inputs(5);
            INST(4, Opcode::CatchPhi).ref().ThrowableInsts({}).Inputs();
        }
        BASIC_BLOCK(5, 8) {}
        BASIC_BLOCK(6, 8) {}
        BASIC_BLOCK(8, -1)
        {
            INST(10, Opcode::Phi).i32().Inputs(8, 2);
            INST(13, Opcode::SaveState).Inputs().SrcVregs({});
            INST(14, Opcode::LoadAndInitClass).ref().Inputs(13);
            INST(15, Opcode::StoreStatic).s32().Inputs(14, 10);
            INST(16, Opcode::ReturnVoid);
        }
    }
    BB(2).SetTryBegin(true);
    BB(4).SetTry(true);
    BB(3).SetTryEnd(true);
    INS(1).CastToTry()->SetTryEndBlock(&BB(3));
    BB(9).SetCatchBegin(true);
    BB(9).SetCatch(true);
    BB(6).SetCatchEnd(true);
    BB(6).SetCatch(true);
    GraphChecker(graph).Check();

    graph->RunPass<TryCatchResolving>();
    auto expected_graph = CreateGraphWithDefaultRuntime();
    GRAPH(expected_graph)
    {
        CONSTANT(8, 10).i32();
        BASIC_BLOCK(8, -1)
        {
            INST(13, Opcode::SaveState).Inputs().SrcVregs({});
            INST(14, Opcode::LoadAndInitClass).ref().Inputs(13);
            INST(15, Opcode::StoreStatic).s32().Inputs(14, 8);
            INST(16, Opcode::ReturnVoid);
        }
    }
    GraphChecker(expected_graph).Check();

    ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
