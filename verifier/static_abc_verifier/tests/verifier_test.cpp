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

#include "verifier.h"
#include "test_helper.h"

#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <cstring>

namespace ark::static_verifier {

class VerifierTest : public testing::Test {};
namespace {
constexpr uint8_t UNSUPPORTED_VERSION_BUILD = 99;
constexpr uint8_t CORRUPTED_BYTE = 0xFF;
constexpr uint32_t FILE_SIZE_PADDING = 100;
constexpr size_t K_CHECKSUM_FIELD_FIRST_BYTE_INDEX = offsetof(AbcHeader, checksum);
constexpr size_t K_CHECKSUM_FIELD_SECOND_BYTE_INDEX = offsetof(AbcHeader, checksum) + 1U;
constexpr uint8_t K_CONTENT_CORRUPT_XOR_MASK = 0xFF;
}  // namespace

// ===================== Magic Tests =====================

TEST_F(VerifierTest, ValidMagic)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyMagic());
}

TEST_F(VerifierTest, InvalidMagic)
{
    auto data = test::BuildMinimalValidAbc();
    data[0] = 'X';

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyMagic());
    ASSERT_FALSE(verifier.GetErrors().empty());
    EXPECT_EQ(verifier.GetErrors()[0].code, ErrorCode::INVALID_MAGIC);
}

// ===================== Version Tests =====================

TEST_F(VerifierTest, ValidVersionMin)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyVersion());
}

TEST_F(VerifierTest, ValidVersionMax)
{
    auto data = test::BuildMinimalValidAbc();
    std::copy_n(STATIC_MAX_VERSION.data(), VERSION_SIZE, data.data() + offsetof(AbcHeader, version));
    // Recompute checksum
    const uint8_t *content = data.data() + FILE_CONTENT_OFFSET;
    uint32_t checksum = test::ComputeAdler32(content, data.size() - FILE_CONTENT_OFFSET);
    std::copy_n(reinterpret_cast<const uint8_t *>(&checksum), sizeof(checksum),
                data.data() + offsetof(AbcHeader, checksum));

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyVersion());
}

TEST_F(VerifierTest, InvalidVersionTooLow)
{
    auto data = test::BuildMinimalValidAbc();
    std::array<uint8_t, VERSION_SIZE> lowVersion = {0, 0, 0, 1};
    std::copy_n(lowVersion.data(), VERSION_SIZE, data.data() + offsetof(AbcHeader, version));
    // Recompute checksum
    const uint8_t *content = data.data() + FILE_CONTENT_OFFSET;
    uint32_t checksum = test::ComputeAdler32(content, data.size() - FILE_CONTENT_OFFSET);
    std::copy_n(reinterpret_cast<const uint8_t *>(&checksum), sizeof(checksum),
                data.data() + offsetof(AbcHeader, checksum));

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyVersion());
    ASSERT_FALSE(verifier.GetErrors().empty());
    EXPECT_EQ(verifier.GetErrors()[0].code, ErrorCode::INVALID_VERSION);
}

TEST_F(VerifierTest, InvalidVersionTooHigh)
{
    auto data = test::BuildMinimalValidAbc();
    std::array<uint8_t, VERSION_SIZE> highVersion = {0, 0, 0, UNSUPPORTED_VERSION_BUILD};
    std::copy_n(highVersion.data(), VERSION_SIZE, data.data() + offsetof(AbcHeader, version));
    const uint8_t *content = data.data() + FILE_CONTENT_OFFSET;
    uint32_t checksum = test::ComputeAdler32(content, data.size() - FILE_CONTENT_OFFSET);
    std::copy_n(reinterpret_cast<const uint8_t *>(&checksum), sizeof(checksum),
                data.data() + offsetof(AbcHeader, checksum));

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyVersion());
}

// ===================== Checksum Tests =====================

TEST_F(VerifierTest, ValidChecksum)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyChecksum());
}

TEST_F(VerifierTest, InvalidChecksum)
{
    auto data = test::BuildMinimalValidAbc();
    // Corrupt checksum
    data[K_CHECKSUM_FIELD_FIRST_BYTE_INDEX] = CORRUPTED_BYTE;
    data[K_CHECKSUM_FIELD_SECOND_BYTE_INDEX] = CORRUPTED_BYTE;

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyChecksum());
    ASSERT_FALSE(verifier.GetErrors().empty());
    EXPECT_EQ(verifier.GetErrors()[0].code, ErrorCode::CHECKSUM_MISMATCH);
}

TEST_F(VerifierTest, CorruptedContentBreaksChecksum)
{
    auto data = test::BuildMinimalValidAbc();
    // Corrupt content after checksum (don't touch magic/checksum)
    data[FILE_CONTENT_OFFSET] ^= K_CONTENT_CORRUPT_XOR_MASK;

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyChecksum());
}

// ===================== File Size Tests =====================

TEST_F(VerifierTest, ValidFileSize)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyFileSize());
}

TEST_F(VerifierTest, FileSizeMismatch)
{
    auto data = test::BuildMinimalValidAbc();
    // Make header claim a bigger size
    uint32_t fakeSize = static_cast<uint32_t>(data.size() + FILE_SIZE_PADDING);
    std::copy_n(reinterpret_cast<const uint8_t *>(&fakeSize), sizeof(fakeSize),
                data.data() + offsetof(AbcHeader, fileSize));

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyFileSize());
    ASSERT_FALSE(verifier.GetErrors().empty());
    EXPECT_EQ(verifier.GetErrors()[0].code, ErrorCode::FILE_SIZE_MISMATCH);
}

// ===================== Header Consistency Tests =====================

TEST_F(VerifierTest, ValidHeaderConsistency)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyHeaderConsistency());
}

TEST_F(VerifierTest, ClassIdxOffExceedsFileSize)
{
    auto data = test::BuildMinimalValidAbc();
    auto *header = reinterpret_cast<AbcHeader *>(data.data());
    header->classIdxOff = header->fileSize + 1;
    header->numClasses = 1;

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyHeaderConsistency());
}

// ===================== Class Index Tests =====================

TEST_F(VerifierTest, ValidClassIndex)
{
    auto data = test::BuildAbcWithClasses(5);
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyClassIndex());
}

TEST_F(VerifierTest, ClassIndexOutOfBounds)
{
    auto data = test::BuildAbcWithClasses(2);
    // Make a class offset point beyond file
    auto *header = reinterpret_cast<AbcHeader *>(data.data());
    auto *classIdx = reinterpret_cast<uint32_t *>(data.data() + header->classIdxOff);
    classIdx[1] = header->fileSize + FILE_SIZE_PADDING;

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyClassIndex());
    ASSERT_FALSE(verifier.GetErrors().empty());
    EXPECT_EQ(verifier.GetErrors()[0].code, ErrorCode::INVALID_CLASS_INDEX);
}

// ===================== Full Verify Tests =====================

TEST_F(VerifierTest, FullVerifyValid)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    auto result = verifier.Verify();
    EXPECT_TRUE(result.valid);
    EXPECT_TRUE(result.errors.empty());
}

TEST_F(VerifierTest, FullVerifyWithClasses)
{
    auto data = test::BuildAbcWithClasses(10);
    StaticAbcVerifier verifier(data.data(), data.size());
    auto result = verifier.Verify();
    EXPECT_TRUE(result.valid);
}

TEST_F(VerifierTest, FullVerifyNull)
{
    StaticAbcVerifier verifier(nullptr, 0);
    auto result = verifier.Verify();
    EXPECT_FALSE(result.valid);
    EXPECT_FALSE(result.errors.empty());
}

TEST_F(VerifierTest, FullVerifyNonExistentFile)
{
    StaticAbcVerifier verifier("/nonexistent/path/file.abc");
    auto result = verifier.Verify();
    EXPECT_FALSE(result.valid);
}

// ===================== Region Header Tests =====================

TEST_F(VerifierTest, NoRegionHeadersIsValid)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyRegionHeaders());
}

// ===================== Foreign Section Tests =====================

TEST_F(VerifierTest, NoForeignSectionIsValid)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyForeignSection());
}

TEST_F(VerifierTest, ForeignSectionOutOfBounds)
{
    auto data = test::BuildMinimalValidAbc();
    auto *header = reinterpret_cast<AbcHeader *>(data.data());
    header->foreignOff = 0;
    header->foreignSize = header->fileSize + FILE_SIZE_PADDING;

    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_FALSE(verifier.VerifyForeignSection());
}

}  // namespace ark::static_verifier
