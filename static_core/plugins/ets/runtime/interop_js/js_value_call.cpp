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
[[nodiscard]] static ALWAYS_INLINE inline bool ConvertNapiVal(InteropCtx *ctx, FClsResolv &resolve_ref_cls,
                                                              FStorePrim &store_prim, FStoreRef &store_ref,
                                                              panda_file::Type type, napi_value js_val)
{
    auto env = ctx->GetJSEnv();

    auto unwrap_val = [&](auto conv_tag) {
        using Convertor = typename decltype(conv_tag)::type;  // conv_tag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;          // NOLINT(readability-identifier-naming)
        auto res = Convertor::Unwrap(ctx, env, js_val);
        if (UNLIKELY(!res.has_value())) {
            return false;
        }
        if constexpr (std::is_pointer_v<cpptype>) {
            store_ref(AsEtsObject(res.value())->GetCoreType());
        } else {
            store_prim(Value(res.value()).GetAsLong());
        }
        return true;
    };

    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID: {
            // do nothing
            return true;
        }
        case panda_file::Type::TypeId::U1:
            return unwrap_val(helpers::TypeIdentity<JSConvertU1>());
        case panda_file::Type::TypeId::I8:
            return unwrap_val(helpers::TypeIdentity<JSConvertI8>());
        case panda_file::Type::TypeId::U8:
            return unwrap_val(helpers::TypeIdentity<JSConvertU8>());
        case panda_file::Type::TypeId::I16:
            return unwrap_val(helpers::TypeIdentity<JSConvertI16>());
        case panda_file::Type::TypeId::U16:
            return unwrap_val(helpers::TypeIdentity<JSConvertU16>());
        case panda_file::Type::TypeId::I32:
            return unwrap_val(helpers::TypeIdentity<JSConvertI32>());
        case panda_file::Type::TypeId::U32:
            return unwrap_val(helpers::TypeIdentity<JSConvertU32>());
        case panda_file::Type::TypeId::I64:
            return unwrap_val(helpers::TypeIdentity<JSConvertI64>());
        case panda_file::Type::TypeId::U64:
            return unwrap_val(helpers::TypeIdentity<JSConvertU64>());
        case panda_file::Type::TypeId::F32:
            return unwrap_val(helpers::TypeIdentity<JSConvertF32>());
        case panda_file::Type::TypeId::F64:
            return unwrap_val(helpers::TypeIdentity<JSConvertF64>());
        case panda_file::Type::TypeId::REFERENCE: {
            if (UNLIKELY(IsNullOrUndefined(env, js_val))) {
                store_ref(nullptr);
                return true;
            }
            auto klass = resolve_ref_cls();

            if (klass == ctx->GetVoidClass()) {
                // do nothing
                return true;
            }
            // start fastpath
            if (klass == ctx->GetJSValueClass()) {
                return unwrap_val(helpers::TypeIdentity<JSConvertJSValue>());
            }
            if (klass == ctx->GetStringClass()) {
                return unwrap_val(helpers::TypeIdentity<JSConvertString>());
            }
            // start slowpath
            auto refconv = JSRefConvertResolve<true>(ctx, klass);
            if (UNLIKELY(refconv == nullptr)) {
                return false;
            }
            ObjectHeader *res = refconv->Unwrap(ctx, js_val)->GetCoreType();
            store_ref(res);
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
                                                             [[maybe_unused]] FClsResolv &resolve_ref_cls,
                                                             FStore &store_res, panda_file::Type type, FRead &read_val)
{
    auto env = ctx->GetJSEnv();

    auto wrap_prim = [&](auto conv_tag) -> bool {
        using Convertor = typename decltype(conv_tag)::type;  // conv_tag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;          // NOLINT(readability-identifier-naming)
        napi_value res = Convertor::Wrap(env, read_val(helpers::TypeIdentity<cpptype>()));
        store_res(res);
        return res != nullptr;
    };

    auto wrap_ref = [&](auto conv_tag, ObjectHeader *ref) -> bool {
        using Convertor = typename decltype(conv_tag)::type;  // conv_tag acts as lambda template parameter
        using cpptype = typename Convertor::cpptype;          // NOLINT(readability-identifier-naming)
        cpptype value = std::remove_pointer_t<cpptype>::FromEtsObject(EtsObject::FromCoreType(ref));
        napi_value res = Convertor::Wrap(env, value);
        store_res(res);
        return res != nullptr;
    };

    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID: {
            store_res(GetUndefined(env));
            return true;
        }
        case panda_file::Type::TypeId::U1:
            return wrap_prim(helpers::TypeIdentity<JSConvertU1>());
        case panda_file::Type::TypeId::I8:
            return wrap_prim(helpers::TypeIdentity<JSConvertI8>());
        case panda_file::Type::TypeId::U8:
            return wrap_prim(helpers::TypeIdentity<JSConvertU8>());
        case panda_file::Type::TypeId::I16:
            return wrap_prim(helpers::TypeIdentity<JSConvertI16>());
        case panda_file::Type::TypeId::U16:
            return wrap_prim(helpers::TypeIdentity<JSConvertU16>());
        case panda_file::Type::TypeId::I32:
            return wrap_prim(helpers::TypeIdentity<JSConvertI32>());
        case panda_file::Type::TypeId::U32:
            return wrap_prim(helpers::TypeIdentity<JSConvertU32>());
        case panda_file::Type::TypeId::I64:
            return wrap_prim(helpers::TypeIdentity<JSConvertI64>());
        case panda_file::Type::TypeId::U64:
            return wrap_prim(helpers::TypeIdentity<JSConvertU64>());
        case panda_file::Type::TypeId::F32:
            return wrap_prim(helpers::TypeIdentity<JSConvertF32>());
        case panda_file::Type::TypeId::F64:
            return wrap_prim(helpers::TypeIdentity<JSConvertF64>());
        case panda_file::Type::TypeId::REFERENCE: {
            ObjectHeader *ref = read_val(helpers::TypeIdentity<ObjectHeader *>());
            if (UNLIKELY(ref == nullptr)) {
                store_res(GetNull(env));
                return true;
            }
            auto klass = ref->template ClassAddr<Class>();

            ASSERT(resolve_ref_cls()->IsAssignableFrom(klass));
            if (klass == ctx->GetVoidClass()) {
                store_res(GetUndefined(env));
                return true;
            }
            // start fastpath
            if (klass == ctx->GetJSValueClass()) {
                return wrap_ref(helpers::TypeIdentity<JSConvertJSValue>(), ref);
            }
            if (klass == ctx->GetStringClass()) {
                return wrap_ref(helpers::TypeIdentity<JSConvertString>(), ref);
            }
            // start slowpath
            auto refconv = JSRefConvertResolve(ctx, klass);
            auto res = refconv->Wrap(ctx, EtsObject::FromCoreType(ref));
            store_res(res);
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
napi_value EtsCallImpl(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                       EtsObject *this_obj)
{
    ASSERT_MANAGED_CODE();

    auto class_linker = Runtime::GetCurrent()->GetClassLinker();

    auto pf = method->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));
    pda.EnumerateTypes([](panda_file::Type /* unused */) {});  // preload reftypes span

    auto resolve_ref_cls = [&](uint32_t idx) {
        auto klass = class_linker->GetLoadedClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
        ASSERT(klass != nullptr);
        return klass;
    };
    auto load_ref_cls = [&](uint32_t idx) {
        return class_linker->GetClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
    };

    panda_file::ShortyIterator it(method->GetShorty());
    auto ref_arg_idx = static_cast<uint32_t>(!(*it++).IsPrimitive());  // skip retval

    ASSERT(method->IsStatic() == IS_STATIC);
    static constexpr size_t ETS_ARGS_DISP = IS_STATIC ? 0 : 1;

    auto const num_args = method->GetNumArgs() - ETS_ARGS_DISP;
    if (UNLIKELY(num_args != jsargv.size())) {
        InteropCtx::ThrowJSTypeError(ctx->GetJSEnv(), std::string("CallEtsFunction: wrong argc"));
        return nullptr;
    }

    auto ets_args = ctx->GetTempArgs<Value>(method->GetNumArgs());
    {
        HandleScope<ObjectHeader *> ets_handle_scope(coro);

        VMHandle<ObjectHeader> this_obj_handle {};
        if constexpr (!IS_STATIC) {
            this_obj_handle = VMHandle<ObjectHeader>(coro, this_obj->GetCoreType());
        } else {
            (void)this_obj;
            ASSERT(this_obj == nullptr);
        }

        auto ets_boxed_args = ctx->GetTempArgs<ArgValueBox>(num_args);

        // Convert and box in VMHandle if necessary
        for (uint32_t arg_idx = 0; arg_idx < num_args; ++arg_idx, it.IncrementWithoutCheck()) {
            panda_file::Type type = *it;
            auto js_val = jsargv[arg_idx];
            auto cls_resolver = [&]() { return load_ref_cls(ref_arg_idx++); };
            auto store_prim = [&](uint64_t val) { ets_boxed_args[arg_idx] = val; };
            auto store_ref = [&](ObjectHeader *obj) {
                uintptr_t addr = VMHandle<ObjectHeader>(coro, obj).GetAddress();
                ets_boxed_args[arg_idx] = reinterpret_cast<ObjectHeader **>(addr);
            };
            if (UNLIKELY(!ConvertNapiVal(ctx, cls_resolver, store_prim, store_ref, type, js_val))) {
                if (coro->HasPendingException()) {
                    ctx->ForwardEtsException(coro);
                }
                ASSERT(ctx->SanityJSExceptionPending());
                return nullptr;
            }
        }

        // Unbox VMHandles
        for (size_t i = 0; i < num_args; ++i) {
            ArgValueBox &box = ets_boxed_args[i];
            if (std::holds_alternative<ObjectHeader **>(box)) {
                ObjectHeader **slot = std::get<1>(box);
                ets_args[ETS_ARGS_DISP + i] = Value(slot != nullptr ? *slot : nullptr);
            } else {
                ets_args[ETS_ARGS_DISP + i] = Value(std::get<0>(box));
            }
        }
        if constexpr (!IS_STATIC) {
            ets_args[0] = Value(this_obj_handle.GetPtr());
        }
    }

    ctx->GetInteropFrames().push_back({coro->GetCurrentFrame(), false});

    Value ets_res = method->Invoke(coro, ets_args->data());

    ctx->GetInteropFrames().pop_back();

    if (UNLIKELY(coro->HasPendingException())) {
        ctx->ForwardEtsException(coro);
        return nullptr;
    }

    napi_value js_res;
    {
        auto type = method->GetReturnType();
        auto cls_resolver = [&]() { return resolve_ref_cls(0); };
        auto store_res = [&](napi_value res) { js_res = res; };
        auto read_val = [&](auto type_tag) { return ets_res.GetAs<typename decltype(type_tag)::type>(); };
        if (UNLIKELY(!ConvertEtsVal(ctx, cls_resolver, store_res, type, read_val))) {
            ASSERT(ctx->SanityJSExceptionPending());
            return nullptr;
        }
    }
    return js_res;
}

// Explicit instantiation
template napi_value EtsCallImpl<false>(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                                       EtsObject *this_obj);
template napi_value EtsCallImpl<true>(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                                      EtsObject *this_obj);

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

    auto call_target = GetString(env, jsargv[0]);
    std::string package_name {};
    std::string method_name {};

    auto package_sep = call_target.rfind('.');
    if (package_sep != std::string::npos) {
        package_name = call_target.substr(0, package_sep + 1);
        method_name = call_target.substr(package_sep + 1, call_target.size());
    } else {
        method_name = call_target;
    }

    auto entrypoint = package_name + std::string("ETSGLOBAL::") + method_name;
    INTEROP_LOG(DEBUG) << "CallEtsFunction: method name: " << entrypoint;

    auto method_res = Runtime::GetCurrent()->ResolveEntryPoint(entrypoint);
    if (UNLIKELY(!method_res)) {
        InteropCtx::ThrowJSError(env, "CallEtsFunction: can't resolve method " + entrypoint);
        return nullptr;
    }

    ScopedManagedCodeThread managed_scope(coro);
    return EtsCallImplStatic(coro, ctx, method_res.Value(), jsargv.SubSpan(1));
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
    auto js_args = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, js_args->data(), &athis, &data));

    auto shared_ref = static_cast<ets_proxy::SharedReference *>(data);
    ASSERT(shared_ref != nullptr);

    ScopedManagedCodeThread managed_scope(coro);
    auto ets_this = shared_ref->GetEtsObject(ctx);
    ASSERT(ets_this != nullptr);
    auto method = ets_this->GetClass()->GetMethod("invoke");
    ASSERT(method != nullptr);

    return EtsCallImplInstance(coro, ctx, method->GetPandaMethod(), *js_args, ets_this);
}

template <bool IS_NEWCALL, typename FSetupArgs>
static ALWAYS_INLINE inline uint64_t JSRuntimeJSCallImpl(FSetupArgs &setup_args, Method *method, uint8_t *args,
                                                         uint8_t *in_stack_args)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto class_linker = Runtime::GetCurrent()->GetClassLinker();

    auto pf = method->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));
    pda.EnumerateTypes([](panda_file::Type /* unused */) {});  // preload reftypes span

    auto resolve_ref_cls = [&](uint32_t idx) {
        auto klass = class_linker->GetLoadedClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
        ASSERT(klass != nullptr);
        return klass;
    };
    [[maybe_unused]] auto load_ref_cls = [&](uint32_t idx) {
        return class_linker->GetClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
    };

    Span<uint8_t> in_gpr_args(args, arch::ExtArchTraits<RUNTIME_ARCH>::GP_ARG_NUM_BYTES);
    Span<uint8_t> in_fpr_args(in_gpr_args.end(), arch::ExtArchTraits<RUNTIME_ARCH>::FP_ARG_NUM_BYTES);
    arch::ArgReader<RUNTIME_ARCH> arg_reader(in_gpr_args, in_fpr_args, in_stack_args);

    napi_env env = ctx->GetJSEnv();
    NapiScope js_handle_scope(env);

    napi_value js_this;
    napi_value js_fn;
    panda_file::ShortyIterator it;
    uint32_t num_args;
    uint32_t ref_arg_idx;
    {
        arg_reader.Read<Method *>();  // skip method

        auto setup_res = setup_args(ctx, env, method, arg_reader);
        if (UNLIKELY(!setup_res.has_value())) {
            ctx->ForwardJSException(coro);
            return 0;
        }
        std::tie(js_this, js_fn, it, num_args, ref_arg_idx) = setup_res.value();

        if (UNLIKELY(GetValueType(env, js_fn) != napi_function)) {
            ctx->ThrowJSTypeError(env, "call target is not a function");
            ctx->ForwardJSException(coro);
            return 0;
        }
    }

    auto jsargs = ctx->GetTempArgs<napi_value>(num_args);

    for (uint32_t arg_idx = 0; arg_idx < num_args; ++arg_idx, it.IncrementWithoutCheck()) {
        auto cls_resolver = [&]() { return resolve_ref_cls(ref_arg_idx++); };
        auto store_res = [&](napi_value res) { jsargs[arg_idx] = res; };
        auto read_val = [&](auto type_tag) { return arg_reader.Read<typename decltype(type_tag)::type>(); };
        if (UNLIKELY(!ConvertEtsVal(ctx, cls_resolver, store_res, *it, read_val))) {
            ctx->ForwardJSException(coro);
            return 0;
        }
    }

    napi_value js_ret;
    napi_status js_status;
    {
        ctx->GetInteropFrames().push_back({coro->GetCurrentFrame(), true});
        ScopedNativeCodeThread native_scope(coro);

        if constexpr (IS_NEWCALL) {
            js_status = napi_new_instance(env, js_fn, jsargs->size(), jsargs->data(), &js_ret);
        } else {
            js_status = napi_call_function(env, js_this, js_fn, jsargs->size(), jsargs->data(), &js_ret);
        }

        ctx->GetInteropFrames().pop_back();
    }

    if (UNLIKELY(js_status != napi_ok)) {
        INTEROP_FATAL_IF(js_status != napi_pending_exception);
        ctx->ForwardJSException(coro);
        return 0;
    }

    Value ets_ret;

    if constexpr (IS_NEWCALL) {
        INTEROP_FATAL_IF(resolve_ref_cls(0) != ctx->GetJSValueClass());
        auto res = JSConvertJSValue::Unwrap(ctx, env, js_ret);
        if (!res.has_value()) {
            ctx->Fatal("newcall result unwrap failed, but shouldnt");
        }
        ets_ret = Value(res.value()->GetCoreType());
    } else {
        panda_file::Type type = method->GetReturnType();
        auto cls_resolver = [&]() { return load_ref_cls(0); };
        auto store_prim = [&](uint64_t val) { ets_ret = Value(val); };
        auto store_ref = [&](ObjectHeader *obj) { ets_ret = Value(obj); };
        if (UNLIKELY(!ConvertNapiVal(ctx, cls_resolver, store_prim, store_ref, type, js_ret))) {
            if (NapiIsExceptionPending(env)) {
                ctx->ForwardJSException(coro);
            }
            ASSERT(ctx->SanityETSExceptionPending());
            return 0;
        }
    }

    return static_cast<uint64_t>(ets_ret.GetAsLong());
}

static inline std::optional<std::pair<napi_value, napi_value>> CompilerResolveQualifiedJSCall(
    napi_env env, napi_value js_val, coretypes::String *qname_str)
{
    napi_value js_this {};
    ASSERT(qname_str->IsMUtf8());
    auto qname = std::string_view(utf::Mutf8AsCString(qname_str->GetDataMUtf8()), qname_str->GetMUtf8Length());

    auto resolve_name = [&](const std::string &name) -> bool {
        js_this = js_val;
        INTEROP_LOG(DEBUG) << "JSRuntimeJSCall: resolve name: " << name;
        napi_status rc = napi_get_named_property(env, js_val, name.c_str(), &js_val);
        if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
            ASSERT(NapiIsExceptionPending(env));
            return false;
        }
        return true;
    };
    js_this = js_val;
    if (UNLIKELY(!WalkQualifiedName(qname, resolve_name))) {
        return std::nullopt;
    }
    return std::make_pair(js_this, js_val);
}

template <bool IS_NEWCALL>
static ALWAYS_INLINE inline uint64_t JSRuntimeJSCallBase(Method *method, uint8_t *args, uint8_t *in_stack_args)
{
    auto argsetup =
        []([[maybe_unused]] InteropCtx * ctx, napi_env env, Method * methodd,
           arch::ArgReader<RUNTIME_ARCH> & arg_reader) __attribute__((always_inline))
            ->std::optional<std::tuple<napi_value, napi_value, panda_file::ShortyIterator, uint32_t, uint32_t>>
    {
        ASSERT(methodd->IsStatic());

        panda_file::ShortyIterator it(methodd->GetShorty());
        uint32_t num_args = methodd->GetNumArgs() - 2;
        uint32_t ref_arg_idx = !(*it++).IsPrimitive() + 2;
        it.IncrementWithoutCheck();
        it.IncrementWithoutCheck();

        napi_value js_val = JSConvertJSValue::Wrap(env, arg_reader.Read<JSValue *>());
        auto qname_str = arg_reader.Read<coretypes::String *>();
        ASSERT(qname_str->ClassAddr<Class>()->IsStringClass());
        auto res = CompilerResolveQualifiedJSCall(env, js_val, qname_str);
        if (UNLIKELY(!res.has_value())) {
            ASSERT(NapiIsExceptionPending(env));
            return std::nullopt;
        }
        return std::make_tuple(res->first, res->second, it, num_args, ref_arg_idx);
    };
    return JSRuntimeJSCallImpl<IS_NEWCALL>(argsetup, method, args, in_stack_args);
}

extern "C" uint64_t JSRuntimeJSCall(Method *method, uint8_t *args, uint8_t *in_stack_args)
{
    return JSRuntimeJSCallBase</*IS_NEWCALL=*/false>(method, args, in_stack_args);
}
extern "C" void JSRuntimeJSCallBridge(Method *method, ...);

extern "C" uint64_t JSRuntimeJSNew(Method *method, uint8_t *args, uint8_t *in_stack_args)
{
    return JSRuntimeJSCallBase</*IS_NEWCALL=*/true>(method, args, in_stack_args);
}
extern "C" void JSRuntimeJSNewBridge(Method *method, ...);

extern "C" uint64_t JSRuntimeJSCallByValue(Method *method, uint8_t *args, uint8_t *in_stack_args)
{
    auto argsetup =
        []([[maybe_unused]] InteropCtx * ctx, napi_env env, Method * methodd,
           arch::ArgReader<RUNTIME_ARCH> & arg_reader) __attribute__((always_inline))
            ->std::optional<std::tuple<napi_value, napi_value, panda_file::ShortyIterator, uint32_t, uint32_t>>
    {
        ASSERT(methodd->IsStatic());

        panda_file::ShortyIterator it(methodd->GetShorty());
        uint32_t num_args = methodd->GetNumArgs() - 2;
        uint32_t ref_arg_idx = static_cast<uint32_t>(!(*it++).IsPrimitive()) + 2;
        it.IncrementWithoutCheck();
        it.IncrementWithoutCheck();

        napi_value js_fn = JSConvertJSValue::Wrap(env, arg_reader.Read<JSValue *>());
        napi_value js_this = JSConvertJSValue::Wrap(env, arg_reader.Read<JSValue *>());

        return std::make_tuple(js_this, js_fn, it, num_args, ref_arg_idx);
    };
    return JSRuntimeJSCallImpl</*IS_NEWCALL=*/false>(argsetup, method, args, in_stack_args);
}
extern "C" void JSRuntimeJSCallByValueBridge(Method *method, ...);

extern "C" uint64_t JSProxyCall(Method *method, uint8_t *args, uint8_t *in_stack_args)
{
    auto argsetup =
        [](InteropCtx * ctx, napi_env env, Method * methodd, arch::ArgReader<RUNTIME_ARCH> & arg_reader)
            __attribute__((always_inline))
                ->std::optional<std::tuple<napi_value, napi_value, panda_file::ShortyIterator, uint32_t, uint32_t>>
    {
        INTEROP_LOG(DEBUG) << "JSRuntimeJSCallImpl: JSProxy call: " << methodd->GetFullName(true);
        ASSERT(!methodd->IsStatic());

        panda_file::ShortyIterator it(methodd->GetShorty());
        uint32_t num_args = methodd->GetNumArgs() - 1;
        uint32_t ref_arg_idx = static_cast<uint32_t>(!(*it++).IsPrimitive());

        auto *ets_this = arg_reader.Read<EtsObject *>();
        Class *cls = ets_this->GetClass()->GetRuntimeClass();
        auto refconv = JSRefConvertResolve(ctx, cls);
        napi_value js_this = refconv->Wrap(ctx, ets_this);
        ASSERT(GetValueType(env, js_this) == napi_object);
        const char *method_name = utf::Mutf8AsCString(methodd->GetName().data);
        napi_value js_fn;
        napi_status rc = napi_get_named_property(env, js_this, method_name, &js_fn);
        if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
            ASSERT(NapiIsExceptionPending(env));
            return std::nullopt;
        }
        return std::make_tuple(js_this, js_fn, it, num_args, ref_arg_idx);
    };
    return JSRuntimeJSCallImpl</*IS_NEWCALL=*/false>(argsetup, method, args, in_stack_args);
}
extern "C" void JSProxyCallBridge(Method *method, ...);

template <bool IS_NEWCALL>
static void InitJSCallSignatures(coretypes::String *cls_str)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto class_linker = Runtime::GetCurrent()->GetClassLinker();

    std::string class_descriptor(utf::Mutf8AsCString(cls_str->GetDataMUtf8()));
    INTEROP_LOG(DEBUG) << "Intialize jscall signatures for " << class_descriptor;
    EtsClass *ets_class = coro->GetPandaVM()->GetClassLinker()->GetClass(class_descriptor.c_str());
    INTEROP_FATAL_IF(ets_class == nullptr);
    auto klass = ets_class->GetRuntimeClass();

    INTEROP_LOG(DEBUG) << "Bind bridge call methods for " << utf::Mutf8AsCString(klass->GetDescriptor());

    for (auto &method : klass->GetMethods()) {
        if (method.IsConstructor()) {
            continue;
        }
        ASSERT(method.IsStatic());
        auto pf = method.GetPandaFile();
        panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method.GetFileId()));
        pda.EnumerateTypes([](panda_file::Type /* unused */) {});  // preload reftypes span

        void *method_ep = nullptr;
        if constexpr (IS_NEWCALL) {
            method_ep = reinterpret_cast<void *>(JSRuntimeJSNewBridge);
        } else {
            uint32_t const arg_reftype_shift = method.GetReturnType().IsReference() ? 1 : 0;
            ASSERT(method.GetArgType(0).IsReference());  // arg0 is always a reference
            ASSERT(method.GetArgType(1).IsReference());  // arg1 is always a reference
            auto cls1 = class_linker->GetClass(*pf, pda.GetReferenceType(1 + arg_reftype_shift), ctx->LinkerCtx());
            if (cls1->IsStringClass()) {
                method_ep = reinterpret_cast<void *>(JSRuntimeJSCallBridge);
            } else {
                ASSERT(cls1 == ctx->GetJSValueClass());
                method_ep = reinterpret_cast<void *>(JSRuntimeJSCallByValueBridge);
            }
        }
        method.SetCompiledEntryPoint(method_ep);
        method.SetNativePointer(nullptr);
    }
}

uint8_t JSRuntimeInitJSCallClass(EtsString *cls_str)
{
    InitJSCallSignatures<false>(cls_str->GetCoreType());
    return 1;
}

uint8_t JSRuntimeInitJSNewClass(EtsString *cls_str)
{
    InitJSCallSignatures<true>(cls_str->GetCoreType());
    return 1;
}

}  // namespace panda::ets::interop::js
