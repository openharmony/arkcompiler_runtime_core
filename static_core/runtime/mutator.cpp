/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "runtime/include/mutator.h"

#include "include/mutator_status.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/mem/panda_containers.h"

#if defined(ARK_USE_COMMON_RUNTIME)
#include "runtime/mem/gc/cmc/cmc-gc.h"
#endif

namespace ark {

std::ostream &operator<<(std::ostream &stream, Mutator::MutatorType type)
{
    switch (type) {
        case Mutator::MutatorType::MANAGED:
            stream << "MANAGED";
            break;
        case Mutator::MutatorType::COMPILER:
            stream << "COMPILER";
            break;
        case Mutator::MutatorType::GC:
            stream << "GC";
            break;
        default:
            stream << "NONE";
    }
    return stream;
}

std::ostream &operator<<(std::ostream &stream, MutatorStatus status)
{
    switch (status) {
        case MutatorStatus::CREATED:
            return stream << "New";
        case MutatorStatus::RUNNING:
            return stream << "Runnable";
        case MutatorStatus::IS_BLOCKED:
            return stream << "Blocked";
        case MutatorStatus::IS_WAITING:
            return stream << "Waiting";
        case MutatorStatus::IS_TIMED_WAITING:
            return stream << "Timed_waiting";
        case MutatorStatus::IS_SUSPENDED:
            return stream << "Suspended";
        case MutatorStatus::IS_COMPILER_WAITING:
            return stream << "Compiler_waiting";
        case MutatorStatus::IS_WAITING_INFLATION:
            return stream << "Waiting_inflation";
        case MutatorStatus::IS_SLEEPING:
            return stream << "Sleeping";
        case MutatorStatus::IS_TERMINATED_LOOP:
            return stream << "Terminated_loop";
        case MutatorStatus::TERMINATING:
            return stream << "Terminating";
        case MutatorStatus::NATIVE:
            return stream << "Native";
        case MutatorStatus::FINISHED:
            return stream << "Terminated";
        default:
            return stream << "unknown";
    }
}

Mutator::Mutator(PandaVM *vm, MutatorType type) : vm_(vm), type_(type)
{
    ASSERT(vm_ != nullptr);
    mutatorLock_ = vm_->GetMutatorLock();
    // WORKAROUND(v.cherkashin): EcmaScript side build doesn't have GC, so we skip setting barriers for this case
    auto *gc = vm_->GetGC();
    if (gc != nullptr) {
        barrierSet_ = gc->GetBarrierSet();
        InitCardTableData(barrierSet_);
        preBarrierType_ = barrierSet_->GetPreType();
        postBarrierType_ = barrierSet_->GetPostType();
    }
    ASSERT(type_ != MutatorType::NONE);
    InitializeMutatorFlag();
}

Mutator::~Mutator()
{
    // Internal resources should be deleted in OnMutatorTerminate, if GC created them
    ASSERT(satbBuff_ == nullptr);
    ASSERT(g1PostBarrierRingBuffer_ == nullptr);
}

void Mutator::AllocateSatbBuff(mem::InternalAllocatorPtr allocator)
{
    if (allocator == nullptr) {
        allocator = Runtime::GetCurrent()->GetInternalAllocator();
    }
    satbBuff_ = allocator->New<ObjectPointerType[]>(Mutator::PRE_BUFF_CAPACITY);
    satbBuffSize_ = 0U;
}

void Mutator::PushSatbBuff(const std::pair<ObjectPointerType *, size_t> &entry)
{
    ASSERT(satbBuffList_ != nullptr);
    ASSERT(satbBuffMutex_ != nullptr);

    os::memory::LockHolder lock(*satbBuffMutex_);
    satbBuffList_->push_back(entry);
}

CONSTEXPR_IN_RELEASE MutatorFlag GetInitialMutatorFlag()
{
#ifndef NDEBUG
    MutatorFlag initialFlag = Runtime::GetOptions().IsRunGcEverySafepoint() ? GC_ON_SAFEPOINT_REQUEST : NO_FLAGS;
    return initialFlag;
#else
    return NO_FLAGS;
#endif
}

MutatorFlag Mutator::initialMutatorFlag_ = NO_FLAGS;

/* static */
void Mutator::InitializeInitMutatorFlag()
{
    initialMutatorFlag_ = GetInitialMutatorFlag();
}

bool Mutator::TestAllFlags() const
{
    return (fms_.asStruct.flags) != initialMutatorFlag_;  // NOLINT(cppcoreguidelines-pro-type-union-access)
}

void Mutator::SetFlag(MutatorFlag flag)
{
    // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    fms_.asAtomic.flags.fetch_or(flag, std::memory_order_seq_cst);
}

bool Mutator::ExchangeFlags(MutatorFlag oldFlag, MutatorFlag newFlag)
{
    uint16_t oldFlag16 = oldFlag;
    uint16_t newFlag16 = newFlag;
    return fms_.asAtomic.flags.compare_exchange_strong(oldFlag16, newFlag16, std::memory_order_seq_cst);
}

void Mutator::ClearFlag(MutatorFlag flag)
{
    // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    fms_.asAtomic.flags.fetch_and(UINT16_MAX ^ flag, std::memory_order_seq_cst);
}

uint32_t Mutator::ReadFlagsAndMutatorStatusUnsafe()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return fms_.asInt;
}

enum MutatorStatus Mutator::GetStatus() const
{
    // Atomic with acquire order reason: data race with flags with dependecies on reads after
    // the load which should become visible
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return static_cast<enum MutatorStatus>(fms_.asAtomic.status.load(std::memory_order_acquire));
}

uint32_t Mutator::ReadFlagsUnsafe() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return fms_.asStruct.flags;
}

void Mutator::InitializeMutatorFlag()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    fms_.asInt = initialMutatorFlag_;
}

void Mutator::CleanUpMutatorStatus()
{
    InitializeMutatorFlag();
    {
        os::memory::LockHolder lock(suspendLock_);
        suspendCount_ = 0;
    }
    StoreStatus<DONT_CHECK_SAFEPOINT, NO_READLOCK>(MutatorStatus::CREATED);
}

void Mutator::UpdateStatus(enum MutatorStatus status)
{
    MutatorStatus oldStatus = GetStatus();
    if (oldStatus == MutatorStatus::RUNNING && status != MutatorStatus::RUNNING) {
        TransitionFromRunningToSuspended(status);
    } else if (oldStatus != MutatorStatus::RUNNING && status == MutatorStatus::RUNNING) {
        TransitionFromSuspendedToRunning();
    } else if (oldStatus == MutatorStatus::NATIVE && status != MutatorStatus::IS_TERMINATED_LOOP &&
               IsRuntimeTerminated()) {
        // If a daemon thread with NATIVE status was deregistered, it should not access any managed object,
        // i.e. change its status from NATIVE, because such object may already be deleted by the runtime.
        // In case its status is changed, we must call a Safepoint to terminate this thread.
        // For example, if a daemon thread calls ManagedCodeBegin (which changes status from NATIVE to
        // RUNNING), it may be interrupted by a GC thread, which changes status to IS_SUSPENDED.
        StoreStatus<CHECK_SAFEPOINT>(status);
    } else {
        // NB! Status is not a simple bit, without atomics it can produce faulty GetStatus.
        StoreStatus(status);
    }
}

void Mutator::TransitionFromRunningToSuspended(enum MutatorStatus status)
{
    // Do Unlock after StoreStatus, because the thread requesting a suspension should see an updated status
    StoreStatus(status);
    GetMutatorLock()->Unlock();
}

void Mutator::TransitionFromSuspendedToRunning()
{
    // NB! This thread is treated as suspended so when we transition from suspended state to
    // running we need to check suspension flag and counter so SafepointPoll has to be done before
    // acquiring mutator_lock.
    // StoreStatus acquires lock here
    StoreStatus<CHECK_SAFEPOINT, READLOCK>(MutatorStatus::RUNNING);
    if (HaveRefCleanupRequest()) {
        HandleReferencesCleanupRequest();
    }
}

void Mutator::SuspendCheck()
{
    // We should use internal suspension to avoid missing call of IncSuspend
    SuspendImpl(true);
    GetMutatorLock()->Unlock();
    GetMutatorLock()->ReadLock();
    ResumeImpl(true);
}

void Mutator::SuspendImpl(bool internalSuspend)
{
    os::memory::LockHolder lock(suspendLock_);
    LOG(DEBUG, RUNTIME) << "Suspending thread " << os::thread::GetCurrentThreadId();
    if (!internalSuspend) {
        if (IsUserSuspended()) {
            LOG(DEBUG, RUNTIME) << "thread " << os::thread::GetCurrentThreadId() << " is already suspended";
            return;
        }
        userCodeSuspendCount_++;
    }
    auto oldCount = suspendCount_++;
    if (oldCount == 0) {
        SetFlag(SUSPEND_REQUEST);
    }
}

void Mutator::ResumeImpl(bool internalResume)
{
    os::memory::LockHolder lock(suspendLock_);
    LOG(DEBUG, RUNTIME) << "Resuming thread " << os::thread::GetCurrentThreadId();
    if (!internalResume) {
        if (!IsUserSuspended()) {
            LOG(DEBUG, RUNTIME) << "thread " << os::thread::GetCurrentThreadId() << " is already resumed";
            return;
        }
        ASSERT(userCodeSuspendCount_ != 0);
        userCodeSuspendCount_--;
    }
    if (suspendCount_ > 0) {
        suspendCount_--;
        if (suspendCount_ == 0) {
            ClearFlag(SUSPEND_REQUEST);
        }
    }
    // Help for UnregisterExitedThread
    TSAN_ANNOTATE_HAPPENS_BEFORE(&fms_);
    suspendVar_.Signal();
}

void Mutator::SafepointPoll()
{
    if (TestAllFlags()) {
        trace::ScopedTrace scopedTrace("RunSafepoint");
        Safepoint();
    }
}

void Mutator::Safepoint()
{
#if defined(SAFEPOINT_TIME_CHECKER_ENABLED)
    this->ResetSafepointTimer(true);
#endif  // SAFEPOINT_TIME_CHECKER_ENABLED
#if !defined(NDEBUG)
    // NOTE(sarychevkonstantin, #I9624): achieve consistency between mutator lock ownership and IsManaged method
    if (Runtime::GetOptions().IsRunGcEverySafepoint() && mutatorLock_->HasLock()) {
        auto *vm = GetVM();
        vm->GetGCTrigger()->TriggerGcIfNeeded(vm->GetGC());
    }
#endif  // !NDEBUG

    if (UNLIKELY(IsRuntimeTerminated())) {
        OnRuntimeTerminated();
    }
    if (IsSuspended()) {
        WaitSuspension();
    }

#if defined(SAFEPOINT_TIME_CHECKER_ENABLED)
    this->ResetSafepointTimer(false);
#endif  // SAFEPOINT_TIME_CHECKER_ENABLED
}

bool Mutator::IsUserSuspended() const
{
    return userCodeSuspendCount_ > 0;
}

void Mutator::WaitSuspension()
{
    constexpr int TIMEOUT = 100;
    auto oldStatus = GetStatus();
    PrintSuspensionStackIfNeeded();
    UpdateStatus(MutatorStatus::IS_SUSPENDED);
    {
        /* @sync 1
         * @description Right after the thread updates its status to IS_SUSPENDED and right before beginning to wait
         * for actual suspension
         */
        os::memory::LockHolder lock(suspendLock_);
        while (suspendCount_ > 0) {
            suspendVar_.TimedWait(&suspendLock_, TIMEOUT);
            // In case runtime is being terminated, we should abort suspension and release monitors
            if (UNLIKELY(IsRuntimeTerminated())) {
                suspendLock_.Unlock();
                OnRuntimeTerminated();
                UNREACHABLE();
            }
        }
        ASSERT(!IsSuspended());
    }
    UpdateStatus(oldStatus);
}

void Mutator::InitCardTableData(mem::GCBarrierSet *barrier)
{
    auto postBarrierType = barrier->GetPostType();
    switch (postBarrierType) {
        case ark::mem::BarrierType::POST_INTERREGION_BARRIER:
            cardTableAddr_ = std::get<uint8_t *>(barrier->GetPostBarrierOperand("CARD_TABLE_ADDR").GetValue());
            cardTableMinAddr_ = std::get<void *>(barrier->GetPostBarrierOperand("MIN_ADDR").GetValue());
            postWrbOneObject_ = reinterpret_cast<void *>(PostInterRegionBarrierMarkSingleFast);
            postWrbTwoObjects_ = reinterpret_cast<void *>(PostInterRegionBarrierMarkPairFast);
            break;
        case ark::mem::BarrierType::POST_WRB_NONE:
            postWrbOneObject_ = reinterpret_cast<void *>(EmptyPostWriteBarrier);
            postWrbTwoObjects_ = reinterpret_cast<void *>(EmptyPostWriteBarrier);
            break;
        case mem::POST_RB_NONE:
            break;
        case ark::mem::BarrierType::POST_CMC_WRITE_BARRIER:
            postWrbOneObject_ = reinterpret_cast<void *>(CmcPostWriteBarrierSingleFast);
            postWrbTwoObjects_ = reinterpret_cast<void *>(CmcPostWriteBarrierPairFast);
            break;
        case mem::PRE_WRB_NONE:
        case mem::PRE_RB_NONE:
        case mem::PRE_SATB_BARRIER:
        case ark::mem::BarrierType::CMC_READ_BARRIER:
        case ark::mem::BarrierType::PRE_CMC_WRITE_BARRIER:
            LOG(FATAL, RUNTIME) << "Post barrier expected";
            break;
    }
}

void Mutator::InitBuffers()
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    mem::GC *gc = GetVM()->GetGC();
    auto barrier = gc->GetBarrierSet();
    if (barrier->GetPreType() != ark::mem::BarrierType::PRE_WRB_NONE) {
        // we need to recreate buffers if it was detach (we removed all structures) and attach again
        // skip initializing in first attach after constructor
        if (satbBuff_ == nullptr) {
            ASSERT(satbBuff_ == nullptr);
            AllocateSatbBuff();
            ASSERT(g1PostBarrierRingBuffer_ == nullptr);
            g1PostBarrierRingBuffer_ = allocator->New<mem::GCG1BarrierSet::G1PostBarrierRingBufferType>();
        }
    }
}

void Mutator::HandleReferencesCleanupRequest()
{
    ClearReferencesCleanupRequest();
    if (IsRuntimeTerminated() || type_ != MutatorType::MANAGED) {
        return;
    }
    GetVM()->HandleGCRoutineInMutator();
}

#if defined(ARK_USE_COMMON_RUNTIME)
void Mutator::VisitMutatorRoots(const ark::mem::RefFieldVisitor &visitor)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = ark::Runtime::GetCurrent()->GetPandaVM();
    mem::RootManager<EtsLanguageConfig> rootManager(vm);
    auto *managedThread = ManagedThread::CastFromMutator(this);
    rootManager.VisitRootsForThread(managedThread, [&visitor](mem::GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitor(reinterpret_cast<ark::mem::RefField<> &>(*root.GetObjectPointer()));
    });
}

#endif

void Mutator::MakeTSANHappyForThreadState()
{
    TSAN_ANNOTATE_HAPPENS_AFTER(&fms_);
}

}  // namespace ark
