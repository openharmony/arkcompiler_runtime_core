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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_SPACE_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_SPACE_H

#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "runtime/mem/gc/cmc/heap/allocator/alloc_util.h"
#include "runtime/mem/gc/cmc/heap/allocator/allocator.h"
#include "runtime/mem/gc/cmc/heap/allocator/region_manager.h"
#include "runtime/mem/gc/cmc/heap/space/young_space.h"
#include "runtime/mem/gc/cmc/heap/space/old_space.h"
#include "runtime/mem/gc/cmc/heap/space/from_space.h"
#include "runtime/mem/gc/cmc/heap/space/to_space.h"
#include "runtime/mem/gc/cmc/heap/space/nonmovable_space.h"
#include "common_interfaces/thread/mutator.h"
#include "runtime/mem/gc/cmc/heap/space/large_space.h"
#include "runtime/include/mem/panda_containers.h"
#include "common_interfaces/objects/base_object.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace ark::common_vm {
class Taskpool;

// RegionalHeap aims to be the API for other components of runtime
// the complication of implementation is delegated to RegionManager
// allocator should not depend on any assumptions on the details of RegionManager

class RegionalHeap : public Allocator {
public:
    static size_t ToAllocatedSize(size_t objSize)
    {
        size_t size = objSize + HEADER_SIZE;
        return RoundUp<size_t>(size, ALLOC_ALIGN);
    }

    static size_t GetAllocSize(const BaseObject &obj)
    {
        size_t objSize = obj.GetSize();
        return ToAllocatedSize(objSize);
    }

    RegionalHeap()
        : youngSpace_(regionManager_),
          oldSpace_(regionManager_),
          fromSpace_(regionManager_, *this),
          toSpace_(regionManager_),
          nonMovableSpace_(regionManager_),
          largeSpace_(regionManager_)
    {
    }

    NO_INLINE virtual ~RegionalHeap()
    {
        if (allocBufferManager_ != nullptr) {
            delete allocBufferManager_;
            allocBufferManager_ = nullptr;
        }
#if defined(COMMON_SANITIZER_SUPPORT)
        Sanitizer::OnHeapDeallocated(map->GetBaseAddr(), map->GetMappedSize());
#endif
        MemoryMap::DestroyMemoryMap(map_);
    }

    void Init(const RuntimeParam &param) override;

    template <AllocBufferType type>
    RegionDesc *AllocateThreadLocalRegion(bool expectPhysicalMem = false);

    template <AllocBufferType type = AllocBufferType::YOUNG>
    void HandleFullThreadLocalRegion(RegionDesc *region) noexcept
    {
        if (region == RegionDesc::NullRegion()) {
            return;
        }
        ASSERT_PRINT(region->IsThreadLocalRegion() || region->IsToRegion() || region->IsOldRegion(),
                     "unexpected region type");

        if constexpr (type == AllocBufferType::YOUNG) {
            ASSERT_PRINT(!IsGcThread(), "unexpected gc thread for young space");
            youngSpace_.HandleFullThreadLocalRegion(region);
        } else if constexpr (type == AllocBufferType::OLD) {
            ASSERT_PRINT(!IsGcThread(), "unexpected gc thread for old space");
            oldSpace_.HandleFullThreadLocalRegion(region);
        } else if constexpr (type == AllocBufferType::TO) {
            toSpace_.HandleFullThreadLocalRegion(region);
        }
    }

    // only used for deserialize allocation, allocate one region and regard it as full region
    uintptr_t AllocOldRegion();
    uintptr_t AllocateNonMovableRegion();
    uintptr_t AllocLargeRegion(size_t size);
    uintptr_t AllocJitFortRegion(size_t size);
    mem::TLAB *CreateTLAB();

    HeapAddress Allocate(size_t size, AllocType allocType) override;

    HeapAddress AllocateNoGC(size_t size, AllocType allocType) override;

    RegionManager &GetRegionManager() noexcept
    {
        return regionManager_;
    }

    FromSpace &GetFromSpace() noexcept
    {
        return fromSpace_;
    }

    ToSpace &GetToSpace() noexcept
    {
        return toSpace_;
    }

    OldSpace &GetOldSpace() noexcept
    {
        return oldSpace_;
    }

    YoungSpace &GetYoungSpace() noexcept
    {
        return youngSpace_;
    }

    HeapAddress GetSpaceStartAddress() const override
    {
        return reservedStart_;
    }

    HeapAddress GetSpaceEndAddress() const override
    {
        return reservedEnd_;
    }

    size_t GetCurrentCapacity() const override
    {
        return regionManager_.GetInactiveZone() - reservedStart_;
    }
    size_t GetMaxCapacity() const override
    {
        return reservedEnd_ - reservedStart_;
    }

    inline size_t GetRecentAllocatedSize() const
    {
        return youngSpace_.GetRecentAllocatedSize() + nonMovableSpace_.GetRecentAllocatedSize() +
               largeSpace_.GetRecentAllocatedSize();
    }

    // size of objects survived in previous gc.
    size_t GetSurvivedSize() const override
    {
        return fromSpace_.GetSurvivedSize() + toSpace_.GetAllocatedSize() + youngSpace_.GetAllocatedSize() +
               oldSpace_.GetAllocatedSize() + nonMovableSpace_.GetSurvivedSize() + largeSpace_.GetAllocatedSize();
    }

    inline size_t GetUsedUnitCount() const
    {
        return fromSpace_.GetUsedUnitCount() + toSpace_.GetUsedUnitCount() + youngSpace_.GetUsedUnitCount() +
               oldSpace_.GetUsedUnitCount() + nonMovableSpace_.GetUsedUnitCount() + largeSpace_.GetUsedUnitCount();
    }

    size_t GetUsedPageSize() const override
    {
        return GetUsedUnitCount() * RegionDesc::UNIT_SIZE;
    }

    size_t GetAllocatedBytes() const override
    {
        return fromSpace_.GetAllocatedSize() + toSpace_.GetAllocatedSize() + youngSpace_.GetAllocatedSize() +
               oldSpace_.GetAllocatedSize() + nonMovableSpace_.GetAllocatedSize() + LargeObjectSize();
    }

    size_t LargeObjectSize() const override
    {
        return largeSpace_.GetAllocatedSize();
    }

    size_t GetFootprintBytes() const override;

    // OnAllocate is called for all allocations going through Allocate() / AllocateNoGC(),
    // including readonly, large, nonmovable and movable (young/old) objects.
    // It's not called for TLAB allocations (which bypass these methods).
    // OnAllocate records each non-TL allocation in MemStats.
    void OnAllocate(size_t bytes, AllocType allocType);

    // note: it doesn't contain exemptFromRegion
    size_t FromRegionSize() const
    {
        return fromSpace_.GetFromRegionAllocatedSize();
    }
    size_t ToSpaceSize() const
    {
        return toSpace_.GetAllocatedSize();
    }

#ifndef NDEBUG
    bool IsHeapObject(HeapAddress addr) const override;
#endif

    size_t ReclaimGarbageMemory(bool releaseAll) override;

    bool ForEachObject(const std::function<void(BaseObject *)> &visitor, bool safe) const override
    {
        if (UNLIKELY(safe)) {
            regionManager_.ForEachObjectSafe(visitor);
        } else {
            regionManager_.ForEachObjectUnsafe(visitor);
        }
        return true;
    }

    void ExemptFromSpace();

    void CopyFromSpace(Taskpool *threadPool);

    FixHeapTaskList CollectFixTasks()
    {
        FixHeapTaskList taskList;
        youngSpace_.CollectFixTasks(taskList);
        oldSpace_.CollectFixTasks(taskList);
        fromSpace_.CollectFixTasks(taskList);
        toSpace_.CollectFixTasks(taskList);
        nonMovableSpace_.CollectFixTasks(taskList);
        largeSpace_.CollectFixTasks(taskList);

        return taskList;
    }

    size_t CollectLargeGarbage()
    {
        return largeSpace_.CollectLargeGarbage();
    }

    void CollectFromSpaceGarbage()
    {
        regionManager_.CollectFromSpaceGarbage(fromSpace_.GetFromRegionList());
    }

    void HandlePromotion()
    {
        fromSpace_.GetPromotedTo(oldSpace_);
        toSpace_.GetPromotedTo(oldSpace_);
    }

    void AssembleSmallGarbageCandidates()
    {
        youngSpace_.AssembleGarbageCandidates(fromSpace_);
        oldSpace_.AssembleRecentFull();
        if (Heap::GetHeap().GetGCReason() != GCTaskCause::YOUNG_GC_CAUSE) {
            oldSpace_.ClearRSet();
            oldSpace_.AssembleGarbageCandidates(fromSpace_);
            nonMovableSpace_.ClearRSet();
            largeSpace_.ClearRSet();
        }
    }

    void ClearAllGCInfo()
    {
        youngSpace_.ClearAllGCInfo();
        oldSpace_.ClearAllGCInfo();
        toSpace_.ClearAllGCInfo();
        fromSpace_.ClearAllGCInfo();
        nonMovableSpace_.ClearAllGCInfo();
        largeSpace_.ClearAllGCInfo();
    }

    void AssembleGarbageCandidates()
    {
        AssembleSmallGarbageCandidates();
        nonMovableSpace_.AssembleGarbageCandidates();
        largeSpace_.AssembleGarbageCandidates();
    }

    void DumpAllRegionSummary(const char *msg) const;
    void DumpAllRegionStats(const char *msg) const;

    void CountLiveObject(const BaseObject *obj)
    {
        regionManager_.CountLiveObject(obj);
    }

    void PrepareMarking()
    {
        AllocBufferVisitor visitor = [](AllocationBuffer &regionBuffer) {
            RegionDesc *region = regionBuffer.GetRegion<AllocBufferType::YOUNG>();
            if (region != RegionDesc::NullRegion()) {
                region->SetRegionAllocPtr(ToUintPtr(regionBuffer.GetTLAB().GetCurPos()));
                region->SetMarkingLine();
            }
            region = regionBuffer.GetRegion<AllocBufferType::OLD>();
            if (region != RegionDesc::NullRegion()) {
                region->SetMarkingLine();
            }
        };
        VisitAllocBuffers(visitor);

        nonMovableSpace_.PrepareMarking();
    }

    void PrepareForward()
    {
        AllocBufferVisitor visitor = [](AllocationBuffer &regionBuffer) {
            RegionDesc *region = regionBuffer.GetRegion<AllocBufferType::YOUNG>();
            if (region != RegionDesc::NullRegion()) {
                region->SetRegionAllocPtr(ToUintPtr(regionBuffer.GetTLAB().GetCurPos()));
                region->SetCopyLine();
            }
            region = regionBuffer.GetRegion<AllocBufferType::OLD>();
            if (region != RegionDesc::NullRegion()) {
                region->SetCopyLine();
            }
        };
        VisitAllocBuffers(visitor);

        nonMovableSpace_.PrepareForward();
    }

    // markObj
    static bool MarkObject(const BaseObject *obj)
    {
        RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
        return regionInfo->MarkObject(obj);
    }
    static bool ResurrentObject(const BaseObject *obj)
    {
        RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
        return regionInfo->ResurrentObject(obj);
    }

    static bool EnqueueObject(const BaseObject *obj)
    {
        RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
        return regionInfo->EnqueueObject(obj);
    }

    static bool IsSurvivedObject(const BaseObject *obj)
    {
        return IsMarkedObject(obj) || IsResurrectedObject(obj);
    }

    static bool IsMarkedObject(const ObjectHeader *obj)
    {
        RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
        return regionInfo->IsMarkedObject(obj);
    }

    static bool IsResurrectedObject(const BaseObject *obj)
    {
        RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
        return regionInfo->IsResurrectedObject(obj);
    }

    static bool IsEnqueuedObject(const BaseObject *obj)
    {
        RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
        return regionInfo->IsEnqueuedObject(obj);
    }

    static bool IsNewObjectSinceMarking(const BaseObject *object)
    {
        RegionDesc *region = RegionDesc::GetAliveRegionDescAt(object);
        ASSERT_PRINT(region != nullptr, "region is nullptr");
        return region->IsNewObjectSinceMarking(object);
    }

    static bool IsYoungSpaceObject(const BaseObject *object)
    {
        RegionDesc *region = RegionDesc::GetAliveRegionDescAt(object);
        ASSERT_PRINT(region != nullptr, "region is nullptr");
        return region->IsInYoungSpace();
    }

    void CopyRegion(RegionDesc *region);
    void MarkRememberSet(const std::function<void(BaseObject *)> &func);

    friend class Allocator;

private:
    enum class TryAllocationThreshold {
        RESCHEDULE = 3,
        TRIGGER_OOM = 4,
    };
    HeapAddress TryAllocateOnce(size_t allocSize, AllocType allocType);
    bool ShouldRetryAllocation(size_t &tryTimes) const;

    HeapAddress reservedStart_ = 0;
    HeapAddress reservedEnd_ = 0;
    RegionManager regionManager_;
    MemoryMap *map_ {nullptr};

    YoungSpace youngSpace_;
    OldSpace oldSpace_;
    FromSpace fromSpace_;
    ToSpace toSpace_;
    NonMovableSpace nonMovableSpace_;
    LargeSpace largeSpace_;
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_SPACE_H
