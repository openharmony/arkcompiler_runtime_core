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
#ifndef PANDA_RUNTIME_EXECUTION_PRIORITY_QUEUE_H
#define PANDA_RUNTIME_EXECUTION_PRIORITY_QUEUE_H

#include "libarkbase/macros.h"
#include "include/mem/panda_containers.h"
#include "runtime/execution/job_priority.h"

#include <cstdint>
#include <limits>
#include <type_traits>

namespace ark {

enum class ElementOrder { STACK_ORDER, QUEUE_ORDER };

template <typename Element>
class PriorityQueue {
    using Priority = JobPriority;

    class ElementWrapper {
    public:
        explicit ElementWrapper(Element *elem, uint64_t priority) : elem_(elem), priority_(priority)
        {
            ASSERT((priority >> ORDER_BITS) != 0);
        }

        Element *operator*() const
        {
            return elem_;
        }

        Element *operator->() const
        {
            return elem_;
        }

        /// @return compares Element wrappers by priority
        bool operator<(const ElementWrapper &other) const
        {
            return priority_ < other.priority_;
        }

        /// @return priority of wrapped Element
        Priority GetPriority();

    private:
        Element *elem_;
        uint64_t priority_;
    };

public:
    using Queue = PandaDeque<ElementWrapper>;
    using CIterator = typename Queue::const_iterator;
    using Properties = std::unordered_map<Priority, ElementOrder>;

    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    static inline Properties defaultProps_ = {{Priority::LOW_PRIORITY, ElementOrder::QUEUE_ORDER},
                                              {Priority::MEDIUM_PRIORITY, ElementOrder::QUEUE_ORDER},
                                              {Priority::HIGH_PRIORITY, ElementOrder::QUEUE_ORDER},
                                              {Priority::CRITICAL_PRIORITY, ElementOrder::QUEUE_ORDER}};

    /// Creates PriorityQueue with the given properties (Priority -> Order mapping)
    explicit PriorityQueue(Properties props = defaultProps_) : properties_(std::move(props)) {}

    /**
     * Adds a Element to the queue in order of priority.
     * For the stack order, the Element is added at the top.
     * For the queue order - at the end of the priority subqueue.
     */
    void Push(Element *elem, Priority priority);

    /**
     * Checks that queue is not empty,
     * returns Element with the highest priority.
     */
    std::pair<Element *, Priority> Pop();

    /**
     * Allows to iterate over Elements in the priority queue.
     * Guarantees priority ordrer during iteration.
     */
    template <typename Visitor, typename U = std::enable_if_t<std::is_invocable_v<Visitor, Element *>>>
    void IterateOverElements(const Visitor &visitor) const
    {
        for (auto &coroWrapper : elems_) {
            visitor(*coroWrapper);
        }
    }

    /**
     * Allows to apply callback for ElementWrappers in the priority queue.
     * Can be used in pair with RemoveElements.
     */
    template <typename Visitor, typename U = std::enable_if_t<std::is_invocable_v<Visitor, CIterator, CIterator>>>
    void VisitElements(const Visitor &visitor) const
    {
        visitor(elems_.cbegin(), elems_.cend());
    }

    /// Removes Elements from the priority queue by given sorted sequence of iterators
    template <typename Container,
              typename U = std::enable_if_t<std::is_same_v<typename Container::value_type, CIterator>>>
    void RemoveElements(const Container &elementsIterators)
    {
        auto remRangeEnd = elems_.begin();
        for (const auto &iter : elementsIterators) {
            auto mutIter = elems_.begin() + (iter - elems_.cbegin());
            std::iter_swap(remRangeEnd++, mutIter);
        }
        elems_.erase(elems_.begin(), remRangeEnd);
        std::make_heap(elems_.begin(), elems_.end(), Comparator);
    }

    /// @return size of the priority queue
    size_t Size() const
    {
        return elems_.size();
    }

    /// @return true if priority queue is empty
    bool Empty() const
    {
        return elems_.empty();
    }

private:
    uint64_t CalculatePriority(Priority priority);

    static bool Comparator(const ElementWrapper &co1, const ElementWrapper &co2);

    static constexpr uint64_t PRIORITY_COUNT = static_cast<uint64_t>(Priority::PRIORITY_COUNT);
    static constexpr uint64_t ORDER_BITS = std::numeric_limits<uint64_t>::digits - PRIORITY_COUNT;

    Queue elems_;
    Properties properties_;
    uint64_t stackOrder_ = 0;
    uint64_t queueOrder_ = std::numeric_limits<uint64_t>::max() >> PRIORITY_COUNT;
};
}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_PRIORITY_QUEUE_H
