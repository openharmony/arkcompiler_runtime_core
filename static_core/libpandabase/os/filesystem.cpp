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

#include "os/filesystem.h"
#include "utils/logger.h"
#include <sys/types.h>
#include <sys/stat.h>
#if defined(PANDA_TARGET_WINDOWS)
#include <fileapi.h>
#endif

namespace panda::os {

void CreateDirectories(const std::string &folder_name)
{
#ifdef PANDA_TARGET_MOBILE
    constexpr auto DIR_PERMISSIONS = 0775;
    mkdir(folder_name.c_str(), DIR_PERMISSIONS);
#elif PANDA_TARGET_MACOS || PANDA_TARGET_OHOS
    std::filesystem::create_directories(std::filesystem::path(folder_name));
#elif PANDA_TARGET_WINDOWS
    CreateDirectory(folder_name.c_str(), NULL);
#else
    constexpr auto DIR_PERMISSIONS = 0775;
    auto status = mkdir(folder_name.c_str(), DIR_PERMISSIONS);
    if (status != 0) {
        LOG(WARNING, COMMON) << "Failed to create directory \"" << folder_name.c_str() << "\"\n";
        LOG(WARNING, COMMON) << "Return status :" << status << "\n";
    }
#endif
}

bool IsFileExists(const std::string &filepath)
{
    std::ifstream opened_file(filepath);
    return opened_file.good();
}

std::string GetParentDir(const std::string &filepath)
{
    size_t found = filepath.find_last_of("/\\");
    if (found == std::string::npos) {
        return std::string("");
    }
    return filepath.substr(0, found);
}

bool IsDirExists(const std::string &dirpath)
{
    if (dirpath == std::string("")) {
        return true;
    }
    struct stat info {};
    return (stat(dirpath.c_str(), &info) == 0) && ((info.st_mode & (unsigned int)S_IFDIR) != 0U);
}

}  // namespace panda::os
