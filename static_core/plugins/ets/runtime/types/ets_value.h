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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_VALUE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_VALUE_H_

#include "runtime/include/runtime.h"
#include "ets_type.h"

namespace ark::ets {

class EtsObject;

template <typename T>
constexpr EtsType GetEtsType()
{
    if constexpr (std::is_same<T, ets_boolean>::value) {  // NOLINT
        return EtsType::BOOLEAN;
    }
    if constexpr (std::is_same<T, ets_byte>::value) {  // NOLINT
        return EtsType::BYTE;
    }
    if constexpr (std::is_same<T, ets_char>::value) {  // NOLINT
        return EtsType::CHAR;
    }
    if constexpr (std::is_same<T, ets_short>::value) {  // NOLINT
        return EtsType::SHORT;
    }
    if constexpr (std::is_same<T, ets_int>::value) {  // NOLINT
        return EtsType::INT;
    }
    if constexpr (std::is_same<T, ets_long>::value) {  // NOLINT
        return EtsType::LONG;
    }
    if constexpr (std::is_same<T, ets_float>::value) {  // NOLINT
        return EtsType::FLOAT;
    }
    if constexpr (std::is_same<T, ets_double>::value) {  // NOLINT
        return EtsType::DOUBLE;
    }
    if constexpr (std::is_same<T, EtsObject>::value) {  // NOLINT
        return EtsType::OBJECT;
    }
    if constexpr (std::is_same<T, void>::value) {  // NOLINT
        return EtsType::VOID;
    }
    return EtsType::UNKNOWN;
}

class EtsValue {
public:
    EtsValue() = default;

    template <typename T>
    explicit EtsValue(T value)
    {
        static_assert(std::is_arithmetic<T>::value || std::is_same<T, ets_object>::value ||
                      std::is_same<T, std::nullptr_t>::value);
        static_assert(sizeof(T) <= sizeof(holder_));

        memcpy_s(&holder_, sizeof(holder_), &value, sizeof(value));
    }

    template <typename T>
    T GetAs()
    {
        static_assert(std::is_arithmetic<T>::value || std::is_same<T, ets_object>::value);
        static_assert(sizeof(T) <= sizeof(holder_));

        T tmp;
        memcpy_s(&tmp, sizeof(T), &holder_, sizeof(T));
        return tmp;
    }

private:
    int64_t holder_ {0};
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_VALUE_H_