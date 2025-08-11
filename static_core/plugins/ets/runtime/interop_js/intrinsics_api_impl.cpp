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

#include "macros.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "types/ets_object.h"

// NOLINTBEGIN(readability-identifier-naming)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak))
napi_load_module_with_module_request(napi_env env, const char *request_name, napi_value *result);
// NOLINTEND(readability-identifier-naming)

static bool StringStartWith(std::string_view str, std::string_view startStr)
{
    size_t startStrLen = startStr.length();
    return str.compare(0, startStrLen, startStr) == 0;
}

namespace ark::ets::interop::js {

[[maybe_unused]] static bool NotNativeOhmUrl(std::string_view url)
{
    constexpr std::string_view PREFIX_BUNDLE = "@bundle:";
    constexpr std::string_view PREFIX_NORMALIZED = "@normalized:";
    constexpr std::string_view PREFIX_PACKAGE = "@package:";
    return StringStartWith(url, PREFIX_BUNDLE) || StringStartWith(url, PREFIX_NORMALIZED) ||
           StringStartWith(url, PREFIX_PACKAGE) || (url[0] != '@');
}

[[maybe_unused]] static JSValue *LoadJSModule(const PandaString &moduleName)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    napi_value result;
    napi_status status;
    {
        ScopedNativeCodeThread etsNativeScope(coro);
        NapiScope jsHandleScope(env);
        status = napi_load_module_with_module_request(env, moduleName.c_str(), &result);
        if (status == napi_pending_exception) {
            napi_value exp;
            NAPI_CHECK_FATAL(napi_get_and_clear_last_exception(env, &exp));
            NAPI_CHECK_FATAL(napi_fatal_exception(env, exp));
            INTEROP_LOG(FATAL) << "Unable to load module due to exception";
            UNREACHABLE();
        }
    }
    INTEROP_FATAL_IF(status != napi_ok);
    INTEROP_FATAL_IF(IsUndefined(env, result));
    return JSValue::CreateRefValue(coro, ctx, result, napi_object);
}

void JSRuntimeFinalizationRegistryCallback(EtsObject *cbarg)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    ASSERT(cbarg->GetClass()->GetRuntimeClass() == ctx->GetJSValueClass());
    return JSValue::FinalizeETSWeak(ctx, cbarg);
}

JSValue *JSRuntimeNewJSValueDouble(double v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateNumber(coro, ctx, v);
}

JSValue *JSRuntimeNewJSValueBoolean(uint8_t v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateBoolean(coro, ctx, static_cast<bool>(v));
}

JSValue *JSRuntimeNewJSValueString(EtsString *v)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    if (v->GetClass() == PlatformTypes(coro)->interopJSValue) {
        return JSValue::FromEtsObject(v);
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto refconv = JSRefConvertResolve(ctx, v->GetClass()->GetRuntimeClass());
    if (UNLIKELY(refconv == nullptr)) {
        // For builtin without supported converter
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    auto result = refconv->Wrap(ctx, v);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (!res) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }

    return res.value();
}

uint8_t JSRuntimeIsJSValue(EtsObject *v)
{
    if (v == nullptr) {
        return 0U;
    }
    auto coro = EtsCoroutine::GetCurrent();
    return static_cast<uint8_t>(v->GetClass() == PlatformTypes(coro)->interopJSValue);
}

JSValue *JSRuntimeNewJSValueBigInt(EtsBigInt *v)
{
    if (v == nullptr) {
        return nullptr;
    }

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto env = ctx->GetJSEnv();

    NapiScope jsHandleScope(env);

    napi_value jsVal = JSConvertJSValue::Wrap(env, etsJsValue);
    auto res = JSConvertString::Unwrap(ctx, env, jsVal);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    return res.value();
}

EtsObject *JSRuntimeGetValueObject(JSValue *etsJsValue, EtsClass *clsObj)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto cls = clsObj->GetRuntimeClass();

    if (etsJsValue == nullptr) {
        return nullptr;
    }

    // NOTE(kprokopenko): awful solution, see #14765
    if (etsJsValue->AsObject() == ctx->GetNullValue()) {
        return etsJsValue->AsObject();
    }

    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value jsVal = JSConvertJSValue::Wrap(env, etsJsValue);
    if (IsUndefined(env, jsVal)) {
        return nullptr;
    }

    auto refconv = JSRefConvertResolve<true>(ctx, cls);
    if (UNLIKELY(refconv == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    auto res = refconv->Unwrap(ctx, jsVal);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        res = nullptr;
    }
    return res;
}

JSValue *JSRuntimeGetUndefined()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateUndefined(coro, ctx);
}

JSValue *JSRuntimeGetNull()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateNull(coro, ctx);
}

JSValue *JSRuntimeGetGlobal()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    return JSValue::CreateRefValue(coro, ctx, GetGlobal(env), napi_object);
}

JSValue *JSRuntimeCreateObject()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value obj;
    NAPI_CHECK_FATAL(napi_create_object(env, &obj));
    return JSValue::CreateRefValue(coro, ctx, obj, napi_object);
}

JSValue *JSRuntimeCreateArray()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value array;
    NAPI_CHECK_FATAL(napi_create_array(env, &array));
    return JSValue::Create(coro, ctx, array);
}

uint8_t JSRuntimeInstanceOfDynamic(JSValue *object, JSValue *ctor)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto jsObj = JSConvertJSValue::WrapWithNullCheck(env, object);
    auto jsCtor = JSConvertJSValue::WrapWithNullCheck(env, ctor);
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    auto env = ctx->GetJSEnv();

    Class *cls = etsCls->GetRuntimeClass();
    if (!cls->IsInitialized() && !IsStdClass(cls)) {
        // 'js_value' haven't been created from the uninitialized class
        return 0;
    }

    // Check if object has SharedReference
    ets_proxy::SharedReference *sharedRef = [=] {
        NapiScope jsHandleScope(env);
        napi_value jsValue = JSConvertJSValue::Wrap(env, etsJsValue);
        return ctx->GetSharedRefStorage()->GetReference(env, jsValue);
    }();

    if (sharedRef != nullptr) {
        EtsObject *etsObject = sharedRef->GetEtsObject();
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();

    PandaString moduleName = module->GetMutf8();
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    if (NotNativeOhmUrl(moduleName)) {
        return LoadJSModule(moduleName);
    }
#endif
    auto [mod, func] = ResolveModuleName(moduleName);

    napi_value modObj;
    {
        ScopedNativeCodeThread etsNativeScope(coro);
        NapiEscapableScope jsHandleScope(env);
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
        INTEROP_FATAL_IF(IsNull(env, modObj));
        jsHandleScope.Escape(modObj);
    }

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, modObj);
    if (sharedRef != nullptr) {
        return JSValue::FromEtsObject(sharedRef->GetEtsObject());
    }
    return JSValue::CreateRefValue(coro, ctx, modObj, napi_object);
}

uint8_t JSRuntimeStrictEqual([[maybe_unused]] JSValue *lhs, [[maybe_unused]] JSValue *rhs)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    NAPI_CHECK_FATAL(napi_strict_equals(env, JSConvertJSValue::WrapWithNullCheck(env, lhs),
                                        JSConvertJSValue::WrapWithNullCheck(env, rhs), &result));
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasProperty(JSValue *object, EtsString *name)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_property(env, JSConvertJSValue::WrapWithNullCheck(env, object),
                                     JSConvertString::WrapWithNullCheck(env, name), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

JSValue *JSRuntimeGetProperty(JSValue *object, JSValue *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto jsThis = JSConvertJSValue::WrapWithNullCheck(env, object);
    auto key = JSConvertJSValue::WrapWithNullCheck(env, property);

    napi_value result;
    {
        ScopedNativeCodeThread nativeScope(coro);
        NAPI_CHECK_FATAL(napi_get_property(env, jsThis, key, &result));
    }
    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    return res.value();
}

uint8_t JSRuntimeHasPropertyJSValue(JSValue *object, JSValue *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_property(env, JSConvertJSValue::WrapWithNullCheck(env, object),
                                     JSConvertJSValue::WrapWithNullCheck(env, property), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasElement(JSValue *object, int index)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_element(env, JSConvertJSValue::WrapWithNullCheck(env, object), index, &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasOwnProperty(JSValue *object, EtsString *name)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_own_property(env, JSConvertJSValue::WrapWithNullCheck(env, object),
                                         JSConvertString::WrapWithNullCheck(env, name), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasOwnPropertyJSValue(JSValue *object, JSValue *property)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_has_own_property(env, JSConvertJSValue::WrapWithNullCheck(env, object),
                                         JSConvertJSValue::WrapWithNullCheck(env, property), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, coro, result);
    return static_cast<uint8_t>(result);
}

EtsString *JSRuntimeTypeOf(JSValue *object)
{
    if (object == nullptr) {
        return nullptr;
    }
    switch (object->GetType()) {
        case napi_undefined:
            return EtsString::CreateFromMUtf8("undefined");
        case napi_null:
            return EtsString::CreateFromMUtf8("object");
        case napi_boolean:
            return EtsString::CreateFromMUtf8("boolean");
        case napi_number:
            return EtsString::CreateFromMUtf8("number");
        case napi_string:
            return EtsString::CreateFromMUtf8("string");
        case napi_symbol:
            return EtsString::CreateFromMUtf8("object");
        case napi_object: {
            auto coro = EtsCoroutine::GetCurrent();
            auto ctx = InteropCtx::Current(coro);
            INTEROP_CODE_SCOPE_ETS(coro);
            auto env = ctx->GetJSEnv();
            NapiScope jsHandleScope(env);
            napi_value jsValue = JSConvertJSValue::Wrap(env, object);
            // (note: need to use JS_TYPE, #ICIFWJ)
            if (IsConstructor(env, jsValue, "Number")) {
                return EtsString::CreateFromMUtf8("number");
            }
            if (IsConstructor(env, jsValue, "Boolean")) {
                return EtsString::CreateFromMUtf8("boolean");
            }
            if (IsConstructor(env, jsValue, "String")) {
                return EtsString::CreateFromMUtf8("string");
            }
            return EtsString::CreateFromMUtf8("object");
        }
        case napi_function:
            return EtsString::CreateFromMUtf8("function");
        case napi_external:
            return EtsString::CreateFromMUtf8("external");
        case napi_bigint:
            return EtsString::CreateFromMUtf8("bigint");
        default:
            UNREACHABLE();
    }
}

JSValue *JSRuntimeInvoke(JSValue *recv, JSValue *func, EtsArray *args)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    PandaVector<napi_value> realArgs;

    for (size_t i = 0; i < args->GetLength(); i++) {
        auto objHeader = args->GetCoreType()->Get<ObjectHeader *>(i);
        if (objHeader == nullptr) {
            break;
        }

        auto t = helpers::TypeIdentity<JSConvertJSValue>();
        using Convertor = typename decltype(t)::type;
        using Cpptype = typename Convertor::cpptype;
        Cpptype value = std::remove_pointer_t<Cpptype>::FromEtsObject(EtsObject::FromCoreType(objHeader));
        realArgs.push_back(Convertor::Wrap(env, value));
    }

    napi_value retVal;
    napi_status jsStatus;
    auto recvEtsObject = JSConvertJSValue::WrapWithNullCheck(env, recv);
    auto funcEtsObject = JSConvertJSValue::WrapWithNullCheck(env, func);
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_call_function(env, recvEtsObject, funcEtsObject, realArgs.size(), realArgs.data(), &retVal);
    }

    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertJSValue::UnwrapWithNullCheck(ctx, env, retVal).value();
}

JSValue *JSRuntimeInstantiate(JSValue *callable, EtsArray *args)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    PandaVector<napi_value> realArgs;

    for (size_t i = 0; i < args->GetLength(); i++) {
        auto objHeader = args->GetCoreType()->Get<ObjectHeader *>(i);
        if (objHeader == nullptr) {
            break;
        }

        auto t = helpers::TypeIdentity<JSConvertJSValue>();
        using Convertor = typename decltype(t)::type;
        using Cpptype = typename Convertor::cpptype;
        Cpptype value = std::remove_pointer_t<Cpptype>::FromEtsObject(EtsObject::FromCoreType(objHeader));
        realArgs.push_back(Convertor::Wrap(env, value));
    }

    auto ctor = JSConvertJSValue::WrapWithNullCheck(env, callable);
    napi_status jsStatus;
    napi_value retVal;
    {
        ScopedNativeCodeThread nativeScope(coro);
        jsStatus = napi_new_instance(env, ctor, realArgs.size(), realArgs.data(), &retVal);
    }

    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertJSValue::UnwrapWithNullCheck(ctx, env, retVal).value();
}

uint8_t JSRuntimeIsPromise(JSValue *object)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    NAPI_CHECK_FATAL(napi_is_promise(env, JSConvertJSValue::WrapWithNullCheck(env, object), &result));
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeInstanceOfStaticType(JSValue *object, EtsTypeAPIType *paramType)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    EtsHandleScope scope(coro);

    if (object == nullptr || paramType == nullptr) {
        return static_cast<uint8_t>(false);
    }

    EtsHandle<EtsClass> klass(coro, paramType->GetClass());
    EtsField *field = klass->GetFieldIDByName("cls", nullptr);
    auto clsObj = paramType->GetFieldObject(field);
    if (clsObj == nullptr) {
        INTEROP_LOG(ERROR) << "failed to get field by name";
        return static_cast<uint8_t>(false);
    }
    auto cls = reinterpret_cast<EtsClass *>(clsObj);
    napi_value jsValue = JSConvertJSValue::Wrap(env, object);
    if (jsValue == nullptr) {
        return static_cast<uint8_t>(false);
    }
    auto etsObjectRes = JSConvertEtsObject::UnwrapImpl(ctx, env, jsValue);
    if (!etsObjectRes.has_value()) {
        return static_cast<uint8_t>(false);
    }
    EtsObject *etsObject = etsObjectRes.value();
    bool result = cls->IsAssignableFrom(etsObject->GetClass());
    return static_cast<uint8_t>(result);
}

EtsString *JSValueToString(JSValue *object)
{
    ASSERT(object != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    napi_env env = ctx->GetJSEnv();

    NapiScope jsHandleScope(env);

    napi_value strObj;
    [[maybe_unused]] napi_status status;
    {
        auto napiValue = JSConvertJSValue::Wrap(env, object);

        ScopedNativeCodeThread nativeScope(coro);  // NOTE: Scope(Native/Managed)CodeThread should be optional here
        status = napi_coerce_to_string(env, napiValue, &strObj);
    }

#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    // #22503 napi_coerce_to_string in arkui/napi implementation returns napi_ok in case of exception.
    if (UNLIKELY(NapiIsExceptionPending(env))) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
#else
    if (UNLIKELY(status != napi_ok)) {
        INTEROP_FATAL_IF(status != napi_string_expected && status != napi_pending_exception);
        ctx->ForwardJSException(coro);
        return nullptr;
    }
#endif

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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto env = ctx->GetJSEnv();

    napi_value res;
    auto rc = napi_get_element(env, ToLocal(val), index, &res);
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return res;
}

void *CompilerJSCallCheck(void *fn)
{
    auto jsFn = ToLocal(fn);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    napi_env env = ctx->GetJSEnv();
    auto scopesStorage = ctx->GetLocalScopesStorage();
    ASSERT(coro->IsCurrentFrameCompiled());
    scopesStorage->CreateLocalScope(env, coro->GetCurrentFrame());
}

void CompilerDestroyLocalScope()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    napi_env env = ctx->GetJSEnv();
    auto scopesStorage = ctx->GetLocalScopesStorage();
    ASSERT(coro->IsCurrentFrameCompiled());
    scopesStorage->DestroyTopLocalScope(env, coro->GetCurrentFrame());
}

void *CompilerLoadJSConstantPool()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return ctx->GetConstStringStorage()->GetConstantPool();
}

void CompilerInitJSCallClassForCtx(void *klassPtr)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    auto klass = reinterpret_cast<Class *>(klassPtr);
    ctx->GetConstStringStorage()->LoadDynamicCallClass(klass);
}

void *CompilerConvertVoidToLocal()
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    napi_env env = ctx->GetJSEnv();
    return GetUndefined(env);
}

void *CompilerConvertRefTypeToLocal(EtsObject *etsValue)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
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
    HandleScope<ObjectHeader *> scope(coro);
    VMHandle<ObjectHeader> handle(coro, ref);
    auto refconv = JSRefConvertResolve(ctx, klass);
    auto res = refconv->Wrap(ctx, EtsObject::FromCoreType(handle.GetPtr()));
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    napi_env env = ctx->GetJSEnv();
    if (UNLIKELY(IsNull(env, jsVal))) {
        return ctx->GetNullValue();
    }
    if (UNLIKELY(IsUndefined(env, jsVal))) {
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
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto global = GetGlobal(env);

    napi_value jsonInstance;
    NAPI_CHECK_FATAL(napi_get_named_property(env, global, "JSON", &jsonInstance));

    napi_value stringify;
    NAPI_CHECK_FATAL(napi_get_named_property(env, jsonInstance, "stringify", &stringify));

    auto object = JSConvertJSValue::Wrap(env, jsvalue);
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
    if (IsUndefined(env, result)) {
        return EtsString::CreateFromMUtf8("undefined");
    }
    auto res = JSConvertString::Unwrap(ctx, env, result);
    return res.value_or(nullptr);
}

void SettleJsPromise(EtsObject *value, napi_deferred deferred, EtsInt state)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro->GetCoroutineManager()->IsMainWorker(coro));
    auto *ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    napi_env env = ctx->GetJSEnv();
    napi_value completionValue;

    NapiScope napiScope(env);
    if (value == nullptr) {
        completionValue = GetUndefined(env);
    } else {
        auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
        completionValue = refconv->Wrap(ctx, value);
    }
    ScopedNativeCodeThread nativeScope(coro);
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

JSValue *JSRuntimeGetPropertyJSValueyByKey(JSValue *objectValue, JSValue *keyValue)
{
    ASSERT(objectValue != nullptr);
    ASSERT(keyValue != nullptr);

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    NapiScope jsHandleScope(env);

    napi_value objectNapiValue = JSConvertJSValue::Wrap(env, objectValue);
    napi_value keyNapiValue = JSConvertJSValue::Wrap(env, keyValue);

    napi_value result;
    auto status = napi_get_property(env, objectNapiValue, keyNapiValue, &result);
    if (UNLIKELY(status == napi_object_expected || NapiThrownGeneric(status))) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    INTEROP_FATAL_IF(status != napi_ok);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(coro);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    return res.value();
}

EtsEscompatArrayBuffer *TransferArrayBufferToStatic(ESValue *object)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = object->GetEo()->GetNapiValue(ctx->GetJSEnv());

    bool isArrayBuffer = false;
    NAPI_CHECK_FATAL(napi_is_arraybuffer(env, dynamicArrayBuffer, &isArrayBuffer));
    if (!isArrayBuffer) {
        ctx->ThrowETSError(coro, "Dynamic object is not arraybuffer");
    }

    void *data = nullptr;
    size_t byteLength = 0;
    // NOTE(dslynko, #23919): finalize semantics of resizable ArrayBuffers
    NAPI_CHECK_FATAL(napi_get_arraybuffer_info(env, dynamicArrayBuffer, &data, &byteLength));

    [[maybe_unused]] EtsHandleScope s(coro);
    void *etsData = nullptr;
    auto *arrayBuffer = EtsEscompatArrayBuffer::Create(coro, byteLength, &etsData);
    std::copy_n(reinterpret_cast<uint8_t *>(data), byteLength, reinterpret_cast<uint8_t *>(etsData));
    return arrayBuffer;
}

EtsObject *TransferArrayBufferToDynamic(EtsEscompatArrayBuffer *staticArrayBuffer)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = nullptr;
    void *data;
    // NOTE(dslynko, #23919): finalize semantics of resizable ArrayBuffers
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, staticArrayBuffer->GetByteLength(), &data, &dynamicArrayBuffer));
    std::copy_n(reinterpret_cast<const uint8_t *>(staticArrayBuffer->GetData()), staticArrayBuffer->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));

    JSValue *etsJSValue = JSValue::Create(coro, ctx, dynamicArrayBuffer);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

EtsObject *CreateDynamicTypedArray(EtsEscompatArrayBuffer *staticArrayBuffer, int32_t typedArrayType, double length,
                                   double byteOffset)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = nullptr;
    void *data;
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, staticArrayBuffer->GetByteLength(), &data, &dynamicArrayBuffer));
    std::copy_n(reinterpret_cast<const uint8_t *>(staticArrayBuffer->GetData()), staticArrayBuffer->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));

    napi_value dynamicTypedArray = nullptr;
    napi_create_typedarray(env, static_cast<napi_typedarray_type>(typedArrayType), length, dynamicArrayBuffer,
                           byteOffset, &dynamicTypedArray);

    JSValue *etsJSValue = JSValue::Create(coro, ctx, dynamicTypedArray);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

EtsObject *CreateDynamicDataView(EtsEscompatArrayBuffer *staticArrayBuffer, double byteLength, double byteOffset)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = nullptr;
    void *data;
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, staticArrayBuffer->GetByteLength(), &data, &dynamicArrayBuffer));
    std::copy_n(reinterpret_cast<const uint8_t *>(staticArrayBuffer->GetData()), staticArrayBuffer->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));

    napi_value dynamicDataView = nullptr;
    napi_create_dataview(env, byteLength, dynamicArrayBuffer, byteOffset, &dynamicDataView);

    JSValue *etsJSValue = JSValue::Create(coro, ctx, dynamicDataView);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

void SetInteropRuntimeLinker(EtsRuntimeLinker *linker)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    INTEROP_CODE_SCOPE_ETS(coro);

    ctx->SetDefaultLinkerContext(linker);
}

}  // namespace ark::ets::interop::js
