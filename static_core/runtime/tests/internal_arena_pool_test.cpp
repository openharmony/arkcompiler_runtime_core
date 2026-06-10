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

#include "gtest/gtest.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/pool_manager.h"
#include "runtime/mem/internal_arena_allocator.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/mem/internal_allocator-inl.h"
#include "runtime/mem/mem_stats_additional_info.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/include/mem/allocator.h"

#include <atomic>
#include <thread>
#include <vector>

namespace ark::mem::test {

class InternalArenaPoolTest : public testing::Test {
public:
    InternalArenaPoolTest()
    {
        constexpr size_t internalPoolSize = 128_MB;
        MemConfig::Initialize(0, internalPoolSize, 0, 0, 0, 0);
        PoolManager::Initialize();
        memStats_ = new MemStatsType();
        allocator_ = new InternalAllocatorT<InternalAllocatorConfig::PANDA_ALLOCATORS>(memStats_);
    }

    ~InternalArenaPoolTest() override
    {
        delete static_cast<Allocator *>(allocator_);
        PoolManager::Finalize();
        MemConfig::Finalize();
        delete memStats_;
    }

    NO_COPY_SEMANTIC(InternalArenaPoolTest);
    NO_MOVE_SEMANTIC(InternalArenaPoolTest);

protected:
    InternalAllocatorPtr allocator_;
    MemStatsType *memStats_;
};

TEST_F(InternalArenaPoolTest, PoolGetFromEmpty)
{
    InternalArenaPool pool;

    auto *arenaAllocator = pool.Get(allocator_);
    ASSERT_NE(arenaAllocator, nullptr);

    pool.Clear();
}

TEST_F(InternalArenaPoolTest, PoolGetAndRelease)
{
    InternalArenaPool pool;

    auto *allocator1 = pool.Get(allocator_);
    ASSERT_NE(allocator1, nullptr);

    constexpr size_t allocSize = 512;
    void *ptr1 = allocator1->Alloc(allocSize);
    ASSERT_NE(ptr1, nullptr);

    pool.Release(allocator1);
    auto *allocator2 = pool.Get(allocator_);
    ASSERT_NE(allocator2, nullptr);
    ASSERT_EQ(allocator2->GetAllocatedSize(), 0);
    pool.Clear();
}

TEST_F(InternalArenaPoolTest, PoolGetReusesAllocator)
{
    InternalArenaPool pool;

    auto *allocator1 = pool.Get(allocator_);
    ASSERT_NE(allocator1, nullptr);

    constexpr size_t allocSize = 256;
    void *ptr1 = allocator1->Alloc(allocSize);
    ASSERT_NE(ptr1, nullptr);

    auto *allocatorPtr1 = allocator1;
    pool.Release(allocator1);
    auto *allocator2 = pool.Get(allocator_);
    ASSERT_EQ(allocatorPtr1, allocator2);

    pool.Clear();
}

TEST_F(InternalArenaPoolTest, PoolReleaseReplacesOld)
{
    InternalArenaPool pool;

    auto *allocator1 = pool.Get(allocator_);
    ASSERT_NE(allocator1, nullptr);
    constexpr size_t allocSize = 1024;
    allocator1->Alloc(allocSize);

    auto *allocator2 = pool.Get(allocator_);
    ASSERT_NE(allocator2, nullptr);
    ASSERT_NE(allocator1, allocator2);  // Should be different instances

    pool.Release(allocator2);
    auto *allocator3 = pool.Get(allocator_);
    ASSERT_EQ(allocator2, allocator3);  // Should reuse allocator2

    pool.Release(allocator1);
    auto *allocator4 = pool.Get(allocator_);
    ASSERT_EQ(allocator1, allocator4);             // Should get allocator1
    ASSERT_EQ(allocator1->GetAllocatedSize(), 0);  // Because Resize(0) was called

    pool.Release(allocator4);
    pool.Clear();
}

TEST_F(InternalArenaPoolTest, PoolClear)
{
    InternalArenaPool pool;

    auto *allocator = pool.Get(allocator_);
    ASSERT_NE(allocator, nullptr);

    constexpr size_t allocSize = 1024;
    allocator->Alloc(allocSize);
    pool.Release(allocator);
    pool.Clear();

    // Get should create a new allocator
    auto *newAllocator = pool.Get(allocator_);
    ASSERT_NE(newAllocator, nullptr);
    ASSERT_EQ(newAllocator->GetAllocatedSize(), 0);

    pool.Clear();
}

TEST_F(InternalArenaPoolTest, PoolMultipleClears)
{
    InternalArenaPool pool;

    // Multiple clears should be safe
    pool.Clear();
    pool.Clear();
    pool.Clear();

    auto *allocator = pool.Get(allocator_);
    ASSERT_NE(allocator, nullptr);

    pool.Clear();
}

// Helper function for concurrent Get and Release operations
static void ConcurrentGetReleaseWorker(InternalArenaPool *pool, std::atomic<int> *successCount,
                                       InternalAllocatorPtr allocator, int operationsPerThread, size_t allocSize)
{
    for (int i = 0; i < operationsPerThread; ++i) {
        auto *alloc = pool->Get(allocator);
        if (alloc != nullptr) {
            alloc->Alloc(allocSize);
            pool->Release(alloc);
            (*successCount)++;
        }
    }
}

TEST_F(InternalArenaPoolTest, PoolConcurrentGetRelease)
{
    InternalArenaPool pool;
    constexpr int numThreads = 4;
    constexpr int operationsPerThread = 100;
    constexpr size_t allocSize = 64;

    std::vector<std::thread> threads;
    std::atomic<int> successCount {0};

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back(ConcurrentGetReleaseWorker, &pool, &successCount, allocator_, operationsPerThread,
                             allocSize);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    ASSERT_EQ(successCount, numThreads * operationsPerThread);

    pool.Clear();
}

// Helper function for concurrent Get operations
static void ConcurrentGetWorker(InternalArenaPool *pool, std::atomic<int> *successCount, InternalAllocatorPtr allocator,
                                int operationsPerThread)
{
    for (int i = 0; i < operationsPerThread; ++i) {
        auto *alloc = pool->Get(allocator);
        if (alloc != nullptr) {
            (*successCount)++;
        }
    }
}

TEST_F(InternalArenaPoolTest, PoolConcurrentGet)
{
    InternalArenaPool pool;
    constexpr int numThreads = 8;
    constexpr int operationsPerThread = 50;

    std::vector<std::thread> threads;
    std::atomic<int> successCount {0};

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back(ConcurrentGetWorker, &pool, &successCount, allocator_, operationsPerThread);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    // Should get at least some successful allocations
    ASSERT_GT(successCount, 0);

    pool.Clear();
}

TEST_F(InternalArenaPoolTest, PoolMemoryResetAfterRelease)
{
    InternalArenaPool pool;

    auto *allocator = pool.Get(allocator_);
    ASSERT_NE(allocator, nullptr);

    // Make some allocations
    constexpr size_t largeAlloc = 10 * 1024;
    constexpr int numAllocations = 3;
    for (int i = 0; i < numAllocations; i++) {
        allocator->Alloc(largeAlloc);
    }

    size_t allocatedSize = allocator->GetAllocatedSize();
    ASSERT_EQ(allocatedSize, largeAlloc * numAllocations);

    pool.Release(allocator);
    auto *newAllocator = pool.Get(allocator_);
    ASSERT_NE(newAllocator, nullptr);
    ASSERT_EQ(newAllocator->GetAllocatedSize(), 0);

    pool.Clear();
}

TEST_F(InternalArenaPoolTest, PoolDestructorCleanup)
{
    auto *pool = new InternalArenaPool();

    auto *allocator = pool->Get(allocator_);
    ASSERT_NE(allocator, nullptr);

    constexpr size_t testAllocSize = 1024;  // 1 KB test allocation
    allocator->Alloc(testAllocSize);

    delete pool;
}

TEST_F(InternalArenaPoolTest, PoolAllocatorConsistency)
{
    InternalArenaPool pool;

    auto *alloc1 = pool.Get(allocator_);
    ASSERT_NE(alloc1, nullptr);
    // Compare raw pointers instead of AllocatorPtr
    ASSERT_EQ(static_cast<Allocator *>(alloc1->GetInternalAllocator()), static_cast<Allocator *>(allocator_));

    pool.Release(alloc1);

    auto *alloc2 = pool.Get(allocator_);
    ASSERT_NE(alloc2, nullptr);
    ASSERT_EQ(static_cast<Allocator *>(alloc2->GetInternalAllocator()), static_cast<Allocator *>(allocator_));

    pool.Clear();
}

TEST_F(InternalArenaPoolTest, ClassInfoPatternTest)
{
    // Simulates the ClassInfo usage pattern where allocator is obtained,
    // used for some allocations, then released with resize

    InternalArenaPool pool;

    // Simulate ClassInfo construction
    auto *allocator = pool.Get(allocator_);
    ASSERT_NE(allocator, nullptr);

    size_t initialSize = allocator->GetAllocatedSize();
    ASSERT_EQ(initialSize, 0);

    // Simulate allocations during ClassInfo usage
    constexpr size_t allocSize = 1024;
    void *ptr1 = allocator->Alloc(allocSize);
    ASSERT_NE(ptr1, nullptr);
    void *ptr2 = allocator->Alloc(allocSize);
    ASSERT_NE(ptr2, nullptr);

    size_t usedSize = allocator->GetAllocatedSize();
    ASSERT_GT(usedSize, initialSize);

    // Simulate ClassInfo destruction - resize back
    allocator->Resize(initialSize);
    ASSERT_EQ(allocator->GetAllocatedSize(), initialSize);

    // Release back to pool
    pool.Release(allocator);

    // Next "ClassInfo" should reuse the allocator
    auto *newAllocator = pool.Get(allocator_);
    ASSERT_NE(newAllocator, nullptr);

    // Should start from zero again
    ASSERT_EQ(newAllocator->GetAllocatedSize(), 0);

    pool.Clear();
}

TEST_F(InternalArenaPoolTest, NestedClassInfoPatternTest)
{
    // Simulates nested ClassInfo creation (e.g., during proxy class building)
    // Note: This test shows that nested Get calls DON'T share the allocator
    // unless the first one is released first. Each Get creates a new allocator
    // if one isn't already in the pool.

    InternalArenaPool pool;

    // Outer "ClassInfo"
    auto *allocator1 = pool.Get(allocator_);
    ASSERT_NE(allocator1, nullptr);

    size_t pos1 = allocator1->GetAllocatedSize();
    constexpr size_t allocSize = 512;
    allocator1->Alloc(allocSize);

    // Inner "ClassInfo" - this will create a NEW allocator because the first
    // one hasn't been released back to the pool yet
    auto *allocator2 = pool.Get(allocator_);
    ASSERT_NE(allocator2, nullptr);

    // These will be DIFFERENT allocator instances
    ASSERT_NE(allocator1, allocator2);

    // Inner cleanup
    allocator2->Resize(0);
    pool.Release(allocator2);

    // Outer continues
    allocator1->Alloc(allocSize);

    // Outer destruction
    allocator1->Resize(pos1);
    pool.Release(allocator1);

    pool.Clear();
}

}  // namespace ark::mem::test

namespace ark::mem {
template void InternalAllocator<InternalAllocatorConfig::PANDA_ALLOCATORS>::VisitAndRemoveAllPools<ark::MemVisitor>(
    ark::MemVisitor);
template void InternalAllocator<InternalAllocatorConfig::PANDA_ALLOCATORS>::VisitAndRemoveFreePools<ark::MemVisitor>(
    ark::MemVisitor);
}  // namespace ark::mem
