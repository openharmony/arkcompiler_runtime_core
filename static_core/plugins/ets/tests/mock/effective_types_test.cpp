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

#include "libpandabase/utils/utils.h"
#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_class.h"

// NOLINTBEGIN(readability-identifier-naming, readability-named-parameter, readability-magic-numbers)

namespace ark::ets::test {

static const char *TEST_BIN_FILE_NAME = "EffectiveTypesTest.abc";

class EffectiveTypeTest : public testing::Test {
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

static ets_int effective_types_foo_I(EtsEnv *, ets_class, ets_int v)
{
    if (v == 1) {
        return 0;
    }
    return 99_I;
}

static ets_int effective_types_foo_D(EtsEnv *, ets_class, ets_double v)
{
    if (v == 1.0) {
        return 1;
    }
    return 99_I;
}

static ets_double effective_types_foo(EtsEnv *, ets_class)
{
    return 2.0F;
}

static ets_double effective_types_fooUnity(EtsEnv *, ets_class, ets_double v)
{
    if (v == 1.0) {
        return 3.0F;
    }
    return 99.0F;
}

TEST_F(EffectiveTypeTest, TestGetStaticMethod)
{
    ets_class testClass = env->FindClass("EffectiveTypes");
    ASSERT_NE(testClass, nullptr);
    std::array<EtsNativeMethod, 4U> native_methods {
        EtsNativeMethod {"foo", "I:I", reinterpret_cast<void *>(effective_types_foo_I)},
        EtsNativeMethod {"foo", "D:I", reinterpret_cast<void *>(effective_types_foo_D)},
        EtsNativeMethod {"foo", ":D", reinterpret_cast<void *>(effective_types_foo)},
        EtsNativeMethod {"fooUnity", "D:D", reinterpret_cast<void *>(effective_types_fooUnity)}};

    {
        ets_int ret_code = env->RegisterNatives(testClass, native_methods.data(), native_methods.size());
        ASSERT_EQ(ret_code, ETS_OK);
    }

    {
        ets_method method = env->GetStaticp_method(testClass, "main", ":I");
        ASSERT_NE(method, nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        auto res = env->CallStaticIntMethod(testClass, method);
        ASSERT_EQ(res, 0);
    }
}

}  // namespace ark::ets::test

// NOLINTEND(readability-identifier-naming, readability-named-parameter, readability-magic-numbers)
