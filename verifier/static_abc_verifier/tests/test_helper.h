/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef STATIC_ABC_VERIFIER_TEST_HELPER_H
#define STATIC_ABC_VERIFIER_TEST_HELPER_H

#include "abc_file.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ark::static_verifier::test {

inline constexpr uint32_t ADLER32_MODULUS = 65521;

inline uint32_t ComputeAdler32(const uint8_t *data, size_t len)
{
    uint32_t sumA = 1;
    uint32_t sumB = 0;
    for (size_t i = 0; i < len; ++i) {
        sumA = (sumA + data[i]) % ADLER32_MODULUS;
        sumB = (sumB + sumA) % ADLER32_MODULUS;
    }
    return (sumB << 16U) | sumA;
}

std::vector<uint8_t> BuildMinimalValidAbc();
std::vector<uint8_t> BuildAbcWithClasses(uint32_t numClasses);

}  // namespace ark::static_verifier::test

#endif  // STATIC_ABC_VERIFIER_TEST_HELPER_H
