/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_H

#include <functional>

#include "connection/event_loop.h"

namespace ark {
class JsonObject;
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {
class Server : public virtual EventLoop {  // NOLINT(fuchsia-virtual-inheritance)
public:
    virtual void OnValidate(std::function<void()> &&handler) = 0;
    virtual void OnOpen(std::function<void()> &&handler) = 0;
    virtual void OnFail(std::function<void()> &&handler) = 0;

    virtual void Call(const std::string &sessionId, const char *method,
                      std::function<void(JsonObjectBuilder &)> &&params) = 0;

    void Call(const char *method, std::function<void(JsonObjectBuilder &)> &&params)
    {
        Call({}, method, std::move(params));
    }

    void Call(const std::string &sessionId, const char *method)
    {
        Call(sessionId, method, [](JsonObjectBuilder & /* builder */) {});
    }

    void Call(const char *method)
    {
        Call({}, method, [](JsonObjectBuilder & /* builder */) {});
    }

    virtual void OnCall(const char *method, std::function<void(const std::string &sessionId, JsonObjectBuilder &result,
                                                               const JsonObject &params)> &&handler) = 0;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_H
