/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "js_convert_base.h"

namespace panda::ets::interop::js {

template <typename Cpptype>
struct JSConvertNumeric : public JSConvertBase<JSConvertNumeric<Cpptype>, Cpptype> {
    static constexpr const char *TYPE_NAME = "number";

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_integral_v<P>, napi_value> WrapImpl(napi_env env, Cpptype etsVal)
    {
        napi_value jsVal;
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
        if (UNLIKELY(GetValueType(env, jsVal) != napi_number)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        Cpptype etsVal;
        if constexpr (sizeof(Cpptype) >= sizeof(int32_t)) {
            int64_t val;
            NAPI_CHECK_FATAL(napi_get_value_int64(env, jsVal, &val));
            etsVal = val;
        } else if constexpr (std::is_signed_v<Cpptype>) {
            int32_t val;
            NAPI_CHECK_FATAL(napi_get_value_int32(env, jsVal, &val));
            etsVal = val;
        } else {
            uint32_t val;
            NAPI_CHECK_FATAL(napi_get_value_uint32(env, jsVal, &val));
            etsVal = val;
        }
        return etsVal;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, napi_value> WrapImpl(napi_env env, Cpptype etsVal)
    {
        napi_value jsVal;
        NAPI_CHECK_FATAL(napi_create_double(env, etsVal, &jsVal));
        return jsVal;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, std::optional<Cpptype>> UnwrapImpl(
        [[maybe_unused]] InteropCtx *ctx, napi_env env, napi_value jsVal)
    {
        if (UNLIKELY(GetValueType(env, jsVal) != napi_number)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        double val;
        NAPI_CHECK_FATAL(napi_get_value_double(env, jsVal, &val));
        return val;
    }
};

using JSConvertI8 = JSConvertNumeric<int8_t>;
using JSConvertU8 = JSConvertNumeric<uint8_t>;
using JSConvertI16 = JSConvertNumeric<int16_t>;
using JSConvertU16 = JSConvertNumeric<uint16_t>;
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
    if (UNLIKELY(GetValueType(env, jsVal) != napi_boolean)) {
        TypeCheckFailed();
        return {};
    }
    bool val;
    NAPI_CHECK_FATAL(napi_get_value_bool(env, jsVal, &val));
    return val;
}

JSCONVERT_DEFINE_TYPE(StdlibBoolean, EtsObject *);
JSCONVERT_WRAP(StdlibBoolean)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsBoolean> *>(etsVal);
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_get_boolean(env, val->GetValue(), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibBoolean)
{
    bool val;
    NAPI_CHECK_FATAL(napi_get_value_bool(env, jsVal, &val));
    return EtsBoxPrimitive<EtsBoolean>::Create(EtsCoroutine::GetCurrent(), static_cast<EtsBoolean>(val));
}

JSCONVERT_DEFINE_TYPE(StdlibByte, EtsObject *);
JSCONVERT_WRAP(StdlibByte)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsByte> *>(etsVal);
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_create_int32(env, val->GetValue(), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibByte)
{
    int32_t val;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, jsVal, &val));
    if (val < std::numeric_limits<EtsByte>::min() || val > std::numeric_limits<EtsByte>::max()) {
        TypeCheckFailed();
        return {};
    }
    return EtsBoxPrimitive<EtsByte>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(StdlibChar, EtsObject *);
JSCONVERT_WRAP(StdlibChar)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsChar> *>(etsVal);
    napi_value jsVal;
    std::array<char16_t, 2U> str = {static_cast<char16_t>(val->GetValue()), 0};
    NAPI_CHECK_FATAL(napi_create_string_utf16(env, str.data(), 1, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibChar)
{
    napi_valuetype type;
    napi_typeof(env, jsVal, &type);
    EtsChar val;
    if (type == napi_number) {
        int32_t ival;
        NAPI_CHECK_FATAL(napi_get_value_int32(env, jsVal, &ival));
        if (ival < 0 || ival > std::numeric_limits<EtsChar>::max()) {
            TypeCheckFailed();
            return {};
        }
        val = static_cast<uint16_t>(ival);
    } else if (type == napi_string) {
        size_t len = 0;
        NAPI_CHECK_FATAL(napi_get_value_string_utf16(env, jsVal, nullptr, 0, &len));
        if (len != 1) {
            TypeCheckFailed();
            return {};
        }
        char16_t cval;
        NAPI_CHECK_FATAL(napi_get_value_string_utf16(env, jsVal, &cval, 1, &len));
        val = static_cast<EtsChar>(cval);
    } else {
        TypeCheckFailed();
        return {};
    }
    return EtsBoxPrimitive<EtsChar>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(StdlibShort, EtsObject *);
JSCONVERT_WRAP(StdlibShort)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsShort> *>(etsVal);
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_create_int32(env, val->GetValue(), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibShort)
{
    int32_t val;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, jsVal, &val));
    if (val < std::numeric_limits<EtsShort>::min() || val > std::numeric_limits<EtsShort>::max()) {
        TypeCheckFailed();
        return {};
    }
    return EtsBoxPrimitive<EtsShort>::Create(EtsCoroutine::GetCurrent(), static_cast<EtsShort>(val));
}

JSCONVERT_DEFINE_TYPE(StdlibInt, EtsObject *);
JSCONVERT_WRAP(StdlibInt)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsInt> *>(etsVal);
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_create_int32(env, val->GetValue(), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibInt)
{
    EtsInt val;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, jsVal, &val));
    return EtsBoxPrimitive<EtsInt>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(StdlibLong, EtsObject *);
JSCONVERT_WRAP(StdlibLong)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsLong> *>(etsVal);
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_create_int64(env, val->GetValue(), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibLong)
{
    EtsLong val;
    NAPI_CHECK_FATAL(napi_get_value_int64(env, jsVal, &val));
    return EtsBoxPrimitive<EtsLong>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(StdlibFloat, EtsObject *);
JSCONVERT_WRAP(StdlibFloat)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsFloat> *>(etsVal);
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_create_double(env, static_cast<double>(val->GetValue()), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibFloat)
{
    double val;
    NAPI_CHECK_FATAL(napi_get_value_double(env, jsVal, &val));
    auto fval = static_cast<EtsFloat>(val);
    if (fval != val) {
        TypeCheckFailed();
        return {};
    }
    return EtsBoxPrimitive<EtsFloat>::Create(EtsCoroutine::GetCurrent(), fval);
}

JSCONVERT_DEFINE_TYPE(StdlibDouble, EtsObject *);
JSCONVERT_WRAP(StdlibDouble)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsDouble> *>(etsVal);
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_create_double(env, val->GetValue(), &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(StdlibDouble)
{
    EtsDouble val;
    NAPI_CHECK_FATAL(napi_get_value_double(env, jsVal, &val));
    return EtsBoxPrimitive<EtsDouble>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(String, EtsString *);
JSCONVERT_WRAP(String)
{
    napi_value jsVal;
    if (UNLIKELY(etsVal->IsUtf16())) {
        auto str = reinterpret_cast<char16_t *>(etsVal->GetDataUtf16());
        NAPI_CHECK_FATAL(napi_create_string_utf16(env, str, etsVal->GetUtf16Length(), &jsVal));
    } else {
        auto str = utf::Mutf8AsCString(etsVal->GetDataMUtf8());
        // -1 for NULL terminated Mutf8 string!!!
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, str, etsVal->GetMUtf8Length() - 1, &jsVal));
    }
    return jsVal;
}
JSCONVERT_UNWRAP(String)
{
    if (UNLIKELY(GetValueType(env, jsVal) != napi_string)) {
        TypeCheckFailed();
        return {};
    }
    std::string value = GetString(env, jsVal);
    return EtsString::CreateFromUtf8(value.data(), value.length());
}

JSCONVERT_DEFINE_TYPE(Void, EtsVoid *);
JSCONVERT_WRAP(Void)
{
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_get_undefined(env, &jsVal));
    return jsVal;
}
JSCONVERT_UNWRAP(Void)
{
    return EtsVoid::GetInstance();
}

JSCONVERT_DEFINE_TYPE(JSValue, JSValue *);
JSCONVERT_WRAP(JSValue)
{
    return etsVal->GetNapiValue(env);
}
JSCONVERT_UNWRAP(JSValue)
{
    return JSValue::Create(EtsCoroutine::GetCurrent(), ctx, jsVal);
}

// JSError convertors are supposed to box JSValue objects, do not treat them in any other way
JSCONVERT_DEFINE_TYPE(JSError, EtsObject *);
JSCONVERT_WRAP(JSError)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    auto klass = etsVal->GetClass();
    INTEROP_FATAL_IF(klass->GetRuntimeClass() != ctx->GetJSErrorClass());

    // NOTE(vpukhov): remove call after adding a mirror-class for JSError
    auto method = klass->GetMethod("getValue");
    ASSERT(method != nullptr);
    std::array args = {Value(etsVal->GetCoreType())};
    auto val = JSValue::FromCoreType(method->GetPandaMethod()->Invoke(coro, args.data()).GetAs<ObjectHeader *>());
    INTEROP_FATAL_IF(val == nullptr);
    return val->GetNapiValue(env);
}
JSCONVERT_UNWRAP(JSError)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto value = JSValue::Create(coro, ctx, jsVal);
    if (UNLIKELY(value == nullptr)) {
        return {};
    }
    auto res = ctx->CreateETSCoreJSError(coro, value);
    if (UNLIKELY(res == nullptr)) {
        return {};
    }
    return res;
}

JSCONVERT_DEFINE_TYPE(Promise, EtsPromise *);

JSCONVERT_WRAP(Promise)
{
    napi_deferred deferred;
    napi_value jsPromise;
    NAPI_CHECK_FATAL(napi_create_promise(env, &deferred, &jsPromise));
    auto *coro = EtsCoroutine::GetCurrent();
    if (etsVal->GetState() != EtsPromise::STATE_PENDING) {
        EtsObject *value = etsVal->GetValue(coro);
        napi_value completionValue;
        auto ctx = InteropCtx::Current(coro);
        if (value == nullptr) {
            completionValue = GetNull(env);
        } else {
            auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
            completionValue = refconv->Wrap(ctx, value);
        }
        if (etsVal->IsResolved()) {
            NAPI_CHECK_FATAL(napi_resolve_deferred(env, deferred, completionValue));
        } else {
            NAPI_CHECK_FATAL(napi_reject_deferred(env, deferred, completionValue));
        }
    } else {
        coro->GetPandaVM()->AddPromiseListener(etsVal, MakePandaUnique<PendingPromiseListener>(deferred));
    }
    return jsPromise;
}

JSCONVERT_UNWRAP(Promise)
{
    bool isPromise = false;
    NAPI_CHECK_FATAL(napi_is_promise(env, jsVal, &isPromise));
    if (!isPromise) {
        InteropCtx::Fatal("Passed object is not a promise!");
        UNREACHABLE();
    }
    auto currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(currentCoro);
    auto *promise = EtsPromise::Create(currentCoro);
    EtsHandle<EtsPromise> hpromise(currentCoro, promise);
    auto *jsval = JSValue::Create(currentCoro, ctx, jsVal);
    hpromise->SetLinkedPromise(currentCoro, jsval->AsObject());
    return hpromise.GetPtr();
}

JSCONVERT_DEFINE_TYPE(ArrayBuffer, EtsArrayBuffer *);

JSCONVERT_WRAP(ArrayBuffer)
{
    napi_value buf;
    void *data;
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, etsVal->GetByteLength(), &data, &buf));
    std::copy_n(reinterpret_cast<const uint8_t *>(etsVal->GetData()->GetData<const void *>()), etsVal->GetByteLength(),
                reinterpret_cast<uint8_t *>(data));
    return buf;
}

JSCONVERT_UNWRAP(ArrayBuffer)
{
    bool isArraybuf = false;
    NAPI_CHECK_FATAL(napi_is_arraybuffer(env, jsVal, &isArraybuf));
    if (!isArraybuf) {
        InteropCtx::Fatal("Passed object is not an ArrayBuffer!");
        UNREACHABLE();
    }
    void *data = nullptr;
    size_t byteLength = 0;
    NAPI_CHECK_FATAL(napi_get_arraybuffer_info(env, jsVal, &data, &byteLength));
    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(currentCoro);
    EtsClass *arraybufKlass = currentCoro->GetPandaVM()->GetClassLinker()->GetArrayBufferClass();
    EtsHandle<EtsArrayBuffer> buf(currentCoro,
                                  reinterpret_cast<EtsArrayBuffer *>(EtsObject::Create(currentCoro, arraybufKlass)));
    buf->SetByteLength(static_cast<EtsInt>(byteLength));
    buf->SetData(currentCoro, EtsArray::CreateForPrimitive<EtsByteArray>(EtsClassRoot::BYTE_ARRAY, byteLength));
    std::copy_n(reinterpret_cast<uint8_t *>(data), byteLength, buf->GetData()->GetData<EtsByte>());
    return buf.GetPtr();
}

JSCONVERT_DEFINE_TYPE(EtsVoid, EtsVoid *);
JSCONVERT_WRAP(EtsVoid)
{
    return GetUndefined(env);
}
JSCONVERT_UNWRAP(EtsVoid)
{
    return EtsVoid::GetInstance();
}

JSCONVERT_DEFINE_TYPE(EtsUndefined, EtsObject *);
JSCONVERT_WRAP(EtsUndefined)
{
    return GetUndefined(env);
}

JSCONVERT_UNWRAP(EtsUndefined)
{
    if (UNLIKELY(!IsUndefined(env, jsVal))) {
        TypeCheckFailed();
        return {};
    }
    return ctx->GetUndefinedObject();
}

#undef JSCONVERT_DEFINE_TYPE
#undef JSCONVERT_WRAP
#undef JSCONVERT_UNWRAP

template <typename T>
static ALWAYS_INLINE inline std::optional<typename T::cpptype> JSValueGetByName(InteropCtx *ctx, JSValue *jsvalue,
                                                                                const char *name)
{
    auto env = ctx->GetJSEnv();
    napi_value jsVal = jsvalue->GetNapiValue(env);
    napi_status rc = napi_get_named_property(env, jsVal, name, &jsVal);
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        return {};
    }
    return T::UnwrapWithNullCheck(ctx, env, jsVal);
}

template <typename T>
[[nodiscard]] static ALWAYS_INLINE inline bool JSValueSetByName(InteropCtx *ctx, JSValue *jsvalue, const char *name,
                                                                typename T::cpptype etsPropVal)
{
    auto env = ctx->GetJSEnv();
    napi_value jsVal = jsvalue->GetNapiValue(env);
    napi_value jsPropVal = T::WrapWithNullCheck(env, etsPropVal);
    if (UNLIKELY(jsPropVal == nullptr)) {
        return false;
    }
    napi_status rc = napi_set_named_property(env, jsVal, name, jsPropVal);
    return !NapiThrownGeneric(rc);
}

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H
