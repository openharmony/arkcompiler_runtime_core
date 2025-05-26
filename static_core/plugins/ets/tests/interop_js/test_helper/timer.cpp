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

#include "timer.h"
#include "native_engine/native_engine.h"

#include <unordered_map>
#include <iostream>

namespace ark::ets::interop::js::helper {

// CC-OFFNXT(G.NAM.03-CPP) project code style
static uint32_t nextTimerId = 0;
static std::unordered_map<uint32_t, TimerInfo *> timers;

TimerInfo::TimerInfo(napi_env env, napi_ref cb, std::vector<napi_ref> cbArgs, bool repeat)
    : env(env), cb(cb), cbArgs(std::move(cbArgs)), repeat(repeat)
{
    timerId = nextTimerId++;
    timers[timerId] = this;
    timer.data = this;
    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    uv_timer_init(loop, &timer);
}

TimerInfo::~TimerInfo()
{
    napi_delete_reference(env, cb);
    for (auto &ref : cbArgs) {
        napi_delete_reference(env, ref);
    }
    timers.erase(timerId);
    uv_timer_stop(&timer);
    uv_close(reinterpret_cast<uv_handle_t *>(&timer), nullptr);
}

static napi_value RegisterTimer(napi_env env, napi_ref cb, std::vector<napi_ref> cbArgs, int32_t timeout, bool repeat)
{
    auto timerCallback = [](uv_timer_t *timer) {
        auto *timerInfo = reinterpret_cast<TimerInfo *>(timer->data);
        napi_env env = timerInfo->env;
        napi_value callback = nullptr;
        napi_get_reference_value(env, timerInfo->cb, &callback);
        std::vector<napi_value> callbackArgs(timerInfo->cbArgs.size());
        for (auto argIdx = 0U; argIdx < callbackArgs.size(); ++argIdx) {
            napi_get_reference_value(env, timerInfo->cbArgs[argIdx], &callbackArgs[argIdx]);
        }

        auto timerId = timerInfo->timerId;
        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);

        napi_value result = nullptr;
        napi_call_function(env, undefined, callback, callbackArgs.size(), callbackArgs.data(), &result);

        bool isExceptionPending = false;
        napi_is_exception_pending(env, &isExceptionPending);
        if (isExceptionPending || (result == nullptr)) {
            std::cerr << "Error occured during timer callback";
            std::abort();  // CC-OFF(G.STD.16-CPP) fatal error
        }

        if (timers.find(timerId) == timers.end()) {
            // timer was cleared during callback
            return;
        }

        if (timerInfo->repeat) {
            uv_timer_again(timer);
        } else {
            delete timerInfo;
        }
    };

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);

    auto *timerInfo = new TimerInfo(env, cb, std::move(cbArgs), repeat);

    uv_update_time(loop);
    uv_timer_start(&timerInfo->timer, timerCallback, timeout, timeout > 0 ? timeout : 1);
    uv_async_send(&loop->wq_async);

    napi_value timerId = nullptr;
    napi_create_uint32(env, timerInfo->timerId, &timerId);
    return timerId;
}

napi_value SetTimeoutImpl(napi_env env, napi_callback_info info, bool repeat)
{
    size_t argc = 0;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    if (argc < 1) {
        napi_throw_error(env, nullptr, "setTimeout/setInterval: callback info is nullptr");
        return nullptr;
    }

    std::vector<napi_value> argv(argc);
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv.data(), &thisVar, nullptr);

    napi_value undefined;
    napi_get_undefined(env, &undefined);

    bool isCallable = false;
    napi_is_callable(env, argv[0], &isCallable);
    if (!isCallable) {
        return undefined;
    }

    int32_t timeout = 0;
    if (argc > 1) {
        napi_status status = napi_get_value_int32(env, argv[1], &timeout);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "timeout should be number");
            return nullptr;
        }
    }
    if (timeout < 0) {
        napi_throw_error(env, nullptr, "timeout < 0 is unreasonable");
        return nullptr;
    }

    napi_ref callbackRef = nullptr;
    napi_create_reference(env, argv[0], 1, &callbackRef);

    std::vector<napi_ref> callbackArgv;
    for (size_t argsIdx = 2U; argsIdx < argc; argsIdx++) {
        napi_ref callbackArg = nullptr;
        napi_create_reference(env, argv[argsIdx], 1, &callbackArg);
        callbackArgv.push_back(callbackArg);
    }

    return RegisterTimer(env, callbackRef, std::move(callbackArgv), timeout, repeat);
}

napi_value ClearTimerImpl(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        napi_throw_error(env, nullptr, "The number of params must be one");
        return nullptr;
    }

    uint32_t tId;
    napi_status status = napi_get_value_uint32(env, argv[0], &tId);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Function param must be number");
        return nullptr;
    }

    auto timerIt = timers.find(tId);
    if (timerIt == timers.end()) {
        // timer already cleared
        return nullptr;
    }
    timers.erase(timerIt);
    auto *timerInfo = timerIt->second;
    delete timerInfo;

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

}  // namespace ark::ets::interop::js::helper
