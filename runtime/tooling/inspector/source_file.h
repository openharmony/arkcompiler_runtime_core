/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_SOURCE_FILE_H
#define PANDA_TOOLING_INSPECTOR_SOURCE_FILE_H

#include "debug_info_extractor.h"

#include <cstddef>
#include <functional>
#include <string>

namespace panda::panda_file {
class File;
}  // namespace panda::panda_file

namespace panda::tooling {
class PtLocation;
}  // namespace panda::tooling

namespace panda::tooling::inspector {
class SourceFile {
public:
    SourceFile(size_t scriptId, const char *fileName, const panda_file::File *pandaFile,
               panda_file::DebugInfoExtractor debugInfo);

    void EnumerateLocations(size_t lineNumber, const std::function<bool(PtLocation)> &function) const;

    size_t GetScriptId() const
    {
        return scriptId_;
    }

    const std::string &GetFileName() const
    {
        return fileName_;
    }

    const panda_file::File *GetPandaFile() const
    {
        return pandaFile_;
    }

    const panda_file::DebugInfoExtractor &GetDebugInfo() const
    {
        return debugInfo_;
    }

    const std::string &GetSourceCode() const
    {
        return sourceCode_;
    }

private:
    size_t scriptId_;
    std::string fileName_;
    const panda_file::File *pandaFile_;
    panda_file::DebugInfoExtractor debugInfo_;
    std::string sourceCode_;
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_SOURCE_FILE_H
