/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "libpandabase/utils/utf.h"
#include "libpandabase/panda_gen_options/generated/base_options.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/runtime.h"
#include "runtime/scheduler/worker_thread.h"
#include "runtime/scheduler/task.h"
#include "runtime/tests/class_linker_test_extension.h"

namespace ark::scheduler::test {

class WorkerThreadTest : public testing::Test {
public:
    WorkerThreadTest()
    {
        Logger::Initialize(base_options::Options(""));

        RuntimeOptions options;
        auto execPath = ark::os::file::File::GetExecutablePath();
        std::string pandaStdLib = execPath.Value() + "/../pandastdlib/arkstdlib.abc";
        options.SetBootPandaFiles({pandaStdLib});
        Runtime::Create(options);
        mainThread_ = Thread::GetCurrent();
        Thread::SetCurrent(nullptr);
    }

    ~WorkerThreadTest() override
    {
        Thread::SetCurrent(mainThread_);
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(WorkerThreadTest);
    NO_MOVE_SEMANTIC(WorkerThreadTest);

private:
    Thread *mainThread_ = nullptr;
};

TEST_F(WorkerThreadTest, SwitchTasksInOneWorkerThread)
{
    auto wt = WorkerThread::AttachThread();
    ASSERT_EQ(wt, WorkerThread::GetCurrent());
    auto tk1 = Task::Create(Runtime::GetCurrent()->GetPandaVM());

    tk1->SwitchFromWorkerThread();
    ASSERT_EQ(tk1, Task::GetCurrent());
    auto tk2 = Task::Create(Runtime::GetCurrent()->GetPandaVM());
    Task::SuspendCurrent();

    tk2->SwitchFromWorkerThread();
    ASSERT_EQ(tk2, Task::GetCurrent());
    Task::EndCurrent();

    tk1->SwitchFromWorkerThread();
    Task::EndCurrent();

    WorkerThread::DetachThread();
}

}  // namespace ark::scheduler::test
