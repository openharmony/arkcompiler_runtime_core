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

/**
 * @tc.name: null_buffer_xsputn_test_001
 * @tc.desc: test NullBuffer::xsputn method with positive size
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(LoggerTest, null_buffer_xsputn_test_001, TestSize.Level1)
{
    libabckit::NullBuffer nullBuffer;
    const char *testString = "Test string";
    const std::streamsize size = 11;
    const std::streamsize result = nullBuffer.xsputn(testString, size);
    EXPECT_EQ(result, size);
}

/**
 * @tc.name: null_buffer_xsputn_test_002
 * @tc.desc: test NullBuffer::xsputn method with zero size
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(LoggerTest, null_buffer_xsputn_test_002, TestSize.Level1)
{
    libabckit::NullBuffer nullBuffer;
    const char *testString = "Test string";
    const std::streamsize size = 0;
    const std::streamsize result = nullBuffer.xsputn(testString, size);
    EXPECT_EQ(result, size);
}

/**
 * @tc.name: null_buffer_xsputn_test_003
 * @tc.desc: test NullBuffer::xsputn method with large size
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(LoggerTest, null_buffer_xsputn_test_003, TestSize.Level1)
{
    libabckit::NullBuffer nullBuffer;
    const char *testString = "Test string";
    const std::streamsize size = 1000;
    const std::streamsize result = nullBuffer.xsputn(testString, size);
    EXPECT_EQ(result, size);
}

/**
 * @tc.name: null_buffer_xsputn_test_004
 * @tc.desc: test NullBuffer::xsputn method with null pointer
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(LoggerTest, null_buffer_xsputn_test_004, TestSize.Level1)
{
    libabckit::NullBuffer nullBuffer;
    const char *testString = nullptr;
    const std::streamsize size = 10;
    const std::streamsize result = nullBuffer.xsputn(testString, size);
    EXPECT_EQ(result, size);
}
