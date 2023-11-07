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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H_

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
    static constexpr auto FIELD_ATTR         = static_cast<napi_property_attributes>(napi_writable | napi_enumerable);
    static constexpr auto METHOD_ATTR        = napi_default;
    static constexpr auto STATIC_FIELD_ATTR  = static_cast<napi_property_attributes>(napi_writable | napi_enumerable | napi_static);  // NOLINT(hicpp-signed-bitwise)
    static constexpr auto STATIC_METHOD_ATTR = napi_static;
    // clang-format on

    using OverloadsMap = std::unordered_map<uint8_t const *, char const *, utf::Mutf8Hash, utf::Mutf8Equal>;

    static std::unique_ptr<EtsClassWrapper> Create(InteropCtx *ctx, EtsClass *ets_class,
                                                   const char *js_builtin_name = nullptr,
                                                   const OverloadsMap *overloads = nullptr);

    static EtsClassWrapper *Get(InteropCtx *ctx, EtsClass *ets_class);

    static std::unique_ptr<JSRefConvert> CreateJSRefConvertEtsProxy(InteropCtx *ctx, Class *klass);
    static std::unique_ptr<JSRefConvert> CreateJSRefConvertJSProxy(InteropCtx *ctx, Class *klass);

    EtsClass *GetEtsClass()
    {
        return ets_class_;
    }

    napi_value GetJsCtor(napi_env env) const
    {
        return GetReferenceValue(env, js_ctor_ref_);
    }

    bool HasBuiltin() const
    {
        return js_builtin_ctor_ref_ != nullptr;
    }

    napi_value GetBuiltin(napi_env env) const
    {
        ASSERT(HasBuiltin());
        return GetReferenceValue(env, js_builtin_ctor_ref_);
    }

    void SetJSBuiltinMatcher(std::function<EtsObject *(InteropCtx *, napi_value, bool)> &&matcher)
    {
        js_builtin_matcher_ = matcher;
    }

    napi_value Wrap(InteropCtx *ctx, EtsObject *ets_object);
    EtsObject *Unwrap(InteropCtx *ctx, napi_value js_value);

    EtsObject *UnwrapEtsProxy(InteropCtx *ctx, napi_value js_value);
    EtsObject *CreateJSBuiltinProxy(InteropCtx *ctx, napi_value js_value);

    ~EtsClassWrapper() = default;

private:
    explicit EtsClassWrapper(EtsClass *ets_cls) : ets_class_(ets_cls) {}
    NO_COPY_SEMANTIC(EtsClassWrapper);
    NO_MOVE_SEMANTIC(EtsClassWrapper);

    bool SetupHierarchy(InteropCtx *ctx, const char *js_builtin_name);
    std::pair<std::vector<Field *>, std::vector<Method *>> CalculateProperties(const OverloadsMap *overloads);
    std::vector<napi_property_descriptor> BuildJSProperties(Span<Field *> fields, Span<Method *> methods);
    EtsClassWrapper *LookupBaseWrapper(EtsClass *klass);

    static napi_value JSCtorCallback(napi_env env, napi_callback_info cinfo);
    bool CreateAndWrap(napi_env env, napi_value js_newtarget, napi_value js_this, Span<napi_value> js_args);

    static void ThrowJSErrorNotAssignable(napi_env env, EtsClass *from_klass, EtsClass *to_klass);

    Span<EtsFieldWrapper> GetFields()
    {
        return Span<EtsFieldWrapper>(ets_field_wrappers_.get(), num_fields_);
    }

    Span<LazyEtsMethodWrapperLink> GetMethods()
    {
        return Span<LazyEtsMethodWrapperLink>(ets_method_wrappers_.get(), num_methods_);
    }

    EtsClass *const ets_class_ {};
    EtsClassWrapper *base_wrapper_ {};

    LazyEtsMethodWrapperLink ets_ctor_link_ {};
    napi_ref js_ctor_ref_ {};

    // For built-in classes
    napi_ref js_builtin_ctor_ref_ {};
    std::function<EtsObject *(InteropCtx *, napi_value, bool)> js_builtin_matcher_;

    std::unique_ptr<js_proxy::JSProxy> jsproxy_wrapper_ {};

    // NOTE(vpukhov): allocate inplace to reduce memory consumption
    std::unique_ptr<LazyEtsMethodWrapperLink[]> ets_method_wrappers_;  // NOLINT(modernize-avoid-c-arrays)
    std::unique_ptr<EtsFieldWrapper[]> ets_field_wrappers_;            // NOLINT(modernize-avoid-c-arrays)
    uint32_t num_methods_ {};
    uint32_t num_fields_ {};
};

}  // namespace panda::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_CLASS_WRAPPER_H_
