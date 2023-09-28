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

namespace panda::ets {

class EtsVoid : EtsObject {
public:
    EtsVoid() = delete;
    ~EtsVoid() = delete;

    static EtsVoid *GetInstance()
    {
        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        EtsClassLinker *class_linker = coroutine->GetPandaVM()->GetClassLinker();

        EtsClass *builtin_void_class = class_linker->GetClass(panda_file_items::class_descriptors::VOID.data());

        if (!builtin_void_class->IsInitialized()) {
            if (coroutine->HasPendingException()) {
                return nullptr;
            }

            class_linker->InitializeClass(coroutine, builtin_void_class);
        }
        EtsField *instance_field = builtin_void_class->GetStaticFieldIDByName("void_instance");
        return reinterpret_cast<EtsVoid *>(builtin_void_class->GetStaticFieldObject(instance_field));
    }

private:
    NO_COPY_SEMANTIC(EtsVoid);
    NO_MOVE_SEMANTIC(EtsVoid);
};

}  // namespace panda::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_VOID_H_
