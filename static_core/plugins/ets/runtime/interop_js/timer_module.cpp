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
#include <string_view>
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
ani_env *TimerModule::mainAniEnv_ = nullptr;
napi_env TimerModule::jsEnv_ = nullptr;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::unordered_map<uint32_t, uv_timer_t *> TimerModule::timers_;
uint32_t TimerModule::nextTimerId_ = 0;

TimerModule::TimerInfo::TimerInfo(uint32_t id, ani_object funcObject, bool oneShotTimer)
    : id_(id), funcObject_(funcObject), oneShotTimer_(oneShotTimer)
{
}

bool TimerModule::Init(ani_env *env, napi_env jsEnv)
{
    mainTid_ = gettid();
    mainAniEnv_ = env;
    jsEnv_ = jsEnv;

    ani_module module {};
    auto status = env->FindModule("Lescompat;", &module);
    if (status != ANI_OK) {
        INTEROP_LOG(ERROR) << "Cannot find ESCOMPAT module";
        return false;
    }

    std::array methods = {
        ani_native_function {"startTimerImpl", "Lstd/core/Object;IZ:I",
                             reinterpret_cast<void *>(TimerModule::StartTimer)},
        ani_native_function {"stopTimerImpl", "I:V", reinterpret_cast<void *>(TimerModule::StopTimer)},
    };
    return env->Module_BindNativeFunctions(module, methods.data(), methods.size()) == ANI_OK;
}

ets_int TimerModule::StartTimer(ani_env *env, ani_object funcObject, ani_int delayMs, ani_boolean oneShotTimer)
{
    if (!CheckMainThread(env)) {
        return 0;
    }
    uv_loop_t *loop = GetMainEventLoop();
    auto *timer = new uv_timer_t();
    uv_timer_init(loop, timer);

    uint32_t timerId = GetTimerId();

    ani_ref newRef {};
    [[maybe_unused]] auto status = env->GlobalReference_Create(funcObject, &newRef);
    ASSERT(status == ANI_OK);

    timer->data = new TimerInfo(timerId, static_cast<ani_object>(newRef), static_cast<bool>(oneShotTimer));
    timers_[timerId] = timer;

    uv_update_time(loop);
    ani_int intervalMs = !static_cast<bool>(oneShotTimer)
                             ? (delayMs > TimerModule::MIN_INTERVAL_MS ? delayMs : TimerModule::MIN_INTERVAL_MS)
                             : 0;
    uv_timer_start(timer, TimerCallback, delayMs, intervalMs);
    uv_async_send(&loop->wq_async);
    return timerId;
}

void TimerModule::StopTimer(ani_env *env, ani_int timerId)
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
    ani_type result {};
    [[maybe_unused]] auto status = mainAniEnv_->Object_GetType(info->GetFuncObject(), &result);
    ASSERT(status == ANI_OK);

    ani_method invokeMethod {};
    auto resultCls = static_cast<ani_class>(result);
    status = mainAniEnv_->Class_FindMethod(resultCls, ark::ets::INVOKE_METHOD_NAME, nullptr, &invokeMethod);
    ASSERT(status == ANI_OK);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = mainAniEnv_->Object_CallMethod_Void(info->GetFuncObject(), invokeMethod);

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

    [[maybe_unused]] auto status = mainAniEnv_->GlobalReference_Delete(info->GetFuncObject());
    ASSERT(status == ANI_OK);

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

bool TimerModule::CheckMainThread(ani_env *env)
{
    if (gettid() == mainTid_) {
        return true;
    }
    ani_class errorClass {};
    [[maybe_unused]] auto status = env->FindClass("Lstd/core/InternalError;", &errorClass);
    ASSERT(status == ANI_OK);
    ASSERT(errorClass != nullptr);

    ani_method ctor {};
    status = env->Class_FindMethod(errorClass, "<ctor>", "Lstd/core/String;Lescompat/ErrorOptions;:V", &ctor);
    ASSERT(status == ANI_OK);
    ani_string message {};
    std::string_view errorMessage = "The function must be called from the main coroutine";
    status = env->String_NewUTF8(errorMessage.data(), errorMessage.size(), &message);
    if (status != ANI_OK) {
        return false;
    }

    ani_object errorObj {};

    ani_ref param {};
    status = env->GetUndefined(&param);
    ASSERT(status == ANI_OK);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_New(errorClass, ctor, &errorObj, message, param);
    if (status != ANI_OK) {
        return false;
    }
    auto error = static_cast<ani_error>(errorObj);
    status = env->ThrowError(error);
    ASSERT(status == ANI_OK);
    return false;
}
