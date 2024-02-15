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

#include "debug_info_cache.h"

#include "debug_info_extractor.h"

namespace ark::ets::tooling {
const panda_file::LocalVariableTable &DebugInfoCache::GetLocalVariables(Frame *frame)
{
    ASSERT(frame);

    const auto &debugInfo = GetDebugInfo(method->GetPandaFile());
    return debugInfo.GetLocalsWithArguments(frame);
}

const EtsDebugInfoExtractor &DebugInfoCache::GetDebugInfo(const panda_file::File *file)
{
    os::memory::LockHolder lock(debugInfosMutex_);
    auto it = debugInfos_.find(file);
    if (it != debugInfos_.end()) {
        return it->second;
    }
    return AddPandaFile(file);
}

const EtsDebugInfoExtractor &DebugInfoCache::AddPandaFile(const panda_file::File *file)
{
    ASSERT(file);
    return debugInfos_.emplace(std::piecewise_construct, std::forward_as_tuple(file), std::forward_as_tuple(*file))
        .first->second;
}
}  // namespace ark::ets::tooling
