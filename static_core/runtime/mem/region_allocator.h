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
#ifndef PANDA_RUNTIME_MEM_REGION_ALLOCATOR_H
#define PANDA_RUNTIME_MEM_REGION_ALLOCATOR_H

#include <atomic>
#include <cstdint>

#include "runtime/mem/region_space.h"

namespace panda {
class ManagedThread;
struct GCTask;
}  // namespace panda

namespace panda::mem {

class RegionAllocatorLockConfig {
public:
    using CommonLock = os::memory::Mutex;
    using DummyLock = os::memory::DummyLock;
};

using RegionsVisitor = std::function<void(PandaVector<Region *> &vector)>;

/// Return the region which corresponds to the start of the object.
static inline Region *ObjectToRegion(const ObjectHeader *object)
{
    auto *region = reinterpret_cast<Region *>(((ToUintPtr(object)) & ~DEFAULT_REGION_MASK));
    ASSERT(ToUintPtr(PoolManager::GetMmapMemPool()->GetStartAddrPoolForAddr(object)) == ToUintPtr(region));
    // Getting region by object is a bit operation and TSAN doesn't
    // sees the relation between region creation and region access.
    // This annotation suggests TSAN that this code always executes after
    // the region gets created.
    // See the corresponding annotation in RegionAllocatorBase::CreateAndSetUpNewRegion
    TSAN_ANNOTATE_HAPPENS_AFTER(region);
    return region;
}

static inline bool IsSameRegion(const void *o1, const void *o2, size_t region_size_bits)
{
    return ((ToUintPtr(o1) ^ ToUintPtr(o2)) >> region_size_bits == 0);
}

/// Return the region which corresponds to the address.
static inline Region *AddrToRegion(const void *addr)
{
    auto region_addr = PoolManager::GetMmapMemPool()->GetStartAddrPoolForAddr(addr);
    return static_cast<Region *>(region_addr);
}

template <typename LockConfigT>
class RegionAllocatorBase {
public:
    NO_MOVE_SEMANTIC(RegionAllocatorBase);
    NO_COPY_SEMANTIC(RegionAllocatorBase);

    explicit RegionAllocatorBase(MemStatsType *mem_stats, GenerationalSpaces *spaces, SpaceType space_type,
                                 AllocatorType allocator_type, size_t init_space_size, bool extend, size_t region_size,
                                 size_t empty_tenured_regions_max_count);
    explicit RegionAllocatorBase(MemStatsType *mem_stats, GenerationalSpaces *spaces, SpaceType space_type,
                                 AllocatorType allocator_type, RegionPool *shared_region_pool,
                                 size_t empty_tenured_regions_max_count);

    virtual ~RegionAllocatorBase()
    {
        ClearRegionsPool();
    }

    Region *GetRegion(const ObjectHeader *object) const
    {
        return region_space_.GetRegion(object);
    }

    RegionSpace *GetSpace()
    {
        return &region_space_;
    }

    const RegionSpace *GetSpace() const
    {
        return &region_space_;
    }

    PandaVector<Region *> GetAllRegions();

    template <RegionFlag REGION_TYPE, OSPagesPolicy OS_PAGES_POLICY>
    void ReleaseEmptyRegions()
    {
        this->GetSpace()->template ReleaseEmptyRegions<REGION_TYPE, OS_PAGES_POLICY>();
    }

protected:
    void ClearRegionsPool()
    {
        region_space_.FreeAllRegions();

        if (init_block_.GetMem() != nullptr) {
            spaces_->FreeSharedPool(init_block_.GetMem(), init_block_.GetSize());
            init_block_ = NULLPOOL;
        }
    }

    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    Region *AllocRegion(size_t region_size, RegionFlag eden_or_old_or_nonmovable, RegionFlag properties)
    {
        return region_space_.NewRegion(region_size, eden_or_old_or_nonmovable, properties, OS_ALLOC_POLICY);
    }

    SpaceType GetSpaceType() const
    {
        return space_type_;
    }

    template <typename AllocConfigT, OSPagesAllocPolicy OS_ALLOC_POLICY = OSPagesAllocPolicy::NO_POLICY>
    Region *CreateAndSetUpNewRegion(size_t region_size, RegionFlag region_type, RegionFlag properties = IS_UNUSED)
        REQUIRES(region_lock_);

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    LockConfigT region_lock_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    MemStatsType *mem_stats_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    SpaceType space_type_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    GenerationalSpaces *spaces_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    RegionPool region_pool_;  // self created pool, only used by this allocator
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    RegionSpace region_space_;  // the target region space used by this allocator
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Pool init_block_;  // the initial memory block for region allocation
};

/// @brief A region-based bump-pointer allocator.
template <typename AllocConfigT, typename LockConfigT = RegionAllocatorLockConfig::CommonLock>
class RegionAllocator final : public RegionAllocatorBase<LockConfigT> {
public:
    static constexpr bool USE_PARTIAL_TLAB = true;
    static constexpr size_t REGION_SIZE = DEFAULT_REGION_SIZE;

    NO_MOVE_SEMANTIC(RegionAllocator);
    NO_COPY_SEMANTIC(RegionAllocator);

    /**
     * @brief Create new region allocator
     * @param mem_stats - memory statistics
     * @param space_type - space type
     * @param init_space_size - initial continuous space size, 0 means no need for initial space
     * @param extend - true means that will allocate more regions from mmap pool if initial space is not enough
     */
    explicit RegionAllocator(MemStatsType *mem_stats, GenerationalSpaces *spaces,
                             SpaceType space_type = SpaceType::SPACE_TYPE_OBJECT, size_t init_space_size = 0,
                             bool extend = true, size_t empty_tenured_regions_max_count = 0);

    /**
     * @brief Create new region allocator with shared region pool specified
     * @param mem_stats - memory statistics
     * @param space_type - space type
     * @param shared_region_pool - a shared region pool that can be reused by multi-spaces
     */
    explicit RegionAllocator(MemStatsType *mem_stats, GenerationalSpaces *spaces, SpaceType space_type,
                             RegionPool *shared_region_pool, size_t empty_tenured_regions_max_count = 0);

    ~RegionAllocator() override = default;

    template <RegionFlag REGION_TYPE = RegionFlag::IS_EDEN, bool UPDATE_MEMSTATS = true>
    void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT);

    template <typename T>
    T *AllocArray(size_t arr_length)
    {
        return static_cast<T *>(Alloc(sizeof(T) * arr_length));
    }

    void Free([[maybe_unused]] void *mem) {}

    void PinObject(ObjectHeader *object);

    void UnpinObject(ObjectHeader *object);

    /**
     * @brief Create a TLAB of the specified size
     * @param size - required size of tlab
     * @return newly allocated TLAB, TLAB is set to Empty is allocation failed.
     */
    TLAB *CreateTLAB(size_t size);

    /**
     * @brief Create a TLAB in a new region. TLAB will occupy the whole region.
     * @return newly allocated TLAB, TLAB is set to Empty is allocation failed.
     */
    TLAB *CreateRegionSizeTLAB();

    /**
     * @brief Iterates over all objects allocated by this allocator.
     * @param visitor - function pointer or functor
     */
    template <typename ObjectVisitor>
    void IterateOverObjects(const ObjectVisitor &visitor)
    {
        this->GetSpace()->IterateRegions([&](Region *region) { region->IterateOverObjects(visitor); });
    }

    template <typename ObjectVisitor>
    void IterateOverObjectsInRange(const ObjectVisitor &visitor, void *begin, void *end)
    {
        this->GetSpace()->IterateRegions([&](Region *region) {
            if (region->Intersect(ToUintPtr(begin), ToUintPtr(end))) {
                region->IterateOverObjects([&visitor, begin, end](ObjectHeader *obj) {
                    if (ToUintPtr(begin) <= ToUintPtr(obj) && ToUintPtr(obj) < ToUintPtr(end)) {
                        visitor(obj);
                    }
                });
            }
        });
    }

    template <bool INCLUDE_CURRENT_REGION>
    PandaPriorityQueue<std::pair<uint32_t, Region *>> GetTopGarbageRegions();

    /**
     * Return a vector of all regions with the specific type.
     * @tparam regions_type - type of regions needed to proceed.
     * @return vector of all regions with the /param regions_type type
     */
    template <RegionFlag REGIONS_TYPE>
    PandaVector<Region *> GetAllSpecificRegions();

    /**
     * Iterate over all regions with type /param regions_type_from
     * and move all alive objects to the regions with type /param regions_type_to.
     * NOTE: /param regions_type_from and /param regions_type_to can't be equal.
     * @tparam regions_type_from - type of regions needed to proceed.
     * @tparam regions_type_to - type of regions to which we want to move all alive objects.
     * @tparam use_marked_bitmap - if we need to use marked_bitmap from the regions or not.
     * @param death_checker - checker what will return objects status for iterated object.
     * @param move_handler - called for every moved object
     *  can be used as a simple visitor if we enable /param use_marked_bitmap
     */
    template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP = false>
    void CompactAllSpecificRegions(const GCObjectVisitor &death_checker, const ObjectVisitorEx &move_handler);

    template <RegionFlag REGION_TYPE>
    void ClearCurrentRegion()
    {
        ResetCurrentRegion<false, REGION_TYPE>();
    }

    /**
     * Iterate over specific regions from vector
     * and move all alive objects to the regions with type /param regions_type_to.
     * @tparam regions_type_from - type of regions needed to proceed.
     * @tparam regions_type_to - type of regions to which we want to move all alive objects.
     * @tparam use_marked_bitmap - if we need to use marked_bitmap from the regions or not.
     * @param regions - vector of regions needed to proceed.
     * @param death_checker - checker what will return objects status for iterated object.
     * @param move_handler - called for every moved object
     *  can be used as a simple visitor if we enable /param use_marked_bitmap
     */
    template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP = false>
    void CompactSeveralSpecificRegions(const PandaVector<Region *> &regions, const GCObjectVisitor &death_checker,
                                       const ObjectVisitorEx &move_handler);

    /**
     * Iterate over specific region
     * and move all alive objects to the regions with type /param regions_type_to.
     * @tparam regions_type_from - type of regions needed to proceed.
     * @tparam regions_type_to - type of regions to which we want to move all alive objects.
     * @tparam use_marked_bitmap - if we need to use marked_bitmap from the regions or not.
     * @param region - region needed to proceed.
     * @param death_checker - checker what will return objects status for iterated object.
     * @param move_handler - called for every moved object
     *  can be used as a simple visitor if we enable /param use_marked_bitmap
     */
    template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP = false>
    void CompactSpecificRegion(Region *regions, const GCObjectVisitor &death_checker,
                               const ObjectVisitorEx &move_handler);

    template <bool USE_MARKED_BITMAP = false>
    void PromoteYoungRegion(Region *region, const GCObjectVisitor &death_checker,
                            const ObjectVisitor &alive_objects_handler);

    /**
     * Reset all regions with type /param regions_type.
     * @tparam regions_type - type of regions needed to proceed.
     */
    template <RegionFlag REGIONS_TYPE>
    void ResetAllSpecificRegions();

    /**
     * Reset regions from vector.
     * @tparam REGIONS_TYPE - type of regions needed to proceed.
     * @tparam REGIONS_RELEASE_POLICY - region need to be placed in the free queue or returned to mempool.
     * @tparam OS_PAGES_POLICY - if we need to return region pages to OS or not.
     * @tparam NEED_LOCK - if we need to take region lock or not. Use it if we allocate regions in parallel
     * @param regions - vector of regions needed to proceed.
     * @tparam Container - region's container type
     */
    template <RegionFlag REGIONS_TYPE, RegionSpace::ReleaseRegionsPolicy REGIONS_RELEASE_POLICY,
              OSPagesPolicy OS_PAGES_POLICY, bool NEED_LOCK, typename Container>
    void ResetSeveralSpecificRegions(const Container &regions);

    /// Reserve one region if no reserved region
    void ReserveRegionIfNeeded();

    /// Release reserved region to free space
    void ReleaseReservedRegion();

    void VisitAndRemoveAllPools([[maybe_unused]] const MemVisitor &mem_visitor)
    {
        this->ClearRegionsPool();
    }

    constexpr static size_t GetMaxRegularObjectSize()
    {
        return REGION_SIZE - AlignUp(sizeof(Region), DEFAULT_ALIGNMENT_IN_BYTES);
    }

    bool ContainObject(const ObjectHeader *object) const
    {
        return this->GetSpace()->ContainObject(object);
    }

    bool IsLive(const ObjectHeader *object) const
    {
        return this->GetSpace()->IsLive(object);
    }

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::REGION_ALLOCATOR;
    }

    void SetDesiredEdenLength(size_t eden_length)
    {
        this->GetSpace()->SetDesiredEdenLength(eden_length);
    }

private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    template <bool atomic = true, RegionFlag REGION_TYPE>
    Region *GetCurrentRegion()
    {
        Region **cur_region = GetCurrentRegionPointerUnsafe<REGION_TYPE>();
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (atomic) {
            // Atomic with relaxed order reason: data race with cur_region with no synchronization or ordering
            // constraints imposed on other reads or writes
            return reinterpret_cast<std::atomic<Region *> *>(cur_region)->load(std::memory_order_relaxed);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        }
        return *cur_region;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <bool atomic = true, RegionFlag REGION_TYPE>
    void SetCurrentRegion(Region *region)
    {
        Region **cur_region = GetCurrentRegionPointerUnsafe<REGION_TYPE>();
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (atomic) {
            // Atomic with relaxed order reason: data race with cur_region with no synchronization or ordering
            // constraints imposed on other reads or writes
            reinterpret_cast<std::atomic<Region *> *>(cur_region)->store(region, std::memory_order_relaxed);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            *cur_region = region;
        }
    }

    template <RegionFlag REGION_TYPE>
    Region **GetCurrentRegionPointerUnsafe()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
            return &eden_current_region_;
        }
        UNREACHABLE();
        return nullptr;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <bool atomic = true, RegionFlag REGION_TYPE>
    void ResetCurrentRegion()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
            SetCurrentRegion<atomic, REGION_TYPE>(&full_region_);
            return;
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_OLD) {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                os::memory::LockHolder lock(*GetQueueLock<REGION_TYPE>());
                GetRegionQueuePointer<REGION_TYPE>()->clear();
                return;
            }
            GetRegionQueuePointer<REGION_TYPE>()->clear();
            return;
        }
        UNREACHABLE();
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <bool atomic = true, RegionFlag REGION_TYPE>
    bool IsInCurrentRegion(Region *region)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
            return GetCurrentRegion<atomic, REGION_TYPE>() == region;
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_OLD) {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (atomic) {
                os::memory::LockHolder lock(*GetQueueLock<REGION_TYPE>());
                for (auto i : *GetRegionQueuePointer<REGION_TYPE>()) {
                    if (i == region) {
                        return true;
                    }
                }
                return false;
            }
            for (auto i : *GetRegionQueuePointer<REGION_TYPE>()) {
                if (i == region) {
                    return true;
                }
            }
            return false;
        }
        UNREACHABLE();
        return false;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <bool atomic = true, RegionFlag REGION_TYPE>
    Region *PopFromRegionQueue()
    {
        PandaVector<Region *> *region_queue = GetRegionQueuePointer<REGION_TYPE>();
        Region *region = nullptr;
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (atomic) {
            os::memory::LockHolder lock(*GetQueueLock<REGION_TYPE>());
            if (!region_queue->empty()) {
                region = region_queue->back();
                region_queue->pop_back();
            }
            return region;
            // NOLINTNEXTLINE(readability-misleading-indentation)
        }
        if (!region_queue->empty()) {
            region = region_queue->back();
            region_queue->pop_back();
        }
        return region;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <bool atomic = true, RegionFlag REGION_TYPE>
    void PushToRegionQueue(Region *region)
    {
        PandaVector<Region *> *region_queue = GetRegionQueuePointer<REGION_TYPE>();
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (atomic) {
            os::memory::LockHolder lock(*GetQueueLock<REGION_TYPE>());
            region_queue->push_back(region);
            return;
            // NOLINTNEXTLINE(readability-misleading-indentation)
        }
        region_queue->push_back(region);
    }

    template <RegionFlag REGION_TYPE>
    os::memory::Mutex *GetQueueLock()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_OLD) {
            return &old_queue_lock_;
        }
        UNREACHABLE();
        return nullptr;
    }

    template <RegionFlag REGION_TYPE>
    PandaVector<Region *> *GetRegionQueuePointer()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_OLD) {
            return &old_region_queue_;
        }
        UNREACHABLE();
        return nullptr;
    }

    template <RegionFlag REGION_TYPE>
    void *AllocRegular(size_t align_size);
    TLAB *CreateTLABInRegion(Region *region, size_t size);

    Region full_region_;
    Region *eden_current_region_;
    Region *reserved_region_ = nullptr;
    os::memory::Mutex old_queue_lock_;
    PandaVector<Region *> old_region_queue_;
    // To store partially used Regions that can be reused later.
    panda::PandaMultiMap<size_t, Region *, std::greater<size_t>> retained_tlabs_;
    friend class test::RegionAllocatorTest;
};

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
class RegionNonmovableAllocator final : public RegionAllocatorBase<LockConfigT> {
public:
    static constexpr size_t REGION_SIZE = DEFAULT_REGION_SIZE;

    NO_MOVE_SEMANTIC(RegionNonmovableAllocator);
    NO_COPY_SEMANTIC(RegionNonmovableAllocator);

    explicit RegionNonmovableAllocator(MemStatsType *mem_stats, GenerationalSpaces *spaces, SpaceType space_type,
                                       size_t init_space_size = 0, bool extend = true);
    explicit RegionNonmovableAllocator(MemStatsType *mem_stats, GenerationalSpaces *spaces, SpaceType space_type,
                                       RegionPool *shared_region_pool);

    ~RegionNonmovableAllocator() override = default;

    void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT);

    void Free(void *mem);

    void Collect(const GCObjectVisitor &death_checker);

    template <typename ObjectVisitor>
    void IterateOverObjects(const ObjectVisitor &obj_visitor)
    {
        object_allocator_.IterateOverObjects(obj_visitor);
    }

    template <typename MemVisitor>
    void IterateOverObjectsInRange(const MemVisitor &mem_visitor, void *begin, void *end)
    {
        object_allocator_.IterateOverObjectsInRange(mem_visitor, begin, end);
    }

    void VisitAndRemoveAllPools([[maybe_unused]] const MemVisitor &mem_visitor)
    {
        object_allocator_.VisitAndRemoveAllPools([this](void *mem, [[maybe_unused]] size_t size) {
            auto *region = AddrToRegion(mem);
            ASSERT(ToUintPtr(mem) + size == region->End());
            this->GetSpace()->FreeRegion(region);
        });
    }

    void VisitAndRemoveFreeRegions(const RegionsVisitor &region_visitor);

    constexpr static size_t GetMaxSize()
    {
        // NOTE(yxr) : get accurate max payload size in a freelist pool
        return std::min(ObjectAllocator::GetMaxSize(), static_cast<size_t>(REGION_SIZE - 1_KB));
    }

    bool ContainObject(const ObjectHeader *object) const
    {
        return object_allocator_.ContainObject(object);
    }

    bool IsLive(const ObjectHeader *object) const
    {
        ASSERT(this->GetRegion(object)->GetLiveBitmap() != nullptr);
        return this->GetRegion(object)->GetLiveBitmap()->AtomicTest(const_cast<ObjectHeader *>(object));
    }

private:
    void *NewRegionAndRetryAlloc(size_t object_size, Alignment align);

    mutable ObjectAllocator object_allocator_;
};

/// @brief A region-based humongous allocator.
template <typename AllocConfigT, typename LockConfigT = RegionAllocatorLockConfig::CommonLock>
class RegionHumongousAllocator final : public RegionAllocatorBase<LockConfigT> {
public:
    static constexpr size_t REGION_SIZE = DEFAULT_REGION_SIZE;

    NO_MOVE_SEMANTIC(RegionHumongousAllocator);
    NO_COPY_SEMANTIC(RegionHumongousAllocator);

    /**
     * @brief Create new humongous region allocator
     * @param mem_stats - memory statistics
     * @param space_type - space type
     */
    explicit RegionHumongousAllocator(MemStatsType *mem_stats, GenerationalSpaces *spaces, SpaceType space_type);

    ~RegionHumongousAllocator() override = default;

    template <bool UPDATE_MEMSTATS = true>
    void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT);

    template <typename T>
    T *AllocArray(size_t arr_length)
    {
        return static_cast<T *>(Alloc(sizeof(T) * arr_length));
    }

    void Free([[maybe_unused]] void *mem) {}

    void CollectAndRemoveFreeRegions(const RegionsVisitor &region_visitor, const GCObjectVisitor &death_checker);

    /**
     * @brief Iterates over all objects allocated by this allocator.
     * @param visitor - function pointer or functor
     */
    template <typename ObjectVisitor>
    void IterateOverObjects(const ObjectVisitor &visitor)
    {
        this->GetSpace()->IterateRegions([&](Region *region) { region->IterateOverObjects(visitor); });
    }

    template <typename ObjectVisitor>
    void IterateOverObjectsInRange(const ObjectVisitor &visitor, void *begin, void *end)
    {
        this->GetSpace()->IterateRegions([&](Region *region) {
            if (region->Intersect(ToUintPtr(begin), ToUintPtr(end))) {
                region->IterateOverObjects([&visitor, begin, end](ObjectHeader *obj) {
                    if (ToUintPtr(begin) <= ToUintPtr(obj) && ToUintPtr(obj) < ToUintPtr(end)) {
                        visitor(obj);
                    }
                });
            }
        });
    }

    void VisitAndRemoveAllPools([[maybe_unused]] const MemVisitor &mem_visitor)
    {
        this->ClearRegionsPool();
    }

    bool ContainObject(const ObjectHeader *object) const
    {
        return this->GetSpace()->template ContainObject<true>(object);
    }

    bool IsLive(const ObjectHeader *object) const
    {
        return this->GetSpace()->template IsLive<true>(object);
    }

private:
    void ResetRegion(Region *region);
    void Collect(Region *region, const GCObjectVisitor &death_checker);

    // If we change this constant, we will increase fragmentation dramatically
    static_assert(REGION_SIZE / PANDA_POOL_ALIGNMENT_IN_BYTES == 1);
    friend class test::RegionAllocatorTest;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_REGION_ALLOCATOR_H
