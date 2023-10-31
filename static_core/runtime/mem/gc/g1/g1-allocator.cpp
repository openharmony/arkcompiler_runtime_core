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

template <MTModeT MT_MODE>
ObjectAllocatorG1<MT_MODE>::ObjectAllocatorG1(MemStatsType *mem_stats,
                                              [[maybe_unused]] bool create_pygote_space_allocator)
    : ObjectAllocatorGenBase(mem_stats, GCCollectMode::GC_ALL, false)
{
    size_t reserved_tenured_regions_count = Runtime::GetOptions().GetG1NumberOfTenuredRegionsAtMixedCollection();
    object_allocator_ = MakePandaUnique<ObjectAllocator>(mem_stats, &heap_spaces_, SpaceType::SPACE_TYPE_OBJECT, 0,
                                                         true, reserved_tenured_regions_count);
    nonmovable_allocator_ =
        MakePandaUnique<NonMovableAllocator>(mem_stats, &heap_spaces_, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    humongous_object_allocator_ =
        MakePandaUnique<HumongousObjectAllocator>(mem_stats, &heap_spaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    mem_stats_ = mem_stats;
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorG1<MT_MODE>::GetRegularObjectMaxSize()
{
    return ObjectAllocator::GetMaxRegularObjectSize();
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorG1<MT_MODE>::GetLargeObjectMaxSize()
{
    return ObjectAllocator::GetMaxRegularObjectSize();
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsObjectInYoungSpace(const ObjectHeader *obj)
{
    Region *reg_with_obj = ObjectToRegion(obj);
    ASSERT(reg_with_obj != nullptr);
    return reg_with_obj->IsYoung();
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsIntersectedWithYoung(const MemRange &mem_range)
{
    auto young_mem_ranges = GetYoungSpaceMemRanges();
    for (const auto &young_mem_range : young_mem_ranges) {
        if (young_mem_range.IsIntersect(mem_range)) {
            return true;
        }
    }
    return false;
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::HasYoungSpace()
{
    return true;
}

template <MTModeT MT_MODE>
const std::vector<MemRange> &ObjectAllocatorG1<MT_MODE>::GetYoungSpaceMemRanges()
{
    return GetYoungRanges();
}

template <MTModeT MT_MODE>
std::vector<MarkBitmap *> &ObjectAllocatorG1<MT_MODE>::GetYoungSpaceBitmaps()
{
    return GetYoungBitmaps();
}

template <MTModeT MT_MODE>
TLAB *ObjectAllocatorG1<MT_MODE>::CreateNewTLAB([[maybe_unused]] panda::ManagedThread *thread)
{
    TLAB *new_tlab = nullptr;
    if constexpr (MT_MODE == MT_MODE_SINGLE) {
        // For single-threaded VMs allocate a whole region for TLAB
        new_tlab = object_allocator_->CreateRegionSizeTLAB();
    } else {
        new_tlab = object_allocator_->CreateTLAB(TLAB_SIZE);
    }
    if (new_tlab != nullptr) {
        ASAN_UNPOISON_MEMORY_REGION(new_tlab->GetStartAddr(), new_tlab->GetSize());
        MemoryInitialize(new_tlab->GetStartAddr(), new_tlab->GetSize());
        ASAN_POISON_MEMORY_REGION(new_tlab->GetStartAddr(), new_tlab->GetSize());
    }
    return new_tlab;
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorG1<MT_MODE>::GetTLABMaxAllocSize()
{
    if constexpr (MT_MODE == MT_MODE_SINGLE) {
        // For single-threaded VMs we can allocate objects of size up to region size in TLABs.
        return GetYoungAllocMaxSize();
    } else {
        return PANDA_TLAB_MAX_ALLOC_SIZE;
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverObjectsInRange(MemRange mem_range, const ObjectVisitor &object_visitor)
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
template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::ContainObject(const ObjectHeader *obj) const
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

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsLive(const ObjectHeader *obj)
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

template <MTModeT MT_MODE>
void *ObjectAllocatorG1<MT_MODE>::Allocate(size_t size, Alignment align, [[maybe_unused]] panda::ManagedThread *thread,
                                           ObjMemInitPolicy obj_init)
{
    void *mem = nullptr;
    size_t aligned_size;
    aligned_size = AlignUp(size, GetAlignmentInBytes(align));
    if (LIKELY(aligned_size <= GetYoungAllocMaxSize())) {
        mem = object_allocator_->Alloc(size, align);
    } else {
        mem = humongous_object_allocator_->Alloc(size, DEFAULT_ALIGNMENT);
        // Humongous allocations have initialized memory by a default
        return mem;
    }
    if (obj_init == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorG1<MT_MODE>::AllocateNonMovable(size_t size, Alignment align,
                                                     [[maybe_unused]] panda::ManagedThread *thread,
                                                     ObjMemInitPolicy obj_init)
{
    void *mem = nullptr;
    // before pygote fork, allocate small non-movable objects in pygote space
    if (UNLIKELY(IsPygoteAllocEnabled() && pygote_space_allocator_->CanAllocNonMovable(size, align))) {
        mem = pygote_space_allocator_->Alloc(size, align);
    } else {
        size_t aligned_size;
        aligned_size = AlignUp(size, GetAlignmentInBytes(align));
        if (aligned_size <= ObjectAllocator::GetMaxRegularObjectSize()) {
            // NOTE(dtrubenkov): check if we don't need to handle OOM
            mem = nonmovable_allocator_->Alloc(aligned_size, align);
        } else {
            // Humongous objects are non-movable
            mem = humongous_object_allocator_->Alloc(aligned_size, align);
            // Humongous allocations have initialized memory by a default
            return mem;
        }
    }
    if (obj_init == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
Alignment ObjectAllocatorG1<MT_MODE>::CalculateAllocatorAlignment(size_t align)
{
    ASSERT(GetPurpose() == AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT);
    return GetAlignment(align);
}

template <MTModeT MT_MODE>
void *ObjectAllocatorG1<MT_MODE>::AllocateTenured([[maybe_unused]] size_t size)
{
    UNREACHABLE();
    return nullptr;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorG1<MT_MODE>::AllocateTenuredWithoutLocks([[maybe_unused]] size_t size)
{
    UNREACHABLE();
    return nullptr;
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::VisitAndRemoveAllPools(const MemVisitor &mem_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->VisitAndRemoveAllPools(mem_visitor);
    }
    object_allocator_->VisitAndRemoveAllPools(mem_visitor);
    nonmovable_allocator_->VisitAndRemoveAllPools(mem_visitor);
    humongous_object_allocator_->VisitAndRemoveAllPools(mem_visitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::VisitAndRemoveFreePools(const MemVisitor &mem_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->VisitAndRemoveFreePools(mem_visitor);
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverYoungObjects(const ObjectVisitor &object_visitor)
{
    auto young_regions = object_allocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>();
    for (auto r : young_regions) {
        r->template IterateOverObjects(object_visitor);
    }
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorG1<MT_MODE>::GetMaxYoungRegionsCount()
{
    return GetHeapSpace()->GetMaxYoungSize() / REGION_SIZE;
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetYoungRegions()
{
    return object_allocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>();
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetMovableRegions()
{
    return object_allocator_->GetAllRegions();
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetAllRegions()
{
    PandaVector<Region *> regions = object_allocator_->GetAllRegions();
    PandaVector<Region *> non_movable_regions = nonmovable_allocator_->GetAllRegions();
    PandaVector<Region *> humongous_regions = humongous_object_allocator_->GetAllRegions();
    regions.insert(regions.end(), non_movable_regions.begin(), non_movable_regions.end());
    regions.insert(regions.end(), humongous_regions.begin(), humongous_regions.end());
    return regions;
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetNonRegularRegions()
{
    PandaVector<Region *> regions = nonmovable_allocator_->GetAllRegions();
    PandaVector<Region *> humongous_regions = humongous_object_allocator_->GetAllRegions();
    regions.insert(regions.end(), humongous_regions.begin(), humongous_regions.end());
    return regions;
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::CollectNonRegularRegions(const RegionsVisitor &region_visitor,
                                                          const GCObjectVisitor &gc_object_visitor)
{
    nonmovable_allocator_->Collect(gc_object_visitor);
    nonmovable_allocator_->VisitAndRemoveFreeRegions(region_visitor);
    humongous_object_allocator_->CollectAndRemoveFreeRegions(region_visitor, gc_object_visitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverTenuredObjects(const ObjectVisitor &object_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->IterateOverObjects(object_visitor);
    }
    object_allocator_->IterateOverObjects(object_visitor);
    nonmovable_allocator_->IterateOverObjects(object_visitor);
    IterateOverHumongousObjects(object_visitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverHumongousObjects(const ObjectVisitor &object_visitor)
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

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverObjects(const ObjectVisitor &object_visitor)
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

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateRegularSizeObjects(const ObjectVisitor &object_visitor)
{
    object_allocator_->IterateOverObjects(object_visitor);
    nonmovable_allocator_->IterateOverObjects(object_visitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateNonRegularSizeObjects(const ObjectVisitor &object_visitor)
{
    if (pygote_space_allocator_ != nullptr) {
        pygote_space_allocator_->IterateOverObjects(object_visitor);
    }
    humongous_object_allocator_->IterateOverObjects(object_visitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::FreeObjectsMovedToPygoteSpace()
{
    // clear because we have move all objects in it to pygote space
    // NOTE(dtrubenkov): FIX clean object_allocator_
    object_allocator_.reset(new (std::nothrow) ObjectAllocator(mem_stats_, &heap_spaces_));
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::ResetYoungAllocator()
{
    MemStatsType *mem_stats = mem_stats_;
    auto callback = [&mem_stats](ManagedThread *thread) {
        if (!PANDA_TRACK_TLAB_ALLOCATIONS && (thread->GetTLAB()->GetOccupiedSize() != 0)) {
            mem_stats->RecordAllocateObject(thread->GetTLAB()->GetOccupiedSize(), SpaceType::SPACE_TYPE_OBJECT);
        }
        thread->ClearTLAB();
        return true;
    };
    Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(callback);
    object_allocator_->ResetAllSpecificRegions<RegionFlag::IS_EDEN>();
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsObjectInNonMovableSpace(const ObjectHeader *obj)
{
    return nonmovable_allocator_->ContainObject(obj);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::UpdateSpaceData()
{
    ASSERT(GetYoungRanges().empty());
    ASSERT(GetYoungBitmaps().empty());
    for (auto r : object_allocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>()) {
        GetYoungRanges().emplace_back(r->Begin(), r->End());
        GetYoungBitmaps().push_back(r->GetMarkBitmap());
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::CompactYoungRegions(const GCObjectVisitor &death_checker,
                                                     const ObjectVisitorEx &move_checker)
{
    object_allocator_->template CompactAllSpecificRegions<RegionFlag::IS_EDEN, RegionFlag::IS_OLD>(death_checker,
                                                                                                   move_checker);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::CompactTenuredRegions(const PandaVector<Region *> &regions,
                                                       const GCObjectVisitor &death_checker,
                                                       const ObjectVisitorEx &move_checker)
{
    object_allocator_->template CompactSeveralSpecificRegions<RegionFlag::IS_OLD, RegionFlag::IS_OLD>(
        regions, death_checker, move_checker);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::PinObject(ObjectHeader *object)
{
    if (object_allocator_->ContainObject(object)) {
        object_allocator_->PinObject(object);
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::UnpinObject(ObjectHeader *object)
{
    if (object_allocator_->ContainObject(object)) {
        object_allocator_->UnpinObject(object);
    }
}

template class ObjectAllocatorG1<MT_MODE_SINGLE>;
template class ObjectAllocatorG1<MT_MODE_MULTI>;
template class ObjectAllocatorG1<MT_MODE_TASK>;

}  // namespace panda::mem
