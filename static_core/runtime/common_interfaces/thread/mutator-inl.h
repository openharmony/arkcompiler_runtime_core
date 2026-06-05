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

#ifndef COMMON_RUNTIME_COMMON_INTERFACES_THREAD_MUTATOR_INL_H
#define COMMON_RUNTIME_COMMON_INTERFACES_THREAD_MUTATOR_INL_H

#include "include/mutator.h"

#include "common_interfaces/base/common.h"
#include "include/mutator_status.h"

namespace ark::common_vm {
inline void Mutator::DoEnterSaferegion()
{
    SetInSaferegion(ark::MutatorStatus::NATIVE);
    static_cast<ark::Mutator *>(this)->GetMutatorLock()->Unlock();
}

inline bool Mutator::EnterSaferegion([[maybe_unused]] bool updateUnwindContext) noexcept
{
    if (LIKELY(!InSaferegion())) {
        DoEnterSaferegion();
        return true;
    }
    return false;
}

inline bool Mutator::LeaveSaferegion() noexcept
{
    if (LIKELY(InSaferegion())) {
        DoLeaveSaferegion();
        return true;
    }
    return false;
}

inline void Mutator::SetInSaferegion(ark::MutatorStatus state)
{
    static_cast<ark::Mutator *>(this)->StoreStatus(state);
}

inline bool Mutator::InSaferegion() const
{
    auto status = static_cast<const ark::Mutator *>(this)->GetStatus();
    return status != ark::MutatorStatus::RUNNING;
}

inline void Mutator::DoLeaveSaferegion()
{
    SetInSaferegion(ark::MutatorStatus::RUNNING);
    // go slow path if the mutator should suspend
    if (UNLIKELY(HasAnySuspensionRequest())) {
        HandleSuspensionRequest();
    }
}

inline bool Mutator::IsInRunningState() const
{
    return !InSaferegion();
}

inline void Mutator::SetSuspensionFlag(ark::MutatorFlag flag)
{
    static_cast<ark::Mutator *>(this)->SetFlag(flag);
}

inline void Mutator::ClearSuspensionFlag(ark::MutatorFlag flag)
{
    static_cast<ark::Mutator *>(this)->ClearFlag(flag);
}

inline uint32_t Mutator::GetSuspensionFlag() const
{
    return static_cast<const ark::Mutator *>(this)->ReadFlagsUnsafe();
}

inline bool Mutator::HasSuspensionRequest(ark::MutatorFlag flag) const
{
    return static_cast<const ark::Mutator *>(this)->ReadFlag(flag);
}

inline bool Mutator::HasAnySuspensionRequest() const
{
    return static_cast<const ark::Mutator *>(this)->TestAllFlags();
}

inline bool Mutator::HasAnySuspensionRequestExceptCallbacks() const
{
    uint32_t flag = static_cast<const ark::Mutator *>(this)->ReadFlagsUnsafe();
    return (flag & ~ark::SUSPEND_FOR_FINALIZE) != 0;
}

inline bool Mutator::CASSetSuspensionFlag(uint32_t oldFlag, uint32_t newFlag)
{
    return static_cast<ark::Mutator *>(this)->ExchangeFlags((ark::MutatorFlag)oldFlag, (ark::MutatorFlag)newFlag);
}

inline void Mutator::ClearFinalizeRequest()
{
    ClearSuspensionFlag(ark::SUSPEND_FOR_FINALIZE);
}

inline void Mutator::SetFinalizeRequest()
{
    SetSuspensionFlag(ark::SUSPEND_FOR_FINALIZE);
}

}  // namespace ark::common_vm
#endif  // COMMON_RUNTIME_COMMON_INTERFACES_THREAD_MUTATOR_INL_H
