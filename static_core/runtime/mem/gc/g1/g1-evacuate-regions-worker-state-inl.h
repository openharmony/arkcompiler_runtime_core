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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_WORKER_STATE_INL_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_WORKER_STATE_INL_H

#include "runtime/mem/gc/g1/g1-evacuate-regions-worker-state.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/region_space.h"
#include "libpandabase/utils/type_converter.h"
#include "runtime/mem/object-references-iterator-inl.h"

namespace ark::mem {

template <typename LanguageConfig>
G1EvacuateRegionsWorkerState<LanguageConfig>::G1EvacuateRegionsWorkerState(G1GC<LanguageConfig> *gc, uint id,
                                                                           PandaVector<Ref> *queue, SharedState *shared)
    : gc_(gc),
      objectAllocator_(gc_->GetG1ObjectAllocator()),
      cardTable_(gc->GetCardTable()),
      regionSizeBits_(gc_->regionSizeBits_),
      id_(id),
      shared_(shared),
      queue_(queue),
      evacuationObjectPointerHandler_(this)
{
}

template <typename LanguageConfig>
ObjectHeader *G1EvacuateRegionsWorkerState<LanguageConfig>::Evacuate(ObjectHeader *obj, MarkWord markWord)
{
    auto objectSize = GetObjectSize(obj);
    auto alignedSize = AlignUp(objectSize, DEFAULT_ALIGNMENT_IN_BYTES);

    ASSERT(regionTo_ != nullptr);

    void *dst = regionTo_->template Alloc<false>(alignedSize);
    if (dst == nullptr) {
        regionTo_->SetLiveBytes(regionTo_->GetLiveBytes() + liveBytes_);
        liveBytes_ = 0;
        regionTo_ = CreateNewRegion();
        dst = regionTo_->template Alloc<false>(alignedSize);
    }
    // Don't initialize memory for an object here because we will use memcpy anyway
    ASSERT(dst != nullptr);
    [[maybe_unused]] auto copyResult = memcpy_s(dst, objectSize, obj, objectSize);
    ASSERT(copyResult == 0);
    // need to mark as alive moved object
    ASSERT(regionTo_->GetLiveBitmap() != nullptr);

    auto *newObj = static_cast<ObjectHeader *>(dst);
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    // Since we forward with relaxed memory order there are no ordering with above CopyObject and other threads should
    // not examine forwardee content without synchronization
    auto *forwardAddr = SetForwardAddress(obj, newObj, markWord);
    TSAN_ANNOTATE_IGNORE_WRITES_END();

    if (forwardAddr == nullptr) {
        ObjectIterator<LanguageConfig::LANG_TYPE>::template IterateAndDiscoverReferences(
            GetGC(), newObj, &evacuationObjectPointerHandler_);

        if (ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN)) {
            copiedBytesYoung_ += alignedSize;
            copiedObjectsYoung_++;
        } else {
            copiedBytesOld_ += alignedSize;
            copiedObjectsOld_++;
        }

        regionTo_->IncreaseAllocatedObjects();
        regionTo_->GetLiveBitmap()->Set(dst);
        liveBytes_ += alignedSize;

        return newObj;
    }

    regionTo_->UndoAlloc(dst);
    return forwardAddr;
}

template <typename LanguageConfig>
void G1EvacuateRegionsWorkerState<LanguageConfig>::ProcessRef(Ref p)
{
    auto o = ObjectAccessor::Load(p);
    auto *obj = ObjectAccessor::DecodeNotNull(o);
    if (!gc_->InGCSweepRange(obj)) {
        // reference has been already processed
        return;
    }

    // Atomic with relaxed order reason: memory order is not required
    MarkWord markWord = obj->AtomicGetMark(std::memory_order_relaxed);
    if (markWord.GetState() == MarkWord::ObjectState::STATE_GC) {
        obj = reinterpret_cast<ObjectHeader *>(markWord.GetForwardingAddress());
    } else {
        obj = Evacuate(obj, markWord);
    }

    ObjectAccessor::Store(p, obj);

    if (!IsSameRegion(p, obj)) {
        EnqueueCard(p);
    }
}

template <typename LanguageConfig>
ObjectHeader *G1EvacuateRegionsWorkerState<LanguageConfig>::SetForwardAddress(ObjectHeader *src, ObjectHeader *dst,
                                                                              MarkWord markWord)
{
    MarkWord fwdMarkWord = markWord.DecodeFromForwardingAddress(static_cast<MarkWord::MarkWordSize>(ToUintPtr(dst)));
    // Atomic with relaxed order reason: memory order is not required
    auto success = src->AtomicSetMark<true>(markWord, fwdMarkWord, std::memory_order_relaxed);
    if (success) {
        if constexpr (LanguageConfig::LANG_TYPE == LANG_TYPE_DYNAMIC) {
            auto *baseCls = src->ClassAddr<BaseClass>();
            if (baseCls->IsDynamicClass() && static_cast<HClass *>(baseCls)->IsHClass()) {
                // Note: During moving phase, 'src => dst'. Consider the src is a DynClass,
                //       since 'dst' is not in GC-status the 'manage-object' inside 'dst' won't be updated to
                //       'dst'. To fix it, we update 'manage-object' here rather than upating phase.
                size_t offset = ObjectHeader::ObjectHeaderSize() + HClass::GetManagedObjectOffset();
                dst->SetFieldObject<false, false, true>(gc_->GetPandaVm()->GetAssociatedThread(), offset, dst);
            }
        }

        return nullptr;
    }

    return reinterpret_cast<ObjectHeader *>(markWord.GetForwardingAddress());
}

template <typename LanguageConfig>
void G1EvacuateRegionsWorkerState<LanguageConfig>::StartWork()
{
    taskStart_ = ark::time::GetCurrentTimeInNanos();
}

template <typename LanguageConfig>
void G1EvacuateRegionsWorkerState<LanguageConfig>::EndWork()
{
    taskEnd_ = ark::time::GetCurrentTimeInNanos();
}

template <typename LanguageConfig>
void G1EvacuateRegionsWorkerState<LanguageConfig>::EvacuateNonHeapRoots()
{
    time::Timer timer(&scanNonHeapRootsDuration_);
    if (GetShared()->GetWorkersCount() == 1 || GetId() == 0) {
        // Only first worker processes roots, currently it is not parallized
        GCRootVisitor gcMarkCollectionSet = [this](const GCRoot &gcRoot) {
            ASSERT(gcRoot.GetFromObjectHeader() == nullptr);
            auto *rootObject = gcRoot.GetObjectHeader();
            ASSERT(rootObject != nullptr);
            LOG(DEBUG, GC) << "Handle root " << GetDebugInfoAboutObject(rootObject) << " from: " << gcRoot.GetType();
            if (!GetGC()->InGCSweepRange(rootObject)) {
                // Skip non-collection-set roots
                return;
            }
            MarkWord markWord = rootObject->AtomicGetMark(std::memory_order_relaxed);
            if (markWord.GetState() == MarkWord::ObjectState::STATE_GC) {
                // already evacuated by concurrent worker
                return;
            }
            LOG(DEBUG, GC) << "root " << GetDebugInfoAboutObject(rootObject);
            Evacuate(rootObject, markWord);
        };
        GetGC()->VisitRoots(gcMarkCollectionSet, VisitGCRootFlags::ACCESS_ROOT_NONE);
    }
}

class ConcurrentRemSetDistributor {
public:
    explicit ConcurrentRemSetDistributor(std::atomic_size_t &counter) : counter_(counter) {}

    bool Distribute()
    {
        if (claimed_) {
            // Atomic with seq_cst order reason: full order is required to synchronize workers
            indexToProcess_ = counter_.fetch_add(1, std::memory_order_seq_cst);
        }

        claimed_ = currentIndex_ == indexToProcess_;
        currentIndex_++;
        return claimed_;
    }

private:
    size_t currentIndex_ {0};
    size_t indexToProcess_ {0};
    bool claimed_ {true};
    std::atomic_size_t &counter_;
};

class EvenRemSetDistributor {
public:
    EvenRemSetDistributor(uint64_t id, size_t totalCount) : id_(id), totalCount_(totalCount) {}

    bool Distribute()
    {
        auto oldvalue = currentIndex_++;
        return oldvalue % totalCount_ == id_;
    }

private:
    const uint64_t id_;
    const size_t totalCount_;
    size_t currentIndex_ {0};
};

template <typename LanguageConfig>
void G1EvacuateRegionsWorkerState<LanguageConfig>::ScanRemset()
{
    time::Timer timer(&scanRemsetDuration_);
    size_t total = 0;
    EvenRemSetDistributor distributor(GetId(), GetShared()->GetWorkersCount());
    auto remsetVisitor = [this, &distributor, &total](Region *r, const MemRange &range) {
        if (distributor.Distribute()) {
            IterateRefsInMemRange(range, r, &evacuationObjectPointerHandler_);
            total++;
        }
    };

    GetShared()->GetRemset()->Iterate([](Region *r) { return !r->HasFlag(IS_COLLECTION_SET); }, remsetVisitor);
    remsetCount_ = total;

    reenqueueCardsCount_ = cardQueue_.size();
}

template <class LanguageConfig>
template <typename Visitor>
void G1EvacuateRegionsWorkerState<LanguageConfig>::IterateRefsInMemRange(const MemRange &memRange, Region *region,
                                                                         Visitor *visitor)
{
    // Use MarkBitmap instead of LiveBitmap because it concurrently updated during evacuation
    MarkBitmap *bitmap = region->GetMarkBitmap();
    auto *startAddress = ToVoidPtr(memRange.GetStartAddress());
    auto *endAddress = ToVoidPtr(memRange.GetEndAddress());
    auto wrapper = [this, visitor, startAddress, endAddress](void *mem) {
        ObjectIterator<LanguageConfig::LANG_TYPE>::template IterateAndDiscoverReferences(
            GetGC(), static_cast<ObjectHeader *>(mem), visitor, startAddress, endAddress);
    };
    if (region->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
        bitmap->CallForMarkedChunkInHumongousRegion<false>(ToVoidPtr(region->Begin()), wrapper);
    } else {
        bitmap->IterateOverMarkedChunkInRange(startAddress, endAddress, wrapper);
    }
}

template <typename LanguageConfig>
void G1EvacuateRegionsWorkerState<LanguageConfig>::EvacuateLiveObjects()
{
    time::Timer timer(&evacuationDuration_);
    ProcessQueue();

    evacuationAddCardsCount_ = cardQueue_.size() - reenqueueCardsCount_;
}
}  // namespace ark::mem
#endif
