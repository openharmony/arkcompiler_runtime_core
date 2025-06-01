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

class TupleValueGetItemIntTest : public AniGTestTupleOps {
public:
    static constexpr ani_int EXPECTED_RESULT = 300;
};

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemInt)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Int(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntNullTuple)
{
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int result = 0U;
        ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing