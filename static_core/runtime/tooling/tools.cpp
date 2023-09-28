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

#include "runtime/tooling/tools.h"
#include "runtime/tooling/sampler/sampling_profiler.h"

namespace panda::tooling {

sampler::Sampler *Tools::GetSamplingProfiler()
{
    // Singleton instance
    return sampler_;
}

void Tools::CreateSamplingProfiler()
{
    ASSERT(sampler_ == nullptr);
    sampler_ = sampler::Sampler::Create();

    const char *sampler_segv_option = std::getenv("ARK_SAMPLER_DISABLE_SEGV_HANDLER");
    if (sampler_segv_option != nullptr) {
        std::string_view option = sampler_segv_option;
        if (option == "1" || option == "true" || option == "ON") {
            // SEGV handler for sampler is enable by default
            sampler_->SetSegvHandlerStatus(false);
        }
    }
}

bool Tools::StartSamplingProfiler(const std::string &aspt_filename, uint32_t interval)
{
    ASSERT(sampler_ != nullptr);
    sampler_->SetSampleInterval(interval);
    if (aspt_filename.empty()) {
        std::time_t current_time = std::time(nullptr);
        std::tm *local_time = std::localtime(&current_time);
        std::string aspt_filename_time = std::to_string(local_time->tm_hour) + "-" +
                                         std::to_string(local_time->tm_min) + "-" + std::to_string(local_time->tm_sec) +
                                         ".aspt";
        return sampler_->Start(aspt_filename_time.c_str());
    }
    return sampler_->Start(aspt_filename.c_str());
}

void Tools::StopSamplingProfiler()
{
    ASSERT(sampler_ != nullptr);
    sampler_->Stop();
    sampler::Sampler::Destroy(sampler_);
    sampler_ = nullptr;
}

}  // namespace panda::tooling
