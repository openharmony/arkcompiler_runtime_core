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

#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "runtime/include/class_linker-inl.h"

namespace panda::ets::interop::js {

template <bool IS_STATIC>
ALWAYS_INLINE inline uint64_t TSTypeCall(Method *method, uint8_t *args, uint8_t *in_stack_args);

extern "C" void TSTypeCallStaticBridge();
extern "C" uint64_t TSTypeCallStatic(Method *method, uint8_t *args, uint8_t *in_stack_args)
{
    return TSTypeCall<true>(method, args, in_stack_args);
}

extern "C" void TSTypeCallThisBridge();
extern "C" uint64_t TSTypeCallThis(Method *method, uint8_t *args, uint8_t *in_stack_args)
{
    return TSTypeCall<false>(method, args, in_stack_args);
}

extern "C" void TSTypeCallCtorBridge();
extern "C" uint64_t TSTypeCallCtor([[maybe_unused]] Method *method, [[maybe_unused]] uint8_t *args,
                                   [[maybe_unused]] uint8_t *in_stack_args)
{
    InteropCtx::Fatal("tstype ctor not implemented");
    return 0;
}
static void *GetTSTypeGetterBridge(Method *method);

static napi_ref TSModuleRequire(napi_env env, const std::string &name)
{
    // requires module to be placed in global object
    napi_value js_val;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), name.c_str(), &js_val));
    INTEROP_FATAL_IF(IsUndefined(env, js_val));
    napi_ref js_ref;
    NAPI_CHECK_FATAL(napi_create_reference(env, js_val, 1, &js_ref));
    return js_ref;
}

static napi_ref TSNapiModuleRequire(napi_env env, const std::string &name)
{
    INTEROP_LOG(INFO) << "TSNapiModuleRequire: " << name;
    napi_value require_fn;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), "requireNapi", &require_fn));
    INTEROP_FATAL_IF(IsUndefined(env, require_fn));
    napi_value mod_obj;
    {
        napi_value js_name;
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &js_name));
        std::array<napi_value, 1> args = {js_name};
        NAPI_CHECK_FATAL(napi_call_function(env, nullptr, require_fn, args.size(), args.data(), &mod_obj));
    }
    INTEROP_FATAL_IF(IsUndefined(env, mod_obj));
    napi_ref js_ref;
    NAPI_CHECK_FATAL(napi_create_reference(env, mod_obj, 1, &js_ref));
    return js_ref;
}

struct TSTypeNamespace {
    static constexpr const char *TSTYPE_DATA_FIELD = "__tstype";
    static constexpr const char *TSTYPE_PREFIX_GETTER = "_get_";
    static constexpr const char *TSTYPE_PREFIX_SETTER = "_set_";

    static TSTypeNamespace *Create(panda::Class *klass, napi_ref toplevel);
    static TSTypeNamespace *Create(panda::Class *klass, TSTypeNamespace *upper);

    static inline TSTypeNamespace *FromBoundMethod(Method *method)
    {
        return reinterpret_cast<TSTypeNamespace *>(method->GetNativePointer());
    }

    static TSTypeNamespace *FromClass(Class *klass);

    napi_value ResolveValue(napi_env env);

private:
    TSTypeNamespace(panda::Class *klass, napi_ref toplevel, std::vector<const char *> &&resolv)
        : klass_(klass), toplevel_(toplevel), resolv_(resolv)
    {
    }

    void BindMethods();

    panda::Class *klass_ {};
    napi_ref toplevel_ {};
    std::vector<const char *> resolv_;
};

TSTypeNamespace *TSTypeNamespace::FromClass(Class *klass)
{
    auto klass_obj = coretypes::Class::FromRuntimeClass(klass);
    auto field = klass->GetStaticFieldByName(utf::CStringAsMutf8(TSTypeNamespace::TSTYPE_DATA_FIELD));
    INTEROP_FATAL_IF(field == nullptr);
    auto data = reinterpret_cast<TSTypeNamespace *>(klass_obj->GetFieldPrimitive<uint64_t>(*field));
    INTEROP_FATAL_IF(data == nullptr);
    return data;
}

TSTypeNamespace *TSTypeNamespace::Create(panda::Class *klass, napi_ref toplevel)
{
    auto ns = new TSTypeNamespace {klass, toplevel, {}};
    ns->BindMethods();
    return ns;
}

TSTypeNamespace *TSTypeNamespace::Create(panda::Class *klass, TSTypeNamespace *upper)
{
    std::vector<const char *> resolv;
    resolv.reserve(upper->resolv_.size() + 1);
    resolv = upper->resolv_;
    resolv.push_back(klass->GetName().c_str());
    auto ns = new TSTypeNamespace {klass, upper->toplevel_, std::move(resolv)};
    ns->BindMethods();
    return ns;
}

void TSTypeNamespace::BindMethods()
{
    INTEROP_LOG(INFO) << "bind methods: " << klass_->GetName();
    for (auto &method : klass_->GetMethods()) {
        void *ep = nullptr;
        if (method.IsInstanceConstructor()) {
            ep = reinterpret_cast<void *>(TSTypeCallCtorBridge);
        } else if (method.IsStaticConstructor()) {
            // do nothing
        } else if (method.IsStatic()) {
            ep = reinterpret_cast<void *>(TSTypeCallStaticBridge);
        } else {
            auto method_name = method.GetName();
            auto has_prefix = [&](std::string_view const &pref) {
                return strncmp(pref.data(), utf::Mutf8AsCString(method_name.data), pref.size()) == 0;
            };
            if (has_prefix(TSTYPE_PREFIX_GETTER)) {
                ep = GetTSTypeGetterBridge(&method);
            } else if (has_prefix(TSTYPE_PREFIX_SETTER)) {
                InteropCtx::Fatal("support setter");
            } else {
                ep = reinterpret_cast<void *>(TSTypeCallThisBridge);
            }
        }
        method.SetCompiledEntryPoint(ep);
        method.SetNativePointer(this);
    }
}

napi_value TSTypeNamespace::ResolveValue(napi_env env)
{
    napi_value cur_obj;
    NAPI_CHECK_FATAL(napi_get_reference_value(env, toplevel_, &cur_obj));
    INTEROP_LOG(INFO) << "get toplevel: " << klass_->GetName();
    INTEROP_FATAL_IF(IsUndefined(env, cur_obj));

    for (auto const &e : resolv_) {
        napi_value prop;
        NAPI_CHECK_FATAL(napi_get_named_property(env, cur_obj, e, &prop));
        if (IsUndefined(env, prop)) {
            INTEROP_LOG(INFO) << "lookup namespace failure: " << e;
            return nullptr;
        }
        INTEROP_LOG(INFO) << "lookup namespace success: " << e;
        cur_obj = prop;
    }
    return cur_obj;
}

static uint64_t TSTypeCCtor([[maybe_unused]] Method *method, coretypes::String *descriptor,
                            coretypes::String *enclosing_descriptor)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    auto ets_class_linker = coro->GetPandaVM()->GetClassLinker();
    auto tklass = ctx->LinkerCtx()->FindClass(descriptor->GetDataMUtf8());
    INTEROP_FATAL_IF(tklass == nullptr);

    INTEROP_LOG(INFO) << "TSTypeCCtor: " << tklass->GetName();

    TSTypeNamespace *ns;
    if (enclosing_descriptor != nullptr) {
        auto eklass_name = utf::Mutf8AsCString(enclosing_descriptor->GetDataMUtf8());
        if (std::string("@ohos") == eklass_name) {
            ns = TSTypeNamespace::Create(tklass, TSNapiModuleRequire(env, tklass->GetName()));
        } else {
            auto eklass = ets_class_linker->GetClass(eklass_name)->GetRuntimeClass();
            INTEROP_FATAL_IF(eklass == nullptr);
            ets_class_linker->InitializeClass(coro, EtsClass::FromRuntimeClass(eklass));
            ns = TSTypeNamespace::Create(tklass, TSTypeNamespace::FromClass(eklass));
        }
    } else {
        ns = TSTypeNamespace::Create(tklass, TSModuleRequire(env, tklass->GetName()));
    }
    return reinterpret_cast<uint64_t>(ns);
}

template <bool IS_STATIC>
ALWAYS_INLINE inline uint64_t TSTypeCall(Method *method, uint8_t *args, uint8_t *in_stack_args)
{
    INTEROP_LOG(INFO) << "enter TSTypeCall";
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    NapiScope js_handle_scope(env);

    panda::HandleScope<panda::ObjectHeader *> ets_handle_scope(coro);

    Span<uint8_t> in_gpr_args(args, arch::ExtArchTraits<RUNTIME_ARCH>::GP_ARG_NUM_BYTES);
    Span<uint8_t> in_fpr_args(in_gpr_args.end(), arch::ExtArchTraits<RUNTIME_ARCH>::FP_ARG_NUM_BYTES);
    arch::ArgReader<RUNTIME_ARCH> arg_reader(in_gpr_args, in_fpr_args, in_stack_args);

    // Skip method
    arg_reader.Read<Method *>();

    auto pf = method->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, method->GetFileId());
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
    auto class_linker = coro->GetPandaVM()->GetRuntime()->GetClassLinker();

    auto resolve_ref_cls = [&](uint32_t idx) -> Class * {
        auto klass = class_linker->GetLoadedClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
        ASSERT(klass != nullptr);
        return klass;
    };

    INTEROP_FATAL_IF(method->IsStatic() != IS_STATIC);
    auto const num_args = method->GetNumArgs() - (IS_STATIC ? 0 : 1);

    napi_value js_ret {};
    auto jsargs = ctx->GetTempArgs<napi_value>(num_args);
    uint32_t ref_arg_idx = !method->GetReturnType().IsPrimitive();
    panda_file::ShortyIterator it(method->GetShorty());
    ++it;  // retval

    napi_value js_this;
    if constexpr (IS_STATIC) {
        js_this = TSTypeNamespace::FromBoundMethod(method)->ResolveValue(env);
        INTEROP_FATAL_IF(js_this == nullptr);
    } else {
        auto value = arg_reader.Read<ObjectHeader *>();
        auto klass = method->GetClass();
        INTEROP_FATAL_IF(klass->GetBase() != ctx->GetJSValueClass());
        js_this = JSConvertJSValue::Wrap(env, JSValue::FromEtsObject(EtsObject::FromCoreType(value)));
        ++it;  // this
    }

    for (uint32_t arg_idx = 0; arg_idx < num_args; ++arg_idx, ++it) {
        panda_file::Type type = *it;
        napi_value js_val {};
        switch (type.GetId()) {
            case panda::panda_file::Type::TypeId::F64: {
                auto value = arg_reader.Read<double>();
                js_val = JSConvertF64::Wrap(env, value);
                break;
            }
            case panda::panda_file::Type::TypeId::U1: {
                auto value = arg_reader.Read<bool>();
                js_val = JSConvertU1::Wrap(env, value);
                break;
            }
            case panda::panda_file::Type::TypeId::REFERENCE: {
                auto value = arg_reader.Read<ObjectHeader *>();
                auto klass = resolve_ref_cls(ref_arg_idx++);
                if (klass->IsStringClass()) {
                    js_val = JSConvertString::Wrap(env, EtsString::FromEtsObject(EtsObject::FromCoreType(value)));
                    break;
                }
                if (klass == ctx->GetJSValueClass() || klass->GetBase() == ctx->GetJSValueClass()) {
                    js_val = JSConvertJSValue::Wrap(env, JSValue::FromEtsObject(EtsObject::FromCoreType(value)));
                    break;
                }
                InteropCtx::Fatal("unsupported reftype");
                break;
            }
            default:
                InteropCtx::Fatal("unsupported type");
        }
        jsargs[arg_idx] = js_val;
    }

    {
        napi_value js_fn;
        auto method_name = utf::Mutf8AsCString(method->GetName().data);
        INTEROP_LOG(INFO) << "get method " << method_name;
        NAPI_CHECK_FATAL(napi_get_named_property(env, js_this, method_name, &js_fn));
        INTEROP_FATAL_IF(IsUndefined(env, js_fn));
        NAPI_CHECK_FATAL(napi_call_function(env, js_this, js_fn, jsargs->size(), jsargs->data(), &js_ret));
    }

    panda::Value ets_ret;
    panda_file::Type type = method->GetReturnType();
    switch (type.GetId()) {
        case panda::panda_file::Type::TypeId::VOID: {
            ets_ret = panda::Value(uint64_t(0));
            break;
        }
        case panda::panda_file::Type::TypeId::F64: {
            auto res = JSConvertF64::Unwrap(ctx, env, js_ret);
            if (UNLIKELY(!res.has_value())) {
                InteropCtx::Fatal("unwrap failed");
            }
            ets_ret = panda::Value(res.value());
            break;
        }
        case panda::panda_file::Type::TypeId::REFERENCE: {
            auto klass = resolve_ref_cls(0);
            if (klass->IsStringClass()) {
                auto res = JSConvertString::Unwrap(ctx, env, js_ret);
                if (UNLIKELY(!res.has_value())) {
                    InteropCtx::Fatal("unwrap failed");
                }
                ets_ret = panda::Value(res.value()->GetCoreType());
                break;
            }
            if (klass == ctx->GetJSValueClass() || klass->GetBase() == ctx->GetJSValueClass()) {
                if (UNLIKELY(!klass->IsInitialized())) {
                    class_linker->InitializeClass(coro, klass);
                }
                JSValue *value = JSValue::CreateTSTypeDerived(klass, env, js_ret);
                ets_ret = panda::Value(value->GetCoreType());
                break;
            }
            InteropCtx::Fatal("unsupported reftype");
            break;
        }
        default:
            InteropCtx::Fatal("unsupported type");
    }

    INTEROP_LOG(INFO) << "exit TSTypeCall";
    return static_cast<uint64_t>(ets_ret.GetAsLong());
}

template <typename T>
static typename T::cpptype TSTypeGetter([[maybe_unused]] panda::Method *method, JSValue *jsvalue)
{
    auto ctx = InteropCtx::Current();
    constexpr auto METHOD_PREFIX_LEN = std::string_view(TSTypeNamespace::TSTYPE_PREFIX_GETTER).length();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto fname = utf::Mutf8AsCString(method->GetName().data + METHOD_PREFIX_LEN);
    auto res = JSValueGetByName<T>(ctx, jsvalue, fname);
    INTEROP_FATAL_IF(!res);
    return res.value();
}

static void *GetTSTypeGetterBridge(Method *method)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    panda_file::Type type = method->GetReturnType();
    switch (type.GetId()) {
        case panda::panda_file::Type::TypeId::VOID: {
            InteropCtx::Fatal("void getter");
        }
        case panda::panda_file::Type::TypeId::F64: {
            return reinterpret_cast<void *>(TSTypeGetter<JSConvertF64>);
        }
        case panda::panda_file::Type::TypeId::REFERENCE: {
            auto pf = method->GetPandaFile();
            panda_file::MethodDataAccessor mda(*pf, method->GetFileId());
            panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
            auto class_linker = coro->GetPandaVM()->GetRuntime()->GetClassLinker();

            auto resolve_ref_cls = [&](uint32_t idx) {
                auto klass = class_linker->GetLoadedClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
                ASSERT(klass != nullptr);
                return klass;
            };
            auto klass = resolve_ref_cls(0);

            if (klass->IsStringClass()) {
                return reinterpret_cast<void *>(TSTypeGetter<JSConvertString>);
            }
            InteropCtx::Fatal("unsupported reftype");
        }
        default:
            InteropCtx::Fatal("unsupported type");
    }
}

void InitTSTypeExports()
{
    panda::ets::BindNative("LETSGLOBAL;", "__tstype_cctor", reinterpret_cast<void *>(TSTypeCCtor));
}

}  // namespace panda::ets::interop::js
