/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_NATIVE_LIBRARY_PROVIDER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_NATIVE_LIBRARY_PROVIDER_H_

#include <string>
#include "libpandabase/os/mutex.h"
#include "plugins/ets/runtime/ets_native_library.h"
#include "plugins/ets/runtime/napi/ets_napi.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::ets {
class NativeLibraryProvider {
public:
    NativeLibraryProvider() = default;
    ~NativeLibraryProvider() = default;

    NO_COPY_SEMANTIC(NativeLibraryProvider);
    NO_MOVE_SEMANTIC(NativeLibraryProvider);

    std::optional<std::string> LoadLibrary(EtsEnv *env, const PandaString &name, bool shouldVerifyPermission,
                                           const PandaString &fileName);
    void *ResolveSymbol(const PandaString &name) const;

    PandaVector<PandaString> GetLibraryPath() const;
    void SetLibraryPath(const PandaVector<PandaString> &pathes);
    void AddLibraryPath(const PandaString &path);

private:
    mutable os::memory::RWLock lock_;
    std::optional<std::string> CallAniCtor(EtsEnv *env, const EtsNativeLibrary *lib);
    std::optional<std::string> GetCallerClassName(EtsEnv *env);
    bool CheckLibraryPermission(EtsEnv *env, const PandaString &fileName);
    PandaSet<EtsNativeLibrary> libraries_ GUARDED_BY(lock_);
    PandaVector<PandaString> libraryPath_ GUARDED_BY(lock_);
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_NATIVE_LIBRARY_PROVIDER_H_
