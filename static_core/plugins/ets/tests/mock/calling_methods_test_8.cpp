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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace panda::ets::test {

static const char *TEST_BIN_FILE_NAME = "CallingMethodsTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(TEST_BIN_FILE_NAME) {}
};

class MethodsTest : public CallingMethodsTestGeneral {};
class MethodsTestDeath : public CallingMethodsTestGeneral {};

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath8)
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
    EXPECT_DEATH(CallNonvirtualVoidMethodListHelper(env_, nullptr, c_cls, void_id, static_cast<ets_int>(42),
                                                    static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallNonvirtualObjectMethodListHelper(env_, nullptr, c_cls, object_id, nullptr), "");
    EXPECT_DEATH(CallNonvirtualBooleanMethodListHelper(env_, nullptr, c_cls, boolean_id, static_cast<ets_boolean>(1),
                                                       static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallNonvirtualByteMethodListHelper(env_, nullptr, c_cls, byte_id, static_cast<ets_byte>(0),
                                                    static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallNonvirtualCharMethodListHelper(env_, nullptr, c_cls, char_id, static_cast<ets_char>(10),
                                                    static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallNonvirtualShortMethodListHelper(env_, nullptr, c_cls, short_id, static_cast<ets_short>(42),
                                                     static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallNonvirtualIntMethodListHelper(env_, nullptr, c_cls, int_id, nullptr), "");
    EXPECT_DEATH(CallNonvirtualLongMethodListHelper(env_, nullptr, c_cls, long_id, static_cast<ets_long>(42),
                                                    static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallNonvirtualFloatMethodListHelper(env_, nullptr, c_cls, float_id, static_cast<ets_float>(1.0F),
                                                     static_cast<ets_int>(121)),
                 "");
    EXPECT_DEATH(CallNonvirtualDoubleMethodListHelper(env_, nullptr, c_cls, double_id, static_cast<ets_double>(1.0),
                                                      static_cast<ets_int>(121)),
                 "");

    EXPECT_DEATH(CallNonvirtualVoidMethodListHelper(env_, obj, c_cls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualObjectMethodListHelper(env_, obj, c_cls, nullptr, nullptr), "");
    EXPECT_DEATH(CallNonvirtualBooleanMethodListHelper(env_, obj, c_cls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualByteMethodListHelper(env_, obj, c_cls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualCharMethodListHelper(env_, obj, c_cls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualShortMethodListHelper(env_, obj, c_cls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualIntMethodListHelper(env_, obj, c_cls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualLongMethodListHelper(env_, obj, c_cls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualFloatMethodListHelper(env_, obj, c_cls, nullptr), "");
    EXPECT_DEATH(CallNonvirtualDoubleMethodListHelper(env_, obj, c_cls, nullptr), "");

    EXPECT_DEATH(CallNonvirtualVoidMethodListHelper(env_, nullptr, c_cls, void_id), "");
    EXPECT_DEATH(CallNonvirtualObjectMethodListHelper(env_, nullptr, c_cls, object_id, nullptr), "");
    EXPECT_DEATH(CallNonvirtualBooleanMethodListHelper(env_, nullptr, c_cls, boolean_id), "");
    EXPECT_DEATH(CallNonvirtualByteMethodListHelper(env_, nullptr, c_cls, byte_id), "");
    EXPECT_DEATH(CallNonvirtualCharMethodListHelper(env_, nullptr, c_cls, char_id), "");
    EXPECT_DEATH(CallNonvirtualShortMethodListHelper(env_, nullptr, c_cls, short_id), "");
    EXPECT_DEATH(CallNonvirtualIntMethodListHelper(env_, nullptr, c_cls, int_id), "");
    EXPECT_DEATH(CallNonvirtualLongMethodListHelper(env_, nullptr, c_cls, long_id), "");
    EXPECT_DEATH(CallNonvirtualFloatMethodListHelper(env_, nullptr, c_cls, float_id), "");
    EXPECT_DEATH(CallNonvirtualDoubleMethodListHelper(env_, nullptr, c_cls, double_id), "");
}

}  // namespace panda::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)