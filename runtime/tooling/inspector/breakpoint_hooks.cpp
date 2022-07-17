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

#include "breakpoint_hooks.h"
#include "error.h"
#include "inspector.h"
#include "json_property.h"
#include "server.h"
#include "source_file.h"

#include "debug_info_extractor.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"
#include "utils/logger.h"
#include "utils/string_helpers.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <list>
#include <regex>
#include <set>
#include <string>

using namespace std::placeholders;  // NOLINT(google-build-using-namespace)
using panda::helpers::string::ParseInt;

namespace panda::tooling::inspector {
BreakpointHooks::BreakpointHooks(Inspector &inspector) : inspector_(inspector)
{
    inspector.GetServer().OnCall("Debugger.getPossibleBreakpoints",
                                 std::bind(&BreakpointHooks::GetPossibleBreakpoints, this, _1, _2));
    inspector.GetServer().OnCall("Debugger.removeBreakpoint", std::bind(&BreakpointHooks::RemoveBreakpoint, this, _2));
    inspector.GetServer().OnCall("Debugger.setBreakpoint",
                                 std::bind(&BreakpointHooks::SetBreakpointAtLocation, this, _1, _2));
    inspector.GetServer().OnCall("Debugger.setBreakpointByUrl",
                                 std::bind(&BreakpointHooks::SetBreakpointByUrl, this, _1, _2));
    inspector.GetServer().OnCall("Debugger.setBreakpointsActive",
                                 std::bind(&BreakpointHooks::SetBreakpointsActive, this, _2));
}

void BreakpointHooks::Breakpoint(PtThread thread, Method * /* method */, const PtLocation & /* location */)
{
    if (active_) {
        inspector_.SetPause(thread, "Breakpoint");
    }
}

void BreakpointHooks::GetPossibleBreakpoints(JsonObjectBuilder &result, const JsonObject &params)
{
    const std::string *scriptIdString;
    size_t startLine;
    size_t endLine = ~0U;
    bool restrictToFunction = false;

    if (!GetPropertyOrLog<JsonObject::StringT>(scriptIdString, params, "start", "scriptId") ||
        !GetPropertyOrLog<JsonObject::NumT>(startLine, params, "start", "lineNumber")) {
        return;
    }

    int scriptId;
    if (!ParseInt(*scriptIdString, &scriptId, 0)) {
        LOG(INFO, DEBUGGER) << "Invalid script id: " << *scriptIdString;
        return;
    }

    GetProperty<JsonObject::NumT>(endLine, params, "end", "lineNumber");
    GetProperty<JsonObject::BoolT>(restrictToFunction, params, "restrictToFunction");

    const panda_file::DebugInfoExtractor *debugInfo;

    if (auto sourceFile = inspector_.GetSourceManager().GetSourceFile(scriptId)) {
        debugInfo = &sourceFile->GetDebugInfo();
    } else {
        return;
    }

    std::set<size_t> lineNumbers;

    for (auto methodId : debugInfo->GetMethodIdList()) {
        auto &table = debugInfo->GetLineNumberTable(methodId);

        if (restrictToFunction &&
            (table.empty() || startLine < table.front().line - 1 || table.back().line - 1 < startLine)) {
            continue;
        }

        for (auto &entry : table) {
            size_t lineNumber = entry.line - 1;

            if (startLine <= lineNumber && lineNumber < endLine) {
                lineNumbers.insert(lineNumber);
            }
        }
    }

    result.AddProperty("array", [&](JsonArrayBuilder &array) {
        for (auto lineNumber : lineNumbers) {
            array.Add([&](JsonObjectBuilder &breakLocation) {
                breakLocation.AddProperty("scriptId", std::to_string(scriptId));
                breakLocation.AddProperty("lineNumber", lineNumber);
            });
        }
    });
}

void BreakpointHooks::RemoveBreakpoint(const JsonObject &params)
{
    const std::string *breakpointIdString;
    if (!GetPropertyOrLog<JsonObject::StringT>(breakpointIdString, params, "breakpointId")) {
        return;
    }

    int breakpointId;
    if (!ParseInt(*breakpointIdString, &breakpointId, 0)) {
        LOG(INFO, DEBUGGER) << "Invalid breakpoint id: " << *breakpointIdString;
        return;
    }

    auto [begin, end] = locations_.equal_range(breakpointId);
    std::for_each(begin, end,
                  [&](auto &entry) { HandleError(inspector_.GetDebugger().RemoveBreakpoint(entry.second)); });
    locations_.erase(begin, end);
}

bool BreakpointHooks::SetBreakpoint(size_t breakpointId, const SourceFile &sourceFile, size_t lineNumber)
{
    bool breakpointSet = false;

    sourceFile.EnumerateLocations(lineNumber, [&](auto location) {
        if (!HandleError(inspector_.GetDebugger().SetBreakpoint(location))) {
            return true;
        }

        locations_.emplace(breakpointId, location);
        breakpointSet = true;
        return false;
    });

    return breakpointSet;
}

void BreakpointHooks::SetBreakpointAtLocation(JsonObjectBuilder &result, const JsonObject &params)
{
    const std::string *scriptIdString;
    if (!GetPropertyOrLog<JsonObject::StringT>(scriptIdString, params, "location", "scriptId")) {
        return;
    }

    int scriptId;
    if (!ParseInt(*scriptIdString, &scriptId, 0)) {
        LOG(INFO, DEBUGGER) << "Invalid script id: " << *scriptIdString;
        return;
    }

    size_t lineNumber;
    if (!GetPropertyOrLog<JsonObject::NumT>(lineNumber, params, "location", "lineNumber")) {
        return;
    }

    auto sourceFile = inspector_.GetSourceManager().GetSourceFile(scriptId);
    if (sourceFile == nullptr) {
        return;
    }

    auto breakpointId = GenerateBreakpointId();
    SetBreakpoint(breakpointId, *sourceFile, lineNumber);

    result.AddProperty("breakpointId", std::to_string(breakpointId));
    result.AddProperty("actualLocation", [&](JsonObjectBuilder &location) {
        location.AddProperty("scriptId", std::to_string(scriptId));
        location.AddProperty("lineNumber", lineNumber);
    });
}

void BreakpointHooks::SetBreakpointByUrl(JsonObjectBuilder &result, const JsonObject &params)
{
    size_t lineNumber;
    if (!GetPropertyOrLog<JsonObject::NumT>(lineNumber, params, "lineNumber")) {
        return;
    }

    const std::string *url;
    std::list<const SourceFile *> sourceFiles;

    if (GetPropertyOrLog<JsonObject::StringT>(url, params, "url")) {
        if (url->find("file:///") != 0) {
            LOG(INFO, DEBUGGER) << "URL not supported: " << *url;
            return;
        }

        if (auto sourceFile = inspector_.GetSourceManager().GetSourceFile(url->substr(std::strlen("file:///")))) {
            sourceFiles.push_back(sourceFile);
        } else {
            return;
        }
    } else if (GetPropertyOrLog<JsonObject::StringT>(url, params, "urlRegex")) {
        std::regex regex(*url);

        inspector_.GetSourceManager().EnumerateSourceFiles([&](auto &sourceFile) {
            if (std::regex_match(sourceFile.GetFileName(), regex)) {
                sourceFiles.push_back(&sourceFile);
            }
            return true;
        });
    } else {
        return;
    }

    auto breakpointId = GenerateBreakpointId();
    result.AddProperty("breakpointId", std::to_string(breakpointId));

    for (auto sourceFile = sourceFiles.begin(); sourceFile != sourceFiles.end();) {
        if (SetBreakpoint(breakpointId, **sourceFile, lineNumber)) {
            ++sourceFile;
        } else {
            sourceFile = sourceFiles.erase(sourceFile);
        }
    }

    result.AddProperty("locations", [&](JsonArrayBuilder &locations) {
        for (auto sourceFile : sourceFiles) {
            locations.Add([&](JsonObjectBuilder &location) {
                location.AddProperty("scriptId", std::to_string(sourceFile->GetScriptId()));
                location.AddProperty("lineNumber", lineNumber);
            });
        }
    });
}

void BreakpointHooks::SetBreakpointsActive(const JsonObject &params)
{
    GetPropertyOrLog<JsonObject::BoolT>(active_, params, "active");
}
}  // namespace panda::tooling::inspector
