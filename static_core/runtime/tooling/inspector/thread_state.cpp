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

#include "thread_state.h"

namespace panda::tooling::inspector {
std::vector<BreakpointId> ThreadState::GetBreakpointsByLocation(const PtLocation &location) const
{
    std::vector<BreakpointId> hit_breakpoints;

    auto range = breakpoint_locations_.equal_range(location);
    std::transform(range.first, range.second, std::back_inserter(hit_breakpoints), [](auto &p) { return p.second; });

    return hit_breakpoints;
}

void ThreadState::Reset()
{
    if (step_kind_ != StepKind::BREAK_ON_START) {
        step_kind_ = StepKind::NONE;
    }
    step_locations_.clear();
    method_entered_ = false;
    breakpoints_active_ = true;
    next_breakpoint_id_ = 0;
    breakpoint_locations_.clear();
    pause_on_exceptions_state_ = PauseOnExceptionsState::NONE;
}

void ThreadState::BreakOnStart()
{
    if (!paused_) {
        step_kind_ = StepKind::BREAK_ON_START;
    }
}

void ThreadState::Continue()
{
    step_kind_ = StepKind::NONE;
    paused_ = false;
}

void ThreadState::ContinueTo(std::unordered_set<PtLocation, HashLocation> locations)
{
    step_kind_ = StepKind::CONTINUE_TO;
    step_locations_ = std::move(locations);
    paused_ = false;
}

void ThreadState::StepInto(std::unordered_set<PtLocation, HashLocation> locations)
{
    step_kind_ = StepKind::STEP_INTO;
    method_entered_ = false;
    step_locations_ = std::move(locations);
    paused_ = false;
}

void ThreadState::StepOver(std::unordered_set<PtLocation, HashLocation> locations)
{
    step_kind_ = StepKind::STEP_OVER;
    method_entered_ = false;
    step_locations_ = std::move(locations);
    paused_ = false;
}

void ThreadState::StepOut()
{
    step_kind_ = StepKind::STEP_OUT;
    method_entered_ = true;
    paused_ = false;
}

void ThreadState::Pause()
{
    if (!paused_) {
        step_kind_ = StepKind::PAUSE;
    }
}

void ThreadState::SetBreakpointsActive(bool active)
{
    breakpoints_active_ = active;
}

BreakpointId ThreadState::SetBreakpoint(const std::vector<PtLocation> &locations)
{
    auto id = next_breakpoint_id_++;
    for (auto &location : locations) {
        breakpoint_locations_.emplace(location, id);
    }

    return id;
}

void ThreadState::RemoveBreakpoint(BreakpointId id)
{
    for (auto it = breakpoint_locations_.begin(); it != breakpoint_locations_.end();) {
        if (it->second == id) {
            it = breakpoint_locations_.erase(it);
        } else {
            ++it;
        }
    }
}

void ThreadState::SetPauseOnExceptions(PauseOnExceptionsState state)
{
    pause_on_exceptions_state_ = state;
}

void ThreadState::OnException(bool uncaught)
{
    ASSERT(!paused_);
    switch (pause_on_exceptions_state_) {
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
    switch (step_kind_) {
        case StepKind::NONE:
        case StepKind::BREAK_ON_START:
        case StepKind::CONTINUE_TO:
        case StepKind::PAUSE:
        case StepKind::STEP_INTO: {
            break;
        }

        case StepKind::STEP_OUT:
        case StepKind::STEP_OVER: {
            method_entered_ = false;
            break;
        }
    }
}

bool ThreadState::OnMethodEntry()
{
    ASSERT(!paused_);
    switch (step_kind_) {
        case StepKind::NONE:
        case StepKind::BREAK_ON_START:
        case StepKind::CONTINUE_TO:
        case StepKind::PAUSE:
        case StepKind::STEP_INTO: {
            return false;
        }

        case StepKind::STEP_OUT:
        case StepKind::STEP_OVER: {
            return !std::exchange(method_entered_, true);
        }
    }

    UNREACHABLE();
}

void ThreadState::OnSingleStep(const PtLocation &location)
{
    ASSERT(!paused_);
    if (!breakpoints_active_ || breakpoint_locations_.find(location) == breakpoint_locations_.end()) {
        switch (step_kind_) {
            case StepKind::NONE: {
                paused_ = false;
                break;
            }

            case StepKind::BREAK_ON_START: {
                paused_ = true;
                break;
            }

            case StepKind::CONTINUE_TO: {
                paused_ = step_locations_.find(location) != step_locations_.end();
                break;
            }

            case StepKind::PAUSE: {
                paused_ = true;
                break;
            }

            case StepKind::STEP_INTO: {
                paused_ = step_locations_.find(location) == step_locations_.end();
                break;
            }

            case StepKind::STEP_OUT: {
                paused_ = !method_entered_;
                break;
            }

            case StepKind::STEP_OVER: {
                paused_ = !method_entered_ && step_locations_.find(location) == step_locations_.end();
                break;
            }
        }
    } else {
        paused_ = true;
    }
}
}  // namespace panda::tooling::inspector
