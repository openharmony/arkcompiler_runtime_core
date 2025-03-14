/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

// NOTE(konstanting, #23205): A workaround for the hybrid cmake build. Will be removed soon
// using a separate .cpp file with weak symbols.
#if defined(PANDA_JS_ETS_HYBRID_MODE_NEED_WEAK_SYMBOLS)
extern "C" napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_get_uv_event_loop([[maybe_unused]] napi_env env, [[maybe_unused]] struct uv_loop_s **loop)
{
    // NOTE: Empty stub. Needed only for the corner case with verifier/aot in the hybrid cmake build
    INTEROP_LOG(ERROR) << "napi_add_env_cleanup_hook is implemented in OHOS since 4.1.0, please update" << std::endl;
    return napi_ok;
}
#endif /* PANDA_JS_ETS_HYBRID_MODE_NEED_WEAK_SYMBOLS */

namespace ark::ets::interop::js {

/*static*/
uv_loop_t *EventLoop::GetEventLoop()
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

void EventLoop::RunEventLoop(RunMode mode)
{
    auto *loop = GetEventLoop();
    switch (mode) {
        case EventLoop::RUN_DEFAULT:
            uv_run(loop, UV_RUN_DEFAULT);
            break;
        case EventLoop::RUN_ONCE:
            uv_run(loop, UV_RUN_ONCE);
            break;
        case EventLoop::RUN_NOWAIT:
            uv_run(loop, UV_RUN_NOWAIT);
            break;
        default:
            UNREACHABLE();
    };
}

EventLoopCallbackPoster::EventLoopCallbackPoster()
{
    auto loop = EventLoop::GetEventLoop();
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
    auto destroyCb = [async = this->async_]() {
        auto deleter = [](uv_handle_t *handle) {
            auto *poster = reinterpret_cast<ThreadSafeCallbackQueue *>(handle->data);
            delete poster;
            delete handle;
        };
        uv_close(reinterpret_cast<uv_handle_t *>(async), deleter);
    };
    if (NeedDestroyInPlace()) {
        destroyCb();
        return;
    }
    PostToEventLoop(std::move(destroyCb));
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
    InteropCodeScope<false> codeScope(coro, __PRETTY_FUNCTION__);
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
    [[maybe_unused]] auto *w = Coroutine::GetCurrent()->GetContext<StackfulCoroutineContext>()->GetWorker();
    ASSERT(w->IsMainWorker() || w->InExclusiveMode());
    auto poster = MakePandaUnique<EventLoopCallbackPoster>();
    ASSERT(poster != nullptr);
    return poster;
}

}  // namespace ark::ets::interop::js
