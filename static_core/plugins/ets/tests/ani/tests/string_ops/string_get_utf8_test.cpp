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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class StringGetUtf8StringTest : public AniTest {};

TEST_F(StringGetUtf8StringTest, StringGetUtf8_MultiByte)
{
    const std::string example {"🙂🙂"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 30U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
    ASSERT_STREQ(utfBuffer, "🙂🙂");
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_BasicString)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, example.size());
    ASSERT_STREQ(utfBuffer, "example");
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_NullString)
{
    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    ani_status status = env_->String_GetUTF8(nullptr, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_NullBuffer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    ani_size result = 0U;
    const ani_size bufferSize = 100U;
    status = env_->String_GetUTF8(string, nullptr, bufferSize, &result);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_NullResultPointer)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);

    const uint32_t bufferSize = 100U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, nullptr);
    ASSERT_EQ(status, ANI_INVALID_ARGS);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_BufferTooSmall)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 3U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_BUFFER_TO_SMALL);
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_Managed)
{
    const auto string = static_cast<ani_string>(CallEtsFunction<ani_ref>("string_get_utf8_test", "GetString"));
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    ani_status status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(result, 4U);
    ASSERT_STREQ(utfBuffer, "test");
}

TEST_F(StringGetUtf8StringTest, StringGetUtf8_Repeat)
{
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {0U};  // NOLINT(modernize-avoid-c-arrays)
    ani_size result = 0U;
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; ++i) {
        status = env_->String_GetUTF8(string, utfBuffer, bufferSize, &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_EQ(result, example.size());
        ASSERT_STREQ(utfBuffer, "example");
    }
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
