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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPE_COMPTIME_TRAITS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPE_COMPTIME_TRAITS_H_

#include "ets_type.h"
#include "ets_primitives.h"

namespace panda::ets {

namespace detail {
template <EtsType TYPE>
struct EtsTypeEnumToCppTypeT;

template <>
struct EtsTypeEnumToCppTypeT<EtsType::BOOLEAN> {
    using Type = EtsBoolean;
};

template <>
struct EtsTypeEnumToCppTypeT<EtsType::BYTE> {
    using Type = EtsByte;
};

template <>
struct EtsTypeEnumToCppTypeT<EtsType::CHAR> {
    using Type = EtsChar;
};

template <>
struct EtsTypeEnumToCppTypeT<EtsType::SHORT> {
    using Type = EtsShort;
};

template <>
struct EtsTypeEnumToCppTypeT<EtsType::INT> {
    using Type = EtsInt;
};

template <>
struct EtsTypeEnumToCppTypeT<EtsType::LONG> {
    using Type = EtsLong;
};

template <>
struct EtsTypeEnumToCppTypeT<EtsType::FLOAT> {
    using Type = EtsFloat;
};

template <>
struct EtsTypeEnumToCppTypeT<EtsType::DOUBLE> {
    using Type = EtsDouble;
};

template <>
struct EtsTypeEnumToCppTypeT<EtsType::OBJECT> {
    using Type = EtsObject *;
};
}  // namespace detail

template <EtsType TYPE>
using EtsTypeEnumToCppType = typename detail::EtsTypeEnumToCppTypeT<TYPE>::Type;

/* make switch on EtsType compile-time
 * to func is passed argument, such that
 */
template <typename F>
auto EtsPrimitiveTypeEnumToComptimeConstant(EtsType type, F &&func)
{
    switch (type) {
        case EtsType::BOOLEAN: {
            return func(std::integral_constant<EtsType, EtsType::BOOLEAN> {});
        }
        case EtsType::BYTE: {
            return func(std::integral_constant<EtsType, EtsType::BYTE> {});
        }
        case EtsType::CHAR: {
            return func(std::integral_constant<EtsType, EtsType::CHAR> {});
        }
        case EtsType::SHORT: {
            return func(std::integral_constant<EtsType, EtsType::SHORT> {});
        }
        case EtsType::INT: {
            return func(std::integral_constant<EtsType, EtsType::INT> {});
        }
        case EtsType::LONG: {
            return func(std::integral_constant<EtsType, EtsType::LONG> {});
        }
        case EtsType::FLOAT: {
            return func(std::integral_constant<EtsType, EtsType::FLOAT> {});
        }
        case EtsType::DOUBLE: {
            return func(std::integral_constant<EtsType, EtsType::DOUBLE> {});
        }
        default:
            UNREACHABLE();
    }
}

}  // namespace panda::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPE_COMPTIME_TRAITS_H_
