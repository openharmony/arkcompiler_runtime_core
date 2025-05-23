/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "platforms/target_defaults/default_target_options.h"

namespace ark::default_target_options {

uint32_t GetGcWorkersCount(const std::map<std::string, uint32_t> &modelMap)
{
    return modelMap.at("default");
}

uint32_t GetTaskmanagerWorkersCount(const std::map<std::string, uint32_t> &modelMap)
{
    return modelMap.at("default");
}
}  // namespace ark::default_target_options
