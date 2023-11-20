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

#include "server_endpoint_base.h"

#include "utils/logger.h"

namespace ark::tooling::inspector {
void ServerEndpointBase::Call(const std::string &sessionId, const char *method,
                              std::function<void(JsonObjectBuilder &)> &&params)
{
    EndpointBase::Call(sessionId, std::nullopt, method, std::move(params));
}

void ServerEndpointBase::OnCall(
    const char *method, std::function<void(const std::string &, JsonObjectBuilder &, const JsonObject &)> &&handler)
{
    EndpointBase::OnCall(method, [this, handler = std::move(handler)](auto &sessionId, auto id, auto &params) {
        if (!id) {
            LOG(INFO, DEBUGGER) << "Invalid request: request has no \"id\"";
            return;
        }

        Reply(sessionId, *id,
              std::bind(std::ref(handler), std::cref(sessionId),  // NOLINT(modernize-avoid-bind)
                        std::placeholders::_1, std::cref(params)));
    });
}
}  // namespace ark::tooling::inspector
