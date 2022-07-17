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

#include "runtime/include/file_manager.h"
#include "runtime/include/runtime.h"
#include "libpandabase/os/filesystem.h"

namespace panda {

bool FileManager::LoadAbcFile(std::string_view location, panda_file::File::OpenMode open_mode)
{
    auto pf = panda_file::OpenPandaFile(location, "", open_mode);
    if (pf == nullptr) {
        LOG(ERROR, PANDAFILE) << "Load panda file failed: " << location;
        return false;
    }
    auto runtime = Runtime::GetCurrent();
    if (Runtime::GetOptions().IsEnableAn()) {
        TryLoadAnFileForLocation(location);
        auto aot_file = runtime->GetClassLinker()->GetAotManager()->FindPandaFile(std::string(location));
        if (aot_file != nullptr) {
            pf->SetClassHashTable(aot_file->GetClassHashTable());
        }
    }
    runtime->GetClassLinker()->AddPandaFile(std::move(pf));
    return true;
}

bool FileManager::TryLoadAnFileForLocation(std::string_view panda_file_location)
{
    auto an_location = FileManager::ResolveAnFilePath(panda_file_location);
    if (an_location.empty()) {
        return true;
    }
    auto res = FileManager::LoadAnFile(an_location, false);
    if (res && res.Value()) {
        LOG(INFO, PANDAFILE) << "Successfully load .an file for '" << panda_file_location << "': '" << an_location
                             << "'";
    } else if (!res) {
        LOG(INFO, PANDAFILE) << "Failed to load AOT file: '" << an_location << "': " << res.Error();
    } else {
        LOG(INFO, PANDAFILE) << "Failed to load '" << an_location << "' with unknown reason";
    }
    return true;
}

Expected<bool, std::string> FileManager::LoadAnFile(std::string_view an_location, bool force)
{
    PandaRuntimeInterface runtime_iface;
    auto runtime = Runtime::GetCurrent();
    auto gc_type = Runtime::GetGCType(Runtime::GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
    ASSERT(gc_type != panda::mem::GCType::INVALID_GC);
    auto real_an_file_path = os::GetAbsolutePath(an_location);
    return runtime->GetClassLinker()->GetAotManager()->AddFile(real_an_file_path, &runtime_iface,
                                                               static_cast<uint32_t>(gc_type), force);
}

PandaString FileManager::ResolveAnFilePath(std::string_view abc_path)
{
    // check whether an aot version of this file already exist
    // Todo(Wentao):
    //   1. search ark native file file base on ARCH info from runtime.
    //   2. allow searching an file out of same path of ark bytecode file.
    const PandaString &an_path_suffix = ".an";
    PandaString::size_type pos_start = abc_path.find_last_of('/');
    PandaString::size_type pos_end = abc_path.find_last_of('.');
    if (pos_start != std::string_view::npos && pos_end != std::string_view::npos) {
        LOG(DEBUG, PANDAFILE) << "current abc file path: " << abc_path;
        PandaString abc_path_prefix = PandaString(abc_path.substr(0, pos_start));
        PandaString an_file_path =
            abc_path_prefix + PandaString(abc_path.substr(pos_start, pos_end - pos_start)) + an_path_suffix;

        const char *filename = an_file_path.c_str();
        if (access(filename, F_OK) == 0) {
            return an_file_path;
        }
    }
    LOG(DEBUG, PANDAFILE) << "There is no corresponding .an file for " << abc_path;
    return "";
}

}  // namespace panda
