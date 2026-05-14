/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "libarkbase/os/thread.h"
#include "libarkbase/os/mutex.h"

namespace ark::os::thread {
class ThreadTest : public testing::Test {};

uint32_t g_curThreadId = 0;
bool g_updated = false;
bool g_operated = false;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::Mutex g_mu;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::ConditionVariable g_cv;

#ifdef PANDA_TARGET_UNIX
constexpr int LOWER_PRIOIRITY = 1;
#elif defined(PANDA_TARGET_WINDOWS)
constexpr int LOWER_PRIOIRITY = -1;
#endif
static constexpr uint32_t THREAD_NAME_MAX_LENGTH = 15;

void ThreadFunc()
{
    {
        os::memory::LockHolder lk(g_mu);
        g_curThreadId = GetCurrentThreadId();
        g_updated = true;
    }
    g_cv.Signal();
    {
        // wait for the main thread to Set/GetPriority
        os::memory::LockHolder lk(g_mu);
        while (!g_operated) {
            g_cv.Wait(&g_mu);
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

void ThreadSetNameFunc(const char *expectedName)
{
    {
        os::memory::LockHolder lk(g_mu);
        g_updated = true;
    }
    g_cv.Signal();

    {
        // wait for the main thread to SetThreadName
        os::memory::LockHolder lk(g_mu);
        while (!g_operated) {
            g_cv.Wait(&g_mu);
        }
    }
    NativeHandleType handle = GetNativeHandle();

    std::array<char, THREAD_NAME_MAX_LENGTH + 1> threadName = {0};
#ifdef PANDA_TARGET_UNIX
    auto ret = pthread_getname_np(handle, threadName.data(), threadName.size());
#elif defined(PANDA_TARGET_WINDOWS)
    auto ret = pthread_getname_np(reinterpret_cast<pthread_t>(handle), threadName.data(), threadName.size());
#endif
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(threadName.data(), expectedName);
}

TEST_F(ThreadTest, SetThreadNameTest)
{
    g_updated = false;
    g_operated = false;

    const char *threadName = "TestThread";
    auto newThread = ThreadStart(ThreadSetNameFunc, threadName);

    g_mu.Lock();
    while (!g_updated) {
        g_cv.Wait(&g_mu);
    }

    auto ret = SetThreadName(newThread, threadName);
    g_operated = true;
    g_mu.Unlock();
    g_cv.Signal();
    ASSERT_EQ(ret, 0);

    void *res;
    ThreadJoin(newThread, &res);
}

TEST_F(ThreadTest, SetThreadNameEmptyTest)
{
    g_updated = false;
    g_operated = false;

    const char *emptyName = "";
    auto newThread = ThreadStart(ThreadSetNameFunc, emptyName);
    g_mu.Lock();
    while (!g_updated) {
        g_cv.Wait(&g_mu);
    }

    auto ret = SetThreadName(newThread, emptyName);
    g_operated = true;
    g_mu.Unlock();
    g_cv.Signal();
    ASSERT_EQ(ret, 0);

    void *res;
    ThreadJoin(newThread, &res);
}

TEST_F(ThreadTest, SetThreadNameLongNameTest)
{
    g_updated = false;
    g_operated = false;

    const char *longName = "ThisIsAVeryLongThreadName";
    const char *expectedName = "ThisIsAVeryLong";
    auto newThread = ThreadStart(ThreadSetNameFunc, expectedName);
    g_mu.Lock();
    while (!g_updated) {
        g_cv.Wait(&g_mu);
    }

    auto ret = SetThreadName(newThread, longName);
    g_operated = true;
    g_mu.Unlock();
    g_cv.Signal();
    ASSERT_EQ(ret, 0);

    void *res;
    ThreadJoin(newThread, &res);
}

TEST_F(ThreadTest, SetThreadNameExact16Test)
{
    g_updated = false;
    g_operated = false;

    const char *exact16Name = "Exactly16Chars!";
    const char *expectedName = "Exactly16Chars!";
    auto newThread = ThreadStart(ThreadSetNameFunc, expectedName);
    g_mu.Lock();
    while (!g_updated) {
        g_cv.Wait(&g_mu);
    }

    auto ret = SetThreadName(newThread, exact16Name);
    g_operated = true;
    g_mu.Unlock();
    g_cv.Signal();
    ASSERT_EQ(ret, 0);

    void *res;
    ThreadJoin(newThread, &res);
}

TEST_F(ThreadTest, SetThreadNameExact15Test)
{
    g_updated = false;
    g_operated = false;

    const char *exact15Name = "Exactly15Chars";
    auto newThread = ThreadStart(ThreadSetNameFunc, exact15Name);
    g_mu.Lock();
    while (!g_updated) {
        g_cv.Wait(&g_mu);
    }

    auto ret = SetThreadName(newThread, exact15Name);
    g_operated = true;
    g_mu.Unlock();
    g_cv.Signal();
    ASSERT_EQ(ret, 0);

    void *res;
    ThreadJoin(newThread, &res);
}

TEST_F(ThreadTest, SetThreadNameWithSpecialCharsTest)
{
    g_updated = false;
    g_operated = false;

    const char *specialName = "Thread-123_ABC";
    auto newThread = ThreadStart(ThreadSetNameFunc, specialName);
    g_mu.Lock();
    while (!g_updated) {
        g_cv.Wait(&g_mu);
    }

    auto ret = SetThreadName(newThread, specialName);
    g_operated = true;
    g_mu.Unlock();
    g_cv.Signal();
    ASSERT_EQ(ret, 0);

    void *res;
    ThreadJoin(newThread, &res);
}
}  // namespace ark::os::thread
