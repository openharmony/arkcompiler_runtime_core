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

#include "asio_server.h"
#include "inspector.h"

#include <cstdint>
#include <optional>

namespace panda::tooling {
class DebugInterface;
}  // namespace panda::tooling

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static panda::Runtime::DebugSessionHandle G_DEBUG_SESSION;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static panda::tooling::inspector::AsioServer G_SERVER;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::optional<panda::tooling::inspector::Inspector> G_INSPECTOR;

extern "C" int StartDebugger(uint32_t port, bool break_on_start)
{
    if (G_INSPECTOR) {
        LOG(ERROR, DEBUGGER) << "Debugger has already been started";
        return 1;
    }

    if (!G_SERVER.Start(port)) {
        return 1;
    }

    G_DEBUG_SESSION = panda::Runtime::GetCurrent()->StartDebugSession();
    G_INSPECTOR.emplace(G_SERVER, G_DEBUG_SESSION->GetDebugger(), break_on_start);
    return 0;
}

extern "C" int StopDebugger()
{
    if (!G_INSPECTOR) {
        LOG(ERROR, DEBUGGER) << "Debugger has not been started";
        return 1;
    }

    G_INSPECTOR.reset();
    G_DEBUG_SESSION.reset();
    return static_cast<int>(!G_SERVER.Stop());
}
