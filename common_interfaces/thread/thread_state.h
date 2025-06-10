/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_THREAD_THREAD_STATE_H
#define COMMON_INTERFACES_THREAD_THREAD_STATE_H

#include <atomic>
#include <cstdint>

#include "base/common.h"

namespace common {

enum ThreadFlag : uint16_t {
    NO_FLAGS = 0 << 0,
    SUSPEND_REQUEST = 1 << 0,
    ACTIVE_BARRIER = 1 << 1,
};

constexpr uint32_t THREAD_STATE_OFFSET = 16;
constexpr uint32_t THREAD_FLAGS_MASK = (0x1 << THREAD_STATE_OFFSET) - 1;
enum class ThreadState : uint16_t {
    CREATED = 0,
    RUNNING = 1,
    NATIVE = 2,
    WAIT = 3,
    IS_SUSPENDED = 4,
    TERMINATED = 5,
};

union ThreadStateAndFlags {
    explicit ThreadStateAndFlags(uint32_t val = 0) : asInt(val) {}
    struct {
        volatile uint16_t flags;
        volatile ThreadState state;
    } asStruct;
    struct {
        uint16_t flags;
        ThreadState state;
    } asNonvolatileStruct;
    volatile uint32_t asInt;
    uint32_t asNonvolatileInt;
    std::atomic<uint32_t> asAtomicInt;

private:
    NO_COPY_SEMANTIC_CC(ThreadStateAndFlags);
};

class SuspendBarrier {
public:
    SuspendBarrier() : passBarrierCount_(0) {}

    explicit SuspendBarrier(int32_t count) : passBarrierCount_(count) {}

    void Wait();

    void PassStrongly();

    void Initialize(int32_t count)
    {
        passBarrierCount_.store(count, std::memory_order_relaxed);
    }

private:
    std::atomic<int32_t> passBarrierCount_;
};
}  // namespace common
#endif  // COMMON_INTERFACES_THREAD_THREAD_STATE_H
