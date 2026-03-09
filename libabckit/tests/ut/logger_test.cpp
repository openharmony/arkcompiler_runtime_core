/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "logger.h"

using namespace testing::ext;

class LoggerTest : public ::testing::Test {};

// Test: test-kind=api, api=Logger::NullBuffer::xsputn, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LoggerTest, null_buffer_xsputn_test_001)
{
    libabckit::NullBuffer nullBuffer;
    const char *testString = "Test string";
    const std::streamsize size = 11;
    const std::streamsize result = nullBuffer.xsputn(testString, size);
    EXPECT_EQ(result, size);
}

// Test: test-kind=api, api=Logger::NullBuffer::xsputn, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LoggerTest, null_buffer_xsputn_test_002)
{
    libabckit::NullBuffer nullBuffer;
    const char *testString = "Test string";
    const std::streamsize size = 0;
    const std::streamsize result = nullBuffer.xsputn(testString, size);
    EXPECT_EQ(result, size);
}

// Test: test-kind=api, api=Logger::NullBuffer::xsputn, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LoggerTest, null_buffer_xsputn_test_003)
{
    libabckit::NullBuffer nullBuffer;
    const char *testString = "Test string";
    const std::streamsize size = 1000;
    const std::streamsize result = nullBuffer.xsputn(testString, size);
    EXPECT_EQ(result, size);
}

// Test: test-kind=api, api=Logger::NullBuffer::xsputn, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LoggerTest, null_buffer_xsputn_test_004)
{
    libabckit::NullBuffer nullBuffer;
    const char *testString = nullptr;
    const std::streamsize size = 10;
    const std::streamsize result = nullBuffer.xsputn(testString, size);
    EXPECT_EQ(result, size);
}
