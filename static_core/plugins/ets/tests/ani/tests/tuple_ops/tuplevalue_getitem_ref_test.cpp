/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest_tuple_ops.h"

namespace ark::ets::ani::testing {

class TupleValueGetItemRefTest : public AniGTestTupleOps {};

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRef)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);
    constexpr std::string_view EXPECTED_ELEM("Hello");
    auto internalStr = reinterpret_cast<ani_string>(result);
    std::array<char, EXPECTED_ELEM.size() + 1> elem {};
    ani_size copiedSize = 0;
    ASSERT_EQ(env_->String_GetUTF8SubString(internalStr, 0U, elem.size() - 1, elem.data(), elem.size(), &copiedSize),
              ANI_OK);
    ASSERT_EQ(copiedSize, EXPECTED_ELEM.size());
    ASSERT_STREQ(elem.data(), EXPECTED_ELEM.data());
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Ref(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefNullTuple)
{
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 4U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemRefTest, tupleValueGetItemRefRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_ref_test", "getReferenceTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_ref result {};
        ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);
        constexpr std::string_view EXPECTED_ELEM("Hello");
        auto internalStr = reinterpret_cast<ani_string>(result);
        std::array<char, EXPECTED_ELEM.size() + 1> elem {};
        ani_size copiedSize = 0;
        ASSERT_EQ(
            env_->String_GetUTF8SubString(internalStr, 0U, elem.size() - 1, elem.data(), elem.size(), &copiedSize),
            ANI_OK);
        ASSERT_EQ(copiedSize, EXPECTED_ELEM.size());
        ASSERT_STREQ(elem.data(), EXPECTED_ELEM.data());
    }
}
}  // namespace ark::ets::ani::testing
