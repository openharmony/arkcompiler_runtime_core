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

[[maybe_unused]] static inline EtsVariable *ToInternalVariable(ani_variable variable)
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

NO_UB_SANITIZE static ani_status GetVersion(ani_env *env, uint32_t *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    *result = ANI_VERSION_1;
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

static ani_status PinPrimitiveTypeArray(ani_env *env, ani_fixedarray primitiveArray, void **result)
{
    ASSERT(primitiveArray != nullptr);
    auto pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);

    auto vm = pandaEnv->GetEtsVM();
    if (!vm->GetGC()->IsPinningSupported()) {
        LOG(WARNING, ANI) << "Pinning is not supported with " << mem::GCStringFromType(vm->GetGC()->GetType());
        return ANI_ERROR;
    }

    auto coreArray = s.ToInternalType(primitiveArray)->GetCoreType();
    vm->GetHeapManager()->PinObject(coreArray);

    *result = coreArray->GetData();
    return ANI_OK;
}

static ani_status UnpinPrimitiveTypeArray(ani_env *env, ani_fixedarray primitiveArray)
{
    ASSERT(primitiveArray != nullptr);

    auto pandaEnv = PandaEtsNapiEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);

    auto coreArray = s.ToInternalType(primitiveArray)->GetCoreType();
    auto vm = pandaEnv->GetEtsVM();
    vm->GetHeapManager()->UnpinObject(coreArray);
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
NO_UB_SANITIZE static ani_status FixedArray_Pin(ani_env *env, ani_fixedarray primitive_array, void **result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(primitive_array);
    CHECK_PTR_ARG(result);

    return PinPrimitiveTypeArray(env, primitive_array, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_Unpin(ani_env *env, ani_fixedarray primitive_array,
                                                  [[maybe_unused]] void *data)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(primitive_array);

    return UnpinPrimitiveTypeArray(env, primitive_array);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetLength(ani_env *env, ani_fixedarray array, ani_size *result)
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
NO_UB_SANITIZE static ani_status FixedArray_New_Boolean(ani_env *env, ani_size length, ani_fixedarray_boolean *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsBooleanArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Byte(ani_env *env, ani_size length, ani_fixedarray_byte *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsByteArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Short(ani_env *env, ani_size length, ani_fixedarray_short *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsShortArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Int(ani_env *env, ani_size length, ani_fixedarray_int *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsIntArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Long(ani_env *env, ani_size length, ani_fixedarray_long *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsLongArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Float(ani_env *env, ani_size length, ani_fixedarray_float *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsFloatArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Double(ani_env *env, ani_size length, ani_fixedarray_double *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    return NewPrimitiveTypeArray<EtsDoubleArray>(env, length, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Boolean(ani_env *env, ani_fixedarray_boolean array,
                                                              ani_size offset, ani_size length,
                                                              ani_boolean *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Byte(ani_env *env, ani_fixedarray_byte array, ani_size offset,
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
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Short(ani_env *env, ani_fixedarray_short array, ani_size offset,
                                                            ani_size length, ani_short *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Int(ani_env *env, ani_fixedarray_int array, ani_size offset,
                                                          ani_size length, ani_int *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Long(ani_env *env, ani_fixedarray_long array, ani_size offset,
                                                           ani_size length, ani_long *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Float(ani_env *env, ani_fixedarray_float array, ani_size offset,
                                                            ani_size length, ani_float *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Double(ani_env *env, ani_fixedarray_double array, ani_size offset,
                                                             ani_size length, ani_double *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Boolean(ani_env *env, ani_fixedarray_boolean array,
                                                              ani_size offset, ani_size length,
                                                              const ani_boolean *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Short(ani_env *env, ani_fixedarray_short array, ani_size offset,
                                                            ani_size length, const ani_short *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Int(ani_env *env, ani_fixedarray_int array, ani_size offset,
                                                          ani_size length, const ani_int *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Long(ani_env *env, ani_fixedarray_long array, ani_size offset,
                                                           ani_size length, const ani_long *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Float(ani_env *env, ani_fixedarray_float array, ani_size offset,
                                                            ani_size length, const ani_float *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Double(ani_env *env, ani_fixedarray_double array, ani_size offset,
                                                             ani_size length, const ani_double *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return SetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Byte(ani_env *env, ani_fixedarray_byte array, ani_size offset,
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
NO_UB_SANITIZE static ani_status Class_GetField(ani_env *env, ani_class cls, const char *name, ani_field *result)
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
NO_UB_SANITIZE static ani_status Class_GetStaticField(ani_env *env, ani_class cls, const char *name,
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
NO_UB_SANITIZE static ani_status Class_GetMethod(ani_env *env, ani_class cls, const char *name, const char *signature,
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
NO_UB_SANITIZE static ani_status Class_GetStaticMethod(ani_env *env, ani_class cls, const char *name,
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
NO_UB_SANITIZE static ani_status Object_GetField_Long(ani_env *env, ani_object object, ani_field field,
                                                      ani_long *result)
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

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Ref(ani_env *env, ani_object object, const char *name,
                                                           ani_ref *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(object);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(env);
    EtsCoroutine *coroutine = s.GetCoroutine();
    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> etsObject(coroutine, s.ToInternalType(object));
    ASSERT(etsObject.GetPtr() != nullptr);
    EtsField *etsField = etsObject->GetClass()->GetFieldIDByName(name, nullptr);
    ANI_CHECK_RETURN_IF_EQ(etsField, nullptr, ANI_NOT_FOUND);
    ANI_CHECK_RETURN_IF_NE(etsField->GetEtsType(), AniTypeInfo<ani_ref>::ETS_TYPE_VALUE, ANI_INVALID_TYPE);
    EtsObject *etsRes = etsObject->GetFieldObject(etsField);
    return s.AddLocalRef(etsRes, result);
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
NO_UB_SANITIZE static ani_status GlobalReference_Create(ani_env *env, ani_ref ref, ani_gref *result)
{
    // This is a temporary implementation, it needs to be redone. #22006
    ANI_DEBUG_TRACE(env);
    CHECK_PTR_ARG(ref);
    CHECK_PTR_ARG(result);

    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsObject *internalObject = s.ToInternalType(ref);
    *result = s.AddGlobalRef(internalObject);
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
    NotImplementedAdapter<5>,
    NotImplementedAdapter<6>,
    NotImplementedAdapter<7>,
    NotImplementedAdapter<8>,
    NotImplementedAdapter<9>,
    NotImplementedAdapter<10>,
    NotImplementedAdapter<11>,
    NotImplementedAdapter<12>,
    NotImplementedAdapter<13>,
    NotImplementedAdapter<14>,
    NotImplementedAdapter<15>,
    NotImplementedAdapter<16>,
    NotImplementedAdapter<17>,
    NotImplementedAdapter<18>,
    NotImplementedAdapter<19>,
    NotImplementedAdapter<20>,
    NotImplementedAdapter<21>,
    Object_New,
    Object_New_A,
    Object_New_V,
    NotImplementedAdapter<25>,
    Object_InstanceOf,
    NotImplementedAdapter<27>,
    NotImplementedAdapter<28>,
    NotImplementedAdapter<29>,
    FindModule,
    FindNamespace,
    FindClass,
    NotImplementedAdapter<33>,
    NotImplementedAdapter<34>,
    NotImplementedAdapter<35>,
    NotImplementedAdapter<36>,
    NotImplementedAdapter<37>,
    NotImplementedAdapter<38>,
    NotImplementedAdapter<39>,
    NotImplementedAdapter<40>,
    NotImplementedAdapter<41>,
    NotImplementedAdapter<42>,
    NotImplementedAdapter<43>,
    NotImplementedAdapter<44>,
    Namespace_FindFunction,
    Namespace_FindVariable,
    Module_BindNativeFunctions,
    Namespace_BindNativeFunctions,
    Class_BindNativeMethods,
    NotImplementedAdapter<50>,
    NotImplementedAdapter<51>,
    NotImplementedAdapter<52>,
    NotImplementedAdapter<53>,
    NotImplementedAdapter<54>,
    NotImplementedAdapter<55>,
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
    NotImplementedAdapter<77>,
    NotImplementedAdapter<78>,
    NotImplementedAdapter<79>,
    NotImplementedAdapter<80>,
    NotImplementedAdapter<81>,
    NotImplementedAdapter<82>,
    NotImplementedAdapter<83>,
    NotImplementedAdapter<84>,
    NotImplementedAdapter<85>,
    NotImplementedAdapter<86>,
    NotImplementedAdapter<87>,
    NotImplementedAdapter<88>,
    FixedArray_GetLength,
    FixedArray_New_Boolean,
    NotImplementedAdapter<91>,
    FixedArray_New_Byte,
    FixedArray_New_Short,
    FixedArray_New_Int,
    FixedArray_New_Long,
    FixedArray_New_Float,
    FixedArray_New_Double,
    FixedArray_GetRegion_Boolean,
    NotImplementedAdapter<99>,
    FixedArray_GetRegion_Byte,
    FixedArray_GetRegion_Short,
    FixedArray_GetRegion_Int,
    FixedArray_GetRegion_Long,
    FixedArray_GetRegion_Float,
    FixedArray_GetRegion_Double,
    FixedArray_SetRegion_Boolean,
    NotImplementedAdapter<107>,
    FixedArray_SetRegion_Byte,
    FixedArray_SetRegion_Short,
    FixedArray_SetRegion_Int,
    FixedArray_SetRegion_Long,
    FixedArray_SetRegion_Float,
    FixedArray_SetRegion_Double,
    FixedArray_Pin,
    FixedArray_Unpin,
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
    NotImplementedAdapter<135>,
    NotImplementedAdapter<136>,
    NotImplementedAdapter<137>,
    NotImplementedAdapter<138>,
    NotImplementedAdapter<139>,
    NotImplementedAdapter<140>,
    NotImplementedAdapter<141>,
    NotImplementedAdapter<142>,
    NotImplementedAdapter<143>,
    NotImplementedAdapterVargs<144>,
    NotImplementedAdapter<145>,
    NotImplementedAdapter<146>,
    NotImplementedAdapterVargs<147>,
    NotImplementedAdapter<148>,
    NotImplementedAdapter<149>,
    NotImplementedAdapterVargs<150>,
    NotImplementedAdapter<151>,
    NotImplementedAdapter<152>,
    NotImplementedAdapterVargs<153>,
    NotImplementedAdapter<154>,
    NotImplementedAdapter<155>,
    NotImplementedAdapterVargs<156>,
    NotImplementedAdapter<157>,
    NotImplementedAdapter<158>,
    NotImplementedAdapterVargs<159>,
    NotImplementedAdapter<160>,
    NotImplementedAdapter<161>,
    NotImplementedAdapterVargs<162>,
    NotImplementedAdapter<163>,
    NotImplementedAdapter<164>,
    NotImplementedAdapterVargs<165>,
    NotImplementedAdapter<166>,
    NotImplementedAdapter<167>,
    Function_Call_Ref,
    Function_Call_Ref_A,
    Function_Call_Ref_V,
    NotImplementedAdapterVargs<171>,
    NotImplementedAdapter<172>,
    NotImplementedAdapter<173>,
    NotImplementedAdapter<174>,
    NotImplementedAdapter<175>,
    Class_GetField,
    Class_GetStaticField,
    Class_GetMethod,
    Class_GetStaticMethod,
    NotImplementedAdapter<180>,
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
    NotImplementedAdapterVargs<249>,
    NotImplementedAdapter<250>,
    NotImplementedAdapter<251>,
    NotImplementedAdapterVargs<252>,
    NotImplementedAdapter<253>,
    NotImplementedAdapter<254>,
    NotImplementedAdapterVargs<255>,
    NotImplementedAdapter<256>,
    NotImplementedAdapter<257>,
    NotImplementedAdapterVargs<258>,
    NotImplementedAdapter<259>,
    NotImplementedAdapter<260>,
    NotImplementedAdapterVargs<261>,
    NotImplementedAdapter<262>,
    NotImplementedAdapter<263>,
    NotImplementedAdapterVargs<264>,
    NotImplementedAdapter<265>,
    NotImplementedAdapter<266>,
    NotImplementedAdapterVargs<267>,
    NotImplementedAdapter<268>,
    NotImplementedAdapter<269>,
    NotImplementedAdapterVargs<270>,
    NotImplementedAdapter<271>,
    NotImplementedAdapter<272>,
    NotImplementedAdapterVargs<273>,
    NotImplementedAdapter<274>,
    NotImplementedAdapter<275>,
    NotImplementedAdapterVargs<276>,
    NotImplementedAdapter<277>,
    NotImplementedAdapter<278>,
    NotImplementedAdapterVargs<279>,
    NotImplementedAdapter<280>,
    NotImplementedAdapter<281>,
    NotImplementedAdapter<282>,
    NotImplementedAdapter<283>,
    NotImplementedAdapter<284>,
    NotImplementedAdapter<285>,
    NotImplementedAdapter<286>,
    Object_GetField_Long,
    NotImplementedAdapter<288>,
    NotImplementedAdapter<289>,
    Object_GetField_Ref,
    NotImplementedAdapter<291>,
    NotImplementedAdapter<292>,
    NotImplementedAdapter<293>,
    NotImplementedAdapter<294>,
    Object_SetField_Int,
    Object_SetField_Long,
    NotImplementedAdapter<297>,
    NotImplementedAdapter<298>,
    Object_SetField_Ref,
    NotImplementedAdapter<300>,
    NotImplementedAdapter<301>,
    NotImplementedAdapter<302>,
    NotImplementedAdapter<303>,
    NotImplementedAdapter<304>,
    NotImplementedAdapter<305>,
    NotImplementedAdapter<306>,
    NotImplementedAdapter<307>,
    Object_GetFieldByName_Ref,
    NotImplementedAdapter<309>,
    NotImplementedAdapter<310>,
    NotImplementedAdapter<311>,
    NotImplementedAdapter<312>,
    NotImplementedAdapter<313>,
    NotImplementedAdapter<314>,
    NotImplementedAdapter<315>,
    NotImplementedAdapter<316>,
    NotImplementedAdapter<317>,
    NotImplementedAdapter<318>,
    NotImplementedAdapter<319>,
    NotImplementedAdapter<320>,
    NotImplementedAdapter<321>,
    NotImplementedAdapter<322>,
    NotImplementedAdapter<323>,
    NotImplementedAdapter<324>,
    NotImplementedAdapter<325>,
    NotImplementedAdapter<326>,
    NotImplementedAdapter<327>,
    NotImplementedAdapter<328>,
    NotImplementedAdapter<329>,
    NotImplementedAdapter<330>,
    NotImplementedAdapter<331>,
    NotImplementedAdapter<332>,
    NotImplementedAdapter<333>,
    NotImplementedAdapter<334>,
    NotImplementedAdapter<335>,
    NotImplementedAdapter<336>,
    NotImplementedAdapter<337>,
    NotImplementedAdapter<338>,
    NotImplementedAdapter<339>,
    Object_GetPropertyByName_Int,
    NotImplementedAdapter<341>,
    NotImplementedAdapter<342>,
    NotImplementedAdapter<343>,
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
    NotImplementedAdapterVargs<360>,
    NotImplementedAdapter<361>,
    NotImplementedAdapter<362>,
    NotImplementedAdapterVargs<363>,
    NotImplementedAdapter<364>,
    NotImplementedAdapter<365>,
    Object_CallMethod_Int,
    Object_CallMethod_Int_A,
    Object_CallMethod_Int_V,
    Object_CallMethod_Long,
    Object_CallMethod_Long_A,
    Object_CallMethod_Long_V,
    NotImplementedAdapterVargs<372>,
    NotImplementedAdapter<373>,
    NotImplementedAdapter<374>,
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
    NotImplementedAdapterVargs<414>,
    NotImplementedAdapter<415>,
    NotImplementedAdapter<416>,
    NotImplementedAdapter<417>,
    NotImplementedAdapter<418>,
    NotImplementedAdapter<419>,
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
    NotImplementedAdapter<440>,
    NotImplementedAdapter<441>,
    NotImplementedAdapter<442>,
    NotImplementedAdapter<443>,
    NotImplementedAdapter<444>,
    NotImplementedAdapter<445>,
    NotImplementedAdapter<446>,
    NotImplementedAdapter<447>,
    NotImplementedAdapter<448>,
    NotImplementedAdapter<449>,
    NotImplementedAdapter<450>,
    NotImplementedAdapter<451>,
    NotImplementedAdapter<452>,
    NotImplementedAdapter<453>,
    NotImplementedAdapter<454>,
    NotImplementedAdapter<455>,
    NotImplementedAdapter<456>,
    NotImplementedAdapter<457>,
    NotImplementedAdapter<458>,
    NotImplementedAdapter<459>,
    NotImplementedAdapter<460>,
    NotImplementedAdapter<461>,
    NotImplementedAdapter<462>,
    NotImplementedAdapter<463>,
    NotImplementedAdapterVargs<464>,
    NotImplementedAdapter<465>,
    NotImplementedAdapter<466>,
    NotImplementedAdapterVargs<467>,
    NotImplementedAdapter<468>,
    NotImplementedAdapter<469>,
    NotImplementedAdapterVargs<470>,
    NotImplementedAdapter<471>,
    NotImplementedAdapter<472>,
    NotImplementedAdapter<473>,
    NotImplementedAdapter<474>,
    NotImplementedAdapter<475>,
    NotImplementedAdapter<476>,
    NotImplementedAdapter<477>,
    NotImplementedAdapter<478>,
    NotImplementedAdapter<479>,
    NotImplementedAdapter<480>,
    NotImplementedAdapter<481>,
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
