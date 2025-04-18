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

#ifndef LIBPANDABASE_MEM_ARENA_INL_H
#define LIBPANDABASE_MEM_ARENA_INL_H

#include <memory>
#include "arena.h"
#include "macros.h"
#include "utils/logger.h"
#include "utils/asan_interface.h"

namespace ark {

inline Arena::Arena(size_t buffSize, void *buff) : Arena(buffSize, buff, ARENA_DEFAULT_ALIGNMENT) {}

inline Arena::Arena(size_t buffSize, void *buff, Alignment startAlignment)
    : buff_(buff),
      startPos_(ToVoidPtr(AlignUp(ToUintPtr(buff), GetAlignmentInBytes(startAlignment)))),
      curPos_(startPos_),
      size_(buffSize)
{
    ASSERT(ToUintPtr(buff) == AlignUp(ToUintPtr(buff), GetAlignmentInBytes(ARENA_DEFAULT_ALIGNMENT)));
    ASAN_POISON_MEMORY_REGION(buff_, size_);
    LOG(DEBUG, ALLOC) << "Arena: created with buff addr = " << buff << " size = " << buffSize;
}

inline Arena::~Arena()
{
    ASAN_UNPOISON_MEMORY_REGION(buff_, size_);
    LOG(DEBUG, ALLOC) << "Destroy Arena buff addr = " << buff_ << " size = " << size_;
}

inline void *Arena::Alloc(size_t size, Alignment alignment)
{
    void *ret = nullptr;
    size_t freeSize = GetFreeSize();
    ret = std::align(GetAlignmentInBytes(alignment), size, curPos_, freeSize);
    if (ret != nullptr) {
        ASAN_UNPOISON_MEMORY_REGION(ret, size);
        curPos_ = ToVoidPtr(ToUintPtr(ret) + size);
    }
    LOG(DEBUG, ALLOC) << "Arena::Alloc size = " << size << " alignment = " << alignment << " at addr = " << ret;
    return ret;
}

inline void *Arena::AlignedAlloc(size_t size, [[maybe_unused]] Alignment alignment)
{
    ASSERT(AlignUp(ToUintPtr(curPos_), GetAlignmentInBytes(alignment)) == ToUintPtr(curPos_));
    ASSERT(AlignUp(size, GetAlignmentInBytes(alignment)) == size);
    void *ret = nullptr;
    uintptr_t newCurPos = ToUintPtr(curPos_) + size;
    if (newCurPos <= (ToUintPtr(buff_) + size_)) {
        ret = curPos_;
        curPos_ = ToVoidPtr(newCurPos);
        ASAN_UNPOISON_MEMORY_REGION(ret, size);
    }
    return ret;
}

inline void Arena::LinkTo(Arena *arena)
{
    LOG(DEBUG, ALLOC) << "Link arena " << this << " to " << arena;
    ASSERT(next_ == nullptr);
    next_ = arena;
}

inline void Arena::ClearNextLink()
{
    next_ = nullptr;
}

inline Arena *Arena::GetNextArena() const
{
    return next_;
}

inline size_t Arena::GetFreeSize() const
{
    ASSERT(ToUintPtr(curPos_) >= ToUintPtr(GetStartPos()));
    return size_ - (ToUintPtr(curPos_) - ToUintPtr(GetStartPos()));
}

inline size_t Arena::GetOccupiedSize() const
{
    ASSERT(ToUintPtr(curPos_) >= ToUintPtr(GetStartPos()));
    return ToUintPtr(curPos_) - ToUintPtr(GetStartPos());
}

inline void *Arena::GetArenaEnd() const
{
    return ToVoidPtr(size_ + ToUintPtr(buff_));
}

inline void *Arena::GetAllocatedEnd() const
{
    return ToVoidPtr(ToUintPtr(GetStartPos()) + GetOccupiedSize());
}

inline void *Arena::GetAllocatedStart() const
{
    return GetStartPos();
}

inline bool Arena::InArena(const void *mem) const
{
    return (ToUintPtr(curPos_) > ToUintPtr(mem)) && (ToUintPtr(GetStartPos()) <= ToUintPtr(mem));
}

inline void Arena::Free(void *mem)
{
    ASSERT(InArena(mem));
    ASAN_POISON_MEMORY_REGION(mem, ToUintPtr(curPos_) - ToUintPtr(mem));
    curPos_ = mem;
}

inline void Arena::Resize(size_t newSize)
{
    size_t oldSize = GetOccupiedSize();
    ASSERT(newSize <= oldSize);
    curPos_ = ToVoidPtr(ToUintPtr(GetStartPos()) + newSize);
    ASAN_POISON_MEMORY_REGION(curPos_, oldSize - newSize);
}

inline void Arena::Reset()
{
    Resize(0);
}

inline void Arena::ExpandArena(const void *extraBuff, size_t size)
{
    ASSERT(ToUintPtr(extraBuff) == AlignUp(ToUintPtr(extraBuff), DEFAULT_ALIGNMENT_IN_BYTES));
    ASSERT(ToUintPtr(extraBuff) == ToUintPtr(GetArenaEnd()));
    ASAN_POISON_MEMORY_REGION(extraBuff, size);
    LOG(DEBUG, ALLOC) << "Expand arena: Add " << size << " bytes to the arena " << this;
    size_ += size;
}

}  // namespace ark

#endif  // LIBPANDABASE_MEM_ARENA_INL_H
