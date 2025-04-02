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

class ModuleFindVariableTest : public AniTest {};

TEST_F(ModuleFindVariableTest, get_int_variable)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleX", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
    const ani_int y = 0;
    ani_int x = 1;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, y), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &x), ANI_OK);
    ASSERT_EQ(x, y);
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleS", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(ModuleFindVariableTest, get_ref_variable)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXS", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(ModuleFindVariableTest, get_invalid_variable_name)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "sss", &variable), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindVariable(module, "", &variable), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindVariable(module, nullptr, &variable), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, invalid_args_result)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXS", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, invalid_env)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_variable variable {};
    ASSERT_EQ(env_->c_api->Module_FindVariable(nullptr, module, "moduleXS", &variable), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, invalid_module)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(nullptr, "moduleXS", &variable), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, find_int_variable)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ani_variable variable1 {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleX", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
    variable1 = variable;
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleSSSSSS", &variable), ANI_NOT_FOUND);
    ASSERT_EQ(variable, variable1);
}

TEST_F(ModuleFindVariableTest, many_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_variable variable {};
    char end = 'J';
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        std::string str = "module";
        str += static_cast<char>(random() % (end - 'A') + 'A');
        ASSERT_EQ(env_->Module_FindVariable(module, str.c_str(), &variable), ANI_OK);
    }
}

TEST_F(ModuleFindVariableTest, find_variable_B_in_namespace_A)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleT", &variable), ANI_NOT_FOUND);

    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "moduleT", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(ModuleFindVariableTest, find_const_variable)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXB", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_int y = 0;
    ani_int x = 1;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, y), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &x), ANI_OK);
    ASSERT_EQ(x, y);
}

}  // namespace ark::ets::ani::testing
