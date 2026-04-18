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

#include "runtime/include/managed_thread.h"
#include "runtime/include/panda_vm.h"

#if defined(ARK_USE_COMMON_RUNTIME)
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"
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
    }
    ASSERT(type_ != MutatorType::NONE);
    InitializeMutatorFlag();
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
#if !defined(ARK_USE_COMMON_RUNTIME)
    return (fms_.asStruct.flags) != initialMutatorFlag_;  // NOLINT(cppcoreguidelines-pro-type-union-access)
#else
    return common_vm::Mutator::HasSuspendRequest();
#endif
}

void Mutator::SetFlag(MutatorFlag flag)
{
    // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    fms_.asAtomic.flags.fetch_or(flag, std::memory_order_seq_cst);
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
#if !defined(ARK_USE_COMMON_RUNTIME)
    // Atomic with acquire order reason: data race with flags with dependecies on reads after
    // the load which should become visible
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return static_cast<enum MutatorStatus>(fms_.asAtomic.status.load(std::memory_order_acquire));
#else
    if (common_vm::Mutator::IsInRunningState()) {
        return MutatorStatus::RUNNING;
    }
    return MutatorStatus::NATIVE;
#endif
}

uint32_t Mutator::ReadFlagsUnsafe() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return fms_.asStruct.flags;
}

void Mutator::InitializeMutatorFlag()
{
#if !defined(ARK_USE_COMMON_RUNTIME)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    fms_.asInt = initialMutatorFlag_;
#endif
}

void Mutator::CleanUpMutatorStatus()
{
#if !defined(ARK_USE_COMMON_RUNTIME)
    InitializeMutatorFlag();
    StoreStatus<DONT_CHECK_SAFEPOINT, NO_READLOCK>(MutatorStatus::CREATED);
#endif
}

#if defined(ARK_USE_COMMON_RUNTIME)
NO_THREAD_SAFETY_ANALYSIS void Mutator::UpdateStatus(enum MutatorStatus status)
#else
void Mutator::UpdateStatus(enum MutatorStatus status)
#endif
{
    MutatorStatus oldStatus = GetStatus();
#if !defined(ARK_USE_COMMON_RUNTIME)
    if (oldStatus == MutatorStatus::RUNNING && status != MutatorStatus::RUNNING) {
        TransitionFromRunningToSuspended(status);
    } else if (oldStatus != MutatorStatus::RUNNING && status == MutatorStatus::RUNNING) {
        // NB! This thread is treated as suspended so when we transition from suspended state to
        // running we need to check suspension flag and counter so SafepointPoll has to be done before
        // acquiring mutator_lock.
        // StoreStatus acquires lock here
        StoreStatus<CHECK_SAFEPOINT, READLOCK>(MutatorStatus::RUNNING);
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
#else
    if (oldStatus == MutatorStatus::RUNNING && status != MutatorStatus::RUNNING) {
        common_vm::Mutator::TransferToNativeIfInRunning();
        GetMutatorLock()->Unlock();
    } else if (oldStatus != MutatorStatus::RUNNING && status == MutatorStatus::RUNNING) {
        common_vm::Mutator::TransferToRunningIfInNative();
        GetMutatorLock()->ReadLock();
    }
#endif
}

void Mutator::TransitionFromRunningToSuspended(enum MutatorStatus status)
{
    // Do Unlock after StoreStatus, because the thread requesting a suspension should see an updated status
    StoreStatus(status);
    GetMutatorLock()->Unlock();
}

void Mutator::SuspendCheck()
{
#if !defined(ARK_USE_COMMON_RUNTIME)
    // We should use internal suspension to avoid missing call of IncSuspend
    SuspendImpl(true);
    GetMutatorLock()->Unlock();
    GetMutatorLock()->ReadLock();
    ResumeImpl(true);
#else
    UNREACHABLE();
#endif
}

void Mutator::SuspendImpl(bool internalSuspend)
{
#if !defined(ARK_USE_COMMON_RUNTIME)
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
#else
    UNREACHABLE();
#endif
}

void Mutator::ResumeImpl(bool internalResume)
{
#if !defined(ARK_USE_COMMON_RUNTIME)
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
#else
    UNREACHABLE();
#endif
}

void Mutator::SafepointPoll()
{
#if !defined(ARK_USE_COMMON_RUNTIME)
    if (TestAllFlags()) {
        trace::ScopedTrace scopedTrace("RunSafepoint");
        Safepoint();
    }
#else
    common_vm::Mutator::CheckSafepointIfSuspended();
#endif
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
#if !defined(ARK_USE_COMMON_RUNTIME)
    return userCodeSuspendCount_ > 0;
#else
    UNREACHABLE();
    return false;
#endif
}

void Mutator::WaitSuspension()
{
#if !defined(ARK_USE_COMMON_RUNTIME)
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
#else
    common_vm::Mutator::WaitSuspension();
#endif
}

#if defined(ARK_USE_COMMON_RUNTIME)
void Mutator::VisitMutatorRoots(const common_vm::RefFieldVisitor &visitor)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = ark::Runtime::GetCurrent()->GetPandaVM();
    mem::RootManager<EtsLanguageConfig> rootManager(vm);
    auto *managedThread = ManagedThread::CastFromMutator(this);
    rootManager.VisitRootsForThread(managedThread, [&visitor](mem::GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitor(reinterpret_cast<common_vm::RefField<> &>(*root.GetObjectPointer()));
    });
}

void Mutator::UpdateReadBarrierEntrypoint(common_vm::GCPhase phase)
{
    auto *vm = ark::Runtime::GetCurrent()->GetPandaVM();
    auto *gc = static_cast<mem::CMCGCAdapter<EtsLanguageConfig> *>(vm->GetGC());
    auto *managedThread = ManagedThread::CastFromMutator(this);
    if (phase >= common_vm::GCPhase::GC_PHASE_PRECOPY) {
        gc->EnableReadBarrier(managedThread);
    } else if (phase == common_vm::GCPhase::GC_PHASE_IDLE) {
        gc->DisableReadBarrier(managedThread);
    }
    // Check if a new phase has been added
    ASSERT(phase == common_vm::GCPhase::GC_PHASE_ENUM || phase == common_vm::GCPhase::GC_PHASE_MARK ||
           phase == common_vm::GCPhase::GC_PHASE_PRECOPY || phase == common_vm::GCPhase::GC_PHASE_COPY ||
           phase == common_vm::GCPhase::GC_PHASE_FIX || phase == common_vm::GCPhase::GC_PHASE_IDLE ||
           phase == common_vm::GCPhase::GC_PHASE_POST_MARK || phase == common_vm::GCPhase::GC_PHASE_FINAL_MARK ||
           phase == common_vm::GCPhase::GC_PHASE_REMARK_SATB || phase == common_vm::GCPhase::GC_PHASE_YOUNG_COPY);
}
#endif

void Mutator::MakeTSANHappyForThreadState()
{
#if !defined(ARK_USE_COMMON_RUNTIME)
    TSAN_ANNOTATE_HAPPENS_AFTER(&fms_);
#else
    UNREACHABLE();
#endif
}

#if defined(ARK_USE_COMMON_RUNTIME)

void Mutator::BindMutator()
{
    common_vm::Mutator::BindMutator();
}

void Mutator::UnbindMutator()
{
    if (this != GetCurrent()) {
        return;
    }
    common_vm::Mutator::UnbindMutator();
}

#endif  // ARK_USE_COMMON_RUNTIME

}  // namespace ark
