/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ETS_REGEXP_REGEXP_EXECUTOR_H
#define ETS_REGEXP_REGEXP_EXECUTOR_H

#include "runtime/regexp/ecmascript/regexp_executor.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "utils/logger.h"

namespace ark::ets {

class RegExpExecutor : public ark::RegExpExecutor {
public:
    RegExpMatchResult<PandaString> GetResult(bool isSuccess, bool hasIndices) const;

private:
    std::pair<uint32_t, uint32_t> GetIndices(CaptureState *captureState) const;
    void HandleCaptures(PandaVector<std::pair<bool, PandaString>> &captures,
                        PandaVector<std::pair<uint32_t, uint32_t>> &indices, CaptureState *captureState) const;
};
}  // namespace ark::ets

#endif  // ets_REGEXP_REGEXP_EXECUTOR_H
