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

class GetEnumFromItemTest : public AniTest {};

TEST_F(GetEnumFromItemTest, get_enum_from_item)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("Lget_enum_from_item_test/ToFind;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);

    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 1U, &green), ANI_OK);

    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 2U, &blue), ANI_OK);

    ani_enum fromRed {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromRed), ANI_OK);

    ani_enum fromGreen {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromGreen), ANI_OK);

    ani_enum fromBlue {};
    ASSERT_EQ(env_->EnumItem_GetEnum(red, &fromBlue), ANI_OK);

    ani_boolean isRedEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(aniEnum, fromRed, &isRedEqual), ANI_OK);
    ASSERT_EQ(isRedEqual, ANI_TRUE);

    ani_boolean isGreenEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(aniEnum, fromGreen, &isGreenEqual), ANI_OK);
    ASSERT_EQ(isGreenEqual, ANI_TRUE);

    ani_boolean isBlueEqual = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(aniEnum, fromBlue, &isBlueEqual), ANI_OK);
    ASSERT_EQ(isBlueEqual, ANI_TRUE);
}

TEST_F(GetEnumFromItemTest, invalid_arg_enum)
{
    ani_enum aniEnum {};
    ASSERT_EQ(env_->FindEnum("Lget_enum_from_item_test/ToFind;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(aniEnum, 0U, &red), ANI_OK);

    ani_enum fromRed {};
    ASSERT_EQ(env_->EnumItem_GetEnum(nullptr, &fromRed), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->EnumItem_GetEnum(red, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
