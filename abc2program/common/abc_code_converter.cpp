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
#include "os/file.h"

namespace panda::abc2program {

AbcCodeConverter::AbcCodeConverter(Abc2ProgramKeyData &key_data) : key_data_(key_data)
{
    file_ = &(key_data_.GetAbcFile());
    string_table_ = &(key_data_.GetAbcStringTable());
}

std::string AbcCodeConverter::IDToString(BytecodeInstruction bc_ins, panda_file::File::EntityId method_id,
                                         size_t idx) const
{
    std::stringstream name;
    const auto offset = file_->ResolveOffsetByIndex(method_id, bc_ins.GetId(idx).AsIndex());
    if (bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::METHOD_ID)) {
        LOG(ERROR, ABC2PROGRAM) << "> not support for BytecodeInstruction::Flags::METHOD_ID now: <" << ">";
    } else if (bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::STRING_ID)) {
        std::string str_data = string_table_->GetStringById(offset);
        string_table_->AddStringId(offset);
        name << '\"';
        name << str_data;
        name << '\"';
    } else {
        ASSERT(bc_ins.IsIdMatchFlag(idx, BytecodeInstruction::Flags::LITERALARRAY_ID));
        std::string file_abs_path = os::file::File::GetAbsolutePath(file_->GetFilename()).Value();
        name << file_abs_path << ":" << offset;
    }
    return name.str();
}

}  // namespace panda::abc2program