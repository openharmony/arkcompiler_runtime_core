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

#include "fmutex.h"

#ifdef MC_ON
#include "time.h"
#define FAIL_WITH_MESSAGE(m) ASSERT(0)
#define LOG_MESSAGE(l, m)
#define HELPERS_TO_UNSIGNED(m) m
extern "C" void __VERIFIER_assume(int) __attribute__((__nothrow__));
#else
#include "utils/logger.h"
#include "utils/type_helpers.h"
#define LOG_MESSAGE(l, m) LOG(l, COMMON) << (m)        // NOLINT(cppcoreguidelines-macro-usage)
#define FAIL_WITH_MESSAGE(m) LOG_MESSAGE(FATAL, m)     // NOLINT(cppcoreguidelines-macro-usage)
#define HELPERS_TO_UNSIGNED(m) helpers::ToUnsigned(m)  // NOLINT(cppcoreguidelines-macro-usage)
namespace panda::os::unix::memory::futex {
#endif

// This field is set to false in case of deadlock with daemon threads (only daemon threads
// are not finished and they have state IS_BLOCKED). In this case we should terminate
// those threads ignoring failures on lock structures destructors.
static ATOMIC(bool) DEADLOCK_FLAG = false;

#ifdef MC_ON
// GenMC does not support syscalls(futex)
static ATOMIC_INT FUTEX_SIGNAL;

static inline void FutexWait(ATOMIC_INT *m, int v)
{
    // Atomic with acquire order reason: mutex synchronization
    int s = ATOMIC_LOAD(&FUTEX_SIGNAL, memory_order_acquire);
    // Atomic with acquire order reason: mutex synchronization
    if (ATOMIC_LOAD(m, memory_order_acquire) != v) {
        return;
    }
    // Atomic with acquire order reason: mutex synchronization
    while (ATOMIC_LOAD(&FUTEX_SIGNAL, memory_order_acquire) == s) {
    }
}

static inline void FutexWake(void)
{
    // Atomic with release order reason: mutex synchronization
    ATOMIC_FETCH_ADD(&FUTEX_SIGNAL, 1, memory_order_release);
}
#else
// futex() is defined in header, as it is still included in different places
#endif

bool MutexDoNotCheckOnTerminationLoop()
{
    return DEADLOCK_FLAG;
}

void MutexIgnoreChecksOnTerminationLoop()
{
    DEADLOCK_FLAG = true;
}

int *GetStateAddr(struct fmutex *const m)
{
    return reinterpret_cast<int *>(&m->state_and_waiters);
}

void IncrementWaiters(struct fmutex *const m)
{
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_FETCH_ADD(&m->state_and_waiters, WAITER_INCREMENT, memory_order_relaxed);
}
void DecrementWaiters(struct fmutex *const m)
{
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_FETCH_SUB(&m->state_and_waiters, WAITER_INCREMENT, memory_order_relaxed);
}

int32_t GetWaiters(struct fmutex *const m)
{
    // Atomic with relaxed order reason: mutex synchronization
    return static_cast<int32_t>(static_cast<uint32_t>(ATOMIC_LOAD(&m->state_and_waiters, memory_order_relaxed)) >>
                                static_cast<uint32_t>(WAITER_SHIFT));
}

bool IsHeld(struct fmutex *const m, THREAD_ID thread)
{
    // Atomic with relaxed order reason: mutex synchronization
    return ATOMIC_LOAD(&m->exclusive_owner, memory_order_relaxed) == thread;
}

// Spin for small arguments and yield for longer ones.
static void BackOff(uint32_t i)
{
    static constexpr uint32_t SPIN_MAX = 10;
    if (i <= SPIN_MAX) {
        volatile uint32_t x = 0;  // Volatile to make sure loop is not optimized out.
        const uint32_t spin_count = 10 * i;
        for (uint32_t spin = 0; spin < spin_count; spin++) {
            ++x;
        }
    } else {
#ifndef MC_ON
        thread::Yield();
#endif
    }
}

// Wait until pred is true, or until timeout is reached.
// Return true if the predicate test succeeded, false if we timed out.
static inline bool WaitBrieflyFor(ATOMIC_INT *addr)
{
#ifndef MC_ON
    // We probably don't want to do syscall (switch context) when we use WaitBrieflyFor
    static constexpr uint32_t MAX_BACK_OFF = 10;
    static constexpr uint32_t MAX_ITER = 50;
    for (uint32_t i = 1; i <= MAX_ITER; i++) {
        BackOff(MIN(i, MAX_BACK_OFF));
#endif
        // Atomic with relaxed order reason: mutex synchronization
        int state = ATOMIC_LOAD(addr, memory_order_relaxed);
        if ((HELPERS_TO_UNSIGNED(state) & HELPERS_TO_UNSIGNED(HELD_MASK)) == 0) {
            return true;
        }
#ifndef MC_ON
    }
#endif
    return false;
}

void MutexInit(struct fmutex *const m)
{
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_STORE(&m->exclusive_owner, 0, memory_order_relaxed);
    m->recursive_count = 0;
    m->recursive_mutex = false;
    // Atomic with release order reason: mutex synchronization
    ATOMIC_STORE(&m->state_and_waiters, 0, memory_order_release);
}

void MutexDestroy(struct fmutex *const m)
{
#ifndef PANDA_TARGET_MOBILE
    // We can ignore these checks on devices because we use zygote and do not destroy runtime.
    if (!MutexDoNotCheckOnTerminationLoop()) {
#endif  // PANDA_TARGET_MOBILE
        // Atomic with relaxed order reason: mutex synchronization
        if (ATOMIC_LOAD(&m->state_and_waiters, memory_order_relaxed) != 0) {
            FAIL_WITH_MESSAGE("Mutex destruction failed; state_and_waiters is non zero!");
            // Atomic with relaxed order reason: mutex synchronization
        } else if (ATOMIC_LOAD(&m->exclusive_owner, memory_order_relaxed) != 0) {
            FAIL_WITH_MESSAGE("Mutex destruction failed; mutex has an owner!");
        }
#ifndef PANDA_TARGET_MOBILE
    } else {
        LOG_MESSAGE(WARNING, "Termination loop detected, ignoring Mutex");
    }
#endif  // PANDA_TARGET_MOBILE
}

bool MutexLock(struct fmutex *const m, bool trylock)
{
    if (current_tid == 0) {
        current_tid = GET_CURRENT_THREAD;
    }
    if (m->recursive_mutex) {
        if (IsHeld(m, current_tid)) {
            m->recursive_count++;
            return true;
        }
    }

    ASSERT(!IsHeld(m, current_tid));
    bool done = false;
    while (!done) {
        // Atomic with relaxed order reason: mutex synchronization
        auto cur_state = ATOMIC_LOAD(&m->state_and_waiters, memory_order_relaxed);
        if (LIKELY((HELPERS_TO_UNSIGNED(cur_state) & HELPERS_TO_UNSIGNED(HELD_MASK)) == 0)) {
            // Lock not held, try acquiring it.
            auto new_state = HELPERS_TO_UNSIGNED(cur_state) | HELPERS_TO_UNSIGNED(HELD_MASK);
            done = ATOMIC_CAS_WEAK(&m->state_and_waiters, cur_state, new_state, memory_order_acquire,
                                   memory_order_relaxed);
#ifdef MC_ON
            __VERIFIER_assume(done);
#endif
        } else {
            if (trylock) {
                return false;
            }
            // Failed to acquire, wait for unlock
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            if (!WaitBrieflyFor(&m->state_and_waiters)) {
                // WaitBrieflyFor failed, go to futex wait
                // Increment waiters count.
                IncrementWaiters(m);
                // Update cur_state to be equal to current expected state_and_waiters.
                cur_state += WAITER_INCREMENT;
                // Retry wait until lock not held. In heavy contention situations cur_state check can fail
                // immediately due to repeatedly decrementing and incrementing waiters.
                // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
                while ((HELPERS_TO_UNSIGNED(cur_state) & HELPERS_TO_UNSIGNED(HELD_MASK)) != 0) {
#ifdef MC_ON
                    FutexWait(&m->state_and_waiters, cur_state);
#else
                    // NOLINTNEXTLINE(hicpp-signed-bitwise), NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
                    if (futex(GetStateAddr(m), FUTEX_WAIT_PRIVATE, cur_state, nullptr, nullptr, 0) != 0) {
                        // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
                        if ((errno != EAGAIN) && (errno != EINTR)) {
                            LOG(FATAL, COMMON) << "Futex wait failed!";
                        }
                    }
#endif
                    // Atomic with relaxed order reason: mutex synchronization
                    cur_state = ATOMIC_LOAD(&m->state_and_waiters, memory_order_relaxed);
                }
                DecrementWaiters(m);
            }
        }
    }
    // Mutex is held now
    // Atomic with relaxed order reason: mutex synchronization
    ASSERT((HELPERS_TO_UNSIGNED(ATOMIC_LOAD(&m->state_and_waiters, memory_order_relaxed)) &
            HELPERS_TO_UNSIGNED(HELD_MASK)) != 0);
    // Atomic with relaxed order reason: mutex synchronization
    ASSERT(ATOMIC_LOAD(&m->exclusive_owner, memory_order_relaxed) == 0);
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_STORE(&m->exclusive_owner, current_tid, memory_order_relaxed);
    m->recursive_count++;
    ASSERT(m->recursive_count == 1);  // should be 1 here, there's a separate path for recursive mutex above
    return true;
}

bool MutexTryLockWithSpinning(struct fmutex *const m)
{
    const int max_iter = 10;
    for (int i = 0; i < max_iter; i++) {
        if (MutexLock(m, true)) {
            return true;
        }
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        if (!WaitBrieflyFor(&m->state_and_waiters)) {
            // WaitBrieflyFor failed, means lock is held
            return false;
        }
    }
    return false;
}

void MutexUnlock(struct fmutex *const m)
{
    if (current_tid == 0) {
        current_tid = GET_CURRENT_THREAD;
    }
    if (!IsHeld(m, current_tid)) {
        FAIL_WITH_MESSAGE("Trying to unlock mutex which is not held by current thread");
    }
    m->recursive_count--;
    if (m->recursive_mutex) {
        if (m->recursive_count > 0) {
            return;
        }
    }

    ASSERT(m->recursive_count == 0);  // should be 0 here, there's a separate path for recursive mutex above
    bool done = false;
    // Atomic with relaxed order reason: mutex synchronization
    auto cur_state = ATOMIC_LOAD(&m->state_and_waiters, memory_order_relaxed);
    // Retry CAS until succeess
    while (!done) {
        auto new_state = HELPERS_TO_UNSIGNED(cur_state) & ~HELPERS_TO_UNSIGNED(HELD_MASK);  // State without holding bit
        if ((HELPERS_TO_UNSIGNED(cur_state) & HELPERS_TO_UNSIGNED(HELD_MASK)) == 0) {
            FAIL_WITH_MESSAGE("Mutex unlock got unexpected state, mutex is unlocked?");
        }
        // Reset exclusive owner before changing state to avoid check failures if other thread sees UNLOCKED
        // Atomic with relaxed order reason: mutex synchronization
        ATOMIC_STORE(&m->exclusive_owner, 0, memory_order_relaxed);
        // cur_state should be updated with fetched value on fail
        done = ATOMIC_CAS_WEAK(&m->state_and_waiters, cur_state, new_state, memory_order_release, memory_order_relaxed);
#ifdef MC_ON
        __VERIFIER_assume(done);
#endif
        if (LIKELY(done)) {
            // If we had waiters, we need to do futex call
            if (UNLIKELY(new_state != 0)) {
#ifdef MC_ON
                FutexWake();
#else
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                futex(GetStateAddr(m), FUTEX_WAKE_PRIVATE, WAKE_ONE, nullptr, nullptr, 0);
#endif
            }
        }
    }
}

void MutexLockForOther(struct fmutex *const m, THREAD_ID thread)
{
    // Atomic with relaxed order reason: mutex synchronization
    ASSERT(ATOMIC_LOAD(&m->state_and_waiters, memory_order_relaxed) == 0);
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_STORE(&m->state_and_waiters, HELD_MASK, memory_order_relaxed);
    m->recursive_count = 1;
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_STORE(&m->exclusive_owner, thread, memory_order_relaxed);
}

void MutexUnlockForOther(struct fmutex *const m, THREAD_ID thread)
{
    if (!IsHeld(m, thread)) {
        FAIL_WITH_MESSAGE("Unlocking for thread which doesn't own this mutex");
    }
    // Atomic with relaxed order reason: mutex synchronization
    ASSERT(ATOMIC_LOAD(&m->state_and_waiters, memory_order_relaxed) == HELD_MASK);
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_STORE(&m->state_and_waiters, 0, memory_order_relaxed);
    m->recursive_count = 0;
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_STORE(&m->exclusive_owner, 0, memory_order_relaxed);
}

void ConditionVariableInit(struct CondVar *const cond)
{
    ATOMIC_STORE(&cond->mutex_ptr, nullptr, memory_order_relaxed);
    cond->cond = 0;
    cond->waiters = 0;
}

void ConditionVariableDestroy(struct CondVar *const cond)
{
#ifndef PANDA_TARGET_MOBILE
    if (!MutexDoNotCheckOnTerminationLoop()) {
#endif  // PANDA_TARGET_MOBILE
        // Atomic with relaxed order reason: mutex synchronization
        if (ATOMIC_LOAD(&cond->waiters, memory_order_relaxed) != 0) {
            FAIL_WITH_MESSAGE("CondVar destruction failed; waiters is non zero!");
        }
#ifndef PANDA_TARGET_MOBILE
    } else {
        LOG_MESSAGE(WARNING, "Termination loop detected, ignoring CondVar");
    }
#endif  // PANDA_TARGET_MOBILE
}

int *GetCondAddr(struct CondVar *const v)
{
    return reinterpret_cast<int *>(&v->cond);
}

const int64_t MILLISECONDS_PER_SEC = 1000;
const int64_t NANOSECONDS_PER_MILLISEC = 1000000;
const int64_t NANOSECONDS_PER_SEC = 1000000000;

struct timespec ConvertTime(uint64_t ms, uint64_t ns)
{
    struct timespec time = {0, 0};
    auto seconds = static_cast<time_t>(ms / MILLISECONDS_PER_SEC);
    auto nanoseconds = static_cast<time_t>((ms % MILLISECONDS_PER_SEC) * NANOSECONDS_PER_MILLISEC + ns);
    time.tv_sec = seconds;
    time.tv_nsec += nanoseconds;
    if (time.tv_nsec >= NANOSECONDS_PER_SEC) {
        time.tv_nsec -= NANOSECONDS_PER_SEC;
        time.tv_sec++;
    }
    return time;
}

void Wait(struct CondVar *const cond, struct fmutex *const m)
{
    if (current_tid == 0) {
        current_tid = GET_CURRENT_THREAD;
    }
    if (!IsHeld(m, current_tid)) {
        FAIL_WITH_MESSAGE("CondVar Wait failed; provided mutex is not held by current thread");
    }

    // It's undefined behavior to call Wait with different mutexes on the same condvar
    struct fmutex *old_mutex = nullptr;
    // Atomic with relaxed order reason: mutex synchronization
    while (!ATOMIC_CAS_WEAK(&cond->mutex_ptr, old_mutex, m, memory_order_relaxed, memory_order_relaxed)) {
        // CAS failed, either it was spurious fail and old val is nullptr, or make sure mutex ptr equals to current
        if (old_mutex != m && old_mutex != nullptr) {
            FAIL_WITH_MESSAGE("CondVar Wait failed; mutex_ptr doesn't equal to provided mutex");
        }
    }

    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_FETCH_ADD(&cond->waiters, 1, memory_order_relaxed);
    IncrementWaiters(m);
    auto old_count = m->recursive_count;
    m->recursive_count = 1;
    // Atomic with relaxed order reason: mutex synchronization
    auto cur_cond = ATOMIC_LOAD(&cond->cond, memory_order_relaxed);
    MutexUnlock(m);

#ifdef MC_ON
    FutexWait(&cond->cond, cur_cond);
#else
    // NOLINTNEXTLINE(hicpp-signed-bitwise), NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
    if (futex(GetCondAddr(cond), FUTEX_WAIT_PRIVATE, cur_cond, nullptr, nullptr, 0) != 0) {
        // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
        if ((errno != EAGAIN) && (errno != EINTR)) {
            LOG(FATAL, COMMON) << "Futex wait failed!";
        }
    }
#endif
    MutexLock(m, false);
    m->recursive_count = old_count;
    DecrementWaiters(m);
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_FETCH_SUB(&cond->waiters, 1, memory_order_relaxed);
}

bool TimedWait(struct CondVar *const cond, struct fmutex *const m, uint64_t ms, uint64_t ns, bool is_absolute)
{
    if (current_tid == 0) {
        current_tid = GET_CURRENT_THREAD;
    }
    if (!IsHeld(m, current_tid)) {
        FAIL_WITH_MESSAGE("CondVar Wait failed; provided mutex is not held by current thread");
    }

    // It's undefined behavior to call Wait with different mutexes on the same condvar
    struct fmutex *old_mutex = nullptr;
    // Atomic with relaxed order reason: mutex synchronization
    while (!ATOMIC_CAS_WEAK(&cond->mutex_ptr, old_mutex, m, memory_order_relaxed, memory_order_relaxed)) {
        // CAS failed, either it was spurious fail and old val is nullptr, or make sure mutex ptr equals to current
        if (old_mutex != m && old_mutex != nullptr) {
            FAIL_WITH_MESSAGE("CondVar Wait failed; mutex_ptr doesn't equal to provided mutex");
        }
    }

    bool timeout = false;
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_FETCH_ADD(&cond->waiters, 1, memory_order_relaxed);
    IncrementWaiters(m);
    auto old_count = m->recursive_count;
    m->recursive_count = 1;
    // Atomic with relaxed order reason: mutex synchronization
    auto cur_cond = ATOMIC_LOAD(&cond->cond, memory_order_relaxed);
    MutexUnlock(m);

#ifdef MC_ON
    FutexWait(&cond->cond, cur_cond);
#else
    int wait_flag;
    int match_flag;
    struct timespec time = ConvertTime(ms, ns);
    if (is_absolute) {
        // FUTEX_WAIT_BITSET uses absolute time
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        wait_flag = FUTEX_WAIT_BITSET_PRIVATE;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        match_flag = FUTEX_BITSET_MATCH_ANY;
    } else {
        // FUTEX_WAIT uses relative time
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        wait_flag = FUTEX_WAIT_PRIVATE;
        match_flag = 0;
    }
    if (futex(GetCondAddr(cond), wait_flag, cur_cond, &time, nullptr, match_flag) != 0) {
        if (errno == ETIMEDOUT) {
            timeout = true;
        } else if ((errno != EAGAIN) && (errno != EINTR)) {
            LOG(FATAL, COMMON) << "Futex wait failed!";
        }
    }
#endif
    MutexLock(m, false);
    m->recursive_count = old_count;
    DecrementWaiters(m);
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_FETCH_SUB(&cond->waiters, 1, memory_order_relaxed);
    return timeout;
}

void SignalCount(struct CondVar *const cond, int32_t to_wake)
{
    // Atomic with relaxed order reason: mutex synchronization
    if (ATOMIC_LOAD(&cond->waiters, memory_order_relaxed) == 0) {
        // No waiters, do nothing
        return;
    }

    if (current_tid == 0) {
        current_tid = GET_CURRENT_THREAD;
    }
    // Atomic with relaxed order reason: mutex synchronization
    auto mutex = ATOMIC_LOAD(&cond->mutex_ptr, memory_order_relaxed);
    // If this condvar has waiters, mutex_ptr should be set
    ASSERT(mutex != nullptr);
    // Atomic with relaxed order reason: mutex synchronization
    ATOMIC_FETCH_ADD(&cond->cond, 1, memory_order_relaxed);

#ifdef MC_ON
    FutexWake();
#else
    if (IsHeld(mutex, current_tid)) {
        // This thread is owner of current mutex, do requeue to mutex waitqueue
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        bool success = futex(GetCondAddr(cond), FUTEX_REQUEUE_PRIVATE, 0, reinterpret_cast<const timespec *>(to_wake),
                             GetStateAddr(mutex), 0) != -1;
        if (!success) {
            FAIL_WITH_MESSAGE("Futex requeue failed!");
        }
    } else {
        // Mutex is not held by this thread, do wake
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        futex(GetCondAddr(cond), FUTEX_WAKE_PRIVATE, to_wake, nullptr, nullptr, 0);
    }
#endif
}

#ifndef MC_ON
}  // namespace panda::os::unix::memory::futex
#endif
