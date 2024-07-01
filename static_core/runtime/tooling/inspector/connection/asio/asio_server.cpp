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

#include "asio_server.h"
#include "asio_config.h"

#include "utils/logger.h"
#include "websocketpp/uri.hpp"

#include <memory>
#include <system_error>

#define CONFIG AsioConfig  // NOLINT(cppcoreguidelines-macro-usage)
#include "server_endpoint-inl.h"
#undef CONFIG

namespace ark::tooling::inspector {
bool AsioServer::Initialize()
{
    if (initialized_) {
        return true;
    }

    std::error_code ec;

    endpoint_.init_asio(ec);
    if (ec) {
        LOG(ERROR, DEBUGGER) << "Failed to initialize endpoint";
        return false;
    }

    endpoint_.set_reuse_addr(true);
    initialized_ = true;
    return true;
}

websocketpp::uri_ptr AsioServer::Start(uint32_t port)
{
    if (!Initialize()) {
        return nullptr;
    }

    std::error_code ec;

    endpoint_.listen(port, ec);
    if (ec) {
        LOG(ERROR, DEBUGGER) << "Failed to bind Inspector server on port " << port;
        return nullptr;
    }

    endpoint_.start_accept(ec);

    if (!ec) {
        auto ep = endpoint_.get_local_endpoint(ec);

        if (!ec) {
            LOG(INFO, DEBUGGER) << "Inspector server listening on " << ep;
            return std::make_shared<websocketpp::uri>(false, ep.address().to_string(), ep.port(), "/");
        }

        LOG(ERROR, DEBUGGER) << "Failed to get the TCP endpoint";
    } else {
        LOG(ERROR, DEBUGGER) << "Failed to start Inspector connection acceptance loop";
    }

    endpoint_.stop_listening(ec);
    return nullptr;
}

bool AsioServer::Stop()
{
    if (!Initialize()) {
        return false;
    }

    // Stop accepting new connections.
    Kill();
    // Close the current connection.
    Close();

    std::error_code ec;
    endpoint_.stop_listening(ec);
    return !ec;
}
}  // namespace ark::tooling::inspector
