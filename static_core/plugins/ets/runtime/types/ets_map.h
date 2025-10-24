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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MAP_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MAP_H

#include "ets_escompat_array.h"
#include "ets_object.h"

namespace ark::ets {

namespace test {
class EtsMapTest;
}  // namespace test

class EtsEscompatMapEntry : public EtsObject {
public:
    EtsEscompatMapEntry() = delete;
    ~EtsEscompatMapEntry() = delete;

    NO_COPY_SEMANTIC(EtsEscompatMapEntry);
    NO_MOVE_SEMANTIC(EtsEscompatMapEntry);

    static EtsEscompatMapEntry *Create(EtsCoroutine *coro, EtsHandle<EtsObject> &keyHandle,
                                       EtsHandle<EtsObject> &valHandle)
    {
        ASSERT(coro != nullptr);
        const EtsPlatformTypes *platformTypes = PlatformTypes(coro);
        EtsHandle<EtsEscompatMapEntry> entryHandle(coro,
                                                   FromEtsObject(EtsObject::Create(coro, platformTypes->coreMapEntry)));
        if (UNLIKELY(entryHandle.GetPtr() == nullptr)) {
            return nullptr;
        }

        entryHandle->SetKey(coro, keyHandle.GetPtr());
        entryHandle->SetVal(coro, valHandle.GetPtr());

        return entryHandle.GetPtr();
    }

    static EtsEscompatMapEntry *Create(EtsCoroutine *coro)
    {
        ASSERT(coro != nullptr);
        const EtsPlatformTypes *platformTypes = PlatformTypes(coro);
        return FromEtsObject(EtsObject::Create(coro, platformTypes->coreMapEntry));
    }

    static EtsEscompatMapEntry *FromCoreType(ObjectHeader *obj)
    {
        return reinterpret_cast<EtsEscompatMapEntry *>(obj);
    }

    static EtsEscompatMapEntry *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsEscompatMapEntry *>(etsObj);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    static constexpr size_t GetNextOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMapEntry, next_);
    }

    static constexpr size_t GetKeyOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMapEntry, key_);
    }

    static constexpr size_t GetValOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMapEntry, val_);
    }

    EtsInt GetNext()
    {
        return ObjectAccessor::GetPrimitive<EtsInt>(this, GetNextOffset());
    }

    EtsObject *GetKey(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetKeyOffset());
        return EtsEscompatMapEntry::FromCoreType(obj);
    }

    EtsObject *GetVal(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetValOffset());
        return EtsEscompatMapEntry::FromCoreType(obj);
    }

    void SetVal(EtsCoroutine *coro, EtsObject *val)
    {
        ObjectAccessor::SetObject(coro, this, GetValOffset(), val->GetCoreType());
    }

    void SetKey(EtsCoroutine *coro, EtsObject *key)
    {
        ObjectAccessor::SetObject(coro, this, GetKeyOffset(), key->GetCoreType());
    }

private:
    ObjectPointer<EtsObject> key_;
    ObjectPointer<EtsObject> val_;
    ObjectPointer<EtsInt> next_;

    friend class test::EtsMapTest;
};

class EtsEscompatMap : public EtsObject {
private:
    struct PairOfInts {
        EtsInt int1, int2;
    };
    static constexpr size_t MapIdxFactor = sizeof(PairOfInts) / sizeof(EtsInt);

public:
    using MapIdx = EtsInt;
    static constexpr MapIdx MAP_IDX_END = -1;

    EtsEscompatMap() = delete;
    ~EtsEscompatMap() = delete;

    NO_COPY_SEMANTIC(EtsEscompatMap);
    NO_MOVE_SEMANTIC(EtsEscompatMap);

    EtsObject *AsObject()
    {
        return this;
    }

    static constexpr size_t GetDataOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, data_);
    }

    static constexpr size_t GetBucketsOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, buckets_);
    }

    static constexpr size_t GetCapOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, cap_);
    }

    static constexpr size_t GetNumEntriesOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, numEntries_);
    }

    static constexpr size_t GetSizeValOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, sizeVal_);
    }

    static constexpr size_t GetInitialCapacityOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, initialCapacity_);
    }

    EtsInt GetSize()
    {
        return ObjectAccessor::GetPrimitive<EtsInt>(this, GetSizeValOffset());
    }

    EtsInt GetInitialCapacity() const
    {
        return ObjectAccessor::GetPrimitive<EtsInt>(this, GetInitialCapacityOffset());
    }

    MapIdx GetFirstIdx(EtsCoroutine *coro)
    {
        return GetNextIdx(coro, (0 - 1));
    }

    MapIdx GetNextIdx(EtsCoroutine *coro, MapIdx curIdx)
    {
        const EtsInt numEntries = GetNumEntries();
        if (curIdx < numEntries) {
            EtsIntArray *buckets = GetBuckets(coro);
            while (++curIdx < numEntries) {
                if (buckets->Get(MapIdxFactor * curIdx + 1) != 0) {
                    return curIdx;
                }
            }
        }
        return MAP_IDX_END;
    }

    EtsObject *GetKey(EtsCoroutine *coro, EtsInt idx)
    {
        EtsObjectArray *data = GetData(coro);
        return data->Get(MapIdxFactor * idx + 0);
    }

    EtsObject *GetValue(EtsCoroutine *coro, EtsInt idx)
    {
        EtsObjectArray *data = GetData(coro);
        return data->Get(MapIdxFactor * idx + 1);
    }

    static uint32_t GetHashCode(EtsObject *&key);

private:
    EtsObjectArray *GetData(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetDataOffset());
        return reinterpret_cast<EtsObjectArray *>(obj);
    }

    EtsIntArray *GetBuckets(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetBucketsOffset());
        return reinterpret_cast<EtsIntArray *>(obj);
    }

    EtsInt GetNumEntries()
    {
        return ObjectAccessor::GetPrimitive<EtsInt>(this, GetNumEntriesOffset());
    }

private:
    ObjectPointer<EtsObjectArray> data_;
    ObjectPointer<EtsIntArray> buckets_;
    EtsInt cap_;
    EtsInt numEntries_;
    EtsInt sizeVal_;
    EtsInt initialCapacity_;

    friend class test::EtsMapTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MAP_H
