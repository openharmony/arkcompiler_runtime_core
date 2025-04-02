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
#include <array>
#include <string>

#include "api/ani_textdecoder.h"
#include "api/ani_textencoder.h"
#include "api/Util.h"
#include "ohos/init_data.h"
#include "tools/format_logger.h"
#include "stdlib/native/core/stdlib_ani_helpers.h"

using TextDecoder = ark::ets::sdk::util::TextDecoder;

struct ArrayBufferInfo {
    void *data;
    size_t length;
    ani_status retCode;
};

static ArrayBufferInfo GetArrayInfo([[maybe_unused]] ani_env *env, ani_arraybuffer buffer)
{
    void *data;
    size_t length;
    ani_status retCode = env->ArrayBuffer_GetInfo(buffer, &data, &length);
    return {data, length, retCode};
}

static char *GetUint8ArrayInfo(ani_env *env, ani_object array, int32_t &byteLength, int32_t &byteOffset)
{
    ani_ref buffer;
    if (auto retCode = env->Object_GetFieldByName_Ref(array, "buffer", &buffer); retCode != ANI_OK) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: env->Object_GetFieldByName_Ref() failed");
        return nullptr;
    }
    auto bufferInfo = GetArrayInfo(env, static_cast<ani_arraybuffer>(buffer));
    if (bufferInfo.retCode != ANI_OK) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: env->ArrayBuffer_GetInfo() failed");
        return nullptr;
    }

    std::vector<int32_t> vec;
    for (const char *propName : {"byteLength", "byteOffset"}) {
        ani_int value;
        std::string fieldName = std::string {propName} + "Int";
        ani_status retCode = env->Object_GetFieldByName_Int(array, fieldName.c_str(), &value);
        if (retCode != ANI_OK) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            LOG_ERROR_SDK("TextDecoder:: env->Object_GetFieldByName_Int fieldName: %{public}s", fieldName.c_str());
            return nullptr;
        }
        vec.push_back(value);
    }
    byteLength = vec[0];
    byteOffset = vec[1];
    return reinterpret_cast<char *>(bufferInfo.data);
}

static TextDecoder *Unwrapp(ani_env *env, ani_object object)
{
    ani_long textDecoder;
    if (ANI_OK != env->Object_GetFieldByName_Long(object, "nativeDecoder_", &textDecoder)) {
        return nullptr;
    }
    return reinterpret_cast<TextDecoder *>(textDecoder);
}

static ani_string Decode(ani_env *env, ani_object object, ani_object typedArray, ani_boolean stream)
{
    int32_t byteLength;
    int32_t byteOffset;
    const char *data = GetUint8ArrayInfo(env, typedArray, byteLength, byteOffset);
    bool iflag = (stream == ANI_TRUE);
    auto textDecoder = Unwrapp(env, object);
    if (textDecoder != nullptr) {
        // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
        return textDecoder->DecodeToString(env, data, byteOffset, byteLength, iflag);
    }
    return nullptr;
}

static void BindNativeDecoder(ani_env *env, ani_object object, ani_string aniEncoding, ani_int flags)
{
    std::string stringEncoding = ark::ets::stdlib::ConvertFromAniString(env, aniEncoding);
    auto nativeTextDecoder = new TextDecoder(stringEncoding, flags);
    env->Object_SetFieldByName_Long(object, "nativeDecoder_", reinterpret_cast<ani_long>(nativeTextDecoder));
}

static ani_status BindTextDecoder(ani_env *env)
{
    const char *className = "Lapi/util/TextDecoder;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"bindNativeDecoder", "Lstd/core/String;I:V", reinterpret_cast<void *>(BindNativeDecoder)},
        ani_native_function {"decode", "Lescompat/Uint8Array;Z:Lstd/core/String;", reinterpret_cast<void *>(Decode)},
    };

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Cannot bind native methods to className : %{public}s", className);
        return ANI_ERROR;
    }
    return ANI_OK;
}

[[maybe_unused]] static ani_status BindTextEncoder(ani_env *env)
{
    ani_class cls;
    const char *className = "Lapi/util/TextEncoder;";
    if (ANI_OK != env->FindClass(className, &cls)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextEncoder:: Not found %{public}s", className);
        return ANI_ERROR;
    }
    std::array barMethods = {
        ani_native_function {
            "doEncodeInto",
            "Lstd/core/String;Lstd/core/String;:Lescompat/Uint8Array;",
            reinterpret_cast<void *>(ark::ets::sdk::util::DoEncodeInto),
        },
        ani_native_function {
            "doEncodeInfoUint8Array",
            "Lstd/core/String;Lstd/core/String;Lescompat/Uint8Array;:Lapi/util/EncodeIntoUint8ArrayInfo;",
            reinterpret_cast<void *>(ark::ets::sdk::util::DoEncodeIntoUint8Array),
        },
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, barMethods.data(), barMethods.size())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextEncoder:: Cannot bind native methods to %{public}s", className);
        return ANI_ERROR;
    }
    SetOhosIcuDirectory();
    return ANI_OK;
}

static ani_status BindUtilHelper(ani_env *env)
{
    const char *className = "Lapi/util/UtilHelper;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        auto msg = std::string("Cannot find \"") + className + std::string("\" class!");
        ark::ets::stdlib::ThrowNewError(env, "Lstd/core/RuntimeException;", msg.data(), "Lstd/core/String;:V");
        return ANI_ERROR;
    }

    const auto methods = std::array {
        ani_native_function {"generateRandomUUID", "Z:Lstd/core/String;",
                             reinterpret_cast<void *>(ark::ets::sdk::util::ETSApiUtilHelperGenerateRandomUUID)}};

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        std::cerr << "Cannot bind native methods to '" << className << "'" << std::endl;
        return ANI_ERROR;
    };
    return ANI_OK;
}

extern "C" ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }
    // NOLINTNEXTLINE(hicpp-signed-bitwise,-warnings-as-errors)
    auto status = static_cast<ani_status>(BindUtilHelper(env) | BindTextDecoder(env) | BindTextEncoder(env));
    if (status != ANI_OK) {
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
