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
#ifndef PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_IMPL_H
#define PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_IMPL_H

#include <map>
#include "libpandabase/os/mutex.h"
#include <string>
#include <cstddef>

#include <libpandabase/macros.h>
#if defined(PANDA_TARGET_OHOS) && !defined(PANDA_CMAKE_SDK)
#include <dlfcn.h>
#endif
#include "os/library_loader.h"
#include "plugins/ets/runtime/ets_native_library.h"

namespace ark::ets {
class EtsNamespaceManagerImpl {
public:
    static EtsNamespaceManagerImpl &GetInstance();

    void CreateApplicationNs(const std::string &abcPath, const std::vector<std::string> &appLibPath);

    void CreateLdNamespace(const std::string &searchKey, const char *libLoadPath);

    Expected<EtsNativeLibrary, os::Error> LoadNativeLibraryFromNs(const std::string &pathKey, const char *name);

    virtual ~EtsNamespaceManagerImpl();

    NO_COPY_SEMANTIC(EtsNamespaceManagerImpl);

    NO_MOVE_SEMANTIC(EtsNamespaceManagerImpl);

private:
    EtsNamespaceManagerImpl() = default;
#if defined(PANDA_TARGET_OHOS) && !defined(PANDA_CMAKE_SDK)
    std::map<std::string, Dl_namespace> nsMap_;
#endif
    mutable os::memory::RWLock lock_;
    std::map<std::string, char *> appLibPathMap_ GUARDED_BY(lock_);
};
}  // namespace ark::ets
#endif  // !PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_IMPL_H