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

class ClassCallStaticMethodByNameShortTest : public AniTest {
public:
    void GetClassData(ani_class *clsResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
};

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short)
{
    ani_class cls;
    GetClassData(&cls);

    ani_short value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "sum", nullptr, &value, 5U, 6U), ANI_OK);
    ASSERT_EQ(value, 5U + 6U);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v)
{
    ani_class cls;
    GetClassData(&cls);

    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "sum", nullptr, &value, 5U, 6U), ANI_OK);
    ASSERT_EQ(value, 5U + 6U);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = 5U;
    args[1U].s = 6U;

    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, &value, args), ANI_OK);
    ASSERT_EQ(value, 5U + 6U);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_class)
{
    ani_short value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, nullptr, "sum", nullptr, &value, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_short value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, nullptr, nullptr, &value, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_short value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "aa", nullptr, &value, 5U, 6U), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "sum", nullptr, nullptr, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_class)
{
    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(nullptr, "sum", nullptr, &value, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, nullptr, nullptr, &value, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "aa", nullptr, &value, 5U, 6U), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "sum", nullptr, nullptr, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_class)
{
    ani_value args[2U];
    args[0U].s = 5U;
    args[1U].s = 6U;
    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(nullptr, "sum", nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = 5U;
    args[1U].s = 6U;
    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, nullptr, nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = 5U;
    args[1U].s = 6U;
    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "aa", nullptr, &value, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = 5U;
    args[1U].s = 6U;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_args)
{
    ani_class cls;
    GetClassData(&cls);

    ani_short value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, &value, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
