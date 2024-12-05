/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_ETS_RUNTIME_ETS_MARK_WORD_H
#define PLUGINS_ETS_RUNTIME_ETS_MARK_WORD_H

#include <cstdint>

#include "libpandabase/macros.h"
#include "runtime/mark_word.h"
#include "plugins/ets/runtime/ets_object_state_info.h"

namespace ark::ets {

class EtsObject;

// 64 bits ets object header for high-end devices: (32 bits pointer)
// |--------------------------------------------------------------------------------------|--------------------|
// |                                   Object Header (64 bits)                            |        State       |
// |-----------------------------------------------------|--------------------------------|--------------------|
// |                 Mark Word (32 bits)                 |      Class Word (32 bits)      |                    |
// |-----------------------------------------------------|--------------------------------|--------------------|
// |         nothing:28         | RB:1 | GC:1 | state:00 |     OOP to metadata object     |       Unlock       |
// |-----------------------------------------------------|--------------------------------|--------------------|
// |           Info:28          | RB:1 | GC:1 | state:01 |     OOP to metadata object     |      Use Info      |
// |-----------------------------------------------------|--------------------------------|--------------------|
// |        Hash:27       | I:1 | RB:1 | GC:1 | state:10 |     OOP to metadata object     |     EtsHashed      |
// |-----------------------------------------------------|--------------------------------|--------------------|
// |    InteropIndex:27   | I:1 | RB:1 | GC:1 | state:10 |     OOP to metadata object     |   HasInteropIndex  |
// |-----------------------------------------------------|--------------------------------|--------------------|
// |           Forwarding address:30          | state:11 |     OOP to metadata object     |         GC         |
// |-----------------------------------------------------|--------------------------------|--------------------|
//
// It is similar to mark word object header except Hash and InteropIndex states.
// `I` - bit that indicate usage of InteropIndex, 1 - HashInteropIndex, 0 - EtsHashed
//
class EtsMarkWord : private MarkWord {
public:
    // EtsMarkWord implement specific logic of HASH usage
    // Big enum
    enum EtsMarkWordRepresentation : MarkWordSize {
        INTEROP_INDEX_FLAG_SIZE = 1UL,
        INTEROP_INDEX_SIZE = HASH_SIZE - INTEROP_INDEX_FLAG_SIZE,
        HASH_SIZE = HASH_SIZE - INTEROP_INDEX_FLAG_SIZE,

        HASH_STATE_ETS_HASH = 0,
        HASH_STATE_INTEROP_INDEX = 1,

        INTEROP_INDEX_FLAG_SHIFT = HASH_STATUS_SHIFT,
        INTEROP_INDEX_FLAG_MASK = (1UL << INTEROP_INDEX_FLAG_SIZE) - 1UL,
        INTEROP_INDEX_FLAG_MASK_IN_PLACE = INTEROP_INDEX_FLAG_MASK << INTEROP_INDEX_FLAG_SHIFT,

        // Interop index state masks and shifts
        INTEROP_INDEX_SHIFT = HASH_STATUS_SHIFT + INTEROP_INDEX_FLAG_SIZE,
        INTEROP_INDEX_MASK = (1UL << INTEROP_INDEX_SIZE) - 1UL,
        INTEROP_INDEX_MASK_IN_PLACE = INTEROP_INDEX_MASK << INTEROP_INDEX_SHIFT,

        // Ets Hash state
        HASH_SHIFT = HASH_STATUS_SHIFT + INTEROP_INDEX_FLAG_SIZE,
        HASH_MASK = (1UL << HASH_SIZE) - 1UL,
        HASH_MASK_IN_PLACE = HASH_MASK << HASH_SHIFT,

        INFO_TABLE_POINTER_SHIFT = MONITOR_POINTER_SHIFT,
        INFO_TABLE_POINTER_MASK = MONITOR_POINTER_MASK,
        INFO_TABLE_POINTER_MASK_IN_PLACE = MONITOR_POINTER_MASK_IN_PLACE,
        INFO_TABLE_POINTER_MAX_COUNT = MONITOR_POINTER_MAX_COUNT,
    };

    enum EtsObjectState {
        STATE_UNLOCKED = MarkWord::STATE_UNLOCKED,
        UNUSED_STATE_STATE_LIGHT_LOCKED = MarkWord::STATE_LIGHT_LOCKED,
        STATE_USE_INFO = MarkWord::STATE_HEAVY_LOCKED,
        STATE_GC = MarkWord::STATE_GC,
        // STATE_HASHED and STATE_HAS_INTEROP_INDEX looks similar for MarkWord, but is different for EtsMarkWord
        STATE_HASHED = MarkWord::STATE_HASHED,
        STATE_HAS_INTEROP_INDEX = (INTEROP_INDEX_FLAG_SIZE << MarkWord::STATUS_SIZE) | MarkWord::STATE_HASHED,
    };

    // MarkWord specific methods
    // Value of Mark Word
    MarkWordSize GetValue() const
    {
        return MarkWord::GetValue();
    }

    // For GC bit
    bool IsMarkedForGC() const
    {
        return MarkWord::IsMarkedForGC();
    };

    EtsMarkWord SetMarkedForGC()
    {
        return EtsMarkWord(MarkWord::SetMarkedForGC());
    };

    EtsMarkWord SetUnMarkedForGC()
    {
        return EtsMarkWord(MarkWord::SetUnMarkedForGC());
    }

    // For read barrier bit
    bool IsReadBarrierSet() const
    {
        return MarkWord::IsReadBarrierSet();
    }

    EtsMarkWord SetReadBarrier()
    {
        return EtsMarkWord(MarkWord::SetReadBarrier());
    }

    EtsMarkWord ClearReadBarrier()
    {
        return EtsMarkWord(MarkWord::ClearReadBarrier());
    }

    // For Info state state
    EtsObjectStateInfo::Id GetInfoId() const
    {
        return static_cast<EtsObjectStateInfo::Id>(MarkWord::GetMonitorId());
    }

    EtsObjectState GetState() const
    {
        auto state = MarkWord::GetState();
        if (state == MarkWord::STATE_HASHED) {
            return GetStateBasedOnMarkHash();
        }
        return static_cast<EtsObjectState>(state);
    }

    EtsMarkWord DecodeFromHash(uint32_t hash)
    {
        return EtsMarkWord::FromMarkWord(
            MarkWord::DecodeFromHash((hash << INTEROP_INDEX_FLAG_SIZE) | HASH_STATE_ETS_HASH));
    }

    EtsMarkWord DecodeFromInteropIndex(uint32_t hash)
    {
        return EtsMarkWord::FromMarkWord(
            MarkWord::DecodeFromHash((hash << INTEROP_INDEX_FLAG_SIZE) | HASH_STATE_INTEROP_INDEX));
    }

    EtsMarkWord DecodeFromInfo(EtsObjectStateInfo::Id id)
    {
        return EtsMarkWord(MarkWord::DecodeFromMonitor(static_cast<Monitor::MonitorId>(id)));
    }

    EtsMarkWord DecodeFromUnlocked()
    {
        return EtsMarkWord(MarkWord::DecodeFromUnlocked());
    }

    uint32_t GetHash() const
    {
        return (GetValue() >> HASH_SHIFT) & HASH_MASK;
    }

    uint32_t GetInteropIndex() const
    {
        return (GetValue() >> INTEROP_INDEX_SHIFT) & INTEROP_INDEX_MASK;
    }

private:
    EtsObjectState GetStateBasedOnMarkHash() const
    {
        auto hashState = MarkWord::GetHash() & INTEROP_INDEX_FLAG_MASK;
        switch (hashState) {
            case HASH_STATE_ETS_HASH: {
                return EtsObjectState::STATE_HASHED;
            }
            case HASH_STATE_INTEROP_INDEX: {
                return EtsObjectState::STATE_HAS_INTEROP_INDEX;
            }
            default: {
                LOG(FATAL, RUNTIME) << "Unsupported Hash State";
                return EtsObjectState::STATE_HASHED;
            }
        }
    }

    // Methods for MarkWord usage
    static EtsMarkWord FromMarkWord(MarkWord word)
    {
        return *(const_cast<EtsMarkWord *>(reinterpret_cast<const EtsMarkWord *>(&word)));
    }

    MarkWord ToMark() const
    {
        return *(const_cast<MarkWord *>(reinterpret_cast<const MarkWord *>(this)));
    }

    explicit EtsMarkWord(const MarkWord &value) : MarkWord(value) {}

    friend EtsObject;
};

static_assert(sizeof(EtsMarkWord) == sizeof(MarkWord));

}  // namespace ark::ets

#endif  // PLUGINS_ETS_RUNTIME_ETS_MARK_WORD_H