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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class VariableSetValueCharTest : public AniTest {};

TEST_F(VariableSetValueCharTest, set_char_value)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "aChar", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_char value = 'B';
    ASSERT_EQ(env_->Variable_SetValue_Char(variable, value), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkCharValue", value), ANI_TRUE);
}

TEST_F(VariableSetValueCharTest, set_char_value_invalid_value_type)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "aDouble", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_char value = 'B';
    ASSERT_EQ(env_->Variable_SetValue_Char(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueCharTest, set_char_value_invalid_args_variable)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_char value = 'B';
    ASSERT_EQ(env_->Variable_SetValue_Char(nullptr, value), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)