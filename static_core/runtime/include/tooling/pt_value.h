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
#ifndef PANDA_TOOLING_PT_VALUE_H
#define PANDA_TOOLING_PT_VALUE_H

#include <cstdint>
#include "libpandabase/macros.h"

namespace ark::tooling {
// Deprecated API
class PtValueMeta {
public:
    explicit PtValueMeta(uint64_t data = 0) : data_(data) {}

    uint64_t GetData() const
    {
        return data_;
    }

    void SetData(uint64_t data)
    {
        data_ = data;
    }

    ~PtValueMeta() = default;

    DEFAULT_COPY_SEMANTIC(PtValueMeta);
    DEFAULT_MOVE_SEMANTIC(PtValueMeta);

private:
    uint64_t data_;  // Language dependent data
};

// Deprecated API
class PtValue {
public:
    explicit PtValue(int64_t value = 0) : value_(value) {}

    PtValue(int64_t value, PtValueMeta /* unused */) : value_(value) {}

    int64_t GetValue() const
    {
        return value_;
    }

    void SetValue(int64_t value)
    {
        value_ = value;
    }

    ~PtValue() = default;

    DEFAULT_COPY_SEMANTIC(PtValue);
    DEFAULT_MOVE_SEMANTIC(PtValue);

private:
    int64_t value_;
};
}  // namespace ark::tooling

#endif  // PANDA_TOOLING_PT_VALUE_H
