/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_method.h"

#include <functional>

namespace ark::ets::interop::js::ets_proxy {

EtsMethodSet EtsMethodSet::Create(EtsMethod *singleMethod)
{
    ASSERT(nullptr != singleMethod);
    EtsClass *const enclosingClass = singleMethod->GetClass();

    return EtsMethodSet(singleMethod, enclosingClass);
}

const char *EtsMethodSet::GetName() const
{
    ASSERT(anyMethod_ != nullptr);
    return anyMethod_->GetName();
}

void EtsMethodSet::MergeWith(const EtsMethodSet &other)
{
    if (other.entries_.size() > entries_.size()) {
        entries_.resize(other.entries_.size());
    }
    for (uint32_t i = 0; i < other.entries_.size(); ++i) {
        EtsMethod *newMethod = other.entries_[i];
        if (nullptr == newMethod) {
            continue;
        }
        if (nullptr == entries_[i]) {
            entries_[i] = other.entries_[i];
        } else {
            isValid_ = false;  // error: duplicate number of parameters
        }
    }
}

}  // namespace ark::ets::interop::js::ets_proxy
