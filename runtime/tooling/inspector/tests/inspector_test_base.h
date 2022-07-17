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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_INSPECTOR_TEST_BASE_H
#define PANDA_TOOLING_INSPECTOR_TEST_INSPECTOR_TEST_BASE_H

#include "../inspector.h"
#include "client.h"
#include "combined_event_loop.h"
#include "test_debugger.h"
#include "test_logger.h"
#include "test_server.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace panda {
class Class;
class JsonObject;
class JsonObjectBuilder;
}  // namespace panda

namespace panda::tooling::inspector::test {
class InspectorTestBase : public testing::Test {
protected:
    virtual bool AttachDebugger() const
    {
        return true;
    }

    void SetUp() override;
    void TearDown() override;

    Class *LoadSourceFile(const std::string &source);

    template <typename Function, typename Result = std::invoke_result_t<Function, const JsonObject &>,
              typename = std::enable_if_t<std::is_void_v<Result>>>
    void CallSync(const char *method, std::function<void(JsonObjectBuilder &)> &&params, Function &&handler)
    {
        client_.Call(method, std::move(params),
                     [&](auto &object) { std::invoke(std::forward<Function>(handler), object); });
        (server_ + client_).Poll();
    }

    void CallSync(const char *method, std::function<void(JsonObjectBuilder &)> &&params)
    {
        CallSync(method, std::move(params), [](auto & /* result */) {});
    }

    template <typename Function, typename Result = std::invoke_result_t<Function, const JsonObject &>,
              typename = std::enable_if_t<!std::is_void_v<Result>>>
    Result CallSync(const char *method, std::function<void(JsonObjectBuilder &)> &&params, Function &&handler)
    {
        Result result;
        CallSync(method, std::move(params),
                 [&](auto &object) { result = std::invoke(std::forward<Function>(handler), object); });
        return result;
    }

    TestLogger logger_ {TestLogger::FAIL_ON_ERROR | TestLogger::OUTPUT_ON_FAIL};
    testing::StrictMock<TestDebugger> debugger_;
    Client client_ {"client", logger_};
    TestServer server_ {"server", logger_};
    Inspector inspector_ {server_, &debugger_};

private:
    virtual void SetUpSourceFiles() {}
};
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_INSPECTOR_TEST_BASE_H
