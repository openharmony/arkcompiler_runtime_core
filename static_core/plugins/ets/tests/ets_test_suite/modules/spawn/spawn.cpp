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

#include <sstream>

#include "libpandabase/macros.h"
#include "libpandabase/os/system_environment.h"
#include "plugins/ets/runtime/napi/ets_napi.h"

namespace {

std::vector<ets_string> SplitString(EtsEnv *env, std::string_view from, char delim)
{
    std::vector<ets_string> list;
    std::istringstream iss {from.data()};
    std::string part;
    while (std::getline(iss, part, delim)) {
        list.emplace_back(env->NewStringUTF(part.data()));
    }
    return list;
}

}  // namespace

extern "C" {
// NOLINTNEXTLINE(readability-identifier-naming)
ETS_EXPORT ets_objectArray ETS_CALL getAppAbcFiles(EtsEnv *env, [[maybe_unused]] ets_class /* unused */)
{
    auto appAbcFiles = ark::os::system_environment::GetEnvironmentVar("APP_ABC_FILES");
    const auto paths = SplitString(env, appAbcFiles, ':');
    ASSERT(!paths.empty());

    auto stringClass = env->FindClass("std/core/String");
    ASSERT(stringClass != nullptr);
    auto pathsArray = env->NewObjectsArray(paths.size(), stringClass, paths[0]);
    for (size_t i = 0; i < paths.size(); ++i) {
        env->SetObjectArrayElement(pathsArray, i, paths[i]);
    }
    return pathsArray;
}

ETS_EXPORT ets_int ETS_CALL EtsNapiOnLoad(EtsEnv *env)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    EtsNativeMethod method {"getAppAbcFiles", ":[Lstd/core/String;", reinterpret_cast<void *>(getAppAbcFiles)};

    ets_class spawnGlobalClass = env->FindClass("@spawn/spawn/ETSGLOBAL");
    ASSERT(spawnGlobalClass != nullptr);
    [[maybe_unused]] auto status = env->RegisterNatives(spawnGlobalClass, &method, 1);
    ASSERT(status == ETS_OK);
    return ETS_NAPI_VERSION_1_0;
}
}
