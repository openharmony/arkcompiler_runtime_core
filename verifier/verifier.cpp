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

namespace panda::verifier {

Verifier::Verifier(const std::string &filename)
{
    auto file_to_verify = panda_file::File::Open(filename);
    file_.swap(file_to_verify);
}

bool Verifier::Verify()
{
    CollectIdInfos();

    if (!VerifyChecksum()) {
        return false;
    }

    if (!VerifyConstantPool()) {
        return false;
    }

    if (!VerifyRegisterIndex()) {
        return false;
    }

    return true;
}

void Verifier::CollectIdInfos()
{
    if (file_ == nullptr) {
        return;
    }
    GetMethodIds();
    GetLiteralIds();
    CheckConstantPool(verifier::ActionType::COLLECTINFOS);
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

    if (!CheckConstantPool(verifier::ActionType::CHECKCONSTPOOL)) {
        return false;
    }

    if (!VerifyLiteralArrays()) {
        return false;
    }

    return true;
}

bool Verifier::VerifyRegisterIndex()
{
    if (file_ == nullptr) {
        return false;
    }

    for (const auto &method_id : method_ids_) {
        panda_file::MethodDataAccessor method_accessor {*file_, method_id};
        if (!method_accessor.GetCodeId().has_value()) {
            continue;
        }
        panda_file::CodeDataAccessor code_data(*file_, method_accessor.GetCodeId().value());
        const uint32_t reg_nums = code_data.GetNumVregs();
        const uint32_t arg_nums = code_data.GetNumArgs();
        const uint32_t max_reg_idx = reg_nums + arg_nums;
        auto bc_ins = BytecodeInstruction(code_data.GetInstructions());
        const auto bc_ins_last = bc_ins.JumpTo(code_data.GetCodeSize());
        ASSERT(arg_nums >= DEFAULT_ARGUMENT_NUMBER);
        while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
            const size_t count = GetVRegCount(bc_ins);
            if (count == 0) { // Skip instructions that do not use registers
                bc_ins = bc_ins.GetNext();
                continue;
            }
            if (!CheckVRegIdx(bc_ins, count, max_reg_idx)) {
                return false;
            }
            bc_ins = bc_ins.GetNext();
        }
    }
    return true;
}

bool Verifier::VerifyConstantPoolIndex()
{
    if (file_ == nullptr) {
        return false;
    }

    if (!CheckConstantPool(verifier::ActionType::CHECKCONSTPOOLIDX)) {
        return false;
    }

    return true;
}

bool Verifier::VerifyConstantPoolContent()
{
    if (file_ == nullptr) {
        return false;
    }

    if (!CheckConstantPool(verifier::ActionType::CHECKCONSTPOOLCONTENT)) {
        return false;
    }

    if (!VerifyLiteralArrays()) {
        return false;
    }

    return true;
}

void Verifier::GetMethodIds()
{
    if (method_ids_.size() != 0) {
        return;
    }
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
    if (literal_ids_.size() != 0) {
        return;
    }
    const auto literal_arrays = file_->GetLiteralArrays();
    for (const auto literal_id : literal_arrays) {
        literal_ids_.emplace_back(literal_id);
    }
}

bool Verifier::CheckConstantPoolActions(const verifier::ActionType type, panda_file::File::EntityId method_id)
{
    switch (type) {
        case verifier::ActionType::CHECKCONSTPOOL: {
            return CheckConstantPoolIndex(method_id) && CheckConstantPoolMethodContent(method_id);
        }
        case verifier::ActionType::CHECKCONSTPOOLIDX: {
            return CheckConstantPoolIndex(method_id);
        }
        case verifier::ActionType::CHECKCONSTPOOLCONTENT: {
            return CheckConstantPoolMethodContent(method_id);
        }
        case verifier::ActionType::COLLECTINFOS: {
            if (std::find(method_ids_.begin(), method_ids_.end(), method_id) == method_ids_.end()) {
                method_ids_.emplace_back(method_id);
            }
            return true;
        }
        default: {
            return true;
        }
    }
}

void Verifier::CollectModuleLiteralId(const panda_file::File::EntityId &field_id)
{
    panda_file::FieldDataAccessor field_accessor(*file_, field_id);
    const auto literal_id = field_accessor.GetValue<uint32_t>().value();
    if (std::find(literal_ids_.begin(), literal_ids_.end(), literal_id) != literal_ids_.end()) {
        module_literals_.insert(literal_id);
    }
}

bool Verifier::CheckConstantPool(const verifier::ActionType type)
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
            bool check_res = true;
            class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
                check_res &= CheckConstantPoolActions(type, method_accessor.GetMethodId());
            });
            if (!check_res) {
                return false;
            }
            if (type == verifier::ActionType::COLLECTINFOS) {
                class_accessor.EnumerateFields([&](panda_file::FieldDataAccessor &field_accessor) -> void {
                    CollectModuleLiteralId(field_accessor.GetFieldId());
                });
            }
        }
    }

    return true;
}

size_t Verifier::GetVRegCount(const BytecodeInstruction &bc_ins)
{
    size_t idx = 0; // Represents the idxTH register index in an instruction
    BytecodeInstruction::Format format = bc_ins.GetFormat();
    while (bc_ins.HasVReg(format, idx)) {
        idx++;
    }
    return idx;
}

bool Verifier::CheckVRegIdx(const BytecodeInstruction &bc_ins, const size_t count, const uint32_t max_reg_idx)
{
    for (size_t idx = 0; idx < count; idx++) { // Represents the idxTH register index in an instruction
        uint16_t reg_idx = bc_ins.GetVReg(idx);
        if (reg_idx >= max_reg_idx) {
            LOG(ERROR, VERIFIER) << "register index out of bounds. register index is (0x" << std::hex
                                 << reg_idx << ")" << std::endl;
            return false;
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

std::optional<int64_t> Verifier::GetFirstImmFromInstruction(const BytecodeInstruction &bc_ins)
{
    std::optional<int64_t> first_imm = std::optional<int64_t> {};
    size_t index = 0;
    const auto format = bc_ins.GetFormat();
    if (bc_ins.HasImm(format, index)) {
        first_imm = bc_ins.GetImm64(index);
    }

    return first_imm;
}

std::optional<uint64_t> Verifier::GetSlotNumberFromAnnotation(panda_file::MethodDataAccessor &method_accessor)
{
    std::optional<uint64_t> slot_number {};
    method_accessor.EnumerateAnnotations([&](panda_file::File::EntityId annotation_id) {
        panda_file::AnnotationDataAccessor ada(*file_, annotation_id);
        auto *annotation_name = reinterpret_cast<const char *>(file_->GetStringData(ada.GetClassId()).data);
        if (::strcmp("L_ESSlotNumberAnnotation;", annotation_name) == 0) {
            uint32_t elem_count = ada.GetCount();
            for (uint32_t i = 0; i < elem_count; i++) {
                panda_file::AnnotationDataAccessor::Elem adae = ada.GetElement(i);
                auto *elem_name = reinterpret_cast<const char *>(file_->GetStringData(adae.GetNameId()).data);
                if (::strcmp("SlotNumber", elem_name) == 0) {
                    slot_number = adae.GetScalarValue().GetValue();
                }
            }
        }
    });
    return slot_number;
}

bool Verifier::VerifyLiteralId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id,
                               size_t idx /* = 0 */)
{
    const auto arg_literal_idx = bc_ins.GetId(idx).AsIndex();
    const auto arg_literal_id = file_->ResolveMethodIndex(method_id, arg_literal_idx);
    const auto literal_id = panda_file::File::EntityId(arg_literal_id).GetOffset();
    auto iter = std::find(literal_ids_.begin(), literal_ids_.end(), literal_id);
    if (iter == literal_ids_.end()) {
        LOG(ERROR, VERIFIER) << "Verify literal_id failed. literal_id(0x" << std::hex << arg_literal_id << ")!";
        return false;
    }
    return true;
}

bool Verifier::VerifyMethodIdInLiteralArray(const uint32_t &id)
{
    const auto method_id = panda_file::File::EntityId(id);
    auto iter = std::find(method_ids_.begin(), method_ids_.end(), method_id);
    if (iter == method_ids_.end()) {
        LOG(ERROR, VERIFIER) << "Invalid method id(0x" << id << ") in literal array";
        return false;
    }
    return true;
}

bool Verifier::VerifyStringIdInLiteralArray(const uint32_t &id)
{
    auto string_data = file_->GetStringData(panda_file::File::EntityId(id));
    auto desc = std::string(utf::Mutf8AsCString(string_data.data));
    if (string_data.utf16_length != desc.length()) {
        LOG(ERROR, VERIFIER) << "Invalid string value(0x" << id << ") in literal array";
        return false;
    }
    return true;
}

bool Verifier::VerifyLiteralIdInLiteralArray(const uint32_t &id)
{
    auto iter = std::find(literal_ids_.begin(), literal_ids_.end(), id);
    if (iter == literal_ids_.end()) {
        LOG(ERROR, VERIFIER) << "Invalid literal id(0x" << id << ") in literal array";
        return false;
    }
    return true;
}

bool Verifier::VerifySingleLiteralArray(const panda_file::File::EntityId &literal_id)
{
    auto sp = file_->GetSpanFromId(literal_id);
    const auto literal_vals_num = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    for (size_t i = 0; i < literal_vals_num; i += 2U) { // 2u skip literal item
        const auto tag = static_cast<panda_file::LiteralTag>(panda_file::helpers::Read<panda_file::TAG_SIZE>(&sp));
        switch (tag) {
            case panda_file::LiteralTag::TAGVALUE:
            case panda_file::LiteralTag::BOOL:
            case panda_file::LiteralTag::ACCESSOR:
            case panda_file::LiteralTag::NULLVALUE:
            case panda_file::LiteralTag::BUILTINTYPEINDEX: {
                sp = sp.SubSpan(sizeof(uint8_t)); // run next sp
                break;
            }
            case panda_file::LiteralTag::METHODAFFILIATE: {
                sp = sp.SubSpan(sizeof(uint16_t));
                break;
            }
            case panda_file::LiteralTag::INTEGER:
            case panda_file::LiteralTag::FLOAT:
            case panda_file::LiteralTag::GETTER:
            case panda_file::LiteralTag::SETTER:
            case panda_file::LiteralTag::GENERATORMETHOD:
            case panda_file::LiteralTag::LITERALBUFFERINDEX:
            case panda_file::LiteralTag::ASYNCGENERATORMETHOD: {
                sp = sp.SubSpan(sizeof(uint32_t));
                break;
            }
            case panda_file::LiteralTag::DOUBLE: {
                sp = sp.SubSpan(sizeof(uint64_t));
                break;
            }
            case panda_file::LiteralTag::ARRAY_U1:
            case panda_file::LiteralTag::ARRAY_U8:
            case panda_file::LiteralTag::ARRAY_I8:
            case panda_file::LiteralTag::ARRAY_U16:
            case panda_file::LiteralTag::ARRAY_I16:
            case panda_file::LiteralTag::ARRAY_U32:
            case panda_file::LiteralTag::ARRAY_I32:
            case panda_file::LiteralTag::ARRAY_U64:
            case panda_file::LiteralTag::ARRAY_I64:
            case panda_file::LiteralTag::ARRAY_F32:
            case panda_file::LiteralTag::ARRAY_F64:
            case panda_file::LiteralTag::ARRAY_STRING: {
                i = literal_vals_num;
                break;
            }
            case panda_file::LiteralTag::STRING: {
                const auto value = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
                if (!VerifyStringIdInLiteralArray(value)) {
                    return false;
                }
                break;
            }
            case panda_file::LiteralTag::METHOD: {
                const auto value = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
                inner_method_map_.emplace(literal_id.GetOffset(), value);
                if (!VerifyMethodIdInLiteralArray(value)) {
                    return false;
                }
                break;
            }
            case panda_file::LiteralTag::LITERALARRAY: {
                const auto value = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&sp));
                inner_literal_map_.emplace(literal_id.GetOffset(), value);
                if (!VerifyLiteralIdInLiteralArray(value)) {
                    return false;
                }
                break;
            }
            default: {
                LOG(ERROR, VERIFIER) << "Invalid literal tag";
                return false;
            }
        }
    }
    return true;
}

bool Verifier::IsModuleLiteralId(const panda_file::File::EntityId &id) const
{
    return module_literals_.find(id.GetOffset()) != module_literals_.end();
}

bool Verifier::VerifyLiteralArrays()
{
    for (const auto &arg_literal_id : literal_ids_) {
        const auto literal_id = panda_file::File::EntityId(arg_literal_id);
        if (!IsModuleLiteralId(literal_id) && !VerifySingleLiteralArray(literal_id)) {
            return false;
        }
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

bool Verifier::IsJumpInstruction(const Opcode &ins_opcode)
{
    bool valid = true;

    switch (ins_opcode) {
        case Opcode::JMP_IMM8:
        case Opcode::JMP_IMM16:
        case Opcode::JEQZ_IMM8:
        case Opcode::JEQZ_IMM16:
        case Opcode::JNEZ_IMM8:
        case Opcode::JSTRICTEQZ_IMM8:
        case Opcode::JNSTRICTEQZ_IMM8:
        case Opcode::JEQNULL_IMM8:
        case Opcode::JNENULL_IMM8:
        case Opcode::JSTRICTEQNULL_IMM8:
        case Opcode::JNSTRICTEQNULL_IMM8:
        case Opcode::JEQUNDEFINED_IMM8:
        case Opcode::JNEUNDEFINED_IMM8:
        case Opcode::JSTRICTEQUNDEFINED_IMM8:
        case Opcode::JNSTRICTEQUNDEFINED_IMM8:
        case Opcode::JEQ_V8_IMM8:
        case Opcode::JNE_V8_IMM8:
        case Opcode::JSTRICTEQ_V8_IMM8:
        case Opcode::JNSTRICTEQ_V8_IMM8:
        case Opcode::JMP_IMM32:
        case Opcode::JEQZ_IMM32:
        case Opcode::JNEZ_IMM16:
        case Opcode::JNEZ_IMM32:
        case Opcode::JSTRICTEQZ_IMM16:
        case Opcode::JNSTRICTEQZ_IMM16:
        case Opcode::JEQNULL_IMM16:
        case Opcode::JNENULL_IMM16:
        case Opcode::JSTRICTEQNULL_IMM16:
        case Opcode::JNSTRICTEQNULL_IMM16:
        case Opcode::JEQUNDEFINED_IMM16:
        case Opcode::JNEUNDEFINED_IMM16:
        case Opcode::JSTRICTEQUNDEFINED_IMM16:
        case Opcode::JEQ_V8_IMM16:
        case Opcode::JNE_V8_IMM16:
        case Opcode::JSTRICTEQ_V8_IMM16:
        case Opcode::JNSTRICTEQ_V8_IMM16: {
            valid = true;
            break;
        }
        default: {
            valid = false;
            break;
        }
    }
    return valid;
}

bool Verifier::VerifyJumpInstruction(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                                     const BytecodeInstruction &bc_ins_first)
{
    // update maximum forward offset
    const auto bc_ins_forward_size = bc_ins_last.GetAddress() - bc_ins.GetAddress();
    // update maximum backward offset
    const auto bc_ins_backward_size = bc_ins.GetAddress() - bc_ins_first.GetAddress();
    if (!bc_ins.IsPrimaryOpcodeValid()) {
        LOG(ERROR, VERIFIER) << "Verify primaryOpcode failed!";
        return false;
    }

    Opcode ins_opcode = bc_ins.GetOpcode();
    if (IsJumpInstruction(ins_opcode)) {
        std::optional<int64_t> immdata = GetFirstImmFromInstruction(bc_ins);
        if (!immdata.has_value()) {
            LOG(ERROR, VERIFIER) << "Get immediate data failed!";
            return false;
        }

        if (immdata.value() > 0) {
            // immdata -1,excluding instruction location offset
            if (bc_ins_forward_size - 1 <= immdata.value()) {
                LOG(ERROR, VERIFIER) << "Jump forward out of boundary";
                return false;
            }
        } else if (immdata.value() < 0) {
            // immdata -1,excluding instruction location offset
            if ((bc_ins_backward_size - 1) + immdata.value() < 0) {
                LOG(ERROR, VERIFIER) << "Jump backward out of boundary";
                return false;
            }
        }
    }
    return true;
}

bool Verifier::GetIcSlotFromInstruction(const BytecodeInstruction &bc_ins, uint32_t &first_slot_index,
                                        bool &has_slot, bool &is_two_slot)
{
    std::optional<uint64_t> first_imm = {};
    if (bc_ins.HasFlag(BytecodeInstruction::Flags::ONE_SLOT)) {
        first_imm = GetFirstImmFromInstruction(bc_ins);
        if (!first_imm.has_value()) {
            LOG(ERROR, VERIFIER) << "Get first immediate data failed!";
            return false;
        }
        first_slot_index = first_imm.value();
        is_two_slot = false;
        has_slot = true;
    } else if (bc_ins.HasFlag(BytecodeInstruction::Flags::TWO_SLOT)) {
        first_imm = GetFirstImmFromInstruction(bc_ins);
        if (!first_imm.has_value()) {
            LOG(ERROR, VERIFIER) << "Get first immediate data failed!";
            return false;
        }
        first_slot_index = first_imm.value();
        has_slot = true;
        is_two_slot = true;
    }

    return true;
}

bool Verifier::VerifySlotNumber(panda_file::MethodDataAccessor &method_accessor, const uint32_t &slot_number,
                                const panda_file::File::EntityId &method_id)
{
    const auto ann_slot_number = GetSlotNumberFromAnnotation(method_accessor);
    if (!ann_slot_number.has_value()) {
        LOG(ERROR, VERIFIER) << "Fail to get slot number from annotation";
        return false;
    }
    if (slot_number == ann_slot_number.value()) {
        return true;
    }

    LOG(ERROR, VERIFIER) << "Slot number has been falsified in method 0x" << method_id;
    return false;
}

bool Verifier::CheckConstantPoolMethodContent(const panda_file::File::EntityId &method_id)
{
    panda_file::MethodDataAccessor method_accessor(*file_, method_id);
    ASSERT(method_accessor.GetCodeId().has_value());
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    const auto ins_size = code_accessor.GetCodeSize();
    const auto ins_arr = code_accessor.GetInstructions();
    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size);
    const auto bc_ins_init = bc_ins; // initial PC value
    uint32_t ins_slot_num = 0;
    bool has_slot = false;
    bool is_two_slot = false;

    while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
        if (!VerifyJumpInstruction(bc_ins, bc_ins_last, bc_ins_init)) {
            LOG(ERROR, VERIFIER) << "Invalid target position of jump instruction";
            return false;
        }
        if (!GetIcSlotFromInstruction(bc_ins, ins_slot_num, has_slot, is_two_slot)) {
            LOG(ERROR, VERIFIER) << "Get first slot index failed!";
            return false;
        }
        bc_ins = bc_ins.GetNext();
    }

    if (has_slot) {
        if (is_two_slot) {
            ins_slot_num += 1; // when there are two slots for the last instruction, the slot index increases
        }
        ins_slot_num += 1; // slot index starts with zero
    }

    return true;
}

bool Verifier::CheckConstantPoolIndex(const panda_file::File::EntityId &method_id)
{
    panda_file::MethodDataAccessor method_accessor(*file_, method_id);

    ASSERT(method_accessor.GetCodeId().has_value());
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    const auto ins_size = code_accessor.GetCodeSize();
    const auto ins_arr = code_accessor.GetInstructions();

    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size);

    while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
        if (bc_ins.HasFlag(BytecodeInstruction::Flags::LITERALARRAY_ID)) {
            // the idx of any instruction with a literal id is 0 except defineclasswithbuffer
            size_t idx = 0;
            if (bc_ins.GetOpcode() == BytecodeInstruction::Opcode::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8 ||
                bc_ins.GetOpcode() == BytecodeInstruction::Opcode::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8) {
                idx = 1;
            }
            if (!VerifyLiteralId(bc_ins, method_id, idx)) {
                return false;
            }
        }
        if (bc_ins.HasFlag(BytecodeInstruction::Flags::METHOD_ID)) {
            if (!VerifyMethodId(bc_ins, method_id)) {
                return false;
            }
        }
        if (bc_ins.HasFlag(BytecodeInstruction::Flags::STRING_ID)) {
            if (!VerifyStringId(bc_ins, method_id)) {
                return false;
            }
        }
        bc_ins = bc_ins.GetNext();
    }
    return true;
}
} // namespace panda::verifier
