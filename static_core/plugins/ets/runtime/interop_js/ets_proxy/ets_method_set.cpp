/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_set.h"

#include <algorithm>
#include "plugins/ets/runtime/types/ets_method.h"

#include <functional>

namespace ark::ets::interop::js::ets_proxy {

EtsMethodSet EtsMethodSet::Create(EtsMethod *singleMethod)
{
    ASSERT(nullptr != singleMethod);
    return EtsMethodSet(singleMethod, singleMethod->GetClass());
}

EtsMethodSet EtsMethodSet::Create(EtsMethod *singleMethod, const std::string &jsName)
{
    ASSERT(singleMethod != nullptr);
    return EtsMethodSet(singleMethod, singleMethod->GetClass(), jsName);
}

const char *EtsMethodSet::GetName() const
{
    return jsPublicName_.c_str();
}

void EtsMethodSet::MergeWith(const EtsMethodSet &other)
{
    if (other.entries_.size() > entries_.size()) {
        entries_.resize(other.entries_.size());
    }
    for (uint32_t i = 0; i < other.entries_.size(); ++i) {
        for (auto *newMethod : other.entries_[i]) {
            if (newMethod != nullptr) {
                entries_[i].push_back(newMethod);
            }
        }
    }
}

void EtsMethodSet::SortMethods()
{
    for (auto &bucket : entries_) {
        std::sort(bucket.begin(), bucket.end(), [](EtsMethod *a, EtsMethod *b) {
            if (a == nullptr) {
                return false;
            }
            if (b == nullptr) {
                return true;
            }
            int32_t lineA = a->GetLineNumFromBytecodeOffset(0);
            int32_t lineB = b->GetLineNumFromBytecodeOffset(0);
            if (lineA != lineB) {
                return lineA < lineB;
            }
            return a->GetMethodId() < b->GetMethodId();
        });
    }
}

}  // namespace ark::ets::interop::js::ets_proxy
