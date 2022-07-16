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

#ifndef PANDA_RUNTIME_GC_TASK_H_
#define PANDA_RUNTIME_GC_TASK_H_

#include <cstdint>

#include "runtime/include/mem/allocator.h"

namespace panda {

class ManagedThread;

namespace mem {
class GC;
}  // namespace mem

/**
 * Causes are ordered by priority. Bigger index - means bigger priority in GC
 */
enum class GCTaskCause : uint8_t {
    INVALID_CAUSE = 0,
    YOUNG_GC_CAUSE,  // if young space is full
    PYGOTE_FORK_CAUSE,
    STARTUP_COMPLETE_CAUSE,
    NATIVE_ALLOC_CAUSE,
    HEAP_USAGE_THRESHOLD_CAUSE,
    EXPLICIT_CAUSE,  // System.gc
    OOM_CAUSE,       // if all heap is full
};

/**
 * Collection types are ordered by gc collection scale
 *
 * @see GCTask::UpdateGCCollectionType
 */
enum class GCCollectionType : uint8_t { NONE = 0, YOUNG, MIXED, TENURED, FULL };

struct GCTask {
    explicit GCTask(GCTaskCause reason, uint64_t target_time = 0U) : GCTask(reason, target_time, nullptr) {}

    explicit GCTask(GCTaskCause reason, ManagedThread *caller_thread) : GCTask(reason, 0U, caller_thread) {}

    explicit GCTask(GCTaskCause reason, uint64_t target_time, ManagedThread *caller_thread)
    {
        this->reason_ = reason;
        this->target_time_ = target_time;
        this->caller_thread_ = caller_thread;
        this->collection_type_ = GCCollectionType::NONE;
    }

    /**
     * Update collection type in the gc task if the new coolcetion type is bigger
     * @param collection_type new gc collection type
     */
    void UpdateGCCollectionType(GCCollectionType collection_type);

    uint64_t GetTargetTime() const
    {
        return target_time_;
    }

    // NOLINTNEXTLINE(google-runtime-references)
    virtual void Run(mem::GC &gc);

    virtual void Release(mem::InternalAllocatorPtr allocator);

    virtual ~GCTask() = default;

    GCTask(const GCTask &other) = default;
    GCTask &operator=(const GCTask &other) = default;
    GCTask(GCTask &&other) = default;
    GCTask &operator=(GCTask &&other) = default;

    GCTaskCause reason_;                // NOLINT(misc-non-private-member-variables-in-classes)
    GCCollectionType collection_type_;  // NOLINT(misc-non-private-member-variables-in-classes)
    ManagedThread *caller_thread_;      // NOLINT(misc-non-private-member-variables-in-classes)

private:
    uint64_t target_time_;
};

std::ostream &operator<<(std::ostream &os, const GCTaskCause &cause);
std::ostream &operator<<(std::ostream &os, const GCCollectionType &collection_type);

}  // namespace panda

#endif  // PANDA_RUNTIME_GC_TASK_H_
