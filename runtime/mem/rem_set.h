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

#include "runtime/mem/gc/card_table.h"

namespace panda::mem {

namespace test {
class RemSetTest;
}  // namespace test

using CardPtr = CardTable::CardPtr;
using CardList = PandaVector<CardPtr>;

class RemSetLockConfig {
public:
    using CommonLock = os::memory::RecursiveMutex;
    using DummyLock = os::memory::DummyLock;
};

class Region;

/**
 * \brief Set in the Region. To record the regions and cards reference to this region.
 */
template <typename LockConfigT = RemSetLockConfig::CommonLock>
class RemSet {
public:
    explicit RemSet(Region *region, CardTable *card_table);

    ~RemSet();

    NO_COPY_SEMANTIC(RemSet);
    NO_MOVE_SEMANTIC(RemSet);

    template <bool need_lock = true>
    void AddRef(const ObjectHeader *from_obj_addr);

    template <bool need_lock = true, typename ObjectVisitor>
    void VisitMarkedCards(const ObjectVisitor &object_visitor);

    template <bool need_lock = true, typename CardVisitor>
    void ProceedMarkedCards(const CardVisitor &card_visitor);

    void Clear();

    Region *GetRegion()
    {
        return region_;
    }

    template <bool need_lock = true>
    CardList *GetCardList(Region *region);

    template <bool need_lock = true>
    static void InvalidateRegion(Region *invalid_region);

    /**
     * Used in the barrier. Record the reference from the region of obj_addr to the region of value_addr.
     * @param obj_addr - address of the object
     * @param value_addr - address of the reference in the field
     */
    template <bool need_lock = true>
    static void AddRefWithAddr(const ObjectHeader *obj_addr, const ObjectHeader *value_addr);

    /**
     * Used in the barrier. Record the reference from the region of addr to the region of the reference in it's fields.
     * @param addr - address of the object
     */
    static void TraverseObjectToAddRef(const void *addr);

    void Dump(std::ostream &out);

private:
    friend class test::RemSetTest;

    // used for testing only
    void SetCardTable(CardTable *card_table)
    {
        card_table_ = card_table;
    }

    CardPtr GetCardPtr(const void *addr);
    MemRange GetMemoryRange(CardPtr card);
    template <bool need_lock>
    PandaUnorderedSet<Region *> *GetRefRegions();
    template <bool need_lock>
    void AddRefRegion(Region *region);
    template <bool need_lock>
    void RemoveFromRegion(Region *region);
    template <bool need_lock>
    void RemoveRefRegion(Region *region);

    Region *region_;
    LockConfigT rem_set_lock_;
    // TODO(alovkov): make value a Set?
    PandaUnorderedMap<Region *, CardList *> regions_;
    PandaUnorderedSet<Region *> ref_regions_;
    InternalAllocatorPtr allocator_;

    CardTable *card_table_;
};
}  // namespace panda::mem
#endif  // PANDA_MEM_GC_G1_REM_SET_H
