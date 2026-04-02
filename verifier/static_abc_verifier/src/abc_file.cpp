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

#include "abc_file.h"

#include <fstream>
#include <cstring>

namespace ark::static_verifier {

struct AbcFile::FactoryHelper : AbcFile {
    FactoryHelper() = default;
};

std::unique_ptr<AbcFile> AbcFile::CreateEmpty()
{
    return std::make_unique<FactoryHelper>();
}

std::unique_ptr<AbcFile> AbcFile::Open(const std::string &filename)
{
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) {
        return nullptr;
    }

    auto size = static_cast<size_t>(ifs.tellg());
    if (size < sizeof(AbcHeader)) {
        return nullptr;
    }

    ifs.seekg(0, std::ios::beg);
    auto file = AbcFile::CreateEmpty();
    file->data_.resize(size);
    if (!ifs.read(reinterpret_cast<char *>(file->data_.data()), static_cast<std::streamsize>(size))) {
        return nullptr;
    }

    return file;
}

std::unique_ptr<AbcFile> AbcFile::OpenFromMemory(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(AbcHeader)) {
        return nullptr;
    }

    auto file = AbcFile::CreateEmpty();
    file->data_.assign(data, data + size);
    return file;
}

const AbcHeader *AbcFile::GetHeader() const
{
    if (data_.size() < sizeof(AbcHeader)) {
        return nullptr;
    }
    return reinterpret_cast<const AbcHeader *>(data_.data());
}

const RegionHeader *AbcFile::GetRegionHeaders() const
{
    auto *header = GetHeader();
    if (header == nullptr || !IsRangeValid(header->indexSectionOff,
                                           header->numIndexes * sizeof(RegionHeader))) {
        return nullptr;
    }
    return reinterpret_cast<const RegionHeader *>(data_.data() + header->indexSectionOff);
}

uint32_t AbcFile::GetNumRegionHeaders() const
{
    auto *header = GetHeader();
    return header != nullptr ? header->numIndexes : 0;
}

const uint32_t *AbcFile::GetClassIndex() const
{
    auto *header = GetHeader();
    if (header == nullptr || !IsRangeValid(header->classIdxOff,
                                           header->numClasses * sizeof(uint32_t))) {
        return nullptr;
    }
    return reinterpret_cast<const uint32_t *>(data_.data() + header->classIdxOff);
}

uint32_t AbcFile::GetNumClasses() const
{
    auto *header = GetHeader();
    return header != nullptr ? header->numClasses : 0;
}

const uint32_t *AbcFile::GetLiteralArrayIndex() const
{
    auto *header = GetHeader();
    if (header == nullptr || !IsRangeValid(header->literalarrayIdxOff,
                                           header->numLiteralarrays * sizeof(uint32_t))) {
        return nullptr;
    }
    return reinterpret_cast<const uint32_t *>(data_.data() + header->literalarrayIdxOff);
}

uint32_t AbcFile::GetNumLiteralArrays() const
{
    auto *header = GetHeader();
    return header != nullptr ? header->numLiteralarrays : 0;
}

bool AbcFile::IsRangeValid(uint32_t offset, uint32_t size) const
{
    if (offset > data_.size()) {
        return false;
    }
    return (data_.size() - offset) >= size;
}

}  // namespace ark::static_verifier
