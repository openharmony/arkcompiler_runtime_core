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

#include "intrinsics.h"

#include "plugins/ets/runtime/types/ets_typeapi_method.h"
#include "plugins/ets/runtime/entrypoints/entrypoints.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/include/class-inl.h"

namespace ark::ets::intrinsics {

static void CreateProxyConstructor(Class *klass, EtsMethod *proxyClassConstructor, Method *out)
{
    ASSERT(klass != nullptr);
    ASSERT(out != nullptr);
    ASSERT(proxyClassConstructor != nullptr);

    new (out) Method(EtsMethod::ToRuntimeMethod(proxyClassConstructor));
    out->SetClass(klass);

    out->SetAccessFlags((out->GetAccessFlags() & ~ACC_PROTECTED) | ACC_PUBLIC);
    // NOTE(kurnevichstanislav): possibly need additional support of some cases #30568.
}

static void CreateProxyMethod(Class *klass, EtsMethod *method, Method *out)
{
    ASSERT(klass != nullptr);
    ASSERT(method != nullptr);
    ASSERT(out != nullptr);

    new (out) Method(method->GetPandaMethod());

    constexpr uint32_t FLAGS_TO_REMOVE = ACC_ABSTRACT | ACC_DEFAULT_INTERFACE_METHOD;

    ASSERT(out->IsPublic());
    out->SetAccessFlags((out->GetAccessFlags() & ~FLAGS_TO_REMOVE) | ACC_FINAL);
    out->SetClass(klass);
    out->SetCompiledEntryPoint(entrypoints::GetEtsProxyEntryPoint());

    // Set original method of interface for which we create proxy method.
    // This pointer will be passed to the proxy handler invoke/set/get.
    EtsMethod::FromRuntimeMethod(out)->SetProxyPointer(method);
}

static Span<Method> GenerateMethods(EtsHandle<EtsEscompatArray> &methods, Class *klass,
                                    EtsClassLinkerExtension *classLinkerExt, InternalAllocatorPtr allocator)
{
    ASSERT(klass != nullptr);
    ASSERT(classLinkerExt != nullptr);

    size_t idx = 0;

    constexpr size_t NUM_OF_CONSTRUCTORS = 1U;
    size_t numMethods = methods->GetActualLengthFromEscompatArray() + NUM_OF_CONSTRUCTORS;
    Span<Method> proxyMethods(allocator->AllocArray<Method>(numMethods), numMethods);

    CreateProxyConstructor(klass, classLinkerExt->GetPlatformTypes()->coreReflectProxyConstructor,
                           &proxyMethods[idx++]);

    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    for (EtsInt i = 0; i < methods->GetActualLengthFromEscompatArray(); ++i) {
        auto refOpt = methods->GetRef(coroutine, i);
        ASSERT(refOpt.has_value());
        EtsObject *methodObject = refOpt.value();

        auto reflectMethod = EtsReflectMethod::FromEtsObject(methodObject);
        auto *method = reflectMethod->GetEtsMethod();
        CreateProxyMethod(klass, method, &proxyMethods[idx++]);
    }

    return proxyMethods;
}

static Span<Class *> TrasformInterfaces(EtsObjectArray *interfaces, InternalAllocatorPtr allocator)
{
    Span<Class *> proxyInterfaces;

    uint32_t intfNum = interfaces->GetLength();
    if (intfNum > 0) {
        proxyInterfaces = Span(allocator->AllocArray<Class *>(intfNum), intfNum);
        for (uint32_t idx = 0; idx < intfNum; ++idx) {
            EtsObject *iface = interfaces->Get(idx);
            ASSERT(iface != nullptr);
            proxyInterfaces[idx] = EtsClass::FromEtsClassObject(iface)->GetRuntimeClass();
        }
    }

    return proxyInterfaces;
}

extern "C" EtsClass *ReflectProxyGenerateProxy(EtsRuntimeLinker *linker, EtsString *name, ObjectHeader *interfaces,
                                               EtsEscompatArray *methods)
{
    ASSERT(linker != nullptr);
    ASSERT(name != nullptr);
    ASSERT(interfaces != nullptr);
    ASSERT(methods != nullptr);

    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsString> nameH(coroutine, name);
    EtsHandle<EtsObjectArray> interfacesH(coroutine, EtsObjectArray::FromCoreType(interfaces));
    EtsHandle<EtsEscompatArray> methodsH(coroutine, methods);

    PandaString descriptor;
    ClassHelper::GetDescriptor(utf::CStringAsMutf8(nameH->GetMutf8().data()), &descriptor);
    const uint8_t *descriptorMutf8 = utf::CStringAsMutf8(descriptor.c_str());

    auto *linkerCtx = linker->GetClassLinkerContext();
    auto *classLinkerExt = coroutine->GetPandaVM()->GetClassLinker()->GetEtsClassLinkerExtension();

    // Allocate temporarily class in purpose to link all created methods and pass all runtime assertions
    // (IsProxy() as example) during linkage. After all method are created successfully this class must be
    // deallocated and methods must be relinked to the result class created by `BuildClass`.
    auto *tempKlass = classLinkerExt->CreateClass(descriptorMutf8, 0, 0, sizeof(EtsClass));
    if (tempKlass == nullptr) {
        ASSERT(coroutine->HasPendingException());
        return nullptr;
    }
    tempKlass->SetLoadContext(linkerCtx);

    constexpr uint32_t PROXY_CLASS_ACCESS_FLAGS = ACC_PROXY | ACC_FINAL;
    tempKlass->SetAccessFlags(PROXY_CLASS_ACCESS_FLAGS);

    auto internalAllocator = coroutine->GetPandaVM()->GetHeapManager()->GetInternalAllocator();
    Span<Method> generatedProxyMethods = GenerateMethods(methodsH, tempKlass, classLinkerExt, internalAllocator);
    Span<Class *> proxyInterfaces = TrasformInterfaces(interfacesH.GetPtr(), internalAllocator);

    Class *baseProxyClass = classLinkerExt->GetPlatformTypes()->coreReflectProxy->GetRuntimeClass();
    Class *proxyClass = Runtime::GetCurrent()->GetClassLinker()->BuildClass(
        descriptorMutf8, true, PROXY_CLASS_ACCESS_FLAGS, generatedProxyMethods, Span<Field>(), baseProxyClass,
        proxyInterfaces, linkerCtx, false);
    if (proxyClass == nullptr) {
        LOG(ERROR, ETS) << "Can't build proxy class " << nameH->GetMutf8();
        internalAllocator->Free(generatedProxyMethods.data());
        internalAllocator->Free(proxyInterfaces.data());
        classLinkerExt->FreeClass(tempKlass);
        return nullptr;
    }

    proxyClass->SetFieldIndex(baseProxyClass->GetFieldIndex());
    proxyClass->SetMethodIndex(baseProxyClass->GetMethodIndex());
    proxyClass->SetClassIndex(baseProxyClass->GetClassIndex());

    proxyClass->SetState(Class::State::INITIALIZING);
    proxyClass->SetState(Class::State::INITIALIZED);
    classLinkerExt->FreeClass(tempKlass);

    return EtsClass::FromRuntimeClass(proxyClass);
}

}  // namespace ark::ets::intrinsics
