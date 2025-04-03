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

class ModuleFindEnumTest : public AniTest {
public:
    void CheckEnumValue(ani_enum aniEnum, const std::string &name, int value)
    {
        ani_enum_item item {};
        ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, name.c_str(), &item), ANI_OK);
        ani_int valueInt {};
        ASSERT_EQ(env_->EnumItem_GetValue_Int(item, &valueInt), ANI_OK);
        ASSERT_EQ(valueInt, value);
    }
};

TEST_F(ModuleFindEnumTest, find_enum)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_enum aniEnum {};
    ani_enum aniEnum1 {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColor;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    CheckEnumValue(aniEnum, "RED", 0);  // 0 for test in sts
    aniEnum1 = aniEnum;
    ASSERT_EQ(env_->Module_FindEnum(module, "LColorAAA;", &aniEnum), ANI_NOT_FOUND);
    ASSERT_EQ(aniEnum, aniEnum1);
    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColorInt;", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);
    const int value = 5;  // 5 for test in sts
    CheckEnumValue(aniEnumInt, "RED", value);
    ani_enum aniEnumString {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColorString;", &aniEnumString), ANI_OK);
    ASSERT_NE(aniEnumString, nullptr);
}

TEST_F(ModuleFindEnumTest, invalid_args)
{
    ani_enum aniEnum {};
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ASSERT_EQ(env_->c_api->Module_FindEnum(nullptr, module, "LBColor;", &aniEnum), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindEnum(nullptr, "LBColor;", &aniEnum), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindEnum(module, nullptr, &aniEnum), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindEnum(module, "", &aniEnum), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindEnum(module, "#BColor;", &aniEnum), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindEnum(module, "LBColor;", &aniEnum), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindEnum(module, "LBColor", &aniEnum), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindEnum(module, "LBColor;", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindEnumTest, many_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_enum aniEnum {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColor;", &aniEnum), ANI_OK);
    char end = 'C';
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        std::string str = "LColor";
        str += static_cast<char>(random() % (end - 'A') + 'A');
        str += ";";
        ASSERT_EQ(env_->Module_FindEnum(module, str.c_str(), &aniEnum), ANI_OK);
        ASSERT_NE(aniEnum, nullptr);
    }
}

TEST_F(ModuleFindEnumTest, find_enum_B_in_namespace_A)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_enum aniEnum {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColorAA;", &aniEnum), ANI_NOT_FOUND);

    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Lops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorAA;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    const int value = 5;  // 5 for test in ets
    CheckEnumValue(aniEnum, "RED", value);
}
}  // namespace ark::ets::ani::testing
