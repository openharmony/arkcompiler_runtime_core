/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef RUNTIME_TRACE_H
#define RUNTIME_TRACE_H

#ifdef PANDA_USE_HITRACE
#include "hitrace_meter/hitrace_meter.h"
constexpr uint64_t TRACER_TAG = HITRACE_TAG_ARK;
#else
#include <cstdint>
#include <string>
constexpr uint64_t TRACER_TAG = 0ULL;

extern "C" {
inline void StartTrace([[maybe_unused]] uint64_t tag, [[maybe_unused]] const std::string &name,
                       [[maybe_unused]] float limit = -1)
{
}
inline void FinishTrace([[maybe_unused]] uint64_t tag) {}

inline void StartAsyncTrace([[maybe_unused]] uint64_t tag, [[maybe_unused]] const std::string &name,
                            [[maybe_unused]] int32_t taskId, [[maybe_unused]] float limit = -1)
{
}
inline void FinishAsyncTrace([[maybe_unused]] uint64_t tag, [[maybe_unused]] const std::string &name,
                             [[maybe_unused]] int32_t taskId)
{
}

inline void CountTrace([[maybe_unused]] uint64_t tag, [[maybe_unused]] const std::string &name,
                       [[maybe_unused]] int64_t count)
{
}
}
#endif

namespace ark {

class Tracer {
public:
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    inline static constexpr char LAUNCH[] = "Launch";
    inline static constexpr char COROUTINE_EXECUTION[] = "CoroutineExecution";
    inline static constexpr char WORKERS_EXPANSION[] = "WorkersExpansion";
    inline static constexpr char WORKERS_CONTRACTION[] = "WorkersContraction";
    // NOLINTEND(modernize-avoid-c-arrays)

    static void Start(const std::string &name, float limit = -1)
    {
        StartTrace(TRACER_TAG, name, limit);
    }

    static void Finish()
    {
        FinishTrace(TRACER_TAG);
    }

    static void StartAsync(const std::string &name, int32_t taskId, float limit = -1)
    {
        StartAsyncTrace(TRACER_TAG, name, taskId, limit);
    }

    static void FinishAsync(const std::string &name, int32_t taskId)
    {
        FinishAsyncTrace(TRACER_TAG, name, taskId);
    }

    static void Count(const std::string &name, int64_t count)
    {
        CountTrace(TRACER_TAG, name, count);
    }
};

}  // namespace ark

#endif  // RUNTIME_TRACE_H
