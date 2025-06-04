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

#include "connection/server_endpoint_base.h"

#include "utils/logger.h"

namespace ark::tooling::inspector {

/// Empty response, returned on success
class EmptyResponse final : public JsonSerializable {
public:
    EmptyResponse() = default;

    DEFAULT_COPY_SEMANTIC(EmptyResponse);
    DEFAULT_MOVE_SEMANTIC(EmptyResponse);

    ~EmptyResponse() override = default;

    void Serialize([[maybe_unused]] JsonObjectBuilder &builder) const override {}
};

void ServerEndpointBase::Call(const std::string &sessionId, const char *method,
                              std::function<void(JsonObjectBuilder &)> &&params)
{
    EndpointBase::Call(sessionId, std::nullopt, method, std::move(params));
}

void ServerEndpointBase::OnCall(const char *method, Handler &&handler)
{
    EndpointBase::OnCall(method, [this, handler = std::move(handler)](auto &sessionId, auto id, auto &params) {
        if (!id) {
            LOG(INFO, DEBUGGER) << "Invalid request: request has no \"id\"";
            return;
        }

        // Execute the handler's code
        auto optResult = handler(sessionId, params);
        if (optResult) {
            if (*optResult) {
                JsonSerializable &res = **optResult;
                Reply(sessionId, *id, res);
            } else {
                Reply(sessionId, *id, EmptyResponse());
            }
        } else {
            ReplyError(sessionId, *id, std::move(optResult.Error()));
        }
    });
}
}  // namespace ark::tooling::inspector
