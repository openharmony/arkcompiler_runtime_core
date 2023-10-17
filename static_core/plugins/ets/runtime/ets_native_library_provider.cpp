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

namespace panda::ets {
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

    auto load_res = EtsNativeLibrary::Load(name);
    if (!load_res) {
        return load_res.Error().ToString();
    }

    const EtsNativeLibrary *lib = nullptr;
    {
        os::memory::WriteLockHolder lock(lock_);

        auto [it, inserted] = libraries_.emplace(std::move(load_res.Value()));
        if (!inserted) {
            return {};
        }

        lib = &*it;
    }
    ASSERT(lib != nullptr);

    if (auto on_load_symbol = lib->FindSymbol("EtsNapiOnLoad")) {
        using EtsNapiOnLoadHandle = ets_int (*)(EtsEnv *);
        auto on_load_handle = reinterpret_cast<EtsNapiOnLoadHandle>(on_load_symbol.Value());
        ets_int ets_napi_version = on_load_handle(env);
        if (!napi::CheckVersionEtsNapi(ets_napi_version)) {
            return {"Unsupported Ets napi version " + std::to_string(ets_napi_version)};
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
}  // namespace panda::ets
