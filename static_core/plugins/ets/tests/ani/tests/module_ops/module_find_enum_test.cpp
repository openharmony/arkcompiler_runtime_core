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

void RetrieveStringFromAni(ani_env *env, ani_string string, std::string &resString)
{
    ani_size result = 0U;
    ASSERT_EQ(env->String_GetUTF8Size(string, &result), ANI_OK);
    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const ani_size bufferExtension = 10U;
    resString.resize(substrSize + bufferExtension);
    ani_size resSize = resString.size();
    result = 0U;
    auto status = env->String_GetUTF8SubString(string, substrOffset, substrSize, resString.data(), resSize, &result);
    ASSERT_EQ(status, ANI_OK);
}

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

TEST_F(ModuleFindEnumTest, find_enum_combine_scenes_001)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_enum aniEnum {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColor;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);

    ani_enum fromRed {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromRed), ANI_OK);

    ani_boolean isRedEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(aniEnum, fromRed, &isRedEqual), ANI_OK);
    ASSERT_EQ(isRedEqual, ANI_TRUE);

    ani_int redVal = 5U;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(red, &redVal), ANI_OK);
    ASSERT_EQ(redVal, 0U);
}

TEST_F(ModuleFindEnumTest, find_enum_combine_scenes_002)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_enum aniEnum {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColor;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);

    ani_enum_item red2 {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red2), ANI_OK);

    ani_boolean isRedEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(red, red2, &isRedEqual), ANI_OK);
    ASSERT_EQ(isRedEqual, ANI_TRUE);
}

TEST_F(ModuleFindEnumTest, find_enum_combine_scenes_003)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/module_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_enum aniEnum {};
    ASSERT_EQ(env_->Module_FindEnum(module, "LColor;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);

    std::string itemName {};
    ani_string redName {};
    ASSERT_EQ(env_->EnumItem_GetName(red, &redName), ANI_OK);
    RetrieveStringFromAni(env_, redName, itemName);
    ASSERT_STREQ(itemName.data(), "RED");

    ani_size redIndex = 5U;
    ASSERT_EQ(env_->EnumItem_GetIndex(red, &redIndex), ANI_OK);
    ASSERT_EQ(redIndex, 0U);
}

TEST_F(ModuleFindEnumTest, check_initialization)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_enum_test", &module), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_enum_test", false));
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_enum_test.Color"));
    ani_enum enm {};
    ASSERT_EQ(env_->Module_FindEnum(module, "Color", &enm), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_enum_test.Color"));
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_enum_test", false));
}

}  // namespace ark::ets::ani::testing
