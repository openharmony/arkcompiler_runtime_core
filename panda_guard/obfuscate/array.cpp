/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "array.h"

#include "configs/guard_context.h"
#include "program.h"

void panda::guard::Array::RefreshNeedUpdate()
{
    this->needUpdate_ = GuardContext::GetInstance()->GetGuardOptions()->IsFileNameObfEnabled();
}

void panda::guard::Array::Update()
{
    if (!node_.has_value()) {
        return;
    }
    const auto &node = node_.value();
    if (node->name_ == node->obfName_) {
        return;
    }

    std::string updatedLiteralArrayIdx = this->literalArrayIdx_;
    updatedLiteralArrayIdx.replace(updatedLiteralArrayIdx.find(node->name_), node->name_.size(), node->obfName_);

    UpdateLiteralArrayTableIdx(this->literalArrayIdx_, updatedLiteralArrayIdx);
    this->literalArrayIdx_ = updatedLiteralArrayIdx;

    for (auto &info : this->defineInsList_) {
        info.ins_->GetId(INDEX_0) = updatedLiteralArrayIdx;
    }
}
