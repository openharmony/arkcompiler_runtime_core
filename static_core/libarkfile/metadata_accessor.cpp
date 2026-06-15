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

#include "zlib.h"
#include "libarkfile/file.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/metadata_helper.h"

namespace ark::panda_file {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda_file::helpers;

#if defined(METADATA_VERBOSE) && METADATA_VERBOSE
uint8_t curLogLevel_ = 0;
#endif

#define CUR_METADATA_LOGGER_COMPONENT METADATA_ACCESSOR

void MetadataAccessor::SetFile(const File &pandaFile)
{
    ASSERT(this->pandaFile_ == nullptr);

    this->pandaFile_ = &pandaFile;

    BuildIndex();
}

void MetadataAccessor::BuildIndex()
{
    LOG_METADATA_ENABLE();

    const auto metadataInfoSpan = pandaFile_->GetMetadata();
    if (metadataInfoSpan.empty()) {
        return;
    }

    const auto *metadata = reinterpret_cast<const uint32_t *>(metadataInfoSpan.data());

    const uint32_t numMetadataItems = metadata[0];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // Validate that metadata span is not definitely malformed, based on sizes
    ASSERT((1 + numMetadataItems * INDEX_ITEM_SIZE) * sizeof(uint32_t) <= metadataInfoSpan.size());

    LOG_METADATA("building metadata index based on " << pandaFile_->GetFullFileName());
    uint64_t uncompressedMetadataSize = 0;
    LOG_METADATA_NESTING_INC();
    for (uint32_t i = 0; i < numMetadataItems; i++) {
        const auto baseOff = 1 + i * 3;
        const auto pkgNameOff =
            pandaFile_->GetBase() + metadata[baseOff];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto moduleNameOff =
            pandaFile_->GetBase() + metadata[baseOff + 1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto size = metadata[baseOff + 2];            // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto pkgName = pandaFile_->GetStringData(pandaFile_->GetIdFromPointer(pkgNameOff)).ToString();
        const auto moduleName = pandaFile_->GetStringData(pandaFile_->GetIdFromPointer(moduleNameOff)).ToString();

        metadataIndex_[MetadataModuleId(pkgName, moduleName)] = {uncompressedMetadataSize, size};

        uncompressedMetadataSize += size;
        LOG_METADATA("added " << pkgName << ":" << moduleName << " to the index (metadata size: " << size << " bytes)");
    }
    LOG_METADATA_NESTING_DEC();
    uncompressedMetadataSize_ = uncompressedMetadataSize;
    metadataSpan_ = metadataInfoSpan.SubSpan((1 + numMetadataItems * INDEX_ITEM_SIZE) * sizeof(uint32_t));
    LOG_METADATA("built metadata index for "
                 << numMetadataItems << " modules (metadata total size: " << uncompressedMetadataSize << " bytes)");

    LOG_METADATA_DISABLE();
}

EncodedMetadata MetadataAccessor::CompressMetadata(const MetadataByModules &metadata)
{
    LOG_METADATA_ENABLE();

    LOG_METADATA("compressing metadata (" << metadata.size() << " modules)");
    EncodedMetadata entireMetadata;

    LOG_METADATA_NESTING_INC();
    for (const auto &[moduleId, moduleMetadata] : metadata) {
        LOG_METADATA("adding metadata of module " << moduleId.ToString() << " (" << moduleMetadata.size() << " bytes)");
        entireMetadata.insert(entireMetadata.end(), moduleMetadata.begin(), moduleMetadata.end());
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

EncodedMetadata MetadataAccessor::UncompressMetadata(const EncodedMetadata &compressedMetadata) const
{
    LOG_METADATA_ENABLE();

    LOG_METADATA("uncompressing metadata (" << compressedMetadata.size() << " bytes)");

    ASSERT(uncompressedMetadataSize_ != 0);

    // Bellow is an assumption that uncompressed metadata is less than size of whole abc file,
    // to avoid unbounded allocation due to malformed metadata section (size fields)
    ASSERT(uncompressedMetadataSize_ < pandaFile_->GetHeader()->fileSize);

    auto uncompressedMetadata =
        std::make_unique<uint8_t[]>(uncompressedMetadataSize_);          // NOLINT(modernize-avoid-c-arrays)
    unsigned long uncompressedMetadataSize = uncompressedMetadataSize_;  // NOLINT(google-runtime-int)
    const auto res = uncompress(uncompressedMetadata.get(), &uncompressedMetadataSize, compressedMetadata.data(),
                                compressedMetadata.size());
    if (res != 0) {
        LOG_METADATA("uncompressing metadata error (code " << res << ")");
        uncompressedMetadata.reset();
        return {};  // Handle errors properly
    }

    LOG_METADATA("metadata uncompressed: " << compressedMetadata.size() << " bytes -> " << uncompressedMetadataSize
                                           << " bytes");

    LOG_METADATA_DISABLE();

    return {uncompressedMetadata.get(), uncompressedMetadata.get() + uncompressedMetadataSize_};
}

EncodedMetadata MetadataAccessor::GetMetadataForModule(const MetadataModuleId &moduleId)
{
    if (uncompressedMetadataSize_ == 0) {
        return {};
    }
    const auto data = metadataSpan_.data();
    const auto uncompressedMetadata = UncompressMetadata(
        {data, data + metadataSpan_.size()});  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return GetMetadataForModule(moduleId, uncompressedMetadata);
}

EncodedMetadata MetadataAccessor::GetMetadataForModule(const MetadataModuleId &moduleId,
                                                       const EncodedMetadata &uncompressedMetadata)
{
    if (uncompressedMetadata.empty()) {
        return {};
    }

    const auto &[offset, size] = metadataIndex_[moduleId];
    const auto metadataStartPos = uncompressedMetadata.begin() + offset;
    return {metadataStartPos, metadataStartPos + size};
}

MetadataByModules MetadataAccessor::GetMetadataForPackage(const MetadataModuleId &moduleId)
{
    LOG_METADATA_ENABLE();

    if (uncompressedMetadataSize_ == 0) {
        LOG_METADATA("accessing metadata for package " << moduleId.ToString() << ": metadata is empty");
        return {};
    }
    const auto data = metadataSpan_.data();
    const auto uncompressedMetadata = UncompressMetadata(
        {data, data + metadataSpan_.size()});  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (uncompressedMetadata.empty()) {
        return {};
    }

    MetadataByModules metadata;
    for (const auto &[moduleId_, metadataInfo] : metadataIndex_) {
        if (moduleId_.GetPkgName() == moduleId.GetPkgName()) {
            const auto &[offset, size] = metadataInfo;
            const auto metadataStartPos = uncompressedMetadata.begin() + offset;
            metadata[moduleId_] = {metadataStartPos, metadataStartPos + size};
        }
    }
    LOG_METADATA("accessing metadata for package " << moduleId.ToString() << ": " << metadata.size()
                                                   << " bytes in total");

    LOG_METADATA_DISABLE();

    return metadata;
}

MetadataByModules MetadataAccessor::GetMetadata()
{
    LOG_METADATA_ENABLE();

    if (uncompressedMetadataSize_ == 0) {
        LOG_METADATA("accessing metadata for abc file: metadata is empty");
        return {};
    }

    const auto data = metadataSpan_.data();
    const auto uncompressedMetadata = UncompressMetadata(
        {data, data + metadataSpan_.size()});  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (uncompressedMetadata.empty()) {
        return {};
    }

    MetadataByModules metadata;
    for (const auto &[moduleId, _] : metadataIndex_) {
        metadata[moduleId] = GetMetadataForModule(moduleId, uncompressedMetadata);
    }

    LOG_METADATA("accessing metadata for abc file: " << metadata.size() << " bytes in total");

    LOG_METADATA_DISABLE();

    return metadata;
}

}  // namespace ark::panda_file