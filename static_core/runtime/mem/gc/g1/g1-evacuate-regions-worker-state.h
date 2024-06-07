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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_WORKER_STATE_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_WORKER_STATE_H

#include "runtime/mem/lock_config_helper.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/mark_word.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/g1/g1-object-pointer-handlers.h"

namespace ark::mem {
template <typename LanguageConfig>
class G1GC;

template <MTModeT MODE>
class ObjectAllocatorG1;

class CardTable;

class SharedState;

/// Data and methods to evacuate live objects by one worker.
template <typename LanguageConfig>
class G1EvacuateRegionsWorkerState {
public:
    using Ref = typename ObjectReference<LanguageConfig::LANG_TYPE>::Type;

    G1EvacuateRegionsWorkerState(G1GC<LanguageConfig> *gc, uint id, PandaVector<Ref> *queue, SharedState *shared);

    uint GetId() const
    {
        return id_;
    };

    void ProcessQueue()
    {
        while (!queue_->empty()) {
            Ref p = queue_->back();
            queue_->pop_back();
            ProcessRef(p);
        }
    }

    void PushToQueue(Ref p)
    {
        queue_->push_back(p);
    }

    /// Evacuate single live object, traverse its fields and enqueue found references to the queue_
    ObjectHeader *Evacuate(ObjectHeader *obj, MarkWord markWord);

    G1GC<LanguageConfig> *GetGC()
    {
        return gc_;
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

    size_t GetLiveBytes() const
    {
        return liveBytes_;
    }

    Region *GetRegionTo()
    {
        return regionTo_;
    }

    SharedState *GetShared()
    {
        return shared_;
    }

    bool IsSameRegion(void *ref, void *obj) const
    {
        return ark::mem::IsSameRegion(ref, obj, regionSizeBits_);
    }

    void EnqueueCard(void *p)
    {
        auto *card = cardTable_->GetCardPtr(ToUintPtr(p));
        if (card != latestCard_) {
            cardQueue_.insert(card);
            latestCard_ = card;
        }
    }

    template <typename Visitor>
    void VisitCards(Visitor visitor)
    {
        for (auto *card : cardQueue_) {
            visitor(card);
        }
    }

    uint64_t GetTaskStart() const
    {
        return taskStart_;
    }

    uint64_t GetTaskEnd() const
    {
        return taskEnd_;
    }

    void DumpStat() const
    {
        LOG(INFO, GC) << "=== Worker #" << GetId();
        LOG(INFO, GC) << "non heap roots scan duration " << ark::helpers::TimeConverter(scanNonHeapRootsDuration_);
        LOG(INFO, GC) << "added refs from non heap roots " << addNonHeapRootsRefsCount_;
        LOG(INFO, GC) << "remset scan duration " << ark::helpers::TimeConverter(scanRemsetDuration_);
        LOG(INFO, GC) << "remset count " << remsetCount_;
        LOG(INFO, GC) << "added refs from remset " << foundRemsetRefsCount_;
        LOG(INFO, GC) << "reenque cards " << reenqueueCardsCount_;
        LOG(INFO, GC) << "evacuation duration " << ark::helpers::TimeConverter(evacuationDuration_);
        LOG(INFO, GC) << "evacuation add cards " << evacuationAddCardsCount_;
    }

    void Work()
    {
        StartWork();

        Init();
        EvacuateNonHeapRoots();
        ScanRemset();
        EvacuateLiveObjects();

        EndWork();
    }

private:
    void StartWork();
    void Init()
    {
        constexpr size_t RESERVED_SIZE = 200000;
        queue_->reserve(RESERVED_SIZE);
        regionTo_ = GetNextRegion();
    }

    void EndWork();
    void EvacuateNonHeapRoots();
    void ScanRemset();
    template <typename Visitor>
    void IterateRefsInMemRange(const MemRange &memRange, Region *region, Visitor *visitor);
    void EvacuateLiveObjects();

    Region *GetNextRegion()
    {
        auto *region = objectAllocator_->template PopFromOldRegionQueue<true>();
        if (region != nullptr) {
            return region;
        }
        return CreateNewRegion();
    }

    Region *CreateNewRegion()
    {
        return objectAllocator_->template CreateAndSetUpNewOldRegion<true>();
    }

    void ProcessRef(Ref p);

    ObjectHeader *SetForwardAddress(ObjectHeader *src, ObjectHeader *dst, MarkWord markWord);

    G1GC<LanguageConfig> *gc_;
    ObjectAllocatorG1<LanguageConfig::MT_MODE> *objectAllocator_;
    CardTable *cardTable_;
    size_t regionSizeBits_;
    uint id_;
    SharedState *shared_;
    PandaVector<Ref> *queue_;
    EvacuationObjectPointerHandler<LanguageConfig> evacuationObjectPointerHandler_;

    PandaSet<CardTable::CardPtr> cardQueue_;
    CardTable::CardPtr latestCard_ {nullptr};

    uint64_t taskStart_ {0};
    uint64_t taskEnd_ {0};

    size_t copiedBytesYoung_ {0};
    size_t copiedObjectsYoung_ {0};
    size_t copiedBytesOld_ {0};
    size_t copiedObjectsOld_ {0};
    size_t liveBytes_ {0};
    Region *regionTo_ {nullptr};

    uint64_t scanNonHeapRootsDuration_ {0};
    uint64_t addNonHeapRootsRefsCount_ {0};

    uint64_t scanRemsetDuration_ {0};
    uint64_t foundRemsetRefsCount_ {0};
    uint64_t remsetCount_ {0};

    uint64_t evacuationDuration_ {0};
    uint64_t reenqueueCardsCount_ {0};
    uint64_t evacuationAddCardsCount_ {0};
};

/// Common data which is shared between workers.
class SharedState {
public:
    SharedState(Thread *gcThread, size_t workersCount, RemSet<> *remset)
        : gcThread_(gcThread), workersCount_(workersCount), remset_(remset)
    {
    }

    Thread *GetGcThread() const
    {
        return gcThread_;
    }

    size_t GetWorkersCount() const
    {
        return workersCount_;
    }

    RemSet<> *GetRemset() const
    {
        return remset_;
    }

    /// Counter which can be used to distribute remset processing between workers
    std::atomic<size_t> &GetRemsetCounter()
    {
        return remsetCounter_;
    }

private:
    Thread *gcThread_;
    size_t workersCount_;
    std::atomic<size_t> remsetCounter_ {};
    RemSet<> *remset_;
};

}  // namespace ark::mem
#endif
