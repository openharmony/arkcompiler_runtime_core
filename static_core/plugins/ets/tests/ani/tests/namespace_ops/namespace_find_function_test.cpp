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

TEST_F(NamespaceFindFunctionTest, find_int_function)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/fnns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialIntValue", ":I", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(NamespaceFindFunctionTest, find_ref_function)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/fnns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialStringValue", ":Lstd/core/String;", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(NamespaceFindFunctionTest, invalid_arg_namespace)
{
    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(nullptr, "getInitialStringValue", ":Lstd/core/String;", &fn),
              ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindFunctionTest, invalid_arg_name)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/fnns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Namespace_FindFunction(ns, nullptr, ":Lstd/core/String;", &fn), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindFunctionTest, invalid_arg_result)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lnamespace_find_function_test/fnns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getInitialStringValue", ":Lstd/core/String;", nullptr),
              ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
