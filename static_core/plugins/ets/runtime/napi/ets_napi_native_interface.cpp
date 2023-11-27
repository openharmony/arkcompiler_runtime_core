/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

#define ETS_NAPI_DEBUG_TRACE(env)

namespace panda::ets::napi {
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
    ArgVector<Value> parsed_args;
    parsed_args.reserve(method->GetNumArgs());
    if (object != nullptr) {
        parsed_args.emplace_back(s->ToInternalType(object)->GetCoreType());
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
                parsed_args.emplace_back(va_arg(args, uint32_t));
                break;
            case TypeId::I8:
            case TypeId::I16:
            case TypeId::I32:
                parsed_args.emplace_back(va_arg(args, int32_t));
                break;
            case TypeId::I64:
                parsed_args.emplace_back(va_arg(args, int64_t));
                break;
            case TypeId::F32:
                parsed_args.push_back(ConstructValueFromFloatingPoint(static_cast<float>(va_arg(args, double))));
                break;
            case TypeId::F64:
                parsed_args.push_back(ConstructValueFromFloatingPoint(va_arg(args, double)));
                break;
            case TypeId::REFERENCE: {
                auto param = va_arg(args, ets_object);
                parsed_args.emplace_back(param != nullptr ? s->ToInternalType(param)->GetCoreType() : nullptr);
                break;
            }
            default:
                LOG(FATAL, ETS_NAPI) << "Unexpected argument type";
                break;
        }
    }
    return parsed_args;
}

static ArgVector<Value> GetArgValues(ScopedManagedCodeFix *s, EtsMethod *method, const ets_value *args,
                                     ets_object object)
{
    ArgVector<Value> parsed_args;
    parsed_args.reserve(method->GetNumArgs());
    if (object != nullptr) {
        parsed_args.emplace_back(s->ToInternalType(object)->GetCoreType());
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
                parsed_args.emplace_back(arg->z);
                break;
            case TypeId::U16:
                parsed_args.emplace_back(arg->c);
                break;
            case TypeId::I8:
                parsed_args.emplace_back(arg->b);
                break;
            case TypeId::I16:
                parsed_args.emplace_back(arg->s);
                break;
            case TypeId::I32:
                parsed_args.emplace_back(arg->i);
                break;
            case TypeId::I64:
                parsed_args.emplace_back(arg->j);
                break;
            case TypeId::F32:
                parsed_args.push_back(ConstructValueFromFloatingPoint(arg->f));
                break;
            case TypeId::F64:
                parsed_args.push_back(ConstructValueFromFloatingPoint(arg->d));
                break;
            case TypeId::REFERENCE: {
                parsed_args.emplace_back(s->ToInternalType(arg->l)->GetCoreType());
                break;
            }
            default:
                LOG(FATAL, ETS_NAPI) << "Unexpected argument type";
                break;
        }
        ++arg;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return parsed_args;
}

EtsMethod *ToInternalType(ets_method method_id)
{
    return reinterpret_cast<EtsMethod *>(method_id);
}

ets_method ToEtsNapiType(EtsMethod *method_id)
{
    return reinterpret_cast<ets_method>(method_id);
}

EtsField *ToInternalType(ets_field field_id)
{
    return reinterpret_cast<EtsField *>(field_id);
}

ets_field ToEtsNapiType(EtsField *field_id)
{
    return reinterpret_cast<ets_field>(field_id);
}

static EtsMethod *ResolveVirtualMethod(ScopedManagedCodeFix *s, ets_object object, ets_method method_id)
{
    EtsMethod *method = ToInternalType(method_id);

    if (UNLIKELY(method->IsStatic())) {
        LOG(ERROR, ETS_NAPI) << "Called ResolveVirtualMethod of static method, invalid ETS NAPI usage";
        return method;
    }

    EtsObject *obj = s->ToInternalType(object);
    return obj->GetClass()->ResolveVirtualMethod(method);
}

template <bool IS_VIRTUAL, typename NapiType, typename EtsValueType, typename Args>
static NapiType GeneralMethodCall(EtsEnv *env, ets_object obj, ets_method method_id, Args args)
{
    EtsMethod *method = nullptr;
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    if constexpr (IS_VIRTUAL) {
        method = ResolveVirtualMethod(&s, obj, method_id);
    } else {
        method = ToInternalType(method_id);
    }
    ASSERT(method != nullptr);

    ArgVector<Value> values = std::move(GetArgValues(&s, method, args, obj));
    EtsValue res = method->Invoke(&s, const_cast<Value *>(values.data()));

    // Now NapiType and EtsValueType are the same, but later it could be changed
    static_assert(std::is_same_v<NapiType, EtsValueType>);
    return res.GetAs<EtsValueType>();
}

static void CheckMethodReturnType(ets_method method_id, EtsType type)
{
    EtsMethod *method = ToInternalType(method_id);
    if (method->GetReturnValueType() != type &&
        (type != EtsType::VOID || method->GetReturnValueType() != EtsType::OBJECT)) {
        LOG(FATAL, ETS_NAPI) << "Return type mismatch";
    }
}

template <typename EtsNapiType, typename InternalType>
static inline EtsNapiType GetPrimitiveTypeField(EtsEnv *env, ets_object obj, ets_field field_id)
{
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(field_id);

    EtsField *internal_field_id = ToInternalType(field_id);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_object = s.ToInternalType(obj);

    return static_cast<EtsNapiType>(internal_object->GetFieldPrimitive<InternalType>(internal_field_id));
}

template <typename EtsNapiType>
static inline void SetPrimitiveTypeField(EtsEnv *env, ets_object obj, ets_field field_id, EtsNapiType value)
{
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(field_id);

    EtsField *internal_field_id = ToInternalType(field_id);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_object = s.ToInternalType(obj);

    internal_object->SetFieldPrimitive(internal_field_id, value);
}

template <typename EtsNapiType, typename InternalType>
static inline EtsNapiType GetPrimitiveTypeStaticField(EtsEnv *env, ets_field field_id)
{
    ETS_NAPI_ABORT_IF_NULL(field_id);

    EtsField *internal_field_id = ToInternalType(field_id);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = internal_field_id->GetDeclaringClass();

    return static_cast<EtsNapiType>(internal_class->GetStaticFieldPrimitive<InternalType>(internal_field_id));
}

template <typename EtsNapiType>
static inline void SetPrimitiveTypeStaticField(EtsEnv *env, ets_field field_id, EtsNapiType value)
{
    ETS_NAPI_ABORT_IF_NULL(field_id);

    EtsField *internal_field_id = ToInternalType(field_id);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = internal_field_id->GetDeclaringClass();

    internal_class->SetStaticFieldPrimitive(internal_field_id, value);
}

inline PandaString ToClassDescriptor(const char *class_name)
{
    if (class_name[0] == '[') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return class_name;
    }
    return PandaString("L") + class_name + ';';
}

ClassLinkerContext *GetClassLinkerContext(ScopedManagedCodeFix *soa)
{
    auto stack = StackWalker::Create(soa->Coroutine());
    if (!stack.HasFrame()) {
        return nullptr;
    }

    auto *method = EtsMethod::FromRuntimeMethod(stack.GetMethod());

    if (method != nullptr) {
        return method->GetClass()->GetClassLoader();
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
    EtsClassLinker *class_linker = corutine->GetPandaVM()->GetClassLinker();
    bool is_initialized = class_linker->InitializeClass(corutine, klass);

    if (!is_initialized) {
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

    auto panda_env = PandaEtsNapiEnv::ToPandaEtsEnv(env);
    ScopedManagedCodeFix s(panda_env);

    auto vm = panda_env->GetEtsVM();
    if (!vm->GetGC()->IsPinningSupported()) {
        LOG(FATAL, ETS_NAPI) << "Pinning is not supported with " << mem::GCStringFromType(vm->GetGC()->GetType());
        return nullptr;
    }

    auto core_array = s.ToInternalType(array)->GetCoreType();
    vm->GetHeapManager()->PinObject(core_array);

    return core_array->GetData();
}

template <typename T>
static T *PinPrimitiveTypeArray(EtsEnv *env, ets_array array)
{
    return reinterpret_cast<T *>(PinRawDataOfPrimitiveArray(env, array));
}

static void UnpinPrimitiveTypeArray(EtsEnv *env, ets_array array)
{
    ETS_NAPI_ABORT_IF_NULL(array);

    auto panda_env = PandaEtsNapiEnv::ToPandaEtsEnv(env);
    ScopedManagedCodeFix s(panda_env);

    auto core_array = s.ToInternalType(array)->GetCoreType();
    auto vm = panda_env->GetEtsVM();
    vm->GetHeapManager()->UnpinObject(core_array);
}

template <typename T>
static void GetPrimitiveTypeArrayRegion(EtsEnv *env, ets_array array, ets_size start, ets_size len, T *buf)
{
    ETS_NAPI_ABORT_IF_NULL(array);
    ETS_NAPI_ABORT_IF(len != 0 && buf == nullptr);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsArray *internal_array = s.ToInternalType(array);
    if (start < 0 || len < 0 || static_cast<size_t>(start) + len > internal_array->GetLength()) {
        PandaStringStream ss;
        ss << "Array index out of bounds: start = " << start << ", len = " << len
           << ", array size = " << internal_array->GetLength();
        s.ThrowNewException(EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy_s(buf, len * sizeof(T), internal_array->GetData<T>() + start, len * sizeof(T));
}

template <typename T>
static void SetPrimitiveTypeArrayRegion(EtsEnv *env, ets_array array, ets_size start, ets_size len, T *buf)
{
    ETS_NAPI_ABORT_IF_NULL(array);
    ETS_NAPI_ABORT_IF(len != 0 && buf == nullptr);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsArray *internal_array = s.ToInternalType(array);
    if (start < 0 || len < 0 || static_cast<size_t>(start) + len > internal_array->GetLength()) {
        PandaStringStream ss;
        ss << "Array index out of bounds: start = " << start << ", len = " << len
           << ", array size = " << internal_array->GetLength();
        s.ThrowNewException(EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy_s(internal_array->GetData<std::remove_const_t<T>>() + start, len * sizeof(T), buf, len * sizeof(T));
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

    auto ets_env = PandaEtsNapiEnv::ToPandaEtsEnv(env);
    ScopedManagedCodeFix s(ets_env);

    if (name == nullptr) {
        s.ThrowNewException(EtsNapiException::NO_CLASS_DEF_FOUND, "Null pointer passed as a class name");
        return nullptr;
    }

    PandaString class_descriptor = ToClassDescriptor(name);
    EtsClassLinker *class_linker = ets_env->GetEtsVM()->GetClassLinker();
    EtsClass *klass = class_linker->GetClass(class_descriptor.c_str(), true, GetClassLinkerContext(&s));

    if (ets_env->HasPendingException()) {
        EtsThrowable *current_exception = ets_env->GetThrowable();
        std::string_view exception_string = current_exception->GetClass()->GetDescriptor();
        if (exception_string == panda_file_items::class_descriptors::CLASS_NOT_FOUND_EXCEPTION) {
            ets_env->ClearException();

            PandaStringStream ss;
            ss << "Class '" << name << "' is not found";

            s.ThrowNewException(EtsNapiException::NO_CLASS_DEF_FOUND, ss.str().c_str());

            return nullptr;
        }
    }
    ETS_NAPI_RETURN_NULL_IF_NULL(klass);
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
    auto base_cls = s.ToInternalType(cls)->GetBase();
    ETS_NAPI_RETURN_NULL_IF_NULL(base_cls);
    ASSERT_MANAGED_CODE();
    return reinterpret_cast<ets_class>(s.AddLocalRef(reinterpret_cast<EtsObject *>(base_cls)));
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

    PandaEtsNapiEnv *panda_ets_env = PandaEtsNapiEnv::ToPandaEtsEnv(env);

    ScopedManagedCodeFix s(panda_ets_env);
    EtsClass *exception_class = s.ToInternalType(cls);
    EtsCoroutine *coroutine = panda_ets_env->GetEtsCoroutine();
    ThrowEtsException(coroutine, exception_class->GetDescriptor(), message);
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

    auto error_klass = env->GetObjectClass(error);
    auto to_string_method = env->Getp_method(error_klass, "toString", ":Lstd/core/String;");
    auto error_string = env->CallObjectMethod(error, to_string_method);

    auto console_klass = env->FindClass("std/core/Console");

    auto core_global_class = env->FindClass("std/core/ETSGLOBAL");
    ETS_NAPI_ABORT_IF_NULL(core_global_class);
    auto console_field = env->GetStaticp_field(core_global_class, "console", "Lstd/core/Console;");
    ETS_NAPI_ABORT_IF_NULL(console_field);
    auto console_obj = env->GetStaticObjectField(core_global_class, console_field);

    auto println_method = env->Getp_method(console_klass, "println", "Lstd/core/String;:Lstd/core/void;");
    env->CallVoidMethod(console_obj, println_method, error_string);
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
    EtsReference *result_ets_ref = EtsObjectToEtsRef(result);
    EtsReference *ets_ref =
        PandaEtsNapiEnv::ToPandaEtsEnv(env)->GetEtsReferenceStorage()->PopLocalEtsFrame(result_ets_ref);
    return EtsRefToEtsObject(ets_ref);
}

NO_UB_SANITIZE static ets_object NewGlobalRef(EtsEnv *env, ets_object obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_NULL_IF_NULL(obj);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_object = s.ToInternalType(obj);
    return s.AddGlobalRef(internal_object);
}

NO_UB_SANITIZE static void DeleteGlobalRef([[maybe_unused]] EtsEnv *env, ets_object global_ref)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_IF_NULL(global_ref);

    PandaEtsVM *ets_vm = PandaEtsVM::GetCurrent();
    ets_vm->DeleteGlobalRef(global_ref);
}

NO_UB_SANITIZE static void DeleteLocalRef(EtsEnv *env, ets_object local_ref)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_IF_NULL(local_ref);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    s.DelLocalRef(local_ref);
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
    ETS_NAPI_RETURN_NULL_IF_NULL(ref);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_object = s.ToInternalType(ref);
    return s.AddLocalRef(internal_object);
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

    EtsMethod *method = (sig == nullptr ? klass->GetMethod(name) : klass->GetMethod(name, sig));
    if (method == nullptr || method->IsStatic()) {
        PandaStringStream ss;
        ss << "Method " << klass->GetRuntimeClass()->GetName() << "::" << name
           << " sig = " << (sig == nullptr ? "nullptr" : sig) << " is not found";
        s.ThrowNewException(EtsNapiException::NO_SUCH_METHOD, ss.str().c_str());
        return nullptr;
    }

    return ToEtsNapiType(method);
}

NO_UB_SANITIZE static ets_object CallObjectMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::OBJECT);
    return GeneralMethodCall<true, ets_object, ets_object>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_object CallObjectMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_object res = CallObjectMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_object CallObjectMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                       const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::OBJECT);
    return GeneralMethodCall<true, ets_object, ets_object>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_boolean CallBooleanMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BOOLEAN);
    return GeneralMethodCall<true, ets_boolean, EtsBoolean>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_boolean CallBooleanMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_boolean res = CallBooleanMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_boolean CallBooleanMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                         const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BOOLEAN);
    return GeneralMethodCall<true, ets_boolean, EtsBoolean>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_byte CallByteMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BYTE);
    return GeneralMethodCall<true, ets_byte, EtsByte>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_byte CallByteMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_byte res = CallByteMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_byte CallByteMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                   const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BYTE);
    return GeneralMethodCall<true, ets_byte, EtsByte>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_char CallCharMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::CHAR);
    return GeneralMethodCall<true, ets_char, EtsChar>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_char CallCharMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_char res = CallCharMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_char CallCharMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                   const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::CHAR);
    return GeneralMethodCall<true, ets_char, EtsChar>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_short CallShortMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::SHORT);
    return GeneralMethodCall<true, ets_short, EtsShort>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_short CallShortMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_short res = CallShortMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_short CallShortMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                     const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::SHORT);
    return GeneralMethodCall<true, ets_short, EtsShort>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_int CallIntMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::INT);
    return GeneralMethodCall<true, ets_int, EtsInt>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_int CallIntMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_int res = CallIntMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_int CallIntMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                 const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::INT);
    return GeneralMethodCall<true, ets_int, EtsInt>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_long CallLongMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::LONG);
    return GeneralMethodCall<true, ets_long, EtsLong>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_long CallLongMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_long res = CallLongMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_long CallLongMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                   const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::LONG);
    return GeneralMethodCall<true, ets_long, EtsLong>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_float CallFloatMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::FLOAT);
    return GeneralMethodCall<true, ets_float, EtsFloat>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_float CallFloatMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_float res = CallFloatMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_float CallFloatMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                     const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::FLOAT);
    return GeneralMethodCall<true, ets_float, EtsFloat>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_double CallDoubleMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::DOUBLE);
    return GeneralMethodCall<true, ets_double, EtsDouble>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_double CallDoubleMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_double res = CallDoubleMethodList(env, obj, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_double CallDoubleMethodArray(EtsEnv *env, ets_object obj, ets_method method_id,
                                                       const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::DOUBLE);
    return GeneralMethodCall<true, ets_double, EtsDouble>(env, obj, method_id, args);
}

NO_UB_SANITIZE static void CallVoidMethodList(EtsEnv *env, ets_object obj, ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::VOID);
    // Use any primitive type as template parameter and jist ignore the result
    GeneralMethodCall<true, ets_boolean, EtsBoolean>(env, obj, method_id, args);
}

NO_UB_SANITIZE static void CallVoidMethod(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    CallVoidMethodList(env, obj, method_id, args);
    va_end(args);
}

NO_UB_SANITIZE static void CallVoidMethodArray(EtsEnv *env, ets_object obj, ets_method method_id, const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::VOID);
    // Use any primitive type as template parameter and jist ignore the result
    GeneralMethodCall<true, ets_boolean, EtsBoolean>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_object CallNonvirtualObjectMethodList(EtsEnv *env, ets_object obj,
                                                                [[maybe_unused]] ets_class cls, ets_method method_id,
                                                                va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::OBJECT);
    return GeneralMethodCall<false, ets_object, ets_object>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_object CallNonvirtualObjectMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                            ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_object res = CallNonvirtualObjectMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_object CallNonvirtualObjectMethodArray(EtsEnv *env, ets_object obj,
                                                                 [[maybe_unused]] ets_class cls, ets_method method_id,
                                                                 const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::OBJECT);
    return GeneralMethodCall<false, ets_object, ets_object>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_boolean CallNonvirtualBooleanMethodList(EtsEnv *env, ets_object obj,
                                                                  [[maybe_unused]] ets_class cls, ets_method method_id,
                                                                  va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BOOLEAN);
    return GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_boolean CallNonvirtualBooleanMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                              ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_boolean res = CallNonvirtualBooleanMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_boolean CallNonvirtualBooleanMethodArray(EtsEnv *env, ets_object obj,
                                                                   [[maybe_unused]] ets_class cls, ets_method method_id,
                                                                   const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BOOLEAN);
    return GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_byte CallNonvirtualByteMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                            ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BYTE);
    return GeneralMethodCall<false, ets_byte, EtsByte>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_byte CallNonvirtualByteMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                        ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_byte res = CallNonvirtualByteMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_byte CallNonvirtualByteMethodArray(EtsEnv *env, ets_object obj,
                                                             [[maybe_unused]] ets_class cls, ets_method method_id,
                                                             const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BYTE);
    return GeneralMethodCall<false, ets_byte, EtsByte>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_char CallNonvirtualCharMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                            ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::CHAR);
    return GeneralMethodCall<false, ets_char, EtsChar>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_char CallNonvirtualCharMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                        ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_char res = CallNonvirtualCharMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_char CallNonvirtualCharMethodArray(EtsEnv *env, ets_object obj,
                                                             [[maybe_unused]] ets_class cls, ets_method method_id,
                                                             const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::CHAR);
    return GeneralMethodCall<false, ets_char, EtsChar>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_short CallNonvirtualShortMethodList(EtsEnv *env, ets_object obj,
                                                              [[maybe_unused]] ets_class cls, ets_method method_id,
                                                              va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::SHORT);
    return GeneralMethodCall<false, ets_short, EtsShort>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_short CallNonvirtualShortMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                          ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_short res = CallNonvirtualShortMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_short CallNonvirtualShortMethodArray(EtsEnv *env, ets_object obj,
                                                               [[maybe_unused]] ets_class cls, ets_method method_id,
                                                               const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::SHORT);
    return GeneralMethodCall<false, ets_short, EtsShort>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_int CallNonvirtualIntMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                          ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::INT);
    return GeneralMethodCall<false, ets_int, EtsInt>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_int CallNonvirtualIntMethod(EtsEnv *env, ets_object obj, ets_class cls, ets_method method_id,
                                                      ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_int res = CallNonvirtualIntMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_int CallNonvirtualIntMethodArray(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                           ets_method method_id, const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::INT);
    return GeneralMethodCall<false, ets_int, EtsInt>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_long CallNonvirtualLongMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                            ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::LONG);
    return GeneralMethodCall<false, ets_long, EtsLong>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_long CallNonvirtualLongMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                        ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_long res = CallNonvirtualLongMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_long CallNonvirtualLongMethodArray(EtsEnv *env, ets_object obj,
                                                             [[maybe_unused]] ets_class cls, ets_method method_id,
                                                             const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::LONG);
    return GeneralMethodCall<false, ets_long, EtsLong>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_float CallNonvirtualFloatMethodList(EtsEnv *env, ets_object obj,
                                                              [[maybe_unused]] ets_class cls, ets_method method_id,
                                                              va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::FLOAT);
    return GeneralMethodCall<false, ets_float, EtsFloat>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_float CallNonvirtualFloatMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                          ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_float res = CallNonvirtualFloatMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_float CallNonvirtualFloatMethodArray(EtsEnv *env, ets_object obj,
                                                               [[maybe_unused]] ets_class cls, ets_method method_id,
                                                               const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::FLOAT);
    return GeneralMethodCall<false, ets_float, EtsFloat>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_double CallNonvirtualDoubleMethodList(EtsEnv *env, ets_object obj,
                                                                [[maybe_unused]] ets_class cls, ets_method method_id,
                                                                va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::DOUBLE);
    return GeneralMethodCall<false, ets_double, EtsDouble>(env, obj, method_id, args);
}

NO_UB_SANITIZE static ets_double CallNonvirtualDoubleMethod(EtsEnv *env, ets_object obj, ets_class cls,
                                                            ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_double res = CallNonvirtualDoubleMethodList(env, obj, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_double CallNonvirtualDoubleMethodArray(EtsEnv *env, ets_object obj,
                                                                 [[maybe_unused]] ets_class cls, ets_method method_id,
                                                                 const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::DOUBLE);
    return GeneralMethodCall<false, ets_double, EtsDouble>(env, obj, method_id, args);
}

NO_UB_SANITIZE static void CallNonvirtualVoidMethodList(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                        ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::VOID);
    // Use any primitive type as template parameter and jist ignore the result
    GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, obj, method_id, args);
}

NO_UB_SANITIZE static void CallNonvirtualVoidMethod(EtsEnv *env, ets_object obj, ets_class cls, ets_method method_id,
                                                    ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    CallNonvirtualVoidMethodList(env, obj, cls, method_id, args);
    va_end(args);
}

NO_UB_SANITIZE static void CallNonvirtualVoidMethodArray(EtsEnv *env, ets_object obj, [[maybe_unused]] ets_class cls,
                                                         ets_method method_id, const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::VOID);
    // Use any primitive type as template parameter and jist ignore the result
    GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, obj, method_id, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ets_field Getp_field(EtsEnv *env, ets_class cls, const char *name, const char *sig)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(name);
    ETS_NAPI_ABORT_IF_NULL(sig);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = GetInternalClass(env, cls, &s);
    if (internal_class == nullptr) {
        return nullptr;
    }

    EtsField *internal_field_id = internal_class->GetFieldIDByName(name, sig);
    if (internal_field_id == nullptr) {
        PandaStringStream ss;
        ss << "Field " << internal_class->GetRuntimeClass()->GetName() << "::" << name << " sig = " << sig
           << " is not found";
        s.ThrowNewException(EtsNapiException::NO_SUCH_FIELD, ss.str().c_str());
        return nullptr;
    }

    return ToEtsNapiType(internal_field_id);
}

NO_UB_SANITIZE static ets_object GetObjectField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(p_field);

    EtsField *internal_field_id = ToInternalType(p_field);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_object = s.ToInternalType(obj);
    EtsObject *ret_obj = internal_object->GetFieldObject(internal_field_id);
    if (ret_obj == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<ets_object>(s.AddLocalRef(ret_obj));
}

NO_UB_SANITIZE static ets_boolean GetBooleanField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_boolean, EtsBoolean>(env, obj, p_field);
}

NO_UB_SANITIZE static ets_byte GetByteField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_byte, EtsByte>(env, obj, p_field);
}

NO_UB_SANITIZE static ets_char GetCharField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_char, EtsChar>(env, obj, p_field);
}

NO_UB_SANITIZE static ets_short GetShortField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_short, EtsShort>(env, obj, p_field);
}

NO_UB_SANITIZE static ets_int GetIntField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_int, EtsInt>(env, obj, p_field);
}

NO_UB_SANITIZE static ets_long GetLongField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_long, EtsLong>(env, obj, p_field);
}

NO_UB_SANITIZE static ets_float GetFloatField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_float, EtsFloat>(env, obj, p_field);
}

NO_UB_SANITIZE static ets_double GetDoubleField(EtsEnv *env, ets_object obj, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeField<ets_double, EtsDouble>(env, obj, p_field);
}

NO_UB_SANITIZE static void SetObjectField(EtsEnv *env, ets_object obj, ets_field p_field, ets_object value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ETS_NAPI_ABORT_IF_NULL(p_field);

    EtsField *internal_field_id = ToInternalType(p_field);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_value = s.ToInternalType(value);

    s.ToInternalType(obj)->SetFieldObject(internal_field_id, internal_value);
}

NO_UB_SANITIZE static void SetBooleanField(EtsEnv *env, ets_object obj, ets_field p_field, ets_boolean value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, p_field, value);
}

NO_UB_SANITIZE static void SetByteField(EtsEnv *env, ets_object obj, ets_field p_field, ets_byte value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, p_field, value);
}

NO_UB_SANITIZE static void SetCharField(EtsEnv *env, ets_object obj, ets_field p_field, ets_char value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, p_field, value);
}

NO_UB_SANITIZE static void SetShortField(EtsEnv *env, ets_object obj, ets_field p_field, ets_short value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, p_field, value);
}

NO_UB_SANITIZE static void SetIntField(EtsEnv *env, ets_object obj, ets_field p_field, ets_int value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, p_field, value);
}

NO_UB_SANITIZE static void SetLongField(EtsEnv *env, ets_object obj, ets_field p_field, ets_long value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, p_field, value);
}

NO_UB_SANITIZE static void SetFloatField(EtsEnv *env, ets_object obj, ets_field p_field, ets_float value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, p_field, value);
}

NO_UB_SANITIZE static void SetDoubleField(EtsEnv *env, ets_object obj, ets_field p_field, ets_double value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeField(env, obj, p_field, value);
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

    EtsMethod *method = (sig == nullptr ? klass->GetMethod(name) : klass->GetMethod(name, sig));
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
                                                            ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::OBJECT);
    return GeneralMethodCall<false, ets_object, ets_object>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_object CallStaticObjectMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_object res = CallStaticObjectMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_object CallStaticObjectMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                             ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::OBJECT);
    return GeneralMethodCall<false, ets_object, ets_object>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_boolean CallStaticBooleanMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                              ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BOOLEAN);
    return GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_boolean CallStaticBooleanMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_boolean res = CallStaticBooleanMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_boolean CallStaticBooleanMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                               ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BOOLEAN);
    return GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_byte CallStaticByteMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BYTE);
    return GeneralMethodCall<false, ets_byte, EtsByte>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_byte CallStaticByteMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_byte res = CallStaticByteMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_byte CallStaticByteMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::BYTE);
    return GeneralMethodCall<false, ets_byte, EtsByte>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_char CallStaticCharMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::CHAR);
    return GeneralMethodCall<false, ets_char, EtsChar>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_char CallStaticCharMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_char res = CallStaticCharMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_char CallStaticCharMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::CHAR);
    return GeneralMethodCall<false, ets_char, EtsChar>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_short CallStaticShortMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                          ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::SHORT);
    return GeneralMethodCall<false, ets_short, EtsShort>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_short CallStaticShortMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_short res = CallStaticShortMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_short CallStaticShortMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                           ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::SHORT);
    return GeneralMethodCall<false, ets_short, EtsShort>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_int CallStaticIntMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls, ets_method method_id,
                                                      va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::INT);
    return GeneralMethodCall<false, ets_int, EtsInt>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_int CallStaticIntMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_int res = CallStaticIntMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_int CallStaticIntMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::INT);
    return GeneralMethodCall<false, ets_int, EtsInt>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_long CallStaticLongMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::LONG);
    return GeneralMethodCall<false, ets_long, EtsLong>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_long CallStaticLongMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_long res = CallStaticLongMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_long CallStaticLongMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::LONG);
    return GeneralMethodCall<false, ets_long, EtsLong>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_float CallStaticFloatMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                          ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::FLOAT);
    return GeneralMethodCall<false, ets_float, EtsFloat>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_float CallStaticFloatMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_float res = CallStaticFloatMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_float CallStaticFloatMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                           ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::FLOAT);
    return GeneralMethodCall<false, ets_float, EtsFloat>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_double CallStaticDoubleMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                            ets_method method_id, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::DOUBLE);
    return GeneralMethodCall<false, ets_double, EtsDouble>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static ets_double CallStaticDoubleMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    ets_double res = CallStaticDoubleMethodList(env, cls, method_id, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_double CallStaticDoubleMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                             ets_method method_id, ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::DOUBLE);
    return GeneralMethodCall<false, ets_double, EtsDouble>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static void CallStaticVoidMethodList(EtsEnv *env, [[maybe_unused]] ets_class cls, ets_method method_id,
                                                    va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::VOID);
    // Use any primitive type as template parameter and just ignore the result
    GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, nullptr, method_id, args);
}

NO_UB_SANITIZE static void CallStaticVoidMethod(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    va_list args;
    va_start(args, method_id);
    CallStaticVoidMethodList(env, cls, method_id, args);
    va_end(args);
}

NO_UB_SANITIZE static void CallStaticVoidMethodArray(EtsEnv *env, [[maybe_unused]] ets_class cls, ets_method method_id,
                                                     ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(method_id);
    CheckMethodReturnType(method_id, EtsType::VOID);
    // Use any primitive type as template parameter and just ignore the result
    GeneralMethodCall<false, ets_boolean, EtsBoolean>(env, nullptr, method_id, args);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ets_field GetStaticp_field(EtsEnv *env, ets_class cls, const char *name, const char *sig)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(name);
    ETS_NAPI_ABORT_IF_NULL(sig);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = GetInternalClass(env, cls, &s);
    if (internal_class == nullptr) {
        return nullptr;
    }

    EtsField *internal_field_id = internal_class->GetStaticFieldIDByName(name, sig);
    if (internal_field_id == nullptr) {
        PandaStringStream ss;
        ss << "Static field " << internal_class->GetRuntimeClass()->GetName() << "::" << name << " sig = " << sig
           << " is not found";
        s.ThrowNewException(EtsNapiException::NO_SUCH_FIELD, ss.str().c_str());
        return nullptr;
    }

    return ToEtsNapiType(internal_field_id);
}

NO_UB_SANITIZE static ets_object GetStaticObjectField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls,
                                                      ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(p_field);

    EtsField *internal_field_id = ToInternalType(p_field);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = internal_field_id->GetDeclaringClass();
    EtsObject *ret_obj = internal_class->GetStaticFieldObject(internal_field_id);
    if (ret_obj == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<ets_object>(s.AddLocalRef(ret_obj));
}

NO_UB_SANITIZE static ets_boolean GetStaticBooleanField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls,
                                                        ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_boolean, EtsBoolean>(env, p_field);
}

NO_UB_SANITIZE static ets_byte GetStaticByteField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_byte, EtsByte>(env, p_field);
}

NO_UB_SANITIZE static ets_char GetStaticCharField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_char, EtsChar>(env, p_field);
}

NO_UB_SANITIZE static ets_short GetStaticShortField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls,
                                                    ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_short, EtsShort>(env, p_field);
}

NO_UB_SANITIZE static ets_int GetStaticIntField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_int, EtsInt>(env, p_field);
}

NO_UB_SANITIZE static ets_long GetStaticLongField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_long, EtsLong>(env, p_field);
}

NO_UB_SANITIZE static ets_float GetStaticFloatField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls,
                                                    ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_float, EtsFloat>(env, p_field);
}

NO_UB_SANITIZE static ets_double GetStaticDoubleField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls,
                                                      ets_field p_field)
{
    ETS_NAPI_DEBUG_TRACE(env);
    return GetPrimitiveTypeStaticField<ets_double, EtsDouble>(env, p_field);
}

NO_UB_SANITIZE static void SetStaticObjectField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                                ets_object value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(p_field);

    EtsField *internal_field_id = ToInternalType(p_field);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_value = s.ToInternalType(value);

    internal_field_id->GetDeclaringClass()->SetStaticFieldObject(internal_field_id, internal_value);
}

NO_UB_SANITIZE static void SetStaticBooleanField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                                 ets_boolean value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, p_field, value);
}

NO_UB_SANITIZE static void SetStaticByteField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                              ets_byte value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, p_field, value);
}

NO_UB_SANITIZE static void SetStaticCharField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                              ets_char value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, p_field, value);
}

NO_UB_SANITIZE static void SetStaticShortField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                               ets_short value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, p_field, value);
}

NO_UB_SANITIZE static void SetStaticIntField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                             ets_int value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, p_field, value);
}

NO_UB_SANITIZE static void SetStaticLongField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                              ets_long value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, p_field, value);
}

NO_UB_SANITIZE static void SetStaticFloatField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                               ets_float value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, p_field, value);
}

NO_UB_SANITIZE static void SetStaticDoubleField(EtsEnv *env, [[maybe_unused]] ets_class unused_cls, ets_field p_field,
                                                ets_double value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    SetPrimitiveTypeStaticField(env, p_field, value);
}

NO_UB_SANITIZE static ets_string NewString(EtsEnv *env, const ets_char *unicode_chars, ets_size len)
{
    ETS_NAPI_DEBUG_TRACE(env);
    if (unicode_chars == nullptr) {
        ETS_NAPI_ABORT_IF_NE(len, 0);
    }
    ETS_NAPI_ABORT_IF_LZ(len);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internal_string = EtsString::CreateFromUtf16(unicode_chars, len);
    if (internal_string == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<ets_string>(s.AddLocalRef(reinterpret_cast<EtsObject *>(internal_string)));
}

NO_UB_SANITIZE static ets_size GetStringLength(EtsEnv *env, ets_string string)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internal_string = s.ToInternalType(string);
    return static_cast<ets_size>(internal_string->GetLength());
}

NO_UB_SANITIZE static const ets_char *GetStringChars(EtsEnv *env, ets_string string, ets_boolean *is_copy)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    if (is_copy != nullptr) {
        *is_copy = ETS_TRUE;
    }
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internal_string = s.ToInternalType(string);
    size_t len = internal_string->GetUtf16Length();
    void *buf = EtsAlloc(len * sizeof(uint16_t));
    if (buf == nullptr) {
        LOG(ERROR, ETS_NAPI) << __func__ << ": cannot copy string";
        return nullptr;
    }
    internal_string->CopyDataUtf16(buf, len);
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
    auto internal_string = EtsString::CreateFromMUtf8(bytes);
    if (internal_string == nullptr) {
        s.ThrowNewException(EtsNapiException::OUT_OF_MEMORY, "Could not allocate memory for string");
        return nullptr;
    }
    return reinterpret_cast<ets_string>(s.AddLocalRef(reinterpret_cast<EtsObject *>(internal_string)));
}

NO_UB_SANITIZE static ets_size GetStringUTFLength(EtsEnv *env, ets_string string)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internal_string = s.ToInternalType(string);
    // NOTE(m.morozov): ensure that place for \0 is included
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    return internal_string->GetMUtf8Length() - 1;
}

NO_UB_SANITIZE static const char *GetStringUTFChars(EtsEnv *env, ets_string string, ets_boolean *is_copy)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(string);
    if (is_copy != nullptr) {
        *is_copy = ETS_TRUE;
    }
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto internal_string = s.ToInternalType(string);
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    size_t len = internal_string->GetMUtf8Length();
    void *buf = EtsAlloc(len);
    if (buf == nullptr) {
        LOG(ERROR, ETS_NAPI) << __func__ << ": cannot copy string";
        return nullptr;
    }
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    internal_string->CopyDataMUtf8(buf, len, true);
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
    EtsArray *internal_array = s.ToInternalType(array);
    return static_cast<ets_size>(internal_array->GetLength());
}

// NOTE(kropacheva): change name after conflicts resolved
NO_UB_SANITIZE static ets_objectArray NewObjectsArray(EtsEnv *env, ets_size length, ets_class element_class,
                                                      ets_object initial_element)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_LZ(length);
    ETS_NAPI_ABORT_IF_NULL(element_class);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = s.ToInternalType(element_class);
    EtsObjectArray *array = EtsObjectArray::Create(internal_class, static_cast<uint32_t>(length));
    if (array == nullptr) {
        PandaStringStream ss;
        ss << "Could not allocate array of " << internal_class->GetRuntimeClass()->GetName() << " of " << length
           << " elements";
        s.ThrowNewException(EtsNapiException::OUT_OF_MEMORY, ss.str().c_str());
        return nullptr;
    }
    if (initial_element != nullptr) {
        for (decltype(length) i = 0; i < length; ++i) {
            array->Set(static_cast<uint32_t>(i), s.ToInternalType(initial_element));
        }
    }
    return reinterpret_cast<ets_objectArray>(s.AddLocalRef(reinterpret_cast<EtsObject *>(array)));
}

NO_UB_SANITIZE static ets_object GetObjectArrayElement(EtsEnv *env, ets_objectArray array, ets_size index)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(array);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObjectArray *internal_array = s.ToInternalType(array);
    if (index < 0 || index >= static_cast<ets_size>(internal_array->GetLength())) {
        PandaStringStream ss;
        ss << "Could not access " << index << " element, array length = " << internal_array->GetLength();
        s.ThrowNewException(EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return nullptr;
    }

    return s.AddLocalRef(internal_array->Get(static_cast<uint32_t>(index)));
}

NO_UB_SANITIZE static void SetObjectArrayElement(EtsEnv *env, ets_objectArray array, ets_size index, ets_object value)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(array);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObjectArray *internal_array = s.ToInternalType(array);
    if (index < 0 || index >= static_cast<ets_size>(internal_array->GetLength())) {
        PandaStringStream ss;
        ss << "Could not access " << index << " element, array length = " << internal_array->GetLength();
        s.ThrowNewException(EtsNapiException::ARRAY_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    EtsObject *internal_value = s.ToInternalType(value);
    if (internal_value != nullptr) {
        auto *component_class = internal_array->GetClass()->GetComponentType();
        if (!internal_value->IsInstanceOf(component_class)) {
            PandaStringStream ss;
            ss << internal_value->GetClass()->GetRuntimeClass()->GetName();
            ss << "cannot be stored in an array of type ";
            ss << component_class->GetRuntimeClass()->GetName();
            s.ThrowNewException(EtsNapiException::ARRAY_STORE, ss.str().c_str());
            return;
        }
    }
    internal_array->Set(static_cast<uint32_t>(index), internal_value);
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

NO_UB_SANITIZE static ets_int RegisterNatives(EtsEnv *env, ets_class cls, const EtsNativeMethod *methods,
                                              ets_int n_methods)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_LZ(n_methods);
    if (n_methods == 0) {
        return ETS_OK;
    }
    ETS_NAPI_ABORT_IF_NULL(methods);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *klass = s.ToInternalType(cls);
    for (ets_int i = 0; i < n_methods; ++i) {
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        EtsMethod *method = (methods[i].signature == nullptr ? klass->GetMethod(methods[i].name)
                                                             : klass->GetMethod(methods[i].name, methods[i].signature));
        if (method == nullptr || !method->IsNative()) {
            PandaStringStream ss;
            ss << "Method " << klass->GetRuntimeClass()->GetName() << "::" << methods[i].name
               << " sig = " << (methods[i].signature == nullptr ? "nullptr" : methods[i].signature)
               << " not found or not native";
            s.ThrowNewException(EtsNapiException::NO_SUCH_METHOD, ss.str().c_str());
            return ETS_ERR_INVAL;
        }
        method->RegisterNativeImpl(methods[i].func);
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
            method->UnregisterNativeImpl();
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
    EtsString *internal_string = s.ToInternalType(str);
    if (start < 0 || len < 0 || static_cast<size_t>(start) + len > internal_string->GetUtf16Length()) {
        PandaStringStream ss;
        ss << "String index out of bounds: start = " << start << ", len = " << len
           << ", string size = " << internal_string->GetUtf16Length();
        s.ThrowNewException(EtsNapiException::STRING_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    internal_string->CopyDataRegionUtf16(buf, start, len, len);
}

NO_UB_SANITIZE static void GetStringUTFRegion(EtsEnv *env, ets_string str, ets_size start, ets_size len, char *buf)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(str);
    ETS_NAPI_ABORT_IF_NULL(buf);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsString *internal_string = s.ToInternalType(str);
    if (start < 0 || len < 0 || static_cast<size_t>(start) + len > internal_string->GetUtf16Length()) {
        PandaStringStream ss;
        ss << "String index out of bounds: start = " << start << ", len = " << len
           << ", string size = " << internal_string->GetUtf16Length();
        s.ThrowNewException(EtsNapiException::STRING_INDEX_OUT_OF_BOUNDS, ss.str().c_str());
        return;
    }
    // NOTE(m.morozov): check after mutf8 vs utf8 decision
    internal_string->CopyDataRegionMUtf8(buf, start, len, internal_string->GetMUtf8Length());
}

NO_UB_SANITIZE static ets_object AllocObject(EtsEnv *env, ets_class cls)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = GetInternalClass(env, cls, &s);
    if (internal_class == nullptr) {
        return nullptr;
    }

    if (internal_class->IsAbstract() || internal_class->IsInterface()) {
        PandaStringStream ss;
        ss << "Class " << internal_class->GetRuntimeClass()->GetName() << " is interface or abstract";
        s.ThrowNewException(EtsNapiException::INSTANTIATION, ss.str().c_str());
        return nullptr;
    }
    if (internal_class->GetRuntimeClass()->IsStringClass()) {
        EtsString *str = EtsString::CreateNewEmptyString();
        if (UNLIKELY(str == nullptr)) {
            return nullptr;
        }
        return s.AddLocalRef(reinterpret_cast<EtsObject *>(str));
    }

    EtsObject *obj = EtsObject::Create(internal_class);
    if (UNLIKELY(obj == nullptr)) {
        return nullptr;
    }
    return s.AddLocalRef(reinterpret_cast<EtsObject *>(obj));
}

NO_UB_SANITIZE static ets_object NewObjectList(EtsEnv *env, ets_class cls, ets_method p_method, va_list args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(p_method);

    ets_object new_object = AllocObject(env, cls);
    if (new_object == nullptr) {
        return nullptr;
    }
    CallNonvirtualVoidMethodList(env, new_object, cls, p_method, args);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    if (PandaEtsNapiEnv::ToPandaEtsEnv(env)->HasPendingException()) {
        return nullptr;
    }
    return new_object;
}

NO_UB_SANITIZE static ets_object NewObject(EtsEnv *env, ets_class cls, ets_method p_method, ...)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(p_method);

    va_list args;
    va_start(args, p_method);
    ets_object res = NewObjectList(env, cls, p_method, args);
    va_end(args);
    return res;
}

NO_UB_SANITIZE static ets_object NewObjectArray(EtsEnv *env, ets_class cls, ets_method p_method, const ets_value *args)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    ETS_NAPI_ABORT_IF_NULL(p_method);

    ets_object new_object = AllocObject(env, cls);
    if (new_object == nullptr) {
        return nullptr;
    }
    CallNonvirtualVoidMethodArray(env, new_object, cls, p_method, args);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    if (PandaEtsNapiEnv::ToPandaEtsEnv(env)->HasPendingException()) {
        return nullptr;
    }
    return new_object;
}

NO_UB_SANITIZE static ets_class GetObjectClass(EtsEnv *env, ets_object obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(obj);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_object = s.ToInternalType(obj);

    return reinterpret_cast<ets_class>(s.AddLocalRef(reinterpret_cast<EtsObject *>(internal_object->GetClass())));
}

NO_UB_SANITIZE static ets_boolean IsInstanceOf(EtsEnv *env, ets_object obj, ets_class cls)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_ABORT_IF_NULL(cls);
    if (obj == nullptr) {
        return ETS_TRUE;
    }

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsClass *internal_class = s.ToInternalType(cls);
    EtsObject *internal_object = s.ToInternalType(obj);

    return internal_object->IsInstanceOf(internal_class) ? ETS_TRUE : ETS_FALSE;
}

NO_UB_SANITIZE static ets_objectRefType GetObjectRefType(EtsEnv *env, ets_object obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    if (UNLIKELY(obj == nullptr || !s.IsValidRef(obj))) {
        return EtsInvalidRefType;
    }

    switch (mem::ReferenceStorage::GetObjectType(reinterpret_cast<panda::mem::Reference *>(obj))) {
        case panda::mem::Reference::ObjectType::GLOBAL:
            return EtsGlobalRefType;
        case panda::mem::Reference::ObjectType::WEAK:
            return EtsWeakGlobalRefType;
        case panda::mem::Reference::ObjectType::LOCAL:
        case panda::mem::Reference::ObjectType::STACK:
            return EtsLocalRefType;
        default:
            UNREACHABLE();
    }
    UNREACHABLE();
}

NO_UB_SANITIZE static ets_weak NewWeakGlobalRef(EtsEnv *env, ets_object obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_NULL_IF_NULL(obj);

    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    EtsObject *internal_object = s.ToInternalType(obj);
    ets_weak ret = s.AddWeakGlobalRef(internal_object);
    if (ret == nullptr) {
        s.ThrowNewException(EtsNapiException::OUT_OF_MEMORY, "Could not allocate object");
        return nullptr;
    }
    return ret;
}

NO_UB_SANITIZE static void DeleteWeakGlobalRef([[maybe_unused]] EtsEnv *env, ets_weak obj)
{
    ETS_NAPI_DEBUG_TRACE(env);
    ETS_NAPI_RETURN_IF_NULL(obj);

    PandaEtsVM *ets_vm = PandaEtsVM::GetCurrent();
    ets_vm->DeleteWeakGlobalRef(obj);
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
};
// clang-format on

const ETS_NativeInterface *GetNativeInterface()
{
    return &NATIVE_INTERFACE;
}
}  // namespace panda::ets::napi

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
