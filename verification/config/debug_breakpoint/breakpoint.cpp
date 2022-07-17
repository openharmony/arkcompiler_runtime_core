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

#ifndef NDEBUG

#include "breakpoint_private.h"

#include "verification/config/context/context.h"
#include "verification/util/optional_ref.h"
#include "verification/util/str.h"

#include "verifier_messages.h"

#include "utils/logger.h"

namespace panda::verifier::debug {

template <typename BreakpointConfigT>
OptionalRef<Offsets> BreakpointsForName(const BreakpointConfigT &breakpoint_config, const PandaString &method_name)
{
    auto iter = std::find_if(breakpoint_config.begin(), breakpoint_config.end(),
                             [&](const auto &pair) { return pair.first == method_name; });
    if (iter == breakpoint_config.end()) {
        return {};
    }
    return iter->second;
}

void AddBreakpointConfig(const PandaString &method_name, Offset offset)
{
    auto &breakpoint_config = DebugContext::GetCurrent().ManagedBreakpoints.Config;
    auto opt_breakpoints = BreakpointsForName(breakpoint_config, method_name);
    if (opt_breakpoints.HasRef()) {
        opt_breakpoints->push_back(offset);
    } else {
        breakpoint_config.push_back({method_name, Offsets {offset}});
    }
    LOG_VERIFIER_DEBUG_BREAKPOINT_ADDED_INFO(method_name, offset);
}

void DebugContext::InsertBreakpoints(const PandaString &method_name, Method::UniqId id)
{
    auto opt_breakpoints = BreakpointsForName(ManagedBreakpoints.Config, method_name);
    if (opt_breakpoints.HasRef()) {
        for (const auto offset : opt_breakpoints.Get()) {
            LOG_VERIFIER_DEBUG_BREAKPOINT_SET_INFO(method_name, id, offset);
            ManagedBreakpoints.Breakpoint.Apply([&](auto &breakpoint_map) { breakpoint_map[id].insert(offset); });
        }
    }
}

bool CheckManagedBreakpoint(Method::UniqId id, Offset offset)
{
    const auto &breakpoints = DebugContext::GetCurrent().ManagedBreakpoints.Breakpoint;
    return breakpoints.Apply([&](const auto &breakpoint_map) {
        auto iter = breakpoint_map.find(id);
        if (iter == breakpoint_map.end()) {
            return false;
        }
        return iter->second.count(offset) > 0;
    });
}

bool ManagedBreakpointPresent(Method::UniqId id)
{
    return DebugContext::GetCurrent().ManagedBreakpoints.Breakpoint->count(id) > 0;
}

}  // namespace panda::verifier::debug

#endif
