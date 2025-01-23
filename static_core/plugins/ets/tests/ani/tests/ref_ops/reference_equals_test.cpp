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

class ReferenceEqualsTest : public AniTest {};

TEST_F(ReferenceEqualsTest, check_null_and_null)
{
    auto nullRef1 = CallEtsFunction<ani_ref>("GetNull");
    auto nullRef2 = CallEtsFunction<ani_ref>("GetNull");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_Equals(nullRef1, nullRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, check_null_and_undefined)
{
    auto nullRef = CallEtsFunction<ani_ref>("GetNull");
    auto undefinedRef = CallEtsFunction<ani_ref>("GetUndefined");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_Equals(nullRef, undefinedRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, check_null_and_object)
{
    auto nullRef = CallEtsFunction<ani_ref>("GetNull");
    auto objectRef = CallEtsFunction<ani_ref>("GetObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_Equals(nullRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, check_undefined_and_undefined)
{
    auto undefinedRef1 = CallEtsFunction<ani_ref>("GetUndefined");
    auto undefinedRef2 = CallEtsFunction<ani_ref>("GetUndefined");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_Equals(undefinedRef1, undefinedRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, check_undefined_and_object)
{
    auto undefinedRef = CallEtsFunction<ani_ref>("GetUndefined");
    auto objectRef = CallEtsFunction<ani_ref>("GetObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_Equals(undefinedRef, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}

TEST_F(ReferenceEqualsTest, check_object_and_object)
{
    auto objectRef1 = CallEtsFunction<ani_ref>("GetObject");
    auto objectRef2 = CallEtsFunction<ani_ref>("GetObject");
    ani_boolean isEquals;
    ASSERT_EQ(env_->Reference_Equals(objectRef1, objectRef2, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
}

TEST_F(ReferenceEqualsTest, invalid_argument)
{
    auto ref = CallEtsFunction<ani_ref>("GetNull");
    ASSERT_EQ(env_->Reference_Equals(ref, ref, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
