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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TIMER_MODULE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TIMER_MODULE_H

#include <uv.h>
#include <node_api.h>
#include <unordered_map>
#include <sys/types.h>
#include "libpandabase/macros.h"
#include "plugins/ets/runtime/napi/ets_napi.h"

class TimerModule {
public:
    static bool Init(EtsEnv *env, napi_env jsEnv);
    static ets_int StartTimer(EtsEnv *env, ets_class klass, ets_object funcObject, ets_int delayMs,
                              ets_boolean oneShotTimer);
    static void StopTimer(EtsEnv *env, ets_class klass, ets_int timerId);

private:
    class TimerInfo {
    public:
        TimerInfo(uint32_t id, ets_object funcObject, bool oneShotTimer);
        ~TimerInfo() = default;

        uint32_t GetId() const
        {
            return id_;
        }

        ets_object GetFuncObject() const
        {
            return funcObject_;
        }

        bool IsOneShotTimer() const
        {
            return oneShotTimer_;
        }

    private:
        uint32_t id_;
        ets_object funcObject_;
        bool oneShotTimer_;

        NO_COPY_SEMANTIC(TimerInfo);
        NO_MOVE_SEMANTIC(TimerInfo);
    };

    static void TimerCallback(uv_timer_t *timer);
    static void DisarmTimer(uv_timer_t *timer);
    static void FreeTimer(uv_handle_t *timer);
    static uv_loop_t *GetMainEventLoop();
    static uint32_t GetTimerId();
    static bool CheckMainThread(EtsEnv *env);

    static pid_t mainTid_;
    static EtsEnv *mainEtsEnv_;
    static napi_env jsEnv_;
    static std::unordered_map<uint32_t, uv_timer_t *> timers_;
    static uint32_t nextTimerId_;
};

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TIMER_MODULE_H
