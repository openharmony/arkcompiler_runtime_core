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
#include <iostream>
#include <array>

namespace ark::ets::ani::testing {

class NamespaceBindNativeFunctionsTest : public AniTest {};

static ani_int Sum([[maybe_unused]] ani_env *env, ani_int a, ani_int b)
{
    return a + b;
}

static ani_string Concat(ani_env *env, ani_string s0, ani_string s1)
{
    std::string str0;
    AniTest::GetStdString(env, s0, str0);
    std::string str1;
    AniTest::GetStdString(env, s1, str1);

    std::string newStr = str0 + str1;
    ani_string result;
    ani_status status = env->String_NewUTF8(newStr.c_str(), newStr.length(), &result);
    if (status != ANI_OK) {
        std::cerr << "Cannot create new utf8 string" << std::endl;
        std::abort();
    }
    return result;
}

TEST_F(NamespaceBindNativeFunctionsTest, bind_native_functions)
{
    ani_namespace ns;
    ASSERT_EQ(env_->FindNamespace("Lnamespace_bind_native_functions_test/ops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    const char *concatSignature = "Lstd/core/String;Lstd/core/String;:Lstd/core/String;";
    std::array functions = {
        ani_native_function {"sum", "II:I", reinterpret_cast<void *>(Sum)},
        ani_native_function {"concat", concatSignature, reinterpret_cast<void *>(Concat)},
    };
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("namespace_bind_native_functions_test", "checkSum"), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("namespace_bind_native_functions_test", "checkConcat"), ANI_TRUE);
}

TEST_F(NamespaceBindNativeFunctionsTest, already_binded_function)
{
    ani_namespace ns;
    ASSERT_EQ(env_->FindNamespace("Lnamespace_bind_native_functions_test/ops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    const char *concatSignature = "Lstd/core/String;Lstd/core/String;:Lstd/core/String;";
    std::array functions = {
        ani_native_function {"sum", "II:I", reinterpret_cast<void *>(Sum)},
        ani_native_function {"concat", concatSignature, reinterpret_cast<void *>(Concat)},
    };
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()), ANI_OK);
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()), ANI_ALREADY_BINDED);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("namespace_bind_native_functions_test", "checkSum"), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("namespace_bind_native_functions_test", "checkConcat"), ANI_TRUE);
}

TEST_F(NamespaceBindNativeFunctionsTest, invalid_ns)
{
    const char *concatSignature = "Lstd/core/String;Lstd/core/String;:Lstd/core/String;";
    std::array functions = {
        ani_native_function {"sum", "II:I", reinterpret_cast<void *>(Sum)},
        ani_native_function {"concat", concatSignature, reinterpret_cast<void *>(Concat)},
    };
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(nullptr, functions.data(), functions.size()), ANI_INVALID_ARGS);
}

TEST_F(NamespaceBindNativeFunctionsTest, invalid_functions)
{
    const char *concatSignature = "Lstd/core/String;Lstd/core/String;:Lstd/core/String;";
    std::array functions = {
        ani_native_function {"sum", "II:I", reinterpret_cast<void *>(Sum)},
        ani_native_function {"concat", concatSignature, reinterpret_cast<void *>(Concat)},
    };
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(nullptr, nullptr, functions.size()), ANI_INVALID_ARGS);
}

TEST_F(NamespaceBindNativeFunctionsTest, function_not_found)
{
    ani_namespace ns;
    ASSERT_EQ(env_->FindNamespace("Lnamespace_bind_native_functions_test/ops;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    const char *concatSignature = "Lstd/core/String;Lstd/core/String;:Lstd/core/String;";
    std::array functions = {
        ani_native_function {"sumX", "II:I", reinterpret_cast<void *>(Sum)},
        ani_native_function {"concat", concatSignature, reinterpret_cast<void *>(Concat)},
    };
    ASSERT_EQ(env_->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()), ANI_NOT_FOUND);
}

}  // namespace ark::ets::ani::testing
