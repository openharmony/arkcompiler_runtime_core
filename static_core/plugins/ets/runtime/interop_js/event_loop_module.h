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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_EVENT_LOOP_MODULE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_EVENT_LOOP_MODULE_H

#include <uv.h>
#include <node_api.h>
#include <queue>

#include "runtime/include/external_callback_poster.h"
#include "libpandabase/os/mutex.h"

namespace ark::ets::interop::js {

class EventLoopCallbackPoster : public CallbackPoster {
    class ThreadSafeCallbackQueue {
        using CallbackQueue = std::queue<WrappedCallback>;

    public:
        ThreadSafeCallbackQueue() = default;
        NO_COPY_SEMANTIC(ThreadSafeCallbackQueue);
        NO_MOVE_SEMANTIC(ThreadSafeCallbackQueue);
        ~ThreadSafeCallbackQueue() = default;

        void PushCallback(WrappedCallback &&callback, uv_async_t *async);
        void ExecuteAllCallbacks();

    private:
        os::memory::RecursiveMutex lock_;
        CallbackQueue callbackQueue_ GUARDED_BY(lock_);
    };

public:
    static_assert(PANDA_ETS_INTEROP_JS);
    EventLoopCallbackPoster();
    ~EventLoopCallbackPoster() override;
    NO_COPY_SEMANTIC(EventLoopCallbackPoster);
    NO_MOVE_SEMANTIC(EventLoopCallbackPoster);

    static uv_loop_t *GetEventLoop();

private:
    void PostImpl(WrappedCallback &&callback) override;

    void PostToEventLoop(WrappedCallback &&callback);

    static void AsyncEventToExecuteCallbacks(uv_async_t *async);

    uv_async_t *async_ = nullptr;
    ThreadSafeCallbackQueue *callbackQueue_;
};

class EventLoopCallbackPosterFactoryImpl : public CallbackPosterFactoryIface {
public:
    EventLoopCallbackPosterFactoryImpl() = default;
    ~EventLoopCallbackPosterFactoryImpl() override = default;
    NO_COPY_SEMANTIC(EventLoopCallbackPosterFactoryImpl);
    NO_MOVE_SEMANTIC(EventLoopCallbackPosterFactoryImpl);

    /// @brief Method should create a unique instance of CallbackPoster with inputted strategy of posting
    PandaUniquePtr<CallbackPoster> CreatePoster() override;
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_EVENT_LOOP_MODULE_H
