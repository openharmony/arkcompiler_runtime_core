/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_LOG_LOG_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_LOG_LOG_H

#include <cstdint>
#include <string>

#include <securec.h>

#include "common_components/base/time_utils.h"

#include "libarkbase/utils/logger.h"

#ifdef ENABLE_HILOG
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif
#endif

#if defined(ENABLE_HITRACE)
#include "hitrace_meter.h"
// CC-OFFNXT(G.PRE.02) code readability, standard log macro approach
#define OHOS_HITRACE(level, name, customArgs) HITRACE_METER_NAME_EX(level, HITRACE_TAG_ARK, name, customArgs)
#define OHOS_HITRACE_START(level, name, customArgs) StartTraceEx(level, HITRACE_TAG_ARK, name, customArgs)
#define OHOS_HITRACE_FINISH(level) FinishTraceEx(level, HITRACE_TAG_ARK)
#define OHOS_HITRACE_COUNT(level, name, count) CountTraceEx(level, HITRACE_TAG_ARK, name, count)
#else
#define OHOS_HITRACE(level, name, customArgs)
#define OHOS_HITRACE_START(level, name, customArgs)
#define OHOS_HITRACE_FINISH(level)
#define OHOS_HITRACE_COUNT(level, name, count)
#endif

namespace common_vm {

#define COMMON_PHASE_TIMER(...) Timer ARK_pt_##__LINE__(__VA_ARGS__)

std::string Pretty(uint64_t number) noexcept;

class Timer {
public:
    explicit Timer(const std::string pName) : name_(pName)
    {
        if (ark::Logger::IsLoggingOn(ark::Logger::Level::DEBUG, ark::Logger::Component::GC)) {
            startTime_ = TimeUtil::MicroSeconds();
        }
    }

    ~Timer()
    {
        if (ark::Logger::IsLoggingOn(ark::Logger::Level::DEBUG, ark::Logger::Component::GC)) {
            uint64_t stopTime = TimeUtil::MicroSeconds();
            uint64_t diffTime = stopTime - startTime_;
            LOG(DEBUG, GC) << name_ << " time: " << Pretty(diffTime) << "us";
        }
    }

private:
    std::string name_;
    uint64_t startTime_ = 0;
};
}  // namespace common_vm
#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_LOG_LOG_H
