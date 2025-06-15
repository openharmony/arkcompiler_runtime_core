/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "file_writer.h"
#include "zlib.h"

namespace ark::panda_file {

MemoryWriter::MemoryWriter() : checksum_(adler32(0, nullptr, 0)) {}

bool MemoryWriter::WriteBytes(const std::vector<uint8_t> &bytes)
{
    if (bytes.empty()) {
        return true;
    }
    if (countChecksum_) {
        checksum_ = adler32(checksum_, bytes.data(), bytes.size());
    }
    data_.insert(data_.end(), bytes.cbegin(), bytes.cend());
    return true;
}

MemoryBufferWriter::MemoryBufferWriter(uint8_t *buffer, size_t size)
    : sp_(buffer, size), checksum_(adler32(0, nullptr, 0))
{
}

bool MemoryBufferWriter::WriteBytes(const std::vector<uint8_t> &bytes)
{
    if (bytes.empty()) {
        return true;
    }
    if (countChecksum_) {
        checksum_ = adler32(checksum_, bytes.data(), bytes.size());
    }
    auto subSp = sp_.SubSpan(offset_, bytes.size());
    if (memcpy_s(subSp.data(), subSp.size(), bytes.data(), bytes.size()) != 0) {
        return false;
    }
    offset_ += bytes.size();
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
}

FileWriter::~FileWriter()
{
    if (file_ != nullptr) {
        fclose(file_);
    }
    file_ = nullptr;
}

bool FileWriter::WriteByte(uint8_t data)
{
    return WriteBytes({data});
}

bool FileWriter::WriteBytes(const std::vector<uint8_t> &bytes)
{
    if (file_ == nullptr) {
        return false;
    }

    if (bytes.empty()) {
        return true;
    }

    if (countChecksum_) {
        checksum_ = adler32(checksum_, bytes.data(), bytes.size());
    }

    if (fwrite(bytes.data(), sizeof(decltype(bytes.back())), bytes.size(), file_) != bytes.size()) {
        return false;
    }

    offset_ += bytes.size();
    return true;
}

}  // namespace ark::panda_file
