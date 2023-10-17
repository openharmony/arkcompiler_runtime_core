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
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/peepholes.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
class BoundsAnalysisTest : public CommonTest {
public:
    BoundsAnalysisTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

    // ll, lr, rl, rr - test bounds
    // rll, rlr, rrl, rrr - result bounds
    void CCTest(ConditionCode cc, int64_t ll, int64_t lr, int64_t rl, int64_t rr, int64_t rll, int64_t rlr, int64_t rrl,
                int64_t rrr)
    {
        auto [nrl, nrr] = BoundsRange::TryNarrowBoundsByCC(cc, {BoundsRange(ll, lr), BoundsRange(rl, rr)});
        ASSERT_EQ(nrl.GetLeft(), rll);
        ASSERT_EQ(nrl.GetRight(), rlr);
        ASSERT_EQ(nrr.GetLeft(), rrl);
        ASSERT_EQ(nrr.GetRight(), rrr);
    }

private:
    Graph *graph_ {nullptr};
};
// NOLINTBEGIN(readability-magic-numbers)
TEST_F(BoundsAnalysisTest, NegTest)
{
    BoundsRange r1 = BoundsRange(-1, 5);
    BoundsRange res1 = r1.Neg();
    EXPECT_EQ(res1.GetLeft(), -5);
    EXPECT_EQ(res1.GetRight(), 1);

    BoundsRange r2 = BoundsRange(1, 5);
    BoundsRange res2 = r2.Neg();
    EXPECT_EQ(res2.GetLeft(), -5);
    EXPECT_EQ(res2.GetRight(), -1);

    BoundsRange r3 = BoundsRange(-5, -1);
    BoundsRange res3 = r3.Neg();
    EXPECT_EQ(res3.GetLeft(), 1);
    EXPECT_EQ(res3.GetRight(), 5);
}

TEST_F(BoundsAnalysisTest, AbsTest)
{
    BoundsRange r1 = BoundsRange(1, 5);
    BoundsRange res1 = r1.Abs();
    EXPECT_EQ(res1.GetLeft(), 1);
    EXPECT_EQ(res1.GetRight(), 5);

    BoundsRange r2 = BoundsRange(-5, -1);
    BoundsRange res2 = r2.Abs();
    EXPECT_EQ(res2.GetLeft(), 1);
    EXPECT_EQ(res2.GetRight(), 5);

    BoundsRange r3 = BoundsRange(-1, 5);
    BoundsRange res3 = r3.Abs();
    EXPECT_EQ(res3.GetLeft(), 0);
    EXPECT_EQ(res3.GetRight(), 5);

    BoundsRange r4 = BoundsRange(-10, 5);
    BoundsRange res4 = r4.Abs();
    EXPECT_EQ(res4.GetLeft(), 0);
    EXPECT_EQ(res4.GetRight(), 10);
}

TEST_F(BoundsAnalysisTest, MulTest)
{
    BoundsRange r1 = BoundsRange(-7, 5);
    BoundsRange r2 = BoundsRange(9, 13);
    BoundsRange r3 = BoundsRange(2, 5);
    BoundsRange r4 = BoundsRange(-4, -1);
    BoundsRange r5 = BoundsRange(-5, -2);
    BoundsRange r6 = BoundsRange(1, 4);

    // All posible variations for GetLeft and GetRight
    BoundsRange res;
    res = r1.Mul(r1);
    EXPECT_EQ(res.GetLeft(), -7 * 5);    // min = ll * rr
    EXPECT_EQ(res.GetRight(), -7 * -7);  // max = ll * rl

    res = r2.Mul(r2);
    EXPECT_EQ(res.GetLeft(), 9 * 9);     // min = ll * rl
    EXPECT_EQ(res.GetRight(), 13 * 13);  // max = lr * rr

    res = r3.Mul(r4);
    EXPECT_EQ(res.GetLeft(), 5 * -4);   // min = lr * rl
    EXPECT_EQ(res.GetRight(), 2 * -1);  // max = ll * rr

    res = r4.Mul(r5);
    EXPECT_EQ(res.GetLeft(), -1 * -2);   // min = lr * rr
    EXPECT_EQ(res.GetRight(), -4 * -5);  // max = ll * rl

    res = r5.Mul(r6);
    EXPECT_EQ(res.GetLeft(), -5 * 4);   // min = ll * rr
    EXPECT_EQ(res.GetRight(), -2 * 1);  // max = lr * rl
}

TEST_F(BoundsAnalysisTest, DivTest)
{
    auto l1 = BoundsRange(-7, 5);
    auto l2 = BoundsRange(5, 7);
    auto l3 = BoundsRange(-7, -5);
    auto r1 = BoundsRange(2);
    auto r2 = BoundsRange(-2);
    BoundsRange res;

    res = l1.Div(r1);
    EXPECT_EQ(res.GetLeft(), -3);
    EXPECT_EQ(res.GetRight(), 2);
    res = l1.Div(r2);
    EXPECT_EQ(res.GetLeft(), -2);
    EXPECT_EQ(res.GetRight(), 3);

    res = l2.Div(r1);
    EXPECT_EQ(res.GetLeft(), 2);
    EXPECT_EQ(res.GetRight(), 3);
    res = l2.Div(r2);
    EXPECT_EQ(res.GetLeft(), -3);
    EXPECT_EQ(res.GetRight(), -2);

    res = l3.Div(r1);
    EXPECT_EQ(res.GetLeft(), -3);
    EXPECT_EQ(res.GetRight(), -2);
    res = l3.Div(r2);
    EXPECT_EQ(res.GetLeft(), 2);
    EXPECT_EQ(res.GetRight(), 3);
}

TEST_F(BoundsAnalysisTest, OverflowTest)
{
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(INT64_MAX, INT64_MAX), INT64_MAX);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(INT64_MAX, 1), INT64_MAX);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(1, INT64_MAX), INT64_MAX);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(INT64_MIN, INT64_MIN), INT64_MIN);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(INT64_MIN, -1), INT64_MIN);
    ASSERT_EQ(BoundsRange::AddWithOverflowCheck(-1, INT64_MIN), INT64_MIN);

    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(0, INT64_MAX), 0);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, 0), 0);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(0, INT64_MIN), 0);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, 0), 0);

    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, INT64_MAX), INT64_MAX);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, 2), INT64_MAX);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, INT64_MIN), INT64_MAX);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, -2), INT64_MAX);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, INT64_MIN), INT64_MIN);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MAX, -2), INT64_MIN);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, INT64_MAX), INT64_MIN);
    ASSERT_EQ(BoundsRange::MulWithOverflowCheck(INT64_MIN, 2), INT64_MIN);

    ASSERT_EQ(BoundsRange::DivWithOverflowCheck(INT64_MIN, -1), INT64_MIN);
}

TEST_F(BoundsAnalysisTest, BoundsNarrowing)
{
    // case 1
    CCTest(ConditionCode::CC_GT, 10, 50, 20, 60, 21, 50, 20, 49);
    CCTest(ConditionCode::CC_A, 10, 50, 20, 60, 21, 50, 20, 49);
    CCTest(ConditionCode::CC_GE, 10, 50, 20, 60, 20, 50, 20, 50);
    CCTest(ConditionCode::CC_AE, 10, 50, 20, 60, 20, 50, 20, 50);
    CCTest(ConditionCode::CC_LT, 10, 50, 20, 60, 10, 50, 20, 60);
    CCTest(ConditionCode::CC_B, 10, 50, 20, 60, 10, 50, 20, 60);
    CCTest(ConditionCode::CC_LE, 10, 50, 20, 60, 10, 50, 20, 60);
    CCTest(ConditionCode::CC_BE, 10, 50, 20, 60, 10, 50, 20, 60);
    CCTest(ConditionCode::CC_EQ, 10, 50, 20, 60, 20, 50, 20, 50);
    CCTest(ConditionCode::CC_NE, 10, 50, 20, 60, 10, 50, 20, 60);

    // case 2
    CCTest(ConditionCode::CC_GT, 10, 20, 50, 60, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_A, 10, 20, 50, 60, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_GE, 10, 20, 50, 60, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_AE, 10, 20, 50, 60, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_LT, 10, 20, 50, 60, 10, 20, 50, 60);
    CCTest(ConditionCode::CC_B, 10, 20, 50, 60, 10, 20, 50, 60);
    CCTest(ConditionCode::CC_LE, 10, 20, 50, 60, 10, 20, 50, 60);
    CCTest(ConditionCode::CC_BE, 10, 20, 50, 60, 10, 20, 50, 60);
    CCTest(ConditionCode::CC_EQ, 10, 20, 50, 60, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_NE, 10, 20, 50, 60, 10, 20, 50, 60);

    // case 3
    CCTest(ConditionCode::CC_GT, 10, 60, 20, 50, 21, 60, 20, 50);
    CCTest(ConditionCode::CC_A, 10, 60, 20, 50, 21, 60, 20, 50);
    CCTest(ConditionCode::CC_GE, 10, 60, 20, 50, 20, 60, 20, 50);
    CCTest(ConditionCode::CC_AE, 10, 60, 20, 50, 20, 60, 20, 50);
    CCTest(ConditionCode::CC_LT, 10, 60, 20, 50, 10, 49, 20, 50);
    CCTest(ConditionCode::CC_B, 10, 60, 20, 50, 10, 49, 20, 50);
    CCTest(ConditionCode::CC_LE, 10, 60, 20, 50, 10, 50, 20, 50);
    CCTest(ConditionCode::CC_BE, 10, 60, 20, 50, 10, 50, 20, 50);
    CCTest(ConditionCode::CC_EQ, 10, 60, 20, 50, 20, 50, 20, 50);
    CCTest(ConditionCode::CC_NE, 10, 60, 20, 50, 10, 60, 20, 50);

    // case 4
    CCTest(ConditionCode::CC_GT, 20, 60, 10, 50, 20, 60, 10, 50);
    CCTest(ConditionCode::CC_A, 20, 60, 10, 50, 20, 60, 10, 50);
    CCTest(ConditionCode::CC_GE, 20, 60, 10, 50, 20, 60, 10, 50);
    CCTest(ConditionCode::CC_AE, 20, 60, 10, 50, 20, 60, 10, 50);
    CCTest(ConditionCode::CC_LT, 20, 60, 10, 50, 20, 49, 21, 50);
    CCTest(ConditionCode::CC_B, 20, 60, 10, 50, 20, 49, 21, 50);
    CCTest(ConditionCode::CC_LE, 20, 60, 10, 50, 20, 50, 20, 50);
    CCTest(ConditionCode::CC_BE, 20, 60, 10, 50, 20, 50, 20, 50);
    CCTest(ConditionCode::CC_EQ, 20, 60, 10, 50, 20, 50, 20, 50);
    CCTest(ConditionCode::CC_NE, 20, 60, 10, 50, 20, 60, 10, 50);

    // case 5
    CCTest(ConditionCode::CC_GT, 50, 60, 10, 20, 50, 60, 10, 20);
    CCTest(ConditionCode::CC_A, 50, 60, 10, 20, 50, 60, 10, 20);
    CCTest(ConditionCode::CC_GE, 50, 60, 10, 20, 50, 60, 10, 20);
    CCTest(ConditionCode::CC_AE, 50, 60, 10, 20, 50, 60, 10, 20);
    CCTest(ConditionCode::CC_LT, 50, 60, 10, 20, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_B, 50, 60, 10, 20, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_LE, 50, 60, 10, 20, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_BE, 50, 60, 10, 20, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_EQ, 50, 60, 10, 20, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_NE, 50, 60, 10, 20, 50, 60, 10, 20);

    // case 6
    CCTest(ConditionCode::CC_GT, 20, 50, 10, 60, 20, 50, 10, 49);
    CCTest(ConditionCode::CC_A, 20, 50, 10, 60, 20, 50, 10, 49);
    CCTest(ConditionCode::CC_GE, 20, 50, 10, 60, 20, 50, 10, 50);
    CCTest(ConditionCode::CC_AE, 20, 50, 10, 60, 20, 50, 10, 50);
    CCTest(ConditionCode::CC_LT, 20, 50, 10, 60, 20, 50, 21, 60);
    CCTest(ConditionCode::CC_B, 20, 50, 10, 60, 20, 50, 21, 60);
    CCTest(ConditionCode::CC_LE, 20, 50, 10, 60, 20, 50, 20, 60);
    CCTest(ConditionCode::CC_BE, 20, 50, 10, 60, 20, 50, 20, 60);
    CCTest(ConditionCode::CC_EQ, 20, 50, 10, 60, 20, 50, 20, 50);
    CCTest(ConditionCode::CC_NE, 20, 50, 10, 60, 20, 50, 10, 60);
}

TEST_F(BoundsAnalysisTest, IntervalCollisions)
{
    // Bounds collision
    CCTest(ConditionCode::CC_GT, -10, -5, -5, 0, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_LT, -5, 0, -10, -5, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    // Single value interval collision
    CCTest(ConditionCode::CC_GT, 0, 20, 20, 20, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_LT, 0, 20, 0, 0, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_GT, 16, 16, 16, 32, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
    CCTest(ConditionCode::CC_LT, 32, 32, 16, 32, INT64_MIN, INT64_MAX, INT64_MIN, INT64_MAX);
}

TEST_F(BoundsAnalysisTest, UnionTest)
{
    ArenaVector<BoundsRange> ranges(GetGraph()->GetAllocator()->Adapter());
    ranges.emplace_back(BoundsRange(-10, 100));
    ranges.emplace_back(BoundsRange(-20, 15));
    auto range = BoundsRange::Union(ranges);
    ASSERT_EQ(range.GetLeft(), -20);
    ASSERT_EQ(range.GetRight(), 100);
    ranges.clear();

    ranges.emplace_back(BoundsRange(INT64_MIN, -10));
    ranges.emplace_back(BoundsRange(10, INT64_MAX));
    range = BoundsRange::Union(ranges);
    ASSERT_EQ(range.GetLeft(), INT64_MIN);
    ASSERT_EQ(range.GetRight(), INT64_MAX);
}

TEST_F(BoundsAnalysisTest, TypeFitting)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u16();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).u32();
        CONSTANT(3, 0x23).s64();
        CONSTANT(4, 0x30).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            // v0 -- ANYTHING
            // v1 -- ANYTHING
            // v2 -- ANYTHING
            INST(5, Opcode::Compare).b().CC(CC_GT).Inputs(1, 3);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3, 5)
        {
            // v0 -- ANYTHING
            // v1 <= 0x23
            // v2 -- ANYTHING
            INST(8, Opcode::Compare).b().CC(CC_GT).Inputs(2, 4);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(5, -1)
        {
            // v0 -- ANYTHING
            // v1 <= 0x23
            // v2 <= 0x30
            INST(11, Opcode::Return).u16().Inputs(0);
        }
        BASIC_BLOCK(3, -1)
        {
            // v0 -- ANYTHING
            // v1 -- ANYTHING
            // v2 -- ANYTHING
            INST(14, Opcode::ReturnI).u16().Imm(0x23);
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    BoundsRange range;

    // BB2
    range = rinfo->FindBoundsRange(&BB(2), &INS(0));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), UINT16_MAX);

    range = rinfo->FindBoundsRange(&BB(2), &INS(1));
    EXPECT_EQ(range.GetLeft(), INT64_MIN);
    EXPECT_EQ(range.GetRight(), INT64_MAX);

    range = rinfo->FindBoundsRange(&BB(2), &INS(2));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), UINT32_MAX);

    // BB4
    range = rinfo->FindBoundsRange(&BB(2), &INS(0));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), UINT16_MAX);

    range = rinfo->FindBoundsRange(&BB(4), &INS(1));
    EXPECT_EQ(range.GetLeft(), INT64_MIN);
    EXPECT_EQ(range.GetRight(), 0x23);

    range = rinfo->FindBoundsRange(&BB(2), &INS(2));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), UINT32_MAX);

    // BB5
    range = rinfo->FindBoundsRange(&BB(2), &INS(0));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), UINT16_MAX);

    range = rinfo->FindBoundsRange(&BB(4), &INS(1));
    EXPECT_EQ(range.GetLeft(), INT64_MIN);
    EXPECT_EQ(range.GetRight(), 0x23);

    range = rinfo->FindBoundsRange(&BB(5), &INS(2));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), 0x30);

    // BB3
    range = rinfo->FindBoundsRange(&BB(2), &INS(0));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), UINT16_MAX);

    range = rinfo->FindBoundsRange(&BB(2), &INS(1));
    EXPECT_EQ(range.GetLeft(), INT64_MIN);
    EXPECT_EQ(range.GetRight(), INT64_MAX);

    range = rinfo->FindBoundsRange(&BB(2), &INS(2));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), UINT32_MAX);
}

TEST_F(BoundsAnalysisTest, NullCompare)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(3, nullptr);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().CC(CC_NE).Inputs(3, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(7, Opcode::SaveState).Inputs(0, 4).SrcVregs({2, 3});
            INST(8, Opcode::NullCheck).ref().Inputs(0, 7);
            INST(9, Opcode::LoadObject).s64().Inputs(8).TypeId(243);
            INST(10, Opcode::Return).s64().Inputs(9);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(13, Opcode::ReturnI).s64().Imm(0x14);
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    EXPECT_EQ(rinfo->FindBoundsRange(&BB(2), &INS(0)).GetLeft(), 0);
    EXPECT_EQ(rinfo->FindBoundsRange(&BB(2), &INS(0)).GetRight(), BoundsRange::GetMax(INS(0).GetType()));

    EXPECT_NE(rinfo->FindBoundsRange(&BB(3), &INS(0)).GetLeft(), 0);
    EXPECT_EQ(rinfo->FindBoundsRange(&BB(3), &INS(0)).GetRight(), BoundsRange::GetMax(INS(0).GetType()));

    EXPECT_EQ(rinfo->FindBoundsRange(&BB(4), &INS(0)).GetLeft(), 0);
    EXPECT_EQ(rinfo->FindBoundsRange(&BB(4), &INS(0)).GetRight(), 0);
}

TEST_F(BoundsAnalysisTest, InitMoreThenTest)
{
    // for (int i = 10, i < 0, i++) {}
    // this loop is counable, but init > test value.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        CONSTANT(1, 1);
        CONSTANT(2, 0);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);

            INST(10, Opcode::Add).s32().Inputs(4, 1);
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 2);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    auto range = rinfo->FindBoundsRange(&BB(3), &INS(4));
    EXPECT_EQ(range.GetLeft(), INT32_MIN);
    EXPECT_EQ(range.GetRight(), INT32_MAX);
}

TEST_F(BoundsAnalysisTest, ModTest)
{
    auto res = BoundsRange(0, 10).Mod(BoundsRange(0, 2));
    EXPECT_EQ(res.GetLeft(), 0);
    EXPECT_EQ(res.GetRight(), 1);

    res = BoundsRange(0, 10).Mod(BoundsRange(0));
    EXPECT_EQ(res.GetLeft(), INT64_MIN);
    EXPECT_EQ(res.GetRight(), INT64_MAX);

    // It's correct situation, when right value is 0, and left value isn't 0.
    res = BoundsRange(-179, -179).Mod(BoundsRange(INT64_MIN, 0));
    EXPECT_EQ(res.GetLeft(), -179);
    EXPECT_EQ(res.GetRight(), 0);

    res = BoundsRange(5, 10).Mod(BoundsRange(-3));
    EXPECT_EQ(res.GetLeft(), 0);
    EXPECT_EQ(res.GetRight(), 2);

    res = BoundsRange(-10, -5).Mod(BoundsRange(-3));
    EXPECT_EQ(res.GetLeft(), -2);
    EXPECT_EQ(res.GetRight(), 0);

    res = BoundsRange(-10, 10).Mod(BoundsRange(3));
    EXPECT_EQ(res.GetLeft(), -2);
    EXPECT_EQ(res.GetRight(), 2);

    res = BoundsRange(-3, 3).Mod(BoundsRange(-10, 10));
    EXPECT_EQ(res.GetLeft(), -3);
    EXPECT_EQ(res.GetRight(), 3);
}

TEST_F(BoundsAnalysisTest, LoopWithBigStep)
{
    // for (int i = 0, i < 5, i += 10) {}
    // this loop is countable, and init + step > test value.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 5);
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);

            INST(10, Opcode::Add).s32().Inputs(4, 2);
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 1);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    auto range = rinfo->FindBoundsRange(&BB(3), &INS(4));
    EXPECT_EQ(range.GetLeft(), 0);
    EXPECT_EQ(range.GetRight(), 4);
}

TEST_F(BoundsAnalysisTest, LoopWithBigStep2)
{
    // for (int i = 1, i < 6, i += 2) {}
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 6);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);

            INST(10, Opcode::Add).s32().Inputs(4, 2);
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 1);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }
    auto rinfo = GetGraph()->GetBoundsRangeInfo();
    auto range = rinfo->FindBoundsRange(&BB(3), &INS(4));
    EXPECT_EQ(range.GetLeft(), 0);
    // Check that right bound is (upper - 1)
    EXPECT_EQ(range.GetRight(), 5);
}

TEST_F(BoundsAnalysisTest, ShrTest)
{
    auto r1 = BoundsRange(2);
    auto max = BoundsRange();
    auto r2 = BoundsRange(4, 8);
    auto neg = BoundsRange(-1);

    EXPECT_EQ(r2.Shr(r2), max);
    EXPECT_EQ(r2.Shr(neg), max);
    EXPECT_EQ(r2.Shr(r1), BoundsRange(1, 2));

    EXPECT_EQ(BoundsRange(-1).Shr(BoundsRange(4)), BoundsRange(0x0FFFFFFFFFFFFFFF));
    EXPECT_EQ(BoundsRange(-1, 16).Shr(BoundsRange(4)), BoundsRange(0, 0x0FFFFFFFFFFFFFFF));
    EXPECT_EQ(BoundsRange(9, 15).Shr(BoundsRange(2)), BoundsRange(2, 3));
    EXPECT_EQ(BoundsRange(-15, -9).Shr(BoundsRange(2)), BoundsRange(0x3FFFFFFFFFFFFFFC, 0x3FFFFFFFFFFFFFFD));
    EXPECT_EQ(BoundsRange(-100, 100).Shr(BoundsRange(4)), BoundsRange(0, 0x0FFFFFFFFFFFFFFF));
}

TEST_F(BoundsAnalysisTest, AShrTest)
{
    auto r1 = BoundsRange(2);
    auto max = BoundsRange();
    auto r2 = BoundsRange(4, 8);
    auto neg = BoundsRange(-1);

    EXPECT_EQ(r2.AShr(r2), max);
    EXPECT_EQ(r2.AShr(neg), max);
    EXPECT_EQ(r2.AShr(r1), BoundsRange(1, 2));

    EXPECT_EQ(BoundsRange(-1).AShr(BoundsRange(4)), BoundsRange(-1));
    EXPECT_EQ(BoundsRange(-1, 16).AShr(BoundsRange(4)), BoundsRange(-1, 1));
}

TEST_F(BoundsAnalysisTest, ShlTest)
{
    auto max = BoundsRange();
    auto r = BoundsRange(4, 8);
    auto neg = BoundsRange(-1);

    EXPECT_EQ(r.Shl(r), max);
    EXPECT_EQ(r.Shl(neg), max);

    EXPECT_EQ(r.Shl(BoundsRange(2)), BoundsRange(16, 32));
    EXPECT_EQ(BoundsRange(-1).Shl(BoundsRange(4)), BoundsRange(0xFFFFFFFFFFFFFFF0));
    EXPECT_EQ(BoundsRange(-1, 16).Shl(BoundsRange(4)), BoundsRange(0xFFFFFFFFFFFFFFF0, 256));
    EXPECT_EQ(BoundsRange(16, 0x0FFFFFFFFFFFFFFF).Shl(BoundsRange(4)), BoundsRange());
}

TEST_F(BoundsAnalysisTest, AShrDivTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 4);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Div).s32().Inputs(0, 1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto bb = &BB(2);

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 31);
        CONSTANT(5, 30);  // type size - log2(4)
        CONSTANT(8, 2);   // log2(4)
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::AShr).s32().Inputs(0, 1);
            INST(4, Opcode::Shr).s32().Inputs(2, 5);
            INST(6, Opcode::Add).s32().Inputs(4, 0);
            INST(7, Opcode::AShr).s32().Inputs(6, 8);
            INST(3, Opcode::Return).s32().Inputs(7);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    auto bri = GetGraph()->GetBoundsRangeInfo();
    auto range = bri->FindBoundsRange(bb, bb->GetLastInst()->GetInput(0).GetInst());
    ASSERT_EQ(range, BoundsRange(INT32_MIN / 4, INT32_MAX / 4));
}

TEST_F(BoundsAnalysisTest, AndTest)
{
    EXPECT_EQ(BoundsRange().And(BoundsRange(1, 2)), BoundsRange());
    EXPECT_EQ(BoundsRange().And(BoundsRange(0x2)), BoundsRange(0, 0x2));
    EXPECT_EQ(BoundsRange().And(BoundsRange(0x3)), BoundsRange(0, 0x3));
    EXPECT_EQ(BoundsRange().And(BoundsRange(-1)), BoundsRange());
    EXPECT_EQ(BoundsRange().And(BoundsRange(0x8000000000000000)), BoundsRange());
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
