/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_OBJECTS_STATE_WORD_H
#define COMMON_INTERFACES_OBJECTS_STATE_WORD_H

#include <stddef.h>
#include <cstdint>

namespace panda {
class TypeInfo;
}

namespace panda {
using panda::TypeInfo;
using MAddress = uintptr_t;
class BaseObject;

class ObjectState {
public:
    // Forwarding: object is now being exclusively forwarded by some gc thread or mutator.
    // Forwarded: object is forwarded, thus has 2 versions: from-version and to-verison.
    enum ObjectStateCode : uint8_t {
        INITIAL = 0,  // default state, not-forwarded.
        LOCKED = 1,   // locked/forwarding

        // states for object from-version which has been forwarded.
        FORWARDED = 3,
        TO_VERSION = 4,
    };

    static constexpr size_t STATE_BIT_COUNT = 2;

    // constructure and destructure
    ObjectState()
    {
        SetStateBits(0);
    }
    ObjectState(uint16_t word) : stateBits(word) {}
    ObjectState(ObjectStateCode state) : stateBits(static_cast<uint16_t>(state)) {}
    ObjectState(const ObjectState &state) : stateBits(state.GetStateBits()) {}

    ~ObjectState() = default;

    ObjectState AtomicGetObjectState() const
    {
        return ObjectState(AtomicGetStateBits());
    }

    ObjectStateCode GetStateCode() const
    {
        return static_cast<ObjectStateCode>(stateCode);
    }
    void SetStateCode(ObjectStateCode state)
    {
        stateCode = static_cast<uint16_t>(state);
    }

    bool IsForwardableState() const
    {
        return GetStateCode() == INITIAL;
    }
    bool IsLockedState() const
    {
        return GetStateCode() == LOCKED;
    }
    bool IsForwardedState() const
    {
        return GetStateCode() == FORWARDED;
    }

    union {
        struct {
            // the address of class metadata is at least 8-byte aligned.
            // so the lowest 3 bits can be reused to encode state.
            uint16_t stateCode : STATE_BIT_COUNT;
        };
        uint16_t stateBits;
    };

    uint16_t GetStateBits() const
    {
        return stateBits;
    }
    uint16_t AtomicGetStateBits() const
    {
        return __atomic_load_n(&stateBits, __ATOMIC_ACQUIRE);
    }

    void SetStateBits(uint16_t newState)
    {
        stateBits = newState;
    }
    void AtomicSetStateBits(uint16_t newState)
    {
        __atomic_store_n(&stateBits, newState, __ATOMIC_RELEASE);
    }

    bool CompareExchangeStateBits(uint16_t expected, uint16_t newState)
    {
#if defined(__x86_64__)
        bool success =
            __atomic_compare_exchange_n(&stateBits, &expected, newState, true, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
#else
        // due to "Spurious Failure" of compare_exchange_weak, compare_exchange_strong is chosen.
        bool success =
            __atomic_compare_exchange_n(&stateBits, &expected, newState, false, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);
#endif
        return success;
    }
};

class StateWord {
public:
    static constexpr size_t ADDRESS_BIT_COUNT = 48;
    static constexpr uint64_t ADDRESS_ALIGN_MASK = 8 - 1;

    static constexpr size_t LOW_ADDRESS_BIT_COUNT = 32;
    static constexpr uint64_t LOW_ADDRESS_SHIFT = 0;
    static constexpr uint64_t LOW_ADDRESS_MASK = (1ull << LOW_ADDRESS_BIT_COUNT) - 1;

    static constexpr size_t HIGH_ADDRESS_BIT_COUNT = 16;
    static constexpr uint64_t HIGH_ADDRESS_SHIFT = 32;
    static constexpr uint64_t HIGH_ADDRESS_MASK = (1ull << HIGH_ADDRESS_BIT_COUNT) - 1;

    StateWord() = default;

    void SetTypeInfo(TypeInfo *typeInfo)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(typeInfo);
        SetAddress(address);
    }

    TypeInfo *GetTypeInfo() const
    {
        return reinterpret_cast<TypeInfo *>(GetAddress());
    }

    bool IsValidStateWord() const
    {
        return GetTypeInfo() != nullptr;
    }
    StateWord GetStateWord() const
    {
        return StateWord(addressLow32, addressHigh16, GetObjectState());
    }

    ObjectState GetObjectState() const
    {
        return objectState.AtomicGetObjectState();
    }
    ObjectState::ObjectStateCode GetStateCode() const
    {
        return objectState.GetStateCode();
    }

    bool IsForwardableState() const
    {
        return objectState.IsForwardableState();
    }
    bool IsForwardedState() const
    {
        return objectState.IsForwardedState();
    }

    // bool IsForwardingState() const { return objectState.IsForwardingState(); }
    bool IsLockedWord() const
    {
        return objectState.IsLockedState();
    }
    void SetStateCode(ObjectState::ObjectStateCode state)
    {
        objectState.SetStateCode(state);
    }

    bool TryLockStateWord(const ObjectState current)
    {
        if (current.IsLockedState()) {
            return false;
        }
        return objectState.CompareExchangeStateBits(current.GetStateBits(), ObjectState::LOCKED);
    }

    void UnlockStateWord(const ObjectState newState)
    {
        do {
            ObjectState current = objectState.AtomicGetObjectState();
            if (objectState.CompareExchangeStateBits(current.GetStateBits(), newState.GetStateBits())) {
                return;
            }
        } while (true);
    }

    void SetForwardingPointerExclusive(MAddress fwdPtr)
    {
        SetAddress(fwdPtr);
        objectState = ObjectState::FORWARDED;
    }

    BaseObject *GetForwardingPointer() const
    {
        return reinterpret_cast<BaseObject *>(GetAddress());
    }

private:
    explicit StateWord(uint32_t low32, uint16_t hi16, ObjectState state)
        : addressLow32(low32), addressHigh16(hi16), objectState(state)
    {
    }

    MAddress GetAddress() const
    {
        uintptr_t low = this->addressLow32;
        uintptr_t high = this->addressHigh16;
        uintptr_t address = (high << HIGH_ADDRESS_SHIFT) | low;
        return address;
    }

    void SetAddress(MAddress address)
    {
        this->addressLow32 = (address >> LOW_ADDRESS_SHIFT) & LOW_ADDRESS_MASK;
        this->addressHigh16 = (address >> HIGH_ADDRESS_SHIFT) & HIGH_ADDRESS_MASK;
    }

    // for type info or forwarding pointer.
    uint32_t addressLow32;
    uint16_t addressHigh16;
    ObjectState objectState;
};

static_assert(sizeof(StateWord) == sizeof(uint64_t), "illegal size of StateBits");
}  // namespace panda
#endif  // COMMON_INTERFACES_OBJECTS_STATE_WORD_H
