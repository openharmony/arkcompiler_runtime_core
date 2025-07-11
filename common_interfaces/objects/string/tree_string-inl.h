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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_INL_H
#define COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_INL_H

#include "objects/string/base_string.h"
#include "objects/string/tree_string.h"

namespace common {
template <typename Allocator, typename WriteBarrier,
          objects_traits::enable_if_is_allocate<Allocator, BaseObject *>,
          objects_traits::enable_if_is_write_barrier<WriteBarrier>>
TreeString *TreeString::Create(Allocator &&allocator, WriteBarrier &&writeBarrier, ReadOnlyHandle<BaseString> left,
                               ReadOnlyHandle<BaseString> right, uint32_t length, bool compressed)
{
    auto string = TreeString::Cast(
        std::invoke(std::forward<Allocator>(allocator), TreeString::SIZE, ObjectType::TREE_STRING));
    string->InitLengthAndFlags(length, compressed);
    string->SetRawHashcode(0);
    string->SetLeftSubString(std::forward<WriteBarrier>(writeBarrier), left.GetBaseObject());
    string->SetRightSubString(std::forward<WriteBarrier>(writeBarrier), right.GetBaseObject());
    return string;
}


template <typename ReadBarrier>
bool TreeString::IsFlat(ReadBarrier &&readBarrier) const
{
    auto strRight = BaseString::Cast(GetRightSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
    return strRight->GetLength() == 0;
}

template <bool verify, typename ReadBarrier>
uint16_t TreeString::Get(ReadBarrier &&readBarrier, int32_t index) const
{
    int32_t length = static_cast<int32_t>(GetLength());
    if constexpr (verify) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }

    if (IsFlat(std::forward<ReadBarrier>(readBarrier))) {
        BaseString *left = BaseString::Cast(GetLeftSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
        return left->At<verify>(std::forward<ReadBarrier>(readBarrier), index);
    }
    const BaseString *string = this;
    while (true) {
        if (string->IsTreeString()) {
            BaseString *left = BaseString::Cast(
                TreeString::ConstCast(string)->GetLeftSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
            if (static_cast<int32_t>(left->GetLength()) > index) {
                string = left;
            } else {
                index -= static_cast<int32_t>(left->GetLength());
                string = BaseString::Cast(
                    TreeString::ConstCast(string)->GetRightSubString<BaseObject *>(
                        std::forward<ReadBarrier>(readBarrier)));
            }
        } else {
            return string->At<verify>(std::forward<ReadBarrier>(readBarrier), index);
        }
    }
    UNREACHABLE_CC();
}
}
#endif //COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_INL_H