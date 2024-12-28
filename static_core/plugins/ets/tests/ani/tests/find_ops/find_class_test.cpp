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

class FindClassTest : public AniTest {};

TEST_F(FindClassTest, has_class)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPoint;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
}

TEST_F(FindClassTest, class_not_found)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("bla-bla-bla", &cls), ANI_NOT_FOUND);
}

TEST_F(FindClassTest, invalid_argument_1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass(nullptr, &cls), ANI_INVALID_ARGS);
}

TEST_F(FindClassTest, invalid_argument_2)
{
    ASSERT_EQ(env_->FindClass("LPoint;", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
