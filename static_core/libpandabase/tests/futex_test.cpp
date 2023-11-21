/*
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

#include "platforms/unix/libpandabase/futex/fmutex.h"

#include "gtest/gtest.h"
#include "utils/logger.h"

namespace panda {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
struct panda::os::unix::memory::futex::fmutex FUTEX;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
struct panda::os::unix::memory::futex::CondVar CONDVAR;
volatile int GLOBAL;

constexpr std::chrono::duration DELAY = std::chrono::milliseconds(10U);

// The thread just modifies shared data under locks
void Writer()
{
    MutexLock(&FUTEX, false);
    int new_val = GLOBAL + 1U;
    std::this_thread::sleep_for(DELAY);
    GLOBAL = new_val;
    MutexUnlock(&FUTEX);
}

// The thread modifies shared data (meaning it reached Wait function)
// After wake it modifies the shared data again
// There should not be any other writer in parallel
void Waiter()
{
    MutexLock(&FUTEX, false);
    GLOBAL++;
    int old_val = GLOBAL;
    do {
        // spurious wake is possible
        Wait(&CONDVAR, &FUTEX);
    } while (old_val == GLOBAL);
    MutexUnlock(&FUTEX);
    int new_val = GLOBAL + 1U;
    // Check that waiter is correctly waken
    ASSERT_EQ(new_val, GLOBAL + 1U);
    GLOBAL = new_val;
}

// The thread modifies shared data (meaning it reached Wait function)
// After wake it modifies the shared data again under lock
// There may be other writers in parallel
void Syncwaiter()
{
    MutexLock(&FUTEX, false);
    GLOBAL++;
    int old_val = GLOBAL;
    do {
        // spurious wake is possible
        Wait(&CONDVAR, &FUTEX);
    } while (old_val == GLOBAL);
    int new_val = GLOBAL + 1U;
    // Check that waiter is correctly waken
    ASSERT_EQ(new_val, GLOBAL + 1U);
    GLOBAL = new_val;
    MutexUnlock(&FUTEX);
}

// The thread modifies shared data (meaning it reached TimedWait function)
// Likely it is not waken by signal and is interrupted by timeout
void Timedwaiter()
{
    MutexLock(&FUTEX, false);
    GLOBAL++;
    bool ret = TimedWait(&CONDVAR, &FUTEX, 1U, 0U, false);
    ASSERT_TRUE(ret);
    if (ret) {
        // Timeout
        GLOBAL++;
        MutexUnlock(&FUTEX);
        return;
    }
    MutexUnlock(&FUTEX);
}

class FutexTest : public testing::Test {
public:
    FutexTest()
    {
        MutexInit(&FUTEX);
        ConditionVariableInit(&CONDVAR);
    }

    ~FutexTest() override
    {
        MutexDestroy(&FUTEX);
        ConditionVariableDestroy(&CONDVAR);
    }

    NO_COPY_SEMANTIC(FutexTest);
    NO_MOVE_SEMANTIC(FutexTest);

protected:
};

// The test checks basic lock protection
// If two values in critical section differ - it is an error
TEST(FutexTest, LockUnlockTest)
{
    GLOBAL = 0;
    std::thread thr(Writer);
    MutexLock(&FUTEX, false);
    int val1 = GLOBAL;
    // Wait a bit, to get a chance to Writer
    std::this_thread::sleep_for(DELAY);
    int val2 = GLOBAL;
    MutexUnlock(&FUTEX);
    // Check that Writer cannot change GLOBAL in critical section
    ASSERT_EQ(val1, val2);
    thr.join();
    val1 = GLOBAL;
    // Check that both Writer and main correctly incremented in critical section
    ASSERT_EQ(val1, 1U);
}

// The test checks trylock operation
TEST(FutexTest, TrylockTest)
{
    GLOBAL = 0;
    std::thread thr(Writer);
    do {
    } while (!MutexLock(&FUTEX, true));
    int new_val = GLOBAL + 1U;
    // Wait a bit, to get a chance to Writer
    std::this_thread::sleep_for(DELAY);
    GLOBAL = new_val;
    MutexUnlock(&FUTEX);
    thr.join();
    int val1 = GLOBAL;
    // Check that both Writer and main correctly incremented in critical section
    ASSERT_EQ(val1, 2U);
}

// The test checks work with multiple writers
TEST(FutexTest, MultiWriteTest)
{
    GLOBAL = 0;
    std::thread thr(Writer);
    std::thread thr2(Writer);
    std::thread thr3(Writer);
    std::thread thr4(Writer);
    std::thread thr5(Writer);
    thr.join();
    thr2.join();
    thr3.join();
    thr4.join();
    thr5.join();
    int val1 = GLOBAL;
    // Check that threads correctly incremented in critical section
    ASSERT_EQ(val1, 5U);
}

// The test checks work with recursive futexes
TEST(FutexTest, RecursiveFutexTest)
{
    GLOBAL = 0;
    FUTEX.recursive_mutex = true;
    std::thread thr(Writer);
    MutexLock(&FUTEX, false);
    MutexLock(&FUTEX, false);
    int val1 = GLOBAL;
    MutexUnlock(&FUTEX);
    // Wait a bit, to get a chance to Writer
    std::this_thread::sleep_for(DELAY);
    // Read after the first unlock to check, if we are still in critical section
    int val2 = GLOBAL;
    MutexUnlock(&FUTEX);
    thr.join();
    // Check that Writer cannot change GLOBAL in recursive critical section
    ASSERT_EQ(val1, val2);
    FUTEX.recursive_mutex = false;
}

// The test checks basic wait-notify actions
// We start Waiter, which reaches Wait() function
// Then main send SignalCount
TEST(FutexTest, WaitNotifyTest)
{
    GLOBAL = 0;
    std::thread thr(Waiter);
    bool cond = false;
    do {
        // MutexLock is neccessary to avoid interleaving before Wait()
        MutexLock(&FUTEX, false);
        cond = GLOBAL == 1;
        MutexUnlock(&FUTEX);
        std::this_thread::sleep_for(DELAY);
    } while (!cond);
    // Waiter reached Wait()

    // Lock for TSAN
    MutexLock(&FUTEX, false);
    GLOBAL++;
    MutexUnlock(&FUTEX);
    SignalCount(&CONDVAR, panda::os::unix::memory::futex::WAKE_ONE);
    thr.join();
    GLOBAL++;
    // Check that waiter is correctly finished
    ASSERT_EQ(GLOBAL, 4U);
}

// The test checks basic timedwait-notify actions
// We start Waiter, which reaches Timedwait() function and then exits by timeout
// Then main send SignalCount
TEST(FutexTest, TimedwaitTest)
{
    GLOBAL = 0;
    std::thread thr(Timedwaiter);
    // Just wait for timeout
    thr.join();
    GLOBAL++;
    // Check that waiter is correctly finished
    ASSERT_EQ(GLOBAL, 3U);
}

// The test checks wait-notifyAll operations in case of multiple waiters
// We start Waiters, which reach Wait() function
// Then main send SignalAll to waiters
TEST(FutexTest, WakeAllTest)
{
    GLOBAL = 0;
    constexpr int NUM = 5;
    std::thread thr(Syncwaiter);
    std::thread thr2(Syncwaiter);
    std::thread thr3(Syncwaiter);
    std::thread thr4(Syncwaiter);
    std::thread thr5(Syncwaiter);

    bool cond = false;
    do {
        // MutexLock is neccessary to avoid interleaving before Wait()
        MutexLock(&FUTEX, false);
        cond = GLOBAL == NUM;
        MutexUnlock(&FUTEX);
        std::this_thread::sleep_for(DELAY);
    } while (!cond);
    // All threads reached Wait

    // Lock for TSAN
    MutexLock(&FUTEX, false);
    GLOBAL++;
    MutexUnlock(&FUTEX);
    SignalCount(&CONDVAR, panda::os::unix::memory::futex::WAKE_ALL);

    thr.join();
    thr2.join();
    thr3.join();
    thr4.join();
    thr5.join();
    GLOBAL++;
    // Check that waiter is correctly finished by timeout
    ASSERT_EQ(GLOBAL, 2U * NUM + 2U);
}

}  // namespace panda
