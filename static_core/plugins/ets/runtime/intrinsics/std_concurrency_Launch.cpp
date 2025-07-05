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
#include "types/ets_promise.h"
#include "types/ets_typeapi_create.h"
#include "types/ets_type_comptime_traits.h"
#include "mem/vm_handle.h"

#include <type_traits>

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

template <typename CoroResult>
// CC-OFFNXT(huge_method) solid logic
ObjectHeader *Launch(EtsObject *func, EtsArray *arr, bool abortFlag, bool postToMain = false)
{
    static_assert(std::is_same<CoroResult, EtsJob>::value || std::is_same<CoroResult, EtsPromise>::value);

    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
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
    if (arr == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return nullptr;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    VMHandle<EtsArray> array(coro, arr->GetCoreType());
    VMHandle<EtsObject> function(coro, func->GetCoreType());
    EtsMethod *method = ResolveInvokeMethod(coro, function);
    if (method == nullptr) {
        return nullptr;
    }

    // create the coro and put it to the ready queue
    CoroResult *coroResult = CoroResult::Create(coro);
    if (UNLIKELY(coroResult == nullptr)) {
        return nullptr;
    }
    EtsHandle<CoroResult> coroResultHandle(coro, coroResult);

    PandaEtsVM *etsVm = coro->GetPandaVM();
    auto *coroManager = coro->GetCoroutineManager();
    auto ref = etsVm->GetGlobalObjectStorage()->Add(coroResultHandle.GetPtr(), mem::Reference::ObjectType::GLOBAL);
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(ref, coroManager);

    // since transferring arguments from frame registers (which are local roots for GC) to a C++ vector
    // introduces the potential risk of pointer invalidation in case GC moves the referenced objects,
    // we would like to do this transfer below all potential GC invocation points
    PandaVector<Value> realArgs = CreateArgsVector(function, method, array);
    auto mode = postToMain ? CoroutineLaunchMode::MAIN_WORKER : CoroutineLaunchMode::DEFAULT;
    bool launchResult = coro->GetCoroutineManager()->Launch(evt, method->GetPandaMethod(), std::move(realArgs), mode,
                                                            EtsCoroutine::LAUNCH, abortFlag);
    if (UNLIKELY(!launchResult)) {
        // Launch failed. The exception in the current coro should be already set by Launch(),
        // just return null as the result and clean up the allocated resources.
        ASSERT(coro->HasPendingException());
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(evt);
        return nullptr;
    }

    return coroResultHandle.GetPtr();
}

extern "C" {
EtsJob *PostToMainThread(EtsObject *func, EtsArray *arr, EtsBoolean abortFlag)
{
    return static_cast<EtsJob *>(Launch<EtsJob>(func, arr, abortFlag != 0U, true));
}
}

extern "C" {
EtsJob *EtsLaunchInternalJobNative(EtsObject *func, EtsArray *arr, EtsBoolean abortFlag)
{
    return static_cast<EtsJob *>(Launch<EtsJob>(func, arr, abortFlag != 0U));
}

void EtsLaunchSameWorker(EtsObject *callback)
{
    auto *coro = EtsCoroutine::GetCurrent();
    EtsHandleScope scope(coro);
    VMHandle<EtsObject> hCallback(coro, callback->GetCoreType());
    ASSERT(hCallback.GetPtr() != nullptr);
    PandaVector<Value> args = {Value(hCallback->GetCoreType())};
    auto *method = ResolveInvokeMethod(coro, hCallback);
    auto *coroMan = coro->GetCoroutineManager();
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, coroMan);
    [[maybe_unused]] auto launched =
        coroMan->Launch(evt, method->GetPandaMethod(), std::move(args), CoroutineLaunchMode::SAME_WORKER,
                        EtsCoroutine::TIMER_CALLBACK, true);
    ASSERT(launched);
}
}
}  // namespace ark::ets::intrinsics
