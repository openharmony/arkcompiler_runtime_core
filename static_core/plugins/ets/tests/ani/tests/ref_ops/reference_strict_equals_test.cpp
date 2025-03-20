/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ReferenceStrictEqualsTest : public AniTest {};

TEST_F(ReferenceStrictEqualsTest, check_null_and_null)
{
    auto nullRef1 = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetNull");
    auto nullRef2 = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetNull");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(nullRef1, nullRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceStrictEqualsTest, check_null_and_undefined)
{
    auto nullRef = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetNull");
    auto undefinedRef = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetUndefined");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(nullRef, undefinedRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceStrictEqualsTest, check_null_and_object)
{
    auto nullRef = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetNull");
    auto objectRef = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(nullRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceStrictEqualsTest, check_undefined_and_undefined)
{
    auto undefinedRef1 = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetUndefined");
    auto undefinedRef2 = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetUndefined");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(undefinedRef1, undefinedRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceStrictEqualsTest, check_undefined_and_object)
{
    auto undefinedRef = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetUndefined");
    auto objectRef = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(undefinedRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceStrictEqualsTest, check_object_and_object)
{
    auto objectRef1 = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetObject");
    auto objectRef2 = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_StrictEquals(objectRef1, objectRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceStrictEqualsTest, invalid_argument)
{
    auto ref = CallEtsFunction<ani_ref>("reference_strict_equals_test", "GetNull");
    ASSERT_EQ(env_->Reference_StrictEquals(ref, ref, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ReferenceStrictEqualsTest, invalid_env)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Reference_StrictEquals(nullptr, undefinedRef, undefinedRef, &isEquals), ANI_INVALID_ARGS);
}

TEST_F(ReferenceStrictEqualsTest, invalid_result)
{
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ASSERT_EQ(env_->Reference_StrictEquals(undefinedRef, undefinedRef, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
