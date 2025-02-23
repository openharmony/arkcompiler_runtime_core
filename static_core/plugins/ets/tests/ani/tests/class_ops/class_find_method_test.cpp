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
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassFindMethodTest : public AniTest {};

TEST_F(ClassFindMethodTest, has_method_1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", "II:I", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindMethodTest, has_method_2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindMethodTest, has_method_3)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LB;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindMethodTest, has_method_4)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LB;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_override_method", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_static_method newMethod;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_B", ":LB;", &newMethod), ANI_OK);
    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

    ani_int res;
    ani_int arg1 = 2U;
    ani_int arg2 = 3U;
    ASSERT_EQ(env_->Object_CallMethod_Int(static_cast<ani_object>(ref), method, &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, arg1 * arg2);
}

TEST_F(ClassFindMethodTest, method_not_found_1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "bla_bla_bla", nullptr, &method), ANI_NOT_FOUND);
}

TEST_F(ClassFindMethodTest, method_not_found_2)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", "bla_bla_bla", &method), ANI_NOT_FOUND);
}

TEST_F(ClassFindMethodTest, invalid_argument_name)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass(nullptr, &cls), ANI_INVALID_ARGS);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, nullptr, nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(ClassFindMethodTest, invalid_argument_cls)
{
    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(nullptr, "int_method", nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(ClassFindMethodTest, invalid_argument_result)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", nullptr, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
