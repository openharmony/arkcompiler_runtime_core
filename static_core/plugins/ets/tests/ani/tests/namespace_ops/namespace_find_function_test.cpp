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

class NamespaceFindFunctionTest : public AniTest {};

TEST_F(NamespaceFindFunctionTest, find_function01)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/fnns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialIntValue", ":I", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);
    }

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialStringValue", ":Lstd/core/String;", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialDoubleValue", ":D", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialIntValue", ":I", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_function02)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/fnns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn1 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialIntValue", ":I", &fn1), ANI_OK);
    ASSERT_NE(fn1, nullptr);

    ani_function fn2 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialStringValue", ":Lstd/core/String;", &fn2), ANI_OK);
    ASSERT_NE(fn2, nullptr);

    ani_function fn3 {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialDoubleValue", ":D", &fn3), ANI_OK);
    ASSERT_NE(fn3, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_function03)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/fnns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method result {};
    ani_function fn {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":V", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "C", "I:Lstd/core/Promise;", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_function04)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/testA;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "B", ":I", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_function05)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/testA;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "LA;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->c_api->Namespace_FindFunction(nullptr, ns, "B", ":I", &fn), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindFunction(nullptr, "B", ":I", &fn), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, nullptr, ":I", &fn), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "", ":I", &fn), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "BA", ":I", &fn), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "B", nullptr, &fn), ANI_OK);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "B", "", &fn), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "B", "D:II", &fn), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "B", "D:II", nullptr), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindFunctionTest, duplicate_no_signature)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/fnns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "overloaded", nullptr, &fn), ANI_AMBIGUOUS);
}
}  // namespace ark::ets::ani::testing
