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
#include "mem/mem.h"
#include "mem/mem_pool.h"
#include "mem/mmap_mem_pool-inl.h"

#include "gtest/gtest.h"

namespace panda {

class MMapMemPoolTest : public testing::Test {
public:
    MMapMemPoolTest()
    {
        instance_ = nullptr;
    }

    ~MMapMemPoolTest() override
    {
        delete instance_;
        FinalizeMemConfig();
    }

    NO_MOVE_SEMANTIC(MMapMemPoolTest);
    NO_COPY_SEMANTIC(MMapMemPoolTest);

protected:
    MmapMemPool *CreateMMapMemPool(size_t object_pool_size = 0, size_t internal_size = 0, size_t compiler_size = 0,
                                   size_t code_size = 0, size_t frames_size = 0, size_t stacks_size = 0)
    {
        ASSERT(instance_ == nullptr);
        SetupMemConfig(object_pool_size, internal_size, compiler_size, code_size, frames_size, stacks_size);
        instance_ = new MmapMemPool();
        return instance_;
    }

    void ReturnedToOsTest(OSPagesPolicy first_pool_policy, OSPagesPolicy second_pool_policy,
                          OSPagesPolicy third_pool_policy, bool need_fourth_pool, bool big_pool_realloc)
    {
        static constexpr size_t POOL_COUNT = 3;
        static constexpr size_t MMAP_MEMORY_SIZE = 16_MB;
        static constexpr size_t BIG_POOL_ALLOC_SIZE = 12_MB;
        MmapMemPool *mem_pool = CreateMMapMemPool(MMAP_MEMORY_SIZE);
        std::array<Pool, POOL_COUNT> pools {{NULLPOOL, NULLPOOL, NULLPOOL}};
        Pool fourth_pool = NULLPOOL;

        for (size_t i = 0; i < POOL_COUNT; i++) {
            pools[i] = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
            ASSERT_TRUE(pools[i].GetMem() != nullptr);
            FillMemory(pools[i].GetMem(), pools[i].GetSize());
        }
        if (need_fourth_pool) {
            fourth_pool = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
            ASSERT_TRUE(fourth_pool.GetMem() != nullptr);
            FillMemory(fourth_pool.GetMem(), fourth_pool.GetSize());
        }

        if (first_pool_policy == OSPagesPolicy::NO_RETURN) {
            mem_pool->template FreePool<OSPagesPolicy::NO_RETURN>(pools[0].GetMem(), pools[0].GetSize());
        } else {
            mem_pool->template FreePool<OSPagesPolicy::IMMEDIATE_RETURN>(pools[0].GetMem(), pools[0].GetSize());
        }
        if (third_pool_policy == OSPagesPolicy::NO_RETURN) {
            mem_pool->template FreePool<OSPagesPolicy::NO_RETURN>(pools[2].GetMem(), pools[2].GetSize());
        } else {
            mem_pool->template FreePool<OSPagesPolicy::IMMEDIATE_RETURN>(pools[2].GetMem(), pools[2].GetSize());
        }
        if (second_pool_policy == OSPagesPolicy::NO_RETURN) {
            mem_pool->template FreePool<OSPagesPolicy::NO_RETURN>(pools[1].GetMem(), pools[1].GetSize());
        } else {
            mem_pool->template FreePool<OSPagesPolicy::IMMEDIATE_RETURN>(pools[1].GetMem(), pools[1].GetSize());
        }

        if (big_pool_realloc) {
            auto final_pool = mem_pool->template AllocPool<OSPagesAllocPolicy::ZEROED_MEMORY>(
                BIG_POOL_ALLOC_SIZE, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
            ASSERT_TRUE(final_pool.GetMem() != nullptr);
            IsZeroMemory(final_pool.GetMem(), final_pool.GetSize());
            mem_pool->FreePool(final_pool.GetMem(), final_pool.GetSize());
        } else {
            // reallocate again
            for (size_t i = 0; i < POOL_COUNT; i++) {
                pools[i] = mem_pool->template AllocPool<OSPagesAllocPolicy::ZEROED_MEMORY>(
                    4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
                ASSERT_TRUE(pools[i].GetMem() != nullptr);
                IsZeroMemory(pools[i].GetMem(), pools[i].GetSize());
            }
            for (size_t i = 0; i < POOL_COUNT; i++) {
                mem_pool->FreePool(pools[i].GetMem(), pools[i].GetSize());
            }
        }
        if (need_fourth_pool) {
            mem_pool->FreePool(fourth_pool.GetMem(), fourth_pool.GetSize());
        }
        DeleteMMapMemPool(mem_pool);
    }

private:
    void SetupMemConfig(size_t object_pool_size, size_t internal_size, size_t compiler_size, size_t code_size,
                        size_t frames_size, size_t stacks_size)
    {
        mem::MemConfig::Initialize(object_pool_size, internal_size, compiler_size, code_size, frames_size, stacks_size);
    }

    void FinalizeMemConfig()
    {
        mem::MemConfig::Finalize();
    }

    void DeleteMMapMemPool(MmapMemPool *mem_pool)
    {
        ASSERT(instance_ == mem_pool);
        delete mem_pool;
        FinalizeMemConfig();
        instance_ = nullptr;
    }

    void FillMemory(void *start, size_t size)
    {
        size_t it_end = size / sizeof(uint64_t);
        auto *pointer = static_cast<uint64_t *>(start);
        for (size_t i = 0; i < it_end; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            pointer[i] = MAGIC_VALUE;
        }
    }

    bool IsZeroMemory(void *start, size_t size)
    {
        size_t it_end = size / sizeof(uint64_t);
        auto *pointer = static_cast<uint64_t *>(start);
        for (size_t i = 0; i < it_end; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (pointer[i] != 0) {
                return false;
            }
        }
        return true;
    }

    static constexpr uint64_t MAGIC_VALUE = 0xDEADBEEF;

    MmapMemPool *instance_;
};

TEST_F(MMapMemPoolTest, HeapOOMTest)
{
    MmapMemPool *mem_pool = CreateMMapMemPool(4_MB);

    auto pool1 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool1.GetMem() != nullptr);
    auto pool2 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool2.GetMem() == nullptr);
    auto pool3 =
        mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool3.GetMem() == nullptr);
    auto pool4 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool4.GetMem() == nullptr);

    mem_pool->FreePool(pool1.GetMem(), pool1.GetSize());
}

TEST_F(MMapMemPoolTest, HeapOOMAndAllocInOtherSpacesTest)
{
    MmapMemPool *mem_pool = CreateMMapMemPool(4_MB, 1_MB, 1_MB, 1_MB, 1_MB, 1_MB);

    auto pool1 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool1.GetMem() != nullptr);
    auto pool2 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool2.GetMem() == nullptr);
    auto pool3 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_COMPILER, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool3.GetMem() != nullptr);
    auto pool4 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_CODE, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool4.GetMem() != nullptr);
    auto pool5 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_INTERNAL, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool5.GetMem() != nullptr);
    auto pool6 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_FRAMES, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool6.GetMem() != nullptr);
    auto pool7 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_NATIVE_STACKS, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool7.GetMem() != nullptr);

    // cleaning
    mem_pool->FreePool(pool1.GetMem(), pool1.GetSize());
    mem_pool->FreePool(pool3.GetMem(), pool3.GetSize());
    mem_pool->FreePool(pool4.GetMem(), pool4.GetSize());
    mem_pool->FreePool(pool5.GetMem(), pool5.GetSize());
    mem_pool->FreePool(pool6.GetMem(), pool6.GetSize());
    mem_pool->FreePool(pool7.GetMem(), pool7.GetSize());
}

TEST_F(MMapMemPoolTest, GetAllocatorInfoTest)
{
    static constexpr AllocatorType ALLOC_TYPE = AllocatorType::BUMP_ALLOCATOR;
    static constexpr size_t POOL_SIZE = 4_MB;
    static constexpr size_t POINTER_POOL_OFFSET = 1_MB;
    ASSERT_TRUE(POINTER_POOL_OFFSET < POOL_SIZE);
    MmapMemPool *mem_pool = CreateMMapMemPool(POOL_SIZE * 2, 0, 0, 0);
    int *allocator_addr = new int();
    Pool pool_with_alloc_addr =
        mem_pool->AllocPool(POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT, ALLOC_TYPE, allocator_addr);
    Pool pool_without_alloc_addr = mem_pool->AllocPool(POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT, ALLOC_TYPE);
    ASSERT_TRUE(pool_with_alloc_addr.GetMem() != nullptr);
    ASSERT_TRUE(pool_without_alloc_addr.GetMem() != nullptr);

    void *first_pool_pointer = ToVoidPtr(ToUintPtr(pool_with_alloc_addr.GetMem()) + POINTER_POOL_OFFSET);
    ASSERT_TRUE(ToUintPtr(mem_pool->GetAllocatorInfoForAddr(first_pool_pointer).GetAllocatorHeaderAddr()) ==
                ToUintPtr(allocator_addr));
    ASSERT_TRUE(mem_pool->GetAllocatorInfoForAddr(first_pool_pointer).GetType() == ALLOC_TYPE);
    ASSERT_TRUE(ToUintPtr(mem_pool->GetStartAddrPoolForAddr(first_pool_pointer)) ==
                ToUintPtr(pool_with_alloc_addr.GetMem()));

    void *second_pool_pointer = ToVoidPtr(ToUintPtr(pool_without_alloc_addr.GetMem()) + POINTER_POOL_OFFSET);
    ASSERT_TRUE(ToUintPtr(mem_pool->GetAllocatorInfoForAddr(second_pool_pointer).GetAllocatorHeaderAddr()) ==
                ToUintPtr(pool_without_alloc_addr.GetMem()));
    ASSERT_TRUE(mem_pool->GetAllocatorInfoForAddr(second_pool_pointer).GetType() == ALLOC_TYPE);
    ASSERT_TRUE(ToUintPtr(mem_pool->GetStartAddrPoolForAddr(second_pool_pointer)) ==
                ToUintPtr(pool_without_alloc_addr.GetMem()));

    // cleaning
    mem_pool->FreePool(pool_with_alloc_addr.GetMem(), pool_with_alloc_addr.GetSize());
    mem_pool->FreePool(pool_without_alloc_addr.GetMem(), pool_without_alloc_addr.GetSize());

    delete allocator_addr;
}

TEST_F(MMapMemPoolTest, CheckLimitsForInternalSpacesTest)
{
#ifndef PANDA_TARGET_32
    MmapMemPool *mem_pool = CreateMMapMemPool(1_GB, 1_GB, 1_GB, 1_GB, 1_GB, 1_GB);
    Pool object_pool = mem_pool->AllocPool(1_GB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::BUMP_ALLOCATOR);
    Pool internal_pool = mem_pool->AllocPool(1_GB, SpaceType::SPACE_TYPE_COMPILER, AllocatorType::BUMP_ALLOCATOR);
    Pool code_pool = mem_pool->AllocPool(1_GB, SpaceType::SPACE_TYPE_CODE, AllocatorType::BUMP_ALLOCATOR);
    Pool compiler_pool = mem_pool->AllocPool(1_GB, SpaceType::SPACE_TYPE_INTERNAL, AllocatorType::BUMP_ALLOCATOR);
    Pool frames_pool = mem_pool->AllocPool(1_GB, SpaceType::SPACE_TYPE_FRAMES, AllocatorType::BUMP_ALLOCATOR);
    Pool stacks_pool = mem_pool->AllocPool(1_GB, SpaceType::SPACE_TYPE_NATIVE_STACKS, AllocatorType::BUMP_ALLOCATOR);
    // Check that these pools has been created successfully
    ASSERT_TRUE(object_pool.GetMem() != nullptr);
    ASSERT_TRUE(internal_pool.GetMem() != nullptr);
    ASSERT_TRUE(code_pool.GetMem() != nullptr);
    ASSERT_TRUE(compiler_pool.GetMem() != nullptr);
    ASSERT_TRUE(frames_pool.GetMem() != nullptr);
    ASSERT_TRUE(stacks_pool.GetMem() != nullptr);
    // Check that part of internal pools located in 64 bits address space
    bool internal =
        (ToUintPtr(internal_pool.GetMem()) + internal_pool.GetSize() - 1U) > std::numeric_limits<uint32_t>::max();
    bool code = (ToUintPtr(code_pool.GetMem()) + code_pool.GetSize() - 1U) > std::numeric_limits<uint32_t>::max();
    bool compiler =
        (ToUintPtr(compiler_pool.GetMem()) + compiler_pool.GetSize() - 1U) > std::numeric_limits<uint32_t>::max();
    bool frame = (ToUintPtr(frames_pool.GetMem()) + frames_pool.GetSize() - 1U) > std::numeric_limits<uint32_t>::max();
    bool stacks = (ToUintPtr(stacks_pool.GetMem()) + stacks_pool.GetSize() - 1U) > std::numeric_limits<uint32_t>::max();
    ASSERT_TRUE(internal || code || compiler || frame || stacks);

    // cleaning
    mem_pool->FreePool(object_pool.GetMem(), object_pool.GetSize());
    mem_pool->FreePool(internal_pool.GetMem(), internal_pool.GetSize());
    mem_pool->FreePool(code_pool.GetMem(), code_pool.GetSize());
    mem_pool->FreePool(compiler_pool.GetMem(), compiler_pool.GetSize());
    mem_pool->FreePool(frames_pool.GetMem(), frames_pool.GetSize());
    mem_pool->FreePool(stacks_pool.GetMem(), stacks_pool.GetSize());
#endif
}

TEST_F(MMapMemPoolTest, PoolReturnTest)
{
    MmapMemPool *mem_pool = CreateMMapMemPool(8_MB);

    auto pool1 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool1.GetMem() != nullptr);
    auto pool2 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool2.GetMem() != nullptr);
    auto pool3 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool3.GetMem() == nullptr);
    mem_pool->FreePool(pool1.GetMem(), pool1.GetSize());
    mem_pool->FreePool(pool2.GetMem(), pool2.GetSize());

    auto pool4 = mem_pool->AllocPool(6_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool4.GetMem() != nullptr);
    auto pool5 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool5.GetMem() != nullptr);
    auto pool6 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool6.GetMem() != nullptr);
    mem_pool->FreePool(pool6.GetMem(), pool6.GetSize());
    mem_pool->FreePool(pool4.GetMem(), pool4.GetSize());
    mem_pool->FreePool(pool5.GetMem(), pool5.GetSize());

    auto pool7 = mem_pool->AllocPool(8_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool7.GetMem() != nullptr);
    mem_pool->FreePool(pool7.GetMem(), pool7.GetSize());
}

TEST_F(MMapMemPoolTest, CheckEnoughPoolsTest)
{
    static constexpr size_t MMAP_MEM_POOL_SIZE = 10_MB;
    static constexpr size_t POOL_SIZE = 4_MB;
    MmapMemPool *mem_pool = CreateMMapMemPool(MMAP_MEM_POOL_SIZE);

    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));
    auto pool1 = mem_pool->AllocPool(3_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(1, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    auto pool2 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    mem_pool->FreePool(pool1.GetMem(), pool1.GetSize());
    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(1, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    mem_pool->FreePool(pool2.GetMem(), pool2.GetSize());
    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));

    auto pool3 = mem_pool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    auto pool4 = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(1, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    mem_pool->FreePool(pool3.GetMem(), pool3.GetSize());
    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));
    auto pool5 = mem_pool->AllocPool(5_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(1, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    mem_pool->FreePool(pool5.GetMem(), pool5.GetSize());
    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));
    mem_pool->FreePool(pool4.GetMem(), pool4.GetSize());
    ASSERT_TRUE(mem_pool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(mem_pool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));
}

TEST_F(MMapMemPoolTest, TestMergeWithFreeSpace)
{
    static constexpr size_t MMAP_MEM_POOL_SIZE = 10_MB;
    MmapMemPool *mem_pool = CreateMMapMemPool(MMAP_MEM_POOL_SIZE);
    auto pool = mem_pool->AllocPool(7_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    mem_pool->FreePool(pool.GetMem(), pool.GetSize());
    pool = mem_pool->AllocPool(8_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool.GetMem() != nullptr);
    ASSERT_EQ(8_MB, pool.GetSize());
    mem_pool->FreePool(pool.GetMem(), pool.GetSize());
}

TEST_F(MMapMemPoolTest, ReturnedToOsPlusZeroingMemoryTest)
{
    for (OSPagesPolicy a : {OSPagesPolicy::NO_RETURN, OSPagesPolicy::IMMEDIATE_RETURN}) {
        for (OSPagesPolicy b : {OSPagesPolicy::NO_RETURN, OSPagesPolicy::IMMEDIATE_RETURN}) {
            for (OSPagesPolicy c : {OSPagesPolicy::NO_RETURN, OSPagesPolicy::IMMEDIATE_RETURN}) {
                for (bool d : {false, true}) {
                    for (bool e : {false, true}) {
                        ReturnedToOsTest(a, b, c, d, e);
                    }
                }
            }
        }
    }
}

}  // namespace panda
