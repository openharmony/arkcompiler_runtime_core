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

class TupleValueGetItemBooleanTest : public AniGTestTupleOps {};

TEST_F(TupleValueGetItemBooleanTest, tupleValueGetItemBoolean)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_boolean_test", "getBooleanTuple");
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->TupleValue_GetItem_Boolean(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(TupleValueGetItemBooleanTest, tupleValueGetItemBooleanNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_boolean_test", "getBooleanTuple");
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Boolean(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemBooleanTest, tupleValueGetItemBooleanNullTuple)
{
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->TupleValue_GetItem_Boolean(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemBooleanTest, tupleValueGetItemBooleanIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_boolean_test", "getBooleanTuple");
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->TupleValue_GetItem_Boolean(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemBooleanTest, tupleValueGetItemBooleanIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_boolean_test", "getBooleanTuple");
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->TupleValue_GetItem_Boolean(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemBooleanTest, tupleValueGetItemBooleanNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_boolean_test", "getBooleanTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Boolean(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemBooleanTest, tupleValueGetItemBooleanRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_boolean_test", "getBooleanTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_boolean result = ANI_FALSE;
        ASSERT_EQ(env_->TupleValue_GetItem_Boolean(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
    }
}
}  // namespace ark::ets::ani::testing