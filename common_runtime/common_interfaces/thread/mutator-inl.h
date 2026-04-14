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

#include "common_interfaces/thread/mutator.h"

#include "common_interfaces/base/common.h"

namespace common_vm {
inline void Mutator::DoEnterSaferegion()
{
    // set current mutator in saferegion.
    SetInSaferegion(SAFE_REGION_TRUE);
}

inline bool Mutator::EnterSaferegion([[maybe_unused]] bool updateUnwindContext) noexcept
{
    if (LIKELY_CC(!InSaferegion())) {
        DoEnterSaferegion();
        return true;
    }
    return false;
}

inline bool Mutator::LeaveSaferegion() noexcept
{
    if (LIKELY_CC(InSaferegion())) {
        DoLeaveSaferegion();
        return true;
    }
    return false;
}

void Mutator::TransferToRunning()
{
    DoLeaveSaferegion();
}

void Mutator::TransferToNative()
{
    DoEnterSaferegion();
}

bool Mutator::TransferToRunningIfInNative()
{
    return LeaveSaferegion();
}

bool Mutator::TransferToNativeIfInRunning()
{
    return EnterSaferegion(false);
}

}  // namespace common_vm
#endif  // COMMON_RUNTIME_COMMON_INTERFACES_THREAD_MUTATOR_INL_H
