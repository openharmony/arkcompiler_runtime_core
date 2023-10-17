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

// NOLINTBEGIN(readability-magic-numbers)

namespace panda::ets::test {

static const char *TEST_BIN_FILE_NAME = "CallingMethodsTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(TEST_BIN_FILE_NAME) {}
};

class MethodsTest : public CallingMethodsTestGeneral {};
class MethodsTestDeath : public CallingMethodsTestGeneral {};

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath7)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class c_cls = env_->FindClass("C");
    ASSERT_NE(c_cls, nullptr);
    ets_class d_cls = env_->FindClass("D");
    ASSERT_NE(d_cls, nullptr);
    ets_object obj = env_->AllocObject(d_cls);
    ASSERT_NE(obj, nullptr);

    ets_method void_id = env_->Getp_method(c_cls, "void_method", "II:Lstd/core/void;");
    ASSERT_NE(void_id, nullptr);
    ets_method object_id = env_->Getp_method(c_cls, "object_method", ":LA;");
    ASSERT_NE(object_id, nullptr);
    ets_method boolean_id = env_->Getp_method(c_cls, "boolean_method", "ZI:Z");
    ASSERT_NE(boolean_id, nullptr);
    ets_method byte_id = env_->Getp_method(c_cls, "byte_method", "BI:B");
    ASSERT_NE(byte_id, nullptr);
    ets_method char_id = env_->Getp_method(c_cls, "char_method", "CI:C");
    ASSERT_NE(char_id, nullptr);
    ets_method short_id = env_->Getp_method(c_cls, "short_method", "SI:S");
    ASSERT_NE(short_id, nullptr);
    ets_method int_id = env_->Getp_method(c_cls, "int_method", ":I");
    ASSERT_NE(int_id, nullptr);
    ets_method long_id = env_->Getp_method(c_cls, "long_method", "JI:J");
    ASSERT_NE(long_id, nullptr);
    ets_method float_id = env_->Getp_method(c_cls, "float_method", "FI:F");
    ASSERT_NE(float_id, nullptr);
    ets_method double_id = env_->Getp_method(c_cls, "double_method", "DI:D");
    ASSERT_NE(double_id, nullptr);

    // CallNonvirtual<Type>MethodArray part
    ets_value int_tmp;
    int_tmp.i = 121;
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42);
    const std::vector<ets_value> void_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallNonvirtualVoidMethodArray(nullptr, c_cls, void_id, void_args.data()), "");

    EXPECT_DEATH(env_->CallNonvirtualObjectMethodArray(nullptr, c_cls, object_id, nullptr), "");

    tmp.z = static_cast<ets_boolean>(1);
    const std::vector<ets_value> boolean_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethodArray(nullptr, c_cls, boolean_id, boolean_args.data()), "");

    tmp.b = static_cast<ets_byte>(1);
    const std::vector<ets_value> byte_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallNonvirtualByteMethodArray(nullptr, c_cls, byte_id, byte_args.data()), "");

    tmp.c = static_cast<ets_char>(1);
    const std::vector<ets_value> char_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallNonvirtualCharMethodArray(nullptr, c_cls, char_id, char_args.data()), "");

    tmp.s = static_cast<ets_short>(1);
    const std::vector<ets_value> short_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallNonvirtualShortMethodArray(nullptr, c_cls, short_id, short_args.data()), "");

    EXPECT_DEATH(env_->CallNonvirtualIntMethodArray(nullptr, c_cls, int_id, nullptr), "");

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> long_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallNonvirtualLongMethodArray(nullptr, c_cls, long_id, long_args.data()), "");

    tmp.j = static_cast<ets_float>(1.0F);
    const std::vector<ets_value> float_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallNonvirtualFloatMethodArray(nullptr, c_cls, float_id, float_args.data()), "");

    tmp.j = static_cast<ets_double>(1.0);
    const std::vector<ets_value> double_args = {tmp, int_tmp};
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethodArray(nullptr, c_cls, double_id, double_args.data()), "");

    EXPECT_DEATH(env_->CallNonvirtualVoidMethodArray(nullptr, c_cls, void_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualObjectMethodArray(nullptr, c_cls, object_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethodArray(nullptr, c_cls, boolean_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualByteMethodArray(nullptr, c_cls, byte_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualCharMethodArray(nullptr, c_cls, char_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualShortMethodArray(nullptr, c_cls, short_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualIntMethodArray(nullptr, c_cls, int_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualLongMethodArray(nullptr, c_cls, long_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualFloatMethodArray(nullptr, c_cls, float_id, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethodArray(nullptr, c_cls, double_id, nullptr), "");

    EXPECT_DEATH(env_->CallNonvirtualVoidMethodArray(obj, c_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualObjectMethodArray(obj, c_cls, nullptr, long_args.data()), "");
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethodArray(obj, c_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualByteMethodArray(obj, c_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualCharMethodArray(obj, c_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualShortMethodArray(obj, c_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualIntMethodArray(obj, c_cls, nullptr, long_args.data()), "");
    EXPECT_DEATH(env_->CallNonvirtualLongMethodArray(obj, c_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualFloatMethodArray(obj, c_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethodArray(obj, c_cls, nullptr, nullptr), "");
}

}  // namespace panda::ets::test

// NOLINTEND(readability-magic-numbers)