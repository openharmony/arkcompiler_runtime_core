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

#ifndef PANDA_TOOLING_INSPECTOR_DEBUG_INFO_CACHE_H
#define PANDA_TOOLING_INSPECTOR_DEBUG_INFO_CACHE_H

#include "disassembler/disasm_backed_debug_info_extractor.h"

#include "method.h"
#include "tooling/debugger.h"
#include "runtime/include/typed_value.h"

#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace panda::tooling::inspector {
class DebugInfoCache final {
public:
    DebugInfoCache() = default;
    ~DebugInfoCache() = default;

    NO_COPY_SEMANTIC(DebugInfoCache);
    NO_MOVE_SEMANTIC(DebugInfoCache);

    void AddPandaFile(const panda_file::File &file);
    void GetSourceLocation(const PtFrame &frame, std::string_view &source_file, std::string_view &method_name,
                           size_t &line_number);
    std::unordered_set<PtLocation, HashLocation> GetCurrentLineLocations(const PtFrame &frame);
    std::unordered_set<PtLocation, HashLocation> GetContinueToLocations(std::string_view source_file,
                                                                        size_t line_number);
    std::vector<PtLocation> GetBreakpointLocations(const std::function<bool(std::string_view)> &source_file_filter,
                                                   size_t line_number, std::set<std::string_view> &source_files);
    std::set<size_t> GetValidLineNumbers(std::string_view source_file, size_t start_line, size_t end_line,
                                         bool restrict_to_function);

    std::map<std::string, TypedValue> GetLocals(const PtFrame &frame);

    std::string GetSourceCode(std::string_view source_file);

private:
    const panda_file::DebugInfoExtractor &GetDebugInfo(const panda_file::File *file);

    template <typename PFF, typename MF, typename H>
    void EnumerateLineEntries(PFF &&panda_file_filter, MF &&method_filter, H &&handler)
    {
        os::memory::LockHolder lock(debug_infos_mutex_);

        for (auto &[file, debug_info] : debug_infos_) {
            if (!panda_file_filter(file, debug_info)) {
                continue;
            }

            for (auto method_id : debug_info.GetMethodIdList()) {
                if (!method_filter(file, debug_info, method_id)) {
                    continue;
                }

                auto &table = debug_info.GetLineNumberTable(method_id);
                for (auto it = table.begin(); it != table.end(); ++it) {
                    auto next = it + 1;
                    if (!handler(file, debug_info, method_id, *it, next != table.end() ? &*next : nullptr)) {
                        break;
                    }
                }
            }
        }
    }

    os::memory::Mutex debug_infos_mutex_;
    std::unordered_map<const panda_file::File *, disasm::DisasmBackedDebugInfoExtractor> debug_infos_
        GUARDED_BY(debug_infos_mutex_);

    os::memory::Mutex disassemblies_mutex_;
    std::unordered_map<std::string_view, std::pair<const panda_file::File &, panda_file::File::EntityId>> disassemblies_
        GUARDED_BY(disassemblies_mutex_);
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_DEBUG_INFO_CACHE_H
