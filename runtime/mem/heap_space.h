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

#ifndef PANDA_RUNTIME_MEM_HEAP_SPACE_H
#define PANDA_RUNTIME_MEM_HEAP_SPACE_H

#include "libpandabase/mem/mem_pool.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/macros.h"

#include <optional>

namespace panda::mem {

namespace test {
class HeapSpaceTest;
class MemStatsGenGCTest;
template <typename ObjectAllocator, bool RegularSpace>
class RegionAllocatorTestBase;
class RegionAllocatorTest;
class RegionNonmovableObjectAllocatorTest;
class RegionGarbageChoosingTest;
class RemSetTest;
}  // namespace test

// Object allocation flow:
//                                                     --------+
//   +------+           +-----------+       +------------+     |
//   |  GC  | <----+    |  Runtime  |  ...  |  Compiler  |     |
//   +------+      |    +-----------+   |   +------------+     |
//                 |          |     +---+         |            |
//                 +-----+    |     |   +---------+            | Standard flow for object allocation
//                       |    |     |   |                      |
//                       v    v     v   v                      |
//                    +-------------------+                    |
//                    |  ObjectAllocator  |                    |
//                    +-------------------+            --------+
//                       ^      |
//                       |      | (if need new pool)
//  +--------------------+      |
//  |                           v
//  | if need trigger GC +-------------+
//  + <------------------|  HeapSpace  | <---- Here we compute heap limits after GC and make the allocation decision
//  |      NULLPOOL      +-------------+
//  |                           | (if can alloc)
//  |                           v
//  |        Pool        +-------------+
//  +---------<----------| PoolManager |
//                       +-------------+
/**
 * \brief Class for description of object spaces and limits (minimum, maximum, current sizes).
 * Pools for object spaces is allocated via this class. Also this class cooperate with PoolManager for info getting
 * about object heap.
 * Class is some virtual layer between PoolManager and Object allocators and GC for object pools allocations.
 * HeapSpace is used for non-generational-based GC and as base class for GenerationalSpaces class
 */
class HeapSpace {
public:
    explicit HeapSpace() = default;
    NO_COPY_SEMANTIC(HeapSpace);
    NO_MOVE_SEMANTIC(HeapSpace);
    virtual ~HeapSpace() = default;

    /**
     * \brief Heap space inilialization
     * @param initial_size initial (also minimum) heap space size
     * @param max_size maximum heap space size
     * @param min_free_percentage minimum possible percentage of free memory in this heap space, use for heap increasing
     * computation
     * @param max_free_percentage maximum possible percentage of free memory in this heap space, use for heap reducing
     * computation
     */
    void Initialize(size_t initial_size, size_t max_size, uint32_t min_free_percentage, uint32_t max_free_percentage);

    /**
     * \brief Compute new size of heap space
     */
    virtual void ComputeNewSize();

    /**
     * \brief Get current used heap size
     */
    virtual size_t GetHeapSize() const;

    /**
     * \brief Try to allocate pool via PoolManager
     */
    [[nodiscard]] virtual Pool TryAllocPool(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                            void *allocator_ptr);

    /**
     * \brief Try to allocate arena via PoolManager
     */
    [[nodiscard]] virtual Arena *TryAllocArena(size_t arena_size, SpaceType space_type, AllocatorType allocator_type,
                                               void *allocator_ptr);

    /**
     * \brief Free pool via PoolManager
     */
    void FreePool(void *pool_mem, size_t pool_size);

    /**
     * \brief Free arena via PoolManager
     */
    void FreeArena(Arena *arena);

    double GetMinFreePercentage() const
    {
        return min_free_percentage_;
    }

    double GetMaxFreePercentage() const
    {
        return max_free_percentage_;
    }

    void SetIsWorkGC(bool value)
    {
        is_work_gc_ = value;
    }

protected:
    class ObjectMemorySpace {
    public:
        explicit ObjectMemorySpace() = default;
        DEFAULT_COPY_SEMANTIC(ObjectMemorySpace);
        DEFAULT_NOEXCEPT_MOVE_SEMANTIC(ObjectMemorySpace);
        ~ObjectMemorySpace() = default;

        void Initialize(size_t initial_size, size_t max_size);

        size_t GetCurrentSize() const
        {
            return current_size_;
        }

        size_t GetMaxSize() const
        {
            return max_size_;
        }

        size_t GetCurrentNonOccupiedSize() const
        {
            return max_size_ - current_size_;
        }

        void UseFullSpace()
        {
            current_size_ = max_size_;
        }

        /**
         * \brief Compute new size of heap space
         */
        void ComputeNewSize(size_t free_bytes, double min_free_percentage, double max_free_percentage);

        /**
         * \brief Increase current heap space
         * @param bytes bytes count for heap space increasing
         */
        void IncreaseBy(uint64_t bytes);

        /**
         * \brief Reduce current heap space
         * @param bytes bytes count for heap space reducing
         */
        void ReduceBy(size_t bytes);

        // If we have too big pool for allocation then after space increasing we can have not memory for this pool,
        // so we save pool size and increase heap space to allocate such pool
        size_t saved_pool_size {0};  // NOLINT(misc-non-private-member-variables-in-classes)

    private:
        // min_size_ <= current_size_ <= max_size_
        size_t current_size_ {0};
        size_t min_size_ {0};
        size_t max_size_ {0};
    };

    // For avoid zero division
    static constexpr uint32_t MAX_FREE_PERCENTAGE = 99;

    void InitializePercentages(uint32_t min_free_percentage, uint32_t max_free_percentage);

    [[nodiscard]] Pool TryAllocPoolBase(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                        void *allocator_ptr, size_t current_free_bytes_in_space,
                                        ObjectMemorySpace *mem_space);

    [[nodiscard]] Arena *TryAllocArenaBase(size_t arena_size, SpaceType space_type, AllocatorType allocator_type,
                                           void *allocator_ptr, size_t current_free_bytes_in_space,
                                           ObjectMemorySpace *mem_space);

    /**
     * \brief Check that we can to allocate requested size into target space
     *
     * @param pool_size size of memory chunk for allocation
     * @param current_free_bytes_in_space current freebytes count into target space
     * @param mem_space target space in which there will be allocation
     *
     * @return std::optional value. If result has value then it contains bytes count for heap increasing (need during
     * GC, for example), else need to trigger GC
     */
    std::optional<size_t> WillAlloc(size_t pool_size, size_t current_free_bytes_in_space,
                                    const ObjectMemorySpace *mem_space) const;

    /**
     * @return current maximum size of this heap space
     */
    size_t GetCurrentSize() const;

    /**
     * @return free bytes count for current heap space size
     */
    size_t GetCurrentFreeBytes(size_t bytes_not_in_this_space = 0) const;

    inline bool IsWorkGC() const
    {
        return is_work_gc_;
    }

    mutable os::memory::RWLock heap_lock_;  // NOLINT(misc-non-private-member-variables-in-classes)
    ObjectMemorySpace mem_space_;           // NOLINT(misc-non-private-member-variables-in-classes)
    bool is_initialized_ {false};           // NOLINT(misc-non-private-member-variables-in-classes)

private:
    // if GC wants allocate memory, but we have not needed memory for this then we increase current heap space
    bool is_work_gc_ {false};
    double min_free_percentage_ {0};
    double max_free_percentage_ {0};

    friend class panda::mem::test::RegionGarbageChoosingTest;
    template <typename ObjectAllocator, bool RegularSpace>
    friend class panda::mem::test::RegionAllocatorTestBase;
    friend class panda::mem::test::RegionAllocatorTest;
    friend class panda::mem::test::RegionNonmovableObjectAllocatorTest;
    friend class panda::mem::test::RemSetTest;
    friend class panda::mem::test::HeapSpaceTest;
};

// Y - memory chunk is used for objects in young space
// T - memory chunk is used for objects in tenured space
// F - memory chunk which is not used for any space
//
//  +---------------------------------------------------------------------------------------------------------+
//  |                 Real object memory chunks location allocating via PoolManager                           |
//  |---------------------------------------------------------------------------------------------------------|
//  | shared pool (use for G1-GC)                        young, tenured and free pools                        |
//  |-----------------------------|---------------------------------------------------------------------------|
//  |~~~|       |   |~~~|~~~|     |~~~~~|         |~~~~~~~~~|       |~~~~~~~~~~~~~|                           |
//  |~~~|       |   |~~~|~~~|     |~~~~~|    F    |~~~~~~~~~|   F   |~~~~~~~~~~~~~|                           |
//  |~Y~|   F   | F |~Y~|~T~|  F  |~~Y~~|  (free  |~~~~T~~~~| (free |~~~~~~T~~~~~~|  non-occupied memory (F)  |
//  |~~~|       |   |~~~|~~~|     |~~~~~|   pool  |~~~~~~~~~|  pool |~~~~~~~~~~~~~|                           |
//  |~~~|       |   |~~~|~~~|     |~~~~~|   map)  |~~~~~~~~~|  map) |~~~~~~~~~~~~~|                           |
//  +---------------------------------------------------------------------------------------------------------+
//  ^ |               |   |          |                 |                   |      ^                           ^
//  | |               |   |          |                 |        +----------+      |                           |
//  | |   +-----------+   |          |                 |        |                 |                           |
//  | |   |    +----------|----------+  +--------------+        |    all occupied memory via PoolManager      |
//  | |   |    |          +-------+     |          +------------+                                             |
//  | |   |    |                  |     |          |                                                          |
//  v v   v    v                  v     v          v                                                          v
//  +---------------------------|-----------------------------------------------------------------------------+
//  |~~~|~~~|~~~~~|     |       |~~~|~~~~~~~|~~~~~~~~~~~~~|                         |                         |
//  |~~~|~~~|~~~~~|     |       |~~~|~~~~~~~|~~~~~~~~~~~~~|                         |                         |
//  |~Y~|~Y~|~~Y~~|  F  |   F   |~T~|~~~T~~~|~~~~~~T~~~~~~|            F            |            F            |
//  |~~~|~~~|~~~~~|     |       |~~~|~~~~~~~|~~~~~~~~~~~~~|                         |                         |
//  |~~~|~~~|~~~~~|     |       |~~~|~~~~~~~|~~~~~~~~~~~~~|                         |                         |
//  |-------------|-----|-------|-------------------------|-------------------------|-------------------------|
//  |     used    |     |       |          used           |                         |                         |
//  |-------------------|-------|---------------------------------------------------|-------------------------|
//  | current max size  |       |               current max size                    |                         |
//  |---------------------------|-----------------------------------------------------------------------------|
//  |        Young space        |                            Tenured space                                    |
//  |---------------------------|-----------------------------------------------------------------------------|
//  |                    young-space-size                                                            object-memory-pool
//  |                       argument                                                                      argument
//  |---------------------------------------------------------------------------------------------------------|
//  |                                   Generations memory representation                                     |
//  +---------------------------------------------------------------------------------------------------------+
//
//  init-young-space-size argument set minimum (initial) current max size for Young space
//  init-object-memory-pool argument set minimum (initial) current max size for heap
/**
 * Class for description of object spaces and limits (minimum, maximum, current sizes) for generatinal-based GC.
 * Pools for young and tenured spaces is allocated via this class. Also this class cooperate with PoolManager for info
 * getting about object spaces. If current max size is not enough for new pool in some space then need to trigger GC
 */
class GenerationalSpaces final : public HeapSpace {
public:
    explicit GenerationalSpaces() = default;
    NO_COPY_SEMANTIC(GenerationalSpaces);
    NO_MOVE_SEMANTIC(GenerationalSpaces);
    ~GenerationalSpaces() override = default;

    void Initialize(size_t initial_young_size, bool was_set_initial_young_size, size_t max_young_size,
                    bool was_set_max_young_size, size_t initial_total_size, size_t max_total_size,
                    uint32_t min_free_percentage, uint32_t max_free_percentage);

    /**
     * \brief Compute new sizes for young and tenured spaces
     */
    void ComputeNewSize() override;

    size_t GetHeapSize() const override;

    // Use for CanAllocInSpace
    static constexpr bool IS_YOUNG_SPACE = true;
    static constexpr bool IS_TENURED_SPACE = false;

    /**
     * @return true if we can allocate requested size in space
     *
     * @param is_young young or tenured space
     * @param chunk_size memory size for allocating in space
     */
    bool CanAllocInSpace(bool is_young, size_t chunk_size) const;

    size_t GetCurrentMaxYoungSize() const;

    size_t GetMaxYoungSize() const;

    void UseFullYoungSpace();

    /**
     * \brief Allocate alone young pool (use for Gen-GC) with maximum possible size of young space
     */
    [[nodiscard]] Pool AllocAlonePoolForYoung(SpaceType space_type, AllocatorType allocator_type, void *allocator_ptr);

    /**
     * \brief Try allocate pool for young space (use for G1-GC).
     * If free size in young space less requested pool size then pool can be allocate only during GC work
     */
    [[nodiscard]] Pool TryAllocPoolForYoung(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                            void *allocator_ptr);

    /**
     * \brief Try allocate pool for tenured space (use for generational-based GC).
     * If free size in tenured space less requested pool size then pool can be allocate only during GC work
     */
    [[nodiscard]] Pool TryAllocPoolForTenured(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                              void *allocator_ptr);

    [[nodiscard]] Pool TryAllocPool(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                    void *allocator_ptr) final;

    [[nodiscard]] Arena *TryAllocArenaForTenured(size_t arena_size, SpaceType space_type, AllocatorType allocator_type,
                                                 void *allocator_ptr);

    [[nodiscard]] Arena *TryAllocArena(size_t arena_size, SpaceType space_type, AllocatorType allocator_type,
                                       void *allocator_ptr) final;

    /**
     * \brief Allocate pool shared usage between young and tenured spaces (use for regions in G1-GC).
     */
    [[nodiscard]] Pool AllocSharedPool(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                       void *allocator_ptr);

    void IncreaseYoungOccupiedInSharedPool(size_t chunk_size);

    void IncreaseTenuredOccupiedInSharedPool(size_t chunk_size);

    void ReduceYoungOccupiedInSharedPool(size_t chunk_size);

    void ReduceTenuredOccupiedInSharedPool(size_t chunk_size);

    void FreeSharedPool(void *pool_mem, size_t pool_size);

    void FreeYoungPool(void *pool_mem, size_t pool_size);

    void PromoteYoungPool(size_t pool_size);

    void FreeTenuredPool(void *pool_mem, size_t pool_size);

    size_t GetCurrentFreeTenuredSize() const;

    size_t GetCurrentFreeYoungSize() const;

private:
    void ComputeNewYoung();
    void ComputeNewTenured();

    size_t GetCurrentFreeTenuredSizeUnsafe() const;
    size_t GetCurrentFreeYoungSizeUnsafe() const;

    ObjectMemorySpace young_space_;
    size_t young_size_in_separate_pools_ {0};

    // These use for allocate shared (for young and tenured) pools and allocate blocks from them
    size_t shared_pools_size_ {0};
    size_t young_size_in_shared_pools_ {0};
    size_t tenured_size_in_shared_pools_ {0};

    friend class panda::mem::test::HeapSpaceTest;
    friend class panda::mem::test::MemStatsGenGCTest;
    friend class panda::mem::test::RegionGarbageChoosingTest;
    template <typename ObjectAllocator, bool RegularSpace>
    friend class panda::mem::test::RegionAllocatorTestBase;
    friend class panda::mem::test::RegionAllocatorTest;
    friend class panda::mem::test::RegionNonmovableObjectAllocatorTest;
    friend class panda::mem::test::RemSetTest;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_HEAP_SPACE_H
