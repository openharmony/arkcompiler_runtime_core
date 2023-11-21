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

TEST_F(MethodsTest, CallMethodsTestGeneral6)
{
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

    // CallNonvirtual<type>Method part
    env_->CallNonvirtualVoidMethod(obj, c_cls, void_id, static_cast<ets_int>(1), static_cast<ets_int>(121));
    ets_field d_member_id = env_->Getp_field(d_cls, "member", "I");
    ASSERT_NE(d_member_id, nullptr);
    EXPECT_EQ(env_->GetIntField(obj, d_member_id), static_cast<ets_int>(0));

    ets_class a_cls = env_->FindClass("A");
    ASSERT_NE(a_cls, nullptr);
    ets_object a_obj = env_->CallNonvirtualObjectMethod(obj, c_cls, object_id);
    ASSERT_NE(a_obj, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(a_obj, a_cls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(obj, d_member_id), static_cast<ets_int>(0));

    EXPECT_EQ(env_->CallNonvirtualBooleanMethod(obj, c_cls, boolean_id, static_cast<ets_boolean>(1),
                                                static_cast<ets_int>(121)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallNonvirtualByteMethod(obj, c_cls, byte_id, static_cast<ets_byte>(1), static_cast<ets_int>(121)),
              static_cast<ets_byte>(0));
    EXPECT_EQ(env_->CallNonvirtualCharMethod(obj, c_cls, char_id, static_cast<ets_char>(1), static_cast<ets_int>(121)),
              static_cast<ets_char>(0));
    EXPECT_EQ(
        env_->CallNonvirtualShortMethod(obj, c_cls, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121)),
        static_cast<ets_short>(0));
    EXPECT_EQ(env_->CallNonvirtualIntMethod(obj, c_cls, int_id), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallNonvirtualLongMethod(obj, c_cls, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121)),
              static_cast<ets_long>(0));
    EXPECT_EQ(
        env_->CallNonvirtualFloatMethod(obj, c_cls, float_id, static_cast<ets_float>(1.0F), static_cast<ets_int>(121)),
        static_cast<ets_float>(0.0F));
    EXPECT_EQ(env_->CallNonvirtualDoubleMethod(obj, c_cls, double_id, static_cast<ets_double>(1.0),
                                               static_cast<ets_int>(121)),
              static_cast<ets_double>(0.0));

    // CallNonvirtual<Type>MethodArray part
    ets_value int_tmp;
    int_tmp.i = 121;
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42);
    const std::vector<ets_value> void_args = {tmp, int_tmp};
    env_->CallNonvirtualVoidMethodArray(obj, c_cls, void_id, void_args.data());
    EXPECT_EQ(env_->GetIntField(obj, d_member_id), static_cast<ets_int>(0));

    ets_object a_obj_from_array_func = env_->CallNonvirtualObjectMethodArray(obj, c_cls, object_id, nullptr);
    ASSERT_NE(a_obj_from_array_func, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(a_obj_from_array_func, a_cls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(obj, d_member_id), static_cast<ets_int>(0));

    tmp.z = static_cast<ets_boolean>(1);
    const std::vector<ets_value> boolean_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallNonvirtualBooleanMethodArray(obj, c_cls, boolean_id, boolean_args.data()),
              static_cast<ets_boolean>(0));

    tmp.b = static_cast<ets_byte>(1);
    const std::vector<ets_value> byte_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallNonvirtualByteMethodArray(obj, c_cls, byte_id, byte_args.data()), static_cast<ets_byte>(0));

    tmp.c = static_cast<ets_char>(1);
    const std::vector<ets_value> char_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallNonvirtualCharMethodArray(obj, c_cls, char_id, char_args.data()), static_cast<ets_char>(0));

    tmp.s = static_cast<ets_short>(1);
    const std::vector<ets_value> short_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallNonvirtualShortMethodArray(obj, c_cls, short_id, short_args.data()), static_cast<ets_short>(0));

    EXPECT_EQ(env_->CallNonvirtualIntMethodArray(obj, c_cls, int_id, nullptr), static_cast<ets_int>(0));

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> long_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallNonvirtualLongMethodArray(obj, c_cls, long_id, long_args.data()), static_cast<ets_long>(0));

    tmp.j = static_cast<ets_float>(1.0F);
    const std::vector<ets_value> float_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallNonvirtualFloatMethodArray(obj, c_cls, float_id, float_args.data()),
              static_cast<ets_float>(0.0F));

    tmp.j = static_cast<ets_double>(1.0);
    const std::vector<ets_value> double_args = {tmp, int_tmp};
    EXPECT_EQ(env_->CallNonvirtualDoubleMethodArray(obj, c_cls, double_id, double_args.data()),
              static_cast<ets_double>(0.0));

    // CallNonvirtual<type>MethodList part
    CallNonvirtualVoidMethodListHelper(env_, obj, c_cls, void_id, static_cast<ets_int>(1), static_cast<ets_int>(121));
    EXPECT_EQ(env_->GetIntField(obj, d_member_id), static_cast<ets_int>(0));

    ets_object a_obj_from_list_func = CallNonvirtualObjectMethodListHelper(env_, obj, c_cls, object_id);
    ASSERT_NE(a_obj_from_list_func, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(a_obj_from_list_func, a_cls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(obj, d_member_id), static_cast<ets_int>(0));

    EXPECT_EQ(CallNonvirtualBooleanMethodListHelper(env_, obj, c_cls, boolean_id, static_cast<ets_boolean>(1),
                                                    static_cast<ets_int>(121)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(CallNonvirtualByteMethodListHelper(env_, obj, c_cls, byte_id, static_cast<ets_byte>(1),
                                                 static_cast<ets_int>(121)),
              static_cast<ets_byte>(0));
    EXPECT_EQ(CallNonvirtualCharMethodListHelper(env_, obj, c_cls, char_id, static_cast<ets_char>(1),
                                                 static_cast<ets_int>(121)),
              static_cast<ets_char>(0));
    EXPECT_EQ(CallNonvirtualShortMethodListHelper(env_, obj, c_cls, short_id, static_cast<ets_short>(1),
                                                  static_cast<ets_int>(121)),
              static_cast<ets_short>(0));
    EXPECT_EQ(CallNonvirtualIntMethodListHelper(env_, obj, c_cls, int_id), static_cast<ets_int>(0));
    EXPECT_EQ(CallNonvirtualLongMethodListHelper(env_, obj, c_cls, long_id, static_cast<ets_long>(1),
                                                 static_cast<ets_int>(121)),
              static_cast<ets_long>(0));
    EXPECT_EQ(CallNonvirtualFloatMethodListHelper(env_, obj, c_cls, float_id, static_cast<ets_float>(1.0F),
                                                  static_cast<ets_int>(121)),
              static_cast<ets_float>(0.0F));
    EXPECT_EQ(CallNonvirtualDoubleMethodListHelper(env_, obj, c_cls, double_id, static_cast<ets_double>(1.0),
                                                   static_cast<ets_int>(121)),
              static_cast<ets_double>(0.0));

    // Check class null argument
    EXPECT_EQ(env_->CallNonvirtualBooleanMethod(obj, nullptr, boolean_id, static_cast<ets_boolean>(1),
                                                static_cast<ets_int>(121)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(
        env_->CallNonvirtualByteMethod(obj, nullptr, byte_id, static_cast<ets_byte>(1), static_cast<ets_int>(121)),
        static_cast<ets_byte>(0));
    EXPECT_EQ(
        env_->CallNonvirtualCharMethod(obj, nullptr, char_id, static_cast<ets_char>(1), static_cast<ets_int>(121)),
        static_cast<ets_char>(0));
    EXPECT_EQ(
        env_->CallNonvirtualShortMethod(obj, nullptr, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121)),
        static_cast<ets_short>(0));
    EXPECT_EQ(env_->CallNonvirtualIntMethod(obj, nullptr, int_id), static_cast<ets_int>(0));
    EXPECT_EQ(
        env_->CallNonvirtualLongMethod(obj, nullptr, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121)),
        static_cast<ets_long>(0));
    EXPECT_EQ(env_->CallNonvirtualFloatMethod(obj, nullptr, float_id, static_cast<ets_float>(1.0F),
                                              static_cast<ets_int>(121)),
              static_cast<ets_float>(0.0F));
    EXPECT_EQ(env_->CallNonvirtualDoubleMethod(obj, nullptr, double_id, static_cast<ets_double>(1.0),
                                               static_cast<ets_int>(121)),
              static_cast<ets_double>(0.0));

    EXPECT_EQ(env_->CallNonvirtualBooleanMethodArray(obj, nullptr, boolean_id, boolean_args.data()),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallNonvirtualByteMethodArray(obj, nullptr, byte_id, byte_args.data()), static_cast<ets_byte>(0));
    EXPECT_EQ(env_->CallNonvirtualCharMethodArray(obj, nullptr, char_id, char_args.data()), static_cast<ets_char>(0));
    EXPECT_EQ(env_->CallNonvirtualShortMethodArray(obj, nullptr, short_id, short_args.data()),
              static_cast<ets_short>(0));
    EXPECT_EQ(env_->CallNonvirtualIntMethodArray(obj, nullptr, int_id, nullptr), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallNonvirtualLongMethodArray(obj, nullptr, long_id, long_args.data()), static_cast<ets_long>(0));
    EXPECT_EQ(env_->CallNonvirtualFloatMethodArray(obj, nullptr, float_id, float_args.data()),
              static_cast<ets_float>(0.0F));
    EXPECT_EQ(env_->CallNonvirtualDoubleMethodArray(obj, nullptr, double_id, double_args.data()),
              static_cast<ets_double>(0.0));

    EXPECT_EQ(CallNonvirtualBooleanMethodListHelper(env_, obj, nullptr, boolean_id, static_cast<ets_boolean>(1),
                                                    static_cast<ets_int>(121)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(CallNonvirtualByteMethodListHelper(env_, obj, nullptr, byte_id, static_cast<ets_byte>(1),
                                                 static_cast<ets_int>(121)),
              static_cast<ets_byte>(0));
    EXPECT_EQ(CallNonvirtualCharMethodListHelper(env_, obj, nullptr, char_id, static_cast<ets_char>(1),
                                                 static_cast<ets_int>(121)),
              static_cast<ets_char>(0));
    EXPECT_EQ(CallNonvirtualShortMethodListHelper(env_, obj, nullptr, short_id, static_cast<ets_short>(1),
                                                  static_cast<ets_int>(121)),
              static_cast<ets_short>(0));
    EXPECT_EQ(CallNonvirtualIntMethodListHelper(env_, obj, nullptr, int_id), static_cast<ets_int>(0));
    EXPECT_EQ(CallNonvirtualLongMethodListHelper(env_, obj, nullptr, long_id, static_cast<ets_long>(1),
                                                 static_cast<ets_int>(121)),
              static_cast<ets_long>(0));
    EXPECT_EQ(CallNonvirtualFloatMethodListHelper(env_, obj, nullptr, float_id, static_cast<ets_float>(1.0F),
                                                  static_cast<ets_int>(121)),
              static_cast<ets_float>(0.0F));
    EXPECT_EQ(CallNonvirtualDoubleMethodListHelper(env_, obj, nullptr, double_id, static_cast<ets_double>(1.0),
                                                   static_cast<ets_int>(121)),
              static_cast<ets_double>(0.0));

    // NOTE(m.morozov): Uncomment this test, when virtual calls will be implemented
    /*
    // Call<type>Method part
    env->CallVoidMethod(obj, void_id, static_cast<ets_int>(11), static_cast<ets_int>(121));
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(11));

    ets_object a_obj_from_direct_calls = env->CallObjectMethod(obj, object_id);
    ASSERT_NE(a_obj_from_direct_calls, nullptr);
    EXPECT_EQ(env->IsInstanceOf(a_obj_from_direct_calls, a_cls), ETS_TRUE);
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(42));

    EXPECT_EQ(env->CallBooleanMethod(obj, boolean_id, static_cast<ets_boolean>(1), static_cast<ets_int>(121)),
    static_cast<ets_boolean>(1)); EXPECT_EQ(env->CallByteMethod(obj, byte_id, static_cast<ets_byte>(1),
    static_cast<ets_int>(121)), static_cast<ets_byte>(1)); EXPECT_EQ(env->CallCharMethod(obj, char_id,
    static_cast<ets_char>(1), static_cast<ets_int>(121)), static_cast<ets_char>(1)); EXPECT_EQ(env->CallShortMethod(obj,
    short_id, static_cast<ets_short>(1), static_cast<ets_int>(121)), static_cast<ets_short>(1));
    EXPECT_EQ(env->CallIntMethod(obj, int_id), static_cast<ets_int>(1));
    EXPECT_EQ(env->CallLongMethod(obj, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121)),
    static_cast<ets_long>(1)); EXPECT_EQ(env->CallFloatMethod(obj, float_id, static_cast<ets_float>(1.0F),
    static_cast<ets_int>(121)), static_cast<ets_float>(1.0F)); EXPECT_EQ(env->CallDoubleMethod(obj, double_id,
    static_cast<ets_double>(1.0), static_cast<ets_int>(121)), static_cast<ets_double>(1.0));

    // Call<type>MethodArray part
    env->CallVoidMethodArray(obj, void_id, void_args.data());
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(42));

    ets_object a_obj_from_Array_direct_calls = env->CallObjectMethodArray(obj, object_id, nullptr);
    ASSERT_NE(a_obj_from_Array_direct_calls, nullptr);
    EXPECT_EQ(env->IsInstanceOf(a_obj_from_Array_direct_calls, a_cls), ETS_TRUE);
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(42));

    EXPECT_EQ(env->CallBooleanMethodArray(obj, boolean_id, boolean_args.data()), static_cast<ets_boolean>(1));
    EXPECT_EQ(env->CallByteMethodArray(obj, byte_id, byte_args.data()), static_cast<ets_byte>(1));
    EXPECT_EQ(env->CallCharMethodArray(obj, char_id, char_args.data()), static_cast<ets_char>(1));
    EXPECT_EQ(env->CallShortMethodArray(obj, short_id, short_args.data()), static_cast<ets_short>(1));
    EXPECT_EQ(env->CallIntMethodArray(obj, int_id, nullptr), static_cast<ets_int>(1));
    EXPECT_EQ(env->CallLongMethodArray(obj, long_id, long_args.data()), static_cast<ets_long>(1));
    EXPECT_EQ(env->CallFloatMethodArray(obj, float_id, float_args.data()), static_cast<ets_float>(1.0F));
    EXPECT_EQ(env->CallDoubleMethodArray(obj, double_id, double_args.data()), static_cast<ets_double>(1.0));

    // Call<type>MethodList part
    CallVoidMethodListHelper(env, obj, void_id, static_cast<ets_int>(84), static_cast<ets_int>(121));
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(84));

    ets_object a_obj_from_List_direct_calls = CallObjectMethodListHelper(env, obj, object_id);
    ASSERT_NE(a_obj_from_List_direct_calls, nullptr);
    EXPECT_EQ(env->IsInstanceOf(a_obj_from_List_direct_calls, a_cls), ETS_TRUE);
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(42));

    EXPECT_EQ(CallBooleanMethodListHelper(env, obj, boolean_id, static_cast<ets_boolean>(1), static_cast<ets_int>(121)),
    static_cast<ets_boolean>(1)); EXPECT_EQ(CallByteMethodListHelper(env, obj, byte_id, static_cast<ets_byte>(1),
    static_cast<ets_int>(121)), static_cast<ets_byte>(1)); EXPECT_EQ(CallCharMethodListHelper(env, obj, char_id,
    static_cast<ets_char>(1), static_cast<ets_int>(121)), static_cast<ets_char>(1));
    EXPECT_EQ(CallShortMethodListHelper(env, obj, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121)),
    static_cast<ets_short>(1)); EXPECT_EQ(CallIntMethodListHelper(env, obj, int_id), static_cast<ets_int>(1));
    EXPECT_EQ(CallLongMethodListHelper(env, obj, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121)),
    static_cast<ets_long>(1)); EXPECT_EQ(CallFloatMethodListHelper(env, obj, float_id, static_cast<ets_float>(1.0F),
    static_cast<ets_int>(121)), static_cast<ets_float>(1.0F)); EXPECT_EQ(CallDoubleMethodListHelper(env, obj, double_id,
    static_cast<ets_double>(1.0), static_cast<ets_int>(121)), static_cast<ets_double>(1.0));
    */
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath6)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    // Static methods part
    EXPECT_DEATH(env_->GetStaticp_method(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->GetStaticp_method(nullptr, "foo", nullptr), "");
    EXPECT_DEATH(env_->GetStaticp_method(nullptr, nullptr, "I:I"), "");
    EXPECT_DEATH(env_->GetStaticp_method(nullptr, "foo", "I:I"), "");

    ets_class a_cls = env_->FindClass("A");
    ASSERT_NE(a_cls, nullptr);
    EXPECT_DEATH(env_->GetStaticp_method(a_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->GetStaticp_method(a_cls, nullptr, "I:I"), "");

    // Non static methods part
    EXPECT_DEATH(env_->Getp_method(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->Getp_method(nullptr, "foo", nullptr), "");
    EXPECT_DEATH(env_->Getp_method(nullptr, nullptr, "I:I"), "");
    EXPECT_DEATH(env_->Getp_method(nullptr, "foo", "I:I"), "");

    EXPECT_DEATH(env_->Getp_method(a_cls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->Getp_method(a_cls, nullptr, "I:I"), "");
}

}  // namespace panda::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)