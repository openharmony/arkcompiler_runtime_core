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
    void ReportCollectionEnd(uint64_t end_time, const CollectionSet &collection_set);

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
    int64_t PredictYoungCollectionTimeInMicros(size_t eden_length) const;
    int64_t EstimatePredictionErrorInMicros() const;
    int64_t PredictOldCollectionTimeInMicros(size_t live_bytes, size_t live_objects) const;
    int64_t PredictOldCollectionTimeInMicros(Region *region) const;

private:
    double PredictPromotedRegions(size_t eden_length) const;
    size_t EstimatePromotionTimeInMicros(size_t promoted_regions) const;

    static constexpr uint64_t DEFAULT_PROMOTION_COST = 50;
    const uint64_t promotion_cost_ {DEFAULT_PROMOTION_COST};
    uint64_t previous_young_collection_end_;
    uint64_t current_young_collection_start_ {0};
    uint64_t scan_remset_start_ {0};
    uint64_t scan_remset_end_ {0};
    uint64_t marking_start_ {0};
    uint64_t marking_end_ {0};
    uint64_t evacuation_start_ {0};
    uint64_t evacuation_end_ {0};
    uint64_t update_refs_start_ {0};
    uint64_t update_refs_end_ {0};
    panda::Sequence allocation_rate_seq_;
    panda::Sequence copied_bytes_seq_;
    panda::Sequence copying_bytes_rate_seq_;
    panda::Sequence scan_remset_time_seq_;
    panda::Sequence live_objecst_seq_;
    panda::Sequence marking_rate_seq_;
    panda::Sequence update_refs_time_seq_;
    panda::Sequence promotion_seq_;
    panda::Sequence prediction_error_seq_;
    panda::Sequence live_objects_seq_;
    static constexpr double DEFAULT_CONFIDENCE_FACTOR = 0.5;
    G1Predictor predictor_ {DEFAULT_CONFIDENCE_FACTOR};
    std::atomic<size_t> copied_bytes_ {0};
    std::atomic<size_t> promoted_regions_ {0};
    std::atomic<size_t> live_objects_ {0};
};
}  // namespace panda::mem
#endif
