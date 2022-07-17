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

#include "utils/logger.h"

#include <cstdint>
#include <optional>

namespace panda::tooling {
class DebugInterface;
}  // namespace panda::tooling

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static panda::tooling::inspector::AsioServer g_server;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::optional<panda::tooling::inspector::Inspector> g_inspector;

extern "C" int StartDebugger(uint32_t port, panda::tooling::DebugInterface *debugger, void * /* unused */)
{
    if (g_inspector) {
        LOG(ERROR, DEBUGGER) << "Debugger has already been started";
        return 1;
    }

    if (!g_server.Start(port)) {
        return 1;
    }

    g_inspector.emplace(g_server, debugger);
    return 0;
}

extern "C" int StopDebugger()
{
    if (!g_inspector) {
        LOG(ERROR, DEBUGGER) << "Debugger has not been started";
        return 1;
    }

    g_inspector.reset();
    return static_cast<int>(!g_server.Stop());
}
