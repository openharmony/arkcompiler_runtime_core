/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COMPILER_QUEUE_AGED_COUNTER_PRIORITY_H_
#define PANDA_RUNTIME_COMPILER_QUEUE_AGED_COUNTER_PRIORITY_H_

#include "runtime/compiler_queue_counter_priority.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/method-inl.h"

namespace panda {

/** The aged counter priority queue works mostly as counter priority queue (see description),
 *  but it sorts the methods by its aged hotness counters.
 *  If the aged counter is less then some death value, it is considered as expired and is removed.
 *  Epoch duration and death counter is configured.
 *  This queue is thread unsafe (should be used under lock).
 */
class CompilerPriorityAgedCounterQueue : public CompilerPriorityCounterQueue {
public:
    explicit CompilerPriorityAgedCounterQueue(mem::InternalAllocatorPtr allocator, uint64_t max_length,
                                              uint64_t death_counter_value, uint64_t epoch_duration)
        : CompilerPriorityCounterQueue(allocator, max_length, 0 /* unused */)
    {
        death_counter_value_ = death_counter_value;
        epoch_duration_ = epoch_duration;
        if (epoch_duration_ <= 0) {
            LOG(FATAL, COMPILATION_QUEUE) << "Incorrect value of epoch duration: " << epoch_duration_;
        }
        SetQueueName("priority aged counter compilation queue");
    }

protected:
    bool UpdateCounterAndCheck(CompilationQueueElement *element) override
    {
        // Rounding
        uint64_t current_time = time::GetCurrentTimeInMillis();
        ASSERT(current_time >= element->GetTimestamp());
        uint64_t duration = current_time - element->GetTimestamp();
        uint64_t shift = duration / epoch_duration_;
        uint64_t aged_counter = 0;
        constexpr int MAX_SHIFT = 16;
        if (shift < MAX_SHIFT) {
            // hotness_counter is uint16_t and shifts more that 16 are useless
            // Moreover, shifts for 32 and more leads to strange failures
            // For greater values just use preset 0
            aged_counter = element->GetContext().GetMethod()->GetHotnessCounter() >> shift;
        }
        element->UpdateCounter(aged_counter);
        return (aged_counter < death_counter_value_);
    }

private:
    uint64_t death_counter_value_;
    uint64_t epoch_duration_;
};

}  // namespace panda

#endif  // PANDA_RUNTIME_COMPILER_QUEUE_AGED_COUNTER_PRIORITY_H_
