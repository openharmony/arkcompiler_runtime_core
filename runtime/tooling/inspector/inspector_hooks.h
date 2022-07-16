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

#ifndef PANDA_TOOLING_INSPECTOR_INSPECTOR_HOOKS_H
#define PANDA_TOOLING_INSPECTOR_INSPECTOR_HOOKS_H

#include "os/mutex.h"
#include "tooling/debug_interface.h"
#include "tooling/pt_thread.h"

#include <memory>
#include <utility>
#include <vector>

namespace panda {
class Method;
}  // namespace panda

namespace panda::tooling {
class PtLocation;
}  // namespace panda::tooling

namespace panda::tooling::inspector {
class Inspector;

class InspectorHooks : public PtHooks {
public:
    template <typename... Hooks>
    explicit InspectorHooks(Inspector &inspector, Hooks... hooks) : inspector_(inspector)
    {
        (hooks_.emplace_back(std::move(hooks)), ...);
    }

    void Breakpoint(PtThread thread, Method *method, const PtLocation &location) override
    {
        RunHooks<PtThread, Method *, const PtLocation &>(&PtHooks::Breakpoint, thread, method, location);
    }

    void FramePop(PtThread thread, Method *method, bool wasPoppedByException) override
    {
        RunHooks<PtThread, Method *, bool>(&PtHooks::FramePop, thread, method, wasPoppedByException);
    }

    void MethodEntry(PtThread thread, Method *method) override
    {
        RunHooks<PtThread, Method *>(&PtHooks::MethodEntry, thread, method);
    }

    void SingleStep(PtThread thread, Method *method, const PtLocation &location) override
    {
        RunHooks<PtThread, Method *, const PtLocation &>(&PtHooks::SingleStep, thread, method, location);
    }

    void VmInitialization(PtThread thread) override
    {
        RunHooks<PtThread>(&PtHooks::VmInitialization, thread);
    }

private:
    // Out-of-line definition to break include cycle.
    bool HandlePendingPause() REQUIRES(mutex_);

    template <typename... Param, typename... Arg>
    void RunHooks(void (PtHooks::*method)(Param...), Arg &&... arg)
    {
        os::memory::LockHolder lock(mutex_);

        for (auto &hooks : hooks_) {
            (hooks.get()->*method)(std::forward<Arg>(arg)...);

            if (HandlePendingPause()) {
                return;
            }
        }
    }

    os::memory::Mutex mutex_;
    Inspector &inspector_ GUARDED_BY(mutex_);
    std::vector<std::unique_ptr<PtHooks>> hooks_ GUARDED_BY(mutex_);
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_INSPECTOR_HOOKS_H
