/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "ets_coroutine.h"
#include "ets_platform_types.h"
#include "types/ets_box_primitive.h"
#include "types/ets_box_primitive-inl.h"
#include "types/ets_array.h"
#include "types/ets_string.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {
class EtsStringFromCharCodeTest : public testing::Test {
public:
    EtsStringFromCharCodeTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsStringFromCharCodeTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsStringFromCharCodeTest);
    NO_MOVE_SEMANTIC(EtsStringFromCharCodeTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    template <typename DoubleIter>
    EtsString *CreateNewStringFromCharCodes(DoubleIter first, DoubleIter last)
    {
        using CharCodeArray = EtsString::CharCodeArray;
        using CharCode = std::remove_pointer_t<CharCodeArray::ValueType>;
        EtsClass *klass = CharCode::GetEtsBoxClass(coroutine_);
        ASSERT(klass != nullptr);
        CharCodeArray *charCodeArray = CharCodeArray::Create(klass, std::distance(first, last));
        std::for_each(first, last, [&charCodeArray, this, idx = 0U](double d) mutable {
            auto *boxedValue = CharCode::Create(coroutine_, d);
            charCodeArray->Set(idx++, boxedValue);
        });

        return EtsString::CreateNewStringFromCharCode(charCodeArray);
    }

    EtsString *CreateNewStringFromCharCodes(const std::vector<double> &codes)
    {
        return CreateNewStringFromCharCodes(codes.begin(), codes.end());
    }

    static EtsString *CreateNewStringFromCharCode(double code)
    {
        return EtsString::CreateNewStringFromCharCode(code);
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsStringFromCharCodeTest, CreateNewCompressedStringFromCharCodes)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("Helloff\n");
    EtsString *stringFromCompressedCharCodes =
        CreateNewStringFromCharCodes({0x48, 0x65, 0x6C, 0x6C, 0x6F, 4294901862, 0xffff0066, 10.316});
    ASSERT_TRUE(stringFromCompressedCharCodes->GetCoreType()->IsMUtf8());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewCompressedStringFromCharCode)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("A");
    EtsString *stringFromCompressedCharCodes = CreateNewStringFromCharCodes({0x41});
    ASSERT_TRUE(stringFromCompressedCharCodes->GetCoreType()->IsMUtf8());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCodes->GetCoreType()));

    EtsString *stringFromCompressedCharCode = CreateNewStringFromCharCode(0x41);
    ASSERT_TRUE(stringFromCompressedCharCode->GetCoreType()->IsMUtf8());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewUncompressedStringFromCharCode)
{
    std::vector<uint16_t> data = {0x3B2};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    EtsString *stringFromUncompressedCharCodes = CreateNewStringFromCharCodes({0x3B2});
    ASSERT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));

    EtsString *stringFromUncompressedCharCode = CreateNewStringFromCharCode(0x3B2);
    ASSERT_TRUE(stringFromUncompressedCharCode->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewUncompressedStringFromCharCodes)
{
    std::vector<uint16_t> data = {0x3B2, 'A', 'B', 'C', 'D', 0xac, 0xff9c, 0, 0xffff, 1, 0xffff, 0, 0, 0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    std::vector<double> charCodes {0x3B2,
                                   0x41,
                                   66.3,
                                   67.00009,
                                   68.99998,
                                   172.9999,
                                   -100,
                                   static_cast<double>(0x7fffffffffffffff),
                                   static_cast<double>(0x1fffffffffffff),
                                   static_cast<double>(-0x1fffffffffffff),
                                   static_cast<double>(0xffff),
                                   static_cast<double>(0x10000),
                                   static_cast<double>(0x8000000000000000),
                                   0};
    EtsString *stringFromUncompressedCharCodes = CreateNewStringFromCharCodes(charCodes);
    ASSERT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewEmptyStringFromCharCode)
{
    EtsString *emptyString = EtsString::CreateNewEmptyString();
    EtsString *stringFromCharCodes = CreateNewStringFromCharCodes({});
    ASSERT_TRUE(stringFromCharCodes->GetCoreType()->IsMUtf8());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(emptyString->GetCoreType(), stringFromCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromNaNCharCode)
{
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    EtsString *stringFromUncompressedCharCodes =
        CreateNewStringFromCharCodes({std::numeric_limits<double>::quiet_NaN()});
    ASSERT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));

    EtsString *stringFromUncompressedCharCode = CreateNewStringFromCharCode(std::numeric_limits<double>::quiet_NaN());
    ASSERT_TRUE(stringFromUncompressedCharCode->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromInfinityCharCode)
{
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    EtsString *stringFromUncompressedCharCodes =
        CreateNewStringFromCharCodes({std::numeric_limits<double>::infinity()});
    ASSERT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));

    EtsString *stringFromUncompressedCharCode = CreateNewStringFromCharCode(std::numeric_limits<double>::infinity());
    ASSERT_TRUE(stringFromUncompressedCharCode->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCode->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromNaNAndInfinityCharCodes)
{
    std::vector<uint16_t> data = {0, 0, 0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    EtsString *stringFromUncompressedCharCodes =
        CreateNewStringFromCharCodes({std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
                                      -std::numeric_limits<double>::infinity()});
    ASSERT_TRUE(stringFromUncompressedCharCodes->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromUncompressedCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromMaxAvailableCharCode)
{
    std::vector<uint16_t> data = {0xffff};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    EtsString *stringFromMaxCharCodes1 = CreateNewStringFromCharCodes({9007199254740991.0});
    ASSERT_TRUE(stringFromMaxCharCodes1->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromMaxCharCodes1->GetCoreType()));

    EtsString *stringFromMaxCharCode1 = CreateNewStringFromCharCode(9007199254740991.0);
    ASSERT_TRUE(stringFromMaxCharCode1->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromMaxCharCode1->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromMinAvailableCharCode)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("\x01");
    EtsString *stringFromMaxCharCodes1 = CreateNewStringFromCharCodes({-9007199254740991.0});
    ASSERT_TRUE(stringFromMaxCharCodes1->GetCoreType()->IsMUtf8());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromMaxCharCodes1->GetCoreType()));

    EtsString *stringFromMaxCharCode1 = CreateNewStringFromCharCode(-9007199254740991.0);
    ASSERT_TRUE(stringFromMaxCharCode1->GetCoreType()->IsMUtf8());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromMaxCharCode1->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeCharCode)
{
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    EtsString *stringFromHugeCharCodes1 = CreateNewStringFromCharCodes({18446744073709551616.0});
    ASSERT_TRUE(stringFromHugeCharCodes1->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes1->GetCoreType()));

    EtsString *stringFromHugeCharCode1 = CreateNewStringFromCharCode(18446744073709551616.0);
    ASSERT_TRUE(stringFromHugeCharCode1->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode1->GetCoreType()));

    EtsString *stringFromHugeCharCodes2 = CreateNewStringFromCharCodes({18446744073709551617.0});
    ASSERT_TRUE(stringFromHugeCharCodes2->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes2->GetCoreType()));

    EtsString *stringFromHugeCharCode2 = CreateNewStringFromCharCode(18446744073709551617.0);
    ASSERT_TRUE(stringFromHugeCharCode2->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode2->GetCoreType()));

    EtsString *stringFromHugeCharCodes3 = CreateNewStringFromCharCodes({9007199254740992.0});
    ASSERT_TRUE(stringFromHugeCharCodes3->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes3->GetCoreType()));

    EtsString *stringFromHugeCharCode3 = CreateNewStringFromCharCode(9007199254740992.0);
    ASSERT_TRUE(stringFromHugeCharCode3->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode3->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeNegativeCharCode)
{
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    EtsString *stringFromHugeCharCodes1 = CreateNewStringFromCharCodes({-18446744073709551616.0});
    ASSERT_TRUE(stringFromHugeCharCodes1->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes1->GetCoreType()));

    EtsString *stringFromHugeCharCode1 = CreateNewStringFromCharCode(-18446744073709551616.0);
    ASSERT_TRUE(stringFromHugeCharCode1->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode1->GetCoreType()));

    EtsString *stringFromHugeCharCodes2 = CreateNewStringFromCharCodes({-18446744073709551617.0});
    ASSERT_TRUE(stringFromHugeCharCodes2->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes2->GetCoreType()));

    EtsString *stringFromHugeCharCode2 = CreateNewStringFromCharCode(-18446744073709551617.0);
    ASSERT_TRUE(stringFromHugeCharCode2->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode2->GetCoreType()));

    EtsString *stringFromHugeCharCodes3 = CreateNewStringFromCharCodes({-9007199254740992.0});
    ASSERT_TRUE(stringFromHugeCharCodes3->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes3->GetCoreType()));

    EtsString *stringFromHugeCharCode3 = CreateNewStringFromCharCode(-9007199254740992.0);
    ASSERT_TRUE(stringFromHugeCharCode3->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode3->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeCharCodes)
{
    std::vector<uint16_t> data = {0, 0, 0, 0, 0, 0, 0xffff, 0x1};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), static_cast<ets_int>(data.size()));
    std::vector<double> charCodes {18446744073709551616.0,  18446744073709551617.0,  9007199254740992.0,
                                   -18446744073709551616.0, -18446744073709551617.0, -9007199254740992.0,
                                   9007199254740991.0,      -9007199254740991.0};
    EtsString *stringFromHugeCharCodes = CreateNewStringFromCharCodes(charCodes);
    ASSERT_TRUE(stringFromHugeCharCodes->GetCoreType()->IsUtf16());
    ASSERT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes->GetCoreType()));
}
}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
