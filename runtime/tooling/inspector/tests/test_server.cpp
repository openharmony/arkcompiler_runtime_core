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

#include "test_server.h"
#include "test_config.h"

#include "websocketpp/connection.hpp"

#include <cstddef>
#include <system_error>

#define CONFIG test::TestConfig
#include "../server_endpoint-inl.h"
#undef CONFIG

namespace panda::tooling::inspector::test {
TestServer::TestServer(std::string_view name, TestLogger &logger) : TestEventLoop(name, logger) {}

void TestServer::Connect(const websocketpp::connection<TestConfig>::ptr &clientConnection,
                         TestEventLoop &clientEventLoop)
{
    auto serverConnection = endpoint_.get_connection();

    using weak = websocketpp::connection<TestConfig>::weak_ptr;

    serverConnection->set_write_handler(
        [connection = weak(clientConnection), &clientEventLoop](auto /* hdl */, const char *buffer, size_t size) {
            clientEventLoop.Push(connection.lock(), {buffer, buffer + size});
            return std::error_code();
        });

    clientConnection->set_write_handler(
        [this, connection = weak(serverConnection)](auto /* hdl */, const char *buffer, size_t size) {
            Push(connection.lock(), {buffer, buffer + size});
            return std::error_code();
        });

    serverConnection->start();
    clientConnection->start();
}
}  // namespace panda::tooling::inspector::test
