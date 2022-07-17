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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_EXPECT_PAUSES_H
#define PANDA_TOOLING_INSPECTOR_TEST_EXPECT_PAUSES_H

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

namespace panda {
class Method;
}  // namespace panda

namespace panda::tooling::inspector::test {
class Client;
class TestMethod;

struct Pause {
    struct CallFrame {
        const char *functionName;
        size_t lineNumber;
    };

    const char *reason;
    std::vector<CallFrame> callFrames;
};

Pause::CallFrame CallFrame(Method *method, size_t lineNumber);
Pause::CallFrame CallFrame(const TestMethod &method, size_t lineNumber);

template <typename... CallFrame>
Pause BreakOnStart(CallFrame &&... callFrame)
{
    return {"Break on start", {std::forward<CallFrame>(callFrame)...}};
}

template <typename... CallFrame>
Pause Breakpoint(CallFrame &&... callFrame)
{
    return {"Breakpoint", {std::forward<CallFrame>(callFrame)...}};
}

template <typename... CallFrame>
Pause Step(CallFrame &&... callFrame)
{
    return {"Step", {std::forward<CallFrame>(callFrame)...}};
}

void ExpectPauses(Client &client, std::initializer_list<Pause> pauses);
}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_EXPECT_PAUSES_H
