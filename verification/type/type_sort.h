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

#ifndef _PANDA_TYPE_SORT_HPP
#define _PANDA_TYPE_SORT_HPP

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

namespace panda::verifier {
using SortIdx = size_t;

class SortNames {
public:
    SortNames(const PandaString &bot, const PandaString &top)
    {
        operator[](bot);
        operator[](top);
    }
    DEFAULT_COPY_SEMANTIC(SortNames);
    DEFAULT_MOVE_SEMANTIC(SortNames);

    ~SortNames() = default;

    const PandaString &operator[](SortIdx sort) const
    {
        return SortToName_[sort];
    }

    SortIdx operator[](const PandaString &name)
    {
        auto s = NameToSort_.find(name);
        if (s != NameToSort_.end()) {
            return s->second;
        }
        SortIdx sort = SortToName_.size();
        SortToName_.push_back(name);
        NameToSort_[name] = sort;
        return sort;
    }

private:
    PandaUnorderedMap<PandaString, SortIdx> NameToSort_;
    PandaVector<PandaString> SortToName_;
};
}  // namespace panda::verifier

#endif  // !_PANDA_TYPE_SORT_HPP
