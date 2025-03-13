/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "evaluation/conditions_storage.h"

namespace ark::tooling::inspector {
void ConditionsStorage::AddCondition(BreakpointId id, const std::string &condition)
{
    [[maybe_unused]] auto res = conditions_.emplace(id, std::make_pair(condition, nullptr));
    ASSERT(res.second);
}

void ConditionsStorage::RemoveCondition(BreakpointId id)
{
    [[maybe_unused]] auto res = conditions_.erase(id);
    ASSERT(res == 1);
}

bool ConditionsStorage::HasCondition(BreakpointId id) const
{
    return conditions_.find(id) != conditions_.end();
}

void ConditionsStorage::Clear()
{
    conditions_.clear();
}

Expected<bool, Error> ConditionsStorage::EvaluateCondition(BreakpointId id)
{
    auto iter = conditions_.find(id);
    ASSERT(iter != conditions_.end());
    // Condition is evaluated in the context of top frame
    auto res = evaluationEngine_.EvaluateExpression(0, iter->second.first, &iter->second.second);
    if (!res) {
        return Unexpected(res.Error());
    }
    if (res.Value().second != nullptr) {
        LOG(WARNING, DEBUGGER) << "Breakpoint #" << id << " condition evaluated with an exception";
        return false;
    }
    return res.Value().first.GetAsU1();
}
}  // namespace ark::tooling::inspector
