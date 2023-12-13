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

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath5)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        // Call<type>Method part
        EXPECT_DEATH(env_->CallStaticVoidMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticObjectMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticBooleanMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticByteMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticCharMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticShortMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticIntMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticLongMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticFloatMethod(nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticDoubleMethod(nullptr, nullptr), "");

        // Call<type>MethodArray part
        EXPECT_DEATH(env_->CallStaticVoidMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticObjectMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticBooleanMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticByteMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticCharMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticShortMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticIntMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticLongMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticFloatMethodArray(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticDoubleMethodArray(nullptr, nullptr, nullptr), "");

        // Call<type>MethodList part
        EXPECT_DEATH(CallStaticVoidMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticObjectMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticBooleanMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticByteMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticCharMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticShortMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticIntMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticLongMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticFloatMethodListHelper(env_, nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticDoubleMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    }
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath6)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        ets_class cls = env_->FindClass("F");
        ASSERT_NE(cls, nullptr);

        // Call<type>Method part
        EXPECT_DEATH(env_->CallStaticVoidMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticObjectMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticBooleanMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticByteMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticCharMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticShortMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticIntMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticLongMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticFloatMethod(cls, nullptr), "");
        EXPECT_DEATH(env_->CallStaticDoubleMethod(cls, nullptr), "");

        // Call<type>MethodArray part
        EXPECT_DEATH(env_->CallStaticVoidMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticObjectMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticBooleanMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticByteMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticCharMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticShortMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticIntMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticLongMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticFloatMethodArray(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->CallStaticDoubleMethodArray(cls, nullptr, nullptr), "");

        // Call<type>MethodList part
        EXPECT_DEATH(CallStaticVoidMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticObjectMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticBooleanMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticByteMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticCharMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticShortMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticIntMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticLongMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticFloatMethodListHelper(env_, cls, nullptr, nullptr), "");
        EXPECT_DEATH(CallStaticDoubleMethodListHelper(env_, cls, nullptr, nullptr), "");
    }
}

TEST_F(MethodsTest, CallMethodsTestGeneral7)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);

    ets_method voidId = env_->GetStaticp_method(cls, "void_method", "I:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->GetStaticp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->GetStaticp_method(cls, "boolean_method", "Z:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->GetStaticp_method(cls, "byte_method", "B:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->GetStaticp_method(cls, "char_method", "C:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->GetStaticp_method(cls, "short_method", "S:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->GetStaticp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->GetStaticp_method(cls, "long_method", "J:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->GetStaticp_method(cls, "float_method", "F:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->GetStaticp_method(cls, "double_method", "D:D");
    ASSERT_NE(doubleId, nullptr);

    ets_method voidIdNoSig = env_->GetStaticp_method(cls, "void_method", nullptr);
    ASSERT_EQ(voidId, voidIdNoSig);
    ets_method objectIdNoSig = env_->GetStaticp_method(cls, "object_method", nullptr);
    ASSERT_EQ(objectId, objectIdNoSig);
    ets_method booleanIdNoSig = env_->GetStaticp_method(cls, "boolean_method", nullptr);
    ASSERT_EQ(booleanId, booleanIdNoSig);
    ets_method byteIdNoSig = env_->GetStaticp_method(cls, "byte_method", nullptr);
    ASSERT_EQ(byteId, byteIdNoSig);
    ets_method charIdNoSig = env_->GetStaticp_method(cls, "char_method", nullptr);
    ASSERT_EQ(charId, charIdNoSig);
    ets_method shortIdNoSig = env_->GetStaticp_method(cls, "short_method", nullptr);
    ASSERT_EQ(shortId, shortIdNoSig);
    ets_method intIdNoSig = env_->GetStaticp_method(cls, "int_method", nullptr);
    ASSERT_EQ(intId, intIdNoSig);
    ets_method longIdNoSig = env_->GetStaticp_method(cls, "long_method", nullptr);
    ASSERT_EQ(longId, longIdNoSig);
    ets_method floatIdNoSig = env_->GetStaticp_method(cls, "float_method", nullptr);
    ASSERT_EQ(floatId, floatIdNoSig);
    ets_method doubleIdNoSig = env_->GetStaticp_method(cls, "double_method", nullptr);
    ASSERT_EQ(doubleId, doubleIdNoSig);
}

TEST_F(MethodsTest, CallMethodsTestGeneral8)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);

    ets_method voidId = env_->GetStaticp_method(cls, "void_method", "I:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->GetStaticp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->GetStaticp_method(cls, "boolean_method", "Z:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->GetStaticp_method(cls, "byte_method", "B:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->GetStaticp_method(cls, "char_method", "C:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->GetStaticp_method(cls, "short_method", "S:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->GetStaticp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->GetStaticp_method(cls, "long_method", "J:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->GetStaticp_method(cls, "float_method", "F:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->GetStaticp_method(cls, "double_method", "D:D");
    ASSERT_NE(doubleId, nullptr);

    // CallStatic<type>Method part
    env_->CallStaticVoidMethod(cls, voidId, static_cast<ets_int>(42_I));

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_object aObj = env_->CallStaticObjectMethod(cls, objectId);
    ASSERT_NE(aObj, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObj, aCls), ETS_TRUE);

    EXPECT_EQ(env_->CallStaticBooleanMethod(cls, booleanId, static_cast<ets_boolean>(1)), static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallStaticByteMethod(cls, byteId, static_cast<ets_byte>(1)), static_cast<ets_byte>(0));
    EXPECT_EQ(env_->CallStaticCharMethod(cls, charId, static_cast<ets_char>(1)), static_cast<ets_char>(0));
    EXPECT_EQ(env_->CallStaticShortMethod(cls, shortId, static_cast<ets_short>(1)), static_cast<ets_short>(0));
    EXPECT_EQ(env_->CallStaticIntMethod(cls, intId), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallStaticLongMethod(cls, longId, static_cast<ets_long>(1)), static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(env_->CallStaticFloatMethod(cls, floatId, static_cast<ets_float>(1.0F)),
                    static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(env_->CallStaticDoubleMethod(cls, doubleId, static_cast<ets_double>(1.0)),
                     static_cast<ets_double>(0.0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral9)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);

    ets_method voidId = env_->GetStaticp_method(cls, "void_method", "I:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->GetStaticp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->GetStaticp_method(cls, "boolean_method", "Z:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->GetStaticp_method(cls, "byte_method", "B:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->GetStaticp_method(cls, "char_method", "C:C");
    ASSERT_NE(charId, nullptr);

    // CallStatic<Type>MethodArray part
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42_I);
    std::vector<ets_value> voidArgs = {tmp};
    env_->CallStaticVoidMethodArray(cls, voidId, voidArgs.data());

    ets_object aObjFromArrayFunc = env_->CallObjectMethodArray(cls, objectId, nullptr);
    ASSERT_NE(aObjFromArrayFunc, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObjFromArrayFunc, aCls), ETS_TRUE);

    tmp.z = static_cast<ets_boolean>(1);
    std::vector<ets_value> booleanArgs = {tmp};
    EXPECT_EQ(env_->CallStaticBooleanMethodArray(cls, booleanId, booleanArgs.data()), static_cast<ets_boolean>(0));

    tmp.b = static_cast<ets_byte>(1);
    std::vector<ets_value> byteArgs = {tmp};
    EXPECT_EQ(env_->CallStaticByteMethodArray(cls, byteId, byteArgs.data()), static_cast<ets_byte>(0));

    tmp.c = static_cast<ets_char>(1);
    std::vector<ets_value> charArgs = {tmp};
    EXPECT_EQ(env_->CallStaticCharMethodArray(cls, charId, charArgs.data()), static_cast<ets_char>(0));

    EXPECT_EQ(env_->CallStaticBooleanMethodArray(nullptr, booleanId, booleanArgs.data()), static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallStaticByteMethodArray(nullptr, byteId, byteArgs.data()), static_cast<ets_byte>(0));
    EXPECT_EQ(env_->CallStaticCharMethodArray(nullptr, charId, charArgs.data()), static_cast<ets_char>(0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral10)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);

    ets_method shortId = env_->GetStaticp_method(cls, "short_method", "S:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->GetStaticp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->GetStaticp_method(cls, "long_method", "J:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->GetStaticp_method(cls, "float_method", "F:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->GetStaticp_method(cls, "double_method", "D:D");
    ASSERT_NE(doubleId, nullptr);

    // CallStatic<Type>MethodArray part
    ets_value tmp;
    tmp.s = static_cast<ets_short>(1);
    std::vector<ets_value> shortArgs = {tmp};
    EXPECT_EQ(env_->CallStaticShortMethodArray(cls, shortId, shortArgs.data()), static_cast<ets_short>(0));

    EXPECT_EQ(env_->CallStaticIntMethodArray(cls, intId, nullptr), static_cast<ets_int>(0));

    tmp.j = static_cast<ets_long>(1);
    std::vector<ets_value> longArgs = {tmp};
    EXPECT_EQ(env_->CallStaticLongMethodArray(cls, longId, longArgs.data()), static_cast<ets_long>(0));

    tmp.j = static_cast<ets_float>(1.0F);
    std::vector<ets_value> floatArgs = {tmp};
    EXPECT_FLOAT_EQ(env_->CallStaticFloatMethodArray(cls, floatId, floatArgs.data()), static_cast<ets_float>(0.0F));

    tmp.j = static_cast<ets_double>(1.0);
    std::vector<ets_value> doubleArgs = {tmp};
    EXPECT_DOUBLE_EQ(env_->CallStaticDoubleMethodArray(cls, doubleId, doubleArgs.data()), static_cast<ets_double>(0.0));

    EXPECT_EQ(env_->CallStaticShortMethodArray(nullptr, shortId, shortArgs.data()), static_cast<ets_short>(0));
    EXPECT_EQ(env_->CallStaticIntMethodArray(nullptr, intId, nullptr), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallStaticLongMethodArray(nullptr, longId, longArgs.data()), static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(env_->CallStaticFloatMethodArray(nullptr, floatId, floatArgs.data()), static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(env_->CallStaticDoubleMethodArray(nullptr, doubleId, doubleArgs.data()),
                     static_cast<ets_double>(0.0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral11)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);
    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);

    ets_method voidId = env_->GetStaticp_method(cls, "void_method", "I:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->GetStaticp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->GetStaticp_method(cls, "boolean_method", "Z:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->GetStaticp_method(cls, "byte_method", "B:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->GetStaticp_method(cls, "char_method", "C:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->GetStaticp_method(cls, "short_method", "S:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->GetStaticp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->GetStaticp_method(cls, "long_method", "J:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->GetStaticp_method(cls, "float_method", "F:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->GetStaticp_method(cls, "double_method", "D:D");
    ASSERT_NE(doubleId, nullptr);

    // CallStatic<type>MethodList part
    CallStaticVoidMethodListHelper(env_, cls, voidId, static_cast<ets_int>(42_I), static_cast<ets_int>(121_I));

    ets_object aObjFromListFunc = CallObjectMethodListHelper(env_, cls, objectId);
    ASSERT_NE(aObjFromListFunc, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(aObjFromListFunc, aCls), ETS_TRUE);

    EXPECT_EQ(CallStaticBooleanMethodListHelper(env_, cls, booleanId, static_cast<ets_boolean>(1)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(CallStaticByteMethodListHelper(env_, cls, byteId, static_cast<ets_byte>(1)), static_cast<ets_byte>(0));
    EXPECT_EQ(CallStaticCharMethodListHelper(env_, cls, charId, static_cast<ets_char>(1)), static_cast<ets_char>(0));
    EXPECT_EQ(CallStaticShortMethodListHelper(env_, cls, shortId, static_cast<ets_short>(1)),
              static_cast<ets_short>(0));
    EXPECT_EQ(CallStaticIntMethodListHelper(env_, cls, intId), static_cast<ets_int>(0));
    EXPECT_EQ(CallStaticLongMethodListHelper(env_, cls, longId, static_cast<ets_long>(1)), static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(CallStaticFloatMethodListHelper(env_, cls, floatId, static_cast<ets_float>(1.0F)),
                    static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(CallStaticDoubleMethodListHelper(env_, cls, doubleId, static_cast<ets_double>(1.0)),
                     static_cast<ets_double>(0.0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral12)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);

    ets_method voidId = env_->GetStaticp_method(cls, "void_method", "I:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->GetStaticp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->GetStaticp_method(cls, "boolean_method", "Z:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->GetStaticp_method(cls, "byte_method", "B:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->GetStaticp_method(cls, "char_method", "C:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->GetStaticp_method(cls, "short_method", "S:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->GetStaticp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->GetStaticp_method(cls, "long_method", "J:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->GetStaticp_method(cls, "float_method", "F:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->GetStaticp_method(cls, "double_method", "D:D");
    ASSERT_NE(doubleId, nullptr);

    // Check null class argument
    EXPECT_EQ(env_->CallStaticBooleanMethod(nullptr, booleanId, static_cast<ets_boolean>(1)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(env_->CallStaticByteMethod(nullptr, byteId, static_cast<ets_byte>(1)), static_cast<ets_byte>(0));
    EXPECT_EQ(env_->CallStaticCharMethod(nullptr, charId, static_cast<ets_char>(1)), static_cast<ets_char>(0));
    EXPECT_EQ(env_->CallStaticShortMethod(nullptr, shortId, static_cast<ets_short>(1)), static_cast<ets_short>(0));
    EXPECT_EQ(env_->CallStaticIntMethod(nullptr, intId), static_cast<ets_int>(0));
    EXPECT_EQ(env_->CallStaticLongMethod(nullptr, longId, static_cast<ets_long>(1)), static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(env_->CallStaticFloatMethod(nullptr, floatId, static_cast<ets_float>(1.0F)),
                    static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(env_->CallStaticDoubleMethod(nullptr, doubleId, static_cast<ets_double>(1.0)),
                     static_cast<ets_double>(0.0));
}

TEST_F(MethodsTest, CallMethodsTestGeneral13)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);

    ets_method voidId = env_->GetStaticp_method(cls, "void_method", "I:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->GetStaticp_method(cls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->GetStaticp_method(cls, "boolean_method", "Z:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->GetStaticp_method(cls, "byte_method", "B:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->GetStaticp_method(cls, "char_method", "C:C");
    ASSERT_NE(charId, nullptr);
    ets_method shortId = env_->GetStaticp_method(cls, "short_method", "S:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->GetStaticp_method(cls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->GetStaticp_method(cls, "long_method", "J:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->GetStaticp_method(cls, "float_method", "F:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->GetStaticp_method(cls, "double_method", "D:D");
    ASSERT_NE(doubleId, nullptr);

    EXPECT_EQ(CallStaticBooleanMethodListHelper(env_, nullptr, booleanId, static_cast<ets_boolean>(1)),
              static_cast<ets_boolean>(0));
    EXPECT_EQ(CallStaticByteMethodListHelper(env_, nullptr, byteId, static_cast<ets_byte>(1)),
              static_cast<ets_byte>(0));
    EXPECT_EQ(CallStaticCharMethodListHelper(env_, nullptr, charId, static_cast<ets_char>(1)),
              static_cast<ets_char>(0));
    EXPECT_EQ(CallStaticShortMethodListHelper(env_, nullptr, shortId, static_cast<ets_short>(1)),
              static_cast<ets_short>(0));
    EXPECT_EQ(CallStaticIntMethodListHelper(env_, nullptr, intId), static_cast<ets_int>(0));
    EXPECT_EQ(CallStaticLongMethodListHelper(env_, nullptr, longId, static_cast<ets_long>(1)),
              static_cast<ets_long>(0));
    EXPECT_FLOAT_EQ(CallStaticFloatMethodListHelper(env_, nullptr, floatId, static_cast<ets_float>(1.0F)),
                    static_cast<ets_float>(0.0F));
    EXPECT_DOUBLE_EQ(CallStaticDoubleMethodListHelper(env_, nullptr, doubleId, static_cast<ets_double>(1.0)),
                     static_cast<ets_double>(0.0));
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
