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
#ifndef PANDA_RUNTIME_MEM_GC_G1_CARD_HANDLER_H
#define PANDA_RUNTIME_MEM_GC_G1_CARD_HANDLER_H

#include "runtime/include/object_header.h"
#include "runtime/mem/gc/card_table.h"

namespace panda::mem {
class Region;
class CardHandler {
public:
    explicit CardHandler(CardTable *cardTable) : cardTable_(cardTable) {}

    bool Handle(CardTable::CardPtr cardPtr);

protected:
    virtual bool HandleObject(ObjectHeader *objectHeader, void *begin, void *end) = 0;

    CardTable *GetCardTable()
    {
        return cardTable_;
    }

private:
    CardTable *cardTable_;
};
}  // namespace panda::mem

#endif
