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

#ifndef PANDA_MEM_GC_G1_REM_SET_H
#define PANDA_MEM_GC_G1_REM_SET_H

#include <limits>

namespace panda::mem {

namespace test {
class RemSetTest;
}  // namespace test

class RemSetLockConfig {
public:
    using CommonLock = os::memory::RecursiveMutex;
    using DummyLock = os::memory::DummyLock;
};

class Region;

/// @brief Set in the Region. To record the regions and cards reference to this region.
template <typename LockConfigT = RemSetLockConfig::CommonLock>
class RemSet {
public:
    RemSet() = default;
    ~RemSet();

    NO_COPY_SEMANTIC(RemSet);
    NO_MOVE_SEMANTIC(RemSet);

    template <bool NEED_LOCK = true>
    void AddRef(const ObjectHeader *from_obj_addr, size_t offset);

    template <typename RegionPred, typename MemVisitor>
    void Iterate(const RegionPred &region_pred, const MemVisitor &visitor);
    template <typename Visitor>
    void IterateOverObjects(const Visitor &visitor);

    void Clear();

    template <bool NEED_LOCK = true>
    static void InvalidateRegion(Region *invalid_region);

    template <bool NEED_LOCK = true>
    static void InvalidateRefsFromRegion(Region *invalid_region);

    size_t Size() const
    {
        return bitmaps_.size();
    }

    /**
     * Used in the barrier. Record the reference from the region of obj_addr to the region of value_addr.
     * @param obj_addr - address of the object
     * @param offset   - offset in the object where value is stored
     * @param value_addr - address of the reference in the field
     */
    template <bool NEED_LOCK = true>
    static void AddRefWithAddr(const ObjectHeader *obj_addr, size_t offset, const ObjectHeader *value_addr);

    void Dump(std::ostream &out);

    class Bitmap {
    public:
        static constexpr size_t GetBitmapSizeInBytes()
        {
            return sizeof(bitmap_);
        }

        static constexpr size_t GetNumBits()
        {
            return GetBitmapSizeInBytes() * BITS_PER_BYTE;
        }

        void Set(size_t idx)
        {
            size_t elem_idx = idx / ELEM_BITS;
            ASSERT(elem_idx < SIZE);
            size_t bit_offset = idx - elem_idx * ELEM_BITS;
            bitmap_[elem_idx] |= 1ULL << bit_offset;
        }

        template <typename Visitor>
        void Iterate(const MemRange &range, const Visitor &visitor) const
        {
            size_t mem_size = (range.GetEndAddress() - range.GetStartAddress()) / GetNumBits();
            uintptr_t start_addr = range.GetStartAddress();
            for (size_t i = 0; i < SIZE; ++i) {
                uintptr_t addr = start_addr + i * mem_size * ELEM_BITS;
                uint64_t elem = bitmap_[i];
                while (elem > 0) {
                    if (elem & 1ULL) {
                        visitor(MemRange(addr, addr + mem_size));
                    }
                    elem >>= 1ULL;
                    addr += mem_size;
                }
            }
        }

    private:
        static constexpr size_t SIZE = 8U;
        static constexpr size_t ELEM_BITS = std::numeric_limits<uint64_t>::digits;
        std::array<uint64_t, SIZE> bitmap_ {0};
    };

private:
    static size_t GetIdxInBitmap(uintptr_t addr, uintptr_t bitmap_begin_addr);
    template <bool NEED_LOCK>
    PandaUnorderedSet<Region *> *GetRefRegions();
    template <bool NEED_LOCK>
    void AddRefRegion(Region *region);
    template <bool NEED_LOCK>
    void RemoveFromRegion(Region *region);
    template <bool NEED_LOCK>
    void RemoveRefRegion(Region *region);

    LockConfigT rem_set_lock_;
    // TODO(alovkov): make value a Set?
    PandaUnorderedMap<uintptr_t, Bitmap> bitmaps_;
    PandaUnorderedSet<Region *> ref_regions_;

    friend class test::RemSetTest;
};
}  // namespace panda::mem
#endif  // PANDA_MEM_GC_G1_REM_SET_H
