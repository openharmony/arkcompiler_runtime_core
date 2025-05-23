/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "thread_state.h"

#include <sstream>

#include "include/tooling/pt_location.h"
#include "macros.h"

#include "error.h"
#include "types/numeric_id.h"

namespace ark::tooling::inspector {
std::vector<BreakpointId> ThreadState::GetBreakpointsByLocation(const PtLocation &location) const
{
    std::vector<BreakpointId> hitBreakpoints;

    auto range = breakpointLocations_.equal_range(location);
    std::transform(range.first, range.second, std::back_inserter(hitBreakpoints), [](auto &p) { return p.second; });

    return hitBreakpoints;
}

void ThreadState::Reset()
{
    if (stepKind_ != StepKind::BREAK_ON_START) {
        stepKind_ = StepKind::NONE;
    }
    stepLocations_.clear();
    methodEntered_ = false;
    breakpointsActive_ = true;
    skipAllPauses_ = false;
    mixedDebugEnabled_ = false;
    nextBreakpointId_ = 0;
    breakpointLocations_.clear();
    breakpointConditions_.Clear();
    pauseOnExceptionsState_ = PauseOnExceptionsState::NONE;
}

void ThreadState::BreakOnStart()
{
    if (!paused_) {
        stepKind_ = StepKind::BREAK_ON_START;
    }
}

void ThreadState::Continue()
{
    stepKind_ = StepKind::NONE;
    paused_ = false;
}

void ThreadState::ContinueTo(std::unordered_set<PtLocation, HashLocation> locations)
{
    stepKind_ = StepKind::CONTINUE_TO;
    stepLocations_ = std::move(locations);
    paused_ = false;
}

void ThreadState::StepInto(std::unordered_set<PtLocation, HashLocation> locations)
{
    stepKind_ = StepKind::STEP_INTO;
    methodEntered_ = false;
    stepLocations_ = std::move(locations);
    paused_ = false;
}

void ThreadState::StepOver(std::unordered_set<PtLocation, HashLocation> locations)
{
    stepKind_ = StepKind::STEP_OVER;
    methodEntered_ = false;
    stepLocations_ = std::move(locations);
    paused_ = false;
}

void ThreadState::StepOut()
{
    stepKind_ = StepKind::STEP_OUT;
    methodEntered_ = true;
    paused_ = false;
}

void ThreadState::Pause()
{
    if (!paused_ && !skipAllPauses_) {
        stepKind_ = StepKind::PAUSE;
    }
}

void ThreadState::SetBreakpointsActive(bool active)
{
    breakpointsActive_ = active;
}

void ThreadState::SetSkipAllPauses(bool skip)
{
    skipAllPauses_ = skip;
}

// NOTE(fangting, #25108): implement "NativeOut" events when in mixed debug mode
void ThreadState::SetMixedDebugEnabled(bool mixedDebugEnabled)
{
    mixedDebugEnabled_ = mixedDebugEnabled;
}

BreakpointId ThreadState::SetBreakpoint(const std::vector<PtLocation> &locations, const std::string *condition)
{
    ASSERT(!locations.empty());

    auto id = nextBreakpointId_++;
    for (auto &location : locations) {
        breakpointLocations_.emplace(location, id);
    }

    if (condition != nullptr) {
        ASSERT(locations.size() == 1);
        breakpointConditions_.AddCondition(id, *condition);
    }

    return id;
}

void ThreadState::RemoveBreakpoint(BreakpointId id)
{
    for (auto it = breakpointLocations_.begin(); it != breakpointLocations_.end();) {
        if (it->second == id) {
            it = breakpointLocations_.erase(it);
        } else {
            ++it;
        }
    }
    if (breakpointConditions_.HasCondition(id)) {
        breakpointConditions_.RemoveCondition(id);
    }
}

void ThreadState::SetPauseOnExceptions(PauseOnExceptionsState state)
{
    pauseOnExceptionsState_ = state;
}

void ThreadState::OnException(bool uncaught)
{
    ASSERT(!paused_);
    if (skipAllPauses_) {
        return;
    }
    switch (pauseOnExceptionsState_) {
        case PauseOnExceptionsState::NONE:
            break;
        case PauseOnExceptionsState::CAUGHT:
            paused_ = !uncaught;
            break;
        case PauseOnExceptionsState::UNCAUGHT:
            paused_ = uncaught;
            break;
        case PauseOnExceptionsState::ALL:
            paused_ = true;
            break;
    }
}

void ThreadState::OnFramePop()
{
    ASSERT(!paused_);
    switch (stepKind_) {
        case StepKind::NONE:
        case StepKind::BREAK_ON_START:
        case StepKind::CONTINUE_TO:
        case StepKind::PAUSE:
        case StepKind::STEP_INTO: {
            break;
        }

        case StepKind::STEP_OUT:
        case StepKind::STEP_OVER: {
            methodEntered_ = false;
            break;
        }
    }
}

bool ThreadState::OnMethodEntry()
{
    ASSERT(!paused_);
    switch (stepKind_) {
        case StepKind::NONE:
        case StepKind::BREAK_ON_START:
        case StepKind::CONTINUE_TO:
        case StepKind::PAUSE:
        case StepKind::STEP_INTO: {
            return false;
        }

        case StepKind::STEP_OUT:
        case StepKind::STEP_OVER: {
            return !std::exchange(methodEntered_, true);
        }
    }

    UNREACHABLE();
}

void ThreadState::OnSingleStep(const PtLocation &location)
{
    ASSERT(!paused_);

    if (ShouldStopAtBreakpoint(location)) {
        paused_ = true;
        return;
    }

    switch (stepKind_) {
        case StepKind::NONE: {
            paused_ = false;
            break;
        }

        case StepKind::BREAK_ON_START: {
            paused_ = true;
            break;
        }

        case StepKind::CONTINUE_TO: {
            paused_ = stepLocations_.find(location) != stepLocations_.end();
            break;
        }

        case StepKind::PAUSE: {
            paused_ = true;
            break;
        }

        case StepKind::STEP_INTO: {
            paused_ = stepLocations_.find(location) == stepLocations_.end();
            break;
        }

        case StepKind::STEP_OUT: {
            paused_ = !methodEntered_;
            break;
        }

        case StepKind::STEP_OVER: {
            paused_ = !methodEntered_ && stepLocations_.find(location) == stepLocations_.end();
            break;
        }
    }
}

static std::string DumpBreakpoint(BreakpointId bpId, const PtLocation &loc)
{
    std::stringstream ss;
    ss << "breakpoint #" << bpId << " at " << loc.GetPandaFile() << ':' << loc.GetMethodId() << ':'
       << loc.GetBytecodeOffset();
    return ss.str();
}

bool ThreadState::ShouldStopAtBreakpoint(const PtLocation &location)
{
    if (!breakpointsActive_ || skipAllPauses_) {
        return false;
    }

    auto rng = breakpointLocations_.equal_range(location);
    for (auto iter = rng.first; iter != rng.second; ++iter) {
        auto bpId = iter->second;
        if (!breakpointConditions_.HasCondition(bpId)) {
            // Should pause unconditionally
            return true;
        }
        auto res = breakpointConditions_.EvaluateCondition(bpId);
        if (!res) {
            LOG(WARNING, DEBUGGER) << DumpBreakpoint(bpId, location) << " condition evaluation failure";
            HandleError(res.Error());
        } else if (res.Value()) {
            return true;
        }
    }
    return false;
}
}  // namespace ark::tooling::inspector
