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

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "runtime/mem/local_object_handle.h"

#include "plugins/ets/runtime/interop_js/js_refconvert_array.h"

namespace panda::ets::interop::js {

// JSRefConvert adapter for builtin reference types
template <typename Conv>
class JSRefConvertBuiltin : public JSRefConvert {
public:
    JSRefConvertBuiltin() : JSRefConvert(this) {}

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *obj)
    {
        using ObjType = std::remove_pointer_t<typename Conv::cpptype>;
        return Conv::Wrap(ctx->GetJSEnv(), FromEtsObject<ObjType>(obj));
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value js_value)
    {
        auto res = Conv::Unwrap(ctx, ctx->GetJSEnv(), js_value);
        if (!res) {
            return nullptr;
        }
        return AsEtsObject(res.value());
    }
};

template <typename Conv>
static inline void RegisterBuiltinRefConvertor(JSRefConvertCache *cache, Class *klass)
{
    [[maybe_unused]] bool res = CheckClassInitialized<true>(klass);
    ASSERT(res);
    cache->Insert(klass, std::unique_ptr<JSRefConvert>(new JSRefConvertBuiltin<Conv>()));
}

static ets_proxy::EtsClassWrapper *RegisterEtsProxyForStdClass(
    InteropCtx *ctx, std::string_view descriptor, char const *js_builtin_name = nullptr,
    const ets_proxy::EtsClassWrapper::OverloadsMap *overloads = nullptr)
{
    auto coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    EtsClassLinker *ets_class_linker = vm->GetClassLinker();
    auto ets_class = ets_class_linker->GetClass(descriptor.data());
    if (UNLIKELY(ets_class == nullptr)) {
        ctx->Fatal(std::string("nonexisting class ") + descriptor.data());
    }

    // create ets_proxy bound to js builtin-constructor
    ets_proxy::EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();
    std::unique_ptr<ets_proxy::EtsClassWrapper> wrapper =
        ets_proxy::EtsClassWrapper::Create(ctx, ets_class, js_builtin_name, overloads);
    if (UNLIKELY(wrapper == nullptr)) {
        ctx->Fatal(std::string("ets_proxy creation failed for ") + ets_class->GetDescriptor());
    }
    return cache->Insert(ets_class, std::move(wrapper));
}

// NOLINTNEXTLINE(readability-function-size)
static void RegisterCompatConvertors(InteropCtx *ctx)
{
    /******************************************************************************/
    // Helpers

    namespace descriptors = panda_file_items::class_descriptors;

    auto not_implemented = [](char const *name) __attribute__((noreturn, noinline))
    {
        InteropCtx::Fatal(std::string("compat.") + name + " box is not implemented");
    };
    auto not_assignable = [](char const *name) __attribute__((noinline))->EtsObject *
    {
        JSConvertTypeCheckFailed(name);
        return nullptr;
    };

    auto std_ctor_ref = [](InteropCtx *ctxx, char const *name) {
        napi_env env = ctxx->GetJSEnv();
        napi_value val;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), name, &val));
        INTEROP_FATAL_IF(IsUndefined(env, val));
        napi_ref ref;
        NAPI_CHECK_FATAL(napi_create_reference(env, val, 1, &ref));
        return ref;
    };
    auto check_instanceof = [](napi_env env, napi_value val, napi_ref ctor_ref) {
        bool result;
        NAPI_CHECK_FATAL(napi_instanceof(env, val, GetReferenceValue(env, ctor_ref), &result));
        return result;
    };

    auto builtin_convert = [](auto conv_tag, InteropCtx *ctxx, napi_env env, napi_value js_value) -> EtsObject * {
        auto res = decltype(conv_tag)::type::UnwrapImpl(ctxx, env, js_value);
        if (UNLIKELY(!res.has_value())) {
            return nullptr;
        }
        return AsEtsObject(res.value());
    };

    napi_value js_global_ets;
    {
        auto env = ctx->GetJSEnv();
        NAPI_CHECK_FATAL(napi_create_object(env, &js_global_ets));
        NAPI_CHECK_FATAL(napi_set_named_property(env, GetGlobal(env), "ets", js_global_ets));
    }

    auto register_class = [ctx, js_global_ets](std::string_view descriptor, char const *js_builtin_name = nullptr,
                                               const ets_proxy::EtsClassWrapper::OverloadsMap *overloads = nullptr) {
        ets_proxy::EtsClassWrapper *wclass = RegisterEtsProxyForStdClass(ctx, descriptor, js_builtin_name, overloads);
        auto env = ctx->GetJSEnv();
        auto name = wclass->GetEtsClass()->GetRuntimeClass()->GetName();
        auto js_ctor = wclass->GetJsCtor(env);
        NAPI_CHECK_FATAL(napi_set_named_property(env, js_global_ets, name.c_str(), js_ctor));
        if (js_builtin_name != nullptr) {
            NAPI_CHECK_FATAL(napi_set_named_property(env, js_global_ets, js_builtin_name, js_ctor));
        }
        return wclass;
    };

    constexpr const ets_proxy::EtsClassWrapper::OverloadsMap *NO_OVERLOADS = nullptr;
    constexpr const char *NO_MIRROR = nullptr;

    /******************************************************************************/
    // Definitions for StdClasses

    auto w_object = register_class(descriptors::OBJECT, "Object");

    static const ets_proxy::EtsClassWrapper::OverloadsMap W_ERROR_OVERLOADS {
        {utf::CStringAsMutf8("<ctor>"), "Lstd/core/String;:V"}};
    auto w_error = register_class(descriptors::ERROR, "Error", &W_ERROR_OVERLOADS);
    register_class(descriptors::EXCEPTION, nullptr, &W_ERROR_OVERLOADS);

    static const std::array STD_EXCEPTIONS_LIST = {
        // Errors
        std::make_tuple("Lstd/core/AssertionError;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/DivideByZeroError;", NO_MIRROR, NO_OVERLOADS),
        std::make_tuple("Lstd/core/NullPointerError;", NO_MIRROR, NO_OVERLOADS),
        std::make_tuple("Lstd/core/UncatchedExceptionError;", NO_MIRROR, NO_OVERLOADS),
        std::make_tuple("Lstd/core/RangeError;", NO_MIRROR, &W_ERROR_OVERLOADS),
        // Exceptions
        std::make_tuple("Lstd/core/NullPointerException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/NoDataException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/ArgumentOutOfRangeException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/IndexOutOfBoundsException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/IllegalStateException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/ArithmeticException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/ClassCastException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/ClassNotFoundException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/UnsupportedOperationException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/ArrayStoreException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/NegativeArraySizeException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/IllegalMonitorStateException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/IllegalArgumentException;", NO_MIRROR, &W_ERROR_OVERLOADS),
        std::make_tuple("Lstd/core/InvalidDate;", NO_MIRROR, NO_OVERLOADS),
    };
    for (const auto &[descr, mirror, ovl] : STD_EXCEPTIONS_LIST) {
        register_class(descr, mirror, ovl);
    }

    static const ets_proxy::EtsClassWrapper::OverloadsMap W_ARRAY_OVERLOADS = {
        {utf::CStringAsMutf8("at"), "I:Lstd/core/Object;"},
        {utf::CStringAsMutf8("with"), "DLstd/core/Object;:Lescompat/Array;"},
        {utf::CStringAsMutf8("map"), "LFunctionalInterface-std-core-Object-std-core-Object-0;:Lescompat/Array;"},
        {utf::CStringAsMutf8("forEach"), "LFunctionalInterface-std-core-Object-std-core-void-0;:Lstd/core/void;"},
        {utf::CStringAsMutf8("pop"), ":Lstd/core/Object;"},
        {utf::CStringAsMutf8("fill"), "Lstd/core/Object;:Lescompat/Array;"},
        {utf::CStringAsMutf8("flat"), ":Lescompat/Array;"},
        {utf::CStringAsMutf8("join"), ":Lstd/core/String;"},
        {utf::CStringAsMutf8("push"), "Lstd/core/Object;:D"},
        {utf::CStringAsMutf8("some"), "LFunctionalInterface-std-core-Object-u1-0;:Z"},
        {utf::CStringAsMutf8("sort"), ":Lescompat/Array;"},
        {utf::CStringAsMutf8("every"), "LFunctionalInterface-std-core-Object-u1-0;:Z"},
        {utf::CStringAsMutf8("shift"), ":Lstd/core/Object;"},
        {utf::CStringAsMutf8("slice"), "I:Lescompat/Array;"},
        {utf::CStringAsMutf8("<ctor>"), ":V"},
        {utf::CStringAsMutf8("filter"), "LFunctionalInterface-std-core-Object-u1-0;:Lescompat/Array;"},
        {utf::CStringAsMutf8("<get>length"), ":D"},
        {utf::CStringAsMutf8("reduce"),
         "LFunctionalInterface-std-core-Object-std-core-Object-std-core-Object-0;:Lstd/core/Object;"},
        {utf::CStringAsMutf8("splice"), "II:Lescompat/Array;"},
        {utf::CStringAsMutf8("findLast"), "LFunctionalInterface-std-core-Object-u1-0;:Lstd/core/Object;"},
        {utf::CStringAsMutf8("toSorted"), ":Lescompat/Array;"},
        {utf::CStringAsMutf8("findIndex"), "LFunctionalInterface-std-core-Object-u1-0;:D"},
        {utf::CStringAsMutf8("toSpliced"), "II:Lescompat/Array;"},
        {utf::CStringAsMutf8("copyWithin"), "II:Lescompat/Array;"},
        {utf::CStringAsMutf8("removeMany"), "II:[Lstd/core/Object;"},
        {utf::CStringAsMutf8("toReversed"), ":Lescompat/Array;"},
        {utf::CStringAsMutf8("indexOf"), "Lstd/core/Object;:D"},
        {utf::CStringAsMutf8("includes"), "Lstd/core/Object;:Z"},
        {utf::CStringAsMutf8("lastIndexOf"), "Lstd/core/Object;:D"},
        {utf::CStringAsMutf8("reduceRight"),
         "LFunctionalInterface-std-core-Object-std-core-Object-std-core-Object-0;:Lstd/core/Object;"},
        {utf::CStringAsMutf8("find"), "LFunctionalInterface-std-core-Object-u1-0;:Lstd/core/Object;"},
        {utf::CStringAsMutf8("from"), "[Lstd/core/Object;:Lescompat/Array;"},
        {utf::CStringAsMutf8("isArray"), "[Lstd/core/Object;:Z"},
        {utf::CStringAsMutf8("flatMap"),
         "LFunctionalInterface-std-core-Object-f64-std-core-Object-0;:Lescompat/Array;"},
        {utf::CStringAsMutf8("toLocaleString"), ":Lstd/core/String;"},
    };
    auto w_array = register_class(descriptors::ARRAY, "Array", &W_ARRAY_OVERLOADS);

    NAPI_CHECK_FATAL(napi_object_seal(ctx->GetJSEnv(), js_global_ets));
    /******************************************************************************/
    // JS built-in matchers for StdClasses

    auto m_array = [=](InteropCtx *ctxx, napi_value js_value, bool verified = true) -> EtsObject * {
        napi_env env = ctxx->GetJSEnv();
        bool is_instanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_array(env, js_value, &is_instanceof));
            if (!is_instanceof) {
                return not_assignable("Array");
            }
        }
        return w_array->CreateJSBuiltinProxy(ctxx, js_value);
    };
    w_array->SetJSBuiltinMatcher(m_array);

    // NOTE(vpukhov): compat: obtain from class wrappers when implemented
    napi_ref ctor_typeerror = std_ctor_ref(ctx, "TypeError");
    napi_ref ctor_rangeerror = std_ctor_ref(ctx, "RangeError");
    napi_ref ctor_referenceerror = std_ctor_ref(ctx, "ReferenceError");

    auto m_error = [=](InteropCtx *ctxx, napi_value js_value, bool verified = true) -> EtsObject * {
        napi_env env = ctxx->GetJSEnv();
        bool is_instanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_error(env, js_value, &is_instanceof));
            if (!is_instanceof) {
                return not_assignable("Error");
            }
        }
        // NOTE(vpukhov): compat: remove when compat/Error is implemented
        return builtin_convert(helpers::TypeIdentity<JSConvertJSError>(), ctxx, env, js_value);

        if (check_instanceof(env, js_value, ctor_typeerror)) {
            not_implemented("TypeError");
        }
        if (check_instanceof(env, js_value, ctor_rangeerror)) {
            not_implemented("RangeError");
        }
        if (check_instanceof(env, js_value, ctor_referenceerror)) {
            not_implemented("ReferenceError");
        }
        not_implemented("Error");
    };
    w_error->SetJSBuiltinMatcher(m_error);

    auto m_object_object = [=](InteropCtx *ctxx, napi_value js_value) -> EtsObject * {
        napi_env env = ctxx->GetJSEnv();
        bool is_instanceof;
        NAPI_CHECK_FATAL(napi_is_array(env, js_value, &is_instanceof));
        if (is_instanceof) {
            return m_array(ctxx, js_value);
        }
        NAPI_CHECK_FATAL(napi_is_arraybuffer(env, js_value, &is_instanceof));
        if (is_instanceof) {
            return builtin_convert(helpers::TypeIdentity<JSConvertArrayBuffer>(), ctxx, env, js_value);
        }
        NAPI_CHECK_FATAL(napi_is_typedarray(env, js_value, &is_instanceof));
        if (is_instanceof) {
            not_implemented("TypedArray");
        }
        NAPI_CHECK_FATAL(napi_is_promise(env, js_value, &is_instanceof));
        if (is_instanceof) {
            return builtin_convert(helpers::TypeIdentity<JSConvertPromise>(), ctxx, env, js_value);
        }
        NAPI_CHECK_FATAL(napi_is_error(env, js_value, &is_instanceof));
        if (is_instanceof) {
            return m_error(ctxx, js_value);
        }
        NAPI_CHECK_FATAL(napi_is_date(env, js_value, &is_instanceof));
        if (is_instanceof) {
            not_implemented("Date");
        }
        NAPI_CHECK_FATAL(napi_is_dataview(env, js_value, &is_instanceof));
        if (is_instanceof) {
            not_implemented("DataView");
        }
        // NOTE(vpukhov): Boolean, Number...
        return builtin_convert(helpers::TypeIdentity<JSConvertJSValue>(), ctxx, env, js_value);
    };

    auto m_object = [=](InteropCtx *ctxx, napi_value js_value, bool verified = true) -> EtsObject * {
        napi_env env = ctxx->GetJSEnv();
        (void)verified;  // ignored for Object

        napi_valuetype js_type = GetValueType(env, js_value);
        switch (js_type) {
            case napi_boolean:
                return builtin_convert(helpers::TypeIdentity<JSConvertStdlibBoolean>(), ctxx, env, js_value);
            case napi_number:
                return builtin_convert(helpers::TypeIdentity<JSConvertStdlibDouble>(), ctxx, env, js_value);
            case napi_string:
                return builtin_convert(helpers::TypeIdentity<JSConvertString>(), ctxx, env, js_value);
            case napi_object:
                return m_object_object(ctx, js_value);
            case napi_symbol:
                [[fallthrough]];
            case napi_function:
                [[fallthrough]];
            case napi_external:
                [[fallthrough]];
            case napi_bigint:
                return builtin_convert(helpers::TypeIdentity<JSConvertJSValue>(), ctxx, env, js_value);
            default:
                ASSERT(!IsNullOrUndefined(env, js_value));
                InteropCtx::Fatal("Bad js_type in Object value matcher");
        };
    };
    w_object->SetJSBuiltinMatcher(m_object);
}

void RegisterBuiltinJSRefConvertors(InteropCtx *ctx)
{
    auto cache = ctx->GetRefConvertCache();
    auto coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    EtsClassLinkerExtension *linker_ext = vm->GetClassLinker()->GetEtsClassLinkerExtension();

    RegisterBuiltinRefConvertor<JSConvertJSValue>(cache, ctx->GetJSValueClass());
    RegisterBuiltinRefConvertor<JSConvertJSError>(cache, ctx->GetJSErrorClass());
    RegisterBuiltinRefConvertor<JSConvertString>(cache, ctx->GetStringClass());
    RegisterBuiltinRefConvertor<JSConvertPromise>(cache, ctx->GetPromiseClass());
    RegisterBuiltinRefConvertor<JSConvertArrayBuffer>(cache, ctx->GetArrayBufferClass());

    RegisterBuiltinRefConvertor<JSConvertStdlibBoolean>(cache, linker_ext->GetBoxBooleanClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibByte>(cache, linker_ext->GetBoxByteClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibChar>(cache, linker_ext->GetBoxCharClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibShort>(cache, linker_ext->GetBoxShortClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibInt>(cache, linker_ext->GetBoxIntClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibLong>(cache, linker_ext->GetBoxLongClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibFloat>(cache, linker_ext->GetBoxFloatClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibDouble>(cache, linker_ext->GetBoxDoubleClass());

    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_U1, JSConvertU1>(cache, linker_ext);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_I32, JSConvertI32>(cache, linker_ext);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_F32, JSConvertF32>(cache, linker_ext);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_I64, JSConvertI64>(cache, linker_ext);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_F64, JSConvertF64>(cache, linker_ext);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_STRING, JSConvertString>(cache, linker_ext);
    // NOTE(vpukhov): jsvalue[] specialization, currently uses JSRefConvertArrayRef

    RegisterCompatConvertors(ctx);
}

}  // namespace panda::ets::interop::js
