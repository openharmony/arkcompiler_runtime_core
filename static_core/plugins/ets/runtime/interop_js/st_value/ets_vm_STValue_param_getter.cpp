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

namespace ark::ets::interop::js {
bool GetString(napi_env env, napi_value jsVal, const std::string &paramName, std::string &paramValue)
{
    napi_valuetype paramType;
    napi_status status = napi_typeof(env, jsVal, &paramType);
    if (status != napi_ok) {
        STValueThrowJSError(env, "Failed to get param type; param=" + paramName);
        return false;
    }

    if (paramType != napi_string) {
        STValueThrowJSError(env, "param are not string type; param=" + paramName);
        return false;
    }

    size_t length = 0;
    NAPI_CHECK_FATAL(napi_get_value_string_utf8(env, jsVal, nullptr, 0, &length));
    paramValue.resize(length + 1, '\0');
    NAPI_CHECK_FATAL(napi_get_value_string_utf8(env, jsVal, paramValue.data(), length + 1, &length));
    paramValue.resize(length);
    return true;
}

bool GetArray(napi_env env, napi_value jsVal, const std::string &arrayName, std::vector<ani_value> &arrayValue)
{
    // 1. check array type
    bool isArray = false;
    NAPI_CHECK_FATAL(napi_is_array(env, jsVal, &isArray));
    if (!isArray) {
        STValueThrowJSError(env, "param are not array type; param=" + arrayName);
        return false;
    }

    // 2. get array length
    uint32_t arrayLength = 0;
    NAPI_CHECK_FATAL(napi_get_array_length(env, jsVal, &arrayLength));
    arrayValue.reserve(arrayLength);

    // 3. fill array element
    for (uint32_t i = 0; i < arrayLength; i++) {
        napi_value element {};
        ani_value value {};
        NAPI_CHECK_FATAL(napi_get_element(env, jsVal, i, &element));
        if (!GetAniValueFromSTValue(env, element, value)) {
            STValueThrowJSError(env, "param array element are not STValue type; param=" + arrayName +
                                         "; index=" + std::to_string(i));
            return false;
        }
        arrayValue.push_back(value);
    }
    return true;
}

bool GetReturnType(napi_env env, const std::string &signature, SType &type)
{
    // 1. there is not : in signature
    size_t colonPos = signature.find(':');
    if (colonPos == std::string::npos) {
        STValueThrowJSError(env, "can not find : in signature;signature=" + signature);
        return false;
    }

    // 2. : is the last char
    if (colonPos == signature.length() - 1) {
        type = SType::VOID;
        return true;
    }

    constexpr size_t colonPositionFromEnd = 2;
    if (colonPos != signature.length() - colonPositionFromEnd) {
        type = SType::REFERENCE;
        return true;
    }

    static const std::unordered_map<char, SType> CHAR_MAP = {
        {'z', SType::BOOLEAN}, {'b', SType::BYTE}, {'c', SType::CHAR},  {'s', SType::SHORT},
        {'i', SType::INT},     {'l', SType::LONG}, {'f', SType::FLOAT}, {'d', SType::DOUBLE}};

    auto returnTypeSign = signature[colonPos + 1];
    auto it = CHAR_MAP.find(returnTypeSign);
    if (it != CHAR_MAP.end()) {
        type = it->second;
        return true;
    }

    std::string msg = "Illegal signature, no matching type for \'";
    msg = msg + returnTypeSign + "\'" + ";signature=" + signature;
    STValueThrowJSError(env, msg);
    return false;
}
}  // namespace ark::ets::interop::js