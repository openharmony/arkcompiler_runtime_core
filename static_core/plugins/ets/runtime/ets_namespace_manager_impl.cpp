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
#include "plugins/ets/runtime/ets_namespace_manager_impl.h"
#include "libpandabase/utils/logger.h"

namespace ark::ets {

EtsNamespaceManagerImpl::~EtsNamespaceManagerImpl()
{
    os::memory::WriteLockHolder lock(lock_);
    for (const auto &item : appLibPathMap_) {
        delete[] item.second;
    }
    std::map<std::string, char *>().swap(appLibPathMap_);
}

void EtsNamespaceManagerImpl::CreateApplicationNs(const std::string &abcPath,
                                                  const std::vector<std::string> &appLibPath)
{
    LOG(INFO, RUNTIME) << "EtsNamespaceManagerImpl::CreateApplicationNs: ets app abcpath = " << abcPath.c_str();
    std::string tmpPath;
    for (const auto &path : appLibPath) {
        if (path.empty()) {
            continue;
        }
        if (!tmpPath.empty()) {
            tmpPath += ":";
        }
        tmpPath += path;
    }
    char *tmp = strdup(tmpPath.c_str());  // Consider using InternalAllocator instead of strdup -> issues#26703
    if (tmp == nullptr) {
        LOG(ERROR, RUNTIME) << "EtsNamespaceManagerImpl::CreateApplicationNs: strdup failed, tmpPath is null.";
        return;
    }
    os::memory::WriteLockHolder writeLock(lock_);
    if (appLibPathMap_.find(abcPath) != appLibPathMap_.end() && appLibPathMap_[abcPath] != nullptr) {
        delete[] appLibPathMap_[abcPath];
    }
    appLibPathMap_[abcPath] = tmp;
    CreateLdNamespace(abcPath, tmp);
    LOG(INFO, RUNTIME) << "EtsNamespaceManagerImpl::CreateApplicationNs: ets app lib path: " << appLibPathMap_[abcPath];
}

EtsNamespaceManagerImpl &EtsNamespaceManagerImpl::GetInstance()
{
    static EtsNamespaceManagerImpl instance;
    return instance;
}

void EtsNamespaceManagerImpl::CreateLdNamespace([[maybe_unused]] const std::string &searchKey,
                                                [[maybe_unused]] const char *libLoadPath)
{
#if defined(PANDA_TARGET_OHOS) && !defined(PANDA_CMAKE_SDK)
    Dl_namespace current_ns;
    Dl_namespace ns;
    std::string nsName = "pathNS_" + searchKey;
    dlns_init(&ns, nsName.c_str());
    dlns_get(nullptr, &current_ns);
    Dl_namespace ndk_ns;
    dlns_get("ndk", &ndk_ns);
    dlns_create2(&ns, libLoadPath, 0);
    dlns_set_namespace_separated(nsName.c_str(), true);
    dlns_set_namespace_permitted_paths(nsName.c_str(), libLoadPath);
    if (strlen(ndk_ns.name) > 0) {
        dlns_inherit(&ns, &ndk_ns, "allow_all_shared_libs");
        dlns_inherit(&ndk_ns, &current_ns, "allow_all_shared_libs");
        dlns_inherit(&current_ns, &ndk_ns, "allow_all_shared_libs");
    }
    nsMap_[searchKey] = ns;
    LOG(INFO, RUNTIME) << "EtsNamespaceManagerImpl::CreateLdNamespace searchKey :" << searchKey.c_str()
                       << " ets create a library loading path :" << libLoadPath;
#endif
}

Expected<EtsNativeLibrary, os::Error> EtsNamespaceManagerImpl::LoadNativeLibraryFromNs(const std::string &pathKey,
                                                                                       const char *name)
{
    LOG(INFO, RUNTIME) << "EtsNamespaceManagerImpl::LoadNativeLibraryFromNs pathKey :" << pathKey.c_str()
                       << " loading library name :" << name;
#if defined(PANDA_TARGET_OHOS) && !defined(PANDA_CMAKE_SDK)
    PandaString errInfo = "EtsNamespaceManagerImpl::LoadNativeLibraryFromNs: ";
    Dl_namespace ns;
    os::memory::ReadLockHolder lock(lock_);
    if (appLibPathMap_.find(pathKey) != appLibPathMap_.end()) {
        ns = nsMap_[pathKey];
    } else {
        if (appLibPathMap_.find("default") != appLibPathMap_.end()) {
            LOG(WARNING, RUNTIME)
                << "EtsNamespaceManagerImpl::LoadNativeLibraryFromNs: not  find appLibPath, use `pathNS_default` ns";
            ns = nsMap_["default"];
        } else {
            errInfo += "pathKey: " + pathKey + " and `default` not found in appLibPathMap_";
            return Unexpected(os::Error(errInfo.c_str()));
        }
    }
    void *nativeHandle = nullptr;
    nativeHandle = dlopen_ns(&ns, name, RTLD_LAZY);
    if (nativeHandle == nullptr) {
        char *dlerr = dlerror();
        auto dlerrMsg =
            dlerr != nullptr ? dlerr : "Error loading path " + std::string(name) + ":No such file or directory";
        errInfo += "load app library failed. " + std::string(dlerrMsg);
        return Unexpected(os::Error(errInfo.c_str()));
    }
    os::library_loader::LibraryHandle handle(nativeHandle);
    return EtsNativeLibrary(PandaString(name), std::move(handle));
#endif
    return Unexpected(os::Error("not support platform"));
}
}  // namespace ark::ets