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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H_

#include "plugins/ets/runtime/interop_js/ets_proxy/typed_pointer.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/wrappers_cache.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/types/ets_method.h"

#include <node_api.h>

namespace panda::ets::interop::js {
class InteropCtx;
}  // namespace panda::ets::interop::js

namespace panda::ets::interop::js::ets_proxy {

class EtsMethodWrapper;
class EtsClassWrapper;

using LazyEtsMethodWrapperLink = TypedPointer<Method, EtsMethodWrapper>;
using EtsMethodWrappersCache = WrappersCache<EtsMethod *, EtsMethodWrapper>;

class EtsMethodWrapper {
public:
    static EtsMethodWrapper *GetMethod(InteropCtx *ctx, EtsMethod *ets_method);
    static EtsMethodWrapper *GetFunction(InteropCtx *ctx, EtsMethod *ets_method);

    napi_value GetJsValue(napi_env env) const
    {
        ASSERT(js_ref_);
        napi_value js_value;
        NAPI_CHECK_FATAL(napi_get_reference_value(env, js_ref_, &js_value));
        return js_value;
    }

    EtsMethod *GetEtsMethod() const
    {
        return ets_method_;
    }

    Method *GetMethod() const
    {
        return ets_method_->GetPandaMethod();
    }

    static inline EtsMethodWrapper *ResolveLazyLink(InteropCtx *ctx, /* in/out */ LazyEtsMethodWrapperLink &lazy_link)
    {
        if (LIKELY(lazy_link.IsResolved())) {
            return lazy_link.GetResolved();
        }
        EtsMethod *ets_method = EtsMethod::FromRuntimeMethod(lazy_link.GetUnresolved());
        EtsMethodWrapper *wrapper = EtsMethodWrapper::GetMethod(ctx, ets_method);
        if (UNLIKELY(wrapper == nullptr)) {
            return nullptr;
        }
        ASSERT(wrapper->js_ref_ == nullptr);
        // Update lazy_link
        lazy_link = LazyEtsMethodWrapperLink(wrapper);
        return wrapper;
    }

    static napi_property_descriptor MakeNapiProperty(Method *method, LazyEtsMethodWrapperLink *lazy_link_space);

    template <bool IS_STATIC, bool IS_FUNC>
    static napi_value EtsMethodCallHandler(napi_env env, napi_callback_info cinfo);

private:
    static std::unique_ptr<EtsMethodWrapper> CreateMethod(EtsMethod *method, EtsClassWrapper *owner);
    static std::unique_ptr<EtsMethodWrapper> CreateFunction(InteropCtx *ctx, EtsMethod *method);

    EtsMethodWrapper(EtsMethod *method, EtsClassWrapper *owner) : ets_method_(method), owner_(owner) {}

    EtsMethod *const ets_method_ {};
    EtsClassWrapper *const owner_ {};  // only for instance methods
    napi_ref js_ref_ {};               // only for functions (ETSGLOBAL::)
};

}  // namespace panda::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H_
