/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "runtime/mem/gc/g1/g1_analytics.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"

namespace panda::mem {
// NOLINTBEGIN(readability-magic-numbers)
class G1AnalyticsTest : public testing::Test {
public:
    G1AnalyticsTest()
    {
        RuntimeOptions options;
        options.SetHeapSizeLimit(64_MB);
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
        for (size_t i = 0; i < ALL_REGIONS_NUM; i++) {
            regions_.emplace_back(nullptr, 0, 0);
        }
    }

    ~G1AnalyticsTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(G1AnalyticsTest);
    NO_MOVE_SEMANTIC(G1AnalyticsTest);

    CollectionSet CreateCollectionSet(size_t eden_length, size_t old_length = 0)
    {
        ASSERT(eden_length + old_length < ALL_REGIONS_NUM);
        PandaVector<Region *> vector;
        auto it = regions_.begin();
        for (size_t i = 0; i < eden_length; ++it, ++i) {
            vector.push_back(&*it);
        }
        auto cs = CollectionSet(vector);
        for (size_t i = 0; i < old_length; ++it, ++i) {
            auto &region = *it;
            region.AddFlag(RegionFlag::IS_OLD);
            cs.AddRegion(&region);
        }
        return cs;
    }

private:
    static constexpr size_t ALL_REGIONS_NUM = 32;
    std::list<Region> regions_;
};

TEST_F(G1AnalyticsTest, UndefinedBehaviorTest)
{
    uint64_t now = 1686312829587000000;
    G1Analytics analytics(now - 20'000'000);
    ASSERT_EQ(0, analytics.PredictYoungCollectionTimeInMicros(16));

    auto collection_set = CreateCollectionSet(16);
    analytics.ReportCollectionStart(now);
    analytics.ReportMarkingStart(now + 1'000'000);
    analytics.ReportMarkingEnd(now + 2'000'000);
    analytics.ReportEvacuationStart(now + 3'000'000);
    analytics.ReportEvacuationEnd(now + 6'000'000);
    analytics.ReportCollectionEnd(now + 10'000'000, collection_set);

    ASSERT_EQ(0, analytics.PredictYoungCollectionTimeInMicros(16));
}

TEST_F(G1AnalyticsTest, AllPromotedUndefinedBehaviorTest)
{
    uint64_t now = 1686312829587000000;
    G1Analytics analytics(now - 20'000'000);
    ASSERT_EQ(0, analytics.PredictYoungCollectionTimeInMicros(16));

    auto collection_set = CreateCollectionSet(16);
    analytics.ReportCollectionStart(now);
    analytics.ReportMarkingStart(now + 1'000'000);
    analytics.ReportMarkingEnd(now + 2'000'000);
    analytics.ReportEvacuationStart(now + 3'000'000);
    analytics.ReportEvacuationEnd(now + 6'000'000);
    for (int i = 0; i < 16; i++) {
        analytics.ReportPromotedRegion();
    }
    analytics.ReportCollectionEnd(now + 10'000'000, collection_set);

    ASSERT_EQ(800, analytics.PredictYoungCollectionTimeInMicros(16));

    now += 20'000'000;

    analytics.ReportCollectionStart(now);
    analytics.ReportLiveObjects(32000);
    analytics.ReportMarkingStart(now + 1'000'000);
    analytics.ReportMarkingEnd(now + 2'000'000);
    analytics.ReportEvacuationStart(now + 3'000'000);
    analytics.ReportEvacuationEnd(now + 6'000'000);
    analytics.ReportCollectionEnd(now + 10'000'000, collection_set);

    ASSERT_EQ(833, analytics.PredictYoungCollectionTimeInMicros(16));
}

TEST_F(G1AnalyticsTest, PredictionTest)
{
    uint64_t now = 1686312829587000000;
    G1Analytics analytics(now - 20'000'000);

    auto collection_set = CreateCollectionSet(16);
    analytics.ReportCollectionStart(now);
    analytics.ReportScanRemsetStart(now + 500'000);
    analytics.ReportScanRemsetEnd(now + 900'000);
    analytics.ReportMarkingStart(now + 1'000'000);
    analytics.ReportMarkingEnd(now + 2'000'000);
    analytics.ReportEvacuationStart(now + 3'000'000);
    analytics.ReportEvacuationEnd(now + 6'000'000);
    analytics.ReportEvacuatedBytes(2 * 1024 * 1024);
    analytics.ReportLiveObjects(32000);
    analytics.ReportUpdateRefsStart(now + 6'000'000);
    analytics.ReportUpdateRefsEnd(now + 7'000'000);
    analytics.ReportCollectionEnd(now + 10'000'000, collection_set);

    ASSERT_EQ(5'000, analytics.EstimatePredictionErrorInMicros());
    ASSERT_EQ(5'400, analytics.PredictYoungCollectionTimeInMicros(16));
    ASSERT_NEAR(0.0008, analytics.PredictAllocationRate(), 1e-6);

    now += 20'000'000;

    collection_set = CreateCollectionSet(10);
    analytics.ReportCollectionStart(now);
    analytics.ReportScanRemsetStart(now + 500'000);
    analytics.ReportScanRemsetEnd(now + 900'000);
    analytics.ReportMarkingStart(now + 1'000'000);
    analytics.ReportMarkingEnd(now + 2'500'000);
    analytics.ReportEvacuationStart(now + 3'000'000);
    analytics.ReportEvacuationEnd(now + 6'500'000);
    analytics.ReportEvacuatedBytes(2 * 1024 * 1024);
    analytics.ReportPromotedRegion();
    analytics.ReportPromotedRegion();
    analytics.ReportLiveObjects(30000);
    analytics.ReportUpdateRefsStart(now + 6'000'000);
    analytics.ReportUpdateRefsEnd(now + 7'500'000);
    analytics.ReportCollectionEnd(now + 10'000'000, collection_set);

    ASSERT_EQ(5'005, analytics.EstimatePredictionErrorInMicros());
    ASSERT_EQ(5'069, analytics.PredictYoungCollectionTimeInMicros(10));
    ASSERT_NEAR(0.000905, analytics.PredictAllocationRate(), 1e-6);

    now += 20'000'000;

    collection_set = CreateCollectionSet(14, 10);
    analytics.ReportCollectionStart(now);
    analytics.ReportScanRemsetStart(now + 500'000);
    analytics.ReportScanRemsetEnd(now + 900'000);
    analytics.ReportMarkingStart(now + 1'000'000);
    analytics.ReportMarkingEnd(now + 3'000'000);
    analytics.ReportEvacuationStart(now + 3'000'000);
    analytics.ReportEvacuationEnd(now + 7'000'000);
    analytics.ReportEvacuatedBytes(2 * 1024 * 1024);
    analytics.ReportPromotedRegion();
    analytics.ReportLiveObjects(33000);
    analytics.ReportUpdateRefsStart(now + 7'000'000);
    analytics.ReportUpdateRefsEnd(now + 8'500'000);
    analytics.ReportCollectionEnd(now + 11'000'000, collection_set);

    ASSERT_EQ(5'005, analytics.EstimatePredictionErrorInMicros());
    ASSERT_EQ(6'526, analytics.PredictYoungCollectionTimeInMicros(14));
    ASSERT_EQ(127, analytics.PredictOldCollectionTimeInMicros(64 * 1024, 100));
    ASSERT_NEAR(0.001151, analytics.PredictAllocationRate(), 1e-6);
}
// NOLINTEND(readability-magic-numbers)
}  // namespace panda::mem
