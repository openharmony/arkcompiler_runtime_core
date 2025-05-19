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
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_enum_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorInt;", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);

    ani_enum aniEnumString {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorString;", &aniEnumString), ANI_OK);
    ASSERT_NE(aniEnumString, nullptr);

    ani_enum_item green {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(aniEnumInt, "GREEN", &green), ANI_OK);

    ani_int greenValInt = 0;
    ASSERT_EQ(env_->EnumItem_GetValue_Int(green, &greenValInt), ANI_OK);
    ASSERT_EQ(greenValInt, 77U);  // 77 for test in ets
}

TEST_F(NamespaceFindEnumTest, find_enum_many_times)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_enum_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum aniEnum {};
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColor;", &aniEnum), ANI_OK);
        ASSERT_NE(aniEnum, nullptr);
    }
}

TEST_F(NamespaceFindEnumTest, invalid_args)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_enum_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum en {};
    ASSERT_EQ(env_->c_api->Namespace_FindEnum(nullptr, ns, "L#Color;", &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindEnum(nullptr, "L#Color;", &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, nullptr, &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "", &en), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "L#ColorA", &en), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "L#Color;", nullptr), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindEnumTest, find_enum_sub_namespace)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_enum_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LsubnsA;", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_enum en {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LNotFound;", &en), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColor;", &en), ANI_OK);
    ASSERT_NE(en, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LsubnsB;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorInt;", &en), ANI_OK);
    ASSERT_NE(en, nullptr);
}

TEST_F(NamespaceFindEnumTest, find_enum_sub_namespace_many_times)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_enum_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace ns1 {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LsubnsA;", &ns1), ANI_OK);
    ASSERT_NE(ns1, nullptr);

    ani_enum en {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns1, "LColor;", &en), ANI_OK);
    ASSERT_NE(en, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LsubnsB;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorInt;", &en), ANI_OK);
    ASSERT_NE(en, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorString;", &en), ANI_OK);
    ASSERT_NE(en, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LtestEnum;", &en), ANI_OK);
    ASSERT_NE(en, nullptr);
}

TEST_F(NamespaceFindEnumTest, find_enum_same_name)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_enum_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum en {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColor;", &en), ANI_OK);
    ASSERT_NE(en, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LsubnsA;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColor;", &en), ANI_OK);
    ASSERT_NE(en, nullptr);
}
}  // namespace ark::ets::ani::testing
