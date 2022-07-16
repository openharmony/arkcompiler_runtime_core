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

#ifndef PANDA_TOOLING_INSPECTOR_STEP_HOOKS_H
#define PANDA_TOOLING_INSPECTOR_STEP_HOOKS_H

#include "tooling/debugger.h"
#include "tooling/debug_interface.h"
#include "tooling/pt_location.h"
#include "tooling/pt_thread.h"

#include <optional>
#include <unordered_map>

namespace panda {
class JsonObject;
}  // namespace panda

namespace panda::tooling::inspector {
class Inspector;

class StepHooks : public PtHooks {
public:
    explicit StepHooks(Inspector &inspector);

    void Breakpoint(PtThread thread, Method * /* method */, const PtLocation &location) override;
    void FramePop(PtThread thread, Method * /* method */, bool /* wasPoppedByException */) override;
    void MethodEntry(PtThread thread, Method * /* method */) override;
    void SingleStep(PtThread thread, Method * /* method */, const PtLocation & /* location */) override;
    void VmInitialization(PtThread thread) override;

private:
    enum StepKind {
        BREAK_ON_START,
        CONTINUE_TO,
        STEP_INTO,
        STEP_OUT,
        STEP_OVER,
    };

    void AddLocation(const PtLocation &location);
    void ContinueToLocation(const JsonObject &params);
    void OnPause();
    void SetBreakOnStart();
    void SetStep(StepKind stepKind);

    struct State {
        StepKind stepKind;
        PtThread thread {PtThread::NONE};
        std::unordered_map<PtLocation, bool, HashLocation> locations;
        bool methodEntered {false};
        bool pauseOnStep {false};
    };

    Inspector &inspector_;
    std::optional<State> state_;
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_STEP_HOOKS_H
