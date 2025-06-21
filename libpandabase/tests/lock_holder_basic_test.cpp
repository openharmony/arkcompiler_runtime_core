/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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


#include "libpandabase/os/mutex.h"
#include <gtest/gtest.h>
#include <thread>

namespace panda {

class LockHolderBasicTest : public testing::Test {};

HWTEST_F(LockHolderBasicTest, LockHolderConstructorDestructorTest, testing::ext::TestSize.Level0)
{
    panda::os::memory::DummyLock lock;
    {
        panda::os::memory::LockHolder<panda::os::memory::DummyLock> lock_holder(lock);
    }
}

HWTEST_F(LockHolderBasicTest, ReadLockHolderConstructorDestructorTest, testing::ext::TestSize.Level0)
{
    panda::os::memory::DummyLock lock;
    {
        panda::os::memory::ReadLockHolder<panda::os::memory::DummyLock> read_lock_holder(lock);
    }
}

HWTEST_F(LockHolderBasicTest, WriteLockHolderConstructorDestructorTest, testing::ext::TestSize.Level0)
{
    panda::os::memory::DummyLock lock;
    {
        panda::os::memory::WriteLockHolder<panda::os::memory::DummyLock> write_lock_holder(lock);
    }
}

HWTEST_F(LockHolderBasicTest, LockHolderExclusivityTest, testing::ext::TestSize.Level0)
{
    panda::os::memory::Mutex lock;
    bool try_lock_success = false;

    {
        panda::os::memory::LockHolder<panda::os::memory::Mutex> lock_holder(lock);

        std::thread t([&lock, &try_lock_success]() NO_THREAD_SAFETY_ANALYSIS {
            try_lock_success = lock.TryLock();
            if (try_lock_success) {
                lock.Unlock();
            }
        });
        t.join();

        EXPECT_FALSE(try_lock_success);
    }

    EXPECT_TRUE(lock.TryLock());
    [&]() NO_THREAD_SAFETY_ANALYSIS {
        lock.Unlock();
    }();
}

HWTEST_F(LockHolderBasicTest, ReadLockPreventsWriteTest, testing::ext::TestSize.Level0)
{
    panda::os::memory::RWLock lock;
    bool try_read_lock_success = false;

    {
        panda::os::memory::ReadLockHolder<panda::os::memory::RWLock> read_lock_holder(lock);

        std::thread t([&lock, &try_read_lock_success]() NO_THREAD_SAFETY_ANALYSIS {
            try_read_lock_success = lock.TryWriteLock();
            if (try_read_lock_success) {
                lock.Unlock();
            }
        });
        t.join();

        EXPECT_FALSE(try_read_lock_success);
    }

    EXPECT_TRUE(lock.TryWriteLock());
    [&]() NO_THREAD_SAFETY_ANALYSIS {
        lock.Unlock();
    }();
}

HWTEST_F(LockHolderBasicTest, WriteLockPreventsReadAndWriteTest, testing::ext::TestSize.Level0)
{
    panda::os::memory::RWLock lock;
    bool try_read_lock_success = false;
    bool try_write_lock_success = false;

    {
        panda::os::memory::WriteLockHolder<panda::os::memory::RWLock> write_lock_holder(lock);

        std::thread t([&]() NO_THREAD_SAFETY_ANALYSIS {
            try_read_lock_success = lock.TryReadLock();
            try_write_lock_success = lock.TryWriteLock();
            if (try_read_lock_success) {
                lock.Unlock();
            }
            if (try_write_lock_success) {
                lock.Unlock();
            }
        });
        t.join();

        EXPECT_FALSE(try_read_lock_success);
        EXPECT_FALSE(try_write_lock_success);
    }

    EXPECT_TRUE(lock.TryWriteLock());
    [&]() NO_THREAD_SAFETY_ANALYSIS {
        lock.Unlock();
    }();
}
} // namespace ark::os::memory::test
