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

#include "libarkfile/metadata_accessor.h"

#include "zlib.h"
#include "libarkfile/file.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/metadata_helper.h"

namespace ark::panda_file {

// CC-OFFNXT(WordsTool.95) sensitive word conflict
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
    abcFilename_ = pandaFile.GetFullFileName();
    LoadMetadata(pandaFile);
}

// Function extracted from 'MetadataAccessor::LoadMetadata' to reduce its size
static bool ProcessModule(const File &pandaFile, const uint32_t *metadata, uint32_t index, EncodedMetadata &&moduleData,
                          MetadataByPackages &packageMetadata)
{
    const std::size_t baseOff = 1U + static_cast<std::size_t>(index) * 3U;
    auto const *const fileData = pandaFile.GetBase();
    auto const fileSize = pandaFile.GetHeader()->fileSize;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (metadata[baseOff] >= fileSize || metadata[baseOff + 1U] >= fileSize) {
        return false;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto const *const pkgNameOff = fileData + metadata[baseOff];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto const *const moduleNameOff = fileData + metadata[baseOff + 1];

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto pkgName = pandaFile.GetStringData(pandaFile.GetIdFromPointer(pkgNameOff)).ToString();
    const auto moduleName = pandaFile.GetStringData(pandaFile.GetIdFromPointer(moduleNameOff)).ToString();

    packageMetadata[pkgName][moduleName] = std::move(moduleData);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    [[maybe_unused]] const auto size = metadata[baseOff + 2];
    LOG_METADATA(pkgName << ":" << moduleName << " (" << size << " bytes)");

    return true;
}

// Function extracted from 'MetadataAccessor::LoadMetadata' to reduce its size
static bool ProcessItems(const File &pandaFile, const uint32_t *metadata, Span<std::uint8_t const> const metadataSpan,
                         z_stream &zs, MetadataByPackages &packageMetadata)
{
    std::size_t totalSize = 0U;
    const uint32_t numMetadataItems = metadata[0];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const uint8_t *inPtr = metadataSpan.data();
    auto inRemaining = metadataSpan.size();
    LOG_METADATA_NESTING_INC();

#if defined(METADATA_VERBOSE) && METADATA_VERBOSE
    auto const finalize = [this, &zs, &prevLoggerLevel]([[maybe_unused]] const std::string_view message) -> void {
#else
    auto const finalize = [&packageMetadata, &zs]([[maybe_unused]] const std::string_view message) -> bool {
#endif
        LOG_METADATA(message);
        LOG_METADATA_NESTING_DEC();
        inflateEnd(&zs);
        LOG_METADATA_DISABLE();
        packageMetadata.clear();  // Handle errors properly: leave metadata_ empty
        return false;
    };

    for (uint32_t i = 0U; i < numMetadataItems; ++i) {
        const auto size = metadata[1U + i * 3U + 2U];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (size > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
            return finalize("Metadata item size " + std::to_string(size) + " exceeds the limit.");
        }

        totalSize += size;
        if (totalSize > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
            return finalize("Metadata total size " + std::to_string(size) + " exceeds the limit.");
        }

        EncodedMetadata buf(size);
        if (!InflateModule(zs, inPtr, inRemaining, buf)) {
            return finalize("Metadata uncompression failed (" + std::to_string(metadataSpan.size()) + " bytes).");
        }

        if (!ProcessModule(pandaFile, metadata, i, std::move(buf), packageMetadata)) {
            return finalize("Cannot access required file data: probably they were corrupted.");
        }
    }

    return true;
}

void MetadataAccessor::LoadMetadata(const File &pandaFile)
{
    const auto metadataInfoSpan = pandaFile.GetMetadata();
    if (metadataInfoSpan.size() < 4U) {
        return;
    }

    LOG_METADATA_ENABLE();
    const auto *metadata = reinterpret_cast<const uint32_t *>(metadataInfoSpan.data());
    const uint32_t numMetadataItems = metadata[0U];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const std::size_t bytesToRead = (1U + numMetadataItems * INDEX_ITEM_SIZE) * sizeof(uint32_t);

    if (bytesToRead > metadataInfoSpan.size()) {
        metadata_.clear();  // Handle errors properly: leave metadata_ empty
        LOG_METADATA("Metadata size is less than required: probably data were corrupted.");
        LOG_METADATA_DISABLE();
        return;
    }

    const auto metadataSpan = metadataInfoSpan.SubSpan(bytesToRead);
    LOG_METADATA("Loading metadata (" << pandaFile.GetFullFileName() << ", " << numMetadataItems << " modules).");

    z_stream zs {};
    if (inflateInit(&zs) != Z_OK) {
        metadata_.clear();  // Handle errors properly: leave metadata_ empty
        LOG_METADATA("Metadata uncompression init failed.");
        LOG_METADATA_DISABLE();
        return;
    }

    if (ProcessItems(pandaFile, metadata, metadataSpan, zs, metadata_)) {
        LOG_METADATA_NESTING_DEC();
        inflateEnd(&zs);
        LOG_METADATA("metadata mem cache recorded (" << metadata_.size() << " modules)");
        LOG_METADATA_DISABLE();
    }
}

std::optional<EncodedMetadata> MetadataAccessor::CompressMetadata(const MetadataByPackages &metadata)
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

    if (entireMetadata.size() > static_cast<size_t>(std::numeric_limits<uLong>::max())) {
        LOG_METADATA("Metadata is too large to compress");
        LOG_METADATA_DISABLE();
        return std::nullopt;
    }

    const auto sourceSize = static_cast<uLong>(entireMetadata.size());
    auto compressedCapacity = compressBound(sourceSize);
    EncodedMetadata compressedMetadata(compressedCapacity);
    auto compressedSize = compressedCapacity;
    auto res = compress(compressedMetadata.data(), &compressedSize, entireMetadata.data(), sourceSize);
    if (res != 0) {
        LOG_METADATA("compressing metadata error (code " << res << ")");
        LOG_METADATA_DISABLE();
        return std::nullopt;  // Handle errors properly
    }

    compressedMetadata.resize(compressedSize);

    LOG_METADATA("compressed metadata: " << entireMetadata.size() << " bytes -> " << compressedSize << " bytes");

    LOG_METADATA_DISABLE();

    return compressedMetadata;
}

MetadataByModules MetadataAccessor::ExtractMetadataForPackage(const std::string &pkgName)
{
    LOG_METADATA_ENABLE();

    const auto it = metadata_.find(pkgName);
    if (it == metadata_.end()) {
        LOG_METADATA("accessing metadata (package " << pkgName << " from " << abcFilename_ << "): empty");
        return {};
    }

    LOG_METADATA("accessing metadata (package " << pkgName << " from " << abcFilename_ << "): " << it->second.size()
                                                << " modules, " << CalcMetadataPkgSize(pkgName, metadata_) << " bytes");
    LOG_METADATA_DISABLE();

    return std::move(it->second);
}

MetadataByPackages MetadataAccessor::ExtractMetadata()
{
    LOG_METADATA_ENABLE();

    if (metadata_.empty()) {
        LOG_METADATA("accessing metadata (from " << abcFilename_ << "): empty");
        return {};
    }

    LOG_METADATA("accessing metadata (from " << abcFilename_ << "): " << metadata_.size() << " packages");

    LOG_METADATA_DISABLE();

    return std::move(metadata_);
}

}  // namespace ark::panda_file
