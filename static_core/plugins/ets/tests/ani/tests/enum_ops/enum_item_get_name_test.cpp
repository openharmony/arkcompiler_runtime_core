/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

class EnumItemGetNameTest : public AniTest {};

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

TEST_F(EnumItemGetNameTest, get_enum_item_name_1)
{
    std::string itemName;

    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("Lenum_item_get_name_test/ToFind;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);

    ani_string redName {};
    ASSERT_EQ(env_->EnumItem_GetName(red, &redName), ANI_OK);
    RetrieveStringFromAni(env_, redName, itemName);
    ASSERT_STREQ(itemName.data(), "RED");
    ani_string greenName {};
    ASSERT_EQ(env_->EnumItem_GetName(green, &greenName), ANI_OK);
    RetrieveStringFromAni(env_, greenName, itemName);
    ASSERT_STREQ(itemName.data(), "GREEN");
    ani_string blueName {};
    ASSERT_EQ(env_->EnumItem_GetName(blue, &blueName), ANI_OK);
    RetrieveStringFromAni(env_, blueName, itemName);
    ASSERT_STREQ(itemName.data(), "BLUE");

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->FindEnum("Lenum_item_get_name_test/ToFindInt;", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);
    ani_enum_item redInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "REDINT", &redInt), ANI_OK);
    ani_enum_item greenInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "GREENINT", &greenInt), ANI_OK);
    ani_enum_item blueInt {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "BLUEINT", &blueInt), ANI_OK);

    ani_string redIntName {};
    ASSERT_EQ(env_->EnumItem_GetName(redInt, &redIntName), ANI_OK);
    RetrieveStringFromAni(env_, redIntName, itemName);
    ASSERT_STREQ(itemName.data(), "REDINT");
    ani_string greenIntName {};
    ASSERT_EQ(env_->EnumItem_GetName(greenInt, &greenIntName), ANI_OK);
    RetrieveStringFromAni(env_, greenIntName, itemName);
    ASSERT_STREQ(itemName.data(), "GREENINT");
    ani_string blueIntName {};
    ASSERT_EQ(env_->EnumItem_GetName(blueInt, &blueIntName), ANI_OK);
    RetrieveStringFromAni(env_, blueIntName, itemName);
    ASSERT_STREQ(itemName.data(), "BLUEINT");
}

TEST_F(EnumItemGetNameTest, get_enum_item_name_2)
{
    std::string itemName;

    ani_enum aniEnumStr {};
    ASSERT_EQ(env_->FindEnum("Lenum_item_get_name_test/ToFindString;", &aniEnumStr), ANI_OK);
    ASSERT_NE(aniEnumStr, nullptr);
    ani_enum_item redStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "REDSTR", &redStr), ANI_OK);
    ani_enum_item greenStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "GREENSTR", &greenStr), ANI_OK);
    ani_enum_item blueStr {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumStr, "BLUESTR", &blueStr), ANI_OK);

    ani_string redStrName {};
    ASSERT_EQ(env_->EnumItem_GetName(redStr, &redStrName), ANI_OK);
    RetrieveStringFromAni(env_, redStrName, itemName);
    ASSERT_STREQ(itemName.data(), "REDSTR");
    ani_string greenStrName {};
    ASSERT_EQ(env_->EnumItem_GetName(greenStr, &greenStrName), ANI_OK);
    RetrieveStringFromAni(env_, greenStrName, itemName);
    ASSERT_STREQ(itemName.data(), "GREENSTR");
    ani_string blueStrName {};
    ASSERT_EQ(env_->EnumItem_GetName(blueStr, &blueStrName), ANI_OK);
    RetrieveStringFromAni(env_, blueStrName, itemName);
    ASSERT_STREQ(itemName.data(), "BLUESTR");
}

TEST_F(EnumItemGetNameTest, invalid_arg_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("Lenum_item_get_name_test/ToFind;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);

    ani_string res {};
    ASSERT_EQ(env_->EnumItem_GetName(nullptr, &res), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->EnumItem_GetName(red, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
