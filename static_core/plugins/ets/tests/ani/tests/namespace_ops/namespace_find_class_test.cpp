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

class NamespaceFindClassTest : public AniTest {};

TEST_F(NamespaceFindClassTest, find_class01)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_class_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LtestA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class02)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_class_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LtestB;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class03)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_class_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "/testB;", &result), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindClassTest, find_class04)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_class_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "Ltesttt;", &result), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindClassTest, find_class05)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_class_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(nullptr, "LtestA;", &result), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindClassTest, find_class06)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_class_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindClassTest, find_class07)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_class_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindClass(ns, "LtestA;", nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing