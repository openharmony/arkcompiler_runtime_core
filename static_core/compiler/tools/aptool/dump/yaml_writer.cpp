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

#include <optional>

#include "compiler/tools/aptool/common/utils.h"
#include "compiler/tools/aptool/dump/yaml_writer.h"

namespace ark::aptool::dump {

using common::FormatHex;
using common::ProfilingData;
using common::QuoteString;

void YamlWriter::WriteLine(const std::string &line)
{
    for (uint32_t i = 0; i < indent_; i++) {
        out_ << "  ";
    }
    out_ << line << '\n';
}

void YamlWriter::IncreaseIndent()
{
    indent_++;
}

void YamlWriter::DecreaseIndent()
{
    ASSERT(indent_ > 0);
    indent_--;
}

namespace {

void WriteBytecode(YamlWriter &writer, const AbcMetadataProvider *metadata, const PandaString *pandaFileName,
                   uint32_t methodIdx, uint32_t pc)
{
    if (metadata == nullptr || pandaFileName == nullptr) {
        return;
    }
    auto instruction = metadata->GetInstructionString(*pandaFileName, methodIdx, pc);
    if (instruction) {
        writer.WriteLine("bytecode: " + QuoteString(*instruction));
    }
}

void WriteInlineCacheTarget(YamlWriter &writer, const std::pair<uint32_t, ark::pgo::PandaFileIdxType> &target,
                            const PandaVector<PandaString> &pandaFiles, const AbcMetadataProvider *metadata)
{
    using InlineCache = ark::pgo::AotProfilingData::AotCallSiteInlineCache;
    if (target.first == static_cast<uint32_t>(InlineCache::MEGAMORPHIC_FLAG)) {
        writer.WriteLine("- megamorphic: true");
        return;
    }
    if (target.second < 0) {
        writer.WriteLine("- unused: true");
        return;
    }
    writer.WriteLine("- class_index: " + std::to_string(target.first));
    writer.IncreaseIndent();
    writer.WriteLine("panda_file_index: " + std::to_string(target.second));
    if (metadata != nullptr) {
        auto pfIdx = static_cast<size_t>(target.second);
        if (pfIdx < pandaFiles.size()) {
            auto descriptor = metadata->GetClassDescriptor(pandaFiles[pfIdx], target.first);
            if (descriptor && !descriptor->empty()) {
                writer.WriteLine("class_descriptor: " + QuoteString(*descriptor));
            }
        }
    }
    writer.DecreaseIndent();
}

// NOLINTNEXTLINE(readability-function-size)
void WriteInlineCaches(YamlWriter &writer, Span<const ark::pgo::AotProfilingData::AotCallSiteInlineCache> caches,
                       const PandaVector<PandaString> &pandaFiles, const AbcMetadataProvider *metadata,
                       const PandaString *methodPandaFileName, uint32_t methodIdx)
{
    if (caches.Empty()) {
        writer.WriteLine("inline_caches: []");
        return;
    }
    writer.WriteLine("inline_caches:");
    writer.IncreaseIndent();
    for (const auto &cache : caches) {
        writer.WriteLine("- pc: " + FormatHex(cache.pc));
        writer.IncreaseIndent();
        WriteBytecode(writer, metadata, methodPandaFileName, methodIdx, cache.pc);
        writer.WriteLine("targets:");
        writer.IncreaseIndent();
        for (const auto &target : cache.classes) {
            WriteInlineCacheTarget(writer, target, pandaFiles, metadata);
        }
        writer.DecreaseIndent();
        writer.DecreaseIndent();
    }
    writer.DecreaseIndent();
}

void WriteBranches(YamlWriter &writer, Span<const ark::pgo::AotProfilingData::AotBranchData> branches,
                   const AbcMetadataProvider *metadata, const PandaString *methodPandaFileName, uint32_t methodIdx)
{
    if (branches.Empty()) {
        writer.WriteLine("branches: []");
        return;
    }
    writer.WriteLine("branches:");
    writer.IncreaseIndent();
    writer.WriteLine("entries:");
    writer.IncreaseIndent();
    for (const auto &branch : branches) {
        writer.WriteLine("- pc: " + FormatHex(branch.pc));
        writer.IncreaseIndent();
        WriteBytecode(writer, metadata, methodPandaFileName, methodIdx, branch.pc);
        writer.WriteLine("takenCount: " + std::to_string(branch.taken));
        writer.WriteLine("notTakenCount: " + std::to_string(branch.notTaken));
        writer.DecreaseIndent();
    }
    writer.DecreaseIndent();
    writer.DecreaseIndent();
}

void WriteThrows(YamlWriter &writer, Span<const ark::pgo::AotProfilingData::AotThrowData> throwsData,
                 const AbcMetadataProvider *metadata, const PandaString *methodPandaFileName, uint32_t methodIdx)
{
    if (throwsData.Empty()) {
        writer.WriteLine("throws: []");
        return;
    }
    writer.WriteLine("throws:");
    writer.IncreaseIndent();
    writer.WriteLine("entries:");
    writer.IncreaseIndent();
    for (const auto &entry : throwsData) {
        writer.WriteLine("- pc: " + FormatHex(entry.pc));
        writer.IncreaseIndent();
        WriteBytecode(writer, metadata, methodPandaFileName, methodIdx, entry.pc);
        writer.WriteLine("takenCount: " + std::to_string(entry.taken));
        writer.DecreaseIndent();
    }
    writer.DecreaseIndent();
    writer.DecreaseIndent();
}

void WritePandaFilesSection(YamlWriter &writer, const ProfileData &profile, const DumpFilterOptions &filters)
{
    const auto &files = profile.GetPandaFiles();
    PandaVector<uint32_t> selected;
    selected.reserve(files.size());
    for (uint32_t idx = 0; idx < files.size(); idx++) {
        if (PandaFileMatches(&files[idx], filters)) {
            selected.push_back(idx);
        }
    }
    if (selected.empty()) {
        writer.WriteLine("PandaFiles: []");
        return;
    }
    writer.WriteLine("PandaFiles:");
    writer.IncreaseIndent();
    for (auto idx : selected) {
        writer.WriteLine("- index: " + std::to_string(idx));
        writer.IncreaseIndent();
        writer.WriteLine("file_name: " + QuoteString(files[idx].c_str()));
        writer.DecreaseIndent();
    }
    writer.DecreaseIndent();
}

// NOLINTNEXTLINE(readability-function-size)
void WriteMethodsSection(YamlWriter &writer, const ProfileData &profile, const AbcMetadataProvider *metadata,
                         const DumpFilterOptions &filters)
{
    const auto &allMethods = profile.GetProfilingData().GetAllMethods();
    const auto &pandaFiles = profile.GetPandaFiles();
    if (allMethods.empty()) {
        writer.WriteLine("Methods: []");
        return;
    }
    writer.WriteLine("Methods:");
    writer.IncreaseIndent();
    for (const auto &pfEntry : allMethods) {
        const PandaString *pandaFileName = nullptr;
        if (pfEntry.first >= 0) {
            auto idx = static_cast<size_t>(pfEntry.first);
            if (idx < pandaFiles.size()) {
                pandaFileName = &pandaFiles[idx];
            }
        }
        if (!PandaFileMatches(pandaFileName, filters)) {
            continue;
        }
        writer.WriteLine("- panda_file_index: " + std::to_string(pfEntry.first));
        writer.IncreaseIndent();
        struct FilteredMethod {
            const ark::pgo::AotProfilingData::AotMethodProfilingData *data;
            std::optional<std::string> classDescriptor;
            std::optional<std::string> methodSignature;
        };
        PandaVector<FilteredMethod> filteredMethods;
        filteredMethods.reserve(pfEntry.second.size());
        for (const auto &methodPair : pfEntry.second) {
            const auto &methodData = methodPair.second;
            std::optional<std::string> classDescriptor;
            std::optional<std::string> methodSignature;
            if (metadata != nullptr && pandaFileName != nullptr) {
                auto methodInfo = metadata->GetMethodInfo(*pandaFileName, methodData.GetMethodIdx());
                if (methodInfo) {
                    if (!methodInfo->classDescriptor.empty()) {
                        classDescriptor = methodInfo->classDescriptor;
                    }
                    if (!methodInfo->methodSignature.empty()) {
                        methodSignature = methodInfo->methodSignature;
                    }
                } else {
                    auto classInfo = metadata->GetClassDescriptor(*pandaFileName, methodData.GetClassIdx());
                    if (classInfo && !classInfo->empty()) {
                        classDescriptor = classInfo;
                    }
                }
            }
            if (!ClassMatches(classDescriptor, filters) || !MethodMatches(methodSignature, filters)) {
                continue;
            }
            filteredMethods.push_back({&methodData, std::move(classDescriptor), std::move(methodSignature)});
        }
        if (filteredMethods.empty()) {
            writer.WriteLine("entries: []");
        } else {
            writer.WriteLine("entries:");
            writer.IncreaseIndent();
            for (const auto &filtered : filteredMethods) {
                writer.WriteLine("- method_index: " + std::to_string(filtered.data->GetMethodIdx()));
                writer.IncreaseIndent();
                writer.WriteLine("class_index: " + std::to_string(filtered.data->GetClassIdx()));
                if (filtered.classDescriptor) {
                    writer.WriteLine("class_descriptor: " + QuoteString(*filtered.classDescriptor));
                }
                if (filtered.methodSignature) {
                    writer.WriteLine("method_signature: " + QuoteString(*filtered.methodSignature));
                }
                WriteInlineCaches(writer, filtered.data->GetInlineCaches(), pandaFiles, metadata, pandaFileName,
                                  filtered.data->GetMethodIdx());
                WriteBranches(writer, filtered.data->GetBranchData(), metadata, pandaFileName,
                              filtered.data->GetMethodIdx());
                WriteThrows(writer, filtered.data->GetThrowData(), metadata, pandaFileName,
                            filtered.data->GetMethodIdx());
                writer.DecreaseIndent();
            }
            writer.DecreaseIndent();
        }
        writer.DecreaseIndent();
    }
    writer.DecreaseIndent();
}

}  // namespace

void DumpProfileToYaml(std::ostream &out, const ProfileData &profile, const AbcMetadataProvider *metadata,
                       const DumpFilterOptions &filters)
{
    YamlWriter writer(out);
    if (!profile.GetClassContext().empty()) {
        writer.WriteLine("ClassCtxStr: " + QuoteString(profile.GetClassContext().c_str()));
    }
    WritePandaFilesSection(writer, profile, filters);
    WriteMethodsSection(writer, profile, metadata, filters);
}

}  // namespace ark::aptool::dump
