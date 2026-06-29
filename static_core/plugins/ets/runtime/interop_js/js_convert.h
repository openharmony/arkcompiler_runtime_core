/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H

#include <variant>
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/interop_js/js_convert_base.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/interop_js/js_convert_stdlib.h"

namespace ark::ets::interop::js {

template <typename Cpptype>
struct JSConvertNumeric : public JSConvertBase<JSConvertNumeric<Cpptype>, Cpptype> {
    static constexpr const char *TYPE_NAME = "number";

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_integral_v<P>, napi_value> WrapImpl(napi_env env, Cpptype etsVal)
    {
        napi_value jsVal;
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        if constexpr (sizeof(Cpptype) >= sizeof(int32_t)) {
            NAPI_CHECK_FATAL(napi_create_int64(env, etsVal, &jsVal));
        } else if constexpr (std::is_signed_v<Cpptype>) {
            NAPI_CHECK_FATAL(napi_create_int32(env, etsVal, &jsVal));
        } else {
            NAPI_CHECK_FATAL(napi_create_uint32(env, etsVal, &jsVal));
        }
        return jsVal;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_integral_v<P>, std::optional<Cpptype>> UnwrapImpl([[maybe_unused]] InteropCtx *ctx,
                                                                                      napi_env env, napi_value jsVal)
    {
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        napi_valuetype valueType = GetValueType(env, jsVal);
        napi_value result = jsVal;
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        Cpptype etsVal;
        if constexpr (sizeof(Cpptype) >= sizeof(int32_t)) {
            int64_t val;
            NAPI_CHECK_FATAL(napi_get_value_int64(env, result, &val));
            etsVal = static_cast<Cpptype>(val);
        } else if constexpr (std::is_signed_v<Cpptype>) {
            int32_t val;
            NAPI_CHECK_FATAL(napi_get_value_int32(env, result, &val));
            etsVal = static_cast<Cpptype>(val);
        } else {
            uint32_t val;
            NAPI_CHECK_FATAL(napi_get_value_uint32(env, result, &val));
            etsVal = static_cast<Cpptype>(val);
        }
        return etsVal;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_integral_v<P>, bool> MatchTypeImpl([[maybe_unused]] napi_env env, napi_value jsVal)
    {
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        napi_valuetype valueType = GetValueType(env, jsVal);
        napi_value result = jsVal;
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            return false;
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            return false;
        }
        double val;
        NAPI_CHECK_FATAL(napi_get_value_double(env, result, &val));
        return std::isfinite(val) && val >= static_cast<double>(std::numeric_limits<Cpptype>::min()) &&
               val <= static_cast<double>(std::numeric_limits<Cpptype>::max()) && val == std::trunc(val);
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, napi_value> WrapImpl(napi_env env, Cpptype etsVal)
    {
        napi_value jsVal;
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        NAPI_CHECK_FATAL(napi_create_double(env, etsVal, &jsVal));
        return jsVal;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, std::optional<Cpptype>> UnwrapImpl(
        [[maybe_unused]] InteropCtx *ctx, napi_env env, napi_value jsVal)
    {
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        napi_valuetype valueType = GetValueType(env, jsVal);
        napi_value result = jsVal;
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        double val;
        NAPI_CHECK_FATAL(napi_get_value_double(env, result, &val));
        return val;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, bool> MatchTypeImpl([[maybe_unused]] napi_env env,
                                                                             napi_value jsVal)
    {
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        napi_valuetype valueType = GetValueType(env, jsVal);
        napi_value result = jsVal;
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_NUMBER, &result)) {
            return false;
        }
        if (UNLIKELY(GetValueType(env, result) != napi_number)) {
            return false;
        }

        if constexpr (std::is_same_v<Cpptype, float>) {
            double val;
            NAPI_CHECK_FATAL(napi_get_value_double(env, result, &val));
            return std::isnan(val) || std::isinf(val) ||
                   (val >= -std::numeric_limits<float>::max() && val <= std::numeric_limits<float>::max());
        }
        return true;
    }

    static bool MatchType(napi_env env, napi_value jsVal)
    {
        return MatchTypeImpl(env, jsVal);
    }
};

using JSConvertI8 = JSConvertNumeric<int8_t>;
using JSConvertU8 = JSConvertNumeric<uint8_t>;
using JSConvertI16 = JSConvertNumeric<int16_t>;
using JSConvertI32 = JSConvertNumeric<int32_t>;
using JSConvertU32 = JSConvertNumeric<uint32_t>;
using JSConvertI64 = JSConvertNumeric<int64_t>;
using JSConvertU64 = JSConvertNumeric<uint64_t>;
using JSConvertF32 = JSConvertNumeric<float>;
using JSConvertF64 = JSConvertNumeric<double>;

JSCONVERT_DEFINE_TYPE(U1, bool);
JSCONVERT_WRAP(U1)
{
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_get_boolean(env, static_cast<bool>(etsVal), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(U1)
{
    if (IsUndefined(env, jsVal)) {
        TypeCheckFailed();
        return {};
    }
    auto objVal = JSConvertStdlibBoolean::UnwrapWithNullCheck(ctx, env, jsVal);
    if (!objVal || (objVal.has_value() && objVal.value() == nullptr)) {
        return {};
    }
    if (objVal.has_value()) {
        return EtsBoxPrimitive<EtsBoolean>::FromCoreType(objVal.value())->GetValue();
    }
    return {};
}
JSCONVERT_MATCH_TYPE_IMPL(U1)
{
    napi_valuetype type = GetValueType<true>(env, jsVal);
    return type == napi_boolean;
}

JSCONVERT_DEFINE_TYPE(U16, char16_t);
JSCONVERT_WRAP(U16)
{
    napi_value jsVal;
    ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_string_utf16(env, &etsVal, 1, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(U16)
{
    if (IsUndefined(env, jsVal)) {
        TypeCheckFailed();
        return {};
    }
    auto objVal = JSConvertStdlibChar::UnwrapWithNullCheck(ctx, env, jsVal);
    if (!objVal || (objVal.has_value() && objVal.value() == nullptr)) {
        return {};
    }
    if (objVal.has_value()) {
        return EtsBoxPrimitive<EtsChar>::FromCoreType(objVal.value())->GetValue();
    }
    return {};
}
JSCONVERT_MATCH_TYPE_IMPL(U16)
{
    napi_valuetype type = GetValueType<true>(env, jsVal);
    return type == napi_string;
}

JSCONVERT_DEFINE_TYPE(String, EtsString *);
JSCONVERT_WRAP(String)
{
    PandaVector<uint8_t> tree8Buf;
    PandaVector<uint16_t> tree16Buf;
    napi_value jsVal;
    if (UNLIKELY(etsVal->IsUtf16())) {
        auto str = reinterpret_cast<char16_t *>(etsVal->IsTreeString() ? etsVal->GetTreeStringDataUtf16(tree16Buf)
                                                                       : etsVal->GetDataUtf16());
        auto length = etsVal->GetUtf16Length();
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        NAPI_CHECK_FATAL(napi_create_string_utf16(env, str, length, &jsVal));
    } else {
        auto str = utf::Mutf8AsCString(etsVal->IsTreeString() ? etsVal->GetTreeStringDataMUtf8(tree8Buf)
                                                              : etsVal->GetDataMUtf8());
        // -1 for NULL terminated Mutf8 string!!!
        auto length = etsVal->GetMUtf8Length();
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, str, length - 1, &jsVal));
    }
    return jsVal;
}
JSCONVERT_UNWRAP(String)
{
    std::variant<std::string, std::u16string> value;
    {
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        napi_value result = jsVal;
        napi_valuetype valueType = GetValueType(env, jsVal);
        if (valueType == napi_object && !GetValueByValueOf(env, jsVal, CONSTRUCTOR_NAME_STRING, &result)) {
            TypeCheckFailed();
            return {};
        }
        if (UNLIKELY(GetValueType(env, result) != napi_string)) {
            TypeCheckFailed();
            return {};
        }
        value = GetStringHybrid(env, result);
    }
    EtsString *resultEtsString = nullptr;
    if (std::holds_alternative<std::string>(value)) {
        const auto &str = std::get<std::string>(value);
        auto utf8Data = reinterpret_cast<const uint8_t *>(str.data());
        auto utf8Length = static_cast<uint32_t>(str.length());
        resultEtsString = EtsString::CreateFromOneByte(utf8Data, utf8Length);
    } else {
        const auto &str = std::get<std::u16string>(value);
        auto utf16Data = reinterpret_cast<const uint16_t *>(str.data());
        auto utf16Length = static_cast<uint32_t>(str.length());
        resultEtsString = EtsString::CreateFromUtf16UnCompressed(utf16Data, utf16Length);
    }
    return resultEtsString;
}
JSCONVERT_MATCH_TYPE_IMPL(String)
{
    napi_valuetype type = GetValueType<true>(env, jsVal);
    return type == napi_string;
}

JSCONVERT_DEFINE_TYPE(BigInt, EtsBigInt *);
JSCONVERT_WRAP(BigInt)
{
    auto size = etsVal->GetBytes()->GetLength();
    auto data = etsVal->GetBytes();

    std::vector<uint32_t> etsArray;
    etsArray.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        etsArray.emplace_back(static_cast<uint32_t>(data->Get(i)));
    }

    SmallVector<uint64_t, 4U> jsArray;
    if (size > 0) {
        jsArray = ConvertBigIntArrayFromEtsToJs(etsArray);
    } else {
        jsArray = {0};
    }

    napi_value jsVal;
    int sign = etsVal->GetSign() < 0 ? 1 : 0;
    ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
    NAPI_CHECK_FATAL(napi_create_bigint_words(env, sign, jsArray.size(), jsArray.data(), &jsVal));

    return jsVal;
}
JSCONVERT_UNWRAP(BigInt)
{
    if (UNLIKELY(GetValueType<true>(env, jsVal) != napi_bigint)) {
        TypeCheckFailed();
        return {};
    }

    auto [words, signBit] = GetBigInt(env, jsVal);
    std::vector<EtsInt> array = ConvertBigIntArrayFromJsToEts(words);

    auto *etsIntArray = EtsIntArray::Create(array.size());
    ASSERT(etsIntArray != nullptr);
    for (uint32_t i = 0; i < array.size(); ++i) {
        etsIntArray->Set(i, array[i]);
    }

    auto executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle etsIntArrayHandle(executionCtx, etsIntArray);

    auto bigintKlass = PlatformTypes(executionCtx)->coreBigInt;
    auto bigInt = EtsBigInt::FromEtsObject(EtsObject::Create(bigintKlass));
    bigInt->SetFieldObject(EtsBigInt::GetBytesOffset(), reinterpret_cast<EtsObject *>(etsIntArrayHandle.GetPtr()));
    bigInt->SetFieldPrimitive(EtsBigInt::GetSignOffset(), array.empty() ? 0 : signBit == 0 ? 1 : -1);

    return bigInt;
}
JSCONVERT_MATCH_TYPE_IMPL(BigInt)
{
    napi_valuetype type = GetValueType<true>(env, jsVal);
    return type == napi_bigint;
}

JSCONVERT_DEFINE_TYPE(JSValue, JSValue *);
JSCONVERT_WRAP(JSValue)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }

    [[maybe_unused]] EtsHandleScope s(executionCtx);
    EtsHandle<JSValue> etsValHandle(executionCtx, etsVal);
    return JSValue::GetNapiValue(executionCtx, ctx, etsValHandle);
}
JSCONVERT_UNWRAP(JSValue)
{
    return JSValue::Create(EtsExecutionContext::GetCurrent(), ctx, jsVal);
}

JSCONVERT_DEFINE_TYPE(Any, EtsObject *);
JSCONVERT_WRAP(Any)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    auto objConv = JSRefConvertResolve(ctx, etsVal->GetClass()->GetRuntimeClass());
    if (objConv == nullptr) {
        ctx->ForwardEtsException(executionCtx);
        return nullptr;
    }
    return objConv->Wrap(ctx, etsVal);
}
JSCONVERT_UNWRAP(Any)
{
    auto *anyClass = EtsExecutionContext::GetCurrent()->GetPandaVM()->GetClassLinker()->GetClassRoot(EtsClassRoot::ANY);
    auto objectConverter = ctx->GetEtsClassWrappersCache()->Lookup(anyClass);
    return objectConverter->Unwrap(ctx, jsVal);
}

JSCONVERT_DEFINE_TYPE(EtsObject, EtsObject *);
JSCONVERT_WRAP(EtsObject)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    auto objConv = JSRefConvertResolve(ctx, etsVal->GetClass()->GetRuntimeClass());
    if (objConv == nullptr) {
        ctx->ForwardEtsException(executionCtx);
        return nullptr;
    }
    return objConv->Wrap(ctx, etsVal);
}
JSCONVERT_UNWRAP(EtsObject)
{
    auto objectConverter = ctx->GetEtsClassWrappersCache()->Lookup(PlatformTypes()->coreObject);
    return objectConverter->Unwrap(ctx, jsVal);
}
JSCONVERT_MATCH_TYPE_IMPL(EtsObject)
{
    napi_valuetype type = GetValueType<true>(env, jsVal);

    if (type == napi_null || type == napi_undefined) {
        return true;
    }

    if (type == napi_object || type == napi_function) {
        return true;  // Generic object match
    }

    return false;
}

// ESError convertors are supposed to box JSValue objects, do not treat them in any other way
JSCONVERT_DEFINE_TYPE(ESError, EtsObject *);
JSCONVERT_WRAP(ESError)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }

    ASSERT_MANAGED_CODE();
    auto klass = etsVal->GetClass();
    INTEROP_FATAL_IF(klass != PlatformTypes()->interopESError);

    auto fieldIdx = etsVal->GetClass()->GetFieldIndexByName("err_");
    auto field = etsVal->GetClass()->GetFieldByIndex(fieldIdx);
    auto etsObject = etsVal->GetFieldObject(field);
    ASSERT(etsObject != nullptr);

    auto method = etsObject->GetClass()->GetInstanceMethod("unwrap", nullptr);
    std::array args {ark::Value(etsObject->GetCoreType())};
    Value res = method->GetPandaMethod()->Invoke(executionCtx->GetMT(), args.data());
    EtsObject *errObject = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());

    interop::js::ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (LIKELY(storage->HasReference(errObject, env))) {
        auto jsThis = storage->GetJsObject(errObject, env);
        return jsThis;
    }
    return JSConvertEtsObject::WrapWithNullCheck(env, errObject);

    UNREACHABLE();
}
JSCONVERT_UNWRAP(ESError)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    EtsObject *etsObject = nullptr;
    bool isError = false;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        NAPI_CHECK_FATAL(napi_is_error(env, jsVal, &isError));
    }
    if (isError) {
        auto jsValueObj = JSValue::Create(executionCtx, ctx, jsVal);
        etsObject = jsValueObj->AsObject();
    } else {
        etsObject = JSConvertAny::UnwrapWithNullCheck(ctx, env, jsVal).value();
    }

    if (UNLIKELY(etsObject == nullptr)) {
        return {};
    }
    auto res = ctx->CreateETSStdCoreESError(executionCtx, etsObject);
    if (UNLIKELY(res == nullptr)) {
        return {};
    }
    return res;
}

JSCONVERT_DEFINE_TYPE(Promise, EtsPromise *);

JSCONVERT_WRAP(Promise)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return {};
    }
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    // SharedReferenceStorage uses object's MarkWord to store interop hash.
    // Also runtime may lock a Promise object (this operation also requires MarkWord modification).
    // Interop hash and locks cannot co-exist. That is why a separate object (EtsPromiseRef) is used for
    // mapping between js and ets Promise objects.
    // When a ets Promise object goes to JS we should get the corresponding EtsPromiseRef object.
    // So the ets Promise object should know about EtsPromiseRef which is stored in 'interopObject' field.
    EtsObject *interopObj = etsVal->GetInteropObject(executionCtx);
    if (interopObj != nullptr && storage->HasReference(interopObj, env)) {
        return storage->GetJsObject(interopObj, env);
    }

    [[maybe_unused]] EtsHandleScope s(executionCtx);
    EtsHandle<EtsPromise> hpromise(executionCtx, etsVal);
    ASSERT(hpromise.GetPtr() != nullptr);
    napi_deferred deferred;
    napi_value jsPromise;
    NAPI_CHECK_FATAL(napi_create_promise(env, &deferred, &jsPromise));

    hpromise->Lock();
    // NOTE(alimovilya, #23064) This if should be removed. Only else branch should remain.
    if (!hpromise->IsPending() && !hpromise->IsLinked()) {  // it will never get PENDING again
        EtsHandle<EtsObject> value(executionCtx, hpromise->GetValue(executionCtx));
        napi_value completionValue;
        if (value.GetPtr() == nullptr) {
            completionValue = GetUndefined(env);
        } else {
            auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
            completionValue = refconv->Wrap(ctx, value.GetPtr());
        }
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        if (hpromise->IsResolved()) {
            NAPI_CHECK_FATAL(napi_resolve_deferred(env, deferred, completionValue));
        } else {
            NAPI_CHECK_FATAL(napi_reject_deferred(env, deferred, completionValue));
        }
    } else {
        // connect->Invoke calls EtsPromiseSubmitCallback that acquires the mutex and checks the state again
        hpromise->Unlock();
        ASSERT_MANAGED_CODE();
        std::array<Value, 2U> args = {Value(hpromise.GetPtr()->GetCoreType()),
                                      Value(reinterpret_cast<int64_t>(deferred))};
        PlatformTypes(executionCtx)
            ->interopPromiseInteropConnectPromise->GetPandaMethod()
            ->Invoke(executionCtx->GetMT(), args.data());
        hpromise->Lock();
    }
    EtsPromiseRef *ref = EtsPromiseRef::Create(executionCtx);
    ref->SetTarget(executionCtx, hpromise->AsObject());
    hpromise->SetInteropObject(executionCtx, ref);
    EtsHandle<EtsObject> refHandle(executionCtx, ref->AsObject());
    [[maybe_unused]] auto *sharedRef = storage->CreateETSObjectRef(ctx, refHandle, jsPromise);
    ASSERT(sharedRef != nullptr);
    hpromise->Unlock();
    return jsPromise;
}

JSCONVERT_UNWRAP(Promise)
{
#ifndef NDEBUG
    bool isPromise = false;
    NAPI_CHECK_FATAL(napi_is_promise(env, jsVal, &isPromise));
    ASSERT(isPromise);
#endif
    auto executionCtx = EtsExecutionContext::GetCurrent();
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, jsVal);
    if (sharedRef != nullptr) {
        auto *ref = reinterpret_cast<EtsPromiseRef *>(sharedRef->GetEtsObject());
        ASSERT(ref->GetTarget(executionCtx) != nullptr);
        return EtsPromise::FromEtsObject(ref->GetTarget(executionCtx));
    }

    [[maybe_unused]] EtsHandleScope s(executionCtx);
    auto *promise = EtsPromise::Create(executionCtx);
    EtsHandle<EtsPromise> hpromise(executionCtx, promise);
    ASSERT(hpromise.GetPtr() != nullptr);
    EtsHandle<EtsPromiseRef> href(executionCtx, EtsPromiseRef::Create(executionCtx));
    href->SetTarget(executionCtx, hpromise->AsObject());
    hpromise->SetInteropObject(executionCtx, href.GetPtr());
    EtsHandle<EtsObject> etsObject(executionCtx, href->AsObject());
    storage->CreateJSObjectRef(ctx, etsObject, jsVal);
    hpromise->SetLinkedPromise(executionCtx, href->AsObject());
    EtsPromise::CreateLink(hpromise->GetLinkedPromise(executionCtx), hpromise.GetPtr());
    return hpromise.GetPtr();
}

JSCONVERT_DEFINE_TYPE(EtsNull, EtsObject *);
JSCONVERT_WRAP(EtsNull)
{
    return GetNull(env);
}

JSCONVERT_UNWRAP(EtsNull)
{
    if (UNLIKELY(!IsNull(env, jsVal))) {
        TypeCheckFailed();
        return {};
    }
    return ctx->GetNullValue();
}

#undef JSCONVERT_DEFINE_TYPE
#undef JSCONVERT_WRAP
#undef JSCONVERT_UNWRAP
#undef JSCONVERT_MATCH_TYPE_IMPL

ALWAYS_INLINE inline bool CheckArgMatchObject(napi_env env, EtsClass *cls, napi_value jsArg)
{
    if (cls->IsStringClass()) {
        return JSConvertString::MatchType(env, jsArg);
    }
    if (cls == PlatformTypes()->coreBigInt) {
        return JSConvertBigInt::MatchType(env, jsArg);
    }

    if (cls->GetRuntimeClass()->IsObjectClass() || cls->GetRuntimeClass()->IsAnyClass() ||
        cls == PlatformTypes()->interopJSValue) {
        return true;
    }

    if (cls == PlatformTypes()->coreArray || cls == PlatformTypes()->coreSet || cls == PlatformTypes()->coreMap) {
        InteropCtx::ThrowJSTypeError(env, "Overload resolution for Array, Map and Set is not supported");
        return false;
    }

    napi_valuetype type = GetValueType<true>(env, jsArg);

    if (type == napi_object || type == napi_function) {
        auto *ctx = InteropCtx::Current();
        if (ctx == nullptr) {
            return false;
        }
        auto *storage = ctx->GetSharedRefStorage();
        auto *sharedRef = storage->GetReference<true>(env, jsArg);
        if (sharedRef != nullptr) {
            EtsObject *etsObj = sharedRef->GetEtsObject();
            if (etsObj != nullptr) {
                return cls->IsAssignableFrom(etsObj->GetClass());
            }
        }
    }
    return JSConvertEtsObject::MatchType(env, jsArg);
}

// NOLINTBEGIN(modernize-avoid-c-arrays)
ALWAYS_INLINE inline bool CheckArgMatchPrimitive(napi_env env, EtsType argType, napi_value jsArg)
{
    using MatchFunc = bool (*)(napi_env, napi_value);
    static constexpr MatchFunc MATCHERS[] = {
        JSConvertU1::MatchType,  JSConvertI8::MatchType,  JSConvertU16::MatchType, JSConvertI16::MatchType,
        JSConvertI32::MatchType, JSConvertI64::MatchType, JSConvertF32::MatchType, JSConvertF64::MatchType,
    };
    auto idx = static_cast<size_t>(argType);
    return idx < std::size(MATCHERS) ? MATCHERS[idx](env, jsArg) : false;
}
// NOLINTEND(modernize-avoid-c-arrays)

ALWAYS_INLINE inline bool CheckArgMatch(napi_env env, EtsMethod *method, uint32_t idx, napi_value jsArg)
{
    EtsType argType = method->GetArgType(idx);
    if (argType == EtsType::OBJECT) {
        return CheckArgMatchObject(env, method->ResolveArgType(idx), jsArg);
    }
    return CheckArgMatchPrimitive(env, argType, jsArg);
}

template <typename T>
ALWAYS_INLINE inline std::optional<typename T::cpptype> JSValueGetByName(InteropCtx *ctx, JSValue *jsvalue,
                                                                         const char *name)
{
    auto env = ctx->GetJSEnv();
    auto executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(executionCtx);
    EtsHandle<JSValue> jsvalueHandle(executionCtx, jsvalue);
    auto jsVal = JSValue::GetNapiValue(executionCtx, ctx, jsvalueHandle);
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_get_named_property(env, jsVal, name, &jsVal);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(executionCtx);
        return {};
    }
    return T::UnwrapWithNullCheck(ctx, env, jsVal);
}

template <typename T>
ALWAYS_INLINE inline void JSValueSetByName(InteropCtx *ctx, JSValue *jsvalue, const char *name,
                                           typename T::cpptype etsPropVal)
{
    auto env = ctx->GetJSEnv();
    auto executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(executionCtx);
    EtsHandle<JSValue> jsvalueHandle(executionCtx, jsvalue);
    NapiScope jsHandleScope(env);
    napi_value jsPropVal = T::WrapWithNullCheck(env, etsPropVal);
    if (UNLIKELY(jsPropVal == nullptr)) {
        return;
    }
    auto jsVal = JSValue::GetNapiValue(executionCtx, ctx, jsvalueHandle);
    napi_status jsStatus;
    {
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        jsStatus = napi_set_named_property(env, jsVal, name, jsPropVal);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(executionCtx);
    }
}

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H
