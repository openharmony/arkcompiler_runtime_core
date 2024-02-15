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

#ifndef PANDA_RUNTIME_INCLUDE_TOOLING_VREG_VALUE_H
#define PANDA_RUNTIME_INCLUDE_TOOLING_VREG_VALUE_H

#include <cstdint>
#include "libpandabase/macros.h"

namespace ark::tooling {
class VRegValue {
public:
    constexpr explicit VRegValue(int64_t value = 0) : value_(value) {}

    constexpr int64_t GetValue() const
    {
        return value_;
    }

    constexpr void SetValue(int64_t value)
    {
        value_ = value;
    }

    ~VRegValue() = default;

    DEFAULT_COPY_SEMANTIC(VRegValue);
    DEFAULT_MOVE_SEMANTIC(VRegValue);

private:
    int64_t value_;
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_INCLUDE_TOOLING_VREG_VALUE_H
