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

#include "test_common.h"

namespace ark::tooling::hprof::test {

constexpr uintptr_t ADDRESS_A = 0x1000U;
constexpr uintptr_t ADDRESS_B = 0x2000U;
constexpr uintptr_t ADDRESS_C = 0x3000U;
constexpr uintptr_t ADDRESS_D = 0x4000U;
constexpr uintptr_t DYNAMIC_ONLY_ADDRESS = 0xA000U;
constexpr uintptr_t STATIC_ONLY_ADDRESS = 0xB000U;

class ObjectIdMapTest : public ::testing::Test {
protected:
    ObjectIdMap &Map()
    {
        return map_;
    }

private:
    ObjectIdMap map_;
};

// --- Construction / NonCopyable ---

TEST_F(ObjectIdMapTest, Constructor_Default)
{
    SUCCEED();
}

// --- FindOrInsert basics: returns sequential nodeId ---

TEST_F(ObjectIdMapTest, FindOrInsertDynamic_ReturnsNodeId)
{
    uint64_t id = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    ASSERT_EQ(id, 2ULL);  // first insertion gets the first even nodeId (2)
}

TEST_F(ObjectIdMapTest, FindOrInsertDynamic_DifferentAddressesDifferentIds)
{
    auto id1 = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    auto id2 = Map().FindOrInsert<Language::STATIC>(ADDRESS_B);
    auto id3 = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_C);
    // Sequential even nodeIds: 2, 4, 6
    ASSERT_EQ(id1, 2ULL);
    ASSERT_EQ(id2, 4ULL);
    ASSERT_EQ(id3, 6ULL);
}

TEST_F(ObjectIdMapTest, FindOrInsertDynamic_Deduplication)
{
    auto id1 = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    auto id2 = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    ASSERT_EQ(id1, id2);  // Same address -> same ID
}

// --- FindOrInsert<STATIC> basics ---

TEST_F(ObjectIdMapTest, FindOrInsertStatic_ReturnsNodeId)
{
    uint64_t id = Map().FindOrInsert<Language::STATIC>(ADDRESS_A);
    ASSERT_EQ(id, 2ULL);  // first insertion gets the first even nodeId (2)
}

TEST_F(ObjectIdMapTest, FindOrInsertStatic_Deduplication)
{
    auto id1 = Map().FindOrInsert<Language::STATIC>(ADDRESS_A);
    auto id2 = Map().FindOrInsert<Language::STATIC>(ADDRESS_A);
    ASSERT_EQ(id1, id2);
}

// --- Cross-VM: same address, first insertion wins ---

TEST_F(ObjectIdMapTest, SameAddress_FirstInsertionWins)
{
    auto dynId = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    auto staId = Map().FindOrInsert<Language::STATIC>(ADDRESS_A);
    // Same address already has a nodeId; STATIC call returns the same nodeId.
    ASSERT_EQ(dynId, staId);
    // Only DYNAMIC count is incremented (first insertion language)
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 1U);
    ASSERT_EQ(Map().CountOf<Language::STATIC>(), 0U);
}

// --- Different addresses produce different identifiers ---

TEST_F(ObjectIdMapTest, DifferentAddresses_DifferentIds)
{
    auto dynId = Map().FindOrInsert<Language::DYNAMIC>(DYNAMIC_ONLY_ADDRESS);
    auto staId = Map().FindOrInsert<Language::STATIC>(STATIC_ONLY_ADDRESS);
    ASSERT_NE(dynId, staId);
    // nodeIds are even: 2 and 4
    ASSERT_EQ(dynId, 2ULL);
    ASSERT_EQ(staId, 4ULL);
}

// --- Default template parameter (DYNAMIC) ---

TEST_F(ObjectIdMapTest, FindOrInsert_DefaultIsDYNAMIC)
{
    auto id = Map().FindOrInsert(ADDRESS_A);
    ASSERT_EQ(id, 2ULL);  // first even nodeId (2), default template param is DYNAMIC
}

// --- Find (non-template, read-only lookup) ---

TEST_F(ObjectIdMapTest, Find_NotFound)
{
    uint64_t id = Map().Find(ADDRESS_A);
    ASSERT_EQ(id, 0U);
}

TEST_F(ObjectIdMapTest, Find_Found)
{
    auto inserted = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    auto found = Map().Find(ADDRESS_A);
    ASSERT_EQ(found, inserted);
}

TEST_F(ObjectIdMapTest, Find_DoesNotInsert)
{
    Map().Find(ADDRESS_A);
    ASSERT_EQ(Map().Count(), 0U);
}

// --- Zero address edge case: gets an even nodeId (address 0 is a valid entry) ---

TEST_F(ObjectIdMapTest, ZeroAddress_ReturnsNodeId)
{
    auto id = Map().FindOrInsert<Language::DYNAMIC>(0);
    ASSERT_EQ(id, 2ULL);  // address 0 gets the first even nodeId (0 is reserved for "not found")
}

// --- Count methods ---

TEST_F(ObjectIdMapTest, Count_InitialZero)
{
    ASSERT_EQ(Map().Count(), 0U);
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 0U);
    ASSERT_EQ(Map().CountOf<Language::STATIC>(), 0U);
}

TEST_F(ObjectIdMapTest, Count_IncrementOnInsert)
{
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_B);
    Map().FindOrInsert<Language::STATIC>(ADDRESS_C);
    ASSERT_EQ(Map().Count(), 3U);
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 2U);
    ASSERT_EQ(Map().CountOf<Language::STATIC>(), 1U);
}

TEST_F(ObjectIdMapTest, Count_DedupDoesNotIncrement)
{
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);  // duplicate
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 1U);
    ASSERT_EQ(Map().Count(), 1U);
}

// FindOrInsert on a frozen map returns 0.
TEST_F(ObjectIdMapTest, FindOrInsert_Frozen_ReturnsZero)
{
    // Fill with one entry, then freeze.
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    ASSERT_EQ(Map().Count(), 1U);
    Map().Freeze();
    ASSERT_TRUE(Map().IsFrozen());

    // A new address on a frozen map is rejected: FindOrInsert returns 0 and
    // no entry is added.
    auto id = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_B);
    ASSERT_EQ(id, 0U);
    ASSERT_EQ(Map().Count(), 1U);

    // An EXISTING address also returns 0 when frozen: the frozen guard runs
    // before the lookup path, so freeze blocks FindOrInsert entirely. The
    // frozen-safe read path is Find().
    auto existingId = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    ASSERT_EQ(existingId, 0U);
    // Find() is frozen-safe and still resolves the entry.
    ASSERT_EQ(Map().Find(ADDRESS_A), 2U);
}

// --- Freeze / Unfreeze ---

TEST_F(ObjectIdMapTest, Freeze_RejectsInsertion)
{
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    Map().Freeze();
    auto id = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_B);
    ASSERT_EQ(id, 0U);  // frozen -> returns 0
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 1U);
}

TEST_F(ObjectIdMapTest, Freeze_FindStillWorks)
{
    auto origId = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    Map().Freeze();
    auto found = Map().Find(ADDRESS_A);
    ASSERT_EQ(found, origId);
}

TEST_F(ObjectIdMapTest, Unfreeze_OnlyFlipsFlag)
{
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    ASSERT_EQ(Map().Count(), 1U);
    ASSERT_FALSE(Map().IsFrozen());

    Map().Freeze();
    ASSERT_TRUE(Map().IsFrozen());

    Map().Unfreeze();
    ASSERT_FALSE(Map().IsFrozen());
    // Data is NOT cleared - Unfreeze only flips frozen_
    ASSERT_EQ(Map().Count(), 1U);
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 1U);
}

TEST_F(ObjectIdMapTest, IsFrozen_TracksState)
{
    ASSERT_FALSE(Map().IsFrozen());
    Map().Freeze();
    ASSERT_TRUE(Map().IsFrozen());
    Map().Unfreeze();
    ASSERT_FALSE(Map().IsFrozen());
}

// --- Reset (clear all data and unfreeze) ---

TEST_F(ObjectIdMapTest, Reset_ClearsAllData)
{
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    Map().FindOrInsert<Language::STATIC>(ADDRESS_B);
    ASSERT_EQ(Map().Count(), 2U);
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 1U);
    ASSERT_EQ(Map().CountOf<Language::STATIC>(), 1U);

    Map().Reset();
    ASSERT_EQ(Map().Count(), 0U);
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 0U);
    ASSERT_EQ(Map().CountOf<Language::STATIC>(), 0U);
    ASSERT_FALSE(Map().IsFrozen());
}

TEST_F(ObjectIdMapTest, Reset_UnfreezesAfterFreeze)
{
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    Map().Freeze();
    ASSERT_TRUE(Map().IsFrozen());

    Map().Reset();
    ASSERT_FALSE(Map().IsFrozen());
    ASSERT_EQ(Map().Count(), 0U);
}

TEST_F(ObjectIdMapTest, Reset_AllowsNewInsertions)
{
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    Map().Freeze();
    Map().Reset();

    // Same address can be re-inserted after Reset - gets fresh nodeId
    uint64_t id = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    ASSERT_EQ(id, 2ULL);  // nodeId restarts at 2 after Reset (first even nodeId)
    ASSERT_EQ(Map().Count(), 1U);
}

TEST_F(ObjectIdMapTest, Reset_EquivalentToFreshMap)
{
    Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    Map().FindOrInsert<Language::STATIC>(ADDRESS_B);
    Map().Freeze();
    Map().Reset();

    // After Reset, the map should behave like a fresh ObjectIdMap
    ObjectIdMap freshMap;
    ASSERT_EQ(Map().Count(), freshMap.Count());
    ASSERT_EQ(Map().IsFrozen(), freshMap.IsFrozen());
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), freshMap.CountOf<Language::DYNAMIC>());
}

// Large-scale insertion must not exhaust memory: a million distinct
// addresses should all resolve without OOM.
TEST_F(ObjectIdMapTest, LargeQuantityInsertion_NoOOM)
{
    // Insert 1 million distinct addresses - should not OOM
    constexpr int LARGE_COUNT = 1000000;
    for (int i = 0; i < LARGE_COUNT; i++) {
        auto id = Map().FindOrInsert<Language::DYNAMIC>(static_cast<uintptr_t>(i) + 1U);
        ASSERT_NE(id, 0ULL);
    }
    ASSERT_EQ(Map().Count(), static_cast<size_t>(LARGE_COUNT));
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), static_cast<size_t>(LARGE_COUNT));
    // Verify the first and last IDs are correct (even nodeIds: addr k -> id 2*k)
    ASSERT_EQ(Map().Find(1), 2ULL);
    ASSERT_EQ(Map().Find(static_cast<uintptr_t>(LARGE_COUNT)), static_cast<uint64_t>(LARGE_COUNT) * 2U);
}

// Once frozen the map is read-only; concurrent Find callers must not crash or
// race on the shared underlying storage.
TEST_F(ObjectIdMapTest, Freeze_MultiThreadedReadOnlySafe)
{
    // Insert some data, then freeze
    for (int i = 0; i < FROZEN_READ_ENTRY_COUNT; i++) {
        Map().FindOrInsert<Language::DYNAMIC>(static_cast<uintptr_t>(i) + 1U);
    }
    Map().Freeze();
    ASSERT_TRUE(Map().IsFrozen());

    // Concurrent Find() on frozen map - should not crash
    std::atomic<int> errorCount {0};
    auto threadFunc = [this, &errorCount]() {
        for (int i = 0; i < FROZEN_READ_ENTRY_COUNT; i++) {
            auto found = Map().Find(static_cast<uintptr_t>(i) + 1U);
            // addr (i+1) -> even nodeId 2*(i+1)
            if (found != static_cast<uint64_t>(i + 1) * 2U) {
                errorCount++;
            }
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(FROZEN_READ_THREAD_COUNT);
    for (int t = 0; t < FROZEN_READ_THREAD_COUNT; t++) {
        threads.emplace_back(threadFunc);
    }
    for (auto &t : threads) {
        t.join();
    }
    // Atomic with relaxed order reason: joining all readers already synchronizes their counter updates.
    ASSERT_EQ(errorCount.load(std::memory_order_relaxed), 0);
}

// A dump session freezes the map; the next session unfreezes it. Entries and
// their nodeIds must survive the freeze/unfreeze cycle so stable ids are
// observed across sessions.
TEST_F(ObjectIdMapTest, CrossSessionPersistence_DataPreserved)
{
    // First "dump session": insert data
    auto dynId1 = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    auto staId1 = Map().FindOrInsert<Language::STATIC>(ADDRESS_B);
    ASSERT_EQ(dynId1, 2ULL);
    ASSERT_EQ(staId1, 4ULL);
    ASSERT_EQ(Map().Count(), 2U);

    // Freeze after first session (simulates end of dump cycle)
    Map().Freeze();

    // Unfreeze for second session - data persists
    Map().Unfreeze();
    ASSERT_EQ(Map().Count(), 2U);
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 1U);
    ASSERT_EQ(Map().CountOf<Language::STATIC>(), 1U);

    // Second session: existing addresses return the same nodeId
    auto dynId2 = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_A);
    auto staId2 = Map().FindOrInsert<Language::STATIC>(ADDRESS_B);
    ASSERT_EQ(dynId2, dynId1);  // same ID as first session
    ASSERT_EQ(staId2, staId1);  // same ID as first session

    // New addresses in second session get new nodeIds
    auto newId = Map().FindOrInsert<Language::DYNAMIC>(ADDRESS_C);
    ASSERT_EQ(newId, 6ULL);
    ASSERT_EQ(Map().Count(), 3U);
}

// --- Per-round liveness: MarkLive / PrepareRound / PruneDead / ForEachLive ---

TEST_F(ObjectIdMapTest, MarkLive_NewAddress_ReturnsTrueAndAssignsNodeId)
{
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_A));
    ASSERT_EQ(Map().Find(ADDRESS_A), 2U);  // first even nodeId
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_B));
    ASSERT_EQ(Map().Find(ADDRESS_B), 4U);
}

TEST_F(ObjectIdMapTest, MarkLive_SameAddressTwice_SecondReturnsFalse)
{
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_A));   // first visit -> push
    ASSERT_FALSE(Map().MarkLive<Language::STATIC>(ADDRESS_A));  // already live -> skip
    ASSERT_EQ(Map().CountLive(), 1U);
}

TEST_F(ObjectIdMapTest, MarkLive_Frozen_ReturnsFalse)
{
    Map().MarkLive<Language::STATIC>(ADDRESS_A);
    Map().Freeze();
    ASSERT_FALSE(Map().MarkLive<Language::STATIC>(ADDRESS_B));
    Map().Unfreeze();
}

TEST_F(ObjectIdMapTest, PrepareRound_MarksAllDeadWithoutErasing)
{
    Map().MarkLive<Language::STATIC>(ADDRESS_A);
    Map().MarkLive<Language::STATIC>(ADDRESS_B);
    ASSERT_EQ(Map().CountLive(), 2U);
    ASSERT_EQ(Map().Count(), 2U);

    Map().PrepareRound();
    // Entries still present (stable nodeIds) but none live.
    ASSERT_EQ(Map().Count(), 2U);
    ASSERT_EQ(Map().CountLive(), 0U);
    ASSERT_EQ(Map().Find(ADDRESS_A), 2U);  // nodeId preserved
    ASSERT_EQ(Map().Find(ADDRESS_B), 4U);
}

TEST_F(ObjectIdMapTest, PruneDead_ErasesUnmarkedEntries)
{
    Map().MarkLive<Language::STATIC>(ADDRESS_A);  // A
    Map().MarkLive<Language::STATIC>(ADDRESS_B);  // B
    Map().MarkLive<Language::STATIC>(ADDRESS_C);  // C

    Map().PrepareRound();
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_A));  // re-mark A only
    Map().PruneDead();

    ASSERT_EQ(Map().Count(), 1U);
    ASSERT_EQ(Map().CountLive(), 1U);
    ASSERT_EQ(Map().Find(ADDRESS_A), 2U);  // A survived with same nodeId
    ASSERT_EQ(Map().Find(ADDRESS_B), 0U);  // B pruned
    ASSERT_EQ(Map().Find(ADDRESS_C), 0U);  // C pruned
}

TEST_F(ObjectIdMapTest, MultiRound_StableNodeIdForSurvivors)
{
    // Round 1: mark A, B, C
    Map().MarkLive<Language::STATIC>(ADDRESS_A);  // A -> 2
    Map().MarkLive<Language::STATIC>(ADDRESS_B);  // B -> 4
    Map().MarkLive<Language::STATIC>(ADDRESS_C);  // C -> 6
    Map().PruneDead();
    ASSERT_EQ(Map().Count(), 3U);

    // Round 2: only A and D reachable
    Map().PrepareRound();
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_A));  // A survivor -> re-visit, id stays 2
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_D));  // D new -> 8
    Map().PruneDead();

    ASSERT_EQ(Map().Find(ADDRESS_A), 2U);  // A stable
    ASSERT_EQ(Map().Find(ADDRESS_D), 8U);  // D new id (B=4 retired, C=6 retired)
    ASSERT_EQ(Map().Find(ADDRESS_B), 0U);  // B dead
    ASSERT_EQ(Map().Find(ADDRESS_C), 0U);  // C dead
    ASSERT_EQ(Map().Count(), 2U);

    // Round 3: A and D both survive - nodeIds unchanged
    Map().PrepareRound();
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_A));
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_D));
    Map().PruneDead();
    ASSERT_EQ(Map().Find(ADDRESS_A), 2U);
    ASSERT_EQ(Map().Find(ADDRESS_D), 8U);
    ASSERT_EQ(Map().Count(), 2U);
}

TEST_F(ObjectIdMapTest, MultiRound_SurvivorRevisited_NotSkippedAsAlreadyVisited)
{
    // Regression for the bug where a survivor across dumps would be wrongly
    // treated as "already visited" and its subgraph skipped. PrepareRound
    // must clear liveness so MarkLive returns true for a survivor.
    Map().MarkLive<Language::STATIC>(ADDRESS_A);
    Map().PruneDead();

    Map().PrepareRound();
    // The survivor must be reported as first-visit-this-round so the BFS
    // re-traverses it. If MarkLive used map-presence (not liveness) it would
    // return false here and skip the object.
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_A));
}

TEST_F(ObjectIdMapTest, ForEachLive_IteratesOnlyLiveEntries)
{
    Map().MarkLive<Language::STATIC>(ADDRESS_A);
    Map().MarkLive<Language::STATIC>(ADDRESS_B);
    Map().MarkLive<Language::STATIC>(ADDRESS_C);
    Map().PrepareRound();
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_B));  // only B live

    std::unordered_map<uintptr_t, ObjectIdMap::NodeId> seen;
    Map().ForEachLive([&seen](uintptr_t addr, ObjectIdMap::NodeId id) { seen[addr] = id; });
    ASSERT_EQ(seen.size(), 1U);
    ASSERT_EQ(seen[ADDRESS_B], 4U);
}

TEST_F(ObjectIdMapTest, CountOf_ReflectsLiveAfterPruneDead)
{
    Map().MarkLive<Language::STATIC>(ADDRESS_A);
    Map().MarkLive<Language::STATIC>(ADDRESS_B);
    Map().MarkLive<Language::STATIC>(ADDRESS_C);
    Map().PrepareRound();
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_A));  // only A survives
    Map().PruneDead();
    ASSERT_EQ(Map().CountOf<Language::STATIC>(), 1U);
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 0U);
}

TEST_F(ObjectIdMapTest, MarkLive_FirstInsertionLanguageWins)
{
    // MarkLive<STATIC> first, then MarkLive<DYNAMIC> on same addr: language
    // is fixed at first insertion (STATIC), counts reflect that.
    ASSERT_TRUE(Map().MarkLive<Language::STATIC>(ADDRESS_A));
    ASSERT_FALSE(Map().MarkLive<Language::DYNAMIC>(ADDRESS_A));  // already live
    Map().PruneDead();
    ASSERT_EQ(Map().CountOf<Language::STATIC>(), 1U);
    ASSERT_EQ(Map().CountOf<Language::DYNAMIC>(), 0U);
}

TEST_F(ObjectIdMapTest, MarkLive_Hybrid_ReturnsFalse)
{
    ASSERT_FALSE(Map().MarkLive<Language::HYBRID>(ADDRESS_A));
    ASSERT_EQ(Map().Count(), 0U);
}

}  // namespace ark::tooling::hprof::test
