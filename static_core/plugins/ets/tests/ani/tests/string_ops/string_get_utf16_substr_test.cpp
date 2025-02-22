/**
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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class StringGetUtf16SubStringTest : public AniTest {};

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_SubstrMultiByte)
{
    const uint16_t example[] = {0xD83D, 0xDE42, 0xD83D, 0xDE42, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 30U;
    uint16_t utf16Buffer[bufferSize];
    ani_size substrOffset = 0U;
    ani_size substrSize = 2U;
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    for (ani_size i = 0; i < result; ++i) {
        ASSERT_EQ(utf16Buffer[i], example[i]);
    }
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_SubstrContainEnd)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size substrOffset = 0U;
    ani_size substrSize = sizeof(example) / sizeof(uint16_t) - 1U;
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    for (ani_size i = 0; i < result; ++i) {
        ASSERT_EQ(utf16Buffer[i], example[i]);
    }
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_StringSizeBound1)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 9U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size substrOffset = 0U;
    ani_size substrSize = sizeof(example) / sizeof(uint16_t) - 1U;
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    for (ani_size i = 0; i < result; ++i) {
        ASSERT_EQ(utf16Buffer[i], example[i]);
    }
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_StringSizeBound2)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 9U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size substrOffset = 0U;
    ani_size substrSize = sizeof(example) / sizeof(uint16_t);
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_EQ(utf16Buffer[7U], 0x0000);
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_NullString)
{
    const ani_size bufferSize = 100U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    ani_status status =
        env_->String_GetUTF16SubString(nullptr, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_NullBuffer)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 100U;
    ani_size result = 0U;
    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, nullptr, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_NullResultPointer)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    const ani_size bufferSize = 100U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, nullptr);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}
TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_GetPartialString)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size substrOffset = 2U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    for (size_t i = 0; i < substrSize; i++) {
        ASSERT_EQ(utf16Buffer[i], example[i + substrOffset]);
    }
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_OffsetOutOfRange1)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    const ani_size substrOffset = 10U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OUT_OF_RANGE);
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_OffsetOutOfRange2)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    const ani_size substrOffset = 2U;
    ani_size substrSize = 6U;
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OUT_OF_RANGE);
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_EmptySubstring)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, 0U, 0U, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 0U);
    ASSERT_EQ(utf16Buffer[0U], 0x0000);
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_BufferTooSmall1)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 1U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size substrOffset = 0U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf16SubStringTest, StringGetUtf16SubString_BufferTooSmall2)
{
    const uint16_t example[] = {0x0065, 0x0078, 0x0061, 0x006D, 0x0070, 0x006C, 0x0065, 0x0000};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size substrOffset = 0U;
    const ani_size substrSize = 10U;
    ani_size result = 0U;
    status = env_->String_GetUTF16SubString(string, substrOffset, substrSize, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)