/*
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

#include "gtest/gtest.h"
#include "os/thread.h"
#include "os/mutex.h"

namespace panda::os::thread {
class ThreadTest : public testing::Test {};

uint32_t CUR_THREAD_ID = 0;
bool UPDATED = false;
bool OPERATED = false;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::Mutex MU;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::ConditionVariable CV;

#ifdef PANDA_TARGET_UNIX
constexpr int LOWER_PRIOIRITY = 1;
#elif defined(PANDA_TARGET_WINDOWS)
constexpr int LOWER_PRIOIRITY = -1;
#endif

void ThreadFunc()
{
    CUR_THREAD_ID = GetCurrentThreadId();
    {
        os::memory::LockHolder lk(MU);
        UPDATED = true;
    }
    CV.Signal();
    {
        // wait for the main thread to Set/GetPriority
        os::memory::LockHolder lk(MU);
        while (!OPERATED) {
            CV.Wait(&MU);
        }
    }
}

TEST_F(ThreadTest, SetCurrentThreadPriorityTest)
{
    // Since setting higher priority needs "sudo" right, we only test lower one here.
    auto ret1 = SetPriority(GetCurrentThreadId(), LOWER_PRIOIRITY);
    auto prio1 = GetPriority(GetCurrentThreadId());
    ASSERT_EQ(prio1, LOWER_PRIOIRITY);

    auto ret2 = SetPriority(GetCurrentThreadId(), LOWEST_PRIORITY);
    auto prio2 = GetPriority(GetCurrentThreadId());
    ASSERT_EQ(prio2, LOWEST_PRIORITY);

#ifdef PANDA_TARGET_UNIX
    ASSERT_EQ(ret1, 0U);
    ASSERT_EQ(ret2, 0U);
#elif defined(PANDA_TARGET_WINDOWS)
    ASSERT_NE(ret1, 0U);
    ASSERT_NE(ret2, 0U);
#endif
}

TEST_F(ThreadTest, SetOtherThreadPriorityTest)
{
    auto parent_pid = GetCurrentThreadId();
    auto parent_prio_before = GetPriority(parent_pid);

    auto new_thread = ThreadStart(ThreadFunc);
    // wait for the new_thread to update CUR_THREAD_ID
    MU.Lock();
    while (!UPDATED) {
        CV.Wait(&MU);
    }
    auto child_pid = CUR_THREAD_ID;

    auto child_prio_before = GetPriority(child_pid);
    (void)child_prio_before;
    auto ret = SetPriority(child_pid, LOWEST_PRIORITY);

    auto child_prio_after = GetPriority(child_pid);
    (void)child_prio_after;
    auto parent_prio_after = GetPriority(parent_pid);

    OPERATED = true;
    MU.Unlock();
    CV.Signal();
    void *res;
    ThreadJoin(new_thread, &res);

    ASSERT_EQ(parent_prio_before, parent_prio_after);
#ifdef PANDA_TARGET_UNIX
    ASSERT_EQ(ret, 0U);
    ASSERT(child_prio_before <= child_prio_after);
#elif defined(PANDA_TARGET_WINDOWS)
    ASSERT_NE(ret, 0U);
    ASSERT(child_prio_after <= child_prio_before);
#endif
}
}  // namespace panda::os::thread
