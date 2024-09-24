/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/include/runtime.h"

namespace ark::ets::intrinsics {

/// @brief Mutex creation in internal space
extern "C" EtsLong InitializeMutex()
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto *mutex = allocator->New<os::memory::Mutex>();
    return reinterpret_cast<EtsLong>(mutex);
}

/// @brief Event creation in internal space
extern "C" EtsLong InitializeEvent()
{
    EtsCoroutine *currentCoro = EtsCoroutine::GetCurrent();
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto *event = allocator->New<GenericEvent>(currentCoro->GetCoroutineManager());
    return reinterpret_cast<EtsLong>(event);
}

/// @brief Mutex deletion
extern "C" void DeleteMutex(EtsLong mutexPtr)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(reinterpret_cast<os::memory::Mutex *>(mutexPtr));
}

/// @brief Event deletion
extern "C" void DeleteEvent(EtsLong eventPtr)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(reinterpret_cast<GenericEvent *>(eventPtr));
}

// Disable thread-safety-analysis because it creates warnings
// due to mutexes' manipulations at the end of functions
#if defined(__clang__)
#pragma clang diagnostic push
// CC-OFFNXT(warning_suppression[C++]) Disable thread-safety-analysis because it creates warnings due to mutexes'
// manipulations at the end of functions. Should be suppressed.
#pragma clang diagnostic ignored "-Wthread-safety-analysis"
#endif

/**
 * @brief Locking mutex
 * Mutex may be unlocked with MutexUnlock or UnblockWaiters functions
 */
extern "C" void MutexLock(EtsLong mutexPtr)
{
    auto *mutex = reinterpret_cast<os::memory::Mutex *>(mutexPtr);
    mutex->Lock();
}

/// @brief Unlocking mutex that was locked with MutexLock function
extern "C" void MutexUnlock(EtsLong mutexPtr)
{
    auto *mutex = reinterpret_cast<os::memory::Mutex *>(mutexPtr);
    mutex->Unlock();
}

/**
 * @brief Setting event to be "not happened", unlocking mutex and
 * awaiting for the event to happen. After "await" we try to lock
 * the mutex again to continue working with the BlockingQueue
 */
extern "C" void Await(EtsLong mutexPtr, EtsLong eventPtr)
{
    EtsCoroutine *currentCoro = EtsCoroutine::GetCurrent();
    auto *event = reinterpret_cast<GenericEvent *>(eventPtr);
    auto *mutex = reinterpret_cast<os::memory::Mutex *>(mutexPtr);
    event->SetNotHappened();
    event->Lock();
    mutex->Unlock();
    currentCoro->GetCoroutineManager()->Await(event);
    mutex->Lock();
}

/**
 * @brief Setting event to be "happened" and unlocking mutex.
 * This function should be called after successful push() to the BlockingQueue
 */
extern "C" void UnblockWaiters(EtsLong mutexPtr, EtsLong eventPtr)
{
    auto *event = reinterpret_cast<GenericEvent *>(eventPtr);
    auto *mutex = reinterpret_cast<os::memory::Mutex *>(mutexPtr);
    event->Happen();
    mutex->Unlock();
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace ark::ets::intrinsics
