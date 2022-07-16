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

#ifndef PANDA_TOOLING_INSPECTOR_INSPECTOR_H
#define PANDA_TOOLING_INSPECTOR_INSPECTOR_H

#include "inspector_hooks.h"
#include "source_manager.h"

#include "macros.h"
#include "tooling/pt_thread.h"

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace panda {
class JsonObjectBuilder;
}  // namespace panda

namespace panda::tooling {
class DebugInterface;
}  // namespace panda::tooling

namespace panda::tooling::inspector {
class Server;

class Inspector {
public:
    Inspector(Server &server, DebugInterface *debugger);
    ~Inspector();
    NO_COPY_SEMANTIC(Inspector);
    NO_MOVE_SEMANTIC(Inspector);

    DebugInterface &GetDebugger() const
    {
        return *debugger_;
    }

    Server &GetServer() const
    {
        return server_;
    }

    SourceManager &GetSourceManager()
    {
        return sourceManager_;
    }

    // Pause the VM if a pause is pending.
    bool HandlePendingPause();

    template <typename Handler>
    void OnPause(Handler &&handler)
    {
        pauseHandlers_.emplace_back(std::forward<Handler>(handler));
    }

    // Set pending pause. If reason is given, notify the client with
    // "Debugger.paused".
    void SetPause(PtThread thread, std::optional<std::string> reason)
    {
        pendingPause_.emplace(PendingPause {thread, std::move(reason)});
    }

    // Resume the VM and return resumed thread.
    PtThread Resume();

private:
    void EnableDebugger(JsonObjectBuilder &result);
    void EnableRuntime();

    struct PendingPause {
        PtThread thread;
        std::optional<std::string> reason;
    };

    Server &server_;
    DebugInterface *debugger_;
    SourceManager sourceManager_;
    std::vector<std::function<void()>> pauseHandlers_;
    InspectorHooks hooks_;
    std::optional<PendingPause> pendingPause_;
    std::optional<PtThread> pausedThread_;
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_INSPECTOR_H
