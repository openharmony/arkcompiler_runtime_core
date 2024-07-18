/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "libpandabase/utils/utils.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "plugins/ets/runtime/types/ets_method.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {

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

    EXPECT_DEATH(env_->NewObjectsArray(5_I, nullptr, nullptr), "");
    EXPECT_DEATH(env_->NewObjectsArray(-5_I, nullptr, nullptr), "");
    EXPECT_DEATH(env_->NewObjectsArray(-5_I, cls, nullptr), "");
    EXPECT_DEATH(env_->NewObjectsArray(-5_I, nullptr, str), "");
    EXPECT_DEATH(env_->NewObjectsArray(-5_I, cls, str), "");
    EXPECT_DEATH(env_->NewObjectsArray(5_I, nullptr, str), "");
}

TEST_F(EtsNativeInterfaceArrayTestDeath, GetObjectArrayElementDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("std/core/String");
    ASSERT_NE(cls, nullptr);
    const std::string example {"sample7"};
    ets_string str = env_->NewStringUTF(example.c_str());
    ASSERT_NE(str, nullptr);
    ets_objectArray array = env_->NewObjectsArray(5_I, cls, str);
    ASSERT_NE(array, nullptr);

    EXPECT_DEATH(env_->GetObjectArrayElement(nullptr, -1), "");
    EXPECT_DEATH(env_->GetObjectArrayElement(nullptr, 1), "");
}

TEST_F(EtsNativeInterfaceArrayTestDeath, SetObjectArrayElementDeathTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    EXPECT_DEATH(env_->SetObjectArrayElement(nullptr, 1, nullptr), "");
    EXPECT_DEATH(env_->SetObjectArrayElement(nullptr, -5_I, nullptr), "");
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

    ets_booleanArray boolArray = env_->NewBooleanArray(10U);
    ets_byteArray byteArray = env_->NewByteArray(10U);
    ets_charArray charArray = env_->NewCharArray(10U);
    ets_shortArray shortArray = env_->NewShortArray(10U);
    ets_intArray intArray = env_->NewIntArray(10U);
    ets_longArray longArray = env_->NewLongArray(10U);
    ets_floatArray floatArray = env_->NewFloatArray(10U);
    ets_doubleArray doubleArray = env_->NewDoubleArray(10U);

    {
        EXPECT_DEATH(env_->GetBooleanArrayRegion(boolArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetByteArrayRegion(byteArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetCharArrayRegion(charArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetShortArrayRegion(shortArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetIntArrayRegion(intArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetLongArrayRegion(longArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetFloatArrayRegion(floatArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->GetDoubleArrayRegion(doubleArray, 0, 1, nullptr), "");
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

    ets_booleanArray boolArray = env_->NewBooleanArray(10U);
    ets_byteArray byteArray = env_->NewByteArray(10U);
    ets_charArray charArray = env_->NewCharArray(10U);
    ets_shortArray shortArray = env_->NewShortArray(10U);
    ets_intArray intArray = env_->NewIntArray(10U);
    ets_longArray longArray = env_->NewLongArray(10U);
    ets_floatArray floatArray = env_->NewFloatArray(10U);
    ets_doubleArray doubleArray = env_->NewDoubleArray(10U);

    {
        EXPECT_DEATH(env_->SetBooleanArrayRegion(boolArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetByteArrayRegion(byteArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetCharArrayRegion(charArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetShortArrayRegion(shortArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetIntArrayRegion(intArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetLongArrayRegion(longArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetFloatArrayRegion(floatArray, 0, 1, nullptr), "");
        EXPECT_DEATH(env_->SetDoubleArrayRegion(doubleArray, 0, 1, nullptr), "");
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

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
