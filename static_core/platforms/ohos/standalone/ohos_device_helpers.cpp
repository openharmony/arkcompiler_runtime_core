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

#include "platforms/ohos/ohos_device_helpers.h"

#include "libpandabase/os/library_loader.h"
#include "libpandabase/utils/logger.h"

namespace ark::ohos_device {
std::string GetHardwareModelString()
{
    auto libHandle = os::library_loader::Load("libbegetutil.z.so");
    if (!libHandle) {
        LOG(ERROR, COMMON) << "Failed to load libbegetutil.z.so";
        return std::string {};
    }

    auto getHardwareModelSymbol = os::library_loader::ResolveSymbol(libHandle.Value(), "GetHardwareModel");
    if (!getHardwareModelSymbol) {
        LOG(ERROR, COMMON) << "Failed to locate GetHardwareModel symbol in libbegetutil.z.so";
        return std::string {};
    }

    using GetHardwareModelHandle = const char *(*)();
    auto *getHardwareModelFunction = reinterpret_cast<GetHardwareModelHandle>(getHardwareModelSymbol.Value());
    return getHardwareModelFunction();
}
}  // namespace ark::ohos_device
