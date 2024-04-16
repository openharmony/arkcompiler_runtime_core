/*
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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_WAIT_LIST_H
#define LIBPANDABASE_TASKMANAGER_UTILS_WAIT_LIST_H

#include "libpandabase/utils/time.h"
#include <atomic>
#include <functional>
#include <optional>
#include <list>
#include <memory>
#include <limits>

namespace ark::taskmanager {

using WaiterId = size_t;
inline constexpr WaiterId INVALID_WAITER_ID = 0U;

template <class T>
class WaitList {
    class WaitValue {
    public:
        WaitValue(T &&value, uint64_t targetTime, size_t id)
            : value_(std::move(value)), targetTime_(targetTime), id_(id)
        {
        }
        DEFAULT_MOVE_SEMANTIC(WaitValue);
        NO_COPY_SEMANTIC(WaitValue);
        ~WaitValue() = default;

        T &&GetValue()
        {
            return std::move(value_);
        }

        const T &GetValue() const
        {
            return value_;
        }

        uint64_t GetTargetTime() const
        {
            return targetTime_;
        }

        void SetTargetTime(uint64_t newTargetTime)
        {
            targetTime_ = newTargetTime;
        }

        WaiterId GetId() const
        {
            return id_;
        }

    private:
        T value_;
        uint64_t targetTime_;
        WaiterId id_;
    };

public:
    /**
     * @returns value with targetTime less than or equal to current time. If such a value doesn't exist returns
     * std::nullopt.
     */
    std::optional<T> GetReadyValue()
    {
        auto currentTime = time::GetCurrentTimeInMillis(true);
        for (auto iter = waitList_.begin(); iter != waitList_.end(); iter = next(iter)) {
            if (currentTime >= iter->GetTargetTime()) {
                T value = iter->GetValue();
                waitList_.erase(iter);
                return value;
            }
        }
        return std::nullopt;
    }

    /// @returns true if there is a value with a targetTime less or equal to current time. Otherwise false.
    bool HaveReadyValue() const
    {
        auto currentTime = time::GetCurrentTimeInMillis(true);
        for (auto iter = waitList_.begin(); iter != waitList_.end(); iter = next(iter)) {
            if (currentTime >= iter->GetTargetTime()) {
                return true;
            }
        }
        return false;
    }

    /// @brief adds value in waitQueues_ with timeToWait if it's setted.
    WaiterId AddValueToWait(T value, uint64_t timeToWait = std::numeric_limits<uint64_t>().max())
    {
        addCounter_++;
        uint64_t currentTime = time::GetCurrentTimeInMillis(true);
        uint64_t targetTime = 0;

        // Check if we need to set max possible time
        if (std::numeric_limits<uint64_t>().max() - timeToWait < currentTime) {
            targetTime = std::numeric_limits<uint64_t>().max();
        } else {
            targetTime = currentTime + timeToWait;
        }

        waitList_.emplace_back(std::move(value), targetTime, addCounter_);
        return waitList_.back().GetId();
    }

    /// @returns value with specified WaiterId. It it doesn't exists returns std::nullopt.
    std::optional<T> GetValueById(WaiterId id)
    {
        auto waitValue =
            std::find_if(waitList_.begin(), waitList_.end(), [id](const auto &value) { return value.GetId() == id; });
        if (waitValue == waitList_.end()) {
            return std::nullopt;
        }
        auto val = waitValue->GetValue();
        waitList_.erase(waitValue);
        return val;
    }

private:
    std::list<WaitValue> waitList_;
    size_t addCounter_ = 0U;
};

}  // namespace ark::taskmanager

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_WAIT_LIST_H
