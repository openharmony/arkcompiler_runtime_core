/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_VOID_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_VOID_H_

#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

class EtsVoid : EtsObject {
public:
    EtsVoid() = delete;
    ~EtsVoid() = delete;

    // NOTE: vpukhov. replace with undefined or move to TLS
    static EtsVoid *GetInstance()
    {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        EtsClassLinker *classLinker = coro->GetPandaVM()->GetClassLinker();
        EtsClass *voidClass = classLinker->GetVoidClass();
        ASSERT(voidClass->IsInitialized());  // do not trigger gc!

        EtsField *instanceField = voidClass->GetStaticFieldIDByName("void_instance");
        return reinterpret_cast<EtsVoid *>(voidClass->GetStaticFieldObject(instanceField));
    }

    static void Initialize()
    {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        EtsClassLinker *classLinker = coro->GetPandaVM()->GetClassLinker();
        EtsClass *voidClass = classLinker->GetVoidClass();
        if (!voidClass->IsInitialized()) {
            classLinker->InitializeClass(coro, voidClass);
        }
    }

    static EtsVoid *FromEtsObject(EtsObject *obj)
    {
        ASSERT(obj->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetVoidClass());
        return static_cast<EtsVoid *>(obj);
    }

private:
    NO_COPY_SEMANTIC(EtsVoid);
    NO_MOVE_SEMANTIC(EtsVoid);
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_VOID_H_
