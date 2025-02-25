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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassCallStaticMethodByNameTest : public AniTest {
public:
    void GetClassData(ani_class *clsResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
};

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    char c = 'C' - 'A';
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Char(env_, cls, "sub", "CC:C", &value, 'A', 'C'), ANI_OK);
    ASSERT_EQ(value, c);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_v)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    char c = 'C' - 'A';
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char(cls, "sub", "CC:C", &value, 'A', 'C'), ANI_OK);
    ASSERT_EQ(value, c);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_A)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char_A(cls, "sub", "CC:C", &value, args), ANI_OK);
    ASSERT_EQ(value, args[1U].c - args[0U].c);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_error_signature)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Char(env_, cls, "sub", "CC:I", &value, 'A', 'C'),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_null_class)
{
    ani_char value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Char(env_, nullptr, "sub", nullptr, &value, 'A', 'C'),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Char(env_, cls, nullptr, nullptr, &value, 'A', 'C'),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Char(env_, cls, "aa", nullptr, &value, 'A', 'C'),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Char(env_, cls, "sub", nullptr, nullptr, 'A', 'C'),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_v_error_signature)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char(cls, "sub", "CC:I", &value, 'A', 'C'), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_v_null_class)
{
    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char(nullptr, "sub", nullptr, &value, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_v_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char(cls, nullptr, nullptr, &value, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_v_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char(cls, "aa", nullptr, &value, 'A', 'C'), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_v_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char(cls, "sub", nullptr, nullptr, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_A_error_signature)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char_A(cls, "sub", "CC:I", &value, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_A_null_class)
{
    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';
    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char_A(nullptr, "sub", nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_A_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';
    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char_A(cls, nullptr, nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_A_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';
    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char_A(cls, "aa", nullptr, &value, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_A_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char_A(cls, "sub", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_char_A_null_args)
{
    ani_class cls;
    GetClassData(&cls);

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Char_A(cls, "sub", nullptr, &value, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
