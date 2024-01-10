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
#include "libpandabase/utils/utils.h"
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

    CollectionSet CreateCollectionSet(size_t edenLength, size_t oldLength = 0)
    {
        ASSERT(edenLength + oldLength < ALL_REGIONS_NUM);
        PandaVector<Region *> vector;
        auto it = regions_.begin();
        for (size_t i = 0; i < edenLength; ++it, ++i) {
            vector.push_back(&*it);
        }
        auto cs = CollectionSet(vector);
        for (size_t i = 0; i < oldLength; ++it, ++i) {
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
    uint64_t now = 1686312829587000000U;
    G1Analytics analytics(now - 20'000'000U);
    ASSERT_EQ(0, analytics.PredictYoungCollectionTimeInMicros(16U));

    auto collectionSet = CreateCollectionSet(16U);
    analytics.ReportCollectionStart(now);
    analytics.ReportMarkingStart(now + 1'000'000U);
    analytics.ReportMarkingEnd(now + 2'000'000U);
    analytics.ReportEvacuationStart(now + 3'000'000U);
    analytics.ReportEvacuationEnd(now + 6'000'000U);
    analytics.ReportCollectionEnd(now + 10'000'000U, collectionSet);

    ASSERT_EQ(0, analytics.PredictYoungCollectionTimeInMicros(16U));
}

TEST_F(G1AnalyticsTest, AllPromotedUndefinedBehaviorTest)
{
    uint64_t now = 1686312829587000000U;
    G1Analytics analytics(now - 20'000'000U);
    ASSERT_EQ(0, analytics.PredictYoungCollectionTimeInMicros(16U));

    auto collectionSet = CreateCollectionSet(16U);
    analytics.ReportCollectionStart(now);
    analytics.ReportMarkingStart(now + 1'000'000U);
    analytics.ReportMarkingEnd(now + 2'000'000U);
    analytics.ReportEvacuationStart(now + 3'000'000U);
    analytics.ReportEvacuationEnd(now + 6'000'000U);
    for (size_t i = 0; i < 16U; i++) {
        analytics.ReportPromotedRegion();
    }
    analytics.ReportCollectionEnd(now + 10'000'000U, collectionSet);

    ASSERT_EQ(800U, analytics.PredictYoungCollectionTimeInMicros(16U));

    now += 20'000'000U;

    analytics.ReportCollectionStart(now);
    analytics.ReportLiveObjects(32000U);
    analytics.ReportMarkingStart(now + 1'000'000U);
    analytics.ReportMarkingEnd(now + 2'000'000U);
    analytics.ReportEvacuationStart(now + 3'000'000U);
    analytics.ReportEvacuationEnd(now + 6'000'000U);
    analytics.ReportCollectionEnd(now + 10'000'000U, collectionSet);

    ASSERT_EQ(833L, analytics.PredictYoungCollectionTimeInMicros(16U));
}

TEST_F(G1AnalyticsTest, PredictionTest)
{
    uint64_t now = 1686312829587000000U;
    G1Analytics analytics(now - 20'000'000U);

    auto collectionSet = CreateCollectionSet(16U);
    analytics.ReportCollectionStart(now);
    analytics.ReportScanRemsetStart(now + 500'000U);
    analytics.ReportScanRemsetEnd(now + 900'000U);
    analytics.ReportMarkingStart(now + 1'000'000U);
    analytics.ReportMarkingEnd(now + 2'000'000U);
    analytics.ReportEvacuationStart(now + 3'000'000U);
    analytics.ReportEvacuationEnd(now + 6'000'000U);
    analytics.ReportEvacuatedBytes(2_MB);
    analytics.ReportLiveObjects(32000U);
    analytics.ReportUpdateRefsStart(now + 6'000'000U);
    analytics.ReportUpdateRefsEnd(now + 7'000'000U);
    analytics.ReportCollectionEnd(now + 10'000'000U, collectionSet);

    ASSERT_EQ(5'000L, analytics.EstimatePredictionErrorInMicros());
    ASSERT_EQ(5'400L, analytics.PredictYoungCollectionTimeInMicros(16U));
    ASSERT_NEAR(0.0008_D, analytics.PredictAllocationRate(), 1e-6_D);

    now += 20'000'000U;

    collectionSet = CreateCollectionSet(10U);
    analytics.ReportCollectionStart(now);
    analytics.ReportScanRemsetStart(now + 500'000U);
    analytics.ReportScanRemsetEnd(now + 900'000U);
    analytics.ReportMarkingStart(now + 1'000'000U);
    analytics.ReportMarkingEnd(now + 2'500'000U);
    analytics.ReportEvacuationStart(now + 3'000'000U);
    analytics.ReportEvacuationEnd(now + 6'500'000U);
    analytics.ReportEvacuatedBytes(2_MB);
    analytics.ReportPromotedRegion();
    analytics.ReportPromotedRegion();
    analytics.ReportLiveObjects(30000U);
    analytics.ReportUpdateRefsStart(now + 6'000'000U);
    analytics.ReportUpdateRefsEnd(now + 7'500'000U);
    analytics.ReportCollectionEnd(now + 10'000'000U, collectionSet);

    ASSERT_EQ(5'005L, analytics.EstimatePredictionErrorInMicros());
    ASSERT_EQ(5'069L, analytics.PredictYoungCollectionTimeInMicros(10U));
    ASSERT_NEAR(0.000905_D, analytics.PredictAllocationRate(), 1e-6_D);

    now += 20'000'000U;

    collectionSet = CreateCollectionSet(14U, 10U);
    analytics.ReportCollectionStart(now);
    analytics.ReportScanRemsetStart(now + 500'000U);
    analytics.ReportScanRemsetEnd(now + 900'000U);
    analytics.ReportMarkingStart(now + 1'000'000U);
    analytics.ReportMarkingEnd(now + 3'000'000U);
    analytics.ReportEvacuationStart(now + 3'000'000U);
    analytics.ReportEvacuationEnd(now + 7'000'000U);
    analytics.ReportEvacuatedBytes(2_MB);
    analytics.ReportPromotedRegion();
    analytics.ReportLiveObjects(33000U);
    analytics.ReportUpdateRefsStart(now + 7'000'000U);
    analytics.ReportUpdateRefsEnd(now + 8'500'000U);
    analytics.ReportCollectionEnd(now + 11'000'000U, collectionSet);

    ASSERT_EQ(5'005L, analytics.EstimatePredictionErrorInMicros());
    ASSERT_EQ(6'526L, analytics.PredictYoungCollectionTimeInMicros(14U));
    ASSERT_EQ(127L, analytics.PredictOldCollectionTimeInMicros(64U * 1024U, 100U));
    ASSERT_NEAR(0.001151_D, analytics.PredictAllocationRate(), 1e-6_D);
}
// NOLINTEND(readability-magic-numbers)
}  // namespace panda::mem
