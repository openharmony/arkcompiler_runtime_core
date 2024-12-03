/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "runtime/handle_scope-inl.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets::intrinsics {

EtsString *StdCoreClassGetNameInternal(EtsClass *cls)
{
    return cls->GetName();
}

EtsClass *StdCoreClassOf(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    return obj->GetClass();
}

EtsClass *StdCoreClassOfCaller()
{
    return GetMethodOwnerClassInFrames(EtsCoroutine::GetCurrent(), 0);
}

void StdCoreClassInitialize(EtsClass *cls)
{
    ASSERT(cls != nullptr);

    if (UNLIKELY(!cls->IsInitialized())) {
        auto coro = EtsCoroutine::GetCurrent();
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
    auto coro = EtsCoroutine::GetCurrent();
    const auto throwCreateInstanceErr = [coro, cls](std::string_view msg) {
        ets::ThrowEtsException(coro, panda_file_items::class_descriptors::ERROR,
                               PandaString(msg) + " " + cls->GetDescriptor());
    };

    if (UNLIKELY(!cls->GetRuntimeClass()->IsInstantiable() || cls->IsArrayClass())) {
        throwCreateInstanceErr("Cannot instantiate");
        return nullptr;
    }

    EtsMethod *ctor = cls->GetDirectMethod(panda_file_items::CTOR.data(), ":V");
    if (UNLIKELY(ctor == nullptr)) {
        throwCreateInstanceErr("No default constructor in");
        return nullptr;
    }

    EtsClassLinker *linker = coro->GetPandaVM()->GetClassLinker();
    if (UNLIKELY(!cls->IsInitialized() && !linker->InitializeClass(coro, cls))) {
        return nullptr;
    }
    EtsObject *obj = EtsObject::Create(cls);
    if (UNLIKELY(obj == nullptr)) {
        return nullptr;
    }

    LocalObjectHandle objHandle(coro, obj);
    std::array<Value, 1> args {Value(obj->GetCoreType())};
    ctor->GetPandaMethod()->Invoke(coro, args.data());
    if (UNLIKELY(coro->HasPendingException())) {
        return nullptr;
    }
    return objHandle.GetPtr();
}

EtsClass *StdCoreRuntimeLinkerLoadClassInternal(EtsClass *ctxClassObj, EtsString *clsName, uint8_t init)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    EtsClassLinker *linker = coro->GetPandaVM()->GetClassLinker();
    ClassLinkerContext *ctx = ctxClassObj->GetRuntimeClass()->GetLoadContext();

    PandaString name = clsName->GetMutf8();
    PandaString descriptor;
    ClassHelper::GetDescriptor(utf::CStringAsMutf8(name.c_str()), &descriptor);

    EtsClass *klass = linker->GetClass(descriptor.c_str(), false, ctx);
    if (UNLIKELY(klass == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    if (UNLIKELY(init != 0 && !klass->IsInitialized())) {
        if (UNLIKELY(!coro->GetPandaVM()->GetClassLinker()->InitializeClass(coro, klass))) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }
    }
    return klass;
}

}  // namespace ark::ets::intrinsics
