/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

static void InitProxyMethod(Class *cls, Method *src, Method *proxy)
{
    new (proxy) Method(src);

    proxy->SetAccessFlags((src->GetAccessFlags() & ~(ACC_ABSTRACT | ACC_DEFAULT_INTERFACE_METHOD)) | ACC_FINAL);
    proxy->SetClass(cls);
    proxy->SetCompiledEntryPoint(reinterpret_cast<void *>(CallJSProxyBridge));
}

/*static*/
std::unique_ptr<JSProxy> JSProxy::Create(EtsClass *etsClass, Span<Method *> targetMethods)
{
    Class *cls = etsClass->GetRuntimeClass();
    ASSERT(!IsProxyClass(cls) && !etsClass->IsFinal());
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
        InitProxyMethod(cls, targetMethods[targetMethodsIdx[i]], &proxyMethods[i]);
    }

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

    auto jsProxy = std::unique_ptr<JSProxy>(new JSProxy(EtsClass::FromRuntimeClass(proxyCls)));
    return jsProxy;
}

}  // namespace ark::ets::interop::js::js_proxy
