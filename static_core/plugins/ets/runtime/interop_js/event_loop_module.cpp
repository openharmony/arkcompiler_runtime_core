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

#include "plugins/ets/runtime/interop_js/event_loop_module.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"

namespace ark::ets::interop::js {

/*static*/
uv_loop_t *EventLoopCallbackPoster::GetEventLoop()
{
    uv_loop_t *loop = nullptr;
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    [[maybe_unused]] auto nstatus = napi_get_uv_event_loop(InteropCtx::Current()->GetJSEnv(), &loop);
    ASSERT(nstatus == napi_ok);
#else
    loop = uv_default_loop();
#endif
    return loop;
}

EventLoopCallbackPoster::EventLoopCallbackPoster()
{
    [[maybe_unused]] auto *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] auto *worker = coro->GetContext<StackfulCoroutineContext>()->GetWorker();
    ASSERT(coro->GetCoroutineManager()->IsMainWorker(coro) || worker->InExclusiveMode());
    auto loop = GetEventLoop();
    // These resources will be deleted in the event loop callback after Runtime destruction,
    // so we need to use a standard allocator
    async_ = new uv_async_t();
    callbackQueue_ = new ThreadSafeCallbackQueue();
    [[maybe_unused]] auto uvstatus = uv_async_init(loop, async_, AsyncEventToExecuteCallbacks);
    ASSERT(uvstatus == 0);
    async_->data = callbackQueue_;
}

EventLoopCallbackPoster::~EventLoopCallbackPoster()
{
    ASSERT(async_ != nullptr);
    PostToEventLoop([async = this->async_]() {
        auto deleter = [](uv_handle_t *handle) {
            auto *poster = reinterpret_cast<ThreadSafeCallbackQueue *>(handle->data);
            delete poster;
            delete handle;
        };
        uv_close(reinterpret_cast<uv_handle_t *>(async), deleter);
    });
}

void EventLoopCallbackPoster::PostImpl(WrappedCallback &&callback)
{
    ASSERT(callback != nullptr);
    PostToEventLoop(std::move(callback));
}

void EventLoopCallbackPoster::PostToEventLoop(WrappedCallback &&callback)
{
    ASSERT(async_ != nullptr);
    callbackQueue_->PushCallback(std::move(callback), async_);
}

/*static*/
void EventLoopCallbackPoster::AsyncEventToExecuteCallbacks(uv_async_t *async)
{
    auto *callbackQueue = reinterpret_cast<ThreadSafeCallbackQueue *>(async->data);
    ASSERT(callbackQueue != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    auto *interopCtx = InteropCtx::Current(coro);
    auto env = interopCtx->GetJsEnvForEventLoopCallbacks();
    InteropCodeScope<false> codeScope(coro, __PRETTY_FUNCTION__);
    JSNapiEnvScope jsEnvScope(interopCtx, env);
    callbackQueue->ExecuteAllCallbacks();
}

void EventLoopCallbackPoster::ThreadSafeCallbackQueue::PushCallback(WrappedCallback &&callback, uv_async_t *async)
{
    {
        os::memory::LockHolder lh(lock_);
        callbackQueue_.push(std::move(callback));
    }
    ASSERT(async != nullptr);
    [[maybe_unused]] auto uvstatus = uv_async_send(async);
    ASSERT(uvstatus == 0);
}

void EventLoopCallbackPoster::ThreadSafeCallbackQueue::ExecuteAllCallbacks()
{
    while (!IsEmpty()) {
        auto localQueue = CallbackQueue {};
        {
            os::memory::LockHolder lh(lock_);
            std::swap(localQueue, callbackQueue_);
        }
        while (!localQueue.empty()) {
            auto callback = std::move(localQueue.front());
            callback();
            localQueue.pop();
        }
    }
}

bool EventLoopCallbackPoster::ThreadSafeCallbackQueue::IsEmpty()
{
    os::memory::LockHolder lh(lock_);
    return callbackQueue_.empty();
}

PandaUniquePtr<CallbackPoster> EventLoopCallbackPosterFactoryImpl::CreatePoster()
{
    auto poster = MakePandaUnique<EventLoopCallbackPoster>();
    ASSERT(poster != nullptr);
    return poster;
}

}  // namespace ark::ets::interop::js
