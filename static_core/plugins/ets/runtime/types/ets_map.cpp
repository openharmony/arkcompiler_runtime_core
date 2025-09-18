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
#include "plugins/ets/runtime/types/ets_bigint.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "intrinsics.h"

namespace ark::ets {
static constexpr int NUMERIC_INT_MAX = std::numeric_limits<int>::max();
static constexpr int NUMERIC_INT_MIN = std::numeric_limits<int>::min();

static inline bool CompareDouble(EtsObject *a, EtsObject *b)
{
    EtsDouble aVal = EtsBoxPrimitive<EtsDouble>::FromCoreType(a)->GetValue();
    EtsDouble bVal = EtsBoxPrimitive<EtsDouble>::FromCoreType(b)->GetValue();
    if (intrinsics::StdCoreDoubleIsNan(aVal) && intrinsics::StdCoreDoubleIsNan(bVal)) {
        return true;
    }
    return aVal == bVal;
}

static inline bool CompareFloat(EtsObject *a, EtsObject *b)
{
    EtsFloat aVal = EtsBoxPrimitive<EtsFloat>::FromCoreType(a)->GetValue();
    EtsFloat bVal = EtsBoxPrimitive<EtsFloat>::FromCoreType(b)->GetValue();
    if (intrinsics::StdCoreFloatIsNan(aVal) && intrinsics::StdCoreFloatIsNan(bVal)) {
        return true;
    }
    return aVal == bVal;
}

static bool Compare(EtsCoroutine *coro, EtsObject *a, EtsObject *b)
{
    if (UNLIKELY(a == b)) {
        return true;
    }

    if (EtsReferenceNullish(coro, a) || EtsReferenceNullish(coro, b)) {
        return EtsReferenceNullish(coro, a) && EtsReferenceNullish(coro, b);
    }

    EtsClass *aKlass = a->GetClass();
    EtsClass *bKlass = b->GetClass();
    auto ptypes = PlatformTypes(coro);
    if (aKlass == ptypes->coreDouble && bKlass == ptypes->coreDouble) {
        return CompareDouble(a, b);
    }

    if (aKlass == ptypes->coreFloat && bKlass == ptypes->coreFloat) {
        return CompareFloat(a, b);
    }

    return EtsReferenceEquals(coro, a, b);
}

template <typename T>
static uint32_t GetHashFloats(EtsObject *obj)
{
    T val = EtsBoxPrimitive<T>::Unbox(obj);
    // Like ETS.
    if (std::isnan(val)) {
        return 0;
    }
    if (val > static_cast<T>(NUMERIC_INT_MAX)) {
        return NUMERIC_INT_MAX;
    }

    if (val < static_cast<T>(NUMERIC_INT_MIN)) {
        return NUMERIC_INT_MIN;
    }

    // AARCH64 needs float->int->uint cast to avoid returning 0.
    return static_cast<uint32_t>(static_cast<int32_t>(val));
}

uint32_t EtsEscompatMap::GetHashCode(EtsObject *&key)
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

    if (klass->IsBigInt()) {
        return (reinterpret_cast<EtsBigInt *>(key))->GetHashCode();
    }

    return key->GetHashCode();
}

static inline EtsEscompatArray *GetBucket(EtsCoroutine *&coro, EtsEscompatMap *&map, EtsInt idx)
{
    return EtsEscompatArray::FromEtsObject(map->GetBuckets(coro)->GetData()->Get(idx));
}

EtsBoolean EtsEscompatMap::Has(EtsEscompatMap *map, EtsObject *key, EtsInt idx)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro->HasPendingException() == false);
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObject> keyHandle(coro, key);
    EtsHandle<EtsEscompatArray> bucket(coro, GetBucket(coro, map, idx));
    if (bucket.GetPtr() != nullptr) {
        uint32_t bucketSize = bucket->GetActualLength();
        for (uint32_t i = 0; i < bucketSize; ++i) {
            auto *entry = reinterpret_cast<EtsEscompatMapEntry *>(bucket->GetData()->Get(i));
            if (Compare(coro, entry->GetKey(coro), keyHandle.GetPtr())) {
                return ToEtsBoolean(true);
            }
        }
    }

    return ToEtsBoolean(false);
}

EtsObject *EtsEscompatMap::Get(EtsEscompatMap *map, EtsObject *key, EtsInt idx)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro->HasPendingException() == false);
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObject> keyHandle(coro, key);
    EtsHandle<EtsEscompatArray> bucket(coro, GetBucket(coro, map, idx));
    if (bucket.GetPtr() != nullptr) {
        uint32_t bucketSize = bucket->GetActualLength();
        for (uint32_t i = 0; i < bucketSize; ++i) {
            EtsHandle<EtsEscompatMapEntry> entry(coro,
                                                 reinterpret_cast<EtsEscompatMapEntry *>(bucket->GetData()->Get(i)));
            if (Compare(coro, entry->GetKey(coro), keyHandle.GetPtr())) {
                return entry->GetVal(coro);
            }
        }
    }

    return nullptr;
}

EtsBoolean EtsEscompatMap::Delete(EtsEscompatMap *map, EtsObject *key, EtsInt idx)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro->HasPendingException() == false);
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObject> keyHandle(coro, key);
    EtsHandle<EtsEscompatArray> bucket(coro, GetBucket(coro, map, idx));
    if (bucket.GetPtr() == nullptr) {
        return ToEtsBoolean(false);
    }

    uint32_t bucketSize = bucket->GetActualLength();
    for (uint32_t i = 0; i < bucketSize; ++i) {
        EtsHandle<EtsEscompatMapEntry> entry(coro, reinterpret_cast<EtsEscompatMapEntry *>(bucket->GetData()->Get(i)));
        if (!Compare(coro, entry->GetKey(coro), keyHandle.GetPtr())) {
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
