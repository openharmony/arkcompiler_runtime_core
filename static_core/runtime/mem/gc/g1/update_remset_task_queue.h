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

#ifndef PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_TASK_QUEUE_H
#define PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_TASK_QUEUE_H

#include "runtime/mem/gc/g1/update_remset_worker.h"

#include "libpandabase/taskmanager/task.h"

namespace panda::mem {

template <class LanguageConfig>
class UpdateRemsetTaskQueue final : public UpdateRemsetWorker<LanguageConfig> {
public:
    UpdateRemsetTaskQueue(G1GC<LanguageConfig> *gc, GCG1BarrierSet::ThreadLocalCardQueues *queue,
                          os::memory::Mutex *queue_lock, size_t region_size, bool update_concurrent,
                          size_t min_concurrent_cards_to_process);
    NO_COPY_SEMANTIC(UpdateRemsetTaskQueue);
    NO_MOVE_SEMANTIC(UpdateRemsetTaskQueue);
    ~UpdateRemsetTaskQueue() final;

private:
    void CreateWorkerImpl() final;

    void DestroyWorkerImpl() final;

    void ContinueProcessCards() REQUIRES(this->update_remset_lock_) final;

    /// @brief Add a new process cards task in task manager if such task does not exist, do nothing otherwise
    void StartProcessCards() REQUIRES(this->update_remset_lock_);

    /* TaskManager specific variables */
    static constexpr taskmanager::TaskProperties UPDATE_REMSET_TASK_PROPERTIES = {
        taskmanager::TaskType::GC, taskmanager::VMType::STATIC_VM, taskmanager::TaskExecutionMode::FOREGROUND};

    bool has_task_in_taskmanager_ GUARDED_BY(this->update_remset_lock_) {false};
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_TASK_QUEUE_H
