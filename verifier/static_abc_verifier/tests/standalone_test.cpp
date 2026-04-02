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

/*
 * Standalone test runner that does NOT require the GTest framework.
 * Used when GTest is unavailable (e.g., in restricted build environments).
 *
 * Build: g++ -std=c++17 -I../src standalone_test.cpp ../src/abc_file.cpp \
 *        ../src/verifier.cpp ../src/verify_api.cpp -o standalone_test
 * Run:   ./standalone_test
 */

#include "abc_file.h"
#include "verifier.h"
#include "verify_api.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>
#include <cstddef>

namespace {

using namespace ark::static_verifier;

int g_testsPassed = 0;
int g_testsFailed = 0;

void ExpectTrue(bool cond, const char *expr, int line)
{
    if (!cond) {
        throw std::runtime_error(std::string("EXPECT_TRUE failed: ") + expr + " at line " +
                                 std::to_string(line));
    }
}

template <class T, class U>
void ExpectEq(const T &lhs, const U &rhs, const char *lhsExpr, const char *rhsExpr, int line)
{
    if (!(lhs == rhs)) {
        throw std::runtime_error(std::string("EXPECT_EQ failed: ") + lhsExpr + " == " + rhsExpr +
                                 " at line " + std::to_string(line));
    }
}

template <class T, class U>
void ExpectNe(const T &lhs, const U &rhs, const char *lhsExpr, const char *rhsExpr, int line)
{
    if (!(lhs != rhs)) {
        throw std::runtime_error(std::string("EXPECT_NE failed: ") + lhsExpr + " != " + rhsExpr +
                                 " at line " + std::to_string(line));
    }
}

void RunTest(const char *name, const std::function<void()> &fn)
{
    std::cout << "  " << name << " ... ";
    try {
        fn();
        std::cout << "PASS" << std::endl;
        g_testsPassed++;
    } catch (const std::exception &e) {
        std::cout << "FAIL: " << e.what() << std::endl;
        g_testsFailed++;
    }
}

constexpr uint8_t CORRUPTED_BYTE = 0xFF;
constexpr uint8_t UNSUPPORTED_VERSION_BUILD = 99;
constexpr size_t K_CHECKSUM_FIELD_FIRST_BYTE_INDEX = offsetof(AbcHeader, checksum);
constexpr size_t K_CHECKSUM_FIELD_SECOND_BYTE_INDEX = offsetof(AbcHeader, checksum) + 1U;

constexpr uint32_t ADLER32_MODULUS = 65521;

uint32_t ComputeAdler32(const uint8_t *data, size_t len)
{
    uint32_t a = 1;
    uint32_t b = 0;
    for (size_t i = 0; i < len; ++i) {
        a = (a + data[i]) % ADLER32_MODULUS;
        b = (b + a) % ADLER32_MODULUS;
    }
    return (b << 16U) | a;
}

std::vector<uint8_t> BuildMinimalValidAbc()
{
    constexpr uint32_t totalSize = sizeof(AbcHeader);
    AbcHeader header {};
    header.magic = ABC_MAGIC;
    header.version = STATIC_MIN_VERSION;
    header.fileSize = totalSize;

    std::vector<uint8_t> data(totalSize);
    std::copy_n(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data.data());

    const uint8_t *content = data.data() + FILE_CONTENT_OFFSET;
    uint32_t checksum = ComputeAdler32(content, totalSize - FILE_CONTENT_OFFSET);
    std::copy_n(reinterpret_cast<const uint8_t *>(&checksum), sizeof(checksum),
                data.data() + offsetof(AbcHeader, checksum));

    return data;
}

// ========== AbcFile tests ==========

void TestAbcFileOpenFromMemory()
{
    auto data = BuildMinimalValidAbc();
    auto file = AbcFile::OpenFromMemory(data.data(), data.size());
    ExpectTrue(file != nullptr, "file != nullptr", __LINE__);
    ExpectTrue(file->IsValid(), "file->IsValid()", __LINE__);
}

void TestAbcFileOpenNull()
{
    auto file = AbcFile::OpenFromMemory(nullptr, 0);
    ExpectTrue(file == nullptr, "file == nullptr", __LINE__);
}

void TestAbcFileOpenTooSmall()
{
    uint8_t small[4] = {};
    auto file = AbcFile::OpenFromMemory(small, sizeof(small));
    ExpectTrue(file == nullptr, "file == nullptr", __LINE__);
}

void TestAbcFileHeader()
{
    auto data = BuildMinimalValidAbc();
    auto file = AbcFile::OpenFromMemory(data.data(), data.size());
    ExpectTrue(file != nullptr, "file != nullptr", __LINE__);
    auto *header = file->GetHeader();
    ExpectTrue(header != nullptr, "header != nullptr", __LINE__);
    ExpectTrue(header->magic == ABC_MAGIC, "header->magic == ABC_MAGIC", __LINE__);
}

// ========== Verifier tests ==========

void TestVerifyValidMagic()
{
    auto data = BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    ExpectTrue(verifier.VerifyMagic(), "verifier.VerifyMagic()", __LINE__);
}

void TestVerifyInvalidMagic()
{
    auto data = BuildMinimalValidAbc();
    data[0] = 'X';
    StaticAbcVerifier verifier(data.data(), data.size());
    ExpectTrue(!verifier.VerifyMagic(), "!verifier.VerifyMagic()", __LINE__);
}

void TestVerifyValidVersion()
{
    auto data = BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    ExpectTrue(verifier.VerifyVersion(), "verifier.VerifyVersion()", __LINE__);
}

void TestVerifyInvalidVersion()
{
    auto data = BuildMinimalValidAbc();
    std::array<uint8_t, VERSION_SIZE> bad = {0, 0, 0, UNSUPPORTED_VERSION_BUILD};
    std::copy_n(bad.data(), VERSION_SIZE, data.data() + offsetof(AbcHeader, version));
    const uint8_t *content = data.data() + FILE_CONTENT_OFFSET;
    uint32_t checksum = ComputeAdler32(content, data.size() - FILE_CONTENT_OFFSET);
    std::copy_n(reinterpret_cast<const uint8_t *>(&checksum), sizeof(checksum),
                data.data() + offsetof(AbcHeader, checksum));

    StaticAbcVerifier verifier(data.data(), data.size());
    ExpectTrue(!verifier.VerifyVersion(), "!verifier.VerifyVersion()", __LINE__);
}

void TestVerifyValidChecksum()
{
    auto data = BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    ExpectTrue(verifier.VerifyChecksum(), "verifier.VerifyChecksum()", __LINE__);
}

void TestVerifyInvalidChecksum()
{
    auto data = BuildMinimalValidAbc();
    data[K_CHECKSUM_FIELD_FIRST_BYTE_INDEX] = CORRUPTED_BYTE;
    data[K_CHECKSUM_FIELD_SECOND_BYTE_INDEX] = CORRUPTED_BYTE;
    StaticAbcVerifier verifier(data.data(), data.size());
    ExpectTrue(!verifier.VerifyChecksum(), "!verifier.VerifyChecksum()", __LINE__);
}

void TestVerifyFileSize()
{
    auto data = BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    ExpectTrue(verifier.VerifyFileSize(), "verifier.VerifyFileSize()", __LINE__);
}

void TestFullVerifyValid()
{
    auto data = BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    auto result = verifier.Verify();
    ExpectTrue(result.valid, "result.valid", __LINE__);
    ExpectTrue(result.errors.empty(), "result.errors.empty()", __LINE__);
}

void TestFullVerifyNull()
{
    StaticAbcVerifier verifier(nullptr, 0);
    auto result = verifier.Verify();
    ExpectTrue(!result.valid, "!result.valid", __LINE__);
}

// ========== C API tests ==========

void TestCApiMemoryValid()
{
    auto data = BuildMinimalValidAbc();
    char errBuf[256] = {};
    int ret = StaticAbcVerifyMemory(data.data(), data.size(), errBuf, sizeof(errBuf));
    ExpectEq(ret, 0, "ret", "0", __LINE__);
}

void TestCApiMemoryNull()
{
    char errBuf[256] = {};
    int ret = StaticAbcVerifyMemory(nullptr, 0, errBuf, sizeof(errBuf));
    ExpectNe(ret, 0, "ret", "0", __LINE__);
}

void TestCApiFileValid()
{
    auto data = BuildMinimalValidAbc();
    std::string path = "/tmp/sav_standalone_test.abc";
    {
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char *>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }

    char errBuf[256] = {};
    int ret = StaticAbcVerifyFile(path.c_str(), errBuf, sizeof(errBuf));
    ExpectEq(ret, 0, "ret", "0", __LINE__);
    std::remove(path.c_str());
}

void TestCApiFileNonExistent()
{
    char errBuf[256] = {};
    int ret = StaticAbcVerifyFile("/no/such/file.abc", errBuf, sizeof(errBuf));
    ExpectNe(ret, 0, "ret", "0", __LINE__);
}

}  // namespace

int main()
{
    std::cout << "=== Static ABC Verifier - Standalone Tests ===" << std::endl;

    std::cout << "\n--- AbcFile ---" << std::endl;
    RunTest("TestAbcFileOpenFromMemory", TestAbcFileOpenFromMemory);
    RunTest("TestAbcFileOpenNull", TestAbcFileOpenNull);
    RunTest("TestAbcFileOpenTooSmall", TestAbcFileOpenTooSmall);
    RunTest("TestAbcFileHeader", TestAbcFileHeader);

    std::cout << "\n--- Verifier ---" << std::endl;
    RunTest("TestVerifyValidMagic", TestVerifyValidMagic);
    RunTest("TestVerifyInvalidMagic", TestVerifyInvalidMagic);
    RunTest("TestVerifyValidVersion", TestVerifyValidVersion);
    RunTest("TestVerifyInvalidVersion", TestVerifyInvalidVersion);
    RunTest("TestVerifyValidChecksum", TestVerifyValidChecksum);
    RunTest("TestVerifyInvalidChecksum", TestVerifyInvalidChecksum);
    RunTest("TestVerifyFileSize", TestVerifyFileSize);
    RunTest("TestFullVerifyValid", TestFullVerifyValid);
    RunTest("TestFullVerifyNull", TestFullVerifyNull);

    std::cout << "\n--- C API ---" << std::endl;
    RunTest("TestCApiMemoryValid", TestCApiMemoryValid);
    RunTest("TestCApiMemoryNull", TestCApiMemoryNull);
    RunTest("TestCApiFileValid", TestCApiFileValid);
    RunTest("TestCApiFileNonExistent", TestCApiFileNonExistent);

    std::cout << "\n=== Results: " << g_testsPassed << " passed, " <<
                 g_testsFailed << " failed ===" << std::endl;

    return g_testsFailed > 0 ? 1 : 0;
}
