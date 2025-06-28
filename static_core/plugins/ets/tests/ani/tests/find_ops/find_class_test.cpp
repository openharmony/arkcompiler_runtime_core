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

class FindClassTest : public AniTest {};

TEST_F(FindClassTest, has_class)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lfind_class_test/Point;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
}

TEST_F(FindClassTest, invalid_arguments)
{
    ani_class cls {};

    ASSERT_EQ(env_->FindClass("find_class_test/bla-bla-bla", &cls), ANI_NOT_FOUND);
    ASSERT_EQ(env_->FindClass(nullptr, &cls), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->FindClass("Lfind_class_test/Point;", nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->c_api->FindClass(nullptr, "LPoint;", &cls), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->FindClass("", &cls), ANI_NOT_FOUND);
    ASSERT_EQ(env_->FindClass("\t", &cls), ANI_NOT_FOUND);
}

TEST_F(FindClassTest, class_is_not_namespace)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lfind_class_test/Point;", &ns), ANI_NOT_FOUND);
}

TEST_F(FindClassTest, class_is_not_module)
{
    ani_module md {};
    ASSERT_EQ(env_->FindModule("LPoint;", &md), ANI_NOT_FOUND);
}

TEST_F(FindClassTest, check_initialization)
{
    ASSERT_FALSE(IsRuntimeClassInitialized("find_class_test.Point"));
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("find_class_test.Point", &cls), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("find_class_test.Point"));
}

}  // namespace ark::ets::ani::testing
