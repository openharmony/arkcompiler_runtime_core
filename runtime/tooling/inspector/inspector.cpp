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

#include "inspector.h"
#include "breakpoint_hooks.h"
#include "error.h"
#include "server.h"
#include "step_hooks.h"

#include "macros.h"
#include "tooling/debug_interface.h"
#include "utils/json_builder.h"
#include "utils/utf.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <string>

using namespace std::literals::string_literals;  // NOLINT(google-build-using-namespace)
using namespace std::placeholders;               // NOLINT(google-build-using-namespace)

namespace panda::tooling::inspector {
Inspector::Inspector(Server &server, DebugInterface *debugger)
    : server_(server),
      debugger_(debugger),
      sourceManager_(server),
      hooks_(*this, std::make_unique<StepHooks>(*this), std::make_unique<BreakpointHooks>(*this))
{
    if (!HandleError(debugger_->RegisterHooks(&hooks_))) {
        return;
    }

    server_.OnCall("Debugger.enable", std::bind(&Inspector::EnableDebugger, this, _1));
    server_.OnCall("Debugger.resume", std::bind(&Inspector::Resume, this));
    server_.OnCall("Runtime.enable", std::bind(&Inspector::EnableRuntime, this));
}

Inspector::~Inspector()
{
    HandleError(debugger_->UnregisterHooks());
}

void Inspector::EnableDebugger(JsonObjectBuilder &result)
{
    result.AddProperty("debuggerId", "debugger");
}

void Inspector::EnableRuntime()
{
    server_.Call("Runtime.executionContextCreated", [](auto &params) {
        params.AddProperty("context", [](JsonObjectBuilder &context) {
            context.AddProperty("id", 0);
            context.AddProperty("origin", "");
            context.AddProperty("name", "context");
        });
    });
}

bool Inspector::HandlePendingPause()
{
    if (!pendingPause_) {
        return false;
    }

    std::for_each(pauseHandlers_.begin(), pauseHandlers_.end(), [](auto &handler) { handler(); });

    auto addCallFrame = [&this](JsonArrayBuilder &callFrames, const PtFrame &frame) {
        auto &sourceFile = sourceManager_.GetOrLoadSourceFile(frame.GetMethod());
        auto &table = sourceFile.GetDebugInfo().GetLineNumberTable(frame.GetMethod()->GetFileId());

        // Line number entry corresponding to the bytecode location is
        // the last such entry for which the bytecode offset is not greater than
        // the given offset. Use `std::upper_bound` to find the first entry
        // for which the condition is not true, and then step back.
        auto lineNumberIter = std::upper_bound(table.begin(), table.end(), frame.GetBytecodeOffset(),
                                               [](auto offset, auto &entry) { return offset < entry.offset; });
        ASSERT(lineNumberIter != table.begin());
        auto lineNumber = std::prev(lineNumberIter)->line - 1;

        callFrames.Add([&](JsonObjectBuilder &callFrame) {
            callFrame.AddProperty("callFrameId", std::to_string(frame.GetFrameId()));
            callFrame.AddProperty("functionName", utf::Mutf8AsCString(frame.GetMethod()->GetName().data));
            callFrame.AddProperty("location", [&](JsonObjectBuilder &location) {
                location.AddProperty("scriptId", std::to_string(sourceFile.GetScriptId()));
                location.AddProperty("lineNumber", lineNumber);
            });
            callFrame.AddProperty("url", "file:///"s + sourceFile.GetFileName());
            callFrame.AddProperty("scopeChain", [](JsonArrayBuilder & /* scopeChain */) {});
            callFrame.AddProperty("this", [](JsonObjectBuilder &thisObj) { thisObj.AddProperty("type", "undefined"); });
        });

        return true;
    };

    if (pendingPause_->reason) {
        server_.Call("Debugger.paused", [&](auto &params) {
            params.AddProperty("callFrames", [&](JsonArrayBuilder &array) {
                HandleError(
                    debugger_->EnumerateFrames(pendingPause_->thread, std::bind(addCallFrame, std::ref(array), _1)));
            });
            params.AddProperty("reason", *pendingPause_->reason);
        });
    }

    pausedThread_.emplace(pendingPause_->thread);
    pendingPause_.reset();

    server_.Run();
    pausedThread_.reset();
    return true;
}

PtThread Inspector::Resume()
{
    server_.Call("Debugger.resumed");
    server_.Pause();
    return *pausedThread_;
}
}  // namespace panda::tooling::inspector
