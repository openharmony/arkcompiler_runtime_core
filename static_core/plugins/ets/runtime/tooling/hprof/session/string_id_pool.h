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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_STRING_ID_POOL_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_STRING_ID_POOL_H

#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ark::tooling::hprof {

using StringId = uint32_t;

inline constexpr StringId INVALID_STRING_ID = UINT32_MAX;

/**
 * @brief String pool with freeze semantics. NOT thread-safe for concurrent modification.
 *
 * The caller must ensure single-threaded access during the unfrozen (writable)
 * phase, consistent with the dump lifecycle - Prepare phase is sequential.
 * After Freeze(), only read-only access is permitted, which is safe for
 * concurrent reads without locking.
 *
 * Lifecycle:
 * - Unfrozen: AddString() is allowed, strings are collected from dumpers.
 * - After Freeze(): AddString() returns INVALID_STRING_ID (read-only mode).
 *   GetStringId/GetStringById remain functional.
 * - Serialization is done via CommonWriter::WriteStringPool().
 * - Unfreeze() reopens for a new dump session (strings persist across sessions).
 */
class StringIdPool {
public:
    StringIdPool() = default;
    ~StringIdPool() = default;
    StringIdPool(const StringIdPool &) = delete;
    StringIdPool &operator=(const StringIdPool &) = delete;
    StringIdPool(StringIdPool &&) = default;
    StringIdPool &operator=(StringIdPool &&) = default;

    /**
     * @brief Add a string to the pool (unfrozen phase only).
     * After Freeze() is called, this returns INVALID_STRING_ID.
     */
    StringId AddString(const std::string &str);

    StringId GetStringId(const std::string &str) const;
    const std::string &GetStringById(StringId id) const;

    /** @brief Freeze the pool - transition to read-only mode. */
    void Freeze();

    /**
     * @brief Unfreeze the pool - allow new AddString() calls again.
     * Used at the start of each dump's Prepare phase so the persistent
     * pool can accumulate new strings across multiple dump sessions.
     * Does NOT clear existing strings - they persist.
     */
    void Unfreeze();

    /** @brief Check if the pool is frozen (read-only). */
    bool IsFrozen() const;

    /** @brief Number of strings currently in the pool. */
    size_t Size() const;

    /**
     * @brief Iterate all strings in order (by ID).
     * Called by CommonWriter::WriteStringPool() for serialization.
     * @param callback  Called for each string: callback(stringId, stringContent).
     */
    void ForEachString(const std::function<void(StringId, const std::string &)> &callback) const;

private:
    StringId nextId_ = 0;
    std::unordered_map<std::string, StringId> strToId_;
    std::vector<std::string> idToStr_;
    bool frozen_ = false;
    std::string emptyString_;
};

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_STRING_ID_POOL_H
