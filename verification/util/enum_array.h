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

#ifndef PANDA_VERIFIER_UTIL_ENUM_ARRAY_H__
#define PANDA_VERIFIER_UTIL_ENUM_ARRAY_H__

#include "macros.h"

#include "enum_tag.h"

#include <type_traits>
#include <array>

namespace panda::verifier {

template <typename T, typename Enum, Enum... Rest>
class EnumArray {
public:
    template <typename = std::enable_if_t<std::is_default_constructible_v<T>>>
    EnumArray() : arr {}
    {
    }

    template <typename... Args>
    EnumArray(Args... args) : arr {}
    {
        arr.fill(T {args...});
    }

    ~EnumArray() = default;

    T &operator[](Enum e)
    {
        return arr[EnumTag::GetIndexFor(e)];
    }

    const T &operator[](Enum e) const
    {
        return arr[EnumTag::GetIndexFor(e)];
    }

private:
    using EnumTag = TagForEnum<Enum, Rest...>;
    std::array<T, EnumTag::Size> arr;
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_UTIL_ENUM_ARRAY_H__
