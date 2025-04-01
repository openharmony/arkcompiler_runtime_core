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

class ModuleFindClassTest : public AniTest {};

TEST_F(ModuleFindClassTest, find_class)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_class_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "LATest;", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ASSERT_EQ(env_->Module_FindClass(module, "LBTest;", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ASSERT_EQ(env_->Module_FindClass(module, "Lops/C;", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, find_interface)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_class_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "LAA;", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ASSERT_EQ(env_->Module_FindClass(module, "LBB;", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, find_abstract_class)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_class_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "LPerson;", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, find_final_class)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_class_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "LChild;", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, invalid_arg_class)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_class_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "ATest;", &kclass), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindClass(module, "LATest", &kclass), ANI_NOT_FOUND);
}

TEST_F(ModuleFindClassTest, invalid_arg_result)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_class_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ASSERT_EQ(env_->Module_FindClass(module, "LATest;", nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
