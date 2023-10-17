/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

namespace panda::ets::interop::js::js_proxy {

extern "C" void JSProxyCallBridge(Method *method, ...);

// Create JSProxy class descriptor that will respond to IsProxyClass
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static std::unique_ptr<uint8_t[]> MakeProxyDescriptor(const uint8_t *descriptor_p)
{
    Span<const uint8_t> descriptor(descriptor_p, utf::Mutf8Size(descriptor_p));

    ASSERT(descriptor.size() > 2);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT(descriptor[0] == 'L');
    ASSERT(descriptor[descriptor.size() - 1] == ';');

    size_t proxy_descriptor_size = descriptor.size() + 3;  // + $$\0
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto proxy_descriptor_data = std::make_unique<uint8_t[]>(proxy_descriptor_size);
    Span<uint8_t> proxy_descriptor(proxy_descriptor_data.get(), proxy_descriptor_size);

    proxy_descriptor[0] = 'L';
    proxy_descriptor[1] = '$';
    memcpy(&proxy_descriptor[2], &descriptor[1], descriptor.size() - 2);
    proxy_descriptor[proxy_descriptor.size() - 3] = '$';
    proxy_descriptor[proxy_descriptor.size() - 2] = ';';
    proxy_descriptor[proxy_descriptor.size() - 1] = '\0';

    return proxy_descriptor_data;
}

/*static*/
std::unique_ptr<JSProxy> JSProxy::Create(EtsClass *ets_class, Span<Method *> proxy_methods)
{
    Class *cls = ets_class->GetRuntimeClass();
    ASSERT(!IsProxyClass(cls));

    auto methods_buffer = new uint8_t[proxy_methods.size() * sizeof(Method)];
    Span<Method> impl_methods {reinterpret_cast<Method *>(methods_buffer), proxy_methods.size()};

    for (size_t i = 0; i < proxy_methods.size(); ++i) {
        auto *m = proxy_methods[i];
        auto new_method = new (&impl_methods[i]) Method(m);
        new_method->SetCompiledEntryPoint(reinterpret_cast<void *>(JSProxyCallBridge));
    }

    auto descriptor = MakeProxyDescriptor(cls->GetDescriptor());
    uint32_t access_flags = cls->GetAccessFlags();
    Span<Field> fields {};
    Class *base_class = cls;
    Span<Class *> interfaces {};
    ClassLinkerContext *context = cls->GetLoadContext();

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    Class *proxy_cls =
        class_linker->BuildClass(descriptor.get(), true, access_flags, {impl_methods.data(), impl_methods.size()},
                                 fields, base_class, interfaces, context, false);
    proxy_cls->SetState(Class::State::INITIALIZING);
    proxy_cls->SetState(Class::State::INITIALIZED);

    ASSERT(IsProxyClass(proxy_cls));

    auto js_proxy = std::unique_ptr<JSProxy>(new JSProxy(EtsClass::FromRuntimeClass(proxy_cls)));
    js_proxy->proxy_methods_.reset(reinterpret_cast<Method *>(methods_buffer));
    return js_proxy;
}

}  // namespace panda::ets::interop::js::js_proxy
