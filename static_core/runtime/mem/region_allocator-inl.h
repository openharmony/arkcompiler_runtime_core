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
#ifndef PANDA_RUNTIME_MEM_REGION_ALLOCATOR_INL_H
#define PANDA_RUNTIME_MEM_REGION_ALLOCATOR_INL_H

#include "libpandabase/mem/mem.h"
#include "libpandabase/utils/logger.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/gc_task.h"
#include "runtime/mem/region_allocator.h"
#include "runtime/mem/region_space-inl.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/mem/alloc_config.h"
#include "runtime/arch/memory_helpers.h"

namespace panda::mem {

template <typename LockConfigT>
RegionAllocatorBase<LockConfigT>::RegionAllocatorBase(MemStatsType *mem_stats, GenerationalSpaces *spaces,
                                                      SpaceType space_type, AllocatorType allocator_type,
                                                      size_t init_space_size, bool extend, size_t region_size,
                                                      size_t empty_tenured_regions_max_count)
    : mem_stats_(mem_stats),
      space_type_(space_type),
      spaces_(spaces),
      region_pool_(region_size, extend, spaces,
                   InternalAllocatorPtr(InternalAllocator<>::GetInternalAllocatorFromRuntime())),
      region_space_(space_type, allocator_type, &region_pool_, empty_tenured_regions_max_count),
      init_block_(0, nullptr)
{
    ASSERT(space_type_ == SpaceType::SPACE_TYPE_OBJECT || space_type_ == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT ||
           space_type_ == SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    init_block_ = NULLPOOL;
    if (init_space_size > 0) {
        ASSERT(init_space_size % region_size == 0);
        init_block_ = spaces_->AllocSharedPool(init_space_size, space_type, AllocatorType::REGION_ALLOCATOR, this);
        ASSERT(init_block_.GetMem() != nullptr);
        ASSERT(init_block_.GetSize() >= init_space_size);
        if (init_block_.GetMem() != nullptr) {
            region_pool_.InitRegionBlock(ToUintPtr(init_block_.GetMem()),
                                         ToUintPtr(init_block_.GetMem()) + init_space_size);
            ASAN_POISON_MEMORY_REGION(init_block_.GetMem(), init_block_.GetSize());
        }
    }
}

template <typename LockConfigT>
RegionAllocatorBase<LockConfigT>::RegionAllocatorBase(MemStatsType *mem_stats, GenerationalSpaces *spaces,
                                                      SpaceType space_type, AllocatorType allocator_type,
                                                      RegionPool *shared_region_pool,
                                                      size_t empty_tenured_regions_max_count)
    : mem_stats_(mem_stats),
      spaces_(spaces),
      space_type_(space_type),
      region_pool_(0, false, spaces, nullptr),  // unused
      region_space_(space_type, allocator_type, shared_region_pool, empty_tenured_regions_max_count),
      init_block_(0, nullptr)  // unused
{
    ASSERT(space_type_ == SpaceType::SPACE_TYPE_OBJECT || space_type_ == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
}

template <typename LockConfigT>
template <typename AllocConfigT, OSPagesAllocPolicy OS_ALLOC_POLICY>
Region *RegionAllocatorBase<LockConfigT>::CreateAndSetUpNewRegion(size_t region_size, RegionFlag region_type,
                                                                  RegionFlag properties)
{
    Region *region = AllocRegion<OS_ALLOC_POLICY>(region_size, region_type, properties);
    if (LIKELY(region != nullptr)) {
        if (region_type == RegionFlag::IS_EDEN) {
            AllocConfigT::OnInitYoungRegion({region->Begin(), region->End()});
        }
        // Do memory barrier here to make sure all threads see references to bitmaps.
        // The situation:
        // A mutator thread allocates a new object. During object allocation the mutator
        // allocates a new region, sets up the region header, allocates object in the region and publishes
        // the reference to the object.
        // GC thread does concurrent marking. It sees the reference to the new object and gets the region
        // by the object address.
        // Since GC thread doesn't locks region_lock_ we need to do memory barrier here to make
        // sure GC thread sees all bitmaps from the region header.
        arch::FullMemoryBarrier();
        // Getting region by object is a bit operation and TSAN doesn't
        // sees the relation between region creation and region access.
        // This annotation suggests TSAN that this code always executes before
        // the region will be accessed.
        // See the corresponding annotation in ObjectToRegion
        TSAN_ANNOTATE_HAPPENS_BEFORE(region);
    }
    return region;
}

template <typename LockConfigT>
PandaVector<Region *> RegionAllocatorBase<LockConfigT>::GetAllRegions()
{
    PandaVector<Region *> vector;
    os::memory::LockHolder lock(this->region_lock_);
    GetSpace()->IterateRegions([&](Region *region) { vector.push_back(region); });
    return vector;
}

template <typename AllocConfigT, typename LockConfigT>
RegionAllocator<AllocConfigT, LockConfigT>::RegionAllocator(MemStatsType *mem_stats, GenerationalSpaces *spaces,
                                                            SpaceType space_type, size_t init_space_size, bool extend,
                                                            size_t empty_tenured_regions_max_count)
    : RegionAllocatorBase<LockConfigT>(mem_stats, spaces, space_type, AllocatorType::REGION_ALLOCATOR, init_space_size,
                                       extend, REGION_SIZE, empty_tenured_regions_max_count),
      full_region_(nullptr, 0, 0),
      eden_current_region_(&full_region_)
{
}

template <typename AllocConfigT, typename LockConfigT>
RegionAllocator<AllocConfigT, LockConfigT>::RegionAllocator(MemStatsType *mem_stats, GenerationalSpaces *spaces,
                                                            SpaceType space_type, RegionPool *shared_region_pool,
                                                            size_t empty_tenured_regions_max_count)
    : RegionAllocatorBase<LockConfigT>(mem_stats, spaces, space_type, AllocatorType::REGION_ALLOCATOR,
                                       shared_region_pool, empty_tenured_regions_max_count),
      full_region_(nullptr, 0, 0),
      eden_current_region_(&full_region_)
{
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGION_TYPE>
void *RegionAllocator<AllocConfigT, LockConfigT>::AllocRegular(size_t align_size)
{
    static constexpr bool IS_ATOMIC = std::is_same_v<LockConfigT, RegionAllocatorLockConfig::CommonLock>;
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
        void *mem = GetCurrentRegion<IS_ATOMIC, REGION_TYPE>()->template Alloc<IS_ATOMIC>(align_size);
        if (mem != nullptr) {
            return mem;
        }

        os::memory::LockHolder lock(this->region_lock_);
        mem = GetCurrentRegion<IS_ATOMIC, REGION_TYPE>()->template Alloc<IS_ATOMIC>(align_size);
        if (mem != nullptr) {
            return mem;
        }

        Region *region = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, REGION_TYPE);
        if (LIKELY(region != nullptr)) {
            // Here we need memory barrier to make the allocation visible
            // in all threads before SetCurrentRegion
            mem = region->template Alloc<IS_ATOMIC>(align_size);
            SetCurrentRegion<IS_ATOMIC, REGION_TYPE>(region);
        }

        return mem;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (REGION_TYPE == RegionFlag::IS_OLD) {
        void *mem = nullptr;
        Region *region_to = PopFromRegionQueue<IS_ATOMIC, REGION_TYPE>();
        if (region_to != nullptr) {
            mem = region_to->template Alloc<false>(align_size);
            if (mem != nullptr) {
                PushToRegionQueue<IS_ATOMIC, REGION_TYPE>(region_to);
                return mem;
            }
        }

        os::memory::LockHolder lock(this->region_lock_);
        region_to = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, REGION_TYPE);
        if (LIKELY(region_to != nullptr)) {
            mem = region_to->template Alloc<false>(align_size);
            PushToRegionQueue<IS_ATOMIC, REGION_TYPE>(region_to);
        }

        return mem;
    }
    UNREACHABLE();
    return nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGION_TYPE, bool UPDATE_MEMSTATS>
void *RegionAllocator<AllocConfigT, LockConfigT>::Alloc(size_t size, Alignment align)
{
    ASSERT(GetAlignmentInBytes(align) % GetAlignmentInBytes(DEFAULT_ALIGNMENT) == 0);
    size_t align_size = AlignUp(size, GetAlignmentInBytes(align));
    void *mem = nullptr;
    // for movable & regular size object, allocate it from a region
    // for nonmovable or large size object, allocate a seprate large region for it
    if (this->GetSpaceType() != SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT &&
        LIKELY(align_size <= GetMaxRegularObjectSize())) {
        mem = AllocRegular<REGION_TYPE>(align_size);
    } else {
        os::memory::LockHolder lock(this->region_lock_);
        Region *region = this->template CreateAndSetUpNewRegion<AllocConfigT>(
            Region::RegionSize(align_size, REGION_SIZE), REGION_TYPE, IS_LARGE_OBJECT);
        if (LIKELY(region != nullptr)) {
            mem = region->Alloc<false>(align_size);
        }
    }
    if (mem != nullptr) {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (UPDATE_MEMSTATS) {
            AllocConfigT::OnAlloc(align_size, this->space_type_, this->mem_stats_);
            AllocConfigT::MemoryInit(mem);
        }
    }
    return mem;
}

template <typename AllocConfigT, typename LockConfigT>
void RegionAllocator<AllocConfigT, LockConfigT>::PinObject(ObjectHeader *object)
{
    auto *region = ObjectToRegion(object);
    ASSERT(region != nullptr);
    region->PinObject();
}

template <typename AllocConfigT, typename LockConfigT>
void RegionAllocator<AllocConfigT, LockConfigT>::UnpinObject(ObjectHeader *object)
{
    auto *region = ObjectToRegion(object);
    ASSERT(region != nullptr);
    region->UnpinObject();
}

template <typename AllocConfigT, typename LockConfigT>
TLAB *RegionAllocator<AllocConfigT, LockConfigT>::CreateTLAB(size_t size)
{
    ASSERT(size <= GetMaxRegularObjectSize());
    ASSERT(AlignUp(size, GetAlignmentInBytes(DEFAULT_ALIGNMENT)) == size);
    TLAB *tlab = nullptr;

    {
        os::memory::LockHolder lock(this->region_lock_);
        Region *region = nullptr;
        // first search in partial tlab map
        auto largest_tlab = retained_tlabs_.begin();
        if (largest_tlab != retained_tlabs_.end() && largest_tlab->first >= size) {
            LOG(DEBUG, ALLOC) << "Use retained tlabs region " << region;
            region = largest_tlab->second;
            retained_tlabs_.erase(largest_tlab);
            ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
        }

        // allocate a free region if none partial tlab has enough space
        if (region == nullptr) {
            region = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, RegionFlag::IS_EDEN);
            if (LIKELY(region != nullptr)) {
                region->CreateTLABSupport();
            }
        }
        if (region != nullptr) {
            tlab = CreateTLABInRegion(region, size);
            auto remaining_size = region->GetRemainingSizeForTLABs();
            if (remaining_size >= size) {
                LOG(DEBUG, ALLOC) << "Add a region " << region << " with remained size " << remaining_size
                                  << " to retained_tlabs";
                retained_tlabs_.insert(std::make_pair(remaining_size, region));
            }
        }
    }

    return tlab;
}

template <typename AllocConfigT, typename LockConfigT>
TLAB *RegionAllocator<AllocConfigT, LockConfigT>::CreateRegionSizeTLAB()
{
    TLAB *tlab = nullptr;

    os::memory::LockHolder lock(this->region_lock_);
    Region *region = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, RegionFlag::IS_EDEN);
    if (LIKELY(region != nullptr)) {
        region->CreateTLABSupport();
        size_t size = region->GetRemainingSizeForTLABs();
        tlab = CreateTLABInRegion(region, size);
    }

    return tlab;
}

template <typename AllocConfigT, typename LockConfigT>
TLAB *RegionAllocator<AllocConfigT, LockConfigT>::CreateTLABInRegion(Region *region, size_t size)
{
    // We don't reuse the same region for different TLABs.
    // Therefore, update the size
    TLAB *tlab = region->CreateTLAB(size);
    ASSERT(tlab != nullptr);
    LOG(DEBUG, ALLOC) << "Found a region " << region << " and create tlab " << tlab << " with memory starts at "
                      << tlab->GetStartAddr() << " and with size " << tlab->GetSize();
    return tlab;
}

template <typename AllocConfigT, typename LockConfigT>
template <bool INCLUDE_CURRENT_REGION>
PandaPriorityQueue<std::pair<uint32_t, Region *>> RegionAllocator<AllocConfigT, LockConfigT>::GetTopGarbageRegions()
{
    PandaPriorityQueue<std::pair<uint32_t, Region *>> queue;
    this->GetSpace()->IterateRegions([&](Region *region) {
        if (region->HasFlag(IS_EDEN) || region->HasFlag(RegionFlag::IS_RESERVED) || region->HasPinnedObjects()) {
            return;
        }
        if constexpr (!INCLUDE_CURRENT_REGION) {
            if (IsInCurrentRegion<true, RegionFlag::IS_OLD>(region)) {
                return;
            }
        }
        auto garbage_bytes = region->GetGarbageBytes();
        queue.push(std::pair<uint32_t, Region *>(garbage_bytes, region));
    });
    return queue;
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE>
PandaVector<Region *> RegionAllocator<AllocConfigT, LockConfigT>::GetAllSpecificRegions()
{
    PandaVector<Region *> vector;
    this->GetSpace()->IterateRegions([&](Region *region) {
        if (region->HasFlag(REGIONS_TYPE)) {
            vector.push_back(region);
        }
    });
    return vector;
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP>
void RegionAllocator<AllocConfigT, LockConfigT>::CompactAllSpecificRegions(const GCObjectVisitor &death_checker,
                                                                           const ObjectVisitorEx &move_handler)
{
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (REGIONS_TYPE_FROM == REGIONS_TYPE_TO) {  // NOLINT(bugprone-suspicious-semicolon)
        // NOTE(aemelenko): Implement it if need to call this method with the same regions type.
        // There is an issue with IterateRegions during creating a new one.
        ASSERT(REGIONS_TYPE_FROM != REGIONS_TYPE_TO);
        ResetCurrentRegion<false, REGIONS_TYPE_TO>();
    }
    this->GetSpace()->IterateRegions([&](Region *region) {
        if (!region->HasFlag(REGIONS_TYPE_FROM)) {
            return;
        }
        CompactSpecificRegion<REGIONS_TYPE_FROM, REGIONS_TYPE_TO, USE_MARKED_BITMAP>(region, death_checker,
                                                                                     move_handler);
    });
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP>
void RegionAllocator<AllocConfigT, LockConfigT>::CompactSeveralSpecificRegions(const PandaVector<Region *> &regions,
                                                                               const GCObjectVisitor &death_checker,
                                                                               const ObjectVisitorEx &move_handler)
{
    for (auto i : regions) {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGIONS_TYPE_FROM == REGIONS_TYPE_TO) {
            [[maybe_unused]] bool founded_region = IsInCurrentRegion<false, REGIONS_TYPE_TO>(i);
            ASSERT(!founded_region);
        }
        CompactSpecificRegion<REGIONS_TYPE_FROM, REGIONS_TYPE_TO, USE_MARKED_BITMAP>(i, death_checker, move_handler);
    }
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP>
void RegionAllocator<AllocConfigT, LockConfigT>::CompactSpecificRegion(Region *region,
                                                                       const GCObjectVisitor &death_checker,
                                                                       const ObjectVisitorEx &move_handler)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (REGIONS_TYPE_FROM == REGIONS_TYPE_TO) {
        // It is bad if we compact one region into itself.
        [[maybe_unused]] bool is_current_region = IsInCurrentRegion<true, REGIONS_TYPE_TO>(region);
        ASSERT(!is_current_region);
    }
    auto create_new_region = [&]() {
        os::memory::LockHolder lock(this->region_lock_);
        Region *region_to = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, REGIONS_TYPE_TO);
        ASSERT(region_to != nullptr);
        return region_to;
    };

    Region *region_to = PopFromRegionQueue<true, REGIONS_TYPE_TO>();
    if (region_to == nullptr) {
        region_to = create_new_region();
    }
    size_t live_bytes = 0;
    // Don't use atomic in this method because we work with not shared region
    auto visitor = [&](ObjectHeader *object) {
        // If we use mark-bitmap then we iterate over alive object, so no need death-checker
        if constexpr (!USE_MARKED_BITMAP) {
            if (death_checker(object) != ObjectStatus::ALIVE_OBJECT) {
                return;
            }
        }
        size_t object_size = GetObjectSize(object);
        size_t aligned_size = AlignUp(object_size, DEFAULT_ALIGNMENT_IN_BYTES);
        void *dst = region_to->template Alloc<false>(aligned_size);
        if (dst == nullptr) {
            region_to->SetLiveBytes(region_to->GetLiveBytes() + live_bytes);
            live_bytes = 0;
            region_to = create_new_region();
            dst = region_to->template Alloc<false>(aligned_size);
        }
        // Don't initialize memory for an object here because we will use memcpy anyway
        ASSERT(dst != nullptr);
        memcpy_s(dst, object_size, object, object_size);
        // need to mark as alive moved object
        ASSERT(region_to->GetLiveBitmap() != nullptr);
        region_to->IncreaseAllocatedObjects();
        region_to->GetLiveBitmap()->Set(dst);
        live_bytes += aligned_size;
        move_handler(object, static_cast<ObjectHeader *>(dst));
    };

    ASSERT(region->HasFlag(REGIONS_TYPE_FROM));

    const std::function<void(ObjectHeader *)> visitor_functor(visitor);
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (USE_MARKED_BITMAP) {
        region->GetMarkBitmap()->IterateOverMarkedChunks(
            [&](void *object_addr) { visitor_functor(static_cast<ObjectHeader *>(object_addr)); });
    } else {  // NOLINT(readability-misleading-indentation)
        region->IterateOverObjects(visitor_functor);
    }
    region_to->SetLiveBytes(region_to->GetLiveBytes() + live_bytes);

    PushToRegionQueue<true, REGIONS_TYPE_TO>(region_to);
}

template <typename AllocConfigT, typename LockConfigT>
void RegionAllocator<AllocConfigT, LockConfigT>::ReserveRegionIfNeeded()
{
    if (reserved_region_ != nullptr) {
        return;
    }
    reserved_region_ = this->GetSpace()->NewRegion(REGION_SIZE, RegionFlag::IS_OLD, RegionFlag::IS_RESERVED);
    ASSERT(reserved_region_ != nullptr);
    reserved_region_->RmvFlag(RegionFlag::IS_OLD);
}

template <typename AllocConfigT, typename LockConfigT>
void RegionAllocator<AllocConfigT, LockConfigT>::ReleaseReservedRegion()
{
    ASSERT(reserved_region_ != nullptr);
    this->GetSpace()->template FreeRegion<RegionSpace::ReleaseRegionsPolicy::NoRelease, OSPagesPolicy::NO_RETURN>(
        reserved_region_);
    reserved_region_ = nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
template <bool USE_MARKED_BITMAP>
void RegionAllocator<AllocConfigT, LockConfigT>::PromoteYoungRegion(Region *region,
                                                                    const GCObjectVisitor &death_checker,
                                                                    const ObjectVisitor &alive_objects_handler)
{
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    // We should create live bitmap here and copy alive object in marked bitmap to it
    region->CreateLiveBitmap();
    region->CloneMarkBitmapToLiveBitmap();
    auto visitor = [&alive_objects_handler, &region](ObjectHeader *object) {
        alive_objects_handler(object);
        region->IncreaseAllocatedObjects();
    };
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (USE_MARKED_BITMAP) {
        region->GetMarkBitmap()->IterateOverMarkedChunks(
            [&visitor](void *object_addr) { visitor(static_cast<ObjectHeader *>(object_addr)); });
    } else {  // NOLINT(readability-misleading-indentation)
        auto live_check_visitor = [&visitor, &death_checker](ObjectHeader *object) {
            if (death_checker(object) == ObjectStatus::ALIVE_OBJECT) {
                visitor(object);
            }
        };
        region->IterateOverObjects(live_check_visitor);
    }
    // We set not actual value here but we will update it later
    region->SetLiveBytes(region->GetAllocatedBytes());
    this->GetSpace()->PromoteYoungRegion(region);
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE>
void RegionAllocator<AllocConfigT, LockConfigT>::ResetAllSpecificRegions()
{
    ResetCurrentRegion<false, REGIONS_TYPE>();
    this->GetSpace()->IterateRegions([&](Region *region) {
        if (!region->HasFlag(REGIONS_TYPE)) {
            return;
        }
        this->GetSpace()->template FreeRegion<RegionSpace::ReleaseRegionsPolicy::NoRelease>(region);
    });
    if constexpr (REGIONS_TYPE == RegionFlag::IS_EDEN) {
        retained_tlabs_.clear();
    }
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE, RegionSpace::ReleaseRegionsPolicy REGIONS_RELEASE_POLICY,
          OSPagesPolicy OS_PAGES_POLICY, bool NEED_LOCK, typename Container>
void RegionAllocator<AllocConfigT, LockConfigT>::ResetSeveralSpecificRegions(const Container &regions)
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(this->region_lock_);
    ASSERT(REGIONS_TYPE != RegionFlag::IS_EDEN);
    ASSERT((REGIONS_TYPE != RegionFlag::IS_EDEN) || (retained_tlabs_.empty()));
    for (Region *region : regions) {
        ASSERT(!(IsInCurrentRegion<false, REGIONS_TYPE>(region)));
        ASSERT(region->HasFlag(REGIONS_TYPE));
        this->GetSpace()->template FreeRegion<REGIONS_RELEASE_POLICY, OS_PAGES_POLICY>(region);
    }
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::RegionNonmovableAllocator(
    MemStatsType *mem_stats, GenerationalSpaces *spaces, SpaceType space_type, size_t init_space_size, bool extend)
    : RegionAllocatorBase<LockConfigT>(mem_stats, spaces, space_type, ObjectAllocator::GetAllocatorType(),
                                       init_space_size, extend, REGION_SIZE, 0),
      object_allocator_(mem_stats, space_type)
{
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::RegionNonmovableAllocator(
    MemStatsType *mem_stats, GenerationalSpaces *spaces, SpaceType space_type, RegionPool *shared_region_pool)
    : RegionAllocatorBase<LockConfigT>(mem_stats, spaces, space_type, ObjectAllocator::GetAllocatorType(),
                                       shared_region_pool, 0),
      object_allocator_(mem_stats, space_type)
{
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void *RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::Alloc(size_t size, Alignment align)
{
    ASSERT(GetAlignmentInBytes(align) % GetAlignmentInBytes(DEFAULT_ALIGNMENT) == 0);
    size_t align_size = AlignUp(size, GetAlignmentInBytes(align));
    ASSERT(align_size <= ObjectAllocator::GetMaxSize());

    void *mem = object_allocator_.Alloc(align_size);
    if (UNLIKELY(mem == nullptr)) {
        mem = NewRegionAndRetryAlloc(size, align);
        if (UNLIKELY(mem == nullptr)) {
            return nullptr;
        }
    }
    auto live_bitmap = this->GetRegion(reinterpret_cast<ObjectHeader *>(mem))->GetLiveBitmap();
    ASSERT(live_bitmap != nullptr);
    live_bitmap->AtomicTestAndSet(mem);
    return mem;
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::Free(void *mem)
{
    this->GetRegion(reinterpret_cast<ObjectHeader *>(mem))->GetLiveBitmap()->AtomicTestAndClear(mem);

    object_allocator_.Free(mem);
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::Collect(
    const GCObjectVisitor &death_checker)
{
    os::memory::LockHolder lock(this->region_lock_);
    object_allocator_.Collect(death_checker);
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::VisitAndRemoveFreeRegions(
    const RegionsVisitor &region_visitor)
{
    os::memory::LockHolder lock(this->region_lock_);
    // Add free region into vector to not do extra work with region_visitor
    // inside object_allocator_.
    PandaVector<Region *> free_regions;

    object_allocator_.VisitAndRemoveFreePools([&free_regions](void *mem, [[maybe_unused]] size_t size) {
        auto *region = AddrToRegion(mem);
        ASSERT(ToUintPtr(mem) + size == region->End());
        // We don't remove this region here, because don't want to do some extra work with visitor here.
        free_regions.push_back(region);
    });

    if (!free_regions.empty()) {
        region_visitor(free_regions);

        for (auto i : free_regions) {
            this->GetSpace()->FreeRegion(i);
        }
    }
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void *RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::NewRegionAndRetryAlloc(size_t object_size,
                                                                                                    Alignment align)
{
    os::memory::LockHolder lock(this->region_lock_);
    size_t pool_head_size = AlignUp(Region::HeadSize(), ObjectAllocator::PoolAlign());
    ASSERT(AlignUp(pool_head_size + object_size, REGION_SIZE) == REGION_SIZE);
    while (true) {
        Region *region = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, RegionFlag::IS_NONMOVABLE);
        if (UNLIKELY(region == nullptr)) {
            return nullptr;
        }
        ASSERT(region->GetLiveBitmap() != nullptr);
        uintptr_t aligned_pool = ToUintPtr(region) + pool_head_size;
        bool added_memory_pool = object_allocator_.AddMemoryPool(ToVoidPtr(aligned_pool), REGION_SIZE - pool_head_size);
        ASSERT(added_memory_pool);
        if (UNLIKELY(!added_memory_pool)) {
            LOG(FATAL, ALLOC) << "ObjectAllocator: couldn't add memory pool to allocator";
        }
        void *mem = object_allocator_.Alloc(object_size, align);
        if (LIKELY(mem != nullptr)) {
            return mem;
        }
    }
    return nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
RegionHumongousAllocator<AllocConfigT, LockConfigT>::RegionHumongousAllocator(MemStatsType *mem_stats,
                                                                              GenerationalSpaces *spaces,
                                                                              SpaceType space_type)
    : RegionAllocatorBase<LockConfigT>(mem_stats, spaces, space_type, AllocatorType::REGION_ALLOCATOR, 0, true,
                                       REGION_SIZE, 0)
{
}

template <typename AllocConfigT, typename LockConfigT>
template <bool UPDATE_MEMSTATS>
void *RegionHumongousAllocator<AllocConfigT, LockConfigT>::Alloc(size_t size, Alignment align)
{
    ASSERT(GetAlignmentInBytes(align) % GetAlignmentInBytes(DEFAULT_ALIGNMENT) == 0);
    size_t align_size = AlignUp(size, GetAlignmentInBytes(align));
    Region *region = nullptr;
    void *mem = nullptr;
    // allocate a seprate large region for object
    {
        os::memory::LockHolder lock(this->region_lock_);
        region = this->template CreateAndSetUpNewRegion<AllocConfigT, OSPagesAllocPolicy::ZEROED_MEMORY>(
            Region::RegionSize(align_size, REGION_SIZE), IS_OLD, IS_LARGE_OBJECT);
        if (LIKELY(region != nullptr)) {
            mem = region->Alloc<false>(align_size);
            ASSERT(mem != nullptr);
            ASSERT(region->GetLiveBitmap() != nullptr);
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (UPDATE_MEMSTATS) {
                AllocConfigT::OnAlloc(region->Size(), this->space_type_, this->mem_stats_);
                // We don't set up memory here because the requested memory should
                // be zeroed
            }
            // Do it after memory init because we can reach this memory after setting live bitmap
            region->GetLiveBitmap()->AtomicTestAndSet(mem);
        }
    }
    return mem;
}

template <typename AllocConfigT, typename LockConfigT>
void RegionHumongousAllocator<AllocConfigT, LockConfigT>::CollectAndRemoveFreeRegions(
    const RegionsVisitor &region_visitor, const GCObjectVisitor &death_checker)
{
    // Add free region into vector to not do extra work with region_visitor during region iteration
    PandaVector<Region *> free_regions;

    {
        os::memory::LockHolder lock(this->region_lock_);
        this->GetSpace()->IterateRegions([&](Region *region) {
            this->Collect(region, death_checker);
            if (region->HasFlag(IS_FREE)) {
                free_regions.push_back(region);
            }
        });
    }

    if (!free_regions.empty()) {
        region_visitor(free_regions);

        for (auto i : free_regions) {
            os::memory::LockHolder lock(this->region_lock_);
            ResetRegion(i);
        }
    }
}

template <typename AllocConfigT, typename LockConfigT>
void RegionHumongousAllocator<AllocConfigT, LockConfigT>::Collect(Region *region, const GCObjectVisitor &death_checker)
{
    ASSERT(region->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ObjectHeader *object_to_proceed = nullptr;
    object_to_proceed = region->GetLargeObject();
    if (death_checker(object_to_proceed) == ObjectStatus::DEAD_OBJECT) {
        region->AddFlag(RegionFlag::IS_FREE);
    }
}

template <typename AllocConfigT, typename LockConfigT>
void RegionHumongousAllocator<AllocConfigT, LockConfigT>::ResetRegion(Region *region)
{
    ASSERT(region->HasFlag(RegionFlag::IS_FREE));
    region->RmvFlag(RegionFlag::IS_FREE);
    this->GetSpace()->FreeRegion(region);
}

template <typename AllocConfigT, typename LockConfigT>
using RegionRunslotsAllocator = RegionNonmovableAllocator<AllocConfigT, LockConfigT, RunSlotsAllocator<AllocConfigT>>;

template <typename AllocConfigT, typename LockConfigT>
using RegionFreeListAllocator = RegionNonmovableAllocator<AllocConfigT, LockConfigT, FreeListAllocator<AllocConfigT>>;

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_REGION_ALLOCATOR_INL_H
