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

#ifndef PANDA_VERIFIER_DEBUG_CONTEXT_H_
#define PANDA_VERIFIER_DEBUG_CONTEXT_H_

#include "verification/config/config.h"
#include "verification/config/debug_breakpoint/breakpoint.h"
#include "verification/config/whitelist/whitelist.h"
#include "verification/util/callable.h"
#include "verification/util/synchronized.h"

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace panda::verifier::debug {
struct DebugContext {
    struct {
        PandaUnorderedMap<PandaString, panda::verifier::callable<bool(const config::Section &)>> SectionHandlers;
    } Config;

#ifndef NDEBUG
    struct {
        // note: this is assumed to be small so stored as a vector (not even sorted, so we use a linear search)
        // if changed, a sorted vector or a PandaMap would likely be better than PandaUnorderedMap for faster comparison
        // accessed for writing only when parsing config, so doesn't need to be synchronized
        PandaVector<std::pair<PandaString, Offsets>> Config;
        Synchronized<PandaUnorderedMap<Method::UniqId, PandaUnorderedSet<uint32_t>>> Breakpoint;
    } ManagedBreakpoints;
#endif

    struct {
        // similar to ManagedBreakpoints.Config
        std::array<PandaVector<PandaString>, static_cast<size_t>(WhitelistKind::__LAST__)> Names;
        std::array<Synchronized<PandaUnorderedSet<Method::UniqId>>, static_cast<size_t>(WhitelistKind::__LAST__)> Id;
        bool isNotEmpty = false;
    } Whitelist;

    static DebugContext &GetCurrent();
    static void Destroy();

    void AddMethod(const Method &method, bool isDebug);

private:
    static DebugContext *instance;

#ifndef NDEBUG
    void InsertBreakpoints(const PandaString &method_name, Method::UniqId id);
#else
    void InsertBreakpoints([[maybe_unused]] const PandaString &method_name, [[maybe_unused]] Method::UniqId id) {}
#endif

    void InsertIntoWhitelist(const PandaString &name, bool isClassName, Method::UniqId id);
};
}  // namespace panda::verifier::debug

#endif  // !PANDA_VERIFIER_DEBUG_CONTEXT_H_
