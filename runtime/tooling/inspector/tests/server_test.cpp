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
#include "combined_event_loop.h"
#include "common.h"
#include "json_object_matcher.h"
#include "test_logger.h"
#include "test_server.h"

#include "gtest/gtest.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"
#include "gmock/gmock.h"

#include <functional>
#include <string_view>
#include <system_error>

using namespace std::literals::string_view_literals;
using testing::_;
using testing::Test;

namespace panda::tooling::inspector::test {
class ServerTest : public Test {
protected:
    TestLogger logger_ {TestLogger::FAIL_ON_ERROR | TestLogger::OUTPUT_ON_FAIL};
    TestServer server_ {"server", logger_};
    Client client_ {"client", logger_};
};

TEST_F(ServerTest, AcceptsOneConnectionAtATime)
{
    logger_.SetFlag(TestLogger::FAIL_ON_ERROR, false);

    unsigned serverConnected = 0;
    unsigned clientConnected = 0;
    unsigned clientFailed = 0;

    auto check = [&serverConnected, &clientConnected, &clientFailed]() {
        if (serverConnected < 1 || clientConnected + clientFailed < 3) {
            return;
        }

        EXPECT_EQ(serverConnected, 1);
        EXPECT_EQ(clientConnected, 1);
        EXPECT_EQ(clientFailed, 2);
    };

    server_.OnConnect([&]() {
        ++serverConnected;
        check();
    });

    auto onClientConnect = [&clientConnected, &clientFailed, &check](std::error_code ec) {
        if (ec) {
            ++clientFailed;
        } else {
            ++clientConnected;
        }
        check();
    };

    Client anotherClient("another client", logger_);
    Client yetAnotherClient("yet another client", logger_);

    client_.Connect(server_, onClientConnect);
    anotherClient.Connect(server_, onClientConnect);
    yetAnotherClient.Connect(server_, onClientConnect);

    (server_ + client_ + anotherClient + yetAnotherClient).Poll();
}

TEST_F(ServerTest, AcceptsSubsequentConnections)
{
    unsigned connected = 0;

    server_.OnConnect([&]() { ++connected; });

    for (auto clientName : {"first client"sv, "second client"sv, "third client"sv}) {
        Client client {clientName, logger_};

        client.Connect(server_, [&](std::error_code ec) {
            ASSERT_TRUE(IsOk(ec));
            ASSERT_TRUE(IsOk(client.Close()));
        });

        (server_ + client).Poll();
    }

    EXPECT_EQ(connected, 3);
}

TEST_F(ServerTest, FailsToCallWithoutConnection)
{
    EXPECT_DEATH(server_.Call("foo"), _);
}

TEST_F(ServerTest, Calls)
{
    server_.OnConnect([&]() {
        server_.Call("foo", [](JsonObjectBuilder &params) { params.AddProperty("x", "bar"); });
        server_.Call("bar");
    });

    client_.OnCall("foo", [](auto &params) {
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
        EXPECT_THAT(params, JsonProperties(JsonProperty<JsonObject::StringT> {"x", "bar"}));
    });

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
    client_.OnCall("bar", [](auto &params) { EXPECT_THAT(params, JsonProperties()); });

    client_.Connect(server_, [&](std::error_code ec) { ASSERT_TRUE(IsOk(ec)); });

    (server_ + client_).Poll();
}
TEST_F(ServerTest, Replies)
{
    server_.OnCall("next",
                   [value = 0](auto &result, auto & /* params */) mutable { result.AddProperty("value", value++); });

    std::function<void()> checkNext = [&, value = 0]() mutable {
        client_.Call("next", [&](const JsonObject &next) {
            // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
            EXPECT_THAT(next, JsonProperties(JsonProperty<JsonObject::NumT> {"value", value}));

            if (value == 5) {
                return;
            }

            value += 1;
            checkNext();
        });
    };

    client_.Connect(server_, [&](std::error_code ec) {
        ASSERT_TRUE(IsOk(ec));
        checkNext();
    });

    (server_ + client_).Poll();
}

TEST_F(ServerTest, Pong)
{
    server_.OnCall("ping", [&, ball = 0](auto & /* result */, auto &ping) mutable {
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
        EXPECT_THAT(ping, JsonProperties(JsonProperty<JsonObject::NumT> {"ball", ball}));

        server_.Call("pong", [ball](JsonObjectBuilder &pong) { pong.AddProperty("ball", ball + 1); });

        ball += 2;
    });

    client_.OnCall("pong", [&, ball = 1](auto &pong) mutable {
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) due to bug in clang-tidy #3553 (gtest repo)
        EXPECT_THAT(pong, JsonProperties(JsonProperty<JsonObject::NumT> {"ball", ball}));

        if (ball == 5) {
            return;
        }

        client_.Call("ping", [ball](JsonObjectBuilder &ping) { ping.AddProperty("ball", ball + 1); });

        ball += 2;
    });

    client_.Connect(server_, [&](std::error_code ec) {
        ASSERT_TRUE(IsOk(ec));
        client_.Call("ping", [](JsonObjectBuilder &ping) { ping.AddProperty("ball", 0); });
    });

    (server_ + client_).Poll();
}
}  // namespace panda::tooling::inspector::test
