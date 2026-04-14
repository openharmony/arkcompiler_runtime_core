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

#ifndef COMMON_RUNTIME_COMMON_INTERFACES_THREAD_THREAD_STATE_TRANSITION_H
#define COMMON_RUNTIME_COMMON_INTERFACES_THREAD_THREAD_STATE_TRANSITION_H

#include "common_interfaces/thread/mutator-inl.h"
#include "common_components/mutator/mutator_manager.h"

namespace common_vm {

enum class MutatorState : uint16_t {
    RUNNING = 0,
    NATIVE = 1,
    WAIT = 2,
};

template <typename T, MutatorState newState>
class MutatorStateTransitionScope final {
    static_assert(std::is_base_of_v<Mutator, T>);

public:
    explicit MutatorStateTransitionScope(T *self) : self_(self)
    {
        DCHECK_CC(self_ != nullptr);
        if constexpr (newState == MutatorState::RUNNING) {
            hasSwitchState_ = self_->TransferToRunningIfInNative();
        } else {
            hasSwitchState_ = self_->TransferToNativeIfInRunning();
        }
    }

    ~MutatorStateTransitionScope()
    {
        if (hasSwitchState_) {
            if constexpr (newState == MutatorState::RUNNING) {
                self_->TransferToNative();
            } else {
                self_->TransferToRunning();
            }
        }
    }

private:
    T *self_;
    bool hasSwitchState_ {false};
    NO_COPY_SEMANTIC_CC(MutatorStateTransitionScope);
};

class MutatorNativeScope final {
public:
    explicit MutatorNativeScope(Mutator *self) : scope_(self)
    {
        DCHECK_CC(!self->IsInRunningState());
    }

    ~MutatorNativeScope() = default;

private:
    MutatorStateTransitionScope<Mutator, MutatorState::NATIVE> scope_;
    NO_COPY_SEMANTIC_CC(MutatorNativeScope);
};

template <typename T>
class MutatorManagedScope final {
    static_assert(std::is_base_of_v<Mutator, T>);

public:
    explicit MutatorManagedScope(T *self) : scope_(self) {}

    ~MutatorManagedScope() = default;

private:
    MutatorStateTransitionScope<T, MutatorState::RUNNING> scope_;
    NO_COPY_SEMANTIC_CC(MutatorManagedScope);
};
}  // namespace common_vm
#endif  // COMMON_RUNTIME_COMMON_INTERFACES_THREAD_THREAD_STATE_TRANSITION_H
