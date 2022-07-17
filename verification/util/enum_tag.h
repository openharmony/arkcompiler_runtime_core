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

#ifndef PANDA_VERIFIER_UTIL_TAG_FOR_ENUM__
#define PANDA_VERIFIER_UTIL_TAG_FOR_ENUM__

#include <cstddef>

#include "utils/bit_utils.h"

#include "macros.h"

namespace panda::verifier {

template <size_t I_num, typename Enum, Enum... Items>
class TagForEnumNumerated {
protected:
    static constexpr size_t Size = 0ULL;

    static size_t GetIndexFor([[maybe_unused]] Enum)
    {
        UNREACHABLE();
    }

    static Enum GetValueFor([[maybe_unused]] size_t)
    {
        UNREACHABLE();
    }
};

// TagForEnumNumerated needs because recursive numeration with Size - 1 gives wrong ordering
template <size_t I_num, typename Enum, Enum I, Enum... Items>
class TagForEnumNumerated<I_num, Enum, I, Items...> : public TagForEnumNumerated<I_num + 1ULL, Enum, Items...> {
    using Base = TagForEnumNumerated<I_num + 1ULL, Enum, Items...>;

protected:
    static constexpr size_t Size = Base::Size + 1ULL;

    static size_t GetIndexFor(Enum e)
    {
        if (e == I) {
            return I_num;
        }
        return Base::GetIndexFor(e);
    }

    static Enum GetValueFor(size_t tag)
    {
        if (tag == I_num) {
            return I;
        }
        return Base::GetValueFor(tag);
    }
};

template <typename Enum, Enum... Items>
class TagForEnum : public TagForEnumNumerated<0ULL, Enum, Items...> {
    using Base = TagForEnumNumerated<0ULL, Enum, Items...>;

public:
    using type = Enum;

    static constexpr size_t Size = Base::Size;
    static constexpr size_t Bits = sizeof(size_t) * 8ULL - panda::Clz(Size);

    static size_t GetIndexFor(Enum e)
    {
        return Base::GetIndexFor(e);
    }

    static Enum GetValueFor(size_t tag)
    {
        return Base::GetValueFor(tag);
    }
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_UTIL_TAG_FOR_ENUM__