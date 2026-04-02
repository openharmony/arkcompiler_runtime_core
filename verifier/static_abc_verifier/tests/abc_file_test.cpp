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

#include "abc_file.h"
#include "test_helper.h"

#include <gtest/gtest.h>

namespace ark::static_verifier {

class AbcFileTest : public testing::Test {};

TEST_F(AbcFileTest, OpenFromMemoryValid)
{
    auto data = test::BuildMinimalValidAbc();
    auto file = AbcFile::OpenFromMemory(data.data(), data.size());

    ASSERT_NE(file, nullptr);
    EXPECT_TRUE(file->IsValid());
    EXPECT_EQ(file->GetFileSize(), data.size());
}

TEST_F(AbcFileTest, OpenFromMemoryNull)
{
    auto file = AbcFile::OpenFromMemory(nullptr, 0);
    EXPECT_EQ(file, nullptr);
}

TEST_F(AbcFileTest, OpenFromMemoryTooSmall)
{
    uint8_t smallData[4] = {0};
    auto file = AbcFile::OpenFromMemory(smallData, sizeof(smallData));
    EXPECT_EQ(file, nullptr);
}

TEST_F(AbcFileTest, OpenNonExistentFile)
{
    auto file = AbcFile::Open("/tmp/non_existent_abc_file_12345.abc");
    EXPECT_EQ(file, nullptr);
}

TEST_F(AbcFileTest, GetHeaderValid)
{
    auto data = test::BuildMinimalValidAbc();
    auto file = AbcFile::OpenFromMemory(data.data(), data.size());

    ASSERT_NE(file, nullptr);
    auto *header = file->GetHeader();
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(header->magic, ABC_MAGIC);
    EXPECT_EQ(header->version, STATIC_MIN_VERSION);
}

TEST_F(AbcFileTest, GetClassIndexWithClasses)
{
    constexpr uint32_t classCount = 3;
    auto data = test::BuildAbcWithClasses(classCount);
    auto file = AbcFile::OpenFromMemory(data.data(), data.size());

    ASSERT_NE(file, nullptr);
    EXPECT_EQ(file->GetNumClasses(), static_cast<uint32_t>(classCount));

    auto *classIdx = file->GetClassIndex();
    ASSERT_NE(classIdx, nullptr);
    for (uint32_t i = 0; i < classCount; ++i) {
        EXPECT_EQ(classIdx[i], sizeof(AbcHeader));
    }
}

TEST_F(AbcFileTest, IsRangeValid)
{
    auto data = test::BuildMinimalValidAbc();
    auto file = AbcFile::OpenFromMemory(data.data(), data.size());

    ASSERT_NE(file, nullptr);
    EXPECT_TRUE(file->IsRangeValid(0, sizeof(AbcHeader)));
    EXPECT_FALSE(file->IsRangeValid(0, data.size() + 1));
    EXPECT_FALSE(file->IsRangeValid(data.size(), 1));
}

}  // namespace ark::static_verifier
