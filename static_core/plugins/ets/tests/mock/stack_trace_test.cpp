/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-pro-type-vararg)

namespace ark::ets::test {
static const char *TEST_BIN_FILE_NAME = "StackTraceTest.abc";

class StackTraceTest : public testing::Test {
protected:
    static void SetUpTestCase()
    {
        if (std::getenv("PANDA_STD_LIB") == nullptr) {
            std::cerr << "PANDA_STD_LIB not set" << std::endl;
            std::abort();
        }
    }

    void SetUp() override
    {
        std::vector<EtsVMOption> options_vector;

        options_vector = {{EtsOptionType::ETS_BOOT_FILE, std::getenv("PANDA_STD_LIB")},
                          {EtsOptionType::ETS_BOOT_FILE, TEST_BIN_FILE_NAME}};

        EtsVMInitArgs vm_args;
        vm_args.version = ETS_NAPI_VERSION_1_0;
        vm_args.options = options_vector.data();
        vm_args.nOptions = static_cast<ets_int>(options_vector.size());

        ASSERT_TRUE(ETS_CreateVM(&vm, &env, &vm_args) == ETS_OK) << "Cannot create ETS VM";
    }

    void TearDown() override
    {
        ASSERT_TRUE(vm->DestroyEtsVM() == ETS_OK) << "Cannot destroy ETS VM";
    }
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    EtsEnv *env {};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    EtsVM *vm {};
};

TEST_F(StackTraceTest, PrintStackTrace)
{
    auto klass = env->FindClass("StackTraceTest");
    ASSERT_NE(klass, nullptr);

    auto method = env->GetStaticp_method(klass, "invokeException", ":V");
    ASSERT_NE(method, nullptr);

    testing::internal::CaptureStdout();

    env->CallStaticVoidMethod(klass, method);

    auto captured = testing::internal::GetCapturedStdout();
    auto pos = 0;

    pos = captured.find("Test message", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.throwing", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.nestedFunc2", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.nestedFunc1", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.invokeException", pos);
    ASSERT_NE(pos, std::string::npos);
}

TEST_F(StackTraceTest, ErrorDescribe)
{
    auto klass = env->FindClass("StackTraceTest");
    ASSERT_NE(klass, nullptr);

    auto method = env->GetStaticp_method(klass, "invokeUnhandledException", ":V");
    ASSERT_NE(method, nullptr);

    testing::internal::CaptureStdout();
    env->CallStaticVoidMethod(klass, method);

    env->ErrorDescribe();
    env->ErrorClear();

    auto captured = testing::internal::GetCapturedStdout();
    auto pos = 0;

    pos = captured.find("Test message", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.throwing", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.nestedFunc2", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.nestedFunc1", pos);
    ASSERT_NE(pos, std::string::npos);

    pos = captured.find("StackTraceTest.invokeUnhandledException", pos);
    ASSERT_NE(pos, std::string::npos);
}

}  // namespace ark::ets::test

// NOLINTEND(readability-identifier-naming, cppcoreguidelines-pro-type-vararg)
