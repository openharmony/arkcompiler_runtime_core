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

#ifndef LIBPANDABASE_MEM_ARENA_ALLOCATOR_H
#define LIBPANDABASE_MEM_ARENA_ALLOCATOR_H

#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <utility>
#include <memory>

#include "concepts.h"
#include "mem/base_mem_stats.h"
#include "malloc_mem_pool-inl.h"
#include "mmap_mem_pool-inl.h"
#include "mem.h"
#include "mem_pool.h"
#include "arena-inl.h"

#define USE_MMAP_POOL_FOR_ARENAS

WEAK_FOR_LTO_START

namespace panda {

constexpr size_t DEFAULT_ARENA_SIZE = PANDA_DEFAULT_ARENA_SIZE;
constexpr Alignment DEFAULT_ARENA_ALIGNMENT = LOG_ALIGN_3;
// Buffer for on stack allocation
constexpr size_t ON_STACK_BUFFER_SIZE = 128 * SIZE_1K;
#ifdef FORCE_ARENA_ALLOCATOR_ON_STACK_CACHE
constexpr bool ON_STACK_ALLOCATION_ENABLED = true;
#else
constexpr bool ON_STACK_ALLOCATION_ENABLED = false;
#endif

constexpr size_t DEFAULT_ON_STACK_ARENA_ALLOCATOR_BUFF_SIZE = 128 * SIZE_1K;

template <typename T, bool USE_OOM_HANDLER>
class ArenaAllocatorAdapter;

template <bool USE_OOM_HANDLER = false>
class ArenaAllocatorT {
public:
    using OOMHandler = std::add_pointer_t<void()>;
    template <typename T>
    using AdapterType = ArenaAllocatorAdapter<T, USE_OOM_HANDLER>;

    PANDA_PUBLIC_API explicit ArenaAllocatorT(SpaceType space_type, BaseMemStats *mem_stats = nullptr,
                                              bool limit_alloc_size_by_pool = false);
    ArenaAllocatorT(OOMHandler oom_handler, SpaceType space_type, BaseMemStats *mem_stats = nullptr,
                    bool limit_alloc_size_by_pool = false);

    PANDA_PUBLIC_API ~ArenaAllocatorT();
    ArenaAllocatorT(const ArenaAllocatorT &) = delete;
    ArenaAllocatorT(ArenaAllocatorT &&) = default;
    ArenaAllocatorT &operator=(const ArenaAllocatorT &) = delete;
    ArenaAllocatorT &operator=(ArenaAllocatorT &&) = default;

    [[nodiscard]] PANDA_PUBLIC_API void *Alloc(size_t size, Alignment align = DEFAULT_ARENA_ALIGNMENT);

    template <typename T, typename... Args>
    [[nodiscard]] std::enable_if_t<!std::is_array_v<T>, T *> New(Args &&...args)
    {
        auto p = reinterpret_cast<void *>(Alloc(sizeof(T)));
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        new (p) T(std::forward<Args>(args)...);
        return reinterpret_cast<T *>(p);
    }

    template <typename T>
    [[nodiscard]] std::enable_if_t<is_unbounded_array_v<T>, std::remove_extent_t<T> *> New(size_t size)
    {
        static constexpr size_t SIZE_BEFORE_DATA_OFFSET =
            AlignUp(sizeof(size_t), GetAlignmentInBytes(DEFAULT_ARENA_ALIGNMENT));
        using ElementType = std::remove_extent_t<T>;
        void *p = Alloc(SIZE_BEFORE_DATA_OFFSET + sizeof(ElementType) * size);
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        *static_cast<size_t *>(p) = size;
        auto *data = ToNativePtr<ElementType>(ToUintPtr(p) + SIZE_BEFORE_DATA_OFFSET);
        ElementType *current_element = data;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (size_t i = 0; i < size; ++i, ++current_element) {
            new (current_element) ElementType();
        }
        return data;
    }

    template <typename T>
    [[nodiscard]] T *AllocArray(size_t arr_length);

    ArenaAllocatorAdapter<void, USE_OOM_HANDLER> Adapter();

    PANDA_PUBLIC_API size_t GetAllocatedSize() const;

    /**
     * @brief Set the size of allocated memory to @param new_size.
     *  Free all memory that exceeds @param new_size bytes in the allocator.
     */
    PANDA_PUBLIC_API void Resize(size_t new_size);

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::ARENA_ALLOCATOR;
    }

protected:
    Arena *arenas_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    template <bool USE_ON_STACK_BUFF, typename DummyArg = void>
    class OnStackBuffT {
    public:
        void *Alloc(size_t size, Alignment align = DEFAULT_ARENA_ALIGNMENT)
        {
            size_t free_size = GetFreeSize();
            void *new_pos = cur_pos_;
            void *ret = std::align(GetAlignmentInBytes(align), size, new_pos, free_size);
            if (ret != nullptr) {
                cur_pos_ = static_cast<char *>(ToVoidPtr(ToUintPtr(ret) + size));
            }
            return ret;
        }

        size_t GetFreeSize() const
        {
            return DEFAULT_ON_STACK_ARENA_ALLOCATOR_BUFF_SIZE - (cur_pos_ - &buff_[0]);
        }

        size_t GetOccupiedSize() const
        {
            return cur_pos_ - &buff_[0];
        }

        void Resize(size_t new_size)
        {
            ASSERT(new_size <= GetOccupiedSize());
            cur_pos_ = static_cast<char *>(ToVoidPtr(ToUintPtr(&buff_[0]) + new_size));
        }

    private:
        std::array<char, ON_STACK_BUFFER_SIZE> buff_ {0};
        char *cur_pos_ = &buff_[0];
    };

    template <typename DummyArg>
    class OnStackBuffT<false, DummyArg> {
    public:
        void *Alloc([[maybe_unused]] size_t size, [[maybe_unused]] Alignment align = DEFAULT_ARENA_ALIGNMENT)
        {
            return nullptr;
        }

        size_t GetOccupiedSize() const
        {
            return 0;
        }

        void Resize(size_t new_size)
        {
            (void)new_size;
        }
    };

    /**
     * @brief Adds Arena from MallocMemPool and links it to active
     * @param pool_size size of new pool
     */
    bool AddArenaFromPool(size_t pool_size);

    /**
     * @brief Allocate new element.
     * Try to allocate new element at current arena or try to add new pool to this allocator and allocate element at new
     * pool
     * @param size new element size
     * @param alignment alignment of new element address
     */
    [[nodiscard]] void *AllocateAndAddNewPool(size_t size, Alignment alignment);

    inline void AllocArenaMemStats(size_t size)
    {
        if (mem_stats_ != nullptr) {
            mem_stats_->RecordAllocateRaw(size, space_type_);
        }
    }

    using OnStackBuff = OnStackBuffT<ON_STACK_ALLOCATION_ENABLED>;
    OnStackBuff buff_;
    BaseMemStats *mem_stats_;
    SpaceType space_type_;
    OOMHandler oom_handler_ {nullptr};
    bool limit_alloc_size_by_pool_ {false};
};

using ArenaAllocator = ArenaAllocatorT<false>;
using ArenaAllocatorWithOOMHandler = ArenaAllocatorT<true>;

template <bool USE_OOM_HANDLER>
class ArenaResizeWrapper {
public:
    explicit ArenaResizeWrapper(ArenaAllocatorT<USE_OOM_HANDLER> *arena_allocator)
        : old_size_(arena_allocator->GetAllocatedSize()), allocator_(arena_allocator)
    {
    }

    ~ArenaResizeWrapper()
    {
        allocator_->Resize(old_size_);
    }

private:
    size_t old_size_;
    ArenaAllocatorT<USE_OOM_HANDLER> *allocator_;

    NO_COPY_SEMANTIC(ArenaResizeWrapper);
    NO_MOVE_SEMANTIC(ArenaResizeWrapper);
};

template <bool USE_OOM_HANDLER>
template <typename T>
T *ArenaAllocatorT<USE_OOM_HANDLER>::AllocArray(size_t arr_length)
{
    // NOTE(Dmitrii Trubenkov): change to the proper implementation
    return static_cast<T *>(Alloc(sizeof(T) * arr_length));
}

}  // namespace panda

WEAK_FOR_LTO_END

#endif  // LIBPANDABASE_MEM_ARENA_ALLOCATOR_H
