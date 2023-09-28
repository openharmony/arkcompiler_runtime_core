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

#include "plugins/ets/runtime/interop_js/js_value_call.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/class_linker-inl.h"

namespace panda::ets::interop::js {

static void JSRuntimeFinalizationQueueCallback(EtsObject *cbarg)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (cbarg->GetClass()->GetRuntimeClass() == ctx->GetJSValueClass()) {
        return JSValue::FinalizeETSWeak(ctx, cbarg);
    }
    return ets_proxy::SharedReference::FinalizeETSWeak(ctx, cbarg);
}

static JSValue *JSRuntimeNewJSValueDouble(double v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    return JSValue::CreateNumber(coro, ctx, v);
}

static JSValue *JSRuntimeNewJSValueString(EtsString *v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    std::string str;
    if (v->IsUtf16()) {
        InteropCtx::Fatal("not implemented");
    } else {
        str = std::string(utf::Mutf8AsCString(v->GetDataMUtf8()));
    }
    return JSValue::CreateString(coro, ctx, std::move(str));
}

static double JSRuntimeGetValueDouble(JSValue *ets_js_value)
{
    return ets_js_value->GetNumber();
}

static uint8_t JSRuntimeGetValueBoolean(JSValue *ets_js_value)
{
    return static_cast<uint8_t>(ets_js_value->GetBoolean());
}

static EtsString *JSRuntimeGetValueString(JSValue *ets_js_value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    NapiScope js_handle_scope(env);

    napi_value js_val = ets_js_value->GetNapiValue(env);
    auto res = JSConvertString::Unwrap(ctx, env, js_val);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return {};
    }

    return res.value();
}

template <typename T>
static typename T::cpptype JSValueNamedGetter(JSValue *ets_js_value, EtsString *ets_prop_name)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    NapiScope js_handle_scope(env);

    PandaString prop_name = ets_prop_name->GetMutf8();
    auto res = JSValueGetByName<T>(ctx, ets_js_value, prop_name.c_str());
    if (UNLIKELY(!res)) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return res.value();
}

template <typename T>
static void JSValueNamedSetter(JSValue *ets_js_value, EtsString *ets_prop_name, typename T::cpptype ets_prop_val)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    NapiScope js_handle_scope(env);

    PandaString prop_name = ets_prop_name->GetMutf8();
    bool res = JSValueSetByName<T>(ctx, ets_js_value, prop_name.c_str(), ets_prop_val);
    if (UNLIKELY(!res)) {
        ctx->ForwardJSException(coro);
    }
}

static JSValue *JSRuntimeGetUndefined()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    return JSValue::CreateUndefined(coro, ctx);
}

static JSValue *JSRuntimeGetNull()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    return JSValue::CreateNull(coro, ctx);
}

static JSValue *JSRuntimeGetGlobal()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    NapiScope js_handle_scope(env);

    return JSValue::CreateRefValue(coro, ctx, GetGlobal(env), napi_object);
}

static JSValue *JSRuntimeCreateObject()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    NapiScope js_handle_scope(env);

    napi_value obj;
    NAPI_CHECK_FATAL(napi_create_object(env, &obj));
    return JSValue::CreateRefValue(coro, ctx, obj, napi_object);
}

uint8_t JSRuntimeInstanceOf(JSValue *object, JSValue *ctor)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    NapiScope js_handle_scope(env);

    auto js_obj = object->GetNapiValue(env);
    auto js_ctor = ctor->GetNapiValue(env);
    bool res;
    napi_status rc = napi_instanceof(env, js_obj, js_ctor, &res);
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        return 0;
    }
    return static_cast<uint8_t>(res);
}

static JSValue *JSRuntimeCreateLambdaProxy(EtsObject *ets_callable)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    ASSERT(ets_callable->GetClass()->GetMethod("invoke") != nullptr);

    NapiScope js_handle_scope(env);

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (LIKELY(storage->HasReference(ets_callable))) {
        ets_proxy::SharedReference *shared_ref = storage->GetReference(ets_callable);
        ASSERT(shared_ref != nullptr);
        return JSValue::CreateRefValue(coro, ctx, shared_ref->GetJsObject(env), napi_function);
    }

    napi_value js_fn;
    ets_proxy::SharedReference *payload_shared_ref = storage->GetNextAlloc();
    NAPI_CHECK_FATAL(
        napi_create_function(env, "invoke", NAPI_AUTO_LENGTH, EtsLambdaProxyInvoke, payload_shared_ref, &js_fn));

    ets_proxy::SharedReference *shared_ref = storage->CreateETSObjectRef(ctx, ets_callable, js_fn);
    if (UNLIKELY(shared_ref == nullptr)) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return nullptr;
    }
    ASSERT(payload_shared_ref == shared_ref);
    return JSValue::CreateRefValue(coro, ctx, js_fn, napi_function);
}

static std::pair<std::string_view, std::string_view> ResolveModuleName(std::string_view module)
{
    static const std::unordered_set<std::string_view> NATIVE_MODULE_LIST = {
        "@system.app",  "@ohos.app",       "@system.router", "@system.curves",
        "@ohos.curves", "@system.matrix4", "@ohos.matrix4"};

    constexpr std::string_view REQUIRE = "require";
    constexpr std::string_view REQUIRE_NAPI = "requireNapi";
    constexpr std::string_view REQUIRE_NATIVE_MODULE = "requireNativeModule";
    constexpr std::string_view SYSTEM_PLUGIN_PREFIX = "@system.";
    constexpr std::string_view OHOS_PLUGIN_PREFIX = "@ohos.";

    if (NATIVE_MODULE_LIST.count(module) != 0) {
        return {module.substr(1), REQUIRE_NATIVE_MODULE};
    }

    if (module.compare(0, SYSTEM_PLUGIN_PREFIX.size(), SYSTEM_PLUGIN_PREFIX) == 0) {
        return {module.substr(SYSTEM_PLUGIN_PREFIX.size()), REQUIRE_NAPI};
    }

    if (module.compare(0, OHOS_PLUGIN_PREFIX.size(), OHOS_PLUGIN_PREFIX) == 0) {
        return {module.substr(OHOS_PLUGIN_PREFIX.size()), REQUIRE_NAPI};
    }

    return {module, REQUIRE};
}

static JSValue *JSRuntimeLoadModule(EtsString *module)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    PandaString module_name = module->GetMutf8();
    auto [mod, func] = ResolveModuleName(module_name);

    NapiScope js_handle_scope(env);

    napi_value require_fn;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), func.data(), &require_fn));

    INTEROP_FATAL_IF(GetValueType(env, require_fn) != napi_function);
    napi_value mod_obj;
    {
        napi_value js_name;
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, mod.data(), NAPI_AUTO_LENGTH, &js_name));
        std::array<napi_value, 1> args = {js_name};
        napi_value recv;
        NAPI_CHECK_FATAL(napi_get_undefined(env, &recv));
        auto status = (napi_call_function(env, recv, require_fn, args.size(), args.data(), &mod_obj));

        if (status == napi_pending_exception) {
            napi_value exp;
            NAPI_CHECK_FATAL(napi_get_and_clear_last_exception(env, &exp));
            NAPI_CHECK_FATAL(napi_fatal_exception(env, exp));
            std::abort();
        }

        INTEROP_FATAL_IF(status != napi_ok);
    }
    INTEROP_FATAL_IF(IsUndefined(env, mod_obj));

    return JSValue::CreateRefValue(coro, ctx, mod_obj, napi_object);
}

static uint8_t JSRuntimeStrictEqual([[maybe_unused]] JSValue *lhs, [[maybe_unused]] JSValue *rhs)
{
    auto ctx = InteropCtx::Current();
    auto env = ctx->GetJSEnv();
    NapiScope js_handle_scope(env);

    bool result = true;
    NAPI_CHECK_FATAL(napi_strict_equals(env, lhs->GetNapiValue(env), rhs->GetNapiValue(env), &result));
    return static_cast<uint8_t>(result);
}

static inline napi_value ToLocal(void *value)
{
    return reinterpret_cast<napi_value>(value);
}

void *CompilerGetJSNamedProperty(void *val, char *prop_str)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    auto js_val = ToLocal(val);
    napi_value res;
    napi_status rc = napi_get_named_property(env, js_val, prop_str, &res);
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        ASSERT(NapiIsExceptionPending(env));
        return nullptr;
    }
    return res;
}

void *CompilerResolveQualifiedJSCall(void *val, EtsString *qname_str)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    auto js_val = ToLocal(val);
    napi_value js_this = js_val;
    auto qname = std::string_view(utf::Mutf8AsCString(qname_str->GetDataMUtf8()), qname_str->GetMUtf8Length());
    auto resolve_name = [&](const std::string &name) -> bool {
        js_this = js_val;
        napi_status rc = napi_get_named_property(env, js_val, name.c_str(), &js_val);
        if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
            ctx->ForwardJSException(coro);
            ASSERT(NapiIsExceptionPending(env));
            return false;
        }
        return true;
    };
    js_this = js_val;
    if (UNLIKELY(!WalkQualifiedName(qname, resolve_name))) {
        return nullptr;
    }
    return js_this;
}

void *CompilerJSCallCheck(void *fn)
{
    auto js_fn = ToLocal(fn);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    if (UNLIKELY(GetValueType(env, js_fn) != napi_function)) {
        ctx->ThrowJSTypeError(env, "call target is not a function");
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return fn;
}

void *CompilerJSCallFunction(void *obj, void *fn, uint32_t argc, void *args)
{
    auto js_this = ToLocal(obj);
    auto js_fn = ToLocal(fn);
    auto js_args = reinterpret_cast<napi_value *>(args);
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    napi_value js_ret;
    napi_status js_status;
    {
        ctx->GetInteropFrames().push_back({coro->GetCurrentFrame(), true});
        ScopedNativeCodeThread native_scope(coro);
        js_status = napi_call_function(env, js_this, js_fn, argc, js_args, &js_ret);

        ctx->GetInteropFrames().pop_back();
    }

    if (UNLIKELY(js_status != napi_ok)) {
        INTEROP_FATAL_IF(js_status != napi_pending_exception);
        ctx->ForwardJSException(coro);
        INTEROP_LOG(DEBUG) << "JSValueJSCall: exit with pending exception";
        return nullptr;
    }
    return js_ret;
}

void *CompilerJSNewInstance(void *fn, uint32_t argc, void *args)
{
    auto js_fn = ToLocal(fn);
    auto js_args = reinterpret_cast<napi_value *>(args);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    napi_value js_ret;
    napi_status js_status;
    {
        ctx->GetInteropFrames().push_back({coro->GetCurrentFrame(), true});
        ScopedNativeCodeThread native_scope(coro);

        js_status = napi_new_instance(env, js_fn, argc, js_args, &js_ret);

        ctx->GetInteropFrames().pop_back();
    }

    if (UNLIKELY(js_status != napi_ok)) {
        INTEROP_FATAL_IF(js_status != napi_pending_exception);
        ctx->ForwardJSException(coro);
        INTEROP_LOG(DEBUG) << "JSValueJSCall: exit with pending exception";
        return nullptr;
    }
    return js_ret;
}

void CreateLocalScope()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    auto scopes_storage = ctx->GetLocalScopesStorage();
    ASSERT(coro->IsCurrentFrameCompiled());
    scopes_storage->CreateLocalScope(env, coro->GetCurrentFrame());
}

void CompilerDestroyLocalScope()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    auto scopes_storage = ctx->GetLocalScopesStorage();
    ASSERT(coro->IsCurrentFrameCompiled());
    scopes_storage->DestroyTopLocalScope(env, coro->GetCurrentFrame());
}

template <typename CONVERTOR>
typename CONVERTOR::cpptype ConvertFromLocal(void *value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    auto js_val = ToLocal(value);
    auto res = CONVERTOR::Unwrap(ctx, env, js_val);
    if (UNLIKELY(!res.has_value())) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        if constexpr (!std::is_pointer_v<typename CONVERTOR::cpptype>) {
            return 0;
        } else {
            return nullptr;
        }
    }
    return res.value();
}

template <>
JSValue *ConvertFromLocal<JSConvertJSValue>(void *value)
{
    auto js_val = ToLocal(value);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    if (UNLIKELY(IsNullOrUndefined(env, js_val))) {
        return nullptr;
    }

    auto res = JSConvertJSValue::Unwrap(ctx, env, js_val);
    if (UNLIKELY(!res.has_value())) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return res.value();
}

void *CompilerConvertVoidToLocal()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    return GetUndefined(env);
}

template <typename T>
void *ConvertToLocal(typename T::cpptype ets_value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    napi_value local_js_value = T::Wrap(env, ets_value);
    if (UNLIKELY(local_js_value == nullptr)) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return local_js_value;
}

void *CompilerConvertRefTypeToLocal(EtsObject *ets_value)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    auto ref = ets_value->GetCoreType();
    if (UNLIKELY(ref == nullptr)) {
        return GetNull(env);
    }

    auto klass = ref->template ClassAddr<Class>();
    // start fastpath
    if (klass == ctx->GetJSValueClass()) {
        auto value = JSValue::FromEtsObject(EtsObject::FromCoreType(ref));
        napi_value res = JSConvertJSValue::Wrap(env, value);
        if (UNLIKELY(res == nullptr)) {
            ctx->ForwardJSException(coro);
        }
        return res;
    }
    if (klass == ctx->GetStringClass()) {
        auto value = EtsString::FromEtsObject(EtsObject::FromCoreType(ref));
        napi_value res = JSConvertString::Wrap(env, value);
        if (UNLIKELY(res == nullptr)) {
            ctx->ForwardJSException(coro);
        }
        return res;
    }
    // start slowpath
    auto refconv = JSRefConvertResolve(ctx, klass);
    auto res = refconv->Wrap(ctx, EtsObject::FromCoreType(ref));
    if (UNLIKELY(res == nullptr)) {
        ctx->ForwardJSException(coro);
    }
    return res;
}

EtsString *CompilerConvertLocalToString(void *value)
{
    auto js_val = ToLocal(value);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    if (UNLIKELY(IsNullOrUndefined(env, js_val))) {
        return nullptr;
    }

    auto res = JSConvertString::Unwrap(ctx, env, js_val);
    if (UNLIKELY(!res.has_value())) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return res.value();
}

EtsObject *CompilerConvertLocalToRefType(void *klass_ptr, void *value)
{
    auto js_val = ToLocal(value);
    auto klass = reinterpret_cast<Class *>(klass_ptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    if (UNLIKELY(IsNullOrUndefined(env, js_val))) {
        return nullptr;
    }

    // start slowpath
    auto refconv = JSRefConvertResolve<true>(ctx, klass);
    if (UNLIKELY(refconv == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    ObjectHeader *res = refconv->Unwrap(ctx, js_val)->GetCoreType();
    if (UNLIKELY(res == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
    }
    return EtsObject::FromCoreType(res);
}

const IntrinsicsAPI G_INTRINSICS_API = {
    JSRuntimeFinalizationQueueCallback,
    JSRuntimeNewJSValueDouble,
    JSRuntimeNewJSValueString,
    JSRuntimeGetValueDouble,
    JSRuntimeGetValueBoolean,
    JSRuntimeGetValueString,
    JSValueNamedGetter<JSConvertJSValue>,
    JSValueNamedGetter<JSConvertF64>,
    JSValueNamedGetter<JSConvertString>,
    JSValueNamedSetter<JSConvertJSValue>,
    JSValueNamedSetter<JSConvertF64>,
    JSValueNamedSetter<JSConvertString>,
    JSRuntimeGetUndefined,
    JSRuntimeGetNull,
    JSRuntimeGetGlobal,
    JSRuntimeCreateObject,
    JSRuntimeInstanceOf,
    JSRuntimeInitJSCallClass,
    JSRuntimeInitJSNewClass,
    JSRuntimeCreateLambdaProxy,
    JSRuntimeLoadModule,
    JSRuntimeStrictEqual,
    CompilerGetJSNamedProperty,
    CompilerResolveQualifiedJSCall,
    CompilerJSCallCheck,
    CompilerJSCallFunction,
    CompilerJSNewInstance,
    CreateLocalScope,
    CompilerDestroyLocalScope,
    CompilerConvertVoidToLocal,
    ConvertToLocal<JSConvertU1>,
    ConvertToLocal<JSConvertU8>,
    ConvertToLocal<JSConvertI8>,
    ConvertToLocal<JSConvertU16>,
    ConvertToLocal<JSConvertI16>,
    ConvertToLocal<JSConvertU32>,
    ConvertToLocal<JSConvertI32>,
    ConvertToLocal<JSConvertU64>,
    ConvertToLocal<JSConvertI64>,
    ConvertToLocal<JSConvertF32>,
    ConvertToLocal<JSConvertF64>,
    ConvertToLocal<JSConvertJSValue>,
    CompilerConvertRefTypeToLocal,
    ConvertFromLocal<JSConvertU1>,
    ConvertFromLocal<JSConvertU8>,
    ConvertFromLocal<JSConvertI8>,
    ConvertFromLocal<JSConvertU16>,
    ConvertFromLocal<JSConvertI16>,
    ConvertFromLocal<JSConvertU32>,
    ConvertFromLocal<JSConvertI32>,
    ConvertFromLocal<JSConvertU64>,
    ConvertFromLocal<JSConvertI64>,
    ConvertFromLocal<JSConvertF32>,
    ConvertFromLocal<JSConvertF64>,
    ConvertFromLocal<JSConvertJSValue>,
    ConvertFromLocal<JSConvertString>,
    CompilerConvertLocalToRefType,
};

const IntrinsicsAPI *GetIntrinsicsAPI()
{
    return &G_INTRINSICS_API;
}

}  // namespace panda::ets::interop::js
