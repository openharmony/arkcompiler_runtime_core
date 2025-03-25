/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

// NOLINTBEGIN(modernize-avoid-c-arrays, readability-magic-numbers)

namespace ark::ets::test {

class EtsNativeInterfaceArrayTest : public MockEtsNapiTestBaseClass {};

TEST_F(EtsNativeInterfaceArrayTest, DISABLED_GetBoolArrayRegionErrorTests)
{
    ets_boolean buf[10U] = {0};
    ets_booleanArray arr = env_->NewBooleanArray(5U);
    EXPECT_DEATH(env_->GetBooleanArrayRegion(arr, 0, 1, nullptr), "");

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetBooleanArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetBooleanArrayRegion(arr, 0, -2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetBooleanArrayRegion(arr, 2_I, 9_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, DISABLED_GetByteArrayRegionErrorTests)
{
    ets_byte buf[10U] = {0};
    ets_byteArray arr = env_->NewByteArray(5U);
    EXPECT_DEATH(env_->GetByteArrayRegion(arr, 0, 1, nullptr), "");

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetByteArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetByteArrayRegion(arr, 0, -2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetByteArrayRegion(arr, 2_I, 9_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, DISABLED_GetCharArrayRegionErrorTests)
{
    ets_char buf[10U] = {0};
    ets_charArray arr = env_->NewCharArray(5U);
    EXPECT_DEATH(env_->GetCharArrayRegion(arr, 0, 1, nullptr), "");

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetCharArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetCharArrayRegion(arr, 0, -2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetCharArrayRegion(arr, 2_I, 9_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, DISABLED_GetShortArrayRegionErrorTests)
{
    ets_short buf[10U] = {0};
    ets_shortArray arr = env_->NewShortArray(5U);
    EXPECT_DEATH(env_->GetShortArrayRegion(arr, 0, 1, nullptr), "");

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetShortArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetShortArrayRegion(arr, 0, -2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetShortArrayRegion(arr, 2_I, 9_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, DISABLED_GetIntArrayRegionErrorTests)
{
    ets_int buf[10U] = {0};
    ets_intArray arr = env_->NewIntArray(5U);
    EXPECT_DEATH(env_->GetIntArrayRegion(arr, 0, 1, nullptr), "");

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetIntArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetIntArrayRegion(arr, 0, -2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetIntArrayRegion(arr, 2_I, 9_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, DISABLED_GetLongArrayRegionErrorTests)
{
    ets_long buf[10U] = {0};
    ets_longArray arr = env_->NewLongArray(5U);
    EXPECT_DEATH(env_->GetLongArrayRegion(arr, 0, 1, nullptr), "");

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetLongArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetLongArrayRegion(arr, 0, -2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetLongArrayRegion(arr, 2_I, 9_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, DISABLED_GetFloatArrayRegionErrorTests)
{
    ets_float buf[10U] = {0};
    ets_floatArray arr = env_->NewFloatArray(5U);
    EXPECT_DEATH(env_->GetFloatArrayRegion(arr, 0, 1, nullptr), "");

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetFloatArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetFloatArrayRegion(arr, 0, -2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetFloatArrayRegion(arr, 2_I, 9_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, DISABLED_GetDoubleArrayRegionErrorTests)
{
    ets_double buf[10U] = {0};
    ets_doubleArray arr = env_->NewDoubleArray(5U);
    EXPECT_DEATH(env_->GetDoubleArrayRegion(arr, 0, 1, nullptr), "");

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetDoubleArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetDoubleArrayRegion(arr, 0, -2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetDoubleArrayRegion(arr, 2_I, 9_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetBoolArrayRegionErrorTests)
{
    ets_boolean buf[10U] = {0};
    ets_booleanArray arr = env_->NewBooleanArray(5_I);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetBooleanArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetByteArrayRegionErrorTests)
{
    ets_byte buf[10U] = {0};
    ets_byteArray arr = env_->NewByteArray(5U);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetByteArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetCharArrayRegionErrorTests)
{
    ets_char buf[10U] = {0};
    ets_charArray arr = env_->NewCharArray(5U);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetCharArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetShortArrayRegionErrorTests)
{
    ets_short buf[10U] = {0};
    ets_shortArray arr = env_->NewShortArray(5U);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetShortArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetIntArrayRegionErrorTests)
{
    ets_int buf[10U] = {0};
    ets_intArray arr = env_->NewIntArray(5U);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetIntArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetLongArrayRegionErrorTests)
{
    ets_long buf[10U] = {0};
    ets_longArray arr = env_->NewLongArray(5U);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetLongArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetFloatArrayRegionErrorTests)
{
    ets_float buf[10U] = {0};
    ets_floatArray arr = env_->NewFloatArray(5U);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetFloatArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetDoubleArrayRegionErrorTests)
{
    ets_double buf[10U] = {0};
    ets_doubleArray arr = env_->NewDoubleArray(5U);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetDoubleArrayRegion(arr, -1, 2_I, buf);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, GetObjectArrayElementErrorTest)
{
    ets_class cls = env_->FindClass("std/core/String");
    ASSERT_NE(cls, nullptr);
    const std::string example {"sample7"};
    ets_string str = env_->NewStringUTF(example.c_str());
    ASSERT_NE(str, nullptr);
    ets_objectArray array = env_->NewObjectsArray(5U, cls, str);
    ASSERT_NE(array, nullptr);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->GetObjectArrayElement(array, -1);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetObjectArrayElement(array, 5_I);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->GetObjectArrayElement(array, 15_I);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetObjectArrayElementErrorTests)
{
    ets_class cls = env_->FindClass("std/core/String");
    ASSERT_NE(cls, nullptr);
    const std::string example {"sample7"};
    ets_string str = env_->NewStringUTF(example.c_str());
    ASSERT_NE(str, nullptr);
    ets_objectArray strArray = env_->NewObjectsArray(5U, cls, str);
    ASSERT_NE(strArray, nullptr);

    const std::string newExample {"new sample"};
    ets_string newStr = env_->NewStringUTF(newExample.c_str());
    ASSERT_NE(newStr, nullptr);

    ets_class errorClass = env_->FindClass("std/core/ArrayIndexOutOfBoundsError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetObjectArrayElement(strArray, -1, newStr);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
    {
        env_->SetObjectArrayElement(strArray, 5_I, newStr);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

TEST_F(EtsNativeInterfaceArrayTest, SetObjectArrayElementErrorTests2)
{
    ets_class cls = env_->FindClass("std/core/String");
    ASSERT_NE(cls, nullptr);
    const std::string example {"sample7"};
    ets_string str = env_->NewStringUTF(example.c_str());
    ASSERT_NE(str, nullptr);
    ets_objectArray strArray = env_->NewObjectsArray(5U, cls, str);
    ASSERT_NE(strArray, nullptr);

    ets_class intCls = env_->FindClass("std/core/Int");
    ASSERT_NE(intCls, nullptr);
    ets_object intObj = env_->AllocObject(intCls);
    ASSERT_NE(intObj, nullptr);

    ets_class errorClass = env_->FindClass("std/core/ArrayStoreError");
    ASSERT_NE(errorClass, nullptr);

    {
        env_->SetObjectArrayElement(strArray, 1, intObj);

        ets_error error = env_->ErrorOccurred();
        ASSERT_NE(error, nullptr);
        env_->ErrorClear();
        ASSERT_EQ(env_->IsInstanceOf(static_cast<ets_object>(error), errorClass), ETS_TRUE);
    }
}

}  // namespace ark::ets::test

// NOLINTEND(modernize-avoid-c-arrays, readability-magic-numbers)
