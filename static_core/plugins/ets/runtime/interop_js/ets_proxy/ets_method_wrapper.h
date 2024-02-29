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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H

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
    static EtsMethodWrapper *GetMethod(InteropCtx *ctx, EtsMethod *etsMethod);
    static EtsMethodWrapper *GetFunction(InteropCtx *ctx, EtsMethod *etsMethod);

    napi_value GetJsValue(napi_env env) const
    {
        ASSERT(jsRef_);
        napi_value jsValue;
        NAPI_CHECK_FATAL(napi_get_reference_value(env, jsRef_, &jsValue));
        return jsValue;
    }

    EtsMethod *GetEtsMethod() const
    {
        return etsMethod_;
    }

    Method *GetMethod() const
    {
        return etsMethod_->GetPandaMethod();
    }

    /**
     * @param ctx - interop context
     * @param lazy_link in/out lazy  method wrapper link
     */
    static inline EtsMethodWrapper *ResolveLazyLink(InteropCtx *ctx, LazyEtsMethodWrapperLink &lazyLink)
    {
        if (LIKELY(lazyLink.IsResolved())) {
            return lazyLink.GetResolved();
        }
        EtsMethod *etsMethod = EtsMethod::FromRuntimeMethod(lazyLink.GetUnresolved());
        EtsMethodWrapper *wrapper = EtsMethodWrapper::GetMethod(ctx, etsMethod);
        if (UNLIKELY(wrapper == nullptr)) {
            return nullptr;
        }
        ASSERT(wrapper->jsRef_ == nullptr);
        // Update lazyLink
        lazyLink = LazyEtsMethodWrapperLink(wrapper);
        return wrapper;
    }

    static napi_property_descriptor MakeNapiProperty(Method *method, LazyEtsMethodWrapperLink *lazyLinkSpace);

    template <bool IS_STATIC, bool IS_FUNC>
    static napi_value EtsMethodCallHandler(napi_env env, napi_callback_info cinfo);

private:
    static std::unique_ptr<EtsMethodWrapper> CreateMethod(EtsMethod *method, EtsClassWrapper *owner);
    static std::unique_ptr<EtsMethodWrapper> CreateFunction(InteropCtx *ctx, EtsMethod *method);

    EtsMethodWrapper(EtsMethod *method, EtsClassWrapper *owner) : etsMethod_(method), owner_(owner) {}

    EtsMethod *const etsMethod_ {};
    EtsClassWrapper *const owner_ {};  // only for instance methods
    napi_ref jsRef_ {};                // only for functions (ETSGLOBAL::)
};

}  // namespace panda::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_WRAPPER_H
