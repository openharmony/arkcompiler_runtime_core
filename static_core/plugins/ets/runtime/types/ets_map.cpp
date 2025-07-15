/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_map.h"
#include "plugins/ets/runtime/types/ets_base_enum.h"

namespace ark::ets {
static constexpr uint32_t HASH_SIGN_SHIFT = 31;
static const uint8_t *g_hashCodeMethod = utf::CStringAsMutf8("$_hashCode");
static constexpr int NUMERIC_INT_MAX = std::numeric_limits<int>::max();
static constexpr int NUMERIC_INT_MIN = std::numeric_limits<int>::min();

template <typename T>
static inline bool CompareFloats(T a, T b)
{
    if (std::isinf(a) && std::isinf(b)) {
        return std::signbit(a) == std::signbit(b);
    }

    if (std::isnan(a) && std::isnan(b)) {
        return true;
    }

    return a == b;
}

template <typename T>
static inline bool Compare(EtsObject *a, EtsObject *b)
{
    T aVal = EtsBoxPrimitive<T>::FromCoreType(a)->GetValue();
    T bVal = EtsBoxPrimitive<T>::FromCoreType(b)->GetValue();
    return aVal == bVal;
}

static bool CompareBoxed(EtsObject *a, EtsObject *b)
{
    EtsClass *aKlass = a->GetClass();
    EtsClass *bKlass = b->GetClass();

    auto type = aKlass->GetBoxedType();
    switch (type) {
        case EtsClass::BoxedType::INT: {
            EtsInt aVal = EtsBoxPrimitive<EtsInt>::Unbox(a);
            EtsInt bVal = bKlass->IsBoxedDouble() ? static_cast<EtsInt>(EtsBoxPrimitive<EtsDouble>::Unbox(b))
                                                  : EtsBoxPrimitive<EtsInt>::Unbox(b);
            return aVal == bVal;
        }
        case EtsClass::BoxedType::DOUBLE: {
            EtsDouble aVal = EtsBoxPrimitive<EtsDouble>::Unbox(a);
            EtsDouble bVal = bKlass->IsBoxedInt() ? static_cast<EtsDouble>(EtsBoxPrimitive<EtsInt>::Unbox(b))
                                                  : EtsBoxPrimitive<EtsDouble>::Unbox(b);
            return CompareFloats(aVal, bVal);
        }
        case EtsClass::BoxedType::FLOAT:
            return CompareFloats(EtsBoxPrimitive<EtsFloat>::Unbox(a), EtsBoxPrimitive<EtsFloat>::Unbox(b));
        case EtsClass::BoxedType::LONG:
            return Compare<EtsLong>(a, b);
        case EtsClass::BoxedType::SHORT:
            return Compare<EtsShort>(a, b);
        case EtsClass::BoxedType::BYTE:
            return Compare<EtsByte>(a, b);
        case EtsClass::BoxedType::CHAR:
            return Compare<EtsChar>(a, b);
        case EtsClass::BoxedType::BOOLEAN:
            return Compare<EtsBoolean>(a, b);
        default:
            LOG(FATAL, RUNTIME) << "Unknown boxed type";
            UNREACHABLE();
    }

    return false;
}

static bool Compare(EtsObject *a, EtsObject *b)
{
    if (a == b) {
        return true;
    }

    if (a == nullptr || b == nullptr) {
        return false;
    }

    EtsClass *aKlass = a->GetClass();
    EtsClass *bKlass = b->GetClass();
    if (aKlass != bKlass) {
        bool isNumericPair =
            (aKlass->IsBoxedInt() && bKlass->IsBoxedDouble()) || (aKlass->IsBoxedDouble() && bKlass->IsBoxedInt());
        if (!isNumericPair) {
            return false;
        }
    }

    if (aKlass->IsBoxed()) {
        return CompareBoxed(a, b);
    }

    if (aKlass->IsStringClass() && bKlass->IsStringClass()) {
        return EtsString::FromEtsObject(a)->StringsAreEqual(b);
    }

    if (aKlass->IsEtsEnum()) {
        auto aEnumValue = EtsBaseEnum::FromEtsObject(a)->GetValue();
        auto bEnumValue = EtsBaseEnum::FromEtsObject(b)->GetValue();
        return aEnumValue == bEnumValue;
    }

    return false;
}

template <typename T>
static inline uint32_t GetHashFloats(EtsObject *obj)
{
    T val = EtsBoxPrimitive<T>::Unbox(obj);
    // Like ETS.
    if (val > static_cast<T>(NUMERIC_INT_MAX)) {
        return NUMERIC_INT_MAX;
    }

    if (val < static_cast<T>(NUMERIC_INT_MIN)) {
        return NUMERIC_INT_MIN;
    }

    // AARCH64 needs float->int->uint cast to avoid returning 0.
    return static_cast<uint32_t>(static_cast<int32_t>(val));
}

static inline uint32_t CallCustomHashCodeMethod(EtsCoroutine *&coro, EtsEscompatMap *&map, EtsObject *&key,
                                                Method *instanceMethod)
{
    ASSERT(coro->HasPendingException() == false);
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsEscompatMap> mapHandle(coro, map);
    EtsHandle<EtsObject> keyHandle(coro, key);
    std::array args {Value(keyHandle->GetCoreType())};
    auto result = instanceMethod->Invoke(coro, args.data());
    map = mapHandle.GetPtr();
    key = keyHandle.GetPtr();
    coro = EtsCoroutine::GetCurrent();
    return result.GetAs<uint32_t>();
}

static uint32_t GetHashCode(EtsCoroutine *&coro, EtsEscompatMap *&map, EtsObject *&key)
{
    auto *klass = key->GetClass();
    if (klass->IsNullValue()) {
        return 0;
    }

    if (klass->IsBoxed()) {
        auto type = klass->GetBoxedType();
        switch (type) {
            case EtsClass::BoxedType::INT:
                return EtsBoxPrimitive<EtsInt>::Unbox(key);
            case EtsClass::BoxedType::DOUBLE:
                return GetHashFloats<EtsDouble>(key);
            case EtsClass::BoxedType::FLOAT:
                return GetHashFloats<EtsFloat>(key);
            case EtsClass::BoxedType::LONG:
                return EtsBoxPrimitive<EtsLong>::Unbox(key);
            case EtsClass::BoxedType::SHORT:
                return EtsBoxPrimitive<EtsShort>::Unbox(key);
            case EtsClass::BoxedType::BYTE:
                return EtsBoxPrimitive<EtsByte>::Unbox(key);
            case EtsClass::BoxedType::CHAR:
                return EtsBoxPrimitive<EtsChar>::Unbox(key);
            case EtsClass::BoxedType::BOOLEAN:
                return EtsBoxPrimitive<EtsBoolean>::Unbox(key);
            default:
                LOG(FATAL, RUNTIME) << "Unknown boxed type";
                UNREACHABLE();
        }
    }

    if (klass->IsStringClass()) {
        return EtsString::FromEtsObject(key)->GetCoreType()->GetHashcode();
    }

    auto *baseClass = klass->GetBase();
    if (klass->IsEtsEnum() || baseClass == nullptr) {
        return key->GetHashCode();
    }

    auto *baseMethod = klass->GetBase()->GetRuntimeClass()->GetVirtualClassMethod(g_hashCodeMethod);
    auto *instanceMethod = klass->GetRuntimeClass()->GetVirtualClassMethod(g_hashCodeMethod);
    if (baseMethod == instanceMethod && baseClass->GetBase() == nullptr) {
        return key->GetHashCode();
    }

    // We need to call the `$_hashCode` method from ETS for compatibility because some classes can override this method
    // with their own implementation.
    return CallCustomHashCodeMethod(coro, map, key, instanceMethod);
}

static inline uint32_t GetBucketIdx(EtsCoroutine *&coro, EtsEscompatMap *&map, EtsObject *&key)
{
    if (key == nullptr) {
        return 1;  // like ETS.
    }

    // Using int for keyHash to maintain ETS compatibility.
    int32_t keyHash = GetHashCode(coro, map, key);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    uint32_t tmp = keyHash >> HASH_SIGN_SHIFT;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return ((keyHash ^ tmp) - tmp) & (map->GetBuckets(coro)->GetActualLength() - 1);
}

static inline EtsEscompatArray *GetBucket(EtsCoroutine *&coro, EtsEscompatMap *&map, EtsObject *&key)
{
    const uint32_t index = GetBucketIdx(coro, map, key);
    return reinterpret_cast<EtsEscompatArray *>(map->GetBuckets(coro)->GetData()->Get(index));
}

EtsBoolean EtsEscompatMap::Has(EtsEscompatMap *map, EtsObject *key)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *bucket = GetBucket(coro, map, key);
    if (bucket != nullptr) {
        uint32_t bucketSize = bucket->GetActualLength();
        for (uint32_t i = 0; i < bucketSize; ++i) {
            auto *entry = reinterpret_cast<EtsEscompatMapEntry *>(bucket->GetData()->Get(i));
            if (Compare(entry->GetKey(coro), key)) {
                return ToEtsBoolean(true);
            }
        }
    }

    return ToEtsBoolean(false);
}

EtsObject *EtsEscompatMap::Get(EtsEscompatMap *map, EtsObject *key)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *bucket = GetBucket(coro, map, key);
    if (bucket != nullptr) {
        uint32_t bucketSize = bucket->GetActualLength();
        for (uint32_t i = 0; i < bucketSize; ++i) {
            auto *entry = reinterpret_cast<EtsEscompatMapEntry *>(bucket->GetData()->Get(i));
            if (Compare(entry->GetKey(coro), key)) {
                return entry->GetVal(coro);
            }
        }
    }

    return nullptr;
}

EtsBoolean EtsEscompatMap::Delete(EtsEscompatMap *map, EtsObject *key)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *bucket = GetBucket(coro, map, key);
    if (bucket == nullptr) {
        return ToEtsBoolean(false);
    }

    uint32_t bucketSize = bucket->GetActualLength();
    for (uint32_t i = 0; i < bucketSize; ++i) {
        auto *entry = reinterpret_cast<EtsEscompatMapEntry *>(bucket->GetData()->Get(i));
        if (!Compare(entry->GetKey(coro), key)) {
            continue;
        }

        entry->GetPrev(coro)->SetNext(coro, entry->GetNext(coro));
        entry->GetNext(coro)->SetPrev(coro, entry->GetPrev(coro));

        [[maybe_unused]] auto lastEntry = bucket->Pop();
        if (i != (bucketSize--)) {
            bucket->SetRef(i, lastEntry);
        }

        map->SetSize(map->GetSize() - 1);
        if (map->GetSize() == 0) {
            map->SetHead(coro, nullptr);
        }
        return ToEtsBoolean(true);
    }

    return ToEtsBoolean(false);
}

}  // namespace ark::ets
