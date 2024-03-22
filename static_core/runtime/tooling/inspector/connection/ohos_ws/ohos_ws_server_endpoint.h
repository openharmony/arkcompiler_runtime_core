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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_WS_OHOS_WS_SERVER_ENDPOINT_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_WS_OHOS_WS_SERVER_ENDPOINT_H

#include "tooling/inspector/connection/server_endpoint_base.h"

#include "server/websocket_server.h"

namespace ark::tooling::inspector {
// Server endpoint based on OHOS websocket implementation
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class OhosWsServerEndpoint : public ServerEndpointBase {
protected:
    using Endpoint = OHOS::ArkCompiler::Toolchain::WebSocketServer;

public:
    OhosWsServerEndpoint() noexcept;

protected:
    Endpoint endpoint_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    void SendMessage(const std::string &message) override
    {
        if (endpoint_.IsConnected()) {
            endpoint_.SendReply(message);
        }
    }
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_WS_OHOS_WS_SERVER_ENDPOINT_H
