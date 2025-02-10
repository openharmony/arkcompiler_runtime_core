/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "json_serialization/serializable.h"
#include "libpandabase/utils/expected.h"
#include "libpandabase/utils/json_builder.h"

#include "connection/event_loop.h"
#include "json_serialization/jrpc_error.h"

namespace ark {
class JsonObject;
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {
class Server : public virtual EventLoop {  // NOLINT(fuchsia-virtual-inheritance)
public:
    using MethodResponse = Expected<std::unique_ptr<JsonSerializable>, JRPCError>;
    using Handler = std::function<MethodResponse(const std::string &, const JsonObject &params)>;

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

    virtual void OnCall(const char *method, Handler &&handler) = 0;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_H
