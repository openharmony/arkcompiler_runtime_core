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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/tooling/hprof/session/string_id_pool.h"
#include "libarkbase/utils/logger.h"

namespace ark::tooling::hprof {

StringId StringIdPool::AddString(const std::string &str)
{
    if (frozen_) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] String insertion rejected: pool is frozen";
        return INVALID_STRING_ID;
    }

    auto [it, inserted] = strToId_.emplace(str, nextId_);
    if (inserted) {
        idToStr_.push_back(str);
        return nextId_++;
    }
    return it->second;
}

StringId StringIdPool::GetStringId(const std::string &str) const
{
    auto it = strToId_.find(str);
    if (it != strToId_.end()) {
        return it->second;
    }
    return INVALID_STRING_ID;
}

const std::string &StringIdPool::GetStringById(StringId id) const
{
    if (id >= idToStr_.size()) {
        return emptyString_;
    }
    return idToStr_[id];
}

void StringIdPool::Freeze()
{
    frozen_ = true;
}

void StringIdPool::Unfreeze()
{
    frozen_ = false;
}

bool StringIdPool::IsFrozen() const
{
    return frozen_;
}

size_t StringIdPool::Size() const
{
    return idToStr_.size();
}

void StringIdPool::ForEachString(const std::function<void(StringId, const std::string &)> &callback) const
{
    for (StringId id = 0; id < idToStr_.size(); ++id) {
        callback(id, idToStr_[id]);
    }
}

}  // namespace ark::tooling::hprof
