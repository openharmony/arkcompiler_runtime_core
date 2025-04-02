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
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/test;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(result, "LB;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace02)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/test;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA;", &result), ANI_OK);
        ASSERT_NE(result, nullptr);
    }
    ASSERT_EQ(env_->Namespace_FindNamespace(result, "LB;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_function method {};
    ASSERT_EQ(env_->Namespace_FindFunction(result, "intMethod", "II:I", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace03)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/test;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(result, "LC;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(result, "LD;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace04)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/test;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace nsB {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA;", &nsB), ANI_OK);
    ASSERT_NE(nsB, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nsB, "LB;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace05)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/test;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LD;", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LE;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(result, "LC;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(result, "LD;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace06)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/test;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace nsC {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA/C;", &nsC), ANI_OK);
    ASSERT_NE(nsC, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(result, "LC;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindNamespaceTest, find_namespace07)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_namespace_test/test;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace nsC {};
    ASSERT_EQ(env_->c_api->Namespace_FindNamespace(nullptr, ns, "LA/C;", &nsC), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindNamespace(nullptr, "LA/C;", &nsC), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, nullptr, &nsC), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "", &nsC), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA/CA;", &nsC), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA/C;", nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing