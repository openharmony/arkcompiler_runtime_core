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

#ifndef PANDA_TOOLING_INSPECTOR_THREAD_STATE_H
#define PANDA_TOOLING_INSPECTOR_THREAD_STATE_H

#include <unordered_set>
#include <vector>

#include "runtime/tooling/debugger.h"

#include "evaluation/conditions_storage.h"
#include "types/numeric_id.h"
#include "types/pause_on_exceptions_state.h"

namespace ark::tooling::inspector {
class DebuggableThread;

class ThreadState final {
public:
    explicit ThreadState(EvaluationEngine &engine) : breakpointConditions_(engine) {}
    ~ThreadState() = default;

    NO_COPY_SEMANTIC(ThreadState);
    NO_MOVE_SEMANTIC(ThreadState);

    bool IsPaused() const
    {
        return paused_;
    }

    std::vector<BreakpointId> GetBreakpointsByLocation(const PtLocation &location) const;

    void Reset();
    void BreakOnStart();
    void Continue();
    void ContinueTo(std::unordered_set<PtLocation, HashLocation> locations);
    void StepInto(std::unordered_set<PtLocation, HashLocation> locations);
    void StepOver(std::unordered_set<PtLocation, HashLocation> locations);
    void StepOut();
    void Pause();
    void SetBreakpointsActive(bool active);
    void SetSkipAllPauses(bool skip);
    void SetMixedDebugEnabled(bool mixedDebugEnabled);
    /**
     * @brief Set a breakpoint with optional condition.
     * @param locations to set breakpoint at, all will have the same BreakpointId.
     * @param condition pointer to string with panda bytecode, will not be saved.
     * @returns BreakpointId of set breakpoint.
     */
    BreakpointId SetBreakpoint(const std::vector<PtLocation> &locations, const std::string *condition = nullptr);
    void RemoveBreakpoint(BreakpointId id);

    template <typename F>
    void EnumerateBreakpoints(const F &func)
    {
        for (const auto &[loc, id] : breakpointLocations_) {
            func(loc, id);
        }
    }

    void SetPauseOnExceptions(PauseOnExceptionsState state);

    void OnException(bool uncaught);
    void OnFramePop();
    bool OnMethodEntry();
    void OnSingleStep(const PtLocation &location);

private:
    enum class StepKind {
        // Just continue execution
        NONE,

        // Stop on the next step event. It is not reset to NONE during resetting the state on a new connection
        BREAK_ON_START,

        // Continue execution until one of the specified locations is reached
        CONTINUE_TO,

        // Stop on the next step event
        PAUSE,

        // Continue execution until one of the locations, other than the current line, is reached
        STEP_INTO,

        // Continue execution until the current frame is popped
        STEP_OUT,

        // Continue execution until both:
        // - The call frame is not a child to the starting position (e.g. we have not entered some method);
        // - One of the locations, other than the current line, is reached.
        STEP_OVER
    };

private:
    bool ShouldStopAtBreakpoint(const PtLocation &location);

private:
    StepKind stepKind_ {StepKind::NONE};

    // The set of locations has different semantics for various kinds of stepping:
    // - for CONTINUE_TO it contains the set of locations we should reach to end the step;
    // - for STEP_INTO and STEP_OVER it contains the set of locations corresponding to the current line
    //   (then, the locations we should leave to end the step).
    std::unordered_set<PtLocation, HashLocation> stepLocations_;

    bool methodEntered_ {false};

    bool breakpointsActive_ {true};
    BreakpointId nextBreakpointId_ {0};
    std::unordered_multimap<PtLocation, BreakpointId, HashLocation> breakpointLocations_;
    ConditionsStorage breakpointConditions_;

    PauseOnExceptionsState pauseOnExceptionsState_ {PauseOnExceptionsState::NONE};

    bool paused_ {false};
    bool skipAllPauses_ {false};
    bool mixedDebugEnabled_ {false};
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_THREAD_STATE_H
