/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <limits>
#include "ani.h"
#include "macros.h"
#include "libpandabase/utils/logger.h"
#include "runtime/coroutines/coroutine_scopes.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/ani/ani_type_info.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_module.h"
#include "plugins/ets/runtime/types/ets_namespace.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/ets_handle_scope.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_PTR_ARG(arg) ANI_CHECK_RETURN_IF_EQ(arg, nullptr, ANI_INVALID_ARGS)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_ENV(env)                                                                           \
    do {                                                                                         \
        ANI_CHECK_RETURN_IF_EQ(env, nullptr, ANI_INVALID_ARGS);                                  \
        bool hasPendingException = ::ark::ets::PandaEnv::FromAniEnv(env)->HasPendingException(); \
        ANI_CHECK_RETURN_IF_EQ(hasPendingException, true, ANI_PENDING_ERROR);                    \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

namespace ark::ets::ani {

template <typename T>
using ArgVector = PandaSmallVector<T>;

using TypeId = panda_file::Type::TypeId;

static inline bool IsUndefined(ani_ref ref)
{
    return ManagedCodeAccessor::IsUndefined(ref);
}

static inline ani_field ToAniField(EtsField *field)
{
    ASSERT(!field->IsStatic());
    return reinterpret_cast<ani_field>(field);
}

static inline EtsField *ToInternalField(ani_field field)
{
    auto *f = reinterpret_cast<EtsField *>(field);
    ASSERT(!f->IsStatic());
    return f;
}

static inline ani_static_field ToAniStaticField(EtsField *field)
{
    ASSERT(field->IsStatic());
    return reinterpret_cast<ani_static_field>(field);
}

[[maybe_unused]] static inline EtsField *ToInternalField(ani_static_field field)
{
    auto *f = reinterpret_cast<EtsField *>(field);
    ASSERT(f->IsStatic());
    return f;
}

static inline ani_variable ToAniVariable(EtsVariable *variable)
{
    return reinterpret_cast<ani_variable>(variable);
}

static inline EtsVariable *ToInternalVariable(ani_variable variable)
{
    return reinterpret_cast<EtsVariable *>(variable);
}

static inline EtsMethod *ToInternalMethod(ani_method method)
{
    auto *m = reinterpret_cast<EtsMethod *>(method);
    ASSERT(!m->IsStatic());
    ASSERT(!m->IsFunction());
    return m;
}

static inline ani_method ToAniMethod(EtsMethod *method)
{
    ASSERT(!method->IsStatic());
    ASSERT(!method->IsFunction());
    return reinterpret_cast<ani_method>(method);
}

static inline EtsMethod *ToInternalMethod(ani_static_method method)
{
    auto *m = reinterpret_cast<EtsMethod *>(method);
    ASSERT(m->IsStatic());
    ASSERT(!m->IsFunction());
    return m;
}

static inline ani_static_method ToAniStaticMethod(EtsMethod *method)
{
    ASSERT(method->IsStatic());
    ASSERT(!method->IsFunction());
    return reinterpret_cast<ani_static_method>(method);
}

static inline EtsMethod *ToInternalFunction(ani_function fn)
{
    auto *m = reinterpret_cast<EtsMethod *>(fn);
    ASSERT(m->IsStatic());
    // NODE: Add assert 'ASSERT(method->IsFunction())' when #22482 is resolved.
    return m;
}

static inline ani_function ToAniFunction(EtsMethod *method)
{
    ASSERT(method->IsStatic());
    // NODE: Add assert 'ASSERT(method->IsFunction())' when #22482 is resolved.
    return reinterpret_cast<ani_function>(method);
}

static void CheckStaticMethodReturnType(ani_static_method method, EtsType type)
{
    EtsMethod *m = ToInternalMethod(method);
    if (UNLIKELY(m->GetReturnValueType() != type)) {
        LOG(FATAL, ANI) << "Return type mismatch";
    }
}

static void CheckMethodReturnType(ani_method method, EtsType type)
{
    EtsMethod *m = ToInternalMethod(method);
    if (UNLIKELY(m->GetReturnValueType() != type)) {
        LOG(FATAL, ANI) << "Return type mismatch";
    }
}

static void CheckFunctionReturnType(ani_function fn, EtsType type)
{
    EtsMethod *m = ToInternalFunction(fn);
    if (UNLIKELY(m->GetReturnValueType() != type)) {
        LOG(FATAL, ANI) << "Return type mismatch";
    }
}

static ClassLinkerContext *GetClassLinkerContext(EtsCoroutine *coroutine)
{
    auto stack = StackWalker::Create(coroutine);
    if (!stack.HasFrame()) {
        return nullptr;
    }

    auto *method = EtsMethod::FromRuntimeMethod(stack.GetMethod());
    if (method != nullptr) {
        return method->GetClass()->GetLoadContext();
    }
    return nullptr;
}

static ani_status DoGetInternalClass(ScopedManagedCodeFix &s, EtsClass *klass, EtsClass **result)
{
    if (klass->IsInitialized()) {
        *result = klass;
        return ANI_OK;
    }

    // Initialize class
    EtsCoroutine *corutine = s.GetCoroutine();
    EtsClassLinker *classLinker = corutine->GetPandaVM()->GetClassLinker();
    bool isInitialized = classLinker->InitializeClass(corutine, klass);
    if (!isInitialized) {
        LOG(ERROR, ANI) << "Cannot initialize class: " << klass->GetDescriptor();
        if (corutine->HasPendingException()) {
            return ANI_PENDING_ERROR;
        }
        return ANI_ERROR;
    }
    *result = klass;
    return ANI_OK;
}

static ani_status GetInternalClass(ScopedManagedCodeFix &s, ani_class cls, EtsClass **result)
{
    EtsClass *klass = s.ToInternalType(cls);
    return DoGetInternalClass(s, klass, result);
}

static ani_status GetInternalNamespace(ScopedManagedCodeFix &s, ani_namespace ns, EtsNamespace **result)
{
    EtsNamespace *nsp = s.ToInternalType(ns);
    EtsClass *klass {};
    ani_status status = DoGetInternalClass(s, nsp->AsClass(), &klass);
    *result = EtsNamespace::FromClass(klass);
    return status;
}

static ani_status GetInternalModule(ScopedManagedCodeFix &s, ani_module module, EtsModule **result)
{
    EtsModule *etsModule = s.ToInternalType(module);
    EtsClass *klass {};
    ani_status status = DoGetInternalClass(s, etsModule->AsClass(), &klass);
    *result = EtsModule::FromClass(klass);
    return status;
}

static Value ConstructValueFromFloatingPoint(float val)
{
    return Value(bit_cast<int32_t>(val));
}

static Value ConstructValueFromFloatingPoint(double val)
{
    return Value(bit_cast<int64_t>(val));
}

static ArgVector<Value> GetArgValues(ScopedManagedCodeFix *s, EtsMethod *method, va_list args, ani_object object)
{
    ArgVector<Value> parsedArgs;
    parsedArgs.reserve(method->GetNumArgs());
    if (object != nullptr) {
        parsedArgs.emplace_back(s->ToInternalType(object)->GetCoreType());
    }

    panda_file::ShortyIterator it(method->GetPandaMethod()->GetShorty());
    panda_file::ShortyIterator end;
    ++it;  // skip the return value
    for (; it != end; ++it) {
        panda_file::Type type = *it;
        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
        switch (type.GetId()) {
            case TypeId::U1:
            case TypeId::U16:
                parsedArgs.emplace_back(va_arg(args, uint32_t));
                break;
            case TypeId::I8:
            case TypeId::I16:
            case TypeId::I32:
                parsedArgs.emplace_back(va_arg(args, int32_t));
                break;
            case TypeId::I64:
                parsedArgs.emplace_back(va_arg(args, int64_t));
                break;
            case TypeId::F32:
                parsedArgs.push_back(ConstructValueFromFloatingPoint(static_cast<float>(va_arg(args, double))));
                break;
            case TypeId::F64:
                parsedArgs.push_back(ConstructValueFromFloatingPoint(va_arg(args, double)));
                break;
            case TypeId::REFERENCE: {
                auto *param = s->ToInternalType(va_arg(args, ani_ref));
                parsedArgs.emplace_back(param != nullptr ? param->GetCoreType() : nullptr);
                break;
            }
            default:
                LOG(FATAL, ANI) << "Unexpected argument type";
                break;
        }
        // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    }
    return parsedArgs;
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
static ArgVector<Value> GetArgValues(ScopedManagedCodeFix *s, EtsMethod *method, const ani_value *args,
                                     ani_object object)
{
    ArgVector<Value> parsedArgs;
    parsedArgs.reserve(method->GetNumArgs());
    if (object != nullptr) {
        parsedArgs.emplace_back(s->ToInternalType(object)->GetCoreType());
    }

    panda_file::ShortyIterator it(method->GetPandaMethod()->GetShorty());
    panda_file::ShortyIterator end;
    ++it;  // skip the return value
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (const auto *arg = args; it != end; ++arg, ++it) {
        panda_file::Type type = *it;
        switch (type.GetId()) {
            case TypeId::U1:
                parsedArgs.emplace_back(arg->z);
                break;
            case TypeId::U16:
                parsedArgs.emplace_back(arg->c);
                break;
            case TypeId::I8:
                parsedArgs.emplace_back(arg->b);
                break;
            case TypeId::I16:
                parsedArgs.emplace_back(arg->s);
                break;
            case TypeId::I32:
                parsedArgs.emplace_back(arg->i);
                break;
            case TypeId::I64:
                parsedArgs.emplace_back(arg->l);
                break;
            case TypeId::F32:
                parsedArgs.push_back(ConstructValueFromFloatingPoint(arg->f));
                break;
            case TypeId::F64:
                parsedArgs.push_back(ConstructValueFromFloatingPoint(arg->d));
                break;
            case TypeId::REFERENCE: {
                auto *param = s->ToInternalType(arg->r);
                parsedArgs.emplace_back(param != nullptr ? param->GetCoreType() : nullptr);
                break;
            }
            default:
                LOG(FATAL, ANI) << "Unexpected argument type";
                break;
        }
    }
    return parsedArgs;
}

static inline EtsMethod *ResolveVirtualMethod(ScopedManagedCodeFix *s, ani_object object, ani_method method)
{
    EtsMethod *m = ToInternalMethod(method);
    if (UNLIKELY(m->IsStatic())) {
        LOG(FATAL, ANI) << "Called ResolveVirtualMethod of static method, invalid ANI usage";
        return m;
    }
    EtsObject *obj = s->ToInternalType(object);
    return obj->GetClass()->ResolveVirtualMethod(m);
}

template <typename EtsValueType, typename AniType, typename MethodType, typename Args>
static ani_status DoGeneralMethodCall(ScopedManagedCodeFix &s, ani_object obj, MethodType method, AniType *result,
                                      Args args)
{
    // Trigger coroutine manager native call events
    ScopedCoroutineNativeCall c(s.GetCoroutine());

    EtsMethod *m = nullptr;
    if constexpr (std::is_same_v<MethodType, ani_method>) {
        m = ResolveVirtualMethod(&s, obj, method);
    } else if constexpr (std::is_same_v<MethodType, ani_static_method>) {
        m = ToInternalMethod(method);
    } else if constexpr (std::is_same_v<MethodType, ani_function>) {
        m = ToInternalFunction(method);
    } else {
        static_assert(!(std::is_same_v<MethodType, ani_method> || std::is_same_v<MethodType, ani_static_method> ||
                        std::is_same_v<MethodType, ani_function>),
                      "Unreachable type");
    }
    ASSERT(m != nullptr);

    EtsValue res {};
    ArgVector<Value> values = GetArgValues(&s, m, args, obj);
    ani_status status = m->Invoke(s, values.data(), &res);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    // Now AniType and EtsValueType are the same, but later it could be changed
    static_assert(std::is_same_v<AniType, EtsValueType>);
    *result = res.GetAs<EtsValueType>();
    return ANI_OK;
}

template <typename EtsValueType, typename AniType, typename MethodType, typename Args>
static ani_status GeneralMethodCall(ani_env *env, ani_object obj, MethodType method, AniType *result, Args args)
{
    ScopedManagedCodeFix s(env);
    return DoGeneralMethodCall<EtsValueType, AniType, MethodType, Args>(s, obj, method, result, args);
}

template <typename EtsValueType, typename AniType, typename Args>
static ani_status GeneralFunctionCall(ani_env *env, ani_function fn, AniType *result, Args args)
{
    auto method = reinterpret_cast<ani_function>(fn);
    ScopedManagedCodeFix s(env);
    return DoGeneralMethodCall<EtsValueType, AniType, ani_function, Args>(s, nullptr, method, result, args);
}

template <typename T>
static inline ani_status GetPrimitiveTypeField(ani_env *env, ani_object object, ani_field field, T *result)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(field);
    CHECK_PTR_ARG(result);

    EtsField *etsField = ToInternalField(field);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<T>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    ScopedManagedCodeFix s(env);
    EtsObject *etsObject = s.ToInternalType(object);
    *result = etsObject->GetFieldPrimitive<T>(etsField);
    return ANI_OK;
}

template <typename T>
static inline ani_status SetPrimitiveTypeField(ani_env *env, ani_object object, ani_field field, T value)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(field);

    EtsField *etsField = ToInternalField(field);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<T>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    ScopedManagedCodeFix s(env);
    EtsObject *etsObject = s.ToInternalType(object);
    etsObject->SetFieldPrimitive(etsField, value);
    return ANI_OK;
}

template <typename T>
static ani_status ClassGetStaticField(ani_env *env, ani_class cls, ani_static_field field, T *result)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(field);
    CHECK_PTR_ARG(result);
    EtsField *etsField = ToInternalField(field);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<T>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
    ScopedManagedCodeFix s(env);
    EtsClass *etsClass = s.ToInternalType(cls);
    *result = etsClass->GetStaticFieldPrimitive<T>(etsField);
    return ANI_OK;
}

template <typename T>
static ani_status ClassSetStaticField(ani_env *env, ani_class cls, ani_static_field field, T value)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(field);
    EtsField *etsField = ToInternalField(field);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<T>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
    ScopedManagedCodeFix s(env);
    EtsClass *etsClass = s.ToInternalType(cls);
    etsClass->SetStaticFieldPrimitive(etsField, value);
    return ANI_OK;
}

template <typename T>
static ani_status ClassGetStaticFieldByName(ani_env *env, ani_class cls, const char *name, T *result)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);
    ScopedManagedCodeFix s(env);
    EtsClass *etsClass;
    ani_status status = GetInternalClass(s, cls, &etsClass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    EtsField *etsField = etsClass->GetStaticFieldIDByName(name, nullptr);
    ANI_CHECK_RETURN_IF_EQ(etsField, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<T>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
    *result = etsClass->GetStaticFieldPrimitive<T>(etsField);
    return ANI_OK;
}

template <typename T>
static ani_status ClassSetStaticFieldByName(ani_env *env, ani_class cls, const char *name, T value)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    ScopedManagedCodeFix s(env);
    EtsClass *etsClass;
    ani_status status = GetInternalClass(s, cls, &etsClass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    EtsField *etsField = etsClass->GetStaticFieldIDByName(name, nullptr);
    ANI_CHECK_RETURN_IF_EQ(etsField, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<T>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
    etsClass->SetStaticFieldPrimitive(etsField, value);
    return ANI_OK;
}

template <bool IS_MODULE, typename T>
static ani_status DoFind(PandaEnv *pandaEnv, const char *descriptor, ScopedManagedCodeFix &s, T *result)
{
    PandaString desc = Mangle::ConvertDescriptor(descriptor);
    EtsClassLinker *classLinker = pandaEnv->GetEtsVM()->GetClassLinker();
    EtsClass *klass = classLinker->GetClass(desc.c_str(), true, GetClassLinkerContext(s.GetCoroutine()));
    if (UNLIKELY(pandaEnv->HasPendingException())) {
        EtsThrowable *currentException = pandaEnv->GetThrowable();
        std::string_view exceptionString = currentException->GetClass()->GetDescriptor();
        if (exceptionString == panda_file_items::class_descriptors::LINKER_CLASS_NOT_FOUND_ERROR) {
            pandaEnv->ClearException();
            return ANI_NOT_FOUND;
        }

        // NOTE: Handle exception
        return ANI_PENDING_ERROR;
    }
    ANI_CHECK_RETURN_IF_EQ(klass, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(klass->IsModule(), IS_MODULE, ANI_NOT_FOUND);

    ASSERT_MANAGED_CODE();
    return s.AddLocalRef(reinterpret_cast<EtsObject *>(klass), reinterpret_cast<ani_ref *>(result));
}

template <bool IS_MODULE, typename T>
static ani_status DoFind(ani_env *env, const char *descriptor, T *result)
{
    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    return DoFind<IS_MODULE>(pandaEnv, descriptor, s, result);
}

template <bool IS_STATIC>
static bool CheckUniqueMethod(EtsClass *klass, const char *name)
{
    size_t nameCounter = 0;
    for (auto &method : klass->GetMethods()) {
        if (method->IsStatic() == IS_STATIC && ::strcmp(method->GetName(), name) == 0) {
            if (++nameCounter == 2U) {
                return false;
            }
        }
    }
    return true;
}

template <bool IS_STATIC_METHOD>
static ani_status DoGetClassMethod(EtsClass *klass, const char *name, const char *signature, EtsMethod **result)
{
    ASSERT_MANAGED_CODE();
    if (signature == nullptr && !CheckUniqueMethod<IS_STATIC_METHOD>(klass, name)) {
        return ANI_AMBIGUOUS;
    }
    EtsMethod *method = [&]() {
        if constexpr (IS_STATIC_METHOD) {
            return klass->GetStaticMethod(name, signature, true);
        } else {
            return klass->GetInstanceMethod(name, signature, true);
        }
    }();
    if (method == nullptr) {
        return ANI_NOT_FOUND;
    }
    *result = method;
    return ANI_OK;
}

template <bool IS_STATIC_METHOD>
static ani_status GetClassMethod(ani_env *env, ani_class cls, const char *name, const char *signature,
                                 EtsMethod **result)
{
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsClass *klass;
    ani_status status = GetInternalClass(s, cls, &klass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    return DoGetClassMethod<IS_STATIC_METHOD>(klass, name, signature, result);
}

template <typename ReturnType, EtsType EXPECT_TYPE, typename Args>
// CC-OFFNXT(G.FUN.01-CPP) solid logic
static ani_status ObjectCallMethodByName(ani_env *env, ani_object object, const char *name, const char *signature,
                                         ReturnType *result, Args args)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(name);

    ScopedManagedCodeFix s(env);
    EtsObject *etsObject = s.ToInternalType(object);
    ASSERT(etsObject != nullptr);
    EtsClass *cls = etsObject->GetClass();
    EtsMethod *method = nullptr;
    ani_status status = DoGetClassMethod<false>(cls, name, signature, &method);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    ASSERT(method != nullptr);
    ani_method aniMethod = ToAniMethod(method);
    CheckMethodReturnType(aniMethod, EXPECT_TYPE);
    return DoGeneralMethodCall<ReturnType>(s, object, aniMethod, result, args);
}

static ani_status GetNamespaceFunction(ani_env *env, ani_namespace ns, const char *name, const char *signature,
                                       EtsMethod **result)
{
    ScopedManagedCodeFix s(env);
    EtsNamespace *etsNs;
    ani_status status = GetInternalNamespace(s, ns, &etsNs);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    return DoGetClassMethod<true>(etsNs->AsClass(), name, signature, result);
}

static ani_status GetModuleFunction(ani_env *env, ani_module ns, const char *name, const char *signature,
                                    EtsMethod **result)
{
    ScopedManagedCodeFix s(env);
    EtsModule *etsModule;
    ani_status status = GetInternalModule(s, ns, &etsModule);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    return DoGetClassMethod<true>(etsModule->AsClass(), name, signature, result);
}

template <typename ReturnType, EtsType EXPECT_TYPE, typename Args>
// CC-OFFNXT(G.FUN.01-CPP) solid logic
static ani_status ClassCallMethodByName(ani_env *env, ani_class cls, const char *name, const char *signature,
                                        ReturnType *result, Args args)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    EtsMethod *method = nullptr;
    ani_status status = GetClassMethod<true>(env, cls, name, signature, &method);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    ASSERT(method != nullptr);
    ani_static_method staticMethod = ToAniStaticMethod(method);
    CheckStaticMethodReturnType(staticMethod, EXPECT_TYPE);
    return GeneralMethodCall<ReturnType>(env, nullptr, staticMethod, result, args);
}

template <bool IS_MODULE, typename T>
static ani_status FindInModule(ani_env *env, ani_module module, const char *targetDescriptor, T *result)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(module);
    CHECK_PTR_ARG(targetDescriptor);
    CHECK_PTR_ARG(result);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    EtsModule *etsModule {};
    ani_status status = GetInternalModule(s, module, &etsModule);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    PandaString descriptor;
    ANI_CHECK_RETURN_IF_NE(etsModule->GetModulePrefix(descriptor), ANI_OK, ANI_INVALID_DESCRIPTOR);
    PandaString className = Mangle::ConvertDescriptor(targetDescriptor);
    ANI_CHECK_RETURN_IF_LE(className.length(), 2U, ANI_INVALID_ARGS);
    ANI_CHECK_RETURN_IF_NE(className[0], 'L', ANI_INVALID_ARGS);

    className[0] = '/';
    descriptor += className;

    return DoFind<IS_MODULE>(pandaEnv, descriptor.c_str(), s, result);
}

NO_UB_SANITIZE static ani_status GetVersion(ani_env *env, uint32_t *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    *result = ANI_VERSION_1;
    return ANI_OK;
}

ani_status GetVM(ani_env *env, ani_vm **result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    *result = PandaEnv::FromAniEnv(env)->GetEtsVM();
    return ANI_OK;
}

static ani_status AllocObject(ScopedManagedCodeFix &s, ani_class cls, ani_object *result)
{
    EtsClass *klass;
    ani_status status = GetInternalClass(s, cls, &klass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    ANI_CHECK_RETURN_IF_EQ(klass->IsAbstract(), true, ANI_INVALID_TYPE);
    ANI_CHECK_RETURN_IF_EQ(klass->IsInterface(), true, ANI_INVALID_TYPE);

    // NODE: Check than we have the ability to create String/FiexedArray in this API, #22280
    ANI_CHECK_RETURN_IF_EQ(klass->IsStringClass(), true, ANI_INVALID_TYPE);
    ANI_CHECK_RETURN_IF_EQ(klass->IsArrayClass(), true, ANI_INVALID_TYPE);

    EtsObject *obj = EtsObject::Create(klass);
    ANI_CHECK_RETURN_IF_EQ(obj, nullptr, ANI_OUT_OF_MEMORY);
    return s.AddLocalRef(obj, reinterpret_cast<ani_ref *>(result));
}

template <typename Args>
static ani_status DoNewObject(ani_env *env, ani_class cls, ani_method method, ani_object *result, Args args)
{
    ani_object object;
    ScopedManagedCodeFix s(env);
    ani_status status = AllocObject(s, cls, &object);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    // Use any primitive type as template parameter and just ignore the result
    ani_int tmp;
    status = DoGeneralMethodCall<EtsInt>(s, object, method, &tmp, args);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    *result = object;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_New_A(ani_env *env, ani_class cls, ani_method method, ani_object *result,
                                              const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    return DoNewObject(env, cls, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_New_V(ani_env *env, ani_class cls, ani_method method, ani_object *result,
                                              va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    return DoNewObject(env, cls, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetType(ani_env *env, ani_object object, ani_type *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsObject *etsObject = s.ToInternalType(object);
    ASSERT(etsObject != nullptr);
    EtsClass *etsClass = etsObject->GetClass();
    ASSERT(etsClass != nullptr);

    s.AddLocalRef(etsClass->AsObject(), reinterpret_cast<ani_ref *>(result));
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_New(ani_env *env, ani_class cls, ani_method method, ani_object *result, ...)
{
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_New_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
ani_status Type_GetSuperClass(ani_env *env, ani_type type, ani_class *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(type);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsClass *cls = s.ToInternalType(type);
    ani_status status = DoGetInternalClass(s, cls, &cls);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    EtsClass *superCls = cls->GetSuperClass();
    if (superCls != nullptr) {
        ASSERT(superCls->IsClass());
        return s.AddLocalRef(superCls->AsObject(), reinterpret_cast<ani_ref *>(result));
    }
    *result = nullptr;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
ani_status Type_IsAssignableFrom(ani_env *env, ani_type fromType, ani_type toType, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fromType);
    CHECK_PTR_ARG(toType);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsClass *toClass = s.ToInternalType(toType);
    ani_status status = DoGetInternalClass(s, toClass, &toClass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    EtsClass *fromClass = s.ToInternalType(fromType);
    status = DoGetInternalClass(s, fromClass, &fromClass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    *result = toClass->IsAssignableFrom(fromClass) ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status FindModule(ani_env *env, const char *moduleDescriptor, ani_module *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(moduleDescriptor);
    CHECK_PTR_ARG(result);

    PandaString desc = Mangle::ConvertDescriptor(moduleDescriptor);
    ANI_CHECK_RETURN_IF_LE(desc.size(), 2U, ANI_INVALID_DESCRIPTOR);
    ANI_CHECK_RETURN_IF_NE(desc.back(), ';', ANI_INVALID_DESCRIPTOR);
    PandaString descriptor(desc.data(), desc.size() - 1);
    descriptor += "/ETSGLOBAL;";

    // NOTE: Check that results is namespace, #22400
    return DoFind<true>(env, descriptor.c_str(), result);
}

NO_UB_SANITIZE static ani_status FindNamespace(ani_env *env, const char *namespaceDescriptor, ani_namespace *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(namespaceDescriptor);
    CHECK_PTR_ARG(result);

    // NOTE: Check that results is namespace, #22400
    return DoFind<true>(env, namespaceDescriptor, result);
}

NO_UB_SANITIZE static ani_status FindClass(ani_env *env, const char *classDescriptor, ani_class *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(classDescriptor);
    CHECK_PTR_ARG(result);

    // NOTE: Check that results is class, #22400
    return DoFind<false>(env, classDescriptor, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_FindFunction(ani_env *env, ani_namespace ns, const char *name,
                                                        const char *signature, ani_function *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(ns);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    EtsMethod *method = nullptr;
    ani_status status = GetNamespaceFunction(env, ns, name, signature, &method);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    *result = ToAniFunction(method);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_FindNamespace(ani_env *env, ani_namespace ns,
                                                         const char *namespaceDescriptor, ani_namespace *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(ns);
    CHECK_PTR_ARG(namespaceDescriptor);
    CHECK_PTR_ARG(result);

    auto pandaEnv = PandaEtsNapiEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    EtsNamespace *etsNs {};
    ani_status status = GetInternalNamespace(s, ns, &etsNs);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    std::string_view nsDescriptor = etsNs->AsClass()->GetDescriptor();
    ASSERT(nsDescriptor.size() > 2U);

    PandaString nameDescriptor = Mangle::ConvertDescriptor(namespaceDescriptor);
    if (nameDescriptor[0] == 'L') {
        nameDescriptor[0] = '/';
    } else {
        return ANI_NOT_FOUND;
    }

    PandaString descriptor(nsDescriptor.data(), nsDescriptor.size() - 1);
    descriptor += nameDescriptor;

    // NOTE: Check that results is namespace, #22400
    return DoFind<true>(pandaEnv, descriptor.c_str(), s, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_FindClass(ani_env *env, ani_namespace ns, const char *classDescriptor,
                                                     ani_class *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(ns);
    CHECK_PTR_ARG(classDescriptor);
    CHECK_PTR_ARG(result);

    auto pandaEnv = PandaEtsNapiEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    EtsNamespace *etsNs {};
    ani_status status = GetInternalNamespace(s, ns, &etsNs);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    std::string_view nsDescriptor = etsNs->AsClass()->GetDescriptor();
    ASSERT(nsDescriptor.size() > 2U);

    PandaString clDescriptor = Mangle::ConvertDescriptor(classDescriptor);
    if (clDescriptor[0] == 'L') {
        clDescriptor[0] = '/';
    } else {
        return ANI_NOT_FOUND;
    }

    PandaString descriptor(nsDescriptor.data(), nsDescriptor.size() - 1);
    descriptor += clDescriptor;

    // NOTE: Check that results is class, #22400
    return DoFind<false>(pandaEnv, descriptor.c_str(), s, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_FindVariable(ani_env *env, ani_namespace ns, const char *variableDescriptor,
                                                        ani_variable *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(ns);
    CHECK_PTR_ARG(variableDescriptor);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsNamespace *etsNs {};
    ani_status status = GetInternalNamespace(s, ns, &etsNs);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    EtsVariable *variable = etsNs->GetVariabe(variableDescriptor);
    ANI_CHECK_RETURN_IF_EQ(variable, nullptr, ANI_NOT_FOUND);

    *result = ToAniVariable(variable);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_FindFunction(ani_env *env, ani_module module, const char *name,
                                                     const char *signature, ani_function *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(module);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    EtsMethod *method = nullptr;
    ani_status status = GetModuleFunction(env, module, name, signature, &method);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    *result = ToAniFunction(method);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_FindVariable(ani_env *env, ani_module module, const char *variableDescriptor,
                                                     ani_variable *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(module);
    CHECK_PTR_ARG(variableDescriptor);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsModule *etsModule {};
    ani_status status = GetInternalModule(s, module, &etsModule);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    EtsVariable *variable = etsModule->GetVariabe(variableDescriptor);
    ANI_CHECK_RETURN_IF_EQ(variable, nullptr, ANI_NOT_FOUND);

    *result = ToAniVariable(variable);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_FindClass(ani_env *env, ani_module module, const char *classDescriptor,
                                                  ani_class *result)
{
    ANI_DEBUG_TRACE(env);
    return FindInModule<false>(env, module, classDescriptor, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_FindNamespace(ani_env *env, ani_module module, const char *namespaceDescriptor,
                                                      ani_namespace *result)
{
    ANI_DEBUG_TRACE(env);
    return FindInModule<true>(env, module, namespaceDescriptor, result);
}

template <typename InternalType, typename AniFixedArrayType>
static ani_status NewPrimitiveTypeArray(ani_env *env, ani_size length, AniFixedArrayType *result)
{
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    // EtsArray
    auto *array = InternalType::Create(length);
    ANI_CHECK_RETURN_IF_EQ(array, nullptr, ANI_OUT_OF_MEMORY);
    return s.AddLocalRef(reinterpret_cast<EtsObject *>(array), reinterpret_cast<ani_ref *>(result));
}

template <typename T>
static ani_status GetArrayRegion(EtsArrayObject<EtsBoxPrimitive<T>> *objectArray, size_t start, size_t offset, T *buff)
{
    ASSERT(buff != nullptr);
    ANI_CHECK_RETURN_IF_GT(offset, std::numeric_limits<size_t>::max() - start, ANI_OUT_OF_RANGE);
    size_t end = start + offset;
    ANI_CHECK_RETURN_IF_GT(end, objectArray->GetActualLength(), ANI_OUT_OF_RANGE);

    Class *boxPrimitiveClass = EtsBoxPrimitive<T>::GetBoxClass(EtsCoroutine::GetCurrent());
    for (size_t posArr = start, posBuff = 0; posArr < end; ++posArr, ++posBuff) {
        EtsBoxPrimitive<T> *boxedVal = nullptr;
        [[maybe_unused]] auto getRes = objectArray->GetRef(posArr, &boxedVal);
        ASSERT(getRes);
        ANI_CHECK_RETURN_IF_EQ(boxedVal, nullptr, ANI_ERROR);
        if (boxPrimitiveClass != boxedVal->GetClass()->GetRuntimeClass()) {
            return ANI_INVALID_TYPE;
        }

        auto primitiveValue = boxedVal->GetValue();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        buff[posBuff] = primitiveValue;
    }
    return ANI_OK;
}

template <typename T>
static ani_status SetArrayRegion(ScopedManagedCodeFix &s,
                                 EtsArrayObject<EtsBoxPrimitive<std::remove_const_t<T>>> *objectArray, size_t start,
                                 size_t offset, T *buff)
{
    ASSERT(buff != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();

    ANI_CHECK_RETURN_IF_GT(offset, std::numeric_limits<size_t>::max() - start, ANI_OUT_OF_RANGE);
    size_t end = start + offset;
    ANI_CHECK_RETURN_IF_GT(end, objectArray->GetActualLength(), ANI_OUT_OF_RANGE);

    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle objectArrayHandle(coroutine, objectArray);
    for (size_t posArr = start, posBuff = 0; posArr < end; ++posArr, ++posBuff) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        T value = buff[posBuff];
        auto boxedValue = EtsBoxPrimitive<std::remove_const_t<T>>::Create(coro, value);
        ANI_CHECK_RETURN_IF_EQ(boxedValue, nullptr, ANI_OUT_OF_MEMORY);
        [[maybe_unused]] auto setRes = objectArrayHandle->SetRef(posArr, boxedValue);
        ASSERT(setRes);
    }
    return ANI_OK;
}

template <typename T, typename ArrayType>
static ani_status GetPrimitiveTypeArrayRegion(ani_env *env, ArrayType array, ani_size start, ani_size len, T *buf)
{
    ASSERT(array != nullptr);
    ANI_CHECK_RETURN_IF_EQ(len != 0 && buf == nullptr, true, ANI_INVALID_ARGS);

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsObject *objArray = s.ToInternalType(static_cast<ani_object>(array));
    if (!objArray->IsArrayClass()) {
        EtsArrayObject<EtsBoxPrimitive<T>> *escompatArray = EtsArrayObject<EtsBoxPrimitive<T>>::FromEtsObject(objArray);
        auto length = escompatArray->GetActualLength();
        if (UNLIKELY(start > length || len > (length - start))) {
            return ANI_OUT_OF_RANGE;
        }
        return GetArrayRegion(escompatArray, start, len, buf);
    }

    EtsArray *internalArray = EtsArray::FromEtsObject(objArray);
    auto length = internalArray->GetLength();
    if (UNLIKELY(start > length || len > (length - start))) {
        return ANI_OUT_OF_RANGE;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto res = memcpy_s(buf, len * sizeof(T), internalArray->GetData<T>() + start, len * sizeof(T));
    if (res != 0) {
        UNREACHABLE();
    }
    return ANI_OK;
}

template <typename T, typename ArrayType>
static ani_status SetPrimitiveTypeArrayRegion(ani_env *env, ArrayType array, ani_size start, ani_size len, T *buf)
{
    ASSERT(array != nullptr);
    ANI_CHECK_RETURN_IF_EQ(len != 0 && buf == nullptr, true, ANI_INVALID_ARGS);
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsObject *objArray = s.ToInternalType(static_cast<ani_object>(array));
    if (!objArray->IsArrayClass()) {
        EtsArrayObject<EtsBoxPrimitive<std::remove_const_t<T>>> *escompatArray =
            EtsArrayObject<EtsBoxPrimitive<std::remove_const_t<T>>>::FromEtsObject(objArray);
        auto length = escompatArray->GetActualLength();
        if (UNLIKELY(start > length || len > (length - start))) {
            return ANI_OUT_OF_RANGE;
        }
        return SetArrayRegion(s, escompatArray, start, len, buf);
    }

    EtsArray *internalArray = EtsArray::FromEtsObject(objArray);
    auto length = internalArray->GetLength();
    if (UNLIKELY(start > length || len > (length - start))) {
        return ANI_OUT_OF_RANGE;
    }
    auto data = internalArray->GetData<std::remove_const_t<T>>();
    auto dataLen = len * sizeof(T);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto res = memcpy_s(data + start, dataLen, buf, dataLen);
    if (res != 0) {
        UNREACHABLE();
    }
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetLength(ani_env *env, ani_array array, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsObject *objArray = s.ToInternalType(static_cast<ani_object>(array));
    if (!objArray->IsArrayClass()) {
        auto escompatArray = EtsArrayObject<EtsObject>::FromEtsObject(objArray);
        *result = escompatArray->GetActualLength();
    } else {
        auto etsArray = reinterpret_cast<EtsArray *>(objArray);
        *result = etsArray->GetLength();
    }

    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Boolean(ani_env *env, ani_size length, ani_array_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsBooleanArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Char(ani_env *env, ani_size length, ani_array_char *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsCharArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Byte(ani_env *env, ani_size length, ani_array_byte *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsByteArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Short(ani_env *env, ani_size length, ani_array_short *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsShortArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Int(ani_env *env, ani_size length, ani_array_int *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsIntArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Long(ani_env *env, ani_size length, ani_array_long *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsLongArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Float(ani_env *env, ani_size length, ani_array_float *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsFloatArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Double(ani_env *env, ani_size length, ani_array_double *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsDoubleArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetRegion_Boolean(ani_env *env, ani_array_boolean array, ani_size offset,
                                                         ani_size length, ani_boolean *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetRegion_Char(ani_env *env, ani_array_char array, ani_size offset,
                                                      ani_size length, ani_char *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetRegion_Byte(ani_env *env, ani_array_byte array, ani_size offset,
                                                      ani_size length, ani_byte *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New_Ref(ani_env *env, ani_type type, ani_size length, ani_ref initialElement,
                                               ani_array_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    ANI_CHECK_RETURN_IF_GT(length, std::numeric_limits<uint32_t>::max(), ANI_INVALID_ARGS);
    CHECK_PTR_ARG(type);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsClass *internalClass = s.ToInternalType(type);
    EtsObjectArray *internalArray = EtsObjectArray::Create(internalClass, static_cast<uint32_t>(length));
    ANI_CHECK_RETURN_IF_EQ(internalArray, nullptr, ANI_OUT_OF_MEMORY);
    if (initialElement != nullptr) {
        EtsObject *obj = s.ToInternalType(initialElement);
        for (ani_size i = 0; i < length; i++) {
            internalArray->Set(static_cast<uint32_t>(i), obj);
        }
    }
    return s.AddLocalRef(reinterpret_cast<EtsObject *>(internalArray), reinterpret_cast<ani_ref *>(result));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_Set_Ref(ani_env *env, ani_array_ref array, ani_size index, ani_ref ref)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);

    ScopedManagedCodeFix s(env);
    EtsObject *objArray = s.ToInternalType(reinterpret_cast<ani_object>(array));
    EtsObject *obj = s.ToInternalType(ref);
    if (!objArray->IsArrayClass()) {
        EtsArrayObject<EtsObject> *escompatArray = EtsArrayObject<EtsObject>::FromEtsObject(objArray);
        const auto length = escompatArray->GetActualLength();
        ANI_CHECK_RETURN_IF_GE(index, length, ANI_OUT_OF_RANGE);
        if (!escompatArray->SetRef(index, obj)) {
            return ANI_ERROR;
        }
    } else {
        EtsObjectArray *internalArray = EtsObjectArray::FromEtsObject(objArray);
        const auto length = internalArray->GetLength();
        ANI_CHECK_RETURN_IF_GE(index, length, ANI_OUT_OF_RANGE);

        if (obj != nullptr) {
            auto componentClass = internalArray->GetClass()->GetComponentType();
            if (!obj->IsInstanceOf(componentClass)) {
                return ANI_INVALID_TYPE;
            }
        }
        internalArray->Set(static_cast<uint32_t>(index), obj);
    }
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_Get_Ref(ani_env *env, ani_array_ref array, ani_size index, ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsObject *objArray = s.ToInternalType(reinterpret_cast<ani_object>(array));
    EtsObject *obj = nullptr;
    if (!objArray->IsArrayClass()) {
        EtsArrayObject<EtsObject> *escompatArray = EtsArrayObject<EtsObject>::FromEtsObject(objArray);
        const auto length = escompatArray->GetActualLength();
        ANI_CHECK_RETURN_IF_GE(index, length, ANI_OUT_OF_RANGE);
        if (!escompatArray->GetRef(index, &obj)) {
            return ANI_ERROR;
        }
    } else {
        EtsObjectArray *internalArray = EtsObjectArray::FromEtsObject(objArray);
        const auto length = internalArray->GetLength();
        ANI_CHECK_RETURN_IF_GE(index, length, ANI_OUT_OF_RANGE);

        obj = internalArray->Get(static_cast<uint32_t>(index));
    }
    return s.AddLocalRef(obj, result);
}

template <bool IS_FUNCTION>
static ani_status DoBindNative(ScopedManagedCodeFix &s, const PandaVector<EtsMethod *> &etsMethods,
                               const ani_native_function *functions, ani_size nrFunctions)
{
    ASSERT(etsMethods.size() == nrFunctions);

    // Get global mutex to guarantee that checked native methods don't changed in other thread.
    os::memory::LockHolder lock(s.GetPandaEnv()->GetEtsVM()->GetAniBindMutex());

    // Check native methods
    for (EtsMethod *method : etsMethods) {
        ANI_CHECK_RETURN_IF_EQ(method, nullptr, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_EQ(method->IsNative(), false, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_EQ(method->IsBindedNativeFunction(), true, ANI_ALREADY_BINDED);
    }

    // Bind native methods
    for (ani_size i = 0; i < nrFunctions; ++i) {
        EtsMethod *method = etsMethods[i];       // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const void *ptr = functions[i].pointer;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        bool ret = [method, ptr]() {
            if constexpr (IS_FUNCTION) {
                return method->RegisterNativeFunction(ptr);
            } else {
                return method->RegisterNativeMethod(ptr);
            }
        }();
        LOG_IF(!ret, FATAL, ANI) << "Cannot register native method";
    }
    return ANI_OK;
}

static ani_status DoBindNativeFunctions(ani_env *env, ani_namespace ns, const ani_native_function *functions,
                                        ani_size nrFunctions)
{
    ANI_CHECK_RETURN_IF_EQ(nrFunctions, 0, ANI_OK);
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsNamespace *etsNs = s.ToInternalType(ns);
    PandaVector<EtsMethod *> etsMethods;
    etsMethods.reserve(nrFunctions);
    for (ani_size i = 0; i < nrFunctions; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        etsMethods.push_back(etsNs->GetFunction(functions[i].name, functions[i].signature));
    }
    return DoBindNative<true>(s, etsMethods, functions, nrFunctions);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_BindNativeFunctions(ani_env *env, ani_module module,
                                                            const ani_native_function *functions, ani_size nrFunctions)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(module);
    CHECK_PTR_ARG(functions);

    return DoBindNativeFunctions(env, reinterpret_cast<ani_namespace>(module), functions, nrFunctions);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_BindNativeFunctions(ani_env *env, ani_namespace ns,
                                                               const ani_native_function *functions,
                                                               ani_size nrFunctions)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(ns);
    CHECK_PTR_ARG(functions);

    return DoBindNativeFunctions(env, ns, functions, nrFunctions);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetRegion_Short(ani_env *env, ani_array_short array, ani_size offset,
                                                       ani_size length, ani_short *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetRegion_Int(ani_env *env, ani_array_int array, ani_size offset,
                                                     ani_size length, ani_int *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetRegion_Long(ani_env *env, ani_array_long array, ani_size offset,
                                                      ani_size length, ani_long *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetRegion_Float(ani_env *env, ani_array_float array, ani_size offset,
                                                       ani_size length, ani_float *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetRegion_Double(ani_env *env, ani_array_double array, ani_size offset,
                                                        ani_size length, ani_double *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_SetRegion_Char(ani_env *env, ani_array_char array, ani_size offset,
                                                      ani_size length, const ani_char *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_SetRegion_Boolean(ani_env *env, ani_array_boolean array, ani_size offset,
                                                         ani_size length, const ani_boolean *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_SetRegion_Short(ani_env *env, ani_array_short array, ani_size offset,
                                                       ani_size length, const ani_short *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_SetRegion_Int(ani_env *env, ani_array_int array, ani_size offset,
                                                     ani_size length, const ani_int *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_SetRegion_Long(ani_env *env, ani_array_long array, ani_size offset,
                                                      ani_size length, const ani_long *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_SetRegion_Float(ani_env *env, ani_array_float array, ani_size offset,
                                                       ani_size length, const ani_float *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_SetRegion_Double(ani_env *env, ani_array_double array, ani_size offset,
                                                        ani_size length, const ani_double *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_SetRegion_Byte(ani_env *env, ani_array_byte array, ani_size offset,
                                                      ani_size length, const ani_byte *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_BindNativeMethods(ani_env *env, ani_class cls,
                                                         const ani_native_function *methods, ani_size nrMethods)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(methods);

    ANI_CHECK_RETURN_IF_EQ(nrMethods, 0, ANI_OK);
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsClass *klass = s.ToInternalType(cls);

    PandaVector<EtsMethod *> etsMethods;
    etsMethods.reserve(nrMethods);
    for (ani_size i = 0; i < nrMethods; ++i) {
        ani_native_function m = methods[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const char *signature = m.signature;
        EtsMethod *method =
            signature == nullptr ? klass->GetDirectMethod(m.name) : klass->GetDirectMethod(m.name, signature);
        etsMethods.push_back(method);
    }
    return DoBindNative<false>(s, etsMethods, methods, nrMethods);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_Delete(ani_env *env, ani_ref ref)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    return s.DelLocalRef(ref);
}

template <typename T>
static ani_status SetVariableValue(ani_env *env, ani_variable variable, T value)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(variable);

    EtsVariable *etsVariable = ToInternalVariable(variable);
    EtsField *etsField = etsVariable->AsField();
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<T>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
    EtsClass *cls = etsField->GetDeclaringClass();

    static constexpr auto IS_REF = std::is_same_v<T, ani_ref>;
    ScopedManagedCodeFix s(env);
    if constexpr (IS_REF) {
        EtsObject *object = s.ToInternalType(value);
        cls->SetStaticFieldObject(etsField, object);
    } else {
        cls->SetStaticFieldPrimitive<T>(etsField, value);
    }
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FunctionalObject_Call(ani_env *env, ani_fn_object fn, ani_size argc, ani_ref *argv,
                                                       ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    if (argc != 0) {
        CHECK_PTR_ARG(argv);
    }
    CHECK_PTR_ARG(result);

    ANI_CHECK_RETURN_IF_GT(argc, STD_CORE_FUNCTION_MAX_ARITY, ANI_INVALID_ARGS);

    ani_status status = ANI_OK;
    ScopedManagedCodeFix s(env);
    EtsObject *etsFn = s.ToInternalType(fn);
    EtsClass *etsCls = etsFn->GetClass();

    ANI_CHECK_RETURN_IF_EQ(etsCls->IsFunction(), false, ANI_INVALID_TYPE);

    EtsMethod *method = nullptr;
    PandaStringStream methodName;
    methodName << STD_CORE_FUNCTION_INVOKE_PREFIX << argc;
    status = DoGetClassMethod<false>(etsCls, methodName.str().c_str(), nullptr, &method);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    ArgVector<Value> args = {};
    args.reserve(argc + 1);
    args.emplace_back(s.ToInternalType(fn)->GetCoreType());
    for (ani_size i = 0; i < argc; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        args.emplace_back(s.ToInternalType(argv[i])->GetCoreType());
    }

    EtsValue res {};
    status = method->Invoke(s, args.data(), &res);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    *result = res.GetAs<ani_ref>();
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Boolean(ani_env *env, ani_variable variable, ani_boolean value)
{
    ANI_DEBUG_TRACE(env);
    return SetVariableValue<ani_boolean>(env, variable, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Char(ani_env *env, ani_variable variable, ani_char value)
{
    ANI_DEBUG_TRACE(env);
    return SetVariableValue<ani_char>(env, variable, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Byte(ani_env *env, ani_variable variable, ani_byte value)
{
    ANI_DEBUG_TRACE(env);
    return SetVariableValue<ani_byte>(env, variable, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Short(ani_env *env, ani_variable variable, ani_short value)
{
    ANI_DEBUG_TRACE(env);
    return SetVariableValue<EtsShort>(env, variable, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Int(ani_env *env, ani_variable variable, ani_int value)
{
    ANI_DEBUG_TRACE(env);
    return SetVariableValue<EtsInt>(env, variable, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Long(ani_env *env, ani_variable variable, ani_long value)
{
    ANI_DEBUG_TRACE(env);
    return SetVariableValue<EtsLong>(env, variable, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Float(ani_env *env, ani_variable variable, ani_float value)
{
    ANI_DEBUG_TRACE(env);
    return SetVariableValue<EtsFloat>(env, variable, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Double(ani_env *env, ani_variable variable, ani_double value)
{
    ANI_DEBUG_TRACE(env);
    return SetVariableValue<ani_double>(env, variable, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Ref(ani_env *env, ani_variable variable, ani_ref value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(value);
    return SetVariableValue<ani_ref>(env, variable, value);
}

template <bool IS_STATIC_FIELD>
static ani_status DoGetField(ScopedManagedCodeFix &s, ani_class cls, const char *name, EtsField **result)
{
    EtsClass *klass;
    ani_status status = GetInternalClass(s, cls, &klass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    EtsField *field = [&]() {
        if constexpr (IS_STATIC_FIELD) {
            return klass->GetStaticFieldIDByName(name, nullptr);
        } else {
            EtsField *foundField = klass->GetFieldIDByName(name, nullptr);
            if (UNLIKELY(foundField == nullptr)) {
                // NOTE: Need to look for class property implemented from interface
                auto interfaceFieldName = PandaString("<property>") + name;
                foundField = klass->GetFieldIDByName(interfaceFieldName.c_str(), nullptr);
            }
            return foundField;
        }
    }();
    ANI_CHECK_RETURN_IF_EQ(field, nullptr, ANI_NOT_FOUND);

    *result = field;
    return ANI_OK;
}

template <bool IS_STATIC_FIELD>
static ani_status DoGetField(ani_env *env, ani_class cls, const char *name, EtsField **result)
{
    ScopedManagedCodeFix s(env);
    return DoGetField<IS_STATIC_FIELD>(s, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindField(ani_env *env, ani_class cls, const char *name, ani_field *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    EtsField *field = nullptr;
    ani_status status = DoGetField<false>(env, cls, name, &field);
    if (UNLIKELY(status != ANI_OK)) {
        return status;
    }
    *result = ToAniField(field);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindStaticField(ani_env *env, ani_class cls, const char *name,
                                                       ani_static_field *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    EtsField *field = nullptr;
    ani_status status = DoGetField<true>(env, cls, name, &field);
    if (UNLIKELY(status != ANI_OK)) {
        return status;
    }
    *result = ToAniStaticField(field);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindMethod(ani_env *env, ani_class cls, const char *name, const char *signature,
                                                  ani_method *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    EtsMethod *method = nullptr;
    ani_status status = GetClassMethod<false>(env, cls, name, signature, &method);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    *result = ToAniMethod(method);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindStaticMethod(ani_env *env, ani_class cls, const char *name,
                                                        const char *signature, ani_static_method *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    EtsMethod *method = nullptr;
    ani_status status = GetClassMethod<true>(env, cls, name, signature, &method);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    *result = ToAniStaticMethod(method);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindIterator(ani_env *env, ani_class cls, ani_method *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(result);

    EtsMethod *method = nullptr;
    ani_status status = GetClassMethod<false>(env, cls, "$_iterator", nullptr, &method);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    *result = ToAniMethod(method);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Boolean(ani_env *env, ani_class cls, ani_static_field field,
                                                              ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticField<ani_boolean>(env, cls, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Char(ani_env *env, ani_class cls, ani_static_field field,
                                                           ani_char *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticField<ani_char>(env, cls, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Byte(ani_env *env, ani_class cls, ani_static_field field,
                                                           ani_byte *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticField<ani_byte>(env, cls, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Short(ani_env *env, ani_class cls, ani_static_field field,
                                                            ani_short *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticField<ani_short>(env, cls, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Int(ani_env *env, ani_class cls, ani_static_field field,
                                                          ani_int *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticField<ani_int>(env, cls, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Long(ani_env *env, ani_class cls, ani_static_field field,
                                                           ani_long *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticField<ani_long>(env, cls, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Float(ani_env *env, ani_class cls, ani_static_field field,
                                                            ani_float *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticField<ani_float>(env, cls, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Double(ani_env *env, ani_class cls, ani_static_field field,
                                                             ani_double *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticField<ani_double>(env, cls, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Ref(ani_env *env, ani_class cls, ani_static_field field,
                                                          ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(field);
    CHECK_PTR_ARG(result);

    EtsField *etsField = ToInternalField(field);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<ani_ref>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    ScopedManagedCodeFix s(env);
    EtsClass *etsClass = s.ToInternalType(cls);
    auto *etsRes = etsClass->GetStaticFieldObject(etsField);
    return s.AddLocalRef(etsRes, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Boolean(ani_env *env, ani_class cls, ani_static_field field,
                                                              ani_boolean value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticField(env, cls, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Char(ani_env *env, ani_class cls, ani_static_field field,
                                                           ani_char value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticField(env, cls, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Byte(ani_env *env, ani_class cls, ani_static_field field,
                                                           ani_byte value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticField(env, cls, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Short(ani_env *env, ani_class cls, ani_static_field field,
                                                            ani_short value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticField(env, cls, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Int(ani_env *env, ani_class cls, ani_static_field field,
                                                          ani_int value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticField(env, cls, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Long(ani_env *env, ani_class cls, ani_static_field field,
                                                           ani_long value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticField(env, cls, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Float(ani_env *env, ani_class cls, ani_static_field field,
                                                            ani_float value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticField(env, cls, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Double(ani_env *env, ani_class cls, ani_static_field field,
                                                             ani_double value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticField(env, cls, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Ref(ani_env *env, ani_class cls, ani_static_field field,
                                                          ani_ref value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(field);
    CHECK_PTR_ARG(value);

    EtsField *etsField = ToInternalField(field);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<ani_ref>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    ScopedManagedCodeFix s(env);
    EtsClass *etsClass = s.ToInternalType(cls);
    EtsObject *etsValue = s.ToInternalType(value);
    etsClass->SetStaticFieldObject(etsField, etsValue);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Boolean(ani_env *env, ani_class cls, const char *name,
                                                                    ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticFieldByName(env, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Char(ani_env *env, ani_class cls, const char *name,
                                                                 ani_char *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticFieldByName(env, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Byte(ani_env *env, ani_class cls, const char *name,
                                                                 ani_byte *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticFieldByName(env, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Short(ani_env *env, ani_class cls, const char *name,
                                                                  ani_short *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticFieldByName(env, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Int(ani_env *env, ani_class cls, const char *name,
                                                                ani_int *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticFieldByName(env, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Long(ani_env *env, ani_class cls, const char *name,
                                                                 ani_long *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticFieldByName(env, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Float(ani_env *env, ani_class cls, const char *name,
                                                                  ani_float *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticFieldByName(env, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Double(ani_env *env, ani_class cls, const char *name,
                                                                   ani_double *result)
{
    ANI_DEBUG_TRACE(env);

    return ClassGetStaticFieldByName(env, cls, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Ref(ani_env *env, ani_class cls, const char *name,
                                                                ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsClass *klass;
    ani_status status = GetInternalClass(s, cls, &klass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    EtsField *etsField = klass->GetStaticFieldIDByName(name, nullptr);
    ANI_CHECK_RETURN_IF_EQ(etsField, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<ani_ref>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
    EtsObject *etsRes = klass->GetStaticFieldObject(etsField);
    return s.AddLocalRef(etsRes, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Boolean(ani_env *env, ani_class cls, const char *name,
                                                                    ani_boolean value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticFieldByName(env, cls, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Char(ani_env *env, ani_class cls, const char *name,
                                                                 ani_char value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticFieldByName(env, cls, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Byte(ani_env *env, ani_class cls, const char *name,
                                                                 ani_byte value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticFieldByName(env, cls, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Short(ani_env *env, ani_class cls, const char *name,
                                                                  ani_short value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticFieldByName(env, cls, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Int(ani_env *env, ani_class cls, const char *name,
                                                                ani_int value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticFieldByName(env, cls, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Long(ani_env *env, ani_class cls, const char *name,
                                                                 ani_long value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticFieldByName(env, cls, name, value);
}
// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Float(ani_env *env, ani_class cls, const char *name,
                                                                  ani_float value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticFieldByName(env, cls, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Double(ani_env *env, ani_class cls, const char *name,
                                                                   ani_double value)
{
    ANI_DEBUG_TRACE(env);

    return ClassSetStaticFieldByName(env, cls, name, value);
}
// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Ref(ani_env *env, ani_class cls, const char *name,
                                                                ani_ref value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(value);

    ScopedManagedCodeFix s(env);
    EtsClass *klass;
    ani_status status = GetInternalClass(s, cls, &klass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    EtsField *etsField = klass->GetStaticFieldIDByName(name, nullptr);
    ANI_CHECK_RETURN_IF_EQ(etsField, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<ani_ref>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    EtsObject *etsValue = s.ToInternalType(value);
    klass->SetStaticFieldObject(etsField, etsValue);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
ani_status Class_FindSetter(ani_env *env, ani_class cls, const char *name, ani_method *result)
{
    PandaString setterName("<set>");
    setterName += name;
    return Class_FindMethod(env, cls, setterName.c_str(), nullptr, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
ani_status Class_FindGetter(ani_env *env, ani_class cls, const char *name, ani_method *result)
{
    PandaString getterName("<get>");
    getterName += name;
    return Class_FindMethod(env, cls, getterName.c_str(), nullptr, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
ani_status Class_FindIndexableSetter(ani_env *env, ani_class cls, const char *signature, ani_method *result)
{
    return Class_FindMethod(env, cls, "$_set", signature, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
ani_status Class_FindIndexableGetter(ani_env *env, ani_class cls, const char *signature, ani_method *result)
{
    return Class_FindMethod(env, cls, "$_get", signature, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Boolean_V(ani_env *env, ani_class cls, ani_static_method method,
                                                                  ani_boolean *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::BOOLEAN);
    return GeneralMethodCall<EtsBoolean>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Boolean(ani_env *env, ani_class cls, ani_static_method method,
                                                                ani_boolean *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Boolean_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Boolean_A(ani_env *env, ani_class cls, ani_static_method method,
                                                                  ani_boolean *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::BOOLEAN);
    return GeneralMethodCall<EtsBoolean>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Char_V(ani_env *env, ani_class cls, ani_static_method method,
                                                               ani_char *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::CHAR);
    return GeneralMethodCall<EtsChar>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Char(ani_env *env, ani_class cls, ani_static_method method,
                                                             ani_char *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Char_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Char_A(ani_env *env, ani_class cls, ani_static_method method,
                                                               ani_char *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::CHAR);
    return GeneralMethodCall<EtsChar>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Byte_V(ani_env *env, ani_class cls, ani_static_method method,
                                                               ani_byte *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::BYTE);
    return GeneralMethodCall<EtsByte>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Byte(ani_env *env, ani_class cls, ani_static_method method,
                                                             ani_byte *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Byte_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Byte_A(ani_env *env, ani_class cls, ani_static_method method,
                                                               ani_byte *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::BYTE);
    return GeneralMethodCall<EtsByte>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Short_V(ani_env *env, ani_class cls, ani_static_method method,
                                                                ani_short *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::SHORT);
    return GeneralMethodCall<EtsShort>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Short(ani_env *env, ani_class cls, ani_static_method method,
                                                              ani_short *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Short_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Short_A(ani_env *env, ani_class cls, ani_static_method method,
                                                                ani_short *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::SHORT);
    return GeneralMethodCall<EtsShort>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Int_V(ani_env *env, ani_class cls, ani_static_method method,
                                                              ani_int *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::INT);
    return GeneralMethodCall<EtsInt>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Int(ani_env *env, ani_class cls, ani_static_method method,
                                                            ani_int *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Int_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Int_A(ani_env *env, ani_class cls, ani_static_method method,
                                                              ani_int *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::INT);
    return GeneralMethodCall<EtsInt>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Long_V(ani_env *env, ani_class cls, ani_static_method method,
                                                               ani_long *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::LONG);
    return GeneralMethodCall<EtsLong>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Long(ani_env *env, ani_class cls, ani_static_method method,
                                                             ani_long *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Long_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Long_A(ani_env *env, ani_class cls, ani_static_method method,
                                                               ani_long *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::LONG);
    return GeneralMethodCall<EtsLong>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Float_V(ani_env *env, ani_class cls, ani_static_method method,
                                                                ani_float *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::FLOAT);
    return GeneralMethodCall<EtsFloat>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Float(ani_env *env, ani_class cls, ani_static_method method,
                                                              ani_float *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Float_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Float_A(ani_env *env, ani_class cls, ani_static_method method,
                                                                ani_float *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::FLOAT);
    return GeneralMethodCall<EtsFloat>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Double_V(ani_env *env, ani_class cls, ani_static_method method,
                                                                 ani_double *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::DOUBLE);
    return GeneralMethodCall<EtsDouble>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Double(ani_env *env, ani_class cls, ani_static_method method,
                                                               ani_double *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Double_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Double_A(ani_env *env, ani_class cls, ani_static_method method,
                                                                 ani_double *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::DOUBLE);
    return GeneralMethodCall<EtsDouble>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Boolean(ani_env *env, ani_object object, ani_field field,
                                                         ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);

    return GetPrimitiveTypeField(env, object, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Char(ani_env *env, ani_object object, ani_field field,
                                                      ani_char *result)
{
    ANI_DEBUG_TRACE(env);

    return GetPrimitiveTypeField(env, object, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Byte(ani_env *env, ani_object object, ani_field field,
                                                      ani_byte *result)
{
    ANI_DEBUG_TRACE(env);

    return GetPrimitiveTypeField(env, object, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Short(ani_env *env, ani_object object, ani_field field,
                                                       ani_short *result)
{
    ANI_DEBUG_TRACE(env);

    return GetPrimitiveTypeField(env, object, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Int(ani_env *env, ani_object object, ani_field field, ani_int *result)
{
    ANI_DEBUG_TRACE(env);

    return GetPrimitiveTypeField(env, object, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Long(ani_env *env, ani_object object, ani_field field,
                                                      ani_long *result)
{
    ANI_DEBUG_TRACE(env);

    return GetPrimitiveTypeField(env, object, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Float(ani_env *env, ani_object object, ani_field field,
                                                       ani_float *result)
{
    ANI_DEBUG_TRACE(env);

    return GetPrimitiveTypeField(env, object, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Double(ani_env *env, ani_object object, ani_field field,
                                                        ani_double *result)
{
    ANI_DEBUG_TRACE(env);

    return GetPrimitiveTypeField(env, object, field, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Ref(ani_env *env, ani_object object, ani_field field, ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(field);
    CHECK_PTR_ARG(result);

    EtsField *etsField = ToInternalField(field);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<ani_ref>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    ScopedManagedCodeFix s(env);
    EtsObject *etsObject = s.ToInternalType(object);
    EtsObject *etsRes = etsObject->GetFieldObject(etsField);
    return s.AddLocalRef(etsRes, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Boolean(ani_env *env, ani_object object, ani_field field,
                                                         ani_boolean value)
{
    ANI_DEBUG_TRACE(env);

    return SetPrimitiveTypeField(env, object, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Char(ani_env *env, ani_object object, ani_field field, ani_char value)
{
    ANI_DEBUG_TRACE(env);

    return SetPrimitiveTypeField(env, object, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Byte(ani_env *env, ani_object object, ani_field field, ani_byte value)
{
    ANI_DEBUG_TRACE(env);

    return SetPrimitiveTypeField(env, object, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Short(ani_env *env, ani_object object, ani_field field,
                                                       ani_short value)
{
    ANI_DEBUG_TRACE(env);

    return SetPrimitiveTypeField(env, object, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Int(ani_env *env, ani_object object, ani_field field, ani_int value)
{
    ANI_DEBUG_TRACE(env);

    return SetPrimitiveTypeField(env, object, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Long(ani_env *env, ani_object object, ani_field field, ani_long value)
{
    ANI_DEBUG_TRACE(env);

    return SetPrimitiveTypeField(env, object, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Float(ani_env *env, ani_object object, ani_field field,
                                                       ani_float value)
{
    ANI_DEBUG_TRACE(env);

    return SetPrimitiveTypeField(env, object, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Double(ani_env *env, ani_object object, ani_field field,
                                                        ani_double value)
{
    ANI_DEBUG_TRACE(env);

    return SetPrimitiveTypeField(env, object, field, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Ref(ani_env *env, ani_object object, ani_field field, ani_ref value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(field);
    CHECK_PTR_ARG(value);

    EtsField *etsField = ToInternalField(field);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<ani_ref>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    ScopedManagedCodeFix s(env);
    EtsObject *etsObject = s.ToInternalType(object);
    EtsObject *etsValue = s.ToInternalType(value);
    etsObject->SetFieldObject(etsField, etsValue);
    return ANI_OK;
}

template <typename R>
static ani_status DoGetFieldByName(ani_env *env, ani_object object, const char *name, R *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    static constexpr auto IS_REF = std::is_same_v<R, ani_ref>;
    using Res = std::conditional_t<IS_REF, EtsObject *, R>;

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> etsObject(coroutine, s.ToInternalType(object));
    ASSERT(etsObject.GetPtr() != nullptr);
    EtsField *etsField = etsObject->GetClass()->GetFieldIDByName(name, nullptr);
    ANI_CHECK_RETURN_IF_EQ(etsField, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<R>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    Res etsRes {};
    if constexpr (IS_REF) {
        etsRes = etsObject->GetFieldObject(etsField);
        return s.AddLocalRef(etsRes, result);
    } else {
        etsRes = etsObject->GetFieldPrimitive<R>(etsField);
        *result = etsRes;
        return ANI_OK;
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Boolean(ani_env *env, ani_object object, const char *name,
                                                               ani_boolean *result)
{
    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Char(ani_env *env, ani_object object, const char *name,
                                                            ani_char *result)
{
    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Byte(ani_env *env, ani_object object, const char *name,
                                                            ani_byte *result)
{
    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Short(ani_env *env, ani_object object, const char *name,
                                                             ani_short *result)
{
    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Int(ani_env *env, ani_object object, const char *name,
                                                           ani_int *result)
{
    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Long(ani_env *env, ani_object object, const char *name,
                                                            ani_long *result)
{
    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Float(ani_env *env, ani_object object, const char *name,
                                                             ani_float *result)
{
    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Double(ani_env *env, ani_object object, const char *name,
                                                              ani_double *result)
{
    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Ref(ani_env *env, ani_object object, const char *name,
                                                           ani_ref *result)
{
    return DoGetFieldByName(env, object, name, result);
}

template <typename T>
NO_UB_SANITIZE ani_status ObjectSetFieldByNamePrimitive(ani_env *env, ani_object object, const char *name, T value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> etsObject(coroutine, s.ToInternalType(object));
    ASSERT(etsObject.GetPtr() != nullptr);
    EtsField *etsField = etsObject->GetClass()->GetFieldIDByName(name, nullptr);
    ANI_CHECK_RETURN_IF_EQ(etsField, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<T>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    etsObject->SetFieldPrimitive(etsField, value);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
NO_UB_SANITIZE ani_status Object_SetFieldByName_Boolean(ani_env *env, ani_object object, const char *name,
                                                        ani_boolean value)
{
    ANI_DEBUG_TRACE(env);

    return ObjectSetFieldByNamePrimitive(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
NO_UB_SANITIZE ani_status Object_SetFieldByName_Char(ani_env *env, ani_object object, const char *name, ani_char value)
{
    ANI_DEBUG_TRACE(env);

    return ObjectSetFieldByNamePrimitive(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
NO_UB_SANITIZE ani_status Object_SetFieldByName_Byte(ani_env *env, ani_object object, const char *name, ani_byte value)
{
    ANI_DEBUG_TRACE(env);

    return ObjectSetFieldByNamePrimitive(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
NO_UB_SANITIZE ani_status Object_SetFieldByName_Short(ani_env *env, ani_object object, const char *name,
                                                      ani_short value)
{
    ANI_DEBUG_TRACE(env);

    return ObjectSetFieldByNamePrimitive(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
NO_UB_SANITIZE ani_status Object_SetFieldByName_Int(ani_env *env, ani_object object, const char *name, ani_int value)
{
    ANI_DEBUG_TRACE(env);

    return ObjectSetFieldByNamePrimitive(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
NO_UB_SANITIZE ani_status Object_SetFieldByName_Long(ani_env *env, ani_object object, const char *name, ani_long value)
{
    ANI_DEBUG_TRACE(env);

    return ObjectSetFieldByNamePrimitive(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
NO_UB_SANITIZE ani_status Object_SetFieldByName_Float(ani_env *env, ani_object object, const char *name,
                                                      ani_float value)
{
    ANI_DEBUG_TRACE(env);

    return ObjectSetFieldByNamePrimitive(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
NO_UB_SANITIZE ani_status Object_SetFieldByName_Double(ani_env *env, ani_object object, const char *name,
                                                       ani_double value)
{
    ANI_DEBUG_TRACE(env);

    return ObjectSetFieldByNamePrimitive(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Ref(ani_env *env, ani_object object, const char *name,
                                                           ani_ref value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(value);

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> etsObject(coroutine, s.ToInternalType(object));
    ASSERT(etsObject.GetPtr() != nullptr);
    EtsField *etsField = etsObject->GetClass()->GetFieldIDByName(name, nullptr);
    EtsObject *etsValue = s.ToInternalType(value);
    ANI_CHECK_RETURN_IF_EQ(etsField, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<ani_ref>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

    etsObject->SetFieldObject(etsField, etsValue);
    return ANI_OK;
}

template <typename R>
static ani_status DoGetPropertyByName(ani_env *env, ani_object object, const char *name, R *result)
{
    static constexpr auto IS_REF = std::is_same_v<R, ani_ref>;
    using Res = std::conditional_t<IS_REF, EtsObject *, R>;

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    Res etsRes {};
    EtsHandle<EtsObject> etsObject(coroutine, s.ToInternalType(object));
    EtsHandle<EtsClass> klass(coroutine, etsObject->GetClass());
    EtsField *field = klass->GetFieldIDByName(name, nullptr);
    if (field != nullptr) {
        // Property as field
        ANI_CHECK_RETURN_IF_NE(field->GetEtsType(), AniTypeInfo<R>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
        if constexpr (IS_REF) {
            etsRes = etsObject->GetFieldObject(field);
        } else {
            etsRes = etsObject->GetFieldPrimitive<R>(field);
        }
    } else {
        // Property as getter
        EtsMethod *method = klass->GetInstanceMethod((PandaString(GETTER_BEGIN) + name).c_str(), nullptr);
        ANI_CHECK_RETURN_IF_EQ(method, nullptr, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_EQ(method->IsStatic(), true, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_NE(method->GetNumArgs(), 1, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_NE(method->GetArgType(0), EtsType::OBJECT, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_NE(method->GetReturnValueType(), AniTypeInfo<R>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);

        std::array args = {Value {etsObject->GetCoreType()}};
        Value res = method->GetPandaMethod()->Invoke(coroutine, args.data());
        ANI_CHECK_RETURN_IF_EQ(coroutine->HasPendingException(), true, ANI_PENDING_ERROR);

        if constexpr (IS_REF) {
            etsRes = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
        } else {
            etsRes = res.GetAs<R>();
        }
    }
    if constexpr (IS_REF) {
        return s.AddLocalRef(etsRes, result);
    } else {
        *result = etsRes;
        return ANI_OK;
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Boolean(ani_env *env, ani_object object, const char *name,
                                                                  ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Char(ani_env *env, ani_object object, const char *name,
                                                               ani_char *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Byte(ani_env *env, ani_object object, const char *name,
                                                               ani_byte *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Short(ani_env *env, ani_object object, const char *name,
                                                                ani_short *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Int(ani_env *env, ani_object object, const char *name,
                                                              ani_int *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Long(ani_env *env, ani_object object, const char *name,
                                                               ani_long *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Float(ani_env *env, ani_object object, const char *name,
                                                                ani_float *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Double(ani_env *env, ani_object object, const char *name,
                                                                 ani_double *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Ref(ani_env *env, ani_object object, const char *name,
                                                              ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetPropertyByName(env, object, name, result);
}

template <typename R>
static ani_status DoSetPropertyByName(ani_env *env, ani_object object, const char *name, R value)
{
    static constexpr auto IS_REF = std::is_same_v<R, ani_ref>;

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> etsObject(coroutine, s.ToInternalType(object));
    EtsHandle<EtsClass> klass(coroutine, etsObject->GetClass());
    EtsField *field = klass->GetFieldIDByName(name, nullptr);
    if (field != nullptr) {
        // Property as field
        ANI_CHECK_RETURN_IF_NE(field->GetEtsType(), AniTypeInfo<R>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
        if constexpr (IS_REF) {
            EtsObject *etsValue = s.ToInternalType(value);
            etsObject->SetFieldObject(field, etsValue);
        } else {
            etsObject->SetFieldPrimitive<R>(field, value);
        }
    } else {
        // Property as setter
        EtsMethod *method = klass->GetInstanceMethod((PandaString(SETTER_BEGIN) + name).c_str(), nullptr);
        ANI_CHECK_RETURN_IF_EQ(method, nullptr, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_EQ(method->IsStatic(), true, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_NE(method->GetNumArgs(), 2U, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_NE(method->GetArgType(0), EtsType::OBJECT, ANI_NOT_FOUND);
        ANI_CHECK_RETURN_IF_NE(method->GetArgType(1U), AniTypeInfo<R>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
        ANI_CHECK_RETURN_IF_NE(method->GetReturnValueType(), EtsType::VOID, ANI_INVALID_TYPE);

        std::array<Value, 2U> args;
        args.at(0) = Value {etsObject->GetCoreType()};
        if constexpr (IS_REF) {
            EtsObject *etsValue = s.ToInternalType(value);
            args.at(1U) = Value {etsValue->GetCoreType()};
        } else {
            args.at(1U) = Value {value};
        }

        method->GetPandaMethod()->Invoke(coroutine, args.data());
        ANI_CHECK_RETURN_IF_EQ(coroutine->HasPendingException(), true, ANI_PENDING_ERROR);
    }
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Boolean(ani_env *env, ani_object object, const char *name,
                                                                  ani_boolean value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Char(ani_env *env, ani_object object, const char *name,
                                                               ani_char value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Byte(ani_env *env, ani_object object, const char *name,
                                                               ani_byte value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Short(ani_env *env, ani_object object, const char *name,
                                                                ani_short value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Int(ani_env *env, ani_object object, const char *name,
                                                              ani_int value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Long(ani_env *env, ani_object object, const char *name,
                                                               ani_long value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Float(ani_env *env, ani_object object, const char *name,
                                                                ani_float value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Double(ani_env *env, ani_object object, const char *name,
                                                                 ani_double value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Ref(ani_env *env, ani_object object, const char *name,
                                                              ani_ref value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);

    return DoSetPropertyByName(env, object, name, value);
}

NO_UB_SANITIZE static ani_status ExistUnhandledError(ani_env *env, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(result);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    *result = pandaEnv->HasPendingException() ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status ResetError(ani_env *env)
{
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(env);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    pandaEnv->ClearException();
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status ThrowError(ani_env *env, ani_error err)
{
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(err);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    EtsThrowable *exception = s.ToInternalType(err);
    ANI_CHECK_RETURN_IF_EQ(exception, nullptr, ANI_ERROR);
    PandaEnv::ToPandaEtsEnv(pandaEnv)->SetException(exception);
    return ANI_OK;
}

class ClearExceptionScope {
public:
    explicit ClearExceptionScope(ani_env *env) : env_(env)
    {
        [[maybe_unused]] ani_status status = env_->GetUnhandledError(&error_);
        ASSERT(status == ANI_OK);
        ASSERT(error_ != nullptr);

        status = env_->ResetError();
        ASSERT(status == ANI_OK);
    }

    NO_COPY_SEMANTIC(ClearExceptionScope);
    NO_MOVE_SEMANTIC(ClearExceptionScope);

    ~ClearExceptionScope()
    {
        [[maybe_unused]] ani_status status = ANI_OK;
#ifndef NDEBUG
        ani_boolean exists = ANI_FALSE;
        status = env_->ExistUnhandledError(&exists);
        ASSERT(status == ANI_OK);
        ASSERT(exists == ANI_FALSE);
#endif  // NDEBUG
        status = env_->ThrowError(error_);
        ASSERT(status == ANI_OK);
    }

    ani_error GetError()
    {
        return error_;
    }

private:
    ani_env *env_ {nullptr};
    ani_error error_ {nullptr};
};

// NOLINTBEGIN(clang-analyzer-deadcode.DeadStores)
static ani_status GetErrorDescription(ani_env *env, ani_error error, ani_array_ref *errorDescription)
{
    // Create `console.error` arguments array
    ani_class objectClass = nullptr;
    // NOTE(dslynko, #23447): use cache of well-known classes
    auto status = env->FindClass("Lstd/core/Object;", &objectClass);
    ASSERT(status == ANI_OK);

    ani_ref undefinedRef = nullptr;
    status = env->GetUndefined(&undefinedRef);
    ASSERT(status == ANI_OK);
    ani_array_ref descriptionLinesArray = nullptr;
    status = env->Array_New_Ref(objectClass, static_cast<ani_size>(3U), undefinedRef, &descriptionLinesArray);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    ani_type errorType = nullptr;
    status = env->Object_GetType(error, &errorType);
    ASSERT(status == ANI_OK);

    // Get error message
    ani_ref errorMessage = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethodByName_Ref(error, "toString", ":Lstd/core/String;", &errorMessage);
    ASSERT(status == ANI_OK);
    status = env->Array_Set_Ref(descriptionLinesArray, static_cast<ani_size>(0U), errorMessage);
    ASSERT(status == ANI_OK);

    // Set newline between error message and stack trace
    ani_string newlineString = nullptr;
    std::string_view newline = "\n";
    status = env->String_NewUTF8(newline.data(), newline.size(), &newlineString);
    ASSERT(status == ANI_OK);
    status = env->Array_Set_Ref(descriptionLinesArray, static_cast<ani_size>(1U), newlineString);
    ASSERT(status == ANI_OK);

    // Get stack trace
    ani_method getterMethod = nullptr;
    status = env->Class_FindGetter(static_cast<ani_class>(errorType), "stack", &getterMethod);
    ASSERT(status == ANI_OK);
    ani_ref stackTrace = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethod_Ref(error, getterMethod, &stackTrace);
    ASSERT(status == ANI_OK);

    ani_boolean isUndefined = ANI_FALSE;
    status = env->Reference_IsUndefined(stackTrace, &isUndefined);
    ASSERT(status == ANI_OK);
    if (isUndefined == ANI_TRUE) {
        std::string_view errorInfo = "unable to get stack trace";
        ani_string createdString;
        status = env->String_NewUTF8(errorInfo.data(), errorInfo.size(), &createdString);
        ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
        stackTrace = createdString;
    }
    status = env->Array_Set_Ref(descriptionLinesArray, static_cast<ani_size>(2U), stackTrace);
    ASSERT(status == ANI_OK);

    *errorDescription = descriptionLinesArray;
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status DescribeError(ani_env *env)
{
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(env);

    ani_boolean errorExists = ANI_FALSE;
    auto status = env->ExistUnhandledError(&errorExists);
    ASSERT(status == ANI_OK);
    if (errorExists == ANI_FALSE) {
        return ANI_OK;
    }
    ClearExceptionScope s(env);

    ani_array_ref errorDescription = nullptr;
    status = GetErrorDescription(env, s.GetError(), &errorDescription);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    // Get `std.core.console` global variable
    ani_module stdCoreModule = nullptr;
    status = env->FindModule("Lstd/core;", &stdCoreModule);
    ASSERT(status == ANI_OK);
    ani_variable consoleVar = nullptr;
    status = env->Module_FindVariable(stdCoreModule, "console", &consoleVar);
    ASSERT(status == ANI_OK);
    ani_ref console = nullptr;
    status = env->Variable_GetValue_Ref(consoleVar, &console);
    ASSERT(status == ANI_OK);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    return env->Object_CallMethodByName_Void(static_cast<ani_object>(console), "error", "[Lstd/core/Object;:V",
                                             errorDescription);
}
// NOLINTEND(clang-analyzer-deadcode.DeadStores)

NO_UB_SANITIZE static ani_status GetUnhandledError(ani_env *env, ani_error *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(result);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    EtsThrowable *throwable = pandaEnv->GetThrowable();
    ANI_CHECK_RETURN_IF_EQ(throwable, nullptr, ANI_ERROR);
    return s.AddLocalRef(throwable, reinterpret_cast<ani_ref *>(result));
}

NO_UB_SANITIZE static ani_status Abort([[maybe_unused]] ani_env *env, const char *message)
{
    ANI_DEBUG_TRACE(env);
    LOG(FATAL, ANI) << "EtsAniError: " << message;
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status GetNull(ani_env *env, ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    return s.GetNullRef(result);
}

NO_UB_SANITIZE static ani_status GetUndefined(ani_env *env, ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    return ManagedCodeAccessor::GetUndefinedRef(result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_IsNull(ani_env *env, ani_ref ref, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    // Fast path
    if (IsUndefined(ref)) {
        *result = ANI_FALSE;
        return ANI_OK;
    }
    // Slow path
    ScopedManagedCodeFix s(env);
    *result = s.IsNull(ref) ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_IsUndefined(ani_env *env, ani_ref ref, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    *result = IsUndefined(ref) ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_IsNullishValue(ani_env *env, ani_ref ref, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    // Fast path
    if (IsUndefined(ref)) {
        *result = ANI_TRUE;
        return ANI_OK;
    }
    // Slow path
    ScopedManagedCodeFix s(env);
    *result = s.IsNull(ref) ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_Equals(ani_env *env, ani_ref ref0, ani_ref ref1, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    // Fast path
    if (IsUndefined(ref0) && IsUndefined(ref1)) {
        *result = ANI_TRUE;
        return ANI_OK;
    }
    // Slow path
    ScopedManagedCodeFix s(env);
    bool isEquals = EtsReferenceEquals<false>(s.GetCoroutine(), s.ToInternalType(ref0), s.ToInternalType(ref1));
    *result = isEquals ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_StrictEquals(ani_env *env, ani_ref ref0, ani_ref ref1, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    // Fast path
    if (IsUndefined(ref0) && IsUndefined(ref1)) {
        *result = ANI_TRUE;
        return ANI_OK;
    }
    // Slow path
    ScopedManagedCodeFix s(env);
    bool isStrictEquals = EtsReferenceEquals<true>(s.GetCoroutine(), s.ToInternalType(ref0), s.ToInternalType(ref1));
    *result = isStrictEquals ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_InstanceOf(ani_env *env, ani_object object, ani_type type, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(type);
    CHECK_PTR_ARG(object);

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsClass *internalClass = s.ToInternalType(type);
    EtsObject *internalObject = s.ToInternalType(object);

    // NOTE: Update implementation when all types will be supported. #22003
    *result = internalObject->IsInstanceOf(internalClass) ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Ref_V(ani_env *env, ani_class cls, ani_static_method method,
                                                              ani_ref *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckStaticMethodReturnType(method, EtsType::OBJECT);
    return GeneralMethodCall<ani_ref>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Ref(ani_env *env, ani_class cls, ani_static_method method,
                                                            ani_ref *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethod_Ref_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Ref_A(ani_env *env, ani_class cls, ani_static_method method,
                                                              ani_ref *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::OBJECT);
    return GeneralMethodCall<ani_ref>(env, nullptr, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Void_V(ani_env *env, ani_class cls, ani_static_method method,
                                                               va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);

    CheckStaticMethodReturnType(method, EtsType::VOID);
    ani_boolean result;
    // Use any primitive type as template parameter and just ignore the result
    return GeneralMethodCall<EtsBoolean>(env, nullptr, method, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Void(ani_env *env, ani_class cls, ani_static_method method, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, method);
    ani_status status = Class_CallStaticMethod_Void_V(env, cls, method, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Void_A(ani_env *env, ani_class cls, ani_static_method method,
                                                               const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(args);

    CheckStaticMethodReturnType(method, EtsType::VOID);
    ani_boolean result;
    // Use any primitive type as template parameter and just ignore the result
    return GeneralMethodCall<EtsBoolean>(env, nullptr, method, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Boolean_V(ani_env *env, ani_class cls, const char *name,
                                                                        const char *signature, ani_boolean *result,
                                                                        va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<EtsBoolean, EtsType::BOOLEAN>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Boolean(ani_env *env, ani_class cls, const char *name,
                                                                      const char *signature, ani_boolean *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Boolean_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Boolean_A(ani_env *env, ani_class cls, const char *name,
                                                                        const char *signature, ani_boolean *result,
                                                                        const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<EtsBoolean, EtsType::BOOLEAN>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Char_V(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, ani_char *result,
                                                                     va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<EtsChar, EtsType::CHAR>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Char(ani_env *env, ani_class cls, const char *name,
                                                                   const char *signature, ani_char *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Char_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Char_A(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, ani_char *result,
                                                                     const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<EtsChar, EtsType::CHAR>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Byte_V(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, ani_byte *result,
                                                                     va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<EtsByte, EtsType::BYTE>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Byte(ani_env *env, ani_class cls, const char *name,
                                                                   const char *signature, ani_byte *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Byte_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Byte_A(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, ani_byte *result,
                                                                     const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<EtsByte, EtsType::BYTE>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Short_V(ani_env *env, ani_class cls, const char *name,
                                                                      const char *signature, ani_short *result,
                                                                      va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<EtsShort, EtsType::SHORT>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Short(ani_env *env, ani_class cls, const char *name,
                                                                    const char *signature, ani_short *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Short_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Short_A(ani_env *env, ani_class cls, const char *name,
                                                                      const char *signature, ani_short *result,
                                                                      const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<EtsShort, EtsType::SHORT>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Int_A(ani_env *env, ani_class cls, const char *name,
                                                                    const char *signature, ani_int *result,
                                                                    const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<EtsInt, EtsType::INT>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Int_V(ani_env *env, ani_class cls, const char *name,
                                                                    const char *signature, ani_int *result,
                                                                    va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<EtsInt, EtsType::INT>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Int(ani_env *env, ani_class cls, const char *name,
                                                                  const char *signature, ani_int *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Int_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Long_A(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, ani_long *result,
                                                                     const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<EtsLong, EtsType::LONG>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Long_V(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, ani_long *result,
                                                                     va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<EtsLong, EtsType::LONG>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Long(ani_env *env, ani_class cls, const char *name,
                                                                   const char *signature, ani_long *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Long_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Float_A(ani_env *env, ani_class cls, const char *name,
                                                                      const char *signature, ani_float *result,
                                                                      const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<EtsFloat, EtsType::FLOAT>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Float_V(ani_env *env, ani_class cls, const char *name,
                                                                      const char *signature, ani_float *result,
                                                                      va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<EtsFloat, EtsType::FLOAT>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Float(ani_env *env, ani_class cls, const char *name,
                                                                    const char *signature, ani_float *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Float_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Double_A(ani_env *env, ani_class cls, const char *name,
                                                                       const char *signature, ani_double *result,
                                                                       const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<EtsDouble, EtsType::DOUBLE>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Double_V(ani_env *env, ani_class cls, const char *name,
                                                                       const char *signature, ani_double *result,
                                                                       va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<EtsDouble, EtsType::DOUBLE>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Double(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, ani_double *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Double_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Ref_A(ani_env *env, ani_class cls, const char *name,
                                                                    const char *signature, ani_ref *result,
                                                                    const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ClassCallMethodByName<ani_ref, EtsType::OBJECT>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Ref_V(ani_env *env, ani_class cls, const char *name,
                                                                    const char *signature, ani_ref *result,
                                                                    va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ClassCallMethodByName<ani_ref, EtsType::OBJECT>(env, cls, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Ref(ani_env *env, ani_class cls, const char *name,
                                                                  const char *signature, ani_ref *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Class_CallStaticMethodByName_Ref_V(env, cls, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Void_A(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    ani_boolean result;
    return ClassCallMethodByName<EtsBoolean, EtsType::VOID>(env, cls, name, signature, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Void_V(ani_env *env, ani_class cls, const char *name,
                                                                     const char *signature, va_list args)
{
    ANI_DEBUG_TRACE(env);

    ani_boolean result;
    return ClassCallMethodByName<EtsBoolean, EtsType::VOID>(env, cls, name, signature, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Void(ani_env *env, ani_class cls, const char *name,
                                                                   const char *signature, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, signature);
    ani_status status = Class_CallStaticMethodByName_Void_V(env, cls, name, signature, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_NewUTF8(ani_env *env, const char *utf8_string, ani_size size,
                                                ani_string *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(utf8_string);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    auto internalString = EtsString::CreateFromUtf8(utf8_string, size);
    ANI_CHECK_RETURN_IF_EQ(internalString, nullptr, ANI_OUT_OF_MEMORY);
    return s.AddLocalRef(internalString->AsObject(), reinterpret_cast<ani_ref *>(result));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF8Size(ani_env *env, ani_string string, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(string);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    auto internalString = s.ToInternalType(string);
    *result = internalString->GetUtf8Length();
    return ANI_OK;
}
// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF8(ani_env *env, ani_string string, char *utf8Buffer,
                                                ani_size utf8BufferSize, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(string);
    CHECK_PTR_ARG(utf8Buffer);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    auto internalString = s.ToInternalType(string);
    auto utf8Length = internalString->GetUtf8Length();
    if (UNLIKELY(utf8BufferSize < utf8Length || (utf8BufferSize - utf8Length) < 1)) {
        return ANI_BUFFER_TO_SMALL;
    }
    ani_size actualCopiedSize = internalString->CopyDataRegionUtf8(utf8Buffer, 0, utf8Length, utf8Length);
    ANI_CHECK_RETURN_IF_NE(actualCopiedSize, utf8Length, ANI_ERROR);
    utf8Buffer[actualCopiedSize] = '\0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *result = actualCopiedSize;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF8SubString(ani_env *env, ani_string string, ani_size substr_offset,
                                                         ani_size substrSize, char *utfBuffer, ani_size utfBufferSize,
                                                         ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(string);
    CHECK_PTR_ARG(utfBuffer);
    CHECK_PTR_ARG(result);

    if (UNLIKELY(utfBufferSize < substrSize || (utfBufferSize - substrSize) < 1)) {
        return ANI_BUFFER_TO_SMALL;
    }

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsString *internalString = s.ToInternalType(string);
    auto utf8Length = internalString->GetUtf8Length();
    if (UNLIKELY(substr_offset > utf8Length || substrSize > (utf8Length - substr_offset))) {
        return ANI_OUT_OF_RANGE;
    }
    ani_size actualCopiedSize = internalString->CopyDataRegionUtf8(utfBuffer, substr_offset, substrSize, substrSize);
    utfBuffer[actualCopiedSize] = '\0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *result = actualCopiedSize;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_NewUTF16(ani_env *env, const uint16_t *utf16_string, ani_size utf16_size,
                                                 ani_string *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(utf16_string);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    auto internalString = EtsString::CreateFromUtf16(utf16_string, utf16_size);
    ANI_CHECK_RETURN_IF_EQ(internalString, nullptr, ANI_OUT_OF_MEMORY);
    return s.AddLocalRef(internalString->AsObject(), reinterpret_cast<ani_ref *>(result));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF16Size(ani_env *env, ani_string string, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(string);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    auto internalString = s.ToInternalType(string);
    *result = internalString->GetUtf16Length();
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF16(ani_env *env, ani_string string, uint16_t *utf16Buffer,
                                                 ani_size utf16BufferSize, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(string);
    CHECK_PTR_ARG(utf16Buffer);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsString *internalString = s.ToInternalType(string);
    auto utf16Length = internalString->GetUtf16Length();
    if (UNLIKELY(utf16BufferSize < utf16Length || (utf16BufferSize - utf16Length) < 1)) {
        return ANI_BUFFER_TO_SMALL;
    }
    ani_size actualCopiedSize = internalString->CopyDataRegionUtf16(utf16Buffer, 0, utf16Length, utf16BufferSize);
    ANI_CHECK_RETURN_IF_NE(actualCopiedSize, utf16Length, ANI_ERROR);
    utf16Buffer[actualCopiedSize] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *result = actualCopiedSize;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF16SubString(ani_env *env, ani_string string, ani_size substr_offset,
                                                          ani_size substrSize, uint16_t *utf16Buffer,
                                                          ani_size utf16BufferSize, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(string);
    CHECK_PTR_ARG(utf16Buffer);
    CHECK_PTR_ARG(result);
    if (UNLIKELY(utf16BufferSize < substrSize) || (utf16BufferSize - substrSize) < 1) {
        return ANI_BUFFER_TO_SMALL;
    }
    ScopedManagedCodeFix s(env);
    EtsString *internalString = s.ToInternalType(string);
    auto utf16Length = internalString->GetUtf16Length();
    if (UNLIKELY(substr_offset > utf16Length || substrSize > (utf16Length - substr_offset))) {
        return ANI_OUT_OF_RANGE;
    }
    ani_size actualCopiedSize = internalString->CopyDataRegionUtf16(utf16Buffer, substr_offset, substrSize, substrSize);
    ANI_CHECK_RETURN_IF_NE(actualCopiedSize, substrSize, ANI_ERROR);
    utf16Buffer[actualCopiedSize] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *result = actualCopiedSize;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Boolean_V(ani_env *env, ani_object object, ani_method method,
                                                             ani_boolean *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::BOOLEAN);
    return GeneralMethodCall<EtsBoolean>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Boolean(ani_env *env, ani_object object, ani_method method,
                                                           ani_boolean *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status res = Object_CallMethod_Boolean_V(env, object, method, result, args);
    va_end(args);
    return res;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Boolean_A(ani_env *env, ani_object object, ani_method method,
                                                             ani_boolean *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::BOOLEAN);
    return GeneralMethodCall<EtsBoolean>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Char_V(ani_env *env, ani_object object, ani_method method,
                                                          ani_char *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::CHAR);
    return GeneralMethodCall<EtsChar>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Char(ani_env *env, ani_object object, ani_method method,
                                                        ani_char *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethod_Char_V(env, object, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Char_A(ani_env *env, ani_object object, ani_method method,
                                                          ani_char *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::CHAR);
    return GeneralMethodCall<EtsChar>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Byte_V(ani_env *env, ani_object object, ani_method method,
                                                          ani_byte *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::BYTE);
    return GeneralMethodCall<EtsByte>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Byte(ani_env *env, ani_object object, ani_method method,
                                                        ani_byte *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethod_Byte_V(env, object, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Byte_A(ani_env *env, ani_object object, ani_method method,
                                                          ani_byte *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::BYTE);
    return GeneralMethodCall<EtsByte>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Short_V(ani_env *env, ani_object object, ani_method method,
                                                           ani_short *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::SHORT);
    return GeneralMethodCall<EtsShort>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Short(ani_env *env, ani_object object, ani_method method,
                                                         ani_short *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethod_Short_V(env, object, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Short_A(ani_env *env, ani_object object, ani_method method,
                                                           ani_short *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::SHORT);
    return GeneralMethodCall<EtsShort>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Int_V(ani_env *env, ani_object object, ani_method method,
                                                         ani_int *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::INT);
    return GeneralMethodCall<EtsInt>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Int(ani_env *env, ani_object object, ani_method method,
                                                       ani_int *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethod_Int_V(env, object, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Int_A(ani_env *env, ani_object object, ani_method method,
                                                         ani_int *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::INT);
    return GeneralMethodCall<EtsInt>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Long_A(ani_env *env, ani_object object, ani_method method,
                                                          ani_long *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::LONG);
    return GeneralMethodCall<EtsLong>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Long_V(ani_env *env, ani_object object, ani_method method,
                                                          ani_long *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::LONG);
    return GeneralMethodCall<EtsLong>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Long(ani_env *env, ani_object object, ani_method method,
                                                        ani_long *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status aniResult = Object_CallMethod_Long_V(env, object, method, result, args);
    va_end(args);
    return aniResult;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Float_V(ani_env *env, ani_object object, ani_method method,
                                                           ani_float *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::FLOAT);
    return GeneralMethodCall<EtsFloat>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Float(ani_env *env, ani_object object, ani_method method,
                                                         ani_float *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethod_Float_V(env, object, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Float_A(ani_env *env, ani_object object, ani_method method,
                                                           ani_float *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::FLOAT);
    return GeneralMethodCall<EtsFloat>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Double_V(ani_env *env, ani_object object, ani_method method,
                                                            ani_double *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::DOUBLE);
    return GeneralMethodCall<EtsDouble>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Double(ani_env *env, ani_object object, ani_method method,
                                                          ani_double *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethod_Double_V(env, object, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Double_A(ani_env *env, ani_object object, ani_method method,
                                                            ani_double *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::DOUBLE);
    return GeneralMethodCall<EtsDouble>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Ref_V(ani_env *env, ani_object object, ani_method method,
                                                         ani_ref *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);

    CheckMethodReturnType(method, EtsType::OBJECT);
    return GeneralMethodCall<ani_ref>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Ref(ani_env *env, ani_object object, ani_method method,
                                                       ani_ref *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethod_Ref_V(env, object, method, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Ref_A(ani_env *env, ani_object object, ani_method method,
                                                         ani_ref *result, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::OBJECT);
    return GeneralMethodCall<ani_ref>(env, object, method, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Void_A(ani_env *env, ani_object object, ani_method method,
                                                          const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);
    CHECK_PTR_ARG(args);

    CheckMethodReturnType(method, EtsType::VOID);
    ani_boolean result;
    // Use any primitive type as template parameter and just ignore the result
    return GeneralMethodCall<EtsBoolean>(env, object, method, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Void_V(ani_env *env, ani_object object, ani_method method,
                                                          va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(method);

    CheckMethodReturnType(method, EtsType::VOID);
    ani_boolean result;
    // Use any primitive type as template parameter and just ignore the result
    return GeneralMethodCall<EtsBoolean>(env, object, method, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Void(ani_env *env, ani_object object, ani_method method, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, method);
    ani_status status = Object_CallMethod_Void_V(env, object, method, args);
    va_end(args);
    return status;
}

template <typename R>
static ani_status DoVariableGetValue(ani_env *env, ani_variable variable, R *result)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(variable);
    CHECK_PTR_ARG(result);

    static constexpr auto IS_REF = std::is_same_v<R, ani_ref>;
    using Res = std::conditional_t<IS_REF, EtsObject *, R>;

    ScopedManagedCodeFix s(env);
    EtsVariable *internalVariable = ToInternalVariable(variable);
    EtsField *field = internalVariable->AsField();
    ANI_CHECK_RETURN_IF_NE(field->GetEtsType(), AniTypeInfo<R>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
    EtsClass *cls = field->GetDeclaringClass();
    Res etsRes {};
    if constexpr (IS_REF) {
        etsRes = cls->GetStaticFieldObject(field);
        return s.AddLocalRef(etsRes, result);
    } else {
        etsRes = cls->GetStaticFieldPrimitive<R>(field);
        *result = etsRes;
        return ANI_OK;
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Boolean(ani_env *env, ani_variable variable, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Char(ani_env *env, ani_variable variable, ani_char *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Byte(ani_env *env, ani_variable variable, ani_byte *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Short(ani_env *env, ani_variable variable, ani_short *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Int(ani_env *env, ani_variable variable, ani_int *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Long(ani_env *env, ani_variable variable, ani_long *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Float(ani_env *env, ani_variable variable, ani_float *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Double(ani_env *env, ani_variable variable, ani_double *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Ref(ani_env *env, ani_variable variable, ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    return DoVariableGetValue(env, variable, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Boolean_A(ani_env *env, ani_object object, const char *name,
                                                                   const char *signature, ani_boolean *result,
                                                                   const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<EtsBoolean, EtsType::BOOLEAN>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Boolean_V(ani_env *env, ani_object object, const char *name,
                                                                   const char *signature, ani_boolean *result,
                                                                   va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ObjectCallMethodByName<EtsBoolean, EtsType::BOOLEAN>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Boolean(ani_env *env, ani_object object, const char *name,
                                                                 const char *signature, ani_boolean *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Boolean_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Char_A(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, ani_char *result,
                                                                const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<EtsChar, EtsType::CHAR>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Char_V(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, ani_char *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    return ObjectCallMethodByName<EtsChar, EtsType::CHAR>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Char(ani_env *env, ani_object object, const char *name,
                                                              const char *signature, ani_char *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Char_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Byte_A(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, ani_byte *result,
                                                                const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<EtsByte, EtsType::BYTE>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Byte_V(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, ani_byte *result, va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ObjectCallMethodByName<EtsByte, EtsType::BYTE>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Byte(ani_env *env, ani_object object, const char *name,
                                                              const char *signature, ani_byte *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Byte_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Short_A(ani_env *env, ani_object object, const char *name,
                                                                 const char *signature, ani_short *result,
                                                                 const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<EtsShort, EtsType::SHORT>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Short_V(ani_env *env, ani_object object, const char *name,
                                                                 const char *signature, ani_short *result, va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ObjectCallMethodByName<EtsShort, EtsType::SHORT>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Short(ani_env *env, ani_object object, const char *name,
                                                               const char *signature, ani_short *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Short_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Int_A(ani_env *env, ani_object object, const char *name,
                                                               const char *signature, ani_int *result,
                                                               const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<EtsInt, EtsType::INT>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Int_V(ani_env *env, ani_object object, const char *name,
                                                               const char *signature, ani_int *result, va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ObjectCallMethodByName<EtsInt, EtsType::INT>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Int(ani_env *env, ani_object object, const char *name,
                                                             const char *signature, ani_int *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Int_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Long_A(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, ani_long *result,
                                                                const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<EtsLong, EtsType::LONG>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Long_V(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, ani_long *result, va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ObjectCallMethodByName<EtsLong, EtsType::LONG>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Long(ani_env *env, ani_object object, const char *name,
                                                              const char *signature, ani_long *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Long_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Float_A(ani_env *env, ani_object object, const char *name,
                                                                 const char *signature, ani_float *result,
                                                                 const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<EtsFloat, EtsType::FLOAT>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Float_V(ani_env *env, ani_object object, const char *name,
                                                                 const char *signature, ani_float *result, va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ObjectCallMethodByName<EtsFloat, EtsType::FLOAT>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Float(ani_env *env, ani_object object, const char *name,
                                                               const char *signature, ani_float *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Float_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Double_A(ani_env *env, ani_object object, const char *name,
                                                                  const char *signature, ani_double *result,
                                                                  const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<EtsDouble, EtsType::DOUBLE>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Double_V(ani_env *env, ani_object object, const char *name,
                                                                  const char *signature, ani_double *result,
                                                                  va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ObjectCallMethodByName<EtsDouble, EtsType::DOUBLE>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Double(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, ani_double *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Double_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Ref_A(ani_env *env, ani_object object, const char *name,
                                                               const char *signature, ani_ref *result,
                                                               const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    return ObjectCallMethodByName<ani_ref, EtsType::OBJECT>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Ref_V(ani_env *env, ani_object object, const char *name,
                                                               const char *signature, ani_ref *result, va_list args)
{
    ANI_DEBUG_TRACE(env);

    return ObjectCallMethodByName<ani_ref, EtsType::OBJECT>(env, object, name, signature, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Ref(ani_env *env, ani_object object, const char *name,
                                                             const char *signature, ani_ref *result, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_CallMethodByName_Ref_V(env, object, name, signature, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Void_A(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);

    CHECK_PTR_ARG(args);
    ani_boolean result;
    return ObjectCallMethodByName<EtsBoolean, EtsType::VOID>(env, object, name, signature, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Void_V(ani_env *env, ani_object object, const char *name,
                                                                const char *signature, va_list args)
{
    ANI_DEBUG_TRACE(env);

    ani_boolean result;
    return ObjectCallMethodByName<EtsBoolean, EtsType::VOID>(env, object, name, signature, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Void(ani_env *env, ani_object object, const char *name,
                                                              const char *signature, ...)
{
    ANI_DEBUG_TRACE(env);
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, signature);
    ani_status status = Object_CallMethodByName_Void_V(env, object, name, signature, args);
    va_end(args);
    return status;
}

static ani_size DoGetTupleLength(EtsCoroutine *coroutine, EtsHandle<EtsObject> internalTuple)
{
    if (UNLIKELY(PlatformTypes(coroutine)->coreTupleN->IsAssignableFrom(internalTuple->GetClass()))) {
        EtsClass *klass = internalTuple->GetClass();
        EtsField *field = klass->GetFieldIDByName("$tupleValues", nullptr);
        ASSERT(field != nullptr);

        auto *valuesArray = internalTuple->GetFieldObject(field);
        ASSERT(valuesArray->GetClass()->GetRuntimeClass()->IsObjectArrayClass());

        auto *objectArray = EtsObjectArray::FromCoreType(valuesArray->GetCoreType());
        return objectArray->GetLength();
    }

    return internalTuple->GetClass()->GetFieldsNumber();
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetNumberOfItems(ani_env *env, ani_tuple_value tupleValue, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(tupleValue);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    auto *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> internalTuple(coroutine, s.ToInternalType(tupleValue));

    *result = DoGetTupleLength(coroutine, internalTuple);

    return ANI_OK;
}

template <typename ValueType>
static bool IsCorrectBoxType(EtsCoroutine *coro, EtsObject *element)
{
    ASSERT(element != nullptr);
    ASSERT(element->GetClass() != nullptr);
    auto *boxPrimitiveClass = EtsBoxPrimitive<ValueType>::GetBoxClass(coro);
    return element->GetClass()->GetRuntimeClass() == boxPrimitiveClass;
}

template <typename ValueType>
static ani_status TupleValueGetItem(ani_env *env, ani_object tupleValue, ani_size index, ValueType *result)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(tupleValue);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    auto coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> internalTuple(coroutine, s.ToInternalType(tupleValue));
    ANI_CHECK_RETURN_IF_GE(index, DoGetTupleLength(coroutine, internalTuple), ANI_OUT_OF_RANGE);

    EtsClass *klass = internalTuple->GetClass();
    // NOTE (#24962): Extend implementation to TupleN, when FE supports it
    EtsField *field = klass->GetFieldIDByName(("$" + std::to_string(index)).c_str(), nullptr);
    ASSERT(field != nullptr);

    auto *resultField = internalTuple->GetFieldObject(field);
    ANI_CHECK_RETURN_IF_EQ(IsCorrectBoxType<ValueType>(coroutine, resultField), false, ANI_INVALID_TYPE);

    EtsBoxPrimitive<ValueType> *boxedVal = EtsBoxPrimitive<ValueType>::FromCoreType(resultField);
    auto primitiveValue = boxedVal->GetValue();
    *result = primitiveValue;

    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Boolean(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                            ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueGetItem<ani_boolean>(env, tupleValue, index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Char(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                         ani_char *result)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueGetItem<ani_char>(env, tupleValue, index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Byte(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                         ani_byte *result)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueGetItem<ani_byte>(env, tupleValue, index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Short(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                          ani_short *result)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueGetItem<ani_short>(env, tupleValue, index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Int(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                        ani_int *result)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueGetItem<ani_int>(env, tupleValue, index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Long(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                         ani_long *result)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueGetItem<ani_long>(env, tupleValue, index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Float(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                          ani_float *result)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueGetItem<ani_float>(env, tupleValue, index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Double(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                           ani_double *result)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueGetItem<ani_double>(env, tupleValue, index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Ref(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                        ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(tupleValue);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    auto coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);

    EtsHandle<EtsObject> internalTuple(coroutine, s.ToInternalType(tupleValue));
    ANI_CHECK_RETURN_IF_GE(index, DoGetTupleLength(coroutine, internalTuple), ANI_OUT_OF_RANGE);

    EtsClass *klass = internalTuple->GetClass();
    // NOTE (#24962): Extend implementation to TupleN, when FE supports it
    EtsField *field = klass->GetFieldIDByName(("$" + std::to_string(index)).c_str(), nullptr);
    ASSERT(field != nullptr);

    auto element = internalTuple->GetFieldObject(field);
    return s.AddLocalRef(element, result);
}

template <typename ValueType>
static ani_status TupleValueSetItem(ani_env *env, ani_tuple_value tupleValue, ani_size index, ValueType value)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(tupleValue);

    ScopedManagedCodeFix s(env);
    auto coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);

    EtsHandle<EtsObject> internalTuple(coroutine, s.ToInternalType(tupleValue));
    ANI_CHECK_RETURN_IF_GE(index, DoGetTupleLength(coroutine, internalTuple), ANI_OUT_OF_RANGE);

    EtsHandle<EtsObject> boxed(coroutine, EtsBoxPrimitive<ValueType>::Create(coroutine, value));
    ANI_CHECK_RETURN_IF_EQ(boxed.GetPtr(), nullptr, ANI_OUT_OF_MEMORY);

    EtsHandle<EtsClass> klass(coroutine, internalTuple->GetClass());
    // NOTE (#24962): Extend implementation to TupleN, when FE supports it
    EtsField *field = klass->GetFieldIDByName(("$" + std::to_string(index)).c_str(), nullptr);
    ASSERT(field != nullptr);

    internalTuple->SetFieldObject(field, boxed.GetPtr());

    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Boolean(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                            ani_boolean value)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueSetItem<ani_boolean>(env, tupleValue, index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Char(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                         ani_char value)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueSetItem<ani_char>(env, tupleValue, index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Byte(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                         ani_byte value)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueSetItem<ani_byte>(env, tupleValue, index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Short(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                          ani_short value)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueSetItem<ani_short>(env, tupleValue, index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Int(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                        ani_int value)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueSetItem<ani_int>(env, tupleValue, index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Long(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                         ani_long value)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueSetItem<ani_long>(env, tupleValue, index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Float(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                          ani_float value)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueSetItem<ani_float>(env, tupleValue, index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Double(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                           ani_double value)
{
    ANI_DEBUG_TRACE(env);
    return TupleValueSetItem<ani_double>(env, tupleValue, index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Ref(ani_env *env, ani_tuple_value tupleValue, ani_size index,
                                                        ani_ref value)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(tupleValue);
    CHECK_PTR_ARG(value);

    ScopedManagedCodeFix s(env);
    auto coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);

    EtsHandle<EtsObject> internalTuple(coroutine, s.ToInternalType(tupleValue));
    ANI_CHECK_RETURN_IF_GE(index, DoGetTupleLength(coroutine, internalTuple), ANI_OUT_OF_RANGE);

    EtsHandle<EtsClass> klass(coroutine, internalTuple->GetClass());
    // NOTE (#24962): Extend implementation to TupleN, when FE supports it
    EtsField *field = klass->GetFieldIDByName(("$" + std::to_string(index)).c_str(), nullptr);
    ASSERT(field != nullptr);

    EtsObject *etsValue = s.ToInternalType(value);
    internalTuple->SetFieldObject(field, etsValue);

    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Short_A(ani_env *env, ani_function fn, ani_short *result,
                                                       const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::SHORT);
    return GeneralFunctionCall<ani_short>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Short_V(ani_env *env, ani_function fn, ani_short *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::SHORT);
    return GeneralFunctionCall<ani_short>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Short(ani_env *env, ani_function fn, ani_short *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Short_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Int_A(ani_env *env, ani_function fn, ani_int *result,
                                                     const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::INT);
    return GeneralFunctionCall<ani_int>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Int_V(ani_env *env, ani_function fn, ani_int *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::INT);
    return GeneralFunctionCall<ani_int>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Int(ani_env *env, ani_function fn, ani_int *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Int_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Long_A(ani_env *env, ani_function fn, ani_long *result,
                                                      const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::LONG);
    return GeneralFunctionCall<ani_long>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Long_V(ani_env *env, ani_function fn, ani_long *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::LONG);
    return GeneralFunctionCall<ani_long>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Long(ani_env *env, ani_function fn, ani_long *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Long_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Float_A(ani_env *env, ani_function fn, ani_float *result,
                                                       const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::FLOAT);
    return GeneralFunctionCall<EtsFloat>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Float_V(ani_env *env, ani_function fn, ani_float *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::FLOAT);
    return GeneralFunctionCall<EtsFloat>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Float(ani_env *env, ani_function fn, ani_float *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Float_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Boolean_A(ani_env *env, ani_function fn, ani_boolean *result,
                                                         const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::BOOLEAN);
    return GeneralFunctionCall<EtsBoolean>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Boolean_V(ani_env *env, ani_function fn, ani_boolean *result,
                                                         va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::BOOLEAN);
    return GeneralFunctionCall<EtsBoolean>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Boolean(ani_env *env, ani_function fn, ani_boolean *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Boolean_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Char_A(ani_env *env, ani_function fn, ani_char *result,
                                                      const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::CHAR);
    return GeneralFunctionCall<EtsChar>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Char_V(ani_env *env, ani_function fn, ani_char *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::CHAR);
    return GeneralFunctionCall<EtsChar>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Char(ani_env *env, ani_function fn, ani_char *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Char_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Byte_A(ani_env *env, ani_function fn, ani_byte *result,
                                                      const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::BYTE);
    return GeneralFunctionCall<EtsByte>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Byte_V(ani_env *env, ani_function fn, ani_byte *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::BYTE);
    return GeneralFunctionCall<EtsByte>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Byte(ani_env *env, ani_function fn, ani_byte *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Byte_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Double_A(ani_env *env, ani_function fn, ani_double *result,
                                                        const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::DOUBLE);
    return GeneralFunctionCall<EtsDouble>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Double_V(ani_env *env, ani_function fn, ani_double *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::DOUBLE);
    return GeneralFunctionCall<EtsDouble>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Double(ani_env *env, ani_function fn, ani_double *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Double_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Ref_A(ani_env *env, ani_function fn, ani_ref *result,
                                                     const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::OBJECT);
    return GeneralFunctionCall<ani_ref>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Ref_V(ani_env *env, ani_function fn, ani_ref *result, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(result);

    CheckFunctionReturnType(fn, EtsType::OBJECT);
    return GeneralFunctionCall<ani_ref>(env, fn, result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Ref(ani_env *env, ani_function fn, ani_ref *result, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Function_Call_Ref_V(env, fn, result, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Void_A(ani_env *env, ani_function fn, const ani_value *args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);
    CHECK_PTR_ARG(args);

    CheckFunctionReturnType(fn, EtsType::VOID);
    ani_boolean result;
    // Use any primitive type as template parameter and just ignore the result
    return GeneralFunctionCall<EtsBoolean>(env, fn, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Void_V(ani_env *env, ani_function fn, va_list args)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(fn);

    CheckFunctionReturnType(fn, EtsType::VOID);
    ani_boolean result;
    // Use any primitive type as template parameter and just ignore the result
    return GeneralFunctionCall<EtsBoolean>(env, fn, &result, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Void(ani_env *env, ani_function fn, ...)
{
    ANI_DEBUG_TRACE(env);

    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, fn);
    ani_status status = Function_Call_Void_V(env, fn, args);
    va_end(args);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GlobalReference_Create(ani_env *env, ani_ref ref, ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    return s.AddGlobalRef(ref, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GlobalReference_Delete(ani_env *env, ani_ref gref)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    return s.DelGlobalRef(gref);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status WeakReference_Create(ani_env *env, ani_ref ref, ani_wref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    return s.AddWeakRef(ref, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status WeakReference_Delete(ani_env *env, ani_wref wref)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    return s.DelWeakRef(wref);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status WeakReference_GetReference(ani_env *env, ani_wref wref, ani_boolean *wasReleasedResult,
                                                            ani_ref *refResult)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(wasReleasedResult);
    CHECK_PTR_ARG(refResult);

    ScopedManagedCodeFix s(env);
    return s.GetLocalRef(wref, wasReleasedResult, refResult);
}

template <bool IS_EXTERNAL>
static ani_status DoCreateArrayBuffer(ani_env *env, [[maybe_unused]] void *externalData, size_t length,
                                      [[maybe_unused]] void **resultData, ani_arraybuffer *resultBuffer)
{
    CHECK_PTR_ARG(resultBuffer);
    ANI_CHECK_RETURN_IF_GT(length, static_cast<size_t>(std::numeric_limits<ani_int>::max()), ANI_INVALID_ARGS);

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coro = s.GetCoroutine();

    EtsEscompatArrayBuffer *internalArrayBuffer = nullptr;
    if constexpr (IS_EXTERNAL) {
        internalArrayBuffer = EtsEscompatArrayBuffer::Create(coro, externalData, length, nullptr, nullptr);
    } else {
        internalArrayBuffer = EtsEscompatArrayBuffer::Create(coro, length, resultData);
    }

    ASSERT(coro->HasPendingException() == false);
    ANI_CHECK_RETURN_IF_EQ(internalArrayBuffer, nullptr, ANI_OUT_OF_MEMORY);

    ani_ref arrayObject = nullptr;
    ani_status ret = s.AddLocalRef(internalArrayBuffer, &arrayObject);
    ANI_CHECK_RETURN_IF_NE(ret, ANI_OK, ret);
    ASSERT(arrayObject != nullptr);

    *resultBuffer = reinterpret_cast<ani_arraybuffer>(arrayObject);
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
ani_status CreateArrayBuffer(ani_env *env, size_t length, void **dataResult, ani_arraybuffer *arraybufferResult)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(dataResult);
    CHECK_PTR_ARG(arraybufferResult);

    return DoCreateArrayBuffer<false>(env, nullptr, length, dataResult, arraybufferResult);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status ArrayBuffer_GetInfo(ani_env *env, ani_arraybuffer arraybuffer, void **dataResult,
                                                     size_t *lengthResult)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(arraybuffer);
    CHECK_PTR_ARG(dataResult);
    CHECK_PTR_ARG(lengthResult);

    ScopedManagedCodeFix s(env);
    auto *internalArrayBuffer = s.ToInternalType(arraybuffer);
    *dataResult = internalArrayBuffer->GetData();
    *lengthResult = internalArrayBuffer->GetByteLength();
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnsureEnoughReferences(ani_env *env, ani_size nrRefs)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    return s.EnsureLocalEnoughRefs(nrRefs);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status CreateLocalScope(ani_env *env, ani_size nrRefs)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    return s.CreateLocalScope(nrRefs);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE ani_status DestroyLocalScope(ani_env *env)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    return s.DestroyLocalScope();
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status CreateEscapeLocalScope(ani_env *env, ani_size nrRefs)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    return s.CreateEscapeLocalScope(nrRefs);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status DestroyEscapeLocalScope(ani_env *env, ani_ref ref, ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    return s.DestroyEscapeLocalScope(ref, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FindEnum(ani_env *env, const char *enumDescriptor, ani_enum *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(enumDescriptor);
    CHECK_PTR_ARG(result);

    // NOTE: Check that result is enum, #22400
    return DoFind<false>(env, enumDescriptor, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_FindEnum(ani_env *env, ani_namespace ns, const char *enumDescriptor,
                                                    ani_enum *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(ns);
    CHECK_PTR_ARG(enumDescriptor);
    CHECK_PTR_ARG(result);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(env);
    EtsNamespace *etsNs;
    ani_status status = GetInternalNamespace(s, ns, &etsNs);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    PandaString enumDescriptorPandStr = Mangle::ConvertDescriptor(enumDescriptor);
    if (!enumDescriptorPandStr.empty() && enumDescriptorPandStr[0] == 'L') {
        enumDescriptorPandStr[0] = '/';
    } else {
        return ANI_NOT_FOUND;
    }

    PandaString etsNsDescriptorPandStr(etsNs->AsClass()->GetDescriptor());
    etsNsDescriptorPandStr.pop_back();
    etsNsDescriptorPandStr += enumDescriptorPandStr;

    // NOTE: Check that result is enum, #22400
    return DoFind<false>(pandaEnv, etsNsDescriptorPandStr.c_str(), s, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_FindEnum(ani_env *env, ani_module module, const char *enumDescriptor,
                                                 ani_enum *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(module);
    CHECK_PTR_ARG(enumDescriptor);
    CHECK_PTR_ARG(result);

    // NOTE: Check that result is enum, #22400
    return FindInModule<false>(env, module, enumDescriptor, result);
}

static ani_status GetArrayFromEnum(ScopedManagedCodeFix &s, ani_enum enm, const char *name, EtsObjectArray **result)
{
    EtsField *field = nullptr;
    ani_status status = DoGetField<true>(s, reinterpret_cast<ani_class>(enm), name, &field);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    EtsClass *cls = field->GetDeclaringClass();
    EtsObject *etsObject = cls->GetStaticFieldObject(field);
    *result = EtsObjectArray::FromCoreType(etsObject->GetCoreType());
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Enum_GetEnumItemByName(ani_env *env, ani_enum enm, const char *name,
                                                        ani_enum_item *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(enm);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    // Get index, then call unified function for Enum_GetEnumItemByIndex
    ScopedManagedCodeFix s(env);
    EtsObjectArray *namesArr;
    ani_status status = GetArrayFromEnum(s, enm, EnumArrayNames::NAMES.data(), &namesArr);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    size_t sz = namesArr->GetLength();

    size_t index = 0;
    for (; index < sz; ++index) {
        EtsObject *etsElem = namesArr->Get(index);
        EtsString *etsStr = EtsString::FromEtsObject(etsElem);
        if (etsStr->IsEqual(name)) {
            break;
        }
    }
    if (index >= sz) {
        return ANI_NOT_FOUND;
    }

    EtsObjectArray *itemsArr;
    status = GetArrayFromEnum(s, enm, EnumArrayNames::BOXED_ITEMS.data(), &itemsArr);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    EtsObject *res = itemsArr->Get(index);

    return s.AddLocalRef(res, reinterpret_cast<ani_ref *>(result));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Enum_GetEnumItemByIndex(ani_env *env, ani_enum enm, ani_size index,
                                                         ani_enum_item *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(enm);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsObjectArray *itemsArr;
    ani_status status = GetArrayFromEnum(s, enm, EnumArrayNames::BOXED_ITEMS.data(), &itemsArr);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    if (index >= itemsArr->GetLength()) {
        return ANI_NOT_FOUND;
    }
    EtsObject *res = itemsArr->Get(index);

    return s.AddLocalRef(res, reinterpret_cast<ani_ref *>(result));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetEnum(ani_env *env, ani_enum_item enum_item, ani_enum *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(enum_item);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsObject *internalEnumItem = s.ToInternalType(enum_item);
    EtsClass *enumClass = internalEnumItem->GetClass();

    return s.AddLocalRef(reinterpret_cast<EtsObject *>(enumClass), reinterpret_cast<ani_ref *>(result));
}

template <typename T>
static T *GetArrayFromInternalEnum(EtsHandle<EtsClass> enumClass, const char *name)
{
    EtsField *field = enumClass->GetStaticFieldIDByName(name, nullptr);
    if (field == nullptr) {
        return nullptr;
    }

    EtsClass *cls = field->GetDeclaringClass();
    EtsObject *etsObject = cls->GetStaticFieldObject(field);
    return reinterpret_cast<T *>(etsObject->GetCoreType());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetValue_Int(ani_env *env, ani_enum_item enum_item, ani_int *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(enum_item);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> internalEnumItem(coroutine, s.ToInternalType(enum_item));
    EtsHandle<EtsClass> enumClass(coroutine, internalEnumItem->GetClass());
    EtsField *etsField = enumClass->GetFieldIDByName("#ordinal", nullptr);
    auto ordinal = internalEnumItem->GetFieldPrimitive<int32_t>(etsField);

    auto *itemsArr = GetArrayFromInternalEnum<EtsIntArray>(enumClass, EnumArrayNames::VALUES.data());
    // If enum values are strings, there is no ValuesArray array, so incorrect call to method
    if (itemsArr == nullptr) {
        return ANI_INVALID_ARGS;
    }
    *result = itemsArr->Get(ordinal);
    return ANI_OK;
}

static ani_status GetStringArrayFromEnumItem(ani_env *env, ani_enum_item enumItem, const char *arrayName,
                                             ani_string *result)
{
    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> internalEnumItem(coroutine, s.ToInternalType(enumItem));
    EtsHandle<EtsClass> enumClass(coroutine, internalEnumItem->GetClass());
    EtsField *etsField = enumClass->GetFieldIDByName("#ordinal", nullptr);
    auto ordinal = internalEnumItem->GetFieldPrimitive<int32_t>(etsField);

    auto *itemsArr = GetArrayFromInternalEnum<EtsObjectArray>(enumClass, arrayName);
    EtsObject *stringObj = itemsArr->Get(ordinal);
    return s.AddLocalRef(stringObj, reinterpret_cast<ani_ref *>(result));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetValue_String(ani_env *env, ani_enum_item enum_item, ani_string *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(enum_item);
    CHECK_PTR_ARG(result);

    return GetStringArrayFromEnumItem(env, enum_item, EnumArrayNames::STRING_VALUES.data(), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetName(ani_env *env, ani_enum_item enum_item, ani_string *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(enum_item);
    CHECK_PTR_ARG(result);

    return GetStringArrayFromEnumItem(env, enum_item, EnumArrayNames::NAMES.data(), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetIndex(ani_env *env, ani_enum_item enum_item, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(enum_item);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> internalEnumItem(coroutine, s.ToInternalType(enum_item));
    EtsHandle<EtsClass> enumClass(coroutine, internalEnumItem->GetClass());
    EtsField *etsField = enumClass->GetFieldIDByName("#ordinal", nullptr);
    auto ordinal = internalEnumItem->GetFieldPrimitive<int32_t>(etsField);

    *result = ordinal;
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Promise_New(ani_env *env, ani_resolver *resultResolver, ani_object *resultPromise)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(resultResolver);
    CHECK_PTR_ARG(resultPromise);

    ScopedManagedCodeFix s(env);
    EtsPromise *promise = EtsPromise::Create(s.GetCoroutine());
    ANI_CHECK_RETURN_IF_EQ(promise, nullptr, ANI_OUT_OF_MEMORY);
    ani_status status = s.AddLocalRef(promise->AsObject(), reinterpret_cast<ani_ref *>(resultPromise));
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    return s.AddGlobalRef(promise->AsObject(), reinterpret_cast<ani_ref *>(resultResolver));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status PromiseResolver_Resolve(ani_env *env, ani_resolver resolver, ani_ref resolution)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    EtsPromise *promise = s.ToInternalType(resolver);
    ANI_CHECK_RETURN_IF_EQ(promise, nullptr, ANI_INVALID_ARGS);
    ani_status status = s.DelGlobalRef(reinterpret_cast<ani_ref>(resolver));
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    EtsObject *value = s.ToInternalType(resolution);
    promise->Resolve(s.GetCoroutine(), value);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status PromiseResolver_Reject(ani_env *env, ani_resolver resolver, ani_error rejection)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);

    ScopedManagedCodeFix s(env);
    EtsPromise *promise = s.ToInternalType(resolver);
    ANI_CHECK_RETURN_IF_EQ(promise, nullptr, ANI_INVALID_ARGS);
    ani_status status = s.DelGlobalRef(reinterpret_cast<ani_ref>(resolver));
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    EtsObject *error = s.ToInternalType(rejection);
    promise->Reject(s.GetCoroutine(), error);
    return ANI_OK;
}

[[noreturn]] static void NotImplementedAPI(int nr)
{
    LOG(FATAL, ANI) << "Not implemented interaction_api, nr=" << nr;
    UNREACHABLE();
}

template <int NR, typename R, typename... Args>
static R NotImplementedAdapter([[maybe_unused]] Args... args)
{
    NotImplementedAPI(NR);
}

template <int NR, typename R, typename... Args>
static R NotImplementedAdapterVargs([[maybe_unused]] Args... args, ...)
{
    NotImplementedAPI(NR);
}

// clang-format off
const __ani_interaction_api INTERACTION_API = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    GetVersion,
    GetVM,
    Object_New,
    Object_New_A,
    Object_New_V,
    Object_GetType,
    Object_InstanceOf,
    Type_GetSuperClass,
    Type_IsAssignableFrom,
    FindModule,
    FindNamespace,
    FindClass,
    FindEnum,
    Module_FindNamespace,
    Module_FindClass,
    Module_FindEnum,
    Module_FindFunction,
    Module_FindVariable,
    Namespace_FindNamespace,
    Namespace_FindClass,
    Namespace_FindEnum,
    Namespace_FindFunction,
    Namespace_FindVariable,
    Module_BindNativeFunctions,
    Namespace_BindNativeFunctions,
    Class_BindNativeMethods,
    Reference_Delete,
    EnsureEnoughReferences,
    CreateLocalScope,
    DestroyLocalScope,
    CreateEscapeLocalScope,
    DestroyEscapeLocalScope,
    ThrowError,
    ExistUnhandledError,
    GetUnhandledError,
    ResetError,
    DescribeError,
    Abort,
    GetNull,
    GetUndefined,
    Reference_IsNull,
    Reference_IsUndefined,
    Reference_IsNullishValue,
    Reference_Equals,
    Reference_StrictEquals,
    String_NewUTF16,
    String_GetUTF16Size,
    String_GetUTF16,
    String_GetUTF16SubString,
    String_NewUTF8,
    String_GetUTF8Size,
    String_GetUTF8,
    String_GetUTF8SubString,
    Array_GetLength,
    Array_New_Boolean,
    Array_New_Char,
    Array_New_Byte,
    Array_New_Short,
    Array_New_Int,
    Array_New_Long,
    Array_New_Float,
    Array_New_Double,
    Array_GetRegion_Boolean,
    Array_GetRegion_Char,
    Array_GetRegion_Byte,
    Array_GetRegion_Short,
    Array_GetRegion_Int,
    Array_GetRegion_Long,
    Array_GetRegion_Float,
    Array_GetRegion_Double,
    Array_SetRegion_Boolean,
    Array_SetRegion_Char,
    Array_SetRegion_Byte,
    Array_SetRegion_Short,
    Array_SetRegion_Int,
    Array_SetRegion_Long,
    Array_SetRegion_Float,
    Array_SetRegion_Double,
    Array_New_Ref,
    Array_Set_Ref,
    Array_Get_Ref,
    Enum_GetEnumItemByName,
    Enum_GetEnumItemByIndex,
    EnumItem_GetEnum,
    EnumItem_GetValue_Int,
    EnumItem_GetValue_String,
    EnumItem_GetName,
    EnumItem_GetIndex,
    FunctionalObject_Call,
    Variable_SetValue_Boolean,
    Variable_SetValue_Char,
    Variable_SetValue_Byte,
    Variable_SetValue_Short,
    Variable_SetValue_Int,
    Variable_SetValue_Long,
    Variable_SetValue_Float,
    Variable_SetValue_Double,
    Variable_SetValue_Ref,
    Variable_GetValue_Boolean,
    Variable_GetValue_Char,
    Variable_GetValue_Byte,
    Variable_GetValue_Short,
    Variable_GetValue_Int,
    Variable_GetValue_Long,
    Variable_GetValue_Float,
    Variable_GetValue_Double,
    Variable_GetValue_Ref,
    Function_Call_Boolean,
    Function_Call_Boolean_A,
    Function_Call_Boolean_V,
    Function_Call_Char,
    Function_Call_Char_A,
    Function_Call_Char_V,
    Function_Call_Byte,
    Function_Call_Byte_A,
    Function_Call_Byte_V,
    Function_Call_Short,
    Function_Call_Short_A,
    Function_Call_Short_V,
    Function_Call_Int,
    Function_Call_Int_A,
    Function_Call_Int_V,
    Function_Call_Long,
    Function_Call_Long_A,
    Function_Call_Long_V,
    Function_Call_Float,
    Function_Call_Float_A,
    Function_Call_Float_V,
    Function_Call_Double,
    Function_Call_Double_A,
    Function_Call_Double_V,
    Function_Call_Ref,
    Function_Call_Ref_A,
    Function_Call_Ref_V,
    Function_Call_Void,
    Function_Call_Void_A,
    Function_Call_Void_V,
    Class_FindField,
    Class_FindStaticField,
    Class_FindMethod,
    Class_FindStaticMethod,
    Class_FindSetter,
    Class_FindGetter,
    Class_FindIndexableGetter,
    Class_FindIndexableSetter,
    Class_FindIterator,
    Class_GetStaticField_Boolean,
    Class_GetStaticField_Char,
    Class_GetStaticField_Byte,
    Class_GetStaticField_Short,
    Class_GetStaticField_Int,
    Class_GetStaticField_Long,
    Class_GetStaticField_Float,
    Class_GetStaticField_Double,
    Class_GetStaticField_Ref,
    Class_SetStaticField_Boolean,
    Class_SetStaticField_Char,
    Class_SetStaticField_Byte,
    Class_SetStaticField_Short,
    Class_SetStaticField_Int,
    Class_SetStaticField_Long,
    Class_SetStaticField_Float,
    Class_SetStaticField_Double,
    Class_SetStaticField_Ref,
    Class_GetStaticFieldByName_Boolean,
    Class_GetStaticFieldByName_Char,
    Class_GetStaticFieldByName_Byte,
    Class_GetStaticFieldByName_Short,
    Class_GetStaticFieldByName_Int,
    Class_GetStaticFieldByName_Long,
    Class_GetStaticFieldByName_Float,
    Class_GetStaticFieldByName_Double,
    Class_GetStaticFieldByName_Ref,
    Class_SetStaticFieldByName_Boolean,
    Class_SetStaticFieldByName_Char,
    Class_SetStaticFieldByName_Byte,
    Class_SetStaticFieldByName_Short,
    Class_SetStaticFieldByName_Int,
    Class_SetStaticFieldByName_Long,
    Class_SetStaticFieldByName_Float,
    Class_SetStaticFieldByName_Double,
    Class_SetStaticFieldByName_Ref,
    Class_CallStaticMethod_Boolean,
    Class_CallStaticMethod_Boolean_A,
    Class_CallStaticMethod_Boolean_V,
    Class_CallStaticMethod_Char,
    Class_CallStaticMethod_Char_A,
    Class_CallStaticMethod_Char_V,
    Class_CallStaticMethod_Byte,
    Class_CallStaticMethod_Byte_A,
    Class_CallStaticMethod_Byte_V,
    Class_CallStaticMethod_Short,
    Class_CallStaticMethod_Short_A,
    Class_CallStaticMethod_Short_V,
    Class_CallStaticMethod_Int,
    Class_CallStaticMethod_Int_A,
    Class_CallStaticMethod_Int_V,
    Class_CallStaticMethod_Long,
    Class_CallStaticMethod_Long_A,
    Class_CallStaticMethod_Long_V,
    Class_CallStaticMethod_Float,
    Class_CallStaticMethod_Float_A,
    Class_CallStaticMethod_Float_V,
    Class_CallStaticMethod_Double,
    Class_CallStaticMethod_Double_A,
    Class_CallStaticMethod_Double_V,
    Class_CallStaticMethod_Ref,
    Class_CallStaticMethod_Ref_A,
    Class_CallStaticMethod_Ref_V,
    Class_CallStaticMethod_Void,
    Class_CallStaticMethod_Void_A,
    Class_CallStaticMethod_Void_V,
    Class_CallStaticMethodByName_Boolean,
    Class_CallStaticMethodByName_Boolean_A,
    Class_CallStaticMethodByName_Boolean_V,
    Class_CallStaticMethodByName_Char,
    Class_CallStaticMethodByName_Char_A,
    Class_CallStaticMethodByName_Char_V,
    Class_CallStaticMethodByName_Byte,
    Class_CallStaticMethodByName_Byte_A,
    Class_CallStaticMethodByName_Byte_V,
    Class_CallStaticMethodByName_Short,
    Class_CallStaticMethodByName_Short_A,
    Class_CallStaticMethodByName_Short_V,
    Class_CallStaticMethodByName_Int,
    Class_CallStaticMethodByName_Int_A,
    Class_CallStaticMethodByName_Int_V,
    Class_CallStaticMethodByName_Long,
    Class_CallStaticMethodByName_Long_A,
    Class_CallStaticMethodByName_Long_V,
    Class_CallStaticMethodByName_Float,
    Class_CallStaticMethodByName_Float_A,
    Class_CallStaticMethodByName_Float_V,
    Class_CallStaticMethodByName_Double,
    Class_CallStaticMethodByName_Double_A,
    Class_CallStaticMethodByName_Double_V,
    Class_CallStaticMethodByName_Ref,
    Class_CallStaticMethodByName_Ref_A,
    Class_CallStaticMethodByName_Ref_V,
    Class_CallStaticMethodByName_Void,
    Class_CallStaticMethodByName_Void_A,
    Class_CallStaticMethodByName_Void_V,
    Object_GetField_Boolean,
    Object_GetField_Char,
    Object_GetField_Byte,
    Object_GetField_Short,
    Object_GetField_Int,
    Object_GetField_Long,
    Object_GetField_Float,
    Object_GetField_Double,
    Object_GetField_Ref,
    Object_SetField_Boolean,
    Object_SetField_Char,
    Object_SetField_Byte,
    Object_SetField_Short,
    Object_SetField_Int,
    Object_SetField_Long,
    Object_SetField_Float,
    Object_SetField_Double,
    Object_SetField_Ref,
    Object_GetFieldByName_Boolean,
    Object_GetFieldByName_Char,
    Object_GetFieldByName_Byte,
    Object_GetFieldByName_Short,
    Object_GetFieldByName_Int,
    Object_GetFieldByName_Long,
    Object_GetFieldByName_Float,
    Object_GetFieldByName_Double,
    Object_GetFieldByName_Ref,
    Object_SetFieldByName_Boolean,
    Object_SetFieldByName_Char,
    Object_SetFieldByName_Byte,
    Object_SetFieldByName_Short,
    Object_SetFieldByName_Int,
    Object_SetFieldByName_Long,
    Object_SetFieldByName_Float,
    Object_SetFieldByName_Double,
    Object_SetFieldByName_Ref,
    Object_GetPropertyByName_Boolean,
    Object_GetPropertyByName_Char,
    Object_GetPropertyByName_Byte,
    Object_GetPropertyByName_Short,
    Object_GetPropertyByName_Int,
    Object_GetPropertyByName_Long,
    Object_GetPropertyByName_Float,
    Object_GetPropertyByName_Double,
    Object_GetPropertyByName_Ref,
    Object_SetPropertyByName_Boolean,
    Object_SetPropertyByName_Char,
    Object_SetPropertyByName_Byte,
    Object_SetPropertyByName_Short,
    Object_SetPropertyByName_Int,
    Object_SetPropertyByName_Long,
    Object_SetPropertyByName_Float,
    Object_SetPropertyByName_Double,
    Object_SetPropertyByName_Ref,
    Object_CallMethod_Boolean,
    Object_CallMethod_Boolean_A,
    Object_CallMethod_Boolean_V,
    Object_CallMethod_Char,
    Object_CallMethod_Char_A,
    Object_CallMethod_Char_V,
    Object_CallMethod_Byte,
    Object_CallMethod_Byte_A,
    Object_CallMethod_Byte_V,
    Object_CallMethod_Short,
    Object_CallMethod_Short_A,
    Object_CallMethod_Short_V,
    Object_CallMethod_Int,
    Object_CallMethod_Int_A,
    Object_CallMethod_Int_V,
    Object_CallMethod_Long,
    Object_CallMethod_Long_A,
    Object_CallMethod_Long_V,
    Object_CallMethod_Float,
    Object_CallMethod_Float_A,
    Object_CallMethod_Float_V,
    Object_CallMethod_Double,
    Object_CallMethod_Double_A,
    Object_CallMethod_Double_V,
    Object_CallMethod_Ref,
    Object_CallMethod_Ref_A,
    Object_CallMethod_Ref_V,
    Object_CallMethod_Void,
    Object_CallMethod_Void_A,
    Object_CallMethod_Void_V,
    Object_CallMethodByName_Boolean,
    Object_CallMethodByName_Boolean_A,
    Object_CallMethodByName_Boolean_V,
    Object_CallMethodByName_Char,
    Object_CallMethodByName_Char_A,
    Object_CallMethodByName_Char_V,
    Object_CallMethodByName_Byte,
    Object_CallMethodByName_Byte_A,
    Object_CallMethodByName_Byte_V,
    Object_CallMethodByName_Short,
    Object_CallMethodByName_Short_A,
    Object_CallMethodByName_Short_V,
    Object_CallMethodByName_Int,
    Object_CallMethodByName_Int_A,
    Object_CallMethodByName_Int_V,
    Object_CallMethodByName_Long,
    Object_CallMethodByName_Long_A,
    Object_CallMethodByName_Long_V,
    Object_CallMethodByName_Float,
    Object_CallMethodByName_Float_A,
    Object_CallMethodByName_Float_V,
    Object_CallMethodByName_Double,
    Object_CallMethodByName_Double_A,
    Object_CallMethodByName_Double_V,
    Object_CallMethodByName_Ref,
    Object_CallMethodByName_Ref_A,
    Object_CallMethodByName_Ref_V,
    Object_CallMethodByName_Void,
    Object_CallMethodByName_Void_A,
    Object_CallMethodByName_Void_V,
    TupleValue_GetNumberOfItems,
    TupleValue_GetItem_Boolean,
    TupleValue_GetItem_Char,
    TupleValue_GetItem_Byte,
    TupleValue_GetItem_Short,
    TupleValue_GetItem_Int,
    TupleValue_GetItem_Long,
    TupleValue_GetItem_Float,
    TupleValue_GetItem_Double,
    TupleValue_GetItem_Ref,
    TupleValue_SetItem_Boolean,
    TupleValue_SetItem_Char,
    TupleValue_SetItem_Byte,
    TupleValue_SetItem_Short,
    TupleValue_SetItem_Int,
    TupleValue_SetItem_Long,
    TupleValue_SetItem_Float,
    TupleValue_SetItem_Double,
    TupleValue_SetItem_Ref,
    GlobalReference_Create,
    GlobalReference_Delete,
    WeakReference_Create,
    WeakReference_Delete,
    WeakReference_GetReference,
    CreateArrayBuffer,
    NotImplementedAdapter<445>,
    ArrayBuffer_GetInfo,
    Promise_New,
    PromiseResolver_Resolve,
    PromiseResolver_Reject
};
// clang-format on

const __ani_interaction_api *GetInteractionAPI()
{
    return &INTERACTION_API;
}

bool IsVersionSupported(uint32_t version)
{
    return version == ANI_VERSION_1;
}

}  // namespace ark::ets::ani
