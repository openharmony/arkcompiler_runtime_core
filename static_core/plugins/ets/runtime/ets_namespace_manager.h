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
#ifndef PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_H
#define PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_H

#include <map>
#include <string>
#include <libpandabase/macros.h>
#include <functional>
using CreateNamespaceCallback = std::function<bool(const std::string &bundleModuleName, std::string &namespaceName)>;

// key is abcPath, value is namespaceName
using AppBundleModuleNamePathMap = std::map<std::string, std::string>;
namespace ark::ets {
class PANDA_PUBLIC_API EtsNamespaceManager {
public:
    static void SetAppLibPaths(const AppBundleModuleNamePathMap &appModuleNames, CreateNamespaceCallback &cb);
    ~EtsNamespaceManager() = default;

    NO_COPY_SEMANTIC(EtsNamespaceManager);
    NO_MOVE_SEMANTIC(EtsNamespaceManager);

private:
    EtsNamespaceManager() = default;
};
}  // namespace ark::ets
#endif  // !PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_H