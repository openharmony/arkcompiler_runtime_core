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

#include "plugins/ets/runtime/ets_native_library_provider.h"

#include "ets_native_library.h"
#include "include/mem/panda_containers.h"
#include "include/mem/panda_string.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/napi/ets_napi_invoke_interface.h"

namespace ark::ets {

namespace {

Expected<EtsNativeLibrary, os::Error> LoadFromPath(const PandaVector<PandaString> &pathes, const PandaString &name)
{
    if (name.find('/') == std::string::npos) {
        for (const auto &path : pathes) {
            if (path.empty()) {
                continue;
            }
            PandaStringStream s;
            s << path << '/' << name;
            if (auto loadRes = EtsNativeLibrary::Load(s.str())) {
                return loadRes;
            }
        }
    }
    return EtsNativeLibrary::Load(name);
}

}  // namespace

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

    auto loadRes = LoadFromPath(GetLibraryPath(), name);
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
            return "Unsupported Ets napi version " + std::to_string(etsNapiVersion);
        }
    } else if (auto res = lib->FindSymbol("ANI_Constructor")) {
        using AniCtor = ani_status (*)(ani_vm *, uint32_t *);
        auto ctor = reinterpret_cast<AniCtor>(res.Value());
        uint32_t version;
        ani_vm *vm = PandaEnv::FromEtsEnv(env)->GetEtsVM();
        ani_status status = ctor(vm, &version);
        if (status != ANI_OK) {
            return "ANI_Constructor returns an error: " + std::to_string(status);
        }
        if (!ani::IsVersionSupported(version)) {
            return "Unsupported ANI version: " + std::to_string(version);
        }
    } else {
        // NODE: Return error "Native library doesn't have ANI_Constructor" when ets_napi will be dropped. #22232
        LOG(WARNING, ANI) << name << " doesn't contain ANI_Constructor";
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

PandaVector<PandaString> NativeLibraryProvider::GetLibraryPath() const
{
    os::memory::ReadLockHolder lock(lock_);
    return libraryPath_;
}

void NativeLibraryProvider::SetLibraryPath(const PandaVector<PandaString> &pathes)
{
    os::memory::WriteLockHolder lock(lock_);
    libraryPath_ = pathes;
}

void NativeLibraryProvider::AddLibraryPath(const PandaString &path)
{
    os::memory::WriteLockHolder lock(lock_);
    libraryPath_.emplace_back(path);
}

}  // namespace ark::ets
