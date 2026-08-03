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

#ifndef ANI_GTEST_FIXEDARRAY_OPS_H
#define ANI_GTEST_FIXEDARRAY_OPS_H

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class AniGTestFixedArrayOps : public AniTest {
public:
    static constexpr ani_size LENGTH_3 = 3;
    static constexpr ani_size LENGTH_5 = 5;
    static constexpr ani_size LENGTH_6 = 6;
    static constexpr ani_size LENGTH_10 = 10;
    static constexpr ani_int LOOP_COUNT = 3;
};
}  // namespace ark::ets::ani::testing

#endif  // ANI_GTEST_FIXEDARRAY_OPS_H
