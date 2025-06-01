/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PROFILING_LOADER_H
#define PROFILING_LOADER_H

#include "jit/profiling_data.h"
#include "libprofile/aot_profiling_data.h"

namespace ark {
class PANDA_PUBLIC_API ProfilingLoader {
public:
    Expected<PandaString, PandaString> LoadProfile(const PandaString &fileName);

    pgo::AotProfilingData &GetAotProfilingData()
    {
        return aotProfilingData_;
    }

    template <typename ClassResolverT>
    ProfilingData *CreateProfilingData(const pgo::AotProfilingData::AotMethodProfilingData &methodProfile,
                                       mem::InternalAllocatorPtr allocator, ClassResolverT &&classResolver)
    {
        auto inlineCaches = methodProfile.GetInlineCaches();
        auto branches = methodProfile.GetBranchData();
        auto throws = methodProfile.GetThrowData();
        return ProfilingData::Make(allocator, inlineCaches.size(), branches.size(), throws.size(),
                                   [&](void *data, void *vcallsMem, void *branchesMem, void *throwsMem) {
                                       return new (data) ProfilingData(
                                           CreateInlineCaches(vcallsMem, inlineCaches, classResolver),
                                           CreateBranchData(branchesMem, branches), CreateThrowData(throwsMem, throws));
                                   });  // CC-OFF(G.FMT.02) project code style
    }

private:
    template <typename ClassResolverT>
    Span<CallSiteInlineCache> CreateInlineCaches(
        void *inlineCachesMem, Span<const pgo::AotProfilingData::AotCallSiteInlineCache> aotInlineCaches,
        ClassResolverT &&classResolver)
    {
        auto inlineCaches = Span(reinterpret_cast<CallSiteInlineCache *>(inlineCachesMem), aotInlineCaches.size());
        for (size_t i = 0; i < aotInlineCaches.size(); i++) {
            inlineCaches[i].Init(aotInlineCaches[i].pc);
            for (auto &cls : aotInlineCaches[i].classes) {
                if (auto *klass = classResolver(cls.first, cls.second)) {
                    inlineCaches[i].UpdateInlineCaches(klass);
                }
            }
        }

        return inlineCaches;
    }

    Span<BranchData> CreateBranchData(void *branchesMem,
                                      Span<const pgo::AotProfilingData::AotBranchData> aotBranchData);
    Span<ThrowData> CreateThrowData(void *throwsMem, Span<const pgo::AotProfilingData::AotThrowData> aotThrowData);

    PandaVector<PandaString> pandaFiles_;
    pgo::AotProfilingData aotProfilingData_;
};
}  // namespace ark
#endif  // PROFILING_LOADER_H
