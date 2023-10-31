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

#ifndef RUNTIME_MEM_GC_G1_G1_ALLOCATOR_H
#define RUNTIME_MEM_GC_G1_G1_ALLOCATOR_H

#include "runtime/include/mem/allocator.h"
#include "runtime/mem/region_allocator.h"
#include "runtime/mem/region_allocator-inl.h"
#include "runtime/mem/gc/g1/g1-allocator_constants.h"

namespace panda::mem {
class ObjectAllocConfigWithCrossingMap;
class ObjectAllocConfig;
class TLAB;

template <MTModeT MT_MODE = MT_MODE_MULTI>
class ObjectAllocatorG1 final : public ObjectAllocatorGenBase {
    static constexpr size_t TLAB_SIZE = 4_KB;  // TLAB size for young gen

    using ObjectAllocator = RegionAllocator<ObjectAllocConfig>;
    using NonMovableAllocator = RegionNonmovableAllocator<ObjectAllocConfig, RegionAllocatorLockConfig::CommonLock,
                                                          FreeListAllocator<ObjectAllocConfig>>;
    using HumongousObjectAllocator =
        RegionHumongousAllocator<ObjectAllocConfig>;  // Allocator used for humongous objects

    // REGION_SIZE should not change here.
    // If it is necessary to change this value, it must be done through changes to G1_REGION_SIZE
    static constexpr size_t REGION_SIZE = mem::G1_REGION_SIZE;
    static_assert(REGION_SIZE == mem::G1_REGION_SIZE);

public:
    NO_MOVE_SEMANTIC(ObjectAllocatorG1);
    NO_COPY_SEMANTIC(ObjectAllocatorG1);

    explicit ObjectAllocatorG1(MemStatsType *mem_stats, bool create_pygote_space_allocator);

    ~ObjectAllocatorG1() final = default;

    void *Allocate(size_t size, Alignment align, [[maybe_unused]] panda::ManagedThread *thread,
                   ObjMemInitPolicy obj_init) final;

    void *AllocateNonMovable(size_t size, Alignment align, [[maybe_unused]] panda::ManagedThread *thread,
                             ObjMemInitPolicy obj_init) final;

    void PinObject(ObjectHeader *object) final;

    void UnpinObject(ObjectHeader *object) final;

    void VisitAndRemoveAllPools(const MemVisitor &mem_visitor) final;

    void VisitAndRemoveFreePools(const MemVisitor &mem_visitor) final;

    void IterateOverYoungObjects(const ObjectVisitor &object_visitor) final;

    size_t GetMaxYoungRegionsCount();

    PandaVector<Region *> GetYoungRegions();

    PandaVector<Region *> GetMovableRegions();

    PandaVector<Region *> GetAllRegions();

    /// Returns a vector which contains non-movable and humongous regions
    PandaVector<Region *> GetNonRegularRegions();

    void IterateOverTenuredObjects(const ObjectVisitor &object_visitor) final;

    void IterateOverHumongousObjects(const ObjectVisitor &object_visitor);

    void IterateOverObjects(const ObjectVisitor &object_visitor) final;

    /// @brief iterates all objects in object allocator
    void IterateRegularSizeObjects(const ObjectVisitor &object_visitor) final;

    /// @brief iterates objects in all allocators except object allocator
    void IterateNonRegularSizeObjects(const ObjectVisitor &object_visitor) final;

    void FreeObjectsMovedToPygoteSpace() final;

    void Collect(const GCObjectVisitor &gc_object_visitor, GCCollectMode collect_mode) final
    {
        (void)gc_object_visitor;
        (void)collect_mode;
        UNREACHABLE();
    }

    /**
     * Collect non regular regions (i.e. remove dead objects from Humongous and NonMovable regions
     * and remove empty regions).
     */
    void CollectNonRegularRegions(const RegionsVisitor &region_visitor, const GCObjectVisitor &gc_object_visitor);

    size_t GetRegularObjectMaxSize() final;

    size_t GetLargeObjectMaxSize() final;

    bool IsObjectInYoungSpace(const ObjectHeader *obj) final;

    bool IsIntersectedWithYoung(const MemRange &mem_range) final;

    bool HasYoungSpace() final;

    const std::vector<MemRange> &GetYoungSpaceMemRanges() final;

    template <bool INCLUDE_CURRENT_REGION>
    PandaPriorityQueue<std::pair<uint32_t, Region *>> GetTopGarbageRegions()
    {
        return object_allocator_->template GetTopGarbageRegions<INCLUDE_CURRENT_REGION>();
    }

    std::vector<MarkBitmap *> &GetYoungSpaceBitmaps() final;

    void ReserveRegionIfNeeded()
    {
        object_allocator_->ReserveRegionIfNeeded();
    }

    void ReleaseReservedRegion()
    {
        object_allocator_->ReleaseReservedRegion();
    }

    void ResetYoungAllocator() final;

    template <RegionFlag REGIONS_TYPE, RegionSpace::ReleaseRegionsPolicy REGIONS_RELEASE_POLICY,
              OSPagesPolicy OS_PAGES_POLICY, bool NEED_LOCK, typename Container>
    void ResetRegions(const Container &regions)
    {
        object_allocator_
            ->ResetSeveralSpecificRegions<REGIONS_TYPE, REGIONS_RELEASE_POLICY, OS_PAGES_POLICY, NEED_LOCK>(regions);
    }

    TLAB *CreateNewTLAB(panda::ManagedThread *thread) final;

    size_t GetTLABMaxAllocSize() final;

    bool IsTLABSupported() final
    {
        return true;
    }

    void IterateOverObjectsInRange(MemRange mem_range, const ObjectVisitor &object_visitor) final;

    bool ContainObject(const ObjectHeader *obj) const final;

    bool IsLive(const ObjectHeader *obj) final;

    size_t VerifyAllocatorStatus() final
    {
        LOG(FATAL, ALLOC) << "Not implemented";
        return 0;
    }

    [[nodiscard]] void *AllocateLocal([[maybe_unused]] size_t size, [[maybe_unused]] Alignment align,
                                      [[maybe_unused]] panda::ManagedThread *thread) final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorGen: AllocateLocal not supported";
        return nullptr;
    }

    bool IsObjectInNonMovableSpace(const ObjectHeader *obj) final;

    void UpdateSpaceData() final;

    void CompactYoungRegions(const GCObjectVisitor &death_checker, const ObjectVisitorEx &move_checker);

    template <RegionFlag REGION_TYPE, bool USE_MARKBITMAP = false>
    void CompactRegion(Region *region, const GCObjectVisitor &death_checker, const ObjectVisitorEx &move_checker)
    {
        object_allocator_->template CompactSpecificRegion<REGION_TYPE, RegionFlag::IS_OLD, USE_MARKBITMAP>(
            region, death_checker, move_checker);
    }

    template <bool USE_MARKBITMAP>
    void PromoteYoungRegion(Region *region, const GCObjectVisitor &death_checker,
                            const ObjectVisitor &promotion_checker)
    {
        ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
        object_allocator_->template PromoteYoungRegion<USE_MARKBITMAP>(region, death_checker, promotion_checker);
    }

    void CompactTenuredRegions(const PandaVector<Region *> &regions, const GCObjectVisitor &death_checker,
                               const ObjectVisitorEx &move_checker);

    void ClearCurrentTenuredRegion()
    {
        object_allocator_->template ClearCurrentRegion<IS_OLD>();
    }

    static constexpr size_t GetRegionSize()
    {
        return REGION_SIZE;
    }

    bool HaveTenuredSize(size_t num_regions) const
    {
        return object_allocator_->GetSpace()->GetPool()->HaveTenuredSize(num_regions * ObjectAllocator::REGION_SIZE);
    }

    bool HaveFreeRegions(size_t num_regions) const
    {
        return object_allocator_->GetSpace()->GetPool()->HaveFreeRegions(num_regions, ObjectAllocator::REGION_SIZE);
    }

    static constexpr size_t GetYoungAllocMaxSize()
    {
        // NOTE(dtrubenkov): FIX to more meaningful value
        return ObjectAllocator::GetMaxRegularObjectSize();
    }

    template <RegionFlag REGION_TYPE, OSPagesPolicy OS_PAGES_POLICY>
    void ReleaseEmptyRegions()
    {
        object_allocator_->ReleaseEmptyRegions<REGION_TYPE, OS_PAGES_POLICY>();
    }

    void SetDesiredEdenLength(size_t eden_length)
    {
        object_allocator_->SetDesiredEdenLength(eden_length);
    }

private:
    Alignment CalculateAllocatorAlignment(size_t align) final;

    PandaUniquePtr<ObjectAllocator> object_allocator_ {nullptr};
    PandaUniquePtr<NonMovableAllocator> nonmovable_allocator_ {nullptr};
    PandaUniquePtr<HumongousObjectAllocator> humongous_object_allocator_ {nullptr};
    MemStatsType *mem_stats_ {nullptr};

    void *AllocateTenured(size_t size) final;

    void *AllocateTenuredWithoutLocks(size_t size) final;

    friend class AllocTypeConfigG1;
};

}  // namespace panda::mem

#endif  // RUNTIME_MEM_GC_G1_G1_ALLOCATOR_H
