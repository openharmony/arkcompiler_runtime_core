/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "plugins/ets/tests/mock/mock_test_helper.h"

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "plugins/ets/runtime/types/ets_method.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace panda::ets::test {

class EtsNativeInterfaceArrayTest : public MockEtsNapiTestBaseClass {};
class EtsNativeInterfaceArrayTestDeath : public MockEtsNapiTestBaseClass {};

TEST_F(EtsNativeInterfaceArrayTestDeath, GetArrayLengthDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    EXPECT_DEATH(env_->GetArrayLength(nullptr), "");
}

TEST_F(EtsNativeInterfaceArrayTestDeath, NewObjectsArrayDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("std/core/String");
    ASSERT_NE(cls, nullptr);
    const std::string example {"sample7"};
    ets_string str = env_->NewStringUTF(example.c_str());
    ASSERT_NE(str, nullptr);

    EXPECT_DEATH(env_->NewObjectsArray(5, nullptr, nullptr), "");
    EXPECT_DEATH(env_->NewObjectsArray(-5, nullptr, nullptr), "");
    EXPECT_DEATH(env_->NewObjectsArray(-5, cls, nullptr), "");
    EXPECT_DEATH(env_->NewObjectsArray(-5, nullptr, str), "");
    EXPECT_DEATH(env_->NewObjectsArray(-5, cls, str), "");
    EXPECT_DEATH(env_->NewObjectsArray(5, nullptr, str), "");
}

TEST_F(EtsNativeInterfaceArrayTestDeath, GetObjectArrayElementDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("std/core/String");
    ASSERT_NE(cls, nullptr);
    const std::string example {"sample7"};
    ets_string str = env_->NewStringUTF(example.c_str());
    ASSERT_NE(str, nullptr);
    ets_objectArray array = env_->NewObjectsArray(5, cls, str);
    ASSERT_NE(array, nullptr);

    EXPECT_DEATH(env_->GetObjectArrayElement(nullptr, -1), "");
    EXPECT_DEATH(env_->GetObjectArrayElement(nullptr, 1), "");
}

TEST_F(EtsNativeInterfaceArrayTestDeath, SetObjectArrayElementDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    EXPECT_DEATH(env_->SetObjectArrayElement(nullptr, 1, nullptr), "");
    EXPECT_DEATH(env_->SetObjectArrayElement(nullptr, -5, nullptr), "");
}

TEST_F(EtsNativeInterfaceArrayTestDeath, NewPrimitiveTypeArrayDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    EXPECT_DEATH(env_->NewBooleanArray(-1), "");
    EXPECT_DEATH(env_->NewByteArray(-1), "");
    EXPECT_DEATH(env_->NewCharArray(-1), "");
    EXPECT_DEATH(env_->NewShortArray(-1), "");
    EXPECT_DEATH(env_->NewIntArray(-1), "");
    EXPECT_DEATH(env_->NewLongArray(-1), "");
    EXPECT_DEATH(env_->NewFloatArray(-1), "");
    EXPECT_DEATH(env_->NewDoubleArray(-1), "");
}

TEST_F(EtsNativeInterfaceArrayTestDeath, GetPrimitiveTypeArrayRegionDeathTests)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        EXPECT_DEATH(env_->GetBooleanArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetByteArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetCharArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetShortArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetIntArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetLongArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetFloatArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetDoubleArrayRegion(nullptr, 0, 1, nullptr), "");
    }

    ets_booleanArray bool_array = env_->NewBooleanArray(10);
    ets_byteArray byte_array = env_->NewByteArray(10);
    ets_charArray char_array = env_->NewCharArray(10);
    ets_shortArray short_array = env_->NewShortArray(10);
    ets_intArray int_array = env_->NewIntArray(10);
    ets_longArray long_array = env_->NewLongArray(10);
    ets_floatArray float_array = env_->NewFloatArray(10);
    ets_doubleArray double_array = env_->NewDoubleArray(10);

    {
        EXPECT_DEATH(env_->GetBooleanArrayRegion(bool_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetByteArrayRegion(byte_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetCharArrayRegion(char_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetShortArrayRegion(short_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetIntArrayRegion(int_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetLongArrayRegion(long_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetFloatArrayRegion(float_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetDoubleArrayRegion(double_array, 0, 1, nullptr), "");
    }
}

TEST_F(EtsNativeInterfaceArrayTestDeath, SetPrimitiveTypeArrayRegionDeathTests)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        EXPECT_DEATH(env_->SetBooleanArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetByteArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetCharArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetShortArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetIntArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetLongArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetFloatArrayRegion(nullptr, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetDoubleArrayRegion(nullptr, 0, 1, nullptr), "");
    }

    ets_booleanArray bool_array = env_->NewBooleanArray(10);
    ets_byteArray byte_array = env_->NewByteArray(10);
    ets_charArray char_array = env_->NewCharArray(10);
    ets_shortArray short_array = env_->NewShortArray(10);
    ets_intArray int_array = env_->NewIntArray(10);
    ets_longArray long_array = env_->NewLongArray(10);
    ets_floatArray float_array = env_->NewFloatArray(10);
    ets_doubleArray double_array = env_->NewDoubleArray(10);

    {
        EXPECT_DEATH(env_->SetBooleanArrayRegion(bool_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetByteArrayRegion(byte_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetCharArrayRegion(char_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetShortArrayRegion(short_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetIntArrayRegion(int_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetLongArrayRegion(long_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetFloatArrayRegion(float_array, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetDoubleArrayRegion(double_array, 0, 1, nullptr), "");
    }
}

TEST_F(EtsNativeInterfaceArrayTestDeath, PinAndUnpinNullPrimitiveTypeArrayDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ASSERT_DEATH(env_->PinByteArray(nullptr), "");
    ASSERT_DEATH(env_->PinBooleanArray(nullptr), "");
    ASSERT_DEATH(env_->PinCharArray(nullptr), "");
    ASSERT_DEATH(env_->PinShortArray(nullptr), "");
    ASSERT_DEATH(env_->PinIntArray(nullptr), "");
    ASSERT_DEATH(env_->PinLongArray(nullptr), "");
    ASSERT_DEATH(env_->PinFloatArray(nullptr), "");
    ASSERT_DEATH(env_->PinDoubleArray(nullptr), "");

    ASSERT_DEATH(env_->UnpinByteArray(nullptr), "");
    ASSERT_DEATH(env_->UnpinBooleanArray(nullptr), "");
    ASSERT_DEATH(env_->UnpinCharArray(nullptr), "");
    ASSERT_DEATH(env_->UnpinShortArray(nullptr), "");
    ASSERT_DEATH(env_->UnpinIntArray(nullptr), "");
    ASSERT_DEATH(env_->UnpinLongArray(nullptr), "");
    ASSERT_DEATH(env_->UnpinFloatArray(nullptr), "");
    ASSERT_DEATH(env_->UnpinDoubleArray(nullptr), "");
}

}  // namespace panda::ets::test

// NOLINTEND(readability-magic-numbers)