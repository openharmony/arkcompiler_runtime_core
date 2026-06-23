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

#include "common_components/heap/collector/task_queue.h"

#include "common_components/heap/collector/collector.h"

namespace ark::common_vm {
bool GCRunner::Execute(void *owner)
{
    ASSERT_PRINT(owner != nullptr, "task queue owner ptr should not be null!");
    auto *collectorProxy = reinterpret_cast<Collector *>(owner);

    switch (taskType_) {
        case GCTask::GCTaskType::GC_TASK_TERMINATE_GC: {
            return false;
        }
        case GCTask::GCTaskType::GC_TASK_INVOKE_GC: {
            GCStats::SetPrevGCStartTime(TimeUtil::NanoSeconds());
            auto task = ToGCTask();
            collectorProxy->RunGarbageCollection(taskIndex_, task);
            GCStats::SetPrevGCFinishTime(TimeUtil::NanoSeconds());
            break;
        }
        case GCTask::GCTaskType::GC_TASK_DUMP_HEAP: {  // LCOV_EXCL_BR_LINE
            LOG(FATAL, COMMON) << "Don't know how to dump heap";
            UNREACHABLE();
            break;
        }
        case GCTask::GCTaskType::GC_TASK_DUMP_HEAP_IDE: {  // LCOV_EXCL_BR_LINE
            LOG(FATAL, COMMON) << "Don't know how to dump heap OOM";
            UNREACHABLE();
            break;
        }

        case GCTask::GCTaskType::GC_TASK_DUMP_HEAP_OOM: {  // LCOV_EXCL_BR_LINE
            LOG(FATAL, COMMON) << "Don't know how to dump heap OOM";
            UNREACHABLE();
            break;
        }
        default:
            LOG(ERROR, COMMON) << "[GC] Error task type: " << static_cast<uint32_t>(taskType_) << " ignored!";
            break;
    }
    return true;
}
}  // namespace ark::common_vm
