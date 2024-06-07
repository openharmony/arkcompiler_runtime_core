/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_TASK_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_TASK_H

#include "runtime/mem/gc/workers/gc_workers_tasks.h"
#include "runtime/mem/gc/g1/g1-evacuate-regions-worker-state-inl.h"

namespace ark::mem {

class GcThreadScope {
public:
    explicit GcThreadScope(Thread *gcThread) : previous_(Thread::GetCurrent())
    {
        Thread::SetCurrent(gcThread);
    }

    ~GcThreadScope()
    {
        Thread::SetCurrent(previous_);
    }

    DEFAULT_COPY_SEMANTIC(GcThreadScope);
    DEFAULT_MOVE_SEMANTIC(GcThreadScope);

private:
    Thread *previous_;
};

template <typename LanguageConfig>
class G1EvacuateRegionsTask : public GCWorkersTask {
public:
    explicit G1EvacuateRegionsTask(G1EvacuateRegionsWorkerState<LanguageConfig> *state)
        : GCWorkersTask(GCWorkersTaskTypes::TASK_EVACUATE_REGIONS, state)
    {
    }

    DEFAULT_COPY_SEMANTIC(G1EvacuateRegionsTask);
    DEFAULT_MOVE_SEMANTIC(G1EvacuateRegionsTask);
    ~G1EvacuateRegionsTask() = default;

    void Work()
    {
        GcThreadScope gcThreadScope(GetState()->GetShared()->GetGcThread());
        GetState()->Work();
    }

private:
    G1EvacuateRegionsWorkerState<LanguageConfig> *GetState() const
    {
        return static_cast<G1EvacuateRegionsWorkerState<LanguageConfig> *>(storage_);
    }
};

template <class LanguageConfig>
class ParallelCompactionTask {
public:
    ParallelCompactionTask(G1GC<LanguageConfig> *gc, ObjectAllocatorG1<LanguageConfig::MT_MODE> *objectAllocator,
                           RemSet<> *remset, const CollectionSet &collectionSet)
        : gc_(gc),
          internalAllocator_(gc->GetInternalAllocator()),
          objectAllocator_(objectAllocator),
          shared_(Thread::GetCurrent(), gc->GetSettings()->GCWorkersCount(), remset),
          queues_(shared_.GetWorkersCount())
    {
        for (auto *region : collectionSet) {
            auto allocated = region->GetAllocatedBytes();
            if (region->HasFlag(RegionFlag::IS_EDEN)) {
                allocatedBytesYoung_ += allocated;
            } else {
                allocatedBytesOld_ += allocated;
            }
        }
    }

    NO_COPY_SEMANTIC(ParallelCompactionTask);
    NO_MOVE_SEMANTIC(ParallelCompactionTask);

    ~ParallelCompactionTask()
    {
        for (auto *state : workerStates_) {
            internalAllocator_->Delete(state);
        }
    }

    void Run()
    {
        Submit();
        gc_->GetWorkersTaskPool()->WaitUntilTasksEnd();
        LOG_IF(gc_->GetSettings()->LogDetailedGCInfoEnabled(), INFO, GC)
            << "Waited all workers for "
            << ark::helpers::TimeConverter(ark::time::GetCurrentTimeInNanos() - beforeSubmitTime_);
    }

    void FlushDirtyCards(GCG1BarrierSet::ThreadLocalCardQueues *queue)
    {
        for (auto *state : workerStates_) {
            state->VisitCards([queue](auto *card) {
                if (!card->IsMarked()) {
                    card->Mark();
                    queue->push_back(card);
                }
            });
        }
    }

    void Finish()
    {
        for (auto *state : workerStates_) {
            state->GetRegionTo()->SetLiveBytes(state->GetRegionTo()->GetLiveBytes() + state->GetLiveBytes());
            objectAllocator_->template PushToOldRegionQueue<true>(state->GetRegionTo());

            copiedBytesYoung_ += state->GetCopiedBytesYoung();
            copiedObjectsYoung_ += state->GetCopiedObjectsYoung();

            copiedBytesOld_ += state->GetCopiedBytesOld();
            copiedObjectsOld_ += state->GetCopiedObjectsOld();
        }

        if (gc_->GetSettings()->LogDetailedGCInfoEnabled()) {
            DumpStats();
        }
    }

    size_t GetCopiedBytesYoung() const
    {
        return copiedBytesYoung_;
    }

    size_t GetCopiedObjectsYoung() const
    {
        return copiedObjectsYoung_;
    }

    size_t GetCopiedBytesOld() const
    {
        return copiedBytesOld_;
    }

    size_t GetCopiedObjectsOld() const
    {
        return copiedObjectsOld_;
    }

    size_t GetFreedBytesYoung() const
    {
        return allocatedBytesYoung_ - copiedBytesYoung_;
    }

    size_t GetFreedBytesOld() const
    {
        return allocatedBytesOld_ - copiedBytesOld_;
    }

private:
    using TaskQueue = PandaVector<typename ObjectReference<LanguageConfig::LANG_TYPE>::Type>;

    void Submit()
    {
        beforeSubmitTime_ = ark::time::GetCurrentTimeInNanos();
        auto workersCount = shared_.GetWorkersCount();
        for (size_t i = 0; i < workersCount; i++) {
            auto *state = internalAllocator_->template New<G1EvacuateRegionsWorkerState<LanguageConfig>>(
                gc_, i, &queues_[i], &shared_);
            bool added = gc_->GetWorkersTaskPool()->AddTask(G1EvacuateRegionsTask(state));
            if (!added) {
                LOG(INFO, GC) << "Not enough workers to add task #" << i;
                state->Work();
            }
            workerStates_.push_back(state);
        }
    }

    void DumpStats() const
    {
        for (auto *state : workerStates_) {
            LOG(INFO, GC) << "Worker #" << state->GetId() << " started +"
                          << ark::helpers::TimeConverter(state->GetTaskStart() - beforeSubmitTime_);
            LOG(INFO, GC) << "Worker #" << state->GetId() << " finished +"
                          << ark::helpers::TimeConverter(state->GetTaskEnd() - beforeSubmitTime_);
            LOG(INFO, GC) << "Worker #" << state->GetId() << " duration "
                          << ark::helpers::TimeConverter(state->GetTaskEnd() - state->GetTaskStart());
            state->DumpStat();
        }
    }

    G1GC<LanguageConfig> *gc_;
    InternalAllocatorPtr internalAllocator_;
    ObjectAllocatorG1<LanguageConfig::MT_MODE> *objectAllocator_;
    PandaVector<G1EvacuateRegionsWorkerState<LanguageConfig> *> workerStates_;
    SharedState shared_;
    PandaVector<TaskQueue> queues_;

    uint64_t beforeSubmitTime_ {0};
    size_t allocatedBytesYoung_ {0};
    size_t allocatedBytesOld_ {0};

    size_t copiedBytesYoung_ {0};
    size_t copiedObjectsYoung_ {0};
    size_t copiedBytesOld_ {0};
    size_t copiedObjectsOld_ {0};
};

}  // namespace ark::mem
#endif
