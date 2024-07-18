/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "runtime/tooling/tools.h"
#include "runtime/tooling/sampler/sampling_profiler.h"

namespace ark::tooling {

sampler::Sampler *Tools::GetSamplingProfiler()
{
    // Singleton instance
    return sampler_;
}

void Tools::CreateSamplingProfiler()
{
    ASSERT(sampler_ == nullptr);
    sampler_ = sampler::Sampler::Create();

    const char *samplerSegvOption = std::getenv("ARK_SAMPLER_DISABLE_SEGV_HANDLER");
    if (samplerSegvOption != nullptr) {
        std::string_view option = samplerSegvOption;
        if (option == "1" || option == "true" || option == "ON") {
            // SEGV handler for sampler is enable by default
            sampler_->SetSegvHandlerStatus(false);
        }
    }
}

bool Tools::StartSamplingProfiler(const std::string &asptFilename, uint32_t interval)
{
    ASSERT(sampler_ != nullptr);
    sampler_->SetSampleInterval(interval);
    if (asptFilename.empty()) {
        std::time_t currentTime = std::time(nullptr);
        std::tm *localTime = std::localtime(&currentTime);
        std::string asptFilenameTime = std::to_string(localTime->tm_hour) + "-" + std::to_string(localTime->tm_min) +
                                       "-" + std::to_string(localTime->tm_sec) + ".aspt";
        return sampler_->Start(asptFilenameTime.c_str());
    }
    return sampler_->Start(asptFilename.c_str());
}

void Tools::StopSamplingProfiler()
{
    ASSERT(sampler_ != nullptr);
    sampler_->Stop();
    sampler::Sampler::Destroy(sampler_);
    sampler_ = nullptr;
}

}  // namespace ark::tooling
