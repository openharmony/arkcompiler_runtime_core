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

#ifndef COMMON_INTERFACES_THREAD_THREAD_STATE_TRANSITION_H
#define COMMON_INTERFACES_THREAD_THREAD_STATE_TRANSITION_H

#include "common_interfaces/thread/mutator-inl.h"
#include "common_interfaces/thread/thread_state.h"
#include "common_components/mutator/mutator_manager.h"

namespace common {

template <typename T, ThreadState newState>
class MutatorStateTransitionScope final {
    static_assert(std::is_base_of_v<Mutator, T>);

public:
    explicit MutatorStateTransitionScope(T *self) : self_(self)
    {
        DCHECK_CC(self_ != nullptr);
        if constexpr (newState == ThreadState::RUNNING) {
            hasSwitchState_ = self_->TransferToRunningIfInNative();
        } else {
            hasSwitchState_ = self_->TransferToNativeIfInRunning();
        }
    }

    ~MutatorStateTransitionScope()
    {
        if (hasSwitchState_) {
            if constexpr (newState == ThreadState::RUNNING) {
                self_->TransferToNative();
            } else {
                self_->TransferToRunning();
            }
        }
    }

private:
    T *self_;
    ThreadState oldState_;
    bool hasSwitchState_ {false};
    NO_COPY_SEMANTIC_CC(MutatorStateTransitionScope);
};

class MutatorSuspensionScope final {
public:
    explicit MutatorSuspensionScope(Mutator *self) : scope_(self)
    {
        DCHECK_CC(!self->IsInRunningState());
    }

    ~MutatorSuspensionScope() = default;

private:
    MutatorStateTransitionScope<Mutator, ThreadState::IS_SUSPENDED> scope_;
    NO_COPY_SEMANTIC_CC(MutatorSuspensionScope);
};

class MutatorNativeScope final {
public:
    explicit MutatorNativeScope(Mutator *self) : scope_(self)
    {
        DCHECK_CC(!self->IsInRunningState());
    }

    ~MutatorNativeScope() = default;

private:
    MutatorStateTransitionScope<Mutator, ThreadState::NATIVE> scope_;
    NO_COPY_SEMANTIC_CC(MutatorNativeScope);
};

template <typename T>
class MutatorManagedScope final {
    static_assert(std::is_base_of_v<Mutator, T>);

public:
    explicit MutatorManagedScope(T *self) : scope_(self) {}

    ~MutatorManagedScope() = default;

private:
    MutatorStateTransitionScope<T, ThreadState::RUNNING> scope_;
    NO_COPY_SEMANTIC_CC(MutatorManagedScope);
};

template <typename T>
class SuspendAllScope final {
    static_assert(std::is_base_of_v<Mutator, T>);

public:
    explicit SuspendAllScope(T *self) : self_(self), scope_(self)
    {
#if defined(TODO_MACRO)
        TRACE_GC(GCStats::Scope::ScopeId::SuspendAll, SharedHeap::GetInstance()->GetEcmaGCStats());
        ECMA_BYTRACE_NAME(HITRACE_LEVEL_COMMERCIAL, HITRACE_TAG_ARK, "SuspendAll", "");
#endif
        DCHECK_CC(self_ != nullptr);
        DCHECK_CC(!self_->IsInRunningState());
        MutatorManager::Instance().StopTheWorld(false, GCPhase::GC_PHASE_IDLE);
    }

    ~SuspendAllScope()
    {
#if defined(TODO_MACRO)
        TRACE_GC(GCStats::Scope::ScopeId::ResumeAll, SharedHeap::GetInstance()->GetEcmaGCStats());
        ECMA_BYTRACE_NAME(HITRACE_LEVEL_COMMERCIAL, HITRACE_TAG_ARK, "ResumeAll", "");
#endif
        MutatorManager::Instance().StartTheWorld();
    }

private:
    T *self_;
    MutatorStateTransitionScope<T, ThreadState::IS_SUSPENDED> scope_;
    NO_COPY_SEMANTIC_CC(SuspendAllScope);
};
}  // namespace common
#endif  // COMMON_INTERFACES_THREAD_THREAD_STATE_TRANSITION_H
