/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_GROUP_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_GROUP_H

#include "runtime/coroutines/coroutine_worker.h"

namespace ark {

/// @brief Coroutine domain consists of the coroutine workers that have something in common
enum class CoroutineWorkerDomain {
    /// single MAIN worker
    MAIN = 0,
    /// regular workers that do not support interop, are not "exclusive" and support coroutine migration
    GENERAL,
    /**
     * workers that are in the "exclusive" mode:
     * - no incoming coroutines allowed
     * - all child coroutines are scheduled to the same worker
     * - no coroutine migration allowed
     */
    EA,
    /// this domain (or domains) consist of the user-provided worker IDs
    EXACT_ID
};

/**
 * @brief A group of coroutine workers
 *
 * A coroutine worker group is the set of workers with a unique ID that can be targeted at the coroutine launch time.
 *
 * CoroutineManager's Launch* APIs accept the CoroutineWorkerGroup unique ID and then schedule the new coroutine to
 * on of the workers from the target worker group.
 */
class CoroutineWorkerGroup {
public:
    // should be changed in future
    using Id = CoroutineWorker::Id;
    static constexpr Id INVALID_ID = CoroutineWorker::INVALID_ID;
    static constexpr Id ANY_ID = INVALID_ID - 1;
    static_assert(INVALID_ID != ANY_ID);

    CoroutineWorkerGroup() = delete;

    static bool IsValidNonAnyId(Id id)
    {
        return (id != ANY_ID && id != INVALID_ID);
    }

    // NOTE(konstanting): need to support all domains, exact way of doing this is to implement.
    // Exact contents of this class is to implement, too. We may have some data structure to track the coroutine worker
    // groups or something else...
    static CoroutineWorkerGroup::Id GenerateId(CoroutineWorkerDomain domain, CoroutineWorker::Id hint)
    {
        if ((domain != CoroutineWorkerDomain::EXACT_ID) || (hint == CoroutineWorker::INVALID_ID)) {
            return INVALID_ID;
        }
        return hint;
    }
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_H */
