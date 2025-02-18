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

#include "ani.h"
#include "macros.h"
#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/ani_type_info.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_namespace.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_PTR_ARG(arg) ANI_CHECK_RETURN_IF_EQ(arg, nullptr, ANI_INVALID_ARGS)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_ENV(env)                                                                           \
    do {                                                                                         \
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
    while (it != end) {
        panda_file::Type type = *it;
        ++it;
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
                auto param = va_arg(args, ani_object);
                parsedArgs.emplace_back(param != nullptr ? s->ToInternalType(param)->GetCoreType() : nullptr);
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
    auto *arg = args;
    while (it != end) {
        panda_file::Type type = *it;
        ++it;
        switch (type.GetId()) {
            case TypeId::U1:
                parsedArgs.emplace_back(arg->z);
                break;
            case TypeId::U32:
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
                parsedArgs.emplace_back(s->ToInternalType(arg->r)->GetCoreType());
                break;
            }
            default:
                LOG(FATAL, ANI) << "Unexpected argument type";
                break;
        }
        ++arg;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
    EtsMethod *m = nullptr;
    if constexpr (std::is_same_v<MethodType, ani_method>) {
        m = ResolveVirtualMethod(&s, obj, method);
    } else if constexpr (std::is_same_v<MethodType, ani_static_method>) {
        m = ToInternalMethod(method);
    } else {
        static_assert(!(std::is_same_v<MethodType, ani_method> || std::is_same_v<MethodType, ani_static_method>),
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
    auto method = reinterpret_cast<ani_static_method>(fn);
    ScopedManagedCodeFix s(env);
    return DoGeneralMethodCall<EtsValueType, AniType, ani_static_method, Args>(s, nullptr, method, result, args);
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
static ani_status DoFind(ani_env *env, const char *name, T *result)
{
    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    EtsClassLinker *classLinker = pandaEnv->GetEtsVM()->GetClassLinker();
    EtsClass *klass = classLinker->GetClass(name, true, GetClassLinkerContext(s.GetCoroutine()));
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

    ASSERT_MANAGED_CODE();
    return s.AddLocalRef(reinterpret_cast<EtsObject *>(klass), reinterpret_cast<ani_ref *>(result));
}

template <bool IS_STATIC_METHOD>
static ani_status DoGetClassMethod(EtsClass *klass, const char *name, const char *signature, EtsMethod **result)
{
    ASSERT_MANAGED_CODE();
    EtsMethod *method = (signature == nullptr ? klass->GetMethod(name) : klass->GetMethod(name, signature));
    if (method == nullptr || method->IsStatic() != IS_STATIC_METHOD) {
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

static ani_status GetNamespaceFunction(ani_env *env, ani_namespace ns, const char *name, const char *signature,
                                       EtsMethod **result)
{
    ScopedManagedCodeFix s(env);
    EtsNamespace *etsNs;
    ani_status status = GetInternalNamespace(s, ns, &etsNs);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);
    return DoGetClassMethod<true>(etsNs->AsClass(), name, signature, result);
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
NO_UB_SANITIZE static ani_status Object_New(ani_env *env, ani_class cls, ani_method method, ani_object *result, ...)
{
    va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, result);
    ani_status status = Object_New_V(env, cls, method, result, args);
    va_end(args);
    return status;
}

NO_UB_SANITIZE static ani_status FindModule(ani_env *env, const char *moduleDescriptor, ani_module *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(moduleDescriptor);
    CHECK_PTR_ARG(result);

    size_t len = strlen(moduleDescriptor);
    ANI_CHECK_RETURN_IF_LE(len, 2U, ANI_INVALID_DESCRIPTOR);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ANI_CHECK_RETURN_IF_NE(moduleDescriptor[len - 1], ';', ANI_INVALID_DESCRIPTOR);
    PandaString descriptor(moduleDescriptor, len - 1);
    descriptor += "/ETSGLOBAL;";

    // NOTE: Check that results is namespace, #22400
    return DoFind(env, descriptor.c_str(), result);
}

NO_UB_SANITIZE static ani_status FindNamespace(ani_env *env, const char *namespaceDescriptor, ani_namespace *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(namespaceDescriptor);
    CHECK_PTR_ARG(result);

    // NOTE: Check that results is namespace, #22400
    return DoFind(env, namespaceDescriptor, result);
}

NO_UB_SANITIZE static ani_status FindClass(ani_env *env, const char *classDescriptor, ani_class *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(classDescriptor);
    CHECK_PTR_ARG(result);

    // NOTE: Check that results is class, #22400
    return DoFind(env, classDescriptor, result);
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

template <typename InternalType, typename AniFixedArrayType>
static ani_status NewPrimitiveTypeArray(ani_env *env, ani_size length, AniFixedArrayType *result)
{
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    // EtsArray
    auto *array = InternalType::Create(length);
    ANI_CHECK_RETURN_IF_EQ(array, nullptr, ANI_OUT_OF_MEMORY);
    return s.AddLocalRef(reinterpret_cast<EtsObject *>(array), reinterpret_cast<ani_ref *>(result));
}

template <typename T, typename ArrayType>
static ani_status GetPrimitiveTypeArrayRegion(ani_env *env, ArrayType array, ani_size start, ani_size len, T *buf)
{
    ASSERT(array != nullptr);
    ANI_CHECK_RETURN_IF_EQ(len != 0 && buf == nullptr, true, ANI_INVALID_ARGS);

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsArray *internalArray = s.ToInternalType(array);
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
    EtsArray *internalArray = s.ToInternalType(array);
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
    EtsArray *internalArray = s.ToInternalType(array);
    *result = internalArray->GetLength();
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
NO_UB_SANITIZE static ani_status Array_GetRegion_Byte(ani_env *env, ani_array_byte array, ani_size offset,
                                                      ani_size length, ani_byte *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
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
        EtsMethod *method = signature == nullptr ? klass->GetMethod(m.name) : klass->GetMethod(m.name, signature);
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

template <bool IS_STATIC_FIELD>
static ani_status DoGetField(ani_env *env, ani_class cls, const char *name, EtsField **result)
{
    ScopedManagedCodeFix s(env);
    EtsClass *klass;
    ani_status status = GetInternalClass(s, cls, &klass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    EtsField *field = [&]() {
        if constexpr (IS_STATIC_FIELD) {
            return klass->GetStaticFieldIDByName(name, nullptr);
        } else {
            return klass->GetFieldIDByName(name, nullptr);
        }
    }();
    ANI_CHECK_RETURN_IF_EQ(field, nullptr, ANI_NOT_FOUND);

    *result = field;
    return ANI_OK;
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
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Char(ani_env *env, ani_object object, const char *name,
                                                            ani_char *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Byte(ani_env *env, ani_object object, const char *name,
                                                            ani_byte *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Short(ani_env *env, ani_object object, const char *name,
                                                             ani_short *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Int(ani_env *env, ani_object object, const char *name,
                                                           ani_int *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Long(ani_env *env, ani_object object, const char *name,
                                                            ani_long *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Float(ani_env *env, ani_object object, const char *name,
                                                             ani_float *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Double(ani_env *env, ani_object object, const char *name,
                                                              ani_double *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return DoGetFieldByName(env, object, name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Ref(ani_env *env, ani_object object, const char *name,
                                                           ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

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
        EtsMethod *method = klass->GetMethod((PandaString(GETTER_BEGIN) + name).c_str());
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

NO_UB_SANITIZE static ani_status ExistUnhandledError(ani_env *env, ani_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(result);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    *result = pandaEnv->HasPendingException() ? ANI_TRUE : ANI_FALSE;
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status ResetError(ani_env *env)
{
    ANI_DEBUG_TRACE(env);
    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    pandaEnv->ClearException();
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status DescribeError(ani_env *env)
{
    ANI_DEBUG_TRACE(env);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    if (!pandaEnv->HasPendingException()) {
        return ANI_OK;
    }

    // NOTE: Implement when #21687 will be solved, #22008
    std::cerr << "DescribeError: method is not implemented" << std::endl;
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
    return s.AddLocalRef(reinterpret_cast<EtsObject *>(internalString), reinterpret_cast<ani_ref *>(result));
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
        *result = cls->GetStaticFieldPrimitive<R>(field);
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
    CHECK_PTR_ARG(wref);
    CHECK_PTR_ARG(wasReleasedResult);
    CHECK_PTR_ARG(refResult);

    ScopedManagedCodeFix s(env);
    return s.GetLocalRef(wref, wasReleasedResult, refResult);
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
    NotImplementedAdapter<25>,
    Object_InstanceOf,
    NotImplementedAdapter<28>,
    NotImplementedAdapter<29>,
    FindModule,
    FindNamespace,
    FindClass,
    NotImplementedAdapter<33>,
    NotImplementedAdapter<35>,
    NotImplementedAdapter<36>,
    NotImplementedAdapter<37>,
    NotImplementedAdapter<38>,
    NotImplementedAdapter<39>,
    NotImplementedAdapter<40>,
    NotImplementedAdapter<41>,
    NotImplementedAdapter<42>,
    Namespace_FindFunction,
    Namespace_FindVariable,
    Module_BindNativeFunctions,
    Namespace_BindNativeFunctions,
    Class_BindNativeMethods,
    Reference_Delete,
    NotImplementedAdapter<51>,
    CreateLocalScope,
    DestroyLocalScope,
    CreateEscapeLocalScope,
    DestroyEscapeLocalScope,
    NotImplementedAdapter<56>,
    ExistUnhandledError,
    NotImplementedAdapter<58>,
    ResetError,
    DescribeError,
    NotImplementedAdapter<61>,
    GetNull,
    GetUndefined,
    Reference_IsNull,
    Reference_IsUndefined,
    Reference_IsNullishValue,
    Reference_Equals,
    Reference_StrictEquals,
    NotImplementedAdapter<69>,
    String_GetUTF16Size,
    NotImplementedAdapter<71>,
    NotImplementedAdapter<72>,
    String_NewUTF8,
    String_GetUTF8Size,
    NotImplementedAdapter<75>,
    String_GetUTF8SubString,
    Array_GetLength,
    Array_New_Boolean,
    NotImplementedAdapter<91>,
    Array_New_Byte,
    Array_New_Short,
    Array_New_Int,
    Array_New_Long,
    Array_New_Float,
    Array_New_Double,
    Array_GetRegion_Boolean,
    NotImplementedAdapter<99>,
    Array_GetRegion_Byte,
    Array_GetRegion_Short,
    Array_GetRegion_Int,
    Array_GetRegion_Long,
    Array_GetRegion_Float,
    Array_GetRegion_Double,
    Array_SetRegion_Boolean,
    NotImplementedAdapter<107>,
    Array_SetRegion_Byte,
    Array_SetRegion_Short,
    Array_SetRegion_Int,
    Array_SetRegion_Long,
    Array_SetRegion_Float,
    Array_SetRegion_Double,
    NotImplementedAdapter<115>,
    NotImplementedAdapter<116>,
    NotImplementedAdapter<117>,
    NotImplementedAdapter<118>,
    NotImplementedAdapter<119>,
    NotImplementedAdapter<120>,
    NotImplementedAdapter<121>,
    NotImplementedAdapter<122>,
    NotImplementedAdapter<123>,
    NotImplementedAdapter<124>,
    NotImplementedAdapter<125>,
    NotImplementedAdapter<126>,
    NotImplementedAdapter<127>,
    NotImplementedAdapter<128>,
    NotImplementedAdapter<129>,
    NotImplementedAdapter<130>,
    NotImplementedAdapter<131>,
    NotImplementedAdapter<132>,
    NotImplementedAdapter<133>,
    NotImplementedAdapter<134>,
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
    NotImplementedAdapterVargs<147>,
    NotImplementedAdapter<148>,
    NotImplementedAdapter<149>,
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
    NotImplementedAdapterVargs<171>,
    NotImplementedAdapter<172>,
    NotImplementedAdapter<173>,
    Class_FindField,
    Class_FindStaticField,
    Class_FindMethod,
    Class_FindStaticMethod,
    NotImplementedAdapter<181>,
    NotImplementedAdapter<182>,
    NotImplementedAdapter<183>,
    NotImplementedAdapter<184>,
    NotImplementedAdapter<185>,
    NotImplementedAdapter<186>,
    NotImplementedAdapter<187>,
    NotImplementedAdapter<188>,
    NotImplementedAdapter<189>,
    NotImplementedAdapter<190>,
    NotImplementedAdapter<191>,
    NotImplementedAdapter<192>,
    NotImplementedAdapter<193>,
    NotImplementedAdapter<194>,
    NotImplementedAdapter<195>,
    NotImplementedAdapter<196>,
    NotImplementedAdapter<197>,
    NotImplementedAdapter<198>,
    NotImplementedAdapter<199>,
    NotImplementedAdapter<200>,
    NotImplementedAdapter<201>,
    NotImplementedAdapter<202>,
    NotImplementedAdapter<203>,
    NotImplementedAdapter<204>,
    NotImplementedAdapter<205>,
    NotImplementedAdapter<206>,
    NotImplementedAdapter<207>,
    NotImplementedAdapter<208>,
    NotImplementedAdapter<209>,
    NotImplementedAdapter<210>,
    NotImplementedAdapter<211>,
    NotImplementedAdapter<212>,
    NotImplementedAdapter<213>,
    NotImplementedAdapter<214>,
    NotImplementedAdapter<215>,
    NotImplementedAdapter<216>,
    NotImplementedAdapter<217>,
    NotImplementedAdapter<218>,
    NotImplementedAdapter<219>,
    NotImplementedAdapter<220>,
    NotImplementedAdapter<221>,
    Class_CallStaticMethod_Boolean,
    Class_CallStaticMethod_Boolean_A,
    Class_CallStaticMethod_Boolean_V,
    NotImplementedAdapterVargs<225>,
    NotImplementedAdapter<226>,
    NotImplementedAdapter<227>,
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
    NotImplementedAdapterVargs<255>,
    NotImplementedAdapter<256>,
    NotImplementedAdapter<257>,
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
    NotImplementedAdapter<283>,
    Object_GetField_Byte,
    Object_GetField_Short,
    Object_GetField_Int,
    Object_GetField_Long,
    Object_GetField_Float,
    Object_GetField_Double,
    Object_GetField_Ref,
    Object_SetField_Boolean,
    NotImplementedAdapter<292>,
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
    NotImplementedAdapter<310>,
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
    NotImplementedAdapter<345>,
    NotImplementedAdapter<346>,
    NotImplementedAdapter<347>,
    NotImplementedAdapter<348>,
    NotImplementedAdapter<349>,
    NotImplementedAdapter<350>,
    NotImplementedAdapter<351>,
    NotImplementedAdapter<352>,
    NotImplementedAdapter<353>,
    Object_CallMethod_Boolean,
    Object_CallMethod_Boolean_A,
    Object_CallMethod_Boolean_V,
    NotImplementedAdapterVargs<357>,
    NotImplementedAdapter<358>,
    NotImplementedAdapter<359>,
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
    NotImplementedAdapterVargs<384>,
    NotImplementedAdapter<385>,
    NotImplementedAdapter<386>,
    NotImplementedAdapterVargs<387>,
    NotImplementedAdapter<388>,
    NotImplementedAdapter<389>,
    NotImplementedAdapterVargs<390>,
    NotImplementedAdapter<391>,
    NotImplementedAdapter<392>,
    NotImplementedAdapterVargs<393>,
    NotImplementedAdapter<394>,
    NotImplementedAdapter<395>,
    NotImplementedAdapterVargs<396>,
    NotImplementedAdapter<397>,
    NotImplementedAdapter<398>,
    NotImplementedAdapterVargs<399>,
    NotImplementedAdapter<400>,
    NotImplementedAdapter<401>,
    NotImplementedAdapterVargs<402>,
    NotImplementedAdapter<403>,
    NotImplementedAdapter<404>,
    NotImplementedAdapterVargs<405>,
    NotImplementedAdapter<406>,
    NotImplementedAdapter<407>,
    NotImplementedAdapterVargs<408>,
    NotImplementedAdapter<409>,
    NotImplementedAdapter<410>,
    NotImplementedAdapterVargs<411>,
    NotImplementedAdapter<412>,
    NotImplementedAdapter<413>,
    NotImplementedAdapter<420>,
    NotImplementedAdapter<421>,
    NotImplementedAdapter<422>,
    NotImplementedAdapter<423>,
    NotImplementedAdapter<424>,
    NotImplementedAdapter<425>,
    NotImplementedAdapter<426>,
    NotImplementedAdapter<427>,
    NotImplementedAdapter<428>,
    NotImplementedAdapter<429>,
    NotImplementedAdapter<430>,
    NotImplementedAdapter<431>,
    NotImplementedAdapter<432>,
    NotImplementedAdapter<433>,
    NotImplementedAdapter<434>,
    NotImplementedAdapter<435>,
    NotImplementedAdapter<436>,
    NotImplementedAdapter<437>,
    NotImplementedAdapter<438>,
    GlobalReference_Create,
    GlobalReference_Delete,
    WeakReference_Create,
    WeakReference_Delete,
    WeakReference_GetReference,
    NotImplementedAdapter<444>,
    NotImplementedAdapter<445>,
    NotImplementedAdapter<446>,
    NotImplementedAdapter<447>,
    NotImplementedAdapter<448>,
    NotImplementedAdapter<449>,
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
