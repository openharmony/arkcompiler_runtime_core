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

#ifndef PANDA_RUNTIME_THREAD_STATUS_H_
#define PANDA_RUNTIME_THREAD_STATUS_H_

#include <cstdint>
#include <ostream>

namespace ark {

enum class MutatorStatus : uint16_t {
    CREATED,
    RUNNING,
    IS_BLOCKED,
    IS_WAITING,
    IS_TIMED_WAITING,
    IS_SUSPENDED,
    IS_COMPILER_WAITING,
    IS_WAITING_INFLATION,
    IS_SLEEPING,
    IS_TERMINATED_LOOP,
    NATIVE,
    TERMINATING,
    FINISHED,
};

enum MutatorFlag {
    NO_FLAGS = 0,
    SUSPEND_REQUEST = 1U << 1U,
    RUNTIME_TERMINATION_REQUEST = 1U << 2U,
    GC_ON_SAFEPOINT_REQUEST = 1U << 3U,
    SUSPEND_FOR_FINALIZE = 1U << 4U,
};

std::ostream &operator<<(std::ostream &stream, MutatorStatus status);

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_H_
