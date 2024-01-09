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

#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/interop_js/js_value_call.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/handle_scope-inl.h"

#include "runtime/mem/vm_handle-inl.h"

namespace panda::ets::interop::js {

// Convert js->ets, throws ETS/JS exceptions
template <typename FClsResolv, typename FStorePrim, typename FStoreRef>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertNapiVal(InteropCtx *ctx, FClsResolv &resolveRefCls,
                                                              FStorePrim &storePrim, FStoreRef &storeRef,
                                                              panda_file::Type type, napi_value jsVal)
{
    auto env = ctx->GetJSEnv();

    auto unwrapVal = [&ctx, &env, &jsVal, &storeRef, &storePrim](auto convTag) {
        using Convertor = typename decltype(convTag)::type;  // convTag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;         // NOLINT(readability-identifier-naming)
        auto res = Convertor::Unwrap(ctx, env, jsVal);
        if (UNLIKELY(!res.has_value())) {
            return false;
        }
        if constexpr (std::is_pointer_v<cpptype>) {
            storeRef(AsEtsObject(res.value())->GetCoreType());
        } else {
            storePrim(Value(res.value()).GetAsLong());
        }
        return true;
    };

    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID: {
            // do nothing
            return true;
        }
        case panda_file::Type::TypeId::U1:
            return unwrapVal(helpers::TypeIdentity<JSConvertU1>());
        case panda_file::Type::TypeId::I8:
            return unwrapVal(helpers::TypeIdentity<JSConvertI8>());
        case panda_file::Type::TypeId::U8:
            return unwrapVal(helpers::TypeIdentity<JSConvertU8>());
        case panda_file::Type::TypeId::I16:
            return unwrapVal(helpers::TypeIdentity<JSConvertI16>());
        case panda_file::Type::TypeId::U16:
            return unwrapVal(helpers::TypeIdentity<JSConvertU16>());
        case panda_file::Type::TypeId::I32:
            return unwrapVal(helpers::TypeIdentity<JSConvertI32>());
        case panda_file::Type::TypeId::U32:
            return unwrapVal(helpers::TypeIdentity<JSConvertU32>());
        case panda_file::Type::TypeId::I64:
            return unwrapVal(helpers::TypeIdentity<JSConvertI64>());
        case panda_file::Type::TypeId::U64:
            return unwrapVal(helpers::TypeIdentity<JSConvertU64>());
        case panda_file::Type::TypeId::F32:
            return unwrapVal(helpers::TypeIdentity<JSConvertF32>());
        case panda_file::Type::TypeId::F64:
            return unwrapVal(helpers::TypeIdentity<JSConvertF64>());
        case panda_file::Type::TypeId::REFERENCE: {
            if (UNLIKELY(IsNullOrUndefined(env, jsVal))) {
                storeRef(nullptr);
                return true;
            }
            auto klass = resolveRefCls();

            // start fastpath
            if (klass == ctx->GetVoidClass()) {
                return unwrapVal(helpers::TypeIdentity<JSConvertEtsVoid>());
            }
            if (klass == ctx->GetJSValueClass()) {
                return unwrapVal(helpers::TypeIdentity<JSConvertJSValue>());
            }
            if (klass == ctx->GetStringClass()) {
                return unwrapVal(helpers::TypeIdentity<JSConvertString>());
            }
            // start slowpath
            auto refconv = JSRefConvertResolve<true>(ctx, klass);
            if (UNLIKELY(refconv == nullptr)) {
                return false;
            }
            ObjectHeader *res = refconv->Unwrap(ctx, jsVal)->GetCoreType();
            storeRef(res);
            return res != nullptr;
        }
        default: {
            ctx->Fatal(std::string("ConvertNapiVal: unsupported typeid ") +
                       panda_file::Type::GetSignatureByTypeId(type));
        }
    }
    UNREACHABLE();
}

// Convert ets->js, throws JS exceptions
template <typename FClsResolv, typename FStore, typename FRead>
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertEtsVal(InteropCtx *ctx,
                                                             [[maybe_unused]] FClsResolv &resolveRefCls,
                                                             FStore &storeRes, panda_file::Type type, FRead &readVal)
{
    auto env = ctx->GetJSEnv();

    auto wrapPrim = [&env, &readVal, &storeRes](auto convTag) -> bool {
        using Convertor = typename decltype(convTag)::type;  // convTag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;         // NOLINT(readability-identifier-naming)
        napi_value res = Convertor::Wrap(env, readVal(helpers::TypeIdentity<cpptype>()));
        storeRes(res);
        return res != nullptr;
    };

    auto wrapRef = [&env, &storeRes](auto convTag, ObjectHeader *ref) -> bool {
        using Convertor = typename decltype(convTag)::type;  // convTag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;         // NOLINT(readability-identifier-naming)
        cpptype value = std::remove_pointer_t<cpptype>::FromEtsObject(EtsObject::FromCoreType(ref));
        napi_value res = Convertor::Wrap(env, value);
        storeRes(res);
        return res != nullptr;
    };

    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID: {
            storeRes(GetUndefined(env));
            return true;
        }
        case panda_file::Type::TypeId::U1:
            return wrapPrim(helpers::TypeIdentity<JSConvertU1>());
        case panda_file::Type::TypeId::I8:
            return wrapPrim(helpers::TypeIdentity<JSConvertI8>());
        case panda_file::Type::TypeId::U8:
            return wrapPrim(helpers::TypeIdentity<JSConvertU8>());
        case panda_file::Type::TypeId::I16:
            return wrapPrim(helpers::TypeIdentity<JSConvertI16>());
        case panda_file::Type::TypeId::U16:
            return wrapPrim(helpers::TypeIdentity<JSConvertU16>());
        case panda_file::Type::TypeId::I32:
            return wrapPrim(helpers::TypeIdentity<JSConvertI32>());
        case panda_file::Type::TypeId::U32:
            return wrapPrim(helpers::TypeIdentity<JSConvertU32>());
        case panda_file::Type::TypeId::I64:
            return wrapPrim(helpers::TypeIdentity<JSConvertI64>());
        case panda_file::Type::TypeId::U64:
            return wrapPrim(helpers::TypeIdentity<JSConvertU64>());
        case panda_file::Type::TypeId::F32:
            return wrapPrim(helpers::TypeIdentity<JSConvertF32>());
        case panda_file::Type::TypeId::F64:
            return wrapPrim(helpers::TypeIdentity<JSConvertF64>());
        case panda_file::Type::TypeId::REFERENCE: {
            ObjectHeader *ref = readVal(helpers::TypeIdentity<ObjectHeader *>());
            if (UNLIKELY(ref == nullptr)) {
                storeRes(GetNull(env));
                return true;
            }
            auto klass = ref->template ClassAddr<Class>();

            ASSERT(resolveRefCls()->IsAssignableFrom(klass));
            // start fastpath
            if (klass == ctx->GetVoidClass()) {
                return wrapRef(helpers::TypeIdentity<JSConvertEtsVoid>(), ref);
            }
            if (klass == ctx->GetJSValueClass()) {
                return wrapRef(helpers::TypeIdentity<JSConvertJSValue>(), ref);
            }
            if (klass == ctx->GetStringClass()) {
                return wrapRef(helpers::TypeIdentity<JSConvertString>(), ref);
            }
            // start slowpath
            auto refconv = JSRefConvertResolve(ctx, klass);
            auto res = refconv->Wrap(ctx, EtsObject::FromCoreType(ref));
            storeRes(res);
            return res != nullptr;
        }
        default: {
            ctx->Fatal(std::string("ConvertEtsVal: unsupported typeid ") +
                       panda_file::Type::GetSignatureByTypeId(type));
        }
    }
    UNREACHABLE();
}

using ArgValueBox = std::variant<uint64_t, ObjectHeader **>;

template <bool IS_STATIC>
napi_value EtsCallImpl(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv, EtsObject *thisObj)
{
    ASSERT_MANAGED_CODE();

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();

    auto pf = method->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));
    pda.EnumerateTypes([](panda_file::Type /* unused */) {});  // preload reftypes span

    auto resolveRefCls = [&classLinker, &pf, &pda, &ctx](uint32_t idx) {
        auto klass = classLinker->GetLoadedClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
        ASSERT(klass != nullptr);
        return klass;
    };
    auto loadRefCls = [&classLinker, &pf, &pda, &ctx](uint32_t idx) {
        return classLinker->GetClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
    };

    panda_file::ShortyIterator it(method->GetShorty());
    auto refArgIdx = static_cast<uint32_t>(!(*it++).IsPrimitive());  // skip retval

    ASSERT(method->IsStatic() == IS_STATIC);
    static constexpr size_t ETS_ARGS_DISP = IS_STATIC ? 0 : 1;

    auto const numArgs = method->GetNumArgs() - ETS_ARGS_DISP;
    if (UNLIKELY(numArgs != jsargv.size())) {
        InteropCtx::ThrowJSTypeError(ctx->GetJSEnv(), std::string("CallEtsFunction: wrong argc"));
        return nullptr;
    }

    auto etsArgs = ctx->GetTempArgs<Value>(method->GetNumArgs());
    {
        HandleScope<ObjectHeader *> etsHandleScope(coro);

        VMHandle<ObjectHeader> thisObjHandle {};
        if constexpr (!IS_STATIC) {
            thisObjHandle = VMHandle<ObjectHeader>(coro, thisObj->GetCoreType());
        } else {
            (void)thisObj;
            ASSERT(thisObj == nullptr);
        }

        auto etsBoxedArgs = ctx->GetTempArgs<ArgValueBox>(numArgs);

        // Convert and box in VMHandle if necessary
        for (uint32_t argIdx = 0; argIdx < numArgs; ++argIdx, it.IncrementWithoutCheck()) {
            panda_file::Type type = *it;
            auto jsVal = jsargv[argIdx];
            auto clsResolver = [&loadRefCls, &refArgIdx]() { return loadRefCls(refArgIdx++); };
            auto storePrim = [&etsBoxedArgs, &argIdx](uint64_t val) { etsBoxedArgs[argIdx] = val; };
            auto storeRef = [&coro, &etsBoxedArgs, &argIdx](ObjectHeader *obj) {
                uintptr_t addr = VMHandle<ObjectHeader>(coro, obj).GetAddress();
                etsBoxedArgs[argIdx] = reinterpret_cast<ObjectHeader **>(addr);
            };
            if (UNLIKELY(!ConvertNapiVal(ctx, clsResolver, storePrim, storeRef, type, jsVal))) {
                if (coro->HasPendingException()) {
                    ctx->ForwardEtsException(coro);
                }
                ASSERT(ctx->SanityJSExceptionPending());
                return nullptr;
            }
        }

        // Unbox VMHandles
        for (size_t i = 0; i < numArgs; ++i) {
            ArgValueBox &box = etsBoxedArgs[i];
            if (std::holds_alternative<ObjectHeader **>(box)) {
                ObjectHeader **slot = std::get<1>(box);
                etsArgs[ETS_ARGS_DISP + i] = Value(slot != nullptr ? *slot : nullptr);
            } else {
                etsArgs[ETS_ARGS_DISP + i] = Value(std::get<0>(box));
            }
        }
        if constexpr (!IS_STATIC) {
            etsArgs[0] = Value(thisObjHandle.GetPtr());
        }
    }

    ctx->GetInteropFrames().push_back({coro->GetCurrentFrame(), false});

    Value etsRes = method->Invoke(coro, etsArgs->data());

    ctx->GetInteropFrames().pop_back();

    if (UNLIKELY(coro->HasPendingException())) {
        ctx->ForwardEtsException(coro);
        return nullptr;
    }

    napi_value jsRes;
    {
        auto type = method->GetReturnType();
        auto clsResolver = [&resolveRefCls]() { return resolveRefCls(0); };
        auto storeRes = [&sRes](napi_value res) { jsRes = res; };
        auto readVal = [&etsRes](auto typeTag) { return etsRes.GetAs<typename decltype(typeTag)::type>(); };
        if (UNLIKELY(!ConvertEtsVal(ctx, clsResolver, storeRes, type, readVal))) {
            ASSERT(ctx->SanityJSExceptionPending());
            return nullptr;
        }
    }
    return jsRes;
}

// Explicit instantiation
template napi_value EtsCallImpl<false>(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                                       EtsObject *thisObj);
template napi_value EtsCallImpl<true>(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                                      EtsObject *thisObj);

napi_value CallEtsFunctionImpl(napi_env env, Span<napi_value> jsargv)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope scope(ctx, env);

    // NOLINTNEXTLINE(readability-container-size-empty)
    if (UNLIKELY(jsargv.size() < 1)) {
        InteropCtx::ThrowJSError(env, "CallEtsFunction: method name required");
        return nullptr;
    }

    if (UNLIKELY(GetValueType(env, jsargv[0]) != napi_string)) {
        InteropCtx::ThrowJSError(env, "CallEtsFunction: method name is not a string");
        return nullptr;
    }

    auto callTarget = GetString(env, jsargv[0]);
    std::string packageName {};
    std::string methodName {};

    auto packageSep = callTarget.rfind('.');
    if (packageSep != std::string::npos) {
        packageName = callTarget.substr(0, packageSep + 1);
        methodName = callTarget.substr(packageSep + 1, callTarget.size());
    } else {
        methodName = callTarget;
    }

    auto entrypoint = packageName + std::string("ETSGLOBAL::") + methodName;
    INTEROP_LOG(DEBUG) << "CallEtsFunction: method name: " << entrypoint;

    auto methodRes = Runtime::GetCurrent()->ResolveEntryPoint(entrypoint);
    if (UNLIKELY(!methodRes)) {
        InteropCtx::ThrowJSError(env, "CallEtsFunction: can't resolve method " + entrypoint);
        return nullptr;
    }

    ScopedManagedCodeThread managedScope(coro);
    return EtsCallImplStatic(coro, ctx, methodRes.Value(), jsargv.SubSpan(1));
}

napi_value EtsLambdaProxyInvoke(napi_env env, napi_callback_info cbinfo)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope envscope(ctx, env);

    size_t argc;
    napi_value athis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, nullptr, &athis, &data));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, jsArgs->data(), &athis, &data));

    auto sharedRef = static_cast<ets_proxy::SharedReference *>(data);
    ASSERT(sharedRef != nullptr);

    ScopedManagedCodeThread managedScope(coro);
    auto etsThis = sharedRef->GetEtsObject(ctx);
    ASSERT(etsThis != nullptr);
    auto method = etsThis->GetClass()->GetMethod("invoke");
    ASSERT(method != nullptr);

    return EtsCallImplInstance(coro, ctx, method->GetPandaMethod(), *jsArgs, etsThis);
}

template <bool IS_NEWCALL, typename FSetupArgs>
static ALWAYS_INLINE inline uint64_t JSRuntimeJSCallImpl(FSetupArgs &setupArgs, Method *method, uint8_t *args,
                                                         uint8_t *inStackArgs)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto classLinker = Runtime::GetCurrent()->GetClassLinker();

    auto pf = method->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));
    pda.EnumerateTypes([](panda_file::Type /* unused */) {});  // preload reftypes span

    auto resolveRefCls = [&classLinker, &pf, &pda, &ctx](uint32_t idx) {
        auto klass = classLinker->GetLoadedClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
        ASSERT(klass != nullptr);
        return klass;
    };
    [[maybe_unused]] auto loadRefCls = [&classLinker, &pf, &pda, &ctx](uint32_t idx) {
        return classLinker->GetClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
    };

    Span<uint8_t> inGprArgs(args, arch::ExtArchTraits<RUNTIME_ARCH>::GP_ARG_NUM_BYTES);
    Span<uint8_t> inFprArgs(inGprArgs.end(), arch::ExtArchTraits<RUNTIME_ARCH>::FP_ARG_NUM_BYTES);
    arch::ArgReader<RUNTIME_ARCH> argReader(inGprArgs, inFprArgs, inStackArgs);

    napi_env env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    napi_value jsThis;
    napi_value jsFn;
    panda_file::ShortyIterator it;
    uint32_t numArgs;
    uint32_t refArgIdx;
    {
        argReader.Read<Method *>();  // skip method

        auto setupRes = setupArgs(ctx, env, method, argReader);
        if (UNLIKELY(!setupRes.has_value())) {
            ctx->ForwardJSException(coro);
            return 0;
        }
        std::tie(jsThis, jsFn, it, numArgs, refArgIdx) = setupRes.value();

        if (UNLIKELY(GetValueType(env, jsFn) != napi_function)) {
            ctx->ThrowJSTypeError(env, "call target is not a function");
            ctx->ForwardJSException(coro);
            return 0;
        }
    }

    auto jsargs = ctx->GetTempArgs<napi_value>(numArgs);

    for (uint32_t argIdx = 0; argIdx < numArgs; ++argIdx, it.IncrementWithoutCheck()) {
        auto clsResolver = [&resolveRefCls, &refArgIdx]() { return resolveRefCls(refArgIdx++); };
        auto storeRes = [&jsargs, &argIdx](napi_value res) { jsargs[argIdx] = res; };
        auto readVal = [&argReader](auto typeTag) { return argReader.Read<typename decltype(typeTag)::type>(); };
        if (UNLIKELY(!ConvertEtsVal(ctx, clsResolver, storeRes, *it, readVal))) {
            ctx->ForwardJSException(coro);
            return 0;
        }
    }

    napi_value jsRet;
    napi_status jsStatus;
    {
        ctx->GetInteropFrames().push_back({coro->GetCurrentFrame(), true});
        ScopedNativeCodeThread nativeScope(coro);

        if constexpr (IS_NEWCALL) {
            jsStatus = napi_new_instance(env, jsFn, jsargs->size(), jsargs->data(), &jsRet);
        } else {
            jsStatus = napi_call_function(env, jsThis, jsFn, jsargs->size(), jsargs->data(), &jsRet);
        }

        ctx->GetInteropFrames().pop_back();
    }

    if (UNLIKELY(jsStatus != napi_ok)) {
        INTEROP_FATAL_IF(jsStatus != napi_pending_exception);
        ctx->ForwardJSException(coro);
        return 0;
    }

    Value etsRet;

    if constexpr (IS_NEWCALL) {
        INTEROP_FATAL_IF(resolveRefCls(0) != ctx->GetJSValueClass());
        auto res = JSConvertJSValue::Unwrap(ctx, env, jsRet);
        if (!res.has_value()) {
            ctx->Fatal("newcall result unwrap failed, but shouldnt");
        }
        etsRet = Value(res.value()->GetCoreType());
    } else {
        panda_file::Type type = method->GetReturnType();
        auto clsResolver = [&loadRefCls]() { return loadRefCls(0); };
        auto storePrim = [&etsRet](uint64_t val) { etsRet = Value(val); };
        auto storeRef = [&etsRet](ObjectHeader *obj) { etsRet = Value(obj); };
        if (UNLIKELY(!ConvertNapiVal(ctx, clsResolver, storePrim, storeRef, type, jsRet))) {
            if (NapiIsExceptionPending(env)) {
                ctx->ForwardJSException(coro);
            }
            ASSERT(ctx->SanityETSExceptionPending());
            return 0;
        }
    }

    return static_cast<uint64_t>(etsRet.GetAsLong());
}

static inline std::optional<std::pair<napi_value, napi_value>> CompilerResolveQualifiedJSCall(
    napi_env env, napi_value jsVal, coretypes::String *qnameStr)
{
    napi_value jsThis {};
    ASSERT(qnameStr->IsMUtf8());
    auto qname = std::string_view(utf::Mutf8AsCString(qnameStr->GetDataMUtf8()), qnameStr->GetMUtf8Length());

    auto resolveName = [&jsThis, &jsVal, &env](const std::string &name) -> bool {
        jsThis = jsVal;
        INTEROP_LOG(DEBUG) << "JSRuntimeJSCall: resolve name: " << name;
        napi_status rc = napi_get_named_property(env, jsVal, name.c_str(), &jsVal);
        if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
            ASSERT(NapiIsExceptionPending(env));
            return false;
        }
        return true;
    };
    jsThis = jsVal;
    if (UNLIKELY(!WalkQualifiedName(qname, resolveName))) {
        return std::nullopt;
    }
    return std::make_pair(jsThis, jsVal);
}

template <bool IS_NEWCALL>
static ALWAYS_INLINE inline uint64_t JSRuntimeJSCallBase(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    auto argsetup =
        []([[maybe_unused]] InteropCtx * ctx, napi_env env, Method * methodd, arch::ArgReader<RUNTIME_ARCH> & argReader)
            __attribute__((always_inline))
                ->std::optional<std::tuple<napi_value, napi_value, panda_file::ShortyIterator, uint32_t, uint32_t>>
    {
        ASSERT(methodd->IsStatic());

        panda_file::ShortyIterator it(methodd->GetShorty());
        uint32_t numArgs = methodd->GetNumArgs() - 2;
        uint32_t refArgIdx = !(*it++).IsPrimitive() + 2;
        it.IncrementWithoutCheck();
        it.IncrementWithoutCheck();

        napi_value jsVal = JSConvertJSValue::Wrap(env, argReader.Read<JSValue *>());
        auto qnameStr = argReader.Read<coretypes::String *>();
        ASSERT(qnameStr->ClassAddr<Class>()->IsStringClass());
        auto res = CompilerResolveQualifiedJSCall(env, jsVal, qnameStr);
        if (UNLIKELY(!res.has_value())) {
            ASSERT(NapiIsExceptionPending(env));
            return std::nullopt;
        }
        return std::make_tuple(res->first, res->second, it, numArgs, refArgIdx);
    };
    return JSRuntimeJSCallImpl<IS_NEWCALL>(argsetup, method, args, inStackArgs);
}

extern "C" uint64_t JSRuntimeJSCall(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    return JSRuntimeJSCallBase</*IS_NEWCALL=*/false>(method, args, inStackArgs);
}
extern "C" void JSRuntimeJSCallBridge(Method *method, ...);

extern "C" uint64_t JSRuntimeJSNew(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    return JSRuntimeJSCallBase</*IS_NEWCALL=*/true>(method, args, inStackArgs);
}
extern "C" void JSRuntimeJSNewBridge(Method *method, ...);

extern "C" uint64_t JSRuntimeJSCallByValue(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    auto argsetup =
        []([[maybe_unused]] InteropCtx * ctx, napi_env env, Method * methodd, arch::ArgReader<RUNTIME_ARCH> & argReader)
            __attribute__((always_inline))
                ->std::optional<std::tuple<napi_value, napi_value, panda_file::ShortyIterator, uint32_t, uint32_t>>
    {
        ASSERT(methodd->IsStatic());

        panda_file::ShortyIterator it(methodd->GetShorty());
        uint32_t numArgs = methodd->GetNumArgs() - 2;
        uint32_t refArgIdx = static_cast<uint32_t>(!(*it++).IsPrimitive()) + 2;
        it.IncrementWithoutCheck();
        it.IncrementWithoutCheck();

        napi_value jsFn = JSConvertJSValue::Wrap(env, argReader.Read<JSValue *>());
        napi_value jsThis = JSConvertJSValue::Wrap(env, argReader.Read<JSValue *>());

        return std::make_tuple(jsThis, jsFn, it, numArgs, refArgIdx);
    };
    return JSRuntimeJSCallImpl</*IS_NEWCALL=*/false>(argsetup, method, args, inStackArgs);
}
extern "C" void JSRuntimeJSCallByValueBridge(Method *method, ...);

extern "C" uint64_t JSProxyCall(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    auto argsetup =
        [](InteropCtx * ctx, napi_env env, Method * methodd, arch::ArgReader<RUNTIME_ARCH> & argReader)
            __attribute__((always_inline))
                ->std::optional<std::tuple<napi_value, napi_value, panda_file::ShortyIterator, uint32_t, uint32_t>>
    {
        INTEROP_LOG(DEBUG) << "JSRuntimeJSCallImpl: JSProxy call: " << methodd->GetFullName(true);
        ASSERT(!methodd->IsStatic());

        panda_file::ShortyIterator it(methodd->GetShorty());
        uint32_t numArgs = methodd->GetNumArgs() - 1;
        uint32_t refArgIdx = static_cast<uint32_t>(!(*it++).IsPrimitive());

        auto *etsThis = argReader.Read<EtsObject *>();
        Class *cls = etsThis->GetClass()->GetRuntimeClass();
        auto refconv = JSRefConvertResolve(ctx, cls);
        napi_value jsThis = refconv->Wrap(ctx, etsThis);
        ASSERT(GetValueType(env, jsThis) == napi_object);
        const char *methodName = utf::Mutf8AsCString(methodd->GetName().data);
        napi_value jsFn;
        napi_status rc = napi_get_named_property(env, jsThis, methodName, &jsFn);
        if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
            ASSERT(NapiIsExceptionPending(env));
            return std::nullopt;
        }
        return std::make_tuple(jsThis, jsFn, it, numArgs, refArgIdx);
    };
    return JSRuntimeJSCallImpl</*IS_NEWCALL=*/false>(argsetup, method, args, inStackArgs);
}
extern "C" void JSProxyCallBridge(Method *method, ...);

template <bool IS_NEWCALL>
static void InitJSCallSignatures(coretypes::String *clsStr)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto classLinker = Runtime::GetCurrent()->GetClassLinker();

    std::string classDescriptor(utf::Mutf8AsCString(clsStr->GetDataMUtf8()));
    INTEROP_LOG(DEBUG) << "Intialize jscall signatures for " << classDescriptor;
    EtsClass *etsClass = coro->GetPandaVM()->GetClassLinker()->GetClass(classDescriptor.c_str());
    INTEROP_FATAL_IF(etsClass == nullptr);
    auto klass = etsClass->GetRuntimeClass();

    INTEROP_LOG(DEBUG) << "Bind bridge call methods for " << utf::Mutf8AsCString(klass->GetDescriptor());

    for (auto &method : klass->GetMethods()) {
        if (method.IsConstructor()) {
            continue;
        }
        ASSERT(method.IsStatic());
        auto pf = method.GetPandaFile();
        panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method.GetFileId()));
        pda.EnumerateTypes([](panda_file::Type /* unused */) {});  // preload reftypes span

        void *methodEp = nullptr;
        if constexpr (IS_NEWCALL) {
            methodEp = reinterpret_cast<void *>(JSRuntimeJSNewBridge);
        } else {
            uint32_t const argReftypeShift = method.GetReturnType().IsReference() ? 1 : 0;
            ASSERT(method.GetArgType(0).IsReference());  // arg0 is always a reference
            ASSERT(method.GetArgType(1).IsReference());  // arg1 is always a reference
            auto cls1 = classLinker->GetClass(*pf, pda.GetReferenceType(1 + argReftypeShift), ctx->LinkerCtx());
            if (cls1->IsStringClass()) {
                methodEp = reinterpret_cast<void *>(JSRuntimeJSCallBridge);
            } else {
                ASSERT(cls1 == ctx->GetJSValueClass());
                methodEp = reinterpret_cast<void *>(JSRuntimeJSCallByValueBridge);
            }
        }
        method.SetCompiledEntryPoint(methodEp);
        method.SetNativePointer(nullptr);
    }
}

uint8_t JSRuntimeInitJSCallClass(EtsString *clsStr)
{
    InitJSCallSignatures<false>(clsStr->GetCoreType());
    return 1;
}

uint8_t JSRuntimeInitJSNewClass(EtsString *clsStr)
{
    InitJSCallSignatures<true>(clsStr->GetCoreType());
    return 1;
}

}  // namespace panda::ets::interop::js
