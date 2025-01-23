/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class StringGetUtf8SubStringTest : public AniTest {};

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SubstrSizeMismatchMultiByte)
{
    const std::string example {"🙂🙂"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 30U;
    char utfBuffer[bufferSize];  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    ani_size substrSize = 7U;
    ani_size result = 0U;
    const ani_size utfBufferSize = 30U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, utfBufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(result, substrSize);
    ASSERT_STREQ(utfBuffer, "🙂");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SubstrContainEnd)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {1U};
    const uint32_t scope = 9U;
    for (auto i = 0U; i < scope; ++i) {
        utfBuffer[i] = 1;
    }
    const uint32_t strEnd = 9U;
    utfBuffer[strEnd] = '\0';
    ani_size substrOffset = 0U;
    ani_size substrSize = example.size();
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "example");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_NullString)
{
    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize];  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    ani_status status =
        env_->String_GetUTF8SubString(nullptr, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_NullBuffer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size result = 0U;
    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    const ani_size bufferSize = 100U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, nullptr, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_NullResultPointer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size substrOffset = 0U;
    ani_size substrSize = 5U;
    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize];
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), nullptr);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_ValidSubstring)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "exam");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_OffsetOutOfRange)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 10U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OUT_OF_RANGE);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SizeOutOfRange)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    const ani_size substrOffset = 34U;
    ani_size substrSize = std::numeric_limits<uint32_t>::max() - 8U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer,
                                           std::numeric_limits<uint32_t>::max(), &result);
    ASSERT_EQ(status, ANI_OUT_OF_RANGE);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_SizeOutOfBounds)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    const ani_size substrSize = 10U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_EmptySubstring)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    ani_size substrSize = 0U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, substrSize);
    ASSERT_STREQ(utfBuffer, "");
}

TEST_F(StringGetUtf8SubStringTest, StringGetUtf8SubString_BufferTooSmall)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 3U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size substrOffset = 0U;
    ani_size substrSize = 4U;
    ani_size result = 0U;
    status = env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)