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
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleX", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleS", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(ModuleFindVariableTest, get_ref_variable)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXS", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(ModuleFindVariableTest, get_invalid_variable_name)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "sss", &variable), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindVariable(module, nullptr, &variable), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, invalid_args_result)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_variable_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXS", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
