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

#ifndef COMPILER_AOT_AOT_FILE_H
#define COMPILER_AOT_AOT_FILE_H

#include "aot_headers.h"
#include "compiler/code_info/code_info.h"
#include "os/library_loader.h"
#include "utils/span.h"
#include "libpandafile/file.h"

#include <string>
#include <array>
#include <memory>
#include <algorithm>

namespace panda::compiler {
class RuntimeInterface;

class AotFile {
public:
    static constexpr std::array MAGIC = {'.', 'a', 'n', '\0'};
    static constexpr std::array VERSION = {'0', '0', '6', '\0'};

    enum AotSlotType { PLT_SLOT = 1, VTABLE_INDEX = 2, CLASS_SLOT = 3, STRING_SLOT = 4, INLINECACHE_SLOT = 5 };

    AotFile(panda::os::library_loader::LibraryHandle &&handle, Span<const uint8_t> aot_data, Span<const uint8_t> code)
        : handle_(std::move(handle)), aot_data_(aot_data), code_(code)
    {
    }

    NO_MOVE_SEMANTIC(AotFile);
    NO_COPY_SEMANTIC(AotFile);
    ~AotFile() = default;

public:
    static Expected<std::unique_ptr<AotFile>, std::string> Open(const std::string &file_name, uint32_t gc_type,
                                                                bool for_dump = false);

    const void *GetCode() const
    {
        return code_.data();
    }

    size_t GetCodeSize() const
    {
        return code_.size();
    }

    auto FileHeaders() const
    {
        return aot_data_.SubSpan<const PandaFileHeader>(GetAotHeader()->files_offset, GetFilesCount());
    }

    auto GetMethodHeader(size_t index) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<const MethodHeader *>(&aot_data_[GetAotHeader()->methods_offset]) + index;
    }

    const MethodHeader *GetMethodHeadersPtr() const
    {
        return reinterpret_cast<const MethodHeader *>(&aot_data_[GetAotHeader()->methods_offset]);
    }

    auto GetClassHeaders(const PandaFileHeader &file_header) const
    {
        return aot_data_.SubSpan<const ClassHeader>(GetAotHeader()->classes_offset +
                                                        file_header.classes_offset * sizeof(ClassHeader),
                                                    file_header.classes_count);
    }

    auto GetClassHashTable(const PandaFileHeader &file_header) const
    {
        return aot_data_.SubSpan<const panda::panda_file::EntityPairHeader>(GetAotHeader()->class_hash_tables_offset +
                                                                                file_header.class_hash_table_offset,
                                                                            file_header.class_hash_table_size);
    }

    const uint8_t *GetMethodsBitmap() const
    {
        return &aot_data_[GetAotHeader()->bitmap_offset];
    }

    size_t GetFilesCount() const
    {
        return GetAotHeader()->files_count;
    }

    const PandaFileHeader *FindPandaFile(const std::string &file_name) const
    {
        auto file_headers = FileHeaders();
        auto res = std::find_if(file_headers.begin(), file_headers.end(), [this, &file_name](auto &header) {
            return file_name == GetString(header.file_name_str);
        });
        return res == file_headers.end() ? nullptr : res;
    }

    const uint8_t *GetMethodCode(const MethodHeader *method_header) const
    {
        return code_.data() + method_header->code_offset;
    }

    const char *GetString(size_t offset) const
    {
        return reinterpret_cast<const char *>(aot_data_.data() + GetAotHeader()->strtab_offset + offset);
    }

    const AotHeader *GetAotHeader() const
    {
        return reinterpret_cast<const AotHeader *>(aot_data_.data());
    }

    const char *GetFileName() const
    {
        return GetString(GetAotHeader()->file_name_str);
    }

    const char *GetCommandLine() const
    {
        return GetString(GetAotHeader()->cmdline_str);
    }

    const char *GetClassContext() const
    {
        return GetString(GetAotHeader()->class_ctx_str);
    }

    bool IsCompiledWithCha() const
    {
        return GetAotHeader()->with_cha != 0U;
    }

    bool IsBootPandaFile() const
    {
        return GetAotHeader()->boot_aot != 0U;
    }

    void InitializeGot(RuntimeInterface *runtime);

    void PatchTable(RuntimeInterface *runtime);

private:
    panda::os::library_loader::LibraryHandle handle_ {nullptr};
    Span<const uint8_t> aot_data_;
    Span<const uint8_t> code_;
};

class AotClass final {
public:
    AotClass() = default;
    AotClass(const AotFile *file, const ClassHeader *header) : aot_file_(file), header_(header) {}
    ~AotClass() = default;
    DEFAULT_COPY_SEMANTIC(AotClass);
    DEFAULT_MOVE_SEMANTIC(AotClass);

    const void *FindMethodCodeEntry(size_t index) const;
    Span<const uint8_t> FindMethodCodeSpan(size_t index) const;
    const MethodHeader *FindMethodHeader(size_t index) const;

    BitVectorSpan GetBitmap() const;

    auto GetMethodHeaders() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return Span(aot_file_->GetMethodHeadersPtr() + header_->methods_offset, header_->methods_count);
    }

    static AotClass Invalid()
    {
        return AotClass();
    }

    bool IsValid() const
    {
        return header_ != nullptr;
    }

private:
    const AotFile *aot_file_ {nullptr};
    const ClassHeader *header_ {nullptr};
};

class AotPandaFile {
public:
    AotPandaFile() = default;
    AotPandaFile(AotFile *file, const PandaFileHeader *header) : aot_file_(file), header_(header)
    {
        LoadClassHashTable();
    }

    DEFAULT_MOVE_SEMANTIC(AotPandaFile);
    DEFAULT_COPY_SEMANTIC(AotPandaFile);
    ~AotPandaFile() = default;

    const AotFile *GetAotFile() const
    {
        return aot_file_;
    }
    const PandaFileHeader *GetHeader()
    {
        return header_;
    }
    const PandaFileHeader *GetHeader() const
    {
        return header_;
    }
    std::string GetFileName() const
    {
        return GetAotFile()->GetString(GetHeader()->file_name_str);
    }
    AotClass GetClass(uint32_t class_id) const;

    Span<const ClassHeader> GetClassHeaders() const
    {
        return aot_file_->GetClassHeaders(*header_);
    }

    CodeInfo GetMethodCodeInfo(const MethodHeader *method_header) const
    {
        return CodeInfo(GetAotFile()->GetMethodCode(method_header), method_header->code_size);
    }

    void LoadClassHashTable()
    {
        class_hash_table_ = GetAotFile()->GetClassHashTable(*header_);
    }

    panda::Span<const panda::panda_file::EntityPairHeader> GetClassHashTable() const
    {
        return class_hash_table_;
    }

private:
    AotFile *aot_file_ {nullptr};
    const PandaFileHeader *header_ {nullptr};
    panda::Span<const panda::panda_file::EntityPairHeader> class_hash_table_;
};
}  // namespace panda::compiler

#endif  // COMPILER_AOT_AOT_FILE_H
