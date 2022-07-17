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

#ifndef PANDA_TOOLING_INSPECTOR_SOURCE_MANAGER_H
#define PANDA_TOOLING_INSPECTOR_SOURCE_MANAGER_H

#include "source_file.h"

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>

namespace panda {
class JsonObject;
class JsonObjectBuilder;
class Method;
}  // namespace panda

namespace panda::panda_file {
class DebugInfoExtractor;
class File;
}  // namespace panda::panda_file

namespace panda::tooling::inspector {
class Server;

class SourceManager {
public:
    explicit SourceManager(Server &server);

    template <typename Function>
    void EnumerateSourceFiles(Function &&function) const
    {
        std::all_of(sourceFiles_.begin(), sourceFiles_.end(), std::forward<Function>(function));
    }

    // Get debug info associated with panda file, loading new source files if needed.
    const panda_file::DebugInfoExtractor &GetDebugInfo(const panda_file::File *pandaFile);

    // Get SourceFile associated with method, creating one if necessary.
    const SourceFile &GetOrLoadSourceFile(Method *method);

    // Query loaded source files.
    const SourceFile *GetSourceFile(size_t scriptId) const;
    const SourceFile *GetSourceFile(std::string_view fileName) const;

private:
    void GetScriptSource(JsonObjectBuilder &result, const JsonObject &params);
    const SourceFile *GetSourceFileInternal(std::string_view fileName) const;

    Server &server_;
    std::vector<SourceFile> sourceFiles_;
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_SOURCE_MANAGER_H
