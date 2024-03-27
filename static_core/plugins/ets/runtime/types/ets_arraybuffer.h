/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_ARRAYBUFFER_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_ARRAYBUFFER_H_

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

namespace test {
class EtsArrayBufferTest;
}  // namespace test

class EtsArrayBuffer : public ObjectHeader {
public:
    EtsArrayBuffer() = delete;
    ~EtsArrayBuffer() = delete;

    NO_COPY_SEMANTIC(EtsArrayBuffer);
    NO_MOVE_SEMANTIC(EtsArrayBuffer);

    static EtsArrayBuffer *FromCoreType(ObjectHeader *obj)
    {
        return reinterpret_cast<EtsArrayBuffer *>(obj);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsArrayBuffer *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsArrayBuffer *>(etsObj);
    }

    EtsInt GetByteLength() const
    {
        return byteLength_;
    }

    void SetByteLength(EtsInt length)
    {
        byteLength_ = length;
    }

    EtsByteArray *GetData()
    {
        return data_;
    }

    void SetData(EtsCoroutine *coro, EtsByteArray *data)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsArrayBuffer, data_), data->GetCoreType());
    }

private:
    ObjectPointer<EtsByteArray> data_;
    EtsInt byteLength_;

    friend class test::EtsArrayBufferTest;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_ARRAYBUFFER_H_
