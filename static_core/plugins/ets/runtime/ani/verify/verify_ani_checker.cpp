/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ani/verify/verify_ani_checker.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <string>

#include "libarkbase/utils/span.h"
#include "libarkfile/file_items.h"
#include "libarkfile/helpers.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ani/ani_converters.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ani/verify/types/internal_ref.h"
#include "plugins/ets/runtime/ani/verify/types/venv.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "plugins/ets/runtime/ani/verify/types/vwref.h"
#include "plugins/ets/runtime/ani/verify/types/vvm.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

namespace ark::ets::ani::verify {

class ANIRefTypeChecker {
public:
    static EtsClass *GetTypeClass(ScopedManagedCodeFix &s, ani_ref ref)
    {
        ASSERT(ref != nullptr);
        return EtsClass::FromEtsClassObject(s.ToInternalType(ref));
    }

    static bool IsClass(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        auto klass = s.ToInternalType(ref)->GetClass();
        return klass->IsClassClass();
    }

    static bool IsEnum(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (!IsClass(s, ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(static_cast<ani_class>(ref));
        return klass->IsEtsEnum();
    }

    static bool IsEnumItem(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        if (s.IsNull(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        return klass->IsEtsEnum();
    }

    static bool IsError(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *errorKlass = PlatformTypes()->coreError;

        return errorKlass->IsAssignableFrom(klass);
    }

    static bool IsString(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *stringKlass = PlatformTypes()->coreString;

        return stringKlass->IsAssignableFrom(klass);
    }

    static bool IsArray(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *arrayKlass = PlatformTypes()->coreArray;

        return arrayKlass->IsAssignableFrom(klass);
    }

    static bool IsTupleValue(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (s.IsNullishValue(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *tupleKlass = PlatformTypes()->coreTuple;

        return tupleKlass->IsAssignableFrom(klass);
    }

    static bool IsArrayBuffer(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *arrayBufferKlass = PlatformTypes()->coreArrayBuffer;

        return arrayBufferKlass->IsAssignableFrom(klass);
    }

    static bool IsFixedArray(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (s.IsNullishValue(ref)) {
            return false;
        }
        return s.ToInternalType(ref)->GetClass()->IsArrayClass();
    }

    static bool IsObject(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (s.IsNullishValue(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *objectKlass = PlatformTypes()->coreObject;

        return objectKlass->IsAssignableFrom(klass);
    }
};

static ClassLinkerContext *GetClassLinkerContext(EtsExecutionContext *executionCtx)
{
    auto stack = StackWalker::Create(executionCtx->GetMT());
    if (!stack.HasFrame()) {
        return nullptr;
    }

    auto *method = EtsMethod::FromRuntimeMethod(stack.GetMethod());
    if (method != nullptr) {
        return method->GetClass()->GetLoadContext();
    }
    return nullptr;
}

static ani_size DoGetTupleLength(EtsExecutionContext *executionCtx, EtsHandle<EtsObject> internalTuple)
{
    if (UNLIKELY(PlatformTypes(executionCtx)->coreTupleN->IsAssignableFrom(internalTuple->GetClass()))) {
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

static Class *GetPrimitiveBoxClass(EtsExecutionContext *executionCtx, EtsType expectedType)
{
    switch (expectedType) {
        case EtsType::BOOLEAN:
            return EtsBoxPrimitive<ani_boolean>::GetBoxClass(executionCtx);
        case EtsType::CHAR:
            return EtsBoxPrimitive<ani_char>::GetBoxClass(executionCtx);
        case EtsType::BYTE:
            return EtsBoxPrimitive<ani_byte>::GetBoxClass(executionCtx);
        case EtsType::SHORT:
            return EtsBoxPrimitive<ani_short>::GetBoxClass(executionCtx);
        case EtsType::INT:
            return EtsBoxPrimitive<ani_int>::GetBoxClass(executionCtx);
        case EtsType::LONG:
            return EtsBoxPrimitive<ani_long>::GetBoxClass(executionCtx);
        case EtsType::FLOAT:
            return EtsBoxPrimitive<ani_float>::GetBoxClass(executionCtx);
        case EtsType::DOUBLE:
            return EtsBoxPrimitive<ani_double>::GetBoxClass(executionCtx);
        default:
            UNREACHABLE();
    }
}

static Class *GetTupleElementRuntimeClass(EtsObject *element)
{
    if (element == nullptr || element->GetClass() == nullptr) {
        return nullptr;
    }

    return element->GetClass()->GetRuntimeClass();
}

static bool IsCorrectTupleElementType(EtsExecutionContext *executionCtx, EtsObject *element, EtsType expectedType)
{
    auto *runtimeClass = GetTupleElementRuntimeClass(element);
    if (runtimeClass == nullptr) {
        return false;
    }
    if (expectedType == EtsType::OBJECT) {
        return true;
    }

    return runtimeClass == GetPrimitiveBoxClass(executionCtx, expectedType);
}

static std::string_view FixedArrayTypeToString(EtsClass *klass)
{
    ASSERT(klass != nullptr);
    ASSERT(klass->IsArrayClass());

    EtsClass *componentType = klass->GetComponentType();
    ASSERT(componentType != nullptr);

    switch (componentType->GetType().GetId()) {
        case panda_file::Type::TypeId::U1:
            return "ani_fixedarray_boolean";
        case panda_file::Type::TypeId::U16:
            return "ani_fixedarray_char";
        case panda_file::Type::TypeId::I8:
            return "ani_fixedarray_byte";
        case panda_file::Type::TypeId::I16:
            return "ani_fixedarray_short";
        case panda_file::Type::TypeId::I32:
            return "ani_fixedarray_int";
        case panda_file::Type::TypeId::I64:
            return "ani_fixedarray_long";
        case panda_file::Type::TypeId::F32:
            return "ani_fixedarray_float";
        case panda_file::Type::TypeId::F64:
            return "ani_fixedarray_double";
        case panda_file::Type::TypeId::REFERENCE:
            return "ani_fixedarray_ref";
        default:
            UNREACHABLE();
            return "";
    }
}

static std::string_view FixedArrayTypeIdToString(panda_file::Type::TypeId type)
{
    switch (type) {
        case panda_file::Type::TypeId::U1:
            return "ani_fixedarray_boolean";
        case panda_file::Type::TypeId::U16:
            return "ani_fixedarray_char";
        case panda_file::Type::TypeId::I8:
            return "ani_fixedarray_byte";
        case panda_file::Type::TypeId::I16:
            return "ani_fixedarray_short";
        case panda_file::Type::TypeId::I32:
            return "ani_fixedarray_int";
        case panda_file::Type::TypeId::I64:
            return "ani_fixedarray_long";
        case panda_file::Type::TypeId::F32:
            return "ani_fixedarray_float";
        case panda_file::Type::TypeId::F64:
            return "ani_fixedarray_double";
        case panda_file::Type::TypeId::REFERENCE:
            return "ani_fixedarray_ref";
        default:
            UNREACHABLE();
            return "";
    }
}

static bool IsExpectedFixedArrayType(EtsClass *klass, panda_file::Type::TypeId expectedType)
{
    if (klass == nullptr || !klass->IsArrayClass()) {
        return false;
    }
    EtsClass *componentType = klass->GetComponentType();
    return componentType != nullptr && componentType->IsPrimitive() && componentType->GetType().GetId() == expectedType;
}

static bool IsFixedArrayRefType(EtsClass *klass)
{
    if (klass == nullptr || !klass->IsArrayClass()) {
        return false;
    }
    EtsClass *componentType = klass->GetComponentType();
    return componentType != nullptr && !componentType->IsPrimitive();
}

class CallArgs {
public:
    explicit CallArgs(const EtsMethod *etsMethod, const ani_value *args)
        : method_(etsMethod->GetPandaMethod()), args_(args)
    {
    }

    size_t GetNrArgs() const
    {
        return method_->GetNumArgs();
    }

    template <typename Callback>
    void ForEachArgs(Callback callback) const
    {
        static_assert(std::is_invocable_r_v<bool, Callback, ani_value, panda_file::Type, size_t>,
                      "wrong callback signature");

        panda_file::ShortyIterator it(method_->GetShorty());
        panda_file::ShortyIterator end;
        ++it;  // skip the return value

        size_t i = 0;
        size_t refArgNumber = 0;
        if (!method_->IsStatic()) {
            ++refArgNumber;
        }
        for (; it != end; ++it) {
            panda_file::Type type = *it;
            if (!callback(args_[i++], type, refArgNumber)) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return;
            }
            if (type.IsReference()) {
                ++refArgNumber;
            }
        }
    }

private:
    const Method *method_ {};
    const ani_value *args_ {};
};

template <bool IS_STATIC>
static bool CheckUniqueMethodByName(EtsClass *klass, const char *name)
{
    ASSERT(klass != nullptr);
    ASSERT(name != nullptr);
    size_t nameCounter = 0;
    auto methodsList = (panda_file_items::CTOR == name) ? klass->GetConstructors() : klass->GetMethods();
    for (auto &method : methodsList) {
        if (method->IsStatic() == IS_STATIC && ::strcmp(method->GetName(), name) == 0) {
            if (++nameCounter == 2U) {
                return false;
            }
        }
    }
    return true;
}

template <bool IS_STATIC_METHOD>
static ani_status ResolveClassMethodByName(EtsClass *klass, const char *name, const char *signature, EtsMethod **result)
{
    ASSERT(klass != nullptr);
    ASSERT(name != nullptr);
    ASSERT(result != nullptr);
    *result = nullptr;

    if (signature == nullptr && !CheckUniqueMethodByName<IS_STATIC_METHOD>(klass, name)) {
        return ANI_AMBIGUOUS;
    }

    std::optional<EtsMethodSignature> methodSignature;
    if (signature != nullptr) {
        Mangle::ConvertSignatureToProto(methodSignature, signature);
        if (!methodSignature.has_value()) {
            return ANI_INVALID_DESCRIPTOR;
        }
    }

    EtsMethod *method = nullptr;
    if constexpr (IS_STATIC_METHOD) {
        method = klass->GetStaticMethod(name, methodSignature);
    } else {
        method = klass->GetInstanceMethod(name, methodSignature);
    }
    if (method == nullptr) {
        return ANI_NOT_FOUND;
    }

    *result = method;
    return ANI_OK;
}

ani_status ResolveNamedMethod(EtsClass *klass, const char *name, const char *signature, bool isStaticMethod,
                              EtsMethod **result)
{
    ASSERT(klass != nullptr);
    ASSERT(name != nullptr);
    ASSERT(result != nullptr);

    if (isStaticMethod) {
        return ResolveClassMethodByName<true>(klass, name, signature, result);
    }
    PandaString methodName = NormalizeMethodNameForAni(name);
    return ResolveClassMethodByName<false>(klass, methodName.c_str(), signature, result);
}

PandaString NormalizeMethodNameForAni(const char *name)
{
    ASSERT(name != nullptr);

    // Note: remove this code as soon as new mangler rules have merged.
    static constexpr std::size_t OLD_GET_SET_PREFIX_LENGTH = std::string_view("<get>").size();
    PandaString methodName(name);
    if (methodName.rfind("<get>", 0) == 0) {
        methodName.replace(0, OLD_GET_SET_PREFIX_LENGTH, "%%get-");
    } else if (methodName.rfind("<set>", 0) == 0) {
        methodName.replace(0, OLD_GET_SET_PREFIX_LENGTH, "%%set-");
    }
    return methodName;
}

static ANIArg::AniMethodArgs MakeMethodArgsFromVvaArgs(EtsMethod *etsMethod, va_list *vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs {etsMethod, nullptr, {}, true};
    ASSERT(etsMethod != nullptr);
    ASSERT(vvaArgs != nullptr);

    va_list copiedArgs;             // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_copy(copiedArgs, *vvaArgs);  // NOLINT(clang-analyzer-valist.Uninitialized)
    auto &args = methodArgs.argsStorage;
    args.reserve(etsMethod->GetNumArgs());
    panda_file::ShortyIterator it(etsMethod->GetPandaMethod()->GetShorty());
    panda_file::ShortyIterator end;
    ++it;  // skip the return value
    for (; it != end; ++it) {
        ani_value value {};
        panda_file::Type type = *it;
        READ_VALUE_FROM_VA_LIST(type, copiedArgs, value, va_end(copiedArgs));
        args.emplace_back(value);
    }
    va_end(copiedArgs);

    methodArgs.vargs = methodArgs.argsStorage.data();
    return methodArgs;
}

// Сheckers must be consistent with the type hierarchy
// ani_ref
//    |
//    +-- (ani_undefined)
//    +-- (ani_null)
//    +-- ani_object
//          |
//          +-- ani_fn_object
//          +-- ani_enum_item
//          +-- ani_error
//          +-- ani_arraybuffer
//          +-- ani_string
//          +-- ani_tuple_value
//          +-- ani_array
//          +-- ani_fixedarray
//          |    |
//          |    +-- ani_fixedarray_boolean
//          |    +-- ani_fixedarray_char
//          |    +-- ani_fixedarray_byte
//          |    +-- ani_fixedarray_short
//          |    +-- ani_fixedarray_int
//          |    +-- ani_fixedarray_long
//          |    +-- ani_fixedarray_float
//          |    +-- ani_fixedarray_double
//          |    +-- ani_fixedarray_ref
//          +-- ani_type
//              |
//              +-- ani_class
//              +-- ani_enum
//              +-- ani_namespace
//              +-- (ani_module)
std::string_view ANIRefTypeToString(ScopedManagedCodeFix &s, ani_ref ref)
{
    if (ManagedCodeAccessor::IsUndefined(ref)) {
        return "undefined";
    }
    if (s.IsNull(ref)) {
        return "null";
    }
    if (ANIRefTypeChecker::IsObject(s, ref)) {
        if (ANIRefTypeChecker::IsError(s, ref)) {
            return "ani_error";
        }
        if (ANIRefTypeChecker::IsString(s, ref)) {
            return "ani_string";
        }
        if (ANIRefTypeChecker::IsEnum(s, ref)) {
            return "ani_enum";
        }
        if (ANIRefTypeChecker::IsTupleValue(s, ref)) {
            return "ani_tuple_value";
        }
        if (ANIRefTypeChecker::IsArray(s, ref)) {
            return "ani_array";
        }
        if (ANIRefTypeChecker::IsArrayBuffer(s, ref)) {
            return "ani_arraybuffer";
        }
        if (ANIRefTypeChecker::IsFixedArray(s, ref)) {
            return FixedArrayTypeToString(s.ToInternalType(ref)->GetClass());
        }
        if (ANIRefTypeChecker::IsClass(s, ref)) {
            return "ani_class";
        }
        if (ANIRefTypeChecker::IsEnumItem(s, ref)) {
            return "ani_enum_item";
        }
        return "ani_object";
    }
    return "ani_ref";
}

std::string_view ANIFuncTypeToString(impl::VMethod::ANIMethodType funcType)
{
    // clang-format off
    switch (funcType) {
        case impl::VMethod::ANIMethodType::FUNCTION:      return "ani_function";
        case impl::VMethod::ANIMethodType::METHOD:        return "ani_method";
        case impl::VMethod::ANIMethodType::STATIC_METHOD: return "ani_static_method";
    }
    // clang-format on
    UNREACHABLE();
}

std::string_view ANIFieldTypeToString(impl::VField::ANIFieldType fieldType)
{
    // clang-format off
    switch (fieldType) {
        case impl::VField::ANIFieldType::FIELD:        return "ani_field";
        case impl::VField::ANIFieldType::STATIC_FIELD: return "ani_static_field";
        case impl::VField::ANIFieldType::VARIABLE:     return "ani_variable";
    }
    // clang-format on
    UNREACHABLE();
}

template <class T>
void FormatPointer(PandaStringStream &ss, T *ptr)
{
    if (ptr == nullptr) {
        ss << "NULL";
    } else {
        ss << ptr;
    }
}

// CC-OFFNXT(G.FUD.05) solid logic
PandaString ANIArg::GetStringValue() const
{
    PandaStringStream ss;
    if (action_ == Action::VERIFY_METHOD_RETURN_TYPE) {
        ss << "<resolved by name>";
        return ss.str();
    }
    switch (type_) {
        case ValueType::ANI_SIZE:
            ss << GetValueSize();
            break;
        case ValueType::UINT32:
            ss << "0x" << std::setfill('0') << std::setw(2U * sizeof(uint32_t)) << std::hex << GetValueU32();
            break;
        case ValueType::METHOD_ARGS:
            if (action_ == Action::VERIFY_METHOD_A_ARGS) {
                const ani_value *args = GetValueMethodArgs()->vargs;
                FormatPointer(ss, args);
            } else if (action_ == Action::VERIFY_METHOD_V_ARGS) {
                // Do nothing
            } else {
                UNREACHABLE();
            }
            break;
        case ValueType::FUNCTIONAL_OBJECT_ARGV:
            if (GetValueFunctionalObjectArgv() == nullptr) {
                FormatPointer(ss, GetValueFunctionalObjectArgv());
            } else {
                FormatPointer(ss, GetValueFunctionalObjectArgv()->argv);
            }
            break;
        default:
            FormatPointer(ss, GetValueRawPointer());
            break;
    }
    return ss.str();
}

// CC-OFFNXT(G.FUD.05) solid logic
PandaString ANIArg::GetStringType() const
{
    if (action_ == Action::VERIFY_METHOD_RETURN_TYPE) {
        return "ani_method";
    }
    // clang-format off
    switch (type_) {
        case ValueType::ANI_ENV:                          return "ani_env *";
        case ValueType::ANI_VM:                           return "ani_vm *";
        case ValueType::ANI_OPTIONS:                      return "ani_options *";
        case ValueType::ANI_SIZE:                         return "ani_size";
        case ValueType::ANI_REF:                          return "ani_ref";
        case ValueType::FUNCTIONAL_OBJECT_ARGV:           return "ani_ref *";
        case ValueType::ANI_FN_OBJECT:                    return "ani_fn_object";
        case ValueType::ANI_MODULE:                       return "ani_module";
        case ValueType::ANI_NAMESPACE:                    return "ani_namespace";
        case ValueType::ANI_TYPE:                         return "ani_ref";
        case ValueType::ANI_WREF:                         return "ani_wref";
        case ValueType::ANI_CLASS:                        return "ani_class";
        case ValueType::ANI_ENUM:                         return "ani_enum";
        case ValueType::ANI_ENUM_ITEM:                    return "ani_enum_item";
        case ValueType::ANI_METHOD:                       return "ani_method";
        case ValueType::ANI_STATIC_METHOD:                return "ani_static_method";
        case ValueType::ANI_FUNCTION:                     return "ani_function";
        case ValueType::ANI_FIELD:                        return "ani_field";
        case ValueType::ANI_STATIC_FIELD:                 return "ani_static_field";
        case ValueType::ANI_VARIABLE:                     return "ani_variable";
        case ValueType::ANI_OBJECT:                       return "ani_object";
        case ValueType::ANI_TUPLE_VALUE:                  return "ani_tuple_value";
        case ValueType::ANI_STRING:                       return "ani_string";
        case ValueType::ANI_VALUE_ARGS:                   return "const ani_value *";
        case ValueType::ANI_VVA_ARGS:                     return "va_list *";
        case ValueType::ANI_UTF8_BUFFER:                  return "char *";
        case ValueType::ANI_UTF8_STRING:                  return "const char *";
        case ValueType::ANI_UTF16_BUFFER:                 return "uint16_t *";
        case ValueType::ANI_UTF16_STRING:                 return "const uint16_t *";
        case ValueType::ANI_ENV_STORAGE:                  return "ani_env **";
        case ValueType::ANI_VM_STORAGE:                   return "ani_vm **";
        case ValueType::ANI_METHOD_STORAGE:               return "ani_method *";
        case ValueType::ANI_STATIC_METHOD_STORAGE:        return "ani_static_method *";
        case ValueType::ANI_FUNCTION_STORAGE:             return "ani_function *";
        case ValueType::ANI_MODULE_STORAGE:               return "ani_module *";
        case ValueType::ANI_NAMESPACE_STORAGE:            return "ani_namespace *";
        case ValueType::ANI_CLASS_STORAGE:                return "ani_class *";
        case ValueType::ANI_FIELD_STORAGE:                return "ani_field *";
        case ValueType::ANI_STATIC_FIELD_STORAGE:         return "ani_static_field *";
        case ValueType::ANI_VARIABLE_STORAGE:             return "ani_variable *";
        case ValueType::ANI_BOOLEAN_STORAGE:              return "ani_boolean *";
        case ValueType::ANI_CHAR_STORAGE:                 return "ani_char *";
        case ValueType::ANI_BYTE_STORAGE:                 return "ani_byte *";
        case ValueType::ANI_SHORT_STORAGE:                return "ani_short *";
        case ValueType::ANI_INT_STORAGE:                  return "ani_int *";
        case ValueType::ANI_LONG_STORAGE:                 return "ani_long *";
        case ValueType::ANI_FLOAT_STORAGE:                return "ani_float *";
        case ValueType::ANI_DOUBLE_STORAGE:               return "ani_double *";
        case ValueType::ANI_REF_STORAGE:                  return "ani_ref *";
        case ValueType::ANI_TYPE_STORAGE:                 return "ani_ref *";
        case ValueType::ANI_WREF_STORAGE:                 return "ani_wref *";
        case ValueType::ANI_OBJECT_STORAGE:               return "ani_object *";
        case ValueType::ANI_ENUM_STORAGE:                 return "ani_enum *";
        case ValueType::ANI_ENUM_ITEM_STORAGE:            return "ani_enum_item *";
        case ValueType::ANI_STRING_STORAGE:               return "ani_string *";
        case ValueType::ANI_SIZE_STORAGE:                 return "ani_size *";
        case ValueType::UINT32_STORAGE:                   return "uint32_t *";
        case ValueType::ANI_BOOLEAN:                      return "ani_boolean";
        case ValueType::ANI_CHAR:                         return "ani_char";
        case ValueType::ANI_BYTE:                         return "ani_byte";
        case ValueType::ANI_SHORT:                        return "ani_short";
        case ValueType::ANI_INT:                          return "ani_int";
        case ValueType::ANI_LONG:                         return "ani_long";
        case ValueType::ANI_FLOAT:                        return "ani_float";
        case ValueType::ANI_DOUBLE:                       return "ani_double";
        case ValueType::UINT32:                           return "uint32_t";
        case ValueType::ANI_ERROR:                        return "ani_error";
        case ValueType::ANI_ERROR_STORAGE:                return "ani_error *";
        case ValueType::ANI_ARRAY:                        return "ani_array";
        case ValueType::ANI_ARRAYBUFFER:                  return "ani_arraybuffer";
        case ValueType::ANI_FIXED_ARRAY:                  return "ani_fixedarray";
        case ValueType::ANI_FIXED_ARRAY_BOOLEAN:          return "ani_fixedarray_boolean";
        case ValueType::ANI_FIXED_ARRAY_CHAR:             return "ani_fixedarray_char";
        case ValueType::ANI_FIXED_ARRAY_BYTE:             return "ani_fixedarray_byte";
        case ValueType::ANI_FIXED_ARRAY_SHORT:            return "ani_fixedarray_short";
        case ValueType::ANI_FIXED_ARRAY_INT:              return "ani_fixedarray_int";
        case ValueType::ANI_FIXED_ARRAY_LONG:             return "ani_fixedarray_long";
        case ValueType::ANI_FIXED_ARRAY_FLOAT:            return "ani_fixedarray_float";
        case ValueType::ANI_FIXED_ARRAY_DOUBLE:           return "ani_fixedarray_double";
        case ValueType::ANI_FIXED_ARRAY_REF:              return "ani_fixedarray_ref";
        case ValueType::ANI_ARRAY_STORAGE:                return "ani_array *";
        case ValueType::ANI_ARRAYBUFFER_STORAGE:          return "ani_arraybuffer *";
        case ValueType::ANI_NATIVE_FUNCTIONS:             return "const ani_native_function *";
        case ValueType::VOID_PTR_STORAGE:                 return "void **";
        case ValueType::ANI_FIXED_ARRAY_BOOLEAN_STORAGE:  return "ani_fixedarray_boolean *";
        case ValueType::ANI_FIXED_ARRAY_CHAR_STORAGE:     return "ani_fixedarray_char *";
        case ValueType::ANI_FIXED_ARRAY_BYTE_STORAGE:     return "ani_fixedarray_byte *";
        case ValueType::ANI_FIXED_ARRAY_SHORT_STORAGE:    return "ani_fixedarray_short *";
        case ValueType::ANI_FIXED_ARRAY_INT_STORAGE:      return "ani_fixedarray_int *";
        case ValueType::ANI_FIXED_ARRAY_LONG_STORAGE:     return "ani_fixedarray_long *";
        case ValueType::ANI_FIXED_ARRAY_FLOAT_STORAGE:    return "ani_fixedarray_float *";
        case ValueType::ANI_FIXED_ARRAY_DOUBLE_STORAGE:   return "ani_fixedarray_double *";
        case ValueType::ANI_FIXED_ARRAY_REF_STORAGE:      return "ani_fixedarray_ref *";
        case ValueType::ANI_RESOLVER:                     return "ani_resolver";
        case ValueType::ANI_RESOLVER_STORAGE:             return "ani_resolver *";
        case ValueType::ANI_REF_CALL_ARGS:                return "ani_ref *";
        case ValueType::CONST_VOID_PTR:                   return "const void *";
        default:                                          UNREACHABLE(); return "";
        case ValueType::METHOD_ARGS:
            if (action_ == Action::VERIFY_METHOD_A_ARGS) {
                return "ani_value *";
            } else if (action_ == Action::VERIFY_METHOD_V_ARGS) {
                return "";
            } else {
                UNREACHABLE();
            }
            break;
    }
    // clang-format on
    UNREACHABLE();
}

static bool IsValidRawAniValue(EnvANIVerifier *envANIVerifier, ani_value v, panda_file::Type type, bool isVaArgs)
{
    panda_file::Type::TypeId typeId = type.GetId();

    if (isVaArgs) {
        constexpr ani_long BOOLEAN_MASK = ~0x1ULL;
        constexpr ani_long CHAR_MASK = ~0xffffULL;
        using I8Limits = std::numeric_limits<int8_t>;
        using I16Limits = std::numeric_limits<int16_t>;
        using I32Limits = std::numeric_limits<int32_t>;

        switch (typeId) {
            // clang-format off
            case panda_file::Type::TypeId::U1:  return 0 == (v.l & BOOLEAN_MASK);  // NOLINT(hicpp-signed-bitwise)
            case panda_file::Type::TypeId::U16: return 0 == (v.l & CHAR_MASK);     // NOLINT(hicpp-signed-bitwise)
            case panda_file::Type::TypeId::I8:  return v.i >= I8Limits::min() && v.i <= I8Limits::max();
            case panda_file::Type::TypeId::I16: return v.i >= I16Limits::min() && v.i <= I16Limits::max();
            case panda_file::Type::TypeId::I32: return v.i >= I32Limits::min() && v.i <= I32Limits::max();
            case panda_file::Type::TypeId::I64: return true;
            case panda_file::Type::TypeId::F32: return true;  // uses double
            case panda_file::Type::TypeId::F64: return true;
            case panda_file::Type::TypeId::REFERENCE:
                return envANIVerifier->IsValidRef(reinterpret_cast<VRef *>(v.r));
            case panda_file::Type::TypeId::NOVALUE: return false;
            // clang-format on
            default:
                break;
        }
    } else {
        switch (typeId) {
            // clang-format off
            case panda_file::Type::TypeId::U1:  return v.b == ANI_FALSE || v.b == ANI_TRUE;
            case panda_file::Type::TypeId::U16: return true;
            case panda_file::Type::TypeId::I8:  return true;
            case panda_file::Type::TypeId::I16: return true;
            case panda_file::Type::TypeId::I32: return true;
            case panda_file::Type::TypeId::I64: return true;
            case panda_file::Type::TypeId::F32: return true;
            case panda_file::Type::TypeId::F64: return true;
            case panda_file::Type::TypeId::REFERENCE:
                return envANIVerifier->IsValidRef(reinterpret_cast<VRef *>(v.r));
            case panda_file::Type::TypeId::NOVALUE: return false;
            // clang-format on
            default:
                break;
        }
    }

    LOG(FATAL, ANI) << "Unexpected argument type: " << static_cast<int>(typeId);
    return false;
}

static bool IsValidMethodArgRefType(ani_env *env, EtsMethod *method, VRef *ref, size_t refArgNumber)
{
    auto realRef = ref->GetRef();
    if (ManagedCodeAccessor::IsUndefined(realRef)) {
        return true;
    }

    ASSERT(env != nullptr);
    ScopedManagedCodeFix s(env);

    const char *methodArgDesc = method->GetRefArgType(refArgNumber);
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto ctx = method->GetClass()->GetLoadContext();
    Class *methodKlass = classLinker->FindLoadedClass(utf::CStringAsMutf8(methodArgDesc), ctx);

    // NOTE(ypigunova): Add check for union types #31219
    if (methodKlass == nullptr) {
        return true;
    }

    Class *inputKlass = s.ToInternalType(realRef)->GetClass()->GetRuntimeClass();
    return methodKlass->IsAssignableFrom(inputKlass);
}

struct ExtArgInfo {
    PandaString name;
    int64_t value;
    panda_file::Type type;
    bool isValid;
};

enum class ANIErrorSeverity { NONE, ERROR, FATAL };

static const char *SeverityToString(ANIErrorSeverity severity)
{
    switch (severity) {
        case ANIErrorSeverity::ERROR:
            return "ERROR";
        case ANIErrorSeverity::FATAL:
            return "FATAL";
        case ANIErrorSeverity::NONE:
            return "NONE";
        default:
            UNREACHABLE();
    }
}

class VerificationResult final {
public:
    VerificationResult() = default;
    VerificationResult(PandaString msg, ANIErrorSeverity s) : err_(std::move(msg)), severity_(s) {}

    explicit operator bool() const
    {
        return err_.has_value();
    }
    const std::optional<PandaString> &GetError() const
    {
        return err_;
    }
    ANIErrorSeverity GetSeverity() const
    {
        return severity_;
    }

    std::optional<PandaString> TakeError()
    {
        return std::move(err_);
    }

private:
    std::optional<PandaString> err_;
    ANIErrorSeverity severity_ = ANIErrorSeverity::NONE;
};

class Verifier {
public:
    explicit Verifier(VVm *vvm, VEnv *venv) : vvm_(vvm), venv_(venv) {}

    VerificationResult VerifyVm(VVm *vvm)
    {
        if (UNLIKELY(vvm == nullptr)) {
            return {"wrong VM pointer", ANIErrorSeverity::ERROR};
        }
        if (UNLIKELY(vvm != vvm_)) {
            return {"wrong VM pointer", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyEnv(VEnv *venv, bool checkPendingError)
    {
        if (UNLIKELY(venv == nullptr)) {
            return {"called from incorrect the native scope", ANIErrorSeverity::ERROR};
        }
        if (UNLIKELY(venv_ == nullptr)) {
            return {"current native thread is not attached", ANIErrorSeverity::FATAL};
        }

        if (UNLIKELY(venv != venv_)) {
            return {"called from incorrect the native scope", ANIErrorSeverity::FATAL};
        }
        auto *pandaEnv = PandaAniEnv::FromAniEnv(venv_->GetEnv());
        ASSERT(pandaEnv == EtsExecutionContext::GetCurrent()->GetPandaAniEnv());
        if (UNLIKELY(checkPendingError && pandaEnv->HasPendingException())) {
            return {"has unhandled an error", ANIErrorSeverity::ERROR};
        }
        return {};
    }

    static VerificationResult VerifySingleOption(const ani_option &opt, size_t index)
    {
        if (opt.option == nullptr) {
            PandaStringStream ss;
            ss << "wrong 'option' pointer, options->options[" << index << "].option == NULL";
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        std::string_view name = opt.option;
        constexpr std::string_view EXT_PREFIX = "--ext:";
        if (name.substr(0, EXT_PREFIX.size()) == EXT_PREFIX) {
            std::string_view subName = name.substr(EXT_PREFIX.size());
            bool isValid = !subName.empty() && (isascii(static_cast<unsigned char>(subName[0])) != 0) &&
                           (std::isalpha(static_cast<unsigned char>(subName[0])) != 0);
            if (!isValid) {
                PandaStringStream ss;
                ss << "wrong 'option' value, options->options[" << index << "].option == " << name;
                return {ss.str(), ANIErrorSeverity::ERROR};
            }
        }
        return {};
    }

    VerificationResult VerifyOptions(const ani_options *options)
    {
        if (options == nullptr) {
            return {};
        }
        if (options->options == nullptr) {
            return {"wrong 'options' pointer, options->options == NULL", ANIErrorSeverity::FATAL};
        }
        const size_t maxNrOptions = 4096;
        if (options->nr_options > maxNrOptions) {
            PandaStringStream ss;
            ss << "'nr_options' value is too large. options->nr_options == " << options->nr_options;
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        for (size_t i = 0; i < options->nr_options; ++i) {
            auto result =
                VerifySingleOption(options->options[i], i);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (result) {
                return result;
            }
        }
        return {};
    }

    template <typename Type>
    VerificationResult VerifyTypeStorage(Type value, std::string_view typeName)
    {
        if (value == nullptr) {
            PandaStringStream ss;
            ss << "nullptr for storing '" << typeName << "'";
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyArrayBufferLength(size_t length)
    {
        if (length > static_cast<size_t>(std::numeric_limits<ani_int>::max())) {
            return {"arraybuffer length is too large", ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyArrayCreateLength(ani_size length)
    {
        skipInitialElement_ = length == 0;
        return {};
    }

    VerificationResult VerifyBoolean(ani_boolean value)
    {
        if (!IsValidAniBoolean(value)) {
            return {"wrong value for ani_boolean", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    template <typename Type>
    VerificationResult VerifyTypePtr(Type value, std::string_view typeName)
    {
        if (value == nullptr) {
            PandaStringStream ss;
            ss << "argument is nullptr, expected " << typeName;
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyEnvVersion(uint32_t version)
    {
        if (!IsVersionSupported(version)) {
            return {"unsupported ANI version", ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyNrRefs(ani_size nrRefs)
    {
        if (nrRefs == 0) {
            return {"nr_refs must be greater than 0", ANIErrorSeverity::ERROR};
        }
        if (nrRefs > std::numeric_limits<uint32_t>::max()) {
            PandaStringStream ss;
            ss << "nr_refs is too large";
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyRef(VRef *vref)
    {
        if (vref == nullptr) {
            return {"reference is nullptr", ANIErrorSeverity::ERROR};
        }
        if (!GetEnvANIVerifier()->IsValidRef(vref)) {
            return {"reference not found (may be deleted, out of scope, or corrupted)", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyEscapeRef(VRef *vref)
    {
        if (vref == nullptr) {
            return {"escape reference is null", ANIErrorSeverity::ERROR};
        }
        if (GetEnvANIVerifier()->IsValidRef(vref)) {
            if (GetEnvANIVerifier()->IsValidGlobalVerifiedRef(vref)) {
                return {"wrong reference type: global reference, expected: local reference", ANIErrorSeverity::ERROR};
            }
            if (GetEnvANIVerifier()->IsValidWeakRef(reinterpret_cast<VWRef *>(vref))) {
                return {"wrong reference type: weak reference, expected: local reference", ANIErrorSeverity::ERROR};
            }
            if (!GetEnvANIVerifier()->IsValidRefInCurrentScope(vref)) {
                return {"the reference was not created in the current EscapeLocalScope", ANIErrorSeverity::ERROR};
            }
            return {};
        }
        return {"escape reference is invalid", ANIErrorSeverity::FATAL};
    }

    VerificationResult VerifyAnyApiRefIsXRefClass(ani_ref ref)
    {
        ScopedManagedCodeFix s(venv_->GetEnv());
        EtsObject *obj = s.ToInternalType(ref);
        if (obj != nullptr && !obj->GetClass()->GetRuntimeClass()->IsXRefClass()) {
            return {"Static types are not supported", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyAnyRef(VRef *vref)
    {
        auto err = VerifyRef(vref);
        if (err) {
            return err;
        }
        return VerifyAnyApiRefIsXRefClass(vref->GetRef());
    }

    VerificationResult VerifyRefCallArgs(VRefCallArgs *desc)
    {
        if (desc == nullptr) {
            return {"wrong pointer to use as argument in 'ani_ref *argv'", ANIErrorSeverity::FATAL};
        }
        desc->ClearReleaseArgvState();
        if (desc->GetArgc() == 0) {
            return {};
        }
        if (desc->GetVargv() == nullptr) {
            return {"wrong pointer to use as argument in 'ani_ref *argv'", ANIErrorSeverity::FATAL};
        }
        Span<VRef *> argvSpan(desc->GetVargv(), desc->GetArgc());
        for (VRef *argRef : argvSpan) {
            auto err = VerifyRef(argRef);
            if (err) {
                return err;
            }
        }
        auto &releaseStorage = desc->MutableReleaseArgvStorage();
        releaseStorage.reserve(desc->GetArgc());
        for (ani_size i = 0; i < desc->GetArgc(); ++i) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            releaseStorage.push_back(desc->GetVargv()[i]->GetRef());
        }
        desc->SetReleaseArgv(releaseStorage.data());
        return {};
    }

    VerificationResult VerifyType(VType *vtype)
    {
        auto err = VerifyRef(vtype);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsClass(s, vtype->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vtype->GetRef());
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        class_ = ANIRefTypeChecker::GetTypeClass(s, vtype->GetRef());
        return {};
    }

    VerificationResult VerifyGlobalRef(VRef *vgref)
    {
        if (UNLIKELY(vgref == nullptr)) {
            return {"reference is null, expected: global reference", ANIErrorSeverity::ERROR};
        }
        auto *envVerifier = GetEnvANIVerifier();
        if (envVerifier->IsValidGlobalVerifiedRef(vgref)) {
            return {};
        }

        if (envVerifier->IsValidRef(vgref)) {
            if (ManagedCodeAccessor::IsUndefined(vgref->GetRef())) {
                return {};
            }
            return {"wrong reference type: local reference, expected: global reference", ANIErrorSeverity::ERROR};
        }
        return {"wrong global reference", ANIErrorSeverity::FATAL};
    }

    VerificationResult VerifyWRef(VWRef *vwref)
    {
        if (UNLIKELY(vwref == nullptr)) {
            return {"reference is null, expected: weak reference", ANIErrorSeverity::ERROR};
        }
        auto *envVerifier = GetEnvANIVerifier();
        if (envVerifier->IsValidWeakRef(vwref)) {
            return {};
        }

        if (envVerifier->IsValidRef(reinterpret_cast<VRef *>(vwref))) {
            if (ManagedCodeAccessor::IsUndefined(vwref->GetWRef())) {
                return {};
            }
            return {"wrong weak reference", ANIErrorSeverity::ERROR};
        }
        return {"wrong weak reference", ANIErrorSeverity::FATAL};
    }

    VerificationResult VerifyFunctionalObject(VFnObject *vfnObject)
    {
        canReadFunctionalObjectArgv_ = false;
        auto err = VerifyRef(vfnObject);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsObject(s, vfnObject->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vfnObject->GetRef());
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        EtsClass *cls = s.ToInternalType(vfnObject->GetRef())->GetClass();
        if (!cls->IsFunction()) {
            return {"wrong functional object", ANIErrorSeverity::ERROR};
        }
        canReadFunctionalObjectArgv_ = true;
        return {};
    }

    VerificationResult VerifyFunctionalObjectArgv(ANIArg::AniFunctionalObjectArgv *args)
    {
        if (args == nullptr) {
            return {"wrong functional object argv", ANIErrorSeverity::FATAL};
        }

        args->releaseArgvStorage.clear();
        args->releaseArgv = args->argv;
        if (args->argc == 0) {
            return {};
        }

        if (args->argv == nullptr) {
            return {"wrong pointer to use as argument in 'ani_ref *'", ANIErrorSeverity::ERROR};
        }
        if (!canReadFunctionalObjectArgv_) {
            // Keep release parity: release checks fn kind before reading argv elements.
            return {};
        }

        args->releaseArgvStorage.reserve(args->argc);
        for (ani_size i = 0; i < args->argc; ++i) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto *varg = reinterpret_cast<VRef *>(args->argv[i]);
            auto refErr = VerifyRef(varg);
            if (refErr) {
                PandaStringStream ss;
                ss << "wrong reference in argv[" << i << "]";
                return {ss.str(), refErr.GetSeverity()};
            }
            args->releaseArgvStorage.push_back(varg->GetRef());
        }
        args->releaseArgv = args->releaseArgvStorage.data();
        return {};
    }

    VerificationResult VerifyClassRef(VRef *vref)
    {
        auto err = VerifyRef(vref);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsClass(s, vref->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vref->GetRef()) << ", expected: ani_class";
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        class_ = s.ToInternalType(static_cast<ani_class>(vref->GetRef()));
        return {};
    }

    VerificationResult VerifyClass(VClass *vclass)
    {
        return VerifyClassRef(vclass);
    }

    VerificationResult VerifyEnumDescriptor(const char *enumDescriptor)
    {
        auto err = VerifyTypePtr(enumDescriptor, "const char *");
        if (err) {
            return err;
        }

        std::optional<PandaString> enumDesc = Mangle::ConvertDescriptor(enumDescriptor);
        if (!enumDesc.has_value()) {
            PandaStringStream ss;
            ss << "enum descriptor is invalid";
            return {ss.str(), ANIErrorSeverity::ERROR};
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto *ctx = GetClassLinkerContext(s.GetExecutionContext());
        if (ctx == nullptr) {
            ctx = classLinker->GetExtension(panda_file::SourceLang::ETS)->GetBootContext();
        }

        Class *klass = classLinker->FindLoadedClass(utf::CStringAsMutf8(enumDesc.value().c_str()), ctx);
        if (klass == nullptr) {
            return {};
        }
        if (!EtsClass::FromRuntimeClass(klass)->IsEtsEnum()) {
            return {"descriptor is not enum", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyModule(VModule *vmodule)
    {
        auto err = VerifyClass(vmodule);
        if (err) {
            return err;
        }
        if (!class_->IsModule()) {
            class_ = nullptr;
            return {"wrong reference", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    template <bool IS_MODULE>
    VerificationResult VerifyFindTypeDescriptor(const char *descriptor, bool allowClassDescriptor,
                                                std::string_view invalidDescriptorError,
                                                std::string_view wrongKindError)
    {
        auto err = VerifyTypePtr(descriptor, "const char *");
        if (err) {
            return err;
        }

        std::optional<PandaString> desc = Mangle::ConvertDescriptor(descriptor, allowClassDescriptor);
        if (!desc.has_value()) {
            return {PandaString(invalidDescriptorError), ANIErrorSeverity::ERROR};
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto *ctx = GetClassLinkerContext(s.GetExecutionContext());
        if (ctx == nullptr) {
            ctx = classLinker->GetExtension(panda_file::SourceLang::ETS)->GetBootContext();
        }

        Class *klass = classLinker->FindLoadedClass(utf::CStringAsMutf8(desc.value().c_str()), ctx);
        if (klass == nullptr) {
            return {};
        }

        if (EtsClass::FromRuntimeClass(klass)->IsModule() != IS_MODULE) {
            return {PandaString(wrongKindError), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyModuleDescriptor(const char *moduleDescriptor)
    {
        auto err = VerifyTypePtr(moduleDescriptor, "const char *");
        if (err) {
            return err;
        }

        std::optional<PandaString> desc = Mangle::ConvertDescriptor(moduleDescriptor);
        if (!desc.has_value()) {
            return {"invalid module descriptor", ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyNamespaceDescriptor(const char *namespaceDescriptor)
    {
        return VerifyFindTypeDescriptor<true>(namespaceDescriptor, false, "invalid namespace descriptor",
                                              "descriptor is not namespace");
    }

    VerificationResult VerifyClassDescriptor(const char *classDescriptor)
    {
        return VerifyFindTypeDescriptor<false>(classDescriptor, true, "invalid class descriptor",
                                               "descriptor is not class");
    }

    VerificationResult VerifyEnum(VEnum *venum)
    {
        auto err = VerifyRef(venum);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsEnum(s, venum->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, venum->GetRef());
            return {ss.str(), ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyEnumItem(VEnumItem *venumItem)
    {
        auto err = VerifyRef(venumItem);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsEnumItem(s, venumItem->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, venumItem->GetRef());
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyNamespace(VNamespace *vnamespace)
    {
        auto err = VerifyClass(vnamespace);
        if (err) {
            return err;
        }
        if (!class_->IsModule()) {
            class_ = nullptr;
            return {"wrong reference", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyString(VString *vstr)
    {
        auto err = VerifyRef(vstr);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsString(s, vstr->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vstr->GetRef());
            return {ss.str(), ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyError(VError *verr)
    {
        if (verr == nullptr) {
            return {"error reference is null", ANIErrorSeverity::ERROR};
        }

        auto errMessage = VerifyRef(verr);
        if (errMessage) {
            return errMessage;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsError(s, verr->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, verr->GetRef()) << ", expected: ani_error";
            return {ss.str(), ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyArray(VArray *varray)
    {
        auto err = VerifyRef(varray);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsArray(s, varray->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, varray->GetRef());
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        return {};
    }

    VerificationResult VerifyTupleValue(VTupleValue *vtupleValue)
    {
        auto err = VerifyRef(vtupleValue);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsTupleValue(s, vtupleValue->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vtupleValue->GetRef());
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        // Save context for subsequent index validation
        currentTupleValue_ = vtupleValue;
        return {};
    }

    VerificationResult VerifyArrayBuffer(VArrayBuffer *varraybuffer)
    {
        auto err = VerifyRef(varraybuffer);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsArrayBuffer(s, varraybuffer->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, varraybuffer->GetRef())
               << ", expected: ani_arraybuffer";
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        return {};
    }

    VerificationResult VerifyFixedArray(VFixedArray *varray)
    {
        return DoVerifyFixedArray(varray, std::nullopt);
    }

    VerificationResult VerifyPrimitivesFixedArray(VFixedArray *varray, panda_file::Type::TypeId expectedType)
    {
        ASSERT(expectedType != panda_file::Type::TypeId::REFERENCE);
        return DoVerifyFixedArray(varray, expectedType);
    }

    VerificationResult DoVerifyFixedArray(VFixedArray *varray, std::optional<panda_file::Type::TypeId> expectedType)
    {
        auto err = VerifyRef(varray);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        EtsClass *klass = s.ToInternalType(varray->GetRef())->GetClass();
        if (!klass->IsArrayClass()) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, varray->GetRef()) << ", expected: ani_fixedarray";
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        if (expectedType.has_value() && !IsExpectedFixedArrayType(klass, expectedType.value())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << FixedArrayTypeToString(klass)
               << ", expected: " << FixedArrayTypeIdToString(expectedType.value());
            return {ss.str(), ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyFixedArrayRef(VFixedArrayRef *varray)
    {
        auto err = VerifyRef(varray);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        EtsClass *klass = s.ToInternalType(varray->GetRef())->GetClass();
        if (!IsFixedArrayRefType(klass)) {
            PandaStringStream ss;
            ss << "wrong reference type: "
               << (klass->IsArrayClass() ? FixedArrayTypeToString(klass) : ANIRefTypeToString(s, varray->GetRef()))
               << ", expected: ani_fixedarray_ref";
            return {ss.str(), ANIErrorSeverity::FATAL};
        }
        class_ = klass;
        return {};
    }

    VerificationResult VerifyArrayInitialRef(VRef *vref)
    {
        if (skipInitialElement_) {
            return {};
        }
        return VerifyRef(vref);
    }

    VerificationResult VerifyFixedArrayInitialRef(VRef *vref)
    {
        if (skipInitialElement_) {
            return {};
        }

        if (vref == nullptr) {
            return {"initial element is null", ANIErrorSeverity::ERROR};
        }

        return VerifyFixedArrayRefAssignable(vref, class_, ANIErrorSeverity::FATAL);
    }

    VerificationResult VerifyFixedArraySetRef(VRef *vref)
    {
        if (class_ == nullptr) {
            return {};
        }
        EtsClass *componentType = class_->GetComponentType();
        return VerifyFixedArrayRefAssignable(vref, componentType, ANIErrorSeverity::ERROR);
    }

    VerificationResult VerifyFixedArrayRefAssignable(VRef *vref, EtsClass *targetClass,
                                                     ANIErrorSeverity wrongTypeSeverity)
    {
        auto err = VerifyRef(vref);
        if (err) {
            return err;
        }

        if (ManagedCodeAccessor::IsUndefined(vref->GetRef()) || targetClass == nullptr) {
            return {};
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        EtsClass *klass = s.ToInternalType(vref->GetRef())->GetClass();
        if (!targetClass->IsAssignableFrom(klass)) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vref->GetRef())
               << ", expected: fixed array component type";
            return {ss.str(), wrongTypeSeverity};
        }
        return {};
    }

    VerificationResult VerifyRegionBuffer(const void *buffer, ani_size length)
    {
        static_cast<void>(length);
        if (buffer == nullptr) {
            return {"native buffer is null", ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyTupleIndex(ani_size index, EtsType tupleElementType)
    {
        if (currentTupleValue_ == nullptr) {
            return {};
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        auto *executionCtx = s.GetExecutionContext();
        EtsHandleScope scope(executionCtx);
        EtsHandle<EtsObject> internalTuple(executionCtx, s.ToInternalType(currentTupleValue_->GetRef()));
        auto tupleLength = DoGetTupleLength(executionCtx, internalTuple);
        if (index >= tupleLength) {
            PandaStringStream ss;
            ss << "tuple index out of range: index " << index << ", length " << tupleLength;
            return {ss.str(), ANIErrorSeverity::ERROR};
        }

        EtsClass *klass = internalTuple->GetClass();
        EtsField *field = klass->GetFieldIDByName(("$" + std::to_string(index)).c_str(), nullptr);
        ASSERT(field != nullptr);

        auto *resultField = internalTuple->GetFieldObject(field);
        if (!IsCorrectTupleElementType(executionCtx, resultField, tupleElementType)) {
            return {"wrong tuple element type at this index", ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyDelLocalRef(VRef *vref)
    {
        if (vref == nullptr) {
            return {"wrong reference", ANIErrorSeverity::ERROR};
        }
        EnvANIVerifier *envANIVerifier = GetEnvANIVerifier();
        if (!envANIVerifier->IsValidRef(vref)) {
            return {"wrong reference", ANIErrorSeverity::FATAL};
        }

        if (!envANIVerifier->IsValidLocalRef(vref) && !envANIVerifier->IsValidStackRef(vref)) {
            return {"wrong reference type: global reference", ANIErrorSeverity::ERROR};
        }

        if (envANIVerifier->IsValidStackRef(vref)) {
            return {"wrong reference", ANIErrorSeverity::FATAL};
        }

        if (!envANIVerifier->CanBeDeletedFromCurrentScope(vref)) {
            return {"a local reference can only be deleted in the scope where it was created", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyThisObject(VObject *vobject)
    {
        auto err = VerifyRef(vobject);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsObject(s, vobject->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vobject->GetRef());
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        EtsObject *etsObject = s.ToInternalType(vobject->GetRef());
        class_ = etsObject->GetClass();
        return {};
    }

    VerificationResult VerifyBoxedPrimitiveObject(VObject *vobject, EtsType primitiveType)
    {
        auto err = VerifyThisObject(vobject);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        EtsObject *etsObject = s.ToInternalType(vobject->GetRef());
        auto *expectedClass = GetPrimitiveBoxClass(s.GetExecutionContext(), primitiveType);
        auto *actualClass = etsObject->GetClass()->GetRuntimeClass();
        auto makeError = [actualClass, expectedClass]() {
            PandaStringStream ss;
            ss << "wrong boxed primitive type, actual: " << actualClass->GetName()
               << ", expected: " << expectedClass->GetName();
            return ss.str();
        };
        if (!etsObject->GetClass()->IsBoxed()) {
            return {makeError(), ANIErrorSeverity::ERROR};
        }
        if (actualClass != expectedClass) {
            return {makeError(), ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyMethodNameImpl(const char *name, bool isStatic)
    {
        auto err = VerifyTypePtr(name, "const char *");
        if (err) {
            return err;
        }
        name_ = name;
        isStaticMethodResolve_ = isStatic;
        return {};
    }

    VerificationResult VerifyMethodName(const char *name)
    {
        return VerifyMethodNameImpl(name, false);
    }

    VerificationResult VerifyStaticMethodName(const char *name)
    {
        return VerifyMethodNameImpl(name, true);
    }

    VerificationResult VerifySignature(const char *signature)
    {
        signature_ = signature;
        return {};
    }

    VerificationResult DoVerifyMethod(impl::VMethod *vmethod, impl::VMethod::ANIMethodType type, EtsType returnType)
    {
        impl::VMethod::ANIMethodType methodType = vmethod->GetType();
        if (methodType != type) {
            PandaStringStream ss;
            ss << "wrong type: " << ANIFuncTypeToString(methodType) << ", expected: " << ANIFuncTypeToString(type);
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        EtsType methodReturnType = vmethod->GetEtsMethod()->GetReturnValueType();
        if (methodReturnType != returnType) {
            PandaStringStream ss;
            ss << "wrong return type: " << EtsTypeToString(methodReturnType)
               << ", expected: " << EtsTypeToString(returnType);
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyCtor(VMethod *vctor, EtsType returnType)
    {
        if (vctor == nullptr) {
            return {"wrong ctor", ANIErrorSeverity::ERROR};
        }
        if (class_ == nullptr) {
            return {"wrong class for ctor", ANIErrorSeverity::ERROR};
        }

        if (!GetEnvANIVerifier()->IsValidMethod(vctor)) {
            return {"wrong ctor", ANIErrorSeverity::FATAL};
        }

        auto err = DoVerifyMethod(vctor, impl::VMethod::ANIMethodType::METHOD, returnType);
        if (err) {
            return err;
        }

        if (!vctor->GetEtsMethod()->IsConstructor()) {
            return {"method is not ctor", ANIErrorSeverity::FATAL};
        }

        if (vctor->GetEtsMethod()->GetClass() != class_) {
            return {"wrong class for ctor", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    template <bool IS_STATIC, typename FieldHandle>
    VerificationResult VerifyReadFieldImpl(FieldHandle *vfield, EtsType fieldType)
    {
        constexpr const char *ARG_NAME = IS_STATIC ? "static_field" : "field";
        constexpr const char *EXPECTED_TYPE = IS_STATIC ? "ani_static_field" : "ani_field";
        constexpr const char *OWNER_NAME = IS_STATIC ? "class" : "object";

        if (vfield == nullptr) {
            PandaStringStream ss;
            ss << ARG_NAME << " is nullptr, expected " << EXPECTED_TYPE;
            return {ss.str(), ANIErrorSeverity::ERROR};
        }

        if (class_ == nullptr) {
            PandaStringStream ss;
            ss << OWNER_NAME << " is nullptr or invalid while verifying " << ARG_NAME;
            return {ss.str(), ANIErrorSeverity::ERROR};
        }

        if (!GetEnvANIVerifier()->IsValidField(vfield)) {
            PandaStringStream ss;
            ss << ARG_NAME << " is not a verified " << EXPECTED_TYPE << " handle or has the wrong ANI handle type";
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        constexpr auto FIELD_KIND =
            IS_STATIC ? impl::VField::ANIFieldType::STATIC_FIELD : impl::VField::ANIFieldType::FIELD;
        auto err = DoVerifyField(vfield, FIELD_KIND, fieldType);
        if (err) {
            return err;
        }

        if (!vfield->GetEtsField()->GetDeclaringClass()->IsAssignableFrom(class_)) {
            PandaStringStream ss;
            ss << ARG_NAME << " does not belong to the " << OWNER_NAME;
            return {ss.str(), ANIErrorSeverity::FATAL};
        }
        return {};
    }

    template <bool IS_STATIC>
    VerificationResult VerifyReadFieldByNameImpl(const char *name, EtsType fieldType)
    {
        auto err = VerifyTypePtr(name, "const char *");
        if (err) {
            return err;
        }

        if (class_ == nullptr) {
            if constexpr (IS_STATIC) {
                return {"wrong class for static field", ANIErrorSeverity::ERROR};
            } else {
                return {"wrong object for field", ANIErrorSeverity::ERROR};
            }
        }

        EtsField *field = nullptr;
        if constexpr (IS_STATIC) {
            field = class_->GetStaticFieldIDByName(name, nullptr);
        } else {
            field = class_->GetFieldIDByName(name, nullptr);
        }

        if (field == nullptr) {
            if constexpr (IS_STATIC) {
                return {"static field not found", ANIErrorSeverity::ERROR};
            } else {
                return {};
            }
        }

        return DoVerifyFieldByName(field, fieldType, IS_STATIC);
    }

    template <bool IS_STATIC, typename FieldHandle>
    VerificationResult VerifyWriteFieldImpl(FieldHandle *vfield, EtsType fieldType)
    {
        auto err = VerifyReadFieldImpl<IS_STATIC>(vfield, fieldType);
        if (err) {
            return err;
        }

        if (vfield->GetEtsField()->IsReadonly()) {
            if constexpr (IS_STATIC) {
                return {"static field is read-only", ANIErrorSeverity::FATAL};
            } else {
                return {"field is read-only", ANIErrorSeverity::FATAL};
            }
        }
        return {};
    }

    template <bool IS_STATIC>
    VerificationResult VerifyWriteFieldByNameImpl(const char *name, EtsType fieldType)
    {
        auto err = VerifyTypePtr(name, "const char *");
        if (err) {
            return err;
        }

        if (class_ == nullptr) {
            if constexpr (IS_STATIC) {
                return {"wrong class for static field", ANIErrorSeverity::ERROR};
            } else {
                return {"wrong object for field", ANIErrorSeverity::ERROR};
            }
        }

        EtsField *field = nullptr;
        if constexpr (IS_STATIC) {
            field = class_->GetStaticFieldIDByName(name, nullptr);
        } else {
            field = class_->GetFieldIDByName(name, nullptr);
        }

        if (field == nullptr) {
            if constexpr (IS_STATIC) {
                return {"static field not found", ANIErrorSeverity::ERROR};
            } else {
                return {};
            }
        }

        err = DoVerifyFieldByName(field, fieldType, IS_STATIC);
        if (err) {
            return err;
        }

        if (field->IsReadonly()) {
            if constexpr (IS_STATIC) {
                return {"static field is read-only", ANIErrorSeverity::FATAL};
            } else {
                return {"field is read-only", ANIErrorSeverity::FATAL};
            }
        }

        return {};
    }

    VerificationResult VerifyReadField(VField *vfield, EtsType fieldType)
    {
        return VerifyReadFieldImpl<false>(vfield, fieldType);
    }

    VerificationResult VerifyReadStaticField(VStaticField *vstaticfield, EtsType staticFieldType)
    {
        return VerifyReadFieldImpl<true>(vstaticfield, staticFieldType);
    }

    VerificationResult VerifyReadFieldByName(const char *name, EtsType fieldType)
    {
        return VerifyReadFieldByNameImpl<false>(name, fieldType);
    }

    VerificationResult VerifyReadStaticFieldByName(const char *name, EtsType staticFieldType)
    {
        return VerifyReadFieldByNameImpl<true>(name, staticFieldType);
    }

    VerificationResult VerifyWriteField(VField *vfield, EtsType fieldType)
    {
        return VerifyWriteFieldImpl<false>(vfield, fieldType);
    }

    VerificationResult VerifyWriteStaticField(VStaticField *vstaticfield, EtsType staticFieldType)
    {
        return VerifyWriteFieldImpl<true>(vstaticfield, staticFieldType);
    }

    VerificationResult VerifyWriteFieldByName(const char *name, EtsType fieldType)
    {
        return VerifyWriteFieldByNameImpl<false>(name, fieldType);
    }

    VerificationResult VerifyWriteStaticFieldByName(const char *name, EtsType staticFieldType)
    {
        return VerifyWriteFieldByNameImpl<true>(name, staticFieldType);
    }

    template <bool IS_STATIC>
    VerificationResult VerifyFindFieldNameImpl(const char *name)
    {
        auto err = VerifyTypePtr(name, "const char *");
        if (err) {
            return err;
        }

        if (class_ == nullptr) {
            if constexpr (IS_STATIC) {
                return {"wrong class for static field", ANIErrorSeverity::ERROR};
            } else {
                return {"wrong class for field", ANIErrorSeverity::ERROR};
            }
        }

        EtsField *field = nullptr;
        if constexpr (IS_STATIC) {
            field = class_->GetStaticFieldIDByName(name, nullptr);
        } else {
            field = class_->GetFieldIDByName(name, nullptr);
        }

        if (field != nullptr) {
            return {};
        }

        if constexpr (IS_STATIC) {
            return {"static field not found", ANIErrorSeverity::ERROR};
        } else {
            return {"field not found", ANIErrorSeverity::ERROR};
        }
    }

    template <bool IS_STATIC>
    VerificationResult VerifyFindMethodSignatureImpl(const char *signature)
    {
        signature_ = signature;
        if (class_ == nullptr) {
            if constexpr (IS_STATIC) {
                return {"wrong class for static method", ANIErrorSeverity::ERROR};
            } else {
                return {"wrong class for method", ANIErrorSeverity::ERROR};
            }
        }
        if (name_ == nullptr) {
            return {};
        }

        if constexpr (IS_STATIC) {
            return VerifyFindMethod(name_, signature_, true, "static method not found");
        } else {
            return VerifyFindMethod(name_, signature_, false, "method not found");
        }
    }

    template <bool IS_SETTER>
    VerificationResult VerifyFindAccessorNameImpl(const char *name)
    {
        auto err = VerifyTypePtr(name, "const char *");
        if (err) {
            return err;
        }
        if (class_ == nullptr) {
            if constexpr (IS_SETTER) {
                return {"wrong class for setter", ANIErrorSeverity::ERROR};
            } else {
                return {"wrong class for getter", ANIErrorSeverity::ERROR};
            }
        }

        PandaString methodName(IS_SETTER ? "%%set-" : "%%get-");
        methodName += name;
        if constexpr (IS_SETTER) {
            return VerifyFindMethod(methodName.c_str(), nullptr, false, "setter not found");
        } else {
            return VerifyFindMethod(methodName.c_str(), nullptr, false, "getter not found");
        }
    }

    template <bool IS_SETTER>
    VerificationResult VerifyFindIndexableSignatureImpl(const char *signature)
    {
        signature_ = signature;
        if (class_ == nullptr) {
            if constexpr (IS_SETTER) {
                return {"wrong class for indexable setter", ANIErrorSeverity::ERROR};
            } else {
                return {"wrong class for indexable getter", ANIErrorSeverity::ERROR};
            }
        }

        if constexpr (IS_SETTER) {
            return VerifyFindMethod("$_set", signature_, false, "indexable setter not found");
        } else {
            return VerifyFindMethod("$_get", signature_, false, "indexable getter not found");
        }
    }

    VerificationResult VerifyFindFieldName(const char *name)
    {
        return VerifyFindFieldNameImpl<false>(name);
    }

    VerificationResult VerifyFindStaticFieldName(const char *name)
    {
        return VerifyFindFieldNameImpl<true>(name);
    }

    VerificationResult VerifyFindVariableName(const char *name)
    {
        auto err = VerifyTypePtr(name, "const char *");
        if (err) {
            return err;
        }

        if (class_ == nullptr) {
            return {"wrong class for variable", ANIErrorSeverity::ERROR};
        }

        EtsField *field = class_->GetStaticFieldIDByName(name, nullptr);
        if (field != nullptr) {
            return {};
        }
        return {"variable not found", ANIErrorSeverity::ERROR};
    }

    VerificationResult VerifyFindFunctionName(const char *name)
    {
        return VerifyMethodNameImpl(name, true);
    }

    VerificationResult VerifyFindMethodSignature(const char *signature)
    {
        return VerifyFindMethodSignatureImpl<false>(signature);
    }

    VerificationResult VerifyFindStaticMethodSignature(const char *signature)
    {
        return VerifyFindMethodSignatureImpl<true>(signature);
    }

    VerificationResult VerifyFindFunctionSignature(const char *signature)
    {
        signature_ = signature;
        if (class_ == nullptr) {
            return {"wrong class for function", ANIErrorSeverity::ERROR};
        }
        if (name_ == nullptr) {
            return {};
        }
        return VerifyFindMethod(name_, signature_, true, "function not found");
    }

    VerificationResult VerifyFindSetterName(const char *name)
    {
        return VerifyFindAccessorNameImpl<true>(name);
    }

    VerificationResult VerifyFindGetterName(const char *name)
    {
        return VerifyFindAccessorNameImpl<false>(name);
    }

    VerificationResult VerifyFindIndexableGetterSignature(const char *signature)
    {
        return VerifyFindIndexableSignatureImpl<false>(signature);
    }

    VerificationResult VerifyFindIndexableSetterSignature(const char *signature)
    {
        return VerifyFindIndexableSignatureImpl<true>(signature);
    }

    VerificationResult VerifyFindIterator(VClass *vclass)
    {
        auto err = VerifyClass(vclass);
        if (err) {
            return err;
        }

        return VerifyFindMethod("$_iterator", nullptr, false, "iterator not found");
    }

    VerificationResult VerifyReadPropertyByName(const char *name, EtsType propertyType)
    {
        auto err = VerifyTypePtr(name, "const char *");
        if (err) {
            return err;
        }
        if (class_ == nullptr) {
            return {};
        }
        EtsField *field = class_->GetFieldIDByName(name, nullptr);
        if (field != nullptr) {
            if (field->GetEtsType() != propertyType) {
                PandaStringStream ss;
                ss << "wrong property type: " << EtsTypeToString(field->GetEtsType())
                   << ", expected: " << EtsTypeToString(propertyType);
                return {ss.str(), ANIErrorSeverity::ERROR};
            }
            return {};
        }

        EtsMethod *method = class_->GetInstanceMethod((PandaString(GETTER_BEGIN) + name).c_str(), nullptr);
        if (method == nullptr || method->IsStatic() || method->GetNumArgs() != 1 ||
            method->GetArgType(0) != EtsType::OBJECT) {
            return {"wrong property: expected an instance getter with receiver object argument",
                    ANIErrorSeverity::ERROR};
        }
        if (method->GetReturnValueType() != propertyType) {
            PandaStringStream ss;
            ss << "wrong property type: " << EtsTypeToString(method->GetReturnValueType())
               << ", expected: " << EtsTypeToString(propertyType);
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyWritePropertyByName(const char *name, EtsType propertyType)
    {
        auto err = VerifyTypePtr(name, "const char *");
        if (err) {
            return err;
        }
        if (class_ == nullptr) {
            return {};
        }
        EtsField *field = class_->GetFieldIDByName(name, nullptr);
        if (field != nullptr) {
            if (field->GetEtsType() != propertyType) {
                PandaStringStream ss;
                ss << "wrong property type: " << EtsTypeToString(field->GetEtsType())
                   << ", expected: " << EtsTypeToString(propertyType);
                return {ss.str(), ANIErrorSeverity::ERROR};
            }
            if (field->IsReadonly()) {
                return {"property is read-only", ANIErrorSeverity::FATAL};
            }
            return {};
        }

        EtsMethod *method = class_->GetInstanceMethod((PandaString(SETTER_BEGIN) + name).c_str(), nullptr);
        if (method == nullptr || method->IsStatic() || method->GetNumArgs() != 2U ||
            method->GetArgType(0) != EtsType::OBJECT) {
            return {"wrong property: expected an instance setter with receiver object argument and value argument",
                    ANIErrorSeverity::ERROR};
        }
        if (method->GetArgType(1U) != propertyType) {
            PandaStringStream ss;
            ss << "wrong property type: " << EtsTypeToString(method->GetArgType(1U))
               << ", expected: " << EtsTypeToString(propertyType);
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        if (method->GetReturnValueType() != EtsType::VOID && method->GetReturnValueType() != EtsType::NOVALUE) {
            PandaStringStream ss;
            ss << "wrong property: setter return type is " << EtsTypeToString(method->GetReturnValueType())
               << ", expected: " << EtsTypeToString(EtsType::VOID) << " or " << EtsTypeToString(EtsType::NOVALUE);
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyReadVariable(VVariable *vvariable, EtsType variableType)
    {
        if (vvariable == nullptr) {
            return {"variable is nullptr", ANIErrorSeverity::ERROR};
        }

        if (!GetEnvANIVerifier()->IsValidField(vvariable)) {
            return {"variable is not a verified ani_variable handle or has the wrong ANI handle type",
                    ANIErrorSeverity::FATAL};
        }

        auto err = DoVerifyField(vvariable, impl::VField::ANIFieldType::VARIABLE, variableType);
        if (err) {
            return err;
        }
        return {};
    }

    VerificationResult VerifyWriteVariable(VVariable *vvariable, EtsType variableType)
    {
        auto err = VerifyReadVariable(vvariable, variableType);
        if (err) {
            return err;
        }

        // Note: Issue #34319
        // At present, the frontend does not preserve `const` as a dedicated bytecode/runtime
        // flag that ANI can verify.
        // After the frontend adaptation keeps this information, the const variable check
        // can work correctly.
        if (vvariable->GetEtsField()->IsReadonly()) {
            return {"variable is const", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyMethod(VMethod *vmethod, EtsType returnType)
    {
        if (vmethod == nullptr) {
            return {"method is nullptr", ANIErrorSeverity::ERROR};
        }
        if (class_ == nullptr) {
            return {"object is nullptr or invalid while verifying method", ANIErrorSeverity::ERROR};
        }
        if (!GetEnvANIVerifier()->IsValidMethod(vmethod)) {
            return {"ani_method handle not found (may be deleted, out of scope, or corrupted)",
                    ANIErrorSeverity::FATAL};
        }

        auto err = DoVerifyMethod(vmethod, impl::VMethod::ANIMethodType::METHOD, returnType);
        if (err) {
            return err;
        }

        if (vmethod->GetEtsMethod()->IsConstructor()) {
            return {"method is ctor", ANIErrorSeverity::FATAL};
        }

        if (!vmethod->GetEtsMethod()->GetClass()->IsAssignableFrom(class_)) {
            return {"method does not belong to the object", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyStaticMethod(VStaticMethod *vstaticmethod, EtsType returnType)
    {
        if (vstaticmethod == nullptr) {
            return {"wrong static method", ANIErrorSeverity::ERROR};
        }
        if (class_ == nullptr) {
            return {"wrong class for method", ANIErrorSeverity::ERROR};
        }
        if (!GetEnvANIVerifier()->IsValidMethod(vstaticmethod)) {
            return {"wrong static method", ANIErrorSeverity::FATAL};
        }
        auto err = DoVerifyMethod(vstaticmethod, impl::VMethod::ANIMethodType::STATIC_METHOD, returnType);
        if (err) {
            return err;
        }

        if (!vstaticmethod->GetEtsMethod()->GetClass()->IsAssignableFrom(class_)) {
            return {"wrong class for method", ANIErrorSeverity::FATAL};
        }
        return {};
    }

    VerificationResult VerifyFunction(VFunction *vfunction, EtsType returnType)
    {
        if (vfunction == nullptr) {
            return {"expected non-null function", ANIErrorSeverity::ERROR};
        }

        if (!GetEnvANIVerifier()->IsValidMethod(vfunction)) {
            return {"wrong function", ANIErrorSeverity::FATAL};
        }
        auto err = DoVerifyMethod(vfunction, impl::VMethod::ANIMethodType::FUNCTION, returnType);
        if (err) {
            return err;
        }

        return {};
    }

    VerificationResult DoVerifyField(impl::VField *vfield, impl::VField::ANIFieldType type, EtsType returnType)
    {
        impl::VField::ANIFieldType fieldType = vfield->GetType();
        if (fieldType != type) {
            PandaStringStream ss;
            ss << "wrong type: " << ANIFieldTypeToString(fieldType) << ", expected: " << ANIFieldTypeToString(type);
            return {ss.str(), ANIErrorSeverity::FATAL};
        }

        EtsType fieldReturnType = vfield->GetEtsField()->GetEtsType();
        if (fieldReturnType != returnType) {
            PandaStringStream ss;
            ss << "wrong value type: " << EtsTypeToString(fieldReturnType)
               << ", expected: " << EtsTypeToString(returnType);
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyMethodReturnType(EtsType returnType)
    {
        if (class_ == nullptr || name_ == nullptr) {
            return {};
        }

        EtsMethod *etsMethod = ResolveMethodByName();
        if (etsMethod == nullptr) {
            return {};
        }

        EtsType methodReturnType = etsMethod->GetReturnValueType();
        if (methodReturnType != returnType) {
            PandaStringStream ss;
            ss << "wrong return type: " << EtsTypeToString(methodReturnType)
               << ", expected: " << EtsTypeToString(returnType);
            return {ss.str(), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult DoVerifyFieldByName(EtsField *field, EtsType expectedType, bool isStaticField)
    {
        if (field->IsStatic() != isStaticField) {
            if (isStaticField) {
                return {"wrong static field", ANIErrorSeverity::FATAL};
            }
            return {"wrong field", ANIErrorSeverity::FATAL};
        }

        if (field->GetEtsType() != expectedType) {
            PandaStringStream ss;
            ss << "wrong field type: " << EtsTypeToString(field->GetEtsType())
               << ", expected: " << EtsTypeToString(expectedType);
            return {ss.str(), ANIErrorSeverity::ERROR};
        }

        if (!field->GetDeclaringClass()->IsAssignableFrom(class_)) {
            if (isStaticField) {
                return {"wrong class for static field", ANIErrorSeverity::FATAL};
            }
            return {"wrong object for field", ANIErrorSeverity::FATAL};
        }

        return {};
    }

    VerificationResult VerifyFindMethod(const char *name, const char *signature, bool isStaticMethod,
                                        std::string_view errorMessage)
    {
        EtsMethod *method = nullptr;
        ani_status status = ResolveNamedMethod(class_, name, signature, isStaticMethod, &method);
        if (status == ANI_OK) {
            return {};
        }
        if (status == ANI_INVALID_DESCRIPTOR) {
            return {"method signature descriptor is invalid", ANIErrorSeverity::ERROR};
        }
        if (status == ANI_AMBIGUOUS) {
            return {"method lookup is ambiguous", ANIErrorSeverity::ERROR};
        }
        return {PandaString(errorMessage), ANIErrorSeverity::ERROR};
    }

    static PandaString GetNativeFunctionError(ani_size index, std::string_view message)
    {
        PandaStringStream ss;
        ss << message << " at index " << index;
        return ss.str();
    }

    static std::string_view GetNativeMethodKind(bool isStaticMethod, bool isClassMethod)
    {
        if (!isClassMethod) {
            return "native function";
        }
        return isStaticMethod ? "static native method" : "native method";
    }

    VerificationResult VerifyNativeFunctions(const ani_native_function *functions, ani_size nrFunctions)
    {
        return DoVerifyNativeFunctions(functions, nrFunctions, true, false);
    }

    VerificationResult VerifyNativeMethods(const ani_native_function *methods, ani_size nrMethods)
    {
        return DoVerifyNativeFunctions(methods, nrMethods, false, true);
    }

    VerificationResult VerifyStaticNativeMethods(const ani_native_function *methods, ani_size nrMethods)
    {
        return DoVerifyNativeFunctions(methods, nrMethods, true, true);
    }

    VerificationResult DoVerifyNativeFunctions(const ani_native_function *functions, ani_size nrFunctions,
                                               bool isStaticMethod, bool isClassMethod)
    {
        auto err = VerifyTypePtr(functions, "const ani_native_function *");
        if (err) {
            return err;
        }
        if (nrFunctions == 0) {
            return {};
        }
        if (class_ == nullptr) {
            PandaStringStream ss;
            ss << "wrong class for " << GetNativeMethodKind(isStaticMethod, isClassMethod);
            return {ss.str(), ANIErrorSeverity::ERROR};
        }

        for (ani_size i = 0; i < nrFunctions; ++i) {
            const auto &function = functions[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            err = VerifyNativeFunction(function, i, isStaticMethod, isClassMethod);
            if (err) {
                return err;
            }
        }
        return {};
    }

    VerificationResult VerifyResolvedNativeFunction(EtsMethod *method, ani_status status, ani_size index,
                                                    std::string_view nativeMethodKind)
    {
        if (status == ANI_AMBIGUOUS) {
            PandaStringStream ss;
            ss << "ambiguous " << nativeMethodKind;
            return {GetNativeFunctionError(index, ss.str()), ANIErrorSeverity::ERROR};
        }
        if (method == nullptr) {
            PandaStringStream ss;
            ss << nativeMethodKind << " not found";
            return {GetNativeFunctionError(index, ss.str()), ANIErrorSeverity::ERROR};
        }
        if (!method->IsNative()) {
            PandaStringStream ss;
            ss << nativeMethodKind << " is not native";
            return {GetNativeFunctionError(index, ss.str()), ANIErrorSeverity::ERROR};
        }
        if (method->IsIntrinsic()) {
            PandaStringStream ss;
            ss << nativeMethodKind << " is intrinsic";
            return {GetNativeFunctionError(index, ss.str()), ANIErrorSeverity::ERROR};
        }
        if (method->IsBoundNativeFunction()) {
            PandaStringStream ss;
            ss << nativeMethodKind << " is already bound";
            return {GetNativeFunctionError(index, ss.str()), ANIErrorSeverity::ERROR};
        }
        return {};
    }

    VerificationResult VerifyNativeFunction(const ani_native_function &function, ani_size index, bool isStaticMethod,
                                            bool isClassMethod)
    {
        std::string_view nativeMethodKind = GetNativeMethodKind(isStaticMethod, isClassMethod);
        if (function.name == nullptr) {
            PandaStringStream ss;
            ss << "wrong " << nativeMethodKind << " name";
            return {GetNativeFunctionError(index, ss.str()), ANIErrorSeverity::FATAL};
        }
        if (function.pointer == nullptr) {
            PandaStringStream ss;
            ss << "wrong " << nativeMethodKind << " pointer";
            return {GetNativeFunctionError(index, ss.str()), ANIErrorSeverity::FATAL};
        }
        std::optional<EtsMethodSignature> methodSignature;
        if (function.signature != nullptr) {
            Mangle::ConvertSignatureToProto(methodSignature, function.signature);
            if (!methodSignature.has_value()) {
                PandaStringStream ss;
                ss << "wrong " << nativeMethodKind << " signature";
                return {GetNativeFunctionError(index, ss.str()), ANIErrorSeverity::ERROR};
            }
        }
        EtsMethod *method = nullptr;
        ani_status status =
            ResolveNativeCallable(function.name, methodSignature, isStaticMethod, isClassMethod, &method);
        return VerifyResolvedNativeFunction(method, status, index, nativeMethodKind);
    }

    ani_status ResolveNativeCallable(const char *name, std::optional<EtsMethodSignature> &methodSignature,
                                     bool isStaticMethod, bool isClassMethod, EtsMethod **result)
    {
        ASSERT(result != nullptr);
        if (!isClassMethod) {
            *result = class_->GetStaticMethod(name, methodSignature);
            return ANI_OK;
        }

        if (isStaticMethod && methodSignature.has_value()) {
            *result = class_->GetDirectMethod(true, name, methodSignature.value());
            return ANI_OK;
        }

        if (!methodSignature.has_value()) {
            bool isUnique = false;
            *result = class_->GetDirectMethod(isStaticMethod, name, &isUnique);
            return isUnique ? ANI_OK : ANI_AMBIGUOUS;
        }

        *result = class_->GetDirectMethod(isStaticMethod, name, methodSignature.value());
        return ANI_OK;
    }

    VerificationResult VerifyMethodAArgs(ANIArg::AniMethodArgs *methodArgs)
    {
        ASSERT(methodArgs != nullptr);
        if (methodArgs->vargs == nullptr) {
            return {"wrong arguments value", ANIErrorSeverity::ERROR};
        }
        if (methodArgs->method == nullptr) {
            return {"wrong method", ANIErrorSeverity::ERROR};
        }
        return DoVerifyMethodArgs(methodArgs);
    }

    VerificationResult VerifyMethodVArgs(ANIArg::AniMethodArgs *methodArgs)
    {
        ASSERT(methodArgs != nullptr);
        if (methodArgs->method == nullptr) {
            return {"wrong method", ANIErrorSeverity::ERROR};
        }
        return DoVerifyMethodArgs(methodArgs);
    }

    VerificationResult VerifyAArgs(const ani_value *vargs)
    {
        if (class_ == nullptr || name_ == nullptr) {
            return {};
        }
        EtsMethod *etsMethod = ResolveMethodByName();
        if (etsMethod == nullptr) {
            return {};
        }
        if (vargs == nullptr && etsMethod->GetNumArgs() != 0) {
            return {"wrong arguments value", ANIErrorSeverity::ERROR};
        }

        methodArgsForExtInfo_ = ANIArg::AniMethodArgs {etsMethod, vargs, {}, false};
        return DoVerifyMethodArgs(&methodArgsForExtInfo_.value());
    }

    VerificationResult VerifyVvaArgs(va_list *vvaArgs)
    {
        if (vvaArgs == nullptr || class_ == nullptr || name_ == nullptr) {
            return {};
        }

        EtsMethod *etsMethod = ResolveMethodByName();
        if (etsMethod == nullptr) {
            return {};
        }

        methodArgsForExtInfo_ = MakeMethodArgsFromVvaArgs(etsMethod, vvaArgs);
        return DoVerifyMethodArgs(&methodArgsForExtInfo_.value());
    }

    VerificationResult DoVerifyMethodArgs(ANIArg::AniMethodArgs *methodArgs)
    {
        CallArgs callArgs(methodArgs->method, methodArgs->vargs);
        EnvANIVerifier *envANIVerifier = GetEnvANIVerifier();
        VerificationResult err;

        callArgs.ForEachArgs([&](ani_value value, panda_file::Type type, size_t refIndex) -> bool {
            if (UNLIKELY(!IsValidRawAniValue(envANIVerifier, value, type, methodArgs->isVaArgs))) {
                err = {"wrong method arguments", ANIErrorSeverity::FATAL};
                return false;
            }
            if (type.IsReference()) {
                if (UNLIKELY(!IsValidMethodArgRefType(venv_->GetEnv(), methodArgs->method,
                                                      reinterpret_cast<VRef *>(value.r), refIndex))) {
                    err = {"wrong method arguments", ANIErrorSeverity::FATAL};
                    return false;
                }
            }
            return true;
        });
        return err;
    }

    VerificationResult VerifyResolver(VResolver *vresolver)
    {
        if (vresolver == nullptr) {
            return {"wrong resolver", ANIErrorSeverity::ERROR};
        }

        if (!GetEnvANIVerifier()->IsValidGlobalResolver(vresolver)) {
            return {"wrong resolver", ANIErrorSeverity::FATAL};
        }

        return {};
    }

    std::optional<PandaVector<ExtArgInfo>> GetExtArgInfoListForResolvedArgs(PandaAniEnv *pandaEnv) const;

private:
    EnvANIVerifier *GetEnvANIVerifier()
    {
        ASSERT(venv_ != nullptr);
        return PandaAniEnv::FromAniEnv(venv_->GetEnv())->GetEnvANIVerifier();
    }

    EtsMethod *ResolveMethodByName()
    {
        if (methodByNameResolved_) {
            return resolvedMethod_;
        }
        methodByNameResolved_ = true;
        if (class_ == nullptr || name_ == nullptr) {
            return nullptr;
        }
        ResolveNamedMethod(class_, name_, signature_, isStaticMethodResolve_, &resolvedMethod_);
        return resolvedMethod_;
    }

    VVm *vvm_ {};
    VEnv *venv_ {};

    EtsClass *class_ {};
    const char *name_ {};
    const char *signature_ {};
    bool methodByNameResolved_ {false};
    bool isStaticMethodResolve_ {false};
    EtsMethod *resolvedMethod_ {nullptr};
    std::optional<ANIArg::AniMethodArgs> methodArgsForExtInfo_ {};
    VTupleValue *currentTupleValue_ {};
    bool skipInitialElement_ {false};
    bool canReadFunctionalObjectArgv_ {true};
};

using CheckerHandler = VerificationResult (*)(Verifier &, const ANIArg &);

static VerificationResult VerifyVm(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_VM);
    return v.VerifyVm(arg.GetValueVm());
}

static VerificationResult VerifyEnv(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENV);
    return v.VerifyEnv(arg.GetValueEnv(), true);
}

static VerificationResult VerifyEnvSkipPendingError(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENV_SKIP_PENDING_ERROR);
    return v.VerifyEnv(arg.GetValueEnv(), false);
}

static VerificationResult VerifyOptions(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_OPTIONS);
    return v.VerifyOptions(arg.GetValueOptions());
}

static VerificationResult VerifyEnvVersion(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENV_VERSION);
    return v.VerifyEnvVersion(arg.GetValueU32());
}

static VerificationResult VerifyNrRefs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_NR_REFS);
    return v.VerifyNrRefs(arg.GetValueSize());
}

static VerificationResult VerifyRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_REF);
    return v.VerifyRef(arg.GetValueRef());
}

static VerificationResult VerifyAnyRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ANY_REF);
    return v.VerifyAnyRef(arg.GetValueRef());
}

static VerificationResult VerifyModule(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_MODULE);
    return v.VerifyModule(arg.GetValueModule());
}

static VerificationResult VerifyNamespace(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_NAMESPACE);
    return v.VerifyNamespace(arg.GetValueNamespace());
}

static VerificationResult VerifyEscapeRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ESCAPE_REF);
    return v.VerifyEscapeRef(arg.GetValueRef());
}

static VerificationResult VerifyType(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_TYPE);
    return v.VerifyType(arg.GetValueType());
}

static VerificationResult VerifyGlobalRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_GLOBAL_REF);
    return v.VerifyGlobalRef(arg.GetValueRef());
}

static VerificationResult VerifyWRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_WREF);
    return v.VerifyWRef(arg.GetValueWRef());
}

static VerificationResult VerifyFunctionalObject(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FUNCTIONAL_OBJECT);
    return v.VerifyFunctionalObject(arg.GetValueFnObject());
}

static VerificationResult VerifyFunctionalObjectArgv(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FUNCTIONAL_OBJECT_ARGV);
    return v.VerifyFunctionalObjectArgv(arg.GetValueFunctionalObjectArgv());
}

static VerificationResult VerifyClass(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_CLASS);
    return v.VerifyClass(arg.GetValueClass());
}

static VerificationResult VerifyEnum(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENUM);
    return v.VerifyEnum(arg.GetValueEnum());
}

static VerificationResult VerifyEnumItem(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENUM_ITEM);
    return v.VerifyEnumItem(arg.GetValueEnumItem());
}

static VerificationResult VerifyString(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STRING);
    return v.VerifyString(arg.GetValueString());
}

static VerificationResult VerifyUTF8Buffer(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_UTF8_BUFFER);
    return v.VerifyTypePtr(arg.GetValueUTF8Buffer(), "char *");
}

static VerificationResult VerifyError(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ERROR);
    return v.VerifyError(arg.GetValueError());
}

static VerificationResult VerifyUTF8String(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_UTF8_STRING);
    return v.VerifyTypePtr(arg.GetValueUTF8String(), "const char *");
}

static VerificationResult VerifyModuleDescriptor(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_MODULE_DESCRIPTOR);
    return v.VerifyModuleDescriptor(arg.GetValueUTF8String());
}

static VerificationResult VerifyNamespaceDescriptor(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_NAMESPACE_DESCRIPTOR);
    return v.VerifyNamespaceDescriptor(arg.GetValueUTF8String());
}

static VerificationResult VerifyClassDescriptor(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_CLASS_DESCRIPTOR);
    return v.VerifyClassDescriptor(arg.GetValueUTF8String());
}

static VerificationResult VerifyEnumDescriptor(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENUM_DESCRIPTOR);
    return v.VerifyEnumDescriptor(arg.GetValueUTF8String());
}

static VerificationResult VerifyMethodName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD_NAME);
    return v.VerifyMethodName(arg.GetValueUTF8String());
}

static VerificationResult VerifyStaticMethodName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STATIC_METHOD_NAME);
    return v.VerifyStaticMethodName(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindMethodSignature(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_METHOD_SIGNATURE);
    return v.VerifyFindMethodSignature(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindStaticMethodSignature(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_STATIC_METHOD_SIGNATURE);
    return v.VerifyFindStaticMethodSignature(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindFunctionName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_FUNCTION_NAME);
    return v.VerifyFindFunctionName(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindFunctionSignature(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_FUNCTION_SIGNATURE);
    return v.VerifyFindFunctionSignature(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindSetterName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_SETTER_NAME);
    return v.VerifyFindSetterName(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindGetterName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_GETTER_NAME);
    return v.VerifyFindGetterName(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindIndexableGetterSignature(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_INDEXABLE_GETTER_SIG);
    return v.VerifyFindIndexableGetterSignature(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindIndexableSetterSignature(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_INDEXABLE_SETTER_SIG);
    return v.VerifyFindIndexableSetterSignature(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindIterator(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_ITERATOR);
    return v.VerifyFindIterator(arg.GetValueClass());
}

static VerificationResult VerifyMethodReturnType(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD_RETURN_TYPE);
    return v.VerifyMethodReturnType(arg.GetReturnType());
}

static VerificationResult VerifySignature(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_SIGNATURE);
    return v.VerifySignature(arg.GetValueUTF8String());
}

static VerificationResult VerifyUTF16Buffer(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_UTF16_BUFFER);
    return v.VerifyTypePtr(arg.GetValueUTF16Buffer(), "uint16_t *");
}

static VerificationResult VerifyUTF16String(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_UTF16_STRING);
    return v.VerifyTypePtr(arg.GetValueUTF16String(), "const uint16_t *");
}

static VerificationResult VerifyArray(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ARRAY);
    return v.VerifyArray(arg.GetValueArray());
}

static VerificationResult VerifyArrayInitialRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ARRAY_INITIAL_REF);
    return v.VerifyArrayInitialRef(arg.GetValueRef());
}

static VerificationResult VerifyTupleValue(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_TUPLE_VALUE);
    return v.VerifyTupleValue(arg.GetValueTupleValue());
}

static VerificationResult VerifyTupleIndex(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_TUPLE_INDEX);
    return v.VerifyTupleIndex(arg.GetValueSize(), arg.GetReturnType());
}

static VerificationResult VerifyArrayBuffer(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ARRAYBUFFER);
    return v.VerifyArrayBuffer(arg.GetValueArrayBuffer());
}

static VerificationResult VerifyFixedArray(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY);
    return v.VerifyFixedArray(arg.GetValueFixedArray());
}

static VerificationResult VerifyFixedArrayBoolean(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_BOOLEAN);
    return v.VerifyPrimitivesFixedArray(arg.GetValueFixedArrayBoolean(), panda_file::Type::TypeId::U1);
}

static VerificationResult VerifyFixedArrayChar(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_CHAR);
    return v.VerifyPrimitivesFixedArray(arg.GetValueFixedArrayChar(), panda_file::Type::TypeId::U16);
}

static VerificationResult VerifyFixedArrayByte(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_BYTE);
    return v.VerifyPrimitivesFixedArray(arg.GetValueFixedArrayByte(), panda_file::Type::TypeId::I8);
}

static VerificationResult VerifyFixedArrayShort(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_SHORT);
    return v.VerifyPrimitivesFixedArray(arg.GetValueFixedArrayShort(), panda_file::Type::TypeId::I16);
}

static VerificationResult VerifyFixedArrayInt(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_INT);
    return v.VerifyPrimitivesFixedArray(arg.GetValueFixedArrayInt(), panda_file::Type::TypeId::I32);
}

static VerificationResult VerifyFixedArrayLong(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_LONG);
    return v.VerifyPrimitivesFixedArray(arg.GetValueFixedArrayLong(), panda_file::Type::TypeId::I64);
}

static VerificationResult VerifyFixedArrayFloat(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_FLOAT);
    return v.VerifyPrimitivesFixedArray(arg.GetValueFixedArrayFloat(), panda_file::Type::TypeId::F32);
}

static VerificationResult VerifyFixedArrayDouble(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_DOUBLE);
    return v.VerifyPrimitivesFixedArray(arg.GetValueFixedArrayDouble(), panda_file::Type::TypeId::F64);
}

static VerificationResult VerifyFixedArrayRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_REF);
    return v.VerifyFixedArrayRef(arg.GetValueFixedArrayRef());
}

static VerificationResult VerifyFixedArrayInitialRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_INITIAL_REF);
    return v.VerifyFixedArrayInitialRef(arg.GetValueRef());
}

static VerificationResult VerifyFixedArraySetRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_SET_REF);
    return v.VerifyFixedArraySetRef(arg.GetValueRef());
}

static VerificationResult VerifyRegionBuffer(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_REGION_BUFFER);
    return v.VerifyRegionBuffer(arg.GetValueConstVoidPtr(), arg.GetNativeFunctionCount());
}

static VerificationResult VerifyDelLocalRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_DEL_LOCAL_REF);
    return v.VerifyDelLocalRef(arg.GetValueRef());
}

static VerificationResult VerifyThisObject(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_THIS_OBJECT);
    return v.VerifyThisObject(arg.GetValueObject());
}

static VerificationResult VerifyBoxedPrimitiveObject(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_BOXED_PRIMITIVE_OBJECT);
    return v.VerifyBoxedPrimitiveObject(arg.GetValueObject(), arg.GetReturnType());
}

static VerificationResult VerifyCtor(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_CTOR);
    return v.VerifyCtor(arg.GetValueMethod(), arg.GetReturnType());
}

static VerificationResult VerifyReadField(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_READ_FIELD);
    return v.VerifyReadField(arg.GetValueField(), arg.GetReturnType());
}

static VerificationResult VerifyReadFieldByName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_READ_FIELD_BY_NAME);
    return v.VerifyReadFieldByName(arg.GetValueUTF8String(), arg.GetReturnType());
}

static VerificationResult VerifyReadStaticFieldByName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_READ_STATIC_FIELD_BY_NAME);
    return v.VerifyReadStaticFieldByName(arg.GetValueUTF8String(), arg.GetReturnType());
}

static VerificationResult VerifyReadStaticField(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_READ_STATIC_FIELD);
    return v.VerifyReadStaticField(arg.GetValueStaticField(), arg.GetReturnType());
}

static VerificationResult VerifyWriteField(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_WRITE_FIELD);
    return v.VerifyWriteField(arg.GetValueField(), arg.GetReturnType());
}

static VerificationResult VerifyWriteFieldByName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_WRITE_FIELD_BY_NAME);
    return v.VerifyWriteFieldByName(arg.GetValueUTF8String(), arg.GetReturnType());
}

static VerificationResult VerifyWriteStaticFieldByName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_WRITE_STATIC_FIELD_BY_NAME);
    return v.VerifyWriteStaticFieldByName(arg.GetValueUTF8String(), arg.GetReturnType());
}

static VerificationResult VerifyWriteStaticField(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_WRITE_STATIC_FIELD);
    return v.VerifyWriteStaticField(arg.GetValueStaticField(), arg.GetReturnType());
}

static VerificationResult VerifyFindFieldName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_FIELD_NAME);
    return v.VerifyFindFieldName(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindStaticFieldName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_STATIC_FIELD_NAME);
    return v.VerifyFindStaticFieldName(arg.GetValueUTF8String());
}

static VerificationResult VerifyFindVariableName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIND_VARIABLE_NAME);
    return v.VerifyFindVariableName(arg.GetValueUTF8String());
}

static VerificationResult VerifyReadPropertyByName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_READ_PROPERTY_BY_NAME);
    return v.VerifyReadPropertyByName(arg.GetValueUTF8String(), arg.GetReturnType());
}

static VerificationResult VerifyWritePropertyByName(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_WRITE_PROPERTY_BY_NAME);
    return v.VerifyWritePropertyByName(arg.GetValueUTF8String(), arg.GetReturnType());
}

static VerificationResult VerifyReadVariable(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_READ_VARIABLE);
    return v.VerifyReadVariable(arg.GetValueVariable(), arg.GetReturnType());
}

static VerificationResult VerifyWriteVariable(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_WRITE_VARIABLE);
    return v.VerifyWriteVariable(arg.GetValueVariable(), arg.GetReturnType());
}

static VerificationResult VerifyMethod(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD);
    return v.VerifyMethod(arg.GetValueMethod(), arg.GetReturnType());
}

static VerificationResult VerifyStaticMethod(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STATIC_METHOD);
    return v.VerifyStaticMethod(arg.GetValueStaticMethod(), arg.GetReturnType());
}

static VerificationResult VerifyFunction(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FUNCTION);
    return v.VerifyFunction(arg.GetValueFunction(), arg.GetReturnType());
}

static VerificationResult VerifyMethodAArgs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD_A_ARGS);
    return v.VerifyMethodAArgs(arg.GetValueMethodArgs());
}

static VerificationResult VerifyMethodVArgs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD_V_ARGS);
    return v.VerifyMethodVArgs(arg.GetValueMethodArgs());
}

static VerificationResult VerifyAArgs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_A_ARGS);
    return v.VerifyAArgs(arg.GetValueValueArgs());
}

static VerificationResult VerifyVvaArgs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_VVA_ARGS);
    return v.VerifyVvaArgs(arg.GetValueVvaArgs());
}

static VerificationResult VerifyVmStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_VM_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueVmStorage(), "ani_vm *");
}

static VerificationResult VerifyEnvStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENV_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueEnvStorage(), "ani_env *");
}

static VerificationResult VerifyMethodStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueMethodStorage(), "ani_method");
}

static VerificationResult VerifyStaticMethodStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STATIC_METHOD_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueStaticMethodStorage(), "ani_static_method");
}

static VerificationResult VerifyFunctionStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FUNCTION_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFunctionStorage(), "ani_function");
}

static VerificationResult VerifyModuleStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_MODULE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueModuleStorage(), "ani_module");
}

static VerificationResult VerifyNamespaceStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_NAMESPACE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueNamespaceStorage(), "ani_namespace");
}

static VerificationResult VerifyFieldStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIELD_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFieldStorage(), "ani_field");
}

static VerificationResult VerifyStaticFieldStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STATIC_FIELD_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueStaticFieldStorage(), "ani_static_field");
}

static VerificationResult VerifyVariableStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_VARIABLE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueVariableStorage(), "ani_variable");
}

static VerificationResult VerifyBooleanStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_BOOLEAN_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueBooleanStorage(), "ani_boolean");
}

static VerificationResult VerifyCharStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_CHAR_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueCharStorage(), "ani_char");
}

static VerificationResult VerifyByteStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_BYTE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueByteStorage(), "ani_byte");
}

static VerificationResult VerifyShortStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_SHORT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueShortStorage(), "ani_short");
}

static VerificationResult VerifyIntStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_INT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueIntStorage(), "ani_int");
}

static VerificationResult VerifyLongStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_LONG_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueLongStorage(), "ani_long");
}

static VerificationResult VerifyFloatStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FLOAT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFloatStorage(), "ani_float");
}

static VerificationResult VerifyDoubleStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_DOUBLE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueDoubleStorage(), "ani_double");
}

static VerificationResult VerifyRefStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_REF_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueRefStorage(), "ani_ref");
}

static VerificationResult VerifyEnumStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENUM_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueEnumStorage(), "ani_enum");
}

static VerificationResult VerifyEnumItemStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENUM_ITEM_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueEnumItemStorage(), "ani_enum_item");
}

static VerificationResult VerifyTypeStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_TYPE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueTypeStorage(), "ani_type");
}

static VerificationResult VerifyWRefStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_WREF_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueWRefStorage(), "ani_wref");
}

static VerificationResult VerifyStringStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STRING_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueStringStorage(), "ani_string");
}

static VerificationResult VerifySizeStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_SIZE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueSizeStorage(), "ani_size");
}

static VerificationResult VerifyVoidPtrStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_VOID_PTR_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueVoidPtrStorage(), "void *");
}

static VerificationResult VerifyArrayBufferStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ARRAYBUFFER_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueArrayBufferStorage(), "ani_arraybuffer");
}

static VerificationResult VerifyU32Storage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_U32_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueU32Storage(), "uint32_t");
}

static VerificationResult VerifySize([[maybe_unused]] Verifier &v, [[maybe_unused]] const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_SIZE);
    return {};
}

static VerificationResult VerifyArrayCreateLength(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ARRAY_CREATE_LENGTH);
    return v.VerifyArrayCreateLength(arg.GetValueSize());
}

static VerificationResult VerifyNativeFunctions(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_NATIVE_FUNCTIONS);
    return v.VerifyNativeFunctions(arg.GetValueNativeFunctions(), arg.GetNativeFunctionCount());
}

static VerificationResult VerifyNativeMethods(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_NATIVE_METHODS);
    return v.VerifyNativeMethods(arg.GetValueNativeFunctions(), arg.GetNativeFunctionCount());
}

static VerificationResult VerifyStaticNativeMethods(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STATIC_NATIVE_METHODS);
    return v.VerifyStaticNativeMethods(arg.GetValueNativeFunctions(), arg.GetNativeFunctionCount());
}

static VerificationResult VerifyPrimitiveValue(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_PRIMITIVE_VALUE);
    if (arg.IsBooleanValue()) {
        return v.VerifyBoolean(arg.GetValueBoolean());
    }
    return {};
}

static VerificationResult VerifyArrayBufferLength(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ARRAYBUFFER_LENGTH);
    return v.VerifyArrayBufferLength(arg.GetValueSize());
}

static VerificationResult VerifyObjectStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_OBJECT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueObjectStorage(), "ani_object");
}

static VerificationResult VerifyClassStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_CLASS_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueClassStorage(), "ani_class");
}

static VerificationResult VerifyErrorStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ERROR_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueErrorStorage(), "ani_error");
}

static VerificationResult VerifyArrayStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ARRAY_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueArrayStorage(), "ani_array");
}

static VerificationResult VerifyFixedArrayBooleanStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_BOOLEAN_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayBooleanStorage(), "ani_fixedarray_boolean");
}

static VerificationResult VerifyFixedArrayCharStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_CHAR_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayCharStorage(), "ani_fixedarray_char");
}

static VerificationResult VerifyFixedArrayByteStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_BYTE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayByteStorage(), "ani_fixedarray_byte");
}

static VerificationResult VerifyFixedArrayShortStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_SHORT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayShortStorage(), "ani_fixedarray_short");
}

static VerificationResult VerifyFixedArrayIntStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_INT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayIntStorage(), "ani_fixedarray_int");
}

static VerificationResult VerifyFixedArrayLongStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_LONG_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayLongStorage(), "ani_fixedarray_long");
}

static VerificationResult VerifyFixedArrayFloatStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_FLOAT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayFloatStorage(), "ani_fixedarray_float");
}

static VerificationResult VerifyFixedArrayDoubleStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_DOUBLE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayDoubleStorage(), "ani_fixedarray_double");
}

static VerificationResult VerifyFixedArrayRefStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_REF_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayRefStorage(), "ani_fixedarray_ref");
}

static VerificationResult VerifyResolver(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_RESOLVER);
    return v.VerifyResolver(arg.GetValueResolver());
}

static VerificationResult VerifyResolverStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_RESOLVER_STORAGE);
    return v.VerifyTypeStorage<VResolver **>(arg.GetValueResolverStorage(), "ani_resolver");
}

static VerificationResult VerifyRefCallArgs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_REF_CALL_ARGS);
    return v.VerifyRefCallArgs(arg.GetValueRefCallArgs());
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
static constexpr std::array<CheckerHandler, helpers::ToUnderlying(ANIArg::Action::NUMBER_OF_ELEMENTS)> HANDLERS = {
// CC-OFFNXT(G.PRE.02) code generation
#define X(ACTION, FN) FN,
    ANI_VERIFICATION_MAP
#undef X
};
// NOLINTEND(cppcoreguidelines-macro-usage)

static void DoReportANI(PandaEtsVM *etsVm, std::string_view functionName, std::string_view message, bool isFatal = true,
                        bool appendSeverity = false)
{
    PandaStringStream ss;
    ss << "DETECT AN ERROR WHEN USING ANI:";
    ss << "\n  ANI method: " << functionName;
    ss << "\n" << message;
    if (appendSeverity) {
        ss << " [" << SeverityToString(isFatal ? ANIErrorSeverity::FATAL : ANIErrorSeverity::ERROR) << "]";
    }
    etsVm->GetANIVerifier()->Report(ss.str(), isFatal);
}

struct ArgInfo {
    ANIArg arg;
    std::optional<PandaString> err;
    ANIErrorSeverity severity = ANIErrorSeverity::NONE;
};

struct ArgsInfo {
    PandaVector<ArgInfo> argInfoList;
    std::optional<PandaVector<ExtArgInfo>> extArgInfoList;
};

static PandaVector<ExtArgInfo> MakeExtArgInfoList(PandaAniEnv *pandaEnv, const ANIArg::AniMethodArgs *methodArgs)
{
    if (methodArgs == nullptr || methodArgs->method == nullptr || methodArgs->vargs == nullptr) {
        return {};
    }

    auto getName = [](size_t index) -> PandaString {
        PandaStringStream ss;
        ss << '[' << index << ']';
        return ss.str();
    };

    PandaVector<ExtArgInfo> extArgInfoList;
    size_t i = 0;

    CallArgs callArgs(methodArgs->method, methodArgs->vargs);
    auto envANIVerifier = pandaEnv->GetEnvANIVerifier();
    callArgs.ForEachArgs([&](ani_value value, panda_file::Type type, size_t refIndex) -> bool {
        PandaString name = getName(i++);
        bool isValid = IsValidRawAniValue(envANIVerifier, value, type, methodArgs->isVaArgs);
        if (isValid && type.IsReference()) {
            isValid =
                IsValidMethodArgRefType(pandaEnv, methodArgs->method, reinterpret_cast<VRef *>(value.r), refIndex);
        }
        extArgInfoList.emplace_back(ExtArgInfo {std::move(name), value.l, type, isValid});
        return true;
    });
    return extArgInfoList;
}

std::optional<PandaVector<ExtArgInfo>> Verifier::GetExtArgInfoListForResolvedArgs(PandaAniEnv *pandaEnv) const
{
    if (!methodArgsForExtInfo_.has_value()) {
        return {};
    }
    return MakeExtArgInfoList(pandaEnv, &methodArgsForExtInfo_.value());
}

static PandaString GetAniTypeByType(panda_file::Type type)
{
    // clang-format off
    switch (type.GetId()) {
        case panda_file::Type::TypeId::U1:        return "ani_boolean";
        case panda_file::Type::TypeId::U16:       return "ani_char";
        case panda_file::Type::TypeId::I8:        return "ani_byte";
        case panda_file::Type::TypeId::I16:       return "ani_short";
        case panda_file::Type::TypeId::I32:       return "ani_int";
        case panda_file::Type::TypeId::I64:       return "ani_long";
        case panda_file::Type::TypeId::F32:       return "ani_float";
        case panda_file::Type::TypeId::F64:       return "ani_double";
        case panda_file::Type::TypeId::REFERENCE: return "ani_ref";
        case panda_file::Type::TypeId::NOVALUE: return "novalue";
        default: UNREACHABLE();
    }
    // clang-format on
    UNREACHABLE();
}

class ReportData final {
private:
    struct MsgArgInfo {
        PandaString name;
        PandaString value;
        PandaString type;
        PandaString error;
    };

public:
    void AddItem(std::string_view name, PandaString value, PandaString type, PandaString error)
    {
        maxNameSize_ = std::max(maxNameSize_, name.size());
        maxValueSize_ = std::max(maxValueSize_, value.size());
        maxTypeSize_ = std::max(maxTypeSize_, type.size());
        items_.push_back({PandaString(name), std::move(value), std::move(type), std::move(error)});
    }

    PandaString ToString() const
    {
        if (items_.empty()) {
            return "";
        }

        PandaStringStream ss;
        ss << std::setfill(' ');
        for (size_t i = 0; i < items_.size(); ++i) {
            const auto &item = items_[i];
            if (i > 0) {
                ss << "\n";
            }

            ss << "    " << std::right << std::setw(maxNameSize_) << item.name << ": " << std::left
               << std::setw(maxValueSize_) << item.value << " | " << std::setw(maxTypeSize_) << item.type << " | "
               << item.error;
        }
        return ss.str();
    }

    bool Empty() const
    {
        return items_.empty();
    }

private:
    PandaVector<MsgArgInfo> items_;
    size_t maxNameSize_ = 0;
    size_t maxValueSize_ = 0;
    size_t maxTypeSize_ = 0;
};

static PandaString FormatInvalidError(const PandaString &error, ANIErrorSeverity severity)
{
    ASSERT(severity == ANIErrorSeverity::ERROR || severity == ANIErrorSeverity::FATAL);

    PandaStringStream ss;
    ss << "INVALID: " << error << " [" << SeverityToString(severity) << "]";
    return ss.str();
}

static void ProcessStandardArgs(const PandaVector<ArgInfo> &argInfoList, ReportData &data)
{
    for (const auto &v : argInfoList) {
        PandaString error = v.err ? FormatInvalidError(v.err.value(), v.severity) : "VALID";
        data.AddItem(v.arg.GetName(), v.arg.GetStringValue(), v.arg.GetStringType(), std::move(error));
    }
}

static void ProcessExtArgs(const PandaVector<ExtArgInfo> &extArgInfoList, ReportData &data, ANIErrorSeverity severity)
{
    for (const auto &v : extArgInfoList) {
        PandaString error = v.isValid ? "VALID" : FormatInvalidError("wrong value", severity);
        PandaStringStream ssValue;
        ssValue << "0x" << std::hex << v.value;
        data.AddItem(v.name, ssValue.str(), GetAniTypeByType(v.type), std::move(error));
    }
}

static void DoANIArgsReport(PandaEtsVM *etsVm, std::string_view functionName, const ArgsInfo &argsInfo, bool isFatal)
{
    ReportData data;
    ProcessStandardArgs(argsInfo.argInfoList, data);
    if (argsInfo.extArgInfoList.has_value()) {
        ProcessExtArgs(argsInfo.extArgInfoList.value(), data,
                       isFatal ? ANIErrorSeverity::FATAL : ANIErrorSeverity::ERROR);
    }
    if (!data.Empty()) {
        DoReportANI(etsVm, functionName, data.ToString(), isFatal);
    }
}

// CC-OFFNXT(G.NAM.03) false positive
bool VerifyANIArgs(std::string_view functionName, std::initializer_list<ANIArg> args)
{
    VVm *vvm = VVm::GetInstance();
    VEnv *venv = VEnv::GetCurrent();
    auto etsVm = PandaEtsVM::FromAniVM(vvm->GetVm());

    PandaVector<ArgInfo> argInfoList;
    Verifier verifier(vvm, venv);
    bool hasAnyError = false;
    bool hasFatalError = false;

    for (const ANIArg &arg : args) {
        auto id = helpers::ToUnderlying(arg.GetAction());
        ASSERT(id < HANDLERS.size());
        CheckerHandler handler = HANDLERS[id];
        ASSERT(handler != nullptr);
        auto err = handler(verifier, arg);
        ANIErrorSeverity severity = err.GetSeverity();
        if (err) {
            hasAnyError = true;
            if (severity == ANIErrorSeverity::FATAL) {
                hasFatalError = true;
            }
        }

        argInfoList.emplace_back(ArgInfo {arg, err.TakeError(), severity});
    }

    if (!hasAnyError) {
        return true;
    }

    const ArgInfo &lastArgInfo = argInfoList.back();
    auto action = lastArgInfo.arg.GetAction();
    ArgsInfo argsInfo {};
    if (venv != nullptr &&
        (action == ANIArg::Action::VERIFY_METHOD_V_ARGS || action == ANIArg::Action::VERIFY_METHOD_A_ARGS)) {
        auto *methodArgs = lastArgInfo.arg.GetValueMethodArgs();
        auto *pandaEnv = PandaAniEnv::FromAniEnv(venv->GetEnv());
        argsInfo.extArgInfoList = MakeExtArgInfoList(pandaEnv, methodArgs);
    } else if (venv != nullptr &&
               (action == ANIArg::Action::VERIFY_VVA_ARGS || action == ANIArg::Action::VERIFY_A_ARGS)) {
        auto *pandaEnv = PandaAniEnv::FromAniEnv(venv->GetEnv());
        argsInfo.extArgInfoList = verifier.GetExtArgInfoListForResolvedArgs(pandaEnv);
    }
    argsInfo.argInfoList = std::move(argInfoList);
    bool isFatal = hasFatalError;
    DoANIArgsReport(etsVm, functionName, argsInfo, isFatal);
    return !isFatal;
}

void VerifyReportANI(std::string_view functionName, std::string_view message)
{
    PandaEtsVM *etsVm = PandaEtsVM::GetCurrent();
    DoReportANI(etsVm, functionName, "    " + std::string(message), true, true);
}
}  // namespace ark::ets::ani::verify
