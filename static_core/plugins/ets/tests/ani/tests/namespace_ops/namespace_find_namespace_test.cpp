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

class NamespaceFindNamespaceTest : public AniTest {};

TEST_F(NamespaceFindNamespaceTest, find_namespace01)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Lrosen;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace02)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Ltest;", &result), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace03)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Ltesttt;", &result), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace04)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Lrosen;", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(tmp, "Ltest;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace05)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Lrosenn;", &tmp), ANI_NOT_FOUND);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(tmp, "Ltest;", &result), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace06)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Lrosen;", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(tmp, "Ltesttt;", &result), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace07)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "/rosen;", &result), ANI_NOT_FOUND);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace08)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nullptr, "Ltest;", &result), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace09)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, nullptr, &result), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace10)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Ltest;", nullptr), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace11)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/anyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Lrosen/test;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}
}  // namespace ark::ets::ani::testing