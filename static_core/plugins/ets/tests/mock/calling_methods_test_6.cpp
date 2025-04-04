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

#include "plugins/ets/tests/mock/calling_methods_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)

namespace ark::ets::test {

static const char *g_testBinFileName = "CallingMethodsTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(g_testBinFileName) {}
};

class MethodsTest : public CallingMethodsTestGeneral {};
class MethodsTestDeath : public CallingMethodsTestGeneral {};

TEST_F(MethodsTest, CallMethodsTestGeneral14)
{
    ets_class cCls = env_->FindClass("CallingMethodsTest/C");
    ASSERT_NE(cCls, nullptr);
    ets_class dCls = env_->FindClass("CallingMethodsTest/D");
    ASSERT_NE(dCls, nullptr);
    ets_object obj = env_->AllocObject(dCls);
    ASSERT_NE(obj, nullptr);

    ets_method voidId = env_->Getp_method(cCls, "void_method", "II:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->Getp_method(cCls, "object_method", ":LCallingMethodsTest/A;");
    ASSERT_NE(objectId, nullptr);

    env_->CallNonvirtualVoidMethod(obj, cCls, voidId, static_cast<ets_int>(1), static_cast<ets_int>(121_I));
    ets_field dMemberId = env_->Getp_field(dCls, "member", "I");
    ASSERT_NE(dMemberId, nullptr);
    EXPECT_EQ(env_->GetIntField(obj, dMemberId), static_cast<ets_int>(0));

    ets_class aCls = env_->FindClass("CallingMethodsTest/A");
    ASSERT_NE(aCls, nullptr);
    ets_object aObj = env_->CallNonvirtualObjectMethod(obj, cCls, objectId);
    ASSERT_NE(aObj, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObj, aCls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(obj, dMemberId), static_cast<ets_int>(0));

    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42_I);
    const std::vector<ets_value> voidArgs = {tmp, intTmp};
    env_->CallNonvirtualVoidMethodArray(obj, cCls, voidId, voidArgs.data());
    EXPECT_EQ(env_->GetIntField(obj, dMemberId), static_cast<ets_int>(0));

    ets_object aObjFromArrayFunc = env_->CallNonvirtualObjectMethodArray(obj, cCls, objectId, nullptr);
    ASSERT_NE(aObjFromArrayFunc, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObjFromArrayFunc, aCls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(obj, dMemberId), static_cast<ets_int>(0));

    CallNonvirtualVoidMethodListHelper(env_, obj, cCls, voidId, static_cast<ets_int>(1), static_cast<ets_int>(121_I));
    EXPECT_EQ(env_->GetIntField(obj, dMemberId), static_cast<ets_int>(0));

    ets_object aObjFromListFunc = CallNonvirtualObjectMethodListHelper(env_, obj, cCls, objectId);
    ASSERT_NE(aObjFromListFunc, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObjFromListFunc, aCls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(obj, dMemberId), static_cast<ets_int>(0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral15)
{
    ets_class cCls = env_->FindClass("CallingMethodsTest/C");
    ASSERT_NE(cCls, nullptr);
    ets_class dCls = env_->FindClass("CallingMethodsTest/D");
    ASSERT_NE(dCls, nullptr);
    ets_object obj = env_->AllocObject(dCls);
    ASSERT_NE(obj, nullptr);

    ets_method booleanId = env_->Getp_method(cCls, "boolean_method", "ZI:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->Getp_method(cCls, "byte_method", "BI:B");
    ASSERT_NE(byteId, nullptr);

    EXPECT_EQ(env_->CallNonvirtualBooleanMethod(obj, cCls, booleanId, static_cast<ets_boolean>(1),
                                                static_cast<ets_int>(121_I)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallNonvirtualByteMethod(obj, cCls, byteId, static_cast<ets_byte>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_byte>(0));

    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;

    tmp.z = static_cast<ets_boolean>(1);
    const std::vector<ets_value> booleanArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallNonvirtualBooleanMethodArray(obj, cCls, booleanId, booleanArgs.data()),
              static_cast<ets_boolean>(0));

    tmp.b = static_cast<ets_byte>(1);
    const std::vector<ets_value> byteArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallNonvirtualByteMethodArray(obj, cCls, byteId, byteArgs.data()), static_cast<ets_byte>(0));

    EXPECT_EQ(CallNonvirtualBooleanMethodListHelper(env_, obj, cCls, booleanId, static_cast<ets_boolean>(1),
                                                    static_cast<ets_int>(121_I)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(CallNonvirtualByteMethodListHelper(env_, obj, cCls, byteId, static_cast<ets_byte>(1),
                                                 static_cast<ets_int>(121_I)),
              static_cast<ets_byte>(0));

    EXPECT_EQ(env_->CallNonvirtualBooleanMethodArray(obj, nullptr, booleanId, booleanArgs.data()),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallNonvirtualByteMethodArray(obj, nullptr, byteId, byteArgs.data()), static_cast<ets_byte>(0));

    EXPECT_EQ(CallNonvirtualBooleanMethodListHelper(env_, obj, nullptr, booleanId, static_cast<ets_boolean>(1),
                                                    static_cast<ets_int>(121_I)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(CallNonvirtualByteMethodListHelper(env_, obj, nullptr, byteId, static_cast<ets_byte>(1),
                                                 static_cast<ets_int>(121_I)),
              static_cast<ets_byte>(0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral16)
{
    ets_class cCls = env_->FindClass("CallingMethodsTest/C");
    ASSERT_NE(cCls, nullptr);
    ets_class dCls = env_->FindClass("CallingMethodsTest/D");
    ASSERT_NE(dCls, nullptr);
    ets_object obj = env_->AllocObject(dCls);
    ASSERT_NE(obj, nullptr);

    ets_method charId = env_->Getp_method(cCls, "char_method", "CI:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->Getp_method(cCls, "short_method", "SI:S");
    ASSERT_NE(shortId, nullptr);

    EXPECT_EQ(env_->CallNonvirtualCharMethod(obj, cCls, charId, static_cast<ets_char>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_char>(0));
    EXPECT_EQ(
        env_->CallNonvirtualShortMethod(obj, cCls, shortId, static_cast<ets_short>(1), static_cast<ets_int>(121_I)),
        static_cast<ets_short>(0));

    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;

    tmp.c = static_cast<ets_char>(1);
    const std::vector<ets_value> charArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallNonvirtualCharMethodArray(obj, cCls, charId, charArgs.data()), static_cast<ets_char>(0));

    tmp.s = static_cast<ets_short>(1);
    const std::vector<ets_value> shortArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallNonvirtualShortMethodArray(obj, cCls, shortId, shortArgs.data()), static_cast<ets_short>(0));

    EXPECT_EQ(CallNonvirtualCharMethodListHelper(env_, obj, cCls, charId, static_cast<ets_char>(1),
                                                 static_cast<ets_int>(121_I)),
              static_cast<ets_char>(0));
    EXPECT_EQ(CallNonvirtualShortMethodListHelper(env_, obj, cCls, shortId, static_cast<ets_short>(1),
                                                  static_cast<ets_int>(121_I)),
              static_cast<ets_short>(0));

    EXPECT_EQ(
        env_->CallNonvirtualCharMethod(obj, nullptr, charId, static_cast<ets_char>(1), static_cast<ets_int>(121_I)),
        static_cast<ets_char>(0));
    EXPECT_EQ(
        env_->CallNonvirtualShortMethod(obj, nullptr, shortId, static_cast<ets_short>(1), static_cast<ets_int>(121_I)),
        static_cast<ets_short>(0));

    EXPECT_EQ(env_->CallNonvirtualCharMethodArray(obj, nullptr, charId, charArgs.data()), static_cast<ets_char>(0));
    EXPECT_EQ(env_->CallNonvirtualShortMethodArray(obj, nullptr, shortId, shortArgs.data()), static_cast<ets_short>(0));

    EXPECT_EQ(CallNonvirtualCharMethodListHelper(env_, obj, nullptr, charId, static_cast<ets_char>(1),
                                                 static_cast<ets_int>(121_I)),
              static_cast<ets_char>(0));
    EXPECT_EQ(CallNonvirtualShortMethodListHelper(env_, obj, nullptr, shortId, static_cast<ets_short>(1),
                                                  static_cast<ets_int>(121_I)),
              static_cast<ets_short>(0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral17)
{
    ets_class cCls = env_->FindClass("CallingMethodsTest/C");
    ASSERT_NE(cCls, nullptr);
    ets_class dCls = env_->FindClass("CallingMethodsTest/D");
    ASSERT_NE(dCls, nullptr);
    ets_object obj = env_->AllocObject(dCls);
    ASSERT_NE(obj, nullptr);

    ets_method intId = env_->Getp_method(cCls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->Getp_method(cCls, "long_method", "JI:J");
    ASSERT_NE(longId, nullptr);

    EXPECT_EQ(env_->CallNonvirtualIntMethod(obj, cCls, intId), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallNonvirtualLongMethod(obj, cCls, longId, static_cast<ets_long>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_long>(0));

    EXPECT_EQ(env_->CallNonvirtualIntMethodArray(obj, cCls, intId, nullptr), static_cast<ets_int>(0));

    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> longArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallNonvirtualLongMethodArray(obj, cCls, longId, longArgs.data()), static_cast<ets_long>(0));

    EXPECT_EQ(CallNonvirtualIntMethodListHelper(env_, obj, cCls, intId), static_cast<ets_int>(0));
    EXPECT_EQ(CallNonvirtualLongMethodListHelper(env_, obj, cCls, longId, static_cast<ets_long>(1),
                                                 static_cast<ets_int>(121_I)),
              static_cast<ets_long>(0));

    EXPECT_EQ(env_->CallNonvirtualIntMethod(obj, nullptr, intId), static_cast<ets_int>(0));
    EXPECT_EQ(
        env_->CallNonvirtualLongMethod(obj, nullptr, longId, static_cast<ets_long>(1), static_cast<ets_int>(121_I)),
        static_cast<ets_long>(0));

    EXPECT_EQ(env_->CallNonvirtualIntMethodArray(obj, nullptr, intId, nullptr), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallNonvirtualLongMethodArray(obj, nullptr, longId, longArgs.data()), static_cast<ets_long>(0));

    EXPECT_EQ(CallNonvirtualIntMethodListHelper(env_, obj, nullptr, intId), static_cast<ets_int>(0));
    EXPECT_EQ(CallNonvirtualLongMethodListHelper(env_, obj, nullptr, longId, static_cast<ets_long>(1),
                                                 static_cast<ets_int>(121_I)),
              static_cast<ets_long>(0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral18)
{
    ets_class cCls = env_->FindClass("CallingMethodsTest/C");
    ASSERT_NE(cCls, nullptr);
    ets_class dCls = env_->FindClass("CallingMethodsTest/D");
    ASSERT_NE(dCls, nullptr);
    ets_object obj = env_->AllocObject(dCls);
    ASSERT_NE(obj, nullptr);

    ets_method floatId = env_->Getp_method(cCls, "float_method", "FI:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->Getp_method(cCls, "double_method", "DI:D");
    ASSERT_NE(doubleId, nullptr);

    EXPECT_EQ(
        env_->CallNonvirtualFloatMethod(obj, cCls, floatId, static_cast<ets_float>(1.0F), static_cast<ets_int>(121_I)),
        static_cast<ets_float>(0.0F));
    EXPECT_EQ(env_->CallNonvirtualDoubleMethod(obj, cCls, doubleId, static_cast<ets_double>(1.0),
                                               static_cast<ets_int>(121_I)),
              static_cast<ets_double>(0.0));

    // CallNonvirtual<Type>MethodArray part
    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;

    tmp.j = static_cast<ets_float>(1.0F);
    const std::vector<ets_value> floatArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallNonvirtualFloatMethodArray(obj, cCls, floatId, floatArgs.data()), static_cast<ets_float>(0.0F));

    tmp.j = static_cast<ets_double>(1.0);
    const std::vector<ets_value> doubleArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallNonvirtualDoubleMethodArray(obj, cCls, doubleId, doubleArgs.data()),
              static_cast<ets_double>(0.0));

    EXPECT_EQ(CallNonvirtualFloatMethodListHelper(env_, obj, cCls, floatId, static_cast<ets_float>(1.0F),
                                                  static_cast<ets_int>(121_I)),
              static_cast<ets_float>(0.0F));
    EXPECT_EQ(CallNonvirtualDoubleMethodListHelper(env_, obj, cCls, doubleId, static_cast<ets_double>(1.0),
                                                   static_cast<ets_int>(121_I)),
              static_cast<ets_double>(0.0));

    EXPECT_EQ(env_->CallNonvirtualFloatMethod(obj, nullptr, floatId, static_cast<ets_float>(1.0F),
                                              static_cast<ets_int>(121_I)),
              static_cast<ets_float>(0.0F));
    EXPECT_EQ(env_->CallNonvirtualDoubleMethod(obj, nullptr, doubleId, static_cast<ets_double>(1.0),
                                               static_cast<ets_int>(121_I)),
              static_cast<ets_double>(0.0));

    EXPECT_EQ(env_->CallNonvirtualFloatMethodArray(obj, nullptr, floatId, floatArgs.data()),
              static_cast<ets_float>(0.0F));
    EXPECT_EQ(env_->CallNonvirtualDoubleMethodArray(obj, nullptr, doubleId, doubleArgs.data()),
              static_cast<ets_double>(0.0));

    EXPECT_EQ(CallNonvirtualFloatMethodListHelper(env_, obj, nullptr, floatId, static_cast<ets_float>(1.0F),
                                                  static_cast<ets_int>(121_I)),
              static_cast<ets_float>(0.0F));
    EXPECT_EQ(CallNonvirtualDoubleMethodListHelper(env_, obj, nullptr, doubleId, static_cast<ets_double>(1.0),
                                                   static_cast<ets_int>(121_I)),
              static_cast<ets_double>(0.0));
}

// NOTE(m.morozov): enable this test, when virtual calls will be implemented
#ifdef ENABLE_THIS_CODE_IN_FUTURE
TEST_F(MethodsTest, CallMethodsTestGeneral19)
{
    // Call<type>Method part
    env->CallVoidMethod(obj, void_id, static_cast<ets_int>(11_I), static_cast<ets_int>(121_I));
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(11_I));

    ets_object a_obj_from_direct_calls = env->CallObjectMethod(obj, object_id);
    ASSERT_NE(a_obj_from_direct_calls, nullptr);
    EXPECT_EQ(env->IsInstanceOf(a_obj_from_direct_calls, a_cls), ETS_TRUE);
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(42_I));

    EXPECT_EQ(env->CallBooleanMethod(obj, boolean_id, static_cast<ets_boolean>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_boolean>(1));
    EXPECT_EQ(env->CallByteMethod(obj, byte_id, static_cast<ets_byte>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_byte>(1));
    EXPECT_EQ(env->CallCharMethod(obj, char_id, static_cast<ets_char>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_char>(1));
    EXPECT_EQ(env->CallShortMethod(obj, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_short>(1));
    EXPECT_EQ(env->CallIntMethod(obj, int_id), static_cast<ets_int>(1));
    EXPECT_EQ(env->CallLongMethod(obj, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_long>(1));
    EXPECT_EQ(env->CallFloatMethod(obj, float_id, static_cast<ets_float>(1.0F), static_cast<ets_int>(121_I)),
              static_cast<ets_float>(1.0F));
    EXPECT_EQ(env->CallDoubleMethod(obj, double_id, static_cast<ets_double>(1.0), static_cast<ets_int>(121_I)),
              static_cast<ets_double>(1.0));

    // Call<type>MethodArray part
    env->CallVoidMethodArray(obj, void_id, void_args.data());
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(42_I));

    ets_object a_obj_from_Array_direct_calls = env->CallObjectMethodArray(obj, object_id, nullptr);
    ASSERT_NE(a_obj_from_Array_direct_calls, nullptr);
    EXPECT_EQ(env->IsInstanceOf(a_obj_from_Array_direct_calls, a_cls), ETS_TRUE);
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(42_I));

    EXPECT_EQ(env->CallBooleanMethodArray(obj, boolean_id, boolean_args.data()), static_cast<ets_boolean>(1));
    EXPECT_EQ(env->CallByteMethodArray(obj, byte_id, byte_args.data()), static_cast<ets_byte>(1));
    EXPECT_EQ(env->CallCharMethodArray(obj, char_id, char_args.data()), static_cast<ets_char>(1));
    EXPECT_EQ(env->CallShortMethodArray(obj, short_id, short_args.data()), static_cast<ets_short>(1));
    EXPECT_EQ(env->CallIntMethodArray(obj, int_id, nullptr), static_cast<ets_int>(1));
    EXPECT_EQ(env->CallLongMethodArray(obj, long_id, long_args.data()), static_cast<ets_long>(1));
    EXPECT_EQ(env->CallFloatMethodArray(obj, float_id, float_args.data()), static_cast<ets_float>(1.0F));
    EXPECT_EQ(env->CallDoubleMethodArray(obj, double_id, double_args.data()), static_cast<ets_double>(1.0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral20)
{
    // Call<type>MethodList part
    CallVoidMethodListHelper(env, obj, void_id, static_cast<ets_int>(84_I), static_cast<ets_int>(121_I));
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(84_I));

    ets_object a_obj_from_List_direct_calls = CallObjectMethodListHelper(env, obj, object_id);
    ASSERT_NE(a_obj_from_List_direct_calls, nullptr);
    EXPECT_EQ(env->IsInstanceOf(a_obj_from_List_direct_calls, a_cls), ETS_TRUE);
    EXPECT_EQ(env->GetIntField(obj, d_member_id), static_cast<ets_int>(42_I));

    EXPECT_EQ(
        CallBooleanMethodListHelper(env, obj, boolean_id, static_cast<ets_boolean>(1), static_cast<ets_int>(121_I)),
        static_cast<ets_boolean>(1));
    EXPECT_EQ(CallByteMethodListHelper(env, obj, byte_id, static_cast<ets_byte>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_byte>(1));
    EXPECT_EQ(CallCharMethodListHelper(env, obj, char_id, static_cast<ets_char>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_char>(1));
    EXPECT_EQ(CallShortMethodListHelper(env, obj, short_id, static_cast<ets_short>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_short>(1));
    EXPECT_EQ(CallIntMethodListHelper(env, obj, int_id), static_cast<ets_int>(1));
    EXPECT_EQ(CallLongMethodListHelper(env, obj, long_id, static_cast<ets_long>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_long>(1));
    EXPECT_EQ(CallFloatMethodListHelper(env, obj, float_id, static_cast<ets_float>(1.0F), static_cast<ets_int>(121_I)),
              static_cast<ets_float>(1.0F));
    EXPECT_EQ(
        CallDoubleMethodListHelper(env, obj, double_id, static_cast<ets_double>(1.0), static_cast<ets_int>(121_I)),
        static_cast<ets_double>(1.0));
}
#endif  // ENABLE_THIS_CODE_IN_FUTURE

TEST_F(MethodsTestDeath, DISABLED_CallMethodsTestGeneralDeath14)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    // Static methods part
    EXPECT_DEATH(env_->GetStaticp_method(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->GetStaticp_method(nullptr, "foo", nullptr), "");
    EXPECT_DEATH(env_->GetStaticp_method(nullptr, nullptr, "I:I"), "");
    EXPECT_DEATH(env_->GetStaticp_method(nullptr, "foo", "I:I"), "");

    ets_class aCls = env_->FindClass("CallingMethodsTest/A");
    ASSERT_NE(aCls, nullptr);
    EXPECT_DEATH(env_->GetStaticp_method(aCls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->GetStaticp_method(aCls, nullptr, "I:I"), "");

    // Non static methods part
    EXPECT_DEATH(env_->Getp_method(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->Getp_method(nullptr, "foo", nullptr), "");
    EXPECT_DEATH(env_->Getp_method(nullptr, nullptr, "I:I"), "");
    EXPECT_DEATH(env_->Getp_method(nullptr, "foo", "I:I"), "");

    EXPECT_DEATH(env_->Getp_method(aCls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->Getp_method(aCls, nullptr, "I:I"), "");
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
