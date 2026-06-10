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
#include <random>
#include <algorithm>

namespace ark::mem::test {

// Test structure to track allocations
struct TestObject {
    static std::atomic<int> constructorCount;
    static std::atomic<int> destructorCount;

    static constexpr size_t dataSize = 64;  // Size of test data buffer

    int value;
    char data[dataSize];

    explicit TestObject(int v = 0) : value(v)
    {
        constructorCount++;
    }

    ~TestObject()
    {
        destructorCount++;
    }

    NO_COPY_SEMANTIC(TestObject);
    NO_MOVE_SEMANTIC(TestObject);
};

std::atomic<int> TestObject::constructorCount {0};
std::atomic<int> TestObject::destructorCount {0};

class InternalArenaAllocatorTest : public testing::Test {
public:
    InternalArenaAllocatorTest()
    {
        constexpr size_t internalPoolSize = 128_MB;
        MemConfig::Initialize(0, internalPoolSize, 0, 0, 0, 0);
        PoolManager::Initialize();
        memStats_ = new MemStatsType();
        allocator_ = new InternalAllocatorT<InternalAllocatorConfig::PANDA_ALLOCATORS>(memStats_);
    }

    ~InternalArenaAllocatorTest() override
    {
        delete static_cast<Allocator *>(allocator_);
        PoolManager::Finalize();
        MemConfig::Finalize();
        delete memStats_;

        // Reset counters
        TestObject::constructorCount = 0;
        TestObject::destructorCount = 0;
    }

    NO_COPY_SEMANTIC(InternalArenaAllocatorTest);
    NO_MOVE_SEMANTIC(InternalArenaAllocatorTest);

protected:
    InternalAllocatorPtr allocator_;
    MemStatsType *memStats_;
};

TEST_F(InternalArenaAllocatorTest, CreateAndDestroy)
{
    InternalArenaAllocator arenaAllocator {allocator_};
    ASSERT_EQ(arenaAllocator.GetAllocatedSize(), 0);
}

TEST_F(InternalArenaAllocatorTest, BasicAllocation)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    constexpr size_t allocSize = 1024;
    void *ptr = arenaAllocator.Alloc(allocSize);
    ASSERT_NE(ptr, nullptr);

    ASSERT_EQ(arenaAllocator.GetAllocatedSize(), allocSize);
}

TEST_F(InternalArenaAllocatorTest, MultipleAllocations)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    constexpr size_t numAllocations = 100;
    constexpr size_t allocSize = 128;

    for (size_t i = 0; i < numAllocations; ++i) {
        void *ptr = arenaAllocator.Alloc(allocSize);
        ASSERT_NE(ptr, nullptr);
    }

    ASSERT_EQ(arenaAllocator.GetAllocatedSize(), numAllocations * allocSize);
}

TEST_F(InternalArenaAllocatorTest, AllocateObject)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    constexpr int testValue = 42;
    auto obj = arenaAllocator.New<TestObject>(testValue);
    ASSERT_NE(obj, nullptr);
    ASSERT_EQ(obj->value, testValue);
    ASSERT_EQ(TestObject::constructorCount, 1);

    // Manually call destructor for arena-allocated objects
    obj->~TestObject();
    ASSERT_EQ(TestObject::destructorCount, 1);
}

TEST_F(InternalArenaAllocatorTest, ResizeAllocator)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    constexpr size_t allocateSize = 1024;
    void *ptr = arenaAllocator.Alloc(allocateSize);
    ASSERT_NE(ptr, nullptr);

    size_t allocatedSize = arenaAllocator.GetAllocatedSize();
    ASSERT_EQ(allocatedSize, allocateSize);

    // Resize to a smaller size
    constexpr size_t resizeSize = 512;
    arenaAllocator.Resize(resizeSize);
    ASSERT_EQ(arenaAllocator.GetAllocatedSize(), resizeSize);
}

TEST_F(InternalArenaAllocatorTest, LargeAllocation)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    constexpr size_t largeSize = 256 * 1024;
    void *ptr = arenaAllocator.Alloc(largeSize);
    ASSERT_NE(ptr, nullptr);

    ASSERT_GE(arenaAllocator.GetAllocatedSize(), largeSize);
}

TEST_F(InternalArenaAllocatorTest, MultipleArenaAllocations)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    // Allocate more than one arena's worth
    constexpr size_t allocSize = 200 * 1024;  // Larger than INTERNAL_ALLOCATOR_ARENA_SIZE
    constexpr int allocNum = 5;
    for (int i = 0; i < allocNum; ++i) {
        void *ptr = arenaAllocator.Alloc(allocSize);
        ASSERT_NE(ptr, nullptr);
    }
}

TEST_F(InternalArenaAllocatorTest, AllocatorAdapter)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    using Adapter = InternalArenaAllocator::Adapter<int>;
    using Vector = std::vector<int, Adapter>;

    Adapter adapter(arenaAllocator);
    Vector vec(adapter);

    constexpr int numElements = 1000;
    for (int i = 0; i < numElements; ++i) {
        vec.push_back(i);
    }

    ASSERT_EQ(vec.size(), numElements);
    ASSERT_EQ(vec[numElements - 1], numElements - 1);
}

TEST_F(InternalArenaAllocatorTest, EmptyAllocation)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    void *ptr = arenaAllocator.Alloc(0);
    ASSERT_NE(ptr, nullptr);
    // Arena allocator behavior for 0-size alloc may vary
    // Just ensure it doesn't crash
    (void)ptr;  // Suppress unused variable warning
}

TEST_F(InternalArenaAllocatorTest, StressTest)
{
    InternalArenaAllocator arenaAllocator {allocator_};

    constexpr int numIterations = 1000;
    constexpr size_t minAllocSize = 64;    // Minimum random allocation size
    constexpr size_t maxAllocSize = 4096;  // Maximum random allocation size
    constexpr int randomSeed = 42;         // Fixed seed for reproducibility
    std::mt19937 gen(randomSeed);
    std::uniform_int_distribution<size_t> sizeDist(minAllocSize, maxAllocSize);

    for (int i = 0; i < numIterations; ++i) {
        size_t size = sizeDist(gen);
        void *ptr = arenaAllocator.Alloc(size);
        ASSERT_NE(ptr, nullptr);
    }
}

}  // namespace ark::mem::test

namespace ark::mem {
template void InternalAllocator<InternalAllocatorConfig::PANDA_ALLOCATORS>::VisitAndRemoveAllPools<ark::MemVisitor>(
    ark::MemVisitor);
template void InternalAllocator<InternalAllocatorConfig::PANDA_ALLOCATORS>::VisitAndRemoveFreePools<ark::MemVisitor>(
    ark::MemVisitor);
}  // namespace ark::mem
