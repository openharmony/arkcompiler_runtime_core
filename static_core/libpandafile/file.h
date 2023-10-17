/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef LIBPANDAFILE_FILE_H_
#define LIBPANDAFILE_FILE_H_

#include "os/mem.h"
#include "utils/span.h"
#include "utils/utf.h"
#include <cstdint>

#include <array>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

namespace panda {
struct EntryFileStat;
}  // namespace panda

namespace panda::panda_file {

class PandaCache;

/*
 * EntityPairHeader Describes pair for hash value of class's descriptor and its entity id offset,
 * used to quickly find class by its descriptor.
 */
struct EntityPairHeader {
    uint32_t descriptor_hash;
    uint32_t entity_id_offset;
    uint32_t next_pos;
};

class File {
public:
    using Index = uint16_t;
    using Index32 = uint32_t;

    static constexpr size_t MAGIC_SIZE = 8;
    static constexpr size_t VERSION_SIZE = 4;
    static const std::array<uint8_t, MAGIC_SIZE> MAGIC;

    struct Header {
        std::array<uint8_t, MAGIC_SIZE> magic;
        uint32_t checksum;
        std::array<uint8_t, VERSION_SIZE> version;
        uint32_t file_size;
        uint32_t foreign_off;
        uint32_t foreign_size;
        uint32_t quickened_flag;
        uint32_t num_classes;
        uint32_t class_idx_off;
        uint32_t num_lnps;
        uint32_t lnp_idx_off;
        uint32_t num_literalarrays;
        uint32_t literalarray_idx_off;
        uint32_t num_indexes;
        uint32_t index_section_off;
    };

    struct RegionHeader {
        uint32_t start;
        uint32_t end;
        uint32_t class_idx_size;
        uint32_t class_idx_off;
        uint32_t method_idx_size;
        uint32_t method_idx_off;
        uint32_t field_idx_size;
        uint32_t field_idx_off;
        uint32_t proto_idx_size;
        uint32_t proto_idx_off;
    };

    struct StringData {
        StringData(uint32_t len, const uint8_t *d) : utf16_length(len), is_ascii(false), data(d) {}
        StringData() = default;
        uint32_t utf16_length;  // NOLINT(misc-non-private-member-variables-in-classes)
        bool is_ascii;          // NOLINT(misc-non-private-member-variables-in-classes)
        const uint8_t *data;    // NOLINT(misc-non-private-member-variables-in-classes)
    };

    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
    class EntityId {
    public:
        explicit constexpr EntityId(uint32_t offset) : offset_(offset) {}

        EntityId() = default;

        ~EntityId() = default;

        bool IsValid() const
        {
            return offset_ > sizeof(Header);
        }

        uint32_t GetOffset() const
        {
            return offset_;
        }

        static constexpr size_t GetSize()
        {
            return sizeof(uint32_t);
        }

        friend bool operator<(const EntityId &l, const EntityId &r)
        {
            return l.offset_ < r.offset_;
        }

        friend bool operator==(const EntityId &l, const EntityId &r)
        {
            return l.offset_ == r.offset_;
        }

        friend std::ostream &operator<<(std::ostream &stream, const EntityId &id)
        {
            return stream << id.offset_;
        }

    private:
        uint32_t offset_ {0};
    };

    enum OpenMode { READ_ONLY, READ_WRITE, WRITE_ONLY };

    StringData GetStringData(EntityId id) const;
    EntityId GetLiteralArraysId() const;

    EntityId GetClassId(const uint8_t *mutf8_name) const;

    EntityId GetClassIdFromClassHashTable(const uint8_t *mutf8_name) const;

    const Header *GetHeader() const
    {
        return reinterpret_cast<const Header *>(GetBase());
    }

    const uint8_t *GetBase() const
    {
        return reinterpret_cast<const uint8_t *>(base_.Get());
    }

    const os::mem::ConstBytePtr &GetPtr() const
    {
        return base_;
    }

    bool IsExternal(EntityId id) const
    {
        const Header *header = GetHeader();
        uint32_t foreign_begin = header->foreign_off;
        uint32_t foreign_end = foreign_begin + header->foreign_size;
        return id.GetOffset() >= foreign_begin && id.GetOffset() < foreign_end;
    }

    EntityId GetIdFromPointer(const uint8_t *ptr) const
    {
        return EntityId(ptr - GetBase());
    }

    Span<const uint8_t> GetSpanFromId(EntityId id) const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        return file.Last(file.size() - id.GetOffset());
    }

    Span<const uint32_t> GetClasses() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        Span class_idx_data = file.SubSpan(header->class_idx_off, header->num_classes * sizeof(uint32_t));
        return Span(reinterpret_cast<const uint32_t *>(class_idx_data.data()), header->num_classes);
    }

    Span<const uint32_t> GetLiteralArrays() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        Span litarr_idx_data = file.SubSpan(header->literalarray_idx_off, header->num_literalarrays * sizeof(uint32_t));
        return Span(reinterpret_cast<const uint32_t *>(litarr_idx_data.data()), header->num_literalarrays);
    }

    Span<const RegionHeader> GetRegionHeaders() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        auto sp = file.SubSpan(header->index_section_off, header->num_indexes * sizeof(RegionHeader));
        return Span(reinterpret_cast<const RegionHeader *>(sp.data()), header->num_indexes);
    }

    const RegionHeader *GetRegionHeader(EntityId id) const
    {
        auto headers = GetRegionHeaders();
        auto offset = id.GetOffset();
        for (const auto &header : headers) {
            if (header.start <= offset && offset < header.end) {
                return &header;
            }
        }
        return nullptr;
    }

    Span<const EntityId> GetClassIndex(const RegionHeader *region_header) const
    {
        auto *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ASSERT(region_header != nullptr);
        auto sp = file.SubSpan(region_header->class_idx_off, region_header->class_idx_size * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), region_header->class_idx_size);
    }

    Span<const EntityId> GetClassIndex(EntityId id) const
    {
        auto *region_header = GetRegionHeader(id);
        ASSERT(region_header != nullptr);
        return GetClassIndex(region_header);
    }

    Span<const EntityId> GetMethodIndex(const RegionHeader *region_header) const
    {
        auto *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ASSERT(region_header != nullptr);
        auto sp = file.SubSpan(region_header->method_idx_off, region_header->method_idx_size * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), region_header->method_idx_size);
    }

    Span<const EntityId> GetMethodIndex(EntityId id) const
    {
        auto *region_header = GetRegionHeader(id);
        ASSERT(region_header != nullptr);
        return GetMethodIndex(region_header);
    }

    Span<const EntityId> GetFieldIndex(const RegionHeader *region_header) const
    {
        auto *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ASSERT(region_header != nullptr);
        auto sp = file.SubSpan(region_header->field_idx_off, region_header->field_idx_size * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), region_header->field_idx_size);
    }

    Span<const EntityId> GetFieldIndex(EntityId id) const
    {
        auto *region_header = GetRegionHeader(id);
        ASSERT(region_header != nullptr);
        return GetFieldIndex(region_header);
    }

    Span<const EntityId> GetProtoIndex(const RegionHeader *region_header) const
    {
        auto *header = GetHeader();
        Span file(GetBase(), header->file_size);
        ASSERT(region_header != nullptr);
        auto sp = file.SubSpan(region_header->proto_idx_off, region_header->proto_idx_size * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(sp.data()), region_header->proto_idx_size);
    }

    Span<const EntityId> GetProtoIndex(EntityId id) const
    {
        auto *region_header = GetRegionHeader(id);
        ASSERT(region_header != nullptr);
        return GetProtoIndex(region_header);
    }

    Span<const EntityId> GetLineNumberProgramIndex() const
    {
        const Header *header = GetHeader();
        Span file(GetBase(), header->file_size);
        Span lnp_idx_data = file.SubSpan(header->lnp_idx_off, header->num_lnps * EntityId::GetSize());
        return Span(reinterpret_cast<const EntityId *>(lnp_idx_data.data()), header->num_lnps);
    }

    EntityId ResolveClassIndex(EntityId id, Index idx) const
    {
        auto index = GetClassIndex(id);
        return index[idx];
    }

    EntityId ResolveMethodIndex(EntityId id, Index idx) const
    {
        auto index = GetMethodIndex(id);
        return index[idx];
    }

    EntityId ResolveFieldIndex(EntityId id, Index idx) const
    {
        auto index = GetFieldIndex(id);
        return index[idx];
    }

    EntityId ResolveProtoIndex(EntityId id, Index idx) const
    {
        auto index = GetProtoIndex(id);
        return index[idx];
    }

    EntityId ResolveLineNumberProgramIndex(Index32 idx) const
    {
        auto index = GetLineNumberProgramIndex();
        return index[idx];
    }

    const std::string &GetFilename() const
    {
        return filename_;
    }

    PandaCache *GetPandaCache() const
    {
        return panda_cache_.get();
    }

    uint32_t GetFilenameHash() const
    {
        return filename_hash_;
    }

    // note: intentionally returns uint64_t instead of the field type due to usage
    uint64_t GetUniqId() const
    {
        return uniq_id_;
    }

    const std::string &GetFullFileName() const
    {
        return full_filename_;
    }

    static constexpr uint32_t GetFileBaseOffset()
    {
        return MEMBER_OFFSET(File, base_);
    }

    Span<const panda::panda_file::EntityPairHeader> GetClassHashTable() const
    {
        return class_hash_table_;
    }

    std::string GetPaddedChecksum() const
    {
        std::stringstream padded_checksum;
        // Length of hexed maximum unit32_t value of checksum (0xFFFFFFFF) is equal to 8
        constexpr size_t CHECKSUM_LENGTH = 8;
        padded_checksum << std::setfill('0') << std::setw(CHECKSUM_LENGTH) << std::hex << GetHeader()->checksum;
        return padded_checksum.str();
    }

    static uint32_t CalcFilenameHash(const std::string &filename);

    static std::unique_ptr<const File> Open(std::string_view filename, OpenMode open_mode = READ_ONLY);

    static std::unique_ptr<const File> OpenFromMemory(os::mem::ConstBytePtr &&ptr);

    static std::unique_ptr<const File> OpenFromMemory(os::mem::ConstBytePtr &&ptr, std::string_view filename);

    static std::unique_ptr<const File> OpenUncompressedArchive(int fd, const std::string_view &filename, size_t size,
                                                               uint32_t offset, OpenMode open_mode = READ_ONLY);

    void SetClassHashTable(panda::Span<const panda::panda_file::EntityPairHeader> class_hash_table) const
    {
        class_hash_table_ = class_hash_table;
    }

    ~File();

    NO_COPY_SEMANTIC(File);
    NO_MOVE_SEMANTIC(File);

private:
    File(std::string filename, os::mem::ConstBytePtr &&base);

    os::mem::ConstBytePtr base_;
    const std::string filename_;
    const uint32_t filename_hash_;
    const std::string full_filename_;
    std::unique_ptr<PandaCache> panda_cache_;
    const uint32_t uniq_id_;
    mutable panda::Span<const panda::panda_file::EntityPairHeader> class_hash_table_;
};

static_assert(File::GetFileBaseOffset() == 0);

inline bool operator==(const File::StringData &string_data1, const File::StringData &string_data2)
{
    if (string_data1.utf16_length != string_data2.utf16_length) {
        return false;
    }

    return utf::IsEqual(string_data1.data, string_data2.data);
}

inline bool operator!=(const File::StringData &string_data1, const File::StringData &string_data2)
{
    return !(string_data1 == string_data2);
}

inline bool operator<(const File::StringData &string_data1, const File::StringData &string_data2)
{
    if (string_data1.utf16_length == string_data2.utf16_length) {
        return utf::CompareMUtf8ToMUtf8(string_data1.data, string_data2.data) < 0;
    }

    return string_data1.utf16_length < string_data2.utf16_length;
}

/*
 * OpenPandaFileOrZip from location which specicify the name.
 */
std::unique_ptr<const File> OpenPandaFileOrZip(std::string_view location,
                                               panda_file::File::OpenMode open_mode = panda_file::File::READ_ONLY);

/*
 * OpenPandaFileFromMemory from file buffer.
 */
std::unique_ptr<const File> OpenPandaFileFromMemory(const void *buffer, size_t size);

/*
 * OpenPandaFile from location which specicify the name.
 */
std::unique_ptr<const File> OpenPandaFile(std::string_view location, std::string_view archive_filename = "",
                                          panda_file::File::OpenMode open_mode = panda_file::File::READ_ONLY);

/*
 * Check ptr point valid panda file: magic
 */
bool CheckHeader(const os::mem::ConstBytePtr &ptr, const std::string_view &filename = "");

// NOLINTNEXTLINE(readability-identifier-naming)
extern const char *ARCHIVE_FILENAME;
}  // namespace panda::panda_file

namespace std {
template <>
struct hash<panda::panda_file::File::EntityId> {
    std::size_t operator()(panda::panda_file::File::EntityId id) const
    {
        return std::hash<uint32_t> {}(id.GetOffset());
    }
};
}  // namespace std

#endif
