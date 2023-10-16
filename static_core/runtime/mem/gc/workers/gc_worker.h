/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKER_H
#define PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKER_H

#include "libpandabase/taskmanager/task.h"
#include "runtime/mem/gc/gc_queue.h"

namespace panda {
// Forward declaration for GCWorker
class Thread;
}  // namespace panda

namespace panda::mem {
// Forward declaration for GCWorker
class GC;

/**
 * @class GCWorker represents GC-worker which runs GC tasks on a non-managed thread
 * @brief The class implements interaction between GC and GC-worker. GC-worker can be separate internal GC-thread or
 * a thread from TaskManager
 */
class GCWorker {
public:
    /**
     * @brief Create local gc tasks queue and structure for gc panda thread.
     * Don't create gc thread if it needed
     * @see panda::Thread
     * @see CreateThreadIfNeeded
     * @see DestroyThreadIfNeeded
     *
     * @param gc poiner to used GC
     */
    explicit GCWorker(GC *gc);
    NO_COPY_SEMANTIC(GCWorker);
    NO_MOVE_SEMANTIC(GCWorker);
    ~GCWorker();

    /// @brief Create and start internal gc thread for tasks running if common task manager is not used for GC
    void CreateThreadIfNeeded();
    /// @brief Join and destroy internal gc thread for tasks running if it was created
    void DestroyThreadIfNeeded();

    /**
     * @brief Add new gc task to GC worker. Try to add the task to local queue and then run it on a worker
     * @param task gc task for running on a worker
     * @return true if task was added to local task queue, false - otherwise
     */
    bool AddTask(PandaUniquePtr<GCTask> task);

private:
    static constexpr taskmanager::TaskProperties GC_WORKER_TASK_PROPERTIES = {
        taskmanager::TaskType::GC, taskmanager::VMType::STATIC_VM, taskmanager::TaskExecutionMode::BACKGROUND};

    PandaUniquePtr<GCTask> GetTask();
    void RunGC(PandaUniquePtr<GCTask> task);

    static void GCThreadLoop(GCWorker *gc_worker);

    GC *gc_ {nullptr};
    GCQueueInterface *gc_task_queue_ {nullptr};
    Thread *gc_thread_ {nullptr};
    std::thread *gc_internal_thread_ {nullptr};
    uint32_t collect_number_mod_ {1U};
    os::memory::Mutex gc_task_run_mutex_;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKER_H
