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
        EtsHandle<EtsEscompatMapEntry> entryHandle(
            coro, FromEtsObject(EtsObject::Create(coro, platformTypes->escompatMapEntry)));
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
        return FromEtsObject(EtsObject::Create(coro, platformTypes->escompatMapEntry));
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

    static constexpr size_t GetPrevOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMapEntry, prev_);
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

    EtsEscompatMapEntry *GetPrev(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetPrevOffset());
        return EtsEscompatMapEntry::FromCoreType(obj);
    }

    EtsEscompatMapEntry *GetNext(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetNextOffset());
        return EtsEscompatMapEntry::FromCoreType(obj);
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

    void SetPrev(EtsCoroutine *coro, EtsEscompatMapEntry *prev)
    {
        ObjectAccessor::SetObject(coro, this, GetPrevOffset(), prev->GetCoreType());
    }

    void SetNext(EtsCoroutine *coro, EtsEscompatMapEntry *next)
    {
        ObjectAccessor::SetObject(coro, this, GetNextOffset(), next->GetCoreType());
    }

private:
    ObjectPointer<EtsEscompatMapEntry> prev_;
    ObjectPointer<EtsEscompatMapEntry> next_;
    ObjectPointer<EtsObject> key_;
    ObjectPointer<EtsObject> val_;

    friend class test::EtsMapTest;
};

class EtsEscompatMap : public EtsObject {
public:
    EtsEscompatMap() = delete;
    ~EtsEscompatMap() = delete;

    NO_COPY_SEMANTIC(EtsEscompatMap);
    NO_MOVE_SEMANTIC(EtsEscompatMap);

    EtsObject *AsObject()
    {
        return this;
    }

    static constexpr size_t GetHeadOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, head_);
    }

    static constexpr size_t GetBucketsOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, buckets_);
    }

    static constexpr size_t GetSizeOffset()
    {
        return MEMBER_OFFSET(EtsEscompatMap, sizeVal_);
    }

    EtsEscompatMapEntry *GetHead(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetHeadOffset());
        return EtsEscompatMapEntry::FromCoreType(obj);
    }

    EtsEscompatArray *GetBuckets(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, GetBucketsOffset());
        return reinterpret_cast<EtsEscompatArray *>(obj);
    }

    EtsInt GetSize()
    {
        return ObjectAccessor::GetPrimitive<EtsInt>(this, GetSizeOffset());
    }

    void SetSize(EtsInt size)
    {
        ObjectAccessor::SetPrimitive(this, GetSizeOffset(), size);
    }

    void SetHead(EtsCoroutine *coro, ObjectHeader *head)
    {
        ObjectAccessor::SetObject(coro, this, GetHeadOffset(), head);
    }

    void SetBuckets(EtsCoroutine *coro, EtsEscompatArray *buckets)
    {
        ASSERT(buckets != nullptr);
        ObjectAccessor::SetObject(coro, this, GetBucketsOffset(), buckets->GetCoreType());
    }

    static EtsBoolean Has(EtsEscompatMap *map, EtsObject *key, EtsInt idx);
    static EtsObject *Get(EtsEscompatMap *map, EtsObject *key, EtsInt idx);
    static EtsBoolean Delete(EtsEscompatMap *map, EtsObject *key, EtsInt idx);
    static uint32_t GetHashCode(EtsObject *&key);

private:
    ObjectPointer<EtsEscompatMapEntry> head_;
    ObjectPointer<EtsEscompatArray> buckets_;
    FIELD_UNUSED EtsInt sizeVal_;

    friend class test::EtsMapTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MAP_H
