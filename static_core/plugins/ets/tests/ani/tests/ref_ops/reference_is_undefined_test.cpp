/**
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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ReferenceIsUndefinedTest : public AniTest {
public:
    static constexpr int32_t LOOP_COUNT = 3;
};

TEST_F(ReferenceIsUndefinedTest, check_null)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_undefined_test", "GetNull");
    ani_boolean isUndefined = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_FALSE);
}

TEST_F(ReferenceIsUndefinedTest, check_undefined)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_undefined_test", "GetUndefined");
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(ReferenceIsUndefinedTest, check_object)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_undefined_test", "GetObject");
    ani_boolean isUndefined = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_FALSE);
}

TEST_F(ReferenceIsUndefinedTest, invalid_argument)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_undefined_test", "GetNull");
    ASSERT_EQ(env_->Reference_IsUndefined(ref, nullptr), ANI_INVALID_ARGS);
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Reference_IsUndefined(nullptr, ref, &isUndefined), ANI_INVALID_ARGS);
}

TEST_F(ReferenceIsUndefinedTest, check_undefined_loop)
{
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        auto ref = CallEtsFunction<ani_ref>("reference_is_undefined_test", "GetUndefined");
        ani_boolean isUndefined = ANI_FALSE;
        ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
        ASSERT_EQ(isUndefined, ANI_TRUE);
    }
}

TEST_F(ReferenceIsUndefinedTest, mix_test)
{
    auto ref = CallEtsFunction<ani_ref>("reference_is_undefined_test", "GetUndefined");
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);

    auto objectRef = CallEtsFunction<ani_ref>("reference_is_undefined_test", "GetObject");
    ASSERT_EQ(env_->Reference_IsUndefined(objectRef, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_FALSE);

    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_Equals(ref, objectRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);
}
}  // namespace ark::ets::ani::testing
