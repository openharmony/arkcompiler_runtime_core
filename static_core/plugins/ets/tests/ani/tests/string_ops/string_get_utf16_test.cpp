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

class StringGetUtf16Test : public AniTest {};

TEST_F(StringGetUtf16Test, StringGetUTF16_NullUtf16String)
{
    ani_string string = nullptr;
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    auto status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_EmptyString)
{
    const uint16_t example[] = {0x0000};
    ani_string string;
    auto status = env_->String_NewUTF16(example, 0U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_AsciiString)
{
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    ani_string string;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 5U);
    ASSERT_EQ(utf16Buffer[4U], 0x006F);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_AsciiString1)
{
    const uint16_t example[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x0000};
    ani_string string;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 6U);
    ASSERT_EQ(utf16Buffer[5U], 0x0000);
}

TEST_F(StringGetUtf16Test, StringGetUTF16_NonAsciiString)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 10U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 6U);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferBound1)
{
    const uint16_t example[] = {0x4F60};
    ani_string string;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 2U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 1U);
    ASSERT_EQ(utf16Buffer[0U], 0x4F60);
    ASSERT_EQ(utf16Buffer[1U], 0x0000);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferBound2)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t), &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 8U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 7U);
    ASSERT_EQ(utf16Buffer[6U], 0x0000);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferTooSmall)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string {};
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 6U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferTooSmall1)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    uint16_t utf16Buffer[3U] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, 0U, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
    ASSERT_EQ(result, 0U);
}

TEST_F(StringGetUtf16Test, StringGetutf16BufferTooSmall2)
{
    const uint16_t example[] = {0x4F60, 0x597D, 0x002C, 0x0020, 0x4E16, 0x754C, 0x0000};
    ani_string string;
    auto status = env_->String_NewUTF16(example, sizeof(example) / sizeof(uint16_t) - 1U, &string);
    ASSERT_EQ(status, ANI_OK);
    const ani_size bufferSize = 3U;
    uint16_t utf16Buffer[bufferSize] = {0U};
    ani_size result = 0U;
    status = env_->String_GetUTF16(string, utf16Buffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
    ASSERT_EQ(result, 0U);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)