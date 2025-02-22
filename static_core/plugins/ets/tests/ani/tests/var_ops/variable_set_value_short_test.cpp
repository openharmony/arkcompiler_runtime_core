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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class VariableSetValueShortTest : public AniTest {};

TEST_F(VariableSetValueShortTest, set_short_value_normal)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "shortValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkshortValue", 3U), ANI_TRUE);

    ani_short value = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Short(variable, value), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkshortValue", value), ANI_TRUE);
}

TEST_F(VariableSetValueShortTest, set_short_value_normal_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "shortValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_short maxValue = std::numeric_limits<int16_t>::max();
    ASSERT_EQ(env_->Variable_SetValue_Short(variable, maxValue), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkshortValue", maxValue), ANI_TRUE);

    ani_short minValue = std::numeric_limits<int16_t>::min();
    ASSERT_EQ(env_->Variable_SetValue_Short(variable, minValue), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkshortValue", minValue), ANI_TRUE);
}

TEST_F(VariableSetValueShortTest, set_Short_value_invalid_args_variable_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "floatValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_short value = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Short(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueShortTest, set_Short_value_invalid_args_variable_2)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_short value = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Short(nullptr, value), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)