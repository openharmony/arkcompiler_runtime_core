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

class ModuleFindNamespaceTest : public AniTest {};

TEST_F(ModuleFindNamespaceTest, find_namespace)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ASSERT_EQ(env_->Module_FindNamespace(module, "Louter/inner;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ASSERT_EQ(env_->Module_FindNamespace(module, "LopsT;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
}

TEST_F(ModuleFindNamespaceTest, invalid_arg_class)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "ops;", &ns), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lmodule_find_class_test/ops", &ns), ANI_NOT_FOUND);
}

TEST_F(ModuleFindNamespaceTest, invalid_arg_result)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_namespace_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lmodule_find_class_test/ATest;", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
