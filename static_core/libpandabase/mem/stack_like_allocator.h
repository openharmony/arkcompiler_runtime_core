/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::mem {
static constexpr size_t STACK_LIKE_ALLOCATOR_DEFAUL_MAX_SIZE = 48_MB;

//                                          Allocation flow looks like that:
//
//  1. Allocate big memory piece via mmap.
//  2. Allocate/Free memory in this preallocated memory piece.
//  3. Return nullptr if we reached the limit of created memory piece.

template <Alignment ALIGNMENT = DEFAULT_FRAME_ALIGNMENT, size_t MAX_SIZE = STACK_LIKE_ALLOCATOR_DEFAUL_MAX_SIZE>
class StackLikeAllocator {
public:
    explicit StackLikeAllocator(bool usePoolManager = true, SpaceType spaceType = SpaceType::SPACE_TYPE_FRAMES);
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
        ASSERT(ToUintPtr(freePointer_) >= ToUintPtr(startAddr_));
        return ToUintPtr(freePointer_) - ToUintPtr(startAddr_);
    }

    void SetReservedMemorySize(size_t size)
    {
        ASSERT(GetFullMemorySize() >= size);
        reservedEndAddr_ = ToVoidPtr(ToUintPtr(startAddr_) + size);
    }

    void UseWholeMemory()
    {
        endAddr_ = allocatedEndAddr_;
    }

    void ReserveMemory()
    {
        ASSERT(reservedEndAddr_ != nullptr);
        endAddr_ = reservedEndAddr_;
    }

    size_t GetFullMemorySize() const
    {
        return ToUintPtr(allocatedEndAddr_) - ToUintPtr(startAddr_);
    }

private:
    static constexpr size_t RELEASE_PAGES_SIZE = 256_KB;
    static constexpr size_t RELEASE_PAGES_SHIFT = 18;
    static_assert(RELEASE_PAGES_SIZE == (1U << RELEASE_PAGES_SHIFT));
    static_assert(MAX_SIZE % GetAlignmentInBytes(ALIGNMENT) == 0);
    void *startAddr_ {nullptr};
    void *endAddr_ {nullptr};
    void *freePointer_ {nullptr};
    bool usePoolManager_ {false};
    void *reservedEndAddr_ {nullptr};
    void *allocatedEndAddr_ {nullptr};
};
}  // namespace ark::mem

#endif  // PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_H
