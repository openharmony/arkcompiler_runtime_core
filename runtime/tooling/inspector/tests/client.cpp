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

#include "client.h"
#include "test_server.h"

#include "utils/logger.h"
#include "websocketpp/close.hpp"
#include "websocketpp/uri.hpp"

#include <memory>
#include <system_error>
#include <utility>

namespace panda::tooling::inspector::test {
class ClientCategory : public std::error_category {
public:
    const char *name() const noexcept override
    {
        return "client";
    }

    std::string message(int code) const override
    {
        switch (code) {
            case Client::Error::CONNECTION_ALREADY_PINNED:
                return "Connection already pinned";
            case Client::Error::CONNECTION_FAILED:
                return "Connection failed";
            default:
                return "Unknown";
        }
    }

    static std::error_code Encode(Client::Error error)
    {
        static ClientCategory category;
        return {error, category};
    }
};

void Client::Call(const char *method, std::function<void(JsonObjectBuilder &)> &&params,
                  std::function<void(const JsonObject &)> &&handler)
{
    Endpoint::Call(++id_, method, std::move(params));
    OnResult(id_, std::move(handler));
}

std::error_code Client::Close()
{
    std::error_code ec;
    GetPinnedConnection()->close(websocketpp::close::status::normal, "", ec);
    return ec;
}

void Client::Connect(TestServer &server, std::function<void(std::error_code)> &&cb)
{
    std::error_code ec;

    auto connection = endpoint_.get_connection(std::make_shared<websocketpp::uri>(false, server.GetName(), ""), ec);

    if (ec) {
        return cb(ec);
    }

    connection->set_open_handler([this, cb](auto hdl) {
        if (!Pin(hdl)) {
            return cb(ClientCategory::Encode(CONNECTION_ALREADY_PINNED));
        }

        cb(std::error_code());
    });

    connection->set_fail_handler([this, cb](auto hdl) {
        LOG(INFO, DEBUGGER) << "Failed to connect: " << endpoint_.get_con_from_hdl(hdl)->get_response().get_body();

        cb(ClientCategory::Encode(CONNECTION_FAILED));
    });

    server.Connect(connection, *this);
}
}  // namespace panda::tooling::inspector::test
