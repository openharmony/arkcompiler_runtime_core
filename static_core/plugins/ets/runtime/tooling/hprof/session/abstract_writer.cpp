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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/tooling/hprof/session/abstract_writer.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/tooling/hprof/session/output_stream.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <iterator>
#include <type_traits>

namespace ark::tooling::hprof {
namespace {

template <class T>
void AppendLittleEndian(std::vector<uint8_t> &buffer, T value)
{
    static_assert(std::is_unsigned_v<T>);
    constexpr auto BITS_PER_BYTE = static_cast<T>(CHAR_BIT);
    for (size_t index = 0; index < sizeof(value); ++index) {
        buffer.push_back(static_cast<uint8_t>(value));
        value >>= BITS_PER_BYTE;
    }
}

}  // namespace

AbstractWriter::AbstractWriter(OutputStream *stream, size_t bufferSize) : stream_(stream), bufferSize_(bufferSize)
{
    buffer_.reserve(bufferSize_);
}

AbstractWriter::~AbstractWriter()
{
    EndRecord();
}

void AbstractWriter::BeginRecord(uint8_t tag)
{
    // Auto-flush any pending record group with items
    if (itemCount_ > 0) {
        FlushRecord();
    }

    tag_ = tag;
    recordTime_ = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count());
    itemCount_ = 0;
    buffer_.clear();
}

void AbstractWriter::FinishItem()
{
    itemCount_++;

    if (buffer_.size() >= bufferSize_ && stream_ != nullptr) {
        FlushRecord();
        // After flush, start a new record group for the same tag
        recordTime_ = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count());
        itemCount_ = 0;
        buffer_.clear();
    }
}

void AbstractWriter::EndRecord()
{
    if (itemCount_ > 0) {
        FlushRecord();
    }
    // itemCount_ == 0 -> no items were written, no record to flush
}

void AbstractWriter::FlushRecord()
{
    if (stream_ == nullptr) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Record flush skipped: output stream is null";
        buffer_.clear();
        itemCount_ = 0;
        return;
    }

    // Construct 17-byte header: tag(1) + time(8) + length(4) + count(4)
    std::array<uint8_t, RECORD_HEADER_SIZE> header {};
    header[TAG_OFFSET] = tag_;

    const auto *timestamp = reinterpret_cast<const uint8_t *>(&recordTime_);
    std::copy_n(timestamp, sizeof(recordTime_), std::next(header.begin(), TIMESTAMP_OFFSET));
    auto length = static_cast<uint32_t>(buffer_.size());
    const auto *recordLength = reinterpret_cast<const uint8_t *>(&length);
    std::copy_n(recordLength, sizeof(length), std::next(header.begin(), SIZE_OFFSET));
    const auto *itemCount = reinterpret_cast<const uint8_t *>(&itemCount_);
    std::copy_n(itemCount, sizeof(itemCount_), std::next(header.begin(), COUNT_OFFSET));

    bool ok = stream_->Write(buffer_.data(), buffer_.size(), header.data(), header.size());
    if (!ok) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Record flush failed";
    }

    buffer_.clear();
    itemCount_ = 0;
}

void AbstractWriter::WriteU8(uint8_t value)
{
    buffer_.push_back(value);
}

void AbstractWriter::WriteU16(uint16_t value)
{
    AppendLittleEndian(buffer_, value);
}

void AbstractWriter::WriteU32(uint32_t value)
{
    AppendLittleEndian(buffer_, value);
}

void AbstractWriter::WriteU64(uint64_t value)
{
    AppendLittleEndian(buffer_, value);
}

void AbstractWriter::WriteBytes(const uint8_t *data, size_t size)
{
    if (size == 0 || data == nullptr) {
        return;
    }
    std::copy_n(data, size, std::back_inserter(buffer_));
}

}  // namespace ark::tooling::hprof
