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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ClassBindNativeMethodsTest : public AniTest {};
// NOLINTNEXTLINE(readability-named-parameter)
static ani_int NativeMethodsFooNative(ani_env *, ani_class)
{
    const ani_int answer = 42U;
    return answer;
}

// NOLINTNEXTLINE(readability-named-parameter)
static ani_long NativeMethodsLongFooNative(ani_env *, ani_class)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    return static_cast<ani_long>(84L);
}

// ninja ani_test_class_bindnativemethods_gtests
TEST_F(ClassBindNativeMethodsTest, RegisterNativesTest)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LRegisteringNativeMethodsTest;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":I", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":J", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);
}

TEST_F(ClassBindNativeMethodsTest, RegisterNativesErrorTest)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LRegisteringNativeMethodsTest;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":I", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo11", ":J", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ani_size nrMethods = 2;
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, nullptr, nrMethods), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), nrMethods), ANI_NOT_FOUND);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)