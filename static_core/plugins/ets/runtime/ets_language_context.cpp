/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_itable_builder.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "plugins/ets/runtime/ets_vtable_builder.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/types/ets_error.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include <array>

namespace ark::ets {

void EtsLanguageContext::ThrowException(ManagedThread *thread, const uint8_t *mutf8Name, const uint8_t *mutf8Msg) const
{
    ThrowEtsException(EtsExecutionContext::FromMT(thread), utf::Mutf8AsCString(mutf8Name),
                      utf::Mutf8AsCString(mutf8Msg));
}

PandaUniquePtr<ITableBuilder> EtsLanguageContext::CreateITableBuilder(ClassLinkerErrorHandler *errHandler) const
{
    return MakePandaUnique<EtsITableBuilder>(errHandler);
}

PandaUniquePtr<VTableBuilder> EtsLanguageContext::CreateVTableBuilder(ClassLinkerErrorHandler *errHandler) const
{
    return MakePandaUnique<EtsVTableBuilder>(errHandler);
}

ets::PandaEtsVM *EtsLanguageContext::CreateVM(Runtime *runtime, const RuntimeOptions &options) const
{
    ASSERT(runtime != nullptr);

    auto vm = ets::PandaEtsVM::Create(runtime, options);
    if (!vm) {
        LOG(ERROR, RUNTIME) << vm.Error();
        return nullptr;
    }

    return vm.Value();
}

mem::GC *EtsLanguageContext::CreateGC(mem::GCType gcType, mem::ObjectAllocatorBase *objectAllocator,
                                      const mem::GCSettings &settings) const
{
    return mem::CreateGC<EtsLanguageConfig>(gcType, objectAllocator, settings);
}

void EtsLanguageContext::ThrowStackOverflowException(ManagedThread *thread) const
{
    ASSERT(thread != nullptr);
    EtsExecutionContext *executionCtx = EtsExecutionContext::FromMT(thread);
    EtsClassLinker *classLinker = executionCtx->GetPandaVM()->GetClassLinker();
    const char *classDescriptor = utf::Mutf8AsCString(GetStackOverflowErrorClassDescriptor());
    EtsClass *cls = classLinker->GetClass(classDescriptor, true);
    ASSERT(cls != nullptr);

    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> exc(executionCtx, EtsObject::Create(cls));

    auto *error = EtsError::FromEtsObject(exc.GetPtr());
    if (UNLIKELY(error == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }
    EtsHandle<EtsError> errHandle(executionCtx, error);

    auto *name = cls->GetName();
    if (UNLIKELY(name == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }
    errHandle->SetName(executionCtx, name);

    auto *message = EtsString::CreateNewEmptyString();
    if (UNLIKELY(message == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }
    errHandle->SetMessage(executionCtx, message);

    auto *stackLines = EtsTypedObjectArray<EtsStackTraceElement>::Create(
        PlatformTypes(executionCtx->GetMT())->arkruntimeStackTraceElement, 0U);
    if (UNLIKELY(stackLines == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }
    errHandle->SetStackLines(executionCtx, stackLines);

    executionCtx->GetMT()->SetException(exc.GetPtr()->GetCoreType());
}

VerificationInitAPI EtsLanguageContext::GetVerificationInitAPI() const
{
    VerificationInitAPI vApi;
    vApi.primitiveRootsForVerification = {
        panda_file::Type::TypeId::TAGGED, panda_file::Type::TypeId::VOID, panda_file::Type::TypeId::U1,
        panda_file::Type::TypeId::U8,     panda_file::Type::TypeId::U16,  panda_file::Type::TypeId::U32,
        panda_file::Type::TypeId::U64,    panda_file::Type::TypeId::I8,   panda_file::Type::TypeId::I16,
        panda_file::Type::TypeId::I32,    panda_file::Type::TypeId::I64,  panda_file::Type::TypeId::F32,
        panda_file::Type::TypeId::F64};

    vApi.arrayElementsForVerification = {reinterpret_cast<const uint8_t *>("[Z"),
                                         reinterpret_cast<const uint8_t *>("[B"),
                                         reinterpret_cast<const uint8_t *>("[S"),
                                         reinterpret_cast<const uint8_t *>("[C"),
                                         reinterpret_cast<const uint8_t *>("[I"),
                                         reinterpret_cast<const uint8_t *>("[J"),
                                         reinterpret_cast<const uint8_t *>("[F"),
                                         reinterpret_cast<const uint8_t *>("[D")

    };

    vApi.isNeedClassSyntheticClass = true;
    vApi.isNeedObjectSyntheticClass = false;
    vApi.isNeedStringSyntheticClass = false;

    return vApi;
}

}  // namespace ark::ets
