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

#include "runtime_controller.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <sys/param.h>
#include <unistd.h>

#include "libpandabase/utils/logger.h"
#include "runtime/include/mem/panda_containers.h"

namespace panda {

/**
 * Data directory of applications for default user.
 */
constexpr std::string_view DIR_DATA_DATA = "/data/data/";

/**
 * Data directory of applications for non-default users.
 */
constexpr std::string_view DIR_DATA_USER = "/data/user/";

static bool StartsWith(std::string_view s, std::string_view prefix)
{
    return (s.size() >= prefix.size()) && std::equal(prefix.begin(), prefix.end(), s.begin());
}

static bool StartsWithData(std::string_view path)
{
    if (path.empty() || (path[0] != '/')) {
        return false;
    }
    return StartsWith(path, DIR_DATA_DATA) || StartsWith(path, DIR_DATA_USER);
}

static bool IsInPermitList(std::string_view path)
{
    size_t pos = path.rfind('/');
    if (pos == std::string::npos) {
        LOG(ERROR, RUNTIME) << "Failed to get file name from path: " << path;
        return false;
    }
    std::string_view file_name = path.substr(pos + 1U);
    return StartsWith(file_name, "HMS-Ohos-");
}

bool RuntimeController::CanLoadPandaFileInternal(std::string_view real_path) const
{
    return (!StartsWithData(real_path)) || IsInPermitList(real_path);
}

bool RuntimeController::CanLoadPandaFile(const std::string &path) const
{
    if (IsZidaneApp() && (!IsMultiFramework())) {
        // Avoid large frame.
        PandaVector<char> buffer(PATH_MAX, 0);
        if (realpath(path.c_str(), buffer.data()) == nullptr) {
            LOG(ERROR, RUNTIME) << "Failed to get realpath for " << path;
            return true;  // Allow loading panda file.
        }
        std::string_view real_path = buffer.data();
        bool allow = CanLoadPandaFileInternal(real_path);
        if (!allow) {
            LOG(WARNING, RUNTIME) << "Disallow loading panda file in data directory : " << path;
        }
        return allow;
    }
    return true;
}

}  // namespace panda
