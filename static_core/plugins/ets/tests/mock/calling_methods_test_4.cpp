/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "plugins/ets/tests/mock/calling_methods_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)

namespace panda::ets::test {

static const char *TEST_BIN_FILE_NAME = "CallingMethodsTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(TEST_BIN_FILE_NAME) {}
};

class MethodsTest : public CallingMethodsTestGeneral {};
class MethodsTestDeath : public CallingMethodsTestGeneral {};

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath4)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);

    ets_method void_id = env_->Getp_method(cls, "void_method", "II:Lstd/core/void;");
    ASSERT_NE(void_id, nullptr);
    ets_method object_id = env_->Getp_method(cls, "object_method", ":LA;");
    ASSERT_NE(object_id, nullptr);
    ets_method boolean_id = env_->Getp_method(cls, "boolean_method", "ZI:Z");
    ASSERT_NE(boolean_id, nullptr);
    ets_method byte_id = env_->Getp_method(cls, "byte_method", "BI:B");
    ASSERT_NE(byte_id, nullptr);
    ets_method char_id = env_->Getp_method(cls, "char_method", "CI:C");
    ASSERT_NE(char_id, nullptr);
    ets_method short_id = env_->Getp_method(cls, "short_method", "SI:S");
    ASSERT_NE(short_id, nullptr);
    ets_method int_id = env_->Getp_method(cls, "int_method", ":I");
    ASSERT_NE(int_id, nullptr);
    ets_method long_id = env_->Getp_method(cls, "long_method", "JI:J");
    ASSERT_NE(long_id, nullptr);
    ets_method float_id = env_->Getp_method(cls, "float_method", "FI:F");
    ASSERT_NE(float_id, nullptr);
    ets_method double_id = env_->Getp_method(cls, "double_method", "DI:D");
    ASSERT_NE(double_id, nullptr);

    // Call<Type>Method part
    EXPECT_DEATH(env_->CallVoidMethod(nullptr, void_id, static_cast<ets_int>(42), static_cast<ets_int>(121)), "");
    EXPECT_DEATH(env_->CallObjectMethod(nullptr, object_id), "");
    EXPECT_DEATH(env_->CallBooleanMethod(nullptr, boolean_id, static_cast<ets_boolean>(1), static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(env_->CallByteMethod(nullptr, byte_id, static_cast<ets_byte>(1), static_cast<ets_int>(121)), "");
    EXPECT_DEATH(env_->CallCharMethod(nullptr, char_id, static_cast<ets_char>(1), static_cast<ets_int>(121)), "");
    EXPECT_DEATH(env_->CallShortMethod(nullptr, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121)), "");
    EXPECT_DEATH(env_->CallIntMethod(nullptr, int_id), "");
    EXPECT_DEATH(env_->CallLongMethod(nullptr, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121)), "");
    EXPECT_DEATH(env_->CallFloatMethod(nullptr, float_id, static_cast<ets_float>(1.0F), static_cast<ets_int>(121)), "");
    EXPECT_DEATH(env_->CallDoubleMethod(nullptr, double_id, static_cast<ets_double>(1.0), static_cast<ets_int>(121)),
                 "");

    EXPECT_DEATH(env_->CallVoidMethod(nullptr, void_id), "");
    EXPECT_DEATH(env_->CallObjectMethod(nullptr, object_id), "");
    EXPECT_DEATH(env_->CallBooleanMethod(nullptr, boolean_id), "");
    EXPECT_DEATH(env_->CallByteMethod(nullptr, byte_id), "");
    EXPECT_DEATH(env_->CallCharMethod(nullptr, char_id), "");
    EXPECT_DEATH(env_->CallShortMethod(nullptr, short_id), "");
    EXPECT_DEATH(env_->CallIntMethod(nullptr, int_id), "");
    EXPECT_DEATH(env_->CallLongMethod(nullptr, long_id), "");
    EXPECT_DEATH(env_->CallFloatMethod(nullptr, float_id), "");
    EXPECT_DEATH(env_->CallDoubleMethod(nullptr, double_id), "");

    // Call<Type>MethodArray part
    ets_value int_tmp;
    int_tmp.i = 121;
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42);
    const std::vector<ets_value> void_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallVoidMethodArray(nullptr, void_id, void_args.data()), "");

    EXPECT_DEATH(env_->CallObjectMethodArray(nullptr, object_id, nullptr), "");

    tmp.z = static_cast<ets_boolean>(1);
    const std::vector<ets_value> boolean_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallBooleanMethodArray(nullptr, boolean_id, boolean_args.data()), "");

    tmp.b = static_cast<ets_byte>(1);
    const std::vector<ets_value> byte_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallByteMethodArray(nullptr, byte_id, byte_args.data()), "");

    tmp.c = static_cast<ets_char>(1);
    const std::vector<ets_value> char_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallCharMethodArray(nullptr, char_id, char_args.data()), "");

    tmp.s = static_cast<ets_short>(1);
    const std::vector<ets_value> short_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallShortMethodArray(nullptr, short_id, short_args.data()), "");

    EXPECT_DEATH(env_->CallIntMethodArray(nullptr, int_id, nullptr), "");

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> long_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallLongMethodArray(nullptr, long_id, long_args.data()), "");

    tmp.j = static_cast<ets_float>(1.0F);
    const std::vector<ets_value> float_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallFloatMethodArray(nullptr, float_id, float_args.data()), "");

    tmp.j = static_cast<ets_double>(1.0);
    const std::vector<ets_value> double_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallDoubleMethodArray(nullptr, double_id, double_args.data()), "");

    EXPECT_DEATH(env_->CallVoidMethodArray(nullptr, void_id, nullptr), "");
    EXPECT_DEATH(env_->CallObjectMethodArray(nullptr, object_id, nullptr), "");
    EXPECT_DEATH(env_->CallBooleanMethodArray(nullptr, boolean_id, nullptr), "");
    EXPECT_DEATH(env_->CallByteMethodArray(nullptr, byte_id, nullptr), "");
    EXPECT_DEATH(env_->CallCharMethodArray(nullptr, char_id, nullptr), "");
    EXPECT_DEATH(env_->CallShortMethodArray(nullptr, short_id, nullptr), "");
    EXPECT_DEATH(env_->CallIntMethodArray(nullptr, int_id, nullptr), "");
    EXPECT_DEATH(env_->CallLongMethodArray(nullptr, long_id, nullptr), "");
    EXPECT_DEATH(env_->CallFloatMethodArray(nullptr, float_id, nullptr), "");
    EXPECT_DEATH(env_->CallDoubleMethodArray(nullptr, double_id, nullptr), "");

    // Call<type>MethodList part
    EXPECT_DEATH(CallVoidMethodListHelper(env_, nullptr, void_id, static_cast<ets_int>(42), static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallObjectMethodListHelper(env_, nullptr, object_id), "");
    EXPECT_DEATH(
        CallBooleanMethodListHelper(env_, nullptr, boolean_id, static_cast<ets_boolean>(1), static_cast<ets_int>(121)),
        "");
    EXPECT_DEATH(CallByteMethodListHelper(env_, nullptr, byte_id, static_cast<ets_byte>(1), static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallCharMethodListHelper(env_, nullptr, char_id, static_cast<ets_char>(1), static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(
        CallShortMethodListHelper(env_, nullptr, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121)), "");
    EXPECT_DEATH(CallIntMethodListHelper(env_, nullptr, int_id), "");
    EXPECT_DEATH(CallLongMethodListHelper(env_, nullptr, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(
        CallFloatMethodListHelper(env_, nullptr, float_id, static_cast<ets_float>(1.0F), static_cast<ets_int>(121)),
        "");
    EXPECT_DEATH(
        CallDoubleMethodListHelper(env_, nullptr, double_id, static_cast<ets_double>(1.0), static_cast<ets_int>(121)),
        "");

    EXPECT_DEATH(CallVoidMethodListHelper(env_, nullptr, void_id), "");
    EXPECT_DEATH(CallObjectMethodListHelper(env_, nullptr, object_id), "");
    EXPECT_DEATH(CallBooleanMethodListHelper(env_, nullptr, boolean_id), "");
    EXPECT_DEATH(CallByteMethodListHelper(env_, nullptr, byte_id), "");
    EXPECT_DEATH(CallCharMethodListHelper(env_, nullptr, char_id), "");
    EXPECT_DEATH(CallShortMethodListHelper(env_, nullptr, short_id), "");
    EXPECT_DEATH(CallIntMethodListHelper(env_, nullptr, int_id), "");
    EXPECT_DEATH(CallLongMethodListHelper(env_, nullptr, long_id), "");
    EXPECT_DEATH(CallFloatMethodListHelper(env_, nullptr, float_id), "");
    EXPECT_DEATH(CallDoubleMethodListHelper(env_, nullptr, double_id), "");
}

}  // namespace panda::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)