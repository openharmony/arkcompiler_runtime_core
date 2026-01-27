/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_CORETYPES_ALGORITHM_KNUTH_MORRIS_PRATT_H
#define PANDA_RUNTIME_CORETYPES_ALGORITHM_KNUTH_MORRIS_PRATT_H

#include "include/mem/panda_containers.h"
#include "libarkbase/utils/span.h"
#include <limits>
#include <type_traits>
#include <utility>

namespace ark::coretypes::algo {

template <typename P>
class KmpStringMatcher final {
    static_assert(std::is_integral_v<P>, "KMP handles only arrays of integral types");

public:
    explicit KmpStringMatcher(Span<const P> pattern) : pattern_(pattern), prefixFun_(ComputePrefixFun(pattern)) {}

    template <typename S>
    int32_t IndexOf(Span<const S> subject, int32_t pos = 0) const
    {
        static_assert(std::is_integral_v<S>, "KMP handles only arrays of integral types");
        ASSERT(pos >= 0);
        int32_t q = 0;
        int32_t pch = ToI32(pattern_[0]);
        int32_t patternSize = ToI32(pattern_.size());
        int32_t subjectSize = ToI32(subject.size());
        for (int32_t i = pos; i + patternSize <= q + subjectSize; ++i) {
            int32_t sch = ToI32(subject[i]);
            while (q > 0 && sch != pch) {
                q = prefixFun_[q - 1];
                pch = ToI32(pattern_[q]);
            }
            if (sch == pch) {
                q++;
                if (q < patternSize) {
                    pch = ToI32(pattern_[q]);
                } else {
                    return i + 1 - patternSize;
                }
            }
        }
        return -1;
    }

    template <typename S>
    int32_t LastIndexOf(Span<const S> subject, int32_t pos = std::numeric_limits<int32_t>::max()) const
    {
        static_assert(std::is_integral_v<S>, "KMP handles only arrays of integral types");
        ASSERT(pos >= 0);
        int32_t idx = -1;
        int32_t q = 0;
        int32_t pch = ToI32(pattern_[0]);
        int32_t patternSize = ToI32(pattern_.size());
        int32_t subjectSize = ToI32(subject.size());
        for (int32_t i = 0; i - patternSize < pos && i + patternSize <= q + subjectSize; ++i) {
            int32_t sch = ToI32(subject[i]);
            while (q > 0 && sch != pch) {
                q = prefixFun_[q - 1];
                pch = ToI32(pattern_[q]);
            }
            if (sch == pch) {
                q++;
                if (q < patternSize) {
                    pch = ToI32(pattern_[q]);
                } else {
                    idx = i + 1 - patternSize;
                    q = prefixFun_[q - 1];
                    pch = ToI32(pattern_[q]);
                }
            }
        }
        ASSERT(idx <= pos);
        return idx;
    }

private:
    static int32_t ToI32(size_t size)
    {
        return static_cast<int32_t>(size);
    }

    static PandaVector<int32_t> ComputePrefixFun(Span<const P> pattern)
    {
        ASSERT(!pattern.empty());
        PandaVector<int32_t> prefixFun;
        prefixFun.reserve(pattern.size());
        int32_t k = 0;
        prefixFun.emplace_back(0);
        for (size_t i = 1U; i < pattern.size(); ++i) {
            while (k > 0 && pattern[k] != pattern[i]) {
                k = prefixFun[k - 1];
            }
            if (pattern[k] == pattern[i]) {
                k++;
            }
            prefixFun.emplace_back(k);
        }

        return prefixFun;
    }

    Span<const P> pattern_;
    PandaVector<int32_t> prefixFun_;
};

}  // namespace ark::coretypes::algo

#endif  // PANDA_RUNTIME_CORETYPES_ALGORITHM_KNUTH_MORRIS_PRATT_H
