/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_UTILS_WEIGHTED_ADAPTIVE_TLAB_AVERAGE_H
#define LIBPANDABASE_UTILS_WEIGHTED_ADAPTIVE_TLAB_AVERAGE_H

#include <algorithm>
#include "libpandabase/macros.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {
class WeightedAdaptiveTlabAverage {
public:
    explicit WeightedAdaptiveTlabAverage(float initTlabSize, float upperSumBorder, float maxGrowRatio, float weight,
                                         float desiredFillFraction)
        : maxGrowRatio_(maxGrowRatio),
          lowerSumBorder_(initTlabSize),
          upperSumBorder_(upperSumBorder),
          lastCountedSum_(initTlabSize),
          weight_(weight),
          desiredFillFraction_(desiredFillFraction)
    {
        ASSERT(maxGrowRatio_ > 1.0);
    }

    void StoreNewSampleAndIncreaseIfNeeded(size_t occupiedSize, size_t maxSize)
    {
        // Supposed that this method is called only in case of slow path
        ASSERT(occupiedSize <= maxSize);
        samples_.emplace_back(std::make_pair(occupiedSize, maxSize));
        if (++slowAllocationCount_ > SLOW_ALLOCATION_THRESHOLD) {
            // There are too many slow paths taken by compiler
            // It is better to increase lastCountedSum_ immediately
            static constexpr float INCREASE_RATE = 2.0;
            lastCountedSum_ = std::clamp(lastCountedSum_ * INCREASE_RATE, lowerSumBorder_, upperSumBorder_);
        }
    }

    /**
     * @brief Estimates new TLAB size using saved TLABs' information
     *
     * 1) First of all it checks if we have allocated too many (>= THRESHOLD_COUNT) TLABs
     *    In that case we increase size by (samples_.size() / THRESHOLD_COUNT)
     *    In other case go to ----> 2)
     * 2) If vector with samples is not empty, we start estimating TLABs' average fill fraction
     *    If average fill fraction is less than desiredFillFraction_, then we increase TLABs' size
     *    Else, we reduce TLABs' size
     */
    void ComputeNewSumAndResetSamples()
    {
        float newCountedSum = lastCountedSum_;
        if (samples_.size() >= THRESHOLD_COUNT) {
            // In this case we have a lot of allocated TLABs
            // So, we should consider increasing lastCountedSum_
            // Also need to mind grow ratio (for softer increase/decrease of the sum)
            newCountedSum = std::clamp(lastCountedSum_ * (static_cast<float>(samples_.size()) / THRESHOLD_COUNT),
                                       lastCountedSum_ / maxGrowRatio_, lastCountedSum_ * maxGrowRatio_);
        } else if (!samples_.empty()) {
            float averageFillFraction = -1.0;
            // Average fill fraction estimation
            for (auto &sample : samples_) {
                if (sample.second != 0) {
                    averageFillFraction = averageFillFraction < 0
                                              ? static_cast<float>(sample.first) / sample.second
                                              : static_cast<float>(sample.first) / sample.second * weight_ +
                                                    averageFillFraction * (1.0F - weight_);
                }
            }
            if (averageFillFraction < 0) {
                // it means that zero tlab is the only sample that we currently have
                // in this case we don't have enough information to calculate new TLAB size
                samples_.clear();
                return;
            }
            if (averageFillFraction < desiredFillFraction_) {
                // Should increase TLABs' size
                newCountedSum = std::clamp(lastCountedSum_ * (desiredFillFraction_ / averageFillFraction),
                                           lastCountedSum_ / maxGrowRatio_, lastCountedSum_ * maxGrowRatio_);
            } else {
                // In this case we have few TLABs with good average fill fraction
                // It means that we should consider reducing TLABs' size
                newCountedSum = lastCountedSum_ * REDUCTION_RATE;
            }
        }
        // Mind lower and upper borders
        lastCountedSum_ =
            std::clamp(newCountedSum * weight_ + lastCountedSum_ * (1.0F - weight_), lowerSumBorder_, upperSumBorder_);
        slowAllocationCount_ = 0;
        samples_.clear();
    }

    float GetLastCountedSum() const
    {
        return lastCountedSum_;
    }

    size_t GetLastCountedSumInSizeT() const
    {
        return static_cast<size_t>(lastCountedSum_);
    }

private:
    static constexpr size_t THRESHOLD_COUNT = 4;            // Threshold value showing that we have allocated
                                                            // a lot of TLABs between GC pauses
    static constexpr size_t SLOW_ALLOCATION_THRESHOLD = 3;  // Value used for force increase of lastCountedSum_
    static constexpr float REDUCTION_RATE = 0.75;           // Value used for size reduction
    float maxGrowRatio_;                                    // Max change ratio when new average is estimated
                                                            // (means that (newAverage / average) < maxGrowRatio_)
    float lowerSumBorder_;                                  // Min value that sum may take
    float upperSumBorder_;                                  // Max value that sum may take
    float lastCountedSum_;                                  // Last estimated sum
    float weight_;                                          // From 0 to 1
                                                            // Used for better average estimation
                                                            // Recently observed samples should have greater weight
    float desiredFillFraction_;                             // From 0 to 1
                                                            // If (occupiedSize / maxSize) < desiredFillFraction_ then
                                                            // lastCountedSum_ should grow
    size_t slowAllocationCount_ = 0;                        // Used for force increase of lastCountedSum_
                                                            // if slowAllocationCount_ > SLOW_ALLOCATION_THRESHOLD
    PandaVector<std::pair<size_t, size_t>> samples_;        // Saved samples. Used for estimation of a new sum
};

}  // namespace ark
#endif  // LIBPANDABASE_UTILS_WEIGHTED_ADAPTIVE_TLAB_AVERAGE_H
