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

class ClassCallStaticMethodByNameIntTest : public AniTest {
public:
    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
};

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, cls, "sum", nullptr, &sum, 5U, 6U), ANI_OK);
    ASSERT_EQ(sum, 5U + 6U);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_v)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "sum", nullptr, &sum, 5U, 6U), ANI_OK);
    ASSERT_EQ(sum, 5U + 6U);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_A)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "sum", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, 5U + 6U);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_invalid_cls)
{
    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, nullptr, "sum", nullptr, &sum, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_invalid_name)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, cls, nullptr, nullptr, &sum, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_invalid_result)
{
    ani_class cls;
    GetMethodData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, cls, "sum", nullptr, nullptr, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_v_invalid_cls)
{
    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(nullptr, "sum", nullptr, &sum, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_v_invalid_name)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, nullptr, nullptr, &sum, 5U, 6U), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "sum_not_exist", nullptr, &sum, 5U, 6U), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_v_invalid_result)
{
    ani_class cls;
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "sum", nullptr, nullptr, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_a_invalid_cls)
{
    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;
    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(nullptr, "sum", nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_a_invalid_name)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;
    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, nullptr, nullptr, &sum, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "sum_not_exist", nullptr, &sum, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_a_invalid_result)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_a_invalid_args)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "sum", nullptr, &sum, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
