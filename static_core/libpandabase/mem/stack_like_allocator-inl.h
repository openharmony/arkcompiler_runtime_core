/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_INL_H
#define PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_INL_H

#include <securec.h>
#include "libpandabase/mem/stack_like_allocator.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/asan_interface.h"

namespace panda::mem {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_STACK_LIKE_ALLOCATOR(level) LOG(level, ALLOC) << "StackLikeAllocator: "

template <Alignment ALIGNMENT, size_t MAX_SIZE>
inline StackLikeAllocator<ALIGNMENT, MAX_SIZE>::StackLikeAllocator(bool use_pool_manager, SpaceType space_type)
    : use_pool_manager_(use_pool_manager)
{
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Initializing of StackLikeAllocator";
    ASSERT(RELEASE_PAGES_SIZE == AlignUp(RELEASE_PAGES_SIZE, os::mem::GetPageSize()));
    // MMAP!
    if (use_pool_manager_) {
        start_addr_ = PoolManager::GetMmapMemPool()
                          ->AllocPool(MAX_SIZE, space_type, AllocatorType::STACK_LIKE_ALLOCATOR, this)
                          .GetMem();
    } else {
        start_addr_ = panda::os::mem::MapRWAnonymousWithAlignmentRaw(
            MAX_SIZE, std::max(GetAlignmentInBytes(ALIGNMENT), static_cast<size_t>(panda::os::mem::GetPageSize())));
    }
    if (start_addr_ == nullptr) {
        LOG_STACK_LIKE_ALLOCATOR(FATAL) << "Can't get initial memory";
    }
    free_pointer_ = start_addr_;
    end_addr_ = ToVoidPtr(ToUintPtr(start_addr_) + MAX_SIZE);
    allocated_end_addr_ = end_addr_;
    ASSERT(AlignUp(ToUintPtr(free_pointer_), GetAlignmentInBytes(ALIGNMENT)) == ToUintPtr(free_pointer_));
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Initializing of StackLikeAllocator finished";
}

template <Alignment ALIGNMENT, size_t MAX_SIZE>
inline StackLikeAllocator<ALIGNMENT, MAX_SIZE>::~StackLikeAllocator()
{
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Destroying of StackLikeAllocator";
    if (use_pool_manager_) {
        PoolManager::GetMmapMemPool()->FreePool(start_addr_, MAX_SIZE);
    } else {
        panda::os::mem::UnmapRaw(start_addr_, MAX_SIZE);
    }
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Destroying of StackLikeAllocator finished";
}

template <Alignment ALIGNMENT, size_t MAX_SIZE>
template <bool USE_MEMSET>
inline void *StackLikeAllocator<ALIGNMENT, MAX_SIZE>::Alloc(size_t size)
{
    ASSERT(AlignUp(size, GetAlignmentInBytes(ALIGNMENT)) == size);

    void *ret = nullptr;
    uintptr_t new_cur_pos = ToUintPtr(free_pointer_) + size;
    if (LIKELY(new_cur_pos <= ToUintPtr(end_addr_))) {
        ret = free_pointer_;
        free_pointer_ = ToVoidPtr(new_cur_pos);
        ASAN_UNPOISON_MEMORY_REGION(ret, size);
    } else {
        return nullptr;
    }

    ASSERT(AlignUp(ToUintPtr(ret), GetAlignmentInBytes(ALIGNMENT)) == ToUintPtr(ret));
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (USE_MEMSET) {
        memset_s(ret, size, 0x00, size);
    }
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Allocated memory at addr " << std::hex << ret;
    return ret;
}

template <Alignment ALIGNMENT, size_t MAX_SIZE>
inline void StackLikeAllocator<ALIGNMENT, MAX_SIZE>::Free(void *mem)
{
    ASSERT(ToUintPtr(mem) == AlignUp(ToUintPtr(mem), GetAlignmentInBytes(ALIGNMENT)));
    ASSERT(Contains(mem));
    if ((ToUintPtr(mem) >> RELEASE_PAGES_SHIFT) != (ToUintPtr(free_pointer_) >> RELEASE_PAGES_SHIFT)) {
        // Start address from which we can release pages
        uintptr_t start_addr = AlignUp(ToUintPtr(mem), RELEASE_PAGES_SIZE);
        // We do page release calls each RELEASE_PAGES_SIZE interval,
        // Therefore, we should clear the last RELEASE_PAGES_SIZE interval
        uintptr_t end_addr = AlignUp(ToUintPtr(free_pointer_), RELEASE_PAGES_SIZE);
        os::mem::ReleasePages(start_addr, end_addr);
        LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Release " << std::dec << end_addr - start_addr
                                        << " memory bytes in interval [" << std::hex << start_addr << "; " << end_addr
                                        << "]";
    }
    ASAN_POISON_MEMORY_REGION(mem, ToUintPtr(free_pointer_) - ToUintPtr(mem));
    free_pointer_ = mem;
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Free memory at addr " << std::hex << mem;
}

template <Alignment ALIGNMENT, size_t MAX_SIZE>
inline bool StackLikeAllocator<ALIGNMENT, MAX_SIZE>::Contains(void *mem)
{
    return (ToUintPtr(mem) >= ToUintPtr(start_addr_)) && (ToUintPtr(mem) < ToUintPtr(free_pointer_));
}

#undef LOG_STACK_LIKE_ALLOCATOR
}  // namespace panda::mem

#endif  // PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_INL_H
