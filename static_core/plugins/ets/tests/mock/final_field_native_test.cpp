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

// NOLINTBEGIN(readability-identifier-naming)

namespace ark::ets::test {

static const char *TEST_BIN_FILE_NAME = "FinalFieldNativeTest.abc";

class FinalFieldNativeTest : public testing::Test {
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

TEST_F(FinalFieldNativeTest, FinalFieldTest)
{
    ets_class testClass = env->FindClass("FinalFieldNativeTest/FinalFieldNativeTest");
    ASSERT_NE(testClass, nullptr);

    ets_field field = env->GetStaticp_field(testClass, "boolean_value_final", "Z");
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env->GetStaticBooleanField(testClass, field), ETS_TRUE);

    field = env->GetStaticp_field(testClass, "byte_value_final", "B");
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env->GetStaticByteField(testClass, field), 2_I);

    field = env->GetStaticp_field(testClass, "short_value_final", "S");
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env->GetStaticShortField(testClass, field), 256_I);

    field = env->GetStaticp_field(testClass, "char_value_final", "C");
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env->GetStaticCharField(testClass, field), 32767U);

    field = env->GetStaticp_field(testClass, "int_value_final", "I");
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env->GetStaticIntField(testClass, field), 65536_I);

    field = env->GetStaticp_field(testClass, "long_value_final", "J");
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env->GetStaticLongField(testClass, field), 4294967296L);

    field = env->GetStaticp_field(testClass, "float_value_final", "F");
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env->GetStaticFloatField(testClass, field), 1.0F);

    field = env->GetStaticp_field(testClass, "double_value_final", "D");
    ASSERT_NE(field, nullptr);
    ASSERT_EQ(env->GetStaticDoubleField(testClass, field), 2.0_D);

    field = env->GetStaticp_field(testClass, "string_value_final", "Lstd/core/String;");
    ASSERT_NE(field, nullptr);

    auto str = static_cast<ets_string>(env->GetStaticObjectField(testClass, field));
    const char *bytes = env->GetStringUTFChars(str, nullptr);

    ASSERT_STREQ(bytes, "abc");
    env->ReleaseStringUTFChars(str, bytes);
}

}  // namespace ark::ets::test

// NOLINTEND(readability-identifier-naming)
