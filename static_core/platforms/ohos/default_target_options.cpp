/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include <string>
#include <map>

#include "platforms/target_defaults/default_target_options.h"
#include "platforms/ohos/ohos_device_helpers.h"

namespace ark::default_target_options {

std::string GetTargetString()
{
    std::string model = ark::ohos_device::GetHardwareModelString();
#ifdef PANDA_TARGET_ARM32
    model = "arm32";
#endif
    return model;
}

bool GetCoverageEnable()
{
    return ark::ohos_device::GetCoverageEnable();
}

std::string GetCodeCoverageOutput()
{
    return ark::ohos_device::GetCodeCoverageOutput();
}

bool GetEnableDebugMode()
{
    return ark::ohos_device::GetEnableDebugMode();
}

std::string GetDebuggerLibraryPath()
{
    return ark::ohos_device::GetDebuggerLibraryPath();
}

}  // namespace ark::default_target_options
