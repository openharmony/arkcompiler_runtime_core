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
#include "optimizer/optimizations/vn.h"
#include "optimizer/analysis/alias_analysis.h"

namespace panda::compiler {
class AliasAnalysisTest : public GraphTest {
};

/**
 *    foo (int *arr, int a1, int a2, int a3)
 *        int tmp;
 *        if (a3 < 0)
 *            tmp = arr[a1];
 *        else
 *            tmp = arr[a2];
 *        return tmp + arr[a1] + arr[a2];
 *
 * arr[a1] must alias the second arr[a1] and may alias arr[a2]
 */
TEST_F(AliasAnalysisTest, SimpleLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        CONSTANT(4, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Compare).b().Inputs(3, 4);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(11, Opcode::LoadArray).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(17, Opcode::LoadArray).s32().Inputs(0, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(18, Opcode::Phi).s32().Inputs({{4, 11}, {3, 17}});

            INST(23, Opcode::LoadArray).s32().Inputs(0, 1);

            INST(28, Opcode::LoadArray).s32().Inputs(0, 2);

            INST(29, Opcode::Add).s32().Inputs(28, 23);
            INST(30, Opcode::Add).s32().Inputs(29, 18);
            INST(31, Opcode::Return).s32().Inputs(30);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(0), &INS(0)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(11), &INS(23)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(17), &INS(28)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(11), &INS(28)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(17), &INS(23)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr, int a1, int a2, int a3)
 *        int tmp;
 *        if (a3 < 0)
 *            (NullCheck(arr) for arr[a1]);
 *            (BoundsCheck for a1)
 *            tmp = arr[a1];
 *        else
 *            (NullCheck(arr) for arr[a2]);
 *            (BoundsCheck for a2)
 *            tmp = arr[a2];
 *        (NullCheck(arr) for arr[a1]);
 *        (BoundsCheck for a1)
 *        (NullCheck(arr) for arr[a2]);
 *        (BoundsCheck for a2)
 *        return tmp + arr[a1] + arr[a2];
 *
 * arr[a1] must alias the second arr[a1] and may alias arr[a2]
 */
TEST_F(AliasAnalysisTest, CompleteLoadArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        CONSTANT(4, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Compare).b().Inputs(3, 4);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({2, 3, 4});
            INST(8, Opcode::NullCheck).ref().Inputs(0, 7);
            INST(9, Opcode::LenArray).s32().Inputs(8);
            INST(10, Opcode::BoundsCheck).s32().Inputs(9, 1, 7);
            INST(11, Opcode::LoadArray).s32().Inputs(8, 10);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(13, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({2, 3, 4});
            INST(14, Opcode::NullCheck).ref().Inputs(0, 13);
            INST(15, Opcode::LenArray).s32().Inputs(14);
            INST(16, Opcode::BoundsCheck).s32().Inputs(15, 2, 13);
            INST(17, Opcode::LoadArray).s32().Inputs(14, 16);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(18, Opcode::Phi).s32().Inputs({{4, 11}, {3, 17}});

            INST(19, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({2, 3, 4});
            INST(20, Opcode::NullCheck).ref().Inputs(0, 19);
            INST(21, Opcode::LenArray).s32().Inputs(20);
            INST(22, Opcode::BoundsCheck).s32().Inputs(21, 1, 19);
            INST(23, Opcode::LoadArray).s32().Inputs(20, 22);

            INST(24, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({2, 3, 4});
            INST(25, Opcode::NullCheck).ref().Inputs(0, 24);
            INST(26, Opcode::LenArray).s32().Inputs(25);
            INST(27, Opcode::BoundsCheck).s32().Inputs(26, 2, 24);
            INST(28, Opcode::LoadArray).s32().Inputs(25, 27);

            INST(29, Opcode::Add).s32().Inputs(28, 23);
            INST(30, Opcode::Add).s32().Inputs(29, 18);
            INST(31, Opcode::Return).s32().Inputs(30);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(11), &INS(23)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(17), &INS(28)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(11), &INS(28)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(17), &INS(23)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr, int a1)
 *        (NullCheck(arr) for arr[a1]);
 *        (BoundsCheck for a1)
 *        int tmp = arr[a1];
 *        if (tmp == 0)
 *            (NullCheck(arr) for arr[a1]);
 *            (BoundsCheck for a1)
 *            arr[a1] = tmp;
 *        return tmp;
 *
 * arr[a1] must alias the second arr[a1] and may alias arr[a2]
 */
TEST_F(AliasAnalysisTest, LoadStoreAlias)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        CONSTANT(8, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 1).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).u64().Inputs(3, 5);
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(6, 8);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Return).s64().Inputs(6);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(11, Opcode::SaveState).Inputs(0, 1, 6).SrcVregs({0, 1, 2});
            INST(12, Opcode::NullCheck).ref().Inputs(0, 11);
            INST(13, Opcode::LenArray).s32().Inputs(12);
            INST(14, Opcode::BoundsCheck).s32().Inputs(13, 1, 11);
            INST(15, Opcode::StoreArray).u64().Inputs(12, 14, 6);
            INST(16, Opcode::Return).s64().Inputs(6);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(6), &INS(15)), AliasType::MUST_ALIAS);
}

/**
 *    foo (int *arr1, int *arr2, int a2, int a3)
 *       arr1[a2] = a3
 *       arr2[a3] = a2
 *
 *    All parameters by default may be aliased by each other
 */
TEST_F(AliasAnalysisTest, DifferentObjects)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3, 2).SrcVregs({0, 1, 2, 3, 4});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(7, Opcode::BoundsCheck).s32().Inputs(6, 3, 4);
            INST(8, Opcode::StoreArray).u64().Inputs(5, 7, 2);

            INST(9, Opcode::SaveState).Inputs(0, 1, 2, 3, 3).SrcVregs({0, 1, 2, 3, 4});
            INST(10, Opcode::NullCheck).ref().Inputs(1, 9);
            INST(11, Opcode::LenArray).s32().Inputs(10);
            INST(12, Opcode::BoundsCheck).s32().Inputs(11, 2, 9);
            INST(13, Opcode::StoreArray).u64().Inputs(10, 12, 3);

            INST(14, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(0), &INS(1)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(8), &INS(13)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr, a1, a2)
 *        arr[a1] = a2;
 *        int *tmp = arr;
 *        return tmp[a1];
 *
 * tmp must alias arr
 */
TEST_F(AliasAnalysisTest, ArrayToArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 2).SrcVregs({1, 2, 3, 4});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 1, 3);
            INST(7, Opcode::StoreArray).u64().Inputs(4, 6, 2);

            INST(9, Opcode::SaveState).Inputs(0, 0, 1, 2, 2).SrcVregs({0, 1, 2, 3, 4});
            INST(10, Opcode::NullCheck).ref().Inputs(0, 9);
            INST(11, Opcode::LenArray).s32().Inputs(10);
            INST(12, Opcode::BoundsCheck).s32().Inputs(11, 1, 9);
            INST(13, Opcode::LoadArray).s64().Inputs(10, 12);
            INST(14, Opcode::Return).s64().Inputs(13);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(13)), AliasType::MUST_ALIAS);
}

/**
 *    foo (int *arr0, int *arr1, a2)
 *        int *tmp;
 *        if (a2 == 0)
 *            tmp = arr0;
 *        else
 *            tmp = arr1;
 *        tmp[a2] = 0;
 *        return 0;
 *
 * tmp may alias arr0 and arr1
 */
TEST_F(AliasAnalysisTest, PhiRef)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();
        CONSTANT(4, 0).s64();
        BASIC_BLOCK(2, 4, 3)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(2, 4);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::Phi).ref().Inputs({{3, 0}, {2, 1}});
            INST(8, Opcode::StoreArray).u64().Inputs(7, 2, 4);

            INST(9, Opcode::Return).s64().Inputs(4);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(7), &INS(0)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(7), &INS(1)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr0)
 *        int *x = new int[10];
 *        int tmp = arr0[0];
 *        x[0] = tmp;
 *        return x;
 *
 * arr0 and x are not aliasing
 */
TEST_F(AliasAnalysisTest, NewWithArg)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10).s64();
        CONSTANT(2, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(13, Opcode::SaveState).Inputs(1, 2, 0, 2).SrcVregs({0, 1, 2, 3});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(13).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 1, 13);
            INST(4, Opcode::SaveState).Inputs(1, 2, 3, 0, 2).SrcVregs({0, 1, 2, 3, 4});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LoadArray).u64().Inputs(5, 2);

            INST(7, Opcode::SaveState).Inputs(1, 2, 3, 0, 6).SrcVregs({0, 1, 2, 3, 4});
            INST(8, Opcode::NullCheck).ref().Inputs(3, 7);
            INST(9, Opcode::StoreArray).u64().Inputs(8, 2, 6);
            INST(10, Opcode::Return).ref().Inputs(3);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(3), &INS(0)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(6), &INS(9)), AliasType::NO_ALIAS);
}

/**
 *    foo (int *arr0)
 *        int *x = new int[10];
 *        x[0] = 0;
 *        bar (x);               <- escaping call
 *        x[0] = 0;
 *
 *        int tmp = arr0[0];
 *        x[0] = tmp;
 *        return x;
 *
 * arr0 and x may be aliased because x is escaping because of calling bar function.
 */
TEST_F(AliasAnalysisTest, EscapingNewWithArg)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10).s64();
        CONSTANT(2, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).Inputs(1, 2, 0, 2).SrcVregs({0, 1, 2, 3});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(20).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 1, 20);
            INST(11, Opcode::StoreArray).u64().Inputs(3, 2, 2);
            INST(12, Opcode::CallStatic).ref().InputsAutoType(3, 20);
            INST(13, Opcode::StoreArray).u64().Inputs(3, 2, 2);
            INST(4, Opcode::SaveState).Inputs(1, 2, 3, 0, 2).SrcVregs({0, 1, 2, 3, 4});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LoadArray).u64().Inputs(5, 2);

            INST(7, Opcode::SaveState).Inputs(1, 2, 3, 0, 6).SrcVregs({0, 1, 2, 3, 4});
            INST(8, Opcode::NullCheck).ref().Inputs(3, 7);
            INST(9, Opcode::StoreArray).u64().Inputs(8, 2, 6);
            INST(10, Opcode::Return).ref().Inputs(3);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(3), &INS(0)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(6), &INS(9)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int **arr0)
 *        int *x = new int[10]
 *        arr0[0] = x;        <-- potential escaping
 *        int *y = arr0[10];
 *        x[0] = y[0];
 *
 *    The test aims to track escaping through a sequence of checks.
 */
TEST_F(AliasAnalysisTest, EscapingWithChecks)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 10).s64();
        CONSTANT(2, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(20).TypeId(68);
            INST(9, Opcode::NewArray).ref().Inputs(44, 1, 20);
            INST(10, Opcode::SaveState).Inputs(0, 1, 9).SrcVregs({0, 1, 9});

            // At first glance it is an aritificial sequence of checks but it may
            // happen if several inlinings occurs with multidimentional array operations
            INST(11, Opcode::NullCheck).ref().Inputs(9, 10);
            INST(12, Opcode::RefTypeCheck).ref().Inputs(11, 0, 10);
            INST(15, Opcode::StoreArrayI).ref().Imm(0).Inputs(0, 12);
            INST(16, Opcode::LoadArray).ref().Inputs(0, 1);

            INST(17, Opcode::LoadArrayI).u32().Imm(0).Inputs(16);
            INST(18, Opcode::StoreArrayI).u32().Imm(0).Inputs(9, 17);
            INST(19, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(17), &INS(18)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr0, int a1)
 *        int tmp;
 *        if (a1 == 0)
 *            tmp = arr0[a1 + 10];
 *        else
 *            tmp = 10;
 *            arr0[a1 + 10] = tmp;
 *        return tmp;
 *
 * both arr0[a1 + 10] must alias each other
 */
TEST_F(AliasAnalysisTest, SimpleHeuristic)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        CONSTANT(2, 10).s64();
        CONSTANT(4, 0).s64();

        BASIC_BLOCK(2, 4, 3)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(1, 4);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(21, Opcode::AddI).s32().Imm(10ULL).Inputs(1);
            INST(7, Opcode::SaveState).Inputs(21, 0, 1, 2).SrcVregs({5, 3, 4, 0});
            INST(8, Opcode::NullCheck).ref().Inputs(0, 7);
            INST(9, Opcode::LenArray).s32().Inputs(8);
            INST(10, Opcode::BoundsCheck).s32().Inputs(9, 21, 7);
            INST(11, Opcode::LoadArray).s64().Inputs(8, 10);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(22, Opcode::AddI).s32().Imm(10ULL).Inputs(1);
            INST(14, Opcode::SaveState).Inputs(1, 22, 0, 1, 2).SrcVregs({4, 2, 3, 0, 5});
            INST(15, Opcode::NullCheck).ref().Inputs(0, 14);
            INST(16, Opcode::LenArray).s32().Inputs(15);
            INST(17, Opcode::BoundsCheck).s32().Inputs(16, 22, 14);
            INST(18, Opcode::StoreArray).u64().Inputs(15, 17, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(19, Opcode::Phi).s64().Inputs({{3, 11}, {4, 2}});
            INST(20, Opcode::Return).s64().Inputs(19);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(11), &INS(18)), AliasType::MUST_ALIAS);
}

/**
 *    foo (int32_t *arr0, int64_t *arr1, int64_t a2, int32_t a3)
 *        arr0[a2] = a2;
 *        arr1[a2] = arr0[a3];
 *        return arr1[a3];
 *
 * arr0 cannot alias arr1 due to different types
 */
TEST_F(AliasAnalysisTest, TypeComparison)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // i32[]
        PARAMETER(1, 1).ref();  // i64[]
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3, 2).SrcVregs({0, 1, 2, 3, 4});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).s32().Inputs(5);
            INST(7, Opcode::BoundsCheck).s32().Inputs(6, 2, 4);
            INST(8, Opcode::StoreArray).u16().Inputs(5, 7, 2);

            INST(9, Opcode::SaveState).Inputs(0, 1, 2, 3, 3).SrcVregs({0, 1, 2, 3, 4});
            INST(12, Opcode::BoundsCheck).s32().Inputs(6, 3, 9);
            INST(13, Opcode::LoadArray).u16().Inputs(5, 12);

            INST(14, Opcode::SaveState).Inputs(0, 1, 2, 3, 13).SrcVregs({0, 1, 2, 3, 4});
            INST(15, Opcode::NullCheck).ref().Inputs(1, 14);
            INST(16, Opcode::LenArray).s32().Inputs(15);
            INST(17, Opcode::BoundsCheck).s32().Inputs(16, 2, 14);
            INST(18, Opcode::StoreArray).u32().Inputs(15, 17, 13);

            INST(19, Opcode::SaveState).Inputs(0, 1, 2, 3, 3).SrcVregs({0, 1, 2, 3, 4});
            INST(22, Opcode::BoundsCheck).s32().Inputs(16, 3, 19);
            INST(23, Opcode::LoadArray).u32().Inputs(15, 22);
            INST(25, Opcode::Return).s32().Inputs(23);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(8), &INS(13)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(18), &INS(23)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(8), &INS(18)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(8), &INS(23)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(13), &INS(18)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(13), &INS(23)), AliasType::NO_ALIAS);
}

/**
 *    .function i32 foo(i32[] a0, R1[] a1, R1[] a2) {
 *        movi v0, 0
 *        lda v0
 *        ldarr.obj a1
 *        starr.obj a2, v0
 *
 *        lda v0
 *        ldarr a0
 *        return
 *    }
 *
 * Arrays of primitive types cannot alias arrays of ref type.
 */
TEST_F(AliasAnalysisTest, TypeComparison2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::LoadArray).ref().Inputs(1, 3);
            INST(13, Opcode::StoreArray).ref().Inputs(2, 3, 8);
            INST(18, Opcode::LoadArray).s32().Inputs(0, 3);
            INST(19, Opcode::Return).s32().Inputs(18);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(8), &INS(13)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(8), &INS(18)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(13), &INS(18)), AliasType::NO_ALIAS);
}

/**
 *    foo(int *arr0, int *arr1, int a2)
 *        arr0[0] = a2;
 *        arr0[1] = a2;
 *        arr1[0] = a2;
 *        arr0[a2] = arr0[1];
 *
 * arr0[0] and arr0[1] do not alias each other
 * arr0[0] and arr1[0] may alias each other
 * arr0[a2] and arr0[0] may alias each other
 */
TEST_F(AliasAnalysisTest, LoadStoreImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(2, 2, 0, 1).SrcVregs({5, 4, 2, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheckI).s32().Imm(0).Inputs(5, 3);
            INST(7, Opcode::StoreArrayI).u64().Imm(0).Inputs(4, 2);

            INST(8, Opcode::SaveState).Inputs(2, 2, 0, 1).SrcVregs({5, 4, 2, 3});
            INST(9, Opcode::BoundsCheckI).s32().Imm(1).Inputs(5, 8);
            INST(10, Opcode::StoreArrayI).u64().Imm(1).Inputs(4, 2);

            INST(11, Opcode::SaveState).Inputs(2, 2, 0, 1).SrcVregs({5, 4, 2, 3});
            INST(12, Opcode::NullCheck).ref().Inputs(1, 11);
            INST(13, Opcode::LenArray).s32().Inputs(12);
            INST(14, Opcode::BoundsCheckI).s32().Imm(0).Inputs(13, 11);
            INST(15, Opcode::StoreArrayI).u64().Imm(0).Inputs(12, 2);

            INST(16, Opcode::SaveState).Inputs(2, 1, 0).SrcVregs({4, 3, 2});
            INST(17, Opcode::BoundsCheckI).s32().Imm(1).Inputs(5, 16);
            INST(18, Opcode::LoadArrayI).u64().Imm(1).Inputs(4);

            INST(19, Opcode::SaveState).Inputs(18, 2, 0, 1).SrcVregs({5, 4, 2, 3});
            INST(20, Opcode::BoundsCheck).s32().Inputs(5, 2, 19);
            INST(21, Opcode::StoreArray).u64().Inputs(4, 20, 18);

            INST(22, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(10)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(18)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(15)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(21)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(15), &INS(18)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(10), &INS(18)), AliasType::MUST_ALIAS);
}

/**
 *    .record R1 {}
 *    .record R3 {}
 *    .record R4 {
 *        R1     sf_r1 <static>   # type_id = 349
 *        R1     sf_r2 <static>   # type_id = 363
 *        R3     sf_r3 <static>   # type_id = 377
 *    }
 *    .function void foo() {
 *        ldstatic R4.sf_r1
 *        sta.obj v0
 *        ldstatic R4.sf_r3
 *        sta.obj v1
 *        lda.obj v0
 *        ststatic R4.sf_r3
 *        lda.obj v1
 *        ststatic R4.sf_r2
 *        return.void
 *    }
 *
 * Static acceses MUST_ALIAS themselves and has NO_ALIAS with anything else
 */
TEST_F(AliasAnalysisTest, StaticFields)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(5, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(9, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(9).TypeId(0U);
            INST(7, Opcode::LoadAndInitClass).ref().Inputs(9).TypeId(0U);
            INST(8, Opcode::LoadAndInitClass).ref().Inputs(9).TypeId(0U);
            INST(1, Opcode::LoadStatic).ref().Inputs(6).TypeId(349U);
            INST(2, Opcode::LoadStatic).ref().Inputs(7).TypeId(377U);
            INST(3, Opcode::StoreStatic).ref().Inputs(7, 1).TypeId(377U);
            INST(4, Opcode::StoreStatic).ref().Inputs(8, 2).TypeId(363U);
            INST(25, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    /* Global variable must alias itself */
    ASSERT_EQ(alias.CheckInstAlias(&INS(2), &INS(3)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(1), &INS(2)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(1), &INS(4)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(2), &INS(4)), AliasType::NO_ALIAS);
}

/*
 *    .record R1 {}
 *    .record R7 {
 *        R9 ref<static> # type_id = 211
 *    }
 *    .record R8 {
 *        R9 ref         $ type_id = 268
 *    }
 *    .record R9 {
 *        R1 ref         # type_id = 242
 *    }
 *    .function R1 foo(R8 a0, R9 a1) {
 *        ldstatic R7.ref
 *        sta.obj v1
 *        ldobj v1, R9.ref
 *        sta.obj v0
 *
 *        ldobj a0, R8.ref
 *        sta.obj v1
 *        ldobj v1, R9.ref
 *        stobj a1, R9.ref
 *
 *        lda.obj v0
 *        return.obj
 *    }
 *
 * R7.ref MAY_ALIAS R9.ref from argument
 */
TEST_F(AliasAnalysisTest, StaticFieldObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(16, Opcode::SaveState).Inputs(0, 1).SrcVregs({2, 3});
            INST(9, Opcode::LoadAndInitClass).ref().Inputs(16).TypeId(0U);
            INST(2, Opcode::LoadStatic).Inputs(9).ref().TypeId(211U);
            INST(5, Opcode::LoadObject).ref().Inputs(2).TypeId(242U);
            INST(8, Opcode::LoadObject).ref().Inputs(0).TypeId(268U);
            INST(11, Opcode::LoadObject).ref().Inputs(8).TypeId(242U);
            INST(14, Opcode::StoreObject).ref().Inputs(1, 11).TypeId(242U);
            INST(15, Opcode::Return).ref().Inputs(5);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(14), &INS(11)), AliasType::MAY_ALIAS);
    /* Static field can be asigned outside */
    ASSERT_EQ(alias.CheckInstAlias(&INS(14), &INS(5)), AliasType::MAY_ALIAS);
}

/**
 *    .record R1 {}
 *    .record R2 {
 *        R1 if_r1 # type_id = 338
 *        R1 if_r2 # type_id = 352
 *    }
 *    .function void foo(R2 a0, R1 a1, R1 a2) {
 *        ldobj a0, R2.if_r1
 *        stobj a0, R2.if_r2
 *        lda.obj a1
 *        stobj a0, R2.if_r1
 *        return.void
 *    }
 *
 * Generally, if an object is not created in current scope it may alias any
 * other object. However, different fields even of the same object do not alias
 * each other.
 */
TEST_F(AliasAnalysisTest, ObjectFields)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LoadObject).ref().Inputs(3).TypeId(338U);

            INST(5, Opcode::SaveState).Inputs(0, 1, 4).SrcVregs({0, 1, 2});
            INST(6, Opcode::NullCheck).ref().Inputs(0, 5);
            INST(7, Opcode::StoreObject).ref().Inputs(6, 4).TypeId(352U);

            INST(8, Opcode::SaveState).Inputs(0, 1, 4).SrcVregs({0, 1, 2});
            INST(9, Opcode::NullCheck).ref().Inputs(0, 8);
            INST(10, Opcode::StoreObject).ref().Inputs(9, 1).TypeId(338U);

            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(4), &INS(7)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(10)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(4), &INS(10)), AliasType::MUST_ALIAS);
}

/**
 *    .record R3 {
 *        R1 if_ref # type_id = 302
 *    }
 *    .record R4 {
 *        R3 if_r3  # type_id = 366
 *    }
 *    .function R3 init16(R4 a0, R1 a1) {
 *        ldobj a0, R4.if_r3
 *        sta.obj v1
 *        lda.obj a1
 *        stobj v1, R3.if_ref
 *
 *        newobj v2, R4
 *        newobj v3, R3
 *        lda.obj v3
 *        stobj v2, R4.if_r3
 *        return.obj
 *    }
 *
 * Similar to ObjectFields test but the same fields has NO_ALIAS due to
 * creation of the object in the current scope
 */
TEST_F(AliasAnalysisTest, MoreObjectFields)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({3, 4});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LoadObject).ref().Inputs(3).TypeId(366U);

            INST(5, Opcode::SaveState).Inputs(4, 1).SrcVregs({1, 5});
            INST(6, Opcode::NullCheck).ref().Inputs(4, 5);
            INST(7, Opcode::StoreObject).ref().Inputs(6, 1).TypeId(302U);

            INST(14, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(8, Opcode::NewObject).ref().Inputs(14, 5);
            INST(9, Opcode::NewObject).ref().Inputs(14, 5);
            INST(10, Opcode::SaveState).Inputs(4, 8, 9).SrcVregs({1, 2, 5});
            INST(11, Opcode::NullCheck).ref().Inputs(8, 10);
            INST(12, Opcode::StoreObject).ref().Inputs(11, 9).TypeId(366U);
            INST(13, Opcode::Return).ref().Inputs(9);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(4), &INS(12)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(4), &INS(7)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(12)), AliasType::NO_ALIAS);
}

/*
     struct {
         void *o1
         void *o2
         void *o3
     } st

     foo(uintptr *arr, int a1)
         st.o1 = "my_str"
         st.o2 = "your_str"
         st.o3 = "my_str"

  Values from pool MUST_ALIAS themselves
 */
TEST_F(AliasAnalysisTest, PoolAlias)
{
    uint32_t my_str_id = 0;
    uint32_t your_str_id = 1;
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(12, Opcode::SaveState).NoVregs();
            INST(0, Opcode::LoadString).ref().Inputs(12).TypeId(my_str_id);
            INST(9, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(9);
            INST(1, Opcode::StoreStatic).ref().Inputs(6, 0);

            INST(13, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadString).ref().Inputs(13).TypeId(your_str_id);
            INST(10, Opcode::SaveState).Inputs(2).SrcVregs({0});
            INST(7, Opcode::LoadAndInitClass).ref().Inputs(10);
            INST(3, Opcode::StoreStatic).ref().Inputs(7, 2);

            INST(14, Opcode::SaveState).Inputs(2).SrcVregs({0});
            INST(4, Opcode::LoadString).ref().Inputs(14).TypeId(my_str_id);
            INST(11, Opcode::SaveState).Inputs(4).SrcVregs({0});
            INST(8, Opcode::LoadAndInitClass).ref().Inputs(11);
            INST(5, Opcode::StoreStatic).ref().Inputs(8, 4);

            INST(21, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(0), &INS(0)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(0), &INS(2)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(0), &INS(4)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(2), &INS(4)), AliasType::NO_ALIAS);
}

/*
 *    .function R1 foo(R1[][] a0, i64 a1) {
 *        lda a1
 *        ldarr.obj a0
 *        sta.obj v0
 *        lda a1
 *        ldarr.obj v0
 *        return.obj
 *    }
 *
 */
TEST_F(AliasAnalysisTest, NestedArrays)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::LoadArray).ref().Inputs(0, 2);
            INST(12, Opcode::LoadArray).ref().Inputs(7, 2);
            INST(13, Opcode::Return).ref().Inputs(12);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(7)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(12)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(12), &INS(12)), AliasType::MUST_ALIAS);
}

/**
 * Nested arrays with immediate indices.
 */
TEST_F(AliasAnalysisTest, NestedArraysImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();
        CONSTANT(5, 0x2a).s64();
        CONSTANT(6, 0x3a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::LoadArray).ref().Inputs(0, 2);
            INST(12, Opcode::LoadArrayI).ref().Inputs(7).Imm(5);
            INST(13, Opcode::LoadArrayI).ref().Inputs(7).Imm(6);
            INST(14, Opcode::SaveState).Inputs(0, 12, 13).SrcVregs({0, 1, 2});
            INST(15, Opcode::NullCheck).ref().Inputs(12, 14);
            INST(16, Opcode::NullCheck).ref().Inputs(13, 14);
            INST(17, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(12), &INS(13)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(12)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7), &INS(13)), AliasType::MAY_ALIAS);
}

/*
 *    .function void foo (i64[] a0, i64 a1, i64 a2) {
 *        call.short GetArray
 *        sta.obj v0
 *        lda a2
 *        starr.64 v0, a1
 *        starr.64 a0, a1
 *        return.void
 *    }
 *
 * References obtained from calls are like parameters
 */
TEST_F(AliasAnalysisTest, StaticCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 2).SrcVregs({1, 3});
            INST(4, Opcode::CallStatic).ref().Inputs({{DataType::NO_TYPE, 3}});
            INST(9, Opcode::StoreArray).u64().Inputs(4, 1, 2);
            INST(14, Opcode::StoreArray).u64().Inputs(0, 1, 2);
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(9), &INS(14)), AliasType::MAY_ALIAS);
}

/*
 *    .record R1 {
 *        ref f
 *    }
 *    .function void init3(R1[] a0, R1 a1) {
 *        movi v0, 23
 *        movi v2, 39
 *        lda v0
 *        ldarr.obj a0
 *        sta.obj v1
 *        lda v2
 *        ldarr.obj a0
 *        sta.obj v3
 *        ldobj.64 a1, R1.f
 *        stobj.64 v1, R1.f
 *        stobj.64 v3, R1.f
 *        return.void
 *    }
 *
 * Reference came from Load with immediate index
 */
TEST_F(AliasAnalysisTest, ImmediateRefLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(16, Opcode::LoadArrayI).ref().Inputs(0).Imm(0x17);
            INST(17, Opcode::LoadArrayI).ref().Inputs(0).Imm(0x27);
            INST(10, Opcode::LoadObject).ref().Inputs(1).TypeId(1);
            INST(13, Opcode::StoreObject).ref().Inputs(16, 10).TypeId(1);
            INST(14, Opcode::StoreObject).ref().Inputs(17, 10).TypeId(1);
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(10), &INS(13)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(16), &INS(17)), AliasType::MAY_ALIAS);
}

/* Null pointer dereferences. */
TEST_F(AliasAnalysisTest, NullPtrLoads)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, nullptr).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::LoadArrayI).u64().Inputs(0).Imm(0);
            INST(2, Opcode::LoadArrayI).u64().Inputs(0).Imm(0);
            INST(3, Opcode::Add).u64().Inputs(1, 2);
            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(1), &INS(2)), AliasType::MUST_ALIAS);
}

/* NullCheck in Phi inputs. */
TEST_F(AliasAnalysisTest, NullCheckPhiInput)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(7, nullptr).ref();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(8, Opcode::Compare).b().Inputs(1, 7);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1, 7).SrcVregs({0, 1, 2});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 5);
            INST(3, Opcode::NullCheck).ref().Inputs(1, 5);
            INST(4, Opcode::CallVirtual)
                .v0id()
                .Inputs({{DataType::REFERENCE, 2}, {DataType::REFERENCE, 3}, {DataType::NO_TYPE, 5}});
        }
        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Phi).ref().Inputs({{3, 3}, {2, 7}});
            INST(11, Opcode::LoadArrayI).u64().Inputs(10).Imm(0);
            INST(12, Opcode::LoadArrayI).u64().Inputs(0).Imm(0);
            INST(13, Opcode::Add).u64().Inputs(11, 12);
            INST(14, Opcode::Return).u64().Inputs(13);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(11), &INS(12)), AliasType::MAY_ALIAS);
}

/**
 * Select instruction is very similar to Phi.
 */
TEST_F(AliasAnalysisTest, Select)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(2, 10).s64();
        CONSTANT(3, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(13, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(13).TypeId(68);
            INST(4, Opcode::NewArray).ref().Inputs(44, 2, 13);
            INST(5, Opcode::SelectImm)
                .ref()
                .SrcType(DataType::BOOL)
                .CC(CC_NE)
                .Imm(3)
                .Inputs(0, 1, 3)
                .SetFlag(compiler::inst_flags::NO_CSE)
                .SetFlag(compiler::inst_flags::NO_HOIST);
            INST(6, Opcode::SelectImm)
                .ref()
                .SrcType(DataType::BOOL)
                .CC(CC_NE)
                .Imm(3)
                .Inputs(5, 4, 3)
                .SetFlag(compiler::inst_flags::NO_CSE)
                .SetFlag(compiler::inst_flags::NO_HOIST);
            INST(10, Opcode::Return).ref().Inputs(6);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(4), &INS(5)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(4), &INS(6)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(0), &INS(5)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(1), &INS(5)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(0), &INS(6)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(1), &INS(6)), AliasType::MAY_ALIAS);
}

TEST_F(AliasAnalysisTest, LoadPairObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(12, Opcode::LoadArrayPair).ref().Inputs(0, 1);
            INST(14, Opcode::LoadPairPart).ref().Inputs(12).Imm(0x0);
            INST(13, Opcode::LoadPairPart).ref().Inputs(12).Imm(0x1);
            INST(15, Opcode::LoadObject).s32().Inputs(13).TypeId(42);
            INST(16, Opcode::LoadObject).s32().Inputs(14).TypeId(42);
            INST(17, Opcode::Add).s32().Inputs(15, 16);
            INST(18, Opcode::Return).s32().Inputs(17);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(15), &INS(16)), AliasType::MAY_ALIAS);
}

TEST_F(AliasAnalysisTest, CatchPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Try).CatchTypeIds({0xE1});
        }
        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LoadObject).ref().Inputs(4).TypeId(122);
            INST(6, Opcode::SaveState).Inputs(0, 5).SrcVregs({0, 5});
            INST(7, Opcode::NullCheck).ref().Inputs(5, 6);
            INST(9, Opcode::Return).ref().Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::CatchPhi).ref().Inputs(0, 5);
            INST(11, Opcode::Return).ref().Inputs(10);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(10), &INS(0)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(10), &INS(5)), AliasType::MAY_ALIAS);
}

/**
 *  Test the assert that NullCheck is bypassed through RefTypeCheck
 */
TEST_F(AliasAnalysisTest, RefTypeCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, 1)
        {
            INST(10, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(11, Opcode::NullCheck).ref().Inputs(1, 10);
            INST(14, Opcode::RefTypeCheck).ref().Inputs(0, 11, 10);
            INST(15, Opcode::StoreArrayI).ref().Imm(0).Inputs(0, 14);

            INST(6, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(15), &INS(15)), AliasType::MUST_ALIAS);
}

/**
 *  The field access is determined by field reference (if present) rather than TypeId.
 */
TEST_F(AliasAnalysisTest, InheritedFields)
{
    RuntimeInterface::FieldPtr field = (void *)(0xDEADBEEF);
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::SaveState).NoVregs();
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::LoadStatic).ref().Inputs(2).TypeId(377U).ObjField(field);
            INST(4, Opcode::StoreStatic).ref().Inputs(2, 3).TypeId(378U).ObjField(field);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(3), &INS(4)), AliasType::MUST_ALIAS);
}

TEST_F(AliasAnalysisTest, PairedAccessesAliasing)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(10, Opcode::LoadArrayPairI).ref().Imm(0x0).Inputs(0);
            INST(11, Opcode::LoadPairPart).ref().Imm(0x0).Inputs(10);
            INST(12, Opcode::LoadPairPart).ref().Imm(0x1).Inputs(10);
            INST(14, Opcode::LoadObject).ref().TypeId(3005).Inputs(12);
            INST(17, Opcode::StoreArrayI).s32().Imm(0x0).Inputs(14, 1);
            INST(18, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(17), &INS(17)), AliasType::MUST_ALIAS);
}

/**
 * Volatile load does not quarantee that the same value would be loaded next time.
 */
TEST_F(AliasAnalysisTest, VolatileAliasing)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, 1)
        {
            INST(11, Opcode::LoadObject).ref().TypeId(3005).Volatile().Inputs(0);
            INST(12, Opcode::LoadObject).ref().TypeId(3005).Volatile().Inputs(0);
            INST(13, Opcode::LoadObject).s32().TypeId(4005).Inputs(11);
            INST(14, Opcode::LoadObject).s32().TypeId(4005).Inputs(12);
            INST(15, Opcode::Add).s32().Inputs(13, 14);

            INST(16, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(17, Opcode::LoadAndInitClass).ref().Inputs(16).TypeId(0U);
            INST(18, Opcode::LoadStatic).ref().TypeId(5005).Volatile().Inputs(17);
            INST(19, Opcode::LoadStatic).ref().TypeId(5005).Volatile().Inputs(17);
            INST(20, Opcode::LoadObject).s32().TypeId(4005).Inputs(18);
            INST(21, Opcode::LoadObject).s32().TypeId(4005).Inputs(19);
            INST(22, Opcode::Add).s32().Inputs(20, 21);
            INST(23, Opcode::Add).s32().Inputs(15, 22);
            INST(24, Opcode::Return).s32().Inputs(23);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    AliasAnalysis &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(13), &INS(14)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(20), &INS(21)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(13), &INS(21)), AliasType::MAY_ALIAS);
}
}  //  namespace panda::compiler
