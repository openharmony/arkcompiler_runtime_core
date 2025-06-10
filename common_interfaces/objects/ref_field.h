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

#ifndef COMMON_INTERFACES_OBJECTS_REF_FIELD_H
#define COMMON_INTERFACES_OBJECTS_REF_FIELD_H

#include <atomic>
#include <functional>
#include "objects/base_state_word.h"

namespace common {
class BaseObject;

template <bool isAtomic = false>
class RefField {
public:
    static constexpr uint64_t TAG_WEAK = 0x01ULL;
    static constexpr MAddress REF_UNDEFINED = 0x02ULL;
    // size in bytes
    static constexpr size_t GetSize()
    {
        return sizeof(fieldVal);
    }

    BaseObject *GetTargetObject(std::memory_order order = std::memory_order_relaxed) const
    {
        if (isAtomic) {
            MAddress value = __atomic_load_n(&fieldVal, order);
            return reinterpret_cast<BaseObject *>(RefField<>(value).GetAddress() & (~TAG_WEAK));
        } else {
            return reinterpret_cast<BaseObject *>(this->GetAddress() & (~TAG_WEAK));
        }
    }

    MAddress GetFieldValue(std::memory_order order = std::memory_order_relaxed) const
    {
        if (isAtomic) {
            MAddress value = __atomic_load_n(&fieldVal, order);
            return value;
        } else {
            return fieldVal;
        }
    }

    void SetTargetObject(const BaseObject *obj, std::memory_order order = std::memory_order_relaxed)
    {
        RefField<> newField(obj);
        MAddress newVal = newField.GetFieldValue();
        RefFieldValue oldVal = fieldVal;
        (void)oldVal;

        if (isAtomic) {
            __atomic_store_n(&fieldVal, static_cast<RefFieldValue>(newVal), order);
        } else {
            fieldVal = static_cast<RefFieldValue>(newVal);
        }
    }

    void SetFieldValue(MAddress newVal, std::memory_order order = std::memory_order_relaxed)
    {
        RefFieldValue oldVal = fieldVal;
        (void)oldVal;

        if (isAtomic) {
            __atomic_store_n(&fieldVal, static_cast<RefFieldValue>(newVal), order);
        } else {
            fieldVal = static_cast<RefFieldValue>(newVal);
        }
    }

    bool ClearRef(MAddress expectedValue)
    {
        return CompareExchange(expectedValue, REF_UNDEFINED);
    }

    bool CompareExchange(MAddress expectedValue, MAddress newValue,
                         std::memory_order succOrder = std::memory_order_relaxed,
                         std::memory_order failOrder = std::memory_order_relaxed)
    {
        return __atomic_compare_exchange(&fieldVal, &expectedValue, &newValue, false, succOrder, failOrder);
    }

    bool CompareExchange(const BaseObject *expectedObj, const BaseObject *newObj,
                         std::memory_order succOrder = std::memory_order_relaxed,
                         std::memory_order failOrder = std::memory_order_relaxed)
    {
        return CompareExchange(reinterpret_cast<MAddress>(expectedObj), reinterpret_cast<MAddress>(newObj), succOrder,
                               failOrder);
    }

    MAddress Exchange(MAddress newRef, std::memory_order order = std::memory_order_relaxed)
    {
        MAddress ret = 0;
        __atomic_exchange(&fieldVal, &newRef, &ret, order);
        return static_cast<MAddress>(ret);
    }

    MAddress Exchange(const BaseObject *obj, std::memory_order order = std::memory_order_relaxed)
    {
        return Exchange(reinterpret_cast<MAddress>(obj), order);
    }

    MAddress GetAddress() const
    {
        return address;
    }

    bool IsWeak() const
    {
        return (address & TAG_WEAK);
    }

    // bool IsTagged() const { return isTagged == 1; }
    bool IsTagged() const
    {
        return false;
    }
    uint16_t GetTagID() const
    {
        return tagID;
    }

    ~RefField() = default;
    explicit RefField(MAddress val) : fieldVal(val) {}
    RefField(const RefField &ref) : fieldVal(ref.fieldVal) {}
    explicit RefField(const BaseObject *obj) : fieldVal(0)
    {
        address = reinterpret_cast<MAddress>(obj);
    }
    RefField(const BaseObject* obj, bool forWeak) : fieldVal(0)
    {
        MAddress tag = forWeak ? TAG_WEAK : 0;
        address = reinterpret_cast<MAddress>(obj) | tag;
    }
    RefField(const BaseObject *obj, uint16_t tagged, uint16_t tagid)
        : address(reinterpret_cast<MAddress>(obj)), isTagged(tagged), tagID(tagid), padding(0)
    {
    }

    RefField(RefField &&ref) : fieldVal(ref.fieldVal) {}
    RefField() = delete;
    RefField &operator=(const RefField &) = delete;
    RefField &operator=(const RefField &&) = delete;

private:
    using RefFieldValue = MAddress;

    union {
        struct {
            MAddress address : 48;
            MAddress isTagged : 1;
            MAddress tagID : 1;
            MAddress padding : 14;
        };
        RefFieldValue fieldVal;
    };
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_REF_FIELD_H
