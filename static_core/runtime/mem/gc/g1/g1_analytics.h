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

#ifndef PANDA_MEM_GC_G1_G1_ANALYTICS_H
#define PANDA_MEM_GC_G1_G1_ANALYTICS_H

#include <atomic>
#include "runtime/mem/gc/g1/g1_predictions.h"
#include "runtime/mem/gc/g1/collection_set.h"

namespace panda::mem {
class G1Analytics {
public:
    explicit G1Analytics(uint64_t now);

    void ReportCollectionStart(uint64_t time);
    void ReportCollectionEnd(uint64_t endTime, const CollectionSet &collectionSet);

    void ReportEvacuatedBytes(size_t bytes);
    void ReportScanRemsetStart(uint64_t time);
    void ReportScanRemsetEnd(uint64_t time);
    void ReportMarkingStart(uint64_t time);
    void ReportMarkingEnd(uint64_t time);
    void ReportEvacuationStart(uint64_t time);
    void ReportEvacuationEnd(uint64_t time);
    void ReportUpdateRefsStart(uint64_t time);
    void ReportUpdateRefsEnd(uint64_t time);
    void ReportPromotedRegion();
    void ReportLiveObjects(size_t num);

    double PredictAllocationRate() const;
    int64_t PredictYoungCollectionTimeInMicros(size_t edenLength) const;
    int64_t EstimatePredictionErrorInMicros() const;
    int64_t PredictOldCollectionTimeInMicros(size_t liveBytes, size_t liveObjects) const;
    int64_t PredictOldCollectionTimeInMicros(Region *region) const;

private:
    double PredictPromotedRegions(size_t edenLength) const;
    size_t EstimatePromotionTimeInMicros(size_t promotedRegions) const;

    static constexpr uint64_t DEFAULT_PROMOTION_COST = 50;
    const uint64_t promotionCost_ {DEFAULT_PROMOTION_COST};
    uint64_t previousYoungCollectionEnd_;
    uint64_t currentYoungCollectionStart_ {0};
    uint64_t scanRemsetStart_ {0};
    uint64_t scanRemsetEnd_ {0};
    uint64_t markingStart_ {0};
    uint64_t markingEnd_ {0};
    uint64_t evacuationStart_ {0};
    uint64_t evacuationEnd_ {0};
    uint64_t updateRefsStart_ {0};
    uint64_t updateRefsEnd_ {0};
    panda::Sequence allocationRateSeq_;
    panda::Sequence copiedBytesSeq_;
    panda::Sequence copyingBytesRateSeq_;
    panda::Sequence scanRemsetTimeSeq_;
    panda::Sequence liveObjecstSeq_;
    panda::Sequence markingRateSeq_;
    panda::Sequence updateRefsTimeSeq_;
    panda::Sequence promotionSeq_;
    panda::Sequence predictionErrorSeq_;
    panda::Sequence liveObjectsSeq_;
    static constexpr double DEFAULT_CONFIDENCE_FACTOR = 0.5;
    G1Predictor predictor_ {DEFAULT_CONFIDENCE_FACTOR};
    std::atomic<size_t> copiedBytes_ {0};
    std::atomic<size_t> promotedRegions_ {0};
    std::atomic<size_t> liveObjects_ {0};
};
}  // namespace panda::mem
#endif
