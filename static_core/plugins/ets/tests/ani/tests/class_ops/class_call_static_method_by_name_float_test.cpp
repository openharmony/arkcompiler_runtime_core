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

class ClassCallStaticMethodByNameFloatTest : public AniTest {
public:
    static constexpr ani_float FLOAT_VAL1 = 1.5F;
    static constexpr ani_float FLOAT_VAL2 = 2.5F;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
};

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_float sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Float(env_, cls, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_v)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_float sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_class)
{
    ani_float sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(nullptr, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_name)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_float sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, nullptr, nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum_not_exist", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_result)
{
    ani_class cls;
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum", nullptr, nullptr, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_class)
{
    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(nullptr, "sum", nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_name)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, nullptr, nullptr, &sum, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum_not_exist", nullptr, &sum, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_result)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_args)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_float sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, &sum, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
