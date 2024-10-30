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
#if defined(PANDA_TARGET_OHOS)
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
    ASSERT(coro == coro->GetPandaVM()->GetCoroutineManager()->GetMainThread());
    auto loop = GetEventLoop();
    async_ = Runtime::GetCurrent()->GetInternalAllocator()->New<uv_async_t>();
    callbackQueue_ = Runtime::GetCurrent()->GetInternalAllocator()->New<ThreadSafeCallbackQueue>();
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
            Runtime::GetCurrent()->GetInternalAllocator()->Delete(poster);
            Runtime::GetCurrent()->GetInternalAllocator()->Delete(handle);
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
    callbackQueue->ExecuteAllCallbacks();
}

void EventLoopCallbackPoster::ThreadSafeCallbackQueue::PushCallback(WrappedCallback &&callback, uv_async_t *async)
{
    os::memory::LockHolder lh(lock_);
    callbackQueue_.push(std::move(callback));
    ASSERT(async != nullptr);
    [[maybe_unused]] auto uvstatus = uv_async_send(async);
    ASSERT(uvstatus == 0);
}

void EventLoopCallbackPoster::ThreadSafeCallbackQueue::ExecuteAllCallbacks()
{
    os::memory::LockHolder lh(lock_);
    while (!callbackQueue_.empty()) {
        auto callback = callbackQueue_.front();
        callback();
        callbackQueue_.pop();
    }
}

PandaUniquePtr<CallbackPoster> EventLoopCallbackPosterFactoryImpl::CreatePoster()
{
    auto poster = MakePandaUnique<EventLoopCallbackPoster>();
    ASSERT(poster != nullptr);
    return poster;
}

}  // namespace ark::ets::interop::js
