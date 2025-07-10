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

#ifndef PANDA_RUNTIME_COROUTINES_AFFINITY_MASK_H
#define PANDA_RUNTIME_COROUTINES_AFFINITY_MASK_H

#include "libpandabase/globals.h"

#include "runtime/coroutines/coroutine_worker.h"

namespace ark {

class AffinityMask final {
public:
    using Storage = size_t;

    // maximum workers count should conform to number of bits in the affinity mask
    static constexpr size_t MAX_WORKERS_COUNT = sizeof(Storage) * BITS_PER_BYTE;

    AffinityMask() = default;
    ~AffinityMask() = default;

    DEFAULT_MOVE_SEMANTIC(AffinityMask);
    DEFAULT_COPY_SEMANTIC(AffinityMask);

    size_t NumAllowedWorkers() const
    {
        std::bitset<MAX_WORKERS_COUNT> affinityBits(data_);
        return affinityBits.count();
    }

    bool IsWorkerAllowed(CoroutineWorker::Id workerId) const
    {
        std::bitset<MAX_WORKERS_COUNT> affinityBits(data_);
        return affinityBits.test(workerId);
    }

    AffinityMask &SetWorkerAllowed(CoroutineWorker::Id workerId)
    {
        std::bitset<MAX_WORKERS_COUNT> affinityBits(data_);
        data_ = affinityBits.set(workerId).to_ullong();
        return *this;
    }

    AffinityMask &SetWorkerNotAllowed(CoroutineWorker::Id workerId)
    {
        std::bitset<MAX_WORKERS_COUNT> affinityBits(data_);
        data_ = affinityBits.reset(workerId).to_ullong();
        return *this;
    }

    bool operator==(const AffinityMask &rhs) const
    {
        return this->data_ == rhs.data_;
    }

    // NOTE: constexpr
    static AffinityMask Full()
    {
        return AffinityMask(MASK_FULL);
    }

    // NOTE: constexpr
    static AffinityMask Empty()
    {
        return AffinityMask(MASK_NONE);
    }

private:
    // all ones
    static constexpr Storage MASK_FULL = std::numeric_limits<Storage>::max();
    // all zeros
    static constexpr Storage MASK_NONE = 0U;

    // NOTE: constexpr
    explicit AffinityMask(Storage data) : data_(data) {}

    // every bit means that the corresponding worker is allowed
    Storage data_;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_AFFINITY_MASK_H */
