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

#ifndef PANDA_INTEROP_JS_GTEST_PLUGIN_TIMER_H
#define PANDA_INTEROP_JS_GTEST_PLUGIN_TIMER_H

#include <node_api.h>
#include <uv.h>
#include <vector>

namespace ark::ets::interop::js::helper {

struct TimerInfo {
    explicit TimerInfo(napi_env env, napi_ref cb, std::vector<napi_ref> cbArgs, bool repeat);
    TimerInfo(const TimerInfo &info) = delete;
    TimerInfo(TimerInfo &&info) = delete;
    TimerInfo &operator=(const TimerInfo &info) = delete;
    TimerInfo &operator=(TimerInfo &&info) = delete;
    ~TimerInfo();

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    napi_env env;
    napi_ref cb;
    std::vector<napi_ref> cbArgs;
    bool repeat;
    uv_timer_t *timer;
    uint32_t timerId;
    static constexpr uv_close_cb kTimerCloseCallback = [](uv_handle_t *handle) {
        delete reinterpret_cast<uv_timer_t *>(handle);
    };
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

napi_value SetTimeoutImpl(napi_env env, napi_callback_info info, bool repeat);
napi_value ClearTimerImpl(napi_env env, napi_callback_info info);

}  // namespace ark::ets::interop::js::helper

#endif  // PANDA_INTEROP_JS_GTEST_PLUGIN_TIMER_H
