/*
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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "runtime/include/runtime.h"
#include "source_lang_enum.h"
#include "types/ets_array.h"
#include "types/ets_class.h"
#include "types/ets_field.h"
#include "types/ets_method.h"
#include "types/ets_type.h"
#include "types/ets_job.h"
#include "types/ets_typeapi_create.h"
#include "types/ets_type_comptime_traits.h"
#include "mem/vm_handle.h"

namespace ark::ets::intrinsics {
static EtsMethod *ResolveInvokeMethod(EtsCoroutine *coro, VMHandle<EtsObject> func)
{
    if (func.GetPtr() == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return nullptr;
    }

    EtsMethod *method = func->GetClass()->GetInstanceMethod(INVOKE_METHOD_NAME, nullptr);
    ASSERT(method != nullptr);

    if (method->IsAbstract()) {
        method = func->GetClass()->ResolveVirtualMethod(method);
    }

    if (UNLIKELY(!method->GetPandaMethod()->Verify())) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowVerificationException(ctx, method->GetPandaMethod()->GetFullName());
        return nullptr;
    }

    return method;
}

static PandaVector<Value> CreateArgsVector(VMHandle<EtsObject> func, EtsMethod *method, VMHandle<EtsArray> arr)
{
    size_t methArgsCount = method->GetNumArgs();
    PandaVector<Value> realArgs {methArgsCount};
    size_t firstRealArg = 0;

    if (!method->IsStatic()) {
        realArgs[0] = Value(func->GetCoreType());
        firstRealArg = 1;
    }

    size_t numArgs = arr->GetLength();
    if (methArgsCount - firstRealArg != numArgs) {
        UNREACHABLE();
    }

    for (size_t i = 0; i < numArgs; i++) {
        auto arg = arr->GetCoreType()->Get<ObjectHeader *>(i);
        auto argType = method->GetArgType(firstRealArg + i);
        if (argType == EtsType::OBJECT) {
            realArgs[firstRealArg + i] = Value(arg);
            continue;
        }
        EtsPrimitiveTypeEnumToComptimeConstant(argType, [&](auto type) -> void {
            using T = EtsTypeEnumToCppType<decltype(type)::value>;
            realArgs[firstRealArg + i] =
                Value(EtsBoxPrimitive<T>::FromCoreType(EtsObject::FromCoreType(arg))->GetValue());
        });
    }

    return realArgs;
}

extern "C" {
EtsJob *EtsMlaunchInternalNative(EtsObject *func, EtsArray *arr)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (func == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return nullptr;
    }
    if (coro->GetCoroutineManager()->IsCoroutineSwitchDisabled()) {
        ThrowEtsException(coro, panda_file_items::class_descriptors::INVALID_COROUTINE_OPERATION_ERROR,
                          "Cannot launch coroutines in the current context!");
        return nullptr;
    }

    [[maybe_unused]] EtsHandleScope scope(coro);
    VMHandle<EtsArray> array(coro, arr->GetCoreType());
    VMHandle<EtsObject> function(coro, func->GetCoreType());

    if (arr == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return nullptr;
    }

    EtsMethod *method = ResolveInvokeMethod(coro, function);
    if (method == nullptr) {
        return nullptr;
    }

    // create the coro and put it to the ready queue
    EtsJob *job = EtsJob::Create(coro);
    if (UNLIKELY(job == nullptr)) {
        return nullptr;
    }
    EtsHandle<EtsJob> jobHandle(coro, job);

    PandaEtsVM *etsVm = coro->GetPandaVM();
    auto *coroManager = coro->GetCoroutineManager();
    auto jobRef = etsVm->GetGlobalObjectStorage()->Add(job, mem::Reference::ObjectType::GLOBAL);
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(jobRef, coroManager);

    // since transferring arguments from frame registers (which are local roots for GC) to a C++ vector
    // introduces the potential risk of pointer invalidation in case GC moves the referenced objects,
    // we would like to do this transfer below all potential GC invocation points
    PandaVector<Value> realArgs = CreateArgsVector(function, method, array);

    bool launchResult = coro->GetCoroutineManager()->Launch(evt, method->GetPandaMethod(), std::move(realArgs),
                                                            CoroutineLaunchMode::DEFAULT);
    if (UNLIKELY(!launchResult)) {
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(evt);
        ThrowOutOfMemoryError("OOM");
        return nullptr;
    }

    return jobHandle.GetPtr();
}
}
}  // namespace ark::ets::intrinsics
