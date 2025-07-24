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

#ifndef PANDA_PLUGINS_REFLECT_TYPE_INFO_H
#define PANDA_PLUGINS_REFLECT_TYPE_INFO_H

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

class ReflectTypeInfo : public EtsObject {
public:
    NO_COPY_SEMANTIC(ReflectTypeInfo);
    NO_MOVE_SEMANTIC(ReflectTypeInfo);

    ReflectTypeInfo() = delete;
    ~ReflectTypeInfo() = delete;

    static ReflectTypeInfo *Create(EtsCoroutine *coro, EtsClass *klass)
    {
        return FromEtsObject(EtsObject::Create(coro, klass));
    }

    static ReflectTypeInfo *CreateGivenType(EtsCoroutine *coro, EtsClass *typeToCreate, EtsClass *typeFieldInitializer);

    static ReflectTypeInfo *FromEtsObject(EtsObject *object)
    {
        return static_cast<ReflectTypeInfo *>(object);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    void SetType(EtsCoroutine *coro, EtsClass *klass)
    {
        ASSERT(coro != nullptr);
        ASSERT(klass != nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(ReflectTypeInfo, type_), klass->AsObject()->GetCoreType());
    }

private:
    ObjectPointer<EtsClass> type_;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_REFLECT_TYPE_INFO_H
