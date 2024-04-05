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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_ARRAYASLIST_INT_H
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_ARRAYASLIST_INT_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"

namespace ark::ets {

namespace test {
class EtsArrayAsListIntMembers;
}  // namespace test

class EtsArrayAsListInt : public EtsObject {
public:
    static EtsArrayAsListInt *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsArrayAsListInt *>(etsObj);
    }

    EtsTypedObjectArray<EtsBoxPrimitive<EtsInt>> *GetData()
    {
        return reinterpret_cast<EtsTypedObjectArray<EtsBoxPrimitive<EtsInt>> *>(GetFieldObject(GetDataOffset()));
    }

    uint32_t GetCurSize()
    {
        return helpers::ToUnsigned(GetFieldPrimitive<EtsInt>(GetCurSizeOffset()));
    }

    static constexpr size_t GetDataOffset()
    {
        return MEMBER_OFFSET(EtsArrayAsListInt, data_);
    }

    static constexpr size_t GetCurSizeOffset()
    {
        return MEMBER_OFFSET(EtsArrayAsListInt, curSize_);
    }

    EtsArrayAsListInt() = delete;
    ~EtsArrayAsListInt() = delete;

private:
    NO_COPY_SEMANTIC(EtsArrayAsListInt);
    NO_MOVE_SEMANTIC(EtsArrayAsListInt);

    ObjectPointer<EtsTypedObjectArray<EtsBoxPrimitive<EtsInt>>> data_;
    EtsInt curSize_;

    friend class test::EtsArrayAsListIntMembers;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_ARRAYASLIST_INT_H
