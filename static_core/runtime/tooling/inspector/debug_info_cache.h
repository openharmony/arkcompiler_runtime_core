/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_DEBUG_INFO_CACHE_H
#define PANDA_TOOLING_INSPECTOR_DEBUG_INFO_CACHE_H

#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "disassembler/disasm_backed_debug_info_extractor.h"
#include "include/method.h"
#include "include/typed_value.h"
#include "runtime/tooling/debugger.h"

#include "types/location.h"

namespace ark::tooling::inspector {
class DebugInfoCache final {
public:
    DebugInfoCache() = default;
    ~DebugInfoCache() = default;

    NO_COPY_SEMANTIC(DebugInfoCache);
    NO_MOVE_SEMANTIC(DebugInfoCache);

    void AddPandaFile(const panda_file::File &file);
    void GetSourceLocation(const PtFrame &frame, std::string_view &sourceFile, std::string_view &methodName,
                           size_t &lineNumber);
    std::unordered_set<PtLocation, HashLocation> GetCurrentLineLocations(const PtFrame &frame);
    std::unordered_set<PtLocation, HashLocation> GetContinueToLocations(std::string_view sourceFile, size_t lineNumber);
    std::vector<PtLocation> GetBreakpointLocations(const std::function<bool(std::string_view)> &sourceFileFilter,
                                                   size_t lineNumber, std::set<std::string_view> &sourceFiles);
    std::set<size_t> GetValidLineNumbers(std::string_view sourceFile, size_t startLine, size_t endLine,
                                         bool restrictToFunction);

    std::map<std::string, TypedValue> GetLocals(const PtFrame &frame);

    std::string GetSourceCode(std::string_view sourceFile);

private:
    const panda_file::DebugInfoExtractor &GetDebugInfo(const panda_file::File *file);

    template <typename PFF, typename MF, typename H>
    void EnumerateLineEntries(PFF &&pandaFileFilter, MF &&methodFilter, H &&handler)
    {
        os::memory::LockHolder lock(debugInfosMutex_);

        for (auto &[file, debugInfo] : debugInfos_) {
            if (!pandaFileFilter(file, debugInfo)) {
                continue;
            }

            for (auto methodId : debugInfo.GetMethodIdList()) {
                if (!methodFilter(file, debugInfo, methodId)) {
                    continue;
                }

                EnumerateLineEntries(file, debugInfo, methodId, std::forward<H>(handler));
            }
        }
    }

    template <typename H>
    void EnumerateLineEntries(const panda_file::File *file, const disasm::DisasmBackedDebugInfoExtractor &debugInfo,
                              panda_file::File::EntityId methodId, H &&handler) REQUIRES(debugInfosMutex_)
    {
        auto &table = debugInfo.GetLineNumberTable(methodId);
        for (auto it = table.begin(); it != table.end(); ++it) {
            auto next = it + 1;
            if (!handler(file, debugInfo, methodId, *it, next != table.end() ? &*next : nullptr)) {
                break;
            }
        }
    }

    os::memory::Mutex debugInfosMutex_;
    std::unordered_map<const panda_file::File *, disasm::DisasmBackedDebugInfoExtractor> debugInfos_
        GUARDED_BY(debugInfosMutex_);

    os::memory::Mutex disassembliesMutex_;
    std::unordered_map<std::string_view, std::pair<const panda_file::File &, panda_file::File::EntityId>> disassemblies_
        GUARDED_BY(disassembliesMutex_);
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_DEBUG_INFO_CACHE_H
