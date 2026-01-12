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

#include "platforms/ohos/ohos_device_helpers.h"

#include "syspara/parameter.h"
#include "syspara/parameters.h"

namespace ark::ohos_device {
std::string GetHardwareModelString()
{
    return GetHardwareModel();
}

bool GetCoverageEnable()
{
    return OHOS::system::GetBoolParameter("persist.ark.static.codecoverage.enable", false);
}

std::string GetCodeCoverageOutput()
{
    return "/data/storage/el2/base/files/coverageBytecodeInfo.csv";
}

bool GetEnableDebugMode()
{
    return OHOS::system::GetBoolParameter("persist.ark.enableDebugMode", false);
}

std::string GetDebuggerLibraryPath()
{
#if defined(PANDA_TARGET_ARM32)
    return "/system/lib/libarkinspector.so";
#else
    return "/system/lib64/libarkinspector.so";
#endif
}
}  // namespace ark::ohos_device