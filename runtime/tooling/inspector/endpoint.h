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

#ifndef PANDA_TOOLING_INSPECTOR_ENDPOINT_H
#define PANDA_TOOLING_INSPECTOR_ENDPOINT_H

#include "macros.h"
#include "utils/json_builder.h"
#include "utils/logger.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/frame.hpp"
#pragma GCC diagnostic pop

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace panda {
class JsonObject;
}  // namespace panda

namespace panda::tooling::inspector {
class EndpointBase {
public:
    using Id = double;

private:
    using MethodHandler = std::function<void(std::optional<Id>, const JsonObject &params)>;
    using ResultHandler = std::function<void(const JsonObject &)>;

public:
    void OnCall(const char *method, MethodHandler &&handler)
    {
        methodHandlers_[method] = std::move(handler);
    }

protected:
    void HandleMessage(const std::string &message);

    void OnResult(Id id, ResultHandler &&handler)
    {
        resultHandlers_[id] = std::move(handler);
    }

private:
    std::unordered_map<std::string, MethodHandler> methodHandlers_;
    std::unordered_map<Id, ResultHandler> resultHandlers_;
};

// JSON-RPC endpoint handling the Inspector protocol.
template <typename WsEndpoint>
class Endpoint : public EndpointBase {
public:
    Endpoint()
    {
        endpoint_.set_message_handler([this](auto /* hdl */, auto message) { HandleMessage(message->get_payload()); });
    }

protected:
    void Call(
        std::optional<Id> id, const char *method,
        std::function<void(JsonObjectBuilder &)> &&params = [](JsonObjectBuilder & /* builder */) {})
    {
        Send([id, method, &params](JsonObjectBuilder &call) {
            if (id) {
                call.AddProperty("id", *id);
            }

            call.AddProperty("method", method);
            call.AddProperty("params", std::move(params));
        });
    }

    template <typename Result>
    void Reply(Id id, Result &&result)
    {
        Send([id, &result](JsonObjectBuilder &reply) {
            reply.AddProperty("id", id);
            reply.AddProperty("result", std::forward<Result>(result));
        });
    }

    typename WsEndpoint::connection_ptr GetPinnedConnection()
    {
        ASSERT_PRINT(connection_, "No pinned connection");
        return connection_;
    }

    bool Pin(const websocketpp::connection_hdl &hdl)
    {
        if (connection_) {
            return false;
        }

        connection_ = endpoint_.get_con_from_hdl(hdl);
        return true;
    }

    void Unpin(const websocketpp::connection_hdl &hdl)
    {
        (void)hdl;
        CHECK_EQ(hdl.lock(), connection_);
        connection_.reset();
    }

    WsEndpoint endpoint_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    template <typename BuildFunction>
    void Send(BuildFunction &&build)
    {
        JsonObjectBuilder builder;
        build(builder);
        auto message = std::move(builder).Build();
        LOG(DEBUG, DEBUGGER) << "Sending " << message;
        GetPinnedConnection()->send(message, websocketpp::frame::opcode::text);
    }

    typename WsEndpoint::connection_ptr connection_;
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_ENDPOINT_H
