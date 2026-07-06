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

#ifndef PANDA_RUNTIME_MEM_GC_CMC_CMC_GC_TRIGGER_H
#define PANDA_RUNTIME_MEM_GC_CMC_CMC_GC_TRIGGER_H

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "runtime/mem/gc/gc_trigger.h"

namespace ark::mem {

// Trigger for CMC-GC uses thresholds
class GCCmcTrigger : public GCTrigger {
public:
    GCTriggerType GetType() const override
    {
        return GCTriggerType::CMC_GC;
    }

    void TriggerGcIfNeeded(GC *gc) override;

    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override {}
    void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
        // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
        // on other reads or writes
        lastGcFinishTimeNs_.store(time::GetCurrentTimeInNanos(), std::memory_order_relaxed);
    }

private:
    std::atomic<uint64_t> lastGcFinishTimeNs_ {0};
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_CMC_CMC_GC_TRIGGER_H
