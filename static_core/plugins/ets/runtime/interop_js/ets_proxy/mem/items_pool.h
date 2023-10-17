/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

namespace panda::ets::interop::js::ets_proxy {

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
          data_end_(reinterpret_cast<PaddedItem *>(uintptr_t(data_) + size)),
          current_pos_(reinterpret_cast<PaddedItem *>(data))
    {
        ASSERT(data != nullptr);
        ASSERT(size % PADDED_ITEM_SIZE == 0);
    }
    ~ItemsPool() = default;

    Item *GetNextAlloc() const
    {
        if (free_list_ != nullptr) {
            return &free_list_->item;
        }
        return (current_pos_ < data_end_) ? &current_pos_->item : nullptr;
    }

    Item *AllocItem()
    {
        if (free_list_ != nullptr) {
            PaddedItem *new_item = free_list_;
            free_list_ = free_list_->next;
            return &(new (new_item) PaddedItem())->item;
        }

        if (UNLIKELY(current_pos_ >= data_end_)) {
            // Out of memory
            return nullptr;
        }

        PaddedItem *new_item = current_pos_;
        ++current_pos_;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return &(new (new_item) PaddedItem())->item;
    }

    void FreeItem(Item *item)
    {
        PaddedItem *padded_item = GetPaddedItem(item);
        padded_item->next = free_list_;
        free_list_ = padded_item;
        padded_item->~PaddedItem();
    }

    // NOTE:
    //  This method only checks the validity of the item in the allocated interval
    //  This method does not check whether the item has been allocated or not
    bool IsValidItem(const Item *item) const
    {
        if (UNLIKELY(!IsAligned<alignof(Item)>(uintptr_t(item)))) {
            return false;
        }
        auto addr = uintptr_t(item);
        return uintptr_t(data_) <= addr && addr < uintptr_t(data_end_);
    }

    inline uint32_t GetIndexByItem(Item *item)
    {
        ASSERT(IsValidItem(item));
        ASSERT(uintptr_t(item) % PADDED_ITEM_SIZE == 0);

        PaddedItem *padded_item = GetPaddedItem(item);
        return padded_item - data_;
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
    PaddedItem *const data_end_ {};
    PaddedItem *current_pos_ {};
    PaddedItem *free_list_ {};

    friend testing::ItemsPoolTest;
};

}  // namespace panda::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ITEM_POOL_H_
