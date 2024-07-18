/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
    nextBreakpointId_ = 0;
    breakpointLocations_.clear();
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
    if (!paused_) {
        stepKind_ = StepKind::PAUSE;
    }
}

void ThreadState::SetBreakpointsActive(bool active)
{
    breakpointsActive_ = active;
}

BreakpointId ThreadState::SetBreakpoint(const std::vector<PtLocation> &locations)
{
    auto id = nextBreakpointId_++;
    for (auto &location : locations) {
        breakpointLocations_.emplace(location, id);
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
}

void ThreadState::SetPauseOnExceptions(PauseOnExceptionsState state)
{
    pauseOnExceptionsState_ = state;
}

void ThreadState::OnException(bool uncaught)
{
    ASSERT(!paused_);
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
    if (!breakpointsActive_ || breakpointLocations_.find(location) == breakpointLocations_.end()) {
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
    } else {
        paused_ = true;
    }
}
}  // namespace ark::tooling::inspector
