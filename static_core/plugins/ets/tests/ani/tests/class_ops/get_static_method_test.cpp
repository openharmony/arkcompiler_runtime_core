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

class GetStaticMethodTest : public AniTest {};

TEST_F(GetStaticMethodTest, has_static_method_1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPhone;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method;
    ASSERT_EQ(env_->Class_GetStaticMethod(cls, "get_button_names", ":[Lstd/core/String;", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(GetStaticMethodTest, has_static_method_2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPhone;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method;
    ASSERT_EQ(env_->Class_GetStaticMethod(cls, "get_button_names", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(GetStaticMethodTest, static_method_not_found_1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPhone;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method;
    ASSERT_EQ(env_->Class_GetStaticMethod(cls, "bla_bla_bla", nullptr, &method), ANI_NOT_FOUND);
}

TEST_F(GetStaticMethodTest, static_method_not_found_2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPhone;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method;
    ASSERT_EQ(env_->Class_GetStaticMethod(cls, "get_button_names", "bla_bla_bla", &method), ANI_NOT_FOUND);
}

TEST_F(GetStaticMethodTest, invalid_argument_name)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPhone;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method;
    ASSERT_EQ(env_->Class_GetStaticMethod(cls, nullptr, nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(GetStaticMethodTest, invalid_argument_cls)
{
    ani_static_method method;
    ASSERT_EQ(env_->Class_GetStaticMethod(nullptr, "get_button_names", nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(GetStaticMethodTest, invalid_argument_result)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LPhone;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ASSERT_EQ(env_->Class_GetStaticMethod(cls, "get_button_names", nullptr, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
