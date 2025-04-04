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

TEST_F(MethodsTestDeath, DISABLED_CallMethodsTestGeneralDeath17)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

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
    ets_method booleanId = env_->Getp_method(cCls, "boolean_method", "ZI:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->Getp_method(cCls, "byte_method", "BI:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->Getp_method(cCls, "char_method", "CI:C");
    ASSERT_NE(charId, nullptr);

    EXPECT_DEATH(CallNonvirtualVoidMethodListHelper(env_, nullptr, cCls, voidId, static_cast<ets_int>(42_I),
                                                    static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(CallNonvirtualObjectMethodListHelper(env_, nullptr, cCls, objectId, nullptr), "");
    EXPECT_DEATH(CallNonvirtualBooleanMethodListHelper(env_, nullptr, cCls, booleanId, static_cast<ets_boolean>(1),
                                                       static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(CallNonvirtualByteMethodListHelper(env_, nullptr, cCls, byteId, static_cast<ets_byte>(0),
                                                    static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(CallNonvirtualCharMethodListHelper(env_, nullptr, cCls, charId, static_cast<ets_char>(10U),
                                                    static_cast<ets_int>(121_I)),
                 "");

    EXPECT_DEATH(CallNonvirtualVoidMethodListHelper(env_, obj, cCls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualObjectMethodListHelper(env_, obj, cCls, nullptr, nullptr), "");
    EXPECT_DEATH(CallNonvirtualBooleanMethodListHelper(env_, obj, cCls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualByteMethodListHelper(env_, obj, cCls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualCharMethodListHelper(env_, obj, cCls, nullptr), "");

    EXPECT_DEATH(CallNonvirtualVoidMethodListHelper(env_, nullptr, cCls, voidId), "");
    EXPECT_DEATH(CallNonvirtualObjectMethodListHelper(env_, nullptr, cCls, objectId, nullptr), "");
    EXPECT_DEATH(CallNonvirtualBooleanMethodListHelper(env_, nullptr, cCls, booleanId), "");
    EXPECT_DEATH(CallNonvirtualByteMethodListHelper(env_, nullptr, cCls, byteId), "");
    EXPECT_DEATH(CallNonvirtualCharMethodListHelper(env_, nullptr, cCls, charId), "");
}

TEST_F(MethodsTestDeath, DISABLED_CallMethodsTestGeneralDeath18)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cCls = env_->FindClass("CallingMethodsTest/C");
    ASSERT_NE(cCls, nullptr);
    ets_class dCls = env_->FindClass("CallingMethodsTest/D");
    ASSERT_NE(dCls, nullptr);
    ets_object obj = env_->AllocObject(dCls);
    ASSERT_NE(obj, nullptr);

    ets_method shortId = env_->Getp_method(cCls, "short_method", "SI:S");
    ASSERT_NE(shortId, nullptr);
    ets_method intId = env_->Getp_method(cCls, "int_method", ":I");
    ASSERT_NE(intId, nullptr);
    ets_method longId = env_->Getp_method(cCls, "long_method", "JI:J");
    ASSERT_NE(longId, nullptr);
    ets_method floatId = env_->Getp_method(cCls, "float_method", "FI:F");
    ASSERT_NE(floatId, nullptr);
    ets_method doubleId = env_->Getp_method(cCls, "double_method", "DI:D");
    ASSERT_NE(doubleId, nullptr);

    // CallNonvirtual<Type>MethodArray part
    EXPECT_DEATH(CallNonvirtualShortMethodListHelper(env_, nullptr, cCls, shortId, static_cast<ets_short>(42_I),
                                                     static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(CallNonvirtualIntMethodListHelper(env_, nullptr, cCls, intId, nullptr), "");
    EXPECT_DEATH(CallNonvirtualLongMethodListHelper(env_, nullptr, cCls, longId, static_cast<ets_long>(42_I),
                                                    static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(CallNonvirtualFloatMethodListHelper(env_, nullptr, cCls, floatId, static_cast<ets_float>(1.0F),
                                                     static_cast<ets_int>(121_I)),
                 "");
    EXPECT_DEATH(CallNonvirtualDoubleMethodListHelper(env_, nullptr, cCls, doubleId, static_cast<ets_double>(1.0),
                                                      static_cast<ets_int>(121_I)),
                 "");

    EXPECT_DEATH(CallNonvirtualShortMethodListHelper(env_, obj, cCls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualIntMethodListHelper(env_, obj, cCls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualLongMethodListHelper(env_, obj, cCls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualFloatMethodListHelper(env_, obj, cCls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualDoubleMethodListHelper(env_, obj, cCls, nullptr), "");

    EXPECT_DEATH(CallNonvirtualShortMethodListHelper(env_, nullptr, cCls, shortId), "");
    EXPECT_DEATH(CallNonvirtualIntMethodListHelper(env_, nullptr, cCls, intId), "");
    EXPECT_DEATH(CallNonvirtualLongMethodListHelper(env_, nullptr, cCls, longId), "");
    EXPECT_DEATH(CallNonvirtualFloatMethodListHelper(env_, nullptr, cCls, floatId), "");
    EXPECT_DEATH(CallNonvirtualDoubleMethodListHelper(env_, nullptr, cCls, doubleId), "");
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
