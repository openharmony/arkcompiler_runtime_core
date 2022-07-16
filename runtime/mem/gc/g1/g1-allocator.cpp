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

#include "runtime/include/mem/allocator-inl.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/mem/humongous_obj_allocator-inl.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/include/panda_vm.h"

namespace panda::mem {

template <MTModeT MTMode>
ObjectAllocatorG1<MTMode>::ObjectAllocatorG1(MemStatsType *mem_stats,
                                             [[maybe_unused]] bool create_pygote_space_allocator)
    : ObjectAllocatorGenBase(mem_stats, GCCollectMode::GC_ALL, false)
{
    object_allocator_ = MakePandaUnique<ObjectAllocator>(mem_stats, &heap_spaces_);
    nonmovable_allocator_ =
        MakePandaUnique<NonMovableAllocator>(mem_stats, &heap_spaces_, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    humongous_object_allocator_ =
        MakePandaUnique<HumongousObjectAllocator>(mem_stats, &heap_spaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    mem_stats_ = mem_stats;
}

template <MTModeT MTMode>
size_t ObjectAllocatorG1<MTMode>::GetRegularObjectMaxSize()
{
    return ObjectAllocator::GetMaxRegularObjectSize();
}

template <MTModeT MTMode>
size_t ObjectAllocatorG1<MTMode>::GetLargeObjectMaxSize()
{
    return ObjectAllocator::GetMaxRegularObjectSize();
}

template <MTModeT MTMode>
bool ObjectAllocatorG1<MTMode>::IsAddressInYoungSpace(uintptr_t address)
{
    auto &young_mem_ranges = GetYoungSpaceMemRanges();
    for (const auto &mem_range : young_mem_ranges) {
        if (mem_range.IsAddressInRange(address)) {
            return true;
        }
    }
    return false;
}

template <MTModeT MTMode>
bool ObjectAllocatorG1<MTMode>::IsIntersectedWithYoung(const MemRange &mem_range)
{
    auto young_mem_ranges = GetYoungSpaceMemRanges();
    for (const auto &young_mem_range : young_mem_ranges) {
        if (young_mem_range.IsIntersect(mem_range)) {
            return true;
        }
    }
    return false;
}

template <MTModeT MTMode>
bool ObjectAllocatorG1<MTMode>::HasYoungSpace()
{
    return true;
}

template <MTModeT MTMode>
const std::vector<MemRange> &ObjectAllocatorG1<MTMode>::GetYoungSpaceMemRanges()
{
    return GetYoungRanges();
}

template <MTModeT MTMode>
std::vector<MarkBitmap *> &ObjectAllocatorG1<MTMode>::GetYoungSpaceBitmaps()
{
    return GetYoungBitmaps();
}

template <MTModeT MTMode>
TLAB *ObjectAllocatorG1<MTMode>::CreateNewTLAB([[maybe_unused]] panda::ManagedThread *thread)
{
    // TODO(dtrubenkov): fix this
    return object_allocator_->CreateNewTLAB(thread, TLAB_SIZE);
}

template <MTModeT MTMode>
size_t ObjectAllocatorG1<MTMode>::GetTLABMaxAllocSize()
{
    return PANDA_TLAB_MAX_ALLOC_SIZE;
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::IterateOverObjectsInRange(MemRange mem_range, const ObjectVisitor &object_visitor)
{
    // we need ensure that the mem range related to a card must be located in one allocator
    auto space_type = PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(ToVoidPtr(mem_range.GetStartAddress()));
    switch (space_type) {
        case SpaceType::SPACE_TYPE_OBJECT:
            object_allocator_->IterateOverObjectsInRange(object_visitor, ToVoidPtr(mem_range.GetStartAddress()),
                                                         ToVoidPtr(mem_range.GetEndAddress()));
            break;
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT: {
            if (pygote_space_allocator_ != nullptr) {
                pygote_space_allocator_->IterateOverObjectsInRange(
                    object_visitor, ToVoidPtr(mem_range.GetStartAddress()), ToVoidPtr(mem_range.GetEndAddress()));
            }
            auto region = AddrToRegion(ToVoidPtr(mem_range.GetStartAddress()));
            region->GetLiveBitmap()->IterateOverMarkedChunkInRange(
                ToVoidPtr(mem_range.GetStartAddress()), ToVoidPtr(mem_range.GetEndAddress()),
                [&object_visitor](void *mem) { object_visitor(reinterpret_cast<ObjectHeader *>(mem)); });
            break;
        }
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
            humongous_object_allocator_->IterateOverObjectsInRange(
                object_visitor, ToVoidPtr(mem_range.GetStartAddress()), ToVoidPtr(mem_range.GetEndAddress()));
            break;
        default:
            // if we reach this line, we may have an issue with multiVM CardTable iteration
            UNREACHABLE();
            break;
    }
}

// maybe ObjectAllocatorGen and ObjectAllocatorNoGen should have inheritance relationship
template <MTModeT MTMode>
bool ObjectAllocatorG1<MTMode>::ContainObject(const ObjectHeader *obj) const
{
    if (pygote_space_allocator_ != nullptr && pygote_space_allocator_->ContainObject(obj)) {
        return true;
    }
    if (object_allocator_->ContainObject(obj)) {
        return true;
    }
    if (nonmovable_allocator_->ContainObject(obj)) {
        return true;
    }
    if (humongous_object_allocator_->ContainObject(obj)) {
        return true;
    }

    return false;
}

template <MTModeT MTMode>
bool ObjectAllocatorG1<MTMode>::IsLive(const ObjectHeader *obj)
{
    if (pygote_space_allocator_ != nullptr && pygote_space_allocator_->ContainObject(obj)) {
        return pygote_space_allocator_->IsLive(obj);
    }
    if (object_allocator_->ContainObject(obj)) {
        return object_allocator_->IsLive(obj);
    }
    if (nonmovable_allocator_->ContainObject(obj)) {
        return nonmovable_allocator_->IsLive(obj);
    }
    if (humongous_object_allocator_->ContainObject(obj)) {
        return humongous_object_allocator_->IsLive(obj);
    }
    return false;
}
template <MTModeT MTMode>
void *ObjectAllocatorG1<MTMode>::Allocate(size_t size, Alignment align, [[maybe_unused]] panda::ManagedThread *thread)
{
    void *mem = nullptr;
    size_t aligned_size = AlignUp(size, GetAlignmentInBytes(align));
    if (LIKELY(aligned_size <= GetYoungAllocMaxSize())) {
        mem = object_allocator_->Alloc(size, align);
    } else {
        mem = humongous_object_allocator_->Alloc(size, DEFAULT_ALIGNMENT);
    }
    return mem;
}

template <MTModeT MTMode>
void *ObjectAllocatorG1<MTMode>::AllocateNonMovable(size_t size, Alignment align,
                                                    [[maybe_unused]] panda::ManagedThread *thread)
{
    // before pygote fork, allocate small non-movable objects in pygote space
    if (UNLIKELY(IsPygoteAllocEnabled() && pygote_space_allocator_->CanAllocNonMovable(size, align))) {
        return pygote_space_allocator_->Alloc(size, align);
    }
    void *mem = nullptr;
    size_t aligned_size = AlignUp(size, GetAlignmentInBytes(align));
    if (aligned_size <= ObjectAllocator::GetMaxRegularObjectSize()) {
        // TODO(dtrubenkov): check if we don't need to handle OOM
        mem = nonmovable_allocator_->Alloc(aligned_size, align);
    } else {
        // Humongous objects are non-movable
        mem = humongous_object_allocator_->Alloc(aligned_size, align);
    }
    return mem;
}

template <MTModeT MTMode>
void *ObjectAllocatorG1<MTMode>::AllocateTenured([[maybe_unused]] size_t size)
{
    UNREACHABLE();
    return nullptr;
}

template <MTModeT MTMode>
void *ObjectAllocatorG1<MTMode>::AllocateTenuredWithoutLocks([[maybe_unused]] size_t size)
{
    UNREACHABLE();
    return nullptr;
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::VisitAndRemoveAllPools(const MemVisitor &mem_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->VisitAndRemoveAllPools(mem_visitor);
    }
    object_allocator_->VisitAndRemoveAllPools(mem_visitor);
    nonmovable_allocator_->VisitAndRemoveAllPools(mem_visitor);
    humongous_object_allocator_->VisitAndRemoveAllPools(mem_visitor);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::VisitAndRemoveFreePools(const MemVisitor &mem_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->VisitAndRemoveFreePools(mem_visitor);
    }
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::IterateOverYoungObjects(const ObjectVisitor &object_visitor)
{
    auto young_regions = object_allocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>();
    for (auto r : young_regions) {
        r->template IterateOverObjects(object_visitor);
    }
}

template <MTModeT MTMode>
PandaVector<Region *> ObjectAllocatorG1<MTMode>::GetYoungRegions()
{
    return object_allocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>();
}

template <MTModeT MTMode>
PandaVector<Region *> ObjectAllocatorG1<MTMode>::GetMovableRegions()
{
    return object_allocator_->GetAllRegions();
}

template <MTModeT MTMode>
PandaVector<Region *> ObjectAllocatorG1<MTMode>::GetAllRegions()
{
    PandaVector<Region *> regions = object_allocator_->GetAllRegions();
    PandaVector<Region *> non_movable_regions = nonmovable_allocator_->GetAllRegions();
    PandaVector<Region *> humongous_regions = humongous_object_allocator_->GetAllRegions();
    regions.insert(regions.end(), non_movable_regions.begin(), non_movable_regions.end());
    regions.insert(regions.end(), humongous_regions.begin(), humongous_regions.end());
    return regions;
}

template <MTModeT MTMode>
PandaVector<Region *> ObjectAllocatorG1<MTMode>::GetNonRegularRegions()
{
    PandaVector<Region *> regions = nonmovable_allocator_->GetAllRegions();
    PandaVector<Region *> humongous_regions = humongous_object_allocator_->GetAllRegions();
    regions.insert(regions.end(), humongous_regions.begin(), humongous_regions.end());
    return regions;
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::CollectNonRegularRegions(const RegionsVisitor &region_visitor,
                                                         const GCObjectVisitor &gc_object_visitor)
{
    nonmovable_allocator_->Collect(gc_object_visitor);
    nonmovable_allocator_->VisitAndRemoveFreeRegions(region_visitor);
    humongous_object_allocator_->CollectAndRemoveFreeRegions(region_visitor, gc_object_visitor);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::IterateOverTenuredObjects(const ObjectVisitor &object_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->IterateOverObjects(object_visitor);
    }
    object_allocator_->IterateOverObjects(object_visitor);
    nonmovable_allocator_->IterateOverObjects(object_visitor);
    IterateOverHumongousObjects(object_visitor);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::IterateOverHumongousObjects(const ObjectVisitor &object_visitor)
{
    humongous_object_allocator_->IterateOverObjects(object_visitor);
}

static inline void IterateOverObjectsInRegion(Region *region, const ObjectVisitor &object_visitor)
{
    if (region->GetLiveBitmap() != nullptr) {
        region->GetLiveBitmap()->IterateOverMarkedChunks(
            [&object_visitor](void *mem) { object_visitor(static_cast<ObjectHeader *>(mem)); });
    } else {
        region->IterateOverObjects(object_visitor);
    }
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::IterateOverObjects(const ObjectVisitor &object_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->IterateOverObjects(object_visitor);
    }
    for (Region *region : object_allocator_->GetAllRegions()) {
        IterateOverObjectsInRegion(region, object_visitor);
    }
    for (Region *region : nonmovable_allocator_->GetAllRegions()) {
        IterateOverObjectsInRegion(region, object_visitor);
    }
    for (Region *region : humongous_object_allocator_->GetAllRegions()) {
        IterateOverObjectsInRegion(region, object_visitor);
    }
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::IterateRegularSizeObjects(const ObjectVisitor &object_visitor)
{
    object_allocator_->IterateOverObjects(object_visitor);
    nonmovable_allocator_->IterateOverObjects(object_visitor);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::IterateNonRegularSizeObjects(const ObjectVisitor &object_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->IterateOverObjects(object_visitor);
    }
    humongous_object_allocator_->IterateOverObjects(object_visitor);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::FreeObjectsMovedToPygoteSpace()
{
    // clear because we have move all objects in it to pygote space
    // TODO(dtrubenkov): FIX clean object_allocator_
    object_allocator_.reset(new (std::nothrow) ObjectAllocator(mem_stats_, &heap_spaces_));
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::ResetYoungAllocator()
{
    MemStatsType *mem_stats = mem_stats_;
    auto callback = [&mem_stats](ManagedThread *thread) {
        if (!PANDA_TRACK_TLAB_ALLOCATIONS && (thread->GetTLAB()->GetOccupiedSize() != 0)) {
            mem_stats->RecordAllocateObject(thread->GetTLAB()->GetOccupiedSize(), SpaceType::SPACE_TYPE_OBJECT);
        }
        thread->ClearTLAB();
        return true;
    };
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (MTMode == MT_MODE_MULTI) {
        Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(callback);
    } else if (MTMode == MT_MODE_SINGLE) {  // NOLINT(readability-misleading-indentation)
        callback(Thread::GetCurrent()->GetVM()->GetAssociatedThread());
    } else {
        UNREACHABLE();
    }
    object_allocator_->ResetAllSpecificRegions<RegionFlag::IS_EDEN>();
}

template <MTModeT MTMode>
bool ObjectAllocatorG1<MTMode>::IsObjectInNonMovableSpace(const ObjectHeader *obj)
{
    return nonmovable_allocator_->ContainObject(obj);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::UpdateSpaceData()
{
    ASSERT(GetYoungRanges().empty());
    ASSERT(GetYoungBitmaps().empty());
    for (auto r : object_allocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>()) {
        GetYoungRanges().emplace_back(r->Begin(), r->End());
        GetYoungBitmaps().push_back(r->GetMarkBitmap());
    }
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::CompactYoungRegions(const GCObjectVisitor &death_checker,
                                                    const ObjectVisitorEx &move_checker)
{
    object_allocator_->template CompactAllSpecificRegions<RegionFlag::IS_EDEN, RegionFlag::IS_OLD>(death_checker,
                                                                                                   move_checker);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::PromoteYoungRegion(Region *region, const GCObjectVisitor &death_checker,
                                                   const ObjectVisitor &promotion_checker)
{
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    object_allocator_->template PromoteYoungRegion(region, death_checker, promotion_checker);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::CompactTenuredRegions(const PandaVector<Region *> &regions,
                                                      const GCObjectVisitor &death_checker,
                                                      const ObjectVisitorEx &move_checker)
{
    object_allocator_->template CompactSeveralSpecificRegions<RegionFlag::IS_OLD, RegionFlag::IS_OLD>(
        regions, death_checker, move_checker);
}

template <MTModeT MTMode>
void ObjectAllocatorG1<MTMode>::ClearCurrentRegion()
{
    object_allocator_->template ClearCurrentRegion<RegionFlag::IS_OLD>();
}

template class ObjectAllocatorG1<MT_MODE_SINGLE>;
template class ObjectAllocatorG1<MT_MODE_MULTI>;
template class ObjectAllocatorG1<MT_MODE_TASK>;

}  // namespace panda::mem
