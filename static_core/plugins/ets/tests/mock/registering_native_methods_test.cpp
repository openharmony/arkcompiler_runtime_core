/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "libpandabase/utils/utils.h"
#include "plugins/ets/tests/mock/mock_test_helper.h"

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-pro-type-vararg, readability-named-parameter)

namespace ark::ets::test {

static const char *TEST_BIN_FILE_NAME = "RegisteringNativeMethodsTest.abc";

class RegisteringNativeMethodsTestBase : public MockEtsNapiTestBaseClass {
public:
    RegisteringNativeMethodsTestBase() : MockEtsNapiTestBaseClass(TEST_BIN_FILE_NAME) {}
};

class RegisteringNativeMethodsTest : public RegisteringNativeMethodsTestBase {};
class RegisteringNativeMethodsTestDeath : public RegisteringNativeMethodsTestBase {};

static ets_int NativeMethods_foo_native(EtsEnv *, ets_class)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    return 42_I;
}

static ets_long NativeMethods_long_foo_native(EtsEnv *, ets_class)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    return static_cast<ets_long>(84L);
}

TEST_F(RegisteringNativeMethodsTestDeath, DISABLED_RegisterNativesDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        EXPECT_DEATH(env_->RegisterNatives(nullptr, nullptr, 0), "");
        EXPECT_DEATH(env_->RegisterNatives(nullptr, nullptr, 1), "");
        EXPECT_DEATH(env_->RegisterNatives(nullptr, nullptr, -1), "");
    }

    {
        ets_class cls = env_->FindClass("RegisteringNativeMethodsTest/RegisteringNativeMethodsTest");
        ASSERT_NE(cls, nullptr);
        EXPECT_DEATH(env_->RegisterNatives(cls, nullptr, 1), "");
        EXPECT_DEATH(env_->RegisterNatives(cls, nullptr, -1), "");
    }

    {
        EtsNativeMethod native_method = {"foo", ":I", reinterpret_cast<void *>(NativeMethods_foo_native)};
        EXPECT_DEATH(env_->RegisterNatives(nullptr, &native_method, 0), "");
        EXPECT_DEATH(env_->RegisterNatives(nullptr, &native_method, 1), "");
        EXPECT_DEATH(env_->RegisterNatives(nullptr, &native_method, -1), "");

        ets_class cls = env_->FindClass("RegisteringNativeMethodsTest/RegisteringNativeMethodsTest");
        ASSERT_NE(cls, nullptr);
        EXPECT_DEATH(env_->RegisterNatives(cls, &native_method, -1), "");
    }
}

TEST_F(RegisteringNativeMethodsTest, RegisterNativesTest)
{
    ets_class cls = env_->FindClass("RegisteringNativeMethodsTest/RegisteringNativeMethodsTest");
    ASSERT_NE(cls, nullptr);
    EtsNativeMethod native_method = {"foo", ":I", reinterpret_cast<void *>(NativeMethods_foo_native)};

    ASSERT_EQ(env_->RegisterNatives(cls, &native_method, 0), ETS_OK);
}

// NOTE(m.morozov): Add exception test when exceptions will be implemeted

TEST_F(RegisteringNativeMethodsTest, RegisterNativesArrayTest)
{
    ets_class cls = env_->FindClass("RegisteringNativeMethodsTest/RegisteringNativeMethodsTest");
    ASSERT_NE(cls, nullptr);
    EtsNativeMethod native_method = {"foo", ":I", reinterpret_cast<void *>(NativeMethods_foo_native)};
    EtsNativeMethod long_native_method = {"long_foo", ":J", reinterpret_cast<void *>(NativeMethods_long_foo_native)};
    const std::vector<EtsNativeMethod> methods_src = {native_method, long_native_method};

    ASSERT_EQ(env_->RegisterNatives(cls, methods_src.data(), methods_src.size()), ETS_OK);

    ets_method foo_id = env_->GetStaticp_method(cls, "foo", ":I");
    ASSERT_NE(foo_id, nullptr);
    ASSERT_EQ(env_->CallStaticIntMethod(cls, foo_id), static_cast<ets_int>(42_I));

    ets_method long_foo_id = env_->GetStaticp_method(cls, "long_foo", ":J");
    ASSERT_NE(long_foo_id, nullptr);
    ASSERT_EQ(env_->CallStaticLongMethod(cls, long_foo_id), static_cast<ets_int>(84_I));
}

TEST_F(RegisteringNativeMethodsTestDeath, DISABLED_UnregisterNativesDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ASSERT_DEATH(env_->UnregisterNatives(nullptr), "");
}

TEST_F(RegisteringNativeMethodsTest, UnregisterNativesTest)
{
    {
        ets_class a_cls = env_->FindClass("RegisteringNativeMethodsTest/A");
        ASSERT_NE(a_cls, nullptr);
        ASSERT_EQ(env_->UnregisterNatives(a_cls), ETS_OK);
    }

    {
        ets_class cls = env_->FindClass("RegisteringNativeMethodsTest/RegisteringNativeMethodsTest");
        ASSERT_NE(cls, nullptr);
        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK);
    }

    {
        ets_class cls = env_->FindClass("RegisteringNativeMethodsTest/RegisteringNativeMethodsTest");
        ASSERT_NE(cls, nullptr);
        EtsNativeMethod native_method = {"foo", ":I", reinterpret_cast<void *>(NativeMethods_foo_native)};

        ASSERT_EQ(env_->RegisterNatives(cls, &native_method, 1), ETS_OK);
        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK);
    }

    {
        ets_class cls = env_->FindClass("RegisteringNativeMethodsTest/RegisteringNativeMethodsTest");
        ASSERT_NE(cls, nullptr);
        EtsNativeMethod native_method = {"foo", ":I", reinterpret_cast<void *>(NativeMethods_foo_native)};
        EtsNativeMethod long_native_method = {"long_foo", ":J",
                                              reinterpret_cast<void *>(NativeMethods_long_foo_native)};
        const std::vector<EtsNativeMethod> methods_src = {native_method, long_native_method};

        ASSERT_EQ(env_->RegisterNatives(cls, methods_src.data(), methods_src.size()), ETS_OK);
        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK);
    }
}

}  // namespace ark::ets::test

// NOLINTEND(readability-identifier-naming, cppcoreguidelines-pro-type-vararg, readability-named-parameter)
