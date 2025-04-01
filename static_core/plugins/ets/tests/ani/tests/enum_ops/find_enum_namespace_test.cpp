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

class EnumFindInNamespaceTest : public AniTest {};

TEST_F(EnumFindInNamespaceTest, find_enum)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lfind_enum_namespace_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum aniEnum {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LToFind;", &aniEnum), ANI_OK);
    ASSERT_NE(aniEnum, nullptr);

    ani_enum aniEnumInt {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LToFindInt;", &aniEnumInt), ANI_OK);
    ASSERT_NE(aniEnumInt, nullptr);

    ani_enum aniEnumString {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LToFindString;", &aniEnumString), ANI_OK);
    ASSERT_NE(aniEnumString, nullptr);
}

TEST_F(EnumFindInNamespaceTest, invalid_arg_enum)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lfind_enum_namespace_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum en {};
    ASSERT_EQ(env_->Namespace_FindEnum(nullptr, "LToFind;", &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, nullptr, &en), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LToFind;", nullptr), ANI_INVALID_ARGS);
}

TEST_F(EnumFindInNamespaceTest, invalid_arg_name)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lfind_enum_namespace_test/enumns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum en {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LNotFound;", &en), ANI_NOT_FOUND);
}

}  // namespace ark::ets::ani::testing
