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

#ifndef _PANDA_VERIFIER_ADDR_MAP_HPP_
#define _PANDA_VERIFIER_ADDR_MAP_HPP_

#include "macros.h"
#include "range.h"
#include "bit_vector.h"

#include <cstdint>

namespace panda::verifier {
class AddrMap {
public:
    AddrMap() = delete;
    AddrMap(const void *start_ptr, const void *end_ptr)
        : AddrMap(reinterpret_cast<uintptr_t>(start_ptr), reinterpret_cast<uintptr_t>(end_ptr))
    {
    }

    AddrMap(const void *start_ptr, size_t size)
        : AddrMap(reinterpret_cast<uintptr_t>(start_ptr), reinterpret_cast<uintptr_t>(start_ptr) + size - 1)
    {
    }

    ~AddrMap() = default;

    DEFAULT_MOVE_SEMANTIC(AddrMap);
    DEFAULT_COPY_SEMANTIC(AddrMap);

    bool IsInAddressSpace(const void *ptr) const
    {
        return IsInAddressSpace(reinterpret_cast<uintptr_t>(ptr));
    }

    template <typename PtrType>
    PtrType AddrStart() const
    {
        return reinterpret_cast<PtrType>(addr_range_.Start());
    }

    template <typename PtrType>
    PtrType AddrEnd() const
    {
        return reinterpret_cast<PtrType>(addr_range_.Finish());
    }

    bool Mark(const void *addr_ptr)
    {
        return Mark(addr_ptr, addr_ptr);
    }

    bool Mark(const void *addr_start_ptr, const void *addr_end_ptr)
    {
        return Mark(reinterpret_cast<uintptr_t>(addr_start_ptr), reinterpret_cast<uintptr_t>(addr_end_ptr));
    }

    void Clear()
    {
        Clear(addr_range_.Start(), addr_range_.Finish());
    }

    bool Clear(const void *addr_ptr)
    {
        return Clear(addr_ptr, addr_ptr);
    }

    bool Clear(const void *addr_start_ptr, const void *addr_end_ptr)
    {
        return Clear(reinterpret_cast<uintptr_t>(addr_start_ptr), reinterpret_cast<uintptr_t>(addr_end_ptr));
    }

    bool HasMark(const void *addr_ptr) const
    {
        return HasMarks(addr_ptr, addr_ptr);
    }

    bool HasMarks(const void *addr_start_ptr, const void *addr_end_ptr) const
    {
        return HasMarks(reinterpret_cast<uintptr_t>(addr_start_ptr), reinterpret_cast<uintptr_t>(addr_end_ptr));
    }

    bool HasCommonMarks(const AddrMap &rhs) const
    {
        // todo: work with different addr spaces
        ASSERT(addr_range_ == rhs.addr_range_);
        return BitVector::LazyAndThenIndicesOf<true>(bit_map_, rhs.bit_map_)().IsValid();
    }

    template <typename PtrType>
    bool GetFirstCommonMark(const AddrMap &rhs, PtrType *ptr) const
    {
        // todo: work with different addr spaces
        ASSERT(addr_range_ == rhs.addr_range_);
        Index<size_t> idx = BitVector::LazyAndThenIndicesOf<true>(bit_map_, rhs.bit_map_)();
        if (idx.IsValid()) {
            size_t offset = idx;
            *ptr = reinterpret_cast<PtrType>(addr_range_.IndexOf(offset));
            return true;
        }
        return false;
    }

    // TODO(vdyadov): optimize this function, push blocks enumeration to bit vector level
    //                and refactor it to work with words and ctlz like intrinsics
    template <typename PtrType, typename Callback>
    void EnumerateMarkedBlocks(Callback cb) const
    {
        PtrType start = nullptr;
        PtrType end = nullptr;
        for (auto addr : addr_range_) {
            uintptr_t bit_offset = addr_range_.OffsetOf(addr);
            if (start == nullptr) {
                if (bit_map_[bit_offset]) {
                    start = reinterpret_cast<PtrType>(addr);
                }
            } else {
                if (bit_map_[bit_offset]) {
                    continue;
                }
                end = reinterpret_cast<PtrType>(addr - 1);
                if (!cb(start, end)) {
                    return;
                }
                start = nullptr;
            }
        }
        if (start != nullptr) {
            end = reinterpret_cast<PtrType>(addr_range_.Finish());
            cb(start, end);
        }
    }

    void InvertMarks()
    {
        bit_map_.Invert();
    }

    template <typename PtrType, typename Handler>
    void EnumerateMarksInScope(const void *addr_start_ptr, const void *addr_end_ptr, Handler handler) const
    {
        EnumerateMarksInScope<PtrType>(reinterpret_cast<uintptr_t>(addr_start_ptr),
                                       reinterpret_cast<uintptr_t>(addr_end_ptr), std::move(handler));
    }

private:
    AddrMap(uintptr_t addr_from, uintptr_t addr_to) : addr_range_ {addr_from, addr_to}, bit_map_ {addr_range_.Length()}
    {
    }

    bool IsInAddressSpace(uintptr_t addr) const
    {
        return addr_range_.Contains(addr);
    }

    bool Mark(uintptr_t addr_from, uintptr_t addr_to)
    {
        if (!addr_range_.Contains(addr_from) || !addr_range_.Contains(addr_to)) {
            return false;
        }
        ASSERT(addr_from <= addr_to);
        bit_map_.Set(addr_range_.OffsetOf(addr_from), addr_range_.OffsetOf(addr_to));
        return true;
    }

    bool Clear(uintptr_t addr_from, uintptr_t addr_to)
    {
        if (!addr_range_.Contains(addr_from) || !addr_range_.Contains(addr_to)) {
            return false;
        }
        ASSERT(addr_from <= addr_to);
        bit_map_.Clr(addr_range_.OffsetOf(addr_from), addr_range_.OffsetOf(addr_to));
        return true;
    }

    bool HasMarks(uintptr_t addr_from, uintptr_t addr_to) const
    {
        if (!addr_range_.Contains(addr_from) || !addr_range_.Contains(addr_to)) {
            return false;
        }
        ASSERT(addr_from <= addr_to);
        Index<size_t> first_mark_idx =
            bit_map_.LazyIndicesOf<1>(addr_range_.OffsetOf(addr_from), addr_range_.OffsetOf(addr_to))();
        return first_mark_idx.IsValid();
    }

    template <typename PtrType, typename Handler>
    void EnumerateMarksInScope(uintptr_t addr_from, uintptr_t addr_to, Handler handler) const
    {
        ASSERT(addr_from <= addr_to);
        auto from = addr_range_.OffsetOf(addr_range_.PutInBounds(addr_from));
        auto to = addr_range_.OffsetOf(addr_range_.PutInBounds(addr_to));
        // upper range point is excluded
        for (uintptr_t idx = from; idx < to; idx++) {
            if (bit_map_[idx]) {
                if (!handler(reinterpret_cast<PtrType>(addr_range_.IndexOf(idx)))) {
                    return;
                }
            }
        }
    }

private:
    Range<uintptr_t> addr_range_;
    BitVector bit_map_;
};
}  // namespace panda::verifier

#endif  // !_PANDA_VERIFIER_ADDR_MAP_HPP_
