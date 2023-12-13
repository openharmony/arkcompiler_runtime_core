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

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath7)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);

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
    EXPECT_DEATH(env_->CallVoidMethod(nullptr, voidId, static_cast<ets_int>(42_I), static_cast<ets_int>(121_I)), "");
    EXPECT_DEATH(env_->CallObjectMethod(nullptr, objectId), "");
    EXPECT_DEATH(env_->CallBooleanMethod(nullptr, booleanId, static_cast<ets_boolean>(1), static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(env_->CallByteMethod(nullptr, byteId, static_cast<ets_byte>(1), static_cast<ets_int>(121_I)), "");
    EXPECT_DEATH(env_->CallCharMethod(nullptr, charId, static_cast<ets_char>(1), static_cast<ets_int>(121_I)), "");
    EXPECT_DEATH(env_->CallShortMethod(nullptr, shortId, static_cast<ets_short>(1), static_cast<ets_int>(121_I)), "");
    EXPECT_DEATH(env_->CallIntMethod(nullptr, intId), "");
    EXPECT_DEATH(env_->CallLongMethod(nullptr, longId, static_cast<ets_long>(1), static_cast<ets_int>(121_I)), "");
    EXPECT_DEATH(env_->CallFloatMethod(nullptr, floatId, static_cast<ets_float>(1.0F), static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(env_->CallDoubleMethod(nullptr, doubleId, static_cast<ets_double>(1.0), static_cast<ets_int>(121_I)),
                 "");

    EXPECT_DEATH(env_->CallVoidMethod(nullptr, voidId), "");
    EXPECT_DEATH(env_->CallObjectMethod(nullptr, objectId), "");
    EXPECT_DEATH(env_->CallBooleanMethod(nullptr, booleanId), "");
    EXPECT_DEATH(env_->CallByteMethod(nullptr, byteId), "");
    EXPECT_DEATH(env_->CallCharMethod(nullptr, charId), "");
    EXPECT_DEATH(env_->CallShortMethod(nullptr, shortId), "");
    EXPECT_DEATH(env_->CallIntMethod(nullptr, intId), "");
    EXPECT_DEATH(env_->CallLongMethod(nullptr, longId), "");
    EXPECT_DEATH(env_->CallFloatMethod(nullptr, floatId), "");
    EXPECT_DEATH(env_->CallDoubleMethod(nullptr, doubleId), "");
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath8)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);

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

    // Call<Type>MethodArray part
    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42_I);
    const std::vector<ets_value> voidArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallVoidMethodArray(nullptr, voidId, voidArgs.data()), "");

    EXPECT_DEATH(env_->CallObjectMethodArray(nullptr, objectId, nullptr), "");

    tmp.z = static_cast<ets_boolean>(1);
    const std::vector<ets_value> booleanArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallBooleanMethodArray(nullptr, booleanId, booleanArgs.data()), "");

    tmp.b = static_cast<ets_byte>(1);
    const std::vector<ets_value> byteArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallByteMethodArray(nullptr, byteId, byteArgs.data()), "");

    tmp.c = static_cast<ets_char>(1);
    const std::vector<ets_value> charArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallCharMethodArray(nullptr, charId, charArgs.data()), "");

    EXPECT_DEATH(env_->CallVoidMethodArray(nullptr, voidId, nullptr), "");
    EXPECT_DEATH(env_->CallObjectMethodArray(nullptr, objectId, nullptr), "");
    EXPECT_DEATH(env_->CallBooleanMethodArray(nullptr, booleanId, nullptr), "");
    EXPECT_DEATH(env_->CallByteMethodArray(nullptr, byteId, nullptr), "");
    EXPECT_DEATH(env_->CallCharMethodArray(nullptr, charId, nullptr), "");
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath9)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);

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
    EXPECT_DEATH(env_->CallShortMethodArray(nullptr, shortId, shortArgs.data()), "");

    EXPECT_DEATH(env_->CallIntMethodArray(nullptr, intId, nullptr), "");

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> longArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallLongMethodArray(nullptr, longId, longArgs.data()), "");

    tmp.j = static_cast<ets_float>(1.0F);
    const std::vector<ets_value> floatArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallFloatMethodArray(nullptr, floatId, floatArgs.data()), "");

    tmp.j = static_cast<ets_double>(1.0);
    const std::vector<ets_value> doubleArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallDoubleMethodArray(nullptr, doubleId, doubleArgs.data()), "");

    EXPECT_DEATH(env_->CallShortMethodArray(nullptr, shortId, nullptr), "");
    EXPECT_DEATH(env_->CallIntMethodArray(nullptr, intId, nullptr), "");
    EXPECT_DEATH(env_->CallLongMethodArray(nullptr, longId, nullptr), "");
    EXPECT_DEATH(env_->CallFloatMethodArray(nullptr, floatId, nullptr), "");
    EXPECT_DEATH(env_->CallDoubleMethodArray(nullptr, doubleId, nullptr), "");
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath10)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);

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
    EXPECT_DEATH(
        CallVoidMethodListHelper(env_, nullptr, voidId, static_cast<ets_int>(42_I), static_cast<ets_int>(121_I)), "");
    EXPECT_DEATH(CallObjectMethodListHelper(env_, nullptr, objectId), "");
    EXPECT_DEATH(
        CallBooleanMethodListHelper(env_, nullptr, booleanId, static_cast<ets_boolean>(1), static_cast<ets_int>(121_I)),
        "");
    EXPECT_DEATH(CallByteMethodListHelper(env_, nullptr, byteId, static_cast<ets_byte>(1), static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(CallCharMethodListHelper(env_, nullptr, charId, static_cast<ets_char>(1), static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(
        CallShortMethodListHelper(env_, nullptr, shortId, static_cast<ets_short>(1), static_cast<ets_int>(121_I)), "");
    EXPECT_DEATH(CallIntMethodListHelper(env_, nullptr, intId), "");
    EXPECT_DEATH(CallLongMethodListHelper(env_, nullptr, longId, static_cast<ets_long>(1), static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(
        CallFloatMethodListHelper(env_, nullptr, floatId, static_cast<ets_float>(1.0F), static_cast<ets_int>(121_I)),
        "");
    EXPECT_DEATH(
        CallDoubleMethodListHelper(env_, nullptr, doubleId, static_cast<ets_double>(1.0), static_cast<ets_int>(121_I)),
        "");
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath11)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("C");
    ASSERT_NE(cls, nullptr);

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
    EXPECT_DEATH(CallVoidMethodListHelper(env_, nullptr, voidId), "");
    EXPECT_DEATH(CallObjectMethodListHelper(env_, nullptr, objectId), "");
    EXPECT_DEATH(CallBooleanMethodListHelper(env_, nullptr, booleanId), "");
    EXPECT_DEATH(CallByteMethodListHelper(env_, nullptr, byteId), "");
    EXPECT_DEATH(CallCharMethodListHelper(env_, nullptr, charId), "");
    EXPECT_DEATH(CallShortMethodListHelper(env_, nullptr, shortId), "");
    EXPECT_DEATH(CallIntMethodListHelper(env_, nullptr, intId), "");
    EXPECT_DEATH(CallLongMethodListHelper(env_, nullptr, longId), "");
    EXPECT_DEATH(CallFloatMethodListHelper(env_, nullptr, floatId), "");
    EXPECT_DEATH(CallDoubleMethodListHelper(env_, nullptr, doubleId), "");
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
