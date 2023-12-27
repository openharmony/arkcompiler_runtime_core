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

#include "g1_analytics.h"
#include "libpandabase/utils/time.h"
#include "libpandabase/os/time.h"

namespace panda::mem {
G1Analytics::G1Analytics(uint64_t now) : previousYoungCollectionEnd_(now) {}

void G1Analytics::ReportEvacuatedBytes(size_t bytes)
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    copiedBytes_.fetch_add(bytes, std::memory_order_relaxed);
}

void G1Analytics::ReportScanRemsetStart(uint64_t time)
{
    scanRemsetStart_ = time;
}

void G1Analytics::ReportScanRemsetEnd(uint64_t time)
{
    scanRemsetEnd_ = time;
}

void G1Analytics::ReportMarkingStart(uint64_t time)
{
    markingStart_ = time;
}

void G1Analytics::ReportMarkingEnd(uint64_t time)
{
    markingEnd_ = time;
}

void G1Analytics::ReportEvacuationStart(uint64_t time)
{
    evacuationStart_ = time;
}

void G1Analytics::ReportEvacuationEnd(uint64_t time)
{
    evacuationEnd_ = time;
}

void G1Analytics::ReportUpdateRefsStart(uint64_t time)
{
    updateRefsStart_ = time;
}

void G1Analytics::ReportUpdateRefsEnd(uint64_t time)
{
    updateRefsEnd_ = time;
}

void G1Analytics::ReportPromotedRegion()
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    promotedRegions_.fetch_add(1, std::memory_order_relaxed);
}

void G1Analytics::ReportLiveObjects(size_t num)
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    liveObjects_.fetch_add(num, std::memory_order_relaxed);
}

double G1Analytics::PredictAllocationRate() const
{
    return predictor_.Predict(allocationRateSeq_);
}

int64_t G1Analytics::EstimatePredictionErrorInMicros() const
{
    return predictor_.Predict(predictionErrorSeq_);
}

void G1Analytics::ReportCollectionStart(uint64_t time)
{
    currentYoungCollectionStart_ = time;
    copiedBytes_ = 0;
    promotedRegions_ = 0;
    liveObjects_ = 0;
}

void G1Analytics::ReportCollectionEnd(uint64_t endTime, const CollectionSet &collectionSet)
{
    auto edenLength = collectionSet.Young().size();
    auto appTime = (currentYoungCollectionStart_ - previousYoungCollectionEnd_) / panda::os::time::MICRO_TO_NANO;
    allocationRateSeq_.Add(static_cast<double>(edenLength) / appTime);

    if (collectionSet.Young().size() == collectionSet.size()) {
        if (edenLength != promotedRegions_) {
            auto compactedRegions = edenLength - promotedRegions_;
            liveObjectsSeq_.Add(static_cast<double>(liveObjects_) / compactedRegions);
            copiedBytesSeq_.Add(static_cast<double>(copiedBytes_) / compactedRegions);
            auto estimatedPromotionTime = EstimatePromotionTimeInMicros(promotedRegions_);
            auto evacuationTime = (evacuationEnd_ - evacuationStart_) / panda::os::time::MICRO_TO_NANO;
            if (evacuationTime > estimatedPromotionTime) {
                copyingBytesRateSeq_.Add(static_cast<double>(copiedBytes_) / (evacuationTime - estimatedPromotionTime));
            }
        }

        auto markingTime = (markingEnd_ - markingStart_) / panda::os::time::MICRO_TO_NANO;
        markingRateSeq_.Add(static_cast<double>(liveObjects_) / markingTime);

        auto updateRefsTime = (updateRefsEnd_ - updateRefsStart_) / panda::os::time::MICRO_TO_NANO;
        updateRefsTimeSeq_.Add(static_cast<double>(updateRefsTime));

        ASSERT(edenLength != 0);
        promotionSeq_.Add(static_cast<double>(promotedRegions_) / edenLength);

        auto pauseTime = (endTime - currentYoungCollectionStart_) / panda::os::time::MICRO_TO_NANO;
        predictionErrorSeq_.Add(static_cast<double>(pauseTime) - PredictYoungCollectionTimeInMicros(edenLength));
    }

    auto scanRemsetTime = (scanRemsetEnd_ - scanRemsetStart_) / panda::os::time::MICRO_TO_NANO;
    scanRemsetTimeSeq_.Add(static_cast<double>(scanRemsetTime) / collectionSet.size());

    previousYoungCollectionEnd_ = endTime;
}

int64_t G1Analytics::PredictYoungCollectionTimeInMicros(size_t edenLength) const
{
    auto expectedPromotedRegions = PredictPromotedRegions(edenLength);
    auto expectedCompactedRegions = edenLength - expectedPromotedRegions;
    auto expectedCopiedBytes = expectedCompactedRegions * predictor_.Predict(copiedBytesSeq_);
    auto expectedLiveObjects = expectedCompactedRegions * predictor_.Predict(liveObjectsSeq_);
    auto markingRatePrediction = markingRateSeq_.IsEmpty() ? 0 : predictor_.Predict(markingRateSeq_);
    auto markingCost = markingRatePrediction == 0 ? 0 : expectedLiveObjects / markingRatePrediction;
    auto copyingBytesRatePrediction = copyingBytesRateSeq_.IsEmpty() ? 0 : predictor_.Predict(copyingBytesRateSeq_);
    auto copyingCost = copyingBytesRatePrediction == 0 ? 0 : expectedCopiedBytes / copyingBytesRatePrediction;
    return markingCost + copyingCost + edenLength * predictor_.Predict(scanRemsetTimeSeq_) +
           predictor_.Predict(updateRefsTimeSeq_) + EstimatePromotionTimeInMicros(expectedPromotedRegions);
}

int64_t G1Analytics::PredictOldCollectionTimeInMicros(Region *region) const
{
    auto expectedLiveObjects = region->GetLiveBytes() * region->GetAllocatedObjects() / region->GetAllocatedBytes();
    return PredictOldCollectionTimeInMicros(region->GetLiveBytes(), expectedLiveObjects);
}

int64_t G1Analytics::PredictOldCollectionTimeInMicros(size_t liveBytes, size_t liveObjects) const
{
    // Currently remset scan esimation is too rough. We can improve it after #11263
    auto remsetScanCost = predictor_.Predict(scanRemsetTimeSeq_);
    return liveObjects / predictor_.Predict(markingRateSeq_) + liveBytes / predictor_.Predict(copyingBytesRateSeq_) +
           remsetScanCost;
}

double G1Analytics::PredictPromotedRegions(size_t edenLength) const
{
    return predictor_.Predict(promotionSeq_) * edenLength;
}

size_t G1Analytics::EstimatePromotionTimeInMicros(size_t promotedRegions) const
{
    return promotionCost_ * promotedRegions;
}
}  // namespace panda::mem
