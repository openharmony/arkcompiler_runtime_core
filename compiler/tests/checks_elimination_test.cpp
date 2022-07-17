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
#include "optimizer/optimizations/checks_elimination.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/ir/graph_cloner.h"

namespace panda::compiler {
class ChecksEliminationTest : public CommonTest {
public:
    ChecksEliminationTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

    struct RuntimeInterfaceNotStaticMethod : public compiler::RuntimeInterface {
        bool IsMethodStatic([[maybe_unused]] MethodPtr method) const override
        {
            return false;
        }
    };

    template <bool IsApplied>
    void SimpleTest(int32_t index, int32_t array_len)
    {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0, array_len);
            CONSTANT(1, index);
            BASIC_BLOCK(2, 1)
            {
                INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
                INST(44, Opcode::LoadAndInitClass).ref().Inputs(2).TypeId(68);
                INST(3, Opcode::NewArray).ref().Inputs(44, 0, 2);
                INST(4, Opcode::BoundsCheck).s32().Inputs(0, 1, 2);
                INST(5, Opcode::LoadArray).s32().Inputs(3, 4);
                INST(6, Opcode::Return).s32().Inputs(5);
            }
        }
        graph1->RunPass<ChecksElimination>();
        auto graph2 = CreateEmptyGraph();
        if constexpr (IsApplied) {
            GRAPH(graph2)
            {
                CONSTANT(0, array_len);
                CONSTANT(1, index);
                BASIC_BLOCK(2, 1)
                {
                    INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
                    INST(44, Opcode::LoadAndInitClass).ref().Inputs(2).TypeId(68);
                    INST(3, Opcode::NewArray).ref().Inputs(44, 0, 2);
                    INST(4, Opcode::NOP);
                    INST(5, Opcode::LoadArray).s32().Inputs(3, 1);
                    INST(6, Opcode::Return).s32().Inputs(5);
                }
            }
        } else {
            GRAPH(graph2)
            {
                CONSTANT(0, array_len);
                CONSTANT(1, index);
                BASIC_BLOCK(2, 1)
                {
                    INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
                    INST(44, Opcode::LoadAndInitClass).ref().Inputs(2).TypeId(68);
                    INST(3, Opcode::NewArray).ref().Inputs(44, 0, 2);
                    INST(4, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(2);
                }
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    enum AppliedType { NOT_APPLIED, REMOVED, REPLACED };

    template <AppliedType appliedType>
    void ArithmeticTest(int32_t index, int32_t array_len, Opcode opc, int32_t val)
    {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0, array_len);
            CONSTANT(1, index);
            CONSTANT(2, val);
            BASIC_BLOCK(2, 1)
            {
                INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
                INST(44, Opcode::LoadAndInitClass).ref().Inputs(3).TypeId(68);
                INST(4, Opcode::NewArray).ref().Inputs(44, 0, 3);
                INST(5, opc).s32().Inputs(1, 2);
                INST(6, Opcode::BoundsCheck).s32().Inputs(0, 5, 3);
                INST(7, Opcode::LoadArray).s32().Inputs(4, 6);
                INST(8, Opcode::Return).s32().Inputs(7);
            }
        }
        auto clone = GraphCloner(graph1, graph1->GetAllocator(), graph1->GetLocalAllocator()).CloneGraph();
        bool result = graph1->RunPass<ChecksElimination>();
        auto graph2 = CreateEmptyGraph();
        if constexpr (appliedType == AppliedType::REMOVED) {
            GRAPH(graph2)
            {
                CONSTANT(0, array_len);
                CONSTANT(1, index);
                CONSTANT(2, val);
                BASIC_BLOCK(2, 1)
                {
                    INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
                    INST(44, Opcode::LoadAndInitClass).ref().Inputs(3).TypeId(68);
                    INST(4, Opcode::NewArray).ref().Inputs(44, 0, 3);
                    INST(5, opc).s32().Inputs(1, 2);
                    INST(6, Opcode::NOP);
                    INST(7, Opcode::LoadArray).s32().Inputs(4, 5);
                    INST(8, Opcode::Return).s32().Inputs(7);
                }
            }
            ASSERT_TRUE(result);
        } else if constexpr (appliedType == AppliedType::REPLACED) {
            GRAPH(graph2)
            {
                CONSTANT(0, array_len);
                CONSTANT(1, index);
                CONSTANT(2, val);
                BASIC_BLOCK(2, 1)
                {
                    INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
                    INST(44, Opcode::LoadAndInitClass).ref().Inputs(3).TypeId(68);
                    INST(4, Opcode::NewArray).ref().Inputs(44, 0, 3);
                    INST(5, opc).s32().Inputs(1, 2);
                    INST(6, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(3);
                }
            }
            ASSERT_TRUE(result);
        } else {
            ASSERT_FALSE(result);
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, (appliedType == AppliedType::NOT_APPLIED) ? clone : graph2));
    }

    template <bool IsApplied>
    void ModTest(int32_t array_len, int32_t mod)
    {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0, array_len);
            CONSTANT(1, mod);
            PARAMETER(2, 0).s32();
            BASIC_BLOCK(2, 1)
            {
                INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
                INST(44, Opcode::LoadAndInitClass).ref().Inputs(3).TypeId(68);
                INST(4, Opcode::NewArray).ref().Inputs(44, 0, 3);
                INST(5, Opcode::Mod).s32().Inputs(2, 1);
                INST(6, Opcode::BoundsCheck).s32().Inputs(0, 5, 3);
                INST(7, Opcode::LoadArray).s32().Inputs(4, 6);
                INST(8, Opcode::Return).s32().Inputs(7);
            }
        }
        graph1->RunPass<ChecksElimination>();
        auto graph2 = CreateEmptyGraph();
        if constexpr (IsApplied) {
            GRAPH(graph2)
            {
                CONSTANT(0, array_len);
                CONSTANT(1, mod);
                PARAMETER(2, 0).s32();
                BASIC_BLOCK(2, 1)
                {
                    INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
                    INST(44, Opcode::LoadAndInitClass).ref().Inputs(3).TypeId(68);
                    INST(4, Opcode::NewArray).ref().Inputs(44, 0, 3);
                    INST(5, Opcode::Mod).s32().Inputs(2, 1);
                    INST(6, Opcode::NOP);
                    INST(7, Opcode::LoadArray).s32().Inputs(4, 5);
                    INST(8, Opcode::Return).s32().Inputs(7);
                }
            }
        } else {
            GRAPH(graph2)
            {
                CONSTANT(0, array_len);
                CONSTANT(1, mod);
                PARAMETER(2, 0).s32();
                BASIC_BLOCK(2, 1)
                {
                    INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
                    INST(44, Opcode::LoadAndInitClass).ref().Inputs(3).TypeId(68);
                    INST(4, Opcode::NewArray).ref().Inputs(44, 0, 3);
                    INST(5, Opcode::Mod).s32().Inputs(2, 1);
                    INST(6, Opcode::BoundsCheck).s32().Inputs(0, 5, 3);
                    INST(7, Opcode::LoadArray).s32().Inputs(4, 6);
                    INST(8, Opcode::Return).s32().Inputs(7);
                }
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    template <bool IsApplied>
    void PhiTest(int32_t index, int32_t len_array, int32_t mod)
    {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0, 0).b();
            PARAMETER(1, 1).s32();
            CONSTANT(2, len_array);  // len array
            CONSTANT(3, index);      // index 2
            CONSTANT(12, mod);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
                INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
                INST(4, Opcode::NewArray).ref().Inputs(44, 2, 43);
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
            }
            BASIC_BLOCK(3, 4)
            {
                INST(6, Opcode::Mod).s32().Inputs(1, 12);
            }
            BASIC_BLOCK(4, 1)
            {
                INST(7, Opcode::Phi).s32().Inputs(3, 6);
                INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3, 4).SrcVregs({0, 1, 2, 3, 4});
                INST(9, Opcode::BoundsCheck).s32().Inputs(2, 7, 8);
                INST(10, Opcode::LoadArray).s32().Inputs(4, 9);
                INST(11, Opcode::Return).s32().Inputs(10);
            }
        }
        graph1->RunPass<ChecksElimination>();
        auto graph2 = CreateEmptyGraph();
        if constexpr (IsApplied) {
            GRAPH(graph2)
            {
                PARAMETER(0, 0).b();
                PARAMETER(1, 1).s32();   // index 1
                CONSTANT(2, len_array);  // len array
                CONSTANT(3, index);      // index 2
                CONSTANT(12, mod);
                BASIC_BLOCK(2, 3, 4)
                {
                    INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
                    INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
                    INST(4, Opcode::NewArray).ref().Inputs(44, 2, 43);
                    INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
                }
                BASIC_BLOCK(3, 4)
                {
                    INST(6, Opcode::Mod).s32().Inputs(1, 12);
                }
                BASIC_BLOCK(4, 1)
                {
                    INST(7, Opcode::Phi).s32().Inputs(3, 6);
                    INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3, 4).SrcVregs({0, 1, 2, 3, 4});
                    INST(9, Opcode::NOP);
                    INST(10, Opcode::LoadArray).s32().Inputs(4, 7);
                    INST(11, Opcode::Return).s32().Inputs(10);
                }
            }
        } else {
            GRAPH(graph2)
            {
                PARAMETER(0, 0).b();
                PARAMETER(1, 1).s32();
                CONSTANT(2, len_array);  // len array
                CONSTANT(3, index);      // index 2
                CONSTANT(12, mod);
                BASIC_BLOCK(2, 3, 4)
                {
                    INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
                    INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
                    INST(4, Opcode::NewArray).ref().Inputs(44, 2, 43);
                    INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
                }
                BASIC_BLOCK(3, 4)
                {
                    INST(6, Opcode::Mod).s32().Inputs(1, 12);
                }
                BASIC_BLOCK(4, 1)
                {
                    INST(7, Opcode::Phi).s32().Inputs(3, 6);
                    INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3, 4).SrcVregs({0, 1, 2, 3, 4});
                    INST(9, Opcode::BoundsCheck).s32().Inputs(2, 7, 8);
                    INST(10, Opcode::LoadArray).s32().Inputs(4, 9);
                    INST(11, Opcode::Return).s32().Inputs(10);
                }
            }
        }
    }

protected:
    Graph *graph_ {nullptr};
};

TEST_F(ChecksEliminationTest, NullCheckTest)
{
    // Check Elimination for NullCheck is applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LoadArray).s32().Inputs(3, 1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::NullCheck).ref().Inputs(0, 5);
            INST(7, Opcode::StoreArray).s32().Inputs(6, 1, 4);
            INST(8, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LoadArray).s32().Inputs(3, 1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::NOP);
            INST(7, Opcode::StoreArray).s32().Inputs(3, 1, 4);
            INST(8, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NullCheckTest1)
{
    // Check Elimination for NullCheck isn't applied. Different inputs in NullCheck insts.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(2, 10);

        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LoadArray).s32().Inputs(4, 2);
            INST(6, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(7, Opcode::NullCheck).ref().Inputs(1, 6);
            INST(8, Opcode::StoreArray).s32().Inputs(7, 2, 5);
            INST(9, Opcode::Return).s32().Inputs(5);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, NullCheckTest2)
{
    // Check Elimination for NullCheck isn't applied. NullCheck(5) isn't dominator of NullCheck(8).
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(1, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LoadArray).s32().Inputs(5, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::NullCheck).ref().Inputs(0, 7);
            INST(9, Opcode::LoadArray).s32().Inputs(8, 1);
        }

        BASIC_BLOCK(5, 1)
        {
            INST(10, Opcode::Phi).s32().Inputs(6, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, NullCheckTest3)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);  // initial
        CONSTANT(1, 1);  // increment
        CONSTANT(2, 10);
        PARAMETER(3, 0).ref();
        BASIC_BLOCK(2, 3, 5)
        {
            INST(16, Opcode::LenArray).s32().Inputs(3);
            INST(20, Opcode::SaveStateDeoptimize).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);  // 0 < 10
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(15, Opcode::NullCheck).ref().Inputs(3, 7);
            INST(8, Opcode::BoundsCheck).s32().Inputs(16, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(15, 8, 0);     // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);               // i++
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 2);  // i < 10
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);  // initial
        CONSTANT(1, 1);  // increment
        CONSTANT(2, 10);
        PARAMETER(3, 0).ref();
        CONSTANT(17, nullptr);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(16, Opcode::LenArray).s32().Inputs(3);
            INST(20, Opcode::SaveStateDeoptimize).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            // deoptimize if param == nullptr
            INST(18, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(3, 17);
            INST(19, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(18, 20);
            INST(23, Opcode::Compare).SrcType(DataType::INT32).CC(CC_GT).b().Inputs(2, 16);  // 10 > len_array
            INST(24, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(23, 20);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);  // 0 < 10
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(15, Opcode::NOP);
            INST(8, Opcode::NOP);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 4, 0);      // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);               // i++
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 2);  // i < 10
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NullCheckTest4)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);  // initial
        CONSTANT(1, 1);  // increment
        CONSTANT(2, 10);
        CONSTANT(3, nullptr);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(20, Opcode::SaveStateDeoptimize).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);  // 0 < 10
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(15, Opcode::NullCheck).ref().Inputs(3, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(15, 4, 0);     // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);               // i++
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 2);  // i < 10
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);  // initial
        CONSTANT(1, 1);  // increment
        CONSTANT(2, 10);
        CONSTANT(3, nullptr);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(20, Opcode::SaveStateDeoptimize).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);  // 0 < 10
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 1)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(15, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(7);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTest)
{
    // Check Elimination for NegativeCheck is applied.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NegativeCheck).s32().Inputs(0, 1);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 0, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NOP);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 0, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTestZero)
{
    // Check Elimination for NegativeCheck is applied.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NegativeCheck).s32().Inputs(0, 1);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NOP);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 0, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTest1)
{
    // Check Elimination for NegativeCheck is applied. Negative constant.
    // Check replaced by deoptimize
    GRAPH(GetGraph())
    {
        CONSTANT(0, -5);
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NegativeCheck).s32().Inputs(0, 1);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, -5);
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NEGATIVE_CHECK).Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTest3)
{
    // Check Elimination for NegativeCheck is applied. Positiv value.
    GRAPH(GetGraph())
    {
        CONSTANT(0, -5);
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::Neg).s32().Inputs(0);
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NegativeCheck).s32().Inputs(5, 1);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, -5);
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::Neg).s32().Inputs(0);
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NOP);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 5, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTest4)
{
    // Check Elimination for NegativeCheck is applied. Negative value.
    // Check replaced by deoptimize
    GRAPH(GetGraph())
    {
        CONSTANT(0, 5);
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::Neg).s32().Inputs(0);
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NegativeCheck).s32().Inputs(5, 1);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 5);
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::Neg).s32().Inputs(0);
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NEGATIVE_CHECK).Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, ZeroCheckTest)
{
    // Check Elimination for ZeroCheck is applied.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        CONSTANT(1, 12);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Div).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 10);
        CONSTANT(1, 12);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::NOP);
            INST(4, Opcode::Div).s32().Inputs(0, 1);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, ZeroCheckTestBigConst)
{
    // Check Elimination for ZeroCheck is applied.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        CONSTANT(1, 0x8000000000000000);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Div).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 10);
        CONSTANT(1, 0x8000000000000000);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::NOP);
            INST(4, Opcode::Div).s32().Inputs(0, 1);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, ZeroCheckTest1)
{
    // Check Elimination for ZeroCheck isn't applied. Zero constant.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        CONSTANT(1, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::ZeroCheck).s32().Inputs(1, 2);
            INST(4, Opcode::Div).s32().Inputs(0, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 10);
        CONSTANT(1, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::ZERO_CHECK).Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, ZeroCheckTest2)
{
    // Check Elimination for ZeroCheck is applied. Positiv value.
    GRAPH(GetGraph())
    {
        CONSTANT(0, -5);
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::Abs).s32().Inputs(0);
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NegativeCheck).s32().Inputs(5, 1);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, -5);
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::Abs).s32().Inputs(0);
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NOP);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 5, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeZeroCheckTest)
{
    // Test with NegativeCheck and ZeroCheck.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        CONSTANT(1, 2);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NegativeCheck).s32().Inputs(0, 2);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(2).TypeId(68);
            INST(4, Opcode::NewArray).ref().Inputs(44, 3, 2);
            INST(5, Opcode::ZeroCheck).s32().Inputs(0, 2);
            INST(6, Opcode::Div).s32().Inputs(1, 5);
            INST(7, Opcode::StoreArray).s32().Inputs(4, 0, 6);
            INST(8, Opcode::Return).ref().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1);
        CONSTANT(1, 2);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NOP);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(2).TypeId(68);
            INST(4, Opcode::NewArray).ref().Inputs(44, 0, 2);
            INST(5, Opcode::NOP);
            INST(6, Opcode::Div).s32().Inputs(1, 0);
            INST(7, Opcode::StoreArray).s32().Inputs(4, 0, 6);
            INST(8, Opcode::Return).ref().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, SimpleBoundsCheckTest)
{
    SimpleTest<true>(2, 10);
    SimpleTest<true>(9, 10);
    SimpleTest<true>(0, 10);
    SimpleTest<false>(10, 10);
    SimpleTest<false>(12, 10);
    SimpleTest<false>(-1, 10);
}

TEST_F(ChecksEliminationTest, SimpleBoundsCheckTest1)
{
    // not applied, we know nothing about len array
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // len array
        CONSTANT(1, 5);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(2).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 0, 2);
            INST(4, Opcode::BoundsCheck).s32().Inputs(0, 1, 2);
            INST(5, Opcode::LoadArray).s32().Inputs(3, 4);
            INST(6, Opcode::Return).s32().Inputs(5);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, SimpleBoundsCheckTest2)
{
    // not applied, we know nothing about index
    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        PARAMETER(1, 0).s32();  // index
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(2).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 0, 2);
            INST(4, Opcode::BoundsCheck).s32().Inputs(0, 1, 2);
            INST(5, Opcode::LoadArray).s32().Inputs(3, 4);
            INST(6, Opcode::Return).s32().Inputs(5);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, ArithmeticTest)
{
    ArithmeticTest<AppliedType::REMOVED>(-1, 20, Opcode::Add, 1);
    ArithmeticTest<AppliedType::REMOVED>(-1, 20, Opcode::Add, 5);
    ArithmeticTest<AppliedType::REMOVED>(-1, 20, Opcode::Add, 20);
    ArithmeticTest<AppliedType::REMOVED>(-100, 20, Opcode::Add, 115);
    ArithmeticTest<AppliedType::REPLACED>(-1, 20, Opcode::Add, 0);
    ArithmeticTest<AppliedType::REPLACED>(0, 20, Opcode::Add, 20);
    ArithmeticTest<AppliedType::REPLACED>(-100, 20, Opcode::Add, 5);
    ArithmeticTest<AppliedType::REPLACED>(-100, 20, Opcode::Add, 125);
    // overflow
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MAX, INT32_MAX, Opcode::Add, 1);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MIN, INT32_MAX, Opcode::Add, -1);
    ArithmeticTest<AppliedType::NOT_APPLIED>(1, INT32_MAX, Opcode::Add, INT32_MAX);
    ArithmeticTest<AppliedType::NOT_APPLIED>(-1, INT32_MAX, Opcode::Add, INT32_MIN);

    ArithmeticTest<AppliedType::REMOVED>(20, 20, Opcode::Sub, 1);
    ArithmeticTest<AppliedType::REMOVED>(20, 20, Opcode::Sub, 5);
    ArithmeticTest<AppliedType::REMOVED>(20, 20, Opcode::Sub, 19);
    ArithmeticTest<AppliedType::REPLACED>(25, 20, Opcode::Sub, 3);
    ArithmeticTest<AppliedType::REPLACED>(20, 20, Opcode::Sub, 60);
    // overflow
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MIN, INT32_MAX, Opcode::Sub, 1);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MAX, INT32_MAX, Opcode::Sub, -1);
    ArithmeticTest<AppliedType::NOT_APPLIED>(1, INT32_MAX, Opcode::Sub, INT32_MIN);
    ArithmeticTest<AppliedType::REPLACED>(-1, INT32_MAX, Opcode::Sub, INT32_MAX);

    ArithmeticTest<AppliedType::REMOVED>(1, 20, Opcode::Mod, 20);
    ArithmeticTest<AppliedType::REMOVED>(101, 20, Opcode::Mod, 5);
    ArithmeticTest<AppliedType::REMOVED>(205, 20, Opcode::Mod, 5);
    ArithmeticTest<AppliedType::REMOVED>(16, 5, Opcode::Mod, 3);

    ArithmeticTest<AppliedType::REMOVED>(5, 20, Opcode::Mul, 2);
    ArithmeticTest<AppliedType::REMOVED>(-5, 20, Opcode::Mul, -2);
    ArithmeticTest<AppliedType::REMOVED>(3, 20, Opcode::Mul, 5);
    ArithmeticTest<AppliedType::REPLACED>(5, 20, Opcode::Mul, -2);
    ArithmeticTest<AppliedType::REPLACED>(-2, 20, Opcode::Mul, 5);
    // Zero
    ArithmeticTest<AppliedType::REMOVED>(INT32_MAX, 20, Opcode::Mul, 0);
    ArithmeticTest<AppliedType::REMOVED>(0, 20, Opcode::Mul, INT32_MAX);
    ArithmeticTest<AppliedType::REMOVED>(INT32_MIN, 20, Opcode::Mul, 0);
    ArithmeticTest<AppliedType::REMOVED>(0, 20, Opcode::Mul, INT32_MIN);
    // overflow
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MAX, 20, Opcode::Mul, 2);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MIN, 20, Opcode::Mul, -2);
    ArithmeticTest<AppliedType::NOT_APPLIED>(-2, 20, Opcode::Mul, INT32_MIN);
    ArithmeticTest<AppliedType::NOT_APPLIED>(2, 20, Opcode::Mul, INT32_MAX);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MAX, 20, Opcode::Mul, -2);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MIN, 20, Opcode::Mul, 2);
    ArithmeticTest<AppliedType::NOT_APPLIED>(2, 20, Opcode::Mul, INT32_MIN);
    ArithmeticTest<AppliedType::NOT_APPLIED>(-2, 20, Opcode::Mul, INT32_MAX);
}

TEST_F(ChecksEliminationTest, ModTest)
{
    ModTest<true>(10, 10);
    ModTest<true>(10, 5);
    ModTest<false>(10, 11);
    ModTest<false>(10, 20);
}

TEST_F(ChecksEliminationTest, IfTestTrueBlock)
{
    // we can norrow bounds range for true block
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(2, 10);  // len array
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(0, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(2, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        CONSTANT(2, 10);
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(0, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::NOP);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 0);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTestFalseBlock)
{
    // we can norrow bounds range for false block
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(2, 10);  // len array
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(4, Opcode::Compare).b().CC(CC_GE).Inputs(0, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(2, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    GetGraph()->RunPass<ChecksElimination>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        CONSTANT(2, 10);
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(4, Opcode::Compare).b().CC(CC_GE).Inputs(0, 2);  // index >= len array
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);  // index < 0
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::NOP);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 0);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTestTrueBlock1)
{
    // we can norrow bounds range for true block
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(6, 3, 5)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(0, 14);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(14, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(6, 3, 5)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(0, 14);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::NOP);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 0);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTestTrueBlock2)
{
    // we can norrow bounds range for true block
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_GE).Inputs(0, 3);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(0, 14);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(14, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(6, 3, 5)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_GE).Inputs(0, 3);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(0, 14);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::NOP);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 0);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTestTrueBlock3)
{
    // we can norrow bounds range for true block
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(6, 3, 5)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_GT).Inputs(14, 0);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(14, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(6, 3, 5)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_GT).Inputs(14, 0);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::NOP);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 0);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTestFalseBlock1)
{
    // we can norrow bounds range for false block
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_GE).Inputs(0, 14);  // index >= len array
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);  // index < 0
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(14, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_GE).Inputs(0, 14);  // index >= len array
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);  // index < 0
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::NOP);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 0);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTestFalseBlock2)
{
    // we can norrow bounds range for false block
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);  // index < 0
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(0, 14);  // index >= len array
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(14, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);  // index < 0
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(0, 14);  // index >= len array
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::NOP);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 0);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTestFalseBlock3)
{
    // we can norrow bounds range for false block
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_LE).Inputs(14, 0);  // len array < index
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);  // index < 0
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(14, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(14, Opcode::LenArray).s32().Inputs(1);
            INST(4, Opcode::Compare).b().CC(CC_LE).Inputs(14, 0);  // len array < index
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);  // index < 0
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(9, Opcode::NOP);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 0);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTest1)
{
    // not applied if without compare
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        PARAMETER(13, 2).b();
        CONSTANT(2, 10);  // len array
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(2, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, IfTest2)
{
    // not applied, compare intputs not int32
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        PARAMETER(13, 2).s64();
        PARAMETER(14, 3).s64();
        CONSTANT(2, 10);  // len array
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(15, Opcode::Compare).b().CC(CC_GE).Inputs(13, 14);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(15);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(16, Opcode::Compare).b().CC(CC_GE).Inputs(13, 14);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(16);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::BoundsCheck).s32().Inputs(2, 0, 8);
            INST(10, Opcode::LoadArray).s32().Inputs(1, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(3);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, PhiTest)
{
    PhiTest<true>(5, 10, 10);
    PhiTest<true>(9, 10, 5);
    PhiTest<false>(10, 10, 10);
    PhiTest<false>(-1, 10, 10);
    PhiTest<false>(5, 10, 11);
}

TEST_F(ChecksEliminationTest, SimpleLoopTestInc)
{
    // new_array(len_array)
    // for(int i = 0, i < len_array, i++) begin
    //  boundscheck(len_array, i) - can remove
    //  a[i] = 0
    // end
    // return new_array
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);   // initial
        CONSTANT(1, 1);   // increment
        CONSTANT(2, 10);  // len_array
        BASIC_BLOCK(2, 3)
        {
            INST(43, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 43);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);  // i < len_array
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 8, 0);  // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);           // i++
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);   // initial
        CONSTANT(1, 1);   // increment
        CONSTANT(2, 10);  // len_array
        BASIC_BLOCK(2, 3)
        {
            INST(43, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 43);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);  // i < len_array
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::NOP);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 4, 0);  // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);           // i++
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, SimpleLoopTestIncAfterPeeling)
{
    // new_array(len_array)
    // for(int i = 0, i < len_array, i++) begin
    //  boundscheck(len_array, i) - can remove
    //  a[i] = 0
    // end
    // return new_array
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);   // initial
        CONSTANT(1, 1);   // increment
        CONSTANT(2, 10);  // len_array
        BASIC_BLOCK(2, 3, 5)
        {
            INST(43, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 43);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);  // 0 < len_array
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 8, 0);      // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);               // i++
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 2);  // i < len_array
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);   // initial
        CONSTANT(1, 1);   // increment
        CONSTANT(2, 10);  // len_array
        BASIC_BLOCK(2, 3, 5)
        {
            INST(43, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 43);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);  // 0 < len_array
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::NOP);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 4, 0);      // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);               // i++
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 2);  // i < len_array
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, SimpleLoopTestIncAfterPeeling1)
{
    // new_array(len_array)
    // for(int i = 0, i < len_array, i++) begin
    //  boundscheck(len_array, i) - can remove
    //  a[i] = 0
    // end
    // return new_array
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(3, 0).ref();  // array
        BASIC_BLOCK(2, 3, 5)
        {
            INST(16, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 16);  // 0 < len_array
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(16, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 8, 0);       // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);                // i++
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 16);  // i < len_array
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(3, 0).ref();  // array
        BASIC_BLOCK(2, 3, 5)
        {
            INST(16, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 16);  // 0 < len_array
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 3).SrcVregs({0, 1, 3});
            INST(8, Opcode::NOP);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 4, 0);       // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);                // i++
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 16);  // i < len_array
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, SimpleLoopTestDec)
{
    // new_array(len_array)
    // for(int i = len_array-1, i >= 0, i--) begin
    //  boundscheck(len_array, i) - can remove
    //  a[i] = 0
    // end
    // return new_array
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);   // increment
        CONSTANT(2, 10);  // initial and len_array
        BASIC_BLOCK(2, 3)
        {
            INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 43);
            INST(13, Opcode::Sub).s32().Inputs(2, 1);  // len_array - 1
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(13, 10);
            INST(5, Opcode::Compare).CC(CC_GE).b().Inputs(4, 0);  // i >= 0
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::BoundsCheck).s32().Inputs(2, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 8, 0);  // a[i] = 0
            INST(10, Opcode::Sub).s32().Inputs(4, 1);           // i--
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);   // increment
        CONSTANT(2, 10);  // initial and len_array
        BASIC_BLOCK(2, 3)
        {
            INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 43);
            INST(13, Opcode::Sub).s32().Inputs(2, 1);  // len_array - 1
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(13, 10);
            INST(5, Opcode::Compare).CC(CC_GE).b().Inputs(4, 0);  // i >= 0
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::NOP);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 4, 0);  // a[i] = 0
            INST(10, Opcode::Sub).s32().Inputs(4, 1);           // i--
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, LoopWithUnknowLowerUpperValue)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);         // increment
        PARAMETER(1, 0).s32();  // lower
        PARAMETER(2, 1).s32();  // upper
        PARAMETER(3, 2).ref();  // array
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(7, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1, 2);  // lower < upper
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(1, 13);
            INST(10, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(11, Opcode::BoundsCheck).s32().Inputs(6, 9, 10);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 11, 0);                   // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(9, 0);                              // i++
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13, 2);  // i < upper
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 1);         // increment
        PARAMETER(1, 0).s32();  // lower
        PARAMETER(2, 1).s32();  // upper
        PARAMETER(3, 2).ref();  // array
        CONSTANT(22, 0);
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(20, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1, 22);  // (lower) < 0
            INST(21, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(20, 30);
            INST(18, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(2, 6);  // (upper) > len_array
            INST(19, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(18, 30);
            INST(7, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1, 2);  // lower < X
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(1, 13);
            INST(10, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(11, Opcode::NOP);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 9, 0);                    // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(9, 0);                              // i++
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13, 2);  // i < upper
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, LoopWithUnknowLowerUpperValueDown)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);         // increment
        PARAMETER(1, 0).s32();  // lower
        PARAMETER(2, 1).s32();  // upper
        PARAMETER(3, 2).ref();  // array
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(7, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(2, 1);  // upper > lower
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(2, 13);
            INST(10, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(11, Opcode::BoundsCheck).s32().Inputs(6, 9, 10);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 11, 0);                   // a[i] = 0
            INST(13, Opcode::Sub).s32().Inputs(9, 0);                              // i--
            INST(14, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(13, 1);  // i > lower
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 1);         // increment
        PARAMETER(1, 0).s32();  // lower
        PARAMETER(2, 1).s32();  // upper
        PARAMETER(3, 2).ref();  // array
        CONSTANT(22, 0);
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(20, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1, 22);  // (lower) < 0
            INST(21, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(20, 30);
            INST(18, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(2, 6);  // (upper) > len_array
            INST(19, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(18, 30);
            INST(7, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(2, 1);  // upper > lower
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(2, 13);
            INST(10, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(11, Opcode::NOP);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 9, 0);                    // a[i] = 0
            INST(13, Opcode::Sub).s32().Inputs(9, 0);                              // i--
            INST(14, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(13, 1);  // i > lower
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, UpperOOB)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        CONSTANT(3, 0);
        CONSTANT(25, 1);
        BASIC_BLOCK(4, 3, 2)
        {
            INST(2, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(27, Opcode::SaveState).Inputs(3, 0).SrcVregs({0, 3});
            INST(28, Opcode::NullCheck).ref().Inputs(0, 27);
            INST(29, Opcode::LenArray).s32().Inputs(28);
            INST(30, Opcode::Compare).Inputs(29, 3).CC(CC_LT).b();
            INST(31, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(30);
        }
        BASIC_BLOCK(2, 3, 2)
        {
            INST(4, Opcode::Phi).s32().Inputs(3, 24);
            INST(15, Opcode::SaveState).Inputs(4, 0, 4).SrcVregs({0, 3, 4});
            INST(16, Opcode::NullCheck).ref().Inputs(0, 15);
            INST(18, Opcode::BoundsCheck).s32().Inputs(29, 4, 15);
            INST(19, Opcode::StoreArray).s32().Inputs(16, 18, 3);
            INST(24, Opcode::Add).s32().Inputs(4, 25);
            INST(10, Opcode::Compare).b().CC(CC_LT).Inputs(29, 24);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(26, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).ref();  // array
        CONSTANT(3, 0);
        CONSTANT(25, 1);
        BASIC_BLOCK(4, 3, 2)
        {
            INST(2, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(27, Opcode::SaveState).Inputs(3, 0).SrcVregs({0, 3});
            INST(28, Opcode::NullCheck).ref().Inputs(0, 27);
            INST(29, Opcode::LenArray).s32().Inputs(28);
            INST(33, Opcode::Compare).Inputs(29, 29).CC(CC_GE).b();
            INST(34, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(33, 27);
            INST(30, Opcode::Compare).Inputs(29, 3).CC(CC_LT).b();
            INST(31, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(30);
        }
        BASIC_BLOCK(2, 3, 2)
        {
            INST(4, Opcode::Phi).s32().Inputs(3, 24);
            INST(15, Opcode::SaveState).Inputs(4, 0, 4).SrcVregs({0, 3, 4});
            INST(16, Opcode::NOP);
            INST(18, Opcode::NOP);
            INST(19, Opcode::StoreArray).s32().Inputs(28, 4, 3);
            INST(24, Opcode::Add).s32().Inputs(4, 25);
            INST(10, Opcode::Compare).b().CC(CC_LT).Inputs(29, 24);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, 1)
        {
            INST(26, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, LoopWithUnknowLowerValue)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);         // increment
        PARAMETER(1, 0).s32();  // lower
        PARAMETER(3, 2).ref();  // array
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(7, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1, 6);  // lower < len_array
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(1, 13);
            INST(10, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(11, Opcode::BoundsCheck).s32().Inputs(6, 9, 10);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 11, 0);                   // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(9, 0);                              // i++
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13, 6);  // i < len_array
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 1);         // increment
        PARAMETER(1, 0).s32();  // lower
        PARAMETER(3, 2).ref();  // array
        CONSTANT(22, 0);
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(20, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1, 22);  // lower < 0
            INST(21, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(20, 30);
            INST(7, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1, 6);  // lower < len_aray
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(1, 13);
            INST(10, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(11, Opcode::NOP);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 9, 0);                    // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(9, 0);                              // i++
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13, 6);  // i < upper
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, LoopWithUnknowUpperValueLE)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(2, 0).s32();  // X
        PARAMETER(3, 1).ref();  // array
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(7, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(0, 2);  // i <= X
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(0, 13);
            INST(10, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(11, Opcode::BoundsCheck).s32().Inputs(6, 9, 10);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 11, 0);                   // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(9, 1);                              // i++
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(13, 2);  // i <= X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());

    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(2, 0).s32();  // X
        PARAMETER(3, 1).ref();  // array
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(18, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(2, 6);
            INST(19, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(18, 30);
            INST(7, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(0, 2);  // i <= X
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(0, 13);
            INST(10, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(11, Opcode::NOP);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 9, 0);                    // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(9, 1);                              // i++
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(13, 2);  // i <= X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, LoopWithUnknowUpperValueLT)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(2, 0).s32();  // X
        PARAMETER(3, 1).ref();  // array
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(7, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 2);  // i < X
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(0, 13);
            INST(10, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(11, Opcode::BoundsCheck).s32().Inputs(6, 9, 10);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 11, 0);                   // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(9, 1);                              // i++
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13, 2);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());

    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(2, 0).s32();  // X
        PARAMETER(3, 1).ref();  // array
        BASIC_BLOCK(7, 3, 6)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(18, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(2, 6);
            INST(19, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(18, 30);
            INST(7, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 2);  // i < X
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(9, Opcode::Phi).s32().Inputs(0, 13);
            INST(10, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(11, Opcode::NOP);
            INST(12, Opcode::StoreArray).s32().Inputs(3, 9, 0);                    // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(9, 1);                              // i++
            INST(14, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13, 2);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(16, Opcode::Phi).s32().Inputs(0, 13);
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, LoopSeveralBoundsChecks)
{
    // applied
    // array coping
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(2, 0).ref();  // array1
        PARAMETER(3, 1).ref();  // array2
        PARAMETER(4, 2).s32();  // X

        BASIC_BLOCK(7, 3, 6)
        {
            // check array 1
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(6, Opcode::NullCheck).ref().Inputs(2, 5);
            INST(7, Opcode::LenArray).s32().Inputs(6);
            // check array 2
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::NullCheck).ref().Inputs(3, 8);
            INST(10, Opcode::LenArray).s32().Inputs(9);

            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});

            INST(11, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 4);  // 0 < X
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(13, Opcode::Phi).s32().Inputs(0, 20);  // i
            INST(14, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(15, Opcode::BoundsCheck).s32().Inputs(7, 13, 14);
            INST(16, Opcode::LoadArray).s32().Inputs(2, 15);
            INST(17, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(18, Opcode::BoundsCheck).s32().Inputs(10, 13, 17);
            INST(19, Opcode::StoreArray).s32().Inputs(3, 18, 16);                  // array2[i] = array1[i]
            INST(20, Opcode::Add).s32().Inputs(13, 1);                             // i++
            INST(21, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(20, 4);  // i < X
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(21);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(23, Opcode::Phi).s32().Inputs(0, 20);
            INST(24, Opcode::Return).s32().Inputs(23);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(2, 0).ref();  // array1
        PARAMETER(3, 1).ref();  // array2
        PARAMETER(4, 2).s32();  // X

        BASIC_BLOCK(7, 3, 6)
        {
            // check array 1
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(6, Opcode::NullCheck).ref().Inputs(2, 5);
            INST(7, Opcode::LenArray).s32().Inputs(6);
            // check array 2
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::NullCheck).ref().Inputs(3, 8);
            INST(10, Opcode::LenArray).s32().Inputs(9);

            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(27, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(4, 7);  // X > len_array1
            INST(28, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(27, 30);
            INST(25, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(4, 10);  // X > len_array2
            INST(26, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(25, 30);

            INST(11, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0, 4);  // 0 < X
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(13, Opcode::Phi).s32().Inputs(0, 20);  // i
            INST(14, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(15, Opcode::NOP);
            INST(16, Opcode::LoadArray).s32().Inputs(2, 13);
            INST(17, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(18, Opcode::NOP);
            INST(19, Opcode::StoreArray).s32().Inputs(3, 13, 16);                  // array2[i] = array1[i]
            INST(20, Opcode::Add).s32().Inputs(13, 1);                             // i++
            INST(21, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(20, 4);  // i < X
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(21);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(23, Opcode::Phi).s32().Inputs(0, 20);
            INST(24, Opcode::Return).s32().Inputs(23);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, LoopSeveralIndexesBoundsChecks)
{
    // applied
    // array coping
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(2, 0).ref();  // array
        PARAMETER(3, 1).s32();  // Y
        PARAMETER(4, 2).s32();  // X

        BASIC_BLOCK(7, 3, 6)
        {
            // check array
            INST(5, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(6, Opcode::NullCheck).ref().Inputs(2, 5);
            INST(7, Opcode::LenArray).s32().Inputs(6);

            INST(36, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});

            INST(11, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(3, 4);  // Y < X
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(13, Opcode::Phi).s32().Inputs(3, 20);  // i

            INST(14, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(15, Opcode::BoundsCheck).s32().Inputs(7, 13, 14);
            INST(16, Opcode::LoadArray).s32().Inputs(2, 15);  // array[i]

            INST(26, Opcode::Sub).s32().Inputs(13, 1);  // i - 1
            INST(27, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(28, Opcode::BoundsCheck).s32().Inputs(7, 26, 27);
            INST(29, Opcode::LoadArray).s32().Inputs(2, 28);  // array[i-1]

            INST(30, Opcode::Add).s32().Inputs(13, 1);  // i + 1
            INST(31, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(32, Opcode::BoundsCheck).s32().Inputs(7, 30, 31);
            INST(33, Opcode::LoadArray).s32().Inputs(2, 32);  // array[i+1]

            INST(34, Opcode::Add).s32().Inputs(16, 29);  // array[i-1] + array[i]
            INST(35, Opcode::Add).s32().Inputs(34, 33);  // array[i-1] + array[i] + array[i+1]

            INST(17, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(18, Opcode::BoundsCheck).s32().Inputs(7, 13, 17);
            INST(19, Opcode::StoreArray).s32().Inputs(2, 18, 35);  // array[i] = array[i-1] + array[i] + array[i+1]

            INST(20, Opcode::Add).s32().Inputs(13, 1);                             // i++
            INST(21, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(20, 4);  // i < X
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(21);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(23, Opcode::Phi).s32().Inputs(0, 20);
            INST(24, Opcode::Return).s32().Inputs(23);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);         // initial
        CONSTANT(1, 1);         // increment
        PARAMETER(2, 0).ref();  // array
        PARAMETER(3, 1).s32();  // Y
        PARAMETER(4, 2).s32();  // X
        CONSTANT(43, -1);
        BASIC_BLOCK(7, 3, 6)
        {
            // check array
            INST(5, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(6, Opcode::NullCheck).ref().Inputs(2, 5);
            INST(7, Opcode::LenArray).s32().Inputs(6);

            INST(36, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(37, Opcode::Add).s32().Inputs(3, 43);
            INST(38, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(37, 0);  // Y-1 < 0
            INST(39, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(38, 36);

            INST(40, Opcode::Add).s32().Inputs(4, 1);
            INST(41, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(40, 7);  // X+1 > len_array
            INST(42, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(41, 36);

            INST(11, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(3, 4);  // Y < X
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(3, 3, 6)
        {
            INST(13, Opcode::Phi).s32().Inputs(3, 20);  // i

            INST(14, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(15, Opcode::NOP);
            INST(16, Opcode::LoadArray).s32().Inputs(2, 13);  // array[i]

            INST(26, Opcode::Sub).s32().Inputs(13, 1);  // i - 1
            INST(27, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(28, Opcode::NOP);
            INST(29, Opcode::LoadArray).s32().Inputs(2, 26);  // array[i-1]

            INST(30, Opcode::Add).s32().Inputs(13, 1);  // i + 1
            INST(31, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(32, Opcode::NOP);
            INST(33, Opcode::LoadArray).s32().Inputs(2, 30);  // array[i+1]

            INST(34, Opcode::Add).s32().Inputs(16, 29);  // array[i-1] + array[i]
            INST(35, Opcode::Add).s32().Inputs(34, 33);  // array[i-1] + array[i] + array[i+1]

            INST(17, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(18, Opcode::NOP);
            INST(19, Opcode::StoreArray).s32().Inputs(2, 13, 35);  // array[i] = array[i-1] + array[i] + array[i+1]

            INST(20, Opcode::Add).s32().Inputs(13, 1);                             // i++
            INST(21, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(20, 4);  // i < X
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(21);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(23, Opcode::Phi).s32().Inputs(0, 20);
            INST(24, Opcode::Return).s32().Inputs(23);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, HeadExitLoop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);  // initial
        CONSTANT(1, 1);  // increment
        PARAMETER(2, 0).ref();
        PARAMETER(3, 1).s32();
        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(5, Opcode::NullCheck).ref().Inputs(2, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(7, Opcode::Phi).s32().Inputs(0, 13);
            INST(8, Opcode::Compare).CC(CC_LT).b().Inputs(7, 3);  // i < X
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(10, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(11, Opcode::BoundsCheck).s32().Inputs(6, 7, 10);
            INST(12, Opcode::StoreArray).s32().Inputs(5, 11, 0);  // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(7, 1);             // i++
        }
        BASIC_BLOCK(5, 1)
        {
            INST(14, Opcode::Return).ref().Inputs(5);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);  // initial
        CONSTANT(1, 1);  // increment
        PARAMETER(2, 0).ref();
        PARAMETER(3, 1).s32();
        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(5, Opcode::NullCheck).ref().Inputs(2, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(15, Opcode::Compare).b().CC(ConditionCode::CC_GT).Inputs(3, 6);
            INST(16, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(15, 30);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(7, Opcode::Phi).s32().Inputs(0, 13);
            INST(8, Opcode::Compare).CC(CC_LT).b().Inputs(7, 3);  // i < X
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(10, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(11, Opcode::NOP);
            INST(12, Opcode::StoreArray).s32().Inputs(5, 7, 0);  // a[i] = 0
            INST(13, Opcode::Add).s32().Inputs(7, 1);            // i++
        }
        BASIC_BLOCK(5, 1)
        {
            INST(14, Opcode::Return).ref().Inputs(5);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, LoopTest1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        PARAMETER(13, 0).ref();  // Array
        PARAMETER(27, 1).s32();  // X
        BASIC_BLOCK(2, 6, 3)
        {
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(14, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(0, 27);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 6, 3)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 13, 27).SrcVregs({0, 1, 2, 3});
            INST(16, Opcode::NullCheck).ref().Inputs(13, 7);
            INST(17, Opcode::LenArray).s32().Inputs(16);
            INST(8, Opcode::BoundsCheck).s32().Inputs(17, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(16, 8, 0);                    // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(10, 27);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(26, Opcode::Phi).s32().Inputs(0, 10);
            INST(12, Opcode::Return).s32().Inputs(26);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);  // initial
        CONSTANT(1, 1);  // increment
        PARAMETER(3, 0).ref();
        PARAMETER(2, 1).s32();
        CONSTANT(55, nullptr);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(20, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(33, Opcode::Compare).SrcType(DataType::REFERENCE).CC(CC_EQ).b().Inputs(3, 55);  // array != nullptr
            INST(34, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(33, 20);
            INST(22, Opcode::LenArray).s32().Inputs(3);
            INST(23, Opcode::Compare).SrcType(DataType::INT32).CC(CC_GT).b().Inputs(2, 22);  // X > len_array
            INST(24, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(23, 20);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_GE).b().Inputs(0, 2);  // 0 < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 5, 3)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 3, 2).SrcVregs({0, 1, 2, 3});
            INST(15, Opcode::NOP);
            INST(16, Opcode::LenArray).s32().Inputs(3);
            INST(8, Opcode::NOP);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 4, 0);      // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);               // i++
            INST(13, Opcode::Compare).CC(CC_GE).b().Inputs(10, 2);  // i < X
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(26, Opcode::Phi).s32().Inputs(0, 10);
            INST(12, Opcode::Return).s32().Inputs(26);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, NewlyAlllocatedCheck)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0x1).s64();
        CONSTANT(1, 0x0).s64();
        CONSTANT(5, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(4, Opcode::NewArray).ref().Inputs(44, 0, 43).TypeId(79);
            INST(6, Opcode::SaveState).Inputs(5, 1, 4).SrcVregs({2, 1, 0});
            INST(7, Opcode::NullCheck).ref().Inputs(4, 6);
            INST(10, Opcode::StoreArray).s32().Inputs(7, 1, 5);
            INST(11, Opcode::Return).ref().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0x1).s64();
        CONSTANT(1, 0x0).s64();
        CONSTANT(5, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(4, Opcode::NewArray).ref().Inputs(44, 0, 43).TypeId(79);
            INST(6, Opcode::SaveState).Inputs(5, 1, 4).SrcVregs({2, 1, 0});
            INST(7, Opcode::NOP);
            INST(10, Opcode::StoreArray).s32().Inputs(4, 1, 5);
            INST(11, Opcode::Return).ref().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, DominatedBoundsCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(1, 1, 0).SrcVregs({2, 1, 0});
            INST(4, Opcode::LenArray).s32().Inputs(0);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).s64().Inputs(0, 5);
            INST(7, Opcode::SaveState).Inputs(6, 1, 0).SrcVregs({2, 1, 0});
            INST(10, Opcode::BoundsCheck).s32().Inputs(4, 1, 7);
            INST(11, Opcode::StoreArray).s64().Inputs(0, 10, 6);
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(1, 1, 0).SrcVregs({2, 1, 0});
            INST(4, Opcode::LenArray).s32().Inputs(0);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).s64().Inputs(0, 5);
            INST(7, Opcode::SaveState).Inputs(6, 1, 0).SrcVregs({2, 1, 0});
            INST(10, Opcode::NOP);
            INST(11, Opcode::StoreArray).s64().Inputs(0, 5, 6);
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, GroupedBoundsCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // a
        PARAMETER(1, 1).s32();  // x
        CONSTANT(2, 1);
        CONSTANT(5, -2);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::LenArray).s32().Inputs(0);
            // a[x] = 1
            INST(7, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::BoundsCheck).s32().Inputs(6, 1, 7);
            INST(9, Opcode::StoreArray).s64().Inputs(0, 8, 2);
            // a[x-1] = 1
            INST(10, Opcode::Sub).s32().Inputs(1, 2);
            INST(11, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(12, Opcode::BoundsCheck).s32().Inputs(6, 10, 11);
            INST(13, Opcode::StoreArray).s64().Inputs(0, 12, 2);
            // a[x+1] = 1
            INST(14, Opcode::Add).s32().Inputs(1, 2);
            INST(15, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(16, Opcode::BoundsCheck).s32().Inputs(6, 14, 15);
            INST(17, Opcode::StoreArray).s64().Inputs(0, 16, 2);
            // a[x+(-2)] = 1
            INST(18, Opcode::Add).s32().Inputs(1, 5);
            INST(19, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(20, Opcode::BoundsCheck).s32().Inputs(6, 18, 19);
            INST(21, Opcode::StoreArray).s64().Inputs(0, 20, 2);
            // a[x-(-2)] = 1
            INST(22, Opcode::Sub).s32().Inputs(1, 5);
            INST(23, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(24, Opcode::BoundsCheck).s32().Inputs(6, 22, 23);
            INST(25, Opcode::StoreArray).s64().Inputs(0, 24, 2);
            INST(26, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).ref();  // a
        PARAMETER(1, 1).s32();  // x
        CONSTANT(2, 1);
        CONSTANT(5, -2);
        CONSTANT(3, 2);
        CONSTANT(4, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::LenArray).s32().Inputs(0);
            INST(7, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});

            INST(30, Opcode::Add).s32().Inputs(1, 5);
            INST(31, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::INT32).Inputs(30, 4);
            INST(32, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(31, 7);
            INST(27, Opcode::Add).s32().Inputs(1, 3);
            INST(28, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::INT32).Inputs(27, 6);
            INST(29, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(28, 7);

            // a[x] = 1
            INST(8, Opcode::NOP);
            INST(9, Opcode::StoreArray).s64().Inputs(0, 1, 2);
            // a[x-1] = 1
            INST(10, Opcode::Sub).s32().Inputs(1, 2);
            INST(11, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(12, Opcode::NOP);
            INST(13, Opcode::StoreArray).s64().Inputs(0, 10, 2);
            // a[x+1] = 1
            INST(14, Opcode::Add).s32().Inputs(1, 2);
            INST(15, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(16, Opcode::NOP);
            INST(17, Opcode::StoreArray).s64().Inputs(0, 14, 2);
            // a[x+(-2)] = 1
            INST(18, Opcode::Add).s32().Inputs(1, 5);
            INST(19, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(20, Opcode::NOP);
            INST(21, Opcode::StoreArray).s64().Inputs(0, 18, 2);
            // a[x-(-2)] = 1
            INST(22, Opcode::Sub).s32().Inputs(1, 5);
            INST(23, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(24, Opcode::NOP);
            INST(25, Opcode::StoreArray).s64().Inputs(0, 22, 2);
            INST(26, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, ConsecutiveNullChecks)
{
    builder_->EnableGraphChecker(false);
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(1, Opcode::NullCheck).ref().Inputs(0, 5);
            INST(2, Opcode::NullCheck).ref().Inputs(1, 5);
            INST(3, Opcode::NullCheck).ref().Inputs(2, 5);
            INST(6, Opcode::StoreObject).ref().Inputs(3, 0).TypeId(1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
    }
    builder_->EnableGraphChecker(true);
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(1, Opcode::NullCheck).ref().Inputs(0, 5);
            INST(2, Opcode::NOP);
            INST(3, Opcode::NOP);
            INST(6, Opcode::StoreObject).ref().Inputs(1, 0).TypeId(1);
            INST(4, Opcode::Return).ref().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NullCheckInCallVirt)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 5);
            INST(3, Opcode::NullCheck).ref().Inputs(1, 5);
            INST(4, Opcode::CallVirtual)
                .s32()
                .Inputs({{DataType::REFERENCE, 2}, {DataType::REFERENCE, 3}, {DataType::NO_TYPE, 5}});
            INST(6, Opcode::Return).s32().Inputs(4);
        }
    }
    // Doesn't remove nullchecks if the method static
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));

    // Removes nullcheck from first parameter for not static method
    RuntimeInterfaceNotStaticMethod runtime;
    GetGraph()->SetRuntime(&runtime);
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(2, Opcode::NOP);
            INST(3, Opcode::NullCheck).ref().Inputs(1, 5);
            INST(4, Opcode::CallVirtual)
                .s32()
                .Inputs({{DataType::REFERENCE, 0}, {DataType::REFERENCE, 3}, {DataType::NO_TYPE, 5}});
            INST(6, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, DominatedChecksWithDifferentTypes)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::ZeroCheck).s32().Inputs(0, 2);
            INST(4, Opcode::Div).s32().Inputs(1, 3);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::ZeroCheck).s64().Inputs(0, 5);
            INST(7, Opcode::Mod).s64().Inputs(1, 6);
            INST(20, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).s32().InputsAutoType(4, 7, 20);
            INST(9, Opcode::Return).s32().Inputs(8);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(ChecksEliminationTest, RefTypeCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        PARAMETER(4, 4).s32();
        CONSTANT(5, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(10, Opcode::SaveState).Inputs(0, 1, 2, 2, 5).SrcVregs({0, 1, 2, 3, 4});
            INST(11, Opcode::NullCheck).ref().Inputs(1, 10);
            INST(12, Opcode::LenArray).s32().Inputs(11);
            INST(13, Opcode::BoundsCheck).s32().Inputs(12, 2, 10);
            INST(14, Opcode::RefTypeCheck).ref().Inputs(11, 0, 10);
            INST(15, Opcode::StoreArray).ref().Inputs(11, 13, 14);

            INST(20, Opcode::SaveState).Inputs(0, 1, 2, 2, 5).SrcVregs({0, 1, 2, 3, 4});
            INST(21, Opcode::NullCheck).ref().Inputs(1, 20);
            INST(22, Opcode::LenArray).s32().Inputs(21);
            INST(23, Opcode::BoundsCheck).s32().Inputs(22, 3, 20);
            INST(24, Opcode::RefTypeCheck).ref().Inputs(21, 0, 20);
            INST(25, Opcode::StoreArray).ref().Inputs(21, 23, 24);

            INST(30, Opcode::SaveState).Inputs(0, 1, 2, 2, 5).SrcVregs({0, 1, 2, 3, 4});
            INST(31, Opcode::NullCheck).ref().Inputs(1, 30);
            INST(32, Opcode::LenArray).s32().Inputs(31);
            INST(33, Opcode::BoundsCheck).s32().Inputs(32, 4, 30);
            INST(34, Opcode::RefTypeCheck).ref().Inputs(31, 5, 30);
            INST(35, Opcode::StoreArray).ref().Inputs(31, 33, 34);

            INST(6, Opcode::ReturnVoid).v0id();
        }
    }

    // `24, Opcode::RefTypeCheck` is removed because `14, Opcode::RefTypeCheck` checks equal array and eqaul Reference
    // `34, Opcode::RefTypeCheck` is removed because store value id NullPtr
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        PARAMETER(4, 4).s32();
        CONSTANT(5, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(10, Opcode::SaveState).Inputs(0, 1, 2, 2, 5).SrcVregs({0, 1, 2, 3, 4});
            INST(11, Opcode::NullCheck).ref().Inputs(1, 10);
            INST(12, Opcode::LenArray).s32().Inputs(11);
            INST(13, Opcode::BoundsCheck).s32().Inputs(12, 2, 10);
            INST(14, Opcode::RefTypeCheck).ref().Inputs(11, 0, 10);
            INST(15, Opcode::StoreArray).ref().Inputs(11, 13, 14);

            INST(20, Opcode::SaveState).Inputs(0, 1, 2, 2, 5).SrcVregs({0, 1, 2, 3, 4});
            INST(21, Opcode::NOP);
            INST(22, Opcode::LenArray).s32().Inputs(11);
            INST(23, Opcode::BoundsCheck).s32().Inputs(22, 3, 20);
            INST(24, Opcode::NOP);
            INST(25, Opcode::StoreArray).ref().Inputs(11, 23, 14);

            INST(30, Opcode::SaveState).Inputs(0, 1, 2, 2, 5).SrcVregs({0, 1, 2, 3, 4});
            INST(31, Opcode::NOP);
            INST(32, Opcode::LenArray).s32().Inputs(11);
            INST(33, Opcode::BoundsCheck).s32().Inputs(32, 4, 30);
            INST(34, Opcode::NOP);
            INST(35, Opcode::StoreArray).ref().Inputs(11, 33, 5);

            INST(6, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, RefTypeCheckEqualInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 2).s32();
        PARAMETER(2, 3).s32();
        CONSTANT(3, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(5, Opcode::NewArray).ref().Inputs(0, 3, 4);
            INST(6, Opcode::RefTypeCheck).ref().Inputs(5, 0, 4);
            INST(7, Opcode::StoreArray).ref().Inputs(5, 1, 6);

            INST(14, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(16, Opcode::RefTypeCheck).ref().Inputs(5, 5, 14);
            INST(17, Opcode::StoreArray).ref().Inputs(5, 2, 16);

            INST(18, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(clone, GetGraph()));
}

TEST_F(ChecksEliminationTest, NotLenArrayInput)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(4, 1);
        CONSTANT(7, 10);
        BASIC_BLOCK(2, 3)
        {
            INST(1, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).s32().Inputs(0, 5);
            INST(6, Opcode::Compare).b().CC(CC_GE).Inputs(3, 7);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(5, 3)
        {
            INST(9, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(10, Opcode::NegativeCheck).s32().Inputs(3, 9);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(9).TypeId(68);
            INST(11, Opcode::NewArray).ref().Inputs(44, 10, 9);
            INST(12, Opcode::BoundsCheck).s32().Inputs(3, 0, 9);
            INST(13, Opcode::StoreArray).s32().Inputs(11, 12, 0);
            INST(5, Opcode::Add).s32().Inputs(3, 4);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(2, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(clone, GetGraph()));
}

TEST_F(ChecksEliminationTest, BugWithNullCheck)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);          // initial
        CONSTANT(1, 1);          // increment
        PARAMETER(27, 1).s32();  // X
        BASIC_BLOCK(2, 6, 3)
        {
            INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(13, Opcode::NewArray).ref().Inputs(44, 27, 43);
            INST(30, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(14, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(0, 27);  // i < X
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(3, 6, 3)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 13, 27).SrcVregs({0, 1, 2, 3});
            INST(16, Opcode::NullCheck).ref().Inputs(13, 7);
            INST(17, Opcode::LenArray).s32().Inputs(16);
            INST(8, Opcode::BoundsCheck).s32().Inputs(17, 4, 7);
            INST(9, Opcode::StoreArray).s32().Inputs(16, 8, 0);                    // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);                              // i++
            INST(5, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(10, 27);  // i < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(26, Opcode::Phi).s32().Inputs(0, 10);
            INST(12, Opcode::Return).s32().Inputs(26);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 0);  // initial
        CONSTANT(1, 1);  // increment
        PARAMETER(2, 1).s32();
        CONSTANT(55, nullptr);
        BASIC_BLOCK(2, 5, 3)
        {
            INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2, 43);
            INST(20, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(33, Opcode::Compare).SrcType(DataType::REFERENCE).CC(CC_EQ).b().Inputs(3, 55);  // array != nullptr
            INST(34, Opcode::NOP);
            INST(22, Opcode::LenArray).s32().Inputs(3);
            INST(23, Opcode::Compare).SrcType(DataType::INT32).CC(CC_GT).b().Inputs(2, 22);  // X > len_array
            INST(24, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(23, 20);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_GE).b().Inputs(0, 2);  // 0 < X
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 5, 3)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(7, Opcode::SaveState).Inputs(0, 1, 3, 2).SrcVregs({0, 1, 2, 3});
            INST(15, Opcode::NOP);
            INST(16, Opcode::LenArray).s32().Inputs(3);
            INST(8, Opcode::NOP);
            INST(9, Opcode::StoreArray).s32().Inputs(3, 4, 0);      // a[i] = 0
            INST(10, Opcode::Add).s32().Inputs(4, 1);               // i++
            INST(13, Opcode::Compare).CC(CC_GE).b().Inputs(10, 2);  // i < X
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(26, Opcode::Phi).s32().Inputs(0, 10);
            INST(12, Opcode::Return).s32().Inputs(26);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, LoopWithTwoPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(3, 0);
        CONSTANT(4, 1);
        BASIC_BLOCK(2, 3)
        {
            INST(5, Opcode::SaveStateDeoptimize).Inputs(0, 1, 2, 3, 4).SrcVregs({0, 1, 2, 3, 4});
            INST(6, Opcode::SaveState).Inputs(0, 1, 2, 3, 4).SrcVregs({0, 1, 2, 3, 4});
            INST(7, Opcode::NullCheck).ref().Inputs(0, 6);
            INST(8, Opcode::LoadObject).ref().Inputs(7);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(9, Opcode::Phi).s32().Inputs(1, 17);
            INST(18, Opcode::Phi).s32().Inputs(3, 19);
            INST(10, Opcode::Compare).b().CC(CC_GE).Inputs(9, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(5, 3)
        {
            INST(12, Opcode::SaveState).Inputs(0, 1, 2, 3, 4, 8).SrcVregs({0, 1, 2, 3, 4, 5});
            INST(13, Opcode::NullCheck).ref().Inputs(8, 12);
            INST(14, Opcode::LenArray).s32().Inputs(13);
            INST(15, Opcode::BoundsCheck).s32().Inputs(14, 9, 12);
            INST(16, Opcode::LoadArray).s32().Inputs(13, 15);
            INST(17, Opcode::Add).s32().Inputs(9, 4);
            INST(19, Opcode::Add).s32().Inputs(18, 16);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(20, Opcode::Return).s32().Inputs(18);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(clone, GetGraph()));
}

TEST_F(ChecksEliminationTest, DeoptTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::ZeroCheck).s32().Inputs(0, 2);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(2);
        }
    }
}

TEST_F(ChecksEliminationTest, DeoptimizeIfTest)
{
    // Check Elimination for DeoptimizeIf is applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);
        CONSTANT(10, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LoadArray).s32().Inputs(3, 1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 10);
            INST(6, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(9, 5);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 1, 4);
            INST(8, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);
        CONSTANT(10, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LoadArray).s32().Inputs(3, 1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 10);
            INST(6, Opcode::NOP);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 1, 4);
            INST(8, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, DeoptimizeIfTest2)
{
    // Check Elimination for NullCheck is applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);
        CONSTANT(10, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 10);
            INST(6, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(9, 2);
            INST(4, Opcode::LoadArray).s32().Inputs(0, 1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 5);
            INST(7, Opcode::StoreArray).s32().Inputs(3, 1, 4);
            INST(8, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);
        CONSTANT(10, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 10);
            INST(6, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(9, 2);
            INST(4, Opcode::LoadArray).s32().Inputs(0, 1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NOP);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 1, 4);
            INST(8, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, DeoptimizeIfTest3)
{
    // Check Elimination not applied different objects.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(15, 1).ref();
        CONSTANT(1, 10);
        CONSTANT(10, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 10);
            INST(6, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(9, 2);
            INST(4, Opcode::LoadArray).s32().Inputs(0, 1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(15, 5);
            INST(7, Opcode::StoreArray).s32().Inputs(3, 1, 4);
            INST(8, Opcode::Return).s32().Inputs(4);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, DeoptimizeIfTest4)
{
    // Check Elimination for DeoptimizeIf NULL_CHECK isn't applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(12, nullptr);
        CONSTANT(1, 10);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(1, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(13, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 12);
            INST(14, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(13, 4);
            INST(6, Opcode::LoadArray).s32().Inputs(0, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::NullCheck).ref().Inputs(0, 7);
            INST(9, Opcode::LoadArray).s32().Inputs(8, 1);
        }

        BASIC_BLOCK(5, 1)
        {
            INST(10, Opcode::Phi).s32().Inputs(6, 9);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, DeoptimizeIfInCallVirt)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(2, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(4, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 2);
            INST(5, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(4, 3);
            INST(6, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(1, 2);
            INST(7, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(6, 3);
            INST(8, Opcode::CallVirtual)
                .s32()
                .Inputs({{DataType::REFERENCE, 0}, {DataType::REFERENCE, 1}, {DataType::NO_TYPE, 3}});
            INST(9, Opcode::Return).s32().Inputs(8);
        }
    }
    // Doesn't remove DeoptimizeIf if the method is static
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));

    // Remove DeoptimizeIf for first parameter for not static method
    RuntimeInterfaceNotStaticMethod runtime;
    GetGraph()->SetRuntime(&runtime);
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(2, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(4, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 2);
            INST(5, Opcode::NOP);
            INST(6, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(1, 2);
            INST(7, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(6, 3);
            INST(8, Opcode::CallVirtual)
                .s32()
                .Inputs({{DataType::REFERENCE, 0}, {DataType::REFERENCE, 1}, {DataType::NO_TYPE, 3}});
            INST(9, Opcode::Return).s32().Inputs(8);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, OmitNullCheck)
{
    // Check Elimination for NullCheck is applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LoadArray).s32().Inputs(3, 1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::LoadClass).ref().Inputs(5);
            INST(7, Opcode::CheckCast).TypeId(1).Inputs(0, 6, 5);
            INST(8, Opcode::LoadClass).ref().Inputs(5);
            INST(9, Opcode::IsInstance).TypeId(2).b().Inputs(0, 8, 5);
            INST(10, Opcode::Return).b().Inputs(9);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
    ASSERT_TRUE(INS(7).CastToCheckCast()->GetOmitNullCheck());
    ASSERT_TRUE(INS(9).CastToIsInstance()->GetOmitNullCheck());
}

TEST_F(ChecksEliminationTest, OmitNullCheckAfterDeoptimizeIf)
{
    // Check Elimination for NullCheck is applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);
        CONSTANT(11, nullptr);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_EQ).Inputs(0, 11);
            INST(4, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(3, 2);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::LoadClass).TypeId(1).ref().Inputs(5);
            INST(7, Opcode::CheckCast).TypeId(1).Inputs(0, 6, 5);
            INST(8, Opcode::LoadClass).TypeId(2).ref().Inputs(5);
            INST(9, Opcode::IsInstance).TypeId(2).b().Inputs(0, 8, 5);
            INST(10, Opcode::Return).b().Inputs(9);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
    ASSERT_TRUE(INS(7).CastToCheckCast()->GetOmitNullCheck());
    ASSERT_TRUE(INS(9).CastToIsInstance()->GetOmitNullCheck());
}

TEST_F(ChecksEliminationTest, DoNotOmitNullCheck)
{
    // NullCheck inside CheckCast and IsInstance cannot be omitted. NullCheck doesn't dominate them.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(1, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LoadArray).s32().Inputs(5, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::LoadClass).TypeId(1).ref().Inputs(7);
            INST(9, Opcode::CheckCast).TypeId(1).Inputs(0, 8, 7);
            INST(10, Opcode::LoadClass).TypeId(2).ref().Inputs(7);
            INST(11, Opcode::IsInstance).TypeId(2).b().Inputs(0, 10, 7);
        }

        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Phi).s32().Inputs(6, 1);
            INST(13, Opcode::Return).s32().Inputs(12);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
    ASSERT_FALSE(INS(9).CastToCheckCast()->GetOmitNullCheck());
    ASSERT_FALSE(INS(11).CastToIsInstance()->GetOmitNullCheck());
}

// TODO(schernykh): It's possible to remove boundschecks from this test, but BoundsAnalysis must be upgrade for it.
TEST_F(ChecksEliminationTest, OptimizeBoundsCheckElimination)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(14, 1);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 6, 3)
        {
            INST(43, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 1, 43);
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(2, 0);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 6, 4)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(2, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 6, 5)
        {
            INST(8, Opcode::Add).s32().Inputs(2, 14);
            INST(9, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(8, 1);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(11, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(12, Opcode::BoundsCheck).s32().Inputs(1, 8, 11);
            INST(13, Opcode::StoreArray).s32().Inputs(3, 8, 0);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, BoundsCheckEqualInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(1, 2).s32();
        PARAMETER(2, 3).s32();
        CONSTANT(3, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::SaveState).Inputs(1, 2).SrcVregs({1, 2});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(5, Opcode::NewArray).ref().Inputs(44, 3, 4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(3, 1, 4);
            INST(7, Opcode::StoreArray).s32().Inputs(5, 6, 3);

            INST(14, Opcode::SaveState).Inputs(1, 2).SrcVregs({1, 2});
            INST(15, Opcode::NewArray).ref().Inputs(44, 2, 14);
            INST(16, Opcode::BoundsCheck).s32().Inputs(2, 3, 14);
            INST(17, Opcode::StoreArray).s32().Inputs(15, 16, 3);

            INST(18, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(clone, GetGraph()));
}

}  // namespace panda::compiler
