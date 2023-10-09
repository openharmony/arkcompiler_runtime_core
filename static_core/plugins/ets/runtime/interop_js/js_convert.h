/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H_

#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/class.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_void.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/interop_js/pending_promise_listener.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "types/ets_void.h"

namespace panda::ets::interop::js {

template <typename T>
inline EtsObject *AsEtsObject(T *obj)
{
    static_assert(std::is_base_of_v<ObjectHeader, T>);
    return reinterpret_cast<EtsObject *>(obj);
}

template <typename T>
inline T *FromEtsObject(EtsObject *obj)
{
    static_assert(std::is_base_of_v<ObjectHeader, T>);
    return reinterpret_cast<T *>(obj);
}

void JSConvertTypeCheckFailed(const char *type_name);
inline void JSConvertTypeCheckFailed(const std::string &s)
{
    JSConvertTypeCheckFailed(s.c_str());
}

// Base mixin class of JSConvert interface
// Represents primitive types and some built-in classes, has no state
template <typename Impl, typename ImplCpptype>
struct JSConvertBase {
    JSConvertBase() = delete;
    using cpptype = ImplCpptype;
    static constexpr bool IS_REFTYPE = std::is_pointer_v<cpptype>;
    static constexpr size_t TYPE_SIZE = IS_REFTYPE ? ClassHelper::OBJECT_POINTER_SIZE : sizeof(cpptype);

    static void TypeCheckFailed()
    {
        JSConvertTypeCheckFailed(Impl::TYPE_NAME);
    }

    // Convert ets->js, returns nullptr if failed, throws JS exceptions
    static napi_value Wrap(napi_env env, cpptype ets_val)
    {
        if constexpr (IS_REFTYPE) {
            ASSERT(ets_val != nullptr);
        }
        auto res = Impl::WrapImpl(env, ets_val);
        ASSERT(res != nullptr || InteropCtx::SanityJSExceptionPending());
        return res;
    }

    // Convert js->ets, returns nullopt if failed, throws ETS/JS exceptions
    static std::optional<cpptype> Unwrap(InteropCtx *ctx, napi_env env, napi_value js_val)
    {
        if constexpr (IS_REFTYPE) {
            ASSERT(!IsNullOrUndefined(env, js_val));
        }
        auto res = Impl::UnwrapImpl(ctx, env, js_val);
        ASSERT(res.has_value() || InteropCtx::SanityJSExceptionPending() || InteropCtx::SanityETSExceptionPending());
        return res;
    }

    static napi_value WrapWithNullCheck(napi_env env, cpptype ets_val)
    {
        if constexpr (IS_REFTYPE) {
            if (UNLIKELY(ets_val == nullptr)) {
                return GetNull(env);
            }
        }
        auto res = Impl::WrapImpl(env, ets_val);
        ASSERT(res != nullptr || InteropCtx::SanityJSExceptionPending());
        return res;
    }

    static std::optional<cpptype> UnwrapWithNullCheck(InteropCtx *ctx, napi_env env, napi_value js_val)
    {
        if constexpr (IS_REFTYPE) {
            if (UNLIKELY(IsNullOrUndefined(env, js_val))) {
                return nullptr;
            }
        }
        auto res = Impl::UnwrapImpl(ctx, env, js_val);
        ASSERT(res.has_value() || InteropCtx::SanityJSExceptionPending() || InteropCtx::SanityETSExceptionPending());
        return res;
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define JSCONVERT_DEFINE_TYPE(type, cpptype_)                                                                 \
    struct JSConvert##type : public JSConvertBase<JSConvert##type, cpptype_> {                                \
        static constexpr const char *TYPE_NAME = #type;                                                       \
        /* Must not fail */                                                                                   \
        [[maybe_unused]] static inline napi_value WrapImpl([[maybe_unused]] napi_env env,                     \
                                                           [[maybe_unused]] cpptype ets_val);                 \
        /* May fail */                                                                                        \
        [[maybe_unused]] static inline std::optional<cpptype> UnwrapImpl([[maybe_unused]] InteropCtx *ctx,    \
                                                                         [[maybe_unused]] napi_env env,       \
                                                                         [[maybe_unused]] napi_value js_val); \
    };

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define JSCONVERT_WRAP(type) \
    inline napi_value JSConvert##type::WrapImpl([[maybe_unused]] napi_env env, [[maybe_unused]] cpptype ets_val)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define JSCONVERT_UNWRAP(type)                                                  \
    inline std::optional<JSConvert##type::cpptype> JSConvert##type::UnwrapImpl( \
        [[maybe_unused]] InteropCtx *ctx, [[maybe_unused]] napi_env env, [[maybe_unused]] napi_value js_val)

template <typename Cpptype>
struct JSConvertNumeric : public JSConvertBase<JSConvertNumeric<Cpptype>, Cpptype> {
    static constexpr const char *TYPE_NAME = "number";

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_integral_v<P>, napi_value> WrapImpl(napi_env env, Cpptype ets_val)
    {
        napi_value js_val;
        if constexpr (sizeof(Cpptype) >= sizeof(int32_t)) {
            NAPI_CHECK_FATAL(napi_create_int64(env, ets_val, &js_val));
        } else if constexpr (std::is_signed_v<Cpptype>) {
            NAPI_CHECK_FATAL(napi_create_int32(env, ets_val, &js_val));
        } else {
            NAPI_CHECK_FATAL(napi_create_uint32(env, ets_val, &js_val));
        }
        return js_val;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_integral_v<P>, std::optional<Cpptype>> UnwrapImpl([[maybe_unused]] InteropCtx *ctx,
                                                                                      napi_env env, napi_value js_val)
    {
        if (UNLIKELY(GetValueType(env, js_val) != napi_number)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        Cpptype ets_val;
        if constexpr (sizeof(Cpptype) >= sizeof(int32_t)) {
            int64_t val;
            NAPI_CHECK_FATAL(napi_get_value_int64(env, js_val, &val));
            ets_val = val;
        } else if constexpr (std::is_signed_v<Cpptype>) {
            int32_t val;
            NAPI_CHECK_FATAL(napi_get_value_int32(env, js_val, &val));
            ets_val = val;
        } else {
            uint32_t val;
            NAPI_CHECK_FATAL(napi_get_value_uint32(env, js_val, &val));
            ets_val = val;
        }
        return ets_val;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, napi_value> WrapImpl(napi_env env, Cpptype ets_val)
    {
        napi_value js_val;
        NAPI_CHECK_FATAL(napi_create_double(env, ets_val, &js_val));
        return js_val;
    }

    template <typename P = Cpptype>
    static std::enable_if_t<std::is_floating_point_v<P>, std::optional<Cpptype>> UnwrapImpl(
        [[maybe_unused]] InteropCtx *ctx, napi_env env, napi_value js_val)
    {
        if (UNLIKELY(GetValueType(env, js_val) != napi_number)) {
            JSConvertNumeric::TypeCheckFailed();
            return {};
        }
        double val;
        NAPI_CHECK_FATAL(napi_get_value_double(env, js_val, &val));
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

JSCONVERT_DEFINE_TYPE(U1, bool)
JSCONVERT_WRAP(U1)
{
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_get_boolean(env, static_cast<bool>(ets_val), &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(U1)
{
    if (UNLIKELY(GetValueType(env, js_val) != napi_boolean)) {
        TypeCheckFailed();
        return {};
    }
    bool val;
    NAPI_CHECK_FATAL(napi_get_value_bool(env, js_val, &val));
    return val;
}

JSCONVERT_DEFINE_TYPE(StdlibBoolean, EtsObject *)
JSCONVERT_WRAP(StdlibBoolean)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsBoolean> *>(ets_val);
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_get_boolean(env, val->GetValue(), &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(StdlibBoolean)
{
    bool val;
    NAPI_CHECK_FATAL(napi_get_value_bool(env, js_val, &val));
    return EtsBoxPrimitive<EtsBoolean>::Create(EtsCoroutine::GetCurrent(), static_cast<EtsBoolean>(val));
}

JSCONVERT_DEFINE_TYPE(StdlibByte, EtsObject *)
JSCONVERT_WRAP(StdlibByte)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsByte> *>(ets_val);
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_create_int32(env, val->GetValue(), &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(StdlibByte)
{
    int32_t val;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, js_val, &val));
    if (val < std::numeric_limits<EtsByte>::min() || val > std::numeric_limits<EtsByte>::max()) {
        TypeCheckFailed();
        return {};
    }
    return EtsBoxPrimitive<EtsByte>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(StdlibChar, EtsObject *)
JSCONVERT_WRAP(StdlibChar)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsChar> *>(ets_val);
    napi_value js_val;
    std::array<char16_t, 2> str = {static_cast<char16_t>(val->GetValue()), 0};
    NAPI_CHECK_FATAL(napi_create_string_utf16(env, str.data(), 1, &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(StdlibChar)
{
    napi_valuetype type;
    napi_typeof(env, js_val, &type);
    EtsChar val;
    if (type == napi_number) {
        int32_t ival;
        NAPI_CHECK_FATAL(napi_get_value_int32(env, js_val, &ival));
        if (ival < 0 || ival > std::numeric_limits<EtsChar>::max()) {
            TypeCheckFailed();
            return {};
        }
        val = static_cast<uint16_t>(ival);
    } else if (type == napi_string) {
        size_t len = 0;
        NAPI_CHECK_FATAL(napi_get_value_string_utf16(env, js_val, nullptr, 0, &len));
        if (len != 1) {
            TypeCheckFailed();
            return {};
        }
        char16_t cval;
        NAPI_CHECK_FATAL(napi_get_value_string_utf16(env, js_val, &cval, 1, &len));
        val = static_cast<EtsChar>(cval);
    } else {
        TypeCheckFailed();
        return {};
    }
    return EtsBoxPrimitive<EtsChar>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(StdlibShort, EtsObject *)
JSCONVERT_WRAP(StdlibShort)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsShort> *>(ets_val);
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_create_int32(env, val->GetValue(), &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(StdlibShort)
{
    int32_t val;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, js_val, &val));
    if (val < std::numeric_limits<EtsShort>::min() || val > std::numeric_limits<EtsShort>::max()) {
        TypeCheckFailed();
        return {};
    }
    return EtsBoxPrimitive<EtsShort>::Create(EtsCoroutine::GetCurrent(), static_cast<EtsShort>(val));
}

JSCONVERT_DEFINE_TYPE(StdlibInt, EtsObject *)
JSCONVERT_WRAP(StdlibInt)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsInt> *>(ets_val);
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_create_int32(env, val->GetValue(), &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(StdlibInt)
{
    EtsInt val;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, js_val, &val));
    return EtsBoxPrimitive<EtsInt>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(StdlibLong, EtsObject *)
JSCONVERT_WRAP(StdlibLong)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsLong> *>(ets_val);
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_create_int64(env, val->GetValue(), &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(StdlibLong)
{
    EtsLong val;
    NAPI_CHECK_FATAL(napi_get_value_int64(env, js_val, &val));
    return EtsBoxPrimitive<EtsLong>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(StdlibFloat, EtsObject *)
JSCONVERT_WRAP(StdlibFloat)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsFloat> *>(ets_val);
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_create_double(env, static_cast<double>(val->GetValue()), &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(StdlibFloat)
{
    double val;
    NAPI_CHECK_FATAL(napi_get_value_double(env, js_val, &val));
    auto fval = static_cast<EtsFloat>(val);
    if (fval != val) {
        TypeCheckFailed();
        return {};
    }
    return EtsBoxPrimitive<EtsFloat>::Create(EtsCoroutine::GetCurrent(), fval);
}

JSCONVERT_DEFINE_TYPE(StdlibDouble, EtsObject *)
JSCONVERT_WRAP(StdlibDouble)
{
    auto *val = reinterpret_cast<EtsBoxPrimitive<EtsDouble> *>(ets_val);
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_create_double(env, val->GetValue(), &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(StdlibDouble)
{
    EtsDouble val;
    NAPI_CHECK_FATAL(napi_get_value_double(env, js_val, &val));
    return EtsBoxPrimitive<EtsDouble>::Create(EtsCoroutine::GetCurrent(), val);
}

JSCONVERT_DEFINE_TYPE(String, EtsString *)
JSCONVERT_WRAP(String)
{
    napi_value js_val;
    if (UNLIKELY(ets_val->IsUtf16())) {
        auto str = reinterpret_cast<char16_t *>(ets_val->GetDataUtf16());
        NAPI_CHECK_FATAL(napi_create_string_utf16(env, str, ets_val->GetUtf16Length(), &js_val));
    } else {
        auto str = utf::Mutf8AsCString(ets_val->GetDataMUtf8());
        // -1 for NULL terminated Mutf8 string!!!
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, str, ets_val->GetMUtf8Length() - 1, &js_val));
    }
    return js_val;
}
JSCONVERT_UNWRAP(String)
{
    if (UNLIKELY(GetValueType(env, js_val) != napi_string)) {
        TypeCheckFailed();
        return {};
    }
    std::string value = GetString(env, js_val);
    return EtsString::CreateFromUtf8(value.data(), value.length());
}

JSCONVERT_DEFINE_TYPE(Void, EtsVoid *)
JSCONVERT_WRAP(Void)
{
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_get_undefined(env, &js_val));
    return js_val;
}
JSCONVERT_UNWRAP(Void)
{
    return EtsVoid::GetInstance();
}

JSCONVERT_DEFINE_TYPE(JSValue, JSValue *)
JSCONVERT_WRAP(JSValue)
{
    return ets_val->GetNapiValue(env);
}
JSCONVERT_UNWRAP(JSValue)
{
    return JSValue::Create(EtsCoroutine::GetCurrent(), ctx, js_val);
}

// JSError convertors are supposed to box JSValue objects, do not treat them in any other way
JSCONVERT_DEFINE_TYPE(JSError, EtsObject *)
JSCONVERT_WRAP(JSError)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    auto klass = ets_val->GetClass();
    INTEROP_FATAL_IF(klass->GetRuntimeClass() != ctx->GetJSErrorClass());

    // NOTE(vpukhov): remove call after adding a mirror-class for JSError
    auto method = klass->GetMethod("getValue");
    ASSERT(method != nullptr);
    std::array args = {Value(ets_val->GetCoreType())};
    auto val = JSValue::FromCoreType(method->GetPandaMethod()->Invoke(coro, args.data()).GetAs<ObjectHeader *>());
    INTEROP_FATAL_IF(val == nullptr);
    return val->GetNapiValue(env);
}
JSCONVERT_UNWRAP(JSError)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto value = JSValue::Create(coro, ctx, js_val);
    if (UNLIKELY(value == nullptr)) {
        return {};
    }
    auto res = ctx->CreateETSCoreJSError(coro, value);
    if (UNLIKELY(res == nullptr)) {
        return {};
    }
    return res;
}

JSCONVERT_DEFINE_TYPE(Promise, EtsPromise *)

JSCONVERT_WRAP(Promise)
{
    napi_deferred deferred;
    napi_value js_promise;
    NAPI_CHECK_FATAL(napi_create_promise(env, &deferred, &js_promise));
    auto *coro = EtsCoroutine::GetCurrent();
    if (ets_val->GetState() != EtsPromise::STATE_PENDING) {
        EtsObject *value = ets_val->GetValue(coro);
        napi_value completion_value;
        auto ctx = InteropCtx::Current(coro);
        if (value == nullptr) {
            completion_value = GetNull(env);
        } else {
            auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
            completion_value = refconv->Wrap(ctx, value);
        }
        if (ets_val->IsResolved()) {
            NAPI_CHECK_FATAL(napi_resolve_deferred(env, deferred, completion_value));
        } else {
            NAPI_CHECK_FATAL(napi_reject_deferred(env, deferred, completion_value));
        }
    } else {
        coro->GetPandaVM()->AddPromiseListener(ets_val, MakePandaUnique<PendingPromiseListener>(deferred));
    }
    return js_promise;
}

JSCONVERT_UNWRAP(Promise)
{
    bool is_promise = false;
    NAPI_CHECK_FATAL(napi_is_promise(env, js_val, &is_promise));
    if (!is_promise) {
        InteropCtx::Fatal("Passed object is not a promise!");
        UNREACHABLE();
    }
    auto current_coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(current_coro);
    auto *promise = EtsPromise::Create(current_coro);
    EtsHandle<EtsPromise> hpromise(current_coro, promise);
    auto *jsval = JSValue::Create(current_coro, ctx, js_val);
    hpromise->SetLinkedPromise(current_coro, jsval->AsObject());
    return hpromise.GetPtr();
}

JSCONVERT_DEFINE_TYPE(ArrayBuffer, EtsArrayBuffer *)

JSCONVERT_WRAP(ArrayBuffer)
{
    napi_value buf;
    void *data;
    NAPI_CHECK_FATAL(napi_create_arraybuffer(env, ets_val->GetByteLength(), &data, &buf));
    memcpy(data, ets_val->GetData()->GetData<const void *>(), ets_val->GetByteLength());
    return buf;
}

JSCONVERT_UNWRAP(ArrayBuffer)
{
    bool is_arraybuf = false;
    NAPI_CHECK_FATAL(napi_is_arraybuffer(env, js_val, &is_arraybuf));
    if (!is_arraybuf) {
        InteropCtx::Fatal("Passed object is not an ArrayBuffer!");
        UNREACHABLE();
    }
    void *data = nullptr;
    size_t byte_length = 0;
    NAPI_CHECK_FATAL(napi_get_arraybuffer_info(env, js_val, &data, &byte_length));
    auto *current_coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(current_coro);
    EtsClass *arraybuf_klass = current_coro->GetPandaVM()->GetClassLinker()->GetArrayBufferClass();
    EtsHandle<EtsArrayBuffer> buf(current_coro,
                                  reinterpret_cast<EtsArrayBuffer *>(EtsObject::Create(current_coro, arraybuf_klass)));
    buf->SetByteLength(static_cast<EtsInt>(byte_length));
    buf->SetData(current_coro, EtsArray::CreateForPrimitive<EtsByteArray>(EtsClassRoot::BYTE_ARRAY, byte_length));
    memcpy(buf->GetData()->GetData<EtsByte>(), data, byte_length);
    return buf.GetPtr();
}

JSCONVERT_DEFINE_TYPE(EtsVoid, EtsVoid *)
JSCONVERT_WRAP(EtsVoid)
{
    return GetUndefined(env);
}
JSCONVERT_UNWRAP(EtsVoid)
{
    return EtsVoid::GetInstance();
}

#undef JSCONVERT_DEFINE_TYPE
#undef JSCONVERT_WRAP
#undef JSCONVERT_UNWRAP

template <typename T>
static ALWAYS_INLINE inline std::optional<typename T::cpptype> JSValueGetByName(InteropCtx *ctx, JSValue *jsvalue,
                                                                                const char *name)
{
    auto env = ctx->GetJSEnv();
    napi_value js_val = jsvalue->GetNapiValue(env);
    napi_status rc = napi_get_named_property(env, js_val, name, &js_val);
    if (UNLIKELY(NapiThrownGeneric(rc))) {
        return {};
    }
    return T::UnwrapWithNullCheck(ctx, env, js_val);
}

template <typename T>
[[nodiscard]] static ALWAYS_INLINE inline bool JSValueSetByName(InteropCtx *ctx, JSValue *jsvalue, const char *name,
                                                                typename T::cpptype ets_prop_val)
{
    auto env = ctx->GetJSEnv();
    napi_value js_val = jsvalue->GetNapiValue(env);
    napi_value js_prop_val = T::WrapWithNullCheck(env, ets_prop_val);
    if (UNLIKELY(js_prop_val == nullptr)) {
        return false;
    }
    napi_status rc = napi_set_named_property(env, js_val, name, js_prop_val);
    return !NapiThrownGeneric(rc);
}

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_CONVERT_H_
