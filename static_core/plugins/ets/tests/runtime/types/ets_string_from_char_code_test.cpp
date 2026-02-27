/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "intrinsics.h"
#include "entrypoints_gen.h"
#include "types/ets_box_primitive.h"
#include "types/ets_box_primitive-inl.h"
#include "types/ets_array.h"
#include "types/ets_string.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_string_helpers.h"
#include <climits>

namespace ark::ets::test {
class BasicEtsStringFromCharCodeTest : public testing::Test {
public:
    BasicEtsStringFromCharCodeTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
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

    ~BasicEtsStringFromCharCodeTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(BasicEtsStringFromCharCodeTest);
    NO_MOVE_SEMANTIC(BasicEtsStringFromCharCodeTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    template <typename IntIter>
    EtsObjectArray *CreateCharCodeArray(IntIter first, IntIter last)
    {
        using CharCode = EtsBoxPrimitive<EtsInt>;
        EtsClass *klass = CharCode::GetEtsBoxClass(coroutine_);
        ASSERT(klass != nullptr);
        auto length = std::distance(first, last);
        EtsObjectArray *charCodeArray = EtsObjectArray::Create(klass, length);
        std::for_each(first, last, [&charCodeArray, this, idx = 0U](int d) mutable {
            auto *boxedValue = CharCode::Create(coroutine_, d);
            charCodeArray->Set(idx++, boxedValue);
        });
        return charCodeArray;
    }

    EtsEscompatArray *CreateEtsEscompatArray(const std::vector<int> &codes)
    {
        // Allocate and create the buffer
        auto *buffer = CreateCharCodeArray(codes.begin(), codes.end());
        // Allocate the array object
        auto *array = EtsEscompatArray::Create(coroutine_, codes.size());
        // Fill the array with the pre-created buffer
        ObjectAccessor::SetObject(coroutine_, array, EtsEscompatArray::GetBufferOffset(), buffer->GetCoreType());
        return array;
    }

    template <typename IntIter>
    EtsString *CreateNewStringFromCharCodes(IntIter first, IntIter last)
    {
        EtsObjectArray *charCodeArray = CreateCharCodeArray<IntIter>(first, last);
        return intrinsics::helpers::CreateNewStringFromCharCode(charCodeArray, std::distance(first, last));
    }

    EtsString *CreateNewStringFromCharCodes(const std::vector<int> &codes)
    {
        return CreateNewStringFromCharCodes(codes.begin(), codes.end());
    }

    static EtsString *CreateNewStringFromCharCode(int code)
    {
        return EtsString::CreateNewStringFromCharCode(code);
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

class EtsStringFromCharCodeTest : public BasicEtsStringFromCharCodeTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(EtsStringFromCharCodeTest, CreateNewCompressedStringFromCharCodes)
{
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("Helloff\n");
    EtsString *stringFromCompressedCharCodes =
        CreateNewStringFromCharCodes({0x48, 0x65, 0x6C, 0x6C, 0x6F, -0xFF9A, 0x0fff0066, 10});
    EXPECT_TRUE(stringFromCompressedCharCodes->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromCompressedCharCodes->GetCoreType()));
}
// NOLINTEND(readability-magic-numbers)

TEST_F(EtsStringFromCharCodeTest, CreateNewEmptyStringFromCharCode)
{
    EtsString *emptyString = EtsString::CreateNewEmptyString();
    EtsString *stringFromCharCodes = CreateNewStringFromCharCodes({});
    EXPECT_TRUE(stringFromCharCodes->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(emptyString->GetCoreType(), stringFromCharCodes->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromMaxAvailableCharCode)
{
    std::vector<uint16_t> data = {UINT16_MAX};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromMaxCharCodes1 = CreateNewStringFromCharCodes({INT_MAX});
    EXPECT_TRUE(stringFromMaxCharCodes1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromMaxCharCodes1->GetCoreType()));

    EtsString *stringFromMaxCharCode1 = CreateNewStringFromCharCode(INT_MAX);
    EXPECT_TRUE(stringFromMaxCharCode1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromMaxCharCode1->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromMinAvailableCharCode)
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr int min = -0xffffffff;
    EtsString *expectedCompressedString = EtsString::CreateFromMUtf8("\x01");
    EtsString *stringFromMaxCharCodes1 = CreateNewStringFromCharCodes({int(min)});
    EXPECT_TRUE(stringFromMaxCharCodes1->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromMaxCharCodes1->GetCoreType()));

    EtsString *stringFromMaxCharCode1 = CreateNewStringFromCharCode(min);
    EXPECT_TRUE(stringFromMaxCharCode1->GetCoreType()->IsMUtf8());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedCompressedString->GetCoreType(),
                                                   stringFromMaxCharCode1->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeCharCode)
{
    const int hugeCharCode = UINT16_MAX + 1;
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromHugeCharCodes1 = CreateNewStringFromCharCodes({hugeCharCode});
    EXPECT_TRUE(stringFromHugeCharCodes1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes1->GetCoreType()));

    EtsString *stringFromHugeCharCode1 = CreateNewStringFromCharCode(hugeCharCode);
    EXPECT_TRUE(stringFromHugeCharCode1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode1->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeNegativeCharCode)
{
    const int negHugeCharCode = -UINT16_MAX - 1;
    std::vector<uint16_t> data = {0};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    EtsString *stringFromHugeCharCodes1 = CreateNewStringFromCharCodes({negHugeCharCode});
    EXPECT_TRUE(stringFromHugeCharCodes1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes1->GetCoreType()));

    EtsString *stringFromHugeCharCode1 = CreateNewStringFromCharCode(negHugeCharCode);
    EXPECT_TRUE(stringFromHugeCharCode1->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCode1->GetCoreType()));
}

TEST_F(EtsStringFromCharCodeTest, CreateNewStringFromHugeCharCodes)
{
    const int max = UINT16_MAX + 1;
    std::vector<uint16_t> data = {0, UINT16_MAX, 0x1};
    EtsString *expectedUncompressedString = EtsString::CreateFromUtf16(data.data(), data.size());
    std::vector<int> charCodes {static_cast<int>(max), static_cast<int>(max - 1), static_cast<int>(max + 1)};
    EtsString *stringFromHugeCharCodes = CreateNewStringFromCharCodes(charCodes);
    EXPECT_TRUE(stringFromHugeCharCodes->GetCoreType()->IsUtf16());
    EXPECT_TRUE(coretypes::String::StringsAreEqual(expectedUncompressedString->GetCoreType(),
                                                   stringFromHugeCharCodes->GetCoreType()));
}

}  // namespace ark::ets::test
