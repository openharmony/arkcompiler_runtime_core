/*
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

#include <iterator>
#include <gtest/gtest.h>

#include "libpandabase/mem/mem.h"
#include "runtime/mem/gc/g1/collection_set.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/mem/mem_stats_additional_info.h"

namespace panda::mem {

class CollectionSetTest : public testing::Test {
public:
    CollectionSetTest()
    {
        static constexpr size_t MEMORY_POOL_SIZE = 16_MB;
        MemConfig::Initialize(0, MEMORY_POOL_SIZE, 0, 0);
        PoolManager::Initialize();
        mem_stats_ = new mem::MemStatsType();
        allocator_ = new InternalAllocatorT<mem::InternalAllocatorConfig::PANDA_ALLOCATORS>(mem_stats_);
        // mem::InternalAllocatorPtr allocator_ptr(allocator_);
        mem::InternalAllocator<>::InitInternalAllocatorFromRuntime(static_cast<Allocator *>(allocator_));
    }

    ~CollectionSetTest()
    {
        delete allocator_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
        delete mem_stats_;
        InternalAllocator<>::ClearInternalAllocatorFromRuntime();
    }

private:
    MemStatsType *mem_stats_;
    InternalAllocatorT<mem::InternalAllocatorConfig::PANDA_ALLOCATORS> *allocator_;
};

TEST_F(CollectionSetTest, TestCtor)
{
    Region young_region(nullptr, 0x0, 0x1000);
    young_region.AddFlag(RegionFlag::IS_EDEN);
    PandaVector<Region *> young_regions = {&young_region};

    CollectionSet cs(std::move(young_regions));

    ASSERT_EQ(1U, cs.size());
    auto young = cs.Young();
    auto tenured = cs.Tenured();
    auto humongous = cs.Humongous();
    ASSERT_EQ(1U, std::distance(young.begin(), young.end()));
    ASSERT_EQ(&young_region, *young.begin());
    ASSERT_EQ(0U, std::distance(tenured.begin(), tenured.end()));
    ASSERT_EQ(0U, std::distance(humongous.begin(), humongous.end()));
}

TEST_F(CollectionSetTest, TestAddTenuredRegion)
{
    Region young_region(nullptr, 0x0, 0x1000);
    young_region.AddFlag(RegionFlag::IS_EDEN);
    PandaVector<Region *> young_regions = {&young_region};
    Region tenured_region(nullptr, 0x1000, 0x2000);
    tenured_region.AddFlag(RegionFlag::IS_OLD);

    CollectionSet cs(std::move(young_regions));
    cs.AddRegion(&tenured_region);

    ASSERT_EQ(2U, cs.size());
    auto young = cs.Young();
    auto tenured = cs.Tenured();
    auto humongous = cs.Humongous();
    ASSERT_EQ(1U, std::distance(young.begin(), young.end()));
    ASSERT_EQ(&young_region, *young.begin());
    ASSERT_EQ(1U, std::distance(tenured.begin(), tenured.end()));
    ASSERT_EQ(&tenured_region, *tenured.begin());
    ASSERT_EQ(0U, std::distance(humongous.begin(), humongous.end()));
}

TEST_F(CollectionSetTest, TestAddHumongousRegion)
{
    Region young_region(nullptr, 0x0, 0x1000);
    young_region.AddFlag(RegionFlag::IS_EDEN);
    PandaVector<Region *> young_regions = {&young_region};
    Region humongous_region(nullptr, 0x1000, 0x2000);
    humongous_region.AddFlag(RegionFlag::IS_OLD);
    humongous_region.AddFlag(RegionFlag::IS_LARGE_OBJECT);

    CollectionSet cs(std::move(young_regions));
    cs.AddRegion(&humongous_region);

    ASSERT_EQ(2U, cs.size());
    auto young = cs.Young();
    auto tenured = cs.Tenured();
    auto humongous = cs.Humongous();
    ASSERT_EQ(1U, std::distance(young.begin(), young.end()));
    ASSERT_EQ(&young_region, *young.begin());
    ASSERT_EQ(0U, std::distance(tenured.begin(), tenured.end()));
    ASSERT_EQ(1U, std::distance(humongous.begin(), humongous.end()));
    ASSERT_EQ(&humongous_region, *humongous.begin());
}

TEST_F(CollectionSetTest, TestAddDifferentRegions)
{
    Region young_region(nullptr, 0x0, 0x1000);
    young_region.AddFlag(RegionFlag::IS_EDEN);
    PandaVector<Region *> young_regions = {&young_region};
    Region tenured1_region(nullptr, 0x1000, 0x2000);
    tenured1_region.AddFlag(RegionFlag::IS_OLD);
    Region tenured2_region(nullptr, 0x1000, 0x2000);
    tenured2_region.AddFlag(RegionFlag::IS_OLD);
    Region humongous1_region(nullptr, 0x2000, 0x3000);
    humongous1_region.AddFlag(RegionFlag::IS_OLD);
    humongous1_region.AddFlag(RegionFlag::IS_LARGE_OBJECT);
    Region humongous2_region(nullptr, 0x2000, 0x3000);
    humongous2_region.AddFlag(RegionFlag::IS_OLD);
    humongous2_region.AddFlag(RegionFlag::IS_LARGE_OBJECT);

    CollectionSet cs(std::move(young_regions));
    cs.AddRegion(&humongous1_region);
    cs.AddRegion(&tenured1_region);
    cs.AddRegion(&humongous2_region);
    cs.AddRegion(&tenured2_region);

    ASSERT_EQ(5U, cs.size());
    auto young = cs.Young();
    auto tenured = cs.Tenured();
    auto humongous = cs.Humongous();
    ASSERT_EQ(1U, std::distance(young.begin(), young.end()));
    ASSERT_EQ(&young_region, *young.begin());
    ASSERT_EQ(2U, std::distance(tenured.begin(), tenured.end()));
    ASSERT_EQ(2U, std::distance(humongous.begin(), humongous.end()));
    ASSERT_EQ(5U, std::distance(cs.begin(), cs.end()));
    auto it = cs.begin();
    // one young region
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_EDEN));
    // two tenured regions
    ++it;
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_FALSE((*it)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ++it;
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_FALSE((*it)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    // two humongous regions
    ++it;
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ++it;
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
}

}  // namespace panda::mem
