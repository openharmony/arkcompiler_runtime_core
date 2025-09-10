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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H

#include "plugins/ets/runtime/ani/ani.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

// clang-format off
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define ANI_VERIFICATION_MAP                                    \
    X(VERIFY_VM,                     VerifyVm)                  \
    X(VERIFY_ENV,                    VerifyEnv)                 \
    X(VERIFY_ENV_SKIP_PENDING_ERROR, VerifyEnvSkipPendingError) \
    X(VERIFY_OPTIONS,                VerifyOptions)             \
    X(VERIFY_ENV_VERSION,            VerifyEnvVersion)          \
    X(VERIFY_NR_REFS,                VerifyNrRefs)              \
    X(VERIFY_REF,                    VerifyRef)                 \
    X(VERIFY_CLASS,                  VerifyClass)               \
    X(VERIFY_STRING,                 VerifyString)              \
    X(VERIFY_DEL_LOCAL_REF,          VerifyDelLocalRef)         \
    X(VERIFY_CTOR,                   VerifyCtor)                \
    X(VERIFY_METHOD,                 VerifyMethod)              \
    X(VERIFY_METHOD_A_ARGS,          VerifyMethodAArgs)         \
    X(VERIFY_METHOD_V_ARGS,          VerifyMethodVArgs)         \
    X(VERIFY_VM_STORAGE,             VerifyVmStorage)           \
    X(VERIFY_ENV_STORAGE,            VerifyEnvStorage)          \
    X(VERIFY_BOOLEAN_STORAGE,        VerifyBooleanStorage)      \
    X(VERIFY_REF_STORAGE,            VerifyRefStorage)          \
    X(VERIFY_OBJECT_STORAGE,         VerifyObjectStorage)       \
    X(VERIFY_STRING_STORAGE,         VerifyStringStorage)       \
    X(VERIFY_SIZE_STORAGE,           VerifySizeStorage)         \
    X(VERIFY_UTF16_BUFFER,           VerifyUTF16Buffer)         \
    X(VERIFY_UTF16_STRING,           VerifyUTF16String)         \
    X(VERIFY_UTF8_BUFFER,            VerifyUTF8Buffer)          \
    X(VERIFY_UTF8_STRING,            VerifyUTF8String)          \
    X(VERIFY_SIZE,                   VerifySize)                \

// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define ANI_ARG_TYPES_MAP                                                \
    X(ANI_VM,              Vm,                      VVm *)               \
    X(ANI_ENV,             Env,                     VEnv *)              \
    X(ANI_OPTIONS,         Options,                 const ani_options *) \
    X(ANI_SIZE,            Size,                    ani_size)            \
    X(ANI_BOOLEAN,         Boolean,        ani_boolean)         \
    X(ANI_CHAR,            Char,           ani_char)            \
    X(ANI_BYTE,            Byte,           ani_byte)            \
    X(ANI_SHORT,           Short,          ani_short)           \
    X(ANI_INT,             Int,            ani_int)             \
    X(ANI_LONG,            Long,           ani_long)            \
    X(ANI_FLOAT,           Float,          ani_float)           \
    X(ANI_DOUBLE,          Double,         ani_double)          \
    X(ANI_REF,             Ref,                     VRef *)              \
    X(ANI_CLASS,           Class,                   VClass *)            \
    X(ANI_METHOD,          Method,                  ani_method)          \
    X(ANI_STRING,          String,                  VString *)           \
    X(ANI_VALUE_ARGS,      ValueArgs,               const ani_value *)   \
    X(ANI_ENV_STORAGE,     EnvStorage,              VEnv **)             \
    X(ANI_VM_STORAGE,      VmStorage,               ani_vm **)           \
    X(ANI_BOOLEAN_STORAGE, BooleanStorage, ani_boolean *)       \
    X(ANI_CHAR_STORAGE,    CharStorage,    ani_char *)          \
    X(ANI_BYTE_STORAGE,    ByteStorage,    ani_byte *)          \
    X(ANI_SHORT_STORAGE,   ShortStorage,   ani_short *)         \
    X(ANI_INT_STORAGE,     IntStorage,     ani_int *)           \
    X(ANI_LONG_STORAGE,    LontStorage,    ani_long *)          \
    X(ANI_FLOAT_STORAGE,   FloatStorage,   ani_float *)         \
    X(ANI_DOUBLE_STORAGE,  DoubleStorage,  ani_double *)        \
    X(ANI_REF_STORAGE,     RefStorage,              VRef **)             \
    X(ANI_OBJECT_STORAGE,  ObjectStorage,           VObject **)          \
    X(ANI_STRING_STORAGE,  StringStorage,           VString **)          \
    X(ANI_SIZE_STORAGE,    SizeStorage,             ani_size *)          \
    X(ANI_UTF8_BUFFER,     UTF8Buffer,              char *)              \
    X(ANI_UTF16_BUFFER,    UTF16Buffer,             uint16_t *)          \
    X(ANI_UTF8_STRING,     UTF8String,              const char *)        \
    X(ANI_UTF16_STRING,    UTF16String,             const uint16_t *)    \
    X(UINT32,              U32,                     uint32_t)            \
    X(UINT32x,             U32x,                    uint32_t)            \
    X(METHOD_ARGS,         MethodArgs,              AniMethodArgs *)
// NOLINTEND(cppcoreguidelines-macro-usage)
// clang-format on

namespace ark::ets {
class EtsMethod;
}  // namespace ark::ets

namespace ark::ets::ani::verify {

class VVm;
class VEnv;

class VRef;
class VObject;
class VClass;
class VString;

class ANIArg {
public:
    struct AniMethodArgs {
        ani_method method;
        const ani_value *vargs;  // NOTE: Reblace ani_value by VValue
        PandaSmallVector<ani_value> argsStorage;
        bool isVaArgs;
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

    static ANIArg MakeForClass(VClass *vclass, std::string_view name)
    {
        return ANIArg(ArgValueByClass(vclass), name, Action::VERIFY_CLASS);
    }

    static ANIArg MakeForString(VString *str, std::string_view name)
    {
        return ANIArg(ArgValueByString(str), name, Action::VERIFY_STRING);
    }

    static ANIArg MakeForUTF8Buffer(char *utf8Buffer, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8Buffer(utf8Buffer), name, Action::VERIFY_UTF8_BUFFER);
    }

    static ANIArg MakeForUTF8String(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_UTF8_STRING);
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

    static ANIArg MakeForMethod(ani_method method, std::string_view name, bool isConstructor = false)
    {
        if (isConstructor) {
            return ANIArg(ArgValueByMethod(method), name, Action::VERIFY_CTOR);
        }
        return ANIArg(ArgValueByMethod(method), name, Action::VERIFY_METHOD);
    }

    static ANIArg MakeForMethodArgs(AniMethodArgs *aniMethodArgs, std::string_view name)
    {
        return ANIArg(ArgValueByMethodArgs(aniMethodArgs), name, Action::VERIFY_METHOD_V_ARGS);
    }

    static ANIArg MakeForMethodAArgs(AniMethodArgs *aniMethodArgs, std::string_view name)
    {
        return ANIArg(ArgValueByMethodArgs(aniMethodArgs), name, Action::VERIFY_METHOD_A_ARGS);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForBooleanStorage(ani_boolean *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByBooleanStorage(valueStorage), name, Action::VERIFY_BOOLEAN_STORAGE);
    }

    static ANIArg MakeForRefStorage(VRef **refStorage, std::string_view name)
    {
        return ANIArg(ArgValueByRefStorage(refStorage), name, Action::VERIFY_REF_STORAGE);
    }

    static ANIArg MakeForObjectStorage(VObject **valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByObjectStorage(valueStorage), name, Action::VERIFY_OBJECT_STORAGE);
    }

    static ANIArg MakeForStringStorage(VString **strStorage, std::string_view name)
    {
        return ANIArg(ArgValueByStringStorage(strStorage), name, Action::VERIFY_STRING_STORAGE);
    }

    static ANIArg MakeForSizeStorage(ani_size *sizeStorage, std::string_view name)
    {
        return ANIArg(ArgValueBySizeStorage(sizeStorage), name, Action::VERIFY_SIZE_STORAGE);
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
};

bool VerifyANIArgs(std::string_view functionName, std::initializer_list<ANIArg> args);
void VerifyAbortANI(std::string_view functionName, std::string_view message);

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H
