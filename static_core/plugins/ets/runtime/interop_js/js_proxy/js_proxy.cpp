/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/js_proxy/js_proxy.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js::js_proxy {

extern "C" void CallJSProxyBridge(Method *method, ...);
extern "C" void CallJSFunctionBridge(Method *method, ...);

// Create JSProxy class descriptor that will respond to IsProxyClass
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static std::unique_ptr<uint8_t[]> MakeProxyDescriptor(const uint8_t *descriptorP)
{
    Span<const uint8_t> descriptor(descriptorP, utf::Mutf8Size(descriptorP));

    ASSERT(descriptor.size() > 2U);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT(descriptor[0] == 'L');
    ASSERT(descriptor[descriptor.size() - 1] == ';');

    size_t proxyDescriptorSize = descriptor.size() + 3U;  // + $$\0
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto proxyDescriptorData = std::make_unique<uint8_t[]>(proxyDescriptorSize);
    Span<uint8_t> proxyDescriptor(proxyDescriptorData.get(), proxyDescriptorSize);

    proxyDescriptor[0] = 'L';
    proxyDescriptor[1] = '$';
    std::copy_n(&descriptor[1], descriptor.size() - 2U, &proxyDescriptor[2U]);
    proxyDescriptor[proxyDescriptor.size() - 3U] = '$';
    proxyDescriptor[proxyDescriptor.size() - 2U] = ';';
    proxyDescriptor[proxyDescriptor.size() - 1U] = '\0';

    return proxyDescriptorData;
}

static void InitProxyMethod(Class *cls, Method *src, Method *proxy, void *entryPoint)
{
    new (proxy) Method(src);

    proxy->SetAccessFlags((src->GetAccessFlags() & ~(ACC_ABSTRACT | ACC_DEFAULT_INTERFACE_METHOD)) | ACC_FINAL);
    proxy->SetClass(cls);
    proxy->SetCompiledEntryPoint(entryPoint);
}

/*
 * BuildProxyMethods
 */
static Span<Method> BuildProxyMethods(Class *cls, Span<Method *> targetMethods, void *entryPoint)
{
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();

    PandaVector<size_t> targetMethodsIdx;

    for (size_t i = 0; i < targetMethods.size(); ++i) {
        auto m = targetMethods[i];
        if (!m->IsFinal()) {  // NOTE(vpukhov): consider internal methods, final methods in builtins
            targetMethodsIdx.push_back(i);
        }
    }

    size_t const numTargets = targetMethodsIdx.size();
    Span<Method> proxyMethods {classLinker->GetAllocator()->AllocArray<Method>(numTargets), numTargets};

    for (size_t i = 0; i < numTargets; ++i) {
        InitProxyMethod(cls, targetMethods[targetMethodsIdx[i]], &proxyMethods[i],
                        reinterpret_cast<void *>(entryPoint));
    }

    return proxyMethods;
}

/*static*/
JSProxy *JSProxy::CreateBuiltinProxy(EtsClass *etsClass, Span<Method *> targetMethods)
{
    Class *cls = etsClass->GetRuntimeClass();
    ASSERT(!IsProxyClass(cls) && !etsClass->IsFinal());
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();

    auto proxyMethods = BuildProxyMethods(cls, targetMethods, reinterpret_cast<void *>(CallJSProxyBridge));

    auto descriptor = MakeProxyDescriptor(cls->GetDescriptor());
    uint32_t accessFlags = cls->GetAccessFlags() | ACC_PROXY | ACC_FINAL;
    Span<Field> fields {};
    Class *baseClass = cls;
    Span<Class *> interfaces {};
    ClassLinkerContext *context = cls->GetLoadContext();

    Class *proxyCls = classLinker->BuildClass(descriptor.get(), true, accessFlags, proxyMethods, fields, baseClass,
                                              interfaces, context, false);
    proxyCls->SetState(Class::State::INITIALIZING);
    proxyCls->SetState(Class::State::INITIALIZED);

    ASSERT(IsProxyClass(proxyCls));

    auto jsProxy = Runtime::GetCurrent()->GetInternalAllocator()->New<JSProxy>(EtsClass::FromRuntimeClass(proxyCls));
    return jsProxy;
}

/*static*/
JSProxy *JSProxy::CreateFunctionProxy(EtsClass *functionInterface)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    Class *interfaceCls = functionInterface->GetRuntimeClass();
    ASSERT(interfaceCls->IsInterface());

    // create JSFunctoinProxy class descriptor that will respond to IsProxyClass
    auto descriptor = MakeProxyDescriptor(interfaceCls->GetDescriptor());

    // use `Object` as the base class for the proxy function
    Class *objectClass = ctx->GetObjectClass();

    // get the proxy function class if it is already created
    // otherwise, create the class
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    ClassLinkerContext *context = objectClass->GetLoadContext();

    Class *proxyFunctionCls = classLinker->GetClass(descriptor.get(), true, context);

    if (proxyFunctionCls == nullptr) {
        uint32_t accessFlags = objectClass->GetAccessFlags() | ACC_PROXY | ACC_FINAL;

        auto methods = interfaceCls->GetMethods();
        PandaVector<Method *> methodPtrs;
        for (auto &method : methods) {
            methodPtrs.push_back(&method);
        }

        // build proxy methods
        // proxy invokeN(eg. invoke1) method to JSFunctionCallBackBridge
        // the JSFunctionCallBackBridge will call the JS function
        // and return the result to the caller
        auto invokeNProxyMethod = BuildProxyMethods(interfaceCls, {methodPtrs.data(), methodPtrs.size()},
                                                    reinterpret_cast<void *>(CallJSFunctionBridge));

        // create span of interface class
        // the proxy function is a class that implements the interface functionN (eg.function1)

        Span<Class *> interfacesSpan {classLinker->GetAllocator()->AllocArray<Class *>(1), 1};
        interfacesSpan[0] = interfaceCls;

        // not proxy any fields
        Span<Field> fields {};

        proxyFunctionCls = classLinker->BuildClass(descriptor.get(), true, accessFlags, invokeNProxyMethod, fields,
                                                   objectClass, interfacesSpan, context, false);

        proxyFunctionCls->SetState(Class::State::INITIALIZING);
        proxyFunctionCls->SetState(Class::State::INITIALIZED);
    }

    ASSERT(IsProxyClass(proxyFunctionCls));

    auto functionProxy =
        Runtime::GetCurrent()->GetInternalAllocator()->New<JSProxy>(EtsClass::FromRuntimeClass(proxyFunctionCls));
    return functionProxy;
}

}  // namespace ark::ets::interop::js::js_proxy
