/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

namespace panda::verifier {

template <typename Int, const Int Min, const Int Max>
class TagForInt {
public:
    static constexpr size_t Size = Max - Min + 1;
    static constexpr size_t Bits = sizeof(size_t) * 8ULL - panda::Clz(Size);

    using type = Int;

    static size_t GetIndexFor(Int i)
    {
        ASSERT(Min <= i && i <= Max);
        return i - Min;
    }

    static Int GetValueFor(size_t tag)
    {
        ASSERT(tag < Size);
        return static_cast<Int>(tag) + Min;
    }
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_UTIL_TAG_FOR_INT__