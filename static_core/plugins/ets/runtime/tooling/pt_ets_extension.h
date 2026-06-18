/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_ETS_INSPECTOR_EXTENSION_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_ETS_INSPECTOR_EXTENSION_H

#include "runtime/tooling/pt_default_lang_extension.h"

namespace ark::ets {
class PtEtsExtension : public tooling::PtStaticDefaultExtension {
public:
    explicit PtEtsExtension() : tooling::PtStaticDefaultExtension(panda_file::SourceLang::ETS) {}
    ~PtEtsExtension() override = default;

    NO_COPY_SEMANTIC(PtEtsExtension);
    NO_MOVE_SEMANTIC(PtEtsExtension);

    std::string_view GetThisParameterName() const override
    {
        return "=t";
    }

    bool CollectHybridStackFrames(const std::function<void(const void *frame, bool isStaticFrame)> &callback) override;

    size_t GetDynamicFrameInfo(const void *frame, void *buffer, size_t bufferSize, uintptr_t *fileId,
                               uint32_t *bcOffset) override;

    bool IsSupportHybridStack() override;
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_ETS_INSPECTOR_EXTENSION_H
