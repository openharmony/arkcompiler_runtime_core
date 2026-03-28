/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_REF_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_REF_H

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark {
class ManagedThread;
}  // namespace ark

namespace ark::ets {

namespace test {
class EtsPromiseTest;
}  // namespace test

class EtsPromiseRef : public EtsObject {
public:
    EtsPromiseRef() = delete;
    ~EtsPromiseRef() = delete;

    NO_COPY_SEMANTIC(EtsPromiseRef);
    NO_MOVE_SEMANTIC(EtsPromiseRef);

    PANDA_PUBLIC_API static EtsPromiseRef *Create(EtsExecutionContext *executionCtx);

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsObject *GetTarget(EtsExecutionContext *executionCtx) const
    {
        auto *obj = ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsPromiseRef, target_));
        return EtsObject::FromCoreType(obj);
    }

    void SetTarget(EtsExecutionContext *executionCtx, EtsObject *t)
    {
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsPromiseRef, target_), t->GetCoreType());
    }

private:
    ObjectPointer<EtsObject> target_;

    friend class test::EtsPromiseTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_REF_H
