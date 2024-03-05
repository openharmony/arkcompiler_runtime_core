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

#include "abc_code_converter.h"
#include <filesystem>

namespace panda::abc2program {

std::string AbcCodeConverter::IDToString(BytecodeInstruction bc_ins, panda_file::File::EntityId method_id,
                                         size_t idx) const
{
    std::stringstream name;
    const auto offset = abc_file_->ResolveOffsetByIndex(method_id, bc_ins.GetId(idx).AsIndex());
    std::string str_data = abc_string_table_.GetStringById(offset);
    if (bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::METHOD_ID)) {
        LOG(ERROR, ABC2PROGRAM) << "> not support for BytecodeInstruction::Flags::METHOD_ID now: <" << ">";
    } else if (bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::STRING_ID)) {
        name << '\"';
        name << str_data;
        name << '\"';
    } else {
        ASSERT(bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::LITERALARRAY_ID));
        std::string abc_file_abs_path = std::filesystem::absolute(abc_file_->GetFilename());
        name << abc_file_abs_path << ":" << offset;
    }
    return name.str();
}

}  // namespace panda::abc2program