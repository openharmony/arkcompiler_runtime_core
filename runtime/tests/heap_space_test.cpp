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

#include <gtest/gtest.h>

#include "libpandabase/utils/logger.h"
#include "libpandabase/mem/mem_config.h"
#include "libpandabase/mem/pool_manager.h"
#include "runtime/mem/heap_space.h"

namespace panda::mem::test {

class HeapSpaceTest : public testing::Test {
public:
    explicit HeapSpaceTest()
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
    };

    ~HeapSpaceTest()
    {
        // Logger::Destroy();
    }

protected:
    static constexpr size_t DEFAULT_TEST_HEAP_SIZE = 64_MB;
    static constexpr size_t DEFAULT_TEST_YOUNG_SIZE = 4_MB;

    size_t GetCurrentMaxSize() const
    {
        return heap_space_->GetCurrentSize();
    }

    size_t GetCurrentYoungMaxSize() const
    {
        return gen_spaces_->GetCurrentMaxYoungSize();
    }

    size_t GetCurrentTenuredMaxSize() const
    {
        return gen_spaces_->GetCurrentSize();
    }

    struct HeapSpaceHolder {
        HeapSpaceHolder(size_t initial_heap_size = DEFAULT_TEST_HEAP_SIZE,
                        size_t max_heap_size = DEFAULT_TEST_HEAP_SIZE, uint32_t min_percentage = 30U,
                        uint32_t max_percentage = 70U)
        {
            MemConfig::Initialize(max_heap_size, 0_MB, 0_MB, 0_MB, initial_heap_size);
            PoolManager::Initialize();
            HeapSpaceTest::heap_space_ = new HeapSpace();
            HeapSpaceTest::heap_space_->Initialize(MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                                                   min_percentage, max_percentage);
        }

        ~HeapSpaceHolder() noexcept
        {
            delete HeapSpaceTest::heap_space_;
            HeapSpaceTest::heap_space_ = nullptr;
            PoolManager::Finalize();
            MemConfig::Finalize();
        }
    };

    struct GenerationalSpacesHolder {
        GenerationalSpacesHolder(size_t init_young_size = DEFAULT_TEST_YOUNG_SIZE,
                                 size_t young_size = DEFAULT_TEST_YOUNG_SIZE,
                                 size_t initial_heap_size = DEFAULT_TEST_HEAP_SIZE,
                                 size_t max_heap_size = DEFAULT_TEST_HEAP_SIZE, uint32_t min_percentage = 0U,
                                 uint32_t max_percentage = PERCENT_100_U32)
        {
            MemConfig::Initialize(max_heap_size, 0_MB, 0_MB, 0_MB, initial_heap_size);
            PoolManager::Initialize();
            HeapSpaceTest::gen_spaces_ = new GenerationalSpaces();
            HeapSpaceTest::gen_spaces_->Initialize(init_young_size, true, young_size, true,
                                                   MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                                                   min_percentage, max_percentage);
        }

        ~GenerationalSpacesHolder() noexcept
        {
            delete HeapSpaceTest::gen_spaces_;
            HeapSpaceTest::gen_spaces_ = nullptr;
            PoolManager::Finalize();
            MemConfig::Finalize();
        }
    };

    static HeapSpace *heap_space_;
    static GenerationalSpaces *gen_spaces_;
};

HeapSpace *HeapSpaceTest::heap_space_ = nullptr;
GenerationalSpaces *HeapSpaceTest::gen_spaces_ = nullptr;

TEST_F(HeapSpaceTest, AllocFreeAndCheckSizesTest)
{
    HeapSpaceHolder hsh;

    auto pool_1 =
        heap_space_->TryAllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool_1, NULLPOOL);
    auto pool_2 =
        heap_space_->TryAllocPool(6_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool_2, NULLPOOL);

    ASSERT_EQ(heap_space_->GetHeapSize(), 10_MB);

    heap_space_->FreePool(pool_1.GetMem(), pool_1.GetSize());
    ASSERT_EQ(heap_space_->GetHeapSize(), 6_MB);
    auto *arena_1 =
        heap_space_->TryAllocArena(6_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(arena_1, nullptr);
    ASSERT_EQ(heap_space_->GetHeapSize(), 12_MB);
    heap_space_->FreePool(pool_2.GetMem(), pool_2.GetSize());
    ASSERT_EQ(heap_space_->GetHeapSize(), 6_MB);
    heap_space_->FreeArena(arena_1);
    ASSERT_EQ(heap_space_->GetHeapSize(), 0_MB);
}

TEST_F(HeapSpaceTest, EmulateAllocBeforeAndAfterSTWGCTest)
{
    static constexpr size_t INIT_HEAP_SIZE = 4_MB;
    static constexpr size_t FIRST_POOL_SIZE = INIT_HEAP_SIZE / 2;
    static constexpr size_t SECOND_POOL_SIZE = INIT_HEAP_SIZE * 3 / 4;
    HeapSpaceHolder hsh(INIT_HEAP_SIZE, 2 * INIT_HEAP_SIZE, 0U, PERCENT_100_U32);

    ASSERT_EQ(GetCurrentMaxSize(), INIT_HEAP_SIZE) << "Current heap limit must be equal initial heap size";
    auto pool_1 = heap_space_->TryAllocPool(FIRST_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                            AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool_1, NULLPOOL);
    // -- Emulate simple GC
    heap_space_->ComputeNewSize();
    ASSERT_EQ(GetCurrentMaxSize(), INIT_HEAP_SIZE);
    // --
    auto pool_2 = heap_space_->TryAllocPool(SECOND_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                            AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool_2, NULLPOOL) << "We now can't allocate pool";
    // -- Emulate simple GC
    heap_space_->ComputeNewSize();
    ASSERT_EQ(GetCurrentMaxSize(), FIRST_POOL_SIZE + SECOND_POOL_SIZE);
    // --
    pool_2 = heap_space_->TryAllocPool(SECOND_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                       AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool_2, NULLPOOL);
    heap_space_->FreePool(pool_1.GetMem(), pool_1.GetSize());
    // -- Emulate simple GC
    heap_space_->ComputeNewSize();
    ASSERT_EQ(GetCurrentMaxSize(), FIRST_POOL_SIZE + SECOND_POOL_SIZE);
    // --
    heap_space_->FreePool(pool_2.GetMem(), pool_2.GetSize());
}

TEST_F(HeapSpaceTest, EmulateAllocBeforeAndDuringGenGCTest)
{
    static constexpr size_t INIT_HEAP_SIZE = 16_MB;
    static constexpr size_t FIRST_POOL_SIZE = 4_MB;
    static constexpr size_t SECOND_POOL_SIZE = 10_MB;
    GenerationalSpacesHolder gsh(DEFAULT_TEST_YOUNG_SIZE, DEFAULT_TEST_YOUNG_SIZE, INIT_HEAP_SIZE, INIT_HEAP_SIZE * 2);
    auto young_pool = gen_spaces_->AllocAlonePoolForYoung(SpaceType::SPACE_TYPE_OBJECT,
                                                          AllocatorType::BUMP_ALLOCATOR_WITH_TLABS, nullptr);
    // Check young pool allocation
    ASSERT_NE(young_pool, NULLPOOL);
    ASSERT_EQ(young_pool.GetSize(), DEFAULT_TEST_YOUNG_SIZE);
    // Check current heap space sizes before "runtime work"
    ASSERT_EQ(GetCurrentYoungMaxSize(), DEFAULT_TEST_YOUNG_SIZE);
    ASSERT_EQ(GetCurrentTenuredMaxSize(), INIT_HEAP_SIZE - DEFAULT_TEST_YOUNG_SIZE);

    auto pool_1 = gen_spaces_->TryAllocPoolForYoung(FIRST_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                    AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool_1, NULLPOOL);
    // -- Emulate simple GC
    gen_spaces_->SetIsWorkGC(true);
    pool_1 = gen_spaces_->TryAllocPoolForTenured(FIRST_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                 AllocatorType::RUNSLOTS_ALLOCATOR, nullptr);
    ASSERT_EQ(pool_1.GetSize(), FIRST_POOL_SIZE);
    gen_spaces_->ComputeNewSize();
    ASSERT_EQ(GetCurrentYoungMaxSize(), DEFAULT_TEST_YOUNG_SIZE);
    ASSERT_EQ(GetCurrentTenuredMaxSize(), INIT_HEAP_SIZE - DEFAULT_TEST_YOUNG_SIZE);
    // --
    // Try too big pool, now no space for such pool
    auto pool_2 = gen_spaces_->TryAllocPoolForTenured(SECOND_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                      AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool_2, NULLPOOL) << "Now no space for such pool, so we must have NULLPOOL";
    // -- Emulate simple GC
    gen_spaces_->SetIsWorkGC(true);
    ASSERT_TRUE(gen_spaces_->CanAllocInSpace(false, SECOND_POOL_SIZE)) << "We can allocate pool during GC";
    pool_2 = gen_spaces_->TryAllocPoolForTenured(SECOND_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                 AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool_2.GetSize(), SECOND_POOL_SIZE) << "We can allocate pool during GC";
    ASSERT_EQ(GetCurrentTenuredMaxSize(), FIRST_POOL_SIZE + SECOND_POOL_SIZE);
    gen_spaces_->ComputeNewSize();
    ASSERT_EQ(GetCurrentTenuredMaxSize(), FIRST_POOL_SIZE + 2 * SECOND_POOL_SIZE);
    // --
    gen_spaces_->FreePool(pool_2.GetMem(), pool_2.GetSize());
    gen_spaces_->FreePool(pool_1.GetMem(), pool_1.GetSize());
    gen_spaces_->FreeYoungPool(young_pool.GetMem(), young_pool.GetSize());
}

TEST_F(HeapSpaceTest, SharedPoolTest)
{
    static constexpr size_t INIT_HEAP_SIZE = 32_MB;
    static constexpr size_t INIT_YOUNG_SIZE = 2_MB;
    static constexpr size_t MAX_YOUNG_SIZE = 8_MB;
    static constexpr size_t SHARED_POOL_SIZE = 8_MB;
    static constexpr size_t REGION_SIZE = 1_MB;
    static constexpr bool IS_YOUNG = true;
    GenerationalSpacesHolder gsh(INIT_YOUNG_SIZE, MAX_YOUNG_SIZE, INIT_HEAP_SIZE);
    auto shared_pool = gen_spaces_->AllocSharedPool(SHARED_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                    AllocatorType::REGION_ALLOCATOR, nullptr);
    ASSERT_EQ(shared_pool.GetSize(), SHARED_POOL_SIZE);
    ASSERT_EQ(GetCurrentYoungMaxSize(), INIT_YOUNG_SIZE);
    ASSERT_EQ(GetCurrentTenuredMaxSize(), INIT_HEAP_SIZE - INIT_YOUNG_SIZE);
    ASSERT_TRUE(gen_spaces_->CanAllocInSpace(IS_YOUNG, INIT_YOUNG_SIZE));
    ASSERT_TRUE(gen_spaces_->CanAllocInSpace(!IS_YOUNG, INIT_YOUNG_SIZE));
    gen_spaces_->IncreaseYoungOccupiedInSharedPool(REGION_SIZE);
    gen_spaces_->IncreaseTenuredOccupiedInSharedPool(REGION_SIZE);
    ASSERT_FALSE(gen_spaces_->CanAllocInSpace(IS_YOUNG, INIT_YOUNG_SIZE));
    ASSERT_TRUE(gen_spaces_->CanAllocInSpace(!IS_YOUNG, INIT_YOUNG_SIZE));
    auto pool_1 = gen_spaces_->TryAllocPoolForTenured(REGION_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                      AllocatorType::REGION_ALLOCATOR, nullptr);
    ASSERT_EQ(pool_1.GetSize(), REGION_SIZE);
    gen_spaces_->ReduceYoungOccupiedInSharedPool(REGION_SIZE);
    ASSERT_TRUE(gen_spaces_->CanAllocInSpace(IS_YOUNG, INIT_YOUNG_SIZE));
    gen_spaces_->ReduceTenuredOccupiedInSharedPool(REGION_SIZE);
    ASSERT_TRUE(gen_spaces_->CanAllocInSpace(IS_YOUNG, INIT_YOUNG_SIZE));
    ASSERT_TRUE(gen_spaces_->CanAllocInSpace(!IS_YOUNG, INIT_YOUNG_SIZE));
    gen_spaces_->FreeTenuredPool(pool_1.GetMem(), pool_1.GetSize());
    gen_spaces_->FreeSharedPool(shared_pool.GetMem(), shared_pool.GetSize());
}

}  // namespace panda::mem::test
