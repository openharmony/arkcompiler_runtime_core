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
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ets_napi_env.h"

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

static inline EtsMethod *ToInternalMethod(ani_method method)
{
    auto *m = reinterpret_cast<EtsMethod *>(method);
    ASSERT(!m->IsStatic());
    return m;
}

[[maybe_unused]] static inline ani_method ToAniMethod(EtsMethod *method)
{
    ASSERT(!method->IsStatic());
    return reinterpret_cast<ani_method>(method);
}

static inline EtsMethod *ToInternalMethod(ani_static_method method)
{
    auto *m = reinterpret_cast<EtsMethod *>(method);
    ASSERT(m->IsStatic());
    return m;
}

static inline ani_static_method ToAniStaticMethod(EtsMethod *method)
{
    ASSERT(method->IsStatic());
    return reinterpret_cast<ani_static_method>(method);
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

static EtsClass *GetInternalClass(ScopedManagedCodeFix *s, ani_class cls)
{
    EtsClass *klass = s->ToInternalType(cls);
    if (klass->IsInitialized()) {
        return klass;
    }

    // Initialize class
    EtsCoroutine *corutine = s->GetCoroutine();
    EtsClassLinker *classLinker = corutine->GetPandaVM()->GetClassLinker();
    bool isInitialized = classLinker->InitializeClass(corutine, klass);
    if (!isInitialized) {
        LOG(ERROR, ANI) << "Cannot initialize class: " << klass->GetDescriptor();
        return nullptr;
    }
    return klass;
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
            case TypeId::U32:  // NOTE: Check it
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

[[maybe_unused]] static inline EtsMethod *ResolveVirtualMethod(ScopedManagedCodeFix *s, ani_object object,
                                                               ani_method method)
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
static ani_status GeneralMethodCall(ani_env *env, ani_object obj, MethodType method, AniType *result, Args args)
{
    EtsMethod *m = nullptr;
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    if constexpr (std::is_same_v<MethodType, ani_method>) {
        m = ResolveVirtualMethod(&s, obj, method);
    } else if constexpr (std::is_same_v<MethodType, ani_static_method>) {
        m = ToInternalMethod(method);
    } else {
        static_assert(!(std::is_same_v<MethodType, ani_method> || std::is_same_v<MethodType, ani_static_method>),
                      "Unreachable type");
    }
    ASSERT(m != nullptr);

    ArgVector<Value> values = GetArgValues(&s, m, args, obj);
    EtsValue res = m->Invoke(&s, const_cast<Value *>(values.data()));

    // Now AniType and EtsValueType are the same, but later it could be changed
    static_assert(std::is_same_v<AniType, EtsValueType>);
    *result = res.GetAs<EtsValueType>();
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status GetVersion(ani_env *env, uint32_t *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);

    *result = ANI_VERSION_1;
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status FindClass(ani_env *env, const char *classDescriptor, ani_class *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(classDescriptor);
    CHECK_PTR_ARG(result);

    PandaEnv *pandaEnv = PandaEnv::FromAniEnv(env);
    ScopedManagedCodeFix s(pandaEnv);
    EtsClassLinker *classLinker = pandaEnv->GetEtsVM()->GetClassLinker();
    EtsClass *klass = classLinker->GetClass(classDescriptor, true, GetClassLinkerContext(s.GetCoroutine()));
    if (UNLIKELY(pandaEnv->HasPendingException())) {
        EtsThrowable *currentException = pandaEnv->GetThrowable();
        std::string_view exceptionString = currentException->GetClass()->GetDescriptor();
        if (exceptionString == panda_file_items::class_descriptors::CLASS_NOT_FOUND_EXCEPTION) {
            pandaEnv->ClearException();
            return ANI_NOT_FOUND;
        }

        // NOTE: Handle exception
        return ANI_PENDING_ERROR;
    }
    ANI_CHECK_RETURN_IF_EQ(klass, nullptr, ANI_NOT_FOUND);

    ASSERT_MANAGED_CODE();
    *result = static_cast<ani_class>(s.AddLocalRef(reinterpret_cast<EtsObject *>(klass)));
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

template <typename AniType, typename InternalType>
static AniType NewPrimitiveTypeArray(ani_env *env, ani_size length)
{
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    // EtsArray
    auto *array = InternalType::Create(length);
    if (UNLIKELY(array == nullptr)) {
        return nullptr;
    }
    ani_ref ret = s.AddLocalRef(reinterpret_cast<EtsObject *>(array));
    return reinterpret_cast<AniType>(ret);
}

template <typename T>
static ani_status GetPrimitiveTypeArrayRegion(ani_env *env, ani_fixedarray_byte array, ani_size start, ani_size len,
                                              T *buf)
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

template <typename T>
static ani_status SetPrimitiveTypeArrayRegion(ani_env *env, ani_fixedarray_byte array, ani_size start, ani_size len,
                                              T *buf)
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
NO_UB_SANITIZE static ani_status FixedArray_New_Byte(ani_env *env, ani_size length, ani_fixedarray_byte *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(result);
    *result = NewPrimitiveTypeArray<ani_fixedarray_byte, EtsByteArray>(env, length);
    ANI_CHECK_RETURN_IF_EQ(*result, nullptr, ANI_OUT_OF_MEMORY);
    return ANI_OK;
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
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Byte(ani_env *env, ani_fixedarray_byte array, ani_size offset,
                                                           ani_size length, ani_byte *nativeBuffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(array);
    CHECK_PTR_ARG(nativeBuffer);
    return GetPrimitiveTypeArrayRegion(env, array, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_BindNativeMethods(ani_env *env, ani_class cls,
                                                         const ani_native_function *methods, ani_size nrMethods)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(cls);
    CHECK_PTR_ARG(methods);
    if (nrMethods == 0) {
        return ANI_OK;
    }
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsClass *klass = s.ToInternalType(cls);

    PandaVector<EtsMethod *> etsMethods;
    etsMethods.reserve(nrMethods);
    for (ani_size i = 0; i < nrMethods; ++i) {
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const char *signature = methods[i].signature;
        EtsMethod *method =
            (signature == nullptr ? klass->GetMethod(methods[i].name) : klass->GetMethod(methods[i].name, signature));
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        if (method == nullptr || !method->IsNative()) {
            return ANI_NOT_FOUND;
        }
        etsMethods.push_back(method);
    }
    for (ani_size i = 0; i < nrMethods; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        etsMethods[i]->RegisterNativeImpl(const_cast<void *>(methods[i].pointer));
    }
    return ANI_OK;
}

template <bool IS_STATIC_METHOD>
static ani_status DoGetMethod(ani_env *env, ani_class cls, const char *name, const char *signature, EtsMethod **result)
{
    ScopedManagedCodeFix s(PandaEnv::FromAniEnv(env));
    EtsClass *klass = GetInternalClass(&s, cls);
    if (UNLIKELY(klass == nullptr)) {
        if (s.GetCoroutine()->HasPendingException()) {
            return ANI_PENDING_ERROR;
        }
        return ANI_ERROR;
    }
    EtsMethod *method = (signature == nullptr ? klass->GetMethod(name) : klass->GetMethod(name, signature));
    if (method == nullptr || method->IsStatic() != IS_STATIC_METHOD) {
        return ANI_NOT_FOUND;
    }
    *result = method;
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
    ani_status status = DoGetMethod<false>(env, cls, name, signature, &method);
    if (UNLIKELY(status != ANI_OK)) {
        return status;
    }
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
    ani_status status = DoGetMethod<true>(env, cls, name, signature, &method);
    if (UNLIKELY(status != ANI_OK)) {
        return status;
    }
    *result = ToAniStaticMethod(method);
    return ANI_OK;
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
    if (UNLIKELY(internalString == nullptr)) {
        return ANI_OUT_OF_MEMORY;
    }
    *result = reinterpret_cast<ani_string>(s.AddLocalRef(reinterpret_cast<EtsObject *>(internalString)));
    return ANI_OK;
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
NO_UB_SANITIZE static ani_status GlobalReference_Create(ani_env *env, ani_ref ref,
                                                        [[maybe_unused]] uint32_t initialRefcount, ani_gref *result)
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
    NotImplementedAdapterVargs<22>,
    NotImplementedAdapter<23>,
    NotImplementedAdapter<24>,
    NotImplementedAdapter<25>,
    Object_InstanceOf,
    NotImplementedAdapter<27>,
    NotImplementedAdapter<28>,
    NotImplementedAdapter<29>,
    NotImplementedAdapter<30>,
    NotImplementedAdapter<31>,
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
    NotImplementedAdapter<45>,
    NotImplementedAdapter<46>,
    NotImplementedAdapter<47>,
    NotImplementedAdapter<48>,
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
    NotImplementedAdapter<62>,
    NotImplementedAdapter<63>,
    NotImplementedAdapter<64>,
    NotImplementedAdapter<65>,
    NotImplementedAdapter<66>,
    NotImplementedAdapter<67>,
    NotImplementedAdapter<68>,
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
    NotImplementedAdapter<89>,
    NotImplementedAdapter<90>,
    NotImplementedAdapter<91>,
    FixedArray_New_Byte,
    NotImplementedAdapter<93>,
    NotImplementedAdapter<94>,
    NotImplementedAdapter<95>,
    NotImplementedAdapter<96>,
    NotImplementedAdapter<97>,
    NotImplementedAdapter<98>,
    NotImplementedAdapter<99>,
    FixedArray_GetRegion_Byte,
    NotImplementedAdapter<101>,
    NotImplementedAdapter<102>,
    NotImplementedAdapter<103>,
    NotImplementedAdapter<104>,
    NotImplementedAdapter<105>,
    NotImplementedAdapter<106>,
    NotImplementedAdapter<107>,
    FixedArray_SetRegion_Byte,
    NotImplementedAdapter<109>,
    NotImplementedAdapter<110>,
    NotImplementedAdapter<111>,
    NotImplementedAdapter<112>,
    NotImplementedAdapter<113>,
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
    NotImplementedAdapterVargs<168>,
    NotImplementedAdapter<169>,
    NotImplementedAdapter<170>,
    NotImplementedAdapterVargs<171>,
    NotImplementedAdapter<172>,
    NotImplementedAdapter<173>,
    NotImplementedAdapter<174>,
    NotImplementedAdapter<175>,
    NotImplementedAdapter<176>,
    NotImplementedAdapter<177>,
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
    NotImplementedAdapterVargs<222>,
    NotImplementedAdapter<223>,
    NotImplementedAdapter<224>,
    NotImplementedAdapterVargs<225>,
    NotImplementedAdapter<226>,
    NotImplementedAdapter<227>,
    NotImplementedAdapterVargs<228>,
    NotImplementedAdapter<229>,
    NotImplementedAdapter<230>,
    NotImplementedAdapterVargs<231>,
    NotImplementedAdapter<232>,
    NotImplementedAdapter<233>,
    Class_CallStaticMethod_Int,
    Class_CallStaticMethod_Int_A,
    Class_CallStaticMethod_Int_V,
    NotImplementedAdapterVargs<237>,
    NotImplementedAdapter<238>,
    NotImplementedAdapter<239>,
    NotImplementedAdapterVargs<240>,
    NotImplementedAdapter<241>,
    NotImplementedAdapter<242>,
    NotImplementedAdapterVargs<243>,
    NotImplementedAdapter<244>,
    NotImplementedAdapter<245>,
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
    NotImplementedAdapter<287>,
    NotImplementedAdapter<288>,
    NotImplementedAdapter<289>,
    NotImplementedAdapter<290>,
    NotImplementedAdapter<291>,
    NotImplementedAdapter<292>,
    NotImplementedAdapter<293>,
    NotImplementedAdapter<294>,
    NotImplementedAdapter<295>,
    NotImplementedAdapter<296>,
    NotImplementedAdapter<297>,
    NotImplementedAdapter<298>,
    NotImplementedAdapter<299>,
    NotImplementedAdapter<300>,
    NotImplementedAdapter<301>,
    NotImplementedAdapter<302>,
    NotImplementedAdapter<303>,
    NotImplementedAdapter<304>,
    NotImplementedAdapter<305>,
    NotImplementedAdapter<306>,
    NotImplementedAdapter<307>,
    NotImplementedAdapter<308>,
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
    NotImplementedAdapter<340>,
    NotImplementedAdapter<341>,
    NotImplementedAdapter<342>,
    NotImplementedAdapter<343>,
    NotImplementedAdapter<344>,
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
    NotImplementedAdapterVargs<378>,
    NotImplementedAdapter<379>,
    NotImplementedAdapter<380>,
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
}  // namespace ark::ets::ani
