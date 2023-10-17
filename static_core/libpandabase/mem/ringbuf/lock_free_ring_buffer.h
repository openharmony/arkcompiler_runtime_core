/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef PANDA_LIBPANDABASE_MEM_RING_BUF_LOCK_FREE_BUFFER
#define PANDA_LIBPANDABASE_MEM_RING_BUF_LOCK_FREE_BUFFER

#include <atomic>
#include <cinttypes>
#include <thread>
#include "coherency_line_size.h"
#include "libpandabase/utils/math_helpers.h"

namespace panda::mem {
/// Lock-free single-producer single-consumer ring-buffer. Push can take infinite amount of time if buffer is full.
template <typename T, size_t RING_BUFFER_SIZE>
class LockFreeBuffer {
public:
    static_assert(RING_BUFFER_SIZE > 0U, "0 is invalid size for ring buffer");
    static_assert(panda::helpers::math::IsPowerOfTwo(RING_BUFFER_SIZE));
    static constexpr size_t RING_BUFFER_SIZE_MASK = RING_BUFFER_SIZE - 1;
    static_assert(RING_BUFFER_SIZE_MASK > 0);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    LockFreeBuffer()
    {
        // Atomic with release order reason: threads should see correct initialization
        tail_index_.store(0, std::memory_order_release);
        // Atomic with release order reason: threads should see correct initialization
        head_index_.store(0, std::memory_order_release);
        CheckInvariant();
    }

    bool TryPush(const T val)
    {
        CheckInvariant();
        // Atomic with acquire order reason: push should get the latest value
        const auto current_tail = tail_index_.load(std::memory_order_acquire);
        const auto next_tail = Increment(current_tail);
        // Atomic with acquire order reason: push should get the latest value
        auto local_head = head_index_.load(std::memory_order_acquire);
        if (next_tail != local_head) {
            ASSERT(current_tail < RING_BUFFER_SIZE);
            buffer_[current_tail] = val;
            // Atomic with release order reason: to allow pop to see the latest value
            tail_index_.store(next_tail, std::memory_order_release);
            return true;
        }
        return false;
    }

    void Push(const T val)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        while (!TryPush(val)) {
        };
    }

    bool IsEmpty()
    {
        CheckInvariant();
        // Atomic with acquire order reason: get the latest value
        auto local_head = head_index_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: get the latest value
        auto local_tail = tail_index_.load(std::memory_order_acquire);
        bool is_empty = (local_head == local_tail);
        return is_empty;
    }

    bool TryPop(T *pval)
    {
        CheckInvariant();

        // Atomic with acquire order reason: get the latest value
        auto current_head = head_index_.load(std::memory_order_acquire);

        // Atomic with acquire order reason: get the latest value
        if (current_head == tail_index_.load(std::memory_order_acquire)) {
            return false;
        }

        *pval = buffer_[current_head];
        size_t new_value = Increment(current_head);
        // Atomic with release order reason: let others threads to see the latest value
        head_index_.store(new_value, std::memory_order_release);
        return true;
    }

    T Pop()
    {
        T ret;
        // NOLINTNEXTLINE(readability-braces-around-statements)
        while (!TryPop(&ret)) {
        }
        return ret;
    }

    static constexpr uint32_t GetTailIndexOffset()
    {
        return MEMBER_OFFSET(Self, tail_index_);
    }

    static constexpr uint32_t GetHeadIndexOffset()
    {
        return MEMBER_OFFSET(Self, head_index_);
    }

    static constexpr uint32_t GetBufferOffset()
    {
        return MEMBER_OFFSET(Self, buffer_);
    }

private:
    using Self = LockFreeBuffer<T, RING_BUFFER_SIZE>;

    alignas(panda::COHERENCY_LINE_SIZE) std::atomic<size_t> tail_index_;
    std::atomic<size_t> head_index_;
    alignas(panda::COHERENCY_LINE_SIZE) std::array<T, RING_BUFFER_SIZE> buffer_;

    size_t Increment(size_t n)
    {
        return (n + 1) & RING_BUFFER_SIZE_MASK;
    }

    void CheckInvariant()
    {
#ifndef NDEBUG
        // Atomic with acquire order reason: get the latest value
        [[maybe_unused]] auto local_head = head_index_.load(std::memory_order_acquire);
        ASSERT(local_head < RING_BUFFER_SIZE);

        // Atomic with acquire order reason: get the latest value
        [[maybe_unused]] auto local_tail = tail_index_.load(std::memory_order_acquire);
        ASSERT(local_tail < RING_BUFFER_SIZE);
#endif
    }
};
}  // namespace panda::mem
#endif