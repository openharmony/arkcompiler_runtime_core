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

#ifndef LIBPANDAFILE_METADATA_ACCESSOR_H_
#define LIBPANDAFILE_METADATA_ACCESSOR_H_

#include <map>
#include <optional>

#include "libarkbase/macros.h"
#include "libarkbase/utils/span.h"

namespace ark::panda_file {
class File;
}  // namespace ark::panda_file

namespace ark::panda_file {

using EncodedMetadata = std::vector<uint8_t>;
using MetadataByModules = std::map<std::string, EncodedMetadata>;
using MetadataByPackages = std::map<std::string, MetadataByModules>;

class MetadataAccessor {
public:
    PANDA_PUBLIC_API explicit MetadataAccessor(const File &pandaFile);
    ~MetadataAccessor() = default;

    PANDA_PUBLIC_API MetadataByPackages ExtractMetadata();
    MetadataByModules ExtractMetadataForPackage(const std::string &pkgName);

    [[nodiscard]] static std::optional<EncodedMetadata> CompressMetadata(const MetadataByPackages &metadata);

    NO_COPY_SEMANTIC(MetadataAccessor);
    NO_MOVE_SEMANTIC(MetadataAccessor);

    static constexpr std::size_t const INDEX_ITEM_SIZE = 3U;

private:
    std::string abcFilename_;

    /*
     * NB: This is a movable "back" cache for metadata.
     * During request of the specific part of metadata, it's being moved onto use-site,
     * which means next requests of the same part will be failed.
     * This mechanics allows to avoid extra copies.
     */
    MetadataByPackages metadata_;

    void LoadMetadata(const File &pandaFile);
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_METADATA_ACCESSOR_H_
