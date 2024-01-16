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

namespace ark::mem {
bool CardHandler::Handle(CardTable::CardPtr cardPtr)
{
    bool result = true;
    auto *startAddress = ToVoidPtr(cardTable_->GetCardStartAddress(cardPtr));
    LOG(DEBUG, GC) << "HandleCard card: " << cardTable_->GetMemoryRange(cardPtr);

    // clear card before we process it, because parallel mutator thread can make a write and we would miss it
    cardPtr->Clear();

    ASSERT_DO(IsHeapSpace(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(startAddress)),
              std::cerr << "Invalid space type for the " << startAddress << std::endl);
    auto *region = AddrToRegion(startAddress);
    ASSERT(region != nullptr);
    ASSERT(region->GetLiveBitmap() != nullptr);
    auto *endAddress = ToVoidPtr(cardTable_->GetCardEndAddress(cardPtr));
    auto visitor = [this, &result, startAddress, endAddress](void *mem) {
        auto objectHeader = static_cast<ObjectHeader *>(mem);
        if (objectHeader->ClassAddr<BaseClass>() != nullptr) {
            // Class may be null when we are visiting a card and at the same time a new non-movable
            // object is allocated in the memory region covered by the card.
            result = HandleObject(objectHeader, startAddress, endAddress);
            return result;
        }
        return true;
    };
    if (region->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
        region->GetLiveBitmap()->CallForMarkedChunkInHumongousRegion<true>(ToVoidPtr(region->Begin()), visitor);
    } else {
        region->GetLiveBitmap()->IterateOverMarkedChunkInRangeInterruptible<true>(startAddress, endAddress, visitor);
    }
    return result;
}
}  // namespace ark::mem
