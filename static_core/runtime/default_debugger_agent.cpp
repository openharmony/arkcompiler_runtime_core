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

#include "default_debugger_agent.h"

namespace panda {
DefaultDebuggerAgent::DefaultDebuggerAgent(os::memory::Mutex &mutex)
    : LibraryAgent(mutex, PandaString(Runtime::GetOptions().GetDebuggerLibraryPath()), "StartDebugger", "StopDebugger")
{
}

bool DefaultDebuggerAgent::Load()
{
    debug_session_ = Runtime::GetCurrent()->StartDebugSession();
    if (!debug_session_) {
        LOG(ERROR, RUNTIME) << "Could not start debug session";
        return false;
    }

    if (!LibraryAgent::Load()) {
        debug_session_.reset();
        return false;
    }

    return true;
}

bool DefaultDebuggerAgent::Unload()
{
    auto result = LibraryAgent::Unload();
    debug_session_.reset();
    return result;
}

bool DefaultDebuggerAgent::CallLoadCallback(void *resolved_function)
{
    ASSERT(resolved_function);
    ASSERT(debug_session_);

    using StartDebuggerT = int (*)(uint32_t, bool);
    uint32_t port = Runtime::GetOptions().GetDebuggerPort();
    bool break_on_start = Runtime::GetOptions().IsDebuggerBreakOnStart();
    int res = reinterpret_cast<StartDebuggerT>(resolved_function)(port, break_on_start);
    if (res != 0) {
        LOG(ERROR, RUNTIME) << "'StartDebugger' has failed with " << res;
        return false;
    }

    return true;
}

bool DefaultDebuggerAgent::CallUnloadCallback(void *resolved_function)
{
    ASSERT(resolved_function);

    using StopDebugger = int (*)();
    int res = reinterpret_cast<StopDebugger>(resolved_function)();
    if (res != 0) {
        LOG(ERROR, RUNTIME) << "'StopDebugger' has failed with " << res;
        return false;
    }

    return true;
}
}  // namespace panda
