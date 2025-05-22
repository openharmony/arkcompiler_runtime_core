/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "plugins/ets/runtime/napi/ets_napi_native_interface.h"

#include "libpandabase/macros.h"
#include "libpandafile/shorty_iterator.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/bit_utils.h"
#include "libpandabase/utils/string_helpers.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/value.h"

#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/napi/ets_napi_macros.h"
#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_value.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/napi/ets_napi_helpers.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

#define ETS_NAPI_DEBUG_TRACE(env)
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define CHECK_ENV(env)                                                                                    \
    do {                                                                                                  \
        ets_env *e = env;                                                                                 \
        ETS_NAPI_RETURN_IF_EQ(::ark::ets::PandaEtsNapiEnv::ToPandaEtsEnv(e)->HasPendingException(), true, \
                              ETS_PENDING_EXCEPTION);                                                     \
        ETS_NAPI_RETURN_IF_EQ(e, nullptr, ETS_INVALID_ARG);                                               \
    } while (false)

#define CHECK_ARG(env, arg) ETS_NAPI_RETURN_IF_EQ(arg, nullptr, ETS_INVALID_ARG)
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace ark::ets::napi {
template <typename T>
using ArgVector = PandaSmallVector<T>;

using TypeId = panda_file::Type::TypeId;

// ETS NAPI allocator
void *EtsAlloc(size_t n)
{
    size_t sz = (n + 7) >> 3;  // NOLINT(hicpp-signed-bitwise)
    return new uint64_t[sz];
}

void EtsFree(void *obj)
{
    delete[] reinterpret_cast<uint64_t *>(obj);
}

template <typename T>
void EtsFree(const T *obj)
{
    EtsFree(reinterpret_cast<void *>(const_cast<T *>(obj)));
}

static Value ConstructValueFromFloatingPoint(float val)
{
    return Value(bit_cast<int32_t>(val));
}

static Value ConstructValueFromFloatingPoint(double val)
{
    return Value(bit_cast<int64_t>(val));
}

static ArgVector<Value> GetArgValues(ScopedManagedCodeFix *s, EtsMethod *method, va_list args, ets_object object)
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
                auto param = va_arg(args, ets_object);
                parsedArgs.emplace_back(param != nullptr ? s->ToInternalType(param)->GetCoreType() : nullptr);
                break;
            }
            default:
                LOG(FATAL, ETS_NAPI) << "Unexpected argument type";
                break;
        }
    }
    return parsedArgs;
}

static ArgVector<Value> GetArgValues(ScopedManagedCodeFix *s, EtsMethod *method, const ets_value *args,
                                     ets_object object)
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
                parsedArgs.emplace_back(arg->j);
                break;
            case TypeId::F32:
                parsedArgs.push_back(ConstructValueFromFloatingPoint(arg->f));
                break;
            case TypeId::F64:
                parsedArgs.push_back(ConstructValueFromFloatingPoint(arg->d));
                break;
            case TypeId::REFERENCE: {
                parsedArgs.emplace_back(s->ToInternalType(arg->l)->GetCoreType());
                break;
            }
            default:
                LOG(FATAL, ETS_NAPI) << "Unexpected argument type";
                break;
        }
        ++arg;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return parsedArgs;
}

EtsMethod *ToInternalType(ets_method methodId)
{
    return reinterpret_cast<EtsMethod *>(methodId);
}

ets_method ToEtsNapiType(EtsMethod *methodId)
{
    return reinterpret_cast<ets_method>(methodId);
}

EtsField *ToInternalType(ets_field fieldId)
{
    return reinterpret_cast<EtsField *>(fieldId);
}

ets_field ToEtsNapiType(EtsField *fieldId)
{
    return reinterpret_cast<ets_field>(fieldId);
}

static EtsMethod *ResolveVirtualMethod(ScopedManagedCodeFix *s, ets_object object, ets_method methodId)
{
    EtsMethod *method = ToInternalType(methodId);

    if (UNLIKELY(method->IsStatic())) {
        LOG(ERROR, ETS_NAPI) << "Called ResolveVirtualMethod of static method, invalid ETS NAPI usage";
        return method;
    }

    EtsObject *obj = s->ToInternalType(object);
    return obj->GetClass()->ResolveVirtualMethod(method);
}

template <bool IS_VIRTUAL, typename NapiType, typename EtsValueType, typename Args>
static NapiType GeneralMethodCall(EtsEnv *env, ets_object obj, ets_method methodId, Args args)
{
    EtsMethod *method = nullptr;
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    if constexpr (IS_VIRTUAL) {
        method = ResolveVirtualMethod(&s, obj, methodId);
    } else {
        method = ToInternalType(methodId);
    }
    ASSERT(method != nullptr);

    ArgVector<Value> values = GetArgValues(&s, method, args, obj);
    EtsValue res = method->Invoke(&s, const_cast<Value *>(values.data()));

    // Now NapiType and EtsValueType are the same, but later it could be changed
    static_assert(std::is_same_v<NapiType, EtsValueType>);
    return res.GetAs<EtsValueType>();
}

static void CheckMethodReturnType(ets_method methodId, EtsType type)
{
    EtsMethod *method = ToInternalType(methodId);
    if (method->GetReturnValueType() != type) {
        LOG(FATAL, ETS_NAPI) << "Return type mismatch";
    }
}

template <typename EtsNapiType, typename InternalType>
static inline EtsNapiType GetPrimitiveTypeField(EtsEnv *env, ets_object obj, ets_field fieldId)
{
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(fieldId);

    EtsField *internalFieldId = ToInternalType(fieldId);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalObject = s.ToInternalType(obj);

    return static_cast<EtsNapiType>(internalObject->GetFieldPrimitive<InternalType>(internalFieldId));
}

template <typename EtsNapiType>
static inline void SetPrimitiveTypeField(EtsEnv *env, ets_object obj, ets_field fieldId, EtsNapiType value)
{
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(fieldId);

    EtsField *internalFieldId = ToInternalType(fieldId);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalObject = s.ToInternalType(obj);

    internalObject->SetFieldPrimitive(internalFieldId, value);
}

template <typename EtsNapiType, typename InternalType>
static inline EtsNapiType GetPrimitiveTypeStaticField(EtsEnv *env, ets_field fieldId)
{
    ETS_NAPI_ABORT_IF_NULL(fieldId);

    EtsField *internalFieldId = ToInternalType(fieldId);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internalClass = internalFieldId->GetDeclaringClass();

    return static_cast<EtsNapiType>(internalClass->GetStaticFieldPrimitive<InternalType>(internalFieldId));
}

template <typename EtsNapiType>
static inline void SetPrimitiveTypeStaticField(EtsEnv *env, ets_field fieldId, EtsNapiType value)
{
    ETS_NAPI_ABORT_IF_NULL(fieldId);

    EtsField *internalFieldId = ToInternalType(fieldId);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internalClass = internalFieldId->GetDeclaringClass();

    internalClass->SetStaticFieldPrimitive(internalFieldId, value);
}

inline PandaString ToClassDescriptor(const char *className)
{
    if (className[0] == '[') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return className;
    }
    return PandaString("L") + className + ';';
}

ClassLinkerContext *GetClassLinkerContext(ScopedManagedCodeFix *soa)
{
    auto stack = StackWalker::Create(soa->Coroutine());
    if (!stack.HasFrame()) {
        return nullptr;
    }

    auto *method = EtsMethod::FromRuntimeMethod(stack.GetMethod());

    if (method != nullptr) {
        return method->GetClass()->GetLoadContext();
    }

    return nullptr;
}

static EtsClass *GetInternalClass(EtsEnv *env, ets_class cls, ScopedManagedCodeFix *s)
{
    EtsClass *klass = s->ToInternalType(cls);

    if (klass->IsInitialized()) {
        return klass;
    }

    // Initialize class
    EtsCoroutine *corutine = PandaEtsNapiEnv::ToPandaEtsEnv(env)->GetEtsCoroutine();
    EtsClassLinker *classLinker = corutine->GetPandaVM()->GetClassLinker();
    bool isInitialized = classLinker->InitializeClass(corutine, klass);
    if (!isInitialized) {
        LOG(ERROR, ETS_NAPI) << "Cannot initialize class: " << klass->GetDescriptor();
        return nullptr;
    }

    return klass;
}

template <typename EtsType, typename InternalType>
static EtsType NewPrimitiveTypeArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_ABORT_IF_LZ(length);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto *array = InternalType::Create(static_cast<uint32_t>(length));
    if (UNLIKELY(array == nullptr)) {
        return nullptr;
    }
    ets_object ret = s.AddLocalRef(reinterpret_cast<EtsObject *>(array));
    return reinterpret_cast<EtsType>(ret);
}

static void *PinRawDataOfPrimitiveArray(EtsEnv *env, ets_array array)
{
    ETS_NAPI_ABORT_IF_NULL(array);

    auto pandaEnv = PandaEtsNapiEnv::ToPandaEtsEnv(env);
    ScopedManagedCodeFix s(pandaEnv);

    auto vm = pandaEnv->GetEtsVM();
    if (!vm->GetGC()->IsPinningSupported()) {
        LOG(FATAL, ETS_NAPI) << "Pinning is not supported with " << mem::GCStringFromType(vm->GetGC()->GetType());
        return nullptr;
    }

    auto coreArray = s.ToInternalType(array)->GetCoreType();
    vm->GetHeapManager()->PinObject(coreArray);

    return coreArray->GetData();
}

template <typename T>
static T *PinPrimitiveTypeArray(EtsEnv *env, ets_array array)
{
    return reinterpret_cast<T *>(PinRawDataOfPrimitiveArray(env, array));
}

static void UnpinPrimitiveTypeArray(EtsEnv *env, ets_array array)
{
    ETS_NAPI_ABORT_IF_NULL(array);

    auto pandaEnv = PandaEtsNapiEnv::ToPandaEtsEnv(env);
    ScopedManagedCodeFix s(pandaEnv);

    auto coreArray = s.ToInternalType(array)->GetCoreType();
    auto vm = pandaEnv->GetEtsVM();
    vm->GetHeapManager()->UnpinObject(coreArray);
}

template <typename T>
static void GetPrimitiveTypeArrayRegion(EtsEnv *env, ets_array array, ets_size start, ets_size len, T *buf)
{
    ETS_NAPI_ABORT_IF_NULL(array);
    ETS_NAPI_ABORT_IF(len != 0 && buf == nullptr);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsArray *internalArray = s.ToInternalType(array);
    // NOLINTNEXTLINE(bugprone-misplaced-widening-cast)
    if (start < 0 || len < 0 || static_cast<size_t>(start + len) > internalArray->GetLength()) {
        PandaStringStream ss;
        ss << "Array index out of bounds: start = " << start << ", len = " << len
           << ", array size = " << internalArray->GetLength();
        s.ThrowNewException(EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto res = memcpy_s(buf, len * sizeof(T), internalArray->GetData<T>() + start, len * sizeof(T));
    if (res != 0) {
        UNREACHABLE();
    }
}

template <typename T>
static void SetPrimitiveTypeArrayRegion(EtsEnv *env, ets_array array, ets_size start, ets_size len, T *buf)
{
    ETS_NAPI_ABORT_IF_NULL(array);
    ETS_NAPI_ABORT_IF(len != 0 && buf == nullptr);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsArray *internalArray = s.ToInternalType(array);
    // NOLINTNEXTLINE(bugprone-misplaced-widening-cast)
    if (start < 0 || len < 0 || static_cast<size_t>(start + len) > internalArray->GetLength()) {
        PandaStringStream ss;
        ss << "Array index out of bounds: start = " << start << ", len = " << len
           << ", array size = " << internalArray->GetLength();
        s.ThrowNewException(EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto res =
        memcpy_s(internalArray->GetData<std::remove_const_t<T>>() + start, len * sizeof(T), buf, len * sizeof(T));
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (res != 0) {
        UNREACHABLE();
    }
}

// ETS NAPI implementation

NO_UB_SANITIZE static ets_int GetVersion([[maybe_unused]] EtsEnv *env)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return ETS_NAPI_VERSION_1_0;
}
// DefineClass,
NO_UB_SANITIZE static ets_class FindClass(EtsEnv *env, const char *name)
{
    ETS_NAPI_DEBUG_TRACE(env);

    auto etsEnv = PandaEtsNapiEnv::ToPandaEtsEnv(env);
    ScopedManagedCodeFix s(etsEnv);

    if (name == nullptr) {
        s.ThrowNewException(EtsNapiException::NO_CLASS_DEF_FOUND, "Null pointer passed as a class name");
        return nullptr;
    }

    PandaString classDescriptor = ToClassDescriptor(name);
    EtsClassLinker *classLinker = etsEnv->GetEtsVM()->GetClassLinker();
    EtsClass *klass = classLinker->GetClass(classDescriptor.c_str(), true, GetClassLinkerContext(&s));

    if (etsEnv->HasPendingException()) {
        EtsThrowable *currentException = etsEnv->GetThrowable();
        std::string_view exceptionString = currentException->GetClass()->GetDescriptor();
        if (exceptionString == panda_file_items::class_descriptors::LINKER_CLASS_NOT_FOUND_ERROR) {
            etsEnv->ClearException();

            PandaStringStream ss;
            ss << "Class '" << name << "' is not found";

            s.ThrowNewException(EtsNapiException::NO_CLASS_DEF_FOUND, ss.str().c_str());

            return nullptr;
        }
    }
    ETS_NAPI_RETURN_IF_EQ(klass, nullptr, nullptr);
    ASSERT_MANAGED_CODE();
    return reinterpret_cast<ets_class>(s.AddLocalRef(reinterpret_cast<EtsObject *>(klass)));
}
// FromReflectedMethod,
// FromReflectedField,
// ToReflectedMethod,
NO_UB_SANITIZE static ets_class GetSuperclass(EtsEnv *env, ets_class cls)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto baseCls = s.ToInternalType(cls)->GetBase();
    ETS_NAPI_RETURN_IF_EQ(baseCls, nullptr, nullptr);
    ASSERT_MANAGED_CODE();
    return reinterpret_cast<ets_class>(s.AddLocalRef(reinterpret_cast<EtsObject *>(baseCls)));
}

NO_UB_SANITIZE static ets_boolean IsAssignableFrom(EtsEnv *env, ets_class cls1, ets_class cls2)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls1);
    ETS_NAPI_ABORT_IF_NULL(cls2);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *klass1 = s.ToInternalType(cls1);
    EtsClass *klass2 = s.ToInternalType(cls2);

    return klass2->IsAssignableFrom(klass1) ? ETS_TRUE : ETS_FALSE;
}
// ToReflectedField,
NO_UB_SANITIZE static ets_boolean ErrorCheck(EtsEnv *env)
{
    ETS_NAPI_DEBUG_TRACE(env);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    return static_cast<ets_boolean>(PandaEtsNapiEnv::ToPandaEtsEnv(env)->HasPendingException());
}

NO_UB_SANITIZE static ets_int ThrowError(EtsEnv *env, ets_error obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    if (obj == nullptr) {
        return ETS_ERR;
    }

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsThrowable *exception = s.ToInternalType(obj);
    PandaEtsNapiEnv::ToPandaEtsEnv(env)->SetException(exception);
    return ETS_OK;
}

NO_UB_SANITIZE static ets_int ThrowErrorNew(EtsEnv *env, ets_class cls, const char *message)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);

    PandaEtsNapiEnv *pandaEtsEnv = PandaEtsNapiEnv::ToPandaEtsEnv(env);

    ScopedManagedCodeFix s(pandaEtsEnv);
    EtsClass *exceptionClass = s.ToInternalType(cls);
    EtsCoroutine *coroutine = pandaEtsEnv->GetEtsCoroutine();
    ThrowEtsException(coroutine, exceptionClass->GetDescriptor(), message);
    return ETS_OK;
}

NO_UB_SANITIZE static ets_error ErrorOccurred(EtsEnv *env)
{
    ETS_NAPI_DEBUG_TRACE(env);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsThrowable *error = PandaEtsNapiEnv::ToPandaEtsEnv(env)->GetThrowable();
    if (error == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<ets_error>(s.AddLocalRef(error));
}

NO_UB_SANITIZE static void ErrorDescribe(EtsEnv *env)
{
    ETS_NAPI_DEBUG_TRACE(env);

    auto error = env->ErrorOccurred();
    if (error == nullptr) {
        return;
    }

    env->ErrorClear();

    auto errorKlass = env->GetObjectClass(error);
    auto toStringMethod = env->Getp_method(errorKlass, "toString", ":Lstd/core/String;");
    auto errorString = env->CallObjectMethod(error, toStringMethod);

    auto consoleKlass = env->FindClass("std/core/Console");

    auto coreGlobalClass = env->FindClass("std/core/ETSGLOBAL");
    ETS_NAPI_ABORT_IF_NULL(coreGlobalClass);
    auto consoleField = env->GetStaticp_field(coreGlobalClass, "console", "Lstd/core/Console;");
    ETS_NAPI_ABORT_IF_NULL(consoleField);
    auto consoleObj = env->GetStaticObjectField(coreGlobalClass, consoleField);

    auto printlnMethod = env->Getp_method(consoleKlass, "println", "Lstd/core/String;:V");
    env->CallVoidMethod(consoleObj, printlnMethod, errorString);
    env->ThrowError(error);
}

NO_UB_SANITIZE static void ErrorClear(EtsEnv *env)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    PandaEtsNapiEnv::ToPandaEtsEnv(env)->ClearException();
}

NO_UB_SANITIZE static void FatalError([[maybe_unused]] EtsEnv *env, const char *message)
{
    ETS_NAPI_DEBUG_TRACE(env);
    LOG(FATAL, ETS_NAPI) << "EtsNapiFatalError: " << message;
}

NO_UB_SANITIZE static ets_int PushLocalFrame(EtsEnv *env, ets_int capacity)
{
    ETS_NAPI_DEBUG_TRACE(env);

    if (capacity <= 0) {
        return ETS_ERR_INVAL;
    }

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    bool ret = PandaEtsNapiEnv::ToPandaEtsEnv(env)->GetEtsReferenceStorage()->PushLocalEtsFrame(
        static_cast<uint32_t>(capacity));
    if (!ret) {
        PandaStringStream ss;
        ss << "Could not allocate " << capacity << "bytes";
        s.ThrowNewException(EtsNapiException::OUT_OF_MEMORY, ss.str().c_str());
        return ETS_ERR;
    }
    return ETS_OK;
}

NO_UB_SANITIZE static ets_object PopLocalFrame(EtsEnv *env, ets_object result)
{
    ETS_NAPI_DEBUG_TRACE(env);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsReference *resultEtsRef = EtsObjectToEtsRef(result);
    EtsReference *etsRef =
        PandaEtsNapiEnv::ToPandaEtsEnv(env)->GetEtsReferenceStorage()->PopLocalEtsFrame(resultEtsRef);
    return EtsRefToEtsObject(etsRef);
}

NO_UB_SANITIZE static ets_object NewGlobalRef(EtsEnv *env, ets_object obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_IF_EQ(obj, nullptr, nullptr);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalObject = s.ToInternalType(obj);
    return s.AddGlobalRef(internalObject);
}

NO_UB_SANITIZE static void DeleteGlobalRef([[maybe_unused]] EtsEnv *env, ets_object globalRef)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_VOID_IF_NULL(globalRef);

    PandaEtsVM *etsVm = PandaEtsVM::GetCurrent();
    etsVm->DeleteGlobalRef(globalRef);
}

NO_UB_SANITIZE static void DeleteLocalRef(EtsEnv *env, ets_object localRef)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_VOID_IF_NULL(localRef);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    s.DelLocalRef(localRef);
}

NO_UB_SANITIZE static ets_boolean IsSameObject(EtsEnv *env, ets_object ref1, ets_object ref2)
{
    ETS_NAPI_DEBUG_TRACE(env);

    if (ref1 == ref2) {
        return ETS_TRUE;
    }
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    return s.ToInternalType(ref1) == s.ToInternalType(ref2) ? ETS_TRUE : ETS_FALSE;
}

NO_UB_SANITIZE static ets_object NewLocalRef(EtsEnv *env, ets_object ref)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_IF_EQ(ref, nullptr, nullptr);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalObject = s.ToInternalType(ref);
    return s.AddLocalRef(internalObject);
}

NO_UB_SANITIZE static ets_int EnsureLocalCapacity(EtsEnv *env, ets_int capacity)
{
    ETS_NAPI_DEBUG_TRACE(env);
    if (capacity < 0) {
        return ETS_ERR_INVAL;
    }

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    bool ret =
        PandaEtsNapiEnv::ToPandaEtsEnv(env)->GetEtsReferenceStorage()->GetAsReferenceStorage()->EnsureLocalCapacity(
            static_cast<size_t>(capacity));
    if (!ret) {
        PandaStringStream ss;
        ss << "Could not ensure " << capacity << "bytes";
        s.ThrowNewException(EtsNapiException::OUT_OF_MEMORY, ss.str().c_str());
        return ETS_ERR;
    }
    return ETS_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ets_method Getp_method(EtsEnv *env, ets_class cls, const char *name, const char *sig)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(name);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *klass = GetInternalClass(env, cls, &s);
    if (klass == nullptr) {
        return nullptr;
    }

    EtsMethod *method = klass->GetInstanceMethod(name, sig);
    if (method == nullptr || method->IsStatic()) {
        PandaStringStream ss;
        ss << "Method " << klass->GetRuntimeClass()->GetName() << "::" << name
           << " sig = " << (sig == nullptr ? "nullptr" : sig) << " is not found";
        s.ThrowNewException(EtsNapiException::NO_SUCH_METHOD, ss.str().c_str());
        return nullptr;
    }

    return ToEtsNapiType(method);
}

NO_UB_SANITIZE static ets_object CallObjectMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::OBJECT);
    return GeneralMethodCall<true, ets_object, ets_object>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_object CallObjectMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_object res = CallObjectMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_object CallObjectMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                       const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::OBJECT);
    return GeneralMethodCall<true, ets_object, ets_object>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_boolean CallBooleanMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BOOLEAN);
    return GeneralMethodCall<true, ets_boolean, EtsBoolean>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_boolean CallBooleanMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_boolean res = CallBooleanMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_boolean CallBooleanMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                         const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BOOLEAN);
    return GeneralMethodCall<true, ets_boolean, EtsBoolean>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_byte CallByteMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BYTE);
    return GeneralMethodCall<true, ets_byte, EtsByte>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_byte CallByteMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_byte res = CallByteMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_byte CallByteMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                   const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BYTE);
    return GeneralMethodCall<true, ets_byte, EtsByte>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_char CallCharMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::CHAR);
    return GeneralMethodCall<true, ets_char, EtsChar>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_char CallCharMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_char res = CallCharMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_char CallCharMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                   const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::CHAR);
    return GeneralMethodCall<true, ets_char, EtsChar>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_short CallShortMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::SHORT);
    return GeneralMethodCall<true, ets_short, EtsShort>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_short CallShortMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_short res = CallShortMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_short CallShortMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                     const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::SHORT);
    return GeneralMethodCall<true, ets_short, EtsShort>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_int CallIntMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::INT);
    return GeneralMethodCall<true, ets_int, EtsInt>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_int CallIntMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_int res = CallIntMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_int CallIntMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                 const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::INT);
    return GeneralMethodCall<true, ets_int, EtsInt>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_long CallLongMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::LONG);
    return GeneralMethodCall<true, ets_long, EtsLong>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_long CallLongMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_long res = CallLongMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_long CallLongMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                   const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::LONG);
    return GeneralMethodCall<true, ets_long, EtsLong>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_float CallFloatMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::FLOAT);
    return GeneralMethodCall<true, ets_float, EtsFloat>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_float CallFloatMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_float res = CallFloatMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_float CallFloatMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                     const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::FLOAT);
    return GeneralMethodCall<true, ets_float, EtsFloat>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_double CallDoubleMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::DOUBLE);
    return GeneralMethodCall<true, ets_double, EtsDouble>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_double CallDoubleMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_double res = CallDoubleMethodList(env, obj, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_double CallDoubleMethodArray(EtsEnv *env, ets_object obj, ets_method methodId,
                                                       const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::DOUBLE);
    return GeneralMethodCall<true, ets_double, EtsDouble>(env, obj, methodId, args);
}

NO_UB_SANITIZE static void CallVoidMethodList(EtsEnv *env, ets_object obj, ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::VOID);
    // Use any primitive type as template parameter and jist ignore the result
    GeneralMethodCall<true, ets_boolean, EtsBoolean>(env, obj, methodId, args);
}

NO_UB_SANITIZE static void CallVoidMethod(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    CallVoidMethodList(env, obj, methodId, args);
    va_end(args);
}

NO_UB_SANITIZE static void CallVoidMethodArray(EtsEnv *env, ets_object obj, ets_method methodId, const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::VOID);
    // Use any primitive type as template parameter and jist ignore the result
    GeneralMethodCall<true, ets_boolean, EtsBoolean>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_object CallNonvirtualObjectMethodList(EtsEnv *env, ets_object obj,
                                                                [[maybe_unused]] ets_class cls, ets_method methodId,
                                                                va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::OBJECT);
    return GeneralMethodCall<false, ets_object, ets_object>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_object CallNonvirtualObjectMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                            ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_object res = CallNonvirtualObjectMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_object CallNonvirtualObjectMethodArray(EtsEnv *env, ets_object obj,
                                                                 [[maybe_unused]] ets_class cls, ets_method methodId,
                                                                 const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::OBJECT);
    return GeneralMethodCall<false, ets_object, ets_object>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_boolean CallNonvirtualBooleanMethodList(EtsEnv *env, ets_object obj,
                                                                  [[maybe_unused]] ets_class cls, ets_method methodId,
                                                                  va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BOOLEAN);
    return GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_boolean CallNonvirtualBooleanMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                              ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_boolean res = CallNonvirtualBooleanMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_boolean CallNonvirtualBooleanMethodArray(EtsEnv *env, ets_object obj,
                                                                   [[maybe_unused]] ets_class cls, ets_method methodId,
                                                                   const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BOOLEAN);
    return GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_byte CallNonvirtualByteMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                            ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BYTE);
    return GeneralMethodCall<false, ets_byte, EtsByte>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_byte CallNonvirtualByteMethod(EtsEnv *env, ets_object obj, ets_class cls, ets_method methodId,
                                                        ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_byte res = CallNonvirtualByteMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_byte CallNonvirtualByteMethodArray(EtsEnv *env, ets_object obj,
                                                             [[maybe_unused]] ets_class cls, ets_method methodId,
                                                             const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BYTE);
    return GeneralMethodCall<false, ets_byte, EtsByte>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_char CallNonvirtualCharMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                            ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::CHAR);
    return GeneralMethodCall<false, ets_char, EtsChar>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_char CallNonvirtualCharMethod(EtsEnv *env, ets_object obj, ets_class cls, ets_method methodId,
                                                        ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_char res = CallNonvirtualCharMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_char CallNonvirtualCharMethodArray(EtsEnv *env, ets_object obj,
                                                             [[maybe_unused]] ets_class cls, ets_method methodId,
                                                             const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::CHAR);
    return GeneralMethodCall<false, ets_char, EtsChar>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_short CallNonvirtualShortMethodList(EtsEnv *env, ets_object obj,
                                                              [[maybe_unused]] ets_class cls, ets_method methodId,
                                                              va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::SHORT);
    return GeneralMethodCall<false, ets_short, EtsShort>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_short CallNonvirtualShortMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                          ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_short res = CallNonvirtualShortMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_short CallNonvirtualShortMethodArray(EtsEnv *env, ets_object obj,
                                                               [[maybe_unused]] ets_class cls, ets_method methodId,
                                                               const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::SHORT);
    return GeneralMethodCall<false, ets_short, EtsShort>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_int CallNonvirtualIntMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                          ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::INT);
    return GeneralMethodCall<false, ets_int, EtsInt>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_int CallNonvirtualIntMethod(EtsEnv *env, ets_object obj, ets_class cls, ets_method methodId,
                                                      ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_int res = CallNonvirtualIntMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_int CallNonvirtualIntMethodArray(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                           ets_method methodId, const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::INT);
    return GeneralMethodCall<false, ets_int, EtsInt>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_long CallNonvirtualLongMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                            ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::LONG);
    return GeneralMethodCall<false, ets_long, EtsLong>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_long CallNonvirtualLongMethod(EtsEnv *env, ets_object obj, ets_class cls, ets_method methodId,
                                                        ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_long res = CallNonvirtualLongMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_long CallNonvirtualLongMethodArray(EtsEnv *env, ets_object obj,
                                                             [[maybe_unused]] ets_class cls, ets_method methodId,
                                                             const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::LONG);
    return GeneralMethodCall<false, ets_long, EtsLong>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_float CallNonvirtualFloatMethodList(EtsEnv *env, ets_object obj,
                                                              [[maybe_unused]] ets_class cls, ets_method methodId,
                                                              va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::FLOAT);
    return GeneralMethodCall<false, ets_float, EtsFloat>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_float CallNonvirtualFloatMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                          ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_float res = CallNonvirtualFloatMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_float CallNonvirtualFloatMethodArray(EtsEnv *env, ets_object obj,
                                                               [[maybe_unused]] ets_class cls, ets_method methodId,
                                                               const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::FLOAT);
    return GeneralMethodCall<false, ets_float, EtsFloat>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_double CallNonvirtualDoubleMethodList(EtsEnv *env, ets_object obj,
                                                                [[maybe_unused]] ets_class cls, ets_method methodId,
                                                                va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::DOUBLE);
    return GeneralMethodCall<false, ets_double, EtsDouble>(env, obj, methodId, args);
}

NO_UB_SANITIZE static ets_double CallNonvirtualDoubleMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                            ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_double res = CallNonvirtualDoubleMethodList(env, obj, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_double CallNonvirtualDoubleMethodArray(EtsEnv *env, ets_object obj,
                                                                 [[maybe_unused]] ets_class cls, ets_method methodId,
                                                                 const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::DOUBLE);
    return GeneralMethodCall<false, ets_double, EtsDouble>(env, obj, methodId, args);
}

NO_UB_SANITIZE static void CallNonvirtualVoidMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                        ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::VOID);
    // Use any primitive type as template parameter and jist ignore the result
    GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, obj, methodId, args);
}

NO_UB_SANITIZE static void CallNonvirtualVoidMethod(EtsEnv *env, ets_object obj, ets_class cls, ets_method methodId,
                                                    ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    CallNonvirtualVoidMethodList(env, obj, cls, methodId, args);
    va_end(args);
}

NO_UB_SANITIZE static void CallNonvirtualVoidMethodArray(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                         ets_method methodId, const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::VOID);
    // Use any primitive type as template parameter and jist ignore the result
    GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, obj, methodId, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ets_field Getp_field(EtsEnv *env, ets_class cls, const char *name, const char *sig)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(name);
    ETS_NAPI_ABORT_IF_NULL(sig);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internalClass = GetInternalClass(env, cls, &s);
    if (internalClass == nullptr) {
        return nullptr;
    }

    EtsField *internalFieldId = internalClass->GetFieldIDByName(name, sig);
    if (internalFieldId == nullptr) {
        PandaStringStream ss;
        ss << "Field " << internalClass->GetRuntimeClass()->GetName() << "::" << name << " sig = " << sig
           << " is not found";
        s.ThrowNewException(EtsNapiException::NO_SUCH_FIELD, ss.str().c_str());
        return nullptr;
    }

    return ToEtsNapiType(internalFieldId);
}

NO_UB_SANITIZE static ets_object GetObjectField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(pField);

    EtsField *internalFieldId = ToInternalType(pField);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalObject = s.ToInternalType(obj);
    EtsObject *retObj = internalObject->GetFieldObject(internalFieldId);
    if (retObj == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<ets_object>(s.AddLocalRef(retObj));
}

NO_UB_SANITIZE static ets_boolean GetBooleanField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_boolean, EtsBoolean>(env, obj, pField);
}

NO_UB_SANITIZE static ets_byte GetByteField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_byte, EtsByte>(env, obj, pField);
}

NO_UB_SANITIZE static ets_char GetCharField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_char, EtsChar>(env, obj, pField);
}

NO_UB_SANITIZE static ets_short GetShortField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_short, EtsShort>(env, obj, pField);
}

NO_UB_SANITIZE static ets_int GetIntField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_int, EtsInt>(env, obj, pField);
}

NO_UB_SANITIZE static ets_long GetLongField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_long, EtsLong>(env, obj, pField);
}

NO_UB_SANITIZE static ets_float GetFloatField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_float, EtsFloat>(env, obj, pField);
}

NO_UB_SANITIZE static ets_double GetDoubleField(EtsEnv *env, ets_object obj, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_double, EtsDouble>(env, obj, pField);
}

NO_UB_SANITIZE static void SetObjectField(EtsEnv *env, ets_object obj, ets_field pField, ets_object value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(pField);

    EtsField *internalFieldId = ToInternalType(pField);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalValue = s.ToInternalType(value);

    s.ToInternalType(obj)->SetFieldObject(internalFieldId, internalValue);
}

NO_UB_SANITIZE static void SetBooleanField(EtsEnv *env, ets_object obj, ets_field pField, ets_boolean value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, pField, value);
}

NO_UB_SANITIZE static void SetByteField(EtsEnv *env, ets_object obj, ets_field pField, ets_byte value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, pField, value);
}

NO_UB_SANITIZE static void SetCharField(EtsEnv *env, ets_object obj, ets_field pField, ets_char value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, pField, value);
}

NO_UB_SANITIZE static void SetShortField(EtsEnv *env, ets_object obj, ets_field pField, ets_short value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, pField, value);
}

NO_UB_SANITIZE static void SetIntField(EtsEnv *env, ets_object obj, ets_field pField, ets_int value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, pField, value);
}

NO_UB_SANITIZE static void SetLongField(EtsEnv *env, ets_object obj, ets_field pField, ets_long value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, pField, value);
}

NO_UB_SANITIZE static void SetFloatField(EtsEnv *env, ets_object obj, ets_field pField, ets_float value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, pField, value);
}

NO_UB_SANITIZE static void SetDoubleField(EtsEnv *env, ets_object obj, ets_field pField, ets_double value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, pField, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ets_method GetStaticp_method(EtsEnv *env, ets_class cls, const char *name, const char *sig)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(name);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *klass = GetInternalClass(env, cls, &s);
    if (klass == nullptr) {
        return nullptr;
    }

    EtsMethod *method = klass->GetStaticMethod(name, sig);
    if (method == nullptr || !method->IsStatic()) {
        PandaStringStream ss;
        ss << "Static method " << klass->GetRuntimeClass()->GetName() << "::" << name
           << " sig = " << (sig == nullptr ? "nullptr" : sig) << " is not found";
        s.ThrowNewException(EtsNapiException::NO_SUCH_METHOD, ss.str().c_str());
        return nullptr;
    }

    return ToEtsNapiType(method);
}

NO_UB_SANITIZE static ets_object CallStaticObjectMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                            ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::OBJECT);
    return GeneralMethodCall<false, ets_object, ets_object>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_object CallStaticObjectMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_object res = CallStaticObjectMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_object CallStaticObjectMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                             ets_method methodId, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::OBJECT);
    return GeneralMethodCall<false, ets_object, ets_object>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_boolean CallStaticBooleanMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                              ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BOOLEAN);
    return GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_boolean CallStaticBooleanMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_boolean res = CallStaticBooleanMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_boolean CallStaticBooleanMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                               ets_method methodId, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BOOLEAN);
    return GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_byte CallStaticByteMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BYTE);
    return GeneralMethodCall<false, ets_byte, EtsByte>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_byte CallStaticByteMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_byte res = CallStaticByteMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_byte CallStaticByteMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         ets_method methodId, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::BYTE);
    return GeneralMethodCall<false, ets_byte, EtsByte>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_char CallStaticCharMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::CHAR);
    return GeneralMethodCall<false, ets_char, EtsChar>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_char CallStaticCharMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_char res = CallStaticCharMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_char CallStaticCharMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         ets_method methodId, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::CHAR);
    return GeneralMethodCall<false, ets_char, EtsChar>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_short CallStaticShortMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                          ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::SHORT);
    return GeneralMethodCall<false, ets_short, EtsShort>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_short CallStaticShortMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_short res = CallStaticShortMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_short CallStaticShortMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                           ets_method methodId, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::SHORT);
    return GeneralMethodCall<false, ets_short, EtsShort>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_int CallStaticIntMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls, ets_method methodId,
                                                      va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::INT);
    return GeneralMethodCall<false, ets_int, EtsInt>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_int CallStaticIntMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_int res = CallStaticIntMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_int CallStaticIntMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls, ets_method methodId,
                                                       ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::INT);
    return GeneralMethodCall<false, ets_int, EtsInt>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_long CallStaticLongMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::LONG);
    return GeneralMethodCall<false, ets_long, EtsLong>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_long CallStaticLongMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_long res = CallStaticLongMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_long CallStaticLongMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         ets_method methodId, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::LONG);
    return GeneralMethodCall<false, ets_long, EtsLong>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_float CallStaticFloatMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                          ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::FLOAT);
    return GeneralMethodCall<false, ets_float, EtsFloat>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_float CallStaticFloatMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_float res = CallStaticFloatMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_float CallStaticFloatMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                           ets_method methodId, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::FLOAT);
    return GeneralMethodCall<false, ets_float, EtsFloat>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_double CallStaticDoubleMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                            ets_method methodId, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::DOUBLE);
    return GeneralMethodCall<false, ets_double, EtsDouble>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static ets_double CallStaticDoubleMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    ets_double res = CallStaticDoubleMethodList(env, cls, methodId, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_double CallStaticDoubleMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                             ets_method methodId, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::DOUBLE);
    return GeneralMethodCall<false, ets_double, EtsDouble>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static void CallStaticVoidMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls, ets_method methodId,
                                                    va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::VOID);
    // Use any primitive type as template parameter and just ignore the result
    GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, nullptr, methodId, args);
}

NO_UB_SANITIZE static void CallStaticVoidMethod(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, methodId);
    CallStaticVoidMethodList(env, cls, methodId, args);
    va_end(args);
}

NO_UB_SANITIZE static void CallStaticVoidMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls, ets_method methodId,
                                                     ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(methodId);
    CheckMethodReturnType(methodId, EtsType::VOID);
    // Use any primitive type as template parameter and just ignore the result
    GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, nullptr, methodId, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ets_field GetStaticp_field(EtsEnv *env, ets_class cls, const char *name, const char *sig)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(name);
    ETS_NAPI_ABORT_IF_NULL(sig);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internalClass = GetInternalClass(env, cls, &s);
    if (internalClass == nullptr) {
        return nullptr;
    }

    EtsField *internalFieldId = internalClass->GetStaticFieldIDByName(name, sig);
    if (internalFieldId == nullptr) {
        PandaStringStream ss;
        ss << "Static field " << internalClass->GetRuntimeClass()->GetName() << "::" << name << " sig = " << sig
           << " is not found";
        s.ThrowNewException(EtsNapiException::NO_SUCH_FIELD, ss.str().c_str());
        return nullptr;
    }

    return ToEtsNapiType(internalFieldId);
}

NO_UB_SANITIZE static ets_object GetStaticObjectField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls,
                                                      ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(pField);

    EtsField *internalFieldId = ToInternalType(pField);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internalClass = internalFieldId->GetDeclaringClass();
    EtsObject *retObj = internalClass->GetStaticFieldObject(internalFieldId);
    if (retObj == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<ets_object>(s.AddLocalRef(retObj));
}

NO_UB_SANITIZE static ets_boolean GetStaticBooleanField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls,
                                                        ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_boolean, EtsBoolean>(env, pField);
}

NO_UB_SANITIZE static ets_byte GetStaticByteField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_byte, EtsByte>(env, pField);
}

NO_UB_SANITIZE static ets_char GetStaticCharField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_char, EtsChar>(env, pField);
}

NO_UB_SANITIZE static ets_short GetStaticShortField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_short, EtsShort>(env, pField);
}

NO_UB_SANITIZE static ets_int GetStaticIntField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_int, EtsInt>(env, pField);
}

NO_UB_SANITIZE static ets_long GetStaticLongField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_long, EtsLong>(env, pField);
}

NO_UB_SANITIZE static ets_float GetStaticFloatField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_float, EtsFloat>(env, pField);
}

NO_UB_SANITIZE static ets_double GetStaticDoubleField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls,
                                                      ets_field pField)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_double, EtsDouble>(env, pField);
}

NO_UB_SANITIZE static void SetStaticObjectField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                                ets_object value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(pField);

    EtsField *internalFieldId = ToInternalType(pField);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalValue = s.ToInternalType(value);

    internalFieldId->GetDeclaringClass()->SetStaticFieldObject(internalFieldId, internalValue);
}

NO_UB_SANITIZE static void SetStaticBooleanField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                                 ets_boolean value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, pField, value);
}

NO_UB_SANITIZE static void SetStaticByteField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                              ets_byte value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, pField, value);
}

NO_UB_SANITIZE static void SetStaticCharField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                              ets_char value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, pField, value);
}

NO_UB_SANITIZE static void SetStaticShortField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                               ets_short value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, pField, value);
}

NO_UB_SANITIZE static void SetStaticIntField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                             ets_int value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, pField, value);
}

NO_UB_SANITIZE static void SetStaticLongField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                              ets_long value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, pField, value);
}

NO_UB_SANITIZE static void SetStaticFloatField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                               ets_float value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, pField, value);
}

NO_UB_SANITIZE static void SetStaticDoubleField(EtsEnv *env, [[maybe_unused]] ets_class unusedCls, ets_field pField,
                                                ets_double value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, pField, value);
}

NO_UB_SANITIZE static ets_string NewString(EtsEnv *env, const ets_char *unicodeChars, ets_size len)
{
    ETS_NAPI_DEBUG_TRACE(env);
    if (unicodeChars == nullptr) {
        ETS_NAPI_ABORT_IF_NE(len, 0);
    }
    ETS_NAPI_ABORT_IF_LZ(len);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internalString = EtsString::CreateFromUtf16(unicodeChars, len);
    if (internalString == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<ets_string>(s.AddLocalRef(reinterpret_cast<EtsObject *>(internalString)));
}

NO_UB_SANITIZE static ets_size GetStringLength(EtsEnv *env, ets_string string)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internalString = s.ToInternalType(string);
    return static_cast<ets_size>(internalString->GetLength());
}

NO_UB_SANITIZE static const ets_char *GetStringChars(EtsEnv *env, ets_string string, ets_boolean *isCopy)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    if (isCopy != nullptr) {
        *isCopy = ETS_TRUE;
    }
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internalString = s.ToInternalType(string);
    size_t len = internalString->GetUtf16Length();
    void *buf = EtsAlloc(len * sizeof(uint16_t));
    if (buf == nullptr) {
        LOG(ERROR, ETS_NAPI) << __func__ << ": cannot copy string";
        return nullptr;
    }
    internalString->CopyDataUtf16(buf, len);
    return static_cast<ets_char *>(buf);
}

NO_UB_SANITIZE static void ReleaseStringChars([[maybe_unused]] EtsEnv *env, ets_string string, const ets_char *chars)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    EtsFree(chars);
}

NO_UB_SANITIZE static ets_string NewStringUTF(EtsEnv *env, const char *bytes)
{
    ETS_NAPI_DEBUG_TRACE(env);

    // NOTE(m.morozov): check if this right solution
    if (bytes == nullptr) {
        return nullptr;
    }

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    auto internalString = EtsString::CreateFromMUtf8(bytes);
    if (internalString == nullptr) {
        s.ThrowNewException(EtsNapiException::OUT_OF_MEMORY, "Could not allocate memory for string");
        return nullptr;
    }
    return reinterpret_cast<ets_string>(s.AddLocalRef(reinterpret_cast<EtsObject *>(internalString)));
}

NO_UB_SANITIZE static ets_size GetStringUTFLength(EtsEnv *env, ets_string string)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internalString = s.ToInternalType(string);
    // NOTE(m.morozov): ensure that place for \0 is included
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    return internalString->GetMUtf8Length() - 1;
}

NO_UB_SANITIZE static const char *GetStringUTFChars(EtsEnv *env, ets_string string, ets_boolean *isCopy)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    if (isCopy != nullptr) {
        *isCopy = ETS_TRUE;
    }
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internalString = s.ToInternalType(string);
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    size_t len = internalString->GetMUtf8Length();
    void *buf = EtsAlloc(len);
    if (buf == nullptr) {
        LOG(ERROR, ETS_NAPI) << __func__ << ": cannot copy string";
        return nullptr;
    }
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    internalString->CopyDataMUtf8(buf, len, true);
    return static_cast<char *>(buf);
}

NO_UB_SANITIZE static void ReleaseStringUTFChars([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_string string,
                                                 const char *chars)
{
    ETS_NAPI_DEBUG_TRACE(env);
    EtsFree(chars);
}

NO_UB_SANITIZE static ets_size GetArrayLength(EtsEnv *env, ets_array array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(array);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsArray *internalArray = s.ToInternalType(array);
    return static_cast<ets_size>(internalArray->GetLength());
}

// NOTE(kropacheva): change name after conflicts resolved
NO_UB_SANITIZE static ets_objectArray NewObjectsArray(EtsEnv *env, ets_size length, ets_class elementClass,
                                                      ets_object initialElement)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_LZ(length);
    ETS_NAPI_ABORT_IF_NULL(elementClass);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internalClass = s.ToInternalType(elementClass);
    EtsObjectArray *array = EtsObjectArray::Create(internalClass, static_cast<uint32_t>(length));
    if (array == nullptr) {
        PandaStringStream ss;
        ss << "Could not allocate array of " << internalClass->GetRuntimeClass()->GetName() << " of " << length
           << " elements";
        s.ThrowNewException(EtsNapiException::OUT_OF_MEMORY, ss.str().c_str());
        return nullptr;
    }
    if (initialElement != nullptr) {
        for (decltype(length) i = 0; i < length; ++i) {
            array->Set(static_cast<uint32_t>(i), s.ToInternalType(initialElement));
        }
    }
    return reinterpret_cast<ets_objectArray>(s.AddLocalRef(reinterpret_cast<EtsObject *>(array)));
}

NO_UB_SANITIZE static ets_object GetObjectArrayElement(EtsEnv *env, ets_objectArray array, ets_size index)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(array);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObjectArray *internalArray = s.ToInternalType(array);
    if (index < 0 || index >= static_cast<ets_size>(internalArray->GetLength())) {
        PandaStringStream ss;
        ss << "Could not access " << index << " element, array length = " << internalArray->GetLength();
        s.ThrowNewException(EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return nullptr;
    }

    return s.AddLocalRef(internalArray->Get(static_cast<uint32_t>(index)));
}

NO_UB_SANITIZE static void SetObjectArrayElement(EtsEnv *env, ets_objectArray array, ets_size index, ets_object value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(array);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObjectArray *internalArray = s.ToInternalType(array);
    if (index < 0 || index >= static_cast<ets_size>(internalArray->GetLength())) {
        PandaStringStream ss;
        ss << "Could not access " << index << " element, array length = " << internalArray->GetLength();
        s.ThrowNewException(EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    EtsObject *internalValue = s.ToInternalType(value);
    if (internalValue != nullptr) {
        auto *componentClass = internalArray->GetClass()->GetComponentType();
        if (!internalValue->IsInstanceOf(componentClass)) {
            PandaStringStream ss;
            ss << internalValue->GetClass()->GetRuntimeClass()->GetName();
            ss << "cannot be stored in an array of type ";
            ss << componentClass->GetRuntimeClass()->GetName();
            s.ThrowNewException(EtsNapiException::ARRAY_STORE, ss.str().c_str());
            return;
        }
    }
    internalArray->Set(static_cast<uint32_t>(index), internalValue);
}

NO_UB_SANITIZE static ets_booleanArray NewBooleanArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return NewPrimitiveTypeArray<ets_booleanArray, EtsBooleanArray>(env, length);
}

NO_UB_SANITIZE static ets_byteArray NewByteArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return NewPrimitiveTypeArray<ets_byteArray, EtsByteArray>(env, length);
}

NO_UB_SANITIZE static ets_charArray NewCharArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return NewPrimitiveTypeArray<ets_charArray, EtsCharArray>(env, length);
}

NO_UB_SANITIZE static ets_shortArray NewShortArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return NewPrimitiveTypeArray<ets_shortArray, EtsShortArray>(env, length);
}

NO_UB_SANITIZE static ets_intArray NewIntArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return NewPrimitiveTypeArray<ets_intArray, EtsIntArray>(env, length);
}

NO_UB_SANITIZE static ets_longArray NewLongArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return NewPrimitiveTypeArray<ets_longArray, EtsLongArray>(env, length);
}

NO_UB_SANITIZE static ets_floatArray NewFloatArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return NewPrimitiveTypeArray<ets_floatArray, EtsFloatArray>(env, length);
}

NO_UB_SANITIZE static ets_doubleArray NewDoubleArray(EtsEnv *env, ets_size length)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return NewPrimitiveTypeArray<ets_doubleArray, EtsDoubleArray>(env, length);
}

NO_UB_SANITIZE static ets_boolean *PinBooleanArray(EtsEnv *env, ets_booleanArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return PinPrimitiveTypeArray<ets_boolean>(env, array);
}

NO_UB_SANITIZE static ets_byte *PinByteArray(EtsEnv *env, ets_byteArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return PinPrimitiveTypeArray<ets_byte>(env, array);
}

NO_UB_SANITIZE static ets_char *PinCharArray(EtsEnv *env, ets_charArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return PinPrimitiveTypeArray<ets_char>(env, array);
}

NO_UB_SANITIZE static ets_short *PinShortArray(EtsEnv *env, ets_shortArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return PinPrimitiveTypeArray<ets_short>(env, array);
}

NO_UB_SANITIZE static ets_int *PinIntArray(EtsEnv *env, ets_intArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return PinPrimitiveTypeArray<ets_int>(env, array);
}

NO_UB_SANITIZE static ets_long *PinLongArray(EtsEnv *env, ets_longArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return PinPrimitiveTypeArray<ets_long>(env, array);
}

NO_UB_SANITIZE static ets_float *PinFloatArray(EtsEnv *env, ets_floatArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return PinPrimitiveTypeArray<ets_float>(env, array);
}

NO_UB_SANITIZE static ets_double *PinDoubleArray(EtsEnv *env, ets_doubleArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return PinPrimitiveTypeArray<ets_double>(env, array);
}

NO_UB_SANITIZE static void UnpinBooleanArray(EtsEnv *env, ets_booleanArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    UnpinPrimitiveTypeArray(env, array);
}

NO_UB_SANITIZE static void UnpinByteArray(EtsEnv *env, ets_byteArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    UnpinPrimitiveTypeArray(env, array);
}

NO_UB_SANITIZE static void UnpinCharArray(EtsEnv *env, ets_charArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    UnpinPrimitiveTypeArray(env, array);
}

NO_UB_SANITIZE static void UnpinShortArray(EtsEnv *env, ets_shortArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    UnpinPrimitiveTypeArray(env, array);
}

NO_UB_SANITIZE static void UnpinIntArray(EtsEnv *env, ets_intArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    UnpinPrimitiveTypeArray(env, array);
}

NO_UB_SANITIZE static void UnpinLongArray(EtsEnv *env, ets_longArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    UnpinPrimitiveTypeArray(env, array);
}

NO_UB_SANITIZE static void UnpinFloatArray(EtsEnv *env, ets_floatArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    UnpinPrimitiveTypeArray(env, array);
}

NO_UB_SANITIZE static void UnpinDoubleArray(EtsEnv *env, ets_doubleArray array)
{
    ETS_NAPI_DEBUG_TRACE(env);
    UnpinPrimitiveTypeArray(env, array);
}

NO_UB_SANITIZE static void GetBooleanArrayRegion(EtsEnv *env, ets_booleanArray array, ets_size start, ets_size len,
                                                 ets_boolean *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    GetPrimitiveTypeArrayRegion(env, array, start, len, buf);
}

NO_UB_SANITIZE static void GetByteArrayRegion(EtsEnv *env, ets_byteArray array, ets_size start, ets_size len,
                                              ets_byte *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    GetPrimitiveTypeArrayRegion(env, array, start, len, buf);
}

NO_UB_SANITIZE static void GetCharArrayRegion(EtsEnv *env, ets_charArray array, ets_size start, ets_size len,
                                              ets_char *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    GetPrimitiveTypeArrayRegion(env, array, start, len, buf);
}

NO_UB_SANITIZE static void GetShortArrayRegion(EtsEnv *env, ets_shortArray array, ets_size start, ets_size len,
                                               ets_short *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    GetPrimitiveTypeArrayRegion(env, array, start, len, buf);
}

NO_UB_SANITIZE static void GetIntArrayRegion(EtsEnv *env, ets_intArray array, ets_size start, ets_size len,
                                             ets_int *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    GetPrimitiveTypeArrayRegion(env, array, start, len, buf);
}

NO_UB_SANITIZE static void GetLongArrayRegion(EtsEnv *env, ets_longArray array, ets_size start, ets_size len,
                                              ets_long *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    GetPrimitiveTypeArrayRegion(env, array, start, len, buf);
}

NO_UB_SANITIZE static void GetFloatArrayRegion(EtsEnv *env, ets_floatArray array, ets_size start, ets_size len,
                                               ets_float *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    GetPrimitiveTypeArrayRegion(env, array, start, len, buf);
}

NO_UB_SANITIZE static void GetDoubleArrayRegion(EtsEnv *env, ets_doubleArray array, ets_size start, ets_size len,
                                                ets_double *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    GetPrimitiveTypeArrayRegion(env, array, start, len, buf);
}

NO_UB_SANITIZE static void SetBooleanArrayRegion(EtsEnv *env, ets_booleanArray array, ets_size start, ets_size length,
                                                 const ets_boolean *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeArrayRegion(env, array, start, length, buf);
}

NO_UB_SANITIZE static void SetByteArrayRegion(EtsEnv *env, ets_byteArray array, ets_size start, ets_size length,
                                              const ets_byte *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeArrayRegion(env, array, start, length, buf);
}

NO_UB_SANITIZE static void SetCharArrayRegion(EtsEnv *env, ets_charArray array, ets_size start, ets_size length,
                                              const ets_char *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeArrayRegion(env, array, start, length, buf);
}

NO_UB_SANITIZE static void SetShortArrayRegion(EtsEnv *env, ets_shortArray array, ets_size start, ets_size length,
                                               const ets_short *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeArrayRegion(env, array, start, length, buf);
}

NO_UB_SANITIZE static void SetIntArrayRegion(EtsEnv *env, ets_intArray array, ets_size start, ets_size length,
                                             const ets_int *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeArrayRegion(env, array, start, length, buf);
}

NO_UB_SANITIZE static void SetLongArrayRegion(EtsEnv *env, ets_longArray array, ets_size start, ets_size length,
                                              const ets_long *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeArrayRegion(env, array, start, length, buf);
}

NO_UB_SANITIZE static void SetFloatArrayRegion(EtsEnv *env, ets_floatArray array, ets_size start, ets_size length,
                                               const ets_float *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeArrayRegion(env, array, start, length, buf);
}

NO_UB_SANITIZE static void SetDoubleArrayRegion(EtsEnv *env, ets_doubleArray array, ets_size start, ets_size length,
                                                const ets_double *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeArrayRegion(env, array, start, length, buf);
}

// NOTE(#18101): this workaround should be removed after the annotations are implemented.
static bool ExtractNativeMethodInfo(const char *signature, uint32_t *outNativeFlag, const char **outSignature)
{
    ASSERT(outSignature);

    *outNativeFlag = 0;
    *outSignature = signature;

    if (signature == nullptr) {
        return true;
    }
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (signature[0U] == '#') {
        if (signature[1U] == '\0' || signature[2U] != '$') {
            return false;
        }

        if (signature[1U] == 'F') {
            *outNativeFlag |= ACC_FAST_NATIVE;
        } else if (signature[1U] == 'C') {
            *outNativeFlag |= ACC_CRITICAL_NATIVE;
        } else {
            return false;
        }

        *outSignature = signature + 3U;
        if (**outSignature == '\0') {
            *outSignature = nullptr;
        }
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return true;
}

// NOTE(#18101): this workaround should be removed after the annotations are implemented.
static void SetNativeCallType(EtsMethod *method, uint32_t nativeFlag)
{
    ASSERT(method);

    method->GetPandaMethod()->SetAccessFlags(method->GetAccessFlags() | nativeFlag);

    if (method->IsCriticalNative()) {
        EtsMethod::ToRuntimeMethod(method)->SetCompiledEntryPoint(GetEtsNapiCriticalEntryPoint());
    }  // EtsNapiEntryPoint is set by default.
}

NO_UB_SANITIZE static ets_int RegisterNatives([[maybe_unused]] EtsEnv *env, ets_class cls,
                                              const EtsNativeMethod *methods, ets_int nMethods)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_LZ(nMethods);
    if (nMethods == 0) {
        return ETS_OK;
    }
    ETS_NAPI_ABORT_IF_NULL(methods);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *klass = s.ToInternalType(cls);
    for (ets_int i = 0; i < nMethods; ++i) {
        const char *signature = nullptr;
        uint32_t nativeFlag = 0;

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (!ExtractNativeMethodInfo(methods[i].signature, &nativeFlag, &signature)) {
            PandaStringStream ss;
            ss << "Method " << klass->GetRuntimeClass()->GetName() << "::" << methods[i].name
               << " sig = " << (signature == nullptr ? "nullptr" : signature)
               << " has an invalid signature. Signature prefix for fast native: #F$, for critical native: #C$";
            s.ThrowNewException(EtsNapiException::NO_SUCH_METHOD, ss.str().c_str());
            return ETS_ERR_INVAL;
        }

        EtsMethod *method = klass->GetInstanceMethod(methods[i].name, nullptr);
        method = method == nullptr ? klass->GetStaticMethod(methods[i].name, nullptr) : method;
        if (method == nullptr || !method->IsNative()) {
            PandaStringStream ss;
            ss << "Method " << klass->GetRuntimeClass()->GetName() << "::" << methods[i].name
               << " sig = " << (signature == nullptr ? "nullptr" : signature) << " not found or not native";
            s.ThrowNewException(EtsNapiException::NO_SUCH_METHOD, ss.str().c_str());
            return ETS_ERR_INVAL;
        }

        SetNativeCallType(method, nativeFlag);
        method->RegisterNativeDeprecated(methods[i].func);
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return ETS_OK;
}

NO_UB_SANITIZE static ets_int UnregisterNatives(EtsEnv *env, ets_class cls)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *klass = s.ToInternalType(cls);
    klass->EnumerateMethods([](EtsMethod *method) {
        if (method->IsNative()) {
            method->UnregisterNativeDeprecated();
        }
        return false;
    });
    return ETS_OK;
}

NO_UB_SANITIZE static ets_int GetEtsVM([[maybe_unused]] EtsEnv *env, EtsVM **vm)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(vm);

    if (Runtime::GetCurrent() == nullptr) {
        *vm = nullptr;
        return ETS_ERR;
    }
    *vm = PandaEtsVM::GetCurrent();
    if (*vm == nullptr) {
        return ETS_ERR;
    }
    return ETS_OK;
}

NO_UB_SANITIZE static void GetStringRegion(EtsEnv *env, ets_string str, ets_size start, ets_size len, ets_char *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(str);
    ETS_NAPI_ABORT_IF_NULL(buf);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsString *internalString = s.ToInternalType(str);
    // NOLINTNEXTLINE(bugprone-misplaced-widening-cast)
    if (start < 0 || len < 0 || static_cast<size_t>(start + len) > internalString->GetUtf16Length()) {
        PandaStringStream ss;
        ss << "String index out of bounds: start = " << start << ", len = " << len
           << ", string size = " << internalString->GetUtf16Length();
        s.ThrowNewException(EtsNapiException::STRING_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    internalString->CopyDataRegionUtf16(buf, start, len, len);
}

NO_UB_SANITIZE static void GetStringUTFRegion(EtsEnv *env, ets_string str, ets_size start, ets_size len, char *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(str);
    ETS_NAPI_ABORT_IF_NULL(buf);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsString *internalString = s.ToInternalType(str);
    // NOLINTNEXTLINE(bugprone-misplaced-widening-cast)
    if (start < 0 || len < 0 || static_cast<size_t>(start + len) > internalString->GetUtf16Length()) {
        PandaStringStream ss;
        ss << "String index out of bounds: start = " << start << ", len = " << len
           << ", string size = " << internalString->GetUtf16Length();
        s.ThrowNewException(EtsNapiException::STRING_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    internalString->CopyDataRegionMUtf8(buf, start, len, internalString->GetMUtf8Length());
}

NO_UB_SANITIZE static ets_object AllocObject(EtsEnv *env, ets_class cls)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internalClass = GetInternalClass(env, cls, &s);
    if (internalClass == nullptr) {
        return nullptr;
    }

    if (internalClass->IsAbstract() || internalClass->IsInterface()) {
        PandaStringStream ss;
        ss << "Class " << internalClass->GetRuntimeClass()->GetName() << " is interface or abstract";
        s.ThrowNewException(EtsNapiException::INSTANTIATION, ss.str().c_str());
        return nullptr;
    }
    if (internalClass->GetRuntimeClass()->IsStringClass()) {
        EtsString *str = EtsString::CreateNewEmptyString();
        if (UNLIKELY(str == nullptr)) {
            return nullptr;
        }
        return s.AddLocalRef(reinterpret_cast<EtsObject *>(str));
    }

    EtsObject *obj = EtsObject::Create(internalClass);
    if (UNLIKELY(obj == nullptr)) {
        return nullptr;
    }
    return s.AddLocalRef(reinterpret_cast<EtsObject *>(obj));
}

NO_UB_SANITIZE static ets_object NewObjectList(EtsEnv *env, ets_class cls, ets_method pMethod, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(pMethod);

    ets_object newObject = AllocObject(env, cls);
    if (newObject == nullptr) {
        return nullptr;
    }
    CallNonvirtualVoidMethodList(env, newObject, cls, pMethod, args);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    if (PandaEtsNapiEnv::ToPandaEtsEnv(env)->HasPendingException()) {
        return nullptr;
    }
    return newObject;
}

NO_UB_SANITIZE static ets_object NewObject(EtsEnv *env, ets_class cls, ets_method pMethod, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(pMethod);

    va_list args;
    va_start(args, pMethod);
    ets_object res = NewObjectList(env, cls, pMethod, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_object NewObjectArray(EtsEnv *env, ets_class cls, ets_method pMethod, const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(pMethod);

    ets_object newObject = AllocObject(env, cls);
    if (newObject == nullptr) {
        return nullptr;
    }
    CallNonvirtualVoidMethodArray(env, newObject, cls, pMethod, args);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    if (PandaEtsNapiEnv::ToPandaEtsEnv(env)->HasPendingException()) {
        return nullptr;
    }
    return newObject;
}

NO_UB_SANITIZE static ets_class GetObjectClass(EtsEnv *env, ets_object obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalObject = s.ToInternalType(obj);

    return reinterpret_cast<ets_class>(s.AddLocalRef(reinterpret_cast<EtsObject *>(internalObject->GetClass())));
}

NO_UB_SANITIZE static ets_boolean IsInstanceOf(EtsEnv *env, ets_object obj, ets_class cls)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    if (obj == nullptr) {
        return ETS_TRUE;
    }

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internalClass = s.ToInternalType(cls);
    EtsObject *internalObject = s.ToInternalType(obj);

    return internalObject->IsInstanceOf(internalClass) ? ETS_TRUE : ETS_FALSE;
}

NO_UB_SANITIZE static ets_objectRefType GetObjectRefType(EtsEnv *env, ets_object obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    if (UNLIKELY(obj == nullptr || !s.IsValidRef(obj))) {
        return ETS_INVALID_REF_TYPE;
    }

    switch (mem::ReferenceStorage::GetObjectType(reinterpret_cast<ark::mem::Reference *>(obj))) {
        case ark::mem::Reference::ObjectType::GLOBAL:
            return ETS_GLOBAL_REF_TYPE;
        case ark::mem::Reference::ObjectType::WEAK:
            return ETS_WEAK_GLOBAL_REF_TYPE;
        case ark::mem::Reference::ObjectType::LOCAL:
        case ark::mem::Reference::ObjectType::STACK:
            return ETS_LOCAL_REF_TYPE;
        default:
            UNREACHABLE();
    }
    UNREACHABLE();
}

NO_UB_SANITIZE static ets_weak NewWeakGlobalRef(EtsEnv *env, ets_object obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_IF_EQ(obj, nullptr, nullptr);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internalObject = s.ToInternalType(obj);
    ets_weak ret = s.AddWeakGlobalRef(internalObject);
    if (ret == nullptr) {
        s.ThrowNewException(EtsNapiException::OUT_OF_MEMORY, "Could not allocate object");
        return nullptr;
    }
    return ret;
}

NO_UB_SANITIZE static void DeleteWeakGlobalRef([[maybe_unused]] EtsEnv *env, ets_weak obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_VOID_IF_NULL(obj);

    PandaEtsVM *etsVm = PandaEtsVM::GetCurrent();
    etsVm->DeleteWeakGlobalRef(obj);
}

NO_UB_SANITIZE static ets_status PromiseCreate(EtsEnv *env, ets_deferred *deferred, ets_object *promise)
{
    ETS_NAPI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, promise);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsPromise *internalPromise = EtsPromise::Create(s.Coroutine());
    if (UNLIKELY(internalPromise == nullptr)) {
        return ETS_GENERIC_FAILURE;
    }

    auto *promiseObj = reinterpret_cast<EtsObject *>(internalPromise);
    *promise = s.AddLocalRef(promiseObj);
    *deferred = reinterpret_cast<ets_deferred>(s.AddGlobalRef(promiseObj));
    return ETS_OKAY;
}

NO_UB_SANITIZE static ets_status DeferredResolve(EtsEnv *env, ets_deferred deferred, ets_object resolution)
{
    ETS_NAPI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, resolution);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsPromise *promise = s.ToInternalType(deferred);
    EtsObject *value = s.ToInternalType(resolution);
    EtsCoroutine *coro = PandaEtsNapiEnv::ToPandaEtsEnv(env)->GetEtsCoroutine();

    promise->Resolve(coro, value);
    DeleteGlobalRef(env, reinterpret_cast<ets_object>(deferred));
    return ETS_OKAY;
}

NO_UB_SANITIZE static ets_status DeferredReject(EtsEnv *env, ets_deferred deferred, ets_object rejection)
{
    ETS_NAPI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, rejection);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsPromise *promise = s.ToInternalType(deferred);
    EtsObject *error = s.ToInternalType(rejection);
    EtsCoroutine *coro = PandaEtsNapiEnv::ToPandaEtsEnv(env)->GetEtsCoroutine();

    promise->Reject(coro, error);
    DeleteGlobalRef(env, reinterpret_cast<ets_object>(deferred));
    return ETS_OKAY;
}

static ets_status DoCreateArrayBuffer(ets_env *env, void *externalData, size_t length, EtsFinalize finalizeCB,
                                      void *finalizeHint, void **resultData, ets_arraybuffer *resultBuffer,
                                      bool external)
{
    CHECK_ARG(env, resultBuffer);
    ETS_NAPI_RETURN_IF_GT(length, static_cast<size_t>(std::numeric_limits<ets_int>::max()), ETS_INVALID_ARG);

    PandaEtsNapiEnv *pandaEnv = PandaEtsNapiEnv::ToPandaEtsEnv(env);
    EtsCoroutine *coro = pandaEnv->GetEtsCoroutine();

    ScopedManagedCodeFix s(pandaEnv);

    EtsEscompatArrayBuffer *internalArrayBuffer = nullptr;
    if (external) {
        internalArrayBuffer = EtsEscompatArrayBuffer::Create(coro, externalData, length, finalizeCB, finalizeHint);
    } else {
        internalArrayBuffer = EtsEscompatArrayBuffer::Create(coro, length, resultData);
    }

    ETS_NAPI_RETURN_IF_EQ(coro->HasPendingException(), true, ETS_PENDING_EXCEPTION);
    ASSERT(internalArrayBuffer != nullptr);

    ets_object arrayObject = s.AddLocalRef(internalArrayBuffer);
    ETS_NAPI_RETURN_IF_EQ(coro->HasPendingException(), true, ETS_PENDING_EXCEPTION);
    ASSERT(arrayObject != nullptr);

    *resultBuffer = reinterpret_cast<ets_arraybuffer>(arrayObject);
    return ETS_OKAY;
}

NO_UB_SANITIZE static ets_status ArrayBufferCreate(ets_env *env, size_t byteLength, void **resultData,
                                                   ets_arraybuffer *resultBuffer)
{
    ETS_NAPI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, resultData);
    return DoCreateArrayBuffer(env, nullptr, byteLength, nullptr, nullptr, resultData, resultBuffer, false);
}

NO_UB_SANITIZE static ets_status ArrayBufferCreateExternal(ets_env *env, void *externalData, size_t byteLength,
                                                           EtsFinalize finalizeCB, void *finalizeHint,
                                                           ets_arraybuffer *resultBuffer)
{
    ETS_NAPI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, finalizeCB);
    CHECK_ARG(env, externalData);
    return DoCreateArrayBuffer(env, externalData, byteLength, finalizeCB, finalizeHint, nullptr, resultBuffer, true);
}

NO_UB_SANITIZE static ets_status ArrayBufferGetInfo(ets_env *env, ets_arraybuffer buffer, void **resultData,
                                                    size_t *resultByteLength)
{
    ETS_NAPI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, buffer);
    CHECK_ARG(env, resultData);
    CHECK_ARG(env, resultByteLength);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto *internalArrayBuffer = s.Convert<EtsEscompatArrayBuffer>(buffer);
    *resultData = internalArrayBuffer->GetData();
    *resultByteLength = internalArrayBuffer->GetByteLength();
    return ETS_OKAY;
}

NO_UB_SANITIZE static ets_status ArrayBufferDetach(ets_env *env, ets_arraybuffer buffer)
{
    ETS_NAPI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, buffer);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto *internalArrayBuffer = s.Convert<EtsEscompatArrayBuffer>(buffer);
    if (!internalArrayBuffer->IsDetachable()) {
        return ETS_DETACHABLE_ARRAYBUFFER_EXPECTED;
    }
    internalArrayBuffer->Detach();
    return ETS_OKAY;
}

NO_UB_SANITIZE static ets_status ArrayBufferIsDetached(ets_env *env, ets_arraybuffer buffer, bool *result)
{
    ETS_NAPI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, buffer);
    CHECK_ARG(env, result);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto *internalArrayBuffer = s.Convert<EtsEscompatArrayBuffer>(buffer);
    *result = internalArrayBuffer->WasDetached();
    return ETS_OKAY;
}

// NewDirectByteBuffer,
// GetDirectBufferAddress,
// GetDirectBufferCapacity,

// clang-format off
const ETS_NativeInterface NATIVE_INTERFACE = {
    GetVersion,
    // DefineClass,
    FindClass,
    // FromReflectedMethod,
    // FromReflectedField,
    // ToReflectedMethod,
    GetSuperclass,
    IsAssignableFrom,
    // ToReflectedField,
    ThrowError,
    ThrowErrorNew,
    ErrorOccurred,
    ErrorDescribe,
    ErrorClear,
    FatalError,
    PushLocalFrame,
    PopLocalFrame,
    NewGlobalRef,
    DeleteGlobalRef,
    DeleteLocalRef,
    IsSameObject,
    NewLocalRef,
    EnsureLocalCapacity,
    AllocObject,
    NewObject,
    NewObjectList,
    NewObjectArray,
    GetObjectClass,
    IsInstanceOf,
    Getp_method,
    CallObjectMethod,
    CallObjectMethodList,
    CallObjectMethodArray,
    CallBooleanMethod,
    CallBooleanMethodList,
    CallBooleanMethodArray,
    CallByteMethod,
    CallByteMethodList,
    CallByteMethodArray,
    CallCharMethod,
    CallCharMethodList,
    CallCharMethodArray,
    CallShortMethod,
    CallShortMethodList,
    CallShortMethodArray,
    CallIntMethod,
    CallIntMethodList,
    CallIntMethodArray,
    CallLongMethod,
    CallLongMethodList,
    CallLongMethodArray,
    CallFloatMethod,
    CallFloatMethodList,
    CallFloatMethodArray,
    CallDoubleMethod,
    CallDoubleMethodList,
    CallDoubleMethodArray,
    CallVoidMethod,
    CallVoidMethodList,
    CallVoidMethodArray,
    CallNonvirtualObjectMethod,
    CallNonvirtualObjectMethodList,
    CallNonvirtualObjectMethodArray,
    CallNonvirtualBooleanMethod,
    CallNonvirtualBooleanMethodList,
    CallNonvirtualBooleanMethodArray,
    CallNonvirtualByteMethod,
    CallNonvirtualByteMethodList,
    CallNonvirtualByteMethodArray,
    CallNonvirtualCharMethod,
    CallNonvirtualCharMethodList,
    CallNonvirtualCharMethodArray,
    CallNonvirtualShortMethod,
    CallNonvirtualShortMethodList,
    CallNonvirtualShortMethodArray,
    CallNonvirtualIntMethod,
    CallNonvirtualIntMethodList,
    CallNonvirtualIntMethodArray,
    CallNonvirtualLongMethod,
    CallNonvirtualLongMethodList,
    CallNonvirtualLongMethodArray,
    CallNonvirtualFloatMethod,
    CallNonvirtualFloatMethodList,
    CallNonvirtualFloatMethodArray,
    CallNonvirtualDoubleMethod,
    CallNonvirtualDoubleMethodList,
    CallNonvirtualDoubleMethodArray,
    CallNonvirtualVoidMethod,
    CallNonvirtualVoidMethodList,
    CallNonvirtualVoidMethodArray,
    Getp_field,
    GetObjectField,
    GetBooleanField,
    GetByteField,
    GetCharField,
    GetShortField,
    GetIntField,
    GetLongField,
    GetFloatField,
    GetDoubleField,
    SetObjectField,
    SetBooleanField,
    SetByteField,
    SetCharField,
    SetShortField,
    SetIntField,
    SetLongField,
    SetFloatField,
    SetDoubleField,
    GetStaticp_method,
    CallStaticObjectMethod,
    CallStaticObjectMethodList,
    CallStaticObjectMethodArray,
    CallStaticBooleanMethod,
    CallStaticBooleanMethodList,
    CallStaticBooleanMethodArray,
    CallStaticByteMethod,
    CallStaticByteMethodList,
    CallStaticByteMethodArray,
    CallStaticCharMethod,
    CallStaticCharMethodList,
    CallStaticCharMethodArray,
    CallStaticShortMethod,
    CallStaticShortMethodList,
    CallStaticShortMethodArray,
    CallStaticIntMethod,
    CallStaticIntMethodList,
    CallStaticIntMethodArray,
    CallStaticLongMethod,
    CallStaticLongMethodList,
    CallStaticLongMethodArray,
    CallStaticFloatMethod,
    CallStaticFloatMethodList,
    CallStaticFloatMethodArray,
    CallStaticDoubleMethod,
    CallStaticDoubleMethodList,
    CallStaticDoubleMethodArray,
    CallStaticVoidMethod,
    CallStaticVoidMethodList,
    CallStaticVoidMethodArray,
    GetStaticp_field,
    GetStaticObjectField,
    GetStaticBooleanField,
    GetStaticByteField,
    GetStaticCharField,
    GetStaticShortField,
    GetStaticIntField,
    GetStaticLongField,
    GetStaticFloatField,
    GetStaticDoubleField,
    SetStaticObjectField,
    SetStaticBooleanField,
    SetStaticByteField,
    SetStaticCharField,
    SetStaticShortField,
    SetStaticIntField,
    SetStaticLongField,
    SetStaticFloatField,
    SetStaticDoubleField,
    NewString,
    GetStringLength,
    GetStringChars,
    ReleaseStringChars,
    NewStringUTF,
    GetStringUTFLength,
    GetStringUTFChars,
    ReleaseStringUTFChars,
    GetArrayLength,
    NewObjectsArray,
    GetObjectArrayElement,
    SetObjectArrayElement,
    NewBooleanArray,
    NewByteArray,
    NewCharArray,
    NewShortArray,
    NewIntArray,
    NewLongArray,
    NewFloatArray,
    NewDoubleArray,
    PinBooleanArray,
    PinByteArray,
    PinCharArray,
    PinShortArray,
    PinIntArray,
    PinLongArray,
    PinFloatArray,
    PinDoubleArray,
    UnpinBooleanArray,
    UnpinByteArray,
    UnpinCharArray,
    UnpinShortArray,
    UnpinIntArray,
    UnpinLongArray,
    UnpinFloatArray,
    UnpinDoubleArray,
    GetBooleanArrayRegion,
    GetByteArrayRegion,
    GetCharArrayRegion,
    GetShortArrayRegion,
    GetIntArrayRegion,
    GetLongArrayRegion,
    GetFloatArrayRegion,
    GetDoubleArrayRegion,
    SetBooleanArrayRegion,
    SetByteArrayRegion,
    SetCharArrayRegion,
    SetShortArrayRegion,
    SetIntArrayRegion,
    SetLongArrayRegion,
    SetFloatArrayRegion,
    SetDoubleArrayRegion,
    RegisterNatives,
    UnregisterNatives,
    GetEtsVM,
    GetStringRegion,
    GetStringUTFRegion,
    NewWeakGlobalRef,
    DeleteWeakGlobalRef,
    ErrorCheck,
    // NewDirectByteBuffer,
    // GetDirectBufferAddress,
    // GetDirectBufferCapacity,
    GetObjectRefType,
    PromiseCreate,
    DeferredResolve,
    DeferredReject,
    ArrayBufferCreate,
    ArrayBufferCreateExternal,
    ArrayBufferGetInfo,
    ArrayBufferDetach,
    ArrayBufferIsDetached,
};
// clang-format on

const ETS_NativeInterface *GetNativeInterface()
{
    return &NATIVE_INTERFACE;
}
}  // namespace ark::ets::napi

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
