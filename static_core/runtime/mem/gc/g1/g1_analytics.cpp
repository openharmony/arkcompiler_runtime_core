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
G1Analytics::G1Analytics(uint64_t now) : previous_young_collection_end_(now) {}

void G1Analytics::ReportEvacuatedBytes(size_t bytes)
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    copied_bytes_.fetch_add(bytes, std::memory_order_relaxed);
}

void G1Analytics::ReportScanRemsetStart(uint64_t time)
{
    scan_remset_start_ = time;
}

void G1Analytics::ReportScanRemsetEnd(uint64_t time)
{
    scan_remset_end_ = time;
}

void G1Analytics::ReportMarkingStart(uint64_t time)
{
    marking_start_ = time;
}

void G1Analytics::ReportMarkingEnd(uint64_t time)
{
    marking_end_ = time;
}

void G1Analytics::ReportEvacuationStart(uint64_t time)
{
    evacuation_start_ = time;
}

void G1Analytics::ReportEvacuationEnd(uint64_t time)
{
    evacuation_end_ = time;
}

void G1Analytics::ReportUpdateRefsStart(uint64_t time)
{
    update_refs_start_ = time;
}

void G1Analytics::ReportUpdateRefsEnd(uint64_t time)
{
    update_refs_end_ = time;
}

void G1Analytics::ReportPromotedRegion()
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    promoted_regions_.fetch_add(1, std::memory_order_relaxed);
}

void G1Analytics::ReportLiveObjects(size_t num)
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    live_objects_.fetch_add(num, std::memory_order_relaxed);
}

double G1Analytics::PredictAllocationRate() const
{
    return predictor_.Predict(allocation_rate_seq_);
}

int64_t G1Analytics::EstimatePredictionErrorInMicros() const
{
    return predictor_.Predict(prediction_error_seq_);
}

void G1Analytics::ReportCollectionStart(uint64_t time)
{
    current_young_collection_start_ = time;
    copied_bytes_ = 0;
    promoted_regions_ = 0;
    live_objects_ = 0;
}

void G1Analytics::ReportCollectionEnd(uint64_t end_time, const CollectionSet &collection_set)
{
    auto eden_length = collection_set.Young().size();
    auto app_time = (current_young_collection_start_ - previous_young_collection_end_) / panda::os::time::MICRO_TO_NANO;
    allocation_rate_seq_.Add(static_cast<double>(eden_length) / app_time);

    if (collection_set.Young().size() == collection_set.size()) {
        if (eden_length != promoted_regions_) {
            auto compacted_regions = eden_length - promoted_regions_;
            live_objects_seq_.Add(static_cast<double>(live_objects_) / compacted_regions);
            copied_bytes_seq_.Add(static_cast<double>(copied_bytes_) / compacted_regions);
            auto estimated_promotion_time = EstimatePromotionTimeInMicros(promoted_regions_);
            auto evacuation_time = (evacuation_end_ - evacuation_start_) / panda::os::time::MICRO_TO_NANO;
            if (evacuation_time > estimated_promotion_time) {
                copying_bytes_rate_seq_.Add(static_cast<double>(copied_bytes_) /
                                            (evacuation_time - estimated_promotion_time));
            }
        }

        auto marking_time = (marking_end_ - marking_start_) / panda::os::time::MICRO_TO_NANO;
        marking_rate_seq_.Add(static_cast<double>(live_objects_) / marking_time);

        auto update_refs_time = (update_refs_end_ - update_refs_start_) / panda::os::time::MICRO_TO_NANO;
        update_refs_time_seq_.Add(static_cast<double>(update_refs_time));

        promotion_seq_.Add(static_cast<double>(promoted_regions_) / eden_length);

        auto pause_time = (end_time - current_young_collection_start_) / panda::os::time::MICRO_TO_NANO;
        prediction_error_seq_.Add(static_cast<double>(pause_time) - PredictYoungCollectionTimeInMicros(eden_length));
    }

    auto scan_remset_time = (scan_remset_end_ - scan_remset_start_) / panda::os::time::MICRO_TO_NANO;
    scan_remset_time_seq_.Add(static_cast<double>(scan_remset_time) / collection_set.size());

    previous_young_collection_end_ = end_time;
}

int64_t G1Analytics::PredictYoungCollectionTimeInMicros(size_t eden_length) const
{
    auto expected_promoted_regions = PredictPromotedRegions(eden_length);
    auto expected_compacted_regions = eden_length - expected_promoted_regions;
    auto expected_copied_bytes = expected_compacted_regions * predictor_.Predict(copied_bytes_seq_);
    auto expected_live_objects = expected_compacted_regions * predictor_.Predict(live_objects_seq_);
    auto marking_rate_prediction = marking_rate_seq_.IsEmpty() ? 0 : predictor_.Predict(marking_rate_seq_);
    auto marking_cost = marking_rate_prediction == 0 ? 0 : expected_live_objects / marking_rate_prediction;
    auto copying_bytes_rate_prediction =
        copying_bytes_rate_seq_.IsEmpty() ? 0 : predictor_.Predict(copying_bytes_rate_seq_);
    auto copying_cost = copying_bytes_rate_prediction == 0 ? 0 : expected_copied_bytes / copying_bytes_rate_prediction;
    return marking_cost + copying_cost + eden_length * predictor_.Predict(scan_remset_time_seq_) +
           predictor_.Predict(update_refs_time_seq_) + EstimatePromotionTimeInMicros(expected_promoted_regions);
}

int64_t G1Analytics::PredictOldCollectionTimeInMicros(Region *region) const
{
    auto expected_live_objects = region->GetLiveBytes() * region->GetAllocatedObjects() / region->GetAllocatedBytes();
    return PredictOldCollectionTimeInMicros(region->GetLiveBytes(), expected_live_objects);
}

int64_t G1Analytics::PredictOldCollectionTimeInMicros(size_t live_bytes, size_t live_objects) const
{
    // Currently remset scan esimation is too rough. We can improve it after #11263
    auto remset_scan_cost = predictor_.Predict(scan_remset_time_seq_);
    return live_objects / predictor_.Predict(marking_rate_seq_) +
           live_bytes / predictor_.Predict(copying_bytes_rate_seq_) + remset_scan_cost;
}

double G1Analytics::PredictPromotedRegions(size_t eden_length) const
{
    return predictor_.Predict(promotion_seq_) * eden_length;
}

size_t G1Analytics::EstimatePromotionTimeInMicros(size_t promoted_regions) const
{
    return promotion_cost_ * promoted_regions;
}
}  // namespace panda::mem
