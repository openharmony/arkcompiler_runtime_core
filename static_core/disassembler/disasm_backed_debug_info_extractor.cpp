/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "disasm_backed_debug_info_extractor.h"

namespace panda::disasm {
DisasmBackedDebugInfoExtractor::DisasmBackedDebugInfoExtractor(
    const panda_file::File &file,
    std::function<void(panda_file::File::EntityId, std::string_view)> &&on_disasm_source_name)
    : panda_file::DebugInfoExtractor(&file),
      source_name_prefix_("disasm://" + file.GetFilename() + ":"),
      on_disasm_source_name_(std::move(on_disasm_source_name))
{
    disassembler_.SetFile(file);
}

const panda_file::LineNumberTable &DisasmBackedDebugInfoExtractor::GetLineNumberTable(
    panda_file::File::EntityId method_id) const
{
    if (GetDisassemblySourceName(method_id)) {
        return GetDisassembly(method_id).line_table;
    }

    return DebugInfoExtractor::GetLineNumberTable(method_id);
}

const panda_file::ColumnNumberTable &DisasmBackedDebugInfoExtractor::GetColumnNumberTable(
    panda_file::File::EntityId /* method_id */) const
{
    static panda_file::ColumnNumberTable empty;
    return empty;
}

const panda_file::LocalVariableTable &DisasmBackedDebugInfoExtractor::GetLocalVariableTable(
    panda_file::File::EntityId method_id) const
{
    if (GetDisassemblySourceName(method_id)) {
        return GetDisassembly(method_id).local_variable_table;
    }

    return DebugInfoExtractor::GetLocalVariableTable(method_id);
}

const std::vector<panda_file::DebugInfoExtractor::ParamInfo> &DisasmBackedDebugInfoExtractor::GetParameterInfo(
    panda_file::File::EntityId method_id) const
{
    if (GetDisassemblySourceName(method_id)) {
        return GetDisassembly(method_id).parameter_info;
    }

    return DebugInfoExtractor::GetParameterInfo(method_id);
}

const char *DisasmBackedDebugInfoExtractor::GetSourceFile(panda_file::File::EntityId method_id) const
{
    if (auto &disassembly_source_name = GetDisassemblySourceName(method_id)) {
        return disassembly_source_name->c_str();
    }

    return DebugInfoExtractor::GetSourceFile(method_id);
}

const char *DisasmBackedDebugInfoExtractor::GetSourceCode(panda_file::File::EntityId method_id) const
{
    if (GetDisassemblySourceName(method_id)) {
        return GetDisassembly(method_id).source_code.c_str();
    }

    return DebugInfoExtractor::GetSourceCode(method_id);
}

const std::optional<std::string> &DisasmBackedDebugInfoExtractor::GetDisassemblySourceName(
    panda_file::File::EntityId method_id) const
{
    os::memory::LockHolder lock(mutex_);

    auto it = source_names_.find(method_id);
    if (it != source_names_.end()) {
        return it->second;
    }

    auto source_file = std::string(DebugInfoExtractor::GetSourceFile(method_id));
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_TARGET_MOBILE)
    if (!source_file.empty()) {
#else
    if (os::file::File::IsRegularFile(source_file)) {
#endif
        return source_names_.emplace(method_id, std::nullopt).first->second;
    }

    std::ostringstream name;
    name << source_name_prefix_ << method_id;
    auto &source_name = source_names_.emplace(method_id, name.str()).first->second;

    on_disasm_source_name_(method_id, *source_name);

    return source_name;
}

const DisasmBackedDebugInfoExtractor::Disassembly &DisasmBackedDebugInfoExtractor::GetDisassembly(
    panda_file::File::EntityId method_id) const
{
    os::memory::LockHolder lock(mutex_);

    auto it = disassemblies_.find(method_id);
    if (it != disassemblies_.end()) {
        return it->second;
    }

    pandasm::Function method("", SourceLanguage::PANDA_ASSEMBLY);
    disassembler_.GetMethod(&method, method_id);

    std::ostringstream source_code;
    panda_file::LineNumberTable line_table;
    disassembler_.Serialize(method, source_code, true, &line_table);

    auto params_num = method.GetParamsNum();
    std::vector<ParamInfo> parameter_info(params_num);
    for (auto argument_num = 0U; argument_num < params_num; ++argument_num) {
        parameter_info[argument_num].name = "a" + std::to_string(argument_num);
        parameter_info[argument_num].signature = method.params[argument_num].type.GetDescriptor();
    }

    // We use -1 here as a number for Accumulator, because there is no other way
    // to specify Accumulator as a register for local variable
    auto total_reg_num = method.GetTotalRegs();
    panda_file::LocalVariableTable local_variable_table(total_reg_num + 1, panda_file::LocalVariableInfo {});
    for (auto vreg_num = -1; vreg_num < static_cast<int32_t>(total_reg_num); ++vreg_num) {
        local_variable_table[vreg_num + 1].reg_number = vreg_num;
        local_variable_table[vreg_num + 1].name = vreg_num == -1 ? "acc" : "v" + std::to_string(vreg_num);
        local_variable_table[vreg_num + 1].start_offset = 0;
        local_variable_table[vreg_num + 1].end_offset = UINT32_MAX;
    }

    return disassemblies_
        .emplace(method_id, Disassembly {source_code.str(), std::move(line_table), std::move(parameter_info),
                                         std::move(local_variable_table)})
        .first->second;
}
}  // namespace panda::disasm