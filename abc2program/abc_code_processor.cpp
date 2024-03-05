/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "abc_code_processor.h"
#include <iostream>

namespace panda::abc2program {

AbcCodeProcessor::AbcCodeProcessor(panda_file::File::EntityId entity_id, const panda_file::File &abc_file,
                                   AbcStringTable &abc_string_table, panda_file::File::EntityId method_id)
    : AbcFileEntityProcessor(entity_id, abc_file, abc_string_table), method_id_(method_id)
{
    code_data_accessor_ = std::make_unique<panda_file::CodeDataAccessor>(abc_file_, entity_id_);
    abc_code_converter_ = std::make_unique<AbcCodeConverter>(&abc_file_, abc_string_table);
    FillUpProgramData();
}

void AbcCodeProcessor::FillUpProgramData()
{
    const auto ins_size = code_data_accessor_->GetCodeSize();
    const auto ins_arr = code_data_accessor_->GetInstructions();
    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size);
    size_t reg_nums = code_data_accessor_->GetNumVregs();
    while (bc_ins.GetAddress() != bc_ins_last.GetAddress()) {
        pandasm::Ins pa_ins = abc_code_converter_->BytecodeInstructionToPandasmInstruction(bc_ins, method_id_);
        ins.emplace_back(pa_ins);
        bc_ins = bc_ins.GetNext();
        std::string ins_str = pa_ins.ToString("", true, reg_nums);
        std::cout << ins_str << std::endl;
    }
}

} // namespace panda::abc2program
