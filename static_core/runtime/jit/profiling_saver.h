/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PROFILING_SAVE_H
#define PROFILING_SAVE_H

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "libprofile/aot_profiling_data.h"
#include "runtime/include/class.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/jit/libprofile/pgo_file_builder.h"
#include "runtime/jit/profiling_data.h"

namespace ark {
class ProfilingSaver {
public:
    struct PendingMethodLastSaved {
        PandaVector<ProfilingData::BranchLastSaved> branches;
        PandaVector<uint64_t> throws;
    };
    using PendingLastSavedMap = std::unordered_map<Method *, PendingMethodLastSaved>;

    void AddMethod(pgo::AotProfilingData *profileData, Method *method, int32_t pandaFileIdx,
                   PendingLastSavedMap &pendingLastSaved, bool applyLastSaved);

    // CC-OFFNXT(G.FUN.01-CPP) Save path keeps fast-path and merge/fallback state transitions in one place for clarity.
    // NOLINTNEXTLINE(readability-function-size)
    void SaveProfile(const PandaString &saveFilePath, const PandaString &classCtxStr,
                     PandaList<Method *> &profiledMethods, PandaList<Method *>::const_iterator profiledMethodsFinal,
                     PandaUnorderedSet<std::string> &profiledPandaFiles,
                     const PandaUnorderedSet<std::string> &saveablePandaFiles);

private:
    void UpdateInlineCaches(pgo::AotProfilingData::AotCallSiteInlineCache *ic, std::vector<Class *> &runtimeClasses,
                            pgo::AotProfilingData *profileData);
    void CreateInlineCaches(pgo::AotProfilingData::AotMethodProfilingData *profilingData,
                            Span<CallSiteInlineCache> &runtimeICs, pgo::AotProfilingData *profileData);
    void CreateBranchData(pgo::AotProfilingData::AotMethodProfilingData *profilingData, Span<BranchData> &runtimeBranch,
                          PendingMethodLastSaved &currentLastSaved, const ProfilingData &runtimeProfData,
                          bool applyLastSaved);
    void CreateThrowData(pgo::AotProfilingData::AotMethodProfilingData *profilingData, Span<ThrowData> &runtimeThrow,
                         PendingMethodLastSaved &currentLastSaved, const ProfilingData &runtimeProfData,
                         bool applyLastSaved);
    // CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) grouping related save-state inputs keeps API clear.
    // NOLINTNEXTLINE(readability-function-size)
    uint32_t AddProfiledMethods(pgo::AotProfilingData *profileData, PandaList<Method *> &profiledMethods,
                                PandaList<Method *>::const_iterator profiledMethodsFinal,
                                const PandaUnorderedSet<std::string> &saveablePandaFiles,
                                PendingLastSavedMap &pendingLastSaved, bool applyLastSaved);
};
}  // namespace ark
#endif  // PROFILING_SAVE_H
