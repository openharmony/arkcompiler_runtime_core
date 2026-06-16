/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TOOLING_THREAD_SAMPLING_INFO_H
#define PANDA_RUNTIME_TOOLING_THREAD_SAMPLING_INFO_H

#include <atomic>
#include <csetjmp>
#include <cstdint>
#include "libarkbase/macros.h"

namespace ark::tooling::sampler {

class ThreadSamplingInfo {
public:
    ThreadSamplingInfo() = default;
    ~ThreadSamplingInfo() = default;

    NO_COPY_SEMANTIC(ThreadSamplingInfo);
    NO_MOVE_SEMANTIC(ThreadSamplingInfo);

    bool IsThreadSampling() const
    {
        return isThreadSampling_;
    }

    void SetThreadSampling(bool threadSampling)
    {
        isThreadSampling_ = threadSampling;
    }

    jmp_buf &GetSigSegvJmpEnv()
    {
        return sigsegvJmpEnv_;
    }

    // Plugin slot index assigned by Sampler (index into Sampler::pluginSlots_).
    // -1 means no slot allocated (thread not yet registered via ThreadStart).
    int32_t GetPluginSlotIndex() const
    {
        // Atomic with acquire order reason: Ensure slot data is visible before reading slot index
        return pluginSlotIndex_.load(std::memory_order_acquire);
    }

    void SetPluginSlotIndex(int32_t slotIndex)
    {
        // Atomic with release order reason: Ensure slot index is visible to SIGPROF handler before it reads it
        pluginSlotIndex_.store(slotIndex, std::memory_order_release);
    }

private:
    bool isThreadSampling_ {false};

    // Environment that saved by setjmp in SIGPROF handler in Sampler
    // Used for longjmp in case of SIGSEGV during thread sampling
    jmp_buf sigsegvJmpEnv_ {};

    // Index into Sampler::pluginSlots_. Set by AllocateSlotForThread (ThreadStart),
    // read by SIGPROF handler on the target thread. -1 = unregistered.
    std::atomic<int32_t> pluginSlotIndex_ {-1};
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_THREAD_SAMPLING_INFO_H
