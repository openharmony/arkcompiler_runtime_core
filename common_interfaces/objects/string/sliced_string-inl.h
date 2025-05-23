/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_INL_H
#define COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_INL_H

#include "common_interfaces/objects/string/base_string_declare.h"
#include "common_interfaces/objects/string/sliced_string.h"

namespace panda {
inline uint32_t SlicedString::GetStartIndex() const
{
    uint32_t bits = GetStartIndexAndFlags();
    return StartIndexBits::Decode(bits);
}

inline void SlicedString::SetStartIndex(uint32_t startIndex)
{
    ASSERT(startIndex <= SlicedString::MAX_STRING_LENGTH);
    uint32_t bits = GetStartIndexAndFlags();
    uint32_t newVal = StartIndexBits::Update(bits, startIndex);
    SetStartIndexAndFlags(newVal);
}

inline bool SlicedString::GetHasBackingStore() const
{
    uint32_t bits = GetStartIndexAndFlags();
    return HasBackingStoreBit::Decode(bits);
}

inline void SlicedString::SetHasBackingStore(bool hasBackingStore)
{
    uint32_t bits = GetStartIndexAndFlags();
    uint32_t newVal = HasBackingStoreBit::Update(bits, hasBackingStore);
    SetStartIndexAndFlags(newVal);
}

// Minimum length for a sliced string
template <bool verify, typename ReadBarrier>
uint16_t SlicedString::Get(ReadBarrier &&readBarrier, int32_t index) const
{
    int32_t length = static_cast<int32_t>(GetLength());
    if (verify) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }
    BaseString *parent = BaseString::Cast(GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
    ASSERT(parent->IsLineString());
    if (parent->IsUtf8()) {
        Span<const uint8_t> sp(parent->GetDataUtf8() + GetStartIndex(), length);
        return sp[index];
    }
    Span<const uint16_t> sp(parent->GetDataUtf16() + GetStartIndex(), length);
    return sp[index];
}
}
#endif //COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_INL_H