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

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "runtime/mem/local_object_handle.h"

#include "plugins/ets/runtime/interop_js/js_refconvert_array.h"

namespace ark::ets::interop::js {

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
    cache->Insert(klass, std::make_unique<JSRefConvertBuiltin<Conv>>());
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

namespace {

namespace descriptors = panda_file_items::class_descriptors;

[[maybe_unused]] constexpr const ets_proxy::EtsClassWrapper::OverloadsMap *NO_OVERLOADS = nullptr;
constexpr const char *NO_MIRROR = nullptr;

class CompatConvertorsRegisterer {
private:
    [[noreturn]] void NotImplemented(char const *name) __attribute__((noinline))
    {
        InteropCtx::Fatal(std::string("compat.") + name + " box is not implemented");
    }

    EtsObject *NotAssignable(char const *name) __attribute__((noinline))
    {
        JSConvertTypeCheckFailed(name);
        return nullptr;
    };

    napi_ref StdCtorRef(InteropCtx *ctxx, char const *name)
    {
        napi_env env = ctxx->GetJSEnv();
        napi_value val;
        NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), name, &val));
        INTEROP_FATAL_IF(IsUndefined(env, val));
        napi_ref ref;
        NAPI_CHECK_FATAL(napi_create_reference(env, val, 1, &ref));
        return ref;
    }

    bool CheckInstanceof(napi_env env, napi_value val, napi_ref ctorRef)
    {
        bool result;
        NAPI_CHECK_FATAL(napi_instanceof(env, val, GetReferenceValue(env, ctorRef), &result));
        return result;
    }

    template <typename ConvTag>
    EtsObject *BuiltinConvert(InteropCtx *inCtx, napi_env env, napi_value jsValue)
    {
        auto res = ConvTag::UnwrapImpl(inCtx, env, jsValue);
        if (UNLIKELY(!res.has_value())) {
            return nullptr;
        }
        return AsEtsObject(res.value());
    }

    void RegisterExceptions()
    {
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_ERROR_OVERLOADS {
            {utf::CStringAsMutf8("<ctor>"), "Lstd/core/Object;Lstd/core/Object;:V"}};
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_EXCEPTION_OVERLOADS {
            {utf::CStringAsMutf8("<ctor>"), "Lstd/core/String;:V"}};
        wError_ = RegisterClass(descriptors::ERROR, "Error", &W_ERROR_OVERLOADS);
        RegisterClass(descriptors::EXCEPTION, nullptr, &W_EXCEPTION_OVERLOADS);

        static const std::array STD_EXCEPTIONS_LIST = {
            // Errors
            std::make_tuple("Lstd/core/AssertionError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/DivideByZeroError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/NullPointerError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/UncatchedExceptionError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/RangeError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ArithmeticError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ClassCastError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/IndexOutOfBoundsError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ArrayStoreError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/NegativeArraySizeError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            // Exceptions
            std::make_tuple("Lstd/core/NullPointerException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/NoDataException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/ArgumentOutOfRangeException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/IllegalStateException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/ClassNotFoundException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/UnsupportedOperationException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/IllegalMonitorStateException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/IllegalArgumentException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
        };
        for (const auto &[descr, mirror, ovl] : STD_EXCEPTIONS_LIST) {
            RegisterClass(descr, mirror, ovl);
        }
    }

    ets_proxy::EtsClassWrapper *RegisterClass(std::string_view descriptor, char const *jsBuiltinName = nullptr,
                                              const ets_proxy::EtsClassWrapper::OverloadsMap *overloads = nullptr)
    {
        ets_proxy::EtsClassWrapper *wclass = RegisterEtsProxyForStdClass(ctx_, descriptor, jsBuiltinName, overloads);
        auto env = ctx_->GetJSEnv();
        auto name = wclass->GetEtsClass()->GetRuntimeClass()->GetName();
        auto jsCtor = wclass->GetJsCtor(env);
        NAPI_CHECK_FATAL(napi_set_named_property(env, jsGlobalEts_, name.c_str(), jsCtor));
        if (jsBuiltinName != nullptr) {
            NAPI_CHECK_FATAL(napi_set_named_property(env, jsGlobalEts_, jsBuiltinName, jsCtor));
        }
        return wclass;
    }

    void RegisterArray()
    {
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_ARRAY_OVERLOADS = {
            {utf::CStringAsMutf8("at"), "I:Lstd/core/Object;"},
            {utf::CStringAsMutf8("$_get"), "D:Lstd/core/Object;"},
            {utf::CStringAsMutf8("$_set"), "DLstd/core/Object;:V"},
            {utf::CStringAsMutf8("with"), "DLstd/core/Object;:Lescompat/Array;"},
            {utf::CStringAsMutf8("map"), "Lstd/core/Function1;:Lescompat/Array;"},
            {utf::CStringAsMutf8("forEach"), "Lstd/core/Function1;:V"},
            {utf::CStringAsMutf8("pop"), ":Lstd/core/Object;"},
            {utf::CStringAsMutf8("fill"), "Lstd/core/Object;Lstd/core/Object;Lstd/core/Object;:Lescompat/Array;"},
            {utf::CStringAsMutf8("flat"), ":Lescompat/Array;"},
            {utf::CStringAsMutf8("join"), "Lstd/core/Object;:Lstd/core/String;"},
            {utf::CStringAsMutf8("push"), "[Lstd/core/Object;:D"},
            {utf::CStringAsMutf8("some"), "Lstd/core/Function1;:Z"},
            {utf::CStringAsMutf8("sort"), ":Lescompat/Array;"},
            {utf::CStringAsMutf8("every"), "Lstd/core/Function1;:Z"},
            {utf::CStringAsMutf8("shift"), ":Lstd/core/Object;"},
            {utf::CStringAsMutf8("slice"), "Lstd/core/Object;Lstd/core/Object;:Lescompat/Array;"},
            {utf::CStringAsMutf8("<ctor>"), ":V"},
            {utf::CStringAsMutf8("filter"), "Lstd/core/Function1;:Lescompat/Array;"},
            {utf::CStringAsMutf8("<get>length"), ":D"},
            {utf::CStringAsMutf8("reduce"), "Lstd/core/Function2;:Lstd/core/Object;"},
            {utf::CStringAsMutf8("splice"), "DLstd/core/Object;[Lstd/core/Object;:Lescompat/Array;"},
            {utf::CStringAsMutf8("findLast"), "Lstd/core/Function1;:Lstd/core/Object;"},
            {utf::CStringAsMutf8("toSorted"), ":Lescompat/Array;"},
            {utf::CStringAsMutf8("findIndex"), "Lstd/core/Function1;:D"},
            {utf::CStringAsMutf8("findLastIndex"), "Lstd/core/Function1;:D"},
            {utf::CStringAsMutf8("toSpliced"), "Lstd/core/Object;Lstd/core/Object;:Lescompat/Array;"},
            {utf::CStringAsMutf8("copyWithin"), "II:Lescompat/Array;"},
            {utf::CStringAsMutf8("toReversed"), ":Lescompat/Array;"},
            {utf::CStringAsMutf8("indexOf"), "Lstd/core/Object;Lstd/core/Object;:D"},
            {utf::CStringAsMutf8("includes"), "Lstd/core/Object;Lstd/core/Object;:Z"},
            {utf::CStringAsMutf8("lastIndexOf"), "Lstd/core/Object;Lstd/core/Object;:D"},
            {utf::CStringAsMutf8("reduceRight"), "Lstd/core/Function2;:Lstd/core/Object;"},
            {utf::CStringAsMutf8("find"), "Lstd/core/Function1;:Lstd/core/Object;"},
            {utf::CStringAsMutf8("isArray"), "Lstd/core/Object;:Z"},
            {utf::CStringAsMutf8("flatMap"), "Lstd/core/Function2;:Lescompat/Array;"},
            {utf::CStringAsMutf8("toLocaleString"), ":Lstd/core/String;"},
            {utf::CStringAsMutf8("from"), "Lescompat/Iterable;:Lescompat/Array;"},
        };
        wArray_ = RegisterClass(descriptors::ARRAY, "Array", &W_ARRAY_OVERLOADS);

        NAPI_CHECK_FATAL(napi_object_seal(ctx_->GetJSEnv(), jsGlobalEts_));
    }

    EtsObject *MArray(InteropCtx *ctxx, napi_value jsValue, bool verified = true)
    {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_array(env, jsValue, &isInstanceof));
            if (!isInstanceof) {
                return NotAssignable("Array");
            }
        }
        return wArray_->CreateJSBuiltinProxy(ctxx, jsValue);
    }

    EtsObject *MError(InteropCtx *ctxx, napi_value jsValue, bool verified = true)
    {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_error(env, jsValue, &isInstanceof));
            if (!isInstanceof) {
                return NotAssignable("Error");
            }
        }
        // NOTE(vpukhov): compat: remove when compat/Error is implemented
        return BuiltinConvert<JSConvertJSError>(ctxx, env, jsValue);

        if (CheckInstanceof(env, jsValue, ctorTypeError_)) {
            NotImplemented("TypeError");
        }
        if (CheckInstanceof(env, jsValue, ctorRangeError_)) {
            NotImplemented("RangeError");
        }
        if (CheckInstanceof(env, jsValue, ctorReferenceError_)) {
            NotImplemented("ReferenceError");
        }
        NotImplemented("Error");
    }

    EtsObject *MObjectObject(InteropCtx *ctxx, napi_value jsValue)
    {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        NAPI_CHECK_FATAL(napi_is_array(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return MArray(ctxx, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_arraybuffer(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return BuiltinConvert<JSConvertArrayBuffer>(ctxx, env, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_typedarray(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            NotImplemented("TypedArray");
        }
        NAPI_CHECK_FATAL(napi_is_promise(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return BuiltinConvert<JSConvertPromise>(ctxx, env, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_error(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return MError(ctxx, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_date(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            NotImplemented("Date");
        }
        NAPI_CHECK_FATAL(napi_is_dataview(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            NotImplemented("DataView");
        }
        // NOTE(vpukhov): Boolean, Number...
        return BuiltinConvert<JSConvertJSValue>(ctxx, env, jsValue);
    }

    EtsObject *MObject(InteropCtx *ctxx, napi_value jsValue, bool verified = true)
    {
        napi_env env = ctxx->GetJSEnv();
        (void)verified;  // ignored for Object

        napi_valuetype jsType = GetValueType(env, jsValue);
        switch (jsType) {
            case napi_boolean:
                return BuiltinConvert<JSConvertStdlibBoolean>(ctxx, env, jsValue);
            case napi_number:
                return BuiltinConvert<JSConvertStdlibDouble>(ctxx, env, jsValue);
            case napi_string:
                return BuiltinConvert<JSConvertString>(ctxx, env, jsValue);
            case napi_object:
                return MObjectObject(ctx_, jsValue);
            case napi_undefined:
                return ctx_->GetUndefinedObject();
            case napi_symbol:
                [[fallthrough]];
            case napi_function:
                [[fallthrough]];
            case napi_external:
                [[fallthrough]];
            case napi_bigint:
                return BuiltinConvert<JSConvertJSValue>(ctxx, env, jsValue);
            default:
                ASSERT(!IsNullOrUndefined(env, jsValue));
                InteropCtx::Fatal("Bad jsType in Object value matcher");
        };
    }

public:
    explicit CompatConvertorsRegisterer(InteropCtx *ctx) : ctx_(ctx)
    {
        auto env = ctx_->GetJSEnv();
        NAPI_CHECK_FATAL(napi_create_object(env, &jsGlobalEts_));
        NAPI_CHECK_FATAL(napi_set_named_property(env, GetGlobal(env), "ets", jsGlobalEts_));
    }

    DEFAULT_MOVE_SEMANTIC(CompatConvertorsRegisterer);

    DEFAULT_COPY_SEMANTIC(CompatConvertorsRegisterer);

    ~CompatConvertorsRegisterer() = default;

    void Run()
    {
        wObject_ = RegisterClass(descriptors::OBJECT, "Object");

        RegisterExceptions();

        RegisterArray();

        // NOTE(vpukhov): compat: obtain from class wrappers when implemented
        ctorTypeError_ = StdCtorRef(ctx_, "TypeError");
        ctorRangeError_ = StdCtorRef(ctx_, "RangeError");
        ctorReferenceError_ = StdCtorRef(ctx_, "ReferenceError");

        ASSERT(wError_ != nullptr);
        wError_->SetJSBuiltinMatcher(
            [self = *this](InteropCtx *ctxx, napi_value jsValue, bool verified = true) mutable {
                return self.MError(ctxx, jsValue, verified);
            });

        ASSERT(wObject_ != nullptr);
        wObject_->SetJSBuiltinMatcher(
            [self = *this](InteropCtx *ctxx, napi_value jsValue, bool verified = true) mutable {
                return self.MObject(ctxx, jsValue, verified);
            });

        ASSERT(wArray_ != nullptr);
        wArray_->SetJSBuiltinMatcher(
            [self = *this](InteropCtx *ctxx, napi_value jsValue, bool verified = true) mutable {
                return self.MArray(ctxx, jsValue, verified);
            });
    }

private:
    InteropCtx *ctx_;
    napi_value jsGlobalEts_ {};
    ets_proxy::EtsClassWrapper *wError_ {};
    ets_proxy::EtsClassWrapper *wObject_ {};
    ets_proxy::EtsClassWrapper *wArray_ {};

    napi_ref ctorTypeError_ {nullptr};
    napi_ref ctorRangeError_ {nullptr};
    napi_ref ctorReferenceError_ {nullptr};
};

static_assert(std::is_trivially_copy_constructible_v<CompatConvertorsRegisterer>);
static_assert(std::is_trivially_copy_assignable_v<CompatConvertorsRegisterer>);
static_assert(std::is_trivially_move_constructible_v<CompatConvertorsRegisterer>);
static_assert(std::is_trivially_move_assignable_v<CompatConvertorsRegisterer>);
static_assert(std::is_trivially_destructible_v<CompatConvertorsRegisterer>);

}  // namespace

static void RegisterCompatConvertors(InteropCtx *ctx)
{
    CompatConvertorsRegisterer(ctx).Run();
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
    RegisterBuiltinRefConvertor<JSConvertBigInt>(cache, ctx->GetBigIntClass());
    RegisterBuiltinRefConvertor<JSConvertPromise>(cache, ctx->GetPromiseClass());
    RegisterBuiltinRefConvertor<JSConvertArrayBuffer>(cache, ctx->GetArrayBufferClass());
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

}  // namespace ark::ets::interop::js
