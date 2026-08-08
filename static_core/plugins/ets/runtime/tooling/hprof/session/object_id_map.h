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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_OBJECT_ID_MAP_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_OBJECT_ID_MAP_H

#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace ark::tooling::hprof {

/**
 * @brief Per-dump-session address dedup + nodeId assignment + per-round
 *        liveness tracking for root-reachability marking.
 *
 * Assigns each unique address an even nodeId (2, 4, 6, ...; step 2) so it
 * never collides with the dynamic VM's odd EntryIdMap nodeIds.
 * 0 = invalid sentinel, 1 = SyntheticRoot.
 *
 * Two modes:
 *  - Legacy: FindOrInsert<Lang>(addr) inserts + marks live (caller already
 *    knows the object is reachable).
 *  - Root-reachability (multi-round, stable nodeIds): PrepareRound() marks
 *    all entries dead -> MarkLive<Lang>(addr) re-marks reachable, returning
 *    the BFS visited signal -> PruneDead() erases the still-dead ->
 *    ForEachLive() iterates the surviving set.
 *
 * Liveness is a per-round flag, NOT map presence: a survivor from a previous
 * dump is "dead at start of this round" until MarkLive touches it, so the BFS
 * visited-check stays correct across dumps (survivors are re-traversed, not
 * skipped). Find() is a frozen-safe language-agnostic lookup.
 *
 * Not thread-safe for concurrent write; after Freeze() only read-only access
 * (Find/CountOf/ForEachLive) is allowed. Per-dump reset is
 * PrepareRound()/PruneDead() (nodeIds stay stable); Reset() is teardown-only.
 */
class ObjectIdMap {
public:
    /** Sequential nodeId type - even numbers starting from 2, unique within a dump session. */
    using NodeId = uint32_t;

    ObjectIdMap() = default;
    ~ObjectIdMap() = default;
    ObjectIdMap(const ObjectIdMap &) = delete;
    ObjectIdMap &operator=(const ObjectIdMap &) = delete;
    ObjectIdMap(ObjectIdMap &&) = default;
    ObjectIdMap &operator=(ObjectIdMap &&) = default;

    /**
     * @brief Register an address + assign nodeId + mark live (legacy path).
     *
     * If the address was already seen, returns its previously-assigned nodeId
     * (and marks it live) without changing its language or assigning a new id.
     * If new, assigns the next even nodeId (starting from 2, step 2), records
     * the insertion language, and marks it live.
     *
     * The entry's language is fixed at first insertion; a later FindOrInsert
     * with a different Lang does not retag or double-count (see CountOf for
     * how per-language counts are derived).
     *
     * Unfrozen phase only - returns 0 if frozen.
     *
     * @tparam LANG  Language (DYNAMIC, STATIC, or HYBRID).
     *                  HYBRID returns 0 (not a per-VM counting category).
     * @param addr   Object address.
     * @return nodeId (or 0 if frozen / HYBRID).
     */
    template <Language LANG = Language::DYNAMIC>
    NodeId FindOrInsert(uintptr_t addr);

    /**
     * @brief Language-agnostic nodeId lookup - works in frozen state.
     *
     * Returns the nodeId previously assigned for the given address, or 0 if
     * the address was never registered. After PruneDead, every registered
     * address is live, so a nonzero result == live object.
     *
     * Safe to call after Freeze() (read-only).
     *
     * @param addr   Object address.
     * @return nodeId (or 0 if address was not registered).
     */
    NodeId Find(uintptr_t addr) const;

    /**
     * @brief Total number of registered object addresses (live + dead-before-prune).
     * After PruneDead this equals the live count.
     */
    size_t Count() const;

    /**
     * @brief Number of LIVE entries registered for a specific language type.
     * Useful for HEAP_SUMMARY per-VM breakdown. Derived on-the-fly from live
     * entries tagged with Lang - meaningful after PruneDead (or after
     * FindOrInsert in legacy single-round usage where every insert is live).
     * @tparam LANG  Language (DYNAMIC, STATIC, or HYBRID).
     *                  HYBRID returns 0 (not a per-VM counting category).
     */
    template <Language LANG>
    size_t CountOf() const;

    // -- Per-round liveness lifecycle (root-reachability marking) --

    /**
     * @brief Start a new marking round: mark ALL existing entries dead.
     *
     * Does NOT erase anything. Per-round liveness mechanics and cross-round
     * survivor stability are covered by the class doc. Use this at the start
     * of each dump's Prepare phase instead of Reset() so nodeIds stay stable.
     */
    void PrepareRound();

    /**
     * @brief Mark one object live this round (BFS visited-check primitive).
     *
     * - If the address is new: assign the next even nodeId, tag with Lang,
     *   mark live, return true.
     * - If the address exists and is dead-this-round: mark live, return true.
     * - If the address exists and is already live-this-round: return false
     *   (already visited - caller must NOT push it again).
     *
     * The return value is the BFS visited signal: push onto the worklist only
     * when true. Cross-round survivor correctness is covered by the class doc.
     *
     * Language is fixed at first insertion (FindOrInsert/MarkLive do not retag).
     *
     * Unfrozen phase only - returns false if frozen.
     *
     * @tparam LANG  Language (DYNAMIC, STATIC, or HYBRID). HYBRID returns false.
     * @param addr   Object address.
     * @return true if first-marked-live this round (push to worklist),
     *         false if already live this round / frozen / HYBRID.
     */
    template <Language LANG>
    bool MarkLive(uintptr_t addr);

    /**
     * @brief Erase entries still dead this round.
     *
     * After this call the map contains exactly the live (root-reachable) set
     * for the current round. NodeIds of erased entries are retired (not
     * reused); survivors keep their nodeIds.
     */
    void PruneDead();

    /**
     * @brief Iterate live entries: callback(addr, nodeId).
     *
     * Read-only - safe after Freeze(). After PruneDead this iterates the
     * full surviving set. The callback receives (uintptr_t addr, NodeId id).
     */
    template <class F>
    void ForEachLive(F &&callback) const
    {
        for (auto &[addr, entry] : seen_) {
            if (entry.live) {
                callback(addr, entry.nodeId);
            }
        }
    }

    /// @brief Number of live entries (== Count() after PruneDead).
    size_t CountLive() const;

    /** @brief Freeze - transition to read-only mode. FindOrInsert/MarkLive return 0/false. */
    void Freeze();

    /** @brief Unfreeze - flip back to writable. Does NOT clear data or liveness. */
    void Unfreeze();

    /** @brief Check if the map is frozen (read-only). */
    bool IsFrozen() const;

    /**
     * @brief Reset - clear all data and unfreeze. nodeId counter restarts at 2.
     *
     * Intended for full lifecycle teardown ONLY. For per-dump reset use
     * PrepareRound()/PruneDead() instead, so nodeIds stay stable across dumps.
     * Equivalent to constructing a fresh ObjectIdMap.
     */
    void Reset();

private:
    /** Per-address record: assigned nodeId + per-round liveness + insertion language. */
    struct Entry {
        NodeId nodeId {0};
        bool live {false};
        Language lang {Language::DYNAMIC};
    };

    /**
     * @brief Allocate the next even nodeId.
     * @return Fresh nodeId, or 0 if the nodeId space is exhausted (further
     *         allocation would wrap into the 0 invalid sentinel; caller treats
     *         0 as failure and must not record the entry).
     */
    NodeId AllocateNodeId();

    /** Address -> Entry. */
    std::unordered_map<uintptr_t, Entry> seen_;

    /** First user-assigned nodeId (0 = invalid/not-assigned, 1 = SyntheticRoot). */
    static constexpr NodeId FIRST_USER_NODE_ID = 2;

    /** Next even nodeId to assign (see class doc for the even/odd partition). */
    NodeId nextNodeId_ = FIRST_USER_NODE_ID;
    static constexpr NodeId NODE_ID_STEP = 2;

    bool frozen_ = false;
};

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_OBJECT_ID_MAP_H
