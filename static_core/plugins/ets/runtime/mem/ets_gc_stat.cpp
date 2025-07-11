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

#include "ets_gc_stat.h"

namespace ark::ets {

void FullGCLongTimeListener::GCStarted([[maybe_unused]] const ark::GCTask &task, [[maybe_unused]] size_t heapSize)
{
    startTime_ = time::GetCurrentTimeInNanos();
}

void FullGCLongTimeListener::GCFinished([[maybe_unused]] const ark::GCTask &task,
                                        [[maybe_unused]] size_t heapSizeBeforeGc, [[maybe_unused]] size_t heapSize)
{
    auto duration = time::GetCurrentTimeInNanos() - startTime_;
    if (task.collectionType == GCCollectionType::FULL && duration > LONG_GC_THRESHOLD_NS) {
        LOG(INFO, GC) << "Full GC running time exceed 40 ms, used " << duration << " ns.";
        ++fullGCLongTimeCounter_;
    }
}

uint64_t FullGCLongTimeListener::GetFullGCLongTimeCount() const
{
    return fullGCLongTimeCounter_;
}

FullGCLongTimeListener *FullGCLongTimeListener::CreateGCListener()
{
    return new FullGCLongTimeListener();
}

}  // namespace ark::ets
