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
#include <climits>

namespace ark::ets::ani::testing {

class VariableSetValueFloatTest : public AniTest {};

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
TEST_F(VariableSetValueFloatTest, set_float_value_normal)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "floatValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkFloatValue", 3.14F), ANI_TRUE);

    ani_float value = 6.28F;
    ASSERT_EQ(env_->Variable_SetValue_Float(variable, value), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkFloatValue", value), ANI_TRUE);
}

TEST_F(VariableSetValueFloatTest, set_float_value_normal_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "floatValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_float maxValue = std::numeric_limits<float>::max();
    ASSERT_EQ(env_->Variable_SetValue_Float(variable, maxValue), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkFloatValue", maxValue), ANI_TRUE);

    ani_float minValue = std::numeric_limits<float>::min();
    ASSERT_EQ(env_->Variable_SetValue_Float(variable, minValue), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkFloatValue", minValue), ANI_TRUE);
}

TEST_F(VariableSetValueFloatTest, set_float_value_invalid_args_variable_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "intValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_float value = 6.28F;
    ASSERT_EQ(env_->Variable_SetValue_Float(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueFloatTest, set_float_value_invalid_args_variable_2)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_float value = 6.28F;
    ASSERT_EQ(env_->Variable_SetValue_Float(nullptr, value), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)