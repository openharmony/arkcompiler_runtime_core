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

#include <cstdint>
#include <cstdlib>
#include <array>
#include <sstream>
#include <securec.h>
#include <sys/types.h>
#include <random>

#include "stdlib/native/core/stdlib_ani_helpers.h"
#include "tools/format_logger.h"
#include "Util.h"

namespace ark::ets::sdk::util {

constexpr int UUID_LEN = 37;
constexpr int UUID_STR_LEN_NO_DASH = UUID_LEN - 4;
constexpr int UUID_BINARY_LEN = 16;
constexpr int HEX_BYTE_STR_LENGTH = 3;
constexpr int UUID_VERSION_BYTE_INDEX = 6;
constexpr int UUID_RESERVED_BYTE_INDEX = 8;
constexpr uint32_t NULL_FOUR_HIGH_BITS_IN_16 = 0x0FFF;
constexpr uint32_t RFC4122_UUID_VERSION_MARKER = 0x4000;
constexpr uint32_t NULL_TWO_HIGH_BITS_IN_16 = 0x3FFF;
constexpr uint32_t RFC4122_UUID_RESERVED_BITS = 0x8000;
constexpr uint8_t UUID_LOW_FOUR_BITS_MASK = 0x0F;
constexpr uint8_t UUID_VERSION4_MARK = 0x40;
constexpr uint8_t UUID_LOW_SIX_BITS_MASK = 0x3F;
constexpr uint8_t UUID_RESERVED_MARK = 0x80;

template <typename S>
S GenRandUint()
{
    static auto device = std::random_device();
    static auto randomGenerator = std::mt19937(device());
    static auto range = std::uniform_int_distribution<S>();

    return range(randomGenerator);
}

std::string GenUuid4(ani_env *env)
{
    std::array<char, UUID_LEN> uuidStr = {0};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    int n = snprintf_s(
        uuidStr.begin(), UUID_LEN, UUID_LEN - 1, "%08x-%04x-%04x-%04x-%012x", GenRandUint<uint32_t>(),
        GenRandUint<uint16_t>(), (GenRandUint<uint16_t>() & NULL_FOUR_HIGH_BITS_IN_16) | RFC4122_UUID_VERSION_MARKER,
        (GenRandUint<uint16_t>() & NULL_TWO_HIGH_BITS_IN_16) | RFC4122_UUID_RESERVED_BITS, GenRandUint<uint64_t>());
    if ((n < 0) || (n > static_cast<int>(UUID_LEN))) {
        stdlib::ThrowNewError(env, "Lstd/core/RuntimeException;", "GenerateRandomUUID failed", "Lstd/core/String;:V");
        return std::string();
    }
    std::stringstream res;
    res << uuidStr.data();

    return res.str();
}

std::array<uint8_t, UUID_BINARY_LEN> GenUuid4Binary(ani_env *env)
{
    std::array<char, UUID_STR_LEN_NO_DASH> uuidStr = {0};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    int charsWritten = snprintf_s(
        uuidStr.data(), uuidStr.size(), UUID_STR_LEN_NO_DASH - 1, "%08x%04x%04x%04x%012x", GenRandUint<uint32_t>(),
        GenRandUint<uint16_t>(), (GenRandUint<uint16_t>() & NULL_FOUR_HIGH_BITS_IN_16) | RFC4122_UUID_VERSION_MARKER,
        (GenRandUint<uint16_t>() & NULL_TWO_HIGH_BITS_IN_16) | RFC4122_UUID_RESERVED_BITS, GenRandUint<uint64_t>());
    if ((charsWritten < 0) || (charsWritten > UUID_STR_LEN_NO_DASH)) {
        stdlib::ThrowNewError(env, "Lstd/core/RuntimeException;", "generateRandomBinaryUUID failed",
                              "Lescompat/Uint8Array;:V");
        return {};
    }

    std::array<uint8_t, UUID_BINARY_LEN> uuidBin = {0};
    for (size_t i = 0; i < UUID_BINARY_LEN; ++i) {
        std::array<char, HEX_BYTE_STR_LENGTH> byteStr = {uuidStr[i * 2], uuidStr[i * 2 + 1], 0};
        uuidBin[i] = static_cast<uint8_t>(strtol(byteStr.data(), nullptr, UUID_BINARY_LEN));
    }
    // Set the four most significant bits of the 7th byte to 0100'b, so the UUID version is 4.
    uuidBin[UUID_VERSION_BYTE_INDEX] = static_cast<uint8_t>(
        (static_cast<unsigned int>(uuidBin[UUID_VERSION_BYTE_INDEX]) & UUID_LOW_FOUR_BITS_MASK) | UUID_VERSION4_MARK);

    // Set the two most significant bits of the 9th byte to 10'b, so the UUID variant is RFC4122.
    uuidBin[UUID_RESERVED_BYTE_INDEX] = static_cast<uint8_t>(
        (static_cast<unsigned int>(uuidBin[UUID_RESERVED_BYTE_INDEX]) & UUID_LOW_SIX_BITS_MASK) | UUID_RESERVED_MARK);
    return uuidBin;
}

template <class... Args>
ani_object NewUint8Array(ani_env *env, const char *signature, Args... args)
{
    ani_class arrayClass;
    if (env->FindClass("Lescompat/Uint8Array;", &arrayClass) != ANI_OK) {
        return nullptr;
    }
    ani_method arrayCtor;
    if (env->Class_FindMethod(arrayClass, "<ctor>", signature, &arrayCtor) != ANI_OK) {
        return nullptr;
    }
    ani_object res;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (env->Object_New(arrayClass, arrayCtor, &res, args...) != ANI_OK) {
        return nullptr;
    }
    return res;
}

extern "C" {
ANI_EXPORT ani_string ETSApiUtilHelperGenerateRandomUUID(ani_env *env, [[maybe_unused]] ani_class klass,
                                                         ani_boolean entropyCache)
{
    static std::string lastGeneratedUUID;
    if (entropyCache != ANI_TRUE || lastGeneratedUUID.empty()) {
        lastGeneratedUUID = GenUuid4(env);
    }
    return stdlib::CreateUtf8String(env, lastGeneratedUUID.data(), lastGeneratedUUID.size());
}

ANI_EXPORT ani_object ETSApiUtilHelperGenerateRandomBinaryUUID(ani_env *env, [[maybe_unused]] ani_class klass,
                                                               ani_boolean entropyCache)
{
    static std::array<uint8_t, UUID_BINARY_LEN> lastGeneratedUUID;
    static bool hasUUID = false;
    if (entropyCache != ANI_TRUE || !hasUUID) {
        lastGeneratedUUID = GenUuid4Binary(env);
        hasUUID = true;
    }

    ani_object arr = NewUint8Array(env, "I:V", static_cast<ani_int>(UUID_BINARY_LEN));
    if (arr == nullptr) {
        return nullptr;
    }

    ani_ref buffer;
    if (env->Object_GetFieldByName_Ref(arr, "buffer", &buffer) != ANI_OK) {
        return nullptr;
    }

    void *bufData = nullptr;
    size_t bufLength = 0;
    if (env->ArrayBuffer_GetInfo(static_cast<ani_arraybuffer>(buffer), &bufData, &bufLength) != ANI_OK) {
        return nullptr;
    }

    if (EOK != memcpy_s(bufData, bufLength, lastGeneratedUUID.data(), lastGeneratedUUID.size())) {
        return nullptr;
    }

    return arr;
}
}  // extern "C"

ani_string CreateUtf8String(ani_env *env, const char *data, ani_size size)
{
    ani_string result;
    ANI_FATAL_IF_ERROR(env->String_NewUTF8(data, size, &result));
    return result;
}

ani_ref GetAniNull(ani_env *env)
{
    ani_ref nullObj {};
    ANI_FATAL_IF_ERROR(env->GetNull(&nullObj));
    return nullObj;
}

bool IsNullishValue(ani_env *env, ani_ref ref)
{
    auto isNullish = static_cast<ani_boolean>(false);
    ANI_FATAL_IF_ERROR(env->Reference_IsNullishValue(ref, &isNullish));
    return static_cast<bool>(isNullish);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FUN_UNBOX_METHOD(aniType, typeName, signature)                                                         \
    aniType UnboxTo##typeName(ani_env *env, ani_object value)                                                  \
    {                                                                                                          \
        aniType result {};                                                                                     \
        ANI_FATAL_IF_ERROR(env->Object_CallMethodByName_##typeName(value, "unboxed", ":" signature, &result)); \
        /* CC-OFFNXT(G.PRE.05) function defination, no effects */                                              \
        return result;                                                                                         \
    }
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
BOX_VALUE_LIST(FUN_UNBOX_METHOD);
#undef FUN_UNBOX_METHOD

}  // namespace ark::ets::sdk::util
