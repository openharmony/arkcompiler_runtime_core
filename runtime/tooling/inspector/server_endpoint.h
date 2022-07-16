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
class ServerEndpoint : public Endpoint<websocketpp::server<Config>>, public Server {
public:
    ServerEndpoint() noexcept;

    using Server::Call;
    void Call(const char *method, std::function<void(JsonObjectBuilder &)> &&params) override;

    void OnCall(const char *method,
                std::function<void(JsonObjectBuilder &result, const JsonObject &params)> &&handler) override;
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_SERVER_ENDPOINT_H
