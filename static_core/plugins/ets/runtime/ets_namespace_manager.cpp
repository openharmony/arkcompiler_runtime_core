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

#include <cstring>
#include "plugins/ets/runtime/ets_namespace_manager.h"
#include "plugins/ets/runtime/ets_namespace_manager_impl.h"

namespace ark::ets {

PANDA_PUBLIC_API void EtsNamespaceManager::SetAppLibPaths(const AppLibPathMap &appLibPaths)
{
    LOG(INFO, RUNTIME) << "set ets app library path";
    if (appLibPaths.empty()) {
        LOG(WARNING, RUNTIME) << "no ets app library path to set";
        return;
    }
    EtsNamespaceManagerImpl &instance = EtsNamespaceManagerImpl::GetInstance();
    for (const auto &appLibPath : appLibPaths) {
        instance.CreateApplicationNs(appLibPath.first, appLibPath.second);
    }
}

}  // namespace ark::ets