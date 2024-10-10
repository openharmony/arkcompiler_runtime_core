/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include "SamplerAsmTest.h"

#ifdef __cplusplus
extern "C" {
#endif

// This function have 8 arguments in order to check the case
// when some arguments are put not into registers but on stack.
extern "C" int SumEightElements(int, int, int, int, int, int, int, int);

// NOLINTBEGIN(readability-magic-numbers, readability-named-parameter, cppcoreguidelines-pro-type-vararg,
// hicpp-signed-bitwice)

ETS_EXPORT ets_int ETS_CALL ETS_ETSGLOBAL_NativeSumEightElements([[maybe_unused]] EtsEnv *, [[maybe_unused]] ets_class)
{
    constexpr int MULT = 8;
    constexpr int OFF = 28;
    constexpr int ITERS = 10000000;
    for (size_t j = 0; j < 200U; j++) {
        for (int i = 0; i < ITERS; ++i) {
            int val = SumEightElements(i, i + 1U, i + 2U, i + 3U, i + 4U, i + 5U, i + 6U, i + 7U);
            if (val != MULT * i + OFF) {
                std::cerr << val << std::endl;
                return 1;
            }
        }
    }

    return 0;
}

// NOLINTEND(readability-magic-numbers, readability-named-parameter, cppcoreguidelines-pro-type-vararg,
// hicpp-signed-bitwice)

#ifdef __cplusplus
}
#endif
