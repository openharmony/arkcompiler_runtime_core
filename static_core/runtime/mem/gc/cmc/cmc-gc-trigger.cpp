/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "runtime/mem/gc/cmc/cmc-gc-trigger.h"
#if defined(ARK_USE_COMMON_RUNTIME)
#include "runtime/mem/gc/cmc/heap/allocator/allocator.h"
#include "runtime/mem/gc/cmc/heap/allocator/regional_heap.h"
#include "runtime/mem/gc/cmc/heap/heap.h"
#include "runtime/mem/gc/cmc/cmc-allocator.h"
#endif  // ARK_USE_COMMON_RUNTIME

namespace ark::mem {

void GCCmcTrigger::TriggerGcIfNeeded([[maybe_unused]] GC *gc)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    if (!common_vm::Heap::GetHeap().IsGcStarted()) {
        // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
        // on other reads or writes
        uint64_t gcFinishTime = lastGcFinishTimeNs_.load(std::memory_order_relaxed);
        uint64_t minIntervalNs = common_vm::Heap::GetHeap().GetGCParam().gcInterval;
        if (gcFinishTime != 0 && time::GetCurrentTimeInNanos() - gcFinishTime < minIntervalNs) {
            return;
        }
        auto *cmcAllocator = static_cast<CMCObjectAllocator *>(gc->GetObjectAllocator());
        size_t threshold = cmcAllocator->GetHeapThreshold();
        size_t allocated = common_vm::Heap::GetHeap().GetAllocator().GetAllocatedBytes();
        if (allocated >= threshold) {
            if (cmcAllocator->ShouldRequestYoung()) {
                LOG(DEBUG, GC) << "request heu gc: young " << allocated << ", threshold " << threshold;
                auto task = MakePandaUnique<ark::GCTask>(GCTaskCause::YOUNG_GC_CAUSE);
                gc->Trigger(std::move(task));
            } else {
                LOG(DEBUG, GC) << "request heu gc: allocated " << allocated << ", threshold " << threshold;
                auto task = MakePandaUnique<ark::GCTask>(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
                gc->Trigger(std::move(task));
            }
        }
    }
#endif  // ARK_USE_COMMON_RUNTIME
}

}  // namespace ark::mem
