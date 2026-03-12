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

#ifndef PANDA_COMPILER_TOOLS_APTOOL_COMMON_ABC_METADATA_H
#define PANDA_COMPILER_TOOLS_APTOOL_COMMON_ABC_METADATA_H

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "compiler/tools/aptool/common/profile_loader.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/pandargs.h"
#include "libarkfile/file.h"

namespace ark::aptool::common {

class AbcMetadataProvider {
public:
    struct MethodInfo {
        std::string classDescriptor;
        std::string methodSignature;
    };

    AbcMetadataProvider() = default;

    bool Load(const arg_list_t &files, const arg_list_t &dirs);

    std::optional<MethodInfo> GetMethodInfo(const PandaString &pandaFileName, uint32_t methodIdx) const;

    std::optional<std::string> GetClassDescriptor(const PandaString &pandaFileName, uint32_t classIdx) const;

    std::optional<std::string> GetInstructionString(const PandaString &pandaFileName, uint32_t methodIdx,
                                                    uint32_t pc) const;

    bool VerifyClassContext(const PandaString &classCtxStr, std::string &error) const;

private:
    struct ContextEntry {
        std::string path;
        std::string checksum;
    };

    struct FileRecord {
        fs::path path;
        std::string normalized;
        std::string baseName;
        std::unique_ptr<const panda_file::File> file;
    };

    struct MethodCodeView {
        const uint8_t *instructions {nullptr};
        uint32_t codeSize {0};
    };

    struct MethodKey {
        const panda_file::File *file {nullptr};
        uint32_t methodIdx {0};

        bool operator==(const MethodKey &other) const
        {
            return file == other.file && methodIdx == other.methodIdx;
        }
    };

    struct MethodKeyHash {
        size_t operator()(const MethodKey &key) const
        {
            auto fileHash = std::hash<const panda_file::File *> {}(key.file);
            auto idxHash = std::hash<uint32_t> {}(key.methodIdx);
            return fileHash ^ (idxHash << 1U);
        }
    };

    static std::string NormalizePath(const fs::path &path);

    const FileRecord *FindRecordByName(std::string_view name) const;

    const FileRecord *FindRecordForContext(const ContextEntry &entry) const;

    const MethodCodeView *GetOrCreateMethodCodeView(const FileRecord &record, uint32_t methodIdx) const;

    bool AddFile(const fs::path &path);

    bool LoadFiles(const arg_list_t &files);
    bool CollectFromDirs(const arg_list_t &dirs, arg_list_t &outFiles);

    static std::vector<ContextEntry> ParseClassContext(const PandaString &ctx);

    static std::optional<std::string> GetClassDescriptorInternal(const panda_file::File &file,
                                                                 panda_file::File::EntityId classId);

    static std::string BuildMethodSignature(const panda_file::File &file, panda_file::File::EntityId methodId);

    static bool ContainsMethod(const panda_file::File &file, panda_file::File::EntityId methodId);

    static bool ContainsClass(const panda_file::File &file, panda_file::File::EntityId classId);

    std::vector<std::unique_ptr<FileRecord>> records_;
    std::unordered_map<std::string, FileRecord *> recordsByNormalized_;
    std::unordered_map<std::string, std::vector<FileRecord *>> recordsByBaseName_;
    mutable std::unordered_map<MethodKey, MethodCodeView, MethodKeyHash> codeCache_;
};

}  // namespace ark::aptool::common

#endif  // PANDA_COMPILER_TOOLS_APTOOL_COMMON_ABC_METADATA_H
