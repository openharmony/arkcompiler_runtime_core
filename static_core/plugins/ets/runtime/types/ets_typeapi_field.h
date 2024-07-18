/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_FIELD_H_
#define PANDA_PLUGINS_ETS_TYPEAPI_FIELD_H_

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "types/ets_primitives.h"
#include "types/ets_typeapi.h"

namespace ark::ets {

class EtsCoroutine;

class EtsTypeAPIField : public ObjectHeader {
public:
    EtsTypeAPIField() = delete;
    ~EtsTypeAPIField() = delete;

    NO_COPY_SEMANTIC(EtsTypeAPIField);
    NO_MOVE_SEMANTIC(EtsTypeAPIField);

    static EtsTypeAPIField *Create(EtsCoroutine *etsCoroutine = EtsCoroutine::GetCurrent());

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsTypeAPIField *FromEtsObject(EtsObject *field)
    {
        return reinterpret_cast<EtsTypeAPIField *>(field);
    }

    void SetTypeDesc(EtsString *td)
    {
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsTypeAPIField, td_), td->AsObject()->GetCoreType());
    }

    void SetOwnerTypeDesc(EtsString *td)
    {
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsTypeAPIField, ownerTd_), td->AsObject()->GetCoreType());
    }

    void SetName(EtsString *name)
    {
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsTypeAPIField, name_), name->AsObject()->GetCoreType());
    }

    void SetAccessMod(EtsTypeAPIAccessModifier accessMod)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsTypeAPIField, accessMod_), accessMod);
    }

    void SetAttributes(EtsInt attr)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsTypeAPIField, attr_), attr);
    }

private:
    ObjectPointer<EtsString> td_;
    ObjectPointer<EtsString> ownerTd_;
    ObjectPointer<EtsString> name_;
    FIELD_UNUSED EtsInt attr_;  // note alignment
    FIELD_UNUSED EtsByte accessMod_;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_TYPEAPI_FIELD_H_