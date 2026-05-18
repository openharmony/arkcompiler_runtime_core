/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "common_components/mutator/mutator_manager.h"

#include <thread>

#include "common_components/base/time_utils.h"
#include "common_components/heap/allocator/alloc_buffer.h"
#include "common_components/heap/heap.h"

#include "common_interfaces/thread/mutator-inl.h"
#include "runtime/include/panda_vm.h"
#include "libarkbase/os/mutex.h"

namespace ark::common_vm {
bool g_enableGCTimeoutCheck = true;

bool IsRuntimeThread()
{
    if (static_cast<int>(ThreadLocal::GetThreadType()) >= static_cast<int>(ThreadType::GC_THREAD)) {
        return true;
    }
    return false;
}

bool IsGcThread()
{
    if (static_cast<int>(ThreadLocal::GetThreadType()) == static_cast<int>(ThreadType::GC_THREAD)) {
        return true;
    }
    return false;
}

void MutatorManager::BindMutator(Mutator &mutator) const
{
    ThreadLocalData *tlData = ThreadLocal::GetThreadLocalData();
    if (UNLIKELY(tlData->buffer == nullptr)) {
        (void)AllocationBuffer::GetOrCreateAllocBuffer();
    }
    tlData->mutator = &mutator;
}

void MutatorManager::UnbindMutator(Mutator &mutator) const
{
    ThreadLocalData *tlData = ThreadLocal::GetThreadLocalData();
    tlData->mutator = nullptr;
}

bool MutatorManager::BindMutatorOnly(Mutator *mutator) const
{
    // watch dog thread may call this function and copy barrier may occur, so bind mutator here.
    ThreadLocalData *tlData = ThreadLocal::GetThreadLocalData();
    DCHECK(tlData != nullptr);
    if (tlData->mutator == nullptr) {
        tlData->mutator = mutator;
        return true;
    }
    return false;
}

void MutatorManager::UnbindMutatorOnly() const
{
    ThreadLocalData *tlData = ThreadLocal::GetThreadLocalData();
    tlData->mutator = nullptr;
}

void MutatorManager::DestroyExpiredMutators()
{
    expiringMutatorListLock_.Lock();
    ExpiredMutatorList workList;
    workList.swap(expiringMutators_);
    expiringMutatorListLock_.Unlock();
    for (auto it = workList.begin(); it != workList.end(); ++it) {
        Mutator *expiringMutator = *it;
        delete expiringMutator;
    }
}

void MutatorManager::DestroyMutator(Mutator *mutator)
{
    if (TryAcquireMutatorLockR()) {
        delete mutator;  // call ~Mutator() under mutatorListLock
        MutatorLockRUnlock();
    } else {
        expiringMutatorListLock_.Lock();
        expiringMutators_.push_back(mutator);
        expiringMutatorListLock_.Unlock();
    }
}

void MutatorManager::Init()
{
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
    safepointPageManager_ = new (std::nothrow) SafepointPageManager();
    LOG_IF(UNLIKELY(safepointPageManager_ == nullptr), FATAL, GC) << "new safepointPageManager failed";
    safepointPageManager_->Init();
#endif
    SetSuspensionMutatorCount(0);
}

MutatorManager *MutatorManager::mutatorManager_ = nullptr;

/* static */
MutatorManager &MutatorManager::Instance() noexcept
{
    return *mutatorManager_;
}

/* static */
void MutatorManager::Create()
{
    auto *runtime = Runtime::GetCurrent();
    auto allocator = runtime->GetInternalAllocator();
    mutatorManager_ = allocator->New<MutatorManager>();
    LOG_IF(UNLIKELY(mutatorManager_ == nullptr), FATAL, RUNTIME) << "MutatorManager instance creation failed";
    mutatorManager_->Init();
}

void MutatorManager::Destroy()
{
    if (mutatorManager_ != nullptr) {
        auto *runtime = Runtime::GetCurrent();
        auto allocator = runtime->GetInternalAllocator();
        mutatorManager_->Fini();
        allocator->Delete(mutatorManager_);
        mutatorManager_ = nullptr;
    }
}

void MutatorManager::AcquireMutatorLockW()
{
    uint64_t start = TimeUtil::NanoSeconds();
    bool acquired = TryAcquireMutatorLockW();
    while (!acquired) {
        TimeUtil::SleepForNano(WAIT_LOCK_INTERVAL);
        acquired = TryAcquireMutatorLockW();
        uint64_t now = TimeUtil::NanoSeconds();
        if (!acquired && ((now - start) / SECOND_TO_NANO_SECOND > WAIT_LOCK_TIMEOUT)) {
            LOG(FATAL, COMMON) << "Wait mutator list lock timeout";
            UNREACHABLE();
        }
    }
}

// Visit all mutators, hold mutatorListLock firstly
void MutatorManager::VisitAllMutators(MutatorVisitor func, bool ignoreFinalizer)
{
    {
        ark::os::memory::LockHolder guard(allMutatorListLock_);
        for (auto mutator : allMutatorList_) {
            func(*mutator);
        }
    }
}

void MutatorManager::StopTheWorld(GCPhase phase)
{
    ASSERT(IsGcThread());
    // Block if another thread is holding the stwMutex.
    // Prevent multi-thread doing STW concurrently.
    stwMutex_.Lock();
    // Atomic with seq_cst order reason: data race with stwTriggered_ with requirement for sequentially consistent
    // order where threads observe all modifications in the same order
    stwTriggered_.store(true, std::memory_order_seq_cst);

    size_t mutatorCount = GetMutatorCount();
    if (UNLIKELY(mutatorCount == 0)) {
        AcquireMutatorLockW();
        // Atomic with release order reason: data race with worldStopped_ with dependecies on writes before the store
        worldStopped_.store(true, std::memory_order_release);
        return;
    }
    // set mutatorCount as countOfMutatorsToStop.
    SetSuspensionMutatorCount(static_cast<uint32_t>(mutatorCount));
    DemandSuspensionForStw();
    AcquireMutatorLockW();

    // the world is stopped.
    // Atomic with release order reason: data race with worldStopped_ with dependecies on writes before the store
    worldStopped_.store(true, std::memory_order_release);
}

void MutatorManager::StartTheWorld() noexcept
{
    ASSERT(IsGcThread());
    // Atomic with seq_cst order reason: data race with stwTriggered_ with requirement for sequentially consistent
    // order where threads observe all modifications in the same order
    stwTriggered_.store(false, std::memory_order_seq_cst);
    // Atomic with release order reason: data race with worldStopped_ with dependecies on writes before the store
    worldStopped_.store(false, std::memory_order_release);

    CancelSuspensionAfterStw();
    SetSuspensionMutatorCount(0);

    // wakeup all mutators which blocking on countOfMutatorsToStop futex.
#if defined(_WIN64) || defined(__APPLE__)
    WakeAllMutators();
#else
    (void)Futex(GetStwFutexWord(), FUTEX_WAKE, INT_MAX);
#endif

    MutatorLockWUnlock();

    // Release stwMutex to allow other thread call STW.
    stwMutex_.Unlock();
}

ark::MutatorLock *MutatorManager::GetMutatorLockImpl() const
{
    return PandaVM::GetCurrent()->GetMutatorLock();
}

void MutatorManager::DumpMutators(uint32_t timeoutTimes)
{
    constexpr size_t bufferSize = 4096;
    char buf[bufferSize];
    int index = 0;
    size_t visitedCount = 0;
    size_t visitedSaferegion = 0;
    int firstNotStoppedTid = -1;
    index += sprintf_s(buf, sizeof(buf), "not stopped: ");
    LOG_IF(UNLIKELY(index == -1), FATAL, GC) << "Dump mutators state failed";
    size_t mutatorCount = 0;
    VisitAllMutators([&](const Mutator &mut) {
        mutatorCount++;
#ifndef NDEBUG
        mut.DumpMutator();
#endif
        if (!mut.InSaferegion()) {
            if (firstNotStoppedTid == -1) {
                firstNotStoppedTid = static_cast<int>(mut.GetTid());
            }
            int ret = sprintf_s(buf + index, sizeof(buf) - index, "%u ", mut.GetTid());
            LOG_IF(UNLIKELY(ret == -1), FATAL, GC) << "Dump mutators state failed";
            index += ret;
        } else {
            ++visitedSaferegion;
        }
        ++visitedCount;
    });
    LOG(ERROR, COMMON) << "MutatorList size: " << mutatorCount;

    LOG_IF(UNLIKELY(sprintf_s(buf + index, sizeof(buf) - index, ", total: %u, visited: %zu/%zu",
                              GetSuspensionMutatorCount(), visitedSaferegion, visitedCount) == -1),
           FATAL, GC)
        << "Dump mutators state failed";
    LOG_IF(UNLIKELY(timeoutTimes > MAX_TIMEOUT_TIMES), FATAL, GC)
        << "Waiting mutators entering saferegion timeout status info:" << buf;
    LOG(ERROR, COMMON) << "STW status info: " << buf;
}

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void MutatorManager::DumpForDebug()
{
    size_t count = 0;
    auto func = [&count](Mutator &mutator) {
        mutator.DumpMutator();
        count++;
    };
    VisitAllMutators(func);
    LOG(INFO, COMMON) << "MutatorList size : " << count;
}

void MutatorManager::DumpAllGcInfos()
{
    auto func = [](Mutator &mutator) { mutator.DumpGCInfos(); };
    VisitAllMutators(func);
}
#endif

}  // namespace ark::common_vm
