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

namespace panda::ets::test {

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-pro-type-vararg, readability-magic-numbers)

static const char TEST_BIN_FILE_NAME[] = "CallingMethodsTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(TEST_BIN_FILE_NAME) {}
};

class MethodsTest : public CallingMethodsTestGeneral {};
class MethodsTestDeath : public CallingMethodsTestGeneral {};

TEST_F(MethodsTest, CallMethodsTestGeneral2)
{
    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);
    ets_object obj = env_->AllocObject(cls);
    ASSERT_NE(obj, nullptr);

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

    ets_method void_id_no_sig = env_->Getp_method(cls, "void_method", nullptr);
    ASSERT_EQ(void_id, void_id_no_sig);
    ets_method object_id_no_sig = env_->Getp_method(cls, "object_method", nullptr);
    ASSERT_EQ(object_id, object_id_no_sig);
    ets_method boolean_id_no_sig = env_->Getp_method(cls, "boolean_method", nullptr);
    ASSERT_EQ(boolean_id, boolean_id_no_sig);
    ets_method byte_id_no_sig = env_->Getp_method(cls, "byte_method", nullptr);
    ASSERT_EQ(byte_id, byte_id_no_sig);
    ets_method char_id_no_sig = env_->Getp_method(cls, "char_method", nullptr);
    ASSERT_EQ(char_id, char_id_no_sig);
    ets_method short_id_no_sig = env_->Getp_method(cls, "short_method", nullptr);
    ASSERT_EQ(short_id, short_id_no_sig);
    ets_method int_id_no_sig = env_->Getp_method(cls, "int_method", nullptr);
    ASSERT_EQ(int_id, int_id_no_sig);
    ets_method long_id_no_sig = env_->Getp_method(cls, "long_method", nullptr);
    ASSERT_EQ(long_id, long_id_no_sig);
    ets_method float_id_no_sig = env_->Getp_method(cls, "float_method", nullptr);
    ASSERT_EQ(float_id, float_id_no_sig);
    ets_method double_id_no_sig = env_->Getp_method(cls, "double_method", nullptr);
    ASSERT_EQ(double_id, double_id_no_sig);

    // Call<Type>Method part
    env_->CallVoidMethod(obj, void_id, static_cast<ets_int>(42), static_cast<ets_int>(121));

    ets_class a_cls = env_->FindClass("A");
    ASSERT_NE(a_cls, nullptr);
    ets_object a_obj = env_->CallObjectMethod(obj, object_id);
    ASSERT_NE(a_obj, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(a_obj, a_cls), ETS_TRUE);

    EXPECT_EQ(env_->CallBooleanMethod(obj, boolean_id, static_cast<ets_boolean>(1), static_cast<ets_int>(121)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallByteMethod(obj, byte_id, static_cast<ets_byte>(1), static_cast<ets_int>(121)),
              static_cast<ets_byte>(0));
    EXPECT_EQ(env_->CallCharMethod(obj, char_id, static_cast<ets_char>(1), static_cast<ets_int>(121)),
              static_cast<ets_char>(0));
    EXPECT_EQ(env_->CallShortMethod(obj, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121)),
              static_cast<ets_short>(0));
    EXPECT_EQ(env_->CallIntMethod(obj, int_id), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallLongMethod(obj, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121)),
              static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(env_->CallFloatMethod(obj, float_id, static_cast<ets_float>(1.0F), static_cast<ets_int>(121)),
                    static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(env_->CallDoubleMethod(obj, double_id, static_cast<ets_double>(1.0), static_cast<ets_int>(121)),
                     static_cast<ets_double>(0.0));

    // Call<Type>MethodArray part
    ets_value int_tmp;
    int_tmp.i = 121;
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42);
    const std::vector<ets_value> void_args = {tmp, int_tmp};
    env_->CallVoidMethodArray(obj, void_id, void_args.data());

    ets_object a_obj_from_array_func = env_->CallObjectMethodArray(obj, object_id, nullptr);
    ASSERT_NE(a_obj_from_array_func, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(a_obj_from_array_func, a_cls), ETS_TRUE);

    tmp.z = static_cast<ets_boolean>(1);
    const std::vector<ets_value> boolean_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallBooleanMethodArray(obj, boolean_id, boolean_args.data()), static_cast<ets_boolean>(0));

    tmp.b = static_cast<ets_byte>(1);
    const std::vector<ets_value> byte_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallByteMethodArray(obj, byte_id, byte_args.data()), static_cast<ets_byte>(0));

    tmp.c = static_cast<ets_char>(1);
    const std::vector<ets_value> char_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallCharMethodArray(obj, char_id, char_args.data()), static_cast<ets_char>(0));

    tmp.s = static_cast<ets_short>(1);
    const std::vector<ets_value> short_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallShortMethodArray(obj, short_id, short_args.data()), static_cast<ets_short>(0));

    EXPECT_EQ(env_->CallIntMethodArray(obj, int_id, nullptr), static_cast<ets_int>(0));

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> long_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallLongMethodArray(obj, long_id, long_args.data()), static_cast<ets_long>(0));

    tmp.j = static_cast<ets_float>(1.0F);
    const std::vector<ets_value> float_args = {tmp, int_tmp};
    EXPECT_FLOAT_EQ(env_->CallFloatMethodArray(obj, float_id, float_args.data()), static_cast<ets_float>(0.0F));

    tmp.j = static_cast<ets_double>(1.0);
    const std::vector<ets_value> double_args = {tmp, int_tmp};
    EXPECT_DOUBLE_EQ(env_->CallDoubleMethodArray(obj, double_id, double_args.data()), static_cast<ets_double>(0.0));

    // Call<type>MethodList part
    CallVoidMethodListHelper(env_, obj, void_id, static_cast<ets_int>(42), static_cast<ets_int>(121));

    ets_object a_obj_from_list_func = CallObjectMethodListHelper(env_, obj, object_id);
    ASSERT_NE(a_obj_from_list_func, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(a_obj_from_list_func, a_cls), ETS_TRUE);

    EXPECT_EQ(
        CallBooleanMethodListHelper(env_, obj, boolean_id, static_cast<ets_boolean>(1), static_cast<ets_int>(121)),
        static_cast<ets_boolean>(0));
    EXPECT_EQ(CallByteMethodListHelper(env_, obj, byte_id, static_cast<ets_byte>(1), static_cast<ets_int>(121)),
              static_cast<ets_byte>(0));
    EXPECT_EQ(CallCharMethodListHelper(env_, obj, char_id, static_cast<ets_char>(1), static_cast<ets_int>(121)),
              static_cast<ets_char>(0));
    EXPECT_EQ(CallShortMethodListHelper(env_, obj, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121)),
              static_cast<ets_short>(0));
    EXPECT_EQ(CallIntMethodListHelper(env_, obj, int_id), static_cast<ets_int>(0));
    EXPECT_EQ(CallLongMethodListHelper(env_, obj, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121)),
              static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(
        CallFloatMethodListHelper(env_, obj, float_id, static_cast<ets_float>(1.0F), static_cast<ets_int>(121)),
        static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(
        CallDoubleMethodListHelper(env_, obj, double_id, static_cast<ets_double>(1.0), static_cast<ets_int>(121)),
        static_cast<ets_double>(0.0));
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath2)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        // Call<type>Method part
        EXPECT_DEATH(env_->CallNonvirtualVoidMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualObjectMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualBooleanMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualByteMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualCharMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualShortMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualIntMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualLongMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualFloatMethod(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualDoubleMethod(nullptr, nullptr, nullptr), "");

        // Call<type>MethodArray part
        EXPECT_DEATH(env_->CallNonvirtualVoidMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualObjectMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualBooleanMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualByteMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualCharMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualShortMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualIntMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualLongMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualFloatMethodArray(nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualDoubleMethodArray(nullptr, nullptr, nullptr, nullptr), "");

        // Call<type>MethodList part
        EXPECT_DEATH(CallNonvirtualVoidMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualObjectMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualBooleanMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualByteMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualCharMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualShortMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualIntMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualLongMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualFloatMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualDoubleMethodListHelper(env_, nullptr, nullptr, nullptr, nullptr), "");
    }

    {
        ets_class d_cls = env_->FindClass("D");
        ASSERT_NE(d_cls, nullptr);
        ets_object obj = env_->AllocObject(d_cls);
        ASSERT_NE(obj, nullptr);

        // Call<type>Method part
        EXPECT_DEATH(env_->CallNonvirtualVoidMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualObjectMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualBooleanMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualByteMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualCharMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualShortMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualIntMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualLongMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualFloatMethod(obj, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualDoubleMethod(obj, nullptr, nullptr), "");

        // Call<type>MethodArray part
        EXPECT_DEATH(env_->CallNonvirtualVoidMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualObjectMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualBooleanMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualByteMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualCharMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualShortMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualIntMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualLongMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualFloatMethodArray(obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallNonvirtualDoubleMethodArray(obj, nullptr, nullptr, nullptr), "");

        // Call<type>MethodList part
        EXPECT_DEATH(CallNonvirtualVoidMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualObjectMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualBooleanMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualByteMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualCharMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualShortMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualIntMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualLongMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualFloatMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallNonvirtualDoubleMethodListHelper(env_, obj, nullptr, nullptr, nullptr), "");
    }
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-pro-type-vararg, readability-magic-numbers)

}  // namespace panda::ets::test