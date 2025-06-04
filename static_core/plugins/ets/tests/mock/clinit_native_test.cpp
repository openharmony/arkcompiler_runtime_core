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
#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_class.h"

// NOLINTBEGIN(readability-identifier-naming, readability-magic-numbers)

namespace ark::ets::test {

static const char *TEST_BIN_FILE_NAME = "ClinitNativeTest.abc";

class ClinitNativeTest : public testing::Test {
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

static bool IsInitialized(EtsEnv *env, ets_class cls)
{
    ark::ets::napi::ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = s.ToInternalType(cls);
    if (!internal_class->IsInitialized()) {
        return false;
    }

    auto *field_id = internal_class->GetStaticFieldIDByName("value", "I");
    auto value = internal_class->GetStaticFieldPrimitive<int32_t>(field_id);
    return value == 10_I;
}

TEST_F(ClinitNativeTest, TestGetStaticMethod)
{
    ets_class testClass = env->FindClass("ClinitNativeTest/ClinitNativeTest");
    ASSERT_NE(testClass, nullptr);
    ASSERT_FALSE(IsInitialized(env, testClass));

    ets_method method = env->GetStaticp_method(testClass, "smethod", ":I");
    ASSERT_NE(method, nullptr);
    ASSERT_TRUE(IsInitialized(env, testClass));
}

TEST_F(ClinitNativeTest, TestGetMethod)
{
    ets_class testClass = env->FindClass("ClinitNativeTest/ClinitNativeTest");
    ASSERT_NE(testClass, nullptr);
    ASSERT_FALSE(IsInitialized(env, testClass));

    ets_method method = env->Getp_method(testClass, "imethod", ":I");
    ASSERT_NE(method, nullptr);
    ASSERT_TRUE(IsInitialized(env, testClass));
}

TEST_F(ClinitNativeTest, TestGetStaticField)
{
    ets_class testClass = env->FindClass("ClinitNativeTest/ClinitNativeTest");
    ASSERT_NE(testClass, nullptr);
    ASSERT_FALSE(IsInitialized(env, testClass));

    ets_field field = env->GetStaticp_field(testClass, "sfield", "I");
    ASSERT_NE(field, nullptr);
    ASSERT_TRUE(IsInitialized(env, testClass));
}

TEST_F(ClinitNativeTest, TestGetField)
{
    ets_class testClass = env->FindClass("ClinitNativeTest/ClinitNativeTest");
    ASSERT_NE(testClass, nullptr);
    ASSERT_FALSE(IsInitialized(env, testClass));

    ets_field field = env->Getp_field(testClass, "ifield", "I");
    ASSERT_NE(field, nullptr);
    ASSERT_TRUE(IsInitialized(env, testClass));
}

TEST_F(ClinitNativeTest, TestAllocObject)
{
    ets_class testClass = env->FindClass("ClinitNativeTest/ClinitNativeTest");
    ASSERT_NE(testClass, nullptr);
    ASSERT_FALSE(IsInitialized(env, testClass));

    ets_object obj = env->AllocObject(testClass);
    ASSERT_NE(obj, nullptr);
    ASSERT_TRUE(IsInitialized(env, testClass));
}

}  // namespace ark::ets::test

// NOLINTEND(readability-identifier-naming, readability-magic-numbers)
