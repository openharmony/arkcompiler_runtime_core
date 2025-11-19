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

#include <sstream>

#include "assembler/assembly-type.h"
#include "compiler/tools/aptool/common/abc_metadata.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkfile/proto_data_accessor.h"

namespace ark::aptool::common {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_APTOOL(level) LOG(level, PROFILER) << "APTOOL: "

namespace {

constexpr char CLASS_CTX_ENTRY_DELIM = ':';
constexpr char CLASS_CTX_HASH_DELIM = '*';

class PandasmTypePrinter {
public:
    explicit PandasmTypePrinter(const panda_file::File &file, panda_file::ProtoDataAccessor &pda)
        : file_(file), pda_(pda)
    {
    }

    std::string Dump(panda_file::Type type)
    {
        if (!type.IsReference()) {
            // Convert primitive type to pandasm type name
            pandasm::Type pandasmType = pandasm::Type::FromPrimitiveId(type.GetId());
            return pandasmType.GetPandasmName();
        }
        auto refCount = pda_.GetRefNum();
        if (refIdx_ >= refCount) {
            LOG_APTOOL(WARNING) << "proto reference count mismatch while printing type";
            return "<invalid-ref>";
        }
        auto refType = pda_.GetReferenceType(refIdx_++);
        std::string descriptor(utf::Mutf8AsCString(file_.GetStringData(refType).data));
        // Convert descriptor to pandasm type
        pandasm::Type pandasmType = pandasm::Type::FromDescriptor(descriptor);
        return pandasmType.GetPandasmName();
    }

private:
    const panda_file::File &file_;
    panda_file::ProtoDataAccessor &pda_;
    uint32_t refIdx_ {0};
};

}  // namespace

bool AbcMetadataProvider::Load(const arg_list_t &files, const arg_list_t &dirs)
{
    arg_list_t allFiles(files.begin(), files.end());
    if (!CollectFromDirs(dirs, allFiles)) {
        return false;
    }
    return LoadFiles(allFiles);
}

bool AbcMetadataProvider::LoadFiles(const arg_list_t &files)
{
    if (files.empty()) {
        return false;
    }
    bool allLoaded = true;
    for (const auto &file : files) {
        fs::path path(file);
        if (path.extension() != ".abc") {
            LOG_APTOOL(WARNING) << "skip non-abc file '" << file << "'";
            allLoaded = false;
            continue;
        }
        if (!AddFile(path)) {
            allLoaded = false;
        }
    }
    return allLoaded;
}

bool AbcMetadataProvider::CollectFromDirs(const arg_list_t &dirs, arg_list_t &outFiles)
{
    for (const auto &dir : dirs) {
        fs::path path(dir);
        std::error_code ec;
        if (!fs::is_directory(path, ec)) {
            LOG_APTOOL(ERROR) << "'" << dir << "' is not a directory";
            return false;
        }
        for (const auto &entry : fs::directory_iterator(path, ec)) {
            if (entry.is_regular_file(ec) && entry.path().extension() == ".abc") {
                outFiles.push_back(entry.path().string());
            }
        }
    }
    return true;
}

std::optional<AbcMetadataProvider::MethodInfo> AbcMetadataProvider::GetMethodInfo(const PandaString &pandaFileName,
                                                                                  uint32_t methodIdx) const
{
    auto *record = FindRecordByName(pandaFileName.c_str());
    if (record == nullptr) {
        return std::nullopt;
    }
    panda_file::File::EntityId methodId(methodIdx);
    if (methodId.GetOffset() >= record->file->GetHeader()->fileSize) {
        return std::nullopt;
    }
    auto *region = record->file->GetRegionHeader(methodId);
    if (region == nullptr) {
        LOG_APTOOL(WARNING) << "skip method @" << methodIdx << " in file '" << pandaFileName
                            << "' (invalid method region)";
        return std::nullopt;
    }
    if (!ContainsMethod(*record->file, methodId)) {
        // Allow methods that are missing from the index if we can still read their metadata.
    }
    panda_file::MethodDataAccessor mda(*record->file, methodId);
    MethodInfo info;
    auto classDescriptor = GetClassDescriptorInternal(*record->file, mda.GetClassId());
    if (classDescriptor.has_value()) {
        info.classDescriptor = *classDescriptor;
    }
    info.methodSignature = BuildMethodSignature(*record->file, methodId);
    return info;
}

std::optional<std::string> AbcMetadataProvider::GetClassDescriptor(const PandaString &pandaFileName,
                                                                   uint32_t classIdx) const
{
    auto *record = FindRecordByName(pandaFileName.c_str());
    if (record == nullptr) {
        return std::nullopt;
    }
    panda_file::File::EntityId classId(classIdx);
    if (classId.GetOffset() >= record->file->GetHeader()->fileSize) {
        return std::nullopt;
    }
    if (!ContainsClass(*record->file, classId)) {
        LOG_APTOOL(WARNING) << "skip class @" << classIdx << " in file '" << pandaFileName
                            << "' (not found in class index)";
        return std::nullopt;
    }
    return GetClassDescriptorInternal(*record->file, classId);
}

std::optional<std::string> AbcMetadataProvider::GetInstructionString(const PandaString &pandaFileName,
                                                                     uint32_t methodIdx, uint32_t pc) const
{
    auto *record = FindRecordByName(pandaFileName.c_str());
    if (record == nullptr) {
        return std::nullopt;
    }
    auto *codeView = GetOrCreateMethodCodeView(*record, methodIdx);
    if (codeView == nullptr || codeView->instructions == nullptr || pc >= codeView->codeSize) {
        return std::nullopt;
    }
    const uint8_t *pcAddr = codeView->instructions + pc;
    BytecodeInstruction inst(pcAddr);
    std::ostringstream oss;
    oss << inst;
    return oss.str();
}

bool AbcMetadataProvider::VerifyClassContext(const PandaString &classCtxStr, std::string &error) const
{
    if (records_.empty() || classCtxStr.empty()) {
        return true;
    }
    auto entries = ParseClassContext(classCtxStr);
    for (auto &entry : entries) {
        auto *record = FindRecordForContext(entry);
        if (record == nullptr) {
            LOG_APTOOL(WARNING) << "cannot find .abc file for context entry '" << entry.path
                                << "', skipping checksum verification";
            continue;
        }
        if (record->file->GetPaddedChecksum() != entry.checksum) {
            error = "Checksum mismatch for '" + entry.path + "'";
            return false;
        }
    }
    return true;
}

std::string AbcMetadataProvider::NormalizePath(const fs::path &path)
{
    std::error_code ec;
    auto absolute = fs::absolute(path, ec);
    if (ec) {
        return path.lexically_normal().generic_string();
    }
    return absolute.lexically_normal().generic_string();
}

const AbcMetadataProvider::FileRecord *AbcMetadataProvider::FindRecordByName(std::string_view name) const
{
    if (name.empty()) {
        return nullptr;
    }
    auto normalized = NormalizePath(fs::path(name));
    auto it = recordsByNormalized_.find(normalized);
    if (it != recordsByNormalized_.end()) {
        return it->second;
    }
    auto baseName = fs::path(std::string(name)).filename().string();
    auto baseIt = recordsByBaseName_.find(baseName);
    if (baseIt == recordsByBaseName_.end()) {
        return nullptr;
    }
    return baseIt->second.empty() ? nullptr : baseIt->second.front();
}

const AbcMetadataProvider::FileRecord *AbcMetadataProvider::FindRecordForContext(const ContextEntry &entry) const
{
    auto normalized = NormalizePath(fs::path(entry.path));
    auto it = recordsByNormalized_.find(normalized);
    if (it != recordsByNormalized_.end()) {
        return it->second;
    }
    auto baseName = fs::path(entry.path).filename().string();
    auto baseIt = recordsByBaseName_.find(baseName);
    if (baseIt == recordsByBaseName_.end()) {
        return nullptr;
    }
    for (auto *record : baseIt->second) {
        if (record->file->GetPaddedChecksum() == entry.checksum) {
            return record;
        }
    }
    return nullptr;
}

const AbcMetadataProvider::MethodCodeView *AbcMetadataProvider::GetOrCreateMethodCodeView(const FileRecord &record,
                                                                                          uint32_t methodIdx) const
{
    MethodKey key {record.file.get(), methodIdx};
    auto cacheIt = codeCache_.find(key);
    if (cacheIt != codeCache_.end()) {
        return &cacheIt->second;
    }
    MethodCodeView view;
    panda_file::File::EntityId methodId(methodIdx);
    if (methodId.GetOffset() >= record.file->GetHeader()->fileSize) {
        auto it = codeCache_.emplace(key, view);
        return &it.first->second;
    }
    auto *region = record.file->GetRegionHeader(methodId);
    if (region == nullptr) {
        auto it = codeCache_.emplace(key, view);
        return &it.first->second;
    }
    if (!ContainsMethod(*record.file, methodId)) {
        // Allow methods that are missing from the index if we can still read their metadata.
    }
    panda_file::MethodDataAccessor mda(*record.file, methodId);
    auto codeId = mda.GetCodeId();
    if (codeId.has_value()) {
        panda_file::CodeDataAccessor cda(*record.file, codeId.value());
        view.instructions = cda.GetInstructions();
        view.codeSize = cda.GetCodeSize();
    }
    auto it = codeCache_.emplace(key, view);
    return &it.first->second;
}

bool AbcMetadataProvider::AddFile(const fs::path &path)
{
    std::error_code ec;
    if (!fs::is_regular_file(path, ec)) {
        LOG_APTOOL(ERROR) << "cannot open abc '" << path.string() << "'";
        return false;
    }
    auto normalizedPath = NormalizePath(path);
    if (recordsByNormalized_.find(normalizedPath) != recordsByNormalized_.end()) {
        return true;
    }
    auto file = panda_file::OpenPandaFile(path.string());
    if (file == nullptr) {
        LOG_APTOOL(ERROR) << "failed to read abc '" << path.string() << "'";
        return false;
    }
    auto record = std::make_unique<FileRecord>();
    record->path = path;
    record->normalized = normalizedPath;
    record->baseName = path.filename().string();
    record->file = std::move(file);
    auto *raw = record.get();
    recordsByNormalized_.emplace(record->normalized, raw);
    recordsByBaseName_[record->baseName].push_back(raw);
    records_.push_back(std::move(record));
    return true;
}

std::vector<AbcMetadataProvider::ContextEntry> AbcMetadataProvider::ParseClassContext(const PandaString &ctx)
{
    std::vector<ContextEntry> entries;
    if (ctx.empty()) {
        return entries;
    }
    std::string_view ctxView(ctx.c_str(), ctx.size());
    size_t start = 0;
    while (start < ctxView.size()) {
        auto delim = ctxView.find(CLASS_CTX_ENTRY_DELIM, start);
        auto token = ctxView.substr(start, delim == std::string_view::npos ? ctxView.size() - start : delim - start);
        auto hash = token.rfind(CLASS_CTX_HASH_DELIM);
        if (hash != std::string_view::npos) {
            ContextEntry entry;
            entry.path = std::string(token.substr(0, hash));
            entry.checksum = std::string(token.substr(hash + 1));
            entries.push_back(std::move(entry));
        }
        if (delim == std::string_view::npos) {
            break;
        }
        start = delim + 1;
    }
    return entries;
}

std::optional<std::string> AbcMetadataProvider::GetClassDescriptorInternal(const panda_file::File &file,
                                                                           panda_file::File::EntityId classId)
{
    auto data = file.GetStringData(classId).data;
    if (data == nullptr) {
        return std::nullopt;
    }
    return std::string(utf::Mutf8AsCString(data));
}

std::string AbcMetadataProvider::BuildMethodSignature(const panda_file::File &file, panda_file::File::EntityId methodId)
{
    panda_file::MethodDataAccessor mda(file, methodId);
    auto protoId = panda_file::MethodDataAccessor::GetProtoId(file, methodId);
    panda_file::ProtoDataAccessor pda(file, protoId);
    PandasmTypePrinter printer(file, pda);
    // Skip the return type to advance refIdx_ past its reference slot (if any)
    printer.Dump(pda.GetReturnType());
    auto name = utf::Mutf8AsCString(file.GetStringData(mda.GetNameId()).data);

    // Build signature in pandasm style: methodName:(type1,type2,...)
    std::string signature(name);
    signature += ":(";

    for (uint32_t i = 0; i < pda.GetNumArgs(); i++) {
        if (i > 0) {
            signature += ",";
        }
        signature += printer.Dump(pda.GetArgType(i));
    }
    signature += ")";

    return signature;
}

bool AbcMetadataProvider::ContainsMethod(const panda_file::File &file, panda_file::File::EntityId methodId)
{
    auto *region = file.GetRegionHeader(methodId);
    if (region == nullptr) {
        return false;
    }
    auto index = file.GetMethodIndex(region);
    for (const auto &entry : index) {
        if (entry == methodId) {
            return true;
        }
    }
    return false;
}

bool AbcMetadataProvider::ContainsClass(const panda_file::File &file, panda_file::File::EntityId classId)
{
    auto *region = file.GetRegionHeader(classId);
    if (region == nullptr) {
        return false;
    }
    auto index = file.GetClassIndex(region);
    for (const auto &entry : index) {
        if (entry == classId) {
            return true;
        }
    }
    return false;
}

#undef LOG_APTOOL

}  // namespace ark::aptool::common
