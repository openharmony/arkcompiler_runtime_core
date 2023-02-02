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
#include "optimizer/code_generator/spill_fill_encoder.h"

namespace panda::compiler {
class SpillFillEncoderTest : public GraphTest {
};

bool operator==(const SpillFillData &left, const SpillFillData &right)
{
    return left.SrcType() == right.SrcType() && left.SrcValue() == right.SrcValue() &&
           left.DstType() == right.DstType() && left.DstValue() == right.DstValue() &&
           left.GetType() == right.GetType();
}

TEST_F(SpillFillEncoderTest, SpillFillDataSorting)
{
    ArenaVector<SpillFillData> spill_fills {
        {{LocationType::REGISTER, LocationType::STACK, 1, 0, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::STACK, 0, 2, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::REGISTER, 0, 1, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::REGISTER, 1, 2, DataType::Type::INT64},
         {LocationType::IMMEDIATE, LocationType::REGISTER, 0, 0, DataType::Type::INT64},
         {LocationType::IMMEDIATE, LocationType::REGISTER, 0, 1, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 0, 0, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 1, 1, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 2, 2, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::REGISTER, 2, 4, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 3, 2, DataType::Type::INT64},
         {LocationType::STACK, LocationType::STACK, 7, 9, DataType::Type::INT64},
         {LocationType::STACK, LocationType::STACK, 8, 10, DataType::Type::INT64}},
        GetAllocator()->Adapter()};

    ArenaVector<SpillFillData> expected_order {
        {  // reorder spills
         {LocationType::REGISTER, LocationType::STACK, 0, 2, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::STACK, 1, 0, DataType::Type::INT64},
         // skip move
         {LocationType::REGISTER, LocationType::REGISTER, 0, 1, DataType::Type::INT64},
         {LocationType::REGISTER, LocationType::REGISTER, 1, 2, DataType::Type::INT64},
         // skip imm move
         {LocationType::IMMEDIATE, LocationType::REGISTER, 0, 0, DataType::Type::INT64},
         {LocationType::IMMEDIATE, LocationType::REGISTER, 0, 1, DataType::Type::INT64},
         // reorder fills
         {LocationType::STACK, LocationType::REGISTER, 2, 2, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 1, 1, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 0, 0, DataType::Type::INT64},
         // skip move
         {LocationType::REGISTER, LocationType::REGISTER, 2, 4, DataType::Type::INT64},
         {LocationType::STACK, LocationType::REGISTER, 3, 2, DataType::Type::INT64},
         // skip mem move
         {LocationType::STACK, LocationType::STACK, 7, 9, DataType::Type::INT64},
         {LocationType::STACK, LocationType::STACK, 8, 10, DataType::Type::INT64}},
        GetAllocator()->Adapter()};

    SpillFillEncoder::SortSpillFillData(&spill_fills);
    for (size_t i = 0; i < expected_order.size(); i++) {
        EXPECT_EQ(spill_fills[i], expected_order[i]) << "Mismatch at " << i;
    }
}

TEST_F(SpillFillEncoderTest, CanCombineSpillFills)
{
    auto graph = GetGraph();
    if (graph->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "Supported only on Aarch64";
    }

    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2, 0, DataType::Type::INT64},
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::INT64}, graph));

    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::REGISTER, LocationType::STACK, 0, 2, DataType::Type::INT64},
        {LocationType::REGISTER, LocationType::STACK, 0, 1, DataType::Type::INT64}, graph));

    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2, 0, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::INT64}, graph));

    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2, 0, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::INT8}, graph));

    // different type of moves
    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::REGISTER, LocationType::STACK, 2, 0, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::INT32}, graph));

    // illegal slots order
    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 2, 0, DataType::Type::INT32}, graph));

    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 4, 0, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 0, 0, DataType::Type::INT32}, graph));

    // unaligned access
    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::INT32},
        {LocationType::STACK, LocationType::REGISTER, 0, 0, DataType::Type::INT32}, graph));

    // float 32 are unsupported
    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2, 0, DataType::Type::FLOAT32},
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::FLOAT32}, graph));

    // float 64 are supported
    EXPECT_TRUE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2, 0, DataType::Type::FLOAT64},
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::FLOAT64}, graph));

    // different types
    EXPECT_FALSE(SpillFillEncoder::CanCombineSpillFills(
        {LocationType::STACK, LocationType::REGISTER, 2, 0, DataType::Type::INT64},
        {LocationType::STACK, LocationType::REGISTER, 1, 0, DataType::Type::FLOAT64}, graph));
}
}  // namespace panda::compiler
