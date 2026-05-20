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
#include "zlib.h"

#ifndef PANDA_TARGET_WINDOWS
#include <sys/types.h>
#endif

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

}  // namespace

namespace ark::panda_file {

MemoryWriter::MemoryWriter() : checksum_(adler32(0, nullptr, 0)) {}

bool MemoryWriter::WriteByte(uint8_t byte)
{
    if (countChecksum_) {
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
    if (countChecksum_) {
        checksum_ = adler32(checksum_, bytes, size);
    }
    data_.insert(data_.end(), bytes, bytes + size);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return true;
}

MemoryBufferWriter::MemoryBufferWriter(uint8_t *buffer, size_t size)
    : sp_(buffer, size), checksum_(adler32(0, nullptr, 0))
{
}

bool MemoryBufferWriter::WriteByte(uint8_t byte)
{
    if (countChecksum_) {
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
    if (countChecksum_) {
        checksum_ = adler32(checksum_, bytes, size);
    }
    auto subSp = sp_.SubSpan(offset_, size);
    if (memcpy_s(subSp.data(), subSp.size(), bytes, size) != 0) {
        return false;
    }
    offset_ += size;
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
    if (countChecksum_) {
        checksum_ = adler32(checksum_, &data, 1U);
    }
    if (seekable_ && buffer_.size() == BUFFER_CAPACITY && !FlushBuffer()) {
        return false;
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
    if (UNLIKELY(size == 0)) {
        return true;
    }
    if (file_ == nullptr) {
        return false;
    }

    if (countChecksum_) {
        checksum_ = adler32(checksum_, bytes, size);
    }

    size_t written = 0;
    while (written < size) {
        if (seekable_ && buffer_.size() == BUFFER_CAPACITY && !FlushBuffer()) {
            return false;
        }

        auto chunkSize = seekable_ ? std::min(size - written, BUFFER_CAPACITY - buffer_.size()) : size - written;
        auto oldSize = buffer_.size();
        buffer_.resize(oldSize + chunkSize);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (memcpy_s(buffer_.data() + oldSize, buffer_.size() - oldSize, bytes + written, chunkSize) != 0) {
            return false;
        }
        written += chunkSize;
    }
    offset_ += size;
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
