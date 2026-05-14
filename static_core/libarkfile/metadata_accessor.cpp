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

namespace ark::panda_file {

void MetadataAccessor::SetFile(const File &pandaFile)
{
    ASSERT(this->pandaFile_ == nullptr);

    this->pandaFile_ = &pandaFile;

    BuildIndex();
}

void MetadataAccessor::BuildIndex()
{
    const auto metadataInfoSpan = pandaFile_->GetMetadata();
    if (metadataInfoSpan.empty()) {
        return;
    }

    const auto metadata = metadataInfoSpan.data();

    const uint32_t numMetadataItems = metadata[0];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    uint64_t uncompressedMetadataSize = 0;
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
    }
    uncompressedMetadataSize_ = uncompressedMetadataSize;
    metadataSpan_ = metadataInfoSpan.SubSpan(1 + numMetadataItems * INDEX_ITEM_SIZE);
}

EncodedMetadata MetadataAccessor::CompressMetadata(const MetadataByModules &metadata)
{
    EncodedMetadata entireMetadata;

    for (const auto &[_, moduleMetadata] : metadata) {
        entireMetadata.insert(entireMetadata.end(), moduleMetadata.begin(), moduleMetadata.end());
    }

    unsigned long entireMetadataSize = entireMetadata.size();  // NOLINT(google-runtime-int)
    // Free unused trailing memory after compression
    auto compressedMetadata = std::make_unique<uint8_t[]>(entireMetadataSize);  // NOLINT(modernize-avoid-c-arrays)
    auto res = compress(compressedMetadata.get(), &entireMetadataSize, entireMetadata.data(), entireMetadataSize);
    if (res != 0) {
        compressedMetadata.reset();
        return {};  // Handle errors properly
    }

    return {compressedMetadata.get(), compressedMetadata.get() + entireMetadataSize};
}

EncodedMetadata MetadataAccessor::UncompressMetadata(const EncodedMetadata &compressedMetadata) const
{
    ASSERT(uncompressedMetadataSize_ != 0);

    auto uncompressedMetadata =
        std::make_unique<uint8_t[]>(uncompressedMetadataSize_);          // NOLINT(modernize-avoid-c-arrays)
    unsigned long uncompressedMetadataSize = uncompressedMetadataSize_;  // NOLINT(google-runtime-int)
    const auto res = uncompress(uncompressedMetadata.get(), &uncompressedMetadataSize, compressedMetadata.data(),
                                compressedMetadata.size());
    if (res != 0) {
        uncompressedMetadata.reset();
        return {};  // Handle errors properly
    }

    return {uncompressedMetadata.get(), uncompressedMetadata.get() + uncompressedMetadataSize_};
}

EncodedMetadata MetadataAccessor::GetMetadataFor(const MetadataModuleId &moduleId)
{
    if (uncompressedMetadataSize_ == 0) {
        return {};
    }
    const auto data = reinterpret_cast<const uint8_t *>(metadataSpan_.data());
    const auto uncompressedMetadata = UncompressMetadata(
        {data, data +                                         // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                   metadataSpan_.size() * sizeof(uint32_t) +  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                   sizeof(uint32_t)});
    return GetMetadataFor(moduleId, uncompressedMetadata);
}

EncodedMetadata MetadataAccessor::GetMetadataFor(const MetadataModuleId &moduleId,
                                                 const EncodedMetadata &uncompressedMetadata)
{
    const auto &[offset, size] = metadataIndex_[moduleId];
    const auto metadataStartPos = uncompressedMetadata.begin() + offset;
    return {metadataStartPos, metadataStartPos + size};
}

MetadataByModules MetadataAccessor::GetMetadata()
{
    if (uncompressedMetadataSize_ == 0) {
        return {};
    }

    const auto data = reinterpret_cast<const uint8_t *>(metadataSpan_.data());
    const auto uncompressedMetadata = UncompressMetadata(
        {data, data +                                         // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                   metadataSpan_.size() * sizeof(uint32_t) +  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                   sizeof(uint32_t)});

    MetadataByModules metadata;
    for (const auto &[moduleId, _] : metadataIndex_) {
        metadata[moduleId] = GetMetadataFor(moduleId, uncompressedMetadata);
    }

    return metadata;
}

}  // namespace ark::panda_file