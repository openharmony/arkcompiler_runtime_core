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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace ark::ets::test {

static const char *g_testBinFileName = "CallingMethodsTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(g_testBinFileName) {}
};

class MethodsTest : public CallingMethodsTestGeneral {};
class MethodsTestDeath : public CallingMethodsTestGeneral {};

TEST_F(MethodsTestDeath, DISABLED_CallMethodsTestGeneralDeath12)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("CallingMethodsTest/C");
    ASSERT_NE(cls, nullptr);

    ets_method voidId = env_->Getp_method(cls, "void_method", "II:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->Getp_method(cls, "object_method", ":LCallingMethodsTest/A;");
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

    // Call<type>Method part
    EXPECT_DEATH(
        env_->CallNonvirtualVoidMethod(nullptr, cls, voidId, static_cast<ets_int>(1), static_cast<ets_int>(121_I)), "");
    EXPECT_DEATH(env_->CallNonvirtualObjectMethod(nullptr, cls, objectId), "");
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethod(nullptr, cls, booleanId, static_cast<ets_boolean>(1),
                                                   static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(
        env_->CallNonvirtualByteMethod(nullptr, cls, byteId, static_cast<ets_byte>(1), static_cast<ets_int>(121_I)),
        "");
    EXPECT_DEATH(
        env_->CallNonvirtualCharMethod(nullptr, cls, charId, static_cast<ets_char>(1), static_cast<ets_int>(121_I)),
        "");
    EXPECT_DEATH(
        env_->CallNonvirtualShortMethod(nullptr, cls, shortId, static_cast<ets_short>(1), static_cast<ets_int>(121_I)),
        "");
    EXPECT_DEATH(env_->CallNonvirtualIntMethod(nullptr, cls, intId), "");
    EXPECT_DEATH(
        env_->CallNonvirtualLongMethod(nullptr, cls, longId, static_cast<ets_long>(1), static_cast<ets_int>(121_I)),
        "");
    EXPECT_DEATH(env_->CallNonvirtualFloatMethod(nullptr, cls, floatId, static_cast<ets_float>(1.0F),
                                                 static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethod(nullptr, cls, doubleId, static_cast<ets_double>(1.0),
                                                  static_cast<ets_int>(121_I)),
                 "");
}

TEST_F(MethodsTestDeath, DISABLED_CallMethodsTestGeneralDeath13)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("CallingMethodsTest/C");
    ASSERT_NE(cls, nullptr);

    ets_method voidId = env_->Getp_method(cls, "void_method", "II:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->Getp_method(cls, "object_method", ":LCallingMethodsTest/A;");
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

    // Call<type>Method part
    ets_class dCls = env_->FindClass("CallingMethodsTest/D");
    ASSERT_NE(dCls, nullptr);
    ets_object obj = env_->AllocObject(dCls);
    ASSERT_NE(obj, nullptr);

    EXPECT_DEATH(env_->CallNonvirtualVoidMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualObjectMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualByteMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualCharMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualShortMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualIntMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualLongMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualFloatMethod(obj, cls, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethod(obj, cls, nullptr), "");

    EXPECT_DEATH(env_->CallNonvirtualVoidMethod(nullptr, cls, voidId), "");
    EXPECT_DEATH(env_->CallNonvirtualObjectMethod(nullptr, cls, objectId), "");
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethod(nullptr, cls, booleanId), "");
    EXPECT_DEATH(env_->CallNonvirtualByteMethod(nullptr, cls, byteId), "");
    EXPECT_DEATH(env_->CallNonvirtualCharMethod(nullptr, cls, charId), "");
    EXPECT_DEATH(env_->CallNonvirtualShortMethod(nullptr, cls, shortId), "");
    EXPECT_DEATH(env_->CallNonvirtualIntMethod(nullptr, cls, intId), "");
    EXPECT_DEATH(env_->CallNonvirtualLongMethod(nullptr, cls, longId), "");
    EXPECT_DEATH(env_->CallNonvirtualFloatMethod(nullptr, cls, floatId), "");
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethod(nullptr, cls, doubleId), "");
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
