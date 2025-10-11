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

#include <ani.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <node_api.h>
#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include "ets_coroutine.h"
#include "ets_vm_STValue.h"
#include "include/mem/panda_containers.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"
#include "plugins/ets/runtime/ets_utils.h"

#include "generated/logger_options.h"
#include "compiler_options.h"
#include "compiler/compiler_logger.h"
#include "interop_js/napi_impl/napi_impl.h"
#include "runtime/include/runtime.h"

namespace ark::ets::interop::js {
static napi_value STValueTemplateInvokeFunction(
    napi_env env, [[maybe_unused]] napi_callback_info info,
    const std::function<ani_function(STValueData *, std::string &, std::string &)> &findFunction)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    napi_value jsThis;
    napi_value jsArgv[3];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 3U) {
        ThrowJSBadArgCountError(env, jsArgc, 3U);
        return nullptr;
    }

    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    // 1. extract function name and signature
    napi_value jsFunctionName = jsArgv[0];
    napi_value jsSignatureName = jsArgv[1];
    auto functionName = GetString(env, jsFunctionName);
    auto signatureName = GetString(env, jsSignatureName);

    // 2. extract function
    STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    ani_function aniFunction = findFunction(data, functionName, signatureName);
    if (aniFunction == nullptr) {
        return nullptr;
    }

    // 3. extract invoke params
    napi_value jsInputParams = jsArgv[2];
    bool isArray = false;
    NAPI_CHECK_FATAL(napi_is_array(env, jsInputParams, &isArray));
    if (!isArray) {
        STValueThrowJSError(env, "Input params are not array type.");
        return nullptr;
    }

    uint32_t arrayLength = 0;
    NAPI_CHECK_FATAL(napi_get_array_length(env, jsInputParams, &arrayLength));
    std::vector<ani_value> args(arrayLength);
    for (uint32_t i = 0; i < arrayLength; i++) {
        napi_value element;
        NAPI_CHECK_FATAL(napi_get_element(env, jsInputParams, i, &element));
        args[i] = GetAniValueFromSTValue(env, element);
    }

    // 4. extract returnType
    SType returnType = ParseReturnTypeFromSignature(env, signatureName);

    // 5. invoke func according to returnType
    auto aniEnv = GetAniEnv();
    switch (returnType) {  //    std::variant<ani_ref, ani_boolean, ani_char, ani_byte, ani_short, ani_int, ani_long,
                           //    ani_float, ani_double> DataRef;
        case SType::BOOLEAN: {
            ani_boolean invokeResult = ANI_FALSE;
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Boolean_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_boolean>(env, static_cast<ani_boolean>(invokeResult));
        }
        case SType::CHAR: {
            ani_char invokeResult = 0;
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Char_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_char>(env, static_cast<ani_char>(invokeResult));
        }
        case SType::BYTE: {
            ani_byte invokeResult = 0;
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Byte_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_byte>(env, static_cast<ani_byte>(invokeResult));
        }
        case SType::SHORT: {
            ani_short invokeResult = 0;
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Short_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_short>(env, static_cast<ani_short>(invokeResult));
        }
        case SType::INT: {
            ani_int invokeResult = 0;
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Int_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_int>(env, static_cast<ani_int>(invokeResult));
        }
        case SType::LONG: {
            ani_long invokeResult = 0;
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Long_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_long>(env, static_cast<ani_long>(invokeResult));
        }
        case SType::FLOAT: {
            ani_float invokeResult = 0;
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Float_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_float>(env, static_cast<ani_float>(invokeResult));
        }
        case SType::DOUBLE: {
            ani_double invokeResult = 0;
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Double_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_double>(env, static_cast<ani_double>(invokeResult));
        }
        case SType::REFERENCE: {
            ani_ref invokeResult {};
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Ref_A(aniFunction, &invokeResult, args.data()));
            return CreateSTValueInstance<ani_ref>(env, std::move(invokeResult));
        }
        case SType::VOID: {
            AniCheckAndThrowToDynamic(env, aniEnv->Function_Call_Void_A(aniFunction, args.data()));
            ani_ref undefinedRef {};
            AniCheckAndThrowToDynamic(env, aniEnv->GetUndefined(&undefinedRef));
            return CreateSTValueInstance<ani_ref>(env, std::move(undefinedRef));
        }
        default: {
            ThrowUnsupportedSTypeError(env, returnType);
        }
    }
    return nullptr;
}

napi_value STValueModuleInvokeFunctionImpl(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    auto moduleFindFunction = [env](STValueData *data, std::string &functionName,
                                    std::string &signatureName) -> ani_function {
        auto aniEnv = GetAniEnv();
        ani_module aniModule = reinterpret_cast<ani_module>(data->GetAniRef());
        ani_function aniFunction = nullptr;
        AniCheckAndThrowToDynamic(
            env, aniEnv->Module_FindFunction(aniModule, functionName.c_str(), signatureName.c_str(), &aniFunction));

        return aniFunction;
    };

    return STValueTemplateInvokeFunction(env, info, moduleFindFunction);
}

napi_value STValueNamespaceInvokeFunctionImpl(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    auto namespaceFindFunction = [env](STValueData *data, std::string &functionName,
                                       std::string &signatureName) -> ani_function {
        auto aniEnv = GetAniEnv();
        ani_namespace aniNsp = reinterpret_cast<ani_namespace>(data->GetAniRef());
        ani_function aniFunction = nullptr;
        AniCheckAndThrowToDynamic(
            env, aniEnv->Namespace_FindFunction(aniNsp, functionName.c_str(), signatureName.c_str(), &aniFunction));
        return aniFunction;
    };

    return STValueTemplateInvokeFunction(env, info, namespaceFindFunction);
}

napi_value STValueFunctionalObjectInvokeImpl(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;
    auto *aniEnv = GetAniEnv();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 1) {
        ThrowJSBadArgCountError(env, jsArgc, 1);
        return nullptr;
    }

    napi_value jsThis {};
    napi_value jsArgv[1];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));
    if (!CheckNapiIsArray(env, jsArgv[0])) {
        return nullptr;
    }

    uint32_t argLength = 0;
    NAPI_CHECK_FATAL(napi_get_array_length(env, jsArgv[0], &argLength));

    std::vector<ani_ref> argArray(argLength);
    for (uint32_t argIdx = 0; argIdx < argLength; argIdx++) {
        napi_value jsVal {};
        NAPI_CHECK_FATAL(napi_get_element(env, jsArgv[0], argIdx, &jsVal));
        STValueData *data = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsVal));
        if (!data->IsAniRef()) {
            ThrowJSNonObjectError(env, "in args " + std::to_string(argIdx) + "-th arg");
            return nullptr;
        }
        argArray[argIdx] = data->GetAniRef();
    }

    STValueData *fnData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (fnData == nullptr || !fnData->IsAniRef()) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_fn_object fnObj = static_cast<ani_fn_object>(fnData->GetAniRef());

    ani_ref fnResult {};
    AniCheckAndThrowToDynamic(env, aniEnv->FunctionalObject_Call(fnObj, argLength, argArray.data(), &fnResult));

    return CreateSTValueInstance(env, fnResult);
}

napi_value ObjectInvokeMethodWithNoArgs(napi_env env, ani_object invokeObj, const std::string &nameString,
                                        const std::string &signatureString, SType returnType)
{
    auto aniEnv = GetAniEnv();
    auto name = nameString.c_str();
    auto signature = signatureString.c_str();
    switch (returnType) {
        case SType::BOOLEAN: {
            ani_boolean fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Object_CallMethodByName_Boolean(invokeObj, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::CHAR: {
            ani_char fnResult {};
            AniCheckAndThrowToDynamic(env, aniEnv->Object_CallMethodByName_Char(invokeObj, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::BYTE: {
            ani_byte fnResult {};
            AniCheckAndThrowToDynamic(env, aniEnv->Object_CallMethodByName_Byte(invokeObj, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::SHORT: {
            ani_short fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Object_CallMethodByName_Short(invokeObj, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::INT: {
            ani_int fnResult {};
            AniCheckAndThrowToDynamic(env, aniEnv->Object_CallMethodByName_Int(invokeObj, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::LONG: {
            ani_long fnResult {};
            AniCheckAndThrowToDynamic(env, aniEnv->Object_CallMethodByName_Long(invokeObj, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::FLOAT: {
            ani_float fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Object_CallMethodByName_Float(invokeObj, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::DOUBLE: {
            ani_double fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Object_CallMethodByName_Double(invokeObj, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::REFERENCE: {
            ani_ref fnResult {};
            aniEnv->Object_CallMethodByName_Ref(invokeObj, name, signature, &fnResult);
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::VOID: {
            aniEnv->Object_CallMethodByName_Void(invokeObj, name, signature);
            ani_ref result {};
            AniCheckAndThrowToDynamic(env, aniEnv->GetUndefined(&result));
            return CreateSTValueInstance(env, result);
        }
        default: {
            ThrowUnsupportedSTypeError(env, returnType);
        }
    }
    return nullptr;
}

napi_value ObjectInvokeMethodWithArgs(napi_env env, ani_object invokeObj, const std::string &nameString,
                                      const std::string &signatureString, SType returnType,
                                      std::vector<ani_value> argArray)
{
    auto aniEnv = GetAniEnv();
    auto name = nameString.c_str();
    auto signature = signatureString.c_str();
    auto argArrayData = argArray.data();
    switch (returnType) {
        case SType::BOOLEAN: {
            ani_boolean fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Object_CallMethodByName_Boolean_A(invokeObj, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::CHAR: {
            ani_char fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Object_CallMethodByName_Char_A(invokeObj, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::BYTE: {
            ani_byte fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Object_CallMethodByName_Byte_A(invokeObj, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::SHORT: {
            ani_short fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Object_CallMethodByName_Short_A(invokeObj, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::INT: {
            ani_int fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Object_CallMethodByName_Int_A(invokeObj, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::LONG: {
            ani_long fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Object_CallMethodByName_Long_A(invokeObj, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::FLOAT: {
            ani_float fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Object_CallMethodByName_Float_A(invokeObj, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::DOUBLE: {
            ani_double fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Object_CallMethodByName_Double_A(invokeObj, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::REFERENCE: {
            ani_ref fnResult {};
            aniEnv->Object_CallMethodByName_Ref_A(invokeObj, name, signature, &fnResult, argArrayData);
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::VOID: {
            aniEnv->Object_CallMethodByName_Void_A(invokeObj, name, signature, argArrayData);
            ani_ref result {};
            AniCheckAndThrowToDynamic(env, aniEnv->GetUndefined(&result));
            return CreateSTValueInstance(env, result);
        }
        default: {
            ThrowUnsupportedSTypeError(env, returnType);
        }
    }
    return nullptr;
}

// objectInvokeMethod(name: string, signature: string, returnType: STtype, args: Array<STValue>): STValue
napi_value STValueObjectInvokeMethodImpl(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 3U) {
        ThrowJSBadArgCountError(env, jsArgc, 3U);
        return nullptr;
    }

    napi_value jsThis {};
    napi_value jsArgv[3];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::vector<ani_value> argArray;
    if (!GetArrayFromNapiValue(env, jsArgv[2U], argArray)) {
        return nullptr;
    }

    std::string nameString = GetString(env, jsArgv[0]);
    std::string signatureString = GetString(env, jsArgv[1]);
    STValueData *objData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (objData == nullptr || !objData->IsAniRef()) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_object invokeObj = reinterpret_cast<ani_object>(objData->GetAniRef());

    SType returnType = ParseReturnTypeFromSignature(env, signatureString);
    if (argArray.empty()) {
        return ObjectInvokeMethodWithNoArgs(env, invokeObj, nameString, signatureString, returnType);
    }
    return ObjectInvokeMethodWithArgs(env, invokeObj, nameString, signatureString, returnType, argArray);
}

napi_value ClassInvokeStaticMethodWithNoArgs(napi_env env, ani_class clsClass, const std::string &nameString,
                                             const std::string &signatureString, SType returnType)
{
    auto aniEnv = GetAniEnv();
    auto name = nameString.c_str();
    auto signature = signatureString.c_str();
    switch (returnType) {
        case SType::BOOLEAN: {
            ani_boolean fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Boolean(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::CHAR: {
            ani_char fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Class_CallStaticMethodByName_Char(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::BYTE: {
            ani_byte fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Class_CallStaticMethodByName_Byte(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::SHORT: {
            ani_short fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Class_CallStaticMethodByName_Short(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::INT: {
            ani_int fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Class_CallStaticMethodByName_Int(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::LONG: {
            ani_long fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Class_CallStaticMethodByName_Long(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::FLOAT: {
            ani_float fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Class_CallStaticMethodByName_Float(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::DOUBLE: {
            double fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Double(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::REFERENCE: {
            ani_ref fnResult {};
            AniCheckAndThrowToDynamic(env,
                                      aniEnv->Class_CallStaticMethodByName_Ref(clsClass, name, signature, &fnResult));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::VOID: {
            AniCheckAndThrowToDynamic(env, aniEnv->Class_CallStaticMethodByName_Void(clsClass, name, signature));
            ani_ref result {};
            AniCheckAndThrowToDynamic(env, aniEnv->GetUndefined(&result));
            return CreateSTValueInstance(env, result);
        }
        default: {
            ThrowUnsupportedSTypeError(env, returnType);
        }
    }
    return nullptr;
}

napi_value ClassInvokeStaticMethodWitArgs(napi_env env, ani_class clsClass, const std::string &nameString,
                                          const std::string &signatureString, SType returnType,
                                          std::vector<ani_value> argArray)
{
    auto aniEnv = GetAniEnv();
    auto name = nameString.c_str();
    auto signature = signatureString.c_str();
    auto argArrayData = argArray.data();
    switch (returnType) {
        case SType::BOOLEAN: {
            ani_boolean fnResult {};
            AniCheckAndThrowToDynamic(env, aniEnv->Class_CallStaticMethodByName_Boolean_A(clsClass, name, signature,
                                                                                          &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::CHAR: {
            ani_char fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Char_A(clsClass, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::BYTE: {
            ani_byte fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Byte_A(clsClass, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::SHORT: {
            ani_short fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Short_A(clsClass, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::INT: {
            ani_int fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Int_A(clsClass, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::LONG: {
            ani_long fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Long_A(clsClass, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::FLOAT: {
            ani_float fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Float_A(clsClass, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::DOUBLE: {
            ani_double fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Double_A(clsClass, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::REFERENCE: {
            ani_ref fnResult {};
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Ref_A(clsClass, name, signature, &fnResult, argArrayData));
            return CreateSTValueInstance(env, fnResult);
        }
        case SType::VOID: {
            AniCheckAndThrowToDynamic(
                env, aniEnv->Class_CallStaticMethodByName_Void_A(clsClass, name, signature, argArrayData));
            ani_ref result {};
            AniCheckAndThrowToDynamic(env, aniEnv->GetUndefined(&result));
            return CreateSTValueInstance(env, result);
        }
        default: {
            ThrowUnsupportedSTypeError(env, returnType);
        }
    }
    return nullptr;
}

// classInvokeStaticMethod(name: string, signature: string, returnType: STtype, args: Array<STValue>): STValue
napi_value STValueClassInvokeStaticMethodImpl(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    NAPI_TO_ANI_SCOPE;

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 3U) {
        ThrowJSBadArgCountError(env, jsArgc, 3U);
        return nullptr;
    }

    napi_value jsThis {};
    napi_value jsArgv[3];
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv, &jsThis, nullptr));

    std::vector<ani_value> argArray;
    if (!GetArrayFromNapiValue(env, jsArgv[2U], argArray)) {
        return nullptr;
    }

    std::string nameString = GetString(env, jsArgv[0]);
    std::string signatureString = GetString(env, jsArgv[1]);
    STValueData *clsData = reinterpret_cast<STValueData *>(GetSTValueDataPtr(env, jsThis));
    if (clsData == nullptr || !clsData->IsAniRef()) {
        ThrowJSThisNonObjectError(env);
        return nullptr;
    }
    ani_class clsClass = reinterpret_cast<ani_class>(clsData->GetAniRef());

    SType returnType = ParseReturnTypeFromSignature(env, signatureString);
    if (argArray.empty()) {
        return ClassInvokeStaticMethodWithNoArgs(env, clsClass, nameString, signatureString, returnType);
    }
    return ClassInvokeStaticMethodWitArgs(env, clsClass, nameString, signatureString, returnType, argArray);
}
}  // namespace ark::ets::interop::js