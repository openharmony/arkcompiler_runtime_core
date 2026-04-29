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

#include <cstring>
#include <sstream>
#include <iomanip>

namespace {
constexpr size_t K_VERSION_QUAD_INDEX_MAJOR = 0U;
constexpr size_t K_VERSION_QUAD_INDEX_MINOR = 1U;
constexpr size_t K_VERSION_QUAD_INDEX_PATCH = 2U;
constexpr size_t K_VERSION_QUAD_INDEX_BUILD = 3U;
constexpr uint32_t K_ADLER32_MODULUS = 65521U;
constexpr size_t K_ADLER32_MAX_BLOCK_LENGTH = 5552U;
constexpr int K_HEX_BYTE_FIELD_WIDTH = 2;
}  // namespace

namespace ark::static_verifier {

StaticAbcVerifier::StaticAbcVerifier(const std::string &filename) : filePath_(filename)
{
    file_ = AbcFile::Open(filename);
}

StaticAbcVerifier::StaticAbcVerifier(const uint8_t *data, size_t size)
{
    file_ = AbcFile::OpenFromMemory(data, size);
}

VerifyResult StaticAbcVerifier::Verify()
{
    errors_.clear();

    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "Failed to open or parse ABC file");
        return {false, errors_};
    }

    VerifyMagic();
#ifndef STATIC_ABC_VERIFIER_SKIP_VERSION_CHECK
    VerifyVersion();
#endif
    VerifyChecksum();
    VerifyFileSize();
    VerifyHeaderConsistency();
    VerifyClassIndex();
    VerifyLiteralArrayIndex();
    VerifyRegionHeaders();
    VerifyForeignSection();

#ifdef STATIC_ABC_VERIFIER_USE_ARKFILE
    if (errors_.empty()) {
        VerifyClassesDeep();
        VerifyMethodsAndCode();
        VerifyLiteralArraysDeep();
    }
#endif

    return {errors_.empty(), errors_};
}

bool StaticAbcVerifier::VerifyMagic()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    if (header->magic != ABC_MAGIC) {
        std::ostringstream oss;
        oss << "Invalid magic: expected 'PANDA', got bytes [";
        for (size_t i = 0; i < MAGIC_SIZE; ++i) {
            oss << "0x" << std::hex << std::setw(K_HEX_BYTE_FIELD_WIDTH) << std::setfill('0')
                << static_cast<int>(header->magic[i]);
            if (i + 1 < MAGIC_SIZE) {
                oss << ", ";
            }
        }
        oss << "]";
        AddError(ErrorCode::INVALID_MAGIC, oss.str());
        return false;
    }
    return true;
}

bool StaticAbcVerifier::VerifyVersion()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    if (!VersionInRange(header->version, STATIC_MIN_VERSION, STATIC_MAX_VERSION)) {
        std::ostringstream oss;
        oss << "Unsupported version: "
            << static_cast<int>(header->version[K_VERSION_QUAD_INDEX_MAJOR]) << "."
            << static_cast<int>(header->version[K_VERSION_QUAD_INDEX_MINOR]) << "."
            << static_cast<int>(header->version[K_VERSION_QUAD_INDEX_PATCH]) << "."
            << static_cast<int>(header->version[K_VERSION_QUAD_INDEX_BUILD])
            << ", supported range: "
            << static_cast<int>(STATIC_MIN_VERSION[K_VERSION_QUAD_INDEX_MAJOR]) << "."
            << static_cast<int>(STATIC_MIN_VERSION[K_VERSION_QUAD_INDEX_MINOR]) << "."
            << static_cast<int>(STATIC_MIN_VERSION[K_VERSION_QUAD_INDEX_PATCH]) << "."
            << static_cast<int>(STATIC_MIN_VERSION[K_VERSION_QUAD_INDEX_BUILD])
            << " ~ "
            << static_cast<int>(STATIC_MAX_VERSION[K_VERSION_QUAD_INDEX_MAJOR]) << "."
            << static_cast<int>(STATIC_MAX_VERSION[K_VERSION_QUAD_INDEX_MINOR]) << "."
            << static_cast<int>(STATIC_MAX_VERSION[K_VERSION_QUAD_INDEX_PATCH]) << "."
            << static_cast<int>(STATIC_MAX_VERSION[K_VERSION_QUAD_INDEX_BUILD]);
        AddError(ErrorCode::INVALID_VERSION, oss.str());
        return false;
    }
    return true;
}

bool StaticAbcVerifier::VerifyChecksum()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    if (file_->GetFileSize() < FILE_CONTENT_OFFSET) {
        AddError(ErrorCode::CHECKSUM_MISMATCH, "File too small for checksum verification");
        return false;
    }

    const uint8_t *contentStart = file_->GetBase() + FILE_CONTENT_OFFSET;
    size_t contentLen = file_->GetFileSize() - FILE_CONTENT_OFFSET;
    uint32_t computed = ComputeAdler32(contentStart, contentLen);
    if (header->checksum != computed) {
        std::ostringstream oss;
        oss << "Checksum mismatch: header=0x" << std::hex << header->checksum
            << ", computed=0x" << computed;
        AddError(ErrorCode::CHECKSUM_MISMATCH, oss.str());
        return false;
    }
    return true;
}

bool StaticAbcVerifier::VerifyFileSize()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    if (header->fileSize != file_->GetFileSize()) {
        std::ostringstream oss;
        oss << "File size mismatch: header declares " << header->fileSize
            << " bytes, actual size is " << file_->GetFileSize() << " bytes";
        AddError(ErrorCode::FILE_SIZE_MISMATCH, oss.str());
        return false;
    }
    return true;
}

bool StaticAbcVerifier::VerifyHeaderConsistency()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    bool valid = true;

    if (header->fileSize < sizeof(AbcHeader)) {
        AddError(ErrorCode::INVALID_HEADER, "Header fileSize smaller than header struct");
        valid = false;
    }

    if (header->classIdxOff > 0 && header->classIdxOff >= header->fileSize) {
        AddError(ErrorCode::INVALID_HEADER, "classIdxOff exceeds file size");
        valid = false;
    }

    if (header->literalarrayIdxOff > 0 && header->literalarrayIdxOff >= header->fileSize) {
        AddError(ErrorCode::INVALID_HEADER, "literalarrayIdxOff exceeds file size");
        valid = false;
    }

    if (header->indexSectionOff > 0 && header->indexSectionOff >= header->fileSize) {
        AddError(ErrorCode::INVALID_HEADER, "indexSectionOff exceeds file size");
        valid = false;
    }

    if (header->lnpIdxOff > 0 && header->lnpIdxOff >= header->fileSize) {
        AddError(ErrorCode::INVALID_HEADER, "lnpIdxOff exceeds file size");
        valid = false;
    }

    return valid;
}

bool StaticAbcVerifier::VerifyClassIndex()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    if (header->numClasses == 0) {
        return true;
    }

    uint64_t totalSize = static_cast<uint64_t>(header->numClasses) * sizeof(uint32_t);
    if (!file_->IsRangeValid(header->classIdxOff, static_cast<uint32_t>(totalSize))) {
        AddError(ErrorCode::INVALID_CLASS_INDEX, "Class index region exceeds file bounds");
        return false;
    }

    const auto *classIdx = file_->GetClassIndex();
    if (classIdx == nullptr) {
        AddError(ErrorCode::INVALID_CLASS_INDEX, "Failed to read class index");
        return false;
    }

    bool valid = true;
    for (uint32_t i = 0; i < header->numClasses; ++i) {
        if (classIdx[i] >= header->fileSize) {
            std::ostringstream oss;
            oss << "Class[" << i << "] offset 0x" << std::hex << classIdx[i]
                << " exceeds file size 0x" << header->fileSize;
            AddError(ErrorCode::INVALID_CLASS_INDEX, oss.str());
            valid = false;
        }
    }
    return valid;
}

bool StaticAbcVerifier::VerifyLiteralArrayIndex()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    if (header->numLiteralarrays == 0) {
        return true;
    }

    uint64_t totalSize = static_cast<uint64_t>(header->numLiteralarrays) * sizeof(uint32_t);
    if (!file_->IsRangeValid(header->literalarrayIdxOff, static_cast<uint32_t>(totalSize))) {
        AddError(ErrorCode::INVALID_LITERAL_ARRAY_INDEX,
                 "Literal array index region exceeds file bounds");
        return false;
    }

    const auto *litIdx = file_->GetLiteralArrayIndex();
    if (litIdx == nullptr) {
        AddError(ErrorCode::INVALID_LITERAL_ARRAY_INDEX, "Failed to read literal array index");
        return false;
    }

    bool valid = true;
    for (uint32_t i = 0; i < header->numLiteralarrays; ++i) {
        if (litIdx[i] >= header->fileSize) {
            std::ostringstream oss;
            oss << "LiteralArray[" << i << "] offset 0x" << std::hex << litIdx[i]
                << " exceeds file size 0x" << header->fileSize;
            AddError(ErrorCode::INVALID_LITERAL_ARRAY_INDEX, oss.str());
            valid = false;
        }
    }
    return valid;
}

bool StaticAbcVerifier::VerifyRegionSubIndex(const char *name, uint32_t regionIdx,
                                             uint32_t off, uint32_t size)
{
    if (size == 0) {
        return true;
    }
    if (!file_->IsRangeValid(off, size * sizeof(uint32_t))) {
        std::ostringstream oss;
        oss << "Region[" << regionIdx << "] " << name << " index exceeds file bounds";
        AddError(ErrorCode::INVALID_REGION_HEADER, oss.str());
        return false;
    }
    return true;
}

bool StaticAbcVerifier::VerifyRegionHeaders()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    if (header->numIndexes == 0) {
        return true;
    }

    uint64_t totalSize = static_cast<uint64_t>(header->numIndexes) * sizeof(RegionHeader);
    if (!file_->IsRangeValid(header->indexSectionOff, static_cast<uint32_t>(totalSize))) {
        AddError(ErrorCode::INVALID_REGION_HEADER, "Region headers exceed file bounds");
        return false;
    }

    const auto *regions = file_->GetRegionHeaders();
    if (regions == nullptr) {
        AddError(ErrorCode::INVALID_REGION_HEADER, "Failed to read region headers");
        return false;
    }

    bool valid = true;
    for (uint32_t i = 0; i < header->numIndexes; ++i) {
        const auto &rh = regions[i];
        if (rh.start > rh.end) {
            std::ostringstream oss;
            oss << "Region[" << i << "] start(0x" << std::hex << rh.start
                << ") > end(0x" << rh.end << ")";
            AddError(ErrorCode::INVALID_REGION_HEADER, oss.str());
            valid = false;
        }
        valid &= VerifyRegionSubIndex("class", i, rh.classIdxOff, rh.classIdxSize);
        valid &= VerifyRegionSubIndex("method", i, rh.methodIdxOff, rh.methodIdxSize);
        valid &= VerifyRegionSubIndex("field", i, rh.fieldIdxOff, rh.fieldIdxSize);
        valid &= VerifyRegionSubIndex("proto", i, rh.protoIdxOff, rh.protoIdxSize);
    }
    return valid;
}

bool StaticAbcVerifier::VerifyForeignSection()
{
    if (file_ == nullptr || !file_->IsValid()) {
        AddError(ErrorCode::FILE_OPEN_FAILED, "File not loaded");
        return false;
    }

    const auto *header = file_->GetHeader();
    if (header->foreignSize == 0) {
        return true;
    }

    if (!file_->IsRangeValid(header->foreignOff, header->foreignSize)) {
        AddError(ErrorCode::INVALID_FOREIGN_SECTION,
                 "Foreign section exceeds file bounds");
        return false;
    }
    return true;
}

void StaticAbcVerifier::AddError(ErrorCode code, const std::string &message)
{
    errors_.push_back({code, message});
}

uint32_t StaticAbcVerifier::ComputeAdler32(const uint8_t *data, size_t len)
{
    // largest n such that 255*n*(n+1)/2 + (n+1)*(K_ADLER32_MODULUS-1) < 2^32

    uint32_t a = 1;
    uint32_t b = 0;

    while (len > 0) {
        size_t blockLen = (len > K_ADLER32_MAX_BLOCK_LENGTH) ? K_ADLER32_MAX_BLOCK_LENGTH : len;
        len -= blockLen;
        for (size_t i = 0; i < blockLen; ++i) {
            a += data[i];
            b += a;
        }
        a %= K_ADLER32_MODULUS;
        b %= K_ADLER32_MODULUS;
        data += blockLen;
    }

    return (b << 16U) | a;
}

int StaticAbcVerifier::CompareVersion(const std::array<uint8_t, VERSION_SIZE> &a,
                                      const std::array<uint8_t, VERSION_SIZE> &b)
{
    for (size_t i = 0; i < VERSION_SIZE; ++i) {
        if (a[i] != b[i]) {
            return a[i] < b[i] ? -1 : 1;
        }
    }
    return 0;
}

bool StaticAbcVerifier::VersionInRange(const std::array<uint8_t, VERSION_SIZE> &ver,
                                       const std::array<uint8_t, VERSION_SIZE> &minVer,
                                       const std::array<uint8_t, VERSION_SIZE> &maxVer)
{
    return CompareVersion(ver, minVer) >= 0 && CompareVersion(ver, maxVer) <= 0;
}

}  // namespace ark::static_verifier
