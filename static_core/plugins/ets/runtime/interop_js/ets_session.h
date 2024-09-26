/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_SESSION_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_SESSION_H

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_coroutine.h"

#include <atomic>

namespace ark::ets {
class EtsCoroutine;
}  // namespace ark::ets

namespace ark::ets::interop::js {

/**
 * @brief This class is needed to mark JS -> ETS transitions
 * The first such transition means the start of the ETS session
 * and all other transitions are ignored. This functionality allows
 * to do useful work (e.g. create uv_async_t for current napi_env)
 */
class ScopedETSSession {
public:
    explicit ScopedETSSession(EtsCoroutine *coro) : coro_(coro)
    {
        StartSession(coro_);
    }

    ~ScopedETSSession()
    {
        EndSession(coro_);
    }

    NO_COPY_SEMANTIC(ScopedETSSession);
    NO_MOVE_SEMANTIC(ScopedETSSession);

private:
    static void StartSession(EtsCoroutine *coro)
    {
        // Atomic with acq_rel order reason: sync Start/End session in other threads
        if (transitionCount_++ == 0) {
            // interop with js is allowed only from MAIN worker
            ASSERT(coro->GetCoroutineManager()->IsMainWorker(coro));
            auto poster = coro->GetPandaVM()->CreateCallbackPoster();
            coro->GetCoroutineManager()->SetCallbackPoster(std::move(poster));
        }
    }

    static void EndSession(EtsCoroutine *coro)
    {
        // Atomic with acq_rel order reason: sync Start/End session in other threads
        if (transitionCount_-- == 1) {
            coro->GetCoroutineManager()->TryResetCallbackPoster();
        }
    }

    static inline uint32_t transitionCount_ = 0;

    EtsCoroutine *coro_;
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_SESSION_H
