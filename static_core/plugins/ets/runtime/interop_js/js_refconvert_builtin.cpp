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

#include <string>
#include "interop_js/interop_error.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_array.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_function.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/mem/local_object_handle.h"

// NOLINTBEGIN(readability-identifier-naming)
// CC-OFF(G.FMT.07) project code style
napi_status __attribute__((weak)) napi_is_map(napi_env env, napi_value value, bool *result);
// CC-OFF(G.FMT.07) project code style
napi_status __attribute__((weak)) napi_is_set(napi_env env, napi_value value, bool *result);
// NOLINTEND(readability-identifier-naming)

namespace ark::ets::interop::js {

enum class MObjectObjectType { ARRAY, MAP, SET, PROMISE, ERROR, DATE, NUMBER, BOOLEAN, STRING, JSVALUE };

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
static inline void RegisterBuiltinRefConvertor(JSRefConvertCache *cache, EtsClass *klass)
{
    [[maybe_unused]] bool res = CheckClassInitialized<true>(klass->GetRuntimeClass());
    ASSERT(res);
    cache->Insert(klass->GetRuntimeClass(), std::make_unique<JSRefConvertBuiltin<Conv>>());
}

static ets_proxy::EtsClassWrapper *RegisterEtsProxyForStdClass(
    InteropCtx *ctx, std::string_view descriptor, char const *jsBuiltinName = nullptr,
    const ets_proxy::EtsClassWrapper::OverloadsMap *overloads = nullptr)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    PandaEtsVM *vm = executionCtx->GetPandaVM();
    EtsClassLinker *etsClassLinker = vm->GetClassLinker();
    auto etsClass = etsClassLinker->GetClass(descriptor.data());
    if (UNLIKELY(etsClass == nullptr)) {
        ctx->Fatal(INTEROP_CLASS_NOT_FOUND, std::string("nonexisting class ") + descriptor.data());
    }

    // create ets_proxy bound to js builtin-constructor
    ets_proxy::EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();
    std::unique_ptr<ets_proxy::EtsClassWrapper> wrapper =
        ets_proxy::EtsClassWrapper::Create(ctx, etsClass, jsBuiltinName, overloads);
    if (UNLIKELY(wrapper == nullptr)) {
        ctx->Fatal(INTEROP_PROXY_CREATION_FAILED,
                   std::string("ets_proxy creation failed for ") + etsClass->GetDescriptor());
    }
    return cache->Insert(etsClass, std::move(wrapper));
}

namespace {

[[maybe_unused]] constexpr const ets_proxy::EtsClassWrapper::OverloadsMap *NO_OVERLOADS = nullptr;
constexpr const char *NO_MIRROR = nullptr;

class CompatConvertorsRegisterer {
private:
    [[noreturn]] void NotImplemented(char const *name) __attribute__((noinline))
    {
        InteropCtx::Fatal(INTEROP_FEATURE_NOT_IMPLEMENTED, std::string("compat.") + name + " box is not implemented");
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
        INTEROP_FATAL_IF(GetValueTypeNoScope(env, val) == napi_null);
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
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_ERROR_OVERLOADS = {
            {utf::CStringAsMutf8("<ctor>"), {"Lstd/core/String;Lstd/core/ErrorOptions;:V", 2, "<ctor>"}}};
        wError_ = RegisterClass(PlatformTypes()->coreError->GetDescriptor(), "Error", &W_ERROR_OVERLOADS);

        static const std::array STD_EXCEPTIONS_LIST = {
            // Errors
            std::make_tuple("Lstd/core/RuntimeError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/AssertionError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/DivideByZeroError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/IllegalStateError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/NullPointerError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/UncaughtExceptionError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ArithmeticError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ClassCastError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/IllegalArgumentError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ArrayStoreError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/NoDataError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/LinkerError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/LinkerClassNotFoundError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/ArgumentOutOfRangeError;", NO_MIRROR, &W_ERROR_OVERLOADS),
            std::make_tuple("Lstd/core/UnsupportedOperationError;", NO_MIRROR, &W_ERROR_OVERLOADS),
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
            {utf::CStringAsMutf8("<ctor>"), {":V", 1, "<ctor>"}},
            {utf::CStringAsMutf8("$_get"), {"I:LY;", 2, "$_get"}},
            {utf::CStringAsMutf8("$_set"), {"ILY;:V", 3, "$_set"}},
            {utf::CStringAsMutf8("at"), {"I:LY;", 2, "at"}},
            {utf::CStringAsMutf8("indexOf"), {"LY;Lstd/core/Int;:I", 3, "indexOf"}},
            {utf::CStringAsMutf8("lastIndexOf"), {"LY;Lstd/core/Int;:I", 3, "lastIndexOf"}},
            {utf::CStringAsMutf8("toSpliced"), {"II[LY;:Lstd/core/Array;", 3, "toSpliced"}},
            {utf::CStringAsMutf8("push"), {"Lstd/core/Array;:I", 1, "pushArray"}}};

        wArray_ = RegisterClass(PlatformTypes()->escompatArray->GetDescriptor(), "Array", &W_ARRAY_OVERLOADS);
        wArray_->GetOverloadNameMapping()["pushOne"] = "push";
        wArray_->GetOverloadNameMapping()["pushArray"] = "push";
        NAPI_CHECK_FATAL(napi_object_seal(ctx_->GetJSEnv(), jsGlobalEts_));
    }

    void RegisterError()
    {
        wRangeError_ = RegisterClassWithLeafMatcher(PlatformTypes()->coreRangeError->GetDescriptor(), "RangeError");
        wReferenceError_ =
            RegisterClassWithLeafMatcher(PlatformTypes()->coreReferenceError->GetDescriptor(), "ReferenceError");
        wSyntaxError_ = RegisterClassWithLeafMatcher(PlatformTypes()->coreSyntaxError->GetDescriptor(), "SyntaxError");
        wURIError_ = RegisterClassWithLeafMatcher(PlatformTypes()->coreURIError->GetDescriptor(), "URIError");
        wTypeError_ = RegisterClassWithLeafMatcher(PlatformTypes()->coreTypeError->GetDescriptor(), "TypeError");
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
            {utf::CStringAsMutf8("<ctor>"),
             {"{ULstd/core/Iterable;Lstd/core/Null;Lstd/core/ReadonlyArray;}:V", 2, "<ctor>"}}};
        wMap_ = RegisterClass(PlatformTypes()->coreMap->GetDescriptor(), "Map", &W_MAP_OVERLOADS);
    }

    void RegisterSet()
    {
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_SET_OVERLOADS = {
            {utf::CStringAsMutf8("<ctor>"), {"{U[LY;Lstd/core/Iterable;Lstd/core/Null;}:V", 2, "<ctor>"}}};
        wSet_ = RegisterClass(PlatformTypes()->coreSet->GetDescriptor(), "Set", &W_SET_OVERLOADS);
    }

    void RegisterDate()
    {
        static const ets_proxy::EtsClassWrapper::OverloadsMap W_DATE_OVERLOADS = {
            {utf::CStringAsMutf8("setDate"), {"I:J", 2, "setDate"}},
            {utf::CStringAsMutf8("setUTCDate"), {"I:J", 2, "setUTCDate"}},
            {utf::CStringAsMutf8("setMilliseconds"), {"I:J", 2, "setMilliseconds"}},
            {utf::CStringAsMutf8("setUTCMilliseconds"), {"I:J", 2, "setUTCMilliseconds"}},
            {utf::CStringAsMutf8("setTime"), {"J:J", 2, "setTime"}},
            // NOTE (ikorobkov): uncomment when #33175 will be fixed
            //{utf::CStringAsMutf8("toLocaleString"),
            //{"{ULstd/core/Intl/Locale;Lstd/core/ReadonlyArray;Lstd/core/String;}Lstd/core/Intl/"
            // "DateTimeFormatOptions;:Lstd/core/String;",
            // 2, "toLocaleString"}},
        };
        wDate_ = RegisterClassWithLeafMatcher(PlatformTypes()->coreDate->GetDescriptor(), "Date", &W_DATE_OVERLOADS);
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
        wObject_ = RegisterClass(PlatformTypes()->coreObject->GetDescriptor(), "Object");
    }

    void RegisterAny()
    {
        wAny_ =
            RegisterClass(PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(EtsClassRoot::ANY)->GetDescriptor());
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

        return wError_->CreateJSBuiltinProxy(ctxx, jsValue);
    }

    MObjectObjectType GetObjectObjectType(InteropCtx *ctxx, napi_value jsValue)
    {
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        napi_env env = ctxx->GetJSEnv();
        bool isInstanceof;
        NAPI_CHECK_FATAL(napi_is_promise(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return MObjectObjectType::PROMISE;
        }
        NAPI_CHECK_FATAL(napi_is_error(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return MObjectObjectType::ERROR;
        }
        NAPI_CHECK_FATAL(napi_is_date(env, jsValue, &isInstanceof));
        if (isInstanceof) {
            return MObjectObjectType::DATE;
        }
        if (IsConstructor(env, jsValue, CONSTRUCTOR_NAME_NUMBER)) {
            return MObjectObjectType::NUMBER;
        }
        if (IsConstructor(env, jsValue, CONSTRUCTOR_NAME_BOOLEAN)) {
            return MObjectObjectType::BOOLEAN;
        }
        if (IsConstructor(env, jsValue, CONSTRUCTOR_NAME_STRING)) {
            return MObjectObjectType::STRING;
        }
        return MObjectObjectType::JSVALUE;
    }

    EtsObject *MObjectObject(InteropCtx *ctxx, napi_value jsValue)
    {
        napi_env env = ctxx->GetJSEnv();
        auto objectType = GetObjectObjectType(ctxx, jsValue);
        switch (objectType) {
            case MObjectObjectType::PROMISE: {
                return BuiltinConvert<JSConvertPromise>(ctxx, env, jsValue);
            }
            case MObjectObjectType::ERROR: {
                return MError(ctxx, jsValue);
            }
            case MObjectObjectType::DATE: {
                return MDate(ctxx, jsValue);
            }
            case MObjectObjectType::NUMBER: {
                return BuiltinConvert<JSConvertStdlibDouble>(ctxx, env, jsValue);
            }
            case MObjectObjectType::BOOLEAN: {
                return BuiltinConvert<JSConvertStdlibBoolean>(ctxx, env, jsValue);
            }
            case MObjectObjectType::STRING: {
                return BuiltinConvert<JSConvertString>(ctxx, env, jsValue);
            }
            default:
                return BuiltinConvert<JSConvertJSValue>(ctxx, env, jsValue);
        }
    }

    template <bool IS_OBJECT>
    EtsObject *MObjectOrAny(InteropCtx *ctxx, napi_value jsValue)
    {
        napi_env env = ctxx->GetJSEnv();

        napi_valuetype jsType = GetValueType<true>(env, jsValue);
        switch (jsType) {
            case napi_boolean:
                return BuiltinConvert<JSConvertStdlibBoolean>(ctxx, env, jsValue);
            case napi_number:
                return BuiltinConvert<JSConvertStdlibDouble>(ctxx, env, jsValue);
            case napi_string:
                return BuiltinConvert<JSConvertString>(ctxx, env, jsValue);
            case napi_object:
                return MObjectObject(ctxx, jsValue);
            case napi_null: {
                if constexpr (IS_OBJECT) {
                    return NotAssignable("Object");
                } else {
                    return ctxx->GetNullValue();
                }
            }
            case napi_bigint:
                return BuiltinConvert<JSConvertBigInt>(ctxx, env, jsValue);
            case napi_symbol:
                [[fallthrough]];
            case napi_external:
                return BuiltinConvert<JSConvertJSValue>(ctxx, env, jsValue);
            case napi_function: {
                auto refconv = JSRefConvertResolve<true>(ctxx, PlatformTypes()->coreFunction->GetRuntimeClass());
                ASSERT(refconv != nullptr);
                return EtsObject::FromCoreType(refconv->Unwrap(ctxx, jsValue)->GetCoreType());
            }
            default:
                ASSERT(!IsNullOrUndefined(env, jsValue));
                InteropCtx::Fatal(INTEROP_INVALID_ARGUMENT_VALUE, "Bad jsType in Object value matcher");
        };
    }

    EtsObject *MAny(InteropCtx *ctxx, napi_value jsValue, bool verified = true)
    {
        (void)verified;  // ignored for Any
        return MObjectOrAny<false>(ctxx, jsValue);
    }

    EtsObject *MObject(InteropCtx *ctxx, napi_value jsValue, bool verified = true)
    {
        (void)verified;  // ignored for Object
        return MObjectOrAny<true>(ctxx, jsValue);
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
        RegisterAny();

        RegisterExceptions();

        RegisterDate();
        // #IC4UO2
        RegisterClassWithLeafMatcher(PlatformTypes()->coreMapIteratorImpl->GetDescriptor(), nullptr);
        RegisterClassWithLeafMatcher(PlatformTypes()->coreEmptyMapIteratorImpl->GetDescriptor(), nullptr);
        RegisterClassWithLeafMatcher(PlatformTypes()->coreSetIteratorImpl->GetDescriptor(), nullptr);

        RegisterMap();
        RegisterSet();

        RegisterClassWithLeafMatcher(PlatformTypes()->escompatArrayEntriesIteratorT->GetDescriptor(), nullptr);
        RegisterClassWithLeafMatcher(PlatformTypes()->coreIteratorResult->GetDescriptor(), nullptr);
        RegisterClassWithLeafMatcher(PlatformTypes()->coreArrayKeysIterator->GetDescriptor(), nullptr);
        RegisterClassWithLeafMatcher(PlatformTypes()->coreArrayValuesIteratorT->GetDescriptor(), nullptr);

        RegisterClassWithLeafMatcher(PlatformTypes()->coreReflectProxy->GetDescriptor());

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

        ASSERT(wAny_ != nullptr);
        wAny_->SetJSBuiltinMatcher([self = *this](InteropCtx *ctxx, napi_value jsValue, bool verified = true) mutable {
            return self.MAny(ctxx, jsValue, verified);
        });
    }

private:
    InteropCtx *ctx_;
    napi_value jsGlobalEts_ {};
    ets_proxy::EtsClassWrapper *wError_ {};
    ets_proxy::EtsClassWrapper *wObject_ {};
    ets_proxy::EtsClassWrapper *wAny_ {};
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
    auto executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    PandaEtsVM *vm = executionCtx->GetPandaVM();
    EtsClassLinkerExtension *linkerExt = vm->GetClassLinker()->GetEtsClassLinkerExtension();
    auto ptypes = PlatformTypes(executionCtx);

    RegisterBuiltinRefConvertor<JSConvertJSValue>(cache, ptypes->interopJSValue);
    RegisterBuiltinRefConvertor<JSConvertESError>(cache, ptypes->interopESError);
    RegisterBuiltinRefConvertor<JSConvertString>(cache, ptypes->coreString);
    RegisterBuiltinRefConvertor<JSConvertString>(cache, ptypes->coreLineString);
    RegisterBuiltinRefConvertor<JSConvertString>(cache, ptypes->coreSlicedString);
    RegisterBuiltinRefConvertor<JSConvertString>(cache, ptypes->coreTreeString);

    RegisterBuiltinRefConvertor<JSConvertBigInt>(cache, ptypes->coreBigInt);
    RegisterBuiltinRefConvertor<JSConvertPromise>(cache, ptypes->corePromise);
    RegisterBuiltinRefConvertor<JSConvertEtsNull>(cache, ptypes->coreNull);

    RegisterBuiltinRefConvertor<JSConvertStdlibBoolean>(cache, ptypes->coreBoolean);
    RegisterBuiltinRefConvertor<JSConvertStdlibByte>(cache, ptypes->coreByte);
    RegisterBuiltinRefConvertor<JSConvertStdlibChar>(cache, ptypes->coreChar);
    RegisterBuiltinRefConvertor<JSConvertStdlibShort>(cache, ptypes->coreShort);
    RegisterBuiltinRefConvertor<JSConvertStdlibInt>(cache, ptypes->coreInt);
    RegisterBuiltinRefConvertor<JSConvertStdlibLong>(cache, ptypes->coreLong);
    RegisterBuiltinRefConvertor<JSConvertStdlibFloat>(cache, ptypes->coreFloat);
    RegisterBuiltinRefConvertor<JSConvertStdlibDouble>(cache, ptypes->coreDouble);

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
