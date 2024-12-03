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
#include "runtime/mem/gc/gc_adaptive_stack_inl.h"

namespace ark::mem {

template <typename LanguageConfig>
G1EvacuateRegionsWorkerState<LanguageConfig>::G1EvacuateRegionsWorkerState(G1GC<LanguageConfig> *gc,
                                                                           GCEvacuateRegionsTaskStack<Ref> *refStack)
    : gc_(gc),
      objectAllocator_(gc_->GetG1ObjectAllocator()),
      cardTable_(gc->GetCardTable()),
      regionSizeBits_(gc_->regionSizeBits_),
      refStack_(refStack),
      evacuationObjectPointerHandler_(this)
{
    regionTo_ = GetNextRegion();
}

template <typename LanguageConfig>
G1EvacuateRegionsWorkerState<LanguageConfig>::~G1EvacuateRegionsWorkerState()
{
    os::memory::LockHolder lock(gc_->gcWorkerQueueLock_);

    gc_->memStats_.template RecordCountMovedYoung<false>(GetCopiedObjectsYoung());
    gc_->memStats_.template RecordCountMovedTenured<false>(GetCopiedObjectsOld());
    gc_->copiedBytesYoung_ += GetCopiedBytesYoung();
    gc_->copiedBytesOld_ += GetCopiedBytesOld();

    VisitCards([this](auto *card) {
        if (!card->IsMarked()) {
            card->Mark();
            gc_->updatedRefsQueueTemp_->push_back(card);
        }
    });
    GetRegionTo()->SetLiveBytes(GetRegionTo()->GetLiveBytes() + GetLiveBytes());
    auto *objectAllocator = gc_->GetG1ObjectAllocator();
    objectAllocator->template PushToOldRegionQueue<true>(GetRegionTo());
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
    if (markWord.IsForwarded()) {
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
void G1EvacuateRegionsWorkerState<LanguageConfig>::EvacuateNonHeapRoots()
{
    GCRootVisitor gcMarkCollectionSet = [this](const GCRoot &gcRoot) {
        ASSERT(gcRoot.GetFromObjectHeader() == nullptr);
        auto *rootObject = gcRoot.GetObjectHeader();
        ASSERT(rootObject != nullptr);
        LOG(DEBUG, GC) << "Handle root " << GetDebugInfoAboutObject(rootObject) << " from: " << gcRoot.GetType();
        if (!gc_->InGCSweepRange(rootObject)) {
            // Skip non-collection-set roots
            return;
        }
        MarkWord markWord = rootObject->AtomicGetMark(std::memory_order_relaxed);
        if (markWord.IsForwarded()) {
            return;
        }
        LOG(DEBUG, GC) << "root " << GetDebugInfoAboutObject(rootObject);
        Evacuate(rootObject, markWord);
    };

    gc_->VisitRoots(gcMarkCollectionSet, VisitGCRootFlags::ACCESS_ROOT_NONE);
}

template <typename LanguageConfig>
size_t G1EvacuateRegionsWorkerState<LanguageConfig>::ScanRemset(const RemSet<> &remset)
{
    size_t remsetSize = 0;
    auto remsetVisitor = [this, &remsetSize](Region *region, const MemRange &range) {
        IterateRefsInMemRange(range, region);
        remsetSize++;
    };
    remset.Iterate([](Region *r) { return !r->HasFlag(IS_COLLECTION_SET); }, remsetVisitor);
    return remsetSize;
}

template <class LanguageConfig>
void G1EvacuateRegionsWorkerState<LanguageConfig>::IterateRefsInMemRange(const MemRange &memRange, Region *region)
{
    // Use MarkBitmap instead of LiveBitmap because it concurrently updated during evacuation
    MarkBitmap *bitmap = region->GetMarkBitmap();
    auto *startAddress = ToVoidPtr(memRange.GetStartAddress());
    auto *endAddress = ToVoidPtr(memRange.GetEndAddress());
    auto wrapper = [this, startAddress, endAddress](void *mem) {
        ObjectIterator<LanguageConfig::LANG_TYPE>::template IterateAndDiscoverReferences(
            GetGC(), static_cast<ObjectHeader *>(mem), &evacuationObjectPointerHandler_, startAddress, endAddress);
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
    refStack_->TraverseObjects(*this);
}
}  // namespace ark::mem
#endif
