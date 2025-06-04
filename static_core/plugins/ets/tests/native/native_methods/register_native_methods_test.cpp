/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "plugins/ets/runtime/napi/ets_napi.h"
#include "plugins/ets/tests/native/native_test_helper.h"

namespace ark::ets::test {

class RegisterNativeMethodTest : public EtsNapiTestBaseClass {};

static constexpr const char *CLASS_NAME = "register_native_methods_test/NativeRegistrationTest";

static constexpr const char *GENERIC_NATIVE_METHOD_NAME = "Func0";
static constexpr const char *FAST_NATIVE_METHOD_NAME = "Func0";
static constexpr const char *CRITICAL_NATIVE_METHOD_NAME = "Func1";

static constexpr const char *FAST_NATIVE_PREFIX = "#F$";
static constexpr const char *CRITICAL_NATIVE_PREFIX = "#C$";

static constexpr const char *SIGNATURE_FUNC0 = "I:Z";
static constexpr const char *SIGNATURE_FUNC1 = "JJ:J";

static std::string Concat(const char *prefix, const char *signature)
{
    return std::string(prefix) + std::string(signature);
}

// Return true if registered as FastNative, otherwise return false.
ets_boolean Func0(EtsEnv *env, [[maybe_unused]] ets_int param)
{
    ets_class cls = env->FindClass(CLASS_NAME);

    napi::ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));

    EtsClass *klass = s.ToInternalType(cls);
    ASSERT(klass);

    EtsMethod *method = klass->GetStaticMethod(FAST_NATIVE_METHOD_NAME, nullptr);
    ASSERT(method);

    return static_cast<ets_boolean>(method->IsFastNative());
}

ets_long Func1(ets_long param1, ets_long param2)
{
    return param1 + param2;
}

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
TEST_F(RegisterNativeMethodTest, Generic)
{
    auto cls = env_->FindClass(CLASS_NAME);
    ASSERT_NE(cls, nullptr);

    auto method = env_->GetStaticp_method(cls, GENERIC_NATIVE_METHOD_NAME, SIGNATURE_FUNC0);
    ASSERT_NE(method, nullptr);

    {
        std::array<EtsNativeMethod, 1> etsNativeMethods = {{
            {GENERIC_NATIVE_METHOD_NAME, SIGNATURE_FUNC0, reinterpret_cast<void *>(Func0)},
        }};

        ASSERT_EQ(env_->RegisterNatives(cls, etsNativeMethods.data(), etsNativeMethods.size()), ETS_OK);

        ets_boolean result = env_->CallStaticBooleanMethod(cls, method);
        ASSERT_EQ(result, static_cast<ets_boolean>(false));

        auto *etsMethod = EtsMethod::FromRuntimeMethod(reinterpret_cast<Method *>(method));
        ASSERT_FALSE(etsMethod->IsCriticalNative());
        ASSERT_FALSE(etsMethod->IsFastNative());

        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK);
    }
}

TEST_F(RegisterNativeMethodTest, Fast)
{
    auto cls = env_->FindClass(CLASS_NAME);
    ASSERT_NE(cls, nullptr);

    auto method = env_->GetStaticp_method(cls, FAST_NATIVE_METHOD_NAME, "I:Z");
    ASSERT_NE(method, nullptr);

    {
        auto signature = Concat(FAST_NATIVE_PREFIX, SIGNATURE_FUNC0);

        std::array<EtsNativeMethod, 1> etsNativeMethods = {{
            {FAST_NATIVE_METHOD_NAME, signature.c_str(), reinterpret_cast<void *>(Func0)},
        }};

        ASSERT_EQ(env_->RegisterNatives(cls, etsNativeMethods.data(), etsNativeMethods.size()), ETS_OK);

        ets_boolean result = env_->CallStaticBooleanMethod(cls, method);
        ASSERT_EQ(result, static_cast<ets_boolean>(true));

        auto *etsMethod = EtsMethod::FromRuntimeMethod(reinterpret_cast<Method *>(method));
        ASSERT_TRUE(etsMethod->IsFastNative());
        ASSERT_FALSE(etsMethod->IsCriticalNative());

        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK);
    }
}

TEST_F(RegisterNativeMethodTest, FastWithEmtySignature)
{
    auto cls = env_->FindClass(CLASS_NAME);
    ASSERT_NE(cls, nullptr);

    auto method = env_->GetStaticp_method(cls, FAST_NATIVE_METHOD_NAME, "I:Z");
    ASSERT_NE(method, nullptr);

    {
        std::array<EtsNativeMethod, 1> etsNativeMethods = {{
            {FAST_NATIVE_METHOD_NAME, FAST_NATIVE_PREFIX, reinterpret_cast<void *>(Func0)},
        }};

        ASSERT_EQ(env_->RegisterNatives(cls, etsNativeMethods.data(), etsNativeMethods.size()), ETS_OK);

        ets_boolean result = env_->CallStaticBooleanMethod(cls, method);
        ASSERT_EQ(result, static_cast<ets_boolean>(true));

        auto *etsMethod = EtsMethod::FromRuntimeMethod(reinterpret_cast<Method *>(method));
        ASSERT_TRUE(etsMethod->IsFastNative());
        ASSERT_FALSE(etsMethod->IsCriticalNative());

        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK);
    }
}

TEST_F(RegisterNativeMethodTest, Critical)
{
    auto cls = env_->FindClass(CLASS_NAME);
    ASSERT_NE(cls, nullptr);

    auto method = env_->GetStaticp_method(cls, CRITICAL_NATIVE_METHOD_NAME, "JJ:J");
    ASSERT_NE(method, nullptr);

    {
        auto signature = Concat(CRITICAL_NATIVE_PREFIX, SIGNATURE_FUNC1);

        std::array<EtsNativeMethod, 1> etsNativeMethods = {{
            {CRITICAL_NATIVE_METHOD_NAME, signature.c_str(), reinterpret_cast<void *>(Func1)},
        }};

        ASSERT_EQ(env_->RegisterNatives(cls, etsNativeMethods.data(), etsNativeMethods.size()), ETS_OK);

        ets_long param1 = 10U;
        ets_long param2 = 20U;

        ets_long result = env_->CallStaticLongMethod(cls, method, param1, param2);
        ASSERT_EQ(result, param1 + param2);

        auto *etsMethod = EtsMethod::FromRuntimeMethod(reinterpret_cast<Method *>(method));
        ASSERT_TRUE(etsMethod->IsCriticalNative());
        ASSERT_FALSE(etsMethod->IsFastNative());

        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK);
    }
}

TEST_F(RegisterNativeMethodTest, CriticalWithEmptySignature)
{
    auto cls = env_->FindClass(CLASS_NAME);
    ASSERT_NE(cls, nullptr);

    auto method = env_->GetStaticp_method(cls, CRITICAL_NATIVE_METHOD_NAME, "JJ:J");
    ASSERT_NE(method, nullptr);

    {
        std::array<EtsNativeMethod, 1> etsNativeMethods = {{
            {CRITICAL_NATIVE_METHOD_NAME, CRITICAL_NATIVE_PREFIX, reinterpret_cast<void *>(Func1)},
        }};

        ASSERT_EQ(env_->RegisterNatives(cls, etsNativeMethods.data(), etsNativeMethods.size()), ETS_OK);

        ets_long param1 = 10U;
        ets_long param2 = 20U;

        ets_long result = env_->CallStaticLongMethod(cls, method, param1, param2);
        ASSERT_EQ(result, param1 + param2);

        auto *etsMethod = EtsMethod::FromRuntimeMethod(reinterpret_cast<Method *>(method));
        ASSERT_TRUE(etsMethod->IsCriticalNative());
        ASSERT_FALSE(etsMethod->IsFastNative());

        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK);
    }
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)

}  // namespace ark::ets::test
