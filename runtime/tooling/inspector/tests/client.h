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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_CLIENT_H
#define PANDA_TOOLING_INSPECTOR_TEST_CLIENT_H

#include "../endpoint.h"
#include "test_config.h"
#include "test_event_loop.h"

#include "websocketpp/client.hpp"

#include <functional>
#include <string_view>
#include <system_error>
#include <utility>

namespace panda {
class JsonObject;
class JsonObjectBuilder;
}  // namespace panda

namespace panda::tooling::inspector::test {
class TestLogger;
class TestServer;

class Client : public Endpoint<websocketpp::client<TestConfig>>, public TestEventLoop {
public:
    enum Error {
        CONNECTION_ALREADY_PINNED = 1,
        CONNECTION_FAILED,
    };

    Client(std::string_view name, TestLogger &logger) : TestEventLoop(name, logger) {}

    void Call(
        const char *method, std::function<void(JsonObjectBuilder &)> &&params = [](auto & /* builder */) {},
        std::function<void(const JsonObject &)> &&handler = [](auto & /* result */) {});

    void Call(const char *method, std::function<void(const JsonObject & /* result */)> &&handler)
    {
        Call(
            method, [](auto & /* builder */) {}, std::move(handler));
    }

    std::error_code Close();

    void Connect(TestServer &server, std::function<void(std::error_code)> &&cb);

    template <typename MethodHandler>
    void OnCall(const char *method, MethodHandler &&handler)
    {
        Endpoint::OnCall(method, std::bind(std::forward<MethodHandler>(handler), std::placeholders::_2));
    }

private:
    unsigned id_ {0};
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_CLIENT_H
