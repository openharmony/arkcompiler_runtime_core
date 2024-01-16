/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <ctime>

#include "gtest/gtest.h"
#include "runtime/mem/alloc_config.h"
#include "runtime/mem/bump-allocator-inl.h"
#include "libpandabase/test_utilities.h"

namespace ark::mem {

template <bool USE_TLABS>
using NonObjectBumpAllocator =
    BumpPointerAllocator<EmptyMemoryConfig, BumpPointerAllocatorLockConfig::CommonLock, USE_TLABS>;

class BumpAllocatorTest : public testing::Test {
public:
    BumpAllocatorTest()
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        // NOLINTNEXTLINE(readability-magic-numbers)
        seed_ = 0x0BADDEAD;
#endif
        srand(seed_);
        ark::mem::MemConfig::Initialize(0, 8_MB, 0, 0, 0, 0);
        PoolManager::Initialize();
    }

    ~BumpAllocatorTest() override
    {
        for (auto i : allocatedMemMmap_) {
            ark::os::mem::UnmapRaw(std::get<0>(i), std::get<1>(i));
        }
        for (auto i : allocatedArenas_) {
            delete i;
        }
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
        // Logger::Destroy();
    }

    NO_COPY_SEMANTIC(BumpAllocatorTest);
    NO_MOVE_SEMANTIC(BumpAllocatorTest);

protected:
    Arena *AllocateArena(size_t size)
    {
        void *mem = ark::os::mem::MapRWAnonymousRaw(size);
        ASAN_UNPOISON_MEMORY_REGION(mem, size);
        std::pair<void *, size_t> newPair {mem, size};
        allocatedMemMmap_.push_back(newPair);
        auto arena = new Arena(size, mem);
        allocatedArenas_.push_back(arena);
        return arena;
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    unsigned seed_ {};

private:
    std::vector<std::pair<void *, size_t>> allocatedMemMmap_;
    std::vector<Arena *> allocatedArenas_;
};

DEATH_TEST_F(BumpAllocatorTest, AlignedAlloc)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    constexpr size_t BUFF_SIZE = SIZE_1M;
    constexpr size_t ARRAY_SIZE = 1024;
    auto pool = PoolManager::GetMmapMemPool()->AllocPool(BUFF_SIZE, SpaceType::SPACE_TYPE_INTERNAL,
                                                         AllocatorType::BUMP_ALLOCATOR);
    mem::MemStatsType memStats;
    NonObjectBumpAllocator<false> bpAllocator(pool, SpaceType::SPACE_TYPE_INTERNAL, &memStats);
    Alignment align = DEFAULT_ALIGNMENT;
    std::array<int *, ARRAY_SIZE> arr {};

    size_t mask = GetAlignmentInBytes(align) - 1;

    // Allocations
    srand(seed_);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        arr[i] = static_cast<int *>(bpAllocator.Alloc(sizeof(int), align));
        // NOLINTNEXTLINE(cert-msc50-cpp)
        *arr[i] = rand() % std::numeric_limits<int>::max();
    }

    // Allocations checking
    srand(seed_);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        ASSERT_NE(arr[i], nullptr) << "value of i: " << i << ", align: " << align << ", seed:" << seed_;
        ASSERT_EQ(reinterpret_cast<size_t>(arr[i]) & mask, static_cast<size_t>(0))
            << "value of i: " << i << ", align: " << align << ", seed:" << seed_;
        // NOLINTNEXTLINE(cert-msc50-cpp)
        ASSERT_EQ(*arr[i], rand() % std::numeric_limits<int>::max())
            << "value of i: " << i << ", align: " << align << ", seed:" << seed_;
    }
    static_assert(LOG_ALIGN_MAX != DEFAULT_ALIGNMENT, "We expect minimal alignment != DEFAULT_ALIGNMENT");
    void *ptr;
#ifndef NDEBUG
    EXPECT_DEATH_IF_SUPPORTED(ptr = bpAllocator.Alloc(sizeof(int), LOG_ALIGN_MAX), "alignment == DEFAULT_ALIGNMENT")
        << ", seed:" << seed_;
#endif
    ptr = bpAllocator.Alloc(SIZE_1M);
    ASSERT_EQ(ptr, nullptr) << "Here Alloc with allocation size = 1 MB should return nullptr"
                            << ", seed:" << seed_;
}

TEST_F(BumpAllocatorTest, CreateTLABAndAlloc)
{
    using AllocType = uint64_t;
    static_assert(sizeof(AllocType) % DEFAULT_ALIGNMENT_IN_BYTES == 0);
    constexpr size_t TLAB_SIZE = SIZE_1M;
    constexpr size_t COMMON_BUFFER_SIZE = SIZE_1M;
    constexpr size_t ALLOC_SIZE = sizeof(AllocType);
    constexpr size_t TLAB_ALLOC_COUNT_SIZE = TLAB_SIZE / ALLOC_SIZE;
    constexpr size_t COMMON_ALLOC_COUNT_SIZE = COMMON_BUFFER_SIZE / ALLOC_SIZE;

    size_t mask = DEFAULT_ALIGNMENT_IN_BYTES - 1;

    std::array<AllocType *, TLAB_ALLOC_COUNT_SIZE> tlabElements {};
    std::array<AllocType *, TLAB_ALLOC_COUNT_SIZE> commonElements {};
    auto pool = PoolManager::GetMmapMemPool()->AllocPool(TLAB_SIZE + COMMON_BUFFER_SIZE, SpaceType::SPACE_TYPE_INTERNAL,
                                                         AllocatorType::BUMP_ALLOCATOR);
    mem::MemStatsType memStats;
    NonObjectBumpAllocator<true> allocator(pool, SpaceType::SPACE_TYPE_OBJECT, &memStats, 1);
    {
        // Allocations in common buffer
        srand(seed_);
        for (size_t i = 0; i < COMMON_ALLOC_COUNT_SIZE; ++i) {
            commonElements[i] = static_cast<AllocType *>(allocator.Alloc(sizeof(AllocType)));
            ASSERT_TRUE(commonElements[i] != nullptr) << ", seed:" << seed_;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            *commonElements[i] = rand() % std::numeric_limits<AllocType>::max();
        }

        TLAB *tlab = allocator.CreateNewTLAB(TLAB_SIZE);
        ASSERT_TRUE(tlab != nullptr) << ", seed:" << seed_;
        ASSERT_TRUE(allocator.CreateNewTLAB(TLAB_SIZE) == nullptr) << ", seed:" << seed_;
        // Allocations in TLAB
        srand(seed_);
        for (size_t i = 0; i < TLAB_ALLOC_COUNT_SIZE; ++i) {
            tlabElements[i] = static_cast<AllocType *>(tlab->Alloc(sizeof(AllocType)));
            ASSERT_TRUE(tlabElements[i] != nullptr) << ", seed:" << seed_;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            *tlabElements[i] = rand() % std::numeric_limits<AllocType>::max();
        }

        // Check that we don't have memory in the buffer:
        ASSERT_TRUE(allocator.Alloc(sizeof(AllocType)) == nullptr);
        ASSERT_TRUE(tlab->Alloc(sizeof(AllocType)) == nullptr);

        // Allocations checking in common buffer
        srand(seed_);
        for (size_t i = 0; i < COMMON_ALLOC_COUNT_SIZE; ++i) {
            ASSERT_NE(commonElements[i], nullptr) << "value of i: " << i << ", seed:" << seed_;
            ASSERT_EQ(reinterpret_cast<size_t>(commonElements[i]) & mask, static_cast<size_t>(0))
                << "value of i: " << i << ", seed:" << seed_;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            ASSERT_EQ(*commonElements[i], rand() % std::numeric_limits<AllocType>::max())
                << "value of i: " << i << ", seed:" << seed_;
        }

        // Allocations checking in TLAB
        srand(seed_);
        for (size_t i = 0; i < TLAB_ALLOC_COUNT_SIZE; ++i) {
            ASSERT_NE(tlabElements[i], nullptr) << "value of i: " << i << ", seed:" << seed_;
            ASSERT_EQ(reinterpret_cast<size_t>(tlabElements[i]) & mask, static_cast<size_t>(0))
                << "value of i: " << i << ", seed:" << seed_;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            ASSERT_EQ(*tlabElements[i], rand() % std::numeric_limits<AllocType>::max())
                << "value of i: " << i << ", seed:" << seed_;
        }
    }
    allocator.Reset();
    {
        TLAB *tlab = allocator.CreateNewTLAB(TLAB_SIZE);
        ASSERT_TRUE(tlab != nullptr) << ", seed:" << seed_;
        ASSERT_TRUE(allocator.CreateNewTLAB(TLAB_SIZE) == nullptr) << ", seed:" << seed_;
        // Allocations in TLAB
        srand(seed_);
        for (size_t i = 0; i < TLAB_ALLOC_COUNT_SIZE; ++i) {
            tlabElements[i] = static_cast<AllocType *>(tlab->Alloc(sizeof(AllocType)));
            ASSERT_TRUE(tlabElements[i] != nullptr) << ", seed:" << seed_;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            *tlabElements[i] = rand() % std::numeric_limits<AllocType>::max();
        }

        // Allocations in common buffer
        srand(seed_);
        for (size_t i = 0; i < COMMON_ALLOC_COUNT_SIZE; ++i) {
            commonElements[i] = static_cast<AllocType *>(allocator.Alloc(sizeof(AllocType)));
            ASSERT_TRUE(commonElements[i] != nullptr) << ", seed:" << seed_;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            *commonElements[i] = rand() % std::numeric_limits<AllocType>::max();
        }

        // Check that we don't have memory in the buffer:
        ASSERT_TRUE(allocator.Alloc(sizeof(AllocType)) == nullptr);
        ASSERT_TRUE(tlab->Alloc(sizeof(AllocType)) == nullptr);

        // Allocations checking in TLAB
        srand(seed_);
        for (size_t i = 0; i < TLAB_ALLOC_COUNT_SIZE; ++i) {
            ASSERT_NE(tlabElements[i], nullptr) << "value of i: " << i << ", seed:" << seed_;
            ASSERT_EQ(reinterpret_cast<size_t>(tlabElements[i]) & mask, static_cast<size_t>(0))
                << "value of i: " << i << ", seed:" << seed_;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            ASSERT_EQ(*tlabElements[i], rand() % std::numeric_limits<AllocType>::max())
                << "value of i: " << i << ", seed:" << seed_;
        }

        // Allocations checking in common buffer
        srand(seed_);
        for (size_t i = 0; i < COMMON_ALLOC_COUNT_SIZE; ++i) {
            ASSERT_NE(commonElements[i], nullptr) << "value of i: " << i << ", seed:" << seed_;
            ASSERT_EQ(reinterpret_cast<size_t>(commonElements[i]) & mask, static_cast<size_t>(0))
                << "value of i: " << i << ", seed:" << seed_;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            ASSERT_EQ(*commonElements[i], rand() % std::numeric_limits<AllocType>::max())
                << "value of i: " << i << ", seed:" << seed_;
        }
    }
}

TEST_F(BumpAllocatorTest, CreateTooManyTLABS)
{
    constexpr size_t TLAB_SIZE = SIZE_1M;
    constexpr size_t TLAB_COUNT = 3;
    auto pool = PoolManager::GetMmapMemPool()->AllocPool(TLAB_SIZE * TLAB_COUNT, SpaceType::SPACE_TYPE_INTERNAL,
                                                         AllocatorType::BUMP_ALLOCATOR);
    mem::MemStatsType memStats;
    NonObjectBumpAllocator<true> allocator(pool, SpaceType::SPACE_TYPE_OBJECT, &memStats, TLAB_COUNT - 1);
    {
        for (size_t i = 0; i < TLAB_COUNT - 1; i++) {
            TLAB *tlab = allocator.CreateNewTLAB(TLAB_SIZE);
            ASSERT_TRUE(tlab != nullptr) << ", seed:" << seed_;
        }
        TLAB *tlab = allocator.CreateNewTLAB(TLAB_SIZE);
        ASSERT_TRUE(tlab == nullptr) << ", seed:" << seed_;
    }
    allocator.Reset();
    {
        for (size_t i = 0; i < TLAB_COUNT - 1; i++) {
            TLAB *tlab = allocator.CreateNewTLAB(TLAB_SIZE);
            ASSERT_TRUE(tlab != nullptr) << ", seed:" << seed_;
        }
        TLAB *tlab = allocator.CreateNewTLAB(TLAB_SIZE);
        ASSERT_TRUE(tlab == nullptr) << ", seed:" << seed_;
    }
}

}  // namespace ark::mem
