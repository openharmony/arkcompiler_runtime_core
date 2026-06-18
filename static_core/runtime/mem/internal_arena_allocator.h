/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef RUNTIME_MEM_INTERNAL_ARENA_ALLOCATOR_H
#define RUNTIME_MEM_INTERNAL_ARENA_ALLOCATOR_H

#include "libarkbase/mem/arena.h"
#include "libarkbase/mem/mem.h"
#include "runtime/include/mem/allocator.h"

namespace ark::mem {

class InternalArenaPool;
class InternalArenaAllocator {
    static constexpr size_t INTERNAL_ALLOCATOR_ARENA_SIZE = 128_KB;

public:
    explicit InternalArenaAllocator(InternalAllocatorPtr allocator) : allocator_(allocator) {}

    NO_COPY_SEMANTIC(InternalArenaAllocator);
    NO_MOVE_SEMANTIC(InternalArenaAllocator);

    ~InternalArenaAllocator()
    {
        for (Arena *curr = arenas_; curr != nullptr;) {
            Arena *next = curr->GetNextArena();
            curr->~Arena();
            allocator_->Free(curr);
            curr = next;
        }
    }

    void *Alloc(size_t size)
    {
        if (arenas_ != nullptr) {
            void *ptr = arenas_->Alloc(size);
            if (ptr != nullptr) {
                return ptr;
            }
        }
        size_t minSize = AlignUp(size, sizeof(size_t));
        if (minSize < INTERNAL_ALLOCATOR_ARENA_SIZE - sizeof(Arena)) {
            minSize = INTERNAL_ALLOCATOR_ARENA_SIZE - sizeof(Arena);
        }
        Arena *newArena = AllocArena(minSize);
        if (newArena == nullptr) {
            return nullptr;
        }
        newArena->LinkTo(arenas_);
        arenas_ = newArena;

        return newArena->Alloc(size);
    }

    void Free([[maybe_unused]] void *ptr, [[maybe_unused]] size_t size) {}

    template <typename T>
    [[nodiscard]] T *AllocArray(size_t size)
    {
        return reinterpret_cast<T *>(Alloc(sizeof(T) * size));
    }

    InternalAllocatorPtr GetInternalAllocator() const
    {
        return allocator_;
    }

    template <typename T, typename... Args>
    [[nodiscard]] std::enable_if_t<!std::is_array_v<T>, T *> New(Args &&...args)
    {
        auto p = Alloc(sizeof(T));
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        return new (p) T(std::forward<Args>(args)...);
    }

    template <typename T>
    void MakeContainer(T *&target)
    {
        target = NewContainer<T>();
    }

    template <typename T>
    T *NewContainer()
    {
        return New<T>(Adapter<typename T::value_type>(*this));
    }

    template <typename T>
    class Adapter {
    public:
        using value_type = T;               // NOLINT(readability-identifier-naming)
        using pointer = T *;                // NOLINT(readability-identifier-naming)
        using reference = T &;              // NOLINT(readability-identifier-naming)
        using const_pointer = const T *;    // NOLINT(readability-identifier-naming)
        using const_reference = const T &;  // NOLINT(readability-identifier-naming)
        using size_type = size_t;           // NOLINT(readability-identifier-naming)
        using difference_type = ptrdiff_t;  // NOLINT(readability-identifier-naming)

        // NOLINTNEXTLINE(*-explicit-constructor)
        Adapter(InternalArenaAllocator &allocator) : allocator_(allocator) {}

        template <typename U>
        // NOLINTNEXTLINE(*-explicit-constructor)
        Adapter(const Adapter<U> &that) : allocator_(that.allocator_)
        {
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        pointer allocate()
        {
            return (pointer)allocator_.Alloc(sizeof(T));
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        pointer allocate(size_type n)
        {
            return (pointer)allocator_.Alloc(n * sizeof(T));
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        void deallocate([[maybe_unused]] pointer ptr, [[maybe_unused]] size_type size) {}

        template <typename U>
        bool operator==(const Adapter<U> &that) const
        {
            return &allocator_ == &that.allocator_;
        }

        template <typename U>
        bool operator!=(const Adapter<U> &that) const
        {
            return &allocator_ != &that.allocator_;
        }

    private:
        InternalArenaAllocator &allocator_;

        template <typename U>
        friend class Adapter;
    };

    size_t GetAllocatedSize();

    void Resize(size_t newSize);

private:
    InternalAllocatorPtr allocator_;
    Arena *arenas_ {nullptr};

    Arena *AllocArena(size_t size)
    {
        static constexpr size_t ARENA_ALIGNMENT = GetAlignmentInBytes(ARENA_DEFAULT_ALIGNMENT);
        static constexpr size_t MAX_ALIGNMENT_DRIFT = ARENA_ALIGNMENT - 1U;
        void *ptr = allocator_->Alloc(size + sizeof(Arena) + MAX_ALIGNMENT_DRIFT, ARENA_DEFAULT_ALIGNMENT);
        if (ptr == nullptr) {
            return nullptr;
        }

        uintptr_t arenaAddr = ToUintPtr(ptr);
        uintptr_t buffAddr = AlignUp(arenaAddr + sizeof(Arena), ARENA_ALIGNMENT);
        size_t sizeForBuff = size + sizeof(Arena) + MAX_ALIGNMENT_DRIFT - (buffAddr - arenaAddr);
        ASSERT(sizeForBuff >= size);
        return new (ptr) Arena(sizeForBuff, ToVoidPtr(buffAddr));
    }
};

class InternalArenaPool {
public:
    InternalArenaPool() = default;
    ~InternalArenaPool()
    {
        Clear();
    }

    NO_COPY_SEMANTIC(InternalArenaPool);
    NO_MOVE_SEMANTIC(InternalArenaPool);

    InternalArenaAllocator *Get(InternalAllocatorPtr allocator);

    void Release(InternalArenaAllocator *allocator);

    void Clear();

private:
    void SetArenaPool(InternalArenaAllocator *allocator);

    std::atomic<InternalArenaAllocator *> pool_ {nullptr};
};

}  // namespace ark::mem

#endif
