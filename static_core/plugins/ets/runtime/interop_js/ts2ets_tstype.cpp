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

namespace ark::ets::interop::js {

template <bool IS_STATIC>
ALWAYS_INLINE inline uint64_t TSTypeCall(Method *method, uint8_t *args, uint8_t *inStackArgs);

extern "C" void TSTypeCallStaticBridge();
extern "C" uint64_t TSTypeCallStatic(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    return TSTypeCall<true>(method, args, inStackArgs);
}

extern "C" void TSTypeCallThisBridge();
extern "C" uint64_t TSTypeCallThis(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    return TSTypeCall<false>(method, args, inStackArgs);
}

extern "C" void TSTypeCallCtorBridge();
extern "C" uint64_t TSTypeCallCtor([[maybe_unused]] Method *method, [[maybe_unused]] uint8_t *args,
                                   [[maybe_unused]] uint8_t *inStackArgs)
{
    InteropCtx::Fatal("tstype ctor not implemented");
    return 0;
}
static void *GetTSTypeGetterBridge(Method *method);

static napi_ref TSModuleRequire(napi_env env, const std::string &name)
{
    // requires module to be placed in global object
    napi_value jsVal;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), name.c_str(), &jsVal));
    INTEROP_FATAL_IF(IsUndefined(env, jsVal));
    napi_ref jsRef;
    NAPI_CHECK_FATAL(napi_create_reference(env, jsVal, 1, &jsRef));
    return jsRef;
}

static napi_ref TSNapiModuleRequire(napi_env env, const std::string &name)
{
    INTEROP_LOG(INFO) << "TSNapiModuleRequire: " << name;
    napi_value requireFn;
    NAPI_CHECK_FATAL(napi_get_named_property(env, GetGlobal(env), "requireNapi", &requireFn));
    INTEROP_FATAL_IF(IsUndefined(env, requireFn));
    napi_value modObj;
    {
        napi_value jsName;
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &jsName));
        std::array<napi_value, 1> args = {jsName};
        NAPI_CHECK_FATAL(napi_call_function(env, nullptr, requireFn, args.size(), args.data(), &modObj));
    }
    INTEROP_FATAL_IF(IsUndefined(env, modObj));
    napi_ref jsRef;
    NAPI_CHECK_FATAL(napi_create_reference(env, modObj, 1, &jsRef));
    return jsRef;
}

struct TSTypeNamespace {
    static constexpr const char *TSTYPE_DATA_FIELD = "__tstype";
    static constexpr const char *TSTYPE_PREFIX_GETTER = "_get_";
    static constexpr const char *TSTYPE_PREFIX_SETTER = "_set_";

    static TSTypeNamespace *Create(ark::Class *klass, napi_ref toplevel);
    static TSTypeNamespace *Create(ark::Class *klass, TSTypeNamespace *upper);

    static inline TSTypeNamespace *FromBoundMethod(Method *method)
    {
        return reinterpret_cast<TSTypeNamespace *>(method->GetNativePointer());
    }

    static TSTypeNamespace *FromClass(Class *klass);

    napi_value ResolveValue(napi_env env);

private:
    TSTypeNamespace(ark::Class *klass, napi_ref toplevel, std::vector<const char *> &&resolv)
        : klass_(klass), toplevel_(toplevel), resolv_(resolv)
    {
    }

    void BindMethods();

    ark::Class *klass_ {};
    napi_ref toplevel_ {};
    std::vector<const char *> resolv_;
};

TSTypeNamespace *TSTypeNamespace::FromClass(Class *klass)
{
    auto klassObj = coretypes::Class::FromRuntimeClass(klass);
    auto field = klass->GetStaticFieldByName(utf::CStringAsMutf8(TSTypeNamespace::TSTYPE_DATA_FIELD));
    INTEROP_FATAL_IF(field == nullptr);
    auto data = reinterpret_cast<TSTypeNamespace *>(klassObj->GetFieldPrimitive<uint64_t>(*field));
    INTEROP_FATAL_IF(data == nullptr);
    return data;
}

TSTypeNamespace *TSTypeNamespace::Create(ark::Class *klass, napi_ref toplevel)
{
    auto ns = new TSTypeNamespace {klass, toplevel, {}};
    ns->BindMethods();
    return ns;
}

TSTypeNamespace *TSTypeNamespace::Create(ark::Class *klass, TSTypeNamespace *upper)
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
            auto methodName = method.GetName();
            auto hasPrefix = [&methodName](std::string_view const &pref) {
                return strncmp(pref.data(), utf::Mutf8AsCString(methodName.data), pref.size()) == 0;
            };
            if (hasPrefix(TSTYPE_PREFIX_GETTER)) {
                ep = GetTSTypeGetterBridge(&method);
            } else if (hasPrefix(TSTYPE_PREFIX_SETTER)) {
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
    napi_value curObj;
    NAPI_CHECK_FATAL(napi_get_reference_value(env, toplevel_, &curObj));
    INTEROP_LOG(INFO) << "get toplevel: " << klass_->GetName();
    INTEROP_FATAL_IF(IsUndefined(env, curObj));

    for (auto const &e : resolv_) {
        napi_value prop;
        NAPI_CHECK_FATAL(napi_get_named_property(env, curObj, e, &prop));
        if (IsUndefined(env, prop)) {
            INTEROP_LOG(INFO) << "lookup namespace failure: " << e;
            return nullptr;
        }
        INTEROP_LOG(INFO) << "lookup namespace success: " << e;
        curObj = prop;
    }
    return curObj;
}

static uint64_t TSTypeCCtor([[maybe_unused]] Method *method, coretypes::String *descriptor,
                            coretypes::String *enclosingDescriptor)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();
    auto etsClassLinker = coro->GetPandaVM()->GetClassLinker();
    auto tklass = ctx->LinkerCtx()->FindClass(descriptor->GetDataMUtf8());
    INTEROP_FATAL_IF(tklass == nullptr);

    INTEROP_LOG(INFO) << "TSTypeCCtor: " << tklass->GetName();

    TSTypeNamespace *ns;
    if (enclosingDescriptor != nullptr) {
        auto eklassName = utf::Mutf8AsCString(enclosingDescriptor->GetDataMUtf8());
        if (std::string("@ohos") == eklassName) {
            ns = TSTypeNamespace::Create(tklass, TSNapiModuleRequire(env, tklass->GetName()));
        } else {
            auto eklass = etsClassLinker->GetClass(eklassName)->GetRuntimeClass();
            INTEROP_FATAL_IF(eklass == nullptr);
            etsClassLinker->InitializeClass(coro, EtsClass::FromRuntimeClass(eklass));
            ns = TSTypeNamespace::Create(tklass, TSTypeNamespace::FromClass(eklass));
        }
    } else {
        ns = TSTypeNamespace::Create(tklass, TSModuleRequire(env, tklass->GetName()));
    }
    return reinterpret_cast<uint64_t>(ns);
}

template <bool IS_STATIC>
ALWAYS_INLINE inline uint64_t TSTypeCall(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    INTEROP_LOG(INFO) << "enter TSTypeCall";
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    ark::HandleScope<ark::ObjectHeader *> etsHandleScope(coro);

    Span<uint8_t> inGprArgs(args, arch::ExtArchTraits<RUNTIME_ARCH>::GP_ARG_NUM_BYTES);
    Span<uint8_t> inFprArgs(inGprArgs.end(), arch::ExtArchTraits<RUNTIME_ARCH>::FP_ARG_NUM_BYTES);
    arch::ArgReader<RUNTIME_ARCH> argReader(inGprArgs, inFprArgs, inStackArgs);

    // Skip method
    argReader.Read<Method *>();

    auto pf = method->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, method->GetFileId());
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
    auto classLinker = coro->GetPandaVM()->GetRuntime()->GetClassLinker();

    auto resolveRefCls = [&classLinker, &pf, &pda, &ctx](uint32_t idx) -> Class * {
        auto klass = classLinker->GetLoadedClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
        ASSERT(klass != nullptr);
        return klass;
    };

    INTEROP_FATAL_IF(method->IsStatic() != IS_STATIC);
    auto const numArgs = method->GetNumArgs() - (IS_STATIC ? 0 : 1);

    napi_value jsRet {};
    auto jsargs = ctx->GetTempArgs<napi_value>(numArgs);
    uint32_t refArgIdx = !method->GetReturnType().IsPrimitive();
    panda_file::ShortyIterator it(method->GetShorty());
    ++it;  // retval

    napi_value jsThis;
    if constexpr (IS_STATIC) {
        jsThis = TSTypeNamespace::FromBoundMethod(method)->ResolveValue(env);
        INTEROP_FATAL_IF(jsThis == nullptr);
    } else {
        auto value = argReader.Read<ObjectHeader *>();
        auto klass = method->GetClass();
        INTEROP_FATAL_IF(klass->GetBase() != ctx->GetJSValueClass());
        jsThis = JSConvertJSValue::Wrap(env, JSValue::FromEtsObject(EtsObject::FromCoreType(value)));
        ++it;  // this
    }

    for (uint32_t argIdx = 0; argIdx < numArgs; ++argIdx, ++it) {
        panda_file::Type type = *it;
        napi_value jsVal {};
        switch (type.GetId()) {
            case ark::panda_file::Type::TypeId::F64: {
                auto value = argReader.Read<double>();
                jsVal = JSConvertF64::Wrap(env, value);
                break;
            }
            case ark::panda_file::Type::TypeId::U1: {
                auto value = argReader.Read<bool>();
                jsVal = JSConvertU1::Wrap(env, value);
                break;
            }
            case ark::panda_file::Type::TypeId::REFERENCE: {
                auto value = argReader.Read<ObjectHeader *>();
                auto klass = resolveRefCls(refArgIdx++);
                if (klass->IsStringClass()) {
                    jsVal = JSConvertString::Wrap(env, EtsString::FromEtsObject(EtsObject::FromCoreType(value)));
                    break;
                }
                if (klass == ctx->GetJSValueClass() || klass->GetBase() == ctx->GetJSValueClass()) {
                    jsVal = JSConvertJSValue::Wrap(env, JSValue::FromEtsObject(EtsObject::FromCoreType(value)));
                    break;
                }
                InteropCtx::Fatal("unsupported reftype");
                break;
            }
            default:
                InteropCtx::Fatal("unsupported type");
        }
        jsargs[argIdx] = jsVal;
    }

    {
        napi_value jsFn;
        auto methodName = utf::Mutf8AsCString(method->GetName().data);
        INTEROP_LOG(INFO) << "get method " << methodName;
        NAPI_CHECK_FATAL(napi_get_named_property(env, jsThis, methodName, &jsFn));
        INTEROP_FATAL_IF(IsUndefined(env, jsFn));
        NAPI_CHECK_FATAL(napi_call_function(env, jsThis, jsFn, jsargs->size(), jsargs->data(), &jsRet));
    }

    ark::Value etsRet;
    panda_file::Type type = method->GetReturnType();
    switch (type.GetId()) {
        case ark::panda_file::Type::TypeId::VOID: {
            etsRet = ark::Value(uint64_t(0));
            break;
        }
        case ark::panda_file::Type::TypeId::F64: {
            auto res = JSConvertF64::Unwrap(ctx, env, jsRet);
            if (UNLIKELY(!res.has_value())) {
                InteropCtx::Fatal("unwrap failed");
            }
            etsRet = ark::Value(res.value());
            break;
        }
        case ark::panda_file::Type::TypeId::REFERENCE: {
            auto klass = resolveRefCls(0);
            if (klass->IsStringClass()) {
                auto res = JSConvertString::Unwrap(ctx, env, jsRet);
                if (UNLIKELY(!res.has_value())) {
                    InteropCtx::Fatal("unwrap failed");
                }
                etsRet = ark::Value(res.value()->GetCoreType());
                break;
            }
            if (klass == ctx->GetJSValueClass() || klass->GetBase() == ctx->GetJSValueClass()) {
                if (UNLIKELY(!klass->IsInitialized())) {
                    classLinker->InitializeClass(coro, klass);
                }
                JSValue *value = JSValue::CreateTSTypeDerived(klass, env, jsRet);
                etsRet = ark::Value(value->GetCoreType());
                break;
            }
            InteropCtx::Fatal("unsupported reftype");
            break;
        }
        default:
            InteropCtx::Fatal("unsupported type");
    }

    INTEROP_LOG(INFO) << "exit TSTypeCall";
    return static_cast<uint64_t>(etsRet.GetAsLong());
}

template <typename T>
static typename T::cpptype TSTypeGetter([[maybe_unused]] ark::Method *method, JSValue *jsvalue)
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
        case ark::panda_file::Type::TypeId::VOID: {
            InteropCtx::Fatal("void getter");
        }
        case ark::panda_file::Type::TypeId::F64: {
            return reinterpret_cast<void *>(TSTypeGetter<JSConvertF64>);
        }
        case ark::panda_file::Type::TypeId::REFERENCE: {
            auto pf = method->GetPandaFile();
            panda_file::MethodDataAccessor mda(*pf, method->GetFileId());
            panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
            auto classLinker = coro->GetPandaVM()->GetRuntime()->GetClassLinker();

            auto resolveRefCls = [&classLinker, &pf, &pda, &ctx](uint32_t idx) {
                auto klass = classLinker->GetLoadedClass(*pf, pda.GetReferenceType(idx), ctx->LinkerCtx());
                ASSERT(klass != nullptr);
                return klass;
            };
            auto klass = resolveRefCls(0);

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
    ark::ets::BindNative("LETSGLOBAL;", "__tstype_cctor", reinterpret_cast<void *>(TSTypeCCtor));
}

}  // namespace ark::ets::interop::js
