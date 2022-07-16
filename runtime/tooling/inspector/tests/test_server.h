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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_TEST_SERVER_H
#define PANDA_TOOLING_INSPECTOR_TEST_TEST_SERVER_H

#include "../server_endpoint.h"
#include "test_config.h"
#include "test_event_loop.h"

#include "websocketpp/connection.hpp"

#include <functional>
#include <string_view>
#include <utility>

namespace panda::tooling::inspector::test {
class TestLogger;

class TestServer : public ServerEndpoint<TestConfig>, public TestEventLoop {
public:
    TestServer(std::string_view name, TestLogger &logger);

    void Connect(const websocketpp::connection<TestConfig>::ptr &clientConnection, TestEventLoop &clientEventLoop);

    void OnConnect(std::function<void()> &&handler)
    {
        endpoint_.set_open_handler(std::bind(std::move(handler)));
    }
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_TEST_SERVER_H
