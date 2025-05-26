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

class NamespaceCombinatorialTest : public AniTest {};
TEST_F(NamespaceCombinatorialTest, find01)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorial;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LCls;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindClass(ns, "LClsA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindClass(ns, "LClsB;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find02)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorial;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum result {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColor;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorB;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find03)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorialA;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum result {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColor;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorB;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find04)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorialA;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function result {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "fun", ":I", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_enum result1 {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorA;", &result1), ANI_OK);
    ASSERT_NE(result1, nullptr);

    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorB;", &result1), ANI_OK);
    ASSERT_NE(result1, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find05)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorialB;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function result {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "funA", ":I", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_enum result1 {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorB;", &result1), ANI_OK);
    ASSERT_NE(result1, nullptr);

    ani_class result2 {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LClsB;", &result2), ANI_OK);
    ASSERT_NE(result2, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find06)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorialB;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function result {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "funC", ":I", &result), ANI_NOT_FOUND);

    ani_enum result1 {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LEnuC;", &result1), ANI_NOT_FOUND);

    ani_class result2 {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LClsC;", &result2), ANI_NOT_FOUND);
}

TEST_F(NamespaceCombinatorialTest, find07)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorial;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace result {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Lsub;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_function result1 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "fun", ":I", &result1), ANI_OK);
    ASSERT_NE(result1, nullptr);

    ani_enum result2 {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColor;", &result2), ANI_OK);
    ASSERT_NE(result2, nullptr);

    ani_class result3 {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LCls;", &result3), ANI_OK);
    ASSERT_NE(result3, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find08)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorialC;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LsubA;", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(tmp, "LClsA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LsubB;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_enum result1 {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorA;", &result1), ANI_OK);
    ASSERT_NE(result1, nullptr);

    ani_function result2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "fun", ":I", &result2), ANI_OK);
    ASSERT_NE(result2, nullptr);
}

TEST_F(NamespaceCombinatorialTest, find09)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorialC;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "Lsubc;", &tmp), ANI_NOT_FOUND);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LClsC;", &result), ANI_NOT_FOUND);

    ani_enum result1 {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LEnuE;", &result1), ANI_NOT_FOUND);

    ani_function result2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "funC", ":I", &result2), ANI_NOT_FOUND);
}

TEST_F(NamespaceCombinatorialTest, find10)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_combinatorial_test/combinatorialD;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LsubA;", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LClsA;", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_enum result1 {};
    ASSERT_EQ(env_->Namespace_FindEnum(ns, "LColorA;", &result1), ANI_OK);
    ASSERT_NE(result1, nullptr);

    ani_function result2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "fun", ":I", &result2), ANI_OK);
    ASSERT_NE(result2, nullptr);

    ani_namespace tmp1 {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nullptr, "LsubA;", &tmp1), ANI_INVALID_ARGS);

    ani_class result3 {};
    ASSERT_EQ(env_->Namespace_FindClass(nullptr, "LClsA;", &result3), ANI_INVALID_ARGS);

    ani_enum result4 {};
    ASSERT_EQ(env_->Namespace_FindEnum(nullptr, "LColorA;", &result4), ANI_INVALID_ARGS);

    ani_function result5 {};
    ASSERT_EQ(env_->Namespace_FindFunction(nullptr, "fun", ":I", &result5), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing