/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ITEM_POOL_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ITEM_POOL_H_

#include "libpandabase/macros.h"
#include "libpandabase/utils/math_helpers.h"

namespace ark::ets::interop::js::ets_proxy {

namespace testing {
class ItemsPoolTest;
}  // namespace testing

template <typename Item, uint32_t NR_INDEX_BITS>
class ItemsPool {
    union PaddedItem {
        Item item;
        PaddedItem *next;
        std::array<uint8_t, helpers::math::GetPowerOfTwoValue32(sizeof(Item))> aligned;

        PaddedItem()
        {
            new (&item) Item();
        }
        ~PaddedItem()
        {
            item.~Item();
        }
        NO_COPY_SEMANTIC(PaddedItem);
        NO_MOVE_SEMANTIC(PaddedItem);
    };

    static constexpr size_t MAX_INDEX = 1U << NR_INDEX_BITS;
    static constexpr size_t PADDED_ITEM_SIZE = sizeof(PaddedItem);

    static PaddedItem *GetPaddedItem(Item *item)
    {
        ASSERT(uintptr_t(item) % PADDED_ITEM_SIZE == 0);
        return reinterpret_cast<PaddedItem *>(item);
    }

public:
    static constexpr size_t MAX_POOL_SIZE = (1U << NR_INDEX_BITS) * PADDED_ITEM_SIZE;

    ItemsPool(void *data, size_t size)
        : data_(reinterpret_cast<PaddedItem *>(data)),
          dataEnd_(reinterpret_cast<PaddedItem *>(uintptr_t(data_) + size)),
          currentPos_(reinterpret_cast<PaddedItem *>(data))
    {
        ASSERT(data != nullptr);
        ASSERT(size % PADDED_ITEM_SIZE == 0);
    }
    ~ItemsPool() = default;

    Item *GetNextAlloc() const
    {
        if (freeList_ != nullptr) {
            return &freeList_->item;
        }
        return (currentPos_ < dataEnd_) ? &currentPos_->item : nullptr;
    }

    Item *AllocItem()
    {
        if (freeList_ != nullptr) {
            PaddedItem *newItem = freeList_;
            freeList_ = freeList_->next;
            return &(new (newItem) PaddedItem())->item;
        }

        if (UNLIKELY(currentPos_ >= dataEnd_)) {
            // Out of memory
            return nullptr;
        }

        PaddedItem *newItem = currentPos_;
        ++currentPos_;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return &(new (newItem) PaddedItem())->item;
    }

    void FreeItem(Item *item)
    {
        PaddedItem *paddedItem = GetPaddedItem(item);
        paddedItem->~PaddedItem();
        paddedItem->next = freeList_;
        freeList_ = paddedItem;
    }

    //  This method only checks the validity of the item in the allocated interval
    //  This method does not check whether the item has been allocated or not
    bool IsValidItem(const Item *item) const
    {
        if (UNLIKELY(!IsAligned<alignof(Item)>(uintptr_t(item)))) {
            return false;
        }
        auto addr = uintptr_t(item);
        return uintptr_t(data_) <= addr && addr < uintptr_t(dataEnd_);
    }

    inline uint32_t GetIndexByItem(Item *item)
    {
        ASSERT(IsValidItem(item));
        ASSERT(uintptr_t(item) % PADDED_ITEM_SIZE == 0);

        PaddedItem *paddedItem = GetPaddedItem(item);
        return paddedItem - data_;
    }

    inline Item *GetItemByIndex(uint32_t idx)
    {
        ASSERT(idx < MAX_INDEX);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-union-access)
        return &data_[idx].item;
    }

    NO_COPY_SEMANTIC(ItemsPool);
    NO_MOVE_SEMANTIC(ItemsPool);

private:
    PaddedItem *const data_ {};
    PaddedItem *const dataEnd_ {};
    PaddedItem *currentPos_ {};
    PaddedItem *freeList_ {};

    friend testing::ItemsPoolTest;
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ITEM_POOL_H_
