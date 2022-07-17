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

#include "../ws_logger.h"
#include "test_logger.h"

#include "utils/logger.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "websocketpp/logger/levels.hpp"

using testing::_;
using testing::Test;
using websocketpp::log::alevel;
using websocketpp::log::channel_type_hint;
using websocketpp::log::elevel;

namespace panda::tooling::inspector::test {
class WsLoggerTest : public Test {
protected:
    void SetUp() override
    {
        EXPECT_CALL(logger_, LogLineInternal).Times(0);
    }

    TestLogger logger_;
};

TEST_F(WsLoggerTest, SetsStaticChannels)
{
    WsLogger wsLogger(alevel::connect | alevel::disconnect, channel_type_hint::access);
    EXPECT_TRUE(wsLogger.static_test(alevel::connect));
    EXPECT_FALSE(wsLogger.static_test(alevel::app));
}

TEST_F(WsLoggerTest, SetsDynamicChannels)
{
    WsLogger wsLogger(alevel::connect | alevel::disconnect, channel_type_hint::access);
    EXPECT_FALSE(wsLogger.dynamic_test(alevel::connect));

    wsLogger.set_channels(alevel::connect);
    EXPECT_TRUE(wsLogger.dynamic_test(alevel::connect));
    EXPECT_FALSE(wsLogger.dynamic_test(alevel::disconnect));

    wsLogger.set_channels(alevel::disconnect | alevel::app);
    EXPECT_TRUE(wsLogger.dynamic_test(alevel::connect));
    EXPECT_TRUE(wsLogger.dynamic_test(alevel::disconnect));
    EXPECT_FALSE(wsLogger.dynamic_test(alevel::app));

    wsLogger.set_channels(alevel::none);
    EXPECT_FALSE(wsLogger.dynamic_test(alevel::connect));
    EXPECT_FALSE(wsLogger.dynamic_test(alevel::disconnect));

    wsLogger.set_channels(alevel::all);
    EXPECT_TRUE(wsLogger.dynamic_test(alevel::connect));
    EXPECT_TRUE(wsLogger.dynamic_test(alevel::disconnect));
    EXPECT_FALSE(wsLogger.dynamic_test(alevel::app));

    EXPECT_TRUE(wsLogger.static_test(alevel::connect));
    EXPECT_TRUE(wsLogger.static_test(alevel::disconnect));
    EXPECT_FALSE(wsLogger.static_test(alevel::app));
}

TEST_F(WsLoggerTest, WritesInAccessChannel)
{
    WsLogger wsLogger(alevel::connect, channel_type_hint::access);
    wsLogger.set_channels(alevel::connect);

    EXPECT_CALL(logger_, LogLineInternal(Logger::Level::INFO, Logger::Component::DEBUGGER, "connect: Connect"));
    wsLogger.write(alevel::connect, "Connect");

    wsLogger.write(alevel::app, "App");
}

TEST_F(WsLoggerTest, WritesInErrorChannel)
{
    WsLogger wsLogger(elevel::all, channel_type_hint::error);
    wsLogger.set_channels(elevel::all);

    EXPECT_CALL(logger_, LogLineInternal(Logger::Level::DEBUG, Logger::Component::DEBUGGER, "devel"));
    EXPECT_CALL(logger_, LogLineInternal(Logger::Level::INFO, Logger::Component::DEBUGGER, "library"));
    EXPECT_CALL(logger_, LogLineInternal(Logger::Level::INFO, Logger::Component::DEBUGGER, "info"));
    EXPECT_CALL(logger_, LogLineInternal(Logger::Level::WARNING, Logger::Component::DEBUGGER, "warning"));
    EXPECT_CALL(logger_, LogLineInternal(Logger::Level::ERROR, Logger::Component::DEBUGGER, "error"));

    for (auto level : {elevel::devel, elevel::library, elevel::info, elevel::warn, elevel::rerror}) {
        wsLogger.write(level, elevel::channel_name(level));
    }

    EXPECT_DEATH(wsLogger.write(elevel::fatal, "fatal"), _);
}
}  // namespace panda::tooling::inspector::test
