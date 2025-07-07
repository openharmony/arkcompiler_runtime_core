/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

// NOLINTBEGIN(readability-identifier-naming)
// CC-OFF(G.FMT.07) project code style
napi_status __attribute__((weak)) napi_is_map(napi_env env, napi_value value, bool *result);
// CC-OFF(G.FMT.07) project code style
napi_status __attribute__((weak)) napi_is_set(napi_env env, napi_value value, bool *result);
// NOLINTEND(readability-identifier-naming)

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
    ASSERT(coro != nullptr);
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
        INTEROP_FATAL_IF(IsNull(env, val));
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
            {utf::CStringAsMutf8("<ctor>"), std::make_pair("Lstd/core/String;Lescompat/ErrorOptions;:V", 2)}};
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_EXCEPTION_OVERLOADS {
            {utf::CStringAsMutf8("<ctor>"), std::make_pair("Lstd/core/String;:V", 1)}};
        wError_ = RegisterClass(descriptors::ERROR, "Error", &W_ERROR_OVERLOADS);
        RegisterClass(descriptors::EXCEPTION, nullptr, &W_EXCEPTION_OVERLOADS);

        static const std::array STD_EXCEPTIONS_LIST = {
            // Errors
            std::make_tuple("Lstd/core/AssertionError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/DivideByZeroError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/NullPointerError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/UncaughtExceptionError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ArithmeticError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ClassCastError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/IndexOutOfBoundsError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ArrayStoreError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/NegativeArraySizeError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/LinkerError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/LinkerClassNotFoundError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            // Exceptions
            std::make_tuple("Lstd/core/NoDataException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/ArgumentOutOfRangeException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/IllegalStateException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/UnsupportedOperationException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/IllegalMonitorStateError;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/IllegalArgumentException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
            std::make_tuple("Lstd/core/RuntimeException;", NO_MIRROR, &W_EXCEPTION_OVERLOADS),
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

    ets_proxy::EtsClassWrapper *RegisterClassWithLeafMatcher(
        std::string_view descriptor, char const *jsBuiltinName = nullptr,
        const ets_proxy::EtsClassWrapper::OverloadsMap *overloads = nullptr)
    {
        ets_proxy::EtsClassWrapper *wclass = RegisterClass(descriptor, jsBuiltinName, overloads);
        wclass->SetJSBuiltinMatcher(
            [self = *this, wclass, descriptor](InteropCtx *ctxx, napi_value jsValue, bool verified = true) mutable {
                (void)verified;
                auto env = ctxx->GetJSEnv();
                napi_value constructor;
                bool match = false;
                NAPI_CHECK_FATAL(napi_get_named_property(env, jsValue, "constructor", &constructor));
                NAPI_CHECK_FATAL(napi_strict_equals(env, constructor, wclass->GetBuiltin(env), &match));
                if (match) {
                    return wclass->CreateJSBuiltinProxy(ctxx, jsValue);
                }
                return self.NotAssignable(descriptor.data());
            });
        return wclass;
    }

    void RegisterArray()
    {
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_ARRAY_OVERLOADS = {
            {utf::CStringAsMutf8("<ctor>"), std::make_pair(":V", 1)},
            {utf::CStringAsMutf8("$_get"), std::make_pair("I:Lstd/core/Object;", 2)},
            {utf::CStringAsMutf8("$_set"), std::make_pair("ILstd/core/Object;:V", 3)},
            {utf::CStringAsMutf8("at"), std::make_pair("I:Lstd/core/Object;", 2)},
            {utf::CStringAsMutf8("copyWithin"), std::make_pair("III:Lescompat/Array;", 4)},
            {utf::CStringAsMutf8("fill"),
             std::make_pair("Lstd/core/Object;Lstd/core/Double;Lstd/core/Double;:Lescompat/Array;", 4)},
            {utf::CStringAsMutf8("indexOf"), std::make_pair("Lstd/core/Object;Lstd/core/Double;:D", 3)},
            {utf::CStringAsMutf8("lastIndexOf"), std::make_pair("Lstd/core/Object;I:I", 3)},
            {utf::CStringAsMutf8("slice"), std::make_pair("II:Lescompat/Array;", 3)},
            {utf::CStringAsMutf8("splice"), std::make_pair("I:Lescompat/Array;", 2)},
            {utf::CStringAsMutf8("splice"), std::make_pair("IILescompat/Array;:Lescompat/Array;", 3)},
            {utf::CStringAsMutf8("toSpliced"), std::make_pair("II[Lstd/core/Object;:Lescompat/Array;", 3)},
            {utf::CStringAsMutf8("with"), std::make_pair("DLstd/core/Object;:Lescompat/Array;", 3)},
        };
        wArray_ = RegisterClass(descriptors::ARRAY, "Array", &W_ARRAY_OVERLOADS);
        NAPI_CHECK_FATAL(napi_object_seal(ctx_->GetJSEnv(), jsGlobalEts_));
    }

    void RegisterError()
    {
        wRangeError_ = RegisterClassWithLeafMatcher(descriptors::RANGE_ERROR, "RangeError");
        wReferenceError_ = RegisterClassWithLeafMatcher(descriptors::REFERENCE_ERROR, "ReferenceError");
        wSyntaxError_ = RegisterClassWithLeafMatcher(descriptors::SYNTAX_ERROR, "SyntaxError");
        wURIError_ = RegisterClassWithLeafMatcher(descriptors::URI_ERROR, "URIError");
        wTypeError_ = RegisterClassWithLeafMatcher(descriptors::TYPE_ERROR, "TypeError");
        ASSERT(wRangeError_ != nullptr);
        ASSERT(wReferenceError_ != nullptr);
        ASSERT(wSyntaxError_ != nullptr);
        ASSERT(wURIError_ != nullptr);
        ASSERT(wTypeError_ != nullptr);

        ctorRangeError_ = StdCtorRef(ctx_, "RangeError");
        ctorReferenceError_ = StdCtorRef(ctx_, "ReferenceError");
        ctorSyntaxError_ = StdCtorRef(ctx_, "SyntaxError");
        ctorURIError_ = StdCtorRef(ctx_, "URIError");
        ctorTypeError_ = StdCtorRef(ctx_, "TypeError");
        ASSERT(ctorRangeError_ != nullptr);
        ASSERT(ctorReferenceError_ != nullptr);
        ASSERT(ctorSyntaxError_ != nullptr);
        ASSERT(ctorURIError_ != nullptr);
        ASSERT(ctorTypeError_ != nullptr);
    }

    void RegisterMap()
    {
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_MAP_OVERLOADS = {
            {utf::CStringAsMutf8("<ctor>"), std::make_pair("Lstd/core/Object;:V", 2)}};
        wMap_ = RegisterClassWithLeafMatcher(descriptors::MAP, "Map", &W_MAP_OVERLOADS);
    }

    void RegisterSet()
    {
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_SET_OVERLOADS = {
            {utf::CStringAsMutf8("<ctor>"), std::make_pair("Lstd/core/Object;:V", 2)}};
        wSet_ = RegisterClassWithLeafMatcher(descriptors::SET, "Set", &W_SET_OVERLOADS);
    }

    void RegisterDate()
    {
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_DATE_OVERLOADS {
            {utf::CStringAsMutf8("setDate"), std::make_pair("D:D", 2)},
            {utf::CStringAsMutf8("setUTCDate"), std::make_pair("D:D", 2)},
            {utf::CStringAsMutf8("setMilliseconds"), std::make_pair("D:D", 2)},
            {utf::CStringAsMutf8("setUTCMilliseconds"), std::make_pair("D:D", 2)},
            {utf::CStringAsMutf8("setTime"), std::make_pair("J:V", 2)},
        };
        wDate_ = RegisterClassWithLeafMatcher(descriptors::DATE, "Date", &W_DATE_OVERLOADS);
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

    EtsObject *MMap(InteropCtx *ctxx, napi_value jsValue, bool verified = true)
    {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_map(env, jsValue, &isInstanceof));
            if (!isInstanceof) {
                return NotAssignable("Map");
            }
        }
        return wMap_->CreateJSBuiltinProxy(ctxx, jsValue);
    }

    EtsObject *MSet(InteropCtx *ctxx, napi_value jsValue, bool verified = true)
    {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_set(env, jsValue, &isInstanceof));
            if (!isInstanceof) {
                return NotAssignable("Set");
            }
        }
        return wSet_->CreateJSBuiltinProxy(ctxx, jsValue);
    }

    EtsObject *MDate(InteropCtx *ctxx, napi_value jsValue, bool verified = true)
    {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        if (!verified) {
            NAPI_CHECK_FATAL(napi_is_date(env, jsValue, &isInstanceof));
            if (!isInstanceof) {
                return NotAssignable("Date");
            }
        }
        ASSERT(wDate_ != nullptr);
        return wDate_->CreateJSBuiltinProxy(ctxx, jsValue);
    }

    void RegisterObject()
    {
        wObject_ = RegisterClass(descriptors::OBJECT, "Object");
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

        if (CheckInstanceof(env, jsValue, ctorRangeError_)) {
            return wRangeError_->CreateJSBuiltinProxy(ctxx, jsValue);
        }
        if (CheckInstanceof(env, jsValue, ctorReferenceError_)) {
            return wReferenceError_->CreateJSBuiltinProxy(ctxx, jsValue);
        }
        if (CheckInstanceof(env, jsValue, ctorSyntaxError_)) {
            return wSyntaxError_->CreateJSBuiltinProxy(ctxx, jsValue);
        }
        if (CheckInstanceof(env, jsValue, ctorURIError_)) {
            return wURIError_->CreateJSBuiltinProxy(ctxx, jsValue);
        }
        if (CheckInstanceof(env, jsValue, ctorTypeError_)) {
            return wTypeError_->CreateJSBuiltinProxy(ctxx, jsValue);
        }

        // NOTE(vpukhov): compat: remove when compat/Error is implemented
        return BuiltinConvert<JSConvertESError>(ctxx, env, jsValue);
    }

    EtsObject *MObjectObject(InteropCtx *ctxx, napi_value jsValue)
    {
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        NAPI_CHECK_FATAL(napi_is_array(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return MArray(ctxx, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_map(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return MMap(ctxx, jsValue);
        }
        NAPI_CHECK_FATAL(napi_is_set(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return MSet(ctxx, jsValue);
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
            return MDate(ctxx, jsValue);
        }
        if (IsConstructor(env, jsValue, CONSTRUCTOR_NAME_NUMBER)) {
            return BuiltinConvert<JSConvertStdlibDouble>(ctxx, env, jsValue);
        }
        if (IsConstructor(env, jsValue, CONSTRUCTOR_NAME_BOOLEAN)) {
            return BuiltinConvert<JSConvertStdlibBoolean>(ctxx, env, jsValue);
        }
        if (IsConstructor(env, jsValue, CONSTRUCTOR_NAME_STRING)) {
            return BuiltinConvert<JSConvertString>(ctxx, env, jsValue);
        }
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
            case napi_null:
                return ctx_->GetNullValue();
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
        RegisterObject();

        RegisterExceptions();

        RegisterDate();
        // #IC4UO2
        RegisterClassWithLeafMatcher(descriptors::MAPENTRY, nullptr);
        RegisterClassWithLeafMatcher(descriptors::MAPITERATOR, nullptr);
        RegisterClassWithLeafMatcher(descriptors::EMPTYMAPITERATOR, nullptr);

        RegisterMap();
        RegisterSet();

        RegisterClassWithLeafMatcher(descriptors::ARRAY_ENTRIES_ITERATOR_T, nullptr);
        RegisterClassWithLeafMatcher(descriptors::ITERATOR_RESULT, nullptr);
        RegisterClassWithLeafMatcher(descriptors::ARRAY_KEYS_ITERATOR, nullptr);
        RegisterClassWithLeafMatcher(descriptors::ARRAY_VALUES_ITERATOR_T, nullptr);

        RegisterError();
        RegisterArray();

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
    ets_proxy::EtsClassWrapper *wDate_ {};
    ets_proxy::EtsClassWrapper *wMap_ {};
    ets_proxy::EtsClassWrapper *wSet_ {};
    ets_proxy::EtsClassWrapper *wRangeError_ {};
    ets_proxy::EtsClassWrapper *wReferenceError_ {};
    ets_proxy::EtsClassWrapper *wSyntaxError_ {};
    ets_proxy::EtsClassWrapper *wURIError_ {};
    ets_proxy::EtsClassWrapper *wTypeError_ {};

    napi_ref ctorTypeError_ {nullptr};
    napi_ref ctorRangeError_ {nullptr};
    napi_ref ctorReferenceError_ {nullptr};
    napi_ref ctorSyntaxError_ = {nullptr};
    napi_ref ctorURIError_ = {nullptr};
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
    ASSERT(coro != nullptr);
    PandaEtsVM *vm = coro->GetPandaVM();
    EtsClassLinkerExtension *linkerExt = vm->GetClassLinker()->GetEtsClassLinkerExtension();
    auto ptypes = PlatformTypes(coro);

    RegisterBuiltinRefConvertor<JSConvertJSValue>(cache, ctx->GetJSValueClass());
    RegisterBuiltinRefConvertor<JSConvertESError>(cache, ctx->GetESErrorClass());
    RegisterBuiltinRefConvertor<JSConvertString>(cache, ctx->GetStringClass());
    RegisterBuiltinRefConvertor<JSConvertBigInt>(cache, ctx->GetBigIntClass());
    RegisterBuiltinRefConvertor<JSConvertPromise>(cache, ctx->GetPromiseClass());
    RegisterBuiltinRefConvertor<JSConvertEtsNull>(cache, ctx->GetNullValueClass());

    RegisterBuiltinRefConvertor<JSConvertStdlibBoolean>(cache, ptypes->coreBoolean->GetRuntimeClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibByte>(cache, ptypes->coreByte->GetRuntimeClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibChar>(cache, ptypes->coreChar->GetRuntimeClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibShort>(cache, ptypes->coreShort->GetRuntimeClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibInt>(cache, ptypes->coreInt->GetRuntimeClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibLong>(cache, ptypes->coreLong->GetRuntimeClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibFloat>(cache, ptypes->coreFloat->GetRuntimeClass());
    RegisterBuiltinRefConvertor<JSConvertStdlibDouble>(cache, ptypes->coreDouble->GetRuntimeClass());

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
