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

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {

static const char *g_testBinFileName = "CallingMethodsTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(g_testBinFileName) {}
};

class MethodsTest : public CallingMethodsTestGeneral {};
class MethodsTestDeath : public CallingMethodsTestGeneral {};

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath15)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cCls = env_->FindClass("C");
    ASSERT_NE(cCls, nullptr);
    ets_class dCls = env_->FindClass("D");
    ASSERT_NE(dCls, nullptr);
    ets_object obj = env_->AllocObject(dCls);
    ASSERT_NE(obj, nullptr);

    ets_method voidId = env_->Getp_method(cCls, "void_method", "II:V");
    ASSERT_NE(voidId, nullptr);
    ets_method objectId = env_->Getp_method(cCls, "object_method", ":LA;");
    ASSERT_NE(objectId, nullptr);
    ets_method booleanId = env_->Getp_method(cCls, "boolean_method", "ZI:Z");
    ASSERT_NE(booleanId, nullptr);
    ets_method byteId = env_->Getp_method(cCls, "byte_method", "BI:B");
    ASSERT_NE(byteId, nullptr);
    ets_method charId = env_->Getp_method(cCls, "char_method", "CI:C");
    ASSERT_NE(charId, nullptr);

    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;
    tmp.i = static_cast<ets_int>(42_I);
    const std::vector<ets_value> voidArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallNonvirtualVoidMethodArray(nullptr, cCls, voidId, voidArgs.data()), "");
    EXPECT_DEATH(env_->CallNonvirtualObjectMethodArray(nullptr, cCls, objectId, nullptr), "");

    tmp.z = static_cast<ets_boolean>(1);
    const std::vector<ets_value> booleanArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethodArray(nullptr, cCls, booleanId, booleanArgs.data()), "");

    tmp.b = static_cast<ets_byte>(1);
    const std::vector<ets_value> byteArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallNonvirtualByteMethodArray(nullptr, cCls, byteId, byteArgs.data()), "");

    tmp.c = static_cast<ets_char>(1);
    const std::vector<ets_value> charArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallNonvirtualCharMethodArray(nullptr, cCls, charId, charArgs.data()), "");

    EXPECT_DEATH(env_->CallNonvirtualVoidMethodArray(nullptr, cCls, voidId, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualObjectMethodArray(nullptr, cCls, objectId, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethodArray(nullptr, cCls, booleanId, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualByteMethodArray(nullptr, cCls, byteId, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualCharMethodArray(nullptr, cCls, charId, nullptr), "");

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> longArgs = {tmp, intTmp};

    EXPECT_DEATH(env_->CallNonvirtualVoidMethodArray(obj, cCls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualObjectMethodArray(obj, cCls, nullptr, longArgs.data()), "");
    EXPECT_DEATH(env_->CallNonvirtualBooleanMethodArray(obj, cCls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualByteMethodArray(obj, cCls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualCharMethodArray(obj, cCls, nullptr, nullptr), "");
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath16)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cCls = env_->FindClass("C");
    ASSERT_NE(cCls, nullptr);
    ets_class dCls = env_->FindClass("D");
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
    ets_value intTmp;
    intTmp.i = 121_I;
    ets_value tmp;

    tmp.s = static_cast<ets_short>(1);
    const std::vector<ets_value> shortArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallNonvirtualShortMethodArray(nullptr, cCls, shortId, shortArgs.data()), "");

    EXPECT_DEATH(env_->CallNonvirtualIntMethodArray(nullptr, cCls, intId, nullptr), "");

    tmp.j = static_cast<ets_long>(1);
    const std::vector<ets_value> longArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallNonvirtualLongMethodArray(nullptr, cCls, longId, longArgs.data()), "");

    tmp.j = static_cast<ets_float>(1.0F);
    const std::vector<ets_value> floatArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallNonvirtualFloatMethodArray(nullptr, cCls, floatId, floatArgs.data()), "");

    tmp.j = static_cast<ets_double>(1.0);
    const std::vector<ets_value> doubleArgs = {tmp, intTmp};
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethodArray(nullptr, cCls, doubleId, doubleArgs.data()), "");

    EXPECT_DEATH(env_->CallNonvirtualShortMethodArray(nullptr, cCls, shortId, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualIntMethodArray(nullptr, cCls, intId, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualLongMethodArray(nullptr, cCls, longId, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualFloatMethodArray(nullptr, cCls, floatId, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethodArray(nullptr, cCls, doubleId, nullptr), "");

    EXPECT_DEATH(env_->CallNonvirtualShortMethodArray(obj, cCls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualIntMethodArray(obj, cCls, nullptr, longArgs.data()), "");
    EXPECT_DEATH(env_->CallNonvirtualLongMethodArray(obj, cCls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualFloatMethodArray(obj, cCls, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallNonvirtualDoubleMethodArray(obj, cCls, nullptr, nullptr), "");
}

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
