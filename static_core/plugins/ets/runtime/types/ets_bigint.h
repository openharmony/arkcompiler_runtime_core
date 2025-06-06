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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BIGINT_H
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BIGINT_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets {

namespace test {
class EtsBigIntMembers;
}  // namespace test

class EtsBigInt : public EtsObject {
public:
    static EtsBigInt *FromEtsObject(EtsObject *etsObj)
    {
        ASSERT(etsObj->GetClass()->IsBigInt());
        return reinterpret_cast<EtsBigInt *>(etsObj);
    }

    /* The sign field can have the following values: -1, 0, 1
     * -1 - for negative
     *  0 - for zero value
     *  1 - for positive
     */
    EtsInt GetSign()
    {
        return GetFieldPrimitive<EtsInt>(GetSignOffset());
    }

    EtsIntArray *GetBytes()
    {
        return reinterpret_cast<EtsIntArray *>(GetFieldObject(GetBytesOffset()));
    }

    static constexpr size_t GetBytesOffset()
    {
        return MEMBER_OFFSET(EtsBigInt, bytes_);
    }

    static constexpr size_t GetSignOffset()
    {
        return MEMBER_OFFSET(EtsBigInt, sign_);
    }

    EtsBigInt() = delete;
    ~EtsBigInt() = delete;

private:
    NO_COPY_SEMANTIC(EtsBigInt);
    NO_MOVE_SEMANTIC(EtsBigInt);

    ObjectPointer<EtsIntArray> bytes_;
    EtsInt sign_;

    friend class test::EtsBigIntMembers;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BIGINT_H
