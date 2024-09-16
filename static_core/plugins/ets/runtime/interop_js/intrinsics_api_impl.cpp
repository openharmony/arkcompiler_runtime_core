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

#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/coroutines/stackful_coroutine.h"

namespace ark::ets::interop::js {

void JSRuntimeFinalizationRegistryCallback(EtsObject *cbarg)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (cbarg->GetClass()->GetRuntimeClass() == ctx->GetJSValueClass()) {
        return JSValue::FinalizeETSWeak(ctx, cbarg);
    }
    return ets_proxy::SharedReference::FinalizeETSWeak(ctx, cbarg);
}

JSValue *JSRuntimeNewJSValueDouble(double v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    return JSValue::CreateNumber(coro, ctx, v);
}

JSValue *JSRuntimeNewJSValueBoolean(uint8_t v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    return JSValue::CreateBoolean(coro, ctx, static_cast<bool>(v));
}

JSValue *JSRuntimeNewJSValueString(EtsString *v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    std::string str;
    if (v->IsUtf16()) {
        InteropCtx::Fatal("not implemented");
    } else {
        str = std::string(utf::Mutf8AsCString(v->GetDataMUtf8()), v->GetLength());
    }
    return JSValue::CreateString(coro, ctx, std::move(str));
}

JSValue *JSRuntimeNewJSValueObject(EtsObject *v)
{
    if (v == nullptr) {
        return nullptr;
    }

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto refconv = JSRefConvertResolve(ctx, v->GetClass()->GetRuntimeClass());
    auto result = refconv->Wrap(ctx, v);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (!res) {
        ctx->ForwardJSException(coro);
        return {};
    }

    return res.value();
}

double JSRuntimeGetValueDouble(JSValue *etsJsValue)
{
    return etsJsValue->GetNumber();
}

uint8_t JSRuntimeGetValueBoolean(JSValue *etsJsValue)
{
    return static_cast<uint8_t>(etsJsValue->GetBoolean());
}

EtsString *JSRuntimeGetValueString(JSValue *etsJsValue)
{
    ASSERT(etsJsValue != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    NapiScope jsHandleScope(env);

    napi_value jsVal = etsJsValue->GetNapiValue(env);
    auto res = JSConvertString::Unwrap(ctx, env, jsVal);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return {};
    }

    return res.value();
}

EtsObject *JSRuntimeGetValueObject(JSValue *etsJsValue, EtsClass *clsObj)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto cls = clsObj->GetRuntimeClass();

    if (etsJsValue == nullptr) {
        return nullptr;
    }

    // NOTE(kprokopenko): awful solution, see #14765
    if (etsJsValue->AsObject() == ctx->GetUndefinedObject()) {
        return etsJsValue->AsObject();
    }

    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value jsVal = etsJsValue->GetNapiValue(env);

    auto refconv = JSRefConvertResolve<true>(ctx, cls);
    if (UNLIKELY(refconv == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    return refconv->Unwrap(ctx, jsVal);
}

JSValue *JSRuntimeGetUndefined()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    return JSValue::CreateUndefined(coro, ctx);
}

JSValue *JSRuntimeGetNull()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    return JSValue::CreateNull(coro, ctx);
}

JSValue *JSRuntimeGetGlobal()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    return JSValue::CreateRefValue(coro, ctx, GetGlobal(env), napi_object);
}

JSValue *JSRuntimeCreateObject()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value obj;
    NAPI_CHECK_FATAL(napi_create_object(env, &obj));
    return JSValue::CreateRefValue(coro, ctx, obj, napi_object);
}

uint8_t JSRuntimeInstanceOfDynamic(JSValue *object, JSValue *ctor)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto jsObj = object->GetNapiValue(env);
    auto jsCtor = ctor->GetNapiValue(env);
    bool res;
    napi_status rc = napi_instanceof(env, jsObj, jsCtor, &res);
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        return 0;
    }
    return static_cast<uint8_t>(res);
}

uint8_t JSRuntimeInstanceOfStatic(JSValue *etsJsValue, EtsClass *etsCls)
{
    ASSERT(etsCls != nullptr);

    if (etsJsValue == nullptr) {
        return 0;
    }

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    Class *cls = etsCls->GetRuntimeClass();
    if (!cls->IsInitialized() && !IsStdClass(cls)) {
        // 'js_value' haven't been created from the uninitialized class
        return 0;
    }

    // Check if object has SharedReference
    ets_proxy::SharedReference *sharedRef = [=] {
        NapiScope jsHandleScope(env);
        napi_value jsValue = etsJsValue->GetNapiValue(env);
        return ctx->GetSharedRefStorage()->GetReference(env, jsValue);
    }();

    if (sharedRef != nullptr) {
        EtsObject *etsObject = sharedRef->GetEtsObject(ctx);
        return static_cast<uint8_t>(etsCls->IsAssignableFrom(etsObject->GetClass()));
    }

    if (IsStdClass(cls)) {
        // NOTE(v.cherkashin): Add support compat types, #13577
        INTEROP_LOG(FATAL) << __func__ << " doesn't support compat types";
    }

    return 0;
}

std::pair<std::string_view, std::string_view> ResolveModuleName(std::string_view module)
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

JSValue *JSRuntimeLoadModule(EtsString *module)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();

    PandaString moduleName = module->GetMutf8();
    auto [mod, func] = ResolveModuleName(moduleName);

    napi_value modObj;
    {
        ScopedNativeCodeThread etsNativeScope(coro);
        NapiScope jsHandleScope(env);

        napi_value requireFn;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), func.data(), &requireFn));

        INTEROP_FATAL_IF(GetValueType(env, requireFn) != napi_function);
        {
            napi_value jsName;
            NAPI_CHECK_FATAL(napi_create_string_utf8(env, mod.data(), NAPI_AUTO_LENGTH, &jsName));
            std::array<napi_value, 1> args = {jsName};
            napi_value recv;
            NAPI_CHECK_FATAL(napi_get_undefined(env, &recv));
            auto status = (napi_call_function(env, recv, requireFn, args.size(), args.data(), &modObj));
            if (status == napi_pending_exception) {
                napi_value exp;
                NAPI_CHECK_FATAL(napi_get_and_clear_last_exception(env, &exp));
                NAPI_CHECK_FATAL(napi_fatal_exception(env, exp));
                INTEROP_LOG(FATAL) << "Unable to load module due to exception";
                UNREACHABLE();
            }
            INTEROP_FATAL_IF(status != napi_ok);
        }
        INTEROP_FATAL_IF(IsUndefined(env, modObj));
    }

    return JSValue::CreateRefValue(coro, ctx, modObj, napi_object);
}

uint8_t JSRuntimeStrictEqual([[maybe_unused]] JSValue *lhs, [[maybe_unused]] JSValue *rhs)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = true;
    NAPI_CHECK_FATAL(napi_strict_equals(env, lhs->GetNapiValue(env), rhs->GetNapiValue(env), &result));
    return static_cast<uint8_t>(result);
}

EtsString *JSValueToString(JSValue *object)
{
    ASSERT(object != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();

    napi_value strObj;
    napi_status status;
    {
        auto napiValue = object->GetNapiValue(env);

        ScopedNativeCodeThread nativeScope(coro);  // NOTE: Scope(Native/Managed)CodeThread should be optional here
        status = napi_coerce_to_string(env, napiValue, &strObj);
    }
    if (UNLIKELY(status != napi_ok)) {
        INTEROP_FATAL_IF(status != napi_string_expected && status != napi_pending_exception);
        ctx->ForwardJSException(coro);
        return nullptr;
    }

    auto res = JSConvertString::UnwrapWithNullCheck(ctx, env, strObj);
    if (UNLIKELY(!res.has_value())) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return res.value();
}

napi_value ToLocal(void *value)
{
    return reinterpret_cast<napi_value>(value);
}

void *CompilerGetJSNamedProperty(void *val, char *propStr)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    napi_env env = ctx->GetJSEnv();
    auto jsVal = ToLocal(val);
    napi_value res;
    napi_status rc = napi_get_named_property(env, jsVal, propStr, &res);
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        ASSERT(NapiIsExceptionPending(env));
        return nullptr;
    }
    return res;
}

void *CompilerGetJSProperty(void *val, void *prop)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    napi_env env = ctx->GetJSEnv();
    auto jsVal = ToLocal(val);
    napi_value res;
    napi_status rc = napi_get_property(env, jsVal, ToLocal(prop), &res);
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        ASSERT(NapiIsExceptionPending(env));
        return nullptr;
    }
    return res;
}

void *CompilerGetJSElement(void *val, int32_t index)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    napi_value res;
    auto rc = napi_get_element(env, ToLocal(val), index, &res);
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return res;
}

void *CompilerJSCallCheck(void *fn)
{
    auto jsFn = ToLocal(fn);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    napi_env env = ctx->GetJSEnv();
    if (UNLIKELY(GetValueType(env, jsFn) != napi_function)) {
        ctx->ThrowJSTypeError(env, "call target is not a function");
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return fn;
}

void *CompilerJSNewInstance(void *fn, uint32_t argc, void *args)
{
    auto jsFn = ToLocal(fn);
    auto jsArgs = reinterpret_cast<napi_value *>(args);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    napi_env env = ctx->GetJSEnv();

    napi_value jsRet;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_new_instance(env, jsFn, argc, jsArgs, &jsRet);
    }

    if (UNLIKELY(jsStatus != napi_ok)) {
        INTEROP_FATAL_IF(jsStatus != napi_pending_exception);
        ctx->ForwardJSException(coro);
        INTEROP_LOG(DEBUG) << "JSValueJSCall: exit with pending exception";
        return nullptr;
    }
    return jsRet;
}

void CreateLocalScope()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    auto scopesStorage = ctx->GetLocalScopesStorage();
    ASSERT(coro->IsCurrentFrameCompiled());
    scopesStorage->CreateLocalScope(env, coro->GetCurrentFrame());
}

void CompilerDestroyLocalScope()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    auto scopesStorage = ctx->GetLocalScopesStorage();
    ASSERT(coro->IsCurrentFrameCompiled());
    scopesStorage->DestroyTopLocalScope(env, coro->GetCurrentFrame());
}

void *CompilerLoadJSConstantPool()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    return ctx->GetConstStringStorage()->GetConstantPool();
}

void CompilerInitJSCallClassForCtx(void *klassPtr)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto klass = reinterpret_cast<Class *>(klassPtr);
    ctx->GetConstStringStorage()->LoadDynamicCallClass(klass);
}

void *CompilerConvertVoidToLocal()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    return GetUndefined(env);
}

void *CompilerConvertRefTypeToLocal(EtsObject *etsValue)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    napi_env env = ctx->GetJSEnv();

    auto ref = etsValue->GetCoreType();
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
    auto jsVal = ToLocal(value);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    // NOTE(kprokopenko): can't assign undefined to EtsString *, see #14765
    if (UNLIKELY(IsNullOrUndefined(env, jsVal))) {
        return nullptr;
    }

    auto res = JSConvertString::Unwrap(ctx, env, jsVal);
    if (UNLIKELY(!res.has_value())) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return res.value();
}

EtsObject *CompilerConvertLocalToRefType(void *klassPtr, void *value)
{
    auto jsVal = ToLocal(value);
    auto klass = reinterpret_cast<Class *>(klassPtr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    napi_env env = ctx->GetJSEnv();
    if (UNLIKELY(IsNull(env, jsVal))) {
        return nullptr;
    }

    if (UNLIKELY(IsUndefined(env, jsVal))) {
        return ctx->GetUndefinedObject();
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
    ObjectHeader *res = refconv->Unwrap(ctx, jsVal)->GetCoreType();
    if (UNLIKELY(res == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return EtsObject::FromCoreType(res);
}

EtsString *JSONStringify(JSValue *jsvalue)
{
    ASSERT(jsvalue != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto global = GetGlobal(env);

    napi_value jsonInstance;
    NAPI_CHECK_FATAL(napi_get_named_property(env, global, "JSON", &jsonInstance));

    napi_value stringify;
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsonInstance, "stringify", &stringify));

    auto object = jsvalue->GetNapiValue(env);
    std::array<napi_value, 1> args = {object};

    napi_value result;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_call_function(env, jsonInstance, stringify, args.size(), args.data(), &result);
    }

    if (UNLIKELY(jsStatus != napi_ok)) {
        INTEROP_FATAL_IF(jsStatus != napi_pending_exception);
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    auto res = JSConvertString::Unwrap(ctx, env, result);
    return res.value_or(nullptr);
}

void SettleJsPromise(EtsObject *value, napi_deferred deferred, EtsInt state)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro == coro->GetPandaVM()->GetCoroutineManager()->GetMainThread());
    auto *ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    napi_value completionValue;

    NapiScope napiScope(env);
    if (value == nullptr) {
        napi_get_null(env, &completionValue);
    } else {
        auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
        completionValue = refconv->Wrap(ctx, value);
    }
    napi_status status = state == EtsPromise::STATE_RESOLVED ? napi_resolve_deferred(env, deferred, completionValue)
                                                             : napi_reject_deferred(env, deferred, completionValue);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Cannot resolve promise");
    }
}

void PromiseInteropResolve(EtsObject *value, EtsLong deferred)
{
    SettleJsPromise(value, reinterpret_cast<napi_deferred>(deferred), EtsPromise::STATE_RESOLVED);
}

void PromiseInteropReject(EtsObject *value, EtsLong deferred)
{
    SettleJsPromise(value, reinterpret_cast<napi_deferred>(deferred), EtsPromise::STATE_REJECTED);
}

}  // namespace ark::ets::interop::js
