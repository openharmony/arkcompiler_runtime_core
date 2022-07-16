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
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/try_catch_resolving.h"
#include "optimizer/optimizations/cleanup.h"

namespace panda::compiler {
class TryCatchResolvingTest : public AsmTest {
public:
    TryCatchResolvingTest() : default_compiler_non_optimizing_(options.IsCompilerNonOptimizing())
    {
        options.SetCompilerNonOptimizing(false);
    }

    ~TryCatchResolvingTest() override
    {
        options.SetCompilerNonOptimizing(default_compiler_non_optimizing_);
    }

private:
    bool default_compiler_non_optimizing_;
};

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
        CONSTANT(1, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::Return).Inputs(1).b();
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
}  // namespace panda::compiler
