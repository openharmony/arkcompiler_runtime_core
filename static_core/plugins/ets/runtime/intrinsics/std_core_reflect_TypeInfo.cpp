/**
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

#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/reflect_type_info.h"
#include "plugins/ets/runtime/types/ets_typeapi_method.h"

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_handle_scope.h"

#include "intrinsics.h"

namespace ark::ets::intrinsics {

extern "C" EtsString *TypeInfoGetNameImpl(EtsClass *cls)
{
    return cls->GetName();
}

extern "C" ReflectTypeInfo *TypeInfoOfImpl(EtsClass *cls)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();

    if (cls->IsUnionClass()) {
        return ReflectTypeInfo::CreateGivenType(coro, PlatformTypes(coro)->unionTypeInfo, cls);
    }
    if (cls->IsClass()) {
        return ReflectTypeInfo::CreateGivenType(coro, PlatformTypes(coro)->classTypeInfo, cls);
    }  // NOTE(kurnevichstanislav): #28525 else if ... to be implemented

    return nullptr;
}

static EtsTypeAPIMethod *CreateTypeAPIMethod(EtsCoroutine *coro, EtsClass *cls, EtsMethod *method)
{
    ASSERT(coro != nullptr);
    ASSERT(cls != nullptr);
    ASSERT(method != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    auto typeAPIMethod = EtsHandle<EtsTypeAPIMethod>(coro, EtsTypeAPIMethod::Create(coro));
    ASSERT(typeAPIMethod.GetPtr() != nullptr);

    EtsString *name;
    if (method->IsInstanceConstructor()) {
        name = EtsString::CreateFromMUtf8(CONSTRUCTOR_NAME);
    } else {
        name = method->GetNameString();
    }
    ASSERT(name != nullptr);
    typeAPIMethod.GetPtr()->SetName(name);

    // Set Access Modifier
    EtsTypeAPIAccessModifier accessMod;
    if (method->IsPublic()) {
        accessMod = EtsTypeAPIAccessModifier::PUBLIC;
    } else if (method->IsPrivate()) {
        accessMod = EtsTypeAPIAccessModifier::PRIVATE;
    } else {
        accessMod = EtsTypeAPIAccessModifier::PROTECTED;
    }
    typeAPIMethod.GetPtr()->SetAccessMod(accessMod);

    // Set specific attributes
    uint32_t attr = 0;
    attr |= (method->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= (method->IsConstructor()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::CONSTRUCTOR) : 0U;
    attr |= (method->IsAbstract()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::ABSTRACT) : 0U;
    attr |= (!method->IsDeclaredIn(cls)) ? static_cast<uint32_t>(EtsTypeAPIAttributes::INHERITED) : 0U;
    attr |= (method->IsGetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::GETTER) : 0U;
    attr |= (method->IsSetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::SETTER) : 0U;

    typeAPIMethod.GetPtr()->SetAttributes(attr);
    return typeAPIMethod.GetPtr();
}

static PandaVector<EtsMethod *> GetInstanceMethods(EtsClass *cls, bool onlyPublic)
{
    PandaVector<EtsMethod *> instanceMethods;
    instanceMethods.reserve(cls->GetRuntimeClass()->GetVTableSize());

    cls->EnumerateVtable([&instanceMethods, onlyPublic](Method *method) {
        if (!method->IsConstructor()) {
            if ((onlyPublic && method->IsPublic()) || !onlyPublic) {
                instanceMethods.push_back(EtsMethod::FromRuntimeMethod(method));
            }
        }
        return false;
    });

    return instanceMethods;
}

extern "C" ObjectHeader *ClassTypeInfoNativeIfaceGetInstanceMethods(EtsClass *cls, EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    auto instanceMethods = GetInstanceMethods(cls, publicOnly != 0U);

    EtsClass *methodClass =
        coro->GetPandaVM()->GetClassLinker()->GetClass(panda_file_items::class_descriptors::METHOD.data());
    ASSERT(methodClass != nullptr);
    auto *array = EtsObjectArray::Create(methodClass, instanceMethods.size());

    EtsHandle<EtsObjectArray> arrayH(coro, array);
    ASSERT(arrayH.GetPtr() != nullptr);

    for (size_t idx = 0; idx < instanceMethods.size(); ++idx) {
        arrayH->Set(idx, CreateTypeAPIMethod(coro, cls, instanceMethods[idx])->AsObject());
    }

    return arrayH->AsObject()->GetCoreType();
}

extern "C" EtsTypeAPIMethod *ClassTypeInfoNativeIfaceGetInstanceMethodByName(EtsClass *cls, EtsString *name,
                                                                             EtsBoolean publicOnly)
{
    ASSERT(cls != nullptr);
    ASSERT(name != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    // Hadle case with overloads in case of spec issue#362
    EtsMethod *method = cls->GetInstanceMethod(name->GetUtf8().c_str(), nullptr);
    if (method == nullptr) {
        return nullptr;
    }
    if ((publicOnly != 0U) && !method->IsPublic()) {
        return nullptr;
    }

    return CreateTypeAPIMethod(coro, cls, method);
}

extern "C" EtsClass *ClassTypeInfoNativeIfaceGetBase(EtsClass *cls)
{
    ASSERT(cls != nullptr);
    return cls->GetBase();
}

}  // namespace ark::ets::intrinsics
