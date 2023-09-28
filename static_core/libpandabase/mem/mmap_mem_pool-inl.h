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

#ifndef LIBPANDABASE_MEM_MMAP_MEM_POOL_INLINE_H
#define LIBPANDABASE_MEM_MMAP_MEM_POOL_INLINE_H

#include <utility>
#ifdef PANDA_QEMU_BUILD
// Unfortunately, madvise on QEMU works differently, and we should zeroed pages by hand.
#include <securec.h>
#endif
#include "mmap_mem_pool.h"
#include "mem.h"
#include "os/mem.h"
#include "utils/logger.h"
#include "mem/arena-inl.h"
#include "mem/mem_config.h"
#include "utils/asan_interface.h"

namespace panda {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_MMAP_MEM_POOL(level) LOG(level, MEMORYPOOL) << "MmapMemPool: "

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
inline Pool MmapPoolMap::PopFreePool(size_t size)
{
    auto element = free_pools_.lower_bound(size);
    if (element == free_pools_.end()) {
        return NULLPOOL;
    }
    auto mmap_pool = element->second;
    ASSERT(!mmap_pool->IsUsed(free_pools_.end()));
    auto element_size = element->first;
    ASSERT(element_size == mmap_pool->GetSize());
    auto element_mem = mmap_pool->GetMem();

    mmap_pool->SetFreePoolsIter(free_pools_.end());
    Pool pool(size, element_mem);
    free_pools_.erase(element);
    if (size < element_size) {
        Pool new_pool(element_size - size, ToVoidPtr(ToUintPtr(element_mem) + size));
        mmap_pool->SetSize(size);
        auto new_mmap_pool = new MmapPool(new_pool, free_pools_.end());
        pool_map_.insert(std::pair<void *, MmapPool *>(new_pool.GetMem(), new_mmap_pool));
        auto new_free_pools_iter = free_pools_.insert(std::pair<size_t, MmapPool *>(new_pool.GetSize(), new_mmap_pool));
        new_mmap_pool->SetFreePoolsIter(new_free_pools_iter);
        new_mmap_pool->SetReturnedToOS(mmap_pool->IsReturnedToOS());
    }
    if (OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY && !mmap_pool->IsReturnedToOS()) {
        uintptr_t pool_start = ToUintPtr(pool.GetMem());
        size_t pool_size = pool.GetSize();
        LOG_MMAP_MEM_POOL(DEBUG) << "Return pages to OS from Free Pool to get zeroed memory: start = " << pool.GetMem()
                                 << " with size " << pool_size;
        os::mem::ReleasePages(pool_start, pool_start + pool_size);
    }
    return pool;
}

template <OSPagesPolicy OS_PAGES_POLICY>
inline std::pair<size_t, OSPagesPolicy> MmapPoolMap::PushFreePool(Pool pool)
{
    bool returned_to_os = OS_PAGES_POLICY == OSPagesPolicy::IMMEDIATE_RETURN;
    auto mmap_pool_element = pool_map_.find(pool.GetMem());
    if (UNLIKELY(mmap_pool_element == pool_map_.end())) {
        LOG_MMAP_MEM_POOL(FATAL) << "can't find mmap pool in the pool map when PushFreePool";
    }

    auto mmap_pool = mmap_pool_element->second;
    ASSERT(mmap_pool->IsUsed(free_pools_.end()));

    auto prev_pool = (mmap_pool_element != pool_map_.begin()) ? (prev(mmap_pool_element, 1)->second) : nullptr;
    if (prev_pool != nullptr && !prev_pool->IsUsed(free_pools_.end())) {
        ASSERT(ToUintPtr(prev_pool->GetMem()) + prev_pool->GetSize() == ToUintPtr(mmap_pool->GetMem()));
        returned_to_os = returned_to_os && prev_pool->IsReturnedToOS();
        free_pools_.erase(prev_pool->GetFreePoolsIter());
        prev_pool->SetSize(prev_pool->GetSize() + mmap_pool->GetSize());
        delete mmap_pool;
        pool_map_.erase(mmap_pool_element--);
        mmap_pool = prev_pool;
    }

    auto next_pool = (mmap_pool_element != prev(pool_map_.end(), 1)) ? (next(mmap_pool_element, 1)->second) : nullptr;
    if (next_pool != nullptr && !next_pool->IsUsed(free_pools_.end())) {
        ASSERT(ToUintPtr(mmap_pool->GetMem()) + mmap_pool->GetSize() == ToUintPtr(next_pool->GetMem()));
        returned_to_os = returned_to_os && next_pool->IsReturnedToOS();
        free_pools_.erase(next_pool->GetFreePoolsIter());
        mmap_pool->SetSize(next_pool->GetSize() + mmap_pool->GetSize());
        delete next_pool;
        pool_map_.erase(++mmap_pool_element);
    } else if (next_pool == nullptr) {
        // It is the last pool. Transform it to free space.
        pool_map_.erase(mmap_pool_element);
        size_t size = mmap_pool->GetSize();
        delete mmap_pool;
        if (returned_to_os) {
            return {size, OSPagesPolicy::IMMEDIATE_RETURN};
        }
        return {size, OSPagesPolicy::NO_RETURN};
    }

    auto res = free_pools_.insert(std::pair<size_t, MmapPool *>(mmap_pool->GetSize(), mmap_pool));
    mmap_pool->SetFreePoolsIter(res);
    mmap_pool->SetReturnedToOS(returned_to_os);
    return {0, OS_PAGES_POLICY};
}

inline void MmapPoolMap::IterateOverFreePools(const std::function<void(size_t, MmapPool *)> &visitor)
{
    for (auto &it : free_pools_) {
        visitor(it.first, it.second);
    }
}

inline void MmapPoolMap::AddNewPool(Pool pool)
{
    auto new_mmap_pool = new MmapPool(pool, free_pools_.end());
    pool_map_.insert(std::pair<void *, MmapPool *>(pool.GetMem(), new_mmap_pool));
}

inline size_t MmapPoolMap::GetAllSize() const
{
    size_t bytes = 0;
    for (const auto &pool : free_pools_) {
        bytes += pool.first;
    }
    return bytes;
}

inline bool MmapPoolMap::HaveEnoughFreePools(size_t pools_num, size_t pool_size) const
{
    size_t pools = 0;
    for (auto pool = free_pools_.rbegin(); pool != free_pools_.rend(); pool++) {
        if (pool->first < pool_size) {
            return false;
        }
        pools += pool->first / pool_size;
        if (pools >= pools_num) {
            return true;
        }
    }
    return false;
}

inline MmapMemPool::MmapMemPool()
    : MemPool("MmapMemPool"), non_object_spaces_current_size_ {0}, non_object_spaces_max_size_ {0}
{
    ASSERT(static_cast<uint64_t>(mem::MemConfig::GetHeapSizeLimit()) <= PANDA_MAX_HEAP_SIZE);
    uint64_t object_space_size = mem::MemConfig::GetHeapSizeLimit();
    if (object_space_size > PANDA_MAX_HEAP_SIZE) {
        LOG_MMAP_MEM_POOL(FATAL) << "The memory limits is too high. We can't allocate so much memory from the system";
    }
    ASSERT(object_space_size <= PANDA_MAX_HEAP_SIZE);
#if defined(PANDA_USE_32_BIT_POINTER) && !defined(PANDA_TARGET_WINDOWS)
    void *mem =
        panda::os::mem::MapRWAnonymousInFirst4GB(ToVoidPtr(PANDA_32BITS_HEAP_START_ADDRESS), object_space_size,
                                                 // Object space must be aligned to PANDA_POOL_ALIGNMENT_IN_BYTES
                                                 PANDA_POOL_ALIGNMENT_IN_BYTES);
    ASSERT((ToUintPtr(mem) < PANDA_32BITS_HEAP_END_OBJECTS_ADDRESS) || (object_space_size == 0));
    ASSERT(ToUintPtr(mem) + object_space_size <= PANDA_32BITS_HEAP_END_OBJECTS_ADDRESS);
#else
    // We should get aligned to PANDA_POOL_ALIGNMENT_IN_BYTES size
    void *mem = panda::os::mem::MapRWAnonymousWithAlignmentRaw(object_space_size, PANDA_POOL_ALIGNMENT_IN_BYTES);
#endif
    LOG_IF(((mem == nullptr) && (object_space_size != 0)), FATAL, MEMORYPOOL)
        << "MmapMemPool: couldn't mmap " << object_space_size << " bytes of memory for the system";
    ASSERT(AlignUp(ToUintPtr(mem), PANDA_POOL_ALIGNMENT_IN_BYTES) == ToUintPtr(mem));
    min_object_memory_addr_ = ToUintPtr(mem);
    mmaped_object_memory_size_ = object_space_size;
    common_space_.Initialize(min_object_memory_addr_, object_space_size);
    non_object_spaces_max_size_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_CODE)] = mem::MemConfig::GetCodeCacheSizeLimit();
    non_object_spaces_max_size_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_COMPILER)] =
        mem::MemConfig::GetCompilerMemorySizeLimit();
    non_object_spaces_max_size_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_INTERNAL)] =
        mem::MemConfig::GetInternalMemorySizeLimit();
    // Should be fixed in 9888
    non_object_spaces_max_size_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_FRAMES)] = std::numeric_limits<size_t>::max();
    non_object_spaces_max_size_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_NATIVE_STACKS)] =
        mem::MemConfig::GetNativeStacksMemorySizeLimit();
    LOG_MMAP_MEM_POOL(DEBUG) << "Successfully initialized MMapMemPool. Object memory start from addr "
                             << ToVoidPtr(min_object_memory_addr_) << " Preallocated size is equal to "
                             << object_space_size;
}

inline MmapMemPool::~MmapMemPool()
{
    for (auto i : non_object_mmaped_pools_) {
        Pool pool = std::get<0>(i.second);
        [[maybe_unused]] AllocatorInfo info = std::get<1>(i.second);
        [[maybe_unused]] SpaceType type = std::get<2>(i.second);

        ASSERT(info.GetType() != AllocatorType::UNDEFINED);
        ASSERT(type != SpaceType::SPACE_TYPE_UNDEFINED);
        // does not clears non_object_mmaped_pools_ record because can fail (int munmap(void*, size_t) returned -1)
        FreeRawMemImpl(pool.GetMem(), pool.GetSize());
    }
    non_object_mmaped_pools_.clear();

    void *mmaped_mem_addr = ToVoidPtr(min_object_memory_addr_);
    if (mmaped_mem_addr == nullptr) {
        ASSERT(mmaped_object_memory_size_ == 0);
        return;
    }

    ASSERT(pool_map_.IsEmpty());

    // TODO(dtrubenkov): consider madvise(mem, total_size_, MADV_DONTNEED); when possible
    if (auto unmap_res = panda::os::mem::UnmapRaw(mmaped_mem_addr, mmaped_object_memory_size_)) {
        LOG_MMAP_MEM_POOL(FATAL) << "Destructor unnmap  error: " << unmap_res->ToString();
    }
}

template <class ArenaT, OSPagesAllocPolicy OS_ALLOC_POLICY>
inline ArenaT *MmapMemPool::AllocArenaImpl(size_t size, SpaceType space_type, AllocatorType allocator_type,
                                           const void *allocator_addr)
{
    os::memory::LockHolder lk(lock_);
    LOG_MMAP_MEM_POOL(DEBUG) << "Try to get new arena with size " << std::dec << size << " for "
                             << SpaceTypeToString(space_type);
    Pool pool_for_arena = AllocPoolUnsafe<OS_ALLOC_POLICY>(size, space_type, allocator_type, allocator_addr);
    void *mem = pool_for_arena.GetMem();
    if (UNLIKELY(mem == nullptr)) {
        LOG_MMAP_MEM_POOL(ERROR) << "Failed to allocate new arena"
                                 << " for " << SpaceTypeToString(space_type);
        return nullptr;
    }
    ASSERT(pool_for_arena.GetSize() == size);
    auto arena_buff_offs =
        AlignUp(ToUintPtr(mem) + sizeof(ArenaT), GetAlignmentInBytes(ARENA_DEFAULT_ALIGNMENT)) - ToUintPtr(mem);
    mem = new (mem) ArenaT(size - arena_buff_offs, ToVoidPtr(ToUintPtr(mem) + arena_buff_offs));
    LOG_MMAP_MEM_POOL(DEBUG) << "Allocated new arena with size " << std::dec << pool_for_arena.GetSize()
                             << " at addr = " << std::hex << pool_for_arena.GetMem() << " for "
                             << SpaceTypeToString(space_type);
    return static_cast<ArenaT *>(mem);
}

template <class ArenaT, OSPagesPolicy OS_PAGES_POLICY>
inline void MmapMemPool::FreeArenaImpl(ArenaT *arena)
{
    os::memory::LockHolder lk(lock_);
    size_t size = arena->GetSize() + (ToUintPtr(arena->GetMem()) - ToUintPtr(arena));
    ASSERT(size == AlignUp(size, panda::os::mem::GetPageSize()));
    LOG_MMAP_MEM_POOL(DEBUG) << "Try to free arena with size " << std::dec << size << " at addr = " << std::hex
                             << arena;
    FreePoolUnsafe<OS_PAGES_POLICY>(arena, size);
    LOG_MMAP_MEM_POOL(DEBUG) << "Free arena call finished";
}

inline void *MmapMemPool::AllocRawMemNonObjectImpl(size_t size, SpaceType space_type)
{
    ASSERT(!IsHeapSpace(space_type));
    void *mem = nullptr;
    if (LIKELY(non_object_spaces_max_size_[SpaceTypeToIndex(space_type)] >=
               non_object_spaces_current_size_[SpaceTypeToIndex(space_type)] + size)) {
        mem = panda::os::mem::MapRWAnonymousWithAlignmentRaw(size, PANDA_POOL_ALIGNMENT_IN_BYTES);
        if (mem != nullptr) {
            non_object_spaces_current_size_[SpaceTypeToIndex(space_type)] += size;
        }
    }
    LOG_MMAP_MEM_POOL(DEBUG) << "Occupied memory for " << SpaceTypeToString(space_type) << " - " << std::dec
                             << non_object_spaces_current_size_[SpaceTypeToIndex(space_type)];
    return mem;
}

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
inline void *MmapMemPool::AllocRawMemObjectImpl(size_t size, SpaceType type)
{
    ASSERT(IsHeapSpace(type));
    void *mem = common_space_.template AllocRawMem<OS_ALLOC_POLICY>(size, &common_space_pools_);
    LOG_MMAP_MEM_POOL(DEBUG) << "Occupied memory for " << SpaceTypeToString(type) << " - " << std::dec
                             << common_space_.GetOccupiedMemorySize();
    return mem;
}

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
inline void *MmapMemPool::AllocRawMemImpl(size_t size, SpaceType type)
{
    os::memory::LockHolder lk(lock_);
    ASSERT(size % panda::os::mem::GetPageSize() == 0);
    // NOTE: We need this check because we use this memory for Pools too
    // which require PANDA_POOL_ALIGNMENT_IN_BYTES alignment
    ASSERT(size == AlignUp(size, PANDA_POOL_ALIGNMENT_IN_BYTES));
    void *mem = nullptr;
    switch (type) {
        // Internal spaces
        case SpaceType::SPACE_TYPE_COMPILER:
        case SpaceType::SPACE_TYPE_INTERNAL:
        case SpaceType::SPACE_TYPE_CODE:
        case SpaceType::SPACE_TYPE_FRAMES:
        case SpaceType::SPACE_TYPE_NATIVE_STACKS:
            ASSERT(!IsHeapSpace(type));
            mem = AllocRawMemNonObjectImpl(size, type);
            break;
        // Heap spaces:
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
        case SpaceType::SPACE_TYPE_OBJECT:
            mem = AllocRawMemObjectImpl<OS_ALLOC_POLICY>(size, type);
            break;
        default:
            LOG_MMAP_MEM_POOL(FATAL) << "Try to use incorrect " << SpaceTypeToString(type) << " for AllocRawMem.";
    }
    if (UNLIKELY(mem == nullptr)) {
        LOG_MMAP_MEM_POOL(DEBUG) << "OOM when trying to allocate " << size << " bytes for " << SpaceTypeToString(type);
        // We have OOM and must return nullptr
        mem = nullptr;
    } else {
        LOG_MMAP_MEM_POOL(DEBUG) << "Allocate raw memory with size " << size << " at addr = " << mem << " for "
                                 << SpaceTypeToString(type);
    }
    return mem;
}

/* static */
inline void MmapMemPool::FreeRawMemImpl(void *mem, size_t size)
{
    if (auto unmap_res = panda::os::mem::UnmapRaw(mem, size)) {
        LOG_MMAP_MEM_POOL(FATAL) << "Destructor unnmap  error: " << unmap_res->ToString();
    }
    LOG_MMAP_MEM_POOL(DEBUG) << "Deallocated raw memory with size " << size << " at addr = " << mem;
}

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
inline Pool MmapMemPool::AllocPoolUnsafe(size_t size, SpaceType space_type, AllocatorType allocator_type,
                                         const void *allocator_addr)
{
    ASSERT(size == AlignUp(size, panda::os::mem::GetPageSize()));
    ASSERT(size == AlignUp(size, PANDA_POOL_ALIGNMENT_IN_BYTES));
    Pool pool = NULLPOOL;
    bool add_to_pool_map = false;
    // Try to find free pool from the allocated earlier
    switch (space_type) {
        case SpaceType::SPACE_TYPE_CODE:
        case SpaceType::SPACE_TYPE_COMPILER:
        case SpaceType::SPACE_TYPE_INTERNAL:
        case SpaceType::SPACE_TYPE_FRAMES:
        case SpaceType::SPACE_TYPE_NATIVE_STACKS:
            // We always use mmap for these space types
            break;
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
        case SpaceType::SPACE_TYPE_OBJECT:
            add_to_pool_map = true;
            pool = common_space_pools_.template PopFreePool<OS_ALLOC_POLICY>(size);
            break;
        default:
            LOG_MMAP_MEM_POOL(FATAL) << "Try to use incorrect " << SpaceTypeToString(space_type)
                                     << " for AllocPoolUnsafe.";
    }
    if (pool.GetMem() != nullptr) {
        LOG_MMAP_MEM_POOL(DEBUG) << "Reuse pool with size " << pool.GetSize() << " at addr = " << pool.GetMem()
                                 << " for " << SpaceTypeToString(space_type);
    }
    if (pool.GetMem() == nullptr) {
        void *mem = AllocRawMemImpl<OS_ALLOC_POLICY>(size, space_type);
        if (mem != nullptr) {
            pool = Pool(size, mem);
        }
    }
    if (pool.GetMem() == nullptr) {
        return pool;
    }
    ASAN_UNPOISON_MEMORY_REGION(pool.GetMem(), pool.GetSize());
    if (UNLIKELY(allocator_addr == nullptr)) {
        // Save a pointer to the first byte of a Pool
        allocator_addr = pool.GetMem();
    }
    if (add_to_pool_map) {
        pool_map_.AddPoolToMap(ToVoidPtr(ToUintPtr(pool.GetMem()) - GetMinObjectAddress()), pool.GetSize(), space_type,
                               allocator_type, allocator_addr);
#ifdef PANDA_QEMU_BUILD
        // Unfortunately, madvise on QEMU works differently, and we should zeroed pages by hand.
        if (OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY) {
            memset_s(pool.GetMem(), pool.GetSize(), 0, pool.GetSize());
        }
#endif
    } else {
        AddToNonObjectPoolsMap(std::make_tuple(pool, AllocatorInfo(allocator_type, allocator_addr), space_type));
    }
    os::mem::TagAnonymousMemory(pool.GetMem(), pool.GetSize(), SpaceTypeToString(space_type));
    ASSERT(AlignUp(ToUintPtr(pool.GetMem()), PANDA_POOL_ALIGNMENT_IN_BYTES) == ToUintPtr(pool.GetMem()));
    return pool;
}

template <OSPagesPolicy OS_PAGES_POLICY>
inline void MmapMemPool::FreePoolUnsafe(void *mem, size_t size)
{
    ASSERT(size == AlignUp(size, panda::os::mem::GetPageSize()));
    ASAN_POISON_MEMORY_REGION(mem, size);
    SpaceType pool_space_type = GetSpaceTypeForAddrImpl(mem);
    switch (pool_space_type) {
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
        case SpaceType::SPACE_TYPE_OBJECT: {
            auto free_size = common_space_pools_.PushFreePool<OS_PAGES_POLICY>(Pool(size, mem));
            common_space_.FreeMem(free_size.first, free_size.second);
            break;
        }
        case SpaceType::SPACE_TYPE_COMPILER:
        case SpaceType::SPACE_TYPE_INTERNAL:
        case SpaceType::SPACE_TYPE_CODE:
        case SpaceType::SPACE_TYPE_FRAMES:
        case SpaceType::SPACE_TYPE_NATIVE_STACKS:
            ASSERT(!IsHeapSpace(pool_space_type));
            non_object_spaces_current_size_[SpaceTypeToIndex(pool_space_type)] -= size;
            FreeRawMemImpl(mem, size);
            break;
        default:
            LOG_MMAP_MEM_POOL(FATAL) << "Try to use incorrect " << SpaceTypeToString(pool_space_type)
                                     << " for FreePoolUnsafe.";
    }
    os::mem::TagAnonymousMemory(mem, size, nullptr);
    if (IsHeapSpace(pool_space_type)) {
        pool_map_.RemovePoolFromMap(ToVoidPtr(ToUintPtr(mem) - GetMinObjectAddress()), size);
        if constexpr (OS_PAGES_POLICY == OSPagesPolicy::IMMEDIATE_RETURN) {
            LOG_MMAP_MEM_POOL(DEBUG) << "IMMEDIATE_RETURN and release pages for this pool";
            os::mem::ReleasePages(ToUintPtr(mem), ToUintPtr(mem) + size);
        }
    } else {
        RemoveFromNonObjectPoolsMap(mem);
    }
    LOG_MMAP_MEM_POOL(DEBUG) << "Freed " << std::dec << size << " memory for " << SpaceTypeToString(pool_space_type);
}

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
inline Pool MmapMemPool::AllocPoolImpl(size_t size, SpaceType space_type, AllocatorType allocator_type,
                                       const void *allocator_addr)
{
    os::memory::LockHolder lk(lock_);
    LOG_MMAP_MEM_POOL(DEBUG) << "Try to get new pool with size " << std::dec << size << " for "
                             << SpaceTypeToString(space_type);
    Pool pool = AllocPoolUnsafe<OS_ALLOC_POLICY>(size, space_type, allocator_type, allocator_addr);
    LOG_MMAP_MEM_POOL(DEBUG) << "Allocated new pool with size " << std::dec << pool.GetSize()
                             << " at addr = " << std::hex << pool.GetMem() << " for " << SpaceTypeToString(space_type);
    return pool;
}

template <OSPagesPolicy OS_PAGES_POLICY>
inline void MmapMemPool::FreePoolImpl(void *mem, size_t size)
{
    os::memory::LockHolder lk(lock_);
    LOG_MMAP_MEM_POOL(DEBUG) << "Try to free pool with size " << std::dec << size << " at addr = " << std::hex << mem;
    FreePoolUnsafe<OS_PAGES_POLICY>(mem, size);
    LOG_MMAP_MEM_POOL(DEBUG) << "Free pool call finished";
}

inline void MmapMemPool::AddToNonObjectPoolsMap(std::tuple<Pool, AllocatorInfo, SpaceType> pool_info)
{
    void *pool_addr = std::get<0>(pool_info).GetMem();
    ASSERT(non_object_mmaped_pools_.find(pool_addr) == non_object_mmaped_pools_.end());
    non_object_mmaped_pools_.insert({pool_addr, pool_info});
}

inline void MmapMemPool::RemoveFromNonObjectPoolsMap(void *pool_addr)
{
    auto element = non_object_mmaped_pools_.find(pool_addr);
    ASSERT(element != non_object_mmaped_pools_.end());
    non_object_mmaped_pools_.erase(element);
}

inline std::tuple<Pool, AllocatorInfo, SpaceType> MmapMemPool::FindAddrInNonObjectPoolsMap(const void *addr) const
{
    auto element = non_object_mmaped_pools_.lower_bound(addr);
    uintptr_t pool_start = (element != non_object_mmaped_pools_.end()) ? ToUintPtr(element->first)
                                                                       : (std::numeric_limits<uintptr_t>::max());
    if (ToUintPtr(addr) < pool_start) {
        ASSERT(element != non_object_mmaped_pools_.begin());
        element = std::prev(element);
        pool_start = ToUintPtr(element->first);
    }
    ASSERT(element != non_object_mmaped_pools_.end());
    [[maybe_unused]] uintptr_t pool_end = pool_start + std::get<0>(element->second).GetSize();
    ASSERT((pool_start <= ToUintPtr(addr)) && (ToUintPtr(addr) < pool_end));
    return element->second;
}

inline AllocatorInfo MmapMemPool::GetAllocatorInfoForAddrImpl(const void *addr) const
{
    if ((ToUintPtr(addr) < GetMinObjectAddress()) || (ToUintPtr(addr) >= GetMaxObjectAddress())) {
        os::memory::LockHolder lk(lock_);
        return std::get<1>(FindAddrInNonObjectPoolsMap(addr));
    }
    AllocatorInfo info = pool_map_.GetAllocatorInfo(ToVoidPtr(ToUintPtr(addr) - GetMinObjectAddress()));
    ASSERT(info.GetType() != AllocatorType::UNDEFINED);
    ASSERT(info.GetAllocatorHeaderAddr() != nullptr);
    return info;
}

inline SpaceType MmapMemPool::GetSpaceTypeForAddrImpl(const void *addr) const
{
    if ((ToUintPtr(addr) < GetMinObjectAddress()) || (ToUintPtr(addr) >= GetMaxObjectAddress())) {
        os::memory::LockHolder lk(lock_);
        // <2> is a pointer to SpaceType
        return std::get<2>(FindAddrInNonObjectPoolsMap(addr));
    }
    // Since this method is designed to work without locks for fast space reading, we add an
    // annotation here to prevent a false alarm with PoolMap::PoolInfo::Destroy.
    // This data race is not real because this method can be called only for a valid memory
    // i.e., memory that was initialized and not freed after that.
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    SpaceType space_type = pool_map_.GetSpaceType(ToVoidPtr(ToUintPtr(addr) - GetMinObjectAddress()));
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    ASSERT(space_type != SpaceType::SPACE_TYPE_UNDEFINED);
    return space_type;
}

inline void *MmapMemPool::GetStartAddrPoolForAddrImpl(const void *addr) const
{
    // We optimized this call and expect that it will be used only for object space
    ASSERT(!(ToUintPtr(addr) < GetMinObjectAddress()) || (ToUintPtr(addr) >= GetMaxObjectAddress()));
    void *pool_start_addr = pool_map_.GetFirstByteOfPoolForAddr(ToVoidPtr(ToUintPtr(addr) - GetMinObjectAddress()));
    return ToVoidPtr(ToUintPtr(pool_start_addr) + GetMinObjectAddress());
}

inline size_t MmapMemPool::GetObjectSpaceFreeBytes() const
{
    os::memory::LockHolder lk(lock_);

    size_t unused_bytes = common_space_.GetFreeSpace();
    size_t freed_bytes = common_space_pools_.GetAllSize();
    ASSERT(unused_bytes + freed_bytes <= common_space_.GetMaxSize());
    return unused_bytes + freed_bytes;
}

inline bool MmapMemPool::HaveEnoughPoolsInObjectSpace(size_t pools_num, size_t pool_size) const
{
    os::memory::LockHolder lk(lock_);

    size_t unused_bytes = common_space_.GetFreeSpace();
    ASSERT(pool_size != 0);
    size_t pools = unused_bytes / pool_size;
    if (pools >= pools_num) {
        return true;
    }
    return common_space_pools_.HaveEnoughFreePools(pools_num - pools, pool_size);
}

inline size_t MmapMemPool::GetObjectUsedBytes() const
{
    os::memory::LockHolder lk(lock_);
    ASSERT(common_space_.GetOccupiedMemorySize() >= common_space_pools_.GetAllSize());
    return common_space_.GetOccupiedMemorySize() - common_space_pools_.GetAllSize();
}

inline void MmapMemPool::ReleasePagesInFreePools()
{
    os::memory::LockHolder lk(lock_);
    common_space_pools_.IterateOverFreePools([](size_t pool_size, MmapPool *pool) {
        // Iterate over non returned to OS pools:
        if (!pool->IsReturnedToOS()) {
            pool->SetReturnedToOS(true);
            auto pool_start = ToUintPtr(pool->GetMem());
            LOG_MMAP_MEM_POOL(DEBUG) << "Return pages to OS from Free Pool: start = " << pool->GetMem() << " with size "
                                     << pool_size;
            os::mem::ReleasePages(pool_start, pool_start + pool_size);
        }
    });
    Pool main_pool = common_space_.GetAndClearUnreturnedToOSMemory();
    if (main_pool.GetSize() != 0) {
        auto pool_start = ToUintPtr(main_pool.GetMem());
        LOG_MMAP_MEM_POOL(DEBUG) << "Return pages to OS from common_space: start = " << main_pool.GetMem()
                                 << " with size " << main_pool.GetSize();
        os::mem::ReleasePages(pool_start, pool_start + main_pool.GetSize());
    }
}

#undef LOG_MMAP_MEM_POOL

}  // namespace panda

#endif  // LIBPANDABASE_MEM_MMAP_MEM_POOL_INLINE_H
