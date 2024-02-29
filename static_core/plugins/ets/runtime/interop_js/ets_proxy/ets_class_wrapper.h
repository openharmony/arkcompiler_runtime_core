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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_field_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_object_reference.h"
#include "plugins/ets/runtime/interop_js/js_proxy/js_proxy.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"

#include <node_api.h>
#include <vector>

namespace panda::ets {
class EtsClass;
}  // namespace panda::ets

namespace panda::ets::interop::js::ets_proxy {

using EtsClassWrappersCache = WrappersCache<EtsClass *, EtsClassWrapper>;

class EtsClassWrapper {
public:
    // clang-format off
    static constexpr auto FIELD_ATTR =
        static_cast<napi_property_attributes>(napi_writable | napi_enumerable);
    static constexpr auto METHOD_ATTR = napi_default;
    static constexpr auto STATIC_FIELD_ATTR =
        static_cast<napi_property_attributes>(
            napi_writable | napi_enumerable | napi_static);  // NOLINT(hicpp-signed-bitwise)
    static constexpr auto STATIC_METHOD_ATTR = napi_static;
    // clang-format on

    using OverloadsMap = std::unordered_map<uint8_t const *, char const *, utf::Mutf8Hash, utf::Mutf8Equal>;

    static std::unique_ptr<EtsClassWrapper> Create(InteropCtx *ctx, EtsClass *etsClass,
                                                   const char *jsBuiltinName = nullptr,
                                                   const OverloadsMap *overloads = nullptr);

    static EtsClassWrapper *Get(InteropCtx *ctx, EtsClass *etsClass);

    static std::unique_ptr<JSRefConvert> CreateJSRefConvertEtsProxy(InteropCtx *ctx, Class *klass);
    static std::unique_ptr<JSRefConvert> CreateJSRefConvertJSProxy(InteropCtx *ctx, Class *klass);

    EtsClass *GetEtsClass()
    {
        return etsClass_;
    }

    napi_value GetJsCtor(napi_env env) const
    {
        return GetReferenceValue(env, jsCtorRef_);
    }

    bool HasBuiltin() const
    {
        return jsBuiltinCtorRef_ != nullptr;
    }

    napi_value GetBuiltin(napi_env env) const
    {
        ASSERT(HasBuiltin());
        return GetReferenceValue(env, jsBuiltinCtorRef_);
    }

    void SetJSBuiltinMatcher(std::function<EtsObject *(InteropCtx *, napi_value, bool)> &&matcher)
    {
        jsBuiltinMatcher_ = matcher;
    }

    napi_value Wrap(InteropCtx *ctx, EtsObject *etsObject);
    EtsObject *Unwrap(InteropCtx *ctx, napi_value jsValue);

    EtsObject *UnwrapEtsProxy(InteropCtx *ctx, napi_value jsValue);
    EtsObject *CreateJSBuiltinProxy(InteropCtx *ctx, napi_value jsValue);

    ~EtsClassWrapper() = default;

private:
    explicit EtsClassWrapper(EtsClass *etsCls) : etsClass_(etsCls) {}
    NO_COPY_SEMANTIC(EtsClassWrapper);
    NO_MOVE_SEMANTIC(EtsClassWrapper);

    bool SetupHierarchy(InteropCtx *ctx, const char *jsBuiltinName);
    std::pair<std::vector<Field *>, std::vector<Method *>> CalculateProperties(const OverloadsMap *overloads);
    std::vector<napi_property_descriptor> BuildJSProperties(Span<Field *> fields, Span<Method *> methods);
    EtsClassWrapper *LookupBaseWrapper(EtsClass *klass);

    static napi_value JSCtorCallback(napi_env env, napi_callback_info cinfo);
    bool CreateAndWrap(napi_env env, napi_value jsNewtarget, napi_value jsThis, Span<napi_value> jsArgs);

    static void ThrowJSErrorNotAssignable(napi_env env, EtsClass *fromKlass, EtsClass *toKlass);

    Span<EtsFieldWrapper> GetFields()
    {
        return Span<EtsFieldWrapper>(etsFieldWrappers_.get(), numFields_);
    }

    Span<LazyEtsMethodWrapperLink> GetMethods()
    {
        return Span<LazyEtsMethodWrapperLink>(etsMethodWrappers_.get(), numMethods_);
    }

    EtsClass *const etsClass_ {};
    EtsClassWrapper *baseWrapper_ {};

    LazyEtsMethodWrapperLink etsCtorLink_ {};
    napi_ref jsCtorRef_ {};

    // For built-in classes
    napi_ref jsBuiltinCtorRef_ {};
    std::function<EtsObject *(InteropCtx *, napi_value, bool)> jsBuiltinMatcher_;

    std::unique_ptr<js_proxy::JSProxy> jsproxyWrapper_ {};

    // NOTE(vpukhov): allocate inplace to reduce memory consumption
    std::unique_ptr<LazyEtsMethodWrapperLink[]> etsMethodWrappers_;  // NOLINT(modernize-avoid-c-arrays)
    std::unique_ptr<EtsFieldWrapper[]> etsFieldWrappers_;            // NOLINT(modernize-avoid-c-arrays)
    uint32_t numMethods_ {};
    uint32_t numFields_ {};
};

}  // namespace panda::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H
