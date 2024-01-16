/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_native_library_provider.h"

#include "plugins/ets/runtime/napi/ets_napi_invoke_interface.h"

namespace ark::ets {
std::optional<std::string> NativeLibraryProvider::LoadLibrary(EtsEnv *env, const PandaString &name)
{
    {
        os::memory::ReadLockHolder lock(lock_);

        auto it =
            std::find_if(libraries_.begin(), libraries_.end(), [&name](auto &lib) { return lib.GetName() == name; });
        if (it != libraries_.end()) {
            return {};
        }
    }

    auto loadRes = EtsNativeLibrary::Load(name);
    if (!loadRes) {
        return loadRes.Error().ToString();
    }

    const EtsNativeLibrary *lib = nullptr;
    {
        os::memory::WriteLockHolder lock(lock_);

        auto [it, inserted] = libraries_.emplace(std::move(loadRes.Value()));
        if (!inserted) {
            return {};
        }

        lib = &*it;
    }
    ASSERT(lib != nullptr);

    if (auto onLoadSymbol = lib->FindSymbol("EtsNapiOnLoad")) {
        using EtsNapiOnLoadHandle = ets_int (*)(EtsEnv *);
        auto onLoadHandle = reinterpret_cast<EtsNapiOnLoadHandle>(onLoadSymbol.Value());
        ets_int etsNapiVersion = onLoadHandle(env);
        if (!napi::CheckVersionEtsNapi(etsNapiVersion)) {
            return {"Unsupported Ets napi version " + std::to_string(etsNapiVersion)};
        }
    }

    return {};
}

void *NativeLibraryProvider::ResolveSymbol(const PandaString &name) const
{
    os::memory::ReadLockHolder lock(lock_);

    for (auto &lib : libraries_) {
        if (auto ptr = lib.FindSymbol(name)) {
            return ptr.Value();
        }
    }

    return nullptr;
}
}  // namespace ark::ets
