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

class ReferenceIsNullishTest : public AniTest {};

TEST_F(ReferenceIsNullishTest, check_null)
{
    auto ref = CallEtsFunction<ani_ref>("GetNull");
    ani_boolean isUndefined;
    ASSERT_EQ(env_->Reference_IsNullish(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(ReferenceIsNullishTest, check_undefined)
{
    auto ref = CallEtsFunction<ani_ref>("GetUndefined");
    ani_boolean isUndefined;
    ASSERT_EQ(env_->Reference_IsNullish(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

TEST_F(ReferenceIsNullishTest, check_object)
{
    auto ref = CallEtsFunction<ani_ref>("GetObject");
    ani_boolean isUndefined;
    ASSERT_EQ(env_->Reference_IsNullish(ref, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_FALSE);
}

TEST_F(ReferenceIsNullishTest, invalid_argument)
{
    auto ref = CallEtsFunction<ani_ref>("GetNull");
    ASSERT_EQ(env_->Reference_IsNullish(ref, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
