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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_DEBUG_INFO_CACHE_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_DEBUG_INFO_CACHE_H

#include "plugins/ets/runtime/tooling/ets_debug_info_extractor.h"

#include "method.h"
#include "runtime/include/typed_value.h"

#include <unordered_map>

namespace ark::ets::tooling {
// TODO: consider removing this class
class DebugInfoCache final {
public:
    DebugInfoCache() = default;
    ~DebugInfoCache() = default;

    NO_COPY_SEMANTIC(DebugInfoCache);
    NO_MOVE_SEMANTIC(DebugInfoCache);

    // Returns a table with all local variable including function arguments.
    const panda_file::LocalVariableTable &GetLocalVariables(Frame *frame);

private:
    // Returns debug-info for the given panda file.
    // Causes creation of `EtsDebugInfoExtractor` if this panda file wasn't encountered before.
    const EtsDebugInfoExtractor &GetDebugInfo(const panda_file::File *file);

    const EtsDebugInfoExtractor &AddPandaFile(const panda_file::File *file) REQUIRES(debugInfosMutex_);

    os::memory::Mutex debugInfosMutex_;
    std::unordered_map<const panda_file::File *, EtsDebugInfoExtractor> debugInfos_ GUARDED_BY(debugInfosMutex_);
};
}  // namespace ark::ets::tooling

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_DEBUG_INFO_CACHE_H
