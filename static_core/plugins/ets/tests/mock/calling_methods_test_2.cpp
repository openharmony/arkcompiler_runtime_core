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
#include "plugins/ets/tests/mock/calling_methods_test_helper.h"

namespace ark::ets::test {

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

    ets_method voidId = env_->Getp_method(cls, "void_method", "II:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->Getp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->Getp_method(cls, "boolean_method", "ZI:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->Getp_method(cls, "byte_method", "BI:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->Getp_method(cls, "char_method", "CI:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->Getp_method(cls, "short_method", "SI:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->Getp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->Getp_method(cls, "long_method", "JI:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->Getp_method(cls, "float_method", "FI:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->Getp_method(cls, "double_method", "DI:D");
    ASSERT_NE(doubleId, nullptr);

    ets_method voidIdNoSig = env_->Getp_method(cls, "void_method", nullptr);
    ASSERT_EQ(voidId, voidIdNoSig);
    ets_method objectIdNoSig = env_->Getp_method(cls, "object_method", nullptr);
    ASSERT_EQ(objectId, objectIdNoSig);
    ets_method booleanIdNoSig = env_->Getp_method(cls, "boolean_method", nullptr);
    ASSERT_EQ(booleanId, booleanIdNoSig);
    ets_method byteIdNoSig = env_->Getp_method(cls, "byte_method", nullptr);
    ASSERT_EQ(byteId, byteIdNoSig);
    ets_method charIdNoSig = env_->Getp_method(cls, "char_method", nullptr);
    ASSERT_EQ(charId, charIdNoSig);
    ets_method shortIdNoSig = env_->Getp_method(cls, "short_method", nullptr);
    ASSERT_EQ(shortId, shortIdNoSig);
    ets_method intIdNoSig = env_->Getp_method(cls, "int_method", nullptr);
    ASSERT_EQ(intId, intIdNoSig);
    ets_method longIdNoSig = env_->Getp_method(cls, "long_method", nullptr);
    ASSERT_EQ(longId, longIdNoSig);
    ets_method floatIdNoSig = env_->Getp_method(cls, "float_method", nullptr);
    ASSERT_EQ(floatId, floatIdNoSig);
    ets_method doubleIdNoSig = env_->Getp_method(cls, "double_method", nullptr);
    ASSERT_EQ(doubleId, doubleIdNoSig);
}

TEST_F(MethodsTest, CallMethodsTestGeneral3)
{
    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);
    ets_object obj = env_->AllocObject(cls);
    ASSERT_NE(obj, nullptr);

    ets_method voidId = env_->Getp_method(cls, "void_method", "II:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->Getp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->Getp_method(cls, "boolean_method", "ZI:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->Getp_method(cls, "byte_method", "BI:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->Getp_method(cls, "char_method", "CI:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->Getp_method(cls, "short_method", "SI:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->Getp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->Getp_method(cls, "long_method", "JI:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->Getp_method(cls, "float_method", "FI:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->Getp_method(cls, "double_method", "DI:D");
    ASSERT_NE(doubleId, nullptr);

    // Call<Type>Method part
    env_->CallVoidMethod(obj, voidId, static_cast<ets_int>(42_I), static_cast<ets_int>(121_I));

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_object aObj = env_->CallObjectMethod(obj, objectId);
    ASSERT_NE(aObj, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObj, aCls), ETS_TRUE);

    EXPECT_EQ(env_->CallBooleanMethod(obj, booleanId, static_cast<ets_boolean>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallByteMethod(obj, byteId, static_cast<ets_byte>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_byte>(0));
    EXPECT_EQ(env_->CallCharMethod(obj, charId, static_cast<ets_char>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_char>(0));
    EXPECT_EQ(env_->CallShortMethod(obj, shortId, static_cast<ets_short>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_short>(0));
    EXPECT_EQ(env_->CallIntMethod(obj, intId), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallLongMethod(obj, longId, static_cast<ets_long>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(env_->CallFloatMethod(obj, floatId, static_cast<ets_float>(1.0F), static_cast<ets_int>(121_I)),
                    static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(env_->CallDoubleMethod(obj, doubleId, static_cast<ets_double>(1.0), static_cast<ets_int>(121_I)),
                     static_cast<ets_double>(0.0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral4)
{
    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);
    ets_object obj = env_->AllocObject(cls);
    ASSERT_NE(obj, nullptr);
    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);

    ets_method voidId = env_->Getp_method(cls, "void_method", "II:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->Getp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->Getp_method(cls, "boolean_method", "ZI:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->Getp_method(cls, "byte_method", "BI:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->Getp_method(cls, "char_method", "CI:C");
    ASSERT_NE(charId, nullptr);

    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42_I);
    const std::vector<ets_value> voidArgs = {tmp, intTmp};
    env_->CallVoidMethodArray(obj, voidId, voidArgs.data());

    ets_object aObjFromArrayFunc = env_->CallObjectMethodArray(obj, objectId, nullptr);
    ASSERT_NE(aObjFromArrayFunc, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObjFromArrayFunc, aCls), ETS_TRUE);

    tmp.z = static_cast<ets_boolean>(1);
    const std::vector<ets_value> booleanArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallBooleanMethodArray(obj, booleanId, booleanArgs.data()), static_cast<ets_boolean>(0));

    tmp.b = static_cast<ets_byte>(1);
    const std::vector<ets_value> byteArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallByteMethodArray(obj, byteId, byteArgs.data()), static_cast<ets_byte>(0));

    tmp.c = static_cast<ets_char>(1);
    const std::vector<ets_value> charArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallCharMethodArray(obj, charId, charArgs.data()), static_cast<ets_char>(0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral5)
{
    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);
    ets_object obj = env_->AllocObject(cls);
    ASSERT_NE(obj, nullptr);
    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);

    ets_method shortId = env_->Getp_method(cls, "short_method", "SI:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->Getp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->Getp_method(cls, "long_method", "JI:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->Getp_method(cls, "float_method", "FI:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->Getp_method(cls, "double_method", "DI:D");
    ASSERT_NE(doubleId, nullptr);

    // Call<Type>MethodArray part
    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;

    tmp.s = static_cast<ets_short>(1);
    const std::vector<ets_value> shortArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallShortMethodArray(obj, shortId, shortArgs.data()), static_cast<ets_short>(0));

    EXPECT_EQ(env_->CallIntMethodArray(obj, intId, nullptr), static_cast<ets_int>(0));

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> longArgs = {tmp, intTmp};
    EXPECT_EQ(env_->CallLongMethodArray(obj, longId, longArgs.data()), static_cast<ets_long>(0));

    tmp.j = static_cast<ets_float>(1.0F);
    const std::vector<ets_value> floatArgs = {tmp, intTmp};
    EXPECT_FLOAT_EQ(env_->CallFloatMethodArray(obj, floatId, floatArgs.data()), static_cast<ets_float>(0.0F));

    tmp.j = static_cast<ets_double>(1.0);
    const std::vector<ets_value> doubleArgs = {tmp, intTmp};
    EXPECT_DOUBLE_EQ(env_->CallDoubleMethodArray(obj, doubleId, doubleArgs.data()), static_cast<ets_double>(0.0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral6)
{
    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);
    ets_object obj = env_->AllocObject(cls);
    ASSERT_NE(obj, nullptr);
    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);

    ets_method voidId = env_->Getp_method(cls, "void_method", "II:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->Getp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->Getp_method(cls, "boolean_method", "ZI:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->Getp_method(cls, "byte_method", "BI:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->Getp_method(cls, "char_method", "CI:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->Getp_method(cls, "short_method", "SI:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->Getp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->Getp_method(cls, "long_method", "JI:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->Getp_method(cls, "float_method", "FI:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->Getp_method(cls, "double_method", "DI:D");
    ASSERT_NE(doubleId, nullptr);

    // Call<type>MethodList part
    CallVoidMethodListHelper(env_, obj, voidId, static_cast<ets_int>(42_I), static_cast<ets_int>(121_I));

    ets_object aObjFromListFunc = CallObjectMethodListHelper(env_, obj, objectId);
    ASSERT_NE(aObjFromListFunc, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObjFromListFunc, aCls), ETS_TRUE);

    EXPECT_EQ(
        CallBooleanMethodListHelper(env_, obj, booleanId, static_cast<ets_boolean>(1), static_cast<ets_int>(121_I)),
        static_cast<ets_boolean>(0));
    EXPECT_EQ(CallByteMethodListHelper(env_, obj, byteId, static_cast<ets_byte>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_byte>(0));
    EXPECT_EQ(CallCharMethodListHelper(env_, obj, charId, static_cast<ets_char>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_char>(0));
    EXPECT_EQ(CallShortMethodListHelper(env_, obj, shortId, static_cast<ets_short>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_short>(0));
    EXPECT_EQ(CallIntMethodListHelper(env_, obj, intId), static_cast<ets_int>(0));
    EXPECT_EQ(CallLongMethodListHelper(env_, obj, longId, static_cast<ets_long>(1), static_cast<ets_int>(121_I)),
              static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(
        CallFloatMethodListHelper(env_, obj, floatId, static_cast<ets_float>(1.0F), static_cast<ets_int>(121_I)),
        static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(
        CallDoubleMethodListHelper(env_, obj, doubleId, static_cast<ets_double>(1.0), static_cast<ets_int>(121_I)),
        static_cast<ets_double>(0.0));
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath3)
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
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath4)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    {
        ets_class dCls = env_->FindClass("D");
        ASSERT_NE(dCls, nullptr);
        ets_object obj = env_->AllocObject(dCls);
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

}  // namespace ark::ets::test
