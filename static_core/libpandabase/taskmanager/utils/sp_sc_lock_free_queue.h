/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_SP_SC_LOCK_FREE_QUEUE_H
#define LIBPANDABASE_TASKMANAGER_UTILS_SP_SC_LOCK_FREE_QUEUE_H

#include "libpandabase/macros.h"
#include "libpandabase/utils/math_helpers.h"
#include "coherency_line_size.h"
#include <array>
#include <atomic>

namespace ark::taskmanager::internal {

/**
 * @brief SPSCLockFreeQueue is single producer, single consumer lock free queue.
 * @tparam T: Type of class you want ot store in queue
 * @tparam Allocator: Type of allocator that will be used to allocate nodes
 */
template <class T, class Allocator>
class SPSCLockFreeQueue {
    static constexpr size_t QUEUE_NODE_SIZE = 1UL << 5U;
    static_assert(ark::helpers::math::IsPowerOfTwo(QUEUE_NODE_SIZE));
    static constexpr size_t QUEUE_NODE_MASK = QUEUE_NODE_SIZE - 1;
    static_assert(QUEUE_NODE_MASK > 0);

    struct QueueNode {
        std::array<T, QUEUE_NODE_SIZE> buffer;
        std::atomic<QueueNode *> next = nullptr;
    };

    using QueueNodeAllocatorType = typename std::allocator_traits<Allocator>::template rebind_alloc<QueueNode>;
    template <class U, class OtherAllocator>
    friend class SPSCLockFreeQueue;

public:
    SPSCLockFreeQueue()
    {
        auto *node = GetNewQueueNode();
        // Atomic with relaxed order reason: no order requirement
        head_.store(node, std::memory_order_relaxed);
        // Atomic with relaxed order reason: no order requirement
        tail_.store(node, std::memory_order_relaxed);
    }
    ~SPSCLockFreeQueue()
    {
        ASSERT(pushIndex_ == popIndex_);
        ASSERT(head_ == tail_);
        // Atomic with relaxed order reason: no order requirement
        auto head = head_.load(std::memory_order_relaxed);
        DeleteQueueNode(head);
    }

    NO_COPY_SEMANTIC(SPSCLockFreeQueue);
    NO_MOVE_SEMANTIC(SPSCLockFreeQueue);

    void Push(T &&val)
    {
        // Atomic with relaxed order reason: gets local variable
        auto *tail = tail_.load(std::memory_order_relaxed);
        // Atomic with relaxed order reason: gets local variable
        auto pushIndex = pushIndex_.load(std::memory_order_relaxed);
        if UNLIKELY (GetNodeIndex(pushIndex) == 0) {
            auto *node = GetNewQueueNode();
            // Atomic with relaxed order reason: set in local variable
            tail->next.store(node, std::memory_order_relaxed);
            node->buffer[0] = std::move(val);
            // Atomic with relaxed order reason: set in local variable
            tail_.store(node, std::memory_order_relaxed);
        } else {
            tail->buffer[GetNodeIndex(pushIndex)] = std::move(val);
        }
        // Atomic with release order reason: other threads should see correct value
        pushIndex_.store(pushIndex + 1U, std::memory_order_release);
    }

    bool TryPop(T *val)
    {
        ASSERT(val != nullptr);
        // Atomic with relaxed order reason: gets local variable
        auto *head = head_.load(std::memory_order_relaxed);
        // Atomic With relaxed order reason: gets local variable
        auto popIndex = popIndex_.load(std::memory_order_relaxed);
        // Atomic with acquire order reason: need observe on pushes local variables
        auto pushIndex = pushIndex_.load(std::memory_order_acquire);
        if (popIndex == pushIndex) {
            return false;
        }

        if UNLIKELY (GetNodeIndex(popIndex) == 0) {
            // Atomic with relaxed order reason: gets local variable
            auto *nextHead = head->next.load(std::memory_order_relaxed);
            ASSERT(nextHead != nullptr);
            // Atomic with relaxed order reason: set in local variable
            head_.store(nextHead, std::memory_order_relaxed);
            DeleteQueueNode(head);
            *val = std::move(nextHead->buffer[0]);
        } else {
            *val = std::move(head->buffer[GetNodeIndex(popIndex)]);
        }
        ASSERT(popIndex != SIZE_MAX);
        // Atomic with relaxed order reason: set in local variable
        popIndex_.store(popIndex + 1, std::memory_order_relaxed);

        return true;
    }

    T Pop()
    {
        T val;
        while (!TryPop(&val)) {
        }
        return val;
    }

    bool inline IsEmpty() const
    {
        return Size() == 0;
    }

    size_t inline Size() const
    {
        // Atomic with relaxed order reason: gets correct value
        auto pushIndex = pushIndex_.load(std::memory_order_relaxed);
        // Atomic with relaxed order reason: gets correct value
        auto popIndex = popIndex_.load(std::memory_order_relaxed);
        return pushIndex - popIndex;
    }

private:
    QueueNode *GetNewQueueNode()
    {
        QueueNodeAllocatorType allocator;
        auto *mem = allocator.allocate(1U);
        ASSERT(mem != nullptr);
        return new (mem) QueueNode;
    }

    void DeleteQueueNode(QueueNode *node)
    {
        QueueNodeAllocatorType allocator;
        std::allocator_traits<QueueNodeAllocatorType>::destroy(allocator, node);
        allocator.deallocate(node, 1U);
    }

    size_t GetNodeIndex(size_t index)
    {
        return index & QUEUE_NODE_MASK;
    }

    alignas(ark::COHERENCY_LINE_SIZE) std::atomic<QueueNode *> head_ = {nullptr};
    alignas(ark::COHERENCY_LINE_SIZE) std::atomic<QueueNode *> tail_ = {nullptr};

    alignas(ark::COHERENCY_LINE_SIZE) std::atomic<size_t> popIndex_ = {1UL};
    alignas(ark::COHERENCY_LINE_SIZE) std::atomic<size_t> pushIndex_ = {1UL};
};

}  // namespace ark::taskmanager::internal

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_SP_SC_LOCK_FREE_QUEUE_H
