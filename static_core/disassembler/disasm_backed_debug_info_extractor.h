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

#ifndef PANDA_DISASM_BACKED_DEBUG_INFO_EXTRACTOR_H_
#define PANDA_DISASM_BACKED_DEBUG_INFO_EXTRACTOR_H_

#include "disassembler.h"

namespace panda::disasm {
class PANDA_PUBLIC_API DisasmBackedDebugInfoExtractor : public panda_file::DebugInfoExtractor {
public:
    explicit DisasmBackedDebugInfoExtractor(
        const panda_file::File &file,
        std::function<void(panda_file::File::EntityId, std::string_view)> &&on_disasm_source_name = [](auto, auto) {});

    const panda_file::LineNumberTable &GetLineNumberTable(panda_file::File::EntityId method_id) const override;
    const panda_file::ColumnNumberTable &GetColumnNumberTable(panda_file::File::EntityId method_id) const override;
    const panda_file::LocalVariableTable &GetLocalVariableTable(panda_file::File::EntityId method_id) const override;
    const std::vector<ParamInfo> &GetParameterInfo(panda_file::File::EntityId method_id) const override;
    const char *GetSourceFile(panda_file::File::EntityId method_id) const override;
    const char *GetSourceCode(panda_file::File::EntityId method_id) const override;

private:
    struct Disassembly {
        std::string source_code;
        panda_file::LineNumberTable line_table;
        std::vector<ParamInfo> parameter_info;
        panda_file::LocalVariableTable local_variable_table;
    };

    const std::optional<std::string> &GetDisassemblySourceName(panda_file::File::EntityId method_id) const;
    const Disassembly &GetDisassembly(panda_file::File::EntityId method_id) const;

    const std::string source_name_prefix_;
    const std::function<void(panda_file::File::EntityId, std::string_view)> on_disasm_source_name_;

    mutable os::memory::Mutex mutex_;
    mutable Disassembler disassembler_ GUARDED_BY(mutex_);
    mutable std::unordered_map<panda_file::File::EntityId, std::optional<std::string>> source_names_ GUARDED_BY(mutex_);
    mutable std::unordered_map<panda_file::File::EntityId, Disassembly> disassemblies_ GUARDED_BY(mutex_);
};
}  // namespace panda::disasm

#endif  // PANDA_DISASM_BACKED_DEBUG_INFO_EXTRACTOR_H_
