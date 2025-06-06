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

#include "common_interfaces/objects/string/base_string_declare.h"
#include "common_interfaces/objects/string/tree_string.h"

namespace common {
template <typename ReadBarrier>
bool TreeString::IsFlat(ReadBarrier &&readBarrier) const
{
    auto strSecond = BaseString::Cast(GetSecond<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
    return strSecond->GetLength() == 0;
}

template <bool verify, typename ReadBarrier>
uint16_t TreeString::Get(ReadBarrier &&readBarrier, int32_t index) const
{
    int32_t length = static_cast<int32_t>(GetLength());
    if (verify) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }

    if (IsFlat(std::forward<ReadBarrier>(readBarrier))) {
        BaseString *first = BaseString::Cast(GetFirst<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
        return first->At<verify>(std::forward<ReadBarrier>(readBarrier), index);
    }
    BaseString *string = const_cast<TreeString *>(this);
    while (true) {
        if (string->IsTreeString()) {
            BaseString *first = BaseString::Cast(
                TreeString::Cast(string)->GetFirst<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
            if (static_cast<int32_t>(first->GetLength()) > index) {
                string = first;
            } else {
                index -= static_cast<int32_t>(first->GetLength());
                string = BaseString::Cast(
                    TreeString::Cast(string)->GetSecond<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
            }
        } else {
            return string->At<verify>(std::forward<ReadBarrier>(readBarrier), index);
        }
    }
    UNREACHABLE();
}
}
#endif //COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_INL_H