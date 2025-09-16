/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "runtime/include/managed_thread.h"
#include "runtime/include/runtime.h"
#include "runtime/include/safepoint_timer.h"

namespace ark {

TimerTable *SafepointTimerTable::safepointTimers_ = nullptr;

void SafepointTimerTable::Initialize(TimerTable::TimeDiff recordingTime)
{
    ASSERT(!IsInitialized());
    safepointTimers_ = Runtime::GetCurrent()->GetInternalAllocator()->New<TimerTable>(recordingTime);
}

void SafepointTimerTable::Destroy()
{
    ASSERT(IsInitialized());
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(safepointTimers_);
    safepointTimers_ = nullptr;
}

SafepointTimer::SafepointTimer(std::string_view name) : name_(name)
{
    SafepointTimerTable::RegisterTimer(ManagedThread::GetCurrent()->GetInternalId(), name_);
}

SafepointTimer::~SafepointTimer()
{
    SafepointTimerTable::DisarmTimer(ManagedThread::GetCurrent()->GetInternalId(), name_);
}

}  // namespace ark
