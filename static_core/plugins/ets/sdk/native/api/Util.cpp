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

#include "tools/format_logger.h"
#include "Util.h"
#include "stdlib/native/core/stdlib_ani_helpers.h"

namespace ark::ets::sdk::util {

constexpr int UUID_LEN = 37;
constexpr uint32_t NULL_FOUR_HIGH_BITS_IN_16 = 0x0FFF;
constexpr uint32_t RFC4122_UUID_VERSION_MARKER = 0x4000;
constexpr uint32_t NULL_TWO_HIGH_BITS_IN_16 = 0x3FFF;
constexpr uint32_t RFC4122_UUID_RESERVED_BITS = 0x8000;

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
}  // extern "C"

}  // namespace ark::ets::sdk::util
