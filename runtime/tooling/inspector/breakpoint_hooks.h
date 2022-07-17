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

#ifndef PANDA_TOOLING_INSPECTOR_BREAKPOINT_HOOKS_H
#define PANDA_TOOLING_INSPECTOR_BREAKPOINT_HOOKS_H

#include "tooling/debug_interface.h"
#include "tooling/pt_location.h"

#include <cstddef>
#include <map>

namespace panda {
class JsonObject;
class JsonObjectBuilder;
}  // namespace panda

namespace panda::tooling::inspector {
class Inspector;
class SourceFile;

class BreakpointHooks : public PtHooks {
public:
    explicit BreakpointHooks(Inspector &inspector);

    void Breakpoint(PtThread thread, Method * /* method */, const PtLocation & /* location */) override;

private:
    void GetPossibleBreakpoints(JsonObjectBuilder &result, const JsonObject &params);
    void RemoveBreakpoint(const JsonObject &params);
    bool SetBreakpoint(size_t breakpointId, const SourceFile &sourceFile, size_t lineNumber);
    void SetBreakpointAtLocation(JsonObjectBuilder &result, const JsonObject &params);
    void SetBreakpointByUrl(JsonObjectBuilder &result, const JsonObject &params);
    void SetBreakpointsActive(const JsonObject &params);

    size_t GenerateBreakpointId()
    {
        return nextId_++;
    }

    Inspector &inspector_;
    bool active_ {true};
    std::multimap<size_t, PtLocation> locations_;
    size_t nextId_ {0};
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_BREAKPOINT_HOOKS_H
