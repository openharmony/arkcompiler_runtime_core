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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace ark::ets::test {

static const char *g_testBinFileName = "CallingMethodsTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(g_testBinFileName) {}
};

class MethodsTest : public CallingMethodsTestGeneral {};
class MethodsTestDeath : public CallingMethodsTestGeneral {};

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath1)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    // Call<type>Method part
    EXPECT_DEATH(env_->CallVoidMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallObjectMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallBooleanMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallByteMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallCharMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallShortMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallIntMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallLongMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallFloatMethod(nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallDoubleMethod(nullptr, nullptr), "");

    // Call<type>MethodArray part
    EXPECT_DEATH(env_->CallVoidMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallObjectMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallBooleanMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallByteMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallCharMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallShortMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallIntMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallLongMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallFloatMethodArray(nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallDoubleMethodArray(nullptr, nullptr, nullptr), "");

    // Call<type>MethodList part
    EXPECT_DEATH(CallVoidMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallObjectMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallBooleanMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallByteMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallCharMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallShortMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallIntMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallLongMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallFloatMethodListHelper(env_, nullptr, nullptr, nullptr), "");
    EXPECT_DEATH(CallDoubleMethodListHelper(env_, nullptr, nullptr, nullptr), "");
}

TEST_F(MethodsTestDeath, CallMethodsTestGeneralDeath2)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("A");
    ASSERT_NE(cls, nullptr);
    ets_object obj = env_->AllocObject(cls);
    ASSERT_NE(obj, nullptr);

    // Call<type>Method part
    EXPECT_DEATH(env_->CallVoidMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallObjectMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallBooleanMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallByteMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallCharMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallShortMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallIntMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallLongMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallFloatMethod(obj, nullptr), "");
    EXPECT_DEATH(env_->CallDoubleMethod(obj, nullptr), "");

    // Call<type>MethodArray part
    EXPECT_DEATH(env_->CallVoidMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallObjectMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallBooleanMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallByteMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallCharMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallShortMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallIntMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallLongMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallFloatMethodArray(obj, nullptr, nullptr), "");
    EXPECT_DEATH(env_->CallDoubleMethodArray(obj, nullptr, nullptr), "");

    // Call<type>MethodList part
    EXPECT_DEATH(CallVoidMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallObjectMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallBooleanMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallByteMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallCharMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallShortMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallIntMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallLongMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallFloatMethodListHelper(env_, obj, nullptr, nullptr), "");
    EXPECT_DEATH(CallDoubleMethodListHelper(env_, obj, nullptr, nullptr), "");
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
