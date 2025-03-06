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

class ModuleFindEnumTest : public AniTest {};

TEST_F(ModuleFindEnumTest, find_enum)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_enum aniEnum {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColor;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColorInt;", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);

    ani_enum aniEnumString {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColorString;", &aniEnumString), ANI_OK);
    ASSERT_NE(aniEnumString, nullptr);
}

TEST_F(ModuleFindEnumTest, invalid_arg_enum)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_enum en {};
    ASSERT_EQ(env_->Module_FindEnum(nullptr, "LColor;", &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Module_FindEnum(module, nullptr, &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Module_FindEnum(module, "LColor;", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindEnumTest, invalid_arg_name)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_enum en {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LNotFound;", &en), ANI_NOT_FOUND);
}

}  // namespace ark::ets::ani::testing
