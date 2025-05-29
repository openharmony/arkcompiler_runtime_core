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

#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_class.h"

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-pro-type-vararg, readability-magic-numbers)

namespace ark::ets::test {

static const char *TEST_BIN_FILE_NAME = "StackReferenceCheckTest.abc";

class StackReferenceCheckTest : public testing::Test {
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

static ets_boolean StrCheck(EtsEnv *env, ets_string ets_str)
{
    return env->GetObjectRefType(static_cast<ets_object>(ets_str)) == ETS_LOCAL_REF_TYPE ? ETS_TRUE : ETS_FALSE;
}

TEST_F(StackReferenceCheckTest, SingleObjectStackTest)
{
    ets_string ets_str = env->NewStringUTF("Hello world");
    ASSERT_NE(ets_str, nullptr);

    ets_class cls_A = env->FindClass("StackReferenceCheckTest/StackReferenceCheckTest");
    ASSERT_NE(cls_A, nullptr);

    std::array<EtsNativeMethod, 1> native_methods {
        EtsNativeMethod {"NativeRefStringCheck", "Lstd/core/String;:Z", reinterpret_cast<void *>(StrCheck)}};

    {
        ets_int ret_code = env->RegisterNatives(cls_A, native_methods.data(), native_methods.size());
        ASSERT_EQ(ret_code, ETS_OK);
    }

    {
        ets_method method = env->GetStaticp_method(cls_A, "NativeRefStringCheck", "Lstd/core/String;:Z");
        ASSERT_NE(method, nullptr);

        ets_boolean res_true = env->CallStaticBooleanMethod(cls_A, method, ets_str);
        ASSERT_EQ(res_true, ETS_TRUE);
    }

    {
        ets_int ret_code = env->UnregisterNatives(cls_A);
        ASSERT_EQ(ret_code, ETS_OK);
    }
}

static ets_boolean MultiStrCheck(EtsEnv *env, ets_string ets_str1, ets_string ets_str2, ets_string ets_str3,
                                 ets_string ets_str4)
{
    bool res1 = env->GetObjectRefType(static_cast<ets_object>(ets_str1)) == ETS_LOCAL_REF_TYPE;
    bool res2 = env->GetObjectRefType(static_cast<ets_object>(ets_str2)) == ETS_LOCAL_REF_TYPE;
    bool res3 = env->GetObjectRefType(static_cast<ets_object>(ets_str3)) == ETS_LOCAL_REF_TYPE;
    bool res4 = env->GetObjectRefType(static_cast<ets_object>(ets_str4)) == ETS_LOCAL_REF_TYPE;
    return (res1 && res2 && res3 && res4) ? ETS_TRUE : ETS_FALSE;
}

TEST_F(StackReferenceCheckTest, MultiObjectStackTest)
{
    ets_string ets_str1 = env->NewStringUTF("Hello world");
    ASSERT_NE(ets_str1, nullptr);

    ets_string ets_str2 = env->NewStringUTF("Hello world");
    ASSERT_NE(ets_str2, nullptr);

    ets_string ets_str3 = env->NewStringUTF("Hello world");
    ASSERT_NE(ets_str3, nullptr);

    ets_string ets_str4 = env->NewStringUTF("Hello world");
    ASSERT_NE(ets_str4, nullptr);

    ets_class cls_A = env->FindClass("StackReferenceCheckTest/StackReferenceCheckTest");
    ASSERT_NE(cls_A, nullptr);

    const std::string signature("Lstd/core/String;Lstd/core/String;Lstd/core/String;Lstd/core/String;:Z");
    std::array<EtsNativeMethod, 1> native_methods {
        EtsNativeMethod {"NativeRefMultiStringCheck", signature.c_str(), reinterpret_cast<void *>(MultiStrCheck)}};

    {
        ets_int ret_code = env->RegisterNatives(cls_A, native_methods.data(), native_methods.size());
        ASSERT_EQ(ret_code, ETS_OK);
    }

    {
        ets_method method = env->GetStaticp_method(cls_A, "NativeRefMultiStringCheck", signature.c_str());
        ASSERT_NE(method, nullptr);

        ets_boolean res_true = env->CallStaticBooleanMethod(cls_A, method, ets_str1, ets_str2, ets_str3, ets_str4);
        ASSERT_EQ(res_true, ETS_TRUE);
    }

    {
        ets_int ret_code = env->UnregisterNatives(cls_A);
        ASSERT_EQ(ret_code, ETS_OK);
    }
}

TEST_F(StackReferenceCheckTest, invalidObjectStackTest)
{
    int a = 42;
    bool invalidRef = env->GetObjectRefType(reinterpret_cast<ets_string>(&a)) == ETS_INVALID_REF_TYPE;
    ASSERT_EQ(invalidRef, true);
}

}  // namespace ark::ets::test

// NOLINTEND(readability-identifier-naming, cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
