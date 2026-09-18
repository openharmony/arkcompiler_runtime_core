/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "verifier_internal.h"
#include "annotation_data_accessor.h"
#include "class_data_accessor-inl.h"
#include "libpandafile/util/collect_util.h"
#include "proto_data_accessor-inl.h"
#include "zlib.h"

#include <algorithm>
#include <cstring>
#include <limits>

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

    if (!CollectIdInfos()) {
        return false;
    }

    if (!VerifyConstantPool()) {
        return false;
    }

    return true;
}

bool Verifier::CollectIdInfos()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }
    GetConstantPoolIds();
    if (!ValidateInstructionBasics()) {
        return false;
    }
    if (include_literal_array_ids) {
        GetLiteralIds();
    }
    return CheckConstantPool(verifier::ActionType::COLLECTINFOS);
}

bool Verifier::ResolveInstructionIds(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id,
                                     InstructionIds &ids)
{
    if (bc_ins.HasFlag(BytecodeInstruction::Flags::LITERALARRAY_ID)) {
        // literal id index is 0 except defineclasswithbuffer / definesendableclass
        size_t idx = bc_ins.GetLiteralIndex();
        const auto arg_literal_idx = bc_ins.GetId(idx).AsIndex();
        const auto literal_id = file_->ResolveMethodIndex(method_id, arg_literal_idx);
        if (literal_id.GetOffset() == 0) {
            LOG(ERROR, VERIFIER) << "Literal array index 0x" << std::hex << arg_literal_idx
                                 << " is out of bounds of the constant pool!";
            return false;
        }
        ids.literal_id = literal_id;
    }
    if (bc_ins.HasFlag(BytecodeInstruction::Flags::METHOD_ID)) {
        const auto arg_method_idx = bc_ins.GetId().AsIndex();
        const auto arg_method_id = file_->ResolveMethodIndex(method_id, arg_method_idx);
        if (arg_method_id.GetOffset() == 0) {
            LOG(ERROR, VERIFIER) << "Method index 0x" << std::hex << arg_method_idx
                                 << " is out of bounds of the constant pool!";
            return false;
        }
        ids.method_id = arg_method_id;
    }
    if (bc_ins.HasFlag(BytecodeInstruction::Flags::STRING_ID)) {
        const auto arg_string_idx = bc_ins.GetId().AsIndex();
        const auto string_id = file_->ResolveOffsetByIndex(method_id, arg_string_idx);
        if (string_id.GetOffset() == 0) {
            LOG(ERROR, VERIFIER) << "String index 0x" << std::hex << arg_string_idx
                                 << " is out of bounds of the constant pool!";
            return false;
        }
        ids.string_id = string_id;
    }
    return true;
}

void Verifier::CollectClassLiteralId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &literal_id)
{
    auto opcode = bc_ins.GetOpcode();
    if (opcode == Opcode::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8 ||
        opcode == Opcode::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8) {
        class_literal_ids_.insert(literal_id.GetOffset());
    } else if (opcode == Opcode::CALLRUNTIME_DEFINESENDABLECLASS_PREF_IMM16_ID16_ID16_IMM16_V8) {
        sendable_class_literal_ids_.insert(literal_id.GetOffset());
    }
}

bool Verifier::ValidateMethodInstructionBasics(panda_file::MethodDataAccessor &method_accessor,
                                               std::unordered_set<uint32_t> &validated_literals,
                                               std::vector<uint32_t> &pending_literals)
{
    if (file_->GetIndexHeader(method_accessor.GetMethodId()) == nullptr) {
        LOG(ERROR, VERIFIER) << "Fail to find index header of method 0x" << std::hex
                             << method_accessor.GetMethodId().GetOffset() << "!";
        return false;
    }
    if (!method_accessor.GetCodeId().has_value()) {
        return true;
    }
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    auto bc_ins = BytecodeInstruction(code_accessor.GetInstructions());
    const auto bc_ins_last = bc_ins.JumpTo(code_accessor.GetCodeSize());
    while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
        if (!CheckInstructionOpcodes(bc_ins, bc_ins_last, method_accessor.GetCodeId().value(),
                                     method_accessor.GetMethodId())) {
            return false;
        }
        InstructionIds ids;
        if (!ResolveInstructionIds(bc_ins, method_accessor.GetMethodId(), ids)) {
            return false;
        }
        if (ids.literal_id.has_value() &&
            !ValidateLiteralStructure(ids.literal_id.value(), validated_literals, pending_literals)) {
            return false;
        }
        bc_ins = bc_ins.GetNext();
    }
    return true;
}

bool Verifier::ValidateInstructionBasics()
{
    std::unordered_set<uint32_t> validated_literals;
    std::vector<uint32_t> pending_literals;
    auto validate_record = [&](const panda_file::File::EntityId & /*record_id*/,
                               panda_file::ClassDataAccessor &class_accessor) -> bool {
        bool check_res = true;
        class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
            check_res =
                check_res && ValidateMethodInstructionBasics(method_accessor, validated_literals, pending_literals);
        });
        return check_res;
    };
    if (!ForEachInternalRecord(validate_record)) {
        return false;
    }
    while (!pending_literals.empty()) {
        uint32_t literal_offset = pending_literals.back();
        pending_literals.pop_back();
        if (!ValidateLiteralStructure(panda_file::File::EntityId(literal_offset), validated_literals,
                                      pending_literals)) {
            return false;
        }
    }
    return true;
}

bool Verifier::VerifyChecksum()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
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
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }

    if (!CheckConstantPoolIdsBounds()) {
        return false;
    }

    if (!CheckConstantPoolIndex()) {
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

bool Verifier::VerifyRegisterIndex()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }

    for (const auto id : all_method_ids_) {
        const panda_file::File::EntityId method_id = panda_file::File::EntityId(id);
        panda_file::MethodDataAccessor method_accessor {*file_, method_id};
        if (!method_accessor.GetCodeId().has_value()) {
            continue;
        }
        panda_file::CodeDataAccessor code_data(*file_, method_accessor.GetCodeId().value());
        const uint64_t reg_nums = code_data.GetNumVregs();
        const uint64_t arg_nums = code_data.GetNumArgs();
        const std::optional<uint64_t> valid_regs_num = SafeAdd(reg_nums, arg_nums);
        if (!valid_regs_num.has_value()) {
            LOG(ERROR, VERIFIER) << "Integer overflow detected during register index calculation!";
            return false;
        }
        if (valid_regs_num.value() > MAX_REGISTER_INDEX + 1) {
            LOG(ERROR, VERIFIER) << "Register index exceeds the maximum allowable value (0xffff)!";
            return false;
        }
        auto bc_ins = BytecodeInstruction(code_data.GetInstructions());
        const auto bc_ins_last = bc_ins.JumpTo(code_data.GetCodeSize());
        if (!VerifyMethodNumArgs(code_data, method_accessor)) {
            return false;
        }
        while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
            const size_t count = GetVRegCount(bc_ins);
            if (count == 0) {  // Skip instructions that do not use registers
                bc_ins = bc_ins.GetNext();
                continue;
            }
            if (!CheckVRegIdx(bc_ins, count, valid_regs_num.value())) {
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
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
        return false;
    }

    if (!CheckConstantPoolIdsBounds()) {
        return false;
    }

    if (!CheckConstantPoolIndex()) {
        return false;
    }

    return true;
}

bool Verifier::VerifyConstantPoolContent()
{
    if (file_ == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to verify empty abc file!";
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

void Verifier::GetConstantPoolIds()
{
    if (constant_pool_ids_.size() != 0) {
        return;
    }
    auto index_headers = file_->GetIndexHeaders();
    for (const auto &index_header : index_headers) {
        auto region_indexs = file_->GetMethodIndex(&index_header);
        for (auto &index : region_indexs) {
            constant_pool_ids_.insert(index.GetOffset());
        }
    }
}

void Verifier::GetLiteralIds()
{
    if (literal_ids_.size() != 0) {
        return;
    }

    if (panda_file::ContainsLiteralArrayInHeader(file_->GetHeader()->version)) {
        const auto literal_arrays = file_->GetLiteralArrays();
        PushToLiteralIds(literal_arrays);
    } else {
        panda::libpandafile::CollectUtil collect_util;
        std::unordered_set<uint32_t> literal_array_ids;
        collect_util.CollectLiteralArray(*file_, literal_array_ids);
        PushToLiteralIds(literal_array_ids);
    }
}

template <typename T>
void Verifier::PushToLiteralIds(T &ids)
{
    for (const auto id : ids) {
        literal_ids_.insert(id);
    }
}

bool Verifier::CheckConstantPoolActions(const verifier::ActionType type, panda_file::File::EntityId method_id,
                                        bool module_instructions_allowed)
{
    switch (type) {
        case verifier::ActionType::CHECKCONSTPOOLCONTENT: {
            return CheckConstantPoolMethodContent(method_id);
        }
        case verifier::ActionType::COLLECTINFOS: {
            all_method_ids_.insert(method_id.GetOffset());
            return CollectIdInInstructions(method_id, module_instructions_allowed);
        }
        default: {
            return true;
        }
    }
}

bool Verifier::CheckInstructionOpcodes(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                                       const panda_file::File::EntityId &code_id,
                                       const panda_file::File::EntityId &method_id)
{
    if (!bc_ins.IsPrimaryOpcodeValid()) {
        LOG(ERROR, VERIFIER) << "Fail to verify primary opcode!";
        return false;
    }
    if (!IsSecondaryOpcodeValid(bc_ins)) {
        LOG(ERROR, VERIFIER) << "Fail to verify secondary opcode!";
        return false;
    }
    if (!IsInstructionEndInBounds(bc_ins, bc_ins_last)) {
        ReportCorruptedInstructionSequence(code_id, method_id);
        return false;
    }
    return true;
}

void Verifier::CollectResolvedInstructionIds(const BytecodeInstruction &bc_ins, const InstructionIds &ids)
{
    if (ids.literal_id.has_value()) {
        ins_literal_ids_.insert(ids.literal_id.value().GetOffset());
        CollectClassLiteralId(bc_ins, ids.literal_id.value());
    }
    if (ids.method_id.has_value()) {
        ins_method_ids_.insert(ids.method_id.value().GetOffset());
    }
    if (ids.string_id.has_value()) {
        ins_string_ids_.insert(ids.string_id.value().GetOffset());
    }
}

bool Verifier::CollectIdInInstructions(const panda_file::File::EntityId &method_id, bool module_instructions_allowed)
{
    panda_file::MethodDataAccessor method_accessor(*file_, method_id);
    if (!method_accessor.GetCodeId().has_value()) {
        return false;
    }
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    const auto ins_size = code_accessor.GetCodeSize();
    const auto ins_arr = code_accessor.GetInstructions();

    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size);

    while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
        if (!CheckInstructionOpcodes(bc_ins, bc_ins_last, method_accessor.GetCodeId().value(), method_id)) {
            return false;
        }
        if (!module_instructions_allowed && IsModuleRelatedInstruction(bc_ins)) {
            LOG(ERROR, VERIFIER) << "Module related instruction in non-esmodule record, method 0x" << std::hex
                                 << method_id.GetOffset() << "!";
            return false;
        }
        InstructionIds ids;
        if (!ResolveInstructionIds(bc_ins, method_id, ids)) {
            return false;
        }
        CollectResolvedInstructionIds(bc_ins, ids);
        bc_ins = bc_ins.GetNext();
    }
    return true;
}

bool Verifier::ForEachInternalRecord(
    const std::function<bool(const panda_file::File::EntityId &, panda_file::ClassDataAccessor &)> &cb)
{
    for (uint32_t class_id : file_->GetClasses()) {
        if (class_id >= file_->GetHeader()->file_size) {
            LOG(ERROR, VERIFIER) << "Binary file corrupted. out of bounds (0x" << std::hex << class_id << ")!";
            return false;
        }
        const panda_file::File::EntityId record_id {class_id};
        if (file_->IsExternal(record_id)) {
            continue;
        }
        panda_file::ClassDataAccessor class_accessor {*file_, record_id};
        if (!cb(record_id, class_accessor)) {
            return false;
        }
    }
    return true;
}

bool Verifier::CheckRecord(const panda_file::File::EntityId &record_id, panda_file::ClassDataAccessor &class_accessor,
                           const verifier::ActionType type)
{
    bool record_is_esmodule = false;
    bool record_has_tla = false;
    if (type == verifier::ActionType::COLLECTINFOS) {
        CollectRecordInfo(class_accessor, record_id, record_is_esmodule, record_has_tla);
    }
    bool module_instructions_allowed = record_is_esmodule || has_esmodule_record_class_;
    bool check_res = true;
    class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
        if (type == verifier::ActionType::COLLECTINFOS) {
            method_class_map_.emplace(method_accessor.GetMethodId().GetOffset(), record_id.GetOffset());
        }
        check_res =
            check_res && CheckConstantPoolActions(type, method_accessor.GetMethodId(), module_instructions_allowed);
        if (type == verifier::ActionType::COLLECTINFOS && check_res && (record_has_tla || has_tla_class_)) {
            check_res = VerifyTlaMainFunctionReturn(method_accessor);
        }
    });
    return check_res;
}

bool Verifier::CheckConstantPool(const verifier::ActionType type)
{
    if (type == verifier::ActionType::COLLECTINFOS && !DetectSpecialRecords()) {
        return false;
    }
    return ForEachInternalRecord([this, type](const panda_file::File::EntityId &record_id,
                                              panda_file::ClassDataAccessor &class_accessor) -> bool {
        return CheckRecord(record_id, class_accessor, type);
    });
}

size_t Verifier::GetVRegCount(const BytecodeInstruction &bc_ins)
{
    size_t idx = 0;  // Represents the idxTH register index in an instruction
    BytecodeInstruction::Format format = bc_ins.GetFormat();
    while (bc_ins.HasVReg(format, idx)) {
        idx++;
    }
    return idx;
}

bool Verifier::IsRangeInstAndHasInvalidRegIdx(const BytecodeInstruction &bc_ins, const size_t count,
                                              uint64_t valid_regs_num)
{
    ASSERT(bc_ins.IsRangeInstruction());

    uint64_t reg_idx = bc_ins.GetVReg(FIRST_INDEX);
    if (IsRegIdxOutOfBounds(reg_idx, valid_regs_num)) {  // for [format: +AA/+AAAA vBB vCC], vBB can be verified here
        return true;
    }

    std::optional<uint64_t> max_ins_reg_idx_opt = bc_ins.GetRangeInsLastRegIdx();
    if (!max_ins_reg_idx_opt.has_value()) {
        LOG(ERROR, VERIFIER) << "Integer overflow detected during register index calculation!";
        return true;
    }

    reg_idx = max_ins_reg_idx_opt.value();
    if (IsRegIdxOutOfBounds(reg_idx, valid_regs_num)) {
        return true;
    }

    return false;
}

bool Verifier::IsRegIdxOutOfBounds(uint64_t reg_idx, uint64_t valid_regs_num)
{
    if (reg_idx >= valid_regs_num) {
        LOG(ERROR, VERIFIER) << "Register index out of bounds: 0x" << std::hex << reg_idx << ", Max allowed: 0x"
                             << std::hex << valid_regs_num;
        return true;
    }
    return false;
}

bool Verifier::CheckVRegIdx(const BytecodeInstruction &bc_ins, const size_t count, uint64_t valid_regs_num)
{
    if (bc_ins.IsRangeInstruction() && IsRangeInstAndHasInvalidRegIdx(bc_ins, count, valid_regs_num)) {
        return false;
    }
    for (size_t idx = 0; idx < count; idx++) {  // Represents the idxTH register index in an instruction
        uint16_t reg_idx = bc_ins.GetVReg(idx);
        if (reg_idx >= valid_regs_num) {
            LOG(ERROR, VERIFIER) << "Register index out of bounds: 0x" << std::hex << reg_idx << ", Max allowed: 0x"
                                 << std::hex << valid_regs_num;
            return false;
        }
    }
    return true;
}

bool Verifier::VerifyMethodId(const uint32_t &method_id) const
{
    if (literal_ids_.count(method_id) != 0 || ins_string_ids_.count(method_id) != 0 || !IsDeclaredMethodId(method_id)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method id. method_id(0x" << std::hex << method_id << ")!";
        return false;
    }
    return true;
}

bool Verifier::IsDeclaredMethodId(const uint32_t &method_id) const
{
    if (method_class_map_.find(method_id) != method_class_map_.end()) {
        return true;
    }
    return IsExternalMethodId(method_id);
}

bool Verifier::IsExternalMethodId(const uint32_t &method_id) const
{
    constexpr size_t FOREIGN_METHOD_HEADER_SIZE = 2U * panda_file::IDX_SIZE + panda_file::ID_SIZE;
    const uint32_t file_size = file_->GetHeader()->file_size;
    const panda_file::File::EntityId id {method_id};
    if (!file_->IsExternal(id) || method_id > file_size - FOREIGN_METHOD_HEADER_SIZE) {
        return false;
    }
    auto sp = file_->GetSpanFromId(id);
    auto class_idx = ReadBounded(&sp, panda_file::IDX_SIZE);
    auto proto_idx = ReadBounded(&sp, panda_file::IDX_SIZE);
    auto name_off = ReadBounded(&sp, panda_file::ID_SIZE);
    if (!class_idx.has_value() || !proto_idx.has_value() || !name_off.has_value()) {
        return false;
    }
    return VerifyStringItem(static_cast<uint32_t>(name_off.value()), "external method");
}

bool Verifier::VerifyLiteralId(const uint32_t &literal_id) const
{
    if (all_method_ids_.count(literal_id) != 0 || ins_string_ids_.count(literal_id) != 0) {
        LOG(ERROR, VERIFIER) << "Fail to verify literal id. literal_id(0x" << std::hex << literal_id << ")!";
        return false;
    }
    if (!panda_file::ContainsLiteralArrayInHeader(file_->GetHeader()->version)) {
        return true;
    }
    return literal_ids_.count(literal_id) != 0;
}

bool Verifier::VerifyStringId(const uint32_t &string_id) const
{
    if (ins_method_ids_.count(string_id) != 0 || literal_ids_.count(string_id) != 0) {
        LOG(ERROR, VERIFIER) << "Fail to verify string id. string_id(0x" << std::hex << string_id << ")!";
        return false;
    }
    return VerifyStringItem(string_id, "constant pool");
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

namespace {
bool MatchAnnotationElement(const panda_file::File &file, panda_file::AnnotationDataAccessor &ada,
                            std::string_view element_name, uint64_t &value)
{
    uint32_t elem_count = ada.GetCount();
    for (uint32_t i = 0; i < elem_count; i++) {
        panda_file::AnnotationDataAccessor::Elem adae = ada.GetElement(i);
        auto *elem_name = reinterpret_cast<const char *>(file.GetStringData(adae.GetNameId()).data);
        if (element_name != elem_name) {
            continue;
        }
        value = adae.GetScalarValue().GetValue();
        return true;
    }
    return false;
}
}  // namespace

std::optional<uint64_t> Verifier::FindAnnotationElementValue(panda_file::MethodDataAccessor &method_accessor,
                                                             std::string_view annotation_name,
                                                             std::string_view element_name) const
{
    std::optional<uint64_t> result {};
    method_accessor.EnumerateAnnotations([&](panda_file::File::EntityId annotation_id) {
        panda_file::AnnotationDataAccessor ada(*file_, annotation_id);
        auto *class_name = reinterpret_cast<const char *>(file_->GetStringData(ada.GetClassId()).data);
        if (annotation_name != class_name) {
            return;
        }
        uint64_t value = 0;
        if (MatchAnnotationElement(*file_, ada, element_name, value)) {
            result = value;
        }
    });
    return result;
}

std::optional<uint32_t> Verifier::GetCallTypeFromAnnotation(panda_file::MethodDataAccessor &method_accessor)
{
    auto call_type = FindAnnotationElementValue(method_accessor, CALL_TYPE_ANNOTATION, CALL_TYPE_ELEMENT_NAME);
    if (!call_type.has_value()) {
        return std::nullopt;
    }
    return static_cast<uint32_t>(call_type.value());
}

bool Verifier::VerifyStringItem(uint32_t string_offset, std::string_view where) const
{
    if (string_offset <= sizeof(panda_file::File::Header) || string_offset >= file_->GetHeader()->file_size) {
        LOG(ERROR, VERIFIER) << "Invalid string offset 0x" << std::hex << string_offset << " in " << where << "!";
        return false;
    }
    auto sp = file_->GetSpanFromId(panda_file::File::EntityId(string_offset));
    size_t length_size = 0;
    while (length_size < sp.Size() && length_size < MAX_ULEB128_U32_SIZE) {
        if ((sp[length_size] & 0x80U) == 0) {
            break;
        }
        length_size++;
    }
    if (length_size >= sp.Size() || length_size >= MAX_ULEB128_U32_SIZE) {
        LOG(ERROR, VERIFIER) << "Invalid string offset 0x" << std::hex << string_offset << " in " << where << "!";
        return false;
    }
    uint32_t tagged_utf16_length = 0;
    for (size_t i = 0; i <= length_size; i++) {
        tagged_utf16_length |= static_cast<uint32_t>(sp[i] & 0x7fU) << (i * 7U);
    }
    const uint8_t *data = sp.Data() + length_size + 1;
    size_t data_size = sp.Size() - length_size - 1;
    if (memchr(data, 0, data_size) == nullptr) {
        LOG(ERROR, VERIFIER) << "Invalid string offset 0x" << std::hex << string_offset << " in " << where << "!";
        return false;
    }
    if (!utf::IsValidModifiedUTF8(data) || utf::MUtf8ToUtf16Size(data) != (tagged_utf16_length >> 1U)) {
        LOG(ERROR, VERIFIER) << "Invalid string value 0x" << std::hex << string_offset << " in " << where << "!";
        return false;
    }
    return true;
}

bool Verifier::PrecomputeInstructionIndices(const BytecodeInstruction &bc_ins_start,
                                            const BytecodeInstruction &bc_ins_last)
{
    instruction_index_map_.clear();
    size_t index = 0;
    auto current_ins = bc_ins_start;
    instruction_index_map_[current_ins.GetAddress()] = index;

    while (current_ins.GetAddress() < bc_ins_last.GetAddress()) {
        // Must keep IsPrimaryOpcodeValid is the first check item
        if (!current_ins.IsPrimaryOpcodeValid()) {
            LOG(ERROR, VERIFIER) << "Fail to verify primary opcode!";
            return false;
        }
        if (!IsSecondaryOpcodeValid(current_ins)) {
            LOG(ERROR, VERIFIER) << "Fail to verify secondary opcode!";
            return false;
        }
        if (!IsInstructionEndInBounds(current_ins, bc_ins_last)) {
            return false;
        }
        current_ins = current_ins.GetNext();
        index++;
        instruction_index_map_[current_ins.GetAddress()] = index;
    }
    return true;
}

bool Verifier::IsInstructionEndInBounds(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last)
{
    return bc_ins.GetNext().GetAddress() <= bc_ins_last.GetAddress();
}

bool Verifier::IsMethodBytecodeInstruction(const BytecodeInstruction &bc_ins_cur)
{
    if (instruction_index_map_.find(bc_ins_cur.GetAddress()) != instruction_index_map_.end()) {
        return true;
    }
    return false;
}

bool Verifier::VerifyJumpInstruction(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                                     const BytecodeInstruction &bc_ins_first, const uint8_t *ins_arr,
                                     panda_file::File::EntityId code_id)
{
    const auto bc_ins_forward_size = bc_ins_last.GetAddress() - bc_ins.GetAddress();
    const auto bc_ins_backward_size = bc_ins.GetAddress() - bc_ins_first.GetAddress();

    if (bc_ins.IsJumpInstruction()) {
        std::optional<int64_t> immdata = GetFirstImmFromInstruction(bc_ins);
        if (!immdata.has_value()) {
            LOG(ERROR, VERIFIER) << "Fail to get immediate data!";
            return false;
        }
        if ((immdata.value() > 0) && (immdata.value() >= bc_ins_forward_size)) {
            LOG(ERROR, VERIFIER) << "Jump forward out of boundary";
            return false;
        }
        if ((immdata.value() < 0) && (bc_ins_backward_size + immdata.value() < 0)) {
            LOG(ERROR, VERIFIER) << "Jump backward out of boundary";
            return false;
        }

        const auto bc_ins_dest = bc_ins.JumpTo(immdata.value());
        if (!bc_ins_dest.IsPrimaryOpcodeValid()) {
            LOG(ERROR, VERIFIER) << "Fail to verify target jump primary opcode!";
            return false;
        }
        if (!IsSecondaryOpcodeValid(bc_ins_dest)) {
            LOG(ERROR, VERIFIER) << "Fail to verify target jump secondary opcode!";
            return false;
        }
        if (!IsMethodBytecodeInstruction(bc_ins_dest)) {
            LOG(ERROR, VERIFIER) << "> error encountered at " << code_id.GetOffset() << " (0x" << std::hex
                                 << code_id.GetOffset() << "). incorrect instruction at offset: 0x"
                                 << (bc_ins.GetAddress() - ins_arr) << ": invalid jump offset 0x" << immdata.value()
                                 << " - jumping in the middle of another instruction!";
            return false;
        }
    }

    return true;
}

bool Verifier::VerifyCatchBlockOffsets(panda_file::CodeDataAccessor::CatchBlock &catch_block,
                                       const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last)
{
    const auto handler_begin_offset = catch_block.GetHandlerPc();
    const auto handler_end_offset = handler_begin_offset + catch_block.GetCodeSize();

    const auto handler_begin_bc_ins = bc_ins.JumpTo(handler_begin_offset);
    const auto handler_end_bc_ins = bc_ins.JumpTo(handler_end_offset);

    const bool handler_begin_offset_in_range = bc_ins_last.GetAddress() > handler_begin_bc_ins.GetAddress();
    const bool handler_end_offset_in_range = bc_ins_last.GetAddress() >= handler_end_bc_ins.GetAddress();

    if (!handler_begin_offset_in_range) {
        LOG(ERROR, VERIFIER) << "> Invalid catch block begin offset range! address is: 0x" << std::hex
                             << handler_begin_bc_ins.GetAddress();
        return false;
    }
    if (!IsMethodBytecodeInstruction(handler_begin_bc_ins)) {
        LOG(ERROR, VERIFIER) << "> Invalid catch block begin offset validity! address is: 0x" << std::hex
                             << handler_begin_bc_ins.GetAddress();
        return false;
    }
    if (!handler_end_offset_in_range) {
        LOG(ERROR, VERIFIER) << "> Invalid catch block end offset range! address is: 0x" << std::hex
                             << handler_end_bc_ins.GetAddress();
        return false;
    }
    if (!IsMethodBytecodeInstruction(handler_end_bc_ins)) {
        LOG(ERROR, VERIFIER) << "> Invalid catch block end offset validity! address is: 0x" << std::hex
                             << handler_end_bc_ins.GetAddress();
        return false;
    }
    return true;
}

bool Verifier::VerifyCatchBlocks(panda_file::CodeDataAccessor::TryBlock &try_block, const BytecodeInstruction &bc_ins,
                                 const BytecodeInstruction &bc_ins_last, const panda_file::File::EntityId &method_id)
{
    bool result = true;

    try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
        const auto type_idx = catch_block.GetTypeIdx();
        if (type_idx != panda_file::INVALID_INDEX) {
            const auto catch_cls_id = file_->ResolveClassIndex(method_id, type_idx);
            if (!catch_cls_id.IsValid() || catch_cls_id.GetOffset() >= file_->GetHeader()->file_size) {
                LOG(ERROR, VERIFIER) << "> Invalid catch block type idx! type_idx is: 0x" << std::hex << type_idx
                                     << " of method 0x" << method_id.GetOffset();
                result = false;
                return false;
            }
        }
        if (!VerifyCatchBlockOffsets(catch_block, bc_ins, bc_ins_last)) {
            result = false;
            return false;
        }
        return true;
    });

    return result;
}

bool Verifier::VerifyTryBlockOffsets(const BytecodeInstruction &try_begin_bc_ins,
                                     const BytecodeInstruction &try_end_bc_ins,
                                     const BytecodeInstruction &bc_ins_last)
{
    if (bc_ins_last.GetAddress() <= try_begin_bc_ins.GetAddress()) {
        LOG(ERROR, VERIFIER) << "> Invalid try block begin offset range! address is: 0x" << std::hex
                             << try_begin_bc_ins.GetAddress();
        return false;
    }
    if (!IsMethodBytecodeInstruction(try_begin_bc_ins)) {
        LOG(ERROR, VERIFIER) << "> Invalid try block begin offset validity! address is: 0x" << std::hex
                             << try_begin_bc_ins.GetAddress();
        return false;
    }
    if (bc_ins_last.GetAddress() < try_end_bc_ins.GetAddress()) {
        LOG(ERROR, VERIFIER) << "> Invalid try block end offset range! address is: 0x" << std::hex
                             << try_end_bc_ins.GetAddress();
        return false;
    }
    if (!IsMethodBytecodeInstruction(try_end_bc_ins)) {
        LOG(ERROR, VERIFIER) << "> Invalid try block end offset validity! address is: 0x" << std::hex
                             << try_end_bc_ins.GetAddress();
        return false;
    }
    return true;
}

bool Verifier::VerifyTryBlocks(panda_file::CodeDataAccessor &code_accessor, const BytecodeInstruction &bc_ins,
                               const BytecodeInstruction &bc_ins_last, const panda_file::File::EntityId &method_id)
{
    bool result = true;

    code_accessor.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        const auto try_begin_bc_ins = bc_ins.JumpTo(try_block.GetStartPc());
        const auto try_end_bc_ins = bc_ins.JumpTo(try_block.GetStartPc() + try_block.GetLength());
        if (!VerifyTryBlockOffsets(try_begin_bc_ins, try_end_bc_ins, bc_ins_last)) {
            result = false;
            return false;
        }
        if (!VerifyCatchBlocks(try_block, bc_ins, bc_ins_last, method_id)) {
            LOG(ERROR, VERIFIER) << "Catch block validation failed!";
            result = false;
            return false;
        }

        return true;
    });

    return result;
}

bool Verifier::VerifyMethodRegisterIndex(panda_file::CodeDataAccessor &code_accessor,
                                         std::optional<uint64_t> &valid_regs_num)
{
    const uint64_t reg_nums = code_accessor.GetNumVregs();
    const uint64_t arg_nums = code_accessor.GetNumArgs();
    valid_regs_num = SafeAdd(reg_nums, arg_nums);
    if (!valid_regs_num.has_value()) {
        LOG(ERROR, VERIFIER) << "Integer overflow detected during register index calculation!";
        return false;
    }
    if (valid_regs_num.value() > MAX_REGISTER_INDEX + 1) {
        LOG(ERROR, VERIFIER) << "Register index exceeds the maximum allowable value (0xffff)!";
        return false;
    }
    return true;
}

bool Verifier::VerifyMethodNumArgs(panda_file::CodeDataAccessor &code_accessor,
                                   panda_file::MethodDataAccessor &method_accessor)
{
    const uint64_t num_args = code_accessor.GetNumArgs();
    uint32_t call_type = GetCallTypeFromAnnotation(method_accessor).value_or(DEFAULT_CALL_TYPE) & CALL_TYPE_MASK;
    uint64_t num_args_lower_limit = ((call_type & HAVE_THIS_BIT) != 0 ? 1 : 0) +
                                    ((call_type & HAVE_NEW_TARGET_BIT) != 0 ? 1 : 0) +
                                    ((call_type & HAVE_FUNC_BIT) != 0 ? 1 : 0);
    if (num_args < num_args_lower_limit) {
        LOG(ERROR, VERIFIER) << "Function argument number " << num_args << " is less than the lower limit "
                             << num_args_lower_limit << "!";
        return false;
    }
    return VerifyProtoArgNumber(num_args, method_accessor);
}

bool Verifier::VerifyProtoArgNumber(uint64_t num_args, panda_file::MethodDataAccessor &method_accessor)
{
    if (method_accessor.GetProtoIdx() == panda_file::INVALID_INDEX_16) {
        return true;
    }
    const auto proto_arg_num = GetProtoArgNumber(method_accessor.GetProtoId());
    if (!proto_arg_num.has_value()) {
        LOG(ERROR, VERIFIER) << "Fail to get the parameter number of the proto of method 0x" << std::hex
                             << method_accessor.GetMethodId().GetOffset() << "!";
        return false;
    }
    uint64_t expected = static_cast<uint64_t>(proto_arg_num.value()) + (method_accessor.IsStatic() ? 0U : 1U);
    if (num_args != expected) {
        LOG(ERROR, VERIFIER) << "Function argument number " << num_args
                             << " does not match the parameter number of the proto " << expected << "!";
        return false;
    }
    return true;
}

std::optional<uint32_t> Verifier::GetProtoArgNumber(const panda_file::File::EntityId &proto_id) const
{
    if (proto_id.GetOffset() <= sizeof(panda_file::File::Header) ||
        proto_id.GetOffset() >= file_->GetHeader()->file_size) {
        return std::nullopt;
    }
    constexpr size_t ELEMS_PER_BLOCK = panda_file::SHORTY_ELEM_PER16;
    constexpr size_t ELEM_WIDTH = panda_file::SHORTY_ELEM_WIDTH;
    constexpr size_t ELEM_MASK = panda_file::SHORTY_ELEM_MASK;
    auto sp = file_->GetSpanFromId(proto_id);
    auto block_opt = ReadBounded(&sp, panda_file::SHORTY_ELEM_SIZE);
    if (!block_opt.has_value()) {
        return std::nullopt;
    }
    uint32_t block = static_cast<uint32_t>(block_opt.value());
    uint32_t elem_num = 0;
    while (block != 0) {
        size_t shift = (elem_num % ELEMS_PER_BLOCK) * ELEM_WIDTH;
        if (((block >> shift) & ELEM_MASK) == 0) {
            break;
        }
        elem_num++;
        if (elem_num % ELEMS_PER_BLOCK == 0) {
            block_opt = ReadBounded(&sp, panda_file::SHORTY_ELEM_SIZE);
            if (!block_opt.has_value()) {
                return std::nullopt;
            }
            block = static_cast<uint32_t>(block_opt.value());
        }
    }
    return elem_num == 0 ? std::nullopt : std::optional<uint32_t>(elem_num - 1U);
}

void Verifier::ReportCorruptedInstructionSequence(const panda_file::File::EntityId &code_id,
                                                  const panda_file::File::EntityId &method_id) const
{
    LOG(ERROR, VERIFIER) << "> error encountered at " << code_id.GetOffset() << " (0x" << std::hex
                         << code_id.GetOffset() << "). bytecode instructions sequence corrupted for method 0x"
                         << method_id.GetOffset() << "! went out of bounds";
}

bool Verifier::VerifyMethodInstructions(MethodInfos &infos)
{
    auto current_ins = infos.bc_ins_init;
    auto last_ins = infos.bc_ins_last;
    auto code_id = infos.method_accessor.GetCodeId().value();
    auto method_id = infos.method_id;
    auto valid_regs_num = infos.valid_regs_num;

    while (current_ins.GetAddress() != last_ins.GetAddress()) {
        if (current_ins.GetAddress() > last_ins.GetAddress()) {
            ReportCorruptedInstructionSequence(code_id, method_id);
            return false;
        }
        if (!current_ins.IsJumpInstruction() && !current_ins.IsReturnOrThrowInstruction() &&
            current_ins.GetNext().GetAddress() == last_ins.GetAddress()) {
            ReportCorruptedInstructionSequence(code_id, method_id);
            return false;
        }
        const size_t count = GetVRegCount(current_ins);
        if (count != 0 && !CheckVRegIdx(current_ins, count, valid_regs_num)) {
            return false;
        }
        if (!VerifyJumpInstruction(current_ins, last_ins, infos.bc_ins_init, infos.ins_arr, code_id)) {
            LOG(ERROR, VERIFIER) << "Invalid target position of jump instruction";
            return false;
        }
        if (infos.ic_state.ic_check_enabled && !CollectIcSlotInsn(current_ins, infos.ic_insns)) {
            return false;
        }
        current_ins = current_ins.GetNext();
    }
    return true;
}

bool Verifier::IsSecondaryOpcodeValid(const BytecodeInstruction &bc_ins)
{
    if (!bc_ins.IsPrefixed()) {
        return true;
    }
    uint8_t primary_opcode = bc_ins.GetPrimaryOpcode();
    uint8_t secondary_opcode = bc_ins.GetSecondaryOpcode();
    for (const auto &bound : PREFIX_OPCODE_BOUNDS) {
        if (primary_opcode == bound.prefix) {
            return secondary_opcode <= bound.last_secondary;
        }
    }
    return false;
}

bool Verifier::VerifyMethodInRecord(const uint32_t &method_id) const
{
    auto iter = method_class_map_.find(method_id);
    if (iter == method_class_map_.end()) {
        if (IsExternalMethodId(method_id)) {
            return true;
        }
        LOG(ERROR, VERIFIER) << "Method id(0x" << std::hex << method_id
                             << ") is not declared in any record of the abc file!";
        return false;
    }
    panda_file::MethodDataAccessor method_accessor(*file_, panda_file::File::EntityId(method_id));
    if (method_accessor.GetClassId().GetOffset() != iter->second) {
        LOG(ERROR, VERIFIER) << "Method id(0x" << std::hex << method_id << ") does not belong to the record 0x"
                             << std::hex << iter->second << "!";
        return false;
    }
    return true;
}

bool Verifier::VerifyMethodHeaderInfo(panda_file::CodeDataAccessor &code_accessor,
                                      panda_file::MethodDataAccessor &method_accessor, MethodInfos &infos)
{
    if (code_accessor.GetCodeSize() <= 0) {
        LOG(ERROR, VERIFIER) << "Fail to verify code size!";
        return false;
    }
    std::optional<uint64_t> valid_regs_num;
    if (!VerifyMethodRegisterIndex(code_accessor, valid_regs_num)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method register index!";
        return false;
    }
    infos.valid_regs_num = valid_regs_num.value();
    if (!VerifyMethodNumArgs(code_accessor, method_accessor)) {
        return false;
    }
    return true;
}

bool Verifier::VerifyMethodCodeContent(panda_file::CodeDataAccessor &code_accessor, MethodInfos &infos)
{
    if (!PrecomputeInstructionIndices(infos.bc_ins_init, infos.bc_ins_last)) {
        LOG(ERROR, VERIFIER) << "Fail to precompute instruction indices!";
        return false;
    }
    if (!IsMethodBytecodeInstruction(infos.bc_ins_init)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method first bytecode instruction!";
        return false;
    }
    if (!VerifyTryBlocks(code_accessor, infos.bc_ins_init, infos.bc_ins_last, infos.method_id)) {
        LOG(ERROR, VERIFIER) << "Fail to verify try blocks or catch blocks!";
        return false;
    }
    if (!VerifyMethodInstructions(infos)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method instructions!";
        return false;
    }
    if (!VerifyMethodRegisterInitialization(code_accessor)) {
        LOG(ERROR, VERIFIER) << "Fail to verify method register initialization!";
        return false;
    }
    if (infos.ic_state.ic_check_enabled && !VerifyIcSlotAllocation(infos.ic_insns, infos.ic_state, infos.method_id)) {
        return false;
    }
    return true;
}

bool Verifier::CheckConstantPoolMethodContent(const panda_file::File::EntityId &method_id)
{
    panda_file::MethodDataAccessor method_accessor(*file_, method_id);
    if (!method_accessor.GetCodeId().has_value()) {
        LOG(ERROR, VERIFIER) << "Fail to get code id!";
        return false;
    }
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    const auto ins_size = code_accessor.GetCodeSize();
    const auto ins_arr = code_accessor.GetInstructions();
    auto bc_ins = BytecodeInstruction(ins_arr);
    MethodInfos infos {bc_ins, bc_ins.JumpTo(ins_size), method_accessor, method_id, 0, ins_arr, {}, {}};
    PrepareIcSlotCheck(method_accessor, infos.ic_state);
    if (!VerifyMethodHeaderInfo(code_accessor, method_accessor, infos)) {
        return false;
    }
    if (!VerifyMethodCodeContent(code_accessor, infos)) {
        return false;
    }
    return true;
}

bool Verifier::CheckConstantPoolIndex() const
{
    for (auto &id : ins_method_ids_) {
        if (!VerifyMethodId(id)) {
            return false;
        }
        if (!VerifyMethodInRecord(id)) {
            return false;
        }
    }

    for (auto &id : ins_literal_ids_) {
        if (!VerifyLiteralId(id)) {
            return false;
        }
    }

    for (auto &id : ins_string_ids_) {
        if (!VerifyStringId(id)) {
            return false;
        }
    }

    return true;
}

bool Verifier::CheckConstantPoolIdsBounds() const
{
    const uint32_t file_size = file_->GetHeader()->file_size;
    for (const auto &id : constant_pool_ids_) {
        if (id == 0 || id >= file_size) {
            LOG(ERROR, VERIFIER) << "Constant pool id(0x" << std::hex << id << ") is out of bounds!";
            return false;
        }
    }
    return true;
}

std::optional<uint64_t> Verifier::SafeAdd(uint64_t a, uint64_t b) const
{
    if (a > std::numeric_limits<uint64_t>::max() - b) {
        return std::nullopt;
    }
    return a + b;
}
}  // namespace panda::verifier
