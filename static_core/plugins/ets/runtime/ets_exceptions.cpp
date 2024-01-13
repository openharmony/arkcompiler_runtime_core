/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_exceptions.h"

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace panda::ets {

static EtsClass *GetExceptionClass(EtsCoroutine *coroutine, const char *classDescriptor)
{
    ASSERT(coroutine != nullptr);
    ASSERT(classDescriptor != nullptr);

    EtsClassLinker *classLinker = coroutine->GetPandaVM()->GetClassLinker();
    EtsClass *cls = classLinker->GetClass(classDescriptor, true);
    if (cls == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Class " << classDescriptor << " not found";
        return nullptr;
    }

    if (!classLinker->InitializeClass(coroutine, cls)) {
        LOG(ERROR, CLASS_LINKER) << "Class " << classDescriptor << " cannot be initialized";
        return nullptr;
    }
    return cls;
}

void ThrowEtsException(EtsCoroutine *coroutine, const char *classDescriptor, const char *msg)
{
    ASSERT(coroutine != nullptr);
    ASSERT(coroutine == EtsCoroutine::GetCurrent());

    if (coroutine->IsUsePreAllocObj()) {
        coroutine->SetUsePreAllocObj(false);
        coroutine->SetException(coroutine->GetVM()->GetOOMErrorObject());
        return;
    }

    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> cause(coroutine, EtsObject::FromCoreType(coroutine->GetException()));
    coroutine->ClearException();

    EtsClass *cls = GetExceptionClass(coroutine, classDescriptor);
    if (cls == nullptr) {
        return;
    }

    EtsString *etsMsg = nullptr;
    if (msg != nullptr) {
        etsMsg = EtsString::CreateFromMUtf8(msg);
    } else {
        etsMsg = EtsString::CreateNewEmptyString();
    }
    if (UNLIKELY(etsMsg == nullptr)) {
        // OOM happened during msg allocation
        ASSERT(coroutine->HasPendingException());
        return;
    }

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {panda_file_items::class_descriptors::STRING,
                                                      panda_file_items::class_descriptors::OBJECT});
    EtsMethod *ctor = cls->GetDirectMethod(panda_file_items::CTOR.data(), proto);
    if (ctor == nullptr) {
        LOG(FATAL, RUNTIME) << "No method " << panda_file_items::CTOR << " in class " << classDescriptor;
        return;
    }

    EtsHandle<EtsString> msgHandle(coroutine, etsMsg);
    EtsHandle<EtsObject> excHandle(coroutine, EtsObject::Create(cls));
    // clang-format off
    std::array args {
        Value(excHandle.GetPtr()->GetCoreType()),
        Value(msgHandle.GetPtr()->GetCoreType()),
        Value(cause.GetPtr()->GetCoreType())
    };
    // clang-format on

    EtsMethod::ToRuntimeMethod(ctor)->InvokeVoid(coroutine, args.data());
    if (LIKELY(!coroutine->HasPendingException())) {
        coroutine->SetException(excHandle.GetPtr()->GetCoreType());
    }
}

}  // namespace panda::ets
