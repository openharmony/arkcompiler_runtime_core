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

class TupleValueGetItemDoubleTest : public AniGTestTupleOps {
public:
    static constexpr ani_double EXPECTED_RESULT = 3.14;
};

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDouble)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Double(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleNullTuple)
{
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_double result = 0.0;
        ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing