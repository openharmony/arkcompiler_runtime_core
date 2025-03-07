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

#include <unistd.h>
#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/interop_js/timer_module.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/types/ets_type.h"

// NOTE(konstanting, #23205): in case of interop with Node, libuv will be available automatically via
// the load of the ets_vm_plugin module. But since the libuv users are now in the libarkruntime, we should
// define some weak stubs for libuv functions. Actually it's worth to create a separate cpp file for them,
// like it is done for napi.
#if (!defined(PANDA_TARGET_OHOS) && !defined(PANDA_JS_ETS_HYBRID_MODE)) || \
    defined(PANDA_JS_ETS_HYBRID_MODE_NEED_WEAK_SYMBOLS)
// NOLINTBEGIN(readability-identifier-naming)
// CC-OFFNXT(G.FMT.10-CPP) project code style
extern "C" {
__attribute__((weak)) int uv_async_send([[maybe_unused]] uv_async_t *async)
{
    return -1;
}
__attribute__((weak)) int uv_async_init([[maybe_unused]] uv_loop_t *loop, [[maybe_unused]] uv_async_t *async,
                                        [[maybe_unused]] uv_async_cb async_cb)
{
    return -1;
}

__attribute__((weak)) void uv_update_time([[maybe_unused]] uv_loop_t *loop) {}

__attribute__((weak)) int uv_timer_init([[maybe_unused]] uv_loop_t *loop, [[maybe_unused]] uv_timer_t *handle)
{
    return -1;
}
__attribute__((weak)) int uv_timer_start([[maybe_unused]] uv_timer_t *handle, [[maybe_unused]] uv_timer_cb cb,
                                         [[maybe_unused]] uint64_t timeout, [[maybe_unused]] uint64_t repeat)
{
    return -1;
}
__attribute__((weak)) int uv_timer_stop([[maybe_unused]] uv_timer_t *handle)
{
    return -1;
}
__attribute__((weak)) int uv_timer_again([[maybe_unused]] uv_timer_t *handle)
{
    return -1;
}

__attribute__((weak)) void uv_close([[maybe_unused]] uv_handle_t *handle, [[maybe_unused]] uv_close_cb close_cb) {}

__attribute__((weak)) uv_loop_t *uv_default_loop()
{
    return nullptr;
}
__attribute__((weak)) int uv_run([[maybe_unused]] uv_loop_t *loop, [[maybe_unused]] uv_run_mode mode)
{
    return -1;
}
}
// NOLINTEND(readability-identifier-naming)
#endif /* !defined(PANDA_TARGET_OHOS) && !defined(PANDA_JS_ETS_HYBRID_MODE) */

pid_t TimerModule::mainTid_ = 0;
EtsEnv *TimerModule::mainEtsEnv_ = nullptr;
napi_env TimerModule::jsEnv_ = nullptr;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::unordered_map<uint32_t, uv_timer_t *> TimerModule::timers_;
uint32_t TimerModule::nextTimerId_ = 0;

TimerModule::TimerInfo::TimerInfo(uint32_t id, ets_object funcObject, bool oneShotTimer)
    : id_(id), funcObject_(funcObject), oneShotTimer_(oneShotTimer)
{
}

bool TimerModule::Init(EtsEnv *env, napi_env jsEnv)
{
    mainTid_ = gettid();
    mainEtsEnv_ = env;
    jsEnv_ = jsEnv;

    ets_class globalClass = env->FindClass("escompat/ETSGLOBAL");
    if (globalClass == nullptr) {
        INTEROP_LOG(ERROR) << "Cannot find GLOBAL class";
        return false;
    }

    const std::array<EtsNativeMethod, 2> impls = {
        {{"startTimerImpl", nullptr, reinterpret_cast<void *>(TimerModule::StartTimer)},
         {"stopTimerImpl", nullptr, reinterpret_cast<void *>(TimerModule::StopTimer)}}};
    return env->RegisterNatives(globalClass, impls.data(), impls.size()) == ETS_OK;
}

ets_int TimerModule::StartTimer(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_object funcObject, ets_int delayMs,
                                ets_boolean oneShotTimer)
{
    if (!CheckMainThread(env)) {
        return 0;
    }
    uv_loop_t *loop = GetMainEventLoop();
    auto *timer = new uv_timer_t();
    uv_timer_init(loop, timer);

    uint32_t timerId = GetTimerId();
    timer->data = new TimerInfo(timerId, env->NewGlobalRef(funcObject), static_cast<bool>(oneShotTimer));
    timers_[timerId] = timer;

    uv_update_time(loop);
    uv_timer_start(timer, TimerCallback, delayMs, !static_cast<bool>(oneShotTimer) ? delayMs : 0);
    uv_async_send(&loop->wq_async);
    return timerId;
}

void TimerModule::StopTimer(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_int timerId)
{
    if (!CheckMainThread(env)) {
        return;
    }
    auto it = timers_.find(timerId);
    if (it == timers_.end()) {
        return;
    }
    DisarmTimer(it->second);
}

void TimerModule::TimerCallback(uv_timer_t *timer)
{
    auto *info = reinterpret_cast<TimerInfo *>(timer->data);
    bool oneShotTimer = info->IsOneShotTimer();
    if (oneShotTimer) {
        DisarmTimer(timer);
    }
    ets_class cls = mainEtsEnv_->GetObjectClass(info->GetFuncObject());
    ets_method invokeMethod = mainEtsEnv_->Getp_method(cls, ark::ets::INVOKE_METHOD_NAME, nullptr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    mainEtsEnv_->CallVoidMethod(info->GetFuncObject(), invokeMethod);
    if (!oneShotTimer) {
        uv_timer_again(timer);
    }
}

void TimerModule::DisarmTimer(uv_timer_t *timer)
{
    auto *info = reinterpret_cast<TimerInfo *>(timer->data);
    timers_.erase(info->GetId());
    uv_timer_stop(timer);
    uv_close(reinterpret_cast<uv_handle_t *>(timer), FreeTimer);
}

void TimerModule::FreeTimer(uv_handle_t *timer)
{
    auto *info = reinterpret_cast<TimerInfo *>(timer->data);
    mainEtsEnv_->DeleteGlobalRef(info->GetFuncObject());
    delete info;
    delete timer;
}

/* static */
uv_loop_t *TimerModule::GetMainEventLoop()
{
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    uv_loop_t *loop;
    napi_get_uv_event_loop(ark::ets::interop::js::InteropCtx::Current()->GetJSEnv(), &loop);
    return loop;
#else
    return uv_default_loop();
#endif
}

uint32_t TimerModule::GetTimerId()
{
    return nextTimerId_++;
}

bool TimerModule::CheckMainThread(EtsEnv *env)
{
    if (gettid() == mainTid_) {
        return true;
    }
    ets_class errorClass = env->FindClass("std/core/InternalError");
    ASSERT(errorClass != nullptr);
    env->ThrowErrorNew(errorClass, "The function must be called from the main coroutine");
    return false;
}
