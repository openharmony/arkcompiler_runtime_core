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
#include "optimizer/optimizations/regalloc/cleanup_empty_blocks.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "optimizer/optimizations/cleanup.h"

namespace panda::compiler {
class CleanupEmptyBlocksTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(CleanupEmptyBlocksTest, RemoveEmptyBlockAfterRegAlloc)
{
    if (GetGraph()->GetArch() == Arch::AARCH32 || !BackendSupport(GetGraph()->GetArch())) {
        return;
    }

    // Empty basic block 5 cannot be removed because it is part of a special triangle
    // After RegAlloc inserts BB 7, BB 5 can be removed
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i32();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 6, 3)
        {
            INST(3, Opcode::AndI).i32().Inputs(0).Imm(3);
            INST(4, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            CONSTANT(5, 100);
            INST(6, Opcode::Mod).i32().Inputs(0, 5);
            INST(7, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 6, 5)
        {
            CONSTANT(8, 400);
            INST(9, Opcode::Mod).i32().Inputs(0, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 6) {}
        BASIC_BLOCK(6, -1)
        {
            INST(11, Opcode::Phi).b().Inputs({{2, 1}, {4, 1}, {5, 2}});
            INST(12, Opcode::Return).b().Inputs(11);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(GetGraph()->RunPass<RegAllocGraphColoring>());
    ASSERT_TRUE(CleanupEmptyBlocks(GetGraph()));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i32();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 8, 3)
        {
            INST(3, Opcode::AndI).i32().Inputs(0).Imm(3);
            INST(4, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 6, 4)
        {
            CONSTANT(5, 100);
            INST(6, Opcode::Mod).i32().Inputs(0, 5);
            INST(7, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 7, 6)
        {
            CONSTANT(8, 400);
            INST(9, Opcode::Mod).i32().Inputs(0, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(7, 6)
        {
            INST(13, Opcode::SpillFill);
        }
        BASIC_BLOCK(8, 6)
        {
            INST(14, Opcode::SpillFill);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(11, Opcode::Phi).b().Inputs({{8, 1}, {7, 1}, {3, 2}, {4, 2}});
            INST(12, Opcode::Return).b().Inputs(11);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
