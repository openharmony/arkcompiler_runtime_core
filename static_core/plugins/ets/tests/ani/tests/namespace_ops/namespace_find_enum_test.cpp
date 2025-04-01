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

class NamespaceFindEnumTest : public AniTest {};

TEST_F(NamespaceFindEnumTest, namespace_find_enum)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/namespace_find_enum_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Ltest002A;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum aniEnum {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LEnumA002;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum_item red {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "RED", &red), ANI_OK);
    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "GREEN", &green), ANI_OK);
    ani_enum_item blue {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnum, "BLUE", &blue), ANI_OK);

    ani_int redValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(red, &redValInt), ANI_OK);
    ASSERT_EQ(redValInt, 0U);
    ani_int greenValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(green, &greenValInt), ANI_OK);
    ASSERT_EQ(greenValInt, 1U);
    ani_int blueValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(blue, &blueValInt), ANI_OK);
    ASSERT_EQ(blueValInt, 2U);
}

}  // namespace ark::ets::ani::testing
