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

#ifndef COMMON_INTERFACES_THREAD_THREAD_HOLDER_INL_H
#define COMMON_INTERFACES_THREAD_THREAD_HOLDER_INL_H

#include <unordered_set>

#include "common_interfaces/thread/thread_holder.h"
#include "common_interfaces/thread/mutator-inl.h"

namespace common {
void ThreadHolder::TransferToRunning()
{
    mutator_->DoLeaveSaferegion();
}

void ThreadHolder::TransferToNative()
{
    mutator_->DoEnterSaferegion();
}

bool ThreadHolder::TransferToRunningIfInNative()
{
    return mutator_->LeaveSaferegion();
}

bool ThreadHolder::TransferToNativeIfInRunning()
{
    return mutator_->EnterSaferegion(false);
}
}  // namespace common
#endif  // COMMON_INTERFACES_THREAD_THREAD_HOLDER_INL_H
