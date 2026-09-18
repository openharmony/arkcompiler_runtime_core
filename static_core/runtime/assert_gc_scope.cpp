/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "assert_gc_scope.h"

namespace ark {

std::atomic<int> AssertGCScopeT::gcFlag_ = 0;

void AssertGCScopeT::Enter()
{
    // Atomic with relaxed order reason: there is no synchronization or ordering constraint on other reads or writes.
    gcFlag_.fetch_add(1, std::memory_order_relaxed);
}

void AssertGCScopeT::Exit()
{
    // Atomic with relaxed order reason: there is no synchronization or ordering constraint on other reads or writes.
    gcFlag_.fetch_sub(1, std::memory_order_relaxed);
}

bool AssertGCScopeT::IsAllowed()
{
    // Atomic with relaxed order reason: data race with gcFlag_ with no synchronization or ordering constraints imposed
    // on other reads or writes
    return AssertGCScopeT::gcFlag_.load(std::memory_order_relaxed) == 0;
}
}  // namespace ark
