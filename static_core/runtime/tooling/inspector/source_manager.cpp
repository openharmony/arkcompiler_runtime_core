/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "source_manager.h"
#include "types/numeric_id.h"

#include <string>
#include <string_view>

namespace panda::tooling::inspector {
std::pair<ScriptId, bool> SourceManager::GetScriptId(PtThread thread, std::string_view file_name)
{
    os::memory::LockHolder lock(mutex_);

    auto p = file_name_to_id_.emplace(std::string(file_name), file_name_to_id_.size());
    ScriptId id(p.first->second);

    bool is_new_for_thread = known_sources_[thread].insert(id).second;

    if (p.second) {
        std::string_view name {p.first->first};
        id_to_file_name_.emplace(id, name);
    }

    return {id, is_new_for_thread};
}

std::string_view SourceManager::GetSourceFileName(ScriptId id) const
{
    os::memory::LockHolder lock(mutex_);

    auto it = id_to_file_name_.find(id);
    if (it != id_to_file_name_.end()) {
        return it->second;
    }

    LOG(ERROR, DEBUGGER) << "No file with script id " << id;

    return {};
}

void SourceManager::RemoveThread(PtThread thread)
{
    os::memory::LockHolder lock(mutex_);
    known_sources_.erase(thread);
}
}  // namespace panda::tooling::inspector
