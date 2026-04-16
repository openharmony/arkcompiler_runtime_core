/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_error_options.h"

namespace ark::ets {

static EtsObject *CreateErrorInstance(EtsExecutionContext *executionCtx, EtsClass *cls, EtsHandle<EtsString> msg,
                                      EtsHandle<EtsObject> pending)
{
    EtsHandle<EtsErrorOptions> errOptions(executionCtx, EtsErrorOptions::Create(executionCtx));
    if (UNLIKELY(errOptions.GetPtr() == nullptr)) {
        return nullptr;
    }
    errOptions->SetCause(pending.GetPtr());

    EtsHandle<EtsObject> error(executionCtx, EtsObject::Create(cls));
    if (UNLIKELY(error.GetPtr() == nullptr)) {
        return nullptr;
    }

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {PlatformTypes()->coreString->GetDescriptor(),
                                                      PlatformTypes()->coreErrorOptions->GetDescriptor()});

    EtsMethod *ctor = cls->GetDirectMethod(panda_file_items::CTOR.data(), proto);
    if (ctor == nullptr) {
        LOG(FATAL, RUNTIME) << "No method " << panda_file_items::CTOR << " in class " << cls->GetDescriptor();
        return nullptr;
    }

    std::array args {Value(error.GetPtr()->GetCoreType()), Value(msg.GetPtr()->GetCoreType()),
                     Value(errOptions.GetPtr()->AsObject()->GetCoreType())};
    EtsMethod::ToRuntimeMethod(ctor)->InvokeVoid(executionCtx->GetMT(), args.data());

    if (UNLIKELY(executionCtx->GetMT()->HasPendingException())) {
        return nullptr;
    }
    return error.GetPtr();
}

static EtsObject *CreateErrorInstanceWithCode(EtsExecutionContext *executionCtx, EtsClass *cls, int32_t errorCode,
                                              EtsHandle<EtsString> msg, EtsHandle<EtsObject> pending)
{
    EtsHandle<EtsErrorOptions> errOptions(executionCtx, EtsErrorOptions::Create(executionCtx));
    if (UNLIKELY(errOptions.GetPtr() == nullptr)) {
        return nullptr;
    }
    errOptions->SetCause(pending.GetPtr());

    EtsHandle<EtsObject> error(executionCtx, EtsObject::Create(cls));
    if (UNLIKELY(error.GetPtr() == nullptr)) {
        return nullptr;
    }

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::I32),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {PlatformTypes()->coreString->GetDescriptor(),
                                                      PlatformTypes()->coreErrorOptions->GetDescriptor()});

    EtsMethod *ctor = cls->GetDirectMethod(panda_file_items::CTOR.data(), proto);
    if (ctor == nullptr) {
        LOG(FATAL, RUNTIME) << "No method " << panda_file_items::CTOR << " in class " << cls->GetDescriptor();
        return nullptr;
    }

    EtsString *name = EtsString::CreateFromMUtf8("Error");
    if (UNLIKELY(name == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    std::array args {Value(error.GetPtr()->GetCoreType()), Value(errorCode), Value(msg.GetPtr()->GetCoreType()),
                     Value(errOptions.GetPtr()->AsObject()->GetCoreType())};

    EtsMethod::ToRuntimeMethod(ctor)->InvokeVoid(executionCtx->GetMT(), args.data());

    if (UNLIKELY(executionCtx->GetMT()->HasPendingException())) {
        return nullptr;
    }
    return error.GetPtr();
}

EtsObject *SetupEtsException(EtsExecutionContext *executionCtx, EtsClass *cls, const char *msg)
{
    ASSERT(PlatformTypes(executionCtx)->coreError->IsAssignableFrom(cls));

    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> pending(executionCtx, EtsObject::FromCoreType(executionCtx->GetMT()->GetException()));
    executionCtx->GetMT()->ClearException();

    EtsHandle<EtsString> etsMsg(executionCtx,
                                msg == nullptr ? EtsString::CreateNewEmptyString() : EtsString::CreateFromMUtf8(msg));
    if (UNLIKELY(etsMsg.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    if (!executionCtx->GetPandaVM()->GetClassLinker()->InitializeClass(executionCtx, cls)) {
        LOG(ERROR, CLASS_LINKER) << "Class " << cls->GetDescriptor() << " cannot be initialized";
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    return CreateErrorInstance(executionCtx, cls, etsMsg, pending);
}

void ThrowEtsException(EtsExecutionContext *executionCtx, EtsClass *cls, const char *msg)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(executionCtx->GetMT() == ManagedThread::GetCurrent());

    if (executionCtx->GetMT()->IsUsePreAllocObj()) {
        executionCtx->GetMT()->SetUsePreAllocObj(false);
        executionCtx->GetMT()->SetException(executionCtx->GetPandaVM()->GetOOMErrorObject());
        return;
    }
    EtsObject *exc = SetupEtsException(executionCtx, cls, msg);
    if (LIKELY(exc != nullptr)) {
        executionCtx->GetMT()->SetException(exc->GetCoreType());
    }
    ASSERT(executionCtx->GetMT()->HasPendingException());
}

void ThrowEtsException(EtsExecutionContext *executionCtx, const char *classDescriptor, const char *msg)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(executionCtx->GetMT() == ManagedThread::GetCurrent());

    if (executionCtx->GetMT()->IsUsePreAllocObj()) {
        executionCtx->GetMT()->SetUsePreAllocObj(false);
        executionCtx->GetMT()->SetException(executionCtx->GetPandaVM()->GetOOMErrorObject());
        return;
    }

    // "mock stdlib" used in gtests is broken
    ASSERT(Runtime::GetOptions().ShouldInitializeIntrinsics());
    EtsClass *cls = executionCtx->GetPandaVM()->GetClassLinker()->GetClass(classDescriptor, true);
    if (cls == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Class " << classDescriptor << " not found";
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }

    ThrowEtsException(executionCtx, cls, msg);
}

void ThrowEtsException(EtsExecutionContext *executionCtx, EtsClass *cls, int32_t errorCode, const char *msg)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(executionCtx->GetMT() == ManagedThread::GetCurrent());

    if (executionCtx->GetMT()->IsUsePreAllocObj()) {
        executionCtx->GetMT()->SetUsePreAllocObj(false);
        executionCtx->GetMT()->SetException(executionCtx->GetPandaVM()->GetOOMErrorObject());
        return;
    }

    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> pending(executionCtx, EtsObject::FromCoreType(executionCtx->GetMT()->GetException()));
    executionCtx->GetMT()->ClearException();

    EtsHandle<EtsString> etsMsg(executionCtx,
                                msg == nullptr ? EtsString::CreateNewEmptyString() : EtsString::CreateFromMUtf8(msg));
    if (UNLIKELY(etsMsg.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }
    if (!executionCtx->GetPandaVM()->GetClassLinker()->InitializeClass(executionCtx, cls)) {
        LOG(ERROR, CLASS_LINKER) << "Class " << cls->GetDescriptor() << " cannot be initialized";
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }
    EtsObject *exc = CreateErrorInstanceWithCode(executionCtx, cls, errorCode, etsMsg, pending);
    if (LIKELY(exc != nullptr)) {
        executionCtx->GetMT()->SetException(exc->GetCoreType());
    }
    ASSERT(executionCtx->GetMT()->HasPendingException());
}

}  // namespace ark::ets
