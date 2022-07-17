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

#include "aot_manager.h"
#include "os/filesystem.h"
#include "events/events.h"

namespace panda::compiler {
class RuntimeInterface;

Expected<bool, std::string> AotManager::AddFile(const std::string &file_name, RuntimeInterface *runtime,
                                                uint32_t gc_type, bool force)
{
    if (GetFile(file_name) != nullptr) {
        LOG(DEBUG, AOT) << "Trying to add already existing AOT file: '" << file_name << "'";
        return true;
    }
    auto aot_file = AotFile::Open(file_name, gc_type);
    if (!aot_file) {
        EVENT_AOT_MANAGER(file_name, events::AotManagerAction::OPEN_FAILED);
        return Unexpected("AotFile::Open failed: " + aot_file.Error());
    }
    if (runtime != nullptr) {
        aot_file.Value()->PatchTable(runtime);
        aot_file.Value()->InitializeGot(runtime);
    }

    LOG(DEBUG, AOT) << "AOT file '" << file_name << "' has been loaded, code=" << aot_file.Value()->GetCode()
                    << ", code_size=" << aot_file.Value()->GetCodeSize();
    LOG(DEBUG, AOT) << "  It contains the following panda files:";
    for (auto header : aot_file.Value()->FileHeaders()) {
        LOG(DEBUG, AOT) << "  " << aot_file.Value()->GetString(header.file_name_str);
    }
    aot_files_.push_back(std::move(aot_file.Value()));
    auto &aot_insert_file = aot_files_[aot_files_.size() - 1];
    for (auto header : aot_insert_file->FileHeaders()) {
        auto pf_name = aot_insert_file->GetString(header.file_name_str);
        auto file_header = aot_insert_file->FindPandaFile(pf_name);
        if (force) {
            files_map_[pf_name] = AotPandaFile(aot_insert_file.get(), file_header);
        } else {
            files_map_.emplace(std::make_pair(pf_name, AotPandaFile(aot_insert_file.get(), file_header)));
        }
    }
    EVENT_AOT_MANAGER(file_name, events::AotManagerAction::ADDED);
    return true;
}

const AotPandaFile *AotManager::FindPandaFile(const std::string &file_name)
{
    if (file_name.empty()) {
        return nullptr;
    }
    auto it = files_map_.find(file_name);
    if (it != files_map_.end()) {
        return &it->second;
    }
    return nullptr;
}

const AotFile *AotManager::GetFile(const std::string &file_name) const
{
    auto res = std::find_if(aot_files_.begin(), aot_files_.end(),
                            [&file_name](auto &file) { return file_name == file->GetFileName(); });
    return res == aot_files_.end() ? nullptr : (*res).get();
}

/* We need such kind of complex print because line length of some tool is limited by 4000 characters */
static void FancyClassContextPrint(std::string_view context)
{
    constexpr char DELIMITER = ':';
    size_t start = 0;
    size_t end = context.find(DELIMITER, start);
    while (end != std::string::npos) {
        LOG(ERROR, AOT) << "\t\t" << context.substr(start, end - start);
        start = end + 1;
        end = context.find(DELIMITER, start);
    }
    LOG(ERROR, AOT) << "\t\t" << context.substr(start);
}

static bool CheckFilesInClassContext(std::string_view context, std::string_view aot_context)
{
    constexpr char DELIMITER = ':';
    size_t start = 0;
    size_t end = aot_context.find(DELIMITER, start);
    while (end != std::string::npos) {
        auto file_context = aot_context.substr(start, end - start);
        if (context.find(file_context) == std::string::npos) {
            LOG(ERROR, AOT) << "Cannot found file " << file_context << " in runtime context";
            return false;
        }
        start = end + 1;
        end = aot_context.find(DELIMITER, start);
    }
    return true;
}

void AotManager::VerifyClassHierarchy(bool only_boot)
{
    auto complete_context = boot_class_context_;
    if (!only_boot && !app_class_context_.empty()) {
        if (!complete_context.empty()) {
            complete_context.append(":");
        }
        complete_context.append(app_class_context_);
    }
    auto verify_aot = [this, &complete_context](auto &aot_file) {
        auto context = aot_file->IsBootPandaFile() ? boot_class_context_ : complete_context;
        bool is_check = true;

        if (aot_file->IsCompiledWithCha()) {
            // Aot file context must be prefix of current runtime context
            if (context.rfind(aot_file->GetClassContext(), 0) != 0) {
                is_check = false;
                EVENT_AOT_MANAGER(aot_file->GetFileName(), events::AotManagerAction::CHA_VERIFY_FAILED);
            }
        } else {
            // Aot file context must be contained in current runtime context
            if (!CheckFilesInClassContext(context, aot_file->GetClassContext())) {
                is_check = false;
                EVENT_AOT_MANAGER(aot_file->GetFileName(), events::AotManagerAction::FILE_VERIFY_FAILED);
            }
        }
        if (!is_check) {
            auto boot_pref = aot_file->IsBootPandaFile() ? "boot " : "";
            LOG(ERROR, AOT) << "Cannot use " << boot_pref << "AOT file '" << aot_file->GetFileName() << '\'';
            LOG(ERROR, AOT) << "\tRuntime " << boot_pref << "class context: ";
            FancyClassContextPrint(context);
            LOG(ERROR, AOT) << "\tAOT class context: ";
            FancyClassContextPrint(aot_file->GetClassContext());
            LOG(FATAL, AOT) << "Aborting due to mismatched class hierarchy";
            return true;
        }
        EVENT_AOT_MANAGER(aot_file->GetFileName(), events::AotManagerAction::VERIFIED);
        return false;
    };

    for (auto &cur_aot_file : aot_files_) {
        verify_aot(cur_aot_file);
    }
}

void AotManager::RegisterAotStringRoot(ObjectHeader **slot, bool is_young)
{
    os::memory::LockHolder lock(aot_string_roots_lock_);
    aot_string_gc_roots_.push_back(slot);
    // Atomic with acquire order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after the
    // load which should become visible
    size_t roots_count = aot_string_gc_roots_count_.load(std::memory_order_acquire);
    if (aot_string_young_set_.size() <= roots_count / MASK_WIDTH) {
        aot_string_young_set_.push_back(0);
    }
    if (is_young) {
        has_young_aot_string_refs_ = true;
        aot_string_young_set_[roots_count / MASK_WIDTH] |= 1ULL << (roots_count % MASK_WIDTH);
    }
    // Atomic with acq_rel order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after the
    // load and on writes before the store
    aot_string_gc_roots_count_.fetch_add(1, std::memory_order_acq_rel);
}

bool AotClassContextCollector::operator()(const panda_file::File &pf)
{
    if (!acc_->empty()) {
        acc_->append(":");
    }
    acc_->append(os::GetAbsolutePath(pf.GetFilename()));
    acc_->append("*");
    acc_->append(std::to_string(pf.GetHeader()->checksum));
    return true;
}

}  // namespace panda::compiler
