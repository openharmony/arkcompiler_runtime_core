/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "verifier.h"

#include "zlib.h"

#include "class_data_accessor-inl.h"
#include "code_data_accessor-inl.h"
#include "method_data_accessor-inl.h"


namespace panda::verifier {

Verifier::Verifier(const std::string &filename)
{
    auto file_to_verify = panda_file::File::Open(filename);
    file_.swap(file_to_verify);
}

bool Verifier::Verify()
{
    if (!VerifyChecksum()) {
        return false;
    }

    if (!VerifyConstantPool()) {
        return false;
    }

    return true;
}

bool Verifier::VerifyChecksum()
{
    if (file_ == nullptr) {
        return false;
    }
    uint32_t file_size = file_->GetHeader()->file_size;
    ASSERT(file_size > FILE_CONTENT_OFFSET);
    uint32_t cal_checksum = adler32(1, file_->GetBase() + FILE_CONTENT_OFFSET, file_size - FILE_CONTENT_OFFSET);
    return file_->GetHeader()->checksum == cal_checksum;
}

bool Verifier::VerifyConstantPool()
{
    if (file_ == nullptr) {
        return false;
    }

    GetMethodIds();
    GetLiteralIds();
    if(!CheckConstantPool()) {
        return false;
    }

    return true;
}

void Verifier::GetMethodIds()
{
    auto index_headers = file_->GetIndexHeaders();
    for (const auto &header : index_headers) {
        auto method_index = file_->GetMethodIndex(&header);
        for (auto method_id : method_index) {
            method_ids_.emplace_back(method_id);
        }
    }
}

void Verifier::GetLiteralIds()
{
    const auto literal_arrays = file_->GetLiteralArrays();
    for (const auto literal_id : literal_arrays) {
        literal_ids_.emplace_back(literal_id);
    }
}

bool Verifier::CheckConstantPool()
{
    const auto class_idx = file_->GetClasses();
    for (size_t i = 0; i < class_idx.size(); i++) {
        uint32_t class_id = class_idx[i];
        if (class_id > file_->GetHeader()->file_size) {
            LOG(ERROR, VERIFIER) << "Binary file corrupted. out of bounds (0x" << std::hex
                                 << file_->GetHeader()->file_size;
            return false;
        }
        const panda_file::File::EntityId record_id {class_id};
        if (!file_->IsExternal(record_id)) {
            panda_file::ClassDataAccessor class_accessor {*file_, record_id};
            class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
                CheckConstantPoolInfo(method_accessor.GetMethodId());
            });
        }
    }
    return true;
}

bool Verifier::VerifyMethodId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id)
{
    const auto arg_method_idx = bc_ins.GetId().AsIndex();
    const auto arg_method_id = file_->ResolveMethodIndex(method_id, arg_method_idx);
    auto iter = std::find(method_ids_.begin(), method_ids_.end(), arg_method_id);
    if (iter == method_ids_.end()) {
        LOG(ERROR, VERIFIER) << "Verify method_id failed. method_id(0x" << std::hex << arg_method_id << ")!";
        return false;
    }
    return true;
}

bool Verifier::VerifyLiteralId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id)
{
    const auto arg_literal_idx = bc_ins.GetId().AsIndex();
    const auto arg_literal_id = file_->ResolveMethodIndex(method_id, arg_literal_idx);
    const auto literal_id = panda_file::File::EntityId(arg_literal_id).GetOffset();
    auto iter = std::find(literal_ids_.begin(), literal_ids_.end(), literal_id);
    if (iter == literal_ids_.end()) {
        LOG(ERROR, VERIFIER) << "Verify literal_id failed. literal_id(0x" << std::hex << arg_literal_id << ")!";
        return false;
    }
    return true;
}

bool Verifier::VerifyStringId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id)
{
    const auto arg_string_idx = bc_ins.GetId().AsIndex();
    const auto arg_string_id = file_->ResolveOffsetByIndex(method_id, arg_string_idx);
    if (!arg_string_id.IsValid()) {
        LOG(ERROR, VERIFIER) << "Invalid string_id. string_id(0x" << std::hex << arg_string_id << ")!";
        return false;
    }
    auto string_data = file_->GetStringData(arg_string_id);
    auto desc = std::string(utf::Mutf8AsCString(string_data.data));
    if (string_data.utf16_length != desc.length()) {
        LOG(ERROR, VERIFIER) << "Invalid string_id. string_id(0x" << std::hex << arg_string_id << ")!";
        return false;
    }
    return true;
}

bool Verifier::CheckConstantPoolInfo(const panda_file::File::EntityId &method_id)
{
    panda_file::MethodDataAccessor method_accessor(*file_, method_id);

    ASSERT(method_accessor.GetCodeId().has_value());
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    const auto ins_sz = code_accessor.GetCodeSize();
    const auto ins_arr = code_accessor.GetInstructions();

    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_sz);

    while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
        // fix the scenario when instruction has more than one id, such as defefineclasswithbuffer
        if (bc_ins.HasFlag(BytecodeInstruction::Flags::METHOD_ID)) {
            if (!VerifyMethodId(bc_ins, method_id)) {
                return false;
            }
        } else if (bc_ins.HasFlag(BytecodeInstruction::Flags::LITERALARRAY_ID)) {
            if (!VerifyLiteralId(bc_ins, method_id)) {
                return false;
            }
        } else if (bc_ins.HasFlag(BytecodeInstruction::Flags::STRING_ID)) {
            if (!VerifyStringId(bc_ins, method_id)) {
                return false;
            }
        }
        bc_ins = bc_ins.GetNext();
    }
    return true;
}
} // namespace panda::verifier
