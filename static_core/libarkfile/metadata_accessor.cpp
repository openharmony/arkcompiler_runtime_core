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

#include "libarkfile/metadata_accessor.h"

#include <algorithm>
#include <limits>

#include "zlib.h"
#include "libarkfile/file.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/metadata_helper.h"

namespace ark::panda_file {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda_file::helpers;

#if defined(METADATA_VERBOSE) && METADATA_VERBOSE

uint8_t curLogLevel_ = 0;

uint64_t CalcMetadataPkgSize(const std::string &pkgName, const MetadataByPackages &metadata)
{
    uint64_t size = 0;
    const auto it = metadata.find(pkgName);
    if (it != metadata.end()) {
        for (const auto &[moduleName, moduleMetadata] : it->second) {
            size += moduleMetadata.size();
        }
    }
    return size;
}

#endif

#define CUR_METADATA_LOGGER_COMPONENT METADATA_ACCESSOR

static bool InflateModule(z_stream &zs, const uint8_t *&inPtr, size_t &inRemaining, EncodedMetadata &out)
{
    zs.next_out = out.data();
    zs.avail_out = static_cast<uInt>(out.size());  // module size

    while (zs.avail_out != 0) {
        if (zs.avail_in == 0) {
            if (inRemaining == 0) {
                return false;
            }
            const auto chunk = static_cast<uInt>(std::min<size_t>(inRemaining, std::numeric_limits<uInt>::max()));
            zs.next_in = const_cast<Bytef *>(inPtr);
            zs.avail_in = chunk;
            inPtr += chunk;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            inRemaining -= chunk;
        }
        const int ret = inflate(&zs, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            break;
        }
        if (ret != Z_OK) {
            return false;
        }
    }
    return true;
}

MetadataAccessor::MetadataAccessor(const File &pandaFile)
{
    this->abcFilename_ = pandaFile.GetFullFileName();
    LoadMetadata(pandaFile);
}

void MetadataAccessor::LoadMetadata(const File &pandaFile)
{
    LOG_METADATA_ENABLE();

    const auto metadataInfoSpan = pandaFile.GetMetadata();
    if (metadataInfoSpan.empty()) {
        return;
    }

    const auto *metadata = reinterpret_cast<const uint32_t *>(metadataInfoSpan.data());

    const uint32_t numMetadataItems = metadata[0];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // Validate that metadata span is not definitely malformed, based on sizes
    ASSERT((1 + numMetadataItems * INDEX_ITEM_SIZE) * sizeof(uint32_t) <= metadataInfoSpan.size());

    const auto metadataSpan = metadataInfoSpan.SubSpan((1 + numMetadataItems * INDEX_ITEM_SIZE) * sizeof(uint32_t));

    LOG_METADATA("loading metadata (" << pandaFile.GetFullFileName() << ", " << numMetadataItems << " modules)");

    z_stream zs {};
    if (inflateInit(&zs) != Z_OK) {
        LOG_METADATA("metadata uncompression init failed");
        LOG_METADATA_DISABLE();
        return;  // Handle errors properly: leave metadata_ empty
    }

    const uint8_t *inPtr = metadataSpan.data();
    auto inRemaining = metadataSpan.size();
#if !defined(NDEBUG)
    uint64_t uncompressedMetadataSize = 0;
#endif

    LOG_METADATA_NESTING_INC();
    for (uint32_t i = 0; i < numMetadataItems; i++) {
        const auto baseOff = 1 + i * 3;
        const auto pkgNameOff =
            pandaFile.GetBase() + metadata[baseOff];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto moduleNameOff =
            pandaFile.GetBase() + metadata[baseOff + 1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto size = metadata[baseOff + 2];          // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto pkgName = pandaFile.GetStringData(pandaFile.GetIdFromPointer(pkgNameOff)).ToString();
        const auto moduleName = pandaFile.GetStringData(pandaFile.GetIdFromPointer(moduleNameOff)).ToString();

#if !defined(NDEBUG)
        uncompressedMetadataSize += size;
#endif

        // Bound allocation against malformed size fields: total uncompressed metadata cannot exceed the file.
        ASSERT(uncompressedMetadataSize < pandaFile.GetHeader()->fileSize);

        EncodedMetadata buf(size);
        if (!InflateModule(zs, inPtr, inRemaining, buf)) {
            LOG_METADATA("metadata uncompression failed (" << metadataSpan.size() << " bytes)");
            inflateEnd(&zs);
            metadata_.clear();
            LOG_METADATA_NESTING_DEC();
            LOG_METADATA_DISABLE();
            return;  // Handle errors properly: leave metadata_ empty
        }

        metadata_[pkgName][moduleName] = std::move(buf);

        LOG_METADATA(pkgName << ":" << moduleName << " (" << size << " bytes)");
    }
    LOG_METADATA_NESTING_DEC();

    inflateEnd(&zs);
    ASSERT(uncompressedMetadataSize != 0);

    LOG_METADATA("metadata mem cache recorded (" << metadata_.size() << " modules)");
    LOG_METADATA_DISABLE();
}

EncodedMetadata MetadataAccessor::CompressMetadata(const MetadataByPackages &metadata)
{
    LOG_METADATA_ENABLE();

    LOG_METADATA("compressing metadata (" << metadata.size() << " modules)");
    EncodedMetadata entireMetadata;

    LOG_METADATA_NESTING_INC();
    // Same nested traversal order as the index writer in file_item_container, so module offsets match.
    for (const auto &[pkgName, modules] : metadata) {
        for (const auto &[moduleName, moduleMetadata] : modules) {
            LOG_METADATA("adding metadata of module " << pkgName << ":" << moduleName << " (" << moduleMetadata.size()
                                                      << " bytes)");
            entireMetadata.insert(entireMetadata.end(), moduleMetadata.begin(), moduleMetadata.end());
        }
    }
    LOG_METADATA_NESTING_DEC();

    unsigned long entireMetadataSize = entireMetadata.size();  // NOLINT(google-runtime-int)
    // Free unused trailing memory after compression
    auto compressedMetadata = std::make_unique<uint8_t[]>(entireMetadataSize);  // NOLINT(modernize-avoid-c-arrays)
    auto res = compress(compressedMetadata.get(), &entireMetadataSize, entireMetadata.data(), entireMetadataSize);
    if (res != 0) {
        compressedMetadata.reset();
        LOG_METADATA("compressing metadata error (code " << res << ")");
        return {};  // Handle errors properly
    }

    LOG_METADATA("compressed metadata: " << entireMetadata.size() << " bytes -> " << entireMetadataSize << " bytes");

    LOG_METADATA_DISABLE();

    return {compressedMetadata.get(), compressedMetadata.get() + entireMetadataSize};
}

MetadataByModules MetadataAccessor::ExtractMetadataForPackage(const std::string &pkgName)
{
    LOG_METADATA_ENABLE();

    const auto it = metadata_.find(pkgName);
    if (it == metadata_.end()) {
        LOG_METADATA("accessing metadata (package " << pkgName << " from " << abcFilename << "): empty");
        return {};
    }

    LOG_METADATA("accessing metadata (package " << pkgName << " from " << abcFilename << "): " << it->second.size()
                                                << " modules, " << CalcMetadataPkgSize(pkgName, metadata_) << " bytes");
    LOG_METADATA_DISABLE();

    return std::move(it->second);
}

MetadataByPackages MetadataAccessor::ExtractMetadata()
{
    LOG_METADATA_ENABLE();

    if (metadata_.empty()) {
        LOG_METADATA("accessing metadata (from " << abcFilename << "): empty");
        return {};
    }

    LOG_METADATA("accessing metadata (from " << abcFilename << "): " << metadata_.size() << " packages");

    LOG_METADATA_DISABLE();

    return std::move(metadata_);
}

}  // namespace ark::panda_file