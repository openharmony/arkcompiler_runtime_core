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

#include "test_common.h"

namespace ark::tooling::hprof::test {

RecordInfo ParseRecord(const std::vector<uint8_t> &data, size_t offset)
{
    if (offset + RECORD_HEADER_SIZE > data.size()) {
        return {};
    }
    RecordInfo info;
    info.tag = data[offset + TAG_OFFSET];
    info.bodySize = ReadU32LE(data, offset + SIZE_OFFSET);
    info.count = ReadU32LE(data, offset + COUNT_OFFSET);
    info.bodyStart = offset + RECORD_HEADER_SIZE;
    info.nextOffset = info.bodyStart + info.bodySize;
    if (info.nextOffset > data.size()) {
        return {};
    }
    return info;
}

RecordInfo FindRecordAfterHeader(const std::vector<uint8_t> &data, uint8_t targetTag)
{
    size_t offset = HYBRID_DUMP_HEADER_SIZE;
    while (offset < data.size()) {
        RecordInfo rec = ParseRecord(data, offset);
        if (rec.tag == 0) {
            break;
        }
        if (rec.tag == targetTag) {
            return rec;
        }
        offset = rec.nextOffset;
    }
    return {};
}

}  // namespace ark::tooling::hprof::test
