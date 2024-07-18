/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)

namespace ark::ets::test {

class EtsNativeInterfaceArrayTest : public MockEtsNapiTestBaseClass {};

TEST_F(EtsNativeInterfaceArrayTest, NewPrimitiveArrayTestEmpty)
{
    ets_booleanArray array1 = env_->NewBooleanArray(0);
    ASSERT_NE(array1, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());
    EXPECT_EQ(env_->GetArrayLength(array1), static_cast<ets_size>(0));

    ets_byteArray array2 = env_->NewByteArray(0);
    ASSERT_NE(array2, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());
    EXPECT_EQ(env_->GetArrayLength(array2), static_cast<ets_size>(0));

    ets_charArray array3 = env_->NewCharArray(0);
    ASSERT_NE(array3, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());
    EXPECT_EQ(env_->GetArrayLength(array3), static_cast<ets_size>(0));

    ets_shortArray array4 = env_->NewShortArray(0);
    ASSERT_NE(array4, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());
    EXPECT_EQ(env_->GetArrayLength(array4), static_cast<ets_size>(0));

    ets_intArray array5 = env_->NewIntArray(0);
    ASSERT_NE(array5, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());
    EXPECT_EQ(env_->GetArrayLength(array5), static_cast<ets_size>(0));

    ets_longArray array6 = env_->NewLongArray(0);
    ASSERT_NE(array6, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());
    EXPECT_EQ(env_->GetArrayLength(array6), static_cast<ets_size>(0));

    ets_floatArray array7 = env_->NewFloatArray(0);
    ASSERT_NE(array7, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());
    EXPECT_EQ(env_->GetArrayLength(array7), static_cast<ets_size>(0));

    ets_doubleArray array8 = env_->NewDoubleArray(0);
    ASSERT_NE(array8, nullptr);
    ASSERT_FALSE(env_->ErrorOccurred());
    EXPECT_EQ(env_->GetArrayLength(array8), static_cast<ets_size>(0));
}

TEST_F(EtsNativeInterfaceArrayTest, BooleanArray)
{
    const std::vector<ets_boolean> src = {ETS_FALSE, ETS_FALSE, ETS_FALSE, ETS_TRUE, ETS_FALSE, ETS_FALSE};
    ets_booleanArray array = env_->NewBooleanArray(src.size());
    ASSERT_NE(array, nullptr);

    ets_size length = env_->GetArrayLength(array);
    ASSERT_EQ(length, src.size());

    env_->SetBooleanArrayRegion(array, 0, src.size(), src.data());
    {
        // Use PinBooleanArray
        ets_boolean *buf = env_->PinBooleanArray(array);
        ASSERT_NE(buf, nullptr);
        std::vector<ets_boolean> res1 = {buf, buf + length};
        ASSERT_EQ(res1, src);

        std::vector<ets_boolean> res2(src.size());
        env_->GetBooleanArrayRegion(array, 0, src.size(), res2.data());
        ASSERT_EQ(res2, src);

        env_->UnpinBooleanArray(array);
    }
    {
        // Use GetBooleanArrayRegion
        std::vector<ets_boolean> vec(length);
        env_->GetBooleanArrayRegion(array, 0, length, vec.data());
        ASSERT_EQ(vec, src);
    }
    {
        // Use GetBooleanArrayRegion with shift
        ets_size half = length / 2;
        std::vector<ets_boolean> vec(length - half);
        env_->GetBooleanArrayRegion(array, half, length - half, vec.data());
        ASSERT_TRUE(std::equal(src.begin() + half, src.end(), vec.begin()));
    }
}

TEST_F(EtsNativeInterfaceArrayTest, ByteArray)
{
    const std::vector<ets_byte> src = {std::numeric_limits<ets_byte>::lowest(), -1, 0, 1,
                                       std::numeric_limits<ets_byte>::max()};
    ets_byteArray array = env_->NewByteArray(src.size());
    ASSERT_NE(array, nullptr);

    ets_size length = env_->GetArrayLength(array);
    ASSERT_EQ(length, src.size());

    env_->SetByteArrayRegion(array, 0, src.size(), src.data());
    {
        // Use PinByteArray
        ets_byte *buf = env_->PinByteArray(array);
        ASSERT_NE(buf, nullptr);
        std::vector<ets_byte> res1 = {buf, buf + length};
        ASSERT_EQ(res1, src);

        std::vector<ets_byte> res2(src.size());
        env_->GetByteArrayRegion(array, 0, src.size(), res2.data());
        ASSERT_EQ(res2, src);

        env_->UnpinByteArray(array);
    }
    {
        // Use GetByteArrayRegion
        std::vector<ets_byte> vec(length);
        env_->GetByteArrayRegion(array, 0, length, vec.data());
        ASSERT_EQ(vec, src);
    }
    {
        // Use GetByteArrayRegion with shift
        ets_size half = length / 2;
        std::vector<ets_byte> vec(length - half);
        env_->GetByteArrayRegion(array, half, length - half, vec.data());
        ASSERT_TRUE(std::equal(src.begin() + half, src.end(), vec.begin()));
    }
}

TEST_F(EtsNativeInterfaceArrayTest, CharArray)
{
    const std::vector<ets_char> src = {std::numeric_limits<ets_char>::lowest(), 0, 1, 2, 3,
                                       std::numeric_limits<ets_char>::max()};
    ets_charArray array = env_->NewCharArray(src.size());
    ASSERT_NE(array, nullptr);

    ets_size length = env_->GetArrayLength(array);
    ASSERT_EQ(length, src.size());

    env_->SetCharArrayRegion(array, 0, src.size(), src.data());
    {
        // Use PinCharArray
        ets_char *buf = env_->PinCharArray(array);
        ASSERT_NE(buf, nullptr);
        std::vector<ets_char> res1 = {buf, buf + length};
        ASSERT_EQ(res1, src);

        std::vector<ets_char> res2(src.size());
        env_->GetCharArrayRegion(array, 0, src.size(), res2.data());
        ASSERT_EQ(res2, src);

        env_->UnpinCharArray(array);
    }
    {
        // Use GetCharArrayRegion
        std::vector<ets_char> vec(length);
        env_->GetCharArrayRegion(array, 0, length, vec.data());
        ASSERT_EQ(vec, src);
    }
    {
        // Use GetCharArrayRegion with shift
        ets_size half = length / 2;
        std::vector<ets_char> vec(length - half);
        env_->GetCharArrayRegion(array, half, length - half, vec.data());
        ASSERT_TRUE(std::equal(src.begin() + half, src.end(), vec.begin()));
    }
}

TEST_F(EtsNativeInterfaceArrayTest, ShortArray)
{
    const std::vector<ets_short> src = {std::numeric_limits<ets_short>::lowest(), -1, 0, 1,
                                        std::numeric_limits<ets_short>::max()};
    ets_shortArray array = env_->NewShortArray(src.size());
    ASSERT_NE(array, nullptr);

    ets_size length = env_->GetArrayLength(array);
    ASSERT_EQ(length, src.size());

    env_->SetShortArrayRegion(array, 0, src.size(), src.data());
    {
        // Use PinShortArray
        ets_short *buf = env_->PinShortArray(array);
        ASSERT_NE(buf, nullptr);
        std::vector<ets_short> res1 = {buf, buf + length};
        ASSERT_EQ(res1, src);

        std::vector<ets_short> res2(src.size());
        env_->GetShortArrayRegion(array, 0, src.size(), res2.data());
        ASSERT_EQ(res2, src);

        env_->UnpinShortArray(array);
    }
    {
        // Use GetShortArrayRegion
        std::vector<ets_short> vec(length);
        env_->GetShortArrayRegion(array, 0, length, vec.data());
        ASSERT_EQ(vec, src);
    }
    {
        // Use GetShortArrayRegion with shift
        ets_size half = length / 2;
        std::vector<ets_short> vec(length - half);
        env_->GetShortArrayRegion(array, half, length - half, vec.data());
        ASSERT_TRUE(std::equal(src.begin() + half, src.end(), vec.begin()));
    }
}

TEST_F(EtsNativeInterfaceArrayTest, IntArray)
{
    const std::vector<ets_int> src = {std::numeric_limits<ets_int>::lowest(), -2, -1, 0, 1, 2,
                                      std::numeric_limits<ets_int>::max()};
    ets_intArray array = env_->NewIntArray(src.size());
    ASSERT_NE(array, nullptr);

    ets_size length = env_->GetArrayLength(array);
    ASSERT_EQ(length, src.size());

    env_->SetIntArrayRegion(array, 0, src.size(), src.data());
    {
        // Use PinIntArray
        ets_int *buf = env_->PinIntArray(array);
        ASSERT_NE(buf, nullptr);
        std::vector<ets_int> res1 = {buf, buf + length};
        ASSERT_EQ(res1, src);

        std::vector<ets_int> res2(src.size());
        env_->GetIntArrayRegion(array, 0, src.size(), res2.data());
        ASSERT_EQ(res2, src);

        env_->UnpinIntArray(array);
    }
    {
        // Use GetIntArrayRegion
        std::vector<ets_int> vec(length);
        env_->GetIntArrayRegion(array, 0, length, vec.data());
        ASSERT_EQ(vec, src);
    }
    {
        // Use GetIntArrayRegion with shift
        ets_size half = length / 2;
        std::vector<ets_int> vec(length - half);
        env_->GetIntArrayRegion(array, half, length - half, vec.data());
        ASSERT_TRUE(std::equal(src.begin() + half, src.end(), vec.begin()));
    }
}

TEST_F(EtsNativeInterfaceArrayTest, LongArray)
{
    const std::vector<ets_long> src = {std::numeric_limits<ets_long>::lowest(), -1, 0, 1,
                                       std::numeric_limits<ets_long>::max()};
    ets_longArray array = env_->NewLongArray(src.size());
    ASSERT_NE(array, nullptr);

    ets_size length = env_->GetArrayLength(array);
    ASSERT_EQ(length, src.size());

    env_->SetLongArrayRegion(array, 0, src.size(), src.data());
    {
        // Use PinLongArray
        ets_long *buf = env_->PinLongArray(array);
        ASSERT_NE(buf, nullptr);
        std::vector<ets_long> res1 = {buf, buf + length};
        ASSERT_EQ(res1, src);

        std::vector<ets_long> res2(src.size());
        env_->GetLongArrayRegion(array, 0, src.size(), res2.data());
        ASSERT_EQ(res2, src);

        env_->UnpinLongArray(array);
    }
    {
        // Use GetLongArrayRegion
        std::vector<ets_long> vec(length);
        env_->GetLongArrayRegion(array, 0, length, vec.data());
        ASSERT_EQ(vec, src);
    }
    {
        // Use GetLongArrayRegion with shift
        ets_size half = length / 2;
        std::vector<ets_long> vec(length - half);
        env_->GetLongArrayRegion(array, half, length - half, vec.data());
        ASSERT_TRUE(std::equal(src.begin() + half, src.end(), vec.begin()));
    }
}

TEST_F(EtsNativeInterfaceArrayTest, FloatArray)
{
    const std::vector<ets_float> src = {std::numeric_limits<ets_float>::lowest(), -1.0F, 0.0F, 1.0F,
                                        std::numeric_limits<ets_float>::max()};
    ets_floatArray array = env_->NewFloatArray(src.size());
    ASSERT_NE(array, nullptr);

    ets_size length = env_->GetArrayLength(array);
    ASSERT_EQ(length, src.size());

    env_->SetFloatArrayRegion(array, 0, src.size(), src.data());
    {
        // Use PinFloatArray
        ets_float *buf = env_->PinFloatArray(array);
        ASSERT_NE(buf, nullptr);
        std::vector<ets_float> res1 = {buf, buf + length};
        ASSERT_EQ(res1, src);

        std::vector<ets_float> res2(src.size());
        env_->GetFloatArrayRegion(array, 0, src.size(), res2.data());
        ASSERT_EQ(res2, src);

        env_->UnpinFloatArray(array);
    }
    {
        // Use GetFloatArrayRegion
        std::vector<ets_float> vec(length);
        env_->GetFloatArrayRegion(array, 0, length, vec.data());
        ASSERT_EQ(vec, src);
    }
    {
        // Use GetFloatArrayRegion with shift
        ets_size half = length / 2;
        std::vector<ets_float> vec(length - half);
        env_->GetFloatArrayRegion(array, half, length - half, vec.data());
        ASSERT_TRUE(std::equal(src.begin() + half, src.end(), vec.begin()));
    }
}

TEST_F(EtsNativeInterfaceArrayTest, DoubleArray)
{
    const std::vector<ets_double> src = {std::numeric_limits<ets_double>::lowest(), -1.1, 0, 1.1,
                                         std::numeric_limits<ets_double>::max()};
    ets_doubleArray array = env_->NewDoubleArray(src.size());
    ASSERT_NE(array, nullptr);

    ets_size length = env_->GetArrayLength(array);
    ASSERT_EQ(length, src.size());

    env_->SetDoubleArrayRegion(array, 0, src.size(), src.data());
    {
        // Use PinDoubleArray
        ets_double *buf = env_->PinDoubleArray(array);
        ASSERT_NE(buf, nullptr);
        std::vector<ets_double> res1 = {buf, buf + length};
        ASSERT_EQ(res1, src);

        std::vector<ets_double> res2(src.size());
        env_->GetDoubleArrayRegion(array, 0, src.size(), res2.data());
        ASSERT_EQ(res2, src);

        env_->UnpinDoubleArray(array);
    }
    {
        // Use GetDoubleArrayRegion
        std::vector<ets_double> vec(length);
        env_->GetDoubleArrayRegion(array, 0, length, vec.data());
        ASSERT_EQ(vec, src);
    }
    {
        // Use GetDoubleArrayRegion with shift
        ets_size half = length / 2;
        std::vector<ets_double> vec(length - half);
        env_->GetDoubleArrayRegion(array, half, length - half, vec.data());
        ASSERT_TRUE(std::equal(src.begin() + half, src.end(), vec.begin()));
    }
}

TEST_F(EtsNativeInterfaceArrayTest, NewObjectsArrayZeroAndOneSize)
{
    ets_class cls = env_->FindClass("std/core/String");
    ASSERT_NE(cls, nullptr);
    const std::string example {"sample7"};
    ets_string str = env_->NewStringUTF(example.c_str());
    ASSERT_NE(str, nullptr);

    ets_objectArray arrZero = env_->NewObjectsArray(0, cls, str);
    EXPECT_EQ(env_->GetArrayLength(arrZero), static_cast<ets_size>(0));
    ets_objectArray arrOne = env_->NewObjectsArray(1, cls, str);
    EXPECT_EQ(env_->GetArrayLength(arrOne), static_cast<ets_size>(1));
}

TEST_F(EtsNativeInterfaceArrayTest, NewObjectArrayTest)
{
    //  NOLINTNEXTLINE(bugprone-string-literal-with-embedded-nul)
    const std::vector<std::string> src = {"\t", "\n",       "\0",   "abcdefghijklmnopqrstuvwxyz",
                                          "",   "texttext", "ABCD", "1111111"};
    std::vector<ets_string> etsSrc;
    etsSrc.reserve(src.size());
    for (auto &s : src) {
        etsSrc.push_back(env_->NewStringUTF(s.c_str()));
    }

    ets_class cls = env_->FindClass("std/core/String");
    ASSERT_NE(cls, nullptr);
    ets_objectArray array = env_->NewObjectsArray(src.size(), cls, etsSrc[0]);
    ASSERT_NE(array, nullptr);
    ets_size size = env_->GetArrayLength(array);
    ASSERT_EQ(size, src.size());
    for (ets_size i = 1; i < size; ++i) {
        env_->SetObjectArrayElement(array, i, etsSrc[i]);
    }

    std::vector<std::string> etsRes;
    etsRes.reserve(src.size());
    for (ets_size i = 0; i < size; ++i) {
        auto str = static_cast<ets_string>(env_->GetObjectArrayElement(array, i));
        ets_boolean isCopy;
        const char *res = env_->GetStringUTFChars(str, &isCopy);
        etsRes.emplace_back(res);
        env_->ReleaseStringUTFChars(str, res);
    }
    ASSERT_EQ(etsRes, src);

    ark::ets::napi::ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env_));
    auto *objectArr = s.Convert<EtsObjectArray>(array);
    auto *desc = objectArr->GetClass()->GetDescriptor();

    const char *expectedDesc = "[Lstd/core/String;";
    uint32_t len = 18;
    ASSERT_THAT(std::vector<char>(desc, desc + len), ::testing::ElementsAreArray(expectedDesc, len));
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)