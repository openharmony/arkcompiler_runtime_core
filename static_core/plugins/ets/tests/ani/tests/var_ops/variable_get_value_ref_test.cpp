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

class VariableGetValueRefTest : public AniTest {};

TEST_F(VariableGetValueRefTest, get_double_value)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "name", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_ref nameRef;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &nameRef), ANI_OK);

    auto name = static_cast<ani_string>(nameRef);
    std::array<char, 6U> buffer {};
    ani_size nameSize;
    ASSERT_EQ(env_->String_GetUTF8SubString(name, 0U, 3U, buffer.data(), buffer.size(), &nameSize), ANI_OK);
    ASSERT_EQ(nameSize, 3U);
    ASSERT_STREQ(buffer.data(), "Dam");
}

TEST_F(VariableGetValueRefTest, get_double_invalid_value_type)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "z", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_ref nameRef;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &nameRef), ANI_INVALID_TYPE);
}

TEST_F(VariableGetValueRefTest, invalid_args_variable)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};

    ani_ref nameRef;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &nameRef), ANI_INVALID_ARGS);
}

TEST_F(VariableGetValueRefTest, invalid_args_value)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "name", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
