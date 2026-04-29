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

#ifndef STATIC_ABC_VERIFIER_ABC_FILE_H
#define STATIC_ABC_VERIFIER_ABC_FILE_H

#include <array>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace ark::static_verifier {

inline constexpr size_t MAGIC_SIZE = 8;
inline constexpr size_t VERSION_SIZE = 4;
inline constexpr std::array<uint8_t, MAGIC_SIZE> ABC_MAGIC = {'P', 'A', 'N', 'D', 'A', '\0', '\0', '\0'};

inline constexpr std::array<uint8_t, VERSION_SIZE> STATIC_MIN_VERSION = {0, 0, 0, 6};
inline constexpr std::array<uint8_t, VERSION_SIZE> STATIC_MAX_VERSION = {0, 0, 0, 7};

inline constexpr uint32_t FILE_CONTENT_OFFSET = 12U;  // magic(8) + checksum(4)

#pragma pack(push, 1)
struct AbcHeader {
    std::array<uint8_t, MAGIC_SIZE> magic;
    uint32_t checksum;
    std::array<uint8_t, VERSION_SIZE> version;
    uint32_t fileSize;
    uint32_t foreignOff;
    uint32_t foreignSize;
    uint32_t quickenedFlag;
    uint32_t numClasses;
    uint32_t classIdxOff;
    uint32_t numExportTable;
    uint32_t exportTableOff;
    uint32_t numLnps;
    uint32_t lnpIdxOff;
    uint32_t numLiteralarrays;
    uint32_t literalarrayIdxOff;
    uint32_t numIndexes;
    uint32_t indexSectionOff;
};

struct RegionHeader {
    uint32_t start;
    uint32_t end;
    uint32_t classIdxSize;
    uint32_t classIdxOff;
    uint32_t methodIdxSize;
    uint32_t methodIdxOff;
    uint32_t fieldIdxSize;
    uint32_t fieldIdxOff;
    uint32_t protoIdxSize;
    uint32_t protoIdxOff;
};
#pragma pack(pop)

class AbcFile {
public:
    static std::unique_ptr<AbcFile> Open(const std::string &filename);
    static std::unique_ptr<AbcFile> OpenFromMemory(const uint8_t *data, size_t size);

    const AbcHeader *GetHeader() const;
    const uint8_t *GetBase() const { return data_.data(); }
    size_t GetFileSize() const { return data_.size(); }

    const RegionHeader *GetRegionHeaders() const;
    uint32_t GetNumRegionHeaders() const;

    const uint32_t *GetClassIndex() const;
    uint32_t GetNumClasses() const;

    const uint32_t *GetLiteralArrayIndex() const;
    uint32_t GetNumLiteralArrays() const;

    bool IsValid() const { return !data_.empty() && data_.size() >= sizeof(AbcHeader); }

    bool IsOffsetValid(uint32_t offset) const { return offset < data_.size(); }
    bool IsRangeValid(uint32_t offset, uint32_t size) const;

private:
    AbcFile() = default;
    struct FactoryHelper;
    static std::unique_ptr<AbcFile> CreateEmpty();
    std::vector<uint8_t> data_;
};

}  // namespace ark::static_verifier

#endif  // STATIC_ABC_VERIFIER_ABC_FILE_H
