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

class EnumItemGetIndexTest : public AniTest {};

TEST_F(EnumItemGetIndexTest, get_enum_item_index)
{
    std::string itemName;

    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("Lenum_item_get_index_test/ToFind;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);
    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);

    ani_size redIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(red, &redIndex), ANI_OK);
    ASSERT_EQ(redIndex, 0U);
    ani_size greenIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(green, &greenIndex), ANI_OK);
    ASSERT_EQ(greenIndex, 1U);
    ani_size blueIndex {};
    ASSERT_EQ(env_->EnumItem_GetIndex(blue, &blueIndex), ANI_OK);
    ASSERT_EQ(blueIndex, 2U);
}

TEST_F(EnumItemGetIndexTest, invalid_arg_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("Lenum_item_get_index_test/ToFind;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);

    ani_size res {};
    ASSERT_EQ(env_->EnumItem_GetIndex(nullptr, &res), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->EnumItem_GetIndex(red, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
