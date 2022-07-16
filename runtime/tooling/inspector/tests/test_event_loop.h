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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_TEST_EVENT_LOOP_H
#define PANDA_TOOLING_INSPECTOR_TEST_TEST_EVENT_LOOP_H

#include "../event_loop.h"
#include "test_config.h"

#include "websocketpp/connection.hpp"

#include <deque>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace panda::tooling::inspector::test {
class TestLogger;

class TestEventLoop : public virtual EventLoop {
public:
    TestEventLoop(std::string_view name, TestLogger &logger) : logger_(logger), name_(name) {}

    const std::string &GetName() const
    {
        return name_;
    }

    bool Poll() override;

    void Push(const websocketpp::connection<TestConfig>::ptr &connection, std::vector<char> &&buffer)
    {
        messages_.emplace_back(connection, std::move(buffer));
    }

    bool RunOne() override;

private:
    using Message = std::tuple<websocketpp::connection<TestConfig>::ptr, std::vector<char>>;

    TestLogger &logger_;
    std::string name_;
    std::deque<Message> messages_;
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_TEST_EVENT_LOOP_H
