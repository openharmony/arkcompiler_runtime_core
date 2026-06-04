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

#ifndef PANDA_RUNTIME_TOOLING_TOOLS_H
#define PANDA_RUNTIME_TOOLING_TOOLS_H

#include <memory>
#include "libarkbase/macros.h"

namespace ark {
class Runtime;
}  // namespace ark

namespace ark::tooling {
class CoverageListener;

namespace sampler {
class Sampler;
class StreamWriter;
}  // namespace sampler

enum class DebugSessionAttachErrorCode {
    OK = 0,
    ALREADY_ATTACHED,
    NOT_ALLOWED,
    INVALID,
};

class Tools final {
public:
    Tools() = default;
    ~Tools() = default;

    void CreateSamplingProfiler();
    sampler::Sampler *GetSamplingProfiler();
    PANDA_PUBLIC_API bool StartSamplingProfiler(std::unique_ptr<sampler::StreamWriter> streamWriter, uint32_t interval);
    PANDA_PUBLIC_API void StopSamplingProfiler();
    void DestroySamplingProfiler();
    bool IsSamplingProfilerCreate();

    void CreateCoverageListener(const std::string &filePath);
    CoverageListener *GetCoverageListener();
    void DestroyCoverageListener();

    /**
     * @brief Creates a new debug session for a running application.
     * This function creates a new debug session and prepares runtime for execution with debug events.
     * Pre-conditions:
     *  This function must be invoked from a native thread.
     *  JIT must be globally disabled.
     */
    DebugSessionAttachErrorCode AttachDebugSession();

    bool IsDebugSessionAttachAllowed() const;

private:
    NO_COPY_SEMANTIC(Tools);
    NO_MOVE_SEMANTIC(Tools);

    sampler::Sampler *sampler_ {nullptr};
    CoverageListener *coverageListener_ {nullptr};
};

}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_TOOLS_H
