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

#include <unistd.h>
#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/interop_js/timer_module.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

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
        {{"startTimer", nullptr, reinterpret_cast<void *>(TimerModule::StartTimer)},
         {"stopTimer", nullptr, reinterpret_cast<void *>(TimerModule::StopTimer)}}};
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
    ets_method invokeMethod = mainEtsEnv_->Getp_method(cls, "invoke", nullptr);
    ark::ets::interop::js::JSNapiEnvScope scope(ark::ets::interop::js::InteropCtx::Current(), jsEnv_);
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

uv_loop_t *TimerModule::GetMainEventLoop()
{
#if defined(PANDA_TARGET_OHOS)
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
