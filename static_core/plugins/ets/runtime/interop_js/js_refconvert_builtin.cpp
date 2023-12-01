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

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value jsValue)
    {
        auto res = Conv::Unwrap(ctx, ctx->GetJSEnv(), jsValue);
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
    InteropCtx *ctx, std::string_view descriptor, char const *jsBuiltinName = nullptr,
    const ets_proxy::EtsClassWrapper::OverloadsMap *overloads = nullptr)
{
    auto coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    EtsClassLinker *etsClassLinker = vm->GetClassLinker();
    auto etsClass = etsClassLinker->GetClass(descriptor.data());
    if (UNLIKELY(etsClass == nullptr)) {
        ctx->Fatal(std::string("nonexisting class ") + descriptor.data());
    }

    // create ets_proxy bound to js builtin-constructor
    ets_proxy::EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();
    std::unique_ptr<ets_proxy::EtsClassWrapper> wrapper =
        ets_proxy::EtsClassWrapper::Create(ctx, etsClass, jsBuiltinName, overloads);
    if (UNLIKELY(wrapper == nullptr)) {
        ctx->Fatal(std::string("ets_proxy creation failed for ") + etsClass->GetDescriptor());
    }
    return cache->Insert(etsClass, std::move(wrapper));
}

// NOLINTNEXTLINE(readability-function-size)
static void RegisterCompatConvertors(InteropCtx *ctx)
{
    /******************************************************************************/
    // Helpers

    namespace descriptors = panda_file_items::class_descriptors;

    auto notImplemented = [](char const *name) __attribute__((noreturn, noinline))
    {
        InteropCtx::Fatal(std::string("compat.") + name + " box is not implemented");
    };
    auto notAssignable = [](char const *name) __attribute__((noinline))->EtsObject *
    {
        JSConvertTypeCheckFailed(name);
        return nullptr;
    };

    auto stdCtorRef = [](InteropCtx *ctxx, char const *name) {
        napi_env env = ctxx->GetJSEnv();
        napi_value val;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), name, &val));
        INTEROP_FATAL_IF(IsUndefined(env, val));
        napi_ref ref;
        NAPI_CHECK_FATAL(napi_create_reference(env, val, 1, &ref));
        return ref;
    };
    auto checkInstanceof = [](napi_env env, napi_value val, napi_ref ctorRef) {
        bool result;
        NAPI_CHECK_FATAL(napi_instanceof(env, val, GetReferenceValue(env, ctorRef), &result));
        return result;
    };

    auto builtinConvert = [](auto convTag, InteropCtx *ctxx, napi_env env, napi_value jsValue) -> EtsObject * {
        auto res = decltype(convTag)::type::UnwrapImpl(ctxx, env, jsValue);
        if (UNLIKELY(!res.has_value())) {
            return nullptr;
        }
        return AsEtsObject(res.value());
    };

    napi_value jsGlobalEts;
    {
        auto env = ctx->GetJSEnv();
        NAPI_CHECK_FATAL(napi_create_object(env, &jsGlobalEts));
        NAPI_CHECK_FATAL(napi_set_named_property(env, GetGlobal(env), "ets", jsGlobalEts));
    }

    auto registerClass = [ctx, jsGlobalEts](std::string_view descriptor, char const *jsBuiltinName = nullptr,
                                            const ets_proxy::EtsClassWrapper::OverloadsMap *overloads = nullptr) {
        ets_proxy::EtsClassWrapper *wclass = RegisterEtsProxyForStdClass(ctx, descriptor, jsBuiltinName, overloads);
        auto env = ctx->GetJSEnv();
        auto name = wclass->GetEtsClass()->GetRuntimeClass()->GetName();
        auto jsCtor = wclass->GetJsCtor(env);
        NAPI_CHECK_FATAL(napi_set_named_property(env, jsGlobalEts, name.c_str(), jsCtor));
        if (jsBuiltinName != nullptr) {
            NAPI_CHECK_FATAL(napi_set_named_property(env, jsGlobalEts, jsBuiltinName, jsCtor));
        }
        return wclass;
    };

    constexpr const ets_proxy::EtsClassWrapper::OverloadsMap *NO_OVERLOADS = nullptr;
    constexpr const char *NO_MIRROR = nullptr;

    /******************************************************************************/
    // Definitions for StdClasses

    auto wObject = registerClass(descriptors::OBJECT, "Object");

    static const ets_proxy::EtsClassWrapper::OverloadsMap W_ERROR_OVERLOADS {
        {utf::CStringAsMutf8("<ctor>"), "Lstd/core/String;:V"}};
    auto wError = registerClass(descriptors::ERROR, "Error", &W_ERROR_OVERLOADS);
    registerClass(descriptors::EXCEPTION, nullptr, &W_ERROR_OVERLOADS);

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
        registerClass(descr, mirror, ovl);
    }

    static const ets_proxy::EtsClassWrapper::OverloadsMap W_ARRAY_OVERLOADS = {
        {utf::CStringAsMutf8("at"), "I:Lstd/core/Object;"},
        {utf::CStringAsMutf8("$_get"), "D:Lstd/core/Object;"},
        {utf::CStringAsMutf8("$_set"), "DLstd/core/Object;:Lstd/core/void;"},
        {utf::CStringAsMutf8("with"), "DLstd/core/Object;:Lescompat/Array;"},
        {utf::CStringAsMutf8("map"), "LFunctionalInterface-std-core-Object-std-core-Object-0;:Lescompat/Array;"},
        {utf::CStringAsMutf8("forEach"), "LFunctionalInterface-std-core-Object-std-core-void-0;:Lstd/core/void;"},
        {utf::CStringAsMutf8("pop"), ":Lstd/core/Object;"},
        {utf::CStringAsMutf8("fill"), "Lstd/core/Object;Lstd/core/Object;Lstd/core/Object;:Lescompat/Array;"},
        {utf::CStringAsMutf8("flat"), ":Lescompat/Array;"},
        {utf::CStringAsMutf8("join"), "Lstd/core/Object;:Lstd/core/String;"},
        // NOTE(kprokopenko) make [Lstd/core/Object;:D when #14756 is fixed
        {utf::CStringAsMutf8("push"), "Lstd/core/Object;:D"},
        {utf::CStringAsMutf8("some"), "LFunctionalInterface-std-core-Object-u1-0;:Z"},
        {utf::CStringAsMutf8("sort"), ":Lescompat/Array;"},
        {utf::CStringAsMutf8("every"), "LFunctionalInterface-std-core-Object-u1-0;:Z"},
        {utf::CStringAsMutf8("shift"), ":Lstd/core/Object;"},
        {utf::CStringAsMutf8("slice"), "Lstd/core/Object;Lstd/core/Object;:Lescompat/Array;"},
        {utf::CStringAsMutf8("<ctor>"), ":V"},
        {utf::CStringAsMutf8("filter"), "LFunctionalInterface-std-core-Object-u1-0;:Lescompat/Array;"},
        {utf::CStringAsMutf8("<get>length"), ":D"},
        {utf::CStringAsMutf8("reduce"),
         "LFunctionalInterface-std-core-Object-std-core-Object-std-core-Object-0;:Lstd/core/Object;"},
        {utf::CStringAsMutf8("splice"), "DLstd/core/Object;[Lstd/core/Object;:Lescompat/Array;"},
        {utf::CStringAsMutf8("findLast"), "LFunctionalInterface-std-core-Object-u1-0;:Lstd/core/Object;"},
        {utf::CStringAsMutf8("toSorted"), ":Lescompat/Array;"},
        {utf::CStringAsMutf8("findIndex"), "LFunctionalInterface-std-core-Object-u1-0;:D"},
        {utf::CStringAsMutf8("toSpliced"), "II:Lescompat/Array;"},
        {utf::CStringAsMutf8("copyWithin"), "II:Lescompat/Array;"},
        {utf::CStringAsMutf8("toReversed"), ":Lescompat/Array;"},
        {utf::CStringAsMutf8("indexOf"), "Lstd/core/Object;Lstd/core/Object;:D"},
        {utf::CStringAsMutf8("includes"), "Lstd/core/Object;Lstd/core/Object;:Z"},
        {utf::CStringAsMutf8("lastIndexOf"), "Lstd/core/Object;Lstd/core/Object;:D"},
        {utf::CStringAsMutf8("reduceRight"),
         "LFunctionalInterface-std-core-Object-std-core-Object-std-core-Object-0;:Lstd/core/Object;"},
        {utf::CStringAsMutf8("find"), "LFunctionalInterface-std-core-Object-u1-0;:Lstd/core/Object;"},
        {utf::CStringAsMutf8("isArray"), "Lstd/core/Object;:Z"},
        {utf::CStringAsMutf8("flatMap"),
         "LFunctionalInterface-std-core-Object-f64-std-core-Object-0;:Lescompat/Array;"},
        {utf::CStringAsMutf8("toLocaleString"), ":Lstd/core/String;"},
    };
    auto wArray = registerClass(descriptors::ARRAY, "Array", &W_ARRAY_OVERLOADS);

    NAPI_CHECK_FATAL(napi_object_seal(ctx->GetJSEnv(), jsGlobalEts));
    /******************************************************************************/
    // JS built-in matchers for StdClasses

    auto mArray = [=](InteropCtx *ctxx, napi_value jsValue, bool verified = true) -> EtsObject * {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_array(env, jsValue, &isInstanceof));
            if (!isInstanceof) {
                return notAssignable("Array");
            }
        }
        return wArray->CreateJSBuiltinProxy(ctxx, jsValue);
    };
    wArray->SetJSBuiltinMatcher(mArray);

    // NOTE(vpukhov): compat: obtain from class wrappers when implemented
    napi_ref ctorTypeerror = stdCtorRef(ctx, "TypeError");
    napi_ref ctorRangeerror = stdCtorRef(ctx, "RangeError");
    napi_ref ctorReferenceerror = stdCtorRef(ctx, "ReferenceError");

    auto mError = [=](InteropCtx *ctxx, napi_value jsValue, bool verified = true) -> EtsObject * {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_error(env, jsValue, &isInstanceof));
            if (!isInstanceof) {
                return notAssignable("Error");
            }
        }
        // NOTE(vpukhov): compat: remove when compat/Error is implemented
        return builtinConvert(helpers::TypeIdentity<JSConvertJSError>(), ctxx, env, jsValue);

        if (checkInstanceof(env, jsValue, ctorTypeerror)) {
            notImplemented("TypeError");
        }
        if (checkInstanceof(env, jsValue, ctorRangeerror)) {
            notImplemented("RangeError");
        }
        if (checkInstanceof(env, jsValue, ctorReferenceerror)) {
            notImplemented("ReferenceError");
        }
        notImplemented("Error");
    };
    wError->SetJSBuiltinMatcher(mError);

    auto mObjectObject = [=](InteropCtx *ctxx, napi_value jsValue) -> EtsObject * {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        NAPI_CHECK_FATAL(napi_is_array(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return mArray(ctxx, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_arraybuffer(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return builtinConvert(helpers::TypeIdentity<JSConvertArrayBuffer>(), ctxx, env, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_typedarray(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            notImplemented("TypedArray");
        }
        NAPI_CHECK_FATAL(napi_is_promise(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return builtinConvert(helpers::TypeIdentity<JSConvertPromise>(), ctxx, env, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_error(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return mError(ctxx, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_date(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            notImplemented("Date");
        }
        NAPI_CHECK_FATAL(napi_is_dataview(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            notImplemented("DataView");
        }
        // NOTE(vpukhov): Boolean, Number...
        return builtinConvert(helpers::TypeIdentity<JSConvertJSValue>(), ctxx, env, jsValue);
    };

    auto mObject = [=](InteropCtx *ctxx, napi_value jsValue, bool verified = true) -> EtsObject * {
        napi_env env = ctxx->GetJSEnv();
        (void)verified;  // ignored for Object

        napi_valuetype jsType = GetValueType(env, jsValue);
        switch (jsType) {
            case napi_boolean:
                return builtinConvert(helpers::TypeIdentity<JSConvertStdlibBoolean>(), ctxx, env, jsValue);
            case napi_number:
                return builtinConvert(helpers::TypeIdentity<JSConvertStdlibDouble>(), ctxx, env, jsValue);
            case napi_string:
                return builtinConvert(helpers::TypeIdentity<JSConvertString>(), ctxx, env, jsValue);
            case napi_object:
                return m_object_object(ctx, js_value);
            case napi_undefined:
                return ctx->GetUndefinedObject();
            case napi_symbol:
                [[fallthrough]];
            case napi_function:
                [[fallthrough]];
            case napi_external:
                [[fallthrough]];
            case napi_bigint:
                return builtinConvert(helpers::TypeIdentity<JSConvertJSValue>(), ctxx, env, jsValue);
            default:
                ASSERT(!IsNullOrUndefined(env, jsValue));
                InteropCtx::Fatal("Bad jsType in Object value matcher");
        };
    };
    wObject->SetJSBuiltinMatcher(mObject);
}

void RegisterBuiltinJSRefConvertors(InteropCtx *ctx)
{
    auto cache = ctx->GetRefConvertCache();
    auto coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    EtsClassLinkerExtension *linkerExt = vm->GetClassLinker()->GetEtsClassLinkerExtension();

    RegisterBuiltinRefConvertor<JSConvertJSValue>(cache, ctx->GetJSValueClass());
    RegisterBuiltinRefConvertor<JSConvertJSError>(cache, ctx->GetJSErrorClass());
    RegisterBuiltinRefConvertor<JSConvertString>(cache, ctx->GetStringClass());
    RegisterBuiltinRefConvertor<JSConvertPromise>(cache, ctx->GetPromiseClass());
    RegisterBuiltinRefConvertor<JSConvertArrayBuffer>(cache, ctx->GetArrayBufferClass());
    RegisterBuiltinRefConvertor<JSConvertEtsVoid>(cache, ctx->GetVoidClass());
    RegisterBuiltinRefConvertor<JSConvertEtsUndefined>(cache, ctx->GetUndefinedClass());

    RegisterBuiltinRefConvertor<JSConvertStdlibBoolean>(cache, linkerExt->GetBoxBooleanClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibByte>(cache, linkerExt->GetBoxByteClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibChar>(cache, linkerExt->GetBoxCharClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibShort>(cache, linkerExt->GetBoxShortClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibInt>(cache, linkerExt->GetBoxIntClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibLong>(cache, linkerExt->GetBoxLongClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibFloat>(cache, linkerExt->GetBoxFloatClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibDouble>(cache, linkerExt->GetBoxDoubleClass());

    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_U1, JSConvertU1>(cache, linkerExt);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_I32, JSConvertI32>(cache, linkerExt);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_F32, JSConvertF32>(cache, linkerExt);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_I64, JSConvertI64>(cache, linkerExt);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_F64, JSConvertF64>(cache, linkerExt);
    RegisterBuiltinArrayConvertor<ClassRoot::ARRAY_STRING, JSConvertString>(cache, linkerExt);
    // NOTE(vpukhov): jsvalue[] specialization, currently uses JSRefConvertArrayRef

    RegisterCompatConvertors(ctx);
}

}  // namespace panda::ets::interop::js
