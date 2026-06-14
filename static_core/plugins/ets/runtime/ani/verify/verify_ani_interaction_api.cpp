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

#include "plugins/ets/runtime/ani/verify/verify_ani_interaction_api.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>

#include "ani.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_converters.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ani/verify/types/internal_ref.h"
#include "plugins/ets/runtime/ani/verify/types/venv.h"
#include "plugins/ets/runtime/ani/verify/types/venv-inl.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "plugins/ets/runtime/ani/verify/types/vwref.h"
#include "plugins/ets/runtime/ani/verify/types/vvm.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_cast_api.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_checker.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_resolve.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/mem/panda_containers.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_PTR_ARG(arg) ANI_CHECK_RETURN_IF_EQ(arg, nullptr, ANI_INVALID_ARGS)

#define VERIFY_ANI_ARGS(...)                                                                                  \
    do {                                                                                                      \
        bool success = VerifyANIArgs(__func__, {__VA_ARGS__});                                                \
        auto *aniVerifier = static_cast<PandaEtsVM *>(Runtime::GetCurrent()->GetPandaVM())->GetANIVerifier(); \
        if (aniVerifier == nullptr || !aniVerifier->IsWorkaroundNoCrashIfInvalidUsage()) {                    \
            ANI_CHECK_RETURN_IF_EQ(success, false, ANI_ERROR);                                                \
        }                                                                                                     \
    } while (false)

// CC-OFFNXT(G.PRE.02) should be with define
#define VERIFY_ANI_ABORT_IF_ERROR(fn)                             \
    do {                                                          \
        std::optional<PandaString> err = (fn);                    \
        if (err) {                                                \
            VerifyReportANI(__func__, err.value());               \
            return ANI_ERROR; /* CC-OFF(G.PRE.05) function gen */ \
        }                                                         \
    } while (false)

// CC-OFFNXT(G.PRE.02) should be with define
#define ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult)                 \
    do {                                                                            \
        if (LIKELY((status) == ANI_OK)) {                                           \
            using ResType = std::remove_reference_t<decltype(*(vresult))>;          \
            *(vresult) = static_cast<ResType>((venv)->AddLocalVerifiedRef(result)); \
        }                                                                           \
    } while (false)
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace ark::ets::ani::verify {

static const __ani_interaction_api *GetInteractionAPI(VEnv *venv)
{
    return PandaAniEnv::FromAniEnv(venv->GetEnv())->GetEnvANIVerifier()->GetInteractionAPI();
}

template <typename VFixedArrayType>
static bool IsFixedArrayRegionOutOfRange(VEnv *venv, VFixedArrayType *varray, ani_size offset, ani_size length)
{
    ScopedManagedCodeFix s(venv->GetEnv());
    EtsArray *internalArray = s.ToInternalType(varray->GetRef());
    auto arrayLength = internalArray->GetLength();
    return offset > arrayLength || length > (arrayLength - offset);
}

template <typename T>
static std::optional<PandaString> VerifyRegionBufferSpan(const T *nativeBuffer, ani_size length)
{
    if (nativeBuffer == nullptr || length == 0) {
        return {};
    }

    auto addr = reinterpret_cast<uintptr_t>(nativeBuffer);
    auto maxAddr = std::numeric_limits<uintptr_t>::max();
    if (sizeof(T) > (maxAddr - addr) / length) {
        return "wrong native buffer";
    }
    return {};
}

static std::optional<PandaString> VerifyBooleanRegionBufferValues(const ani_boolean *nativeBuffer, ani_size length)
{
    if (nativeBuffer == nullptr) {
        return {};
    }

    for (ani_size i = 0; i < length; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (!IsValidAniBoolean(nativeBuffer[i])) {
            PandaStringStream ss;
            ss << "wrong value for ani_boolean in native buffer at index " << i;
            return ss.str();
        }
    }
    return {};
}

static PandaSmallVector<ani_value> GetVValueArgs(VEnv *venv, const ANIArg::AniMethodArgs &methodArgs)
{
    (void)venv;
    EtsMethod *etsMethod = methodArgs.method;
    ASSERT(etsMethod != nullptr);

    PandaSmallVector<ani_value> args;
    // NOTE: Check correcting number of args for ani_method/ani_static_method/ani_function
    args.resize(etsMethod->GetNumArgs());

    auto &vArgs = methodArgs.vargs;
    size_t i = 0;

    panda_file::ShortyIterator it(etsMethod->GetPandaMethod()->GetShorty());
    panda_file::ShortyIterator end;
    ++it;  // skip the return value
    for (; it != end; ++it) {
        ani_value vArg = vArgs[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ani_value value {};
        panda_file::Type type = *it;
        if (type.GetId() == panda_file::Type::TypeId::REFERENCE) {
            // NOTE: Add reference validation
            auto ref = ResolveToAniRef(reinterpret_cast<VRef *>(vArg.r));
            value.r = ref;
        } else {
            value = vArg;
        }
        args[i] = value;
        ++i;
    }
    return args;
}

static ANIArg::AniMethodArgs GetVArgsByVVAArgs(EtsMethod *etsMethod, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs {etsMethod, nullptr, {}, true};
    if (etsMethod == nullptr) {
        return methodArgs;
    }
    auto &args = methodArgs.argsStorage;
    args.reserve(etsMethod->GetNumArgs());
    panda_file::ShortyIterator it(etsMethod->GetPandaMethod()->GetShorty());
    panda_file::ShortyIterator end;
    ++it;  // skip the return value
    for (; it != end; ++it) {
        ani_value v {};
        panda_file::Type type = *it;
        READ_VALUE_FROM_VA_LIST(type, vvaArgs, v, (void)0);
        args.emplace_back(v);
    }
    methodArgs.vargs = methodArgs.argsStorage.data();
    return methodArgs;
}

// CC-OFFNXT(G.FUD.05) solid logic
static ANIArg::AniMethodArgs GetVArgsByVVAArgs(impl::VMethod *vmethod, va_list vvaArgs)
{
    return GetVArgsByVVAArgs(GetEtsMethodIfPointerValid(vmethod), vvaArgs);
}

static ani_status ResolveVerifiedMethodByName(VEnv *venv, VObject *vobject, const char *name, const char *signature,
                                              VMethod **vmethod)
{
    ASSERT(vmethod != nullptr);
    ani_method methodHandle {};
    ani_type result {};
    auto objectRef = ResolveToAniRef(vobject);
    ani_status status = GetInteractionAPI(venv)->Object_GetType(venv->GetEnv(), objectRef, &result);
    if (status != ANI_OK) {
        return status;
    }
    auto vtype = static_cast<VType *>(venv->AddLocalVerifiedRef(result));
    status = GetInteractionAPI(venv)->Class_FindMethod(venv->GetEnv(), static_cast<ani_class>(ResolveToAniType(vtype)),
                                                       name, signature, &methodHandle);
    if (status != ANI_OK) {
        return status;
    }

    *vmethod = venv->GetVerifiedMethod(methodHandle);
    ASSERT(*vmethod != nullptr);
    return ANI_OK;
}

static ani_status ResolveVerifiedStaticMethodByName(VEnv *venv, VClass *vclass, const char *name, const char *signature,
                                                    VStaticMethod **vstaticmethod)
{
    ASSERT(vstaticmethod != nullptr);

    ani_static_method staticMethodHandle {};
    auto classRef = ResolveToAniClass(vclass);
    ani_status status =
        GetInteractionAPI(venv)->Class_FindStaticMethod(venv->GetEnv(), classRef, name, signature, &staticMethodHandle);
    if (status != ANI_OK) {
        return status;
    }

    *vstaticmethod = venv->GetVerifiedStaticMethod(staticMethodHandle);
    ASSERT(*vstaticmethod != nullptr);
    return ANI_OK;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GetVersion(VEnv *venv, uint32_t *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForU32Storage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    return GetInteractionAPI(venv)->GetVersion(venv->GetEnv(), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GetVM(VEnv *venv, VVm **result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForVmStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(result);
    ani_vm *vm {};
    ani_status status = GetInteractionAPI(venv)->GetVM(venv->GetEnv(), &vm);
    if (status == ANI_OK) {
        *result = VVm::GetInstance();
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_New(VEnv *venv, VClass *vclass, VMethod *vctor, VObject **vresult, ...)
{
    // NOTE: Validate ctor method before usage

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vresult);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vctor, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "cls"),
        ANIArg::MakeForMethod(vctor, "ctor", EtsType::VOID, true),
        ANIArg::MakeForObjectStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vctor);
    CHECK_PTR_ARG(vresult);
    auto args = GetVValueArgs(venv, methodVArgs);
    ani_object result {};
    auto classRef = ResolveToAniClass(vclass);
    auto ctorMethod = ResolveToAniMethod(vctor);
    ani_status status =
        GetInteractionAPI(venv)->Object_New_A(venv->GetEnv(), classRef, ctorMethod, &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_New_A(VEnv *venv, VClass *vclass, VMethod *vctor, VObject **vresult,
                                              const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vctor), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "cls"),
        ANIArg::MakeForMethod(vctor, "ctor", EtsType::VOID, true),
        ANIArg::MakeForObjectStorage(vresult, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on

    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vctor);
    CHECK_PTR_ARG(vresult);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_object result {};
    auto classRef = ResolveToAniClass(vclass);
    auto ctorMethod = ResolveToAniMethod(vctor);
    ani_status status =
        GetInteractionAPI(venv)->Object_New_A(venv->GetEnv(), classRef, ctorMethod, &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_New_V(VEnv *venv, VClass *vclass, VMethod *vctor, VObject **vresult,
                                              va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vctor, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "cls"),
        ANIArg::MakeForMethod(vctor, "ctor", EtsType::VOID, true),
        ANIArg::MakeForObjectStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on

    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vctor);
    CHECK_PTR_ARG(vresult);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_object result {};
    auto classRef = ResolveToAniClass(vclass);
    auto ctorMethod = ResolveToAniMethod(vctor);
    ani_status status =
        GetInteractionAPI(venv)->Object_New_A(venv->GetEnv(), classRef, ctorMethod, &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetType(VEnv *venv, VObject *vobject, VType **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForTypeStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vresult);
    ani_type typeResult {};
    auto objectRef = ResolveToAniRef(vobject);
    ani_status status = GetInteractionAPI(venv)->Object_GetType(venv->GetEnv(), objectRef, &typeResult);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, typeResult, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_InstanceOf(VEnv *venv, VObject *vobject, VType *vtype, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForType(vtype, "type"),
        ANIArg::MakeForBooleanStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vtype);

    auto objectRef = ResolveToAniRef(vobject);
    auto typeRef = ResolveToAniType(vtype);
    return GetInteractionAPI(venv)->Object_InstanceOf(venv->GetEnv(), objectRef, typeRef, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Type_GetSuperClass(VEnv *venv, VType *vtype, VClass **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForType(vtype, "type"),
        ANIArg::MakeForClassStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtype);
    CHECK_PTR_ARG(vresult);
    ani_class superClass {};
    auto typeRef = ResolveToAniType(vtype);
    ani_status status = GetInteractionAPI(venv)->Type_GetSuperClass(venv->GetEnv(), typeRef, &superClass);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, superClass, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Type_IsAssignableFrom(VEnv *venv, VType *vfromType, VType *vtoType,
                                                       ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForType(vfromType, "from_type"),
        ANIArg::MakeForType(vtoType, "to_type"),
        ANIArg::MakeForBooleanStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfromType);
    CHECK_PTR_ARG(vtoType);

    auto fromTypeRef = ResolveToAniType(vfromType);
    auto toTypeRef = ResolveToAniType(vtoType);
    return GetInteractionAPI(venv)->Type_IsAssignableFrom(venv->GetEnv(), fromTypeRef, toTypeRef, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FindModule(VEnv *venv, const char *moduleDescriptor, VModule **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForModuleDescriptor(moduleDescriptor, "module_descriptor"),
        ANIArg::MakeForModuleStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_module result {};
    ani_status status = GetInteractionAPI(venv)->FindModule(venv->GetEnv(), moduleDescriptor, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FindNamespace(VEnv *venv, const char *namespaceDescriptor, VNamespace **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForNamespaceDescriptor(namespaceDescriptor, "namespace_descriptor"),
        ANIArg::MakeForNamespaceStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_namespace result {};
    ani_status status = GetInteractionAPI(venv)->FindNamespace(venv->GetEnv(), namespaceDescriptor, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FindClass(VEnv *venv, const char *classDescriptor, VClass **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClassDescriptor(classDescriptor, "class_descriptor"),
        ANIArg::MakeForClassStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_class result {};
    ani_status status = GetInteractionAPI(venv)->FindClass(venv->GetEnv(), classDescriptor, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FindEnum(VEnv *venv, const char *enumDescriptor, VEnum **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForEnumDescriptor(enumDescriptor, "enum_descriptor"),
        ANIArg::MakeForEnumStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_enum result {};
    ani_status status = GetInteractionAPI(venv)->FindEnum(venv->GetEnv(), enumDescriptor, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_FindFunction(VEnv *venv, VModule *vmodule, const char *name,
                                                     const char *signature, VFunction **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForModule(vmodule, "module"),
        ANIArg::MakeForFindFunctionName(name, "name"),
        ANIArg::MakeForFindFunctionSignature(signature, "signature"),
        ANIArg::MakeForFunctionStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vmodule);
    CHECK_PTR_ARG(vresult);

    ani_function result {};
    ani_status status = GetInteractionAPI(venv)->Module_FindFunction(
        venv->GetEnv(), ResolveToAniModule(static_cast<VModule *>(vmodule)), name, signature, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedFunction(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_FindVariable(VEnv *venv, VModule *vmodule, const char *name,
                                                     VVariable **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForModule(vmodule, "module"),
        ANIArg::MakeForFindVariableName(name, "name"),
        ANIArg::MakeForVariableStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vmodule);
    CHECK_PTR_ARG(vresult);

    ani_variable result {};
    ani_status status = GetInteractionAPI(venv)->Module_FindVariable(
        venv->GetEnv(), ResolveToAniModule(static_cast<VModule *>(vmodule)), name, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedVariable(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_FindFunction(VEnv *venv, VNamespace *vnamespace, const char *name,
                                                        const char *signature, VFunction **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForNamespace(vnamespace, "ns"),
        ANIArg::MakeForFindFunctionName(name, "name"),
        ANIArg::MakeForFindFunctionSignature(signature, "signature"),
        ANIArg::MakeForFunctionStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vnamespace);
    CHECK_PTR_ARG(vresult);
    ani_function result {};
    ani_status status = GetInteractionAPI(venv)->Namespace_FindFunction(
        venv->GetEnv(), ResolveToAniNamespace(static_cast<VNamespace *>(vnamespace)), name, signature, &result);
    if (LIKELY(status == ANI_OK)) {
        *vresult = venv->GetVerifiedFunction(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_FindVariable(VEnv *venv, VNamespace *vnamespace, const char *name,
                                                        VVariable **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForNamespace(vnamespace, "ns"),
        ANIArg::MakeForFindVariableName(name, "name"),
        ANIArg::MakeForVariableStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vnamespace);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vresult);
    ani_variable result {};
    ani_status status = GetInteractionAPI(venv)->Namespace_FindVariable(
        venv->GetEnv(), ResolveToAniNamespace(static_cast<VNamespace *>(vnamespace)), name, &result);
    if (LIKELY(status == ANI_OK)) {
        *vresult = venv->GetVerifiedVariable(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Module_BindNativeFunctions(VEnv *venv, VModule *vmodule,
                                                            const ani_native_function *functions, ani_size nrFunctions)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForModule(vmodule, "module"),
        ANIArg::MakeForNativeFunctions(functions, nrFunctions, "functions"),
        ANIArg::MakeForSize(nrFunctions, "nr_functions")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vmodule);
    return GetInteractionAPI(venv)->Module_BindNativeFunctions(
        venv->GetEnv(), ResolveToAniModule(static_cast<VModule *>(vmodule)), functions, nrFunctions);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Namespace_BindNativeFunctions(VEnv *venv, VNamespace *vnamespace,
                                                               const ani_native_function *functions,
                                                               ani_size nrFunctions)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForNamespace(vnamespace, "ns"),
        ANIArg::MakeForNativeFunctions(functions, nrFunctions, "functions"),
        ANIArg::MakeForSize(nrFunctions, "nr_functions")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vnamespace);
    return GetInteractionAPI(venv)->Namespace_BindNativeFunctions(
        venv->GetEnv(), ResolveToAniNamespace(static_cast<VNamespace *>(vnamespace)), functions, nrFunctions);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_BindNativeMethods(VEnv *venv, VClass *vclass, const ani_native_function *methods,
                                                         ani_size nrMethods)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForNativeMethods(methods, nrMethods, "methods"),
        ANIArg::MakeForSize(nrMethods, "nr_methods")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    auto classRef = ResolveToAniClass(vclass);
    return GetInteractionAPI(venv)->Class_BindNativeMethods(venv->GetEnv(), classRef, methods, nrMethods);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_Delete(VEnv *venv, VRef *lvref)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForDelLocalRef(lvref, "lref"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(lvref);

    auto *envVerifier = PandaAniEnv::FromAniEnv(venv->GetEnv())->GetEnvANIVerifier();
    bool isRawLocalRef = envVerifier->IsValidRawAniLocalRef(reinterpret_cast<void *>(lvref));

    if (!venv->IsValidLocalVerifiedRef(lvref) && !venv->IsValidStackVerifiedRef(lvref) &&
        !ManagedCodeAccessor::IsUndefined(ResolveToAniRef(lvref)) && !isRawLocalRef) {
        return ANI_INCORRECT_REF;
    }
    auto lref = ResolveToAniRef(lvref);
    if (!isRawLocalRef) {
        venv->DeleteLocalVerifiedRef(lvref);
    }
    ani_status status = GetInteractionAPI(venv)->Reference_Delete(venv->GetEnv(), lref);
    if (status == ANI_OK && isRawLocalRef) {
        envVerifier->RemoveGrandfatheredRawLocalRef(reinterpret_cast<void *>(lvref));
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnsureEnoughReferences(VEnv *venv, ani_size nrRefs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForNrRefs(nrRefs, "nrRefs"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);

    return GetInteractionAPI(venv)->EnsureEnoughReferences(venv->GetEnv(), nrRefs);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status CreateLocalScope(VEnv *venv, ani_size nrRefs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForNrRefs(nrRefs, "nr_refs")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);

    ani_status status = GetInteractionAPI(venv)->CreateLocalScope(venv->GetEnv(), nrRefs);
    if (LIKELY(status == ANI_OK)) {
        venv->CreateLocalScope(nrRefs);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status DestroyLocalScope(VEnv *venv)
{
    // NOTE: We should simultaneously verify args and check possibility of call this method
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);

    VERIFY_ANI_ABORT_IF_ERROR(venv->DestroyLocalScope());
    return GetInteractionAPI(venv)->DestroyLocalScope(venv->GetEnv());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status CreateEscapeLocalScope(VEnv *venv, ani_size nrRefs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForNrRefs(nrRefs, "nr_refs")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);

    ani_status status = GetInteractionAPI(venv)->CreateEscapeLocalScope(venv->GetEnv(), nrRefs);
    if (LIKELY(status == ANI_OK)) {
        venv->CreateEscapeLocalScope(nrRefs);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status DestroyEscapeLocalScope(VEnv *venv, VRef *vref, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForEscapeRef(vref, "ref"),
        ANIArg::MakeForRefStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(vresult);
    if (!venv->IsValidLocalVerifiedRef(vref)) {
        return ANI_INCORRECT_REF;
    }
    auto ref = ResolveToAniRef(vref);
    VERIFY_ANI_ABORT_IF_ERROR(venv->DestroyEscapeLocalScope(vref));
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->DestroyEscapeLocalScope(venv->GetEnv(), ref, &result);
    if (LIKELY(status == ANI_OK)) {
        *vresult = venv->AddLocalVerifiedRef(result);
    } else {
        // NOTE: Handle this case
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status ThrowError(VEnv *venv, VError *verr)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForError(verr, "error")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(verr);
    auto errorRef = ResolveToAniRef(verr);
    return GetInteractionAPI(venv)->ThrowError(venv->GetEnv(), errorRef);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status ExistUnhandledError(VEnv *venv, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->ExistUnhandledError(venv->GetEnv(), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GetUnhandledError(VEnv *venv, VError **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForErrorStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_error result {};
    ani_status status = GetInteractionAPI(venv)->GetUnhandledError(venv->GetEnv(), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status ResetError(VEnv *venv)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);

    return GetInteractionAPI(venv)->ResetError(venv->GetEnv());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status DescribeError(VEnv *venv)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);

    return GetInteractionAPI(venv)->DescribeError(venv->GetEnv());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Abort(VEnv *venv, const char *message)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForUTF8String(message, "message")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(message);

    return GetInteractionAPI(venv)->Abort(venv->GetEnv(), message);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GetNull(VEnv *venv, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->GetNull(venv->GetEnv(), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GetUndefined(VEnv *venv, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->GetUndefined(venv->GetEnv(), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_IsNull(VEnv *venv, VRef *vref, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRef(vref, "ref"),
        ANIArg::MakeForBooleanStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    auto ref = ResolveToAniRef(vref);
    return GetInteractionAPI(venv)->Reference_IsNull(venv->GetEnv(), ref, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_IsUndefined(VEnv *venv, VRef *vref, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRef(vref, "ref"),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    auto ref = ResolveToAniRef(vref);
    return GetInteractionAPI(venv)->Reference_IsUndefined(venv->GetEnv(), ref, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_IsNullishValue(VEnv *venv, VRef *vref, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRef(vref, "ref"),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    auto ref = ResolveToAniRef(vref);
    return GetInteractionAPI(venv)->Reference_IsNullishValue(venv->GetEnv(), ref, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_Equals(VEnv *venv, VRef *vref0, VRef *vref1, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRef(vref0, "ref0"),
        ANIArg::MakeForRef(vref1, "ref1"),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref0);
    CHECK_PTR_ARG(vref1);

    return GetInteractionAPI(venv)->Reference_Equals(venv->GetEnv(), ResolveToAniRef(vref0), ResolveToAniRef(vref1),
                                                     result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Reference_StrictEquals(VEnv *venv, VRef *vref0, VRef *vref1, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRef(vref0, "ref0"),
        ANIArg::MakeForRef(vref1, "ref1"),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref0);
    CHECK_PTR_ARG(vref1);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Reference_StrictEquals(venv->GetEnv(), ResolveToAniRef(vref0),
                                                           ResolveToAniRef(vref1), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_NewUTF16(VEnv *venv, const uint16_t *utf16String, ani_size utf16Size,
                                                 VString **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForUTF16String(utf16String, "utf16String"),
        ANIArg::MakeForSize(utf16Size, "utf16Size"),
        ANIArg::MakeForStringStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(utf16String);
    CHECK_PTR_ARG(vresult);

    ani_string result {};
    ani_status status = GetInteractionAPI(venv)->String_NewUTF16(venv->GetEnv(), utf16String, utf16Size, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF16Size(VEnv *venv, VString *vstring, ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForString(vstring, "string"),
        ANIArg::MakeForSizeStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vstring);
    CHECK_PTR_ARG(result);

    auto stringRef = ResolveToAniRef(vstring);
    return GetInteractionAPI(venv)->String_GetUTF16Size(venv->GetEnv(), stringRef, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF16(VEnv *venv, VString *vstring, uint16_t *utf16Buffer,
                                                 ani_size utf16BufferSize, ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForString(vstring, "string"),
        ANIArg::MakeForUTF16Buffer(utf16Buffer, "utf16Buffer"),
        ANIArg::MakeForSize(utf16BufferSize, "utf16BufferSize"),
        ANIArg::MakeForSizeStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vstring);
    CHECK_PTR_ARG(utf16Buffer);
    CHECK_PTR_ARG(result);

    auto stringRef = ResolveToAniRef(vstring);
    return GetInteractionAPI(venv)->String_GetUTF16(venv->GetEnv(), stringRef, utf16Buffer, utf16BufferSize, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF16SubString(VEnv *venv, VString *vstring, ani_size substrOffset,
                                                          ani_size substrSize, uint16_t *utf16Buffer,
                                                          ani_size utf16BufferSize, ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForString(vstring, "string"),
        ANIArg::MakeForSize(substrOffset, "substrOffset"),
        ANIArg::MakeForSize(substrSize, "substrSize"),
        ANIArg::MakeForUTF16Buffer(utf16Buffer, "utf16Buffer"),
        ANIArg::MakeForSizeStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vstring);
    auto stringRef = ResolveToAniRef(vstring);
    return GetInteractionAPI(venv)->String_GetUTF16SubString(venv->GetEnv(), stringRef, substrOffset, substrSize,
                                                             utf16Buffer, utf16BufferSize, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_NewUTF8(VEnv *venv, const char *utf8String, ani_size utf8Size,
                                                VString **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForUTF8String(utf8String, "utf8String"),
        ANIArg::MakeForSize(utf8Size, "utf8Size"),
        ANIArg::MakeForStringStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(utf8String);
    CHECK_PTR_ARG(vresult);

    ani_string result {};
    ani_status status = GetInteractionAPI(venv)->String_NewUTF8(venv->GetEnv(), utf8String, utf8Size, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF8Size(VEnv *venv, VString *vstring, ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForString(vstring, "string"),
        ANIArg::MakeForSizeStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vstring);
    CHECK_PTR_ARG(result);

    auto stringRef = ResolveToAniRef(vstring);
    return GetInteractionAPI(venv)->String_GetUTF8Size(venv->GetEnv(), stringRef, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF8(VEnv *venv, VString *vstring, char *utf8Buffer, ani_size utf8BufferSize,
                                                ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForString(vstring, "string"),
        ANIArg::MakeForUTF8Buffer(utf8Buffer, "utf8Buffer"),
        ANIArg::MakeForSize(utf8BufferSize, "utf8BufferSize"),
        ANIArg::MakeForSizeStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vstring);
    CHECK_PTR_ARG(utf8Buffer);
    CHECK_PTR_ARG(result);

    auto stringRef = ResolveToAniRef(vstring);
    return GetInteractionAPI(venv)->String_GetUTF8(venv->GetEnv(), stringRef, utf8Buffer, utf8BufferSize, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status String_GetUTF8SubString(VEnv *venv, VString *vstring, ani_size substrOffset,
                                                         ani_size substrSize, char *utf8Buffer, ani_size utf8BufferSize,
                                                         ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForString(vstring, "string"),
        ANIArg::MakeForSize(substrOffset, "substrOffset"),
        ANIArg::MakeForSize(substrSize, "substrSize"),
        ANIArg::MakeForUTF8Buffer(utf8Buffer, "utf8Buffer"),
        ANIArg::MakeForSizeStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vstring);
    CHECK_PTR_ARG(utf8Buffer);
    CHECK_PTR_ARG(result);

    auto stringRef = ResolveToAniRef(vstring);
    return GetInteractionAPI(venv)->String_GetUTF8SubString(venv->GetEnv(), stringRef, substrOffset, substrSize,
                                                            utf8Buffer, utf8BufferSize, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_GetLength(VEnv *venv, VArray *varray, ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForArray(varray, "array"),
        ANIArg::MakeForSizeStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    CHECK_PTR_ARG(result);

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->Array_GetLength(venv->GetEnv(), arrayRef, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_New(VEnv *venv, ani_size length, VRef *vinitialElement, VArray **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForArrayCreateLength(length, "length"),
        ANIArg::MakeForArrayInitialRef(vinitialElement, "initial_element"),
        ANIArg::MakeForArrayStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    if (LIKELY(length != 0)) {
        CHECK_PTR_ARG(vinitialElement);
    }
    CHECK_PTR_ARG(vresult);

    ani_array result {};
    ani_status status =
        GetInteractionAPI(venv)->Array_New(venv->GetEnv(), length, ResolveToAniRef(vinitialElement), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_Set(VEnv *venv, VArray *varray, ani_size index, VRef *vref)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForArray(varray, "array"),
        ANIArg::MakeForSize(index, "index"),
        ANIArg::MakeForRef(vref, "ref")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    CHECK_PTR_ARG(vref);

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->Array_Set(venv->GetEnv(), arrayRef, index, ResolveToAniRef(vref));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_Get(VEnv *venv, VArray *varray, ani_size index, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForArray(varray, "array"),
        ANIArg::MakeForSize(index, "index"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    auto arrayRef = ResolveToAniRef(varray);
    ani_status status = GetInteractionAPI(venv)->Array_Get(venv->GetEnv(), arrayRef, index, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_Push(VEnv *venv, VArray *varray, VRef *vref)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForArray(varray, "array"),
        ANIArg::MakeForRef(vref, "ref")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    CHECK_PTR_ARG(vref);

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->Array_Push(venv->GetEnv(), arrayRef, ResolveToAniRef(vref));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Array_Pop(VEnv *venv, VArray *varray, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForArray(varray, "array"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    auto arrayRef = ResolveToAniRef(varray);
    ani_status status = GetInteractionAPI(venv)->Array_Pop(venv->GetEnv(), arrayRef, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetLength(VEnv *venv, VFixedArray *varray, ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArray(varray, "array"),
        ANIArg::MakeForSizeStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetLength(venv->GetEnv(), arrayRef, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Boolean(VEnv *venv, ani_size length, VFixedArrayBoolean **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForArrayBooleanStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_fixedarray_boolean result {};
    ani_status status = GetInteractionAPI(venv)->FixedArray_New_Boolean(venv->GetEnv(), length, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Char(VEnv *venv, ani_size length, VFixedArrayChar **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForArrayCharStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_fixedarray_char result {};
    ani_status status = GetInteractionAPI(venv)->FixedArray_New_Char(venv->GetEnv(), length, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Byte(VEnv *venv, ani_size length, VFixedArrayByte **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForArrayByteStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_fixedarray_byte result {};
    ani_status status = GetInteractionAPI(venv)->FixedArray_New_Byte(venv->GetEnv(), length, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Short(VEnv *venv, ani_size length, VFixedArrayShort **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForArrayShortStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_fixedarray_short result {};
    ani_status status = GetInteractionAPI(venv)->FixedArray_New_Short(venv->GetEnv(), length, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Int(VEnv *venv, ani_size length, VFixedArrayInt **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForArrayIntStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_fixedarray_int result {};
    ani_status status = GetInteractionAPI(venv)->FixedArray_New_Int(venv->GetEnv(), length, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Long(VEnv *venv, ani_size length, VFixedArrayLong **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForArrayLongStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_fixedarray_long result {};
    ani_status status = GetInteractionAPI(venv)->FixedArray_New_Long(venv->GetEnv(), length, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Float(VEnv *venv, ani_size length, VFixedArrayFloat **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForArrayFloatStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_fixedarray_float result {};
    ani_status status = GetInteractionAPI(venv)->FixedArray_New_Float(venv->GetEnv(), length, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Double(VEnv *venv, ani_size length, VFixedArrayDouble **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForArrayDoubleStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);

    ani_fixedarray_double result {};
    ani_status status = GetInteractionAPI(venv)->FixedArray_New_Double(venv->GetEnv(), length, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Boolean(VEnv *venv, VFixedArrayBoolean *varray, ani_size offset,
                                                              ani_size length, ani_boolean *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayBoolean(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetRegion_Boolean(venv->GetEnv(), arrayRef, offset, length,
                                                                 nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Char(VEnv *venv, VFixedArrayChar *varray, ani_size offset,
                                                           ani_size length, ani_char *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayChar(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetRegion_Char(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Byte(VEnv *venv, VFixedArrayByte *varray, ani_size offset,
                                                           ani_size length, ani_byte *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayByte(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetRegion_Byte(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Short(VEnv *venv, VFixedArrayShort *varray, ani_size offset,
                                                            ani_size length, ani_short *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayShort(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetRegion_Short(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Int(VEnv *venv, VFixedArrayInt *varray, ani_size offset,
                                                          ani_size length, ani_int *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayInt(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetRegion_Int(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Long(VEnv *venv, VFixedArrayLong *varray, ani_size offset,
                                                           ani_size length, ani_long *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayLong(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetRegion_Long(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Float(VEnv *venv, VFixedArrayFloat *varray, ani_size offset,
                                                            ani_size length, ani_float *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayFloat(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetRegion_Float(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_GetRegion_Double(VEnv *venv, VFixedArrayDouble *varray, ani_size offset,
                                                             ani_size length, ani_double *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayDouble(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_GetRegion_Double(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Boolean(VEnv *venv, VFixedArrayBoolean *varray, ani_size offset,
                                                              ani_size length, const ani_boolean *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayBoolean(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
        VERIFY_ANI_ABORT_IF_ERROR(VerifyBooleanRegionBufferValues(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_SetRegion_Boolean(venv->GetEnv(), arrayRef, offset, length,
                                                                 nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Char(VEnv *venv, VFixedArrayChar *varray, ani_size offset,
                                                           ani_size length, const ani_char *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayChar(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_SetRegion_Char(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Byte(VEnv *venv, VFixedArrayByte *varray, ani_size offset,
                                                           ani_size length, const ani_byte *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayByte(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_SetRegion_Byte(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Short(VEnv *venv, VFixedArrayShort *varray, ani_size offset,
                                                            ani_size length, const ani_short *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayShort(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_SetRegion_Short(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Int(VEnv *venv, VFixedArrayInt *varray, ani_size offset,
                                                          ani_size length, const ani_int *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayInt(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_SetRegion_Int(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Long(VEnv *venv, VFixedArrayLong *varray, ani_size offset,
                                                           ani_size length, const ani_long *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayLong(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_SetRegion_Long(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Float(VEnv *venv, VFixedArrayFloat *varray, ani_size offset,
                                                            ani_size length, const ani_float *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayFloat(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_SetRegion_Float(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_SetRegion_Double(VEnv *venv, VFixedArrayDouble *varray, ani_size offset,
                                                             ani_size length, const ani_double *nativeBuffer)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayDouble(varray, "array"),
        ANIArg::MakeForSize(offset, "offset"),
        ANIArg::MakeForSize(length, "length"),
        ANIArg::MakeForRegionBuffer(nativeBuffer, length, "native_buffer")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    if (!IsFixedArrayRegionOutOfRange(venv, varray, offset, length)) {
        VERIFY_ANI_ABORT_IF_ERROR(VerifyRegionBufferSpan(nativeBuffer, length));
    }

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_SetRegion_Double(venv->GetEnv(), arrayRef, offset, length, nativeBuffer);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_New_Ref(VEnv *venv, VType *vtype, ani_size length, VRef *vinitialElement,
                                                    VFixedArrayRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForType(vtype, "type"),
        ANIArg::MakeForArrayCreateLength(length, "length"),
        ANIArg::MakeForFixedArrayInitialRef(vinitialElement, "initial_element"),
        ANIArg::MakeForArrayRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtype);
    ani_ref initialElement = nullptr;
    if (LIKELY(length != 0)) {
        CHECK_PTR_ARG(vinitialElement);
        initialElement = ResolveToAniRef(vinitialElement);
    }
    CHECK_PTR_ARG(vresult);
    ani_fixedarray_ref result {};
    auto typeRef = ResolveToAniType(vtype);
    ani_status status =
        GetInteractionAPI(venv)->FixedArray_New_Ref(venv->GetEnv(), typeRef, length, initialElement, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_Set_Ref(VEnv *venv, VFixedArrayRef *varray, ani_size index, VRef *vref)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayRef(varray, "array"),
        ANIArg::MakeForSize(index, "index"),
        ANIArg::MakeForFixedArraySetRef(vref, "ref")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    CHECK_PTR_ARG(vref);

    auto arrayRef = ResolveToAniRef(varray);
    return GetInteractionAPI(venv)->FixedArray_Set_Ref(venv->GetEnv(), arrayRef, index, ResolveToAniRef(vref));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FixedArray_Get_Ref(VEnv *venv, VFixedArrayRef *varray, ani_size index, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFixedArrayRef(varray, "array"),
        ANIArg::MakeForSize(index, "index"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varray);
    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    auto arrayRef = ResolveToAniRef(varray);
    ani_status status = GetInteractionAPI(venv)->FixedArray_Get_Ref(venv->GetEnv(), arrayRef, index, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Enum_GetEnumItemByName(VEnv *venv, VEnum *venum, const char *name, VEnumItem **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForEnum(venum, "enm"),
        ANIArg::MakeForUTF8String(name, "name"),
        ANIArg::MakeForEnumItemStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(venum);
    CHECK_PTR_ARG(vresult);
    ani_enum_item result {};
    auto enumRef = ResolveToAniEnum(venum);
    ani_status status = GetInteractionAPI(venv)->Enum_GetEnumItemByName(venv->GetEnv(), enumRef, name, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Enum_GetEnumItemByIndex(VEnv *venv, VEnum *venum, ani_size index, VEnumItem **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForEnum(venum, "enm"),
        ANIArg::MakeForSize(index, "index"),
        ANIArg::MakeForEnumItemStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(venum);
    CHECK_PTR_ARG(vresult);
    ani_enum_item result {};
    ani_status status =
        GetInteractionAPI(venv)->Enum_GetEnumItemByIndex(venv->GetEnv(), ResolveToAniEnum(venum), index, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetEnum(VEnv *venv, VEnumItem *venumItem, VEnum **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForEnumItem(venumItem, "enum_item"),
        ANIArg::MakeForEnumStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(venumItem);
    CHECK_PTR_ARG(vresult);
    ani_enum result {};
    auto enumItemRef = ResolveToAniEnumItem(venumItem);
    ani_status status = GetInteractionAPI(venv)->EnumItem_GetEnum(venv->GetEnv(), enumItemRef, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetValue_Int(VEnv *venv, VEnumItem *venumItem, ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForEnumItem(venumItem, "enum_item"),
        ANIArg::MakeForIntStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(venumItem);

    auto enumItemRef = ResolveToAniEnumItem(venumItem);
    return GetInteractionAPI(venv)->EnumItem_GetValue_Int(venv->GetEnv(), enumItemRef, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetValue_String(VEnv *venv, VEnumItem *venumItem, VString **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForEnumItem(venumItem, "enum_item"),
        ANIArg::MakeForStringStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(venumItem);
    CHECK_PTR_ARG(vresult);
    ani_string result {};
    auto enumItemRef = ResolveToAniEnumItem(venumItem);
    ani_status status = GetInteractionAPI(venv)->EnumItem_GetValue_String(venv->GetEnv(), enumItemRef, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetName(VEnv *venv, VEnumItem *venumItem, VString **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForEnumItem(venumItem, "enum_item"),
        ANIArg::MakeForStringStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(venumItem);
    CHECK_PTR_ARG(vresult);
    ani_string result {};
    auto enumItemRef = ResolveToAniEnumItem(venumItem);
    ani_status status = GetInteractionAPI(venv)->EnumItem_GetName(venv->GetEnv(), enumItemRef, &result);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status EnumItem_GetIndex(VEnv *venv, VEnumItem *venumItem, ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForEnumItem(venumItem, "enum_item"),
        ANIArg::MakeForSizeStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(venumItem);

    auto enumItemRef = ResolveToAniEnumItem(venumItem);
    return GetInteractionAPI(venv)->EnumItem_GetIndex(venv->GetEnv(), enumItemRef, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status FunctionalObject_Call(VEnv *venv, VFnObject *vfnObject, ani_size argc, ani_ref *argv,
                                                       VRef **vresult)
{
    ANIArg::AniFunctionalObjectArgv argvArgs {argc, argv, {}, argv};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunctionalObject(vfnObject, "fn"),
        ANIArg::MakeForSize(argc, "argc"),
        ANIArg::MakeForFunctionalObjectArgv(&argvArgs, "argv"),
        ANIArg::MakeForRefStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfnObject);
    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->FunctionalObject_Call(venv->GetEnv(), ResolveToAniRef(vfnObject), argc,
                                                                       argvArgs.releaseArgv, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Boolean(VEnv *venv, VVariable *vvariable, ani_boolean value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::BOOLEAN, AccessMode::READWRITE),
        ANIArg::MakeForBoolean(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);

    return GetInteractionAPI(venv)->Variable_SetValue_Boolean(venv->GetEnv(), ResolveToAniVariable(vvariable), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Char(VEnv *venv, VVariable *vvariable, ani_char value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::CHAR, AccessMode::READWRITE),
        ANIArg::MakeForChar(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);

    return GetInteractionAPI(venv)->Variable_SetValue_Char(venv->GetEnv(), ResolveToAniVariable(vvariable), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Byte(VEnv *venv, VVariable *vvariable, ani_byte value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::BYTE, AccessMode::READWRITE),
        ANIArg::MakeForByte(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);

    return GetInteractionAPI(venv)->Variable_SetValue_Byte(venv->GetEnv(), ResolveToAniVariable(vvariable), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Short(VEnv *venv, VVariable *vvariable, ani_short value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::SHORT, AccessMode::READWRITE),
        ANIArg::MakeForShort(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);

    return GetInteractionAPI(venv)->Variable_SetValue_Short(venv->GetEnv(), ResolveToAniVariable(vvariable), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Int(VEnv *venv, VVariable *vvariable, ani_int value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::INT, AccessMode::READWRITE),
        ANIArg::MakeForInt(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);

    return GetInteractionAPI(venv)->Variable_SetValue_Int(venv->GetEnv(), ResolveToAniVariable(vvariable), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Long(VEnv *venv, VVariable *vvariable, ani_long value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::LONG, AccessMode::READWRITE),
        ANIArg::MakeForLong(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);

    return GetInteractionAPI(venv)->Variable_SetValue_Long(venv->GetEnv(), ResolveToAniVariable(vvariable), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Float(VEnv *venv, VVariable *vvariable, ani_float value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::FLOAT, AccessMode::READWRITE),
        ANIArg::MakeForFloat(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);

    return GetInteractionAPI(venv)->Variable_SetValue_Float(venv->GetEnv(), ResolveToAniVariable(vvariable), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Double(VEnv *venv, VVariable *vvariable, ani_double value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::DOUBLE, AccessMode::READWRITE),
        ANIArg::MakeForDouble(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);

    return GetInteractionAPI(venv)->Variable_SetValue_Double(venv->GetEnv(), ResolveToAniVariable(vvariable), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_SetValue_Ref(VEnv *venv, VVariable *vvariable, VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::OBJECT, AccessMode::READWRITE),
        ANIArg::MakeForRef(vvalue, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(vvalue);

    return GetInteractionAPI(venv)->Variable_SetValue_Ref(venv->GetEnv(), ResolveToAniVariable(vvariable),
                                                          ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Boolean(VEnv *venv, VVariable *vvariable, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::BOOLEAN, AccessMode::READ),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Variable_GetValue_Boolean(venv->GetEnv(), ResolveToAniVariable(vvariable), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Char(VEnv *venv, VVariable *vvariable, ani_char *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::CHAR, AccessMode::READ),
        ANIArg::MakeForCharStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Variable_GetValue_Char(venv->GetEnv(), ResolveToAniVariable(vvariable), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Byte(VEnv *venv, VVariable *vvariable, ani_byte *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::BYTE, AccessMode::READ),
        ANIArg::MakeForByteStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Variable_GetValue_Byte(venv->GetEnv(), ResolveToAniVariable(vvariable), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Short(VEnv *venv, VVariable *vvariable, ani_short *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::SHORT, AccessMode::READ),
        ANIArg::MakeForShortStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Variable_GetValue_Short(venv->GetEnv(), ResolveToAniVariable(vvariable), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Int(VEnv *venv, VVariable *vvariable, ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::INT, AccessMode::READ),
        ANIArg::MakeForIntStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Variable_GetValue_Int(venv->GetEnv(), ResolveToAniVariable(vvariable), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Long(VEnv *venv, VVariable *vvariable, ani_long *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::LONG, AccessMode::READ),
        ANIArg::MakeForLongStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Variable_GetValue_Long(venv->GetEnv(), ResolveToAniVariable(vvariable), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Float(VEnv *venv, VVariable *vvariable, ani_float *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::FLOAT, AccessMode::READ),
        ANIArg::MakeForFloatStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Variable_GetValue_Float(venv->GetEnv(), ResolveToAniVariable(vvariable), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Double(VEnv *venv, VVariable *vvariable, ani_double *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::DOUBLE, AccessMode::READ),
        ANIArg::MakeForDoubleStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Variable_GetValue_Double(venv->GetEnv(), ResolveToAniVariable(vvariable), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Variable_GetValue_Ref(VEnv *venv, VVariable *vvariable, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForVariable(vvariable, "variable", EtsType::OBJECT, AccessMode::READ),
        ANIArg::MakeForRefStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vvariable);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status =
        GetInteractionAPI(venv)->Variable_GetValue_Ref(venv->GetEnv(), ResolveToAniVariable(vvariable), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Boolean(VEnv *venv, VFunction *vfn, ani_boolean *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Function_Call_Boolean_A(venv->GetEnv(), ResolveToAniFunction(vfn),
                                                                         result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Boolean_A(VEnv *venv, VFunction *vfn, ani_boolean *result,
                                                         const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Function_Call_Boolean_A(venv->GetEnv(), ResolveToAniFunction(vfn),
                                                                         result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Boolean_V(VEnv *venv, VFunction *vfn, ani_boolean *result,
                                                         va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Function_Call_Boolean_A(venv->GetEnv(), ResolveToAniFunction(vfn),
                                                                         result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Char(VEnv *venv, VFunction *vfn, ani_char *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Char_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Char_A(VEnv *venv, VFunction *vfn, ani_char *result,
                                                      const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Char_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Char_V(VEnv *venv, VFunction *vfn, ani_char *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Char_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Byte(VEnv *venv, VFunction *vfn, ani_byte *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Byte_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Byte_A(VEnv *venv, VFunction *vfn, ani_byte *result,
                                                      const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Byte_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Byte_V(VEnv *venv, VFunction *vfn, ani_byte *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Byte_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Short(VEnv *venv, VFunction *vfn, ani_short *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Short_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Short_A(VEnv *venv, VFunction *vfn, ani_short *result,
                                                       const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Short_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Short_V(VEnv *venv, VFunction *vfn, ani_short *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Short_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Int(VEnv *venv, VFunction *vfn, ani_int *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Int_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Int_A(VEnv *venv, VFunction *vfn, ani_int *result,
                                                     const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Int_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Int_V(VEnv *venv, VFunction *vfn, ani_int *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Int_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Long(VEnv *venv, VFunction *vfn, ani_long *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Long_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Long_A(VEnv *venv, VFunction *vfn, ani_long *result,
                                                      const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Long_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Long_V(VEnv *venv, VFunction *vfn, ani_long *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Long_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Float(VEnv *venv, VFunction *vfn, ani_float *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Float_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Float_A(VEnv *venv, VFunction *vfn, ani_float *result,
                                                       const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Float_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Float_V(VEnv *venv, VFunction *vfn, ani_float *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Float_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Double(VEnv *venv, VFunction *vfn, ani_double *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Double_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Double_A(VEnv *venv, VFunction *vfn, ani_double *result,
                                                        const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Double_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Double_V(VEnv *venv, VFunction *vfn, ani_double *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Double_A(venv->GetEnv(), ResolveToAniFunction(vfn), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Ref(VEnv *venv, VFunction *vfn, VRef **vresult, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vresult);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(vresult);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_ref result {};
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Ref_A(venv->GetEnv(), ResolveToAniFunction(vfn), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Ref_A(VEnv *venv, VFunction *vfn, VRef **vresult, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(vresult);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_ref result {};
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Ref_A(venv->GetEnv(), ResolveToAniFunction(vfn), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Ref_V(VEnv *venv, VFunction *vfn, VRef **vresult, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(vresult);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_ref result {};
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Ref_A(venv->GetEnv(), ResolveToAniFunction(vfn), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Void(VEnv *venv, VFunction *vfn, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vfn);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::VOID),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Void_A(venv->GetEnv(), ResolveToAniFunction(vfn), args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Void_A(VEnv *venv, VFunction *vfn, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vfn), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::VOID),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Void_A(venv->GetEnv(), ResolveToAniFunction(vfn), args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Function_Call_Void_V(VEnv *venv, VFunction *vfn, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vfn, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFunction(vfn, "function", EtsType::VOID),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfn);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status =
        GetInteractionAPI(venv)->Function_Call_Void_A(venv->GetEnv(), ResolveToAniFunction(vfn), args.data());
    return status;
}

/// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindField(VEnv *venv, VClass *vclass, const char *name, VField **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForFindFieldName(name, "name"),
        ANIArg::MakeForFieldStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vresult);

    ani_field result {};

    ani_status status =
        GetInteractionAPI(venv)->Class_FindField(venv->GetEnv(), ResolveToAniClass(vclass), name, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedField(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindStaticField(VEnv *venv, VClass *vclass, const char *name,
                                                       VStaticField **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForFindStaticFieldName(name, "name"),
        ANIArg::MakeForStaticFieldStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vresult);

    ani_static_field result {};
    ani_status status =
        GetInteractionAPI(venv)->Class_FindStaticField(venv->GetEnv(), ResolveToAniClass(vclass), name, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedStaticField(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindMethod(VEnv *venv, VClass *vclass, const char *name, const char *signature,
                                                  VMethod **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForFindMethodSignature(signature, "signature"),
        ANIArg::MakeForMethodStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vresult);

    ani_method result {};
    ani_status status =
        GetInteractionAPI(venv)->Class_FindMethod(venv->GetEnv(), ResolveToAniClass(vclass), name, signature, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedMethod(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindStaticMethod(VEnv *venv, VClass *vclass, const char *name,
                                                        const char *signature, VStaticMethod **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForFindStaticMethodSignature(signature, "signature"),
        ANIArg::MakeForStaticMethodStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(name);

    CHECK_PTR_ARG(vresult);

    ani_static_method result {};
    ani_status status = GetInteractionAPI(venv)->Class_FindStaticMethod(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedStaticMethod(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindSetter(VEnv *venv, VClass *vclass, const char *name, VMethod **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForFindSetterName(name, "name"),
        ANIArg::MakeForMethodStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vresult);

    ani_method result {};
    ani_status status =
        GetInteractionAPI(venv)->Class_FindSetter(venv->GetEnv(), ResolveToAniClass(vclass), name, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedMethod(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindGetter(VEnv *venv, VClass *vclass, const char *name, VMethod **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForFindGetterName(name, "name"),
        ANIArg::MakeForMethodStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vresult);

    ani_method result {};
    ani_status status =
        GetInteractionAPI(venv)->Class_FindGetter(venv->GetEnv(), ResolveToAniClass(vclass), name, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedMethod(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindIndexableGetter(VEnv *venv, VClass *vclass, const char *signature,
                                                           VMethod **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForFindIndexableGetterSignature(signature, "signature"),
        ANIArg::MakeForMethodStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);

    CHECK_PTR_ARG(vresult);

    ani_method result {};
    ani_status status = GetInteractionAPI(venv)->Class_FindIndexableGetter(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                           signature, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedMethod(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindIndexableSetter(VEnv *venv, VClass *vclass, const char *signature,
                                                           VMethod **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForFindIndexableSetterSignature(signature, "signature"),
        ANIArg::MakeForMethodStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);

    CHECK_PTR_ARG(vresult);

    ani_method result {};
    ani_status status = GetInteractionAPI(venv)->Class_FindIndexableSetter(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                           signature, &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedMethod(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_FindIterator(VEnv *venv, VClass *vclass, VMethod **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFindIterator(vclass, "class"),
        ANIArg::MakeForMethodStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vresult);

    ani_method result {};
    ani_status status = GetInteractionAPI(venv)->Class_FindIterator(venv->GetEnv(), ResolveToAniClass(vclass), &result);
    if (LIKELY((status) == ANI_OK)) {
        *vresult = venv->GetVerifiedMethod(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Boolean(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                              ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::BOOLEAN, AccessMode::READ),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticField_Boolean(venv->GetEnv(), ResolveToAniClass(vcls),
                                                                 ResolveToAniStaticField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Char(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                           ani_char *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::CHAR, AccessMode::READ),
        ANIArg::MakeForCharStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticField_Char(venv->GetEnv(), ResolveToAniClass(vcls),
                                                              ResolveToAniStaticField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Byte(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                           ani_byte *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::BYTE, AccessMode::READ),
        ANIArg::MakeForByteStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticField_Byte(venv->GetEnv(), ResolveToAniClass(vcls),
                                                              ResolveToAniStaticField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Short(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                            ani_short *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::SHORT, AccessMode::READ),
        ANIArg::MakeForShortStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticField_Short(venv->GetEnv(), ResolveToAniClass(vcls),
                                                               ResolveToAniStaticField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Int(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                          ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::INT, AccessMode::READ),
        ANIArg::MakeForIntStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticField_Int(venv->GetEnv(), ResolveToAniClass(vcls),
                                                             ResolveToAniStaticField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Long(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                           ani_long *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::LONG, AccessMode::READ),
        ANIArg::MakeForLongStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticField_Long(venv->GetEnv(), ResolveToAniClass(vcls),
                                                              ResolveToAniStaticField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Float(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                            ani_float *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::FLOAT, AccessMode::READ),
        ANIArg::MakeForFloatStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticField_Float(venv->GetEnv(), ResolveToAniClass(vcls),
                                                               ResolveToAniStaticField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Double(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                             ani_double *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::DOUBLE, AccessMode::READ),
        ANIArg::MakeForDoubleStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticField_Double(venv->GetEnv(), ResolveToAniClass(vcls),
                                                                ResolveToAniStaticField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticField_Ref(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                          VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::OBJECT, AccessMode::READ),
        ANIArg::MakeForRefStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    auto status = GetInteractionAPI(venv)->Class_GetStaticField_Ref(venv->GetEnv(), ResolveToAniClass(vcls),
                                                                    ResolveToAniStaticField(vfield), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Boolean(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                              ani_boolean value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::BOOLEAN, AccessMode::READWRITE),
        ANIArg::MakeForBoolean(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Class_SetStaticField_Boolean(venv->GetEnv(), ResolveToAniClass(vcls),
                                                                 ResolveToAniStaticField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Char(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                           ani_char value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::CHAR, AccessMode::READWRITE),
        ANIArg::MakeForChar(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Class_SetStaticField_Char(venv->GetEnv(), ResolveToAniClass(vcls),
                                                              ResolveToAniStaticField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Byte(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                           ani_byte value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::BYTE, AccessMode::READWRITE),
        ANIArg::MakeForByte(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Class_SetStaticField_Byte(venv->GetEnv(), ResolveToAniClass(vcls),
                                                              ResolveToAniStaticField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Short(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                            ani_short value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::SHORT, AccessMode::READWRITE),
        ANIArg::MakeForShort(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Class_SetStaticField_Short(venv->GetEnv(), ResolveToAniClass(vcls),
                                                               ResolveToAniStaticField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Int(VEnv *venv, VClass *vcls, VStaticField *vfield, ani_int value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::INT, AccessMode::READWRITE),
        ANIArg::MakeForInt(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Class_SetStaticField_Int(venv->GetEnv(), ResolveToAniClass(vcls),
                                                             ResolveToAniStaticField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Long(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                           ani_long value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::LONG, AccessMode::READWRITE),
        ANIArg::MakeForLong(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Class_SetStaticField_Long(venv->GetEnv(), ResolveToAniClass(vcls),
                                                              ResolveToAniStaticField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Float(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                            ani_float value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::FLOAT, AccessMode::READWRITE),
        ANIArg::MakeForFloat(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Class_SetStaticField_Float(venv->GetEnv(), ResolveToAniClass(vcls),
                                                               ResolveToAniStaticField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Double(VEnv *venv, VClass *vcls, VStaticField *vfield,
                                                             ani_double value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::DOUBLE, AccessMode::READWRITE),
        ANIArg::MakeForDouble(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Class_SetStaticField_Double(venv->GetEnv(), ResolveToAniClass(vcls),
                                                                ResolveToAniStaticField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticField_Ref(VEnv *venv, VClass *vcls, VStaticField *vfield, VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticField(vfield, "static_field", EtsType::OBJECT, AccessMode::READWRITE),
        ANIArg::MakeForRef(vvalue, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(vvalue);

    return GetInteractionAPI(venv)->Class_SetStaticField_Ref(venv->GetEnv(), ResolveToAniClass(vcls),
                                                             ResolveToAniStaticField(vfield), ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Boolean(VEnv *venv, VClass *vcls, const char *name,
                                                                    ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::BOOLEAN, AccessMode::READ),
        ANIArg::MakeForBooleanStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticFieldByName_Boolean(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                       result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Char(VEnv *venv, VClass *vcls, const char *name,
                                                                 ani_char *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::CHAR, AccessMode::READ),
        ANIArg::MakeForCharStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticFieldByName_Char(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                    result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Byte(VEnv *venv, VClass *vcls, const char *name,
                                                                 ani_byte *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::BYTE, AccessMode::READ),
        ANIArg::MakeForByteStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticFieldByName_Byte(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                    result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Short(VEnv *venv, VClass *vcls, const char *name,
                                                                  ani_short *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::SHORT, AccessMode::READ),
        ANIArg::MakeForShortStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticFieldByName_Short(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                     result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Int(VEnv *venv, VClass *vcls, const char *name,
                                                                ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::INT, AccessMode::READ),
        ANIArg::MakeForIntStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticFieldByName_Int(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                   result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Long(VEnv *venv, VClass *vcls, const char *name,
                                                                 ani_long *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::LONG, AccessMode::READ),
        ANIArg::MakeForLongStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticFieldByName_Long(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                    result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Float(VEnv *venv, VClass *vcls, const char *name,
                                                                  ani_float *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::FLOAT, AccessMode::READ),
        ANIArg::MakeForFloatStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticFieldByName_Float(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                     result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Double(VEnv *venv, VClass *vcls, const char *name,
                                                                   ani_double *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::DOUBLE, AccessMode::READ),
        ANIArg::MakeForDoubleStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Class_GetStaticFieldByName_Double(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                      result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_GetStaticFieldByName_Ref(VEnv *venv, VClass *vcls, const char *name,
                                                                VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::OBJECT, AccessMode::READ),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status =
        GetInteractionAPI(venv)->Class_GetStaticFieldByName_Ref(venv->GetEnv(), ResolveToAniClass(vcls), name, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Boolean(VEnv *venv, VClass *vcls, const char *name,
                                                                    ani_boolean value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::BOOLEAN, AccessMode::READWRITE),
        ANIArg::MakeForBoolean(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Boolean(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                       value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Char(VEnv *venv, VClass *vcls, const char *name,
                                                                 ani_char value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::CHAR, AccessMode::READWRITE),
        ANIArg::MakeForChar(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Char(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                    value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Byte(VEnv *venv, VClass *vcls, const char *name,
                                                                 ani_byte value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::BYTE, AccessMode::READWRITE),
        ANIArg::MakeForByte(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Byte(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                    value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Short(VEnv *venv, VClass *vcls, const char *name,
                                                                  ani_short value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::SHORT, AccessMode::READWRITE),
        ANIArg::MakeForShort(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Short(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                     value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Int(VEnv *venv, VClass *vcls, const char *name,
                                                                ani_int value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::INT, AccessMode::READWRITE),
        ANIArg::MakeForInt(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Int(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                   value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Long(VEnv *venv, VClass *vcls, const char *name,
                                                                 ani_long value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::LONG, AccessMode::READWRITE),
        ANIArg::MakeForLong(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Long(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                    value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Float(VEnv *venv, VClass *vcls, const char *name,
                                                                  ani_float value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::FLOAT, AccessMode::READWRITE),
        ANIArg::MakeForFloat(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Float(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                     value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Double(VEnv *venv, VClass *vcls, const char *name,
                                                                   ani_double value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::DOUBLE, AccessMode::READWRITE),
        ANIArg::MakeForDouble(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Double(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                      value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_SetStaticFieldByName_Ref(VEnv *venv, VClass *vcls, const char *name,
                                                                VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vcls, "class"),
        ANIArg::MakeForStaticFieldByName(name, "name", EtsType::OBJECT, AccessMode::READWRITE),
        ANIArg::MakeForRef(vvalue, "value")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vcls);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vvalue);

    return GetInteractionAPI(venv)->Class_SetStaticFieldByName_Ref(venv->GetEnv(), ResolveToAniClass(vcls), name,
                                                                   ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Boolean(VEnv *venv, VClass *vclass,
                                                                VStaticMethod *vstaticmethod, ani_boolean *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Boolean_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Boolean_A(VEnv *venv, VClass *vclass,
                                                                  VStaticMethod *vstaticmethod, ani_boolean *result,
                                                                  const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Boolean_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Boolean_V(VEnv *venv, VClass *vclass,
                                                                  VStaticMethod *vstaticmethod, ani_boolean *result,
                                                                  va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Boolean_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Char(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                             ani_char *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Char_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Char_A(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               ani_char *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Char_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Char_V(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               ani_char *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Char_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Byte(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                             ani_byte *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Byte_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Byte_A(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               ani_byte *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Byte_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Byte_V(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               ani_byte *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Byte_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Short(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                              ani_short *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Short_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Short_A(VEnv *venv, VClass *vclass,
                                                                VStaticMethod *vstaticmethod, ani_short *result,
                                                                const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Short_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Short_V(VEnv *venv, VClass *vclass,
                                                                VStaticMethod *vstaticmethod, ani_short *result,
                                                                va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Short_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Int(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                            ani_int *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Int_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Int_A(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                              ani_int *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Int_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Int_V(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                              ani_int *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Int_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Long(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                             ani_long *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Long_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Long_A(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               ani_long *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Long_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Long_V(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               ani_long *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Long_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Float(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                              ani_float *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Float_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Float_A(VEnv *venv, VClass *vclass,
                                                                VStaticMethod *vstaticmethod, ani_float *result,
                                                                const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Float_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Float_V(VEnv *venv, VClass *vclass,
                                                                VStaticMethod *vstaticmethod, ani_float *result,
                                                                va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Float_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Double(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               ani_double *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Double_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Double_A(VEnv *venv, VClass *vclass,
                                                                 VStaticMethod *vstaticmethod, ani_double *result,
                                                                 const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Double_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Double_V(VEnv *venv, VClass *vclass,
                                                                 VStaticMethod *vstaticmethod, ani_double *result,
                                                                 va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Double_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Ref(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                            VRef **vresult, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vresult);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(vresult);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Ref_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Ref_A(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                              VRef **vresult, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(vresult);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Ref_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Ref_V(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                              VRef **vresult, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(vresult);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Ref_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Void(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                             ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vstaticmethod);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::VOID),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Void_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Void_A(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::VOID),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Void_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethod_Void_V(VEnv *venv, VClass *vclass, VStaticMethod *vstaticmethod,
                                                               va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "static_method", EtsType::VOID),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    CHECK_PTR_ARG(vstaticmethod);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Class_CallStaticMethod_Void_A(
        venv->GetEnv(), ResolveToAniClass(vclass), ResolveToAniStaticMethod(vstaticmethod), args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Boolean(VEnv *venv, VClass *vclass, const char *name,
                                                                      const char *signature, ani_boolean *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Boolean_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                           name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Boolean_A(VEnv *venv, VClass *vclass, const char *name,
                                                                        const char *signature, ani_boolean *result,
                                                                        const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on
    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Boolean_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                           name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Boolean_V(VEnv *venv, VClass *vclass, const char *name,
                                                                        const char *signature, ani_boolean *result,
                                                                        va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Boolean_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                           name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Char(VEnv *venv, VClass *vclass, const char *name,
                                                                   const char *signature, ani_char *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Char_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Char_A(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, ani_char *result,
                                                                     const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on
    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Char_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Char_V(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, ani_char *result,
                                                                     va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Char_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Byte(VEnv *venv, VClass *vclass, const char *name,
                                                                   const char *signature, ani_byte *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Byte_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Byte_A(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, ani_byte *result,
                                                                     const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on
    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Byte_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Byte_V(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, ani_byte *result,
                                                                     va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Byte_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Short(VEnv *venv, VClass *vclass, const char *name,
                                                                    const char *signature, ani_short *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Short_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Short_A(VEnv *venv, VClass *vclass, const char *name,
                                                                      const char *signature, ani_short *result,
                                                                      const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Short_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Short_V(VEnv *venv, VClass *vclass, const char *name,
                                                                      const char *signature, ani_short *result,
                                                                      va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Short_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Int(VEnv *venv, VClass *vclass, const char *name,
                                                                  const char *signature, ani_int *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Int_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                       signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Int_A(VEnv *venv, VClass *vclass, const char *name,
                                                                    const char *signature, ani_int *result,
                                                                    const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Int_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                       signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Int_V(VEnv *venv, VClass *vclass, const char *name,
                                                                    const char *signature, ani_int *result,
                                                                    va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Int_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                       signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Long(VEnv *venv, VClass *vclass, const char *name,
                                                                   const char *signature, ani_long *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Long_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Long_A(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, ani_long *result,
                                                                     const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Long_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Long_V(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, ani_long *result,
                                                                     va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Long_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Float(VEnv *venv, VClass *vclass, const char *name,
                                                                    const char *signature, ani_float *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Float_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Float_A(VEnv *venv, VClass *vclass, const char *name,
                                                                      const char *signature, ani_float *result,
                                                                      const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Float_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Float_V(VEnv *venv, VClass *vclass, const char *name,
                                                                      const char *signature, ani_float *result,
                                                                      va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Float_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Double(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, ani_double *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Double_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                          name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Double_A(VEnv *venv, VClass *vclass, const char *name,
                                                                       const char *signature, ani_double *result,
                                                                       const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Double_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                          name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Double_V(VEnv *venv, VClass *vclass, const char *name,
                                                                       const char *signature, ani_double *result,
                                                                       va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Double_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                          name, signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Ref(VEnv *venv, VClass *vclass, const char *name,
                                                                  const char *signature, VRef **vresult, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vresult);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    auto args = GetVValueArgs(venv, methodArgs);
    status = GetInteractionAPI(venv)->Class_CallStaticMethodByName_Ref_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Ref_A(VEnv *venv, VClass *vclass, const char *name,
                                                                    const char *signature, VRef **vresult,
                                                                    const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    CHECK_PTR_ARG(vresult);
    auto args = GetVValueArgs(venv, methodArgs);
    ani_ref result {};
    status = GetInteractionAPI(venv)->Class_CallStaticMethodByName_Ref_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, &result, args.data());

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Ref_V(VEnv *venv, VClass *vclass, const char *name,
                                                                    const char *signature, VRef **vresult,
                                                                    va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on
    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    auto args = GetVValueArgs(venv, methodArgs);
    status = GetInteractionAPI(venv)->Class_CallStaticMethodByName_Ref_A(venv->GetEnv(), ResolveToAniClass(vclass),
                                                                         name, signature, &result, args.data());

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Void(VEnv *venv, VClass *vclass, const char *name,
                                                                   const char *signature, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, signature);

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::VOID),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Void_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Void_A(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vstaticmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::VOID),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Void_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_CallStaticMethodByName_Void_V(VEnv *venv, VClass *vclass, const char *name,
                                                                     const char *signature, va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    VStaticMethod *vstaticmethod = nullptr;
    ani_status status = ResolveVerifiedStaticMethodByName(venv, vclass, name, signature, &vstaticmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vstaticmethod, vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForStaticMethod(vstaticmethod, "method", EtsType::VOID),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Class_CallStaticMethodByName_Void_A(venv->GetEnv(), ResolveToAniClass(vclass), name,
                                                                        signature, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Boolean(VEnv *venv, VObject *vobject, VField *vfield,
                                                         ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::BOOLEAN, AccessMode::READ),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Object_GetField_Boolean(venv->GetEnv(), ResolveToAniRef(vobject),
                                                            ResolveToAniField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Char(VEnv *venv, VObject *vobject, VField *vfield, ani_char *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::CHAR, AccessMode::READ),
        ANIArg::MakeForCharStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Object_GetField_Char(venv->GetEnv(), ResolveToAniRef(vobject),
                                                         ResolveToAniField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Byte(VEnv *venv, VObject *vobject, VField *vfield, ani_byte *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::BYTE, AccessMode::READ),
        ANIArg::MakeForByteStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Object_GetField_Byte(venv->GetEnv(), ResolveToAniRef(vobject),
                                                         ResolveToAniField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Short(VEnv *venv, VObject *vobject, VField *vfield, ani_short *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::SHORT, AccessMode::READ),
        ANIArg::MakeForShortStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Object_GetField_Short(venv->GetEnv(), ResolveToAniRef(vobject),
                                                          ResolveToAniField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Int(VEnv *venv, VObject *vobject, VField *vfield, ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::INT, AccessMode::READ),
        ANIArg::MakeForIntStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Object_GetField_Int(venv->GetEnv(), ResolveToAniRef(vobject),
                                                        ResolveToAniField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Long(VEnv *venv, VObject *vobject, VField *vfield, ani_long *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::LONG, AccessMode::READ),
        ANIArg::MakeForLongStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Object_GetField_Long(venv->GetEnv(), ResolveToAniRef(vobject),
                                                         ResolveToAniField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Float(VEnv *venv, VObject *vobject, VField *vfield, ani_float *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::FLOAT, AccessMode::READ),
        ANIArg::MakeForFloatStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);

    return GetInteractionAPI(venv)->Object_GetField_Float(venv->GetEnv(), ResolveToAniRef(vobject),
                                                          ResolveToAniField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Double(VEnv *venv, VObject *vobject, VField *vfield,
                                                        ani_double *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::DOUBLE, AccessMode::READ),
        ANIArg::MakeForDoubleStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(result);
    return GetInteractionAPI(venv)->Object_GetField_Double(venv->GetEnv(), ResolveToAniRef(vobject),
                                                           ResolveToAniField(vfield), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetField_Ref(VEnv *venv, VObject *vobject, VField *vfield, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::OBJECT, AccessMode::READ),
        ANIArg::MakeForRefStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Object_GetField_Ref(venv->GetEnv(), ResolveToAniRef(vobject),
                                                                     ResolveToAniField(vfield), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Boolean(VEnv *venv, VObject *vobject, VField *vfield,
                                                         ani_boolean value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::BOOLEAN, AccessMode::READWRITE),
        ANIArg::MakeForBoolean(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Object_SetField_Boolean(venv->GetEnv(), ResolveToAniRef(vobject),
                                                            ResolveToAniField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Char(VEnv *venv, VObject *vobject, VField *vfield, ani_char value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::CHAR, AccessMode::READWRITE),
        ANIArg::MakeForChar(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Object_SetField_Char(venv->GetEnv(), ResolveToAniRef(vobject),
                                                         ResolveToAniField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Byte(VEnv *venv, VObject *vobject, VField *vfield, ani_byte value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::BYTE, AccessMode::READWRITE),
        ANIArg::MakeForByte(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Object_SetField_Byte(venv->GetEnv(), ResolveToAniRef(vobject),
                                                         ResolveToAniField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Short(VEnv *venv, VObject *vobject, VField *vfield, ani_short value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::SHORT, AccessMode::READWRITE),
        ANIArg::MakeForShort(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Object_SetField_Short(venv->GetEnv(), ResolveToAniRef(vobject),
                                                          ResolveToAniField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Int(VEnv *venv, VObject *vobject, VField *vfield, ani_int value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::INT, AccessMode::READWRITE),
        ANIArg::MakeForInt(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Object_SetField_Int(venv->GetEnv(), ResolveToAniRef(vobject),
                                                        ResolveToAniField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Long(VEnv *venv, VObject *vobject, VField *vfield, ani_long value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::LONG, AccessMode::READWRITE),
        ANIArg::MakeForLong(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Object_SetField_Long(venv->GetEnv(), ResolveToAniRef(vobject),
                                                         ResolveToAniField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Float(VEnv *venv, VObject *vobject, VField *vfield, ani_float value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::FLOAT, AccessMode::READWRITE),
        ANIArg::MakeForFloat(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Object_SetField_Float(venv->GetEnv(), ResolveToAniRef(vobject),
                                                          ResolveToAniField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Double(VEnv *venv, VObject *vobject, VField *vfield, ani_double value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::DOUBLE, AccessMode::READWRITE),
        ANIArg::MakeForDouble(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);

    return GetInteractionAPI(venv)->Object_SetField_Double(venv->GetEnv(), ResolveToAniRef(vobject),
                                                           ResolveToAniField(vfield), value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetField_Ref(VEnv *venv, VObject *vobject, VField *vfield, VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForField(vfield, "field", EtsType::OBJECT, AccessMode::READWRITE),
        ANIArg::MakeForRef(vvalue, "ref"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vfield);
    CHECK_PTR_ARG(vvalue);

    return GetInteractionAPI(venv)->Object_SetField_Ref(venv->GetEnv(), ResolveToAniRef(vobject),
                                                        ResolveToAniField(vfield), ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Boolean(VEnv *venv, VObject *vobject, const char *name,
                                                               ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::BOOLEAN, AccessMode::READ),
        ANIArg::MakeForBooleanStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetFieldByName_Boolean(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Char(VEnv *venv, VObject *vobject, const char *name,
                                                            ani_char *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::CHAR, AccessMode::READ),
        ANIArg::MakeForCharStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetFieldByName_Char(venv->GetEnv(), ResolveToAniRef(vobject), name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Byte(VEnv *venv, VObject *vobject, const char *name,
                                                            ani_byte *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::BYTE, AccessMode::READ),
        ANIArg::MakeForByteStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetFieldByName_Byte(venv->GetEnv(), ResolveToAniRef(vobject), name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Short(VEnv *venv, VObject *vobject, const char *name,
                                                             ani_short *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::SHORT, AccessMode::READ),
        ANIArg::MakeForShortStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetFieldByName_Short(venv->GetEnv(), ResolveToAniRef(vobject), name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Int(VEnv *venv, VObject *vobject, const char *name,
                                                           ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::INT, AccessMode::READ),
        ANIArg::MakeForIntStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetFieldByName_Int(venv->GetEnv(), ResolveToAniRef(vobject), name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Long(VEnv *venv, VObject *vobject, const char *name,
                                                            ani_long *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::LONG, AccessMode::READ),
        ANIArg::MakeForLongStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetFieldByName_Long(venv->GetEnv(), ResolveToAniRef(vobject), name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Float(VEnv *venv, VObject *vobject, const char *name,
                                                             ani_float *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::FLOAT, AccessMode::READ),
        ANIArg::MakeForFloatStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetFieldByName_Float(venv->GetEnv(), ResolveToAniRef(vobject), name, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Double(VEnv *venv, VObject *vobject, const char *name,
                                                              ani_double *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::DOUBLE, AccessMode::READ),
        ANIArg::MakeForDoubleStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetFieldByName_Double(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                 result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetFieldByName_Ref(VEnv *venv, VObject *vobject, const char *name,
                                                           VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::OBJECT, AccessMode::READ),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vresult);
    ani_ref tmp {};
    ani_status status =
        GetInteractionAPI(venv)->Object_GetFieldByName_Ref(venv->GetEnv(), ResolveToAniRef(vobject), name, &tmp);

    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, tmp, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Boolean(VEnv *venv, VObject *vobject, const char *name,
                                                               ani_boolean value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::BOOLEAN, AccessMode::READWRITE),
        ANIArg::MakeForBoolean(value, "value")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Boolean(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Char(VEnv *venv, VObject *vobject, const char *name,
                                                            ani_char value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::CHAR, AccessMode::READWRITE),
        ANIArg::MakeForChar(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Char(venv->GetEnv(), ResolveToAniRef(vobject), name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Byte(VEnv *venv, VObject *vobject, const char *name,
                                                            ani_byte value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::BYTE, AccessMode::READWRITE),
        ANIArg::MakeForByte(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Byte(venv->GetEnv(), ResolveToAniRef(vobject), name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Short(VEnv *venv, VObject *vobject, const char *name,
                                                             ani_short value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::SHORT, AccessMode::READWRITE),
        ANIArg::MakeForShort(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Short(venv->GetEnv(), ResolveToAniRef(vobject), name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Int(VEnv *venv, VObject *vobject, const char *name,
                                                           ani_int value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::INT, AccessMode::READWRITE),
        ANIArg::MakeForInt(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Int(venv->GetEnv(), ResolveToAniRef(vobject), name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Long(VEnv *venv, VObject *vobject, const char *name,
                                                            ani_long value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::LONG, AccessMode::READWRITE),
        ANIArg::MakeForLong(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Long(venv->GetEnv(), ResolveToAniRef(vobject), name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Float(VEnv *venv, VObject *vobject, const char *name,
                                                             ani_float value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::FLOAT, AccessMode::READWRITE),
        ANIArg::MakeForFloat(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Float(venv->GetEnv(), ResolveToAniRef(vobject), name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Double(VEnv *venv, VObject *vobject, const char *name,
                                                              ani_double value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::DOUBLE, AccessMode::READWRITE),
        ANIArg::MakeForDouble(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Double(venv->GetEnv(), ResolveToAniRef(vobject), name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetFieldByName_Ref(VEnv *venv, VObject *vobject, const char *name, VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForFieldByName(name, "name", EtsType::OBJECT, AccessMode::READWRITE),
        ANIArg::MakeForRef(vvalue, "value")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vvalue);
    return GetInteractionAPI(venv)->Object_SetFieldByName_Ref(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                              ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Boolean(VEnv *venv, VObject *vobject, const char *name,
                                                                  ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::BOOLEAN, AccessMode::READ),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetPropertyByName_Boolean(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                     result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Char(VEnv *venv, VObject *vobject, const char *name,
                                                               ani_char *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::CHAR, AccessMode::READ),
        ANIArg::MakeForCharStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetPropertyByName_Char(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Byte(VEnv *venv, VObject *vobject, const char *name,
                                                               ani_byte *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::BYTE, AccessMode::READ),
        ANIArg::MakeForByteStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetPropertyByName_Byte(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Short(VEnv *venv, VObject *vobject, const char *name,
                                                                ani_short *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::SHORT, AccessMode::READ),
        ANIArg::MakeForShortStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetPropertyByName_Short(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Int(VEnv *venv, VObject *vobject, const char *name,
                                                              ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::INT, AccessMode::READ),
        ANIArg::MakeForIntStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetPropertyByName_Int(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                 result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Long(VEnv *venv, VObject *vobject, const char *name,
                                                               ani_long *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::LONG, AccessMode::READ),
        ANIArg::MakeForLongStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetPropertyByName_Long(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Float(VEnv *venv, VObject *vobject, const char *name,
                                                                ani_float *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::FLOAT, AccessMode::READ),
        ANIArg::MakeForFloatStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetPropertyByName_Float(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Double(VEnv *venv, VObject *vobject, const char *name,
                                                                 ani_double *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::DOUBLE, AccessMode::READ),
        ANIArg::MakeForDoubleStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_GetPropertyByName_Double(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_GetPropertyByName_Ref(VEnv *venv, VObject *vobject, const char *name,
                                                              VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::OBJECT, AccessMode::READ),
        ANIArg::MakeForRefStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    ani_status status =
        GetInteractionAPI(venv)->Object_GetPropertyByName_Ref(venv->GetEnv(), ResolveToAniRef(vobject), name, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Boolean(VEnv *venv, VObject *vobject, const char *name,
                                                                  ani_boolean value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::BOOLEAN, AccessMode::READWRITE),
        ANIArg::MakeForBoolean(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Boolean(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                     value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Char(VEnv *venv, VObject *vobject, const char *name,
                                                               ani_char value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::CHAR, AccessMode::READWRITE),
        ANIArg::MakeForChar(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Char(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Byte(VEnv *venv, VObject *vobject, const char *name,
                                                               ani_byte value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::BYTE, AccessMode::READWRITE),
        ANIArg::MakeForByte(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Byte(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Short(VEnv *venv, VObject *vobject, const char *name,
                                                                ani_short value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::SHORT, AccessMode::READWRITE),
        ANIArg::MakeForShort(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Short(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Int(VEnv *venv, VObject *vobject, const char *name,
                                                              ani_int value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::INT, AccessMode::READWRITE),
        ANIArg::MakeForInt(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Int(venv->GetEnv(), ResolveToAniRef(vobject), name, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Long(VEnv *venv, VObject *vobject, const char *name,
                                                               ani_long value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::LONG, AccessMode::READWRITE),
        ANIArg::MakeForLong(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Long(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Float(VEnv *venv, VObject *vobject, const char *name,
                                                                ani_float value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::FLOAT, AccessMode::READWRITE),
        ANIArg::MakeForFloat(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Float(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Double(VEnv *venv, VObject *vobject, const char *name,
                                                                 ani_double value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::DOUBLE, AccessMode::READWRITE),
        ANIArg::MakeForDouble(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Double(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_SetPropertyByName_Ref(VEnv *venv, VObject *vobject, const char *name,
                                                              VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForPropertyByName(name, "name", EtsType::OBJECT, AccessMode::READWRITE),
        ANIArg::MakeForRef(vvalue, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Object_SetPropertyByName_Ref(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                 ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Boolean(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                           ani_boolean *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BOOLEAN, false),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Boolean_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Boolean_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                             ani_boolean *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BOOLEAN, false),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Boolean_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Boolean_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                             ani_boolean *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BOOLEAN, false),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Boolean_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Char(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                        ani_char *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::CHAR, false),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Char_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Char_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          ani_char *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::CHAR, false),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Char_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Char_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          ani_char *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::CHAR, false),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Char_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Byte(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                        ani_byte *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BYTE, false),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Byte_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Byte_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          ani_byte *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BYTE, false),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Byte_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Byte_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          ani_byte *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BYTE, false),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Byte_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Short(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                         ani_short *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::SHORT, false),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Short_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Short_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                           ani_short *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::SHORT, false),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Short_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Short_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                           ani_short *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::SHORT, false),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Short_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Int(VEnv *venv, VObject *vobject, VMethod *vmethod, ani_int *result,
                                                       ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::INT, false),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Int_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Int_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                         ani_int *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::INT, false),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Int_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Int_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                         ani_int *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::INT, false),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Int_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Long(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                        ani_long *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::LONG, false),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Long_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Long_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          ani_long *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::LONG, false),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Long_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Long_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          ani_long *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::LONG, false),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Long_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Float(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                         ani_float *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::FLOAT, false),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Float_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Float_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                           ani_float *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::FLOAT, false),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Float_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Float_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                           ani_float *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::FLOAT, false),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Float_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Double(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          ani_double *result, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::DOUBLE, false),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Double_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Double_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                            ani_double *result, const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::DOUBLE, false),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Double_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Double_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                            ani_double *result, va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::DOUBLE, false),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(result);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Double_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), result, args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Ref(VEnv *venv, VObject *vobject, VMethod *vmethod, VRef **vresult,
                                                       ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vresult);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::OBJECT, false),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(vresult);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Ref_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Ref_A(VEnv *venv, VObject *vobject, VMethod *vmethod, VRef **vresult,
                                                         const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::OBJECT, false),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(vresult);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Ref_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Ref_V(VEnv *venv, VObject *vobject, VMethod *vmethod, VRef **vresult,
                                                         va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::OBJECT, false),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(vresult);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Ref_A(
        venv->GetEnv(), ResolveToAniRef(vobject), ResolveToAniMethod(vmethod), &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Void(VEnv *venv, VObject *vobject, VMethod *vmethod, ...)
{
    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vmethod);
    ANIArg::AniMethodArgs methodVArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::VOID, false),
        ANIArg::MakeForMethodArgs(&methodVArgs, "..."),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);

    auto args = GetVValueArgs(venv, methodVArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Void_A(venv->GetEnv(), ResolveToAniRef(vobject),
                                                                          ResolveToAniMethod(vmethod), args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Void_A(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          const ani_value *vargs)
{
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::VOID, false),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);
    CHECK_PTR_ARG(vargs);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Void_A(venv->GetEnv(), ResolveToAniRef(vobject),
                                                                          ResolveToAniMethod(vmethod), args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethod_Void_V(VEnv *venv, VObject *vobject, VMethod *vmethod,
                                                          va_list vvaArgs)
{
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::VOID, false),
        ANIArg::MakeForMethodArgs(&methodArgs, "args"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    CHECK_PTR_ARG(vmethod);

    auto args = GetVValueArgs(venv, methodArgs);
    ani_status status = GetInteractionAPI(venv)->Object_CallMethod_Void_A(venv->GetEnv(), ResolveToAniRef(vobject),
                                                                          ResolveToAniMethod(vmethod), args.data());
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Boolean(VEnv *venv, VObject *vobject, const char *name,
                                                                 const char *signature, ani_boolean *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Boolean_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                      signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Boolean_A(VEnv *venv, VObject *vobject, const char *name,
                                                                   const char *signature, ani_boolean *result,
                                                                   const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Boolean_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                      signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Boolean_V(VEnv *venv, VObject *vobject, const char *name,
                                                                   const char *signature, ani_boolean *result,
                                                                   va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Boolean_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                      signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Char(VEnv *venv, VObject *vobject, const char *name,
                                                              const char *signature, ani_char *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Char_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Char_A(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, ani_char *result,
                                                                const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Char_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Char_V(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, ani_char *result,
                                                                va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Char_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Byte(VEnv *venv, VObject *vobject, const char *name,
                                                              const char *signature, ani_byte *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Byte_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Byte_A(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, ani_byte *result,
                                                                const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Byte_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Byte_V(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, ani_byte *result,
                                                                va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Byte_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Short(VEnv *venv, VObject *vobject, const char *name,
                                                               const char *signature, ani_short *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Short_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Short_A(VEnv *venv, VObject *vobject, const char *name,
                                                                 const char *signature, ani_short *result,
                                                                 const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Short_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Short_V(VEnv *venv, VObject *vobject, const char *name,
                                                                 const char *signature, ani_short *result,
                                                                 va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Short_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Int(VEnv *venv, VObject *vobject, const char *name,
                                                             const char *signature, ani_int *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Int_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Int_A(VEnv *venv, VObject *vobject, const char *name,
                                                               const char *signature, ani_int *result,
                                                               const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Int_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Int_V(VEnv *venv, VObject *vobject, const char *name,
                                                               const char *signature, ani_int *result, va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Int_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                  signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Long(VEnv *venv, VObject *vobject, const char *name,
                                                              const char *signature, ani_long *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Long_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Long_A(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, ani_long *result,
                                                                const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Long_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Long_V(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, ani_long *result,
                                                                va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Long_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Float(VEnv *venv, VObject *vobject, const char *name,
                                                               const char *signature, ani_float *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Float_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Float_A(VEnv *venv, VObject *vobject, const char *name,
                                                                 const char *signature, ani_float *result,
                                                                 const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }
    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on
    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Float_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Float_V(VEnv *venv, VObject *vobject, const char *name,
                                                                 const char *signature, ani_float *result,
                                                                 va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Float_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Double(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, ani_double *result, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, result);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Double_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                     signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Double_A(VEnv *venv, VObject *vobject, const char *name,
                                                                  const char *signature, ani_double *result,
                                                                  const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Double_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                     signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Double_V(VEnv *venv, VObject *vobject, const char *name,
                                                                  const char *signature, ani_double *result,
                                                                  va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Double_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                     signature, result, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Ref(VEnv *venv, VObject *vobject, const char *name,
                                                             const char *signature, VRef **vresult, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, vresult);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    auto args = GetVValueArgs(venv, methodArgs);
    status = GetInteractionAPI(venv)->Object_CallMethodByName_Ref_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Ref_A(VEnv *venv, VObject *vobject, const char *name,
                                                               const char *signature, VRef **vresult,
                                                               const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vresult);
    CHECK_PTR_ARG(vargs);
    ani_ref result {};
    auto args = GetVValueArgs(venv, methodArgs);
    status = GetInteractionAPI(venv)->Object_CallMethodByName_Ref_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Ref_V(VEnv *venv, VObject *vobject, const char *name,
                                                               const char *signature, VRef **vresult, va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::OBJECT),
        ANIArg::MakeForRefStorage(vresult, "result"),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    auto args = GetVValueArgs(venv, methodArgs);
    status = GetInteractionAPI(venv)->Object_CallMethodByName_Ref_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                    signature, &result, args.data());
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Void(VEnv *venv, VObject *vobject, const char *name,
                                                              const char *signature, ...)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);

    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    va_list vvaArgs;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(vvaArgs, signature);
    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    va_end(vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::VOID),
        ANIArg::MakeForMethodArgs(&methodArgs, "...")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Void_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Void_A(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, const ani_value *vargs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs {GetEtsMethodIfPointerValid(vmethod), vargs, {}, false};

    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::VOID),
        ANIArg::MakeForMethodAArgs(&methodArgs, "args")
    );
    // clang-format on

    CHECK_PTR_ARG(vargs);
    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Void_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Object_CallMethodByName_Void_V(VEnv *venv, VObject *vobject, const char *name,
                                                                const char *signature, va_list vvaArgs)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSignature(signature, "signature")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    VMethod *vmethod = nullptr;
    ani_status status = ResolveVerifiedMethodByName(venv, vobject, name, signature, &vmethod);
    if (status != ANI_OK) {
        return status;
    }

    ANIArg::AniMethodArgs methodArgs = GetVArgsByVVAArgs(vmethod, vvaArgs);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForObject(vobject, "object"),
        ANIArg::MakeForMethod(vmethod, "method", EtsType::VOID),
        ANIArg::MakeForMethodArgs(&methodArgs, "args")
    );
    // clang-format on

    auto args = GetVValueArgs(venv, methodArgs);
    return GetInteractionAPI(venv)->Object_CallMethodByName_Void_A(venv->GetEnv(), ResolveToAniRef(vobject), name,
                                                                   signature, args.data());
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetNumberOfItems(VEnv *venv, VTupleValue *vtupleValue, ani_size *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForSizeStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);
    return GetInteractionAPI(venv)->TupleValue_GetNumberOfItems(venv->GetEnv(), ResolveToAniRef(vtupleValue), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Boolean(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                            ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::BOOLEAN, "index"),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_GetItem_Boolean(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                               result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Char(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                         ani_char *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::CHAR, "index"),
        ANIArg::MakeForCharStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_GetItem_Char(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                            result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Byte(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                         ani_byte *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::BYTE, "index"),
        ANIArg::MakeForByteStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_GetItem_Byte(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                            result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Short(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                          ani_short *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::SHORT, "index"),
        ANIArg::MakeForShortStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_GetItem_Short(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                             result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Int(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                        ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::INT, "index"),
        ANIArg::MakeForIntStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_GetItem_Int(venv->GetEnv(), ResolveToAniRef(vtupleValue), index, result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Long(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                         ani_long *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::LONG, "index"),
        ANIArg::MakeForLongStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_GetItem_Long(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                            result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Float(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                          ani_float *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::FLOAT, "index"),
        ANIArg::MakeForFloatStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_GetItem_Float(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                             result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Double(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                           ani_double *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::DOUBLE, "index"),
        ANIArg::MakeForDoubleStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_GetItem_Double(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                              result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_GetItem_Ref(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                        VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::OBJECT, "index"),
        ANIArg::MakeForRefStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status =
        GetInteractionAPI(venv)->TupleValue_GetItem_Ref(venv->GetEnv(), ResolveToAniRef(vtupleValue), index, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Boolean(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                            ani_boolean value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::BOOLEAN, "index"),
        ANIArg::MakeForBoolean(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Boolean(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                               value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Char(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                         ani_char value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::CHAR, "index"),
        ANIArg::MakeForChar(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Char(venv->GetEnv(), ResolveToAniRef(vtupleValue), index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Byte(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                         ani_byte value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::BYTE, "index"),
        ANIArg::MakeForByte(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Byte(venv->GetEnv(), ResolveToAniRef(vtupleValue), index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Short(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                          ani_short value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::SHORT, "index"),
        ANIArg::MakeForShort(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Short(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                             value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Int(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                        ani_int value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::INT, "index"),
        ANIArg::MakeForInt(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Int(venv->GetEnv(), ResolveToAniRef(vtupleValue), index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Long(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                         ani_long value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::LONG, "index"),
        ANIArg::MakeForLong(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Long(venv->GetEnv(), ResolveToAniRef(vtupleValue), index, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Float(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                          ani_float value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::FLOAT, "index"),
        ANIArg::MakeForFloat(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Float(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                             value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Double(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                           ani_double value)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::DOUBLE, "index"),
        ANIArg::MakeForDouble(value, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Double(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                              value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status TupleValue_SetItem_Ref(VEnv *venv, VTupleValue *vtupleValue, ani_size index,
                                                        VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForTupleValue(vtupleValue, "tuple_value"),
        ANIArg::MakeForTupleIndex(index, EtsType::OBJECT, "index"),
        ANIArg::MakeForRef(vvalue, "value"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vtupleValue);
    CHECK_PTR_ARG(vvalue);

    return GetInteractionAPI(venv)->TupleValue_SetItem_Ref(venv->GetEnv(), ResolveToAniRef(vtupleValue), index,
                                                           ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GlobalReference_Create(VEnv *venv, VRef *vref, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRef(vref, "ref"),
        ANIArg::MakeForRefStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    auto status = GetInteractionAPI(venv)->GlobalReference_Create(venv->GetEnv(), ResolveToAniRef(vref), &result);
    if (LIKELY(status == ANI_OK)) {
        *vresult = venv->AddGlobalVerifiedRef(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status GlobalReference_Delete(VEnv *venv, VRef *vgref)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForGlobalRef(vgref, "gref"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vgref);
    if (!venv->IsValidGlobalVerifiedRef(vgref) && !ManagedCodeAccessor::IsUndefined(ResolveToAniRef(vgref))) {
        return ANI_INCORRECT_REF;
    }
    ani_ref gref = ResolveToAniRef(vgref);
    ani_status status = GetInteractionAPI(venv)->GlobalReference_Delete(venv->GetEnv(), gref);
    if (LIKELY(status == ANI_OK) && venv->IsValidGlobalVerifiedRef(vgref)) {
        venv->DeleteGlobalVerifiedRef(vgref);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status WeakReference_Create(VEnv *venv, VRef *vref, VWRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForRef(vref, "ref"),
        ANIArg::MakeForWRefStorage(reinterpret_cast<ani_wref *>(vresult), "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(vresult);

    ani_wref runtimeWref {};
    ani_status status =
        GetInteractionAPI(venv)->WeakReference_Create(venv->GetEnv(), ResolveToAniRef(vref), &runtimeWref);
    if (LIKELY(status == ANI_OK)) {
        *vresult = venv->AddVerifiedWeakRef(runtimeWref);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status WeakReference_Delete(VEnv *venv, VWRef *vwref)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env", false),
        ANIArg::MakeForWRef(vwref, "wref"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vwref);
    if (!venv->IsValidWeakRef(vwref) &&
        !ManagedCodeAccessor::IsUndefined(reinterpret_cast<ani_ref>(ResolveToAniRef(vwref)))) {
        return ANI_INCORRECT_REF;
    }
    ani_status status = GetInteractionAPI(venv)->WeakReference_Delete(venv->GetEnv(), ResolveToAniRef(vwref));
    if (LIKELY(status == ANI_OK)) {
        venv->DeleteVerifiedWeakRef(vwref);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status WeakReference_GetReference(VEnv *venv, VWRef *vwref, ani_boolean *wasReleasedResult,
                                                            VRef **vrefResult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForWRef(vwref, "wref"),
        ANIArg::MakeForBooleanStorage(wasReleasedResult, "was_released_result"),
        ANIArg::MakeForRefStorage(vrefResult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vwref);
    CHECK_PTR_ARG(wasReleasedResult);
    CHECK_PTR_ARG(vrefResult);
    if (!venv->IsValidWeakRef(vwref)) {
        if (venv->IsValidGlobalVerifiedRef(reinterpret_cast<VRef *>(vwref))) {
            return ANI_INCORRECT_REF;
        }
        if (!ManagedCodeAccessor::IsUndefined(reinterpret_cast<ani_ref>(ResolveToAniRef(vwref)))) {
            return ANI_INCORRECT_REF;
        }
    }
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->WeakReference_GetReference(venv->GetEnv(), ResolveToAniRef(vwref),
                                                                            wasReleasedResult, &result);
    if (LIKELY(status == ANI_OK)) {
        *vrefResult = result == nullptr ? nullptr : venv->AddLocalVerifiedRef(result);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status CreateArrayBuffer(VEnv *venv, size_t length, void **dataResult,
                                                   VArrayBuffer **varraybufferResult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForArrayBufferLength(length, "length"),
        ANIArg::MakeForVoidPtrStorage(dataResult, "data_result"),
        ANIArg::MakeForArrayBufferStorage(varraybufferResult, "arraybuffer_result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varraybufferResult);
    ani_arraybuffer result {};
    ani_status status = GetInteractionAPI(venv)->CreateArrayBuffer(venv->GetEnv(), length, dataResult, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, varraybufferResult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status ArrayBuffer_GetInfo(VEnv *venv, VArrayBuffer *varraybuffer, void **dataResult,
                                                     size_t *lengthResult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForArrayBuffer(varraybuffer, "arraybuffer"),
        ANIArg::MakeForVoidPtrStorage(dataResult, "data_result"),
        ANIArg::MakeForSizeStorage(lengthResult, "length_result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(varraybuffer);
    return GetInteractionAPI(venv)->ArrayBuffer_GetInfo(venv->GetEnv(), ResolveToAniRef(varraybuffer), dataResult,
                                                        lengthResult);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Promise_New(VEnv *venv, VResolver **vresultResolver, VObject **vresultPromise)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForResolverStorage(vresultResolver, "result_resolver"),
        ANIArg::MakeForPromiseStorage(vresultPromise, "result_promise")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresultResolver);
    CHECK_PTR_ARG(vresultPromise);

    ani_resolver realResolver {};
    ani_object realPromise {};
    ani_status status = GetInteractionAPI(venv)->Promise_New(venv->GetEnv(), &realResolver, &realPromise);
    if (LIKELY(status == ANI_OK)) {
        *vresultResolver = venv->AddGlobalVerifiedResolver(realResolver);
        ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, realPromise, vresultPromise);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status PromiseResolver_Resolve(VEnv *venv, VResolver *vresolver, VRef *vresolution)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForResolver(vresolver, "resolver"),
        ANIArg::MakeForRef(vresolution, "resolution")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresolver);
    CHECK_PTR_ARG(vresolution);

    ani_status status = GetInteractionAPI(venv)->PromiseResolver_Resolve(
        venv->GetEnv(), ResolveToAniResolver(vresolver), ResolveToAniRef(vresolution));
    if (LIKELY(status == ANI_OK)) {
        venv->DeleteGlobalResolver(vresolver);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status PromiseResolver_Reject(VEnv *venv, VResolver *vresolver, VError *vrejection)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForResolver(vresolver, "resolver"),
        ANIArg::MakeForError(vrejection, "rejection")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresolver);
    CHECK_PTR_ARG(vrejection);

    ani_status status = GetInteractionAPI(venv)->PromiseResolver_Reject(venv->GetEnv(), ResolveToAniResolver(vresolver),
                                                                        ResolveToAniRef(vrejection));
    if (LIKELY(status == ANI_OK)) {
        venv->DeleteGlobalResolver(vresolver);
    }
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_InstanceOf(VEnv *venv, VRef *vref, VRef *vtype, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vref, "ref"),
        ANIArg::MakeForAnyRef(vtype, "type"),
        ANIArg::MakeForBooleanStorage(result, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(vtype);

    return GetInteractionAPI(venv)->Any_InstanceOf(venv->GetEnv(), ResolveToAniRef(vref), ResolveToAniRef(vtype),
                                                   result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_GetProperty(VEnv *venv, VRef *vref, const char *name, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vref, "ref"),
        ANIArg::MakeForUTF8String(name, "name"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Any_GetProperty(venv->GetEnv(), ResolveToAniRef(vref), name, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_SetProperty(VEnv *venv, VRef *vref, const char *name, VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vref, "ref"),
        ANIArg::MakeForUTF8String(name, "name"),
        ANIArg::MakeForRef(vvalue, "value")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(name);
    CHECK_PTR_ARG(vvalue);

    return GetInteractionAPI(venv)->Any_SetProperty(venv->GetEnv(), ResolveToAniRef(vref), name,
                                                    ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_GetByIndex(VEnv *venv, VRef *vref, ani_size index, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vref, "ref"),
        ANIArg::MakeForSize(index, "index"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Any_GetByIndex(venv->GetEnv(), ResolveToAniRef(vref), index, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_SetByIndex(VEnv *venv, VRef *vref, ani_size index, VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vref, "ref"),
        ANIArg::MakeForSize(index, "index"),
        ANIArg::MakeForRef(vvalue, "value")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(vvalue);

    return GetInteractionAPI(venv)->Any_SetByIndex(venv->GetEnv(), ResolveToAniRef(vref), index,
                                                   ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_GetByValue(VEnv *venv, VRef *vref, VRef *vkey, VRef **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vref, "ref"),
        ANIArg::MakeForRef(vkey, "key"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(vkey);
    CHECK_PTR_ARG(vresult);

    ani_ref result {};
    ani_status status =
        GetInteractionAPI(venv)->Any_GetByValue(venv->GetEnv(), ResolveToAniRef(vref), ResolveToAniRef(vkey), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_SetByValue(VEnv *venv, VRef *vref, VRef *vkey, VRef *vvalue)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vref, "ref"),
        ANIArg::MakeForRef(vkey, "key"),
        ANIArg::MakeForRef(vvalue, "value")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vref);
    CHECK_PTR_ARG(vkey);
    CHECK_PTR_ARG(vvalue);

    return GetInteractionAPI(venv)->Any_SetByValue(venv->GetEnv(), ResolveToAniRef(vref), ResolveToAniRef(vkey),
                                                   ResolveToAniRef(vvalue));
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_Call(VEnv *venv, VRef *vfunc, ani_size argc, VRef **vargv, VRef **vresult)
{
    VRefCallArgs refArgs(argc, vargv);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vfunc, "func"),
        ANIArg::MakeForSize(argc, "argc"),
        ANIArg::MakeForRefCallArgs(&refArgs, "argv"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vfunc);
    if (argc > 0) {
        CHECK_PTR_ARG(vargv);
    }
    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Any_Call(venv->GetEnv(), ResolveToAniRef(vfunc), argc,
                                                          refArgs.GetReleaseArgv(), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_CallMethod(VEnv *venv, VRef *vself, const char *name, ani_size argc, VRef **vargv,
                                                VRef **vresult)
{
    VRefCallArgs refArgs(argc, vargv);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vself, "self"),
        ANIArg::MakeForMethodName(name, "name"),
        ANIArg::MakeForSize(argc, "argc"),
        ANIArg::MakeForRefCallArgs(&refArgs, "argv"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vself);
    CHECK_PTR_ARG(name);
    if (argc > 0) {
        CHECK_PTR_ARG(vargv);
    }
    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Any_CallMethod(venv->GetEnv(), ResolveToAniRef(vself), name, argc,
                                                                refArgs.GetReleaseArgv(), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Any_New(VEnv *venv, VRef *vctor, ani_size argc, VRef **vargv, VRef **vresult)
{
    VRefCallArgs refArgs(argc, vargv);
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForAnyRef(vctor, "ctor"),
        ANIArg::MakeForSize(argc, "argc"),
        ANIArg::MakeForRefCallArgs(&refArgs, "argv"),
        ANIArg::MakeForRefStorage(vresult, "result")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vctor);
    if (argc > 0) {
        CHECK_PTR_ARG(vargv);
    }
    CHECK_PTR_ARG(vresult);
    ani_ref result {};
    ani_status status = GetInteractionAPI(venv)->Any_New(venv->GetEnv(), ResolveToAniRef(vctor), argc,
                                                         refArgs.GetReleaseArgv(), &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Class_BindStaticNativeMethods(VEnv *venv, VClass *vclass,
                                                               const ani_native_function *methods, ani_size nrMethods)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForClass(vclass, "class"),
        ANIArg::MakeForNativeStaticMethods(methods, nrMethods, "methods"),
        ANIArg::MakeForSize(nrMethods, "nr_methods")
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vclass);
    return GetInteractionAPI(venv)->Class_BindStaticNativeMethods(venv->GetEnv(), ResolveToAniClass(vclass), methods,
                                                                  nrMethods);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Box_Boolean(VEnv *venv, ani_boolean value, VObject **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoolean(value, "value"),
        ANIArg::MakeForObjectStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);
    ani_object result {};
    ani_status status = GetInteractionAPI(venv)->Primitive_Box_Boolean(venv->GetEnv(), value, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Unbox_Boolean(VEnv *venv, VObject *vobject, ani_boolean *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoxedPrimitiveObject(vobject, "obj", EtsType::BOOLEAN),
        ANIArg::MakeForBooleanStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Primitive_Unbox_Boolean(venv->GetEnv(), ResolveToAniRef(vobject), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Box_Byte(VEnv *venv, ani_byte value, VObject **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForByte(value, "value"),
        ANIArg::MakeForObjectStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);
    ani_object result {};
    ani_status status = GetInteractionAPI(venv)->Primitive_Box_Byte(venv->GetEnv(), value, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Unbox_Byte(VEnv *venv, VObject *vobject, ani_byte *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoxedPrimitiveObject(vobject, "obj", EtsType::BYTE),
        ANIArg::MakeForByteStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Primitive_Unbox_Byte(venv->GetEnv(), ResolveToAniRef(vobject), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Box_Char(VEnv *venv, ani_char value, VObject **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForChar(value, "value"),
        ANIArg::MakeForObjectStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);
    ani_object result {};
    ani_status status = GetInteractionAPI(venv)->Primitive_Box_Char(venv->GetEnv(), value, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Unbox_Char(VEnv *venv, VObject *vobject, ani_char *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoxedPrimitiveObject(vobject, "obj", EtsType::CHAR),
        ANIArg::MakeForCharStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Primitive_Unbox_Char(venv->GetEnv(), ResolveToAniRef(vobject), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Box_Short(VEnv *venv, ani_short value, VObject **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForShort(value, "value"),
        ANIArg::MakeForObjectStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);
    ani_object result {};
    ani_status status = GetInteractionAPI(venv)->Primitive_Box_Short(venv->GetEnv(), value, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Unbox_Short(VEnv *venv, VObject *vobject, ani_short *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoxedPrimitiveObject(vobject, "obj", EtsType::SHORT),
        ANIArg::MakeForShortStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Primitive_Unbox_Short(venv->GetEnv(), ResolveToAniRef(vobject), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Box_Int(VEnv *venv, ani_int value, VObject **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForInt(value, "value"),
        ANIArg::MakeForObjectStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);
    ani_object result {};
    ani_status status = GetInteractionAPI(venv)->Primitive_Box_Int(venv->GetEnv(), value, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Unbox_Int(VEnv *venv, VObject *vobject, ani_int *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoxedPrimitiveObject(vobject, "obj", EtsType::INT),
        ANIArg::MakeForIntStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Primitive_Unbox_Int(venv->GetEnv(), ResolveToAniRef(vobject), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Box_Long(VEnv *venv, ani_long value, VObject **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForLong(value, "value"),
        ANIArg::MakeForObjectStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);
    ani_object result {};
    ani_status status = GetInteractionAPI(venv)->Primitive_Box_Long(venv->GetEnv(), value, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Unbox_Long(VEnv *venv, VObject *vobject, ani_long *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoxedPrimitiveObject(vobject, "obj", EtsType::LONG),
        ANIArg::MakeForLongStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Primitive_Unbox_Long(venv->GetEnv(), ResolveToAniRef(vobject), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Box_Float(VEnv *venv, ani_float value, VObject **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForFloat(value, "value"),
        ANIArg::MakeForObjectStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);
    ani_object result {};
    ani_status status = GetInteractionAPI(venv)->Primitive_Box_Float(venv->GetEnv(), value, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Unbox_Float(VEnv *venv, VObject *vobject, ani_float *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoxedPrimitiveObject(vobject, "obj", EtsType::FLOAT),
        ANIArg::MakeForFloatStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Primitive_Unbox_Float(venv->GetEnv(), ResolveToAniRef(vobject), result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Box_Double(VEnv *venv, ani_double value, VObject **vresult)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForDouble(value, "value"),
        ANIArg::MakeForObjectStorage(vresult, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vresult);
    ani_object result {};
    ani_status status = GetInteractionAPI(venv)->Primitive_Box_Double(venv->GetEnv(), value, &result);
    ADD_VERIFIED_LOCAL_REF_IF_OK(status, venv, result, vresult);
    return status;
}

// NOLINTNEXTLINE(readability-identifier-naming)
NO_UB_SANITIZE static ani_status Primitive_Unbox_Double(VEnv *venv, VObject *vobject, ani_double *result)
{
    // clang-format off
    VERIFY_ANI_ARGS(
        ANIArg::MakeForEnv(venv, "env"),
        ANIArg::MakeForBoxedPrimitiveObject(vobject, "obj", EtsType::DOUBLE),
        ANIArg::MakeForDoubleStorage(result, "result"),
    );
    // clang-format on
    CHECK_PTR_ARG(venv);
    CHECK_PTR_ARG(vobject);
    return GetInteractionAPI(venv)->Primitive_Unbox_Double(venv->GetEnv(), ResolveToAniRef(vobject), result);
}

// clang-format off
const __ani_interaction_api VERIFY_INTERACTION_API = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    VERIFY_ANI_CAST_API(GetVersion),
    VERIFY_ANI_CAST_API(GetVM),
    VERIFY_ANI_CAST_API(Object_New),
    VERIFY_ANI_CAST_API(Object_New_A),
    VERIFY_ANI_CAST_API(Object_New_V),
    VERIFY_ANI_CAST_API(Object_GetType),
    VERIFY_ANI_CAST_API(Object_InstanceOf),
    VERIFY_ANI_CAST_API(Type_GetSuperClass),
    VERIFY_ANI_CAST_API(Type_IsAssignableFrom),
    VERIFY_ANI_CAST_API(FindModule),
    VERIFY_ANI_CAST_API(FindNamespace),
    VERIFY_ANI_CAST_API(FindClass),
    VERIFY_ANI_CAST_API(FindEnum),
    VERIFY_ANI_CAST_API(Module_FindFunction),
    VERIFY_ANI_CAST_API(Module_FindVariable),
    VERIFY_ANI_CAST_API(Namespace_FindFunction),
    VERIFY_ANI_CAST_API(Namespace_FindVariable),
    VERIFY_ANI_CAST_API(Module_BindNativeFunctions),
    VERIFY_ANI_CAST_API(Namespace_BindNativeFunctions),
    VERIFY_ANI_CAST_API(Class_BindNativeMethods),
    VERIFY_ANI_CAST_API(Reference_Delete),
    VERIFY_ANI_CAST_API(EnsureEnoughReferences),
    VERIFY_ANI_CAST_API(CreateLocalScope),
    VERIFY_ANI_CAST_API(DestroyLocalScope),
    VERIFY_ANI_CAST_API(CreateEscapeLocalScope),
    VERIFY_ANI_CAST_API(DestroyEscapeLocalScope),
    VERIFY_ANI_CAST_API(ThrowError),
    VERIFY_ANI_CAST_API(ExistUnhandledError),
    VERIFY_ANI_CAST_API(GetUnhandledError),
    VERIFY_ANI_CAST_API(ResetError),
    VERIFY_ANI_CAST_API(DescribeError),
    VERIFY_ANI_CAST_API(Abort),
    VERIFY_ANI_CAST_API(GetNull),
    VERIFY_ANI_CAST_API(GetUndefined),
    VERIFY_ANI_CAST_API(Reference_IsNull),
    VERIFY_ANI_CAST_API(Reference_IsUndefined),
    VERIFY_ANI_CAST_API(Reference_IsNullishValue),
    VERIFY_ANI_CAST_API(Reference_Equals),
    VERIFY_ANI_CAST_API(Reference_StrictEquals),
    VERIFY_ANI_CAST_API(String_NewUTF16),
    VERIFY_ANI_CAST_API(String_GetUTF16Size),
    VERIFY_ANI_CAST_API(String_GetUTF16),
    VERIFY_ANI_CAST_API(String_GetUTF16SubString),
    VERIFY_ANI_CAST_API(String_NewUTF8),
    VERIFY_ANI_CAST_API(String_GetUTF8Size),
    VERIFY_ANI_CAST_API(String_GetUTF8),
    VERIFY_ANI_CAST_API(String_GetUTF8SubString),
    VERIFY_ANI_CAST_API(Array_GetLength),
    VERIFY_ANI_CAST_API(Array_New),
    VERIFY_ANI_CAST_API(Array_Set),
    VERIFY_ANI_CAST_API(Array_Get),
    VERIFY_ANI_CAST_API(Array_Push),
    VERIFY_ANI_CAST_API(Array_Pop),
    VERIFY_ANI_CAST_API(FixedArray_GetLength),
    VERIFY_ANI_CAST_API(FixedArray_New_Boolean),
    VERIFY_ANI_CAST_API(FixedArray_New_Char),
    VERIFY_ANI_CAST_API(FixedArray_New_Byte),
    VERIFY_ANI_CAST_API(FixedArray_New_Short),
    VERIFY_ANI_CAST_API(FixedArray_New_Int),
    VERIFY_ANI_CAST_API(FixedArray_New_Long),
    VERIFY_ANI_CAST_API(FixedArray_New_Float),
    VERIFY_ANI_CAST_API(FixedArray_New_Double),
    VERIFY_ANI_CAST_API(FixedArray_GetRegion_Boolean),
    VERIFY_ANI_CAST_API(FixedArray_GetRegion_Char),
    VERIFY_ANI_CAST_API(FixedArray_GetRegion_Byte),
    VERIFY_ANI_CAST_API(FixedArray_GetRegion_Short),
    VERIFY_ANI_CAST_API(FixedArray_GetRegion_Int),
    VERIFY_ANI_CAST_API(FixedArray_GetRegion_Long),
    VERIFY_ANI_CAST_API(FixedArray_GetRegion_Float),
    VERIFY_ANI_CAST_API(FixedArray_GetRegion_Double),
    VERIFY_ANI_CAST_API(FixedArray_SetRegion_Boolean),
    VERIFY_ANI_CAST_API(FixedArray_SetRegion_Char),
    VERIFY_ANI_CAST_API(FixedArray_SetRegion_Byte),
    VERIFY_ANI_CAST_API(FixedArray_SetRegion_Short),
    VERIFY_ANI_CAST_API(FixedArray_SetRegion_Int),
    VERIFY_ANI_CAST_API(FixedArray_SetRegion_Long),
    VERIFY_ANI_CAST_API(FixedArray_SetRegion_Float),
    VERIFY_ANI_CAST_API(FixedArray_SetRegion_Double),
    VERIFY_ANI_CAST_API(FixedArray_New_Ref),
    VERIFY_ANI_CAST_API(FixedArray_Set_Ref),
    VERIFY_ANI_CAST_API(FixedArray_Get_Ref),
    VERIFY_ANI_CAST_API(Enum_GetEnumItemByName),
    VERIFY_ANI_CAST_API(Enum_GetEnumItemByIndex),
    VERIFY_ANI_CAST_API(EnumItem_GetEnum),
    VERIFY_ANI_CAST_API(EnumItem_GetValue_Int),
    VERIFY_ANI_CAST_API(EnumItem_GetValue_String),
    VERIFY_ANI_CAST_API(EnumItem_GetName),
    VERIFY_ANI_CAST_API(EnumItem_GetIndex),
    VERIFY_ANI_CAST_API(FunctionalObject_Call),
    VERIFY_ANI_CAST_API(Variable_SetValue_Boolean),
    VERIFY_ANI_CAST_API(Variable_SetValue_Char),
    VERIFY_ANI_CAST_API(Variable_SetValue_Byte),
    VERIFY_ANI_CAST_API(Variable_SetValue_Short),
    VERIFY_ANI_CAST_API(Variable_SetValue_Int),
    VERIFY_ANI_CAST_API(Variable_SetValue_Long),
    VERIFY_ANI_CAST_API(Variable_SetValue_Float),
    VERIFY_ANI_CAST_API(Variable_SetValue_Double),
    VERIFY_ANI_CAST_API(Variable_SetValue_Ref),
    VERIFY_ANI_CAST_API(Variable_GetValue_Boolean),
    VERIFY_ANI_CAST_API(Variable_GetValue_Char),
    VERIFY_ANI_CAST_API(Variable_GetValue_Byte),
    VERIFY_ANI_CAST_API(Variable_GetValue_Short),
    VERIFY_ANI_CAST_API(Variable_GetValue_Int),
    VERIFY_ANI_CAST_API(Variable_GetValue_Long),
    VERIFY_ANI_CAST_API(Variable_GetValue_Float),
    VERIFY_ANI_CAST_API(Variable_GetValue_Double),
    VERIFY_ANI_CAST_API(Variable_GetValue_Ref),
    VERIFY_ANI_CAST_API(Function_Call_Boolean),
    VERIFY_ANI_CAST_API(Function_Call_Boolean_A),
    VERIFY_ANI_CAST_API(Function_Call_Boolean_V),
    VERIFY_ANI_CAST_API(Function_Call_Char),
    VERIFY_ANI_CAST_API(Function_Call_Char_A),
    VERIFY_ANI_CAST_API(Function_Call_Char_V),
    VERIFY_ANI_CAST_API(Function_Call_Byte),
    VERIFY_ANI_CAST_API(Function_Call_Byte_A),
    VERIFY_ANI_CAST_API(Function_Call_Byte_V),
    VERIFY_ANI_CAST_API(Function_Call_Short),
    VERIFY_ANI_CAST_API(Function_Call_Short_A),
    VERIFY_ANI_CAST_API(Function_Call_Short_V),
    VERIFY_ANI_CAST_API(Function_Call_Int),
    VERIFY_ANI_CAST_API(Function_Call_Int_A),
    VERIFY_ANI_CAST_API(Function_Call_Int_V),
    VERIFY_ANI_CAST_API(Function_Call_Long),
    VERIFY_ANI_CAST_API(Function_Call_Long_A),
    VERIFY_ANI_CAST_API(Function_Call_Long_V),
    VERIFY_ANI_CAST_API(Function_Call_Float),
    VERIFY_ANI_CAST_API(Function_Call_Float_A),
    VERIFY_ANI_CAST_API(Function_Call_Float_V),
    VERIFY_ANI_CAST_API(Function_Call_Double),
    VERIFY_ANI_CAST_API(Function_Call_Double_A),
    VERIFY_ANI_CAST_API(Function_Call_Double_V),
    VERIFY_ANI_CAST_API(Function_Call_Ref),
    VERIFY_ANI_CAST_API(Function_Call_Ref_A),
    VERIFY_ANI_CAST_API(Function_Call_Ref_V),
    VERIFY_ANI_CAST_API(Function_Call_Void),
    VERIFY_ANI_CAST_API(Function_Call_Void_A),
    VERIFY_ANI_CAST_API(Function_Call_Void_V),
    VERIFY_ANI_CAST_API(Class_FindField),
    VERIFY_ANI_CAST_API(Class_FindStaticField),
    VERIFY_ANI_CAST_API(Class_FindMethod),
    VERIFY_ANI_CAST_API(Class_FindStaticMethod),
    VERIFY_ANI_CAST_API(Class_FindSetter),
    VERIFY_ANI_CAST_API(Class_FindGetter),
    VERIFY_ANI_CAST_API(Class_FindIndexableGetter),
    VERIFY_ANI_CAST_API(Class_FindIndexableSetter),
    VERIFY_ANI_CAST_API(Class_FindIterator),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Boolean),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Char),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Byte),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Short),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Int),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Long),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Float),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Double),
    VERIFY_ANI_CAST_API(Class_GetStaticField_Ref),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Boolean),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Char),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Byte),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Short),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Int),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Long),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Float),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Double),
    VERIFY_ANI_CAST_API(Class_SetStaticField_Ref),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Boolean),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Char),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Byte),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Short),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Int),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Long),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Float),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Double),
    VERIFY_ANI_CAST_API(Class_GetStaticFieldByName_Ref),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Boolean),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Char),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Byte),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Short),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Int),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Long),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Float),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Double),
    VERIFY_ANI_CAST_API(Class_SetStaticFieldByName_Ref),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Boolean),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Boolean_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Boolean_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Char),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Char_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Char_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Byte),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Byte_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Byte_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Short),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Short_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Short_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Int),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Int_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Int_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Long),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Long_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Long_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Float),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Float_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Float_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Double),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Double_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Double_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Ref),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Ref_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Ref_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Void),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Void_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethod_Void_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Boolean),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Boolean_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Boolean_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Char),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Char_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Char_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Byte),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Byte_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Byte_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Short),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Short_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Short_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Int),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Int_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Int_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Long),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Long_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Long_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Float),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Float_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Float_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Double),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Double_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Double_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Ref),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Ref_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Ref_V),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Void),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Void_A),
    VERIFY_ANI_CAST_API(Class_CallStaticMethodByName_Void_V),
    VERIFY_ANI_CAST_API(Object_GetField_Boolean),
    VERIFY_ANI_CAST_API(Object_GetField_Char),
    VERIFY_ANI_CAST_API(Object_GetField_Byte),
    VERIFY_ANI_CAST_API(Object_GetField_Short),
    VERIFY_ANI_CAST_API(Object_GetField_Int),
    VERIFY_ANI_CAST_API(Object_GetField_Long),
    VERIFY_ANI_CAST_API(Object_GetField_Float),
    VERIFY_ANI_CAST_API(Object_GetField_Double),
    VERIFY_ANI_CAST_API(Object_GetField_Ref),
    VERIFY_ANI_CAST_API(Object_SetField_Boolean),
    VERIFY_ANI_CAST_API(Object_SetField_Char),
    VERIFY_ANI_CAST_API(Object_SetField_Byte),
    VERIFY_ANI_CAST_API(Object_SetField_Short),
    VERIFY_ANI_CAST_API(Object_SetField_Int),
    VERIFY_ANI_CAST_API(Object_SetField_Long),
    VERIFY_ANI_CAST_API(Object_SetField_Float),
    VERIFY_ANI_CAST_API(Object_SetField_Double),
    VERIFY_ANI_CAST_API(Object_SetField_Ref),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Boolean),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Char),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Byte),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Short),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Int),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Long),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Float),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Double),
    VERIFY_ANI_CAST_API(Object_GetFieldByName_Ref),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Boolean),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Char),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Byte),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Short),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Int),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Long),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Float),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Double),
    VERIFY_ANI_CAST_API(Object_SetFieldByName_Ref),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Boolean),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Char),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Byte),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Short),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Int),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Long),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Float),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Double),
    VERIFY_ANI_CAST_API(Object_GetPropertyByName_Ref),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Boolean),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Char),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Byte),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Short),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Int),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Long),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Float),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Double),
    VERIFY_ANI_CAST_API(Object_SetPropertyByName_Ref),
    VERIFY_ANI_CAST_API(Object_CallMethod_Boolean),
    VERIFY_ANI_CAST_API(Object_CallMethod_Boolean_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Boolean_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Char),
    VERIFY_ANI_CAST_API(Object_CallMethod_Char_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Char_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Byte),
    VERIFY_ANI_CAST_API(Object_CallMethod_Byte_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Byte_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Short),
    VERIFY_ANI_CAST_API(Object_CallMethod_Short_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Short_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Int),
    VERIFY_ANI_CAST_API(Object_CallMethod_Int_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Int_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Long),
    VERIFY_ANI_CAST_API(Object_CallMethod_Long_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Long_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Float),
    VERIFY_ANI_CAST_API(Object_CallMethod_Float_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Float_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Double),
    VERIFY_ANI_CAST_API(Object_CallMethod_Double_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Double_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Ref),
    VERIFY_ANI_CAST_API(Object_CallMethod_Ref_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Ref_V),
    VERIFY_ANI_CAST_API(Object_CallMethod_Void),
    VERIFY_ANI_CAST_API(Object_CallMethod_Void_A),
    VERIFY_ANI_CAST_API(Object_CallMethod_Void_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Boolean),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Boolean_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Boolean_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Char),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Char_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Char_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Byte),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Byte_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Byte_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Short),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Short_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Short_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Int),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Int_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Int_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Long),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Long_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Long_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Float),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Float_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Float_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Double),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Double_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Double_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Ref),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Ref_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Ref_V),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Void),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Void_A),
    VERIFY_ANI_CAST_API(Object_CallMethodByName_Void_V),
    VERIFY_ANI_CAST_API(TupleValue_GetNumberOfItems),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Boolean),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Char),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Byte),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Short),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Int),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Long),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Float),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Double),
    VERIFY_ANI_CAST_API(TupleValue_GetItem_Ref),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Boolean),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Char),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Byte),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Short),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Int),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Long),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Float),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Double),
    VERIFY_ANI_CAST_API(TupleValue_SetItem_Ref),
    VERIFY_ANI_CAST_API(GlobalReference_Create),
    VERIFY_ANI_CAST_API(GlobalReference_Delete),
    VERIFY_ANI_CAST_API(WeakReference_Create),
    VERIFY_ANI_CAST_API(WeakReference_Delete),
    VERIFY_ANI_CAST_API(WeakReference_GetReference),
    VERIFY_ANI_CAST_API(CreateArrayBuffer),
    VERIFY_ANI_CAST_API(ArrayBuffer_GetInfo),
    VERIFY_ANI_CAST_API(Promise_New),
    VERIFY_ANI_CAST_API(PromiseResolver_Resolve),
    VERIFY_ANI_CAST_API(PromiseResolver_Reject),
    VERIFY_ANI_CAST_API(Any_InstanceOf),
    VERIFY_ANI_CAST_API(Any_GetProperty),
    VERIFY_ANI_CAST_API(Any_SetProperty),
    VERIFY_ANI_CAST_API(Any_GetByIndex),
    VERIFY_ANI_CAST_API(Any_SetByIndex),
    VERIFY_ANI_CAST_API(Any_GetByValue),
    VERIFY_ANI_CAST_API(Any_SetByValue),
    VERIFY_ANI_CAST_API(Any_Call),
    VERIFY_ANI_CAST_API(Any_CallMethod),
    VERIFY_ANI_CAST_API(Any_New),
    VERIFY_ANI_CAST_API(Class_BindStaticNativeMethods),
    VERIFY_ANI_CAST_API(Primitive_Box_Boolean),
    VERIFY_ANI_CAST_API(Primitive_Unbox_Boolean),
    VERIFY_ANI_CAST_API(Primitive_Box_Byte),
    VERIFY_ANI_CAST_API(Primitive_Unbox_Byte),
    VERIFY_ANI_CAST_API(Primitive_Box_Char),
    VERIFY_ANI_CAST_API(Primitive_Unbox_Char),
    VERIFY_ANI_CAST_API(Primitive_Box_Short),
    VERIFY_ANI_CAST_API(Primitive_Unbox_Short),
    VERIFY_ANI_CAST_API(Primitive_Box_Int),
    VERIFY_ANI_CAST_API(Primitive_Unbox_Int),
    VERIFY_ANI_CAST_API(Primitive_Box_Long),
    VERIFY_ANI_CAST_API(Primitive_Unbox_Long),
    VERIFY_ANI_CAST_API(Primitive_Box_Float),
    VERIFY_ANI_CAST_API(Primitive_Unbox_Float),
    VERIFY_ANI_CAST_API(Primitive_Box_Double),
    VERIFY_ANI_CAST_API(Primitive_Unbox_Double),
};
// clang-format on

const __ani_interaction_api *GetVerifyInteractionAPI()
{
    return &VERIFY_INTERACTION_API;
}
}  // namespace ark::ets::ani::verify
