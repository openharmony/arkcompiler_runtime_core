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

#ifndef PANDA_FILE_MANAGER_H_
#define PANDA_FILE_MANAGER_H_

#include "libpandafile/file.h"
#include "runtime/compiler.h"
#include "runtime/include/mem/panda_string.h"

namespace panda {

class FileManager {
public:
    static bool LoadAbcFile(std::string_view location, panda_file::File::OpenMode open_mode);

    static bool TryLoadAnFileForLocation(std::string_view panda_file_location);

    static Expected<bool, std::string> LoadAnFile(std::string_view an_location, bool force = false);

    static PandaString ResolveAnFilePath(std::string_view abc_path);
};

}  // namespace panda

#endif  // PANDA_FILE_MANAGER_H_
