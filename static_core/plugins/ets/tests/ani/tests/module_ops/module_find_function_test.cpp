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

class ModuleFindFunctionTest : public AniTest {};

TEST_F(ModuleFindFunctionTest, find_int_function)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_function_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialIntValue", ":I", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValue", "Lstd/core/String;:V", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValue", "I:V", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValue", ":I", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(ModuleFindFunctionTest, find_ref_function)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_function_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialStringValue", ":Lstd/core/String;", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_moduleName)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_function_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialBool", ":Lstd/core/String;", &fn), ANI_NOT_FOUND);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_signature)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_function_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialStringValue", nullptr, &fn), ANI_OK);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_module)
{
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(nullptr, "getInitialStringValue", ":Lstd/core/String;", &fn), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_name)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_function_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, nullptr, ":Lstd/core/String;", &fn), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_name_in_namespace)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_function_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValueOps", ":I", &fn), ANI_NOT_FOUND);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_result)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_function_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialStringValue", ":Lstd/core/String;", nullptr),
              ANI_INVALID_ARGS);
}

TEST_F(ModuleFindFunctionTest, duplicate_no_signature)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_function_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "overloaded", nullptr, &fn), ANI_AMBIGUOUS);
}

}  // namespace ark::ets::ani::testing
