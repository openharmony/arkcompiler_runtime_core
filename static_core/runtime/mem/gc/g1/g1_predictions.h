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

#ifndef PANDA_MEM_GC_G1_G1_PREDICTIONS_H
#define PANDA_MEM_GC_G1_G1_PREDICTIONS_H

#include "libpandabase/utils/sequence.h"

namespace panda::mem {
class G1Predictor {
public:
    explicit G1Predictor(double confidence_factor) : confidence_factor_(confidence_factor) {}

    double Predict(const panda::Sequence &seq) const
    {
        return seq.GetMean() + confidence_factor_ * seq.GetStdDev();
    }

private:
    double confidence_factor_;
};
}  // namespace panda::mem
#endif
