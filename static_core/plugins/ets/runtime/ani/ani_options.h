/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_H

#include <string>

#include "include/runtime_options.h"

namespace ark::ets::ani {

/**
 * @brief Class represents ANI options
 *
 * It extends RuntimeOptions to provide ANI users possibility to use
 * ANI specific options and runtime options while creating VM
 *
 */
class PANDA_PUBLIC_API ANIOptions : public RuntimeOptions {
public:
    explicit ANIOptions(const std::string &exePath = "") : RuntimeOptions(exePath)
    {
        SetLoadRuntimes({"ets"});
    }
};
}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_OPTIONS_H
