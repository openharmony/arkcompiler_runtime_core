/**
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

#include <algorithm>
#include "liveness_analyzer.h"
#include "live_registers.h"

namespace panda::compiler {

namespace {
struct Split {
    Split(LifeIntervalsIt p_begin, LifeIntervalsIt p_end, LifeNumber p_min, LifeNumber p_max,
          LifeIntervalsTreeNode *p_parent)
        : begin(p_begin), end(p_end), min(p_min), max(p_max), parent(p_parent)
    {
        ASSERT(p_begin < p_end);
        ASSERT(p_min <= p_max);
    }
    LifeIntervalsIt begin;          // NOLINT(misc-non-private-member-variables-in-classes)
    LifeIntervalsIt end;            // NOLINT(misc-non-private-member-variables-in-classes)
    LifeNumber min;                 // NOLINT(misc-non-private-member-variables-in-classes)
    LifeNumber max;                 // NOLINT(misc-non-private-member-variables-in-classes)
    LifeIntervalsTreeNode *parent;  // NOLINT(misc-non-private-member-variables-in-classes)
};

// copy intervals with assigned registers and compute min and max life numbers covered by all these intervals
std::pair<LifeNumber, LifeNumber> CopyIntervals(const ArenaVector<LifeIntervals *> &source,
                                                ArenaVector<LifeIntervals *> *destination)
{
    LifeNumber min_ln = std::numeric_limits<LifeNumber>::max();
    LifeNumber max_ln = 0;
    for (auto &interval : source) {
        for (auto split = interval; !interval->IsPhysical() && split != nullptr; split = split->GetSibling()) {
            if (split->HasReg()) {
                min_ln = std::min(min_ln, split->GetBegin());
                max_ln = std::max(max_ln, split->GetEnd());
                destination->push_back(split);
            }
        }
    }
    return std::make_pair(min_ln, max_ln);
}

LifeIntervalsIt PartitionLeftSplit(const LifeIntervalsIt &left, const LifeIntervalsIt &right, LifeNumber midpoint,
                                   LifeNumber *min_ln, LifeNumber *max_ln)
{
    LifeNumber left_min_ln = std::numeric_limits<LifeNumber>::max();
    LifeNumber left_max_ln = 0;
    auto result = std::partition(left, right, [&midpoint, &left_min_ln, &left_max_ln](const auto &em) {
        if (em->GetEnd() < midpoint) {
            left_min_ln = std::min(left_min_ln, em->GetBegin());
            left_max_ln = std::max(left_max_ln, em->GetEnd());
            return true;
        }
        return false;
    });
    *min_ln = left_min_ln;
    *max_ln = left_max_ln;
    return result;
}

LifeIntervalsIt PartitionRightSplit(const LifeIntervalsIt &left, const LifeIntervalsIt &right, LifeNumber midpoint,
                                    LifeNumber *min_ln, LifeNumber *max_ln)
{
    LifeNumber right_min_ln = std::numeric_limits<LifeNumber>::max();
    LifeNumber right_max_ln = 0;
    auto result = std::partition(left, right, [&midpoint, &right_min_ln, &right_max_ln](const auto &em) {
        if (em->GetBegin() > midpoint) {
            right_min_ln = std::min(right_min_ln, em->GetBegin());
            right_max_ln = std::max(right_max_ln, em->GetEnd());
            return false;
        }
        return true;
    });
    *min_ln = right_min_ln;
    *max_ln = right_max_ln;
    return result;
}
}  // namespace

LifeIntervalsTree *LifeIntervalsTree::BuildIntervalsTree(const ArenaVector<LifeIntervals *> &life_intervals,
                                                         const Graph *graph)
{
    auto alloc = graph->GetAllocator();
    auto lalloc = graph->GetLocalAllocator();
    auto intervals = alloc->New<ArenaVector<LifeIntervals *>>(alloc->Adapter());
    ArenaQueue<const Split *> queue(lalloc->Adapter());

    auto ln_range = CopyIntervals(life_intervals, intervals);
    if (intervals->empty()) {
        return nullptr;
    }
    queue.push(lalloc->New<Split>(intervals->begin(), intervals->end(), ln_range.first, ln_range.second, nullptr));

    LifeIntervalsTreeNode *root {nullptr};

    // Split each interval into three parts:
    // 1) intervals covering mid point;
    // 2) intervals ended before mid point;
    // 3) intervals started after mid point.
    // Allocate tree node for (1), recursively process (2) and (3).
    while (!queue.empty()) {
        auto split = queue.front();
        queue.pop();
        if (split->end - split->begin <= 0) {
            continue;
        }

        auto midpoint = split->min + (split->max - split->min) / 2U;

        LifeNumber left_min_ln;
        LifeNumber left_max_ln;
        auto left_midpoint = PartitionLeftSplit(split->begin, split->end, midpoint, &left_min_ln, &left_max_ln);

        LifeNumber right_min_ln;
        LifeNumber right_max_ln;
        auto right_midpoint = PartitionRightSplit(left_midpoint, split->end, midpoint, &right_min_ln, &right_max_ln);

        std::sort(left_midpoint, right_midpoint,
                  [](LifeIntervals *l, LifeIntervals *r) { return l->GetEnd() > r->GetEnd(); });

        auto node = alloc->New<LifeIntervalsTreeNode>(split->min, split->max, left_midpoint, right_midpoint);
        if (split->parent == nullptr) {
            root = node;
        } else if (split->parent->GetMidpoint() > midpoint) {
            split->parent->SetLeft(node);
        } else {
            split->parent->SetRight(node);
        }
        if (split->begin < left_midpoint) {
            queue.push(lalloc->New<Split>(split->begin, left_midpoint, left_min_ln, left_max_ln, node));
        }
        if (right_midpoint < split->end) {
            queue.push(lalloc->New<Split>(right_midpoint, split->end, right_min_ln, right_max_ln, node));
        }
    }
    return alloc->New<LifeIntervalsTree>(root);
}

}  // namespace panda::compiler
