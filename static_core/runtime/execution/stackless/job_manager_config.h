/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_EXECUTION_STACKLESS_JOB_MANAGER_CONFIG_H
#define PANDA_RUNTIME_EXECUTION_STACKLESS_JOB_MANAGER_CONFIG_H

#include <cstdint>

namespace ark {

struct JobManagerConfig {
    static constexpr uint32_t WORKERS_COUNT_AUTO = 0;

    /// Number of job workers threads for the N:M mode
    const uint32_t workersCount_ = WORKERS_COUNT_AUTO;
    /// Limit on the number of exclusive job worker threads
    const uint32_t exclusiveWorkersLimit_ = 0;
    /// Number of exclusive workers created for runtime needs
    const uint32_t preallocatedExclusiveWorkersCount_ = 0;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_STACKLESS_JOB_MANAGER_CONFIG_H