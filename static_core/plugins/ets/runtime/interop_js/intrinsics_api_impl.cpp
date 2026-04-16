/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <string>
#include "runtime/include/mem/panda_string.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/object_header.h"
#include "runtime/execution/job_execution_context.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/interop_js/xref_object_operator.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/interop_js/interop_error.h"
#include "common_interfaces/objects/base_type.h"

// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak)) napi_load_module_with_module_request(napi_env env, const char *request_name,
                                                                       napi_value *result, const char *abcFilePath);

napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_serialize_hybrid([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value object,
                      [[maybe_unused]] napi_value transfer_list, [[maybe_unused]] napi_value clone_list,
                      [[maybe_unused]] void **result);

napi_status __attribute__((weak)) napi_deserialize_hybrid([[maybe_unused]] napi_env env, [[maybe_unused]] void *buffer,
                                                          [[maybe_unused]] napi_value *object);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)

static bool StringStartWith(std::string_view str, std::string_view startStr)
{
    size_t startStrLen = startStr.length();
    return str.compare(0, startStrLen, startStr) == 0;
}

namespace ark::ets::interop::js {

using common_vm::TaggedType;

[[maybe_unused]] static bool NotNativeOhmUrl(std::string_view url)
{
    constexpr std::string_view PREFIX_BUNDLE = "@bundle:";
    constexpr std::string_view PREFIX_NORMALIZED = "@normalized:";
    constexpr std::string_view PREFIX_PACKAGE = "@package:";
    return StringStartWith(url, PREFIX_BUNDLE) || StringStartWith(url, PREFIX_NORMALIZED) ||
           StringStartWith(url, PREFIX_PACKAGE) || (url[0] != '@');
}

[[maybe_unused]] static JSValue *LoadJSModule(const PandaString &moduleName, const PandaString &abcFilePath)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    napi_value result;
    napi_status status;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        status = napi_load_module_with_module_request(env, moduleName.c_str(), &result, abcFilePath.c_str());
        if (status != napi_ok) {
            INTEROP_LOG(ERROR) << "Unable to load module " << moduleName.c_str() << " due to Forward Exception"
                               << ". Error code: " << std::to_string(INTEROP_MODULE_LOAD_FAILED);
            ctx->ForwardJSException(executionCtx);
            return nullptr;
        }
    }
    if (IsUndefined(env, result) || result == nullptr) {
        // NOLINTNEXTLINE(readability-redundant-string-cstr)
        PandaString exp = PandaString("Unable to load module ") + moduleName.c_str() + " due to Undefined result";
        INTEROP_LOG(ERROR) << exp << ". Error code: " << std::to_string(INTEROP_MODULE_LOAD_FAILED);
        InteropCtx::ThrowETSError(executionCtx, INTEROP_MODULE_LOAD_FAILED, std::string(exp));
        return nullptr;
    }
    return JSValue::CreateRefValue(executionCtx, ctx, result, napi_object);
}

void JSRuntimeFinalizationRegistryCallback(EtsObject *cbarg)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    ASSERT(cbarg->GetClass()->GetRuntimeClass() == PlatformTypes()->interopJSValue->GetRuntimeClass());
    return JSValue::FinalizeETSWeak(ctx, cbarg);
}

JSValue *JSRuntimeNewJSValueDouble(double v)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateNumber(executionCtx, ctx, v);
}

JSValue *JSRuntimeNewJSValueBoolean(uint8_t v)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateBoolean(executionCtx, ctx, static_cast<bool>(v));
}

JSValue *JSRuntimeNewJSValueString(EtsString *v)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    std::string str;
    if (v->IsUtf16()) {
        InteropCtx::Fatal(INTEROP_FEATURE_NOT_IMPLEMENTED, "not implemented");
    } else {
        PandaVector<uint8_t> tree8Buf;
        uint8_t *data = v->IsTreeString() ? v->GetTreeStringDataMUtf8(tree8Buf) : v->GetDataMUtf8();
        str = std::string(utf::Mutf8AsCString(data), v->GetLength());
    }
    return JSValue::CreateString(executionCtx, ctx, std::move(str));
}

JSValue *JSRuntimeNewJSValueObject(EtsObject *v)
{
    if (v == nullptr) {
        return nullptr;
    }

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    if (v->GetClass() == PlatformTypes(executionCtx)->interopJSValue) {
        return JSValue::FromEtsObject(v);
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto refconv = JSRefConvertResolve(ctx, v->GetClass()->GetRuntimeClass());
    if (UNLIKELY(refconv == nullptr)) {
        // For builtin without supported converter
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    auto result = refconv->Wrap(ctx, v);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (!res) {
        ctx->ForwardJSException(executionCtx);
        return nullptr;
    }

    return res.value();
}

uint8_t JSRuntimeIsJSValue(EtsObject *v)
{
    if (v == nullptr) {
        return 0U;
    }
    auto executionCtx = EtsExecutionContext::GetCurrent();
    return static_cast<uint8_t>(v->GetClass() == PlatformTypes(executionCtx)->interopJSValue);
}

JSValue *JSRuntimeNewJSValueBigInt(EtsBigInt *v)
{
    if (v == nullptr) {
        return nullptr;
    }

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto refconv = JSRefConvertResolve(ctx, v->GetClass()->GetRuntimeClass());
    auto result = refconv->Wrap(ctx, v);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (!res) {
        ctx->ForwardJSException(executionCtx);
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

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
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
            ctx->ForwardJSException(executionCtx);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    return res.value();
}

EtsObject *JSRuntimeGetValueObject(EtsObject *etsValue, EtsClass *clsObj)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto cls = clsObj->GetRuntimeClass();

    if (etsValue == nullptr) {
        return nullptr;
    }

    // NOTE(kprokopenko): awful solution, see #14765
    if (etsValue == ctx->GetNullValue()) {
        return etsValue;
    }

    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsVal = JSConvertEtsObject::WrapWithNullCheck(env, etsValue);

    auto refconv = JSRefConvertResolve<true>(ctx, cls);
    if (UNLIKELY(refconv == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(executionCtx);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    auto res = refconv->Unwrap(ctx, jsVal);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(executionCtx);
        }
        res = nullptr;
    }
    return res;
}

JSValue *JSRuntimeGetUndefined()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateUndefined(executionCtx, ctx);
}

JSValue *JSRuntimeGetNull()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return JSValue::CreateNull(executionCtx, ctx);
}

JSValue *JSRuntimeGetGlobal()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value global;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        global = GetGlobal(env);
    }
    return JSValue::CreateRefValue(executionCtx, ctx, global, napi_object);
}

JSValue *JSRuntimeCreateObject()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value obj;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        NAPI_CHECK_FATAL(napi_create_object(env, &obj));
    }
    return JSValue::CreateRefValue(executionCtx, ctx, obj, napi_object);
}

EtsObject *JSRuntimeCreateArray()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value array;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        NAPI_CHECK_FATAL(napi_create_array(env, &array));
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, array).value();
}

uint8_t JSRuntimeInstanceOfDynamic(EtsObject *object, EtsObject *ctor)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto objConv = JSRefConvertResolve(ctx, object->GetClass()->GetRuntimeClass());
    auto ctorConv = JSRefConvertResolve(ctx, ctor->GetClass()->GetRuntimeClass());
    bool res;
    auto targetObj = objConv->Wrap(ctx, object);
    auto ctorObj = ctorConv->Wrap(ctx, ctor);
    napi_status rc;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        rc = napi_instanceof(env, targetObj, ctorObj, &res);
    }
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(executionCtx);
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

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
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
        INTEROP_LOG(FATAL) << __func__ << " doesn't support compat types. Error code: "
                           << std::to_string(INTEROP_TYPE_NOT_ASSIGNABLE);
    }

    return 0;
}

static ALWAYS_INLINE inline bool CheckEtsObjectFoundException(EtsExecutionContext *executionCtx, EtsObject *etsObject)
{
    if (EtsReferenceNullish(executionCtx, etsObject)) {
        PandaString message = "Need object";
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreError, INTEROP_ARGUMENT_TYPE_MISMATCH,
                          message.c_str());
        return true;
    }
    return false;
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
    constexpr std::string_view SYSTEM_PLUGIN_PREFIX_TRANSFORMED = "@system:";
    constexpr size_t SYSTEM_PLUGIN_PREFIX_SIZE = SYSTEM_PLUGIN_PREFIX.size();
    constexpr std::string_view OHOS_PLUGIN_PREFIX = "@ohos.";
    constexpr std::string_view OHOS_PLUGIN_PREFIX_TRANSFORMED = "@ohos:";
    constexpr size_t OHOS_PLUGIN_PREFIX_SIZE = OHOS_PLUGIN_PREFIX.size();
    constexpr std::string_view HMS_PLUGIN_PREFIX = "@hms.";
    constexpr std::string_view HMS_PLUGIN_PREFIX_TRANSFORMED = "@hms:";
    constexpr size_t HMS_PLUGIN_PREFIX_SIZE = HMS_PLUGIN_PREFIX.size();

    if (NATIVE_MODULE_LIST.count(module) != 0) {
        return {module.substr(1), REQUIRE_NATIVE_MODULE};
    }

    if ((module.compare(0, SYSTEM_PLUGIN_PREFIX_SIZE, SYSTEM_PLUGIN_PREFIX) == 0) ||
        (module.compare(0, SYSTEM_PLUGIN_PREFIX_SIZE, SYSTEM_PLUGIN_PREFIX_TRANSFORMED) == 0)) {
        return {module.substr(SYSTEM_PLUGIN_PREFIX_SIZE), REQUIRE_NAPI};
    }

    if ((module.compare(0, OHOS_PLUGIN_PREFIX_SIZE, OHOS_PLUGIN_PREFIX) == 0) ||
        (module.compare(0, OHOS_PLUGIN_PREFIX_SIZE, OHOS_PLUGIN_PREFIX_TRANSFORMED) == 0)) {
        return {module.substr(OHOS_PLUGIN_PREFIX_SIZE), REQUIRE_NAPI};
    }

    if ((module.compare(0, HMS_PLUGIN_PREFIX_SIZE, HMS_PLUGIN_PREFIX) == 0) ||
        (module.compare(0, HMS_PLUGIN_PREFIX_SIZE, HMS_PLUGIN_PREFIX_TRANSFORMED) == 0)) {
        return {module.substr(HMS_PLUGIN_PREFIX_SIZE), REQUIRE_NAPI};
    }

    return {module, REQUIRE};
}

static bool CheckModuleLoadStatus(EtsExecutionContext *executionCtx, napi_status status, const PandaString &moduleName,
                                  napi_value loadedObj)
{
    auto ctx = InteropCtx::Current(executionCtx);
    auto env = ctx->GetJSEnv();
    if (status != napi_ok) {
        INTEROP_LOG(ERROR) << "Unable to load module " << moduleName.c_str() << " due to Forward Exception"
                           << ". Error code: " << std::to_string(INTEROP_MODULE_LOAD_FAILED);
        ctx->ForwardJSException(executionCtx);
        return false;
    }
    if (IsNullOrUndefined(env, loadedObj)) {
        // NOLINTNEXTLINE(readability-redundant-string-cstr)
        PandaString exp = PandaString("Unable to load module ") + moduleName.c_str() + " due to null result";
        INTEROP_LOG(ERROR) << exp << ". Error code: " << std::to_string(INTEROP_MODULE_LOAD_FAILED);
        InteropCtx::ThrowETSError(executionCtx, INTEROP_MODULE_LOAD_FAILED, std::string(exp));
        return false;
    }
    return true;
}

static bool NeedWrapDefault(const PandaString &moduleName, const std::string_view &func)
{
    constexpr std::string_view OHOS_PLUGIN_PREFIX = "@ohos";
    constexpr size_t OHOS_PLUGIN_PREFIX_SIZE = OHOS_PLUGIN_PREFIX.size();
    return moduleName.length() >= OHOS_PLUGIN_PREFIX_SIZE &&
           moduleName.compare(0, OHOS_PLUGIN_PREFIX_SIZE, OHOS_PLUGIN_PREFIX) == 0 && func == "requireNapi";
}

EtsString *GetCallerABCPath()
{
    constexpr std::string_view STATIC_ABC_SUFFIX = "_static.abc";
    constexpr std::string_view ABC_SUFFIX = ".abc";
    constexpr size_t STATIC_ABC_SUFFIX_LEN = STATIC_ABC_SUFFIX.size();

    auto coroutine = EtsCoroutine::GetCurrent();
    std::string abcPath;
    auto stack = StackWalker::Create(coroutine);
    stack.NextFrame();
    if (!stack.HasFrame()) {
        LOG(ERROR, DEBUGGER) << "caller stack has no frame.";
        return EtsString::CreateNewEmptyString();
    }
    auto *method = stack.GetMethod();
    if (LIKELY(method != nullptr)) {
        if (method->GetPandaFile() == nullptr) {
            LOG(ERROR, DEBUGGER) << "caller has no abcfile.";
            return EtsString::CreateNewEmptyString();
        }
        abcPath = method->GetPandaFile()->GetFullFileName();
        size_t pos = abcPath.find(STATIC_ABC_SUFFIX);
        if (pos != std::string::npos) {
            abcPath.replace(pos, STATIC_ABC_SUFFIX_LEN, ABC_SUFFIX);
        }
        return (EtsString *)EtsString::CreateFromMUtf8(abcPath.c_str());
    }
    LOG(ERROR, DEBUGGER) << "caller stack has no method.";
    return EtsString::CreateNewEmptyString();
}

JSValue *JSRuntimeLoadModule(EtsString *module, [[maybe_unused]] EtsString *abcFilePath)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiEscapableScope jsHandleScope(env);

    PandaString moduleName = module->GetMutf8();
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    if (NotNativeOhmUrl(moduleName)) {
        PandaString abcFilePathStr = (abcFilePath != nullptr) ? abcFilePath->GetMutf8() : PandaString("");
        return LoadJSModule(moduleName, abcFilePathStr);
    }
#endif
    auto [mod, func] = ResolveModuleName(moduleName);

    napi_value modObj;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        napi_value requireFn;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), func.data(), &requireFn));
        INTEROP_FATAL_IF(GetValueType(env, requireFn) != napi_function);
        napi_value jsName;
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, mod.data(), NAPI_AUTO_LENGTH, &jsName));
        std::array<napi_value, 1> args = {jsName};
        napi_value recv;
        NAPI_CHECK_FATAL(napi_get_undefined(env, &recv));
        napi_value loadedObj;
        auto status = napi_call_function(env, recv, requireFn, args.size(), args.data(), &loadedObj);
        if (!CheckModuleLoadStatus(executionCtx, status, moduleName, loadedObj)) {
            return nullptr;
        }
        if (NeedWrapDefault(moduleName, func) && GetValueType(env, loadedObj) != napi_function) {
            NAPI_CHECK_FATAL(napi_create_object(env, &modObj));
            NAPI_CHECK_FATAL(napi_set_named_property(env, modObj, "default", loadedObj));
        } else {
            modObj = loadedObj;
        }
        jsHandleScope.Escape(modObj);
    }

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, modObj);
    if (sharedRef != nullptr) {
        return JSValue::FromEtsObject(sharedRef->GetEtsObject());
    }
    return JSValue::CreateRefValue(executionCtx, ctx, modObj, napi_object);
}

JSValue *JSRuntimeLoadModule(EtsString *module)
{
    return JSRuntimeLoadModule(module, GetCallerABCPath());
}

uint8_t JSRuntimeStrictEqual([[maybe_unused]] JSValue *lhs, [[maybe_unused]] JSValue *rhs)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    NAPI_CHECK_FATAL(napi_strict_equals(env, JSConvertJSValue::WrapWithNullCheck(env, lhs),
                                        JSConvertJSValue::WrapWithNullCheck(env, rhs), &result));
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasProperty(EtsObject *etsObject, EtsString *name)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_has_property(env, JSConvertEtsObject::WrapWithNullCheck(env, etsObject),
                                     JSConvertString::WrapWithNullCheck(env, name), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, executionCtx, result);
    return static_cast<uint8_t>(result);
}

static napi_value JSRuntimeGetPropertyImpl(EtsExecutionContext *executionCtx, napi_env env, JSValue *object,
                                           JSValue *property)
{
    auto jsThis = JSConvertJSValue::WrapWithNullCheck(env, object);
    auto key = JSConvertJSValue::WrapWithNullCheck(env, property);

    napi_value result;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        NAPI_CHECK_FATAL(napi_get_property(env, jsThis, key, &result));
    }
    return result;
}

JSValue *JSRuntimeGetProperty(JSValue *object, JSValue *property)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto result = JSRuntimeGetPropertyImpl(executionCtx, env, object, property);
    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(executionCtx);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return {};
    }

    return res.value();
}

void SetPropertyWithObject(JSValue *object, JSValue *property, EtsObject *value)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto jsThis = JSConvertJSValue::WrapWithNullCheck(env, object);
    auto key = JSConvertJSValue::WrapWithNullCheck(env, property);
    auto jsValue = JSConvertEtsObject::WrapWithNullCheck(env, value);

    napi_status jsStatus {};
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_set_property(env, jsThis, key, jsValue);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(executionCtx);
        return;
    }
}

EtsObject *GetPropertyObject(JSValue *object, JSValue *property)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    auto result = JSRuntimeGetPropertyImpl(executionCtx, env, object, property);
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, result).value();
}

EtsObject *InvokeWithObjectReturn(EtsObject *thisObj, EtsObject *func, Span<VMHandle<ObjectHeader>> args)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    PandaVector<napi_value> realArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto realArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        if (realArg == nullptr) {
            ctx->ForwardJSException(executionCtx);
            return nullptr;
        }
        realArgs.push_back(realArg);
    }

    auto receiver = JSConvertEtsObject::WrapWithNullCheck(env, thisObj);
    auto jsFunc = JSConvertEtsObject::WrapWithNullCheck(env, func);
    napi_value jsRet;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_call_function(env, receiver, jsFunc, realArgs.size(), realArgs.data(), &jsRet);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(executionCtx);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsRet).value();
}

uint8_t JSRuntimeHasPropertyObject(EtsObject *object, EtsObject *property)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_has_property(env, JSConvertEtsObject::WrapWithNullCheck(env, object),
                                     JSConvertEtsObject::WrapWithNullCheck(env, property), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, executionCtx, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasElement(EtsObject *object, int index)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        auto objectConv = JSRefConvertResolve(ctx, object->GetClass()->GetRuntimeClass());
        jsStatus = napi_has_element(env, objectConv->Wrap(ctx, object), index, &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, executionCtx, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasOwnProperty(EtsObject *object, EtsString *name)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_has_own_property(env, JSConvertEtsObject::WrapWithNullCheck(env, object),
                                         JSConvertString::WrapWithNullCheck(env, name), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, executionCtx, result);
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeHasOwnPropertyObject(EtsObject *etsObject, EtsObject *etsProperty)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_has_own_property(env, JSConvertEtsObject::WrapWithNullCheck(env, etsObject),
                                         JSConvertEtsObject::WrapWithNullCheck(env, etsProperty), &result);
    }
    CHECK_NAPI_STATUS(jsStatus, ctx, executionCtx, result);
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
            auto executionCtx = EtsExecutionContext::GetCurrent();
            auto ctx = InteropCtx::Current(executionCtx);
            INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
            auto env = ctx->GetJSEnv();
            NapiScope jsHandleScope(env);
            napi_value jsValue = JSConvertJSValue::Wrap(env, object);
            // (note: need to use JS_TYPE, #ICIFWJ)
            if (IsConstructor<true>(env, jsValue, "Number")) {
                return EtsString::CreateFromMUtf8("number");
            }
            if (IsConstructor<true>(env, jsValue, "Boolean")) {
                return EtsString::CreateFromMUtf8("boolean");
            }
            if (IsConstructor<true>(env, jsValue, "String")) {
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

EtsObject *JSRuntimeInvoke(EtsObject *recv, EtsObject *func, EtsArray *args)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, func)) {
        return nullptr;
    }
    HandleScope<ObjectHeader *> scope(executionCtx->GetMT());
    size_t argc = args->GetLength();
    std::vector<VMHandle<ObjectHeader>> argsVec;
    if (argc > 0) {
        argsVec.reserve(argc);
        for (size_t i = 0; i < argc; i++) {
            auto objHeader = args->GetCoreType()->Get<ObjectHeader *>(i);
            argsVec.emplace_back(VMHandle<ObjectHeader>(executionCtx->GetMT(), objHeader));
        }
    }

    Span<VMHandle<ObjectHeader>> internalArgs = Span<VMHandle<ObjectHeader>>(argsVec.data(), argc);
    if (recv == nullptr) {
        return EtsCall(executionCtx->GetMT(), func, internalArgs);
    }
    return EtsCallThis(executionCtx->GetMT(), recv, func, internalArgs);
}

EtsObject *JSRuntimeInstantiate(EtsObject *callable, EtsArray *args)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();

    HandleScope<ObjectHeader *> scope(executionCtx->GetMT());
    size_t argc = args->GetLength();
    std::vector<VMHandle<ObjectHeader>> argsVec;
    if (argc > 0) {
        argsVec.reserve(argc);
        for (size_t i = 0; i < argc; i++) {
            auto objHeader = args->GetCoreType()->Get<ObjectHeader *>(i);
            argsVec.emplace_back(VMHandle<ObjectHeader>(executionCtx->GetMT(), objHeader));
        }
    }
    Span<VMHandle<ObjectHeader>> internalArgs = Span<VMHandle<ObjectHeader>>(argsVec.data(), argc);
    return EtsCallNew(executionCtx->GetMT(), callable, internalArgs);
}

void ESValueAnyIndexedSetter(EtsObject *etsObject, int32_t index, EtsObject *value)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return;
    }
    EtsStbyidx(executionCtx->GetMT(), etsObject, index, value);
}

void ESValueAnyNamedSetter(EtsObject *etsObject, EtsString *etsPropName, EtsObject *value)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return;
    }
    PandaVector<uint8_t> tree8Buf;
    auto name = etsPropName->ConvertToStringView(&tree8Buf);
    PandaString str(name);
    panda_file::File::StringData propName = {etsPropName->GetUtf16Length(), utf::CStringAsMutf8(str.c_str())};
    EtsStbyname(executionCtx->GetMT(), etsObject, propName, value);
}

EtsObject *ESValueAnyIndexedGetter(EtsObject *etsObject, int32_t index)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return nullptr;
    }
    return EtsLdbyidx(executionCtx->GetMT(), etsObject, index);
}

EtsObject *ESValueAnyNamedGetter(EtsObject *etsObject, EtsString *etsPropName)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return nullptr;
    }
    PandaVector<uint8_t> tree8Buf;
    auto name = etsPropName->ConvertToStringView(&tree8Buf);
    PandaString str(name);
    panda_file::File::StringData propName = {etsPropName->GetUtf16Length(), utf::CStringAsMutf8(str.c_str())};
    return EtsLdbyname(executionCtx->GetMT(), etsObject, propName);
}

void ESValueAnyStoreByValue(EtsObject *etsObject, EtsObject *key, EtsObject *value)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return;
    }
    EtsStbyval(executionCtx->GetMT(), etsObject, key, value);
}

EtsObject *ESValueAnyLoadByValue(EtsObject *etsObject, EtsObject *value)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return nullptr;
    }
    return EtsLdbyval(executionCtx->GetMT(), etsObject, value);
}

uint8_t ESValueAnyHasProperty(EtsObject *etsObject, EtsString *name)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    bool result = EtsHasPropertyByName(executionCtx, etsObject, name);
    return static_cast<uint8_t>(result);
}

uint8_t ESValueAnyHasElement(EtsObject *etsObject, int idx)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    bool result = EtsHasPropertyByIdx(executionCtx, etsObject, idx);
    return static_cast<uint8_t>(result);
}

uint8_t ESValueAnyHasPropertyObject(EtsObject *etsObject, EtsObject *property)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    bool result = EtsHasPropertyByValue(executionCtx, etsObject, property, false);
    return static_cast<uint8_t>(result);
}

uint8_t ESValueAnyInstanceOf(EtsObject *etsObject, EtsObject *ctor)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    bool result = false;
    if (etsObject == nullptr || ctor == nullptr) {
        PandaString message = "Need object";
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreError, INTEROP_ARGUMENT_TYPE_MISMATCH,
                          message.c_str());
    } else {
        result = EtsIsinstance(executionCtx->GetMT(), etsObject, ctor);
    }
    return static_cast<uint8_t>(result);
}

uint8_t ESValueAnyHasOwnProperty(EtsObject *etsObject, EtsString *name)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    return static_cast<uint8_t>(EtsHasOwnPropertyByName(executionCtx, etsObject, name));
}

uint8_t ESValueAnyHasOwnPropertyObject(EtsObject *etsObject, EtsObject *property)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (CheckEtsObjectFoundException(executionCtx, etsObject)) {
        return static_cast<uint8_t>(false);
    }
    return static_cast<uint8_t>(EtsHasPropertyByValue(executionCtx, etsObject, property, true));
}

uint8_t ESValueAnyIsStaticValue(EtsObject *etsObject)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return 0;
    }
    auto env = ctx->GetJSEnv();

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (storage->HasReference(etsObject, env)) {
        ets_proxy::SharedReference *sharedRef = storage->GetReference(etsObject);
        if (sharedRef->HasJSFlag()) {
            return static_cast<uint8_t>(false);
        }
    }
    return static_cast<uint8_t>(true);
}

EtsObject *CreateObject(JSValue *ctor, Span<VMHandle<ObjectHeader>> args)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    PandaVector<napi_value> realArgs;

    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        if (arg == nullptr) {
            napi_value jsUndefined;
            napi_get_undefined(env, &jsUndefined);
            realArgs.push_back(jsUndefined);
            continue;
        }
        auto refconv = JSRefConvertResolve(ctx, arg->GetClass()->GetRuntimeClass());
        auto realArg = refconv->Wrap(ctx, arg);
        realArgs.push_back(realArg);
    }

    auto initFunc = JSConvertJSValue::WrapWithNullCheck(env, ctor);
    napi_status jsStatus;
    napi_value retVal;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_new_instance(env, initFunc, realArgs.size(), realArgs.data(), &retVal);
    }

    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(executionCtx);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, retVal).value();
}

uint8_t JSRuntimeIsPromise(JSValue *object)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    bool result = false;
    NAPI_CHECK_FATAL(napi_is_promise(env, JSConvertJSValue::WrapWithNullCheck(env, object), &result));
    return static_cast<uint8_t>(result);
}

uint8_t JSRuntimeInstanceOfStaticType(JSValue *object, EtsTypeAPIType *paramType)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    EtsHandleScope scope(executionCtx);

    if (object == nullptr || paramType == nullptr) {
        return static_cast<uint8_t>(false);
    }

    EtsHandle<EtsTypeAPIType> paramTypeHandle(executionCtx, paramType);
    EtsHandle<JSValue> objectHandle(executionCtx, object);
    EtsField *field = paramType->GetClass()->GetFieldIDByName("cls", nullptr);
    auto clsObj = paramTypeHandle->GetFieldObject(field);
    if (clsObj == nullptr) {
        INTEROP_LOG(ERROR) << "failed to get field by name"
                           << ". Error code: " << std::to_string(INTEROP_FIELD_ACCESS_FAILED);
        return static_cast<uint8_t>(false);
    }
    auto cls = reinterpret_cast<EtsClass *>(clsObj);
    napi_value jsValue = JSConvertJSValue::Wrap(env, objectHandle.GetPtr());
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

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
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

        ScopedNativeCodeThread nativeScope(
            executionCtx->GetMT());  // NOTE: Scope(Native/Managed)CodeThread should be optional here
        status = napi_coerce_to_string(env, napiValue, &strObj);
    }

#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    // #22503 napi_coerce_to_string in arkui/napi implementation returns napi_ok in case of exception.
    if (UNLIKELY(NapiIsExceptionPending(env))) {
        ctx->ForwardJSException(executionCtx);
        return nullptr;
    }
#else
    if (UNLIKELY(status != napi_ok)) {
        INTEROP_FATAL_IF(status != napi_string_expected && status != napi_pending_exception);
        ctx->ForwardJSException(executionCtx);
        return nullptr;
    }
#endif

    auto res = JSConvertString::UnwrapWithNullCheck(ctx, env, strObj);
    if (UNLIKELY(!res.has_value())) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(executionCtx);
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
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    napi_env env = ctx->GetJSEnv();
    auto jsVal = ToLocal(val);
    napi_value res;
    napi_status rc = napi_get_named_property(env, jsVal, propStr, &res);
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(executionCtx);
        ASSERT(NapiIsExceptionPending(env));
        return nullptr;
    }
    return res;
}

void *CompilerGetJSProperty(void *val, void *prop)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    napi_env env = ctx->GetJSEnv();
    auto jsVal = ToLocal(val);
    napi_value res;
    napi_status rc = napi_get_property(env, jsVal, ToLocal(prop), &res);
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(executionCtx);
        ASSERT(NapiIsExceptionPending(env));
        return nullptr;
    }
    return res;
}

void *CompilerGetJSElement(void *val, int32_t index)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto env = ctx->GetJSEnv();

    napi_value res;
    auto rc = napi_get_element(env, ToLocal(val), index, &res);
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        ctx->ForwardJSException(executionCtx);
        return nullptr;
    }
    return res;
}

void *CompilerJSCallCheck(void *fn)
{
    auto jsFn = ToLocal(fn);

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    napi_env env = ctx->GetJSEnv();
    if (UNLIKELY(GetValueType(env, jsFn) != napi_function)) {
        ctx->ThrowJSTypeError(env, INTEROP_CALL_TARGET_IS_NOT_A_FUNCTION, "call target is not a function");
        ctx->ForwardJSException(executionCtx);
        return nullptr;
    }
    return fn;
}

void *CompilerJSNewInstance(void *fn, uint32_t argc, void *args)
{
    auto jsFn = ToLocal(fn);
    auto jsArgs = reinterpret_cast<napi_value *>(args);

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    napi_env env = ctx->GetJSEnv();

    napi_value jsRet;
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_new_instance(env, jsFn, argc, jsArgs, &jsRet);
    }

    if (UNLIKELY(jsStatus != napi_ok)) {
        INTEROP_FATAL_IF(jsStatus != napi_pending_exception);
        ctx->ForwardJSException(executionCtx);
        INTEROP_LOG(DEBUG) << "JSValueJSCall: exit with pending exception";
        return nullptr;
    }
    return jsRet;
}

void CreateLocalScope()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    napi_env env = ctx->GetJSEnv();
    auto scopesStorage = ctx->GetLocalScopesStorage();
    ASSERT(executionCtx->GetMT()->IsCurrentFrameCompiled());
    scopesStorage->CreateLocalScope(env, executionCtx->GetMT()->GetCurrentFrame());
}

void CompilerDestroyLocalScope()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    napi_env env = ctx->GetJSEnv();
    auto scopesStorage = ctx->GetLocalScopesStorage();
    ASSERT(executionCtx->GetMT()->IsCurrentFrameCompiled());
    scopesStorage->DestroyTopLocalScope(env, executionCtx->GetMT()->GetCurrentFrame());
}

void *CompilerLoadJSConstantPool()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    return ctx->GetConstStringStorage()->GetConstantPool();
}

void CompilerInitJSCallClassForCtx(void *klassPtr)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    auto klass = reinterpret_cast<Class *>(klassPtr);
    ctx->GetConstStringStorage()->LoadDynamicCallClass(klass);
}

void *CompilerConvertVoidToLocal()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    napi_env env = ctx->GetJSEnv();
    return GetUndefined(env);
}

void *CompilerConvertRefTypeToLocal(EtsObject *etsValue)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    napi_env env = ctx->GetJSEnv();

    auto ref = etsValue->GetCoreType();
    if (UNLIKELY(ref == nullptr)) {
        return GetNull(env);
    }

    auto klass = ref->template ClassAddr<Class>();
    // start fastpath
    if (klass == PlatformTypes(executionCtx)->interopJSValue->GetRuntimeClass()) {
        auto value = JSValue::FromEtsObject(EtsObject::FromCoreType(ref));
        napi_value res = JSConvertJSValue::Wrap(env, value);
        if (UNLIKELY(res == nullptr)) {
            ctx->ForwardJSException(executionCtx);
        }
        return res;
    }
    if (klass->IsStringClass()) {
        auto value = EtsString::FromEtsObject(EtsObject::FromCoreType(ref));
        napi_value res = JSConvertString::Wrap(env, value);
        if (UNLIKELY(res == nullptr)) {
            ctx->ForwardJSException(executionCtx);
        }
        return res;
    }
    // start slowpath
    HandleScope<ObjectHeader *> scope(executionCtx->GetMT());
    VMHandle<ObjectHeader> handle(executionCtx->GetMT(), ref);
    auto refconv = JSRefConvertResolve(ctx, klass);
    auto res = refconv->Wrap(ctx, EtsObject::FromCoreType(handle.GetPtr()));
    if (UNLIKELY(res == nullptr)) {
        ctx->ForwardJSException(executionCtx);
    }
    return res;
}

EtsString *CompilerConvertLocalToString(void *value)
{
    auto jsVal = ToLocal(value);

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
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
            ctx->ForwardJSException(executionCtx);
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

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
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
            ctx->ForwardJSException(executionCtx);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    ObjectHeader *res = refconv->Unwrap(ctx, jsVal)->GetCoreType();
    if (UNLIKELY(res == nullptr)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(executionCtx);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    return EtsObject::FromCoreType(res);
}

EtsString *JSONStringify(JSValue *jsvalue)
{
    ASSERT(jsvalue != nullptr);

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);
    auto global = GetGlobal(env);
    napi_value jsonInstance;
    napi_value stringify;
    napi_value result;
    auto object = JSConvertJSValue::Wrap(env, jsvalue);
    std::array<napi_value, 1> args = {object};
    napi_status jsStatus;

    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        NAPI_CHECK_FATAL(napi_get_named_property(env, global, "JSON", &jsonInstance));
        NAPI_CHECK_FATAL(napi_get_named_property(env, jsonInstance, "stringify", &stringify));
        jsStatus = napi_call_function(env, jsonInstance, stringify, args.size(), args.data(), &result);
    }
    if (UNLIKELY(jsStatus != napi_ok)) {
        INTEROP_FATAL_IF(jsStatus != napi_pending_exception);
        ctx->ForwardJSException(executionCtx);
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
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetWorker()->IsMainWorker());
    auto *ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    napi_env env = ctx->GetJSEnv();
    napi_value completionValue;

    NapiScope napiScope(env);
    if (value == nullptr) {
        completionValue = GetUndefined(env);
    } else {
        auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
        completionValue = refconv->Wrap(ctx, value);
    }
    ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
    napi_status status = state == EtsPromise::STATE_RESOLVED ? napi_resolve_deferred(env, deferred, completionValue)
                                                             : napi_reject_deferred(env, deferred, completionValue);
    if (status != napi_ok) {
        napi_throw_error(env, std::to_string(INTEROP_NAPI_ERROR_OCCURRED).c_str(), "Cannot resolve promise");
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

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    auto env = ctx->GetJSEnv();

    NapiScope jsHandleScope(env);

    napi_value objectNapiValue = JSConvertJSValue::Wrap(env, objectValue);
    napi_value keyNapiValue = JSConvertJSValue::Wrap(env, keyValue);

    napi_value result;
    auto status = napi_get_property(env, objectNapiValue, keyNapiValue, &result);
    if (UNLIKELY(status == napi_object_expected || NapiThrownGeneric(status))) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(executionCtx);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }
    INTEROP_FATAL_IF(status != napi_ok);

    auto res = JSConvertJSValue::UnwrapWithNullCheck(ctx, env, result);
    if (UNLIKELY(!res)) {
        if (NapiIsExceptionPending(env)) {
            ctx->ForwardJSException(executionCtx);
        }
        ASSERT(ctx->SanityETSExceptionPending());
        return nullptr;
    }

    return res.value();
}

EtsStdCoreArrayBuffer *TransferArrayBufferToStatic(ESValue *object)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    [[maybe_unused]] EtsHandleScope s(executionCtx);
    EtsHandle<JSValue> objHandle(executionCtx, object->GetEo());
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = JSValue::GetNapiValue(executionCtx, ctx, objHandle);

    bool isArrayBuffer = false;
    NAPI_CHECK_FATAL(napi_is_arraybuffer(env, dynamicArrayBuffer, &isArrayBuffer));
    if (!isArrayBuffer) {
        ctx->ThrowETSError(executionCtx, INTEROP_TYPE_NOT_ASSIGNABLE, "Dynamic object is not arraybuffer");
    }

    void *data = nullptr;
    size_t byteLength = 0;
    // NOTE(dslynko, #23919): finalize semantics of resizable ArrayBuffers
    NAPI_CHECK_FATAL(napi_get_arraybuffer_info(env, dynamicArrayBuffer, &data, &byteLength));

    auto *arrayBuffer = EtsStdCoreArrayBuffer::Create(executionCtx, byteLength);
    if (UNLIKELY(arrayBuffer == nullptr)) {
        return nullptr;
    }
    auto *etsData = arrayBuffer->GetData<uint8_t *>();
    if (LIKELY(byteLength > 0 && etsData != nullptr)) {
        std::copy_n(reinterpret_cast<uint8_t *>(data), byteLength, etsData);
    }
    return arrayBuffer;
}

EtsObject *TransferArrayBufferToDynamic(EtsStdCoreArrayBuffer *staticArrayBuffer)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = nullptr;
    void *data;
    // NOTE(dslynko, #23919): finalize semantics of resizable ArrayBuffers
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, staticArrayBuffer->GetByteLength(), &data, &dynamicArrayBuffer));
    std::copy_n(reinterpret_cast<const uint8_t *>(staticArrayBuffer->GetData()), staticArrayBuffer->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));

    JSValue *etsJSValue = JSValue::Create(executionCtx, ctx, dynamicArrayBuffer);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

EtsObject *CreateDynamicTypedArray(EtsStdCoreArrayBuffer *staticArrayBuffer, int32_t typedArrayType, double length,
                                   double byteOffset)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
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

    JSValue *etsJSValue = JSValue::Create(executionCtx, ctx, dynamicTypedArray);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

EtsObject *CreateDynamicDataView(EtsStdCoreArrayBuffer *staticArrayBuffer, double byteLength, double byteOffset)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value dynamicArrayBuffer = nullptr;
    void *data;
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, staticArrayBuffer->GetByteLength(), &data, &dynamicArrayBuffer));
    std::copy_n(reinterpret_cast<const uint8_t *>(staticArrayBuffer->GetData()), staticArrayBuffer->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));

    napi_value dynamicDataView = nullptr;
    napi_create_dataview(env, byteLength, dynamicArrayBuffer, byteOffset, &dynamicDataView);

    JSValue *etsJSValue = JSValue::Create(executionCtx, ctx, dynamicDataView);
    return reinterpret_cast<EtsObject *>(etsJSValue);
}

void SetInteropRuntimeLinker(EtsRuntimeLinker *linker)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);

    ctx->SetDefaultLinkerContext(linker);
}

EtsRuntimeLinker *GetInteropRuntimeLinker()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);

    return ctx->GetDefaultInteropLinker();
}

EtsBoolean IsJSInteropRef(EtsObject *value)
{
    bool res = false;
    if (value->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        res = true;
    }
    return ark::ets::ToEtsBoolean(res);
}

EtsLong SerializeHandle(JSValue *value)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    auto env = ctx->GetJSEnv();

    [[maybe_unused]] EtsHandleScope s(executionCtx);
    EtsHandle<JSValue> valueHandle(executionCtx, value);
    napi_value nv = JSValue::GetRefValue(env, valueHandle);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void *serializedData;
    napi_status status = napi_serialize_hybrid(env, nv, undefined, undefined, &serializedData);
    INTEROP_FATAL_IF(status != napi_ok);

    return reinterpret_cast<EtsLong>(serializedData);
}

JSValue *DeserializeHandle(EtsLong value)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current();
    auto env = ctx->GetJSEnv();

    void *serializedData = reinterpret_cast<void *>(value);

    napi_value retVal;
    napi_status status = napi_deserialize_hybrid(env, serializedData, &retVal);
    INTEROP_FATAL_IF(status != napi_ok);

    return JSValue::Create(executionCtx, ctx, retVal);
}

EtsObject *JSRuntimeInvokeDynamicFunction(EtsObject *functionObject, EtsObjectArray *args)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);

    HandleScope<ObjectHeader *> scope(executionCtx->GetMT());
    size_t argc = args->GetLength();
    PandaVector<VMHandle<ObjectHeader>> argsVec;
    argsVec.reserve(argc);
    for (size_t idx = 0; idx != argc; idx++) {
        ObjectHeader *argHeader = args->Get(idx)->GetCoreType();
        argsVec.emplace_back(VMHandle<ObjectHeader>(executionCtx->GetMT(), argHeader));
    }

    EtsHandle<EtsObject> functionHandle(executionCtx, functionObject);
    auto xRefObjectOperator = XRefObjectOperator::FromEtsObject(functionHandle);
    return xRefObjectOperator.Invoke(executionCtx, Span<VMHandle<ObjectHeader>>(argsVec.data(), argsVec.size()));
}

}  // namespace ark::ets::interop::js
