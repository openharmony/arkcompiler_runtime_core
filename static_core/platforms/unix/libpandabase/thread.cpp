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

#include "os/thread.h"
#include "os/mem.h"

#include "utils/span.h"
#include "utils/logger.h"

#include <chrono>
#include <cstdio>

#include <array>
#include <cstdint>
#include "os/failure_retry.h"
#ifdef PANDA_TARGET_UNIX
#include <fcntl.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <csignal>
#endif
#include <securec.h>
#include <unistd.h>

namespace panda::os::thread {

ThreadId GetCurrentThreadId()
{
#if defined(HAVE_GETTID)
    static_assert(sizeof(decltype(gettid())) == sizeof(ThreadId), "Incorrect alias for ThreadID");
    return static_cast<ThreadId>(gettid());
#elif defined(PANDA_TARGET_MACOS)
    uint64_t tid64;
    pthread_threadid_np(NULL, &tid64);
    return static_cast<ThreadId>(tid64);
#else
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    return static_cast<ThreadId>(syscall(SYS_gettid));
#endif
}

int GetPid()
{
    return getpid();
}

int SetPriority(int thread_id, int prio)
{
    // The priority can be set within range [-20, 19]
    ASSERT(prio <= 19);   // 19: the lowest priority
    ASSERT(prio >= -20);  // -20: the highest priority
    // The return value is 0 if the function succeeds, and -1 if it fails.
    return setpriority(PRIO_PROCESS, thread_id, prio);
}

int GetPriority(int thread_id)
{
    return getpriority(PRIO_PROCESS, thread_id);
}

int SetThreadName(NativeHandleType pthread_handle, const char *name)
{
    ASSERT(pthread_handle != 0);
#if defined(PANDA_TARGET_MACOS)
    return pthread_setname_np(name);
#else
    return pthread_setname_np(pthread_handle, name);
#endif
}

NativeHandleType GetNativeHandle()
{
    return pthread_self();
}

void Yield()
{
    std::this_thread::yield();
}

void NativeSleep(unsigned int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void NativeSleepUS(std::chrono::microseconds us)
{
    std::this_thread::sleep_for(us);
}

void ThreadDetach(NativeHandleType pthread_handle)
{
    pthread_detach(pthread_handle);
}

void ThreadExit(void *ret)
{
    pthread_exit(ret);
}

void ThreadJoin(NativeHandleType pthread_handle, void **ret)
{
    pthread_join(pthread_handle, ret);
}

void ThreadSendSignal(NativeHandleType pthread_handle, int sig)
{
    LOG_IF(pthread_kill(pthread_handle, sig) != 0, FATAL, COMMON) << "pthread_kill failed";
}

int ThreadGetStackInfo(NativeHandleType thread, void **stack_addr, size_t *stack_size, size_t *guard_size)
{
    pthread_attr_t attr;
    int s = pthread_attr_init(&attr);
#ifndef PANDA_TARGET_MACOS
    s += pthread_getattr_np(thread, &attr);
    if (s == 0) {
        s += pthread_attr_getguardsize(&attr, guard_size);
        s += pthread_attr_getstack(&attr, stack_addr, stack_size);
#if defined(PANDA_TARGET_OHOS) && !defined(NDEBUG)
        if (getpid() == gettid()) {
            /**
             *  konstanting:
             *  main thread's stack can automatically grow up to the RLIMIT_STACK by means of the OS,
             *  but MUSL does not care about that and returns the current (already mmap-ped) stack size.
             *  This can lead to complicated errors, so let's adjust the stack size manually.
             */
            struct rlimit lim;
            s += getrlimit(RLIMIT_STACK, &lim);
            if (s == 0) {
                uintptr_t stack_hi_addr = ToUintPtr(*stack_addr) + *stack_size;
                size_t stack_size_limit = lim.rlim_cur;
                // for some reason pthread interfaces subtract 1 page from size regardless of guard size
                uintptr_t stack_lo_addr = stack_hi_addr - stack_size_limit + panda::os::mem::GetPageSize();
                *stack_size = stack_size_limit;
                *stack_addr = ToVoidPtr(stack_lo_addr);
            }
        }
#endif /* defined(PANDA_TARGET_OHOS) && !defined(NDEBUG) */
    }
#else  /* PANDA_TARGET_MACOS */
    s += pthread_attr_getguardsize(&attr, guard_size);
    s += pthread_attr_getstack(&attr, stack_addr, stack_size);
#endif /* PANDA_TARGET_MACOS */
    s += pthread_attr_destroy(&attr);
    return s;
}

}  // namespace panda::os::thread
