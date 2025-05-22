/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_DEBUGGER_ARKAPI_H
#define PANDA_DEBUGGER_ARKAPI_H

#include <string>
#include <functional>

#include "libpandabase/macros.h"

namespace ark {
class PANDA_PUBLIC_API ArkDebugNativeAPI final {
public:
    using DebuggerPostTask = std::function<void(std::function<void()> &&)>;

    static bool StartDebuggerForSocketPair(int tid, int socketfd = -1);
    static bool NotifyDebugMode(int tid, int32_t instanceId = 0, bool debugApp = false);
    static bool StopDebugger();

    ArkDebugNativeAPI() = delete;
    ~ArkDebugNativeAPI() = delete;

    NO_COPY_SEMANTIC(ArkDebugNativeAPI);
    NO_MOVE_SEMANTIC(ArkDebugNativeAPI);
};

}  // namespace ark

#endif  // PANDA_DEBUGGER_ARKAPI_H
