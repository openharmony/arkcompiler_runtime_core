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

#include "verify_api.h"
#include "test_helper.h"

#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdio>
#include <string>
#include <vector>

namespace ark::static_verifier {

namespace {

std::string WriteTempAbcFile(const std::vector<uint8_t> &data)
{
    char tmpl[] = "/tmp/sav_api_test_XXXXXX";
    int fd = mkstemp(tmpl);
    EXPECT_GE(fd, 0);
    if (fd < 0) {
        return "";
    }
    ssize_t w = write(fd, data.data(), data.size());
    close(fd);
    EXPECT_EQ(static_cast<ssize_t>(data.size()), w);
    if (w != static_cast<ssize_t>(data.size())) {
        std::remove(tmpl);
        return "";
    }
    return std::string(tmpl);
}

}  // namespace

class VerifyApiTest : public testing::Test {};

TEST_F(VerifyApiTest, VerifyFileNull)
{
    char errorBuf[1024] = {0};
    int ret = StaticAbcVerifyFile(nullptr, errorBuf, sizeof(errorBuf));
    EXPECT_NE(ret, 0);
}

TEST_F(VerifyApiTest, VerifyFileNonExistent)
{
    char errorBuf[1024] = {0};
    int ret = StaticAbcVerifyFile("/nonexistent/path/abc.abc", errorBuf, sizeof(errorBuf));
    EXPECT_NE(ret, 0);
}

TEST_F(VerifyApiTest, VerifyFileValid)
{
    auto data = test::BuildMinimalValidAbc();
    std::string path = WriteTempAbcFile(data);
    ASSERT_FALSE(path.empty());

    char errorBuf[1024] = {0};
    int ret = StaticAbcVerifyFile(path.c_str(), errorBuf, sizeof(errorBuf));
    EXPECT_EQ(ret, 0);

    std::remove(path.c_str());
}

TEST_F(VerifyApiTest, VerifyFileInvalidMagic)
{
    auto data = test::BuildMinimalValidAbc();
    data[0] = 'X';

    std::string path = WriteTempAbcFile(data);
    ASSERT_FALSE(path.empty());

    char errorBuf[1024] = {0};
    int ret = StaticAbcVerifyFile(path.c_str(), errorBuf, sizeof(errorBuf));
    EXPECT_NE(ret, 0);
    EXPECT_NE(std::string(errorBuf).find("magic"), std::string::npos);

    std::remove(path.c_str());
}

TEST_F(VerifyApiTest, VerifyFileNullErrorBuf)
{
    auto data = test::BuildMinimalValidAbc();
    std::string path = WriteTempAbcFile(data);
    ASSERT_FALSE(path.empty());

    int ret = StaticAbcVerifyFile(path.c_str(), nullptr, 0);
    EXPECT_EQ(ret, 0);

    std::remove(path.c_str());
}

TEST_F(VerifyApiTest, VerifyFileSmallErrorBuf)
{
    auto data = test::BuildMinimalValidAbc();
    data[0] = 'X';

    std::string path = WriteTempAbcFile(data);
    ASSERT_FALSE(path.empty());

    char errorBuf[5] = {0};
    int ret = StaticAbcVerifyFile(path.c_str(), errorBuf, sizeof(errorBuf));
    EXPECT_NE(ret, 0);
    EXPECT_EQ(strlen(errorBuf), 4u);

    std::remove(path.c_str());
}

}  // namespace ark::static_verifier
