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

class VariableSetValueBooleanTest : public AniTest {};

TEST_F(VariableSetValueBooleanTest, set_boolean_value)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "aBool", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable, value), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkBooleanValue", value), ANI_TRUE);
}

TEST_F(VariableSetValueBooleanTest, set_boolean_value_invalid_value_type)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "aByte", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueBooleanTest, set_boolean_value_invalid_args_variable)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(nullptr, value), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing