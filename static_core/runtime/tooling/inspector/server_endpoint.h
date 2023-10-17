/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_SERVER_ENDPOINT_H
#define PANDA_TOOLING_INSPECTOR_SERVER_ENDPOINT_H

#include "endpoint.h"
#include "server.h"

#include "websocketpp/server.hpp"

namespace panda::tooling::inspector {
template <typename Config>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ServerEndpoint : public Endpoint<websocketpp::server<Config>>, public Server {
public:
    ServerEndpoint() noexcept;

    void OnValidate(std::function<void()> &&handler) override
    {
        on_validate_ = std::move(handler);
    }

    void OnOpen(std::function<void()> &&handler) override
    {
        on_open_ = std::move(handler);
    }

    void OnFail(std::function<void()> &&handler) override
    {
        on_fail_ = std::move(handler);
    }

    using Server::Call;
    void Call(const std::string &session_id, const char *method,
              std::function<void(JsonObjectBuilder &)> &&params) override;

    void OnCall(const char *method,
                std::function<void(const std::string &session_id, JsonObjectBuilder &result, const JsonObject &params)>
                    &&handler) override;

private:
    std::function<void()> on_validate_ = []() {};
    std::function<void()> on_open_ = []() {};
    std::function<void()> on_fail_ = []() {};
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_SERVER_ENDPOINT_H
