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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_MACROS_H_
#define PANDA_LIBPANDABASE_PBASE_OS_MACROS_H_

#if defined(PANDA_USE_FUTEX)
#include "platforms/unix/libpandabase/futex/mutex.h"
#elif !defined(PANDA_TARGET_UNIX) && !defined(PANDA_TARGET_WINDOWS)
#error "Unsupported platform"
#endif

#include "clang.h"
#include "macros.h"

#include <atomic>
#include <pthread.h>

namespace panda::os::memory {

// Dummy lock which locks nothing
// but has the same methods as RWLock and Mutex.
// Can be used in Locks Holders.
class DummyLock {
public:
    void Lock() const {}
    void Unlock() const {}
    void ReadLock() const {}
    void WriteLock() const {}
};

#if defined(PANDA_USE_FUTEX)
using Mutex = ark::os::unix::memory::futex::Mutex;
using RecursiveMutex = ark::os::unix::memory::futex::RecursiveMutex;
using RWLock = ark::os::unix::memory::futex::RWLock;
using ConditionVariable = ark::os::unix::memory::futex::ConditionVariable;
#else
class ConditionVariable;

class CAPABILITY("mutex") Mutex {
public:
    PANDA_PUBLIC_API explicit Mutex(bool is_init = true);

    PANDA_PUBLIC_API ~Mutex();

    PANDA_PUBLIC_API void Lock() ACQUIRE();

    PANDA_PUBLIC_API bool TryLock() TRY_ACQUIRE(true);

    PANDA_PUBLIC_API void Unlock() RELEASE();

    // NOTE(mwx851039): Extract common part as an interface.
    static bool DoNotCheckOnTerminationLoop()
    {
        return no_check_for_deadlock_;
    }

    static void IgnoreChecksOnTerminationLoop()
    {
        no_check_for_deadlock_ = true;
    }

protected:
    void Init(pthread_mutexattr_t *attrs);

private:
    pthread_mutex_t mutex_;

    // This field is set to false in case of deadlock with daemon threads (only daemon threads
    // are not finished and they have state IS_BLOCKED). In this case we should terminate
    // those threads ignoring failures on lock structures destructors.
    PANDA_PUBLIC_API static std::atomic_bool no_check_for_deadlock_;

    NO_COPY_SEMANTIC(Mutex);
    NO_MOVE_SEMANTIC(Mutex);

    friend ConditionVariable;
};

class CAPABILITY("mutex") RecursiveMutex : public Mutex {
public:
    PANDA_PUBLIC_API RecursiveMutex();

    ~RecursiveMutex() = default;

    NO_COPY_SEMANTIC(RecursiveMutex);
    NO_MOVE_SEMANTIC(RecursiveMutex);
};

class CAPABILITY("mutex") RWLock {
public:
    PANDA_PUBLIC_API RWLock();

    PANDA_PUBLIC_API ~RWLock();

    PANDA_PUBLIC_API void ReadLock() ACQUIRE_SHARED();

    PANDA_PUBLIC_API void WriteLock() ACQUIRE();

    PANDA_PUBLIC_API bool TryReadLock() TRY_ACQUIRE_SHARED(true);

    PANDA_PUBLIC_API bool TryWriteLock() TRY_ACQUIRE(true);

    PANDA_PUBLIC_API void Unlock() RELEASE_GENERIC();

private:
    pthread_rwlock_t rwlock_;

    NO_COPY_SEMANTIC(RWLock);
    NO_MOVE_SEMANTIC(RWLock);
};

// Some RTOS could not have support for condition variables, so this primitive should be used carefully
class ConditionVariable {
public:
    PANDA_PUBLIC_API ConditionVariable();

    PANDA_PUBLIC_API ~ConditionVariable();

    PANDA_PUBLIC_API void Signal();

    PANDA_PUBLIC_API void SignalAll();

    PANDA_PUBLIC_API void Wait(Mutex *mutex);

    PANDA_PUBLIC_API bool TimedWait(Mutex *mutex, uint64_t ms, uint64_t ns = 0, bool is_absolute = false);

private:
    pthread_cond_t cond_;

    NO_COPY_SEMANTIC(ConditionVariable);
    NO_MOVE_SEMANTIC(ConditionVariable);
};
#endif  // PANDA_USE_FUTEX

using PandaThreadKey = pthread_key_t;
const auto PandaGetspecific = pthread_getspecific;     // NOLINT(readability-identifier-naming)
const auto PandaSetspecific = pthread_setspecific;     // NOLINT(readability-identifier-naming)
const auto PandaThreadKeyCreate = pthread_key_create;  // NOLINT(readability-identifier-naming)

template <class T, bool NEED_LOCK = true>
class SCOPED_CAPABILITY LockHolder {
public:
    explicit LockHolder(T &lock) ACQUIRE(lock) : lock_(lock)
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Lock();
        }
    }

    ~LockHolder() RELEASE()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(LockHolder);
    NO_MOVE_SEMANTIC(LockHolder);
};

template <class T, bool NEED_LOCK = true>
class SCOPED_CAPABILITY ReadLockHolder {
public:
    explicit ReadLockHolder(T &lock) ACQUIRE_SHARED(lock) : lock_(lock)
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.ReadLock();
        }
    }

    ~ReadLockHolder() RELEASE()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(ReadLockHolder);
    NO_MOVE_SEMANTIC(ReadLockHolder);
};

template <class T, bool NEED_LOCK = true>
class SCOPED_CAPABILITY WriteLockHolder {
public:
    explicit WriteLockHolder(T &lock) ACQUIRE(lock) : lock_(lock)
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.WriteLock();
        }
    }

    ~WriteLockHolder() RELEASE()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(WriteLockHolder);
    NO_MOVE_SEMANTIC(WriteLockHolder);
};

}  // namespace panda::os::memory

#endif  // PAND_LIBPANDABASE_PBASE_OS_MACROS_H_
