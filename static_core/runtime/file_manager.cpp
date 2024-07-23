/**
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

#include "runtime/include/file_manager.h"
#include "runtime/include/runtime.h"
#include "libpandabase/os/filesystem.h"

namespace ark {

bool FileManager::LoadAbcFile(std::string_view location, panda_file::File::OpenMode openMode)
{
    auto pf = panda_file::OpenPandaFile(location, "", openMode);
    if (pf == nullptr) {
        LOG(ERROR, PANDAFILE) << "Load panda file failed: " << location;
        return false;
    }
    auto runtime = Runtime::GetCurrent();
    if (Runtime::GetOptions().IsEnableAn() && !Runtime::GetOptions().IsArkAot()) {
        TryLoadAnFileForLocation(location);
        auto aotFile = runtime->GetClassLinker()->GetAotManager()->FindPandaFile(std::string(location));
        if (aotFile != nullptr) {
            pf->SetClassHashTable(aotFile->GetClassHashTable());
        }
    }
    runtime->GetClassLinker()->AddPandaFile(std::move(pf));
    return true;
}

bool FileManager::TryLoadAnFileForLocation(std::string_view pandaFileLocation)
{
    auto anLocation = FileManager::ResolveAnFilePath(pandaFileLocation);
    if (anLocation.empty()) {
        return true;
    }
    auto res = FileManager::LoadAnFile(anLocation, false);
    if (res && res.Value()) {
        LOG(INFO, PANDAFILE) << "Successfully load .an file for '" << pandaFileLocation << "': '" << anLocation << "'";
    } else if (!res) {
        LOG(INFO, PANDAFILE) << "Failed to load AOT file: '" << anLocation << "': " << res.Error();
    } else {
        LOG(INFO, PANDAFILE) << "Failed to load '" << anLocation << "' with unknown reason";
    }
    return true;
}

Expected<bool, std::string> FileManager::LoadAnFile(std::string_view anLocation, bool force)
{
    PandaRuntimeInterface runtimeIface;
    auto runtime = Runtime::GetCurrent();
    auto gcType = Runtime::GetGCType(Runtime::GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
    ASSERT(gcType != ark::mem::GCType::INVALID_GC);
    auto realAnFilePath = os::GetAbsolutePath(anLocation);
    return runtime->GetClassLinker()->GetAotManager()->AddFile(realAnFilePath, &runtimeIface,
                                                               static_cast<uint32_t>(gcType), force);
}

PandaString FileManager::ResolveAnFilePath(std::string_view abcPath)
{
    // check whether an aot version of this file already exist
    // NOTE(Wentao):
    //   1. search ark native file file base on ARCH info from runtime.
    //   2. allow searching an file out of same path of ark bytecode file.
    const PandaString &anPathSuffix = ".an";
    PandaString::size_type posStart = abcPath.find_last_of('/');
    PandaString::size_type posEnd = abcPath.find_last_of('.');
    if (posStart != std::string_view::npos && posEnd != std::string_view::npos) {
        LOG(DEBUG, PANDAFILE) << "current abc file path: " << abcPath;
        PandaString abcPathPrefix = PandaString(abcPath.substr(0, posStart));
        PandaString anFilePath =
            abcPathPrefix + PandaString(abcPath.substr(posStart, posEnd - posStart)) + anPathSuffix;

        const char *filename = anFilePath.c_str();
        if (access(filename, F_OK) == 0) {
            return anFilePath;
        }
    }
    LOG(DEBUG, PANDAFILE) << "There is no corresponding .an file for " << abcPath;
    return "";
}

}  // namespace ark
