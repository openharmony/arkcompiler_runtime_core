/*
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

#include "libpandabase/mem/mem_pool.h"
#include "malloc_mem_pool-inl.h"
#include "mmap_mem_pool-inl.h"
#include "pool_manager.h"
#include "utils/logger.h"

namespace panda {

// default is mmap_mem_pool_
PoolType PoolManager::pool_type_ = PoolType::MMAP;
bool PoolManager::is_initialized_ = false;
MallocMemPool *PoolManager::malloc_mem_pool_ = nullptr;
MmapMemPool *PoolManager::mmap_mem_pool_ = nullptr;

Arena *PoolManager::AllocArena(size_t size, SpaceType space_type, AllocatorType allocator_type,
                               const void *allocator_addr)
{
    if (pool_type_ == PoolType::MMAP) {
        return mmap_mem_pool_->template AllocArenaImpl<Arena, OSPagesAllocPolicy::NO_POLICY>(
            size, space_type, allocator_type, allocator_addr);
    }
    return malloc_mem_pool_->template AllocArenaImpl<Arena, OSPagesAllocPolicy::NO_POLICY>(
        size, space_type, allocator_type, allocator_addr);
}

void PoolManager::FreeArena(Arena *arena)
{
    if (pool_type_ == PoolType::MMAP) {
        return mmap_mem_pool_->template FreeArenaImpl<Arena, OSPagesPolicy::IMMEDIATE_RETURN>(arena);
    }
    return malloc_mem_pool_->template FreeArenaImpl<Arena, OSPagesPolicy::IMMEDIATE_RETURN>(arena);
}

void PoolManager::Initialize(PoolType type)
{
    ASSERT(!is_initialized_);
    is_initialized_ = true;
    pool_type_ = type;
    if (pool_type_ == PoolType::MMAP) {
        mmap_mem_pool_ = new MmapMemPool();
    } else {
        malloc_mem_pool_ = new MallocMemPool();
    }
    LOG(DEBUG, ALLOC) << "PoolManager Initialized";
}

MmapMemPool *PoolManager::GetMmapMemPool()
{
    ASSERT(is_initialized_);
    ASSERT(pool_type_ == PoolType::MMAP);
    return mmap_mem_pool_;
}

MallocMemPool *PoolManager::GetMallocMemPool()
{
    ASSERT(is_initialized_);
    ASSERT(pool_type_ == PoolType::MALLOC);
    return malloc_mem_pool_;
}

void PoolManager::Finalize()
{
    ASSERT(is_initialized_);
    is_initialized_ = false;
    if (pool_type_ == PoolType::MMAP) {
        delete mmap_mem_pool_;
        mmap_mem_pool_ = nullptr;
    } else {
        delete malloc_mem_pool_;
        malloc_mem_pool_ = nullptr;
    }
}

}  // namespace panda
