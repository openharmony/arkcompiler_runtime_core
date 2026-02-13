/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "runtime/execution/priority_queue.h"

#include <algorithm>

namespace ark {

template <typename Element>
typename PriorityQueue<Element>::Priority PriorityQueue<Element>::ElementWrapper::GetPriority()
{
    auto internalPriority = priority_ >> ORDER_BITS;
    auto priority = 0U;
    while (internalPriority != 1) {
        internalPriority >>= 1U;
        priority++;
    }
    ASSERT(priority < PRIORITY_COUNT);
    return static_cast<Priority>(priority);
}

template <typename Element>
void PriorityQueue<Element>::Push(Element *elem, Priority priority)
{
    elems_.emplace_back(elem, CalculatePriority(priority));
    std::push_heap(elems_.begin(), elems_.end(), Comparator);
}

template <typename Element>
std::pair<Element *, typename PriorityQueue<Element>::Priority> PriorityQueue<Element>::Pop()
{
    ASSERT(!Empty());
    std::pop_heap(elems_.begin(), elems_.end(), Comparator);
    auto jobWrapper = elems_.back();
    elems_.pop_back();
    return {*jobWrapper, jobWrapper.GetPriority()};
}

template <typename Element>
uint64_t PriorityQueue<Element>::CalculatePriority(Priority priority)
{
    ASSERT(static_cast<uint32_t>(priority) < PRIORITY_COUNT);
    auto orderIt = properties_.find(priority);
    ASSERT(orderIt != properties_.end());
    uint64_t order = 0;
    switch (orderIt->second) {
        case ElementOrder::STACK_ORDER:
            order = stackOrder_++;
            break;
        case ElementOrder::QUEUE_ORDER:
            order = queueOrder_--;
            break;
        default:
            UNREACHABLE();
    }
    return (1ULL << (ORDER_BITS + static_cast<uint32_t>(priority))) | order;
}

template <typename Element>
bool PriorityQueue<Element>::Comparator(const ElementWrapper &co1, const ElementWrapper &co2)
{
    return co1 < co2;
}

class Coroutine;
class Job;
template class PriorityQueue<Coroutine>;
template class PriorityQueue<Job>;

}  // namespace ark
