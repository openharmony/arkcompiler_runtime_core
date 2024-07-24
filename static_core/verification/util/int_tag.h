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

#ifndef PANDA_VERIFIER_UTIL_TAG_FOR_INT__
#define PANDA_VERIFIER_UTIL_TAG_FOR_INT__

#include <cstddef>

#include "utils/bit_utils.h"

#include "macros.h"

namespace ark::verifier {

template <typename Int, const Int MIN_INT, const Int MAX_INT>
class TagForInt {
public:
    static constexpr size_t SIZE = MAX_INT - MIN_INT + 1;
    static constexpr size_t BITS = sizeof(size_t) * 8ULL - ark::Clz(SIZE);

    using Type = Int;

    static size_t GetIndexFor(Int i)
    {
        ASSERT(MIN_INT <= i);
        ASSERT(i <= MAX_INT);
        return i - MIN_INT;
    }

    static Int GetValueFor(size_t tag)
    {
        ASSERT(tag < SIZE);
        return static_cast<Int>(tag) + MIN_INT;
    }
};

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_UTIL_TAG_FOR_INT__