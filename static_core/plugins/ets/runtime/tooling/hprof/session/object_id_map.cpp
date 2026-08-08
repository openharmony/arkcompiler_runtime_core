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

#include "plugins/ets/runtime/tooling/hprof/session/object_id_map.h"
#include "libarkbase/utils/logger.h"

namespace ark::tooling::hprof {

ObjectIdMap::NodeId ObjectIdMap::AllocateNodeId()
{
    // 0 is the invalid sentinel; stop before nextNodeId_ would wrap into it.
    // (uint32_t addition wraps, so detect overflow with the subtraction form.)
    if (nextNodeId_ > UINT32_MAX - NODE_ID_STEP) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Object ID allocation failed: ID space exhausted";
        return 0;
    }
    NodeId nodeId = nextNodeId_;
    nextNodeId_ += NODE_ID_STEP;
    return nodeId;
}

template <Language LANG>
ObjectIdMap::NodeId ObjectIdMap::FindOrInsert(uintptr_t addr)
{
    if constexpr (LANG == Language::HYBRID) {
        return 0;
    }

    if (frozen_) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Object ID insertion rejected: map is frozen";
        return 0;
    }

    auto it = seen_.find(addr);
    if (it != seen_.end()) {
        // Existing entry: mark live, keep its nodeId and first-insertion language.
        it->second.live = true;
        return it->second.nodeId;
    }

    NodeId nodeId = AllocateNodeId();
    if (nodeId == 0) {
        return 0;  // exhausted — 0 is the failure sentinel callers already expect
    }
    seen_.emplace(addr, Entry {nodeId, true, LANG});
    return nodeId;
}

template <Language LANG>
bool ObjectIdMap::MarkLive(uintptr_t addr)
{
    if constexpr (LANG == Language::HYBRID) {
        return false;
    }

    if (frozen_) {
        LOG(WARNING, RUNTIME) << "[HybDump][Sta] Object liveness update rejected: map is frozen";
        return false;
    }

    auto it = seen_.find(addr);
    if (it == seen_.end()) {
        NodeId nodeId = AllocateNodeId();
        if (nodeId == 0) {
            return false;  // exhausted — cannot register, caller does not push
        }
        seen_.emplace(addr, Entry {nodeId, true, LANG});
        return true;  // first visit this round -> caller pushes to worklist
    }

    Entry &entry = it->second;
    if (entry.live) {
        return false;  // already live this round -> already visited, do not push
    }
    entry.live = true;  // survivor from a previous round, dead-at-start -> re-mark live
    // Language is fixed at first insertion; do not retag.
    return true;
}

ObjectIdMap::NodeId ObjectIdMap::Find(uintptr_t addr) const
{
    auto it = seen_.find(addr);
    if (it != seen_.end()) {
        return it->second.nodeId;
    }
    return 0;
}

size_t ObjectIdMap::Count() const
{
    return seen_.size();
}

template <Language LANG>
size_t ObjectIdMap::CountOf() const
{
    if constexpr (LANG == Language::HYBRID) {
        return 0;
    }
    size_t count = 0;
    for (const auto &[addr, entry] : seen_) {
        if (entry.live && entry.lang == LANG) {
            count++;
        }
    }
    return count;
}

void ObjectIdMap::PrepareRound()
{
    for (auto &[addr, entry] : seen_) {
        entry.live = false;
    }
}

void ObjectIdMap::PruneDead()
{
    for (auto it = seen_.begin(); it != seen_.end();) {
        if (!it->second.live) {
            it = seen_.erase(it);
        } else {
            ++it;
        }
    }
}

size_t ObjectIdMap::CountLive() const
{
    size_t count = 0;
    for (const auto &[addr, entry] : seen_) {
        if (entry.live) {
            count++;
        }
    }
    return count;
}

void ObjectIdMap::Freeze()
{
    frozen_ = true;
}

void ObjectIdMap::Unfreeze()
{
    frozen_ = false;
}

bool ObjectIdMap::IsFrozen() const
{
    return frozen_;
}

void ObjectIdMap::Reset()
{
    seen_.clear();
    nextNodeId_ = FIRST_USER_NODE_ID;
    frozen_ = false;
}

template ObjectIdMap::NodeId ObjectIdMap::FindOrInsert<Language::DYNAMIC>(uintptr_t);
template ObjectIdMap::NodeId ObjectIdMap::FindOrInsert<Language::STATIC>(uintptr_t);
template ObjectIdMap::NodeId ObjectIdMap::FindOrInsert<Language::HYBRID>(uintptr_t);
template bool ObjectIdMap::MarkLive<Language::DYNAMIC>(uintptr_t);
template bool ObjectIdMap::MarkLive<Language::STATIC>(uintptr_t);
template bool ObjectIdMap::MarkLive<Language::HYBRID>(uintptr_t);
template size_t ObjectIdMap::CountOf<Language::DYNAMIC>() const;
template size_t ObjectIdMap::CountOf<Language::STATIC>() const;
template size_t ObjectIdMap::CountOf<Language::HYBRID>() const;

}  // namespace ark::tooling::hprof
