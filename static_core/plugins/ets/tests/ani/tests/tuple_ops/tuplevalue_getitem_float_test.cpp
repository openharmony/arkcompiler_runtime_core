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

class TupleValueGetItemFloatTest : public AniGTestTupleOps {
public:
    static constexpr ani_float EXPECTED_RESULT = 3.14F;
};

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Float(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatNullTuple)
{
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_float result = 0.0F;
        ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing