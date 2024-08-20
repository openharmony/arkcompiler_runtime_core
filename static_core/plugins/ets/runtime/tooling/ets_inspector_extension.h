/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "runtime/tooling/default_inspector_extension.h"

namespace ark::ets {
class EtsInspectorExtension : public tooling::StaticDefaultInspectorExtension {
public:
    explicit EtsInspectorExtension() : tooling::StaticDefaultInspectorExtension(panda_file::SourceLang::ETS) {}
    ~EtsInspectorExtension() override = default;

    NO_COPY_SEMANTIC(EtsInspectorExtension);
    NO_MOVE_SEMANTIC(EtsInspectorExtension);

    std::string_view GetThisParameterName() const override
    {
        return "=t";
    }
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_ETS_INSPECTOR_EXTENSION_H
