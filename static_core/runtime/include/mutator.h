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

#ifndef PANDA_RUNTIME_INCLUDE_MUTATOR_H
#define PANDA_RUNTIME_INCLUDE_MUTATOR_H

#include <cstdint>
#include <type_traits>

#include "runtime/include/locks.h"
#include "runtime/include/mutator_status.h"

#if defined(ARK_USE_COMMON_RUNTIME)
#include "common_interfaces/thread/mutator.h"
#include "common_interfaces/thread/mutator-inl.h"
#endif  // ARK_USE_COMMON_RUNTIME

namespace ark {

// Forward declarations
class PandaVM;
namespace mem {
class GCBarrierSet;
}  // namespace mem

/**
 * @brief Represents an entity that can mutate the managed heap state.
 *
 * The Mutator class is the central abstraction for entities that interact with the managed heap
 * by reading, writing, or allocating objects. Generally, an OS thread can execute several mutators,
 * and a mutator can be executed on several OS threads.
 *
 * @note Mutator instances are associated with threads via thread-local storage (TLS).
 *       Use GetCurrent() and SetCurrent() to access the current thread's mutator.
 */
#if defined(ARK_USE_COMMON_RUNTIME)
class Mutator : public common_vm::Mutator {
#else
class Mutator {
#endif
public:
    /// @enum Indicates the type of mutator
    enum class MutatorType {
        NONE,      // Uninitialized mutator, should not be used in runtime structures
        MANAGED,   // The mutator can execute managed code
        COMPILER,  // Mutator for compiler threads (JIT)
        GC         // GC mutator manipulates heap structure
    };

    Mutator(PandaVM *vm, MutatorType type);

    NO_COPY_SEMANTIC(Mutator);
    NO_MOVE_SEMANTIC(Mutator);

    virtual ~Mutator() = default;

    /// @returns The mutator from the current thread (TLS)
    PANDA_PUBLIC_API static Mutator *GetCurrent();

    /**
     * @brief Sets the mutator in the current thread (TLS)
     * @param mutator The mutator to set
     */
    PANDA_PUBLIC_API static void SetCurrent(Mutator *mutator);

    static constexpr size_t GetVmOffset()
    {
        return MEMBER_OFFSET(Mutator, vm_);
    }

    static constexpr uint32_t GetTlsPreWrbEntrypointOffset()
    {
        return MEMBER_OFFSET(Mutator, preWrbEntrypoint_);
    }

    ALWAYS_INLINE MutatorLock *GetMutatorLock() const
    {
        return mutatorLock_;
    }

    ALWAYS_INLINE PandaVM *GetVM() const
    {
        return vm_;
    }

    ALWAYS_INLINE MutatorType GetMutatorType() const
    {
        return type_;
    }

    ALWAYS_INLINE mem::GCBarrierSet *GetBarrierSet() const
    {
        return barrierSet_;
    }

    ALWAYS_INLINE void *GetPreWrbEntrypoint() const
    {
        // Atomic with relaxed order reason: only atomicity and modification order consistency needed
        return preWrbEntrypoint_.load(std::memory_order_relaxed);
    }

    ALWAYS_INLINE void SetPreWrbEntrypoint(void *entry)
    {
        preWrbEntrypoint_ = entry;
    }

    virtual void OnRuntimeTerminated() {}

    virtual void PrintSuspensionStackIfNeeded() {}

    // NO_THREAD_SANITIZE for invalid TSAN data race report
    NO_THREAD_SANITIZE bool TestAllFlags() const;

    PANDA_PUBLIC_API enum MutatorStatus GetStatus() const;

    void InitializeMutatorFlag();

    void CleanUpMutatorStatus();

#if defined(ARK_USE_COMMON_RUNTIME)
    PANDA_PUBLIC_API NO_THREAD_SAFETY_ANALYSIS void UpdateStatus(enum MutatorStatus status);
#else
    PANDA_PUBLIC_API void UpdateStatus(enum MutatorStatus status);
#endif

    ALWAYS_INLINE bool IsSuspended() const
    {
#if !defined(ARK_USE_COMMON_RUNTIME)
        return ReadFlag(SUSPEND_REQUEST);
#else
        return common_vm::Mutator::HasSuspendRequest();
#endif
    }

    ALWAYS_INLINE bool IsRuntimeTerminated() const
    {
#if !defined(ARK_USE_COMMON_RUNTIME)
        return ReadFlag(RUNTIME_TERMINATION_REQUEST);
#else
        return false;
#endif
    }

    ALWAYS_INLINE void SetRuntimeTerminated()
    {
#if !defined(ARK_USE_COMMON_RUNTIME)
        SetFlag(RUNTIME_TERMINATION_REQUEST);
#else
        UNREACHABLE();
#endif
    }

    bool IsThreadAlive() const
    {
        return GetStatus() != MutatorStatus::FINISHED;
    }

    static constexpr uint32_t GetFlagOffset()
    {
        return MEMBER_OFFSET(Mutator, fms_);
    }

    static void InitializeInitMutatorFlag();

    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    /// Transition to suspended and back to runnable, re-acquire share on mutator_lock_
    PANDA_PUBLIC_API void SuspendCheck() NO_THREAD_SAFETY_ANALYSIS;

    PANDA_PUBLIC_API void SuspendImpl(bool internalSuspend = false);

    PANDA_PUBLIC_API void ResumeImpl(bool internalResume = false);

    PANDA_PUBLIC_API void Safepoint();

    PANDA_PUBLIC_API void SafepointPoll();

    PANDA_PUBLIC_API bool IsUserSuspended() const;

    PANDA_PUBLIC_API void WaitSuspension();

#if defined(ARK_USE_COMMON_RUNTIME)
    void VisitMutatorRoots(const common_vm::RefFieldVisitor &visitor) override;
    void UpdateReadBarrierEntrypoint(common_vm::GCPhase phase) override;
#endif

    void MakeTSANHappyForThreadState();

#if defined(SAFEPOINT_TIME_CHECKER_ENABLED)
    virtual void ResetSafepointTimer([[maybe_unused]] bool needRecord) {}
#endif  // SAFEPOINT_TIME_CHECKER_ENABLED

#if defined(ARK_USE_COMMON_RUNTIME)
    void BindMutator();

    void UnbindMutator();
#endif  // ARK_USE_COMMON_RUNTIME

#if !defined(NDEBUG)
    MutatorLock::MutatorLockState GetLockState() const
    {
        return lockState_;
    }

    void SetLockState(MutatorLock::MutatorLockState state)
    {
        lockState_ = state;
    }
#endif  // !NDEBUG

    /* @sync 1
     * @description This synchronization point can be used to insert a new attribute or method
     * into ManagedThread class.
     */
private:
    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    void TransitionFromRunningToSuspended(enum MutatorStatus status) NO_THREAD_SAFETY_ANALYSIS;

    // Separate functions for NO_THREAD_SANITIZE to suppress TSAN data race report
    NO_THREAD_SANITIZE uint32_t ReadFlagsAndMutatorStatusUnsafe();

    // NO_THREAD_SANITIZE for invalid TSAN data race report
    NO_THREAD_SANITIZE bool ReadFlag(MutatorFlag flag) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return (fms_.asStruct.flags & static_cast<uint16_t>(flag)) != 0;
    }

    // Separate functions for NO_THREAD_SANITIZE to suppress TSAN data race report
    NO_THREAD_SANITIZE uint32_t ReadFlagsUnsafe() const;

    void SetFlag(MutatorFlag flag);

    void ClearFlag(MutatorFlag flag);

    enum SafepointFlag : bool { DONT_CHECK_SAFEPOINT = false, CHECK_SAFEPOINT = true };
    enum ReadlockFlag : bool { NO_READLOCK = false, READLOCK = true };

    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    template <SafepointFlag SAFEPOINT = DONT_CHECK_SAFEPOINT, ReadlockFlag READLOCK_FLAG = NO_READLOCK>
    void StoreStatus(MutatorStatus status) NO_THREAD_SAFETY_ANALYSIS
    {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
        while (true) {
            union FlagsAndMutatorStatus oldFms {
            };
            union FlagsAndMutatorStatus newFms {
            };
            oldFms.asInt = ReadFlagsAndMutatorStatusUnsafe();

            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (SAFEPOINT == CHECK_SAFEPOINT) {  // NOLINT(bugprone-suspicious-semicolon)
                if (oldFms.asStructNonvolatile.flags != initialMutatorFlag_) {
                    // someone requires a safepoint
                    SafepointPoll();
                    continue;
                }
            }

            // mutator lock should be acquired before change status
            // to avoid blocking in running state
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (READLOCK_FLAG == READLOCK) {  // NOLINT(bugprone-suspicious-semicolon)
                GetMutatorLock()->ReadLock();
            }

            // clang-format conflicts with CodeCheckAgent, so disable it here
            // clang-format off
            // If it's not required to check safepoint, CAS would behave the same as
            // regular STORE, just will do extra attempts, in this case it makes sense
            // to use STORE for just thread status 16-bit word.
            if constexpr (SAFEPOINT == DONT_CHECK_SAFEPOINT) {
                auto newStatus = static_cast<uint16_t>(status);
                // Atomic with release order reason: data race with other mutators
                fms_.asAtomic.status.store(newStatus, std::memory_order_release);
                break;
            }

            // if READLOCK, there's chance, someone changed the flags
            // in parallel, let's check before the CAS.
            if constexpr (READLOCK_FLAG == READLOCK) {
                if (ReadFlagsUnsafe() != oldFms.asStructNonvolatile.flags) {
                    GetMutatorLock()->Unlock();
                    continue;
                }
            }

            newFms.asStructNonvolatile.flags = oldFms.asStructNonvolatile.flags;
            newFms.asStructNonvolatile.status = status;
            // Atomic with release order reason: data race with other mutators
            if (fms_.asAtomicInt.compare_exchange_weak(
                oldFms.asNonvolatileInt, newFms.asNonvolatileInt, std::memory_order_release)) {
                // If CAS succeeded, we set new status and no request occurred here, safe to proceed.
                break;
            }
            // Release mutator lock to acquire it on the next loop iteration
            // clang-format on
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (READLOCK_FLAG == READLOCK) {  // NOLINT(bugprone-suspicious-semicolon)
                GetMutatorLock()->Unlock();
            }
        }
        // NOLINTEND(cppcoreguidelines-pro-type-union-access)
    }
    PandaVM *vm_ {nullptr};
    MutatorLock *mutatorLock_ {nullptr};
    MutatorType type_ {MutatorType::NONE};
#if !defined(NDEBUG)
    MutatorLock::MutatorLockState lockState_ = MutatorLock::UNLOCKED;
#endif  // !NDEBUG

    // -- Mutator status part --

    union __attribute__((__aligned__(4))) FlagsAndMutatorStatus {
        FlagsAndMutatorStatus() = default;
        ~FlagsAndMutatorStatus() = default;

        volatile uint32_t asInt;
        uint32_t asNonvolatileInt;
        std::atomic_uint32_t asAtomicInt;

        struct __attribute__((packed)) {
            volatile uint16_t flags;
            volatile enum MutatorStatus status;
        } asStruct;

        struct __attribute__((packed)) {
            uint16_t flags;
            enum MutatorStatus status;
        } asStructNonvolatile;

        struct {
            std::atomic_uint16_t flags;
            std::atomic_uint16_t status;
        } asAtomic;

        NO_COPY_SEMANTIC(FlagsAndMutatorStatus);
        NO_MOVE_SEMANTIC(FlagsAndMutatorStatus);
    };

    PANDA_PUBLIC_API static MutatorFlag initialMutatorFlag_;

    FlagsAndMutatorStatus fms_ {};
    static_assert(sizeof(fms_) == sizeof(uint32_t), "Wrong fms_ size");
    os::memory::ConditionVariable suspendVar_ GUARDED_BY(suspendLock_);
    os::memory::Mutex suspendLock_;
    uint32_t suspendCount_ GUARDED_BY(suspendLock_) = 0;
    std::atomic_uint32_t userCodeSuspendCount_ {0};

    // -- GC barriers part --
    mem::GCBarrierSet *barrierSet_ {nullptr};
    std::atomic<void *> preWrbEntrypoint_ {nullptr};  // if NOT nullptr, stores pointer to PreWrbFunc and indicates we
                                                      // are currently in concurrent marking phase
};

/**
 * @brief RAII wrapper for setting and unsetting the current thread's mutator.
 *
 * This template class provides a scope-based (RAII) mechanism for managing the
 * association between a mutator and the current thread's thread-local storage (TLS).
 * It sets the specified mutator as the current mutator on construction and
 * automatically clears it on destruction.
 *
 * @tparam MutatorType The type of mutator being managed. Must be compatible with
 *                     the Mutator class interface (i.e., inherit from or be Mutator).
 *
 * @note The class asserts that no mutator is already set in the current thread
 *       before setting the new one.
 */
template <typename MutatorType, typename = std::enable_if_t<std::is_base_of_v<Mutator, MutatorType>>>
class ScopedCurrentMutator {
public:
    explicit ScopedCurrentMutator(MutatorType *mutator) : mutator_(mutator)
    {
        ASSERT(Mutator::GetCurrent() == nullptr);
        // Set current mutator
        Mutator::SetCurrent(mutator_);
    }

    ~ScopedCurrentMutator()
    {
        // Reset current mutator
        Mutator::SetCurrent(nullptr);
    }

    NO_COPY_SEMANTIC(ScopedCurrentMutator);
    NO_MOVE_SEMANTIC(ScopedCurrentMutator);

private:
    MutatorType *mutator_;
};  // class ScopedCurrentMutator

std::ostream &operator<<(std::ostream &stream, Mutator::MutatorType type);

/**
 * @class ScopedChangeMutatorStatus provides a scope-based (RAII) mechanism for managing
 * the passed mutator status. It keeps the actual status and updates for the new status in the constructor,
 * and updates back in the destructor
 * @see Mutator::UpdateStatus
 */
class PANDA_PUBLIC_API ScopedChangeMutatorStatus {
public:
    ScopedChangeMutatorStatus(Mutator *mutator, MutatorStatus newStatus) : mutator_(mutator)
    {
        ASSERT(mutator_ != nullptr);
        oldStatus_ = mutator_->GetStatus();
        mutator_->UpdateStatus(newStatus);
    }

    NO_COPY_SEMANTIC(ScopedChangeMutatorStatus);
    NO_MOVE_SEMANTIC(ScopedChangeMutatorStatus);

    ~ScopedChangeMutatorStatus()
    {
        mutator_->UpdateStatus(oldStatus_);
    }

private:
    Mutator *mutator_;
    MutatorStatus oldStatus_;
};  // class ScopedChangeMutatorStatus

}  // namespace ark

#if defined(PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS) && !defined(PANDA_TARGET_ANDROID)
#include "platforms/mobile/runtime/mutator-inl.cpp"
#endif  // PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS

#endif  // PANDA_RUNTIME_INCLUDE_MUTATOR_H
