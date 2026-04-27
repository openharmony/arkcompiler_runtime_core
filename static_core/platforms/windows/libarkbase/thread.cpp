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

#include "libarkbase/os/thread.h"
#include "libarkbase/utils/logger.h"

#include <errhandlingapi.h>
#include <handleapi.h>
#include <process.h>
#include <processthreadsapi.h>
#include <securec.h>
#include <thread>

namespace ark::os::thread {
static constexpr uint32_t THREAD_NAME_MAX_LENGTH = 15;

ThreadId GetCurrentThreadId()
{
    // The function is provided by mingw
    return ::GetCurrentThreadId();
}

int GetPid()
{
    return _getpid();
}

int GetPPid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

int GetUid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

int GetEuid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

uint32_t GetGid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

uint32_t GetEgid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

std::vector<uint32_t> GetGroups()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

int SetPriority(DWORD threadId, int prio)
{
    // The priority can be set within range [-2, 2]
    ASSERT(prio >= -2);  // -2: the lowest priority
    ASSERT(prio <= 2);   // 2: the highest priority
    HANDLE thread = OpenThread(THREAD_SET_INFORMATION, false, threadId);
    if (thread == NULL) {
        LOG(FATAL, COMMON) << "OpenThread failed, error code " << GetLastError();
    }
    auto ret = SetThreadPriority(thread, prio);
    CloseHandle(thread);
    // Thre return value is nonzero if the function succeeds, and zero if it fails.
    return ret;
}

int GetPriority(DWORD threadId)
{
    HANDLE thread = OpenThread(THREAD_QUERY_INFORMATION, false, threadId);
    if (thread == NULL) {
        LOG(FATAL, COMMON) << "OpenThread failed, error code " << GetLastError();
    }
    auto ret = GetThreadPriority(thread);
    CloseHandle(thread);
    return ret;
}

int SetThreadName(NativeHandleType pthreadHandle, const char *name)
{
    ASSERT(pthreadHandle != 0);
    if (name == nullptr) {
        name = "";
    }

    const char *realName = name;
    pthread_t thread = reinterpret_cast<pthread_t>(pthreadHandle);
    std::array<char, THREAD_NAME_MAX_LENGTH + 1> threadName = {0};
    // strlen(name) > 15, Intercept the first 15 characters.
    if (strnlen(name, THREAD_NAME_MAX_LENGTH + 1) > THREAD_NAME_MAX_LENGTH) {
        auto strRes = strncpy_s(threadName.data(), THREAD_NAME_MAX_LENGTH + 1, name, THREAD_NAME_MAX_LENGTH);
        if (UNLIKELY(strRes != EOK)) {
            LOG(ERROR, RUNTIME) << "Intercepting the character string failed, error is " << strRes;
        }

        realName = threadName.data();
    }

    return pthread_setname_np(thread, realName);
}

NativeHandleType GetNativeHandle()
{
    return reinterpret_cast<NativeHandleType>(pthread_self());
}

void Yield()
{
    std::this_thread::yield();
}

void NativeSleep(unsigned int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ThreadDetach(NativeHandleType pthreadHandle)
{
    pthread_detach(reinterpret_cast<pthread_t>(pthreadHandle));
}

void ThreadJoin(NativeHandleType pthreadHandle, void **ret)
{
    pthread_join(reinterpret_cast<pthread_t>(pthreadHandle), ret);
}
}  // namespace ark::os::thread
