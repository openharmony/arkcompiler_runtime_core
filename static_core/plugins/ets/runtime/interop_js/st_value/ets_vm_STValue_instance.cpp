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

#include <ani.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <node_api.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include "ets_coroutine.h"
#include "ets_vm_STValue.h"
#include "include/mem/panda_containers.h"
#include "interop_js/interop_context.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"

#include "compiler_options.h"
#include "compiler/compiler_logger.h"
#include "interop_js/napi_impl/napi_impl.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "runtime/include/runtime.h"

#include "plugins/ets/runtime/interop_js/interop_context_api.h"
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue.h"
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue_param_getter.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_object.h"

// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak)) napi_define_properties(napi_env env, napi_value object, size_t property_count,
                                                         const napi_property_descriptor *properties);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)

namespace ark::ets::interop::js {

napi_value STValueClassInstantiateImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    auto *aniEnv = GetAniEnv();
    AniLocalScope aniScope(aniEnv);

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    napi_value jsThis;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    napi_value jsArgv[2];

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::vector<ani_value> argArray;
    if (!GetArray(env, jsArgv[1], "args", argArray)) {
        return nullptr;
    }

    auto *clsData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (clsData == nullptr || !clsData->IsAniRef() || clsData->IsAniNullOrUndefined(env)) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    auto clsClass = static_cast<ani_class>(clsData->GetAniRef());

    std::string ctorString {};
    if (!GetString(env, jsArgv[0], "ClassCtor", ctorString)) {
        return nullptr;
    }

    ani_method ctorMethod {};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Class_FindMethod(clsClass, "<ctor>", ctorString.c_str(), &ctorMethod));

    if (ctorMethod == nullptr) {
        STValueThrowJSError(env, "Instantiate ctor is Null!;");
        return nullptr;
    }

    ani_object newObject {};
    if (!argArray.empty()) {
        ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_New_A(clsClass, ctorMethod, &newObject, argArray.data()));
    } else {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ANI_CHECK_ERROR_RETURN(env, aniEnv->Object_New(clsClass, ctorMethod, &newObject));
    }
    return CreateSTValueInstance(env, newObject);
}

// static newFixedArrayPrimitive(len: number, elementType: SType): STValue
napi_value STValueNewFixedArrayPrimitiveImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    auto *aniEnv = GetAniEnv();
    AniLocalScope aniScope(aniEnv);

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    napi_value jsArgv[2];

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, nullptr, nullptr));

    napi_valuetype lengthType {};
    napi_typeof(env, jsArgv[0], &lengthType);
    if (lengthType != napi_number) {
        STValueThrowJSError(env, "length type is not number type;");
        return nullptr;
    }
    int32_t arrLength;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, jsArgv[0], &arrLength));
    if (arrLength < 0) {
        STValueThrowJSError(env, "length must be non-negative;");
        return nullptr;
    }

    SType arrayType = GetTypeFromType(env, jsArgv[1]);

    // Element size mapping table
    static constexpr std::array<size_t, 8> ELEMENT_SIZES = {
        sizeof(ani_boolean),  // SType::BOOLEAN
        sizeof(ani_byte),     // SType::BYTE
        sizeof(ani_char),     // SType::CHAR
        sizeof(ani_short),    // SType::SHORT
        sizeof(ani_int),      // SType::INT
        sizeof(ani_long),     // SType::LONG
        sizeof(ani_float),    // SType::FLOAT
        sizeof(ani_double)    // SType::DOUBLE
    };

    if (arrayType < SType::BOOLEAN || arrayType > SType::DOUBLE) {
        ThrowUnsupportedSTypeError(env, arrayType);
        return nullptr;
    }

    size_t elementSize = ELEMENT_SIZES[static_cast<int>(arrayType)];
    size_t bindingSize = std::max(elementSize * arrLength, COMPUTED_STVALUE_BINDING_SIZE);
    auto [jsValue, stvalueData] = CreateSTValueInstance(env, bindingSize);

    ani_fixedarray fixArray {};
    switch (arrayType) {
        case SType::BOOLEAN: {
            ani_fixedarray_boolean boolArray;
            ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Boolean(arrLength, &boolArray));
            fixArray = boolArray;
            break;
        }
        case SType::CHAR: {
            ani_fixedarray_char charArray {};
            ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Char(arrLength, &charArray));
            fixArray = charArray;
            break;
        }
        case SType::BYTE: {
            ani_fixedarray_byte byteArray {};
            ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Byte(arrLength, &byteArray));
            fixArray = byteArray;
            break;
        }
        case SType::SHORT: {
            ani_fixedarray_short shortArray {};
            ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Short(arrLength, &shortArray));
            fixArray = shortArray;
            break;
        }
        case SType::INT: {
            ani_fixedarray_int intArray {};
            ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Int(arrLength, &intArray));
            fixArray = intArray;
            break;
        }
        case SType::LONG: {
            ani_fixedarray_long longArray {};
            ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Long(arrLength, &longArray));
            fixArray = longArray;
            break;
        }
        case SType::FLOAT: {
            ani_fixedarray_float floatArray {};
            ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Float(arrLength, &floatArray));
            fixArray = floatArray;
            break;
        }
        case SType::DOUBLE: {
            ani_fixedarray_double doubleArray {};
            ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Double(arrLength, &doubleArray));
            fixArray = doubleArray;
            break;
        }
        default: {
            ThrowUnsupportedSTypeError(env, arrayType);
            return nullptr;
        }
    }

    stvalueData->SetAniRef(env, static_cast<ani_ref>(fixArray));
    return jsValue;
}

// static newFixedArrayReference(len: number, elementType: STValue, initialElement: STValue): STValue
napi_value STValueNewFixedArrayReferenceImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    auto *aniEnv = GetAniEnv();
    AniLocalScope aniScope(aniEnv);

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 3U) {
        return nullptr;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    napi_value jsArgv[3];

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, nullptr, nullptr));

    int32_t arrLength;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, jsArgv[0], &arrLength));
    if (arrLength < 0) {
        STValueThrowJSError(env, "length must be non-negative;");
        return nullptr;
    }
    size_t bindingSize = std::max(sizeof(ani_ref) * arrLength, COMPUTED_STVALUE_BINDING_SIZE);

    auto *typeData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsArgv[1]));
    if (typeData == nullptr || !typeData->IsAniRef() || typeData->IsAniNullOrUndefined(env)) {
        ThrowJSNonObjectError(env, "elementType");
        return nullptr;
    }
    auto *initData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsArgv[2]));
    if (initData == nullptr || !initData->IsAniRef()) {
        ThrowJSNonObjectError(env, "initialElement");
        return nullptr;
    }

    auto [jsSTValue, stvalueData] = CreateSTValueInstance(env, bindingSize);

    auto arrayType = static_cast<ani_type>(typeData->GetAniRef());
    auto initValue = static_cast<ani_ref>(initData->GetAniRef());

    ani_fixedarray_ref fixRefArray {};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->FixedArray_New_Ref(arrayType, arrLength, initValue, &fixRefArray));

    stvalueData->SetAniRef(env, static_cast<ani_ref>(fixRefArray));
    return jsSTValue;
}

napi_value STValueNewSTArrayImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        InteropCtx::ThrowJSError(env, "NewSTArray: bad args, actual args count: " + std::to_string(jsArgc));
        return nullptr;
    }
    EtsExecutionContext *execCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(execCtx);

    INTEROP_CODE_SCOPE_JS_TO_ETS(execCtx);
    ScopedManagedCodeThread managedScope(execCtx->GetMT());

    EtsClass *arrayClass = PlatformTypes()->coreArray;
    EtsObject *arrayInstance = arrayClass->CreateInstance();
    if (UNLIKELY(arrayInstance == nullptr)) {
        ASSERT(execCtx->GetMT()->HasPendingException());
        InteropCtx::ThrowJSError(env, "Failed to create st.Array");
        return nullptr;
    }

    ets_proxy::EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();
    auto wrapper = cache->Lookup(arrayClass);
    return wrapper->Wrap(ctx, arrayInstance);
}

EtsObject *CreateInstanceWithInt(napi_env env, EtsClass *etsClass, int32_t value)
{
    EtsMethod *ctor = etsClass->GetDirectMethod(panda_file_items::CTOR.data(), "I:V");
    if (UNLIKELY(ctor == nullptr) || !ctor->IsPublic()) {
        InteropCtx::ThrowJSError(env, "No default public constructor in");
        return nullptr;
    }

    EtsExecutionContext *execCtx = EtsExecutionContext::GetCurrent();
    EtsClassLinker *linker = execCtx->GetPandaVM()->GetClassLinker();
    if (UNLIKELY(!etsClass->IsInitialized() && !linker->InitializeClass(execCtx, etsClass))) {
        return nullptr;
    }
    EtsObject *obj = EtsObject::Create(etsClass);
    if (UNLIKELY(obj == nullptr)) {
        return nullptr;
    }

    [[maybe_unused]] EtsHandleScope scope(execCtx);
    EtsHandle<EtsObject> objHandle(execCtx, obj);
    const size_t ctorArgCount = 2;
    std::array<Value, ctorArgCount> args {Value(obj->GetCoreType()), Value(value)};
    ctor->GetPandaMethod()->Invoke(execCtx->GetMT(), args.data());
    if (UNLIKELY(execCtx->GetMT()->HasPendingException())) {
        return nullptr;
    }
    return objHandle.GetPtr();
}

napi_value STValueNewSTSetImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        InteropCtx::ThrowJSError(env, "NewSTSet: bad args, actual args count: " + std::to_string(jsArgc));
        return nullptr;
    }
    EtsExecutionContext *execCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(execCtx);

    INTEROP_CODE_SCOPE_JS_TO_ETS(execCtx);
    ScopedManagedCodeThread managedScope(execCtx->GetMT());

    EtsClass *setClass = PlatformTypes()->coreSet;
    EtsObject *setInstance = CreateInstanceWithInt(env, setClass, 8);
    if (UNLIKELY(setInstance == nullptr)) {
        ASSERT(execCtx->GetMT()->HasPendingException());
        InteropCtx::ThrowJSError(env, "Failed to create st.Set");
        return nullptr;
    }

    ets_proxy::EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();
    auto wrapper = cache->Lookup(setClass);
    return wrapper->Wrap(ctx, setInstance);
}

napi_value STValueNewSTMapImpl(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 0) {
        InteropCtx::ThrowJSError(env, "NewSTMap: bad args, actual args count: " + std::to_string(jsArgc));
        return nullptr;
    }
    EtsExecutionContext *execCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(execCtx);

    INTEROP_CODE_SCOPE_JS_TO_ETS(execCtx);
    ScopedManagedCodeThread managedScope(execCtx->GetMT());

    EtsClass *mapClass = PlatformTypes()->coreMap;
    EtsObject *mapInstance = CreateInstanceWithInt(env, mapClass, 8);
    if (UNLIKELY(mapInstance == nullptr)) {
        ASSERT(execCtx->GetMT()->HasPendingException());
        InteropCtx::ThrowJSError(env, "Failed to create st.Map");
        return nullptr;
    }

    ets_proxy::EtsClassWrappersCache *cache = ctx->GetEtsClassWrappersCache();
    auto wrapper = cache->Lookup(mapClass);
    return wrapper->Wrap(ctx, mapInstance);
}

// static newArray(len: number, initialElement: STValue): STValue
napi_value STValueNewArrayImpl(napi_env env, napi_callback_info info)
{
    INTEROP_TRACE();
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    auto *aniEnv = GetAniEnv();
    AniLocalScope aniScope(aniEnv);

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 2U) {
        ThrowJSBadArgCountError(env, jsArgc, 2U);
        return nullptr;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    napi_value jsArgv[2];

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, nullptr, nullptr));

    napi_valuetype lengthType {};
    napi_typeof(env, jsArgv[0], &lengthType);
    if (lengthType != napi_number) {
        STValueThrowJSError(env, "length type is not number type;");
        return nullptr;
    }

    int32_t arrLength;
    NAPI_CHECK_FATAL(napi_get_value_int32(env, jsArgv[0], &arrLength));
    if (arrLength < 0) {
        STValueThrowJSError(env, "length must be non-negative;");
        return nullptr;
    }

    size_t bindingSize = std::max(sizeof(ani_ref) * arrLength, COMPUTED_STVALUE_BINDING_SIZE);

    napi_value jsElement = jsArgv[1];
    auto *elementData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsElement));

    if (elementData == nullptr || !elementData->IsAniRef()) {
        ThrowJSNonObjectError(env, "initialElement");
        return nullptr;
    }

    auto [jsValue, stvalueData] = CreateSTValueInstance(env, bindingSize);

    auto initValue = static_cast<ani_ref>(elementData->GetAniRef());
    ani_array newArray {};
    ANI_CHECK_ERROR_RETURN(env, aniEnv->Array_New(arrLength, initValue, &newArray));

    stvalueData->SetAniRef(env, static_cast<ani_ref>(newArray));
    return jsValue;
}
}  // namespace ark::ets::interop::js
