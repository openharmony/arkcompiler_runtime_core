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

#include "test_helper.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ark::static_verifier::test {

std::vector<uint8_t> BuildMinimalValidAbc()
{
    constexpr uint32_t kTotalSize = sizeof(AbcHeader);

    AbcHeader header {};
    header.magic = ABC_MAGIC;
    header.version = STATIC_MIN_VERSION;
    header.fileSize = kTotalSize;
    header.foreignOff = 0;
    header.foreignSize = 0;
    header.quickenedFlag = 0;
    header.numClasses = 0;
    header.classIdxOff = 0;
    header.numExportTable = 0;
    header.exportTableOff = 0;
    header.numLnps = 0;
    header.lnpIdxOff = 0;
    header.numLiteralarrays = 0;
    header.literalarrayIdxOff = 0;
    header.numIndexes = 0;
    header.indexSectionOff = 0;

    std::vector<uint8_t> data(kTotalSize);
    std::copy_n(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data.data());

    const uint8_t *contentStart = data.data() + FILE_CONTENT_OFFSET;
    size_t contentLen = kTotalSize - FILE_CONTENT_OFFSET;
    uint32_t checksum = ComputeAdler32(contentStart, contentLen);
    std::copy_n(reinterpret_cast<const uint8_t *>(&checksum), sizeof(checksum),
                data.data() + offsetof(AbcHeader, checksum));

    return data;
}

std::vector<uint8_t> BuildAbcWithClasses(uint32_t numClasses)
{
    uint32_t classIdxOff = sizeof(AbcHeader);
    uint32_t totalSize = classIdxOff + numClasses * sizeof(uint32_t);

    std::vector<uint8_t> data(totalSize, 0);

    AbcHeader header {};
    header.magic = ABC_MAGIC;
    header.version = STATIC_MIN_VERSION;
    header.fileSize = totalSize;
    header.numClasses = numClasses;
    header.classIdxOff = classIdxOff;

    std::copy_n(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data.data());

    auto *classIdx = reinterpret_cast<uint32_t *>(data.data() + classIdxOff);
    for (uint32_t i = 0; i < numClasses; ++i) {
        classIdx[i] = sizeof(AbcHeader);
    }

    const uint8_t *contentStart = data.data() + FILE_CONTENT_OFFSET;
    size_t contentLen = totalSize - FILE_CONTENT_OFFSET;
    uint32_t checksum = ComputeAdler32(contentStart, contentLen);
    std::copy_n(reinterpret_cast<const uint8_t *>(&checksum), sizeof(checksum),
                data.data() + offsetof(AbcHeader, checksum));

    return data;
}

}  // namespace ark::static_verifier::test
