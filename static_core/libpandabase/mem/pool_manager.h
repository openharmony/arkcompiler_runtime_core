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
#ifndef PANDA_POOL_MANAGER_H
#define PANDA_POOL_MANAGER_H

#include "malloc_mem_pool.h"
#include "mmap_mem_pool.h"
#include "arena-inl.h"

namespace panda {
enum class PoolType { MALLOC, MMAP };

class PoolManager {
public:
    ~PoolManager() = default;
    PoolManager() = default;
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(PoolManager);
    DEFAULT_COPY_SEMANTIC(PoolManager);
    PANDA_PUBLIC_API static void Initialize(PoolType type = PoolType::MMAP);
    PANDA_PUBLIC_API static Arena *AllocArena(size_t size, SpaceType space_type, AllocatorType allocator_type,
                                              const void *allocator_addr = nullptr);
    PANDA_PUBLIC_API static void FreeArena(Arena *arena);
    PANDA_PUBLIC_API static MmapMemPool *GetMmapMemPool();
    static MallocMemPool *GetMallocMemPool();

    PANDA_PUBLIC_API static void Finalize();

private:
    static bool is_initialized_;
    static PoolType pool_type_;
    static MallocMemPool *malloc_mem_pool_;
    static MmapMemPool *mmap_mem_pool_;
};

}  // namespace panda

#endif  // PANDA_POOL_MANAGER_H
