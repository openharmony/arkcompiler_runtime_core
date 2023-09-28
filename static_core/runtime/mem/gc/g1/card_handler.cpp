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

#include "card_handler.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/rem_set-inl.h"

namespace panda::mem {
bool CardHandler::Handle(CardTable::CardPtr card_ptr)
{
    bool result = true;
    auto *start_address = ToVoidPtr(card_table_->GetCardStartAddress(card_ptr));
    LOG(DEBUG, GC) << "HandleCard card: " << card_table_->GetMemoryRange(card_ptr);

    // clear card before we process it, because parallel mutator thread can make a write and we would miss it
    card_ptr->Clear();

    ASSERT_DO(IsHeapSpace(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(start_address)),
              std::cerr << "Invalid space type for the " << start_address << std::endl);
    auto *region = AddrToRegion(start_address);
    ASSERT(region != nullptr);
    ASSERT(region->GetLiveBitmap() != nullptr);
    auto *end_address = ToVoidPtr(card_table_->GetCardEndAddress(card_ptr));
    auto visitor = [this, &result, start_address, end_address](void *mem) {
        auto object_header = static_cast<ObjectHeader *>(mem);
        if (object_header->ClassAddr<BaseClass>() != nullptr) {
            // Class may be null when we are visiting a card and at the same time a new non-movable
            // object is allocated in the memory region covered by the card.
            result = HandleObject(object_header, start_address, end_address);
            return result;
        }
        return true;
    };
    if (region->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
        region->GetLiveBitmap()->CallForMarkedChunkInHumongousRegion<true>(ToVoidPtr(region->Begin()), visitor);
    } else {
        region->GetLiveBitmap()->IterateOverMarkedChunkInRangeInterruptible<true>(start_address, end_address, visitor);
    }
    return result;
}
}  // namespace panda::mem
