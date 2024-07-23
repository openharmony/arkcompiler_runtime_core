/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::compiler {
class RuntimeInterface;

Expected<bool, std::string> AotManager::AddFile(const std::string &fileName, RuntimeInterface *runtime, uint32_t gcType,
                                                bool force)
{
    if (GetFile(fileName) != nullptr) {
        LOG(DEBUG, AOT) << "Trying to add already existing AOT file: '" << fileName << "'";
        return true;
    }
    auto aotFile = AotFile::Open(fileName, gcType);
    if (!aotFile) {
        EVENT_AOT_MANAGER(fileName, events::AotManagerAction::OPEN_FAILED);
        return Unexpected("AotFile::Open failed: " + aotFile.Error());
    }
    if (runtime != nullptr) {
        aotFile.Value()->PatchTable(runtime);
        aotFile.Value()->InitializeGot(runtime);
    }

    LOG(DEBUG, AOT) << "AOT file '" << fileName << "' has been loaded, code=" << aotFile.Value()->GetCode()
                    << ", code_size=" << aotFile.Value()->GetCodeSize();
    LOG(DEBUG, AOT) << "  It contains the following panda files:";
    for (auto header : aotFile.Value()->FileHeaders()) {
        LOG(DEBUG, AOT) << "  " << aotFile.Value()->GetString(header.fileNameStr);
    }
    aotFiles_.push_back(std::move(aotFile.Value()));
    auto &aotInsertFile = aotFiles_[aotFiles_.size() - 1];
    for (auto header : aotInsertFile->FileHeaders()) {
        auto pfName = aotInsertFile->GetString(header.fileNameStr);
        auto fileHeader = aotInsertFile->FindPandaFile(pfName);
        if (force) {
            filesMap_[pfName] = AotPandaFile(aotInsertFile.get(), fileHeader);
        } else {
            filesMap_.emplace(std::make_pair(pfName, AotPandaFile(aotInsertFile.get(), fileHeader)));
        }
    }
    EVENT_AOT_MANAGER(fileName, events::AotManagerAction::ADDED);
    return true;
}

const AotPandaFile *AotManager::FindPandaFile(const std::string &fileName)
{
    if (fileName.empty()) {
        return nullptr;
    }
    auto it = filesMap_.find(fileName);
    if (it != filesMap_.end()) {
        return &it->second;
    }
    return nullptr;
}

const AotFile *AotManager::GetFile(const std::string &fileName) const
{
    auto res = std::find_if(aotFiles_.begin(), aotFiles_.end(),
                            [&fileName](auto &file) { return fileName == file->GetFileName(); });
    return res == aotFiles_.end() ? nullptr : (*res).get();
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

static bool CheckFilesInClassContext(std::string_view context, std::string_view aotContext)
{
    constexpr char DELIMITER = ':';
    size_t start = 0;
    size_t end = aotContext.find(DELIMITER, start);
    while (end != std::string::npos) {
        auto fileContext = aotContext.substr(start, end - start);
        if (context.find(fileContext) == std::string::npos) {
            LOG(ERROR, AOT) << "Cannot found file " << fileContext << " in runtime context";
            return false;
        }
        start = end + 1;
        end = aotContext.find(DELIMITER, start);
    }
    return true;
}

void AotManager::VerifyClassHierarchy()
{
    auto completeContext = bootClassContext_;
    if (!appClassContext_.empty()) {
        if (!completeContext.empty()) {
            completeContext.append(":");
        }
        completeContext.append(appClassContext_);
    }
    auto verifyAot = [this, &completeContext](auto &aotFile) {
        auto context = aotFile->IsBootPandaFile() ? bootClassContext_ : completeContext;
        bool isCheck = true;

        if (aotFile->IsCompiledWithCha()) {
            // Aot file context must be prefix of current runtime context
            if (context.rfind(aotFile->GetClassContext(), 0) != 0) {
                isCheck = false;
                EVENT_AOT_MANAGER(aotFile->GetFileName(), events::AotManagerAction::CHA_VERIFY_FAILED);
            }
        } else {
            // Aot file context must be contained in current runtime context
            if (!CheckFilesInClassContext(context, aotFile->GetClassContext())) {
                isCheck = false;
                EVENT_AOT_MANAGER(aotFile->GetFileName(), events::AotManagerAction::FILE_VERIFY_FAILED);
            }
        }
        if (!isCheck) {
            auto bootPref = aotFile->IsBootPandaFile() ? "boot " : "";
            LOG(ERROR, AOT) << "Cannot use " << bootPref << "AOT file '" << aotFile->GetFileName() << '\'';
            LOG(ERROR, AOT) << "\tRuntime " << bootPref << "class context: ";
            FancyClassContextPrint(context);
            LOG(ERROR, AOT) << "\tAOT class context: ";
            FancyClassContextPrint(aotFile->GetClassContext());
            LOG(FATAL, AOT) << "Aborting due to mismatched class hierarchy";
            return true;
        }
        EVENT_AOT_MANAGER(aotFile->GetFileName(), events::AotManagerAction::VERIFIED);
        return false;
    };

    for (auto &curAotFile : aotFiles_) {
        verifyAot(curAotFile);
    }
}

void AotManager::RegisterAotStringRoot(ObjectHeader **slot, bool isYoung)
{
    os::memory::LockHolder lock(aotStringRootsLock_);
    aotStringGcRoots_.push_back(slot);
    // Atomic with acquire order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after the
    // load which should become visible
    size_t rootsCount = aotStringGcRootsCount_.load(std::memory_order_acquire);
    if (aotStringYoungSet_.size() <= rootsCount / MASK_WIDTH) {
        aotStringYoungSet_.push_back(0);
    }
    if (isYoung) {
        hasYoungAotStringRefs_ = true;
        aotStringYoungSet_[rootsCount / MASK_WIDTH] |= 1ULL << (rootsCount % MASK_WIDTH);
    }
    // Atomic with acq_rel order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after the
    // load and on writes before the store
    aotStringGcRootsCount_.fetch_add(1, std::memory_order_acq_rel);
}

bool AotClassContextCollector::operator()(const panda_file::File &pf)
{
    if (!acc_->empty()) {
        acc_->append(":");
    }
    if (useAbsPath_) {
        acc_->append(os::GetAbsolutePath(pf.GetFilename()));
    } else {
        acc_->append(pf.GetFilename());
    }
    acc_->append("*");
    acc_->append(pf.GetPaddedChecksum());
    return true;
}

}  // namespace ark::compiler
