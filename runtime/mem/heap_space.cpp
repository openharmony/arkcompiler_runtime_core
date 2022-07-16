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

#include "runtime/mem/heap_space.h"
#include "libpandabase/mem/mem.h"
#include "libpandabase/mem/pool_manager.h"
#include "libpandabase/mem/mmap_mem_pool-inl.h"

namespace panda::mem {

void HeapSpace::Initialize(size_t initial_size, size_t max_size, uint32_t min_free_percentage,
                           uint32_t max_free_percentage)
{
    ASSERT(!is_initialized_);
    mem_space_.Initialize(initial_size, max_size);
    InitializePercentages(min_free_percentage, max_free_percentage);
    is_initialized_ = true;
}

void HeapSpace::InitializePercentages(uint32_t min_free_percentage, uint32_t max_free_percentage)
{
    min_free_percentage_ = static_cast<double>(std::min(min_free_percentage, MAX_FREE_PERCENTAGE)) / PERCENT_100_U32;
    max_free_percentage_ = static_cast<double>(std::min(max_free_percentage, MAX_FREE_PERCENTAGE)) / PERCENT_100_U32;
}

void HeapSpace::ObjectMemorySpace::Initialize(size_t initial_size, size_t max_size)
{
    min_size_ = initial_size;
    max_size_ = max_size;
    ASSERT(min_size_ <= max_size_);
    // Set current space size as initial_size
    current_size_ = min_size_;
}

inline void HeapSpace::ObjectMemorySpace::IncreaseBy(uint64_t bytes)
{
    current_size_ =
        std::min(AlignUp(current_size_ + bytes, DEFAULT_ALIGNMENT_IN_BYTES), static_cast<uint64_t>(max_size_));
}

inline void HeapSpace::ObjectMemorySpace::ReduceBy(size_t bytes)
{
    ASSERT(current_size_ >= bytes);
    current_size_ = AlignUp(current_size_ - bytes, DEFAULT_ALIGNMENT_IN_BYTES);
    current_size_ = std::max(current_size_, min_size_);
}

void HeapSpace::ObjectMemorySpace::ComputeNewSize(size_t free_bytes, double min_free_percentage,
                                                  double max_free_percentage)
{
    ASSERT(free_bytes <= current_size_);
    // How many bytes are used in space now
    size_t used_bytes = current_size_ - free_bytes;

    uint64_t min_needed_bytes = static_cast<double>(used_bytes) / (1.0 - min_free_percentage);
    if (current_size_ < min_needed_bytes) {
        IncreaseBy(min_needed_bytes - current_size_);
        return;
    }

    uint64_t max_needed_bytes = static_cast<double>(used_bytes) / (1.0 - max_free_percentage);
    if (current_size_ > max_needed_bytes) {
        ReduceBy(current_size_ - max_needed_bytes);
    }
}

inline size_t HeapSpace::GetCurrentFreeBytes(size_t bytes_not_in_this_space) const
{
    ASSERT(is_initialized_);
    size_t used_bytes = PoolManager::GetMmapMemPool()->GetObjectUsedBytes();
    ASSERT(used_bytes >= bytes_not_in_this_space);
    size_t used_bytes_in_current_space = used_bytes - bytes_not_in_this_space;
    ASSERT(GetCurrentSize() >= used_bytes_in_current_space);
    return GetCurrentSize() - used_bytes_in_current_space;
}

void HeapSpace::ComputeNewSize()
{
    os::memory::WriteLockHolder lock(heap_lock_);
    mem_space_.ComputeNewSize(GetCurrentFreeBytes(), min_free_percentage_, max_free_percentage_);
    // Get current free bytes count after computing new size
    size_t current_free_bytes_in_space = GetCurrentFreeBytes();
    // If saved pool size was very big and such pool can not be allocate after GC
    // then we increase space to allocate this pool
    if (mem_space_.saved_pool_size > current_free_bytes_in_space) {
        mem_space_.IncreaseBy(mem_space_.saved_pool_size - current_free_bytes_in_space);
        mem_space_.saved_pool_size = 0;
        // Free bytes after increase space for new pool will = 0, so yet increase space
        mem_space_.ComputeNewSize(0, min_free_percentage_, max_free_percentage_);
    }
    // ComputeNewSize is called on GC end
    SetIsWorkGC(false);
}

size_t HeapSpace::GetHeapSize() const
{
    return PoolManager::GetMmapMemPool()->GetObjectUsedBytes();
}

inline std::optional<size_t> HeapSpace::WillAlloc(size_t pool_size, size_t current_free_bytes_in_space,
                                                  const ObjectMemorySpace *mem_space) const
{
    ASSERT(is_initialized_);
    // If can allocate pool (from free pool map or non-used memory) then just do it
    if (LIKELY(pool_size <= current_free_bytes_in_space)) {
        // We have enough memory for allocation, no need to increase heap
        return {0};
    }
    // If we allocate pool during GC work then we must allocate new pool anyway, so we wiil try to increase heap space
    if (IsWorkGC()) {
        // if requested pool size greater free bytes in current heap space and non occupied memory then we can not
        // allocate such pool, so we need to trigger GC
        if (current_free_bytes_in_space + mem_space->GetCurrentNonOccupiedSize() < pool_size) {
            return std::nullopt;
        }
        // In this case we need increase space for allocate new pool
        return {pool_size - current_free_bytes_in_space};
    }
    // Otherwise we need to trigger GC
    return std::nullopt;
}

size_t HeapSpace::GetCurrentSize() const
{
    return mem_space_.GetCurrentSize();
}

inline Pool HeapSpace::TryAllocPoolBase(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                        void *allocator_ptr, size_t current_free_bytes_in_space,
                                        ObjectMemorySpace *mem_space)
{
    auto increase_bytes_or_not_alloc = WillAlloc(pool_size, current_free_bytes_in_space, mem_space);
    // Increase heap space if needed and allocate pool
    if (increase_bytes_or_not_alloc) {
        mem_space->IncreaseBy(increase_bytes_or_not_alloc.value());
        return PoolManager::GetMmapMemPool()->AllocPool(pool_size, space_type, allocator_type, allocator_ptr);
    }
    // Save pool size for computing new space size
    mem_space->saved_pool_size = pool_size;
    return NULLPOOL;
}

Pool HeapSpace::TryAllocPool(size_t pool_size, SpaceType space_type, AllocatorType allocator_type, void *allocator_ptr)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    return TryAllocPoolBase(pool_size, space_type, allocator_type, allocator_ptr, GetCurrentFreeBytes(), &mem_space_);
}

inline Arena *HeapSpace::TryAllocArenaBase(size_t arena_size, SpaceType space_type, AllocatorType allocator_type,
                                           void *allocator_ptr, size_t current_free_bytes_in_space,
                                           ObjectMemorySpace *mem_space)
{
    auto increase_bytes_or_not_alloc = WillAlloc(arena_size, current_free_bytes_in_space, mem_space);
    // Increase heap space if needed and allocate arena
    if (increase_bytes_or_not_alloc.has_value()) {
        mem_space->IncreaseBy(increase_bytes_or_not_alloc.value());
        return PoolManager::AllocArena(arena_size, space_type, allocator_type, allocator_ptr);
    }
    // Save arena size for computing new space size
    mem_space->saved_pool_size = arena_size;
    return nullptr;
}

Arena *HeapSpace::TryAllocArena(size_t arena_size, SpaceType space_type, AllocatorType allocator_type,
                                void *allocator_ptr)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    return TryAllocArenaBase(arena_size, space_type, allocator_type, allocator_ptr, GetCurrentFreeBytes(), &mem_space_);
}

void HeapSpace::FreePool(void *pool_mem, size_t pool_size)
{
    os::memory::ReadLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    // Just free pool
    PoolManager::GetMmapMemPool()->FreePool(pool_mem, pool_size);
}

void HeapSpace::FreeArena(Arena *arena)
{
    os::memory::ReadLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    // Just free arena
    PoolManager::FreeArena(arena);
}

void GenerationalSpaces::Initialize(size_t initial_young_size, bool was_set_initial_young_size, size_t max_young_size,
                                    bool was_set_max_young_size, size_t initial_total_size, size_t max_total_size,
                                    uint32_t min_free_percentage, uint32_t max_free_percentage)
{
    // Temporary save total heap size parameters and set percetages
    HeapSpace::Initialize(initial_total_size, max_total_size, min_free_percentage, max_free_percentage);

    if (!was_set_initial_young_size && was_set_max_young_size) {
        initial_young_size = max_young_size;
    } else if (initial_young_size > max_young_size) {
        LOG_IF(was_set_initial_young_size && was_set_max_young_size, WARNING, RUNTIME)
            << "Initial young size(init-young-space-size=" << initial_young_size
            << ") is larger than maximum young size (young-space-size=" << max_young_size
            << "). Set maximum young size to " << initial_young_size;
        max_young_size = initial_young_size;
    }
    young_space_.Initialize(initial_young_size, max_young_size);
    ASSERT(young_space_.GetCurrentSize() <= mem_space_.GetCurrentSize());
    ASSERT(young_space_.GetMaxSize() <= mem_space_.GetMaxSize());
    // Use mem_space_ as tenured space
    mem_space_.Initialize(mem_space_.GetCurrentSize() - young_space_.GetCurrentSize(),
                          mem_space_.GetMaxSize() - young_space_.GetMaxSize());
}

size_t GenerationalSpaces::GetCurrentFreeYoungSize() const
{
    os::memory::ReadLockHolder lock(heap_lock_);
    return GetCurrentFreeYoungSizeUnsafe();
}

size_t GenerationalSpaces::GetCurrentFreeTenuredSize() const
{
    os::memory::ReadLockHolder lock(heap_lock_);
    return GetCurrentFreeTenuredSizeUnsafe();
}

size_t GenerationalSpaces::GetCurrentFreeYoungSizeUnsafe() const
{
    size_t all_occupied_young_size = young_size_in_separate_pools_ + young_size_in_shared_pools_;
    ASSERT(young_space_.GetCurrentSize() >= all_occupied_young_size);
    return young_space_.GetCurrentSize() - all_occupied_young_size;
}

size_t GenerationalSpaces::GetCurrentFreeTenuredSizeUnsafe() const
{
    ASSERT(shared_pools_size_ >= tenured_size_in_shared_pools_);
    // bytes_not_in_tenured_space = occupied pools size by young + non-tenured size in shared pool
    return GetCurrentFreeBytes(young_size_in_separate_pools_ + (shared_pools_size_ - tenured_size_in_shared_pools_));
}

void GenerationalSpaces::ComputeNewSize()
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ComputeNewYoung();
    ComputeNewTenured();
    SetIsWorkGC(false);
}

void GenerationalSpaces::ComputeNewYoung()
{
    double min_free_percentage = GetMinFreePercentage();
    double max_free_percentage = GetMaxFreePercentage();
    young_space_.ComputeNewSize(GetCurrentFreeYoungSizeUnsafe(), min_free_percentage, max_free_percentage);
    // Get free bytes count after computing new young size
    size_t free_young_bytes_after_computing = GetCurrentFreeYoungSizeUnsafe();
    // If saved pool size was very big and such pool can not be allocate in young after GC
    // then we increase young space to allocate this pool
    if (young_space_.saved_pool_size > free_young_bytes_after_computing) {
        young_space_.IncreaseBy(young_space_.saved_pool_size - free_young_bytes_after_computing);
        young_space_.saved_pool_size = 0;
        // Free bytes after increase young space for new pool will = 0, so yet increase young space
        young_space_.ComputeNewSize(0, min_free_percentage, max_free_percentage);
    }
}

void GenerationalSpaces::ComputeNewTenured()
{
    double min_free_percentage = GetMinFreePercentage();
    double max_free_percentage = GetMaxFreePercentage();
    mem_space_.ComputeNewSize(GetCurrentFreeTenuredSizeUnsafe(), min_free_percentage, max_free_percentage);
    // Get free bytes count after computing new tenured size
    size_t free_tenured_bytes_after_computing = GetCurrentFreeTenuredSizeUnsafe();
    // If saved pool size was very big and such pool can not be allocate in tenured after GC
    // then we increase tenured space to allocate this pool
    if (mem_space_.saved_pool_size > free_tenured_bytes_after_computing) {
        mem_space_.IncreaseBy(mem_space_.saved_pool_size - free_tenured_bytes_after_computing);
        mem_space_.saved_pool_size = 0;
        // Free bytes after increase tenured space for new pool will = 0, so yet increase tenured space
        mem_space_.ComputeNewSize(0, min_free_percentage, max_free_percentage);
    }
}

size_t GenerationalSpaces::GetHeapSize() const
{
    os::memory::ReadLockHolder lock(heap_lock_);
    size_t used_bytes_in_separate_pools = PoolManager::GetMmapMemPool()->GetObjectUsedBytes() - shared_pools_size_;
    size_t used_bytes_in_shared_pool = young_size_in_shared_pools_ + tenured_size_in_shared_pools_;
    return used_bytes_in_separate_pools + used_bytes_in_shared_pool;
}

bool GenerationalSpaces::CanAllocInSpace(bool is_young, size_t chunk_size) const
{
    os::memory::ReadLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    return is_young ? WillAlloc(chunk_size, GetCurrentFreeYoungSizeUnsafe(), &young_space_).has_value()
                    : WillAlloc(chunk_size, GetCurrentFreeTenuredSizeUnsafe(), &mem_space_).has_value();
}

size_t GenerationalSpaces::GetCurrentMaxYoungSize() const
{
    os::memory::ReadLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    return young_space_.GetCurrentSize();
}

size_t GenerationalSpaces::GetMaxYoungSize() const
{
    ASSERT(is_initialized_);
    return young_space_.GetMaxSize();
}

void GenerationalSpaces::UseFullYoungSpace()
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    young_space_.UseFullSpace();
}

Pool GenerationalSpaces::AllocSharedPool(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                         void *allocator_ptr)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    auto shared_pool = PoolManager::GetMmapMemPool()->AllocPool(pool_size, space_type, allocator_type, allocator_ptr);
    shared_pools_size_ += shared_pool.GetSize();
    return shared_pool;
}

Pool GenerationalSpaces::AllocAlonePoolForYoung(SpaceType space_type, AllocatorType allocator_type, void *allocator_ptr)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    auto young_pool =
        PoolManager::GetMmapMemPool()->AllocPool(young_space_.GetMaxSize(), space_type, allocator_type, allocator_ptr);
    young_size_in_separate_pools_ = young_pool.GetSize();
    return young_pool;
}

Pool GenerationalSpaces::TryAllocPoolForYoung(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                              void *allocator_ptr)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    auto young_pool = TryAllocPoolBase(pool_size, space_type, allocator_type, allocator_ptr,
                                       GetCurrentFreeYoungSizeUnsafe(), &young_space_);
    young_size_in_separate_pools_ += young_pool.GetSize();
    return young_pool;
}

Pool GenerationalSpaces::TryAllocPoolForTenured(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                                void *allocator_ptr)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    return TryAllocPoolBase(pool_size, space_type, allocator_type, allocator_ptr, GetCurrentFreeTenuredSizeUnsafe(),
                            &mem_space_);
}

Pool GenerationalSpaces::TryAllocPool(size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                                      void *allocator_ptr)
{
    return TryAllocPoolForTenured(pool_size, space_type, allocator_type, allocator_ptr);
}

Arena *GenerationalSpaces::TryAllocArenaForTenured(size_t arena_size, SpaceType space_type,
                                                   AllocatorType allocator_type, void *allocator_ptr)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    return TryAllocArenaBase(arena_size, space_type, allocator_type, allocator_ptr, GetCurrentFreeTenuredSizeUnsafe(),
                             &mem_space_);
}

Arena *GenerationalSpaces::TryAllocArena(size_t arena_size, SpaceType space_type, AllocatorType allocator_type,
                                         void *allocator_ptr)
{
    return TryAllocArenaForTenured(arena_size, space_type, allocator_type, allocator_ptr);
}

void GenerationalSpaces::FreeSharedPool(void *pool_mem, size_t pool_size)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(shared_pools_size_ >= pool_size);
    shared_pools_size_ -= pool_size;
    PoolManager::GetMmapMemPool()->FreePool(pool_mem, pool_size);
}

void GenerationalSpaces::FreeYoungPool(void *pool_mem, size_t pool_size)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(young_size_in_separate_pools_ >= pool_size);
    young_size_in_separate_pools_ -= pool_size;
    PoolManager::GetMmapMemPool()->FreePool(pool_mem, pool_size);
}

void GenerationalSpaces::PromoteYoungPool(size_t pool_size)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(young_size_in_separate_pools_ >= pool_size);
    young_size_in_separate_pools_ -= pool_size;
    auto increase_bytes_or_not_alloc = WillAlloc(pool_size, GetCurrentFreeTenuredSizeUnsafe(), &mem_space_);
    ASSERT(increase_bytes_or_not_alloc.has_value());
    mem_space_.IncreaseBy(increase_bytes_or_not_alloc.value());
}

void GenerationalSpaces::FreeTenuredPool(void *pool_mem, size_t pool_size)
{
    // For tenured we just free pool
    HeapSpace::FreePool(pool_mem, pool_size);
}

void GenerationalSpaces::IncreaseYoungOccupiedInSharedPool(size_t chunk_size)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    size_t free_bytes = GetCurrentFreeYoungSizeUnsafe();
    // Here we sure that we must allocate new memory, but if free bytes count less requested size (for example, during
    // GC work) then we increase young space size
    if (free_bytes < chunk_size) {
        young_space_.IncreaseBy(chunk_size - free_bytes);
    }
    young_size_in_shared_pools_ += chunk_size;
    ASSERT(young_size_in_shared_pools_ + tenured_size_in_shared_pools_ <= shared_pools_size_);
}

void GenerationalSpaces::IncreaseTenuredOccupiedInSharedPool(size_t chunk_size)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    size_t free_bytes = GetCurrentFreeTenuredSizeUnsafe();
    // Here we sure that we must allocate new memory, but if free bytes count less requested size (for example, during
    // GC work) then we increase tenured space size
    if (free_bytes < chunk_size) {
        mem_space_.IncreaseBy(chunk_size - free_bytes);
    }
    tenured_size_in_shared_pools_ += chunk_size;
    ASSERT(young_size_in_shared_pools_ + tenured_size_in_shared_pools_ <= shared_pools_size_);
}

void GenerationalSpaces::ReduceYoungOccupiedInSharedPool(size_t chunk_size)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    ASSERT(young_size_in_shared_pools_ >= chunk_size);
    young_size_in_shared_pools_ -= chunk_size;
}

void GenerationalSpaces::ReduceTenuredOccupiedInSharedPool(size_t chunk_size)
{
    os::memory::WriteLockHolder lock(heap_lock_);
    ASSERT(is_initialized_);
    ASSERT(tenured_size_in_shared_pools_ >= chunk_size);
    tenured_size_in_shared_pools_ -= chunk_size;
}

}  // namespace panda::mem
