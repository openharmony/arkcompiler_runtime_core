/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "libarkfile/file_writer.h"
#include "libarkbase/utils/span.h"
#include "zlib.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <vector>

#ifndef PANDA_TARGET_WINDOWS
#include <sys/types.h>
#endif

inline bool Unlikely(bool value)
{
#ifdef __GNUC__
    return static_cast<bool>(__builtin_expect(value ? 1 : 0, 0));
#else
    return value;
#endif
}

namespace {

#ifdef PANDA_TARGET_WINDOWS
using SeekOffset = __int64;
#else
using SeekOffset = off_t;
#endif

bool SeekFile(FILE *file, size_t offset, int origin)
{
    if (file == nullptr || offset > static_cast<size_t>(std::numeric_limits<SeekOffset>::max())) {
        return false;
    }

#ifdef PANDA_TARGET_WINDOWS
    return _fseeki64(file, static_cast<SeekOffset>(offset), origin) == 0;
#else
    return fseeko(file, static_cast<SeekOffset>(offset), origin) == 0;
#endif
}

constexpr size_t DEFAULT_CHECKSUM_MIN_PARALLEL_BYTES = 256U * 1024U;
constexpr unsigned DEFAULT_CHECKSUM_THREADS_CAP = 8U;
constexpr int STRTOL_BASE = 10;

unsigned GetChecksumThreadCount(size_t total)
{
    static const unsigned CACHED = []() -> unsigned {
        const char *env = std::getenv("ARK_WRITE_CHECKSUM_THREADS");
        if (env != nullptr) {
            char *endp = nullptr;
            const int64_t v = std::strtoll(env, &endp, STRTOL_BASE);
            if (endp != env && v >= 0) {
                if (v == 0) {
                    return 1U;
                }
                return static_cast<unsigned>(std::min<int64_t>(v, DEFAULT_CHECKSUM_THREADS_CAP));
            }
        }
        unsigned hc = std::thread::hardware_concurrency();
        if (hc == 0U) {
            hc = 2U;
        }
        return std::min(hc, DEFAULT_CHECKSUM_THREADS_CAP);
    }();

    size_t minBytes = DEFAULT_CHECKSUM_MIN_PARALLEL_BYTES;
    if (const char *env = std::getenv("ARK_WRITE_CHECKSUM_MIN_BYTES"); env != nullptr) {
        char *endp = nullptr;
        const int64_t v = std::strtoll(env, &endp, STRTOL_BASE);
        if (endp != env && v > 0) {
            minBytes = static_cast<size_t>(v);
        }
    }
    if (total < minBytes || CACHED <= 1U) {
        return 1U;
    }
    unsigned n = CACHED;
    while (n > 1U && total / n < (minBytes / 2U)) {
        n /= 2U;
    }
    return std::max(n, 1U);
}

uint32_t ComputeAdler32Parallel(const uint8_t *data, size_t size)
{
    if (size == 0U) {
        return adler32(1U, nullptr, 0U);
    }
    unsigned nThreads = GetChecksumThreadCount(size);
    if (nThreads <= 1U) {
        return adler32(1U, data, static_cast<uInt>(size));
    }
    nThreads = std::max(nThreads, 1U);

    const size_t chunk = (size + nThreads - 1U) / nThreads;
    std::vector<uint32_t> partial(nThreads, 1U);
    std::vector<size_t> lens(nThreads, 0U);

    std::vector<std::thread> workers;
    workers.reserve(nThreads - 1U);
    auto doChunk = [data, size, chunk, &partial, &lens](unsigned idx) {
        size_t b = static_cast<size_t>(idx) * chunk;
        size_t e = std::min(b + chunk, size);
        if (b >= e) {
            return;
        }
        auto chunkSp = ark::Span<const uint8_t>(data, size).SubSpan(b, e - b);
        partial[idx] = adler32(1U, chunkSp.data(), static_cast<uInt>(chunkSp.Size()));
        lens[idx] = e - b;
    };
    for (unsigned i = 0; i + 1U < nThreads; ++i) {
        workers.emplace_back([doChunk, i]() { doChunk(i); });
    }
    doChunk(nThreads - 1U);
    for (auto &t : workers) {
        t.join();
    }

    uint32_t result = partial[0];
    for (unsigned i = 1; i < nThreads; ++i) {
        if (lens[i] == 0U) {
            continue;
        }
        result = static_cast<uint32_t>(adler32_combine(result, partial[i], static_cast<z_off_t>(lens[i])));
    }
    return result;
}

void StoreChecksumLE(uint8_t *dst, uint32_t checksum)
{
    static constexpr size_t MASK = 0xff;
    static constexpr size_t WIDTH = std::numeric_limits<uint8_t>::digits;
    auto sumSp = ark::Span<uint8_t>(dst, sizeof(uint32_t));
    uint32_t v = checksum;
    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
        sumSp[i] = static_cast<uint8_t>(v & MASK);
        v >>= WIDTH;
    }
}

}  // namespace

namespace ark::panda_file {

MemoryWriter::MemoryWriter() : checksum_(adler32(0, nullptr, 0)) {}

bool MemoryWriter::WriteByte(uint8_t byte)
{
    if (Unlikely(countChecksum_)) {
        checksum_ = adler32(checksum_, &byte, 1U);
    }
    data_.push_back(byte);
    return true;
}

bool MemoryWriter::WriteBytes(const std::vector<uint8_t> &bytes)
{
    return WriteBytes(bytes.data(), bytes.size());
}

bool MemoryWriter::WriteBytes(const uint8_t *bytes, size_t size)
{
    if (size == 0) {
        return true;
    }
    if (Unlikely(countChecksum_)) {
        checksum_ = adler32(checksum_, bytes, size);
    }
    data_.insert(data_.end(), bytes, bytes + size);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return true;
}

bool MemoryWriter::WriteRawBytes(const uint8_t *data, size_t size)
{
    return WriteBytes(data, size);
}

bool MemoryWriter::AppendRange(const uint8_t *data, size_t size)
{
    if (size == 0) {
        return true;
    }
    data_.insert(data_.end(), data, data + size);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return true;
}

bool MemoryWriter::FinalizeChecksum(size_t contentBeginOffset, size_t checksumStoreOffset)
{
    if (contentBeginOffset > data_.size() || checksumStoreOffset + sizeof(uint32_t) > data_.size()) {
        return false;
    }
    const size_t len = data_.size() - contentBeginOffset;
    auto contentSp = ark::Span<uint8_t>(data_).SubSpan(contentBeginOffset, len);
    checksum_ = ComputeAdler32Parallel(contentSp.data(), len);
    auto sumSp = ark::Span<uint8_t>(data_).SubSpan(checksumStoreOffset, sizeof(uint32_t));
    StoreChecksumLE(sumSp.data(), checksum_);
    return true;
}

MemoryBufferWriter::MemoryBufferWriter(uint8_t *buffer, size_t size)
    : sp_(buffer, size), checksum_(adler32(0, nullptr, 0))
{
}

bool MemoryBufferWriter::WriteByte(uint8_t byte)
{
    if (Unlikely(countChecksum_)) {
        checksum_ = adler32(checksum_, &byte, 1U);
    }
    auto subSp = sp_.SubSpan(offset_, 1U);
    if (memcpy_s(subSp.data(), subSp.size(), &byte, 1U) != 0) {
        return false;
    }
    offset_++;
    return true;
}

bool MemoryBufferWriter::WriteBytes(const std::vector<uint8_t> &bytes)
{
    return WriteBytes(bytes.data(), bytes.size());
}

bool MemoryBufferWriter::WriteBytes(const uint8_t *bytes, size_t size)
{
    if (size == 0) {
        return true;
    }
    if (Unlikely(countChecksum_)) {
        checksum_ = adler32(checksum_, bytes, size);
    }
    auto subSp = sp_.SubSpan(offset_, size);
    if (memcpy_s(subSp.data(), subSp.size(), bytes, size) != 0) {
        return false;
    }
    offset_ += size;
    return true;
}

bool MemoryBufferWriter::WriteRawBytes(const uint8_t *data, size_t size)
{
    return WriteBytes(data, size);
}

bool MemoryBufferWriter::AppendRange(const uint8_t *data, size_t size)
{
    if (size == 0) {
        return true;
    }
    auto subSp = sp_.SubSpan(offset_, size);
    if (memcpy_s(subSp.data(), subSp.size(), data, size) != 0) {
        return false;
    }
    offset_ += size;
    return true;
}

bool MemoryBufferWriter::FinalizeChecksum(size_t contentBeginOffset, size_t checksumStoreOffset)
{
    if (contentBeginOffset > offset_ || checksumStoreOffset + sizeof(uint32_t) > offset_) {
        return false;
    }
    const size_t len = offset_ - contentBeginOffset;
    auto contentSp = sp_.SubSpan(contentBeginOffset, len);
    checksum_ = ComputeAdler32Parallel(contentSp.data(), len);
    auto sumSp = sp_.SubSpan(checksumStoreOffset, sizeof(uint32_t));
    StoreChecksumLE(sumSp.data(), checksum_);
    return true;
}

FileWriter::FileWriter(const std::string &fileName) : checksum_(adler32(0, nullptr, 0))
{
#ifdef PANDA_TARGET_WINDOWS
    constexpr char const *MODE = "wb";
#else
    constexpr char const *MODE = "wbe";
#endif

    file_ = fopen(fileName.c_str(), MODE);
    seekable_ = SeekFile(file_, 0, SEEK_CUR);
}

FileWriter::~FileWriter()
{
    if (file_ != nullptr) {
        fclose(file_);
    }
    file_ = nullptr;
}

void FileWriter::CountChecksum(bool counting)
{
    countChecksum_ = counting;
}

bool FileWriter::WriteChecksum(size_t offset)
{
    static constexpr size_t MASK = 0xff;
    static constexpr size_t WIDTH = std::numeric_limits<uint8_t>::digits;

    if (file_ == nullptr) {
        return false;
    }

    size_t length = sizeof(uint32_t);
    if (offset + length > offset_) {
        return false;
    }
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    uint8_t bytes[sizeof(uint32_t)];
    uint32_t temp = checksum_;
    for (size_t i = 0; i < length; i++) {
        bytes[i] = temp & MASK;
        temp >>= WIDTH;
    }

    if (!seekable_) {
        if (offset + length > buffer_.size()) {
            return false;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return memcpy_s(buffer_.data() + offset, buffer_.size() - offset, bytes, length) == 0;
    }

    if (!FlushBuffer() || !SeekFile(file_, offset, SEEK_SET)) {
        return false;
    }

    if (fwrite(bytes, sizeof(uint8_t), length, file_) != length) {
        return false;
    }
    return SeekFile(file_, offset_, SEEK_SET);
}

bool FileWriter::WriteByte(uint8_t data)
{
    if (file_ == nullptr) {
        return false;
    }
    if (Unlikely(countChecksum_)) {
        checksum_ = adler32(checksum_, &data, 1U);
    }
    buffer_.push_back(data);
    offset_++;
    return true;
}

bool FileWriter::WriteBytes(const std::vector<uint8_t> &bytes)
{
    return WriteBytes(bytes.data(), bytes.size());
}

bool FileWriter::WriteBytes(const uint8_t *bytes, size_t size)
{
    if (Unlikely(size == 0)) {
        return true;
    }
    if (file_ == nullptr) {
        return false;
    }

    if (Unlikely(countChecksum_)) {
        checksum_ = adler32(checksum_, bytes, size);
    }

    auto oldSize = buffer_.size();
    buffer_.resize(oldSize + size);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (memcpy_s(buffer_.data() + oldSize, buffer_.size() - oldSize, bytes, size) != 0) {
        return false;
    }
    offset_ += size;
    return true;
}

bool FileWriter::WriteRawBytes(const uint8_t *data, size_t size)
{
    return WriteBytes(data, size);
}

bool FileWriter::AppendRange(const uint8_t *data, size_t size)
{
    if (size == 0) {
        return true;
    }
    if (file_ == nullptr) {
        return false;
    }
    auto oldSize = buffer_.size();
    buffer_.resize(oldSize + size);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (memcpy_s(buffer_.data() + oldSize, buffer_.size() - oldSize, data, size) != 0) {
        return false;
    }
    offset_ += size;
    return true;
}

bool FileWriter::FinalizeChecksum(size_t contentBeginOffset, size_t checksumStoreOffset)
{
    if (file_ == nullptr) {
        return false;
    }
    if (offset_ != buffer_.size() || contentBeginOffset > buffer_.size() ||
        checksumStoreOffset + sizeof(uint32_t) > buffer_.size()) {
        return WriteChecksum(checksumStoreOffset);
    }
    const size_t len = buffer_.size() - contentBeginOffset;
    auto contentSp = ark::Span<uint8_t>(buffer_).SubSpan(contentBeginOffset, len);
    checksum_ = ComputeAdler32Parallel(contentSp.data(), len);
    auto sumSp = ark::Span<uint8_t>(buffer_).SubSpan(checksumStoreOffset, sizeof(uint32_t));
    StoreChecksumLE(sumSp.data(), checksum_);
    return true;
}

bool FileWriter::FlushBuffer()
{
    if (file_ == nullptr) {
        return false;
    }
    if (buffer_.empty()) {
        return true;
    }

    size_t written = 0;
    while (written < buffer_.size()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto n = fwrite(buffer_.data() + written, sizeof(uint8_t), buffer_.size() - written, file_);
        if (n == 0) {
            return false;
        }
        written += n;
    }
    buffer_.clear();
    return true;
}

bool FileWriter::FinishWrite()
{
    if (file_ == nullptr) {
        return false;
    }

    bool ret = FlushBuffer();
    ret = fclose(file_) == 0 && ret;
    file_ = nullptr;
    return ret;
}

}  // namespace ark::panda_file
