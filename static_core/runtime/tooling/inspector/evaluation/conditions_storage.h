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

#ifndef PANDA_TOOLING_INSPECTOR_EVALUATION_CONDITIONS_STORAGE_H
#define PANDA_TOOLING_INSPECTOR_EVALUATION_CONDITIONS_STORAGE_H

#include "evaluation/evaluation_engine.h"
#include "types/numeric_id.h"

namespace ark::tooling::inspector {
class ConditionsStorage {
public:
    explicit ConditionsStorage(EvaluationEngine &engine) : evaluationEngine_(engine) {}

    NO_COPY_SEMANTIC(ConditionsStorage);
    NO_MOVE_SEMANTIC(ConditionsStorage);

    ~ConditionsStorage() = default;

    void AddCondition(BreakpointId id, const std::string &condition);

    void RemoveCondition(BreakpointId id);

    bool HasCondition(BreakpointId id) const;

    Expected<bool, Error> EvaluateCondition(BreakpointId id);

private:
    std::unordered_map<BreakpointId, std::pair<std::string, Method *>> conditions_;
    EvaluationEngine &evaluationEngine_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_EVALUATION_CONDITIONS_STORAGE_H
