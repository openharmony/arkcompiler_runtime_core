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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H

#include "plugins/ets/runtime/ani/ani.h"
#include "runtime/include/mem/panda_containers.h"
#include "plugins/ets/runtime/types/ets_type.h"
#include "runtime/include/mem/panda_string.h"

// clang-format off
// NOLINTBEGIN(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-vararg)
// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define ANI_VERIFICATION_MAP                                                      \
    X(VERIFY_VM,                            VerifyVm)                             \
    X(VERIFY_ENV,                           VerifyEnv)                            \
    X(VERIFY_ENV_SKIP_PENDING_ERROR,        VerifyEnvSkipPendingError)            \
    X(VERIFY_OPTIONS,                       VerifyOptions)                        \
    X(VERIFY_ENV_VERSION,                   VerifyEnvVersion)                     \
    X(VERIFY_NR_REFS,                       VerifyNrRefs)                         \
    X(VERIFY_REF,                           VerifyRef)                            \
    X(VERIFY_MODULE,                        VerifyModule)                         \
    X(VERIFY_NAMESPACE,                     VerifyNamespace)                      \
    X(VERIFY_TYPE,                          VerifyType)                           \
    X(VERIFY_GLOBAL_REF,                    VerifyGlobalRef)                      \
    X(VERIFY_WREF,                          VerifyWRef)                           \
    X(VERIFY_FUNCTIONAL_OBJECT,             VerifyFunctionalObject)               \
    X(VERIFY_FUNCTIONAL_OBJECT_ARGV,        VerifyFunctionalObjectArgv)           \
    X(VERIFY_CLASS,                         VerifyClass)                          \
    X(VERIFY_ENUM,                          VerifyEnum)                           \
    X(VERIFY_ENUM_ITEM,                     VerifyEnumItem)                       \
    X(VERIFY_STRING,                        VerifyString)                         \
    X(VERIFY_ERROR,                         VerifyError)                          \
    X(VERIFY_ARRAY,                         VerifyArray)                          \
    X(VERIFY_TUPLE_VALUE,                   VerifyTupleValue)                     \
    X(VERIFY_TUPLE_INDEX,                   VerifyTupleIndex)                     \
    X(VERIFY_ARRAYBUFFER,                   VerifyArrayBuffer)                    \
    X(VERIFY_FIXED_ARRAY,                   VerifyFixedArray)                     \
    X(VERIFY_FIXED_ARRAY_BOOLEAN,           VerifyFixedArrayBoolean)              \
    X(VERIFY_FIXED_ARRAY_CHAR,              VerifyFixedArrayChar)                 \
    X(VERIFY_FIXED_ARRAY_BYTE,              VerifyFixedArrayByte)                 \
    X(VERIFY_FIXED_ARRAY_SHORT,             VerifyFixedArrayShort)                \
    X(VERIFY_FIXED_ARRAY_INT,               VerifyFixedArrayInt)                  \
    X(VERIFY_FIXED_ARRAY_LONG,              VerifyFixedArrayLong)                 \
    X(VERIFY_FIXED_ARRAY_FLOAT,             VerifyFixedArrayFloat)                \
    X(VERIFY_FIXED_ARRAY_DOUBLE,            VerifyFixedArrayDouble)               \
    X(VERIFY_FIXED_ARRAY_REF,               VerifyFixedArrayRef)                  \
    X(VERIFY_FIXED_ARRAY_INITIAL_REF,       VerifyFixedArrayInitialRef)           \
    X(VERIFY_FIXED_ARRAY_SET_REF,           VerifyFixedArraySetRef)               \
    X(VERIFY_REGION_BUFFER,                 VerifyRegionBuffer)                   \
    X(VERIFY_DEL_LOCAL_REF,                 VerifyDelLocalRef)                    \
    X(VERIFY_THIS_OBJECT,                   VerifyThisObject)                     \
    X(VERIFY_BOXED_PRIMITIVE_OBJECT,        VerifyBoxedPrimitiveObject)           \
    X(VERIFY_CTOR,                          VerifyCtor)                           \
    X(VERIFY_READ_FIELD,                    VerifyReadField)                      \
    X(VERIFY_READ_FIELD_BY_NAME,            VerifyReadFieldByName)                \
    X(VERIFY_FIND_FIELD_NAME,               VerifyFindFieldName)                  \
    X(VERIFY_FIND_STATIC_FIELD_NAME,        VerifyFindStaticFieldName)            \
    X(VERIFY_FIND_VARIABLE_NAME,            VerifyFindVariableName)               \
    X(VERIFY_READ_STATIC_FIELD_BY_NAME,     VerifyReadStaticFieldByName)          \
    X(VERIFY_READ_STATIC_FIELD,             VerifyReadStaticField)                \
    X(VERIFY_WRITE_FIELD,                   VerifyWriteField)                     \
    X(VERIFY_WRITE_FIELD_BY_NAME,           VerifyWriteFieldByName)               \
    X(VERIFY_WRITE_STATIC_FIELD_BY_NAME,    VerifyWriteStaticFieldByName)         \
    X(VERIFY_WRITE_STATIC_FIELD,            VerifyWriteStaticField)               \
    X(VERIFY_READ_PROPERTY_BY_NAME,         VerifyReadPropertyByName)             \
    X(VERIFY_WRITE_PROPERTY_BY_NAME,        VerifyWritePropertyByName)            \
    X(VERIFY_READ_VARIABLE,                 VerifyReadVariable)                   \
    X(VERIFY_WRITE_VARIABLE,                VerifyWriteVariable)                  \
    X(VERIFY_METHOD,                        VerifyMethod)                         \
    X(VERIFY_STATIC_METHOD,                 VerifyStaticMethod)                   \
    X(VERIFY_FUNCTION,                      VerifyFunction)                       \
    X(VERIFY_METHOD_A_ARGS,                 VerifyMethodAArgs)                    \
    X(VERIFY_METHOD_V_ARGS,                 VerifyMethodVArgs)                    \
    X(VERIFY_A_ARGS,                        VerifyAArgs)                          \
    X(VERIFY_VVA_ARGS,                      VerifyVvaArgs)                        \
    X(VERIFY_VM_STORAGE,                    VerifyVmStorage)                      \
    X(VERIFY_ENV_STORAGE,                   VerifyEnvStorage)                     \
    X(VERIFY_METHOD_STORAGE,                VerifyMethodStorage)                  \
    X(VERIFY_STATIC_METHOD_STORAGE,         VerifyStaticMethodStorage)            \
    X(VERIFY_FUNCTION_STORAGE,              VerifyFunctionStorage)                \
    X(VERIFY_MODULE_STORAGE,                VerifyModuleStorage)                  \
    X(VERIFY_NAMESPACE_STORAGE,             VerifyNamespaceStorage)               \
    X(VERIFY_FIELD_STORAGE,                 VerifyFieldStorage)                   \
    X(VERIFY_STATIC_FIELD_STORAGE,          VerifyStaticFieldStorage)             \
    X(VERIFY_VARIABLE_STORAGE,              VerifyVariableStorage)                \
    X(VERIFY_BOOLEAN_STORAGE,               VerifyBooleanStorage)                 \
    X(VERIFY_CHAR_STORAGE,                  VerifyCharStorage)                    \
    X(VERIFY_BYTE_STORAGE,                  VerifyByteStorage)                    \
    X(VERIFY_SHORT_STORAGE,                 VerifyShortStorage)                   \
    X(VERIFY_INT_STORAGE,                   VerifyIntStorage)                     \
    X(VERIFY_LONG_STORAGE,                  VerifyLongStorage)                    \
    X(VERIFY_FLOAT_STORAGE,                 VerifyFloatStorage)                   \
    X(VERIFY_DOUBLE_STORAGE,                VerifyDoubleStorage)                  \
    X(VERIFY_ENUM_STORAGE,                  VerifyEnumStorage)                    \
    X(VERIFY_ENUM_ITEM_STORAGE,             VerifyEnumItemStorage)                \
    X(VERIFY_REF_STORAGE,                   VerifyRefStorage)                     \
    X(VERIFY_TYPE_STORAGE,                  VerifyTypeStorage)                    \
    X(VERIFY_WREF_STORAGE,                  VerifyWRefStorage)                    \
    X(VERIFY_OBJECT_STORAGE,                VerifyObjectStorage)                  \
    X(VERIFY_CLASS_STORAGE,                 VerifyClassStorage)                   \
    X(VERIFY_STRING_STORAGE,                VerifyStringStorage)                  \
    X(VERIFY_SIZE_STORAGE,                  VerifySizeStorage)                    \
    X(VERIFY_VOID_PTR_STORAGE,              VerifyVoidPtrStorage)                 \
    X(VERIFY_ARRAYBUFFER_STORAGE,           VerifyArrayBufferStorage)             \
    X(VERIFY_U32_STORAGE,                   VerifyU32Storage)                     \
    X(VERIFY_PRIMITIVE_VALUE,               VerifyPrimitiveValue)                 \
    X(VERIFY_ARRAYBUFFER_LENGTH,            VerifyArrayBufferLength)              \
    X(VERIFY_FIXED_ARRAY_LENGTH,            VerifyFixedArrayLength)               \
    X(VERIFY_NATIVE_FUNCTIONS,              VerifyNativeFunctions)                \
    X(VERIFY_NATIVE_METHODS,                VerifyNativeMethods)                  \
    X(VERIFY_STATIC_NATIVE_METHODS,         VerifyStaticNativeMethods)            \
    X(VERIFY_UTF16_BUFFER,                  VerifyUTF16Buffer)                    \
    X(VERIFY_UTF16_STRING,                  VerifyUTF16String)                    \
    X(VERIFY_UTF8_BUFFER,                   VerifyUTF8Buffer)                     \
    X(VERIFY_UTF8_STRING,                   VerifyUTF8String)                     \
    X(VERIFY_MODULE_DESCRIPTOR,             VerifyModuleDescriptor)               \
    X(VERIFY_NAMESPACE_DESCRIPTOR,          VerifyNamespaceDescriptor)            \
    X(VERIFY_CLASS_DESCRIPTOR,              VerifyClassDescriptor)                \
    X(VERIFY_ENUM_DESCRIPTOR,               VerifyEnumDescriptor)                 \
    X(VERIFY_METHOD_NAME,                   VerifyMethodName)                     \
    X(VERIFY_STATIC_METHOD_NAME,            VerifyStaticMethodName)               \
    X(VERIFY_FIND_METHOD_SIGNATURE,         VerifyFindMethodSignature)            \
    X(VERIFY_FIND_STATIC_METHOD_SIGNATURE,  VerifyFindStaticMethodSignature)      \
    X(VERIFY_FIND_FUNCTION_NAME,            VerifyFindFunctionName)               \
    X(VERIFY_FIND_FUNCTION_SIGNATURE,       VerifyFindFunctionSignature)          \
    X(VERIFY_FIND_SETTER_NAME,              VerifyFindSetterName)                 \
    X(VERIFY_FIND_GETTER_NAME,              VerifyFindGetterName)                 \
    X(VERIFY_FIND_INDEXABLE_GETTER_SIG,     VerifyFindIndexableGetterSignature)   \
    X(VERIFY_FIND_INDEXABLE_SETTER_SIG,     VerifyFindIndexableSetterSignature)   \
    X(VERIFY_FIND_ITERATOR,                 VerifyFindIterator)                   \
    X(VERIFY_METHOD_RETURN_TYPE,            VerifyMethodReturnType)               \
    X(VERIFY_SIGNATURE,                     VerifySignature)                      \
    X(VERIFY_ARRAY_INDEX,                   VerifyArrayIndex)                     \
    X(VERIFY_SIZE,                          VerifySize)                           \
    X(VERIFY_RESOLVER,                      VerifyResolver)                       \
    X(VERIFY_ERROR_STORAGE,                 VerifyErrorStorage)                   \
    X(VERIFY_ARRAY_STORAGE,                 VerifyArrayStorage)                   \
    X(VERIFY_FIXED_ARRAY_BOOLEAN_STORAGE,   VerifyFixedArrayBooleanStorage)       \
    X(VERIFY_FIXED_ARRAY_CHAR_STORAGE,      VerifyFixedArrayCharStorage)          \
    X(VERIFY_FIXED_ARRAY_BYTE_STORAGE,      VerifyFixedArrayByteStorage)          \
    X(VERIFY_FIXED_ARRAY_SHORT_STORAGE,     VerifyFixedArrayShortStorage)         \
    X(VERIFY_FIXED_ARRAY_INT_STORAGE,       VerifyFixedArrayIntStorage)           \
    X(VERIFY_FIXED_ARRAY_LONG_STORAGE,      VerifyFixedArrayLongStorage)          \
    X(VERIFY_FIXED_ARRAY_FLOAT_STORAGE,     VerifyFixedArrayFloatStorage)         \
    X(VERIFY_FIXED_ARRAY_DOUBLE_STORAGE,    VerifyFixedArrayDoubleStorage)        \
    X(VERIFY_FIXED_ARRAY_REF_STORAGE,       VerifyFixedArrayRefStorage)           \
    X(VERIFY_RESOLVER_STORAGE,              VerifyResolverStorage)                \
    X(VERIFY_ANY_REF,                       VerifyAnyRef)                         \
    X(VERIFY_REF_CALL_ARGS,                 VerifyRefCallArgs)                    \

// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define ANI_ARG_TYPES_MAP                                                                    \
    X(ANI_VM,                           Vm,                        VVm *)                    \
    X(ANI_ENV,                          Env,                       VEnv *)                   \
    X(ANI_OPTIONS,                      Options,                   const ani_options *)      \
    X(ANI_SIZE,                         Size,                      ani_size)                 \
    X(ANI_BOOLEAN,                      Boolean,                   ani_boolean)              \
    X(ANI_CHAR,                         Char,                      ani_char)                 \
    X(ANI_BYTE,                         Byte,                      ani_byte)                 \
    X(ANI_SHORT,                        Short,                     ani_short)                \
    X(ANI_INT,                          Int,                       ani_int)                  \
    X(ANI_LONG,                         Long,                      ani_long)                 \
    X(ANI_FLOAT,                        Float,                     ani_float)                \
    X(ANI_DOUBLE,                       Double,                    ani_double)               \
    X(ANI_REF,                          Ref,                       VRef *)                   \
    X(FUNCTIONAL_OBJECT_ARGV,           FunctionalObjectArgv,      AniFunctionalObjectArgv *) \
    X(ANI_FN_OBJECT,                    FnObject,                  VFnObject *)              \
    X(ANI_MODULE,                       Module,                    VModule *)                \
    X(ANI_NAMESPACE,                    Namespace,                 VNamespace *)             \
    X(ANI_TYPE,                         Type,                      VType *)                  \
    X(ANI_WREF,                         WRef,                      ani_wref)                 \
    X(ANI_CLASS,                        Class,                     VClass *)                 \
    X(ANI_ENUM,                         Enum,                      VEnum *)                  \
    X(ANI_ENUM_ITEM,                    EnumItem,                  VEnumItem *)              \
    X(ANI_OBJECT,                       Object,                    VObject *)                \
    X(ANI_TUPLE_VALUE,                  TupleValue,                VTupleValue *)            \
    X(ANI_METHOD,                       Method,                    VMethod *)                \
    X(ANI_STATIC_METHOD,                StaticMethod,              VStaticMethod *)          \
    X(ANI_FUNCTION,                     Function,                  VFunction *)              \
    X(ANI_FIELD,                        Field,                     VField *)                 \
    X(ANI_STATIC_FIELD,                 StaticField,               VStaticField *)           \
    X(ANI_VARIABLE,                     Variable,                  VVariable *)              \
    X(ANI_STRING,                       String,                    VString *)                \
    X(ANI_ERROR,                        Error,                     VError *)                 \
    X(ANI_ARRAY,                        Array,                     VArray *)                 \
    X(ANI_ARRAYBUFFER,                  ArrayBuffer,               VArrayBuffer *)           \
    X(ANI_FIXED_ARRAY,                  FixedArray,                VFixedArray *)            \
    X(ANI_FIXED_ARRAY_BOOLEAN,          FixedArrayBoolean,         VFixedArrayBoolean *)     \
    X(ANI_FIXED_ARRAY_CHAR,             FixedArrayChar,            VFixedArrayChar *)        \
    X(ANI_FIXED_ARRAY_BYTE,             FixedArrayByte,            VFixedArrayByte *)        \
    X(ANI_FIXED_ARRAY_SHORT,            FixedArrayShort,           VFixedArrayShort *)       \
    X(ANI_FIXED_ARRAY_INT,              FixedArrayInt,             VFixedArrayInt *)         \
    X(ANI_FIXED_ARRAY_LONG,             FixedArrayLong,            VFixedArrayLong *)        \
    X(ANI_FIXED_ARRAY_FLOAT,            FixedArrayFloat,           VFixedArrayFloat *)       \
    X(ANI_FIXED_ARRAY_DOUBLE,           FixedArrayDouble,          VFixedArrayDouble *)      \
    X(ANI_FIXED_ARRAY_REF,              FixedArrayRef,             VFixedArrayRef *)         \
    X(ANI_VALUE_ARGS,                   ValueArgs,                 const ani_value *)        \
    X(ANI_NATIVE_FUNCTIONS,             NativeFunctions,           const ani_native_function *) \
    X(ANI_ENV_STORAGE,                  EnvStorage,                VEnv **)                  \
    X(ANI_VM_STORAGE,                   VmStorage,                 VVm **)                   \
    X(ANI_METHOD_STORAGE,               MethodStorage,             VMethod **)               \
    X(ANI_STATIC_METHOD_STORAGE,        StaticMethodStorage,       VStaticMethod **)         \
    X(ANI_FUNCTION_STORAGE,             FunctionStorage,           VFunction **)             \
    X(ANI_MODULE_STORAGE,               ModuleStorage,             VModule **)               \
    X(ANI_NAMESPACE_STORAGE,            NamespaceStorage,          VNamespace **)            \
    X(ANI_FIELD_STORAGE,                FieldStorage,              VField **)                \
    X(ANI_STATIC_FIELD_STORAGE,         StaticFieldStorage,        VStaticField **)          \
    X(ANI_VARIABLE_STORAGE,             VariableStorage,           VVariable **)             \
    X(ANI_BOOLEAN_STORAGE,              BooleanStorage,            ani_boolean *)            \
    X(ANI_CHAR_STORAGE,                 CharStorage,               ani_char *)               \
    X(ANI_BYTE_STORAGE,                 ByteStorage,               ani_byte *)               \
    X(ANI_SHORT_STORAGE,                ShortStorage,              ani_short *)              \
    X(ANI_INT_STORAGE,                  IntStorage,                ani_int *)                \
    X(ANI_LONG_STORAGE,                 LongStorage,               ani_long *)               \
    X(ANI_FLOAT_STORAGE,                FloatStorage,              ani_float *)              \
    X(ANI_DOUBLE_STORAGE,               DoubleStorage,             ani_double *)             \
    X(ANI_REF_STORAGE,                  RefStorage,                VRef **)                  \
    X(ANI_TYPE_STORAGE,                 TypeStorage,               VType **)                 \
    X(ANI_CLASS_STORAGE,                ClassStorage,              VClass **)                \
    X(ANI_WREF_STORAGE,                 WRefStorage,               ani_wref *)               \
    X(ANI_OBJECT_STORAGE,               ObjectStorage,             VObject **)               \
    X(ANI_ENUM_STORAGE,                 EnumStorage,               VEnum **)                 \
    X(ANI_ENUM_ITEM_STORAGE,            EnumItemStorage,           VEnumItem **)             \
    X(ANI_STRING_STORAGE,               StringStorage,             VString **)               \
    X(ANI_SIZE_STORAGE,                 SizeStorage,               ani_size *)               \
    X(VOID_PTR_STORAGE,                 VoidPtrStorage,            void **)                  \
    X(ANI_ARRAYBUFFER_STORAGE,          ArrayBufferStorage,        VArrayBuffer **)          \
    X(UINT32_STORAGE,                   U32Storage,                uint32_t *)               \
    X(ANI_UTF8_BUFFER,                  UTF8Buffer,                char *)                   \
    X(ANI_UTF16_BUFFER,                 UTF16Buffer,               uint16_t *)               \
    X(ANI_UTF8_STRING,                  UTF8String,                const char *)             \
    X(ANI_UTF16_STRING,                 UTF16String,               const uint16_t *)         \
    X(ANI_ERROR_STORAGE,                ErrorStorage,              VError **)                \
    X(UINT32,                           U32,                       uint32_t)                 \
    X(METHOD_ARGS,                      MethodArgs,                AniMethodArgs *)          \
    X(ANI_VVA_ARGS,                     VvaArgs,                   va_list *)                \
    X(ANI_ARRAY_STORAGE,                ArrayStorage,              VArray **)                \
    X(ANI_FIXED_ARRAY_BOOLEAN_STORAGE,  FixedArrayBooleanStorage,  VFixedArrayBoolean **)    \
    X(ANI_FIXED_ARRAY_CHAR_STORAGE,     FixedArrayCharStorage,     VFixedArrayChar **)       \
    X(ANI_FIXED_ARRAY_BYTE_STORAGE,     FixedArrayByteStorage,     VFixedArrayByte **)       \
    X(ANI_FIXED_ARRAY_SHORT_STORAGE,    FixedArrayShortStorage,    VFixedArrayShort **)      \
    X(ANI_FIXED_ARRAY_INT_STORAGE,      FixedArrayIntStorage,      VFixedArrayInt **)        \
    X(ANI_FIXED_ARRAY_LONG_STORAGE,     FixedArrayLongStorage,     VFixedArrayLong **)       \
    X(ANI_FIXED_ARRAY_FLOAT_STORAGE,    FixedArrayFloatStorage,    VFixedArrayFloat **)      \
    X(ANI_FIXED_ARRAY_DOUBLE_STORAGE,   FixedArrayDoubleStorage,   VFixedArrayDouble **)     \
    X(ANI_FIXED_ARRAY_REF_STORAGE,      FixedArrayRefStorage,      VFixedArrayRef **)        \
    X(ANI_RESOLVER,                     Resolver,                  VResolver *)              \
    X(ANI_RESOLVER_STORAGE,             ResolverStorage,           VResolver **)             \
    X(ANI_REF_CALL_ARGS,                RefCallArgs,               VRefCallArgs *)           \
    X(CONST_VOID_PTR,                   ConstVoidPtr,              const void *)              \

// CC-OFFNXT(G.PRE.02-CPP) keep va_list consumption local while sharing switch logic
#define READ_VALUE_FROM_VA_LIST(TYPE, VA_ARGS, VALUE, CLEANUP_ON_UNEXPECTED)             \
    do {                                                                                 \
        switch ((TYPE).GetId()) {                                                        \
            case panda_file::Type::TypeId::U1:                                           \
            case panda_file::Type::TypeId::U16:                                          \
                (VALUE).l = va_arg((VA_ARGS), uint32_t);                                 \
                break;                                                                   \
            case panda_file::Type::TypeId::I8:                                           \
            case panda_file::Type::TypeId::I16:                                          \
            case panda_file::Type::TypeId::I32:                                          \
                (VALUE).i = va_arg((VA_ARGS), int32_t);                                  \
                break;                                                                   \
            case panda_file::Type::TypeId::I64:                                          \
                (VALUE).l = va_arg((VA_ARGS), int64_t);                                  \
                break;                                                                   \
            case panda_file::Type::TypeId::F32:                                          \
                (VALUE).f = static_cast<float>(va_arg((VA_ARGS), double));               \
                break;                                                                   \
            case panda_file::Type::TypeId::F64:                                          \
                (VALUE).d = va_arg((VA_ARGS), double);                                   \
                break;                                                                   \
            case panda_file::Type::TypeId::REFERENCE:                                    \
                if constexpr (sizeof(ani_ref) == sizeof(ani_long)) {                     \
                    (VALUE).l = va_arg((VA_ARGS), int64_t);                              \
                } else {                                                                 \
                    (VALUE).l = va_arg((VA_ARGS), uint32_t);                             \
                }                                                                        \
                break;                                                                   \
            case panda_file::Type::TypeId::NOVALUE:                                      \
                [[fallthrough]];                                                         \
            default:                                                                     \
                CLEANUP_ON_UNEXPECTED;                                                   \
                LOG(FATAL, ANI) << "Unexpected argument type";                           \
                break;                                                                   \
        }                                                                                \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-vararg)
// clang-format on

namespace ark::ets {
class EtsMethod;
class EtsField;
class EtsClass;
}  // namespace ark::ets

namespace ark::ets::ani::verify {

inline bool IsValidAniBoolean(ani_boolean value)
{
    return value == ANI_TRUE || value == ANI_FALSE;
}

PandaString NormalizeMethodNameForAni(const char *name);

ani_status ResolveNamedMethod(EtsClass *klass, const char *name, const char *signature, bool isStaticMethod,
                              EtsMethod **result);
enum class AccessMode {
    READ,
    READWRITE,
};

class VVm;
class VEnv;

class VRef;
class VFnObject;

class VRefCallArgs {
public:
    VRefCallArgs(ani_size argcIn, VRef **vargvIn) : argc_(argcIn), vargv_(vargvIn) {}

    [[nodiscard]] ani_size GetArgc() const
    {
        return argc_;
    }

    [[nodiscard]] VRef **GetVargv() const
    {
        return vargv_;
    }

    [[nodiscard]] ani_ref *GetReleaseArgv() const
    {
        return releaseArgv_;
    }

    void ClearReleaseArgvState()
    {
        releaseArgvStorage_.clear();
        releaseArgv_ = nullptr;
    }

    PandaVector<ani_ref> &MutableReleaseArgvStorage()
    {
        return releaseArgvStorage_;
    }

    void SetReleaseArgv(ani_ref *argv)
    {
        releaseArgv_ = argv;
    }

private:
    ani_size argc_ {};
    VRef **vargv_ {};
    PandaVector<ani_ref> releaseArgvStorage_ {};
    ani_ref *releaseArgv_ {};
};

class VModule;
class VNamespace;
class VObject;
class VType;
class VTupleValue;
class VClass;
class VEnum;
class VEnumItem;
class VMethod;
class VStaticMethod;
class VFunction;
class VField;
class VStaticField;
class VVariable;
class VString;
class VError;
class VArray;
class VArrayBuffer;
class VFixedArray;
class VFixedArrayBoolean;
class VFixedArrayChar;
class VFixedArrayByte;
class VFixedArrayShort;
class VFixedArrayInt;
class VFixedArrayLong;
class VFixedArrayFloat;
class VFixedArrayDouble;
class VFixedArrayRef;
class VResolver;

class ANIArg {
public:
    struct AniMethodArgs {
        EtsMethod *method;
        const ani_value *vargs;  // NOTE: Reblace ani_value by VValue
        PandaSmallVector<ani_value> argsStorage;
        bool isVaArgs;
    };

    struct AniFunctionalObjectArgv {
        ani_size argc {};
        ani_ref *argv {};
        PandaVector<ani_ref> releaseArgvStorage {};
        ani_ref *releaseArgv {};
    };

    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
    // clang-format off
    enum class Action : size_t {
    // CC-OFFNXT(G.PRE.02) code generation
#define X(ACTION, FN) ACTION,
        ANI_VERIFICATION_MAP
#undef X
        NUMBER_OF_ELEMENTS,
    };
    // clang-format on
    // NOLINTEND(cppcoreguidelines-macro-usage)

    static ANIArg MakeForVm(VVm *vm, std::string_view name)
    {
        return ANIArg(ArgValueByVm(vm), name, Action::VERIFY_VM);
    }

    static ANIArg MakeForEnvVersion(uint32_t version, std::string_view name)
    {
        return ANIArg(ArgValueByU32(version), name, Action::VERIFY_ENV_VERSION);
    }

    static ANIArg MakeForEnvStorage(VEnv **envStorage, std::string_view name)
    {
        return ANIArg(ArgValueByEnvStorage(envStorage), name, Action::VERIFY_ENV_STORAGE);
    }

    static ANIArg MakeForMethodStorage(VMethod **methodStorage, std::string_view name)
    {
        return ANIArg(ArgValueByMethodStorage(methodStorage), name, Action::VERIFY_METHOD_STORAGE);
    }

    static ANIArg MakeForStaticMethodStorage(VStaticMethod **staticMethodStorage, std::string_view name)
    {
        return ANIArg(ArgValueByStaticMethodStorage(staticMethodStorage), name, Action::VERIFY_STATIC_METHOD_STORAGE);
    }

    static ANIArg MakeForFunctionStorage(VFunction **functionStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFunctionStorage(functionStorage), name, Action::VERIFY_FUNCTION_STORAGE);
    }

    static ANIArg MakeForModuleStorage(VModule **moduleStorage, std::string_view name)
    {
        return ANIArg(ArgValueByModuleStorage(moduleStorage), name, Action::VERIFY_MODULE_STORAGE);
    }

    static ANIArg MakeForNamespaceStorage(VNamespace **namespaceStorage, std::string_view name)
    {
        return ANIArg(ArgValueByNamespaceStorage(namespaceStorage), name, Action::VERIFY_NAMESPACE_STORAGE);
    }

    static ANIArg MakeForClassStorage(VClass **classStorage, std::string_view name)
    {
        return ANIArg(ArgValueByClassStorage(classStorage), name, Action::VERIFY_CLASS_STORAGE);
    }

    static ANIArg MakeForFieldStorage(VField **fieldStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFieldStorage(fieldStorage), name, Action::VERIFY_FIELD_STORAGE);
    }

    static ANIArg MakeForStaticFieldStorage(VStaticField **staticFieldStorage, std::string_view name)
    {
        return ANIArg(ArgValueByStaticFieldStorage(staticFieldStorage), name, Action::VERIFY_STATIC_FIELD_STORAGE);
    }

    static ANIArg MakeForVariableStorage(VVariable **variableStorage, std::string_view name)
    {
        return ANIArg(ArgValueByVariableStorage(variableStorage), name, Action::VERIFY_VARIABLE_STORAGE);
    }

    static ANIArg MakeForOptions(const ani_options *options, std::string_view name)
    {
        return ANIArg(ArgValueByOptions(options), name, Action::VERIFY_OPTIONS);
    }

    static ANIArg MakeForEnv(VEnv *venv, std::string_view name, bool checkPendingError = true)
    {
        if (checkPendingError) {
            return ANIArg(ArgValueByEnv(venv), name, Action::VERIFY_ENV);
        }
        return ANIArg(ArgValueByEnv(venv), name, Action::VERIFY_ENV_SKIP_PENDING_ERROR);
    }

    static ANIArg MakeForNrRefs(ani_size nrRefs, std::string_view name)
    {
        return ANIArg(ArgValueBySize(nrRefs), name, Action::VERIFY_NR_REFS);
    }

    static ANIArg MakeForRef(VRef *vref, std::string_view name)
    {
        return ANIArg(ArgValueByRef(vref), name, Action::VERIFY_REF);
    }

    static ANIArg MakeForAnyRef(VRef *vref, std::string_view name)
    {
        return ANIArg(ArgValueByRef(vref), name, Action::VERIFY_ANY_REF);
    }

    static ANIArg MakeForModule(VModule *vmodule, std::string_view name)
    {
        return ANIArg(ArgValueByModule(vmodule), name, Action::VERIFY_MODULE);
    }

    static ANIArg MakeForNamespace(VNamespace *vnamespace, std::string_view name)
    {
        return ANIArg(ArgValueByNamespace(vnamespace), name, Action::VERIFY_NAMESPACE);
    }

    static ANIArg MakeForType(VType *vtype, std::string_view name)
    {
        return ANIArg(ArgValueByType(vtype), name, Action::VERIFY_TYPE);
    }

    static ANIArg MakeForGlobalRef(VRef *vgref, std::string_view name)
    {
        return ANIArg(ArgValueByRef(vgref), name, Action::VERIFY_GLOBAL_REF);
    }

    static ANIArg MakeForWRef(ani_wref wref, std::string_view name)
    {
        return ANIArg(ArgValueByWRef(wref), name, Action::VERIFY_WREF);
    }

    static ANIArg MakeForFunctionalObject(VFnObject *vfnObject, std::string_view name)
    {
        return ANIArg(ArgValueByFnObject(vfnObject), name, Action::VERIFY_FUNCTIONAL_OBJECT);
    }

    static ANIArg MakeForFunctionalObjectArgv(AniFunctionalObjectArgv *args, std::string_view name)
    {
        return ANIArg(ArgValueByFunctionalObjectArgv(args), name, Action::VERIFY_FUNCTIONAL_OBJECT_ARGV);
    }

    static ANIArg MakeForClass(VClass *vclass, std::string_view name)
    {
        return ANIArg(ArgValueByClass(vclass), name, Action::VERIFY_CLASS);
    }

    static ANIArg MakeForEnum(VEnum *venum, std::string_view name)
    {
        return ANIArg(ArgValueByEnum(venum), name, Action::VERIFY_ENUM);
    }

    static ANIArg MakeForEnumItem(VEnumItem *venumItem, std::string_view name)
    {
        return ANIArg(ArgValueByEnumItem(venumItem), name, Action::VERIFY_ENUM_ITEM);
    }

    static ANIArg MakeForString(VString *str, std::string_view name)
    {
        return ANIArg(ArgValueByString(str), name, Action::VERIFY_STRING);
    }

    static ANIArg MakeForUTF8Buffer(char *utf8Buffer, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8Buffer(utf8Buffer), name, Action::VERIFY_UTF8_BUFFER);
    }

    static ANIArg MakeForError(VError *verr, std::string_view name)
    {
        return ANIArg(ArgValueByError(verr), name, Action::VERIFY_ERROR);
    }

    static ANIArg MakeForUTF8String(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_UTF8_STRING);
    }

    static ANIArg MakeForModuleDescriptor(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_MODULE_DESCRIPTOR);
    }

    static ANIArg MakeForNamespaceDescriptor(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_NAMESPACE_DESCRIPTOR);
    }

    static ANIArg MakeForClassDescriptor(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_CLASS_DESCRIPTOR);
    }

    static ANIArg MakeForEnumDescriptor(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_ENUM_DESCRIPTOR);
    }

    static ANIArg MakeForMethodName(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_METHOD_NAME);
    }

    static ANIArg MakeForStaticMethodName(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_STATIC_METHOD_NAME);
    }

    static ANIArg MakeForSignature(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_SIGNATURE);
    }

    static ANIArg MakeForMethodReturnType(EtsType returnType, std::string_view name)
    {
        return ANIArg(ArgValueBySize(static_cast<ani_size>(returnType)), name, Action::VERIFY_METHOD_RETURN_TYPE,
                      returnType);
    }

    static ANIArg MakeForPropertyByName(const char *name, std::string_view argName, EtsType propertyType,
                                        AccessMode accessMode)
    {
        if (accessMode == AccessMode::READWRITE) {
            return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_WRITE_PROPERTY_BY_NAME, propertyType);
        }
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_READ_PROPERTY_BY_NAME, propertyType);
    }

    static ANIArg MakeForUTF16Buffer(uint16_t *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF16Buffer(ptr), name, Action::VERIFY_UTF16_BUFFER);
    }

    static ANIArg MakeForUTF16String(const uint16_t *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF16String(ptr), name, Action::VERIFY_UTF16_STRING);
    }

    static ANIArg MakeForSize(ani_size size, std::string_view name)
    {
        return ANIArg(ArgValueBySize(size), name, Action::VERIFY_SIZE);
    }

    static ANIArg MakeForDelLocalRef(VRef *vref, std::string_view name)
    {
        return ANIArg(ArgValueByRef(vref), name, Action::VERIFY_DEL_LOCAL_REF);
    }

    static ANIArg MakeForObject(VObject *vobject, std::string_view name)
    {
        return ANIArg(ArgValueByObject(vobject), name, Action::VERIFY_THIS_OBJECT);
    }

    static ANIArg MakeForBoxedPrimitiveObject(VObject *vobject, std::string_view name, EtsType primitiveType)
    {
        return ANIArg(ArgValueByObject(vobject), name, Action::VERIFY_BOXED_PRIMITIVE_OBJECT, primitiveType);
    }

    static ANIArg MakeForField(VField *vfield, std::string_view name, EtsType fieldType, AccessMode accessMode)
    {
        if (accessMode == AccessMode::READWRITE) {
            return ANIArg(ArgValueByField(vfield), name, Action::VERIFY_WRITE_FIELD, fieldType);
        }
        return ANIArg(ArgValueByField(vfield), name, Action::VERIFY_READ_FIELD, fieldType);
    }

    static ANIArg MakeForFieldByName(const char *name, std::string_view argName, EtsType fieldType,
                                     AccessMode accessMode)
    {
        if (accessMode == AccessMode::READWRITE) {
            return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_WRITE_FIELD_BY_NAME, fieldType);
        }
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_READ_FIELD_BY_NAME, fieldType);
    }

    static ANIArg MakeForFindFieldName(const char *name, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_FIND_FIELD_NAME);
    }

    static ANIArg MakeForFindStaticFieldName(const char *name, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_FIND_STATIC_FIELD_NAME);
    }

    static ANIArg MakeForFindVariableName(const char *name, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_FIND_VARIABLE_NAME);
    }

    static ANIArg MakeForFindMethodSignature(const char *signature, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(signature), argName, Action::VERIFY_FIND_METHOD_SIGNATURE);
    }

    static ANIArg MakeForFindStaticMethodSignature(const char *signature, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(signature), argName, Action::VERIFY_FIND_STATIC_METHOD_SIGNATURE);
    }

    static ANIArg MakeForFindFunctionName(const char *name, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_FIND_FUNCTION_NAME);
    }

    static ANIArg MakeForFindFunctionSignature(const char *signature, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(signature), argName, Action::VERIFY_FIND_FUNCTION_SIGNATURE);
    }

    static ANIArg MakeForFindSetterName(const char *name, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_FIND_SETTER_NAME);
    }

    static ANIArg MakeForFindGetterName(const char *name, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_FIND_GETTER_NAME);
    }

    static ANIArg MakeForFindIndexableGetterSignature(const char *signature, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(signature), argName, Action::VERIFY_FIND_INDEXABLE_GETTER_SIG);
    }

    static ANIArg MakeForFindIndexableSetterSignature(const char *signature, std::string_view argName)
    {
        return ANIArg(ArgValueByUTF8String(signature), argName, Action::VERIFY_FIND_INDEXABLE_SETTER_SIG);
    }

    static ANIArg MakeForFindIterator(VClass *vclass, std::string_view argName)
    {
        return ANIArg(ArgValueByClass(vclass), argName, Action::VERIFY_FIND_ITERATOR);
    }

    static ANIArg MakeForStaticFieldByName(const char *name, std::string_view argName, EtsType staticFieldType,
                                           AccessMode accessMode)
    {
        if (accessMode == AccessMode::READWRITE) {
            return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_WRITE_STATIC_FIELD_BY_NAME,
                          staticFieldType);
        }
        return ANIArg(ArgValueByUTF8String(name), argName, Action::VERIFY_READ_STATIC_FIELD_BY_NAME, staticFieldType);
    }

    static ANIArg MakeForStaticField(VStaticField *vstaticField, std::string_view name, EtsType staticFieldType,
                                     AccessMode accessMode)
    {
        if (accessMode == AccessMode::READWRITE) {
            return ANIArg(ArgValueByStaticField(vstaticField), name, Action::VERIFY_WRITE_STATIC_FIELD,
                          staticFieldType);
        }
        return ANIArg(ArgValueByStaticField(vstaticField), name, Action::VERIFY_READ_STATIC_FIELD, staticFieldType);
    }

    static ANIArg MakeForVariable(VVariable *vvariable, std::string_view name, EtsType variableType,
                                  AccessMode accessMode)
    {
        if (accessMode == AccessMode::READWRITE) {
            return ANIArg(ArgValueByVariable(vvariable), name, Action::VERIFY_WRITE_VARIABLE, variableType);
        }
        return ANIArg(ArgValueByVariable(vvariable), name, Action::VERIFY_READ_VARIABLE, variableType);
    }

    static ANIArg MakeForMethod(VMethod *vmethod, std::string_view name, EtsType returnType, bool isConstructor = false)
    {
        if (isConstructor) {
            return ANIArg(ArgValueByMethod(vmethod), name, Action::VERIFY_CTOR, returnType);
        }
        return ANIArg(ArgValueByMethod(vmethod), name, Action::VERIFY_METHOD, returnType);
    }

    static ANIArg MakeForStaticMethod(VStaticMethod *vstaticmethod, std::string_view name, EtsType returnType)
    {
        return ANIArg(ArgValueByStaticMethod(vstaticmethod), name, Action::VERIFY_STATIC_METHOD, returnType);
    }

    static ANIArg MakeForFunction(VFunction *vfunction, std::string_view name, EtsType returnType)
    {
        return ANIArg(ArgValueByFunction(vfunction), name, Action::VERIFY_FUNCTION, returnType);
    }

    static ANIArg MakeForMethodArgs(AniMethodArgs *aniMethodArgs, std::string_view name)
    {
        return ANIArg(ArgValueByMethodArgs(aniMethodArgs), name, Action::VERIFY_METHOD_V_ARGS);
    }

    static ANIArg MakeForMethodAArgs(AniMethodArgs *aniMethodArgs, std::string_view name)
    {
        return ANIArg(ArgValueByMethodArgs(aniMethodArgs), name, Action::VERIFY_METHOD_A_ARGS);
    }

    static ANIArg MakeForAArgs(const ani_value *vargs, std::string_view name)
    {
        return ANIArg(ArgValueByValueArgs(vargs), name, Action::VERIFY_A_ARGS);
    }

    static ANIArg MakeForVvaArgs(va_list &vvaArgs, std::string_view name)
    {
        return ANIArg(ArgValueByVvaArgs(&vvaArgs), name, Action::VERIFY_VVA_ARGS);
    }

    static ANIArg MakeForArray(VArray *varray, std::string_view name)
    {
        return ANIArg(ArgValueByArray(varray), name, Action::VERIFY_ARRAY);
    }

    static ANIArg MakeForTupleValue(VTupleValue *vtupleValue, std::string_view name)
    {
        return ANIArg(ArgValueByTupleValue(vtupleValue), name, Action::VERIFY_TUPLE_VALUE);
    }

    static ANIArg MakeForTupleIndex(ani_size index, EtsType tupleElementType, std::string_view name)
    {
        return ANIArg(ArgValueBySize(index), name, Action::VERIFY_TUPLE_INDEX, tupleElementType);
    }

    static ANIArg MakeForArrayBuffer(VArrayBuffer *varraybuffer, std::string_view name)
    {
        return ANIArg(ArgValueByArrayBuffer(varraybuffer), name, Action::VERIFY_ARRAYBUFFER);
    }

    static ANIArg MakeForFixedArray(VFixedArray *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArray(varray), name, Action::VERIFY_FIXED_ARRAY);
    }

    static ANIArg MakeForFixedArrayBoolean(VFixedArrayBoolean *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayBoolean(varray), name, Action::VERIFY_FIXED_ARRAY_BOOLEAN);
    }

    static ANIArg MakeForFixedArrayChar(VFixedArrayChar *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayChar(varray), name, Action::VERIFY_FIXED_ARRAY_CHAR);
    }

    static ANIArg MakeForFixedArrayByte(VFixedArrayByte *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayByte(varray), name, Action::VERIFY_FIXED_ARRAY_BYTE);
    }

    static ANIArg MakeForFixedArrayShort(VFixedArrayShort *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayShort(varray), name, Action::VERIFY_FIXED_ARRAY_SHORT);
    }

    static ANIArg MakeForFixedArrayInt(VFixedArrayInt *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayInt(varray), name, Action::VERIFY_FIXED_ARRAY_INT);
    }

    static ANIArg MakeForFixedArrayLong(VFixedArrayLong *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayLong(varray), name, Action::VERIFY_FIXED_ARRAY_LONG);
    }

    static ANIArg MakeForFixedArrayFloat(VFixedArrayFloat *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayFloat(varray), name, Action::VERIFY_FIXED_ARRAY_FLOAT);
    }

    static ANIArg MakeForFixedArrayDouble(VFixedArrayDouble *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayDouble(varray), name, Action::VERIFY_FIXED_ARRAY_DOUBLE);
    }

    static ANIArg MakeForFixedArrayRef(VFixedArrayRef *varray, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayRef(varray), name, Action::VERIFY_FIXED_ARRAY_REF);
    }

    static ANIArg MakeForFixedArrayInitialRef(VRef *vref, std::string_view name)
    {
        return ANIArg(ArgValueByRef(vref), name, Action::VERIFY_FIXED_ARRAY_INITIAL_REF);
    }

    static ANIArg MakeForFixedArraySetRef(VRef *vref, std::string_view name)
    {
        return ANIArg(ArgValueByRef(vref), name, Action::VERIFY_FIXED_ARRAY_SET_REF);
    }

    static ANIArg MakeForRegionBuffer(const void *buffer, ani_size length, std::string_view name)
    {
        return ANIArg(ArgValueByConstVoidPtr(buffer), name, Action::VERIFY_REGION_BUFFER, length);
    }

    static ANIArg MakeForArrayIndex(ani_size index, std::string_view name)
    {
        return ANIArg(ArgValueBySize(index), name, Action::VERIFY_ARRAY_INDEX);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForBooleanStorage(ani_boolean *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByBooleanStorage(valueStorage), name, Action::VERIFY_BOOLEAN_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForCharStorage(ani_char *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByCharStorage(valueStorage), name, Action::VERIFY_CHAR_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForByteStorage(ani_byte *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByByteStorage(valueStorage), name, Action::VERIFY_BYTE_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForShortStorage(ani_short *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByShortStorage(valueStorage), name, Action::VERIFY_SHORT_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForIntStorage(ani_int *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByIntStorage(valueStorage), name, Action::VERIFY_INT_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForLongStorage(ani_long *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByLongStorage(valueStorage), name, Action::VERIFY_LONG_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForFloatStorage(ani_float *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFloatStorage(valueStorage), name, Action::VERIFY_FLOAT_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForDoubleStorage(ani_double *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByDoubleStorage(valueStorage), name, Action::VERIFY_DOUBLE_STORAGE);
    }

    static ANIArg MakeForRefStorage(VRef **refStorage, std::string_view name)
    {
        return ANIArg(ArgValueByRefStorage(refStorage), name, Action::VERIFY_REF_STORAGE);
    }

    static ANIArg MakeForWRefStorage(ani_wref *wrefStorage, std::string_view name)
    {
        return ANIArg(ArgValueByWRefStorage(wrefStorage), name, Action::VERIFY_WREF_STORAGE);
    }

    static ANIArg MakeForObjectStorage(VObject **valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByObjectStorage(valueStorage), name, Action::VERIFY_OBJECT_STORAGE);
    }

    static ANIArg MakeForEnumStorage(VEnum **enumStorage, std::string_view name)
    {
        return ANIArg(ArgValueByEnumStorage(enumStorage), name, Action::VERIFY_ENUM_STORAGE);
    }

    static ANIArg MakeForEnumItemStorage(VEnumItem **enumItemStorage, std::string_view name)
    {
        return ANIArg(ArgValueByEnumItemStorage(enumItemStorage), name, Action::VERIFY_ENUM_ITEM_STORAGE);
    }

    static ANIArg MakeForStringStorage(VString **strStorage, std::string_view name)
    {
        return ANIArg(ArgValueByStringStorage(strStorage), name, Action::VERIFY_STRING_STORAGE);
    }

    static ANIArg MakeForSizeStorage(ani_size *sizeStorage, std::string_view name)
    {
        return ANIArg(ArgValueBySizeStorage(sizeStorage), name, Action::VERIFY_SIZE_STORAGE);
    }

    static ANIArg MakeForVoidPtrStorage(void **ptrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByVoidPtrStorage(ptrStorage), name, Action::VERIFY_VOID_PTR_STORAGE);
    }

    static ANIArg MakeForArrayBufferStorage(VArrayBuffer **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByArrayBufferStorage(arrStorage), name, Action::VERIFY_ARRAYBUFFER_STORAGE);
    }

    static ANIArg MakeForU32Storage(uint32_t *u32Storage, std::string_view name)
    {
        return ANIArg(ArgValueByU32Storage(u32Storage), name, Action::VERIFY_U32_STORAGE);
    }

    static ANIArg MakeForVmStorage(VVm **vmStorage, std::string_view name)
    {
        return ANIArg(ArgValueByVmStorage(vmStorage), name, Action::VERIFY_VM_STORAGE);
    }

    static ANIArg MakeForBoolean(ani_boolean booleanValue, std::string_view name)
    {
        return ANIArg(ArgValueByBoolean(booleanValue), name, Action::VERIFY_PRIMITIVE_VALUE);
    }

    static ANIArg MakeForChar(ani_char charValue, std::string_view name)
    {
        return ANIArg(ArgValueByChar(charValue), name, Action::VERIFY_PRIMITIVE_VALUE);
    }

    static ANIArg MakeForByte(ani_byte byteValue, std::string_view name)
    {
        return ANIArg(ArgValueByByte(byteValue), name, Action::VERIFY_PRIMITIVE_VALUE);
    }

    static ANIArg MakeForShort(ani_short shortValue, std::string_view name)
    {
        return ANIArg(ArgValueByShort(shortValue), name, Action::VERIFY_PRIMITIVE_VALUE);
    }

    static ANIArg MakeForInt(ani_int intValue, std::string_view name)
    {
        return ANIArg(ArgValueByInt(intValue), name, Action::VERIFY_PRIMITIVE_VALUE);
    }

    static ANIArg MakeForLong(ani_long longValue, std::string_view name)
    {
        return ANIArg(ArgValueByLong(longValue), name, Action::VERIFY_PRIMITIVE_VALUE);
    }

    static ANIArg MakeForFloat(ani_float floatValue, std::string_view name)
    {
        return ANIArg(ArgValueByFloat(floatValue), name, Action::VERIFY_PRIMITIVE_VALUE);
    }

    static ANIArg MakeForDouble(ani_double doubleValue, std::string_view name)
    {
        return ANIArg(ArgValueByDouble(doubleValue), name, Action::VERIFY_PRIMITIVE_VALUE);
    }

    static ANIArg MakeForArrayBufferLength(size_t length, std::string_view name)
    {
        return ANIArg(ArgValueBySize(length), name, Action::VERIFY_ARRAYBUFFER_LENGTH);
    }

    static ANIArg MakeForFixedArrayLength(ani_size length, std::string_view name)
    {
        return ANIArg(ArgValueBySize(length), name, Action::VERIFY_FIXED_ARRAY_LENGTH);
    }

    static ANIArg MakeForNativeFunctions(const ani_native_function *functions, ani_size nrFunctions,
                                         std::string_view name)
    {
        return ANIArg(ArgValueByNativeFunctions(functions), name, Action::VERIFY_NATIVE_FUNCTIONS, nrFunctions);
    }

    static ANIArg MakeForNativeMethods(const ani_native_function *methods, ani_size nrMethods, std::string_view name)
    {
        return ANIArg(ArgValueByNativeFunctions(methods), name, Action::VERIFY_NATIVE_METHODS, nrMethods);
    }

    static ANIArg MakeForNativeStaticMethods(const ani_native_function *methods, ani_size nrMethods,
                                             std::string_view name)
    {
        return ANIArg(ArgValueByNativeFunctions(methods), name, Action::VERIFY_STATIC_NATIVE_METHODS, nrMethods);
    }

    static ANIArg MakeForErrorStorage(VError **errStorage, std::string_view name)
    {
        return ANIArg(ArgValueByErrorStorage(errStorage), name, Action::VERIFY_ERROR_STORAGE);
    }

    static ANIArg MakeForArrayStorage(VArray **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByArrayStorage(arrStorage), name, Action::VERIFY_ARRAY_STORAGE);
    }

    static ANIArg MakeForArrayBooleanStorage(VFixedArrayBoolean **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayBooleanStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_BOOLEAN_STORAGE);
    }

    static ANIArg MakeForArrayCharStorage(VFixedArrayChar **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayCharStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_CHAR_STORAGE);
    }

    static ANIArg MakeForArrayByteStorage(VFixedArrayByte **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayByteStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_BYTE_STORAGE);
    }

    static ANIArg MakeForArrayShortStorage(VFixedArrayShort **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayShortStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_SHORT_STORAGE);
    }

    static ANIArg MakeForArrayIntStorage(VFixedArrayInt **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayIntStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_INT_STORAGE);
    }

    static ANIArg MakeForArrayLongStorage(VFixedArrayLong **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayLongStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_LONG_STORAGE);
    }

    static ANIArg MakeForArrayFloatStorage(VFixedArrayFloat **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayFloatStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_FLOAT_STORAGE);
    }

    static ANIArg MakeForArrayDoubleStorage(VFixedArrayDouble **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayDoubleStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_DOUBLE_STORAGE);
    }

    static ANIArg MakeForArrayRefStorage(VFixedArrayRef **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayRefStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_REF_STORAGE);
    }

    static ANIArg MakeForResolver(VResolver *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByResolver(valueStorage), name, Action::VERIFY_RESOLVER);
    }

    static ANIArg MakeForResolverStorage(VResolver **valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByResolverStorage(valueStorage), name, Action::VERIFY_RESOLVER_STORAGE);
    }

    static ANIArg MakeForRefCallArgs(VRefCallArgs *refCallArgs, std::string_view name)
    {
        return ANIArg(ArgValueByRefCallArgs(refCallArgs), name, Action::VERIFY_REF_CALL_ARGS);
    }

    static ANIArg MakeForTypeStorage(VType **valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByTypeStorage(valueStorage), name, Action::VERIFY_TYPE_STORAGE);
    }

    static ANIArg MakeForPromiseStorage(VObject **valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByObjectStorage(valueStorage), name, Action::VERIFY_OBJECT_STORAGE);
    }

    Action GetAction() const
    {
        return action_;
    }

    std::string_view GetName() const
    {
        return name_;
    }

    PandaString GetStringValue() const;
    PandaString GetStringType() const;

    // NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-pro-type-union-access)
    // Generate methods:
    //   <Type> GetValue<Name>() const;
#define X(ENUM_VALUE, NAME, TYPE)                                                   \
    TYPE GetValue##NAME() const                                                     \
    {                                                                               \
        ASSERT(type_ == ValueType::ENUM_VALUE); /* CC-OFF(G.PRE.02) function gen */ \
        return value_.v##NAME;                  /* CC-OFF(G.PRE.05) function gen */ \
    }
    ANI_ARG_TYPES_MAP
#undef X
    // NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-pro-type-union-access)

    void *GetValueRawPointer() const
    {
        return reinterpret_cast<void *>(value_.vVm);  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    EtsType GetReturnType() const
    {
        return returnType_;
    }

    ani_size GetNativeFunctionCount() const
    {
        return nativeFunctionCount_;
    }

    bool IsBooleanValue() const
    {
        return type_ == ValueType::ANI_BOOLEAN;
    }

private:
    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
    union RawValue {
// CC-OFFNXT(G.PRE.02,G.PRE.09) code generation
#define X(ENUM_VALUE, NAME, TYPE) TYPE v##NAME;
        ANI_ARG_TYPES_MAP
#undef X
    };
    static_assert(sizeof(RawValue) == sizeof(double));

    enum class ValueType {
// CC-OFFNXT(G.PRE.02) code generation
#define X(ENUM_VALUE, NAME, TYPE) ENUM_VALUE,
        ANI_ARG_TYPES_MAP
#undef X
    };
    // NOLINTEND(cppcoreguidelines-macro-usage)

    struct ArgValue {
        RawValue value;
        ValueType type;
    };

    explicit ANIArg(ArgValue value, std::string_view name, Action action)
        : value_(value.value), type_(value.type), name_(name), action_(action)
    {
    }

    explicit ANIArg(ArgValue value, std::string_view name, Action action, EtsType returnType)
        : value_(value.value), type_(value.type), name_(name), action_(action), returnType_(returnType)
    {
    }

    explicit ANIArg(ArgValue value, std::string_view name, Action action, ani_size nativeFunctionCount)
        : value_(value.value),
          type_(value.type),
          name_(name),
          action_(action),
          nativeFunctionCount_(nativeFunctionCount)
    {
    }

    // NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-pro-type-union-access)
    // Generate methods:
    //   static ArgValue ArgValueBy<Name>(<Type> value);
#define X(ENUM_VALUE, NAME, TYPE)                                                         \
    static ArgValue ArgValueBy##NAME(TYPE value)                                          \
    {                                                                                     \
        RawValue raw {};                                                                  \
        raw.v##NAME = value;                                                              \
        return {raw, ValueType::ENUM_VALUE}; /* CC-OFF(G.PRE.02,G.PRE.05) function gen */ \
    }
    ANI_ARG_TYPES_MAP
#undef X
    // NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-pro-type-union-access)

    RawValue value_;
    ValueType type_;
    std::string_view name_;
    Action action_ {};

    EtsType returnType_ {EtsType::UNKNOWN};
    ani_size nativeFunctionCount_ {};
};

bool VerifyANIArgs(std::string_view functionName, std::initializer_list<ANIArg> args);
void VerifyReportANI(std::string_view functionName, std::string_view message);

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H
