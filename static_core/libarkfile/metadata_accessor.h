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

#ifndef LIBPANDAFILE_METADATA_ACCESSOR_H_
#define LIBPANDAFILE_METADATA_ACCESSOR_H_

#include <string>
#include <string_view>
#include <unordered_map>

#include "libarkbase/utils/span.h"
#include "libarkbase/macros.h"

namespace ark::panda_file {
class File;

struct MetadataModuleId {
    MetadataModuleId(std::string pkgName, std::string moduleName)
        : pkgName_(std::move(pkgName)), moduleName_(std::move(moduleName))
    {
    }

    ~MetadataModuleId() = default;

    DEFAULT_MOVE_SEMANTIC(MetadataModuleId);
    DEFAULT_COPY_SEMANTIC(MetadataModuleId);

    std::string GetPkgName() const
    {
        return pkgName_;
    }

    std::string GetModuleName() const
    {
        return moduleName_;
    }

    bool operator==(const MetadataModuleId &other) const
    {
        return pkgName_ == other.pkgName_ && moduleName_ == other.moduleName_;
    }

private:
    std::string pkgName_;
    std::string moduleName_;
};
}  // namespace ark::panda_file

template <>
struct std::hash<ark::panda_file::MetadataModuleId> {
    size_t operator()(const ark::panda_file::MetadataModuleId &p) const noexcept
    {
        return hash<std::string>()(p.GetPkgName()) ^ (hash<std::string_view>()(p.GetModuleName()) << 1U);
    }
};

namespace ark::panda_file {

using EncodedMetadata = std::vector<uint8_t>;
using MetadataIndex = std::unordered_map<MetadataModuleId, std::pair<uint32_t, uint32_t>>;
using MetadataByModules = std::unordered_map<MetadataModuleId, EncodedMetadata>;

/*
 * The simplified metadata accessor implementation below.
 * Further, metadata will be loaded in several steps:
 *  1) Loading metadata index (module_name -> (metadata_offset, metadata_size)) into the memory once per abc file;
 *  2) During the first request of metadata when resolving import,
 *     uncompress metadata of all modules and cache it onto the disk / memory;
 *  3) During further requests of metadata, take it from the disk / memory
 *     according to known offset for the specific module.
 */
class MetadataAccessor {
public:
    MetadataAccessor() = default;
    ~MetadataAccessor() = default;

    PANDA_PUBLIC_API void SetFile(const File &pandaFile);
    PANDA_PUBLIC_API MetadataByModules GetMetadata();
    EncodedMetadata GetMetadataFor(const MetadataModuleId &moduleId);

    static MetadataModuleId BuildModuleId(const std::string_view pkgName, const std::string_view moduleName)
    {
        auto pkgNameStr = std::string(pkgName);
        pkgNameStr.erase(pkgNameStr.find_last_not_of('.') + 1, std::string::npos);
        auto moduleNameStr = std::string(moduleName);
        moduleNameStr.erase(moduleNameStr.find_last_not_of('.') + 1, std::string::npos);
        return {pkgNameStr, moduleNameStr};
    }

    [[nodiscard]] static EncodedMetadata CompressMetadata(const MetadataByModules &metadata);

    NO_COPY_SEMANTIC(MetadataAccessor);
    NO_MOVE_SEMANTIC(MetadataAccessor);

    static constexpr auto INDEX_ITEM_SIZE = 3;

private:
    const File *pandaFile_ = nullptr;
    Span<const uint32_t> metadataSpan_;
    MetadataIndex metadataIndex_;  // modules to offsets and sizes
    uint64_t uncompressedMetadataSize_ = 0;

    void BuildIndex();
    [[nodiscard]] EncodedMetadata UncompressMetadata(const EncodedMetadata &compressedMetadata) const;
    EncodedMetadata GetMetadataFor(const MetadataModuleId &moduleId, const EncodedMetadata &uncompressedMetadata);
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_METADATA_ACCESSOR_H_