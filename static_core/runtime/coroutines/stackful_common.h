/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_COROUTINES_STACKFUL_COMMON_H
#define PANDA_RUNTIME_COROUTINES_STACKFUL_COMMON_H

#include "libpandabase/globals.h"

namespace ark::stackful_coroutines {

// every bit means that the corresponding worker is allowed
using AffinityMask = size_t;
// all ones
inline constexpr AffinityMask AFFINITY_MASK_FULL = std::numeric_limits<AffinityMask>::max();
// all zeros
inline constexpr AffinityMask AFFINITY_MASK_NONE = 0U;

// maximum workers count should conform to number of bits in the affinity mask
inline constexpr size_t MAX_WORKERS_COUNT = sizeof(AffinityMask) * BITS_PER_BYTE;
// maximum worker id is bound by the number of bits in the affinity mask
inline constexpr size_t MAX_WORKER_ID = MAX_WORKERS_COUNT - 1U;
// ID's type must be capable of holding values from [0 to MAX_WORKER_ID]
using WorkerId = size_t;
// must be something obviously erroneous (> MAX_WORKER_ID)
inline constexpr WorkerId INVALID_WORKER_ID = std::numeric_limits<WorkerId>::max();
// must be something easy to assign and comprehend
inline constexpr WorkerId MAIN_WORKER_ID = 0U;
// taskpool eaworker limit
inline constexpr size_t TASKPOOL_EAWORKER_LIMIT = 2;
// taskpool eaworker mode
inline static constexpr const char *TASKPOOL_EAWORKER_MODE = "eaworker";
}  // namespace ark::stackful_coroutines

#endif /* PANDA_RUNTIME_COROUTINES_STACKFUL_COMMON_H */
