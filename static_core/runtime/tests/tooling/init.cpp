/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <memory>
#include <thread>

#include "runtime/include/tooling/debug_interface.h"
#include "runtime/tests/tooling/test_runner.h"
#include "test_util.h"

namespace panda::tooling::test {
extern const char *GetCurrentTestName();

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::thread G_DEBUGGER_THREAD;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::unique_ptr<TestRunner> G_RUNNER {nullptr};

extern "C" int StartDebugger(uint32_t /* port */, bool /* break_on_start */)
{
    const char *test_name = GetCurrentTestName();
    G_RUNNER = std::make_unique<TestRunner>(test_name);
    G_DEBUGGER_THREAD = std::thread([] {
        TestUtil::WaitForInit();
        G_RUNNER->Run();
    });
    return 0;
}

extern "C" int StopDebugger()
{
    G_DEBUGGER_THREAD.join();
    G_RUNNER->TerminateTest();
    G_RUNNER.reset();
    return 0;
}
}  // namespace panda::tooling::test
