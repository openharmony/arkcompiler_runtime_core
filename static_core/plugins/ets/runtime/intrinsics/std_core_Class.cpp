/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "include/object_header.h"
#include "intrinsics.h"
#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/handle_scope-inl.h"

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/runtime/interop_js/interop_context.h"
#endif /* PANDA_ETS_INTEROP_JS */

namespace ark::ets::intrinsics {

EtsString *StdCoreClassGetNameInternal(EtsClass *cls)
{
    return cls->GetName();
}

EtsRuntimeLinker *StdCoreClassGetLinker(EtsClass *cls)
{
    return EtsClassLinkerExtension::GetOrCreateEtsRuntimeLinker(cls->GetLoadContext());
}

EtsClass *StdCoreClassOf(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    return obj->GetClass();
}

EtsClass *StdCoreClassCurrent()
{
    return GetMethodOwnerClassInFrames(EtsCoroutine::GetCurrent(), 0);
}

EtsClass *StdCoreClassOfCaller()
{
    return GetMethodOwnerClassInFrames(EtsCoroutine::GetCurrent(), 1);
}

void StdCoreClassInitialize(EtsClass *cls)
{
    ASSERT(cls != nullptr);

    if (UNLIKELY(!cls->IsInitialized())) {
        auto coro = EtsCoroutine::GetCurrent();
        ASSERT(coro != nullptr);
        EtsClassLinker *linker = coro->GetPandaVM()->GetClassLinker();
        linker->InitializeClass(coro, cls);
    }
}

EtsString *StdCoreClassGetDescriptor(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return EtsString::CreateFromMUtf8(cls->GetDescriptor());
}

EtsObject *StdCoreClassCreateInstance(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->CreateInstance();
}

EtsClass *EtsRuntimeLinkerFindLoadedClass(EtsRuntimeLinker *runtimeLinker, EtsString *clsName)
{
    const auto name = clsName->GetMutf8();
    PandaString descriptor;
    const auto *classDescriptor = ClassHelper::GetDescriptor(utf::CStringAsMutf8(name.c_str()), &descriptor);
    return runtimeLinker->FindLoadedClass(classDescriptor);
}

void EtsRuntimeLinkerInitializeContext(EtsRuntimeLinker *runtimeLinker)
{
    auto *ext = PandaEtsVM::GetCurrent()->GetEtsClassLinkerExtension();
    ext->RegisterContext([ext, runtimeLinker]() {
        ASSERT(runtimeLinker->GetClassLinkerContext() == nullptr);
        auto allocator = ext->GetClassLinker()->GetAllocator();
        auto *ctx = allocator->New<EtsClassLinkerContext>(runtimeLinker);
        runtimeLinker->SetClassLinkerContext(ctx);
        return ctx;
    });
#ifdef PANDA_ETS_INTEROP_JS
    // At first call to ets from js, no available class linker context on the stack
    // Thus, register the first available application class linker context as default class linker context for interop
    // Tracked in #24199
    interop::js::InteropCtx::InitializeDefaultLinkerCtxIfNeeded(runtimeLinker);
#endif /* PANDA_ETS_INTEROP_JS */
}

EtsClass *EtsBootRuntimeLinkerFindAndLoadClass(ObjectHeader *runtimeLinker, EtsString *clsName, EtsBoolean init)
{
    const auto name = clsName->GetMutf8();
    PandaString descriptor;
    auto *classDescriptor = ClassHelper::GetDescriptor(utf::CStringAsMutf8(name.c_str()), &descriptor);

    auto *coro = EtsCoroutine::GetCurrent();
    // Use core ClassLinker in order to pass nullptr as error handler,
    // as exception is thrown in managed RuntimeLinker.loadClass
    auto *linker = Runtime::GetCurrent()->GetClassLinker();
    auto *ctx = EtsRuntimeLinker::FromCoreType(runtimeLinker)->GetClassLinkerContext();
    ASSERT(ctx->IsBootContext());
    auto *klass = linker->GetClass(classDescriptor, true, ctx, nullptr);
    if (klass == nullptr) {
        return nullptr;
    }

    if (UNLIKELY(init != 0 && !klass->IsInitialized())) {
        if (UNLIKELY(!linker->InitializeClass(coro, klass))) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }
    }
    return EtsClass::FromRuntimeClass(klass);
}

EtsRuntimeLinker *EtsGetNearestNonBootRuntimeLinker()
{
    auto *coro = EtsCoroutine::GetCurrent();
    for (auto stack = StackWalker::Create(coro); stack.HasFrame(); stack.NextFrame()) {
        auto *method = stack.GetMethod();
        if (LIKELY(method != nullptr)) {
            auto *cls = method->GetClass();
            ASSERT(cls != nullptr);
            auto *ctx = cls->GetLoadContext();
            ASSERT(ctx != nullptr);
            if (!ctx->IsBootContext()) {
                return reinterpret_cast<EtsClassLinkerContext *>(ctx)->GetRuntimeLinker();
            }
        }
    }
    return nullptr;
}

}  // namespace ark::ets::intrinsics
