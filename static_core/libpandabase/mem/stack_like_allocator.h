/*
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
#ifndef PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_H
#define PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_H

#include "libpandabase/mem/mem.h"
#include "libpandabase/mem/pool_map.h"

namespace panda::mem {
static constexpr size_t STACK_LIKE_ALLOCATOR_DEFAUL_MAX_SIZE = 48_MB;

//                                          Allocation flow looks like that:
//
//  1. Allocate big memory piece via mmap.
//  2. Allocate/Free memory in this preallocated memory piece.
//  3. Return nullptr if we reached the limit of created memory piece.

template <Alignment ALIGNMENT = DEFAULT_FRAME_ALIGNMENT, size_t MAX_SIZE = STACK_LIKE_ALLOCATOR_DEFAUL_MAX_SIZE>
class StackLikeAllocator {
public:
    explicit StackLikeAllocator(bool use_pool_manager = true, SpaceType space_type = SpaceType::SPACE_TYPE_FRAMES);
    ~StackLikeAllocator();
    NO_MOVE_SEMANTIC(StackLikeAllocator);
    NO_COPY_SEMANTIC(StackLikeAllocator);

    template <bool USE_MEMSET = true>
    [[nodiscard]] void *Alloc(size_t size);

    void Free(void *mem);

    /// @brief Returns true if address inside current allocator.
    bool Contains(void *mem);

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::STACK_LIKE_ALLOCATOR;
    }

    size_t GetAllocatedSize() const
    {
        ASSERT(ToUintPtr(free_pointer_) >= ToUintPtr(start_addr_));
        return ToUintPtr(free_pointer_) - ToUintPtr(start_addr_);
    }

    void SetReservedMemorySize(size_t size)
    {
        ASSERT(GetFullMemorySize() >= size);
        reserved_end_addr_ = ToVoidPtr(ToUintPtr(start_addr_) + size);
    }

    void UseWholeMemory()
    {
        end_addr_ = allocated_end_addr_;
    }

    void ReserveMemory()
    {
        ASSERT(reserved_end_addr_ != nullptr);
        end_addr_ = reserved_end_addr_;
    }

    size_t GetFullMemorySize() const
    {
        return ToUintPtr(allocated_end_addr_) - ToUintPtr(start_addr_);
    }

private:
    static constexpr size_t RELEASE_PAGES_SIZE = 256_KB;
    static constexpr size_t RELEASE_PAGES_SHIFT = 18;
    static_assert(RELEASE_PAGES_SIZE == (1U << RELEASE_PAGES_SHIFT));
    static_assert(MAX_SIZE % GetAlignmentInBytes(ALIGNMENT) == 0);
    void *start_addr_ {nullptr};
    void *end_addr_ {nullptr};
    void *free_pointer_ {nullptr};
    bool use_pool_manager_ {false};
    void *reserved_end_addr_ {nullptr};
    void *allocated_end_addr_ {nullptr};
};
}  // namespace panda::mem

#endif  // PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_H
