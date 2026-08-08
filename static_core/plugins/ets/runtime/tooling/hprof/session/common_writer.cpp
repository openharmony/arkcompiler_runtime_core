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

#include "plugins/ets/runtime/tooling/hprof/session/common_writer.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/tooling/hprof/session/output_stream.h"
#include "plugins/ets/runtime/tooling/hprof/session/string_id_pool.h"

#include <algorithm>
#include <array>
#include <ctime>
#include <iterator>

namespace ark::tooling::hprof {

CommonWriter::CommonWriter(OutputStream *stream, size_t bufferSize) : AbstractWriter(stream, bufferSize) {}

CommonWriter::~CommonWriter() = default;

template <size_t SIZE>
bool CopyField(std::array<uint8_t, SIZE> &buffer, size_t &position, const void *field, size_t fieldSize)
{
    if (fieldSize > buffer.size() - position) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] File header write failed: buffer overflow at offset=" << position;
        return false;
    }
    const auto *bytes = static_cast<const uint8_t *>(field);
    std::copy_n(bytes, fieldSize, std::next(buffer.begin(), position));
    position += fieldSize;
    return true;
}

void CommonWriter::WriteFileHeader(Language language, uint64_t totalObj, uint64_t totalClass)
{
    std::array<uint8_t, HYBRID_DUMP_HEADER_SIZE> header {};  // 33 bytes
    size_t pos = 0;

    auto timestampMs = static_cast<uint64_t>(std::time(nullptr)) * MS_PER_SEC;
    auto langVal = static_cast<uint8_t>(language);  // 1 byte
    uint32_t zeroFlags = 0;
    auto recordCount = static_cast<uint32_t>(totalObj + totalClass);

    // New layout: version(8) | identifierSize(4) | timestamp(8) | language(1) |
    //             headerSize(4) | recordCount(4) | featureFlags(4)
    // identifierSize reflects the static-side nodeId width (4 bytes); XRef
    // dynAddr and heap-summary counters remain 8 bytes (see dump_format.h).
    if (!CopyField(header, pos, HYBRID_DUMP_VERSION.data(), HYBRID_DUMP_VERSION_SIZE) ||
        !CopyField(header, pos, &STATIC_OBJECT_ID_SIZE, sizeof(uint32_t)) ||
        !CopyField(header, pos, &timestampMs, sizeof(uint64_t)) || !CopyField(header, pos, &langVal, sizeof(uint8_t)) ||
        !CopyField(header, pos, &HYBRID_DUMP_HEADER_SIZE, sizeof(uint32_t)) ||
        !CopyField(header, pos, &recordCount, sizeof(uint32_t)) ||
        !CopyField(header, pos, &zeroFlags, sizeof(uint32_t))) {
        return;
    }

    bool ok = GetStream()->Write(header.data(), header.size());
    if (!ok) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] File header write failed";
    }
}

void CommonWriter::WriteStringItem(uint32_t id, uint32_t length, const uint8_t *data, size_t dataSize)
{
    WriteU32(id);
    WriteU32(length);
    if (dataSize > 0 && data != nullptr) {
        WriteBytes(data, dataSize);
    }
    FinishItem();  // Each string is one item; flush threshold check inside
}

void CommonWriter::WriteStringPool(StringIdPool *pool)
{
    BeginRecord(TAG_STRING_IN_UTF8);
    pool->ForEachString([this](StringId id, const std::string &str) {
        auto len = static_cast<uint32_t>(str.size());
        auto *data = reinterpret_cast<const uint8_t *>(str.data());
        WriteStringItem(id, len, data, str.size());
    });
    EndRecord();  // Flush remaining strings
}

void CommonWriter::WriteHeapSummary(uint64_t totalObj, uint64_t totalClass, size_t dynamicObjCount,
                                    size_t staticObjCount)
{
    BeginRecord(TAG_HEAP_SUMMARY);
    WriteU64(0);                                       // totalLiveBytes - reserved
    WriteU64(totalObj);                                // totalLiveInstances
    WriteU64(0);                                       // totalAllocated - reserved
    WriteU64(0);                                       // totalInstancesAllocated - reserved
    WriteU64(static_cast<uint64_t>(staticObjCount));   // staticObjCount
    WriteU64(static_cast<uint64_t>(dynamicObjCount));  // dynamicObjCount
    WriteU64(totalClass);                              // classCount
    FinishItem();                                      // Mark this single-item record as count=1
    EndRecord();
}

void CommonWriter::WriteXRefEdge(uint32_t fromAddr, uint32_t toAddr, uint8_t direction)
{
    WriteU32(fromAddr);  // dynNodeId - dynamic-side nodeId (4 bytes)
    WriteU32(toAddr);    // staNodeId - static-side nodeId (4 bytes)
    WriteU8(direction);
    FinishItem();  // Each XRef edge is one item; caller wraps with BeginRecord/EndRecord
}

}  // namespace ark::tooling::hprof
