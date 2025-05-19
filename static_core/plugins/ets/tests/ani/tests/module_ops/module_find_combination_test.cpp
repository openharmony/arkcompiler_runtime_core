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

class ModuleFindCombinationTest : public AniTest {};

TEST_F(ModuleFindCombinationTest, many_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_combination_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_class kclass {};
    ani_namespace ns {};
    ani_function fn {};
    ani_variable variable {};
    ani_enum aniEnum {};
    char end = 'J';
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        std::string className = "L";
        std::string namespaceName = "L";
        std::string functionName = "setIntValue";
        std::string variableName = "module";
        std::string enumName = "LColor";

        char ch = (random() % (end - 'A') + 'A');
        className += ch;
        className += ";";
        namespaceName += ch;
        namespaceName += "ops;";
        functionName += ch;
        variableName += ch;
        enumName += ch;
        enumName += ";";
        ASSERT_EQ(env_->Module_FindClass(module, className.c_str(), &kclass), ANI_OK);
        ASSERT_EQ(env_->Module_FindNamespace(module, namespaceName.c_str(), &ns), ANI_OK);
        ASSERT_EQ(env_->Module_FindFunction(module, functionName.c_str(), "I:V", &fn), ANI_OK);
        ASSERT_EQ(env_->Module_FindVariable(module, variableName.c_str(), &variable), ANI_OK);
        ASSERT_EQ(env_->Module_FindEnum(module, enumName.c_str(), &aniEnum), ANI_OK);
    }
}

TEST_F(ModuleFindCombinationTest, invalid_arg)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_combination_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_class kclass {};
    ani_namespace ns {};
    ani_function fn {};
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindClass(module, "LAAAA;", &kclass), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindNamespace(module, "LAAAAops;", &ns), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindFunction(module, "setIntValueAAAA", "I:V", &fn), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleAAAA", &variable), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Module_FindClass(nullptr, "LA;", &kclass), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindNamespace(nullptr, "LAops;", &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindFunction(nullptr, "setIntValueA", "I:V", &fn), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindVariable(nullptr, "moduleA", &variable), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
