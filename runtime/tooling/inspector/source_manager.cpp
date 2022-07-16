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

#include "source_manager.h"
#include "json_property.h"
#include "server.h"

#include "debug_info_extractor.h"
#include "macros.h"
#include "method.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"
#include "utils/logger.h"
#include "utils/string_helpers.h"

#include <algorithm>
#include <functional>
#include <string>

using namespace std::literals::string_literals;  // NOLINT(google-build-using-namespace)
using namespace std::placeholders;               // NOLINT(google-build-using-namespace)
using panda::helpers::string::ParseInt;

namespace panda::tooling::inspector {
SourceManager::SourceManager(Server &server) : server_(server)
{
    server.OnCall("Debugger.getScriptSource", std::bind(&SourceManager::GetScriptSource, this, _1, _2));
}

const panda_file::DebugInfoExtractor &SourceManager::GetDebugInfo(const panda_file::File *pandaFile)
{
    for (auto &sourceFile : sourceFiles_) {
        if (sourceFile.GetPandaFile() == pandaFile) {
            return sourceFile.GetDebugInfo();
        }
    }

    panda_file::DebugInfoExtractor debugInfo(pandaFile);

    for (auto methodId : debugInfo.GetMethodIdList()) {
        auto fileName = debugInfo.GetSourceFile(methodId);
        if (GetSourceFileInternal(fileName) != nullptr) {
            continue;
        }

        auto &sourceFile = sourceFiles_.emplace_back(sourceFiles_.size(), fileName, pandaFile, debugInfo);
        auto &sourceCode = sourceFile.GetSourceCode();

        auto endLine = std::count(sourceCode.begin(), sourceCode.end(), '\n') - 1;
        auto endColumn = sourceCode.size() - sourceCode.rfind('\n', sourceCode.size() - 2) - 2;

        server_.Call("Debugger.scriptParsed", [&](auto &params) {
            params.AddProperty("executionContextId", 0);
            params.AddProperty("scriptId", std::to_string(sourceFile.GetScriptId()));
            params.AddProperty("url", "file:///"s + fileName);
            params.AddProperty("startLine", 0);
            params.AddProperty("startColumn", 1);
            params.AddProperty("endLine", endLine);
            params.AddProperty("endColumn", endColumn);
            params.AddProperty("hash", "");
        });
    }

    ASSERT(!sourceFiles_.empty());
    CHECK_EQ(sourceFiles_.back().GetPandaFile(), pandaFile);
    return sourceFiles_.back().GetDebugInfo();
}

const SourceFile &SourceManager::GetOrLoadSourceFile(Method *method)
{
    return *GetSourceFileInternal(GetDebugInfo(method->GetPandaFile()).GetSourceFile(method->GetFileId()));
}

void SourceManager::GetScriptSource(JsonObjectBuilder &result, const JsonObject &params)
{
    const std::string *scriptIdString;
    if (!GetPropertyOrLog<JsonObject::StringT>(scriptIdString, params, "scriptId")) {
        return;
    }

    int scriptId;
    if (!ParseInt(*scriptIdString, &scriptId, 0)) {
        LOG(INFO, DEBUGGER) << "Invalid script id: " << *scriptIdString;
        return;
    }

    if (auto sourceFile = GetSourceFile(scriptId)) {
        result.AddProperty("scriptSource", sourceFile->GetSourceCode());
    }
}

const SourceFile *SourceManager::GetSourceFile(size_t scriptId) const
{
    for (auto &sourceFile : sourceFiles_) {
        if (sourceFile.GetScriptId() == scriptId) {
            return &sourceFile;
        }
    }
    LOG(INFO, DEBUGGER) << "No file with script id " << scriptId;
    return nullptr;
}

const SourceFile *SourceManager::GetSourceFile(std::string_view fileName) const
{
    if (auto sourceFile = GetSourceFileInternal(fileName)) {
        return sourceFile;
    }
    LOG(INFO, DEBUGGER) << "File not found: " << fileName;
    return nullptr;
}

const SourceFile *SourceManager::GetSourceFileInternal(std::string_view fileName) const
{
    for (auto &sourceFile : sourceFiles_) {
        if (sourceFile.GetFileName() == fileName) {
            return &sourceFile;
        }
    }
    return nullptr;
}
}  // namespace panda::tooling::inspector
