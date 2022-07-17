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

#ifndef RUNTIME_INCLUDE_MEM_ALLOCATOR_INL_H
#define RUNTIME_INCLUDE_MEM_ALLOCATOR_INL_H

#include "runtime/include/mem/allocator.h"
namespace panda::mem {

template <typename AllocT, bool need_lock>
inline void *ObjectAllocatorBase::AllocateSafe(size_t size, Alignment align, AllocT *object_allocator, size_t pool_size,
                                               SpaceType space_type, HeapSpace *heap_space)
{
    void *mem = object_allocator->template Alloc<need_lock>(size, align);
    if (UNLIKELY(mem == nullptr)) {
        return AddPoolsAndAlloc<AllocT, need_lock>(size, align, object_allocator, pool_size, space_type, heap_space);
    }
    return mem;
}

template <typename AllocT, bool need_lock>
inline void *ObjectAllocatorBase::AddPoolsAndAlloc(size_t size, Alignment align, AllocT *object_allocator,
                                                   size_t pool_size, SpaceType space_type, HeapSpace *heap_space)
{
    void *mem = nullptr;
    static os::memory::Mutex pool_lock;
    os::memory::LockHolder<os::memory::Mutex, need_lock> lock(pool_lock);
    while (true) {
        auto pool = heap_space->TryAllocPool(pool_size, space_type, AllocT::GetAllocatorType(), object_allocator);
        if (UNLIKELY(pool.GetMem() == nullptr)) {
            return nullptr;
        }
        bool added_memory_pool = object_allocator->AddMemoryPool(pool.GetMem(), pool.GetSize());
        if (!added_memory_pool) {
            LOG(FATAL, ALLOC) << "ObjectAllocator: couldn't add memory pool to object allocator";
        }
        mem = object_allocator->template Alloc<need_lock>(size, align);
        if (mem != nullptr) {
            break;
        }
    }
    return mem;
}

template <MTModeT MTMode>
template <bool need_lock>
void *ObjectAllocatorGen<MTMode>::AllocateTenuredImpl(size_t size)
{
    void *mem = nullptr;
    Alignment align = DEFAULT_ALIGNMENT;
    size_t aligned_size = AlignUp(size, GetAlignmentInBytes(align));
    if (aligned_size <= ObjectAllocator::GetMaxSize()) {
        size_t pool_size = std::max(PANDA_DEFAULT_POOL_SIZE, ObjectAllocator::GetMinPoolSize());
        mem = AllocateSafe<ObjectAllocator, need_lock>(size, align, object_allocator_, pool_size,
                                                       SpaceType::SPACE_TYPE_OBJECT, &heap_spaces_);
    } else if (aligned_size <= LargeObjectAllocator::GetMaxSize()) {
        size_t pool_size = std::max(PANDA_DEFAULT_POOL_SIZE, LargeObjectAllocator::GetMinPoolSize());
        mem = AllocateSafe<LargeObjectAllocator, need_lock>(size, align, large_object_allocator_, pool_size,
                                                            SpaceType::SPACE_TYPE_OBJECT, &heap_spaces_);
    } else {
        size_t pool_size = std::max(PANDA_DEFAULT_POOL_SIZE, HumongousObjectAllocator::GetMinPoolSize(size));
        mem = AllocateSafe<HumongousObjectAllocator, need_lock>(size, align, humongous_object_allocator_, pool_size,
                                                                SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, &heap_spaces_);
    }
    return mem;
}

}  // namespace panda::mem

#endif  // RUNTIME_INCLUDE_MEM_ALLOCATOR_INL_H
