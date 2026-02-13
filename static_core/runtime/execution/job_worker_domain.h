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

#ifndef PANDA_RUNTIME_EXECUTION_JOB_WORKER_DOMAIN_H
#define PANDA_RUNTIME_EXECUTION_JOB_WORKER_DOMAIN_H

#include <ostream>
#include <sstream>

namespace ark {

/// @brief Job domain consists of the worker threads that have something in common
enum class JobWorkerThreadDomain {
    /// regular workers that do not support interop, are not "exclusive" and support job migration
    GENERAL = 0,
    /// this domain (or domains) consist of the user-provided worker IDs
    EXACT_ID,
    /// single MAIN worker
    MAIN,
    /**
     * workers that are in the "exclusive" mode:
     * - no incoming jobs allowed
     * - all child jobs are scheduled to the same worker
     * - no coroutine migration allowed
     */
    EXCLUSIVE,
};

// CC-OFFNXT(G.FUD.06) switch-case, ODR
inline std::ostream &operator<<(std::ostream &os, JobWorkerThreadDomain domain)
{
    switch (domain) {
        case JobWorkerThreadDomain::GENERAL:
            os << "GENERAL";
            break;
        case JobWorkerThreadDomain::EXACT_ID:
            os << "EXACT_ID";
            break;
        case JobWorkerThreadDomain::MAIN:
            os << "MAIN";
            break;
        case JobWorkerThreadDomain::EXCLUSIVE:
            os << "EXCLUSIVE";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

}  // namespace ark

#endif /* PANDA_RUNTIME_EXECUTION_JOB_WORKER_DOMAIN_H */
