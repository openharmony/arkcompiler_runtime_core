/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class EtsEscompatMap : public EtsObject {
private:
    struct PairOfInts {
        EtsInt int1, int2;
    };
    static constexpr size_t MAP_IDX_FACTOR = sizeof(PairOfInts) / sizeof(EtsInt);

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

    MapIdx GetFirstIdx(EtsExecutionContext *executionCtx)
    {
        return GetNextIdx(executionCtx, (0 - 1));
    }

    MapIdx GetNextIdx(EtsExecutionContext *executionCtx, MapIdx curIdx)
    {
        const EtsInt numEntries = GetNumEntries();
        if (curIdx < numEntries) {
            EtsIntArray *buckets = GetBuckets(executionCtx);
            while (++curIdx < numEntries) {
                if (buckets->Get(MAP_IDX_FACTOR * curIdx + 1) != 0) {
                    return curIdx;
                }
            }
        }
        return MAP_IDX_END;
    }

    EtsObject *GetKey(EtsExecutionContext *executionCtx, EtsInt idx)
    {
        EtsObjectArray *data = GetData(executionCtx);
        return data->Get(MAP_IDX_FACTOR * idx + 0);
    }

    EtsObject *GetValue(EtsExecutionContext *executionCtx, EtsInt idx)
    {
        EtsObjectArray *data = GetData(executionCtx);
        return data->Get(MAP_IDX_FACTOR * idx + 1);
    }

    static uint32_t GetHashCode(EtsObject *key);

private:
    EtsObjectArray *GetData(EtsExecutionContext *executionCtx)
    {
        auto *obj = ObjectAccessor::GetObject(executionCtx->GetMT(), this, GetDataOffset());
        return reinterpret_cast<EtsObjectArray *>(obj);
    }

    EtsIntArray *GetBuckets(EtsExecutionContext *executionCtx)
    {
        auto *obj = ObjectAccessor::GetObject(executionCtx->GetMT(), this, GetBucketsOffset());
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
