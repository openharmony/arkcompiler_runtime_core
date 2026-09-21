/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "class_data_accessor-inl.h"
#include "field_data_accessor-inl.h"

namespace panda::verifier {

void Verifier::CollectModuleLiteralId(const panda_file::File::EntityId &field_id)
{
    panda_file::FieldDataAccessor field_accessor(*file_, field_id);
    const auto literal_id = field_accessor.GetValue<uint32_t>();
    if (!literal_id.has_value()) {
        return;
    }
    if (literal_ids_.count(literal_id.value()) != 0) {
        module_literals_.insert(literal_id.value());
    }
}

void Verifier::CollectRecordField(panda_file::FieldDataAccessor &field_accessor,
                                  const panda_file::File::EntityId &record_id, bool is_esmodule_record_class,
                                  bool &record_is_esmodule, bool &record_has_tla)
{
    panda_file::File::StringData sd = file_->GetStringData(field_accessor.GetNameId());
    std::string_view field_name(utf::Mutf8AsCString(sd.data));
    const auto literal_id = field_accessor.GetValue<uint32_t>();
    if (field_name == MODULE_RECORD_IDX_FIELD) {
        record_is_esmodule = true;
        esmodule_record_ids_.insert(record_id.GetOffset());
        if (literal_id.has_value()) {
            module_record_literal_ids_.insert(literal_id.value());
        }
    } else if (is_esmodule_record_class) {
        if (literal_id.has_value()) {
            module_record_literal_ids_.insert(literal_id.value());
        }
    } else if (field_name == HAS_TOP_LEVEL_AWAIT_FIELD) {
        if (literal_id.has_value() && literal_id.value() != 0) {
            record_has_tla = true;
            tla_record_ids_.insert(record_id.GetOffset());
        }
    } else if (field_name == MODULE_REQUEST_PHASE_IDX_FIELD) {
        if (literal_id.has_value()) {
            lazy_import_literal_ids_.insert(literal_id.value());
        }
    }
}

void Verifier::CollectRecordInfo(panda_file::ClassDataAccessor &class_accessor,
                                 const panda_file::File::EntityId &record_id, bool &record_is_esmodule,
                                 bool &record_has_tla)
{
    record_is_esmodule = false;
    record_has_tla = false;
    std::string_view record_name(utf::Mutf8AsCString(class_accessor.GetDescriptor()));
    bool is_esmodule_record_class = record_name == ES_MODULE_RECORD_CLASS;
    class_accessor.EnumerateFields([&](panda_file::FieldDataAccessor &field_accessor) -> void {
        CollectModuleLiteralId(field_accessor.GetFieldId());
        CollectRecordField(field_accessor, record_id, is_esmodule_record_class, record_is_esmodule, record_has_tla);
    });
}

bool Verifier::DetectSpecialRecords()
{
    return ForEachInternalRecord([this](const panda_file::File::EntityId & /*record_id*/,
                                        panda_file::ClassDataAccessor &class_accessor) -> bool {
        std::string_view record_name(utf::Mutf8AsCString(class_accessor.GetDescriptor()));
        if (record_name == ES_MODULE_RECORD_CLASS) {
            has_esmodule_record_class_ = true;
        } else if (record_name == HAS_TLA_CLASS) {
            has_tla_class_ = true;
        }
        return true;
    });
}

bool Verifier::VerifyModuleRequests(Span<const uint8_t> *sp, uint32_t num_module_requests)
{
    for (uint32_t idx = 0; idx < num_module_requests; idx++) {
        auto module_request_opt = ReadBounded(sp, panda_file::ID_SIZE);
        if (!module_request_opt.has_value()) {
            LOG(ERROR, VERIFIER) << "Module record is out of bounds!";
            return false;
        }
        if (!VerifyStringItem(static_cast<uint32_t>(module_request_opt.value()), "module record")) {
            return false;
        }
    }
    return true;
}

bool Verifier::VerifyModuleEntries(Span<const uint8_t> *sp, uint32_t num_module_requests, uint32_t string_id_count,
                                   bool has_module_request_idx)
{
    auto num_opt = ReadBounded(sp, panda_file::ID_SIZE);
    if (!num_opt.has_value()) {
        LOG(ERROR, VERIFIER) << "Module record is out of bounds!";
        return false;
    }
    for (uint32_t idx = 0; idx < num_opt.value(); idx++) {
        for (uint32_t id_idx = 0; id_idx < string_id_count; id_idx++) {
            auto string_id = ReadBounded(sp, panda_file::ID_SIZE);
            if (!string_id.has_value() ||
                !VerifyStringItem(static_cast<uint32_t>(string_id.value()), "module record")) {
                LOG(ERROR, VERIFIER) << "Invalid module record entry!";
                return false;
            }
        }
        if (has_module_request_idx) {
            auto module_request_idx = ReadBounded(sp, sizeof(uint16_t));
            if (!module_request_idx.has_value() || module_request_idx.value() >= num_module_requests) {
                LOG(ERROR, VERIFIER) << "Invalid module record entry!";
                return false;
            }
        }
    }
    return true;
}

bool Verifier::VerifyModuleRecord(const panda_file::File::EntityId &module_id)
{
    if (module_id.GetOffset() == 0 || module_id.GetOffset() >= file_->GetHeader()->file_size) {
        LOG(ERROR, VERIFIER) << "Invalid module record id 0x" << std::hex << module_id.GetOffset() << "!";
        return false;
    }
    auto sp = file_->GetSpanFromId(module_id);
    if (!ReadBounded(&sp, panda_file::ID_SIZE).has_value()) {
        LOG(ERROR, VERIFIER) << "Module record 0x" << std::hex << module_id.GetOffset() << " is out of bounds!";
        return false;
    }
    auto num_module_requests_opt = ReadBounded(&sp, panda_file::ID_SIZE);
    if (!num_module_requests_opt.has_value()) {
        LOG(ERROR, VERIFIER) << "Module record 0x" << std::hex << module_id.GetOffset() << " is out of bounds!";
        return false;
    }
    const auto num_module_requests = static_cast<uint32_t>(num_module_requests_opt.value());
    if (!VerifyModuleRequests(&sp, num_module_requests)) {
        return false;
    }
    static constexpr std::array<ModuleEntrySection, MODULE_ENTRY_SECTION_NUM> MODULE_ENTRY_SECTIONS {{
        {2, true},   // regular import
        {1, true},   // namespace import
        {2, false},  // local export
        {2, true},   // indirect export
        {0, true},   // star export
    }};
    for (const auto &section : MODULE_ENTRY_SECTIONS) {
        if (!VerifyModuleEntries(&sp, num_module_requests, section.string_id_count, section.has_module_request_idx)) {
            return false;
        }
    }
    return true;
}

bool Verifier::VerifyLazyImportFlags(const panda_file::File::EntityId &literal_id)
{
    if (literal_id.GetOffset() == 0 || literal_id.GetOffset() >= file_->GetHeader()->file_size) {
        LOG(ERROR, VERIFIER) << "Invalid lazy import literal id 0x" << std::hex << literal_id.GetOffset() << "!";
        return false;
    }
    auto sp = file_->GetSpanFromId(literal_id);
    auto num_opt = ReadBounded(&sp, panda_file::ID_SIZE);
    if (!num_opt.has_value() || num_opt.value() > static_cast<uint64_t>(sp.Size())) {
        LOG(ERROR, VERIFIER) << "Lazy import flag array 0x" << std::hex << literal_id.GetOffset()
                             << " is out of bounds!";
        return false;
    }
    return true;
}

bool Verifier::IsModuleLiteralId(const panda_file::File::EntityId &id) const
{
    return module_literals_.find(id.GetOffset()) != module_literals_.end();
}

bool Verifier::IsModuleRelatedInstruction(const BytecodeInstruction &bc_ins)
{
    return OpcodeIsAnyOf(bc_ins.GetOpcode(), kModuleOpcodes);
}

void Verifier::AddJumpTargetIfInRange(const BytecodeInstruction &current_ins, const BytecodeInstruction &bc_ins,
                                      const BytecodeInstruction &bc_ins_last,
                                      std::unordered_set<const uint8_t *> &block_entries)
{
    if (!current_ins.IsJumpInstruction()) {
        return;
    }
    std::optional<int64_t> immdata = GetFirstImmFromInstruction(current_ins);
    if (!immdata.has_value()) {
        return;
    }
    const auto target = current_ins.JumpTo(immdata.value());
    const bool in_range =
        target.GetAddress() >= bc_ins.GetAddress() && target.GetAddress() < bc_ins_last.GetAddress();
    if (in_range) {
        block_entries.insert(target.GetAddress());
    }
}

void Verifier::CollectTlaBlockEntries(panda_file::CodeDataAccessor &code_accessor, const BytecodeInstruction &bc_ins,
                                      const BytecodeInstruction &bc_ins_last,
                                      std::unordered_set<const uint8_t *> &block_entries)
{
    block_entries.insert(bc_ins.GetAddress());
    code_accessor.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            block_entries.insert(bc_ins.JumpTo(catch_block.GetHandlerPc()).GetAddress());
            return true;
        });
        return true;
    });
    auto current_ins = bc_ins;
    while (current_ins.GetAddress() < bc_ins_last.GetAddress()) {
        AddJumpTargetIfInRange(current_ins, bc_ins, bc_ins_last, block_entries);
        current_ins = current_ins.GetNext();
    }
}

bool Verifier::VerifyTlaMainFunctionReturn(panda_file::MethodDataAccessor &method_accessor)
{
    panda_file::File::StringData name_data = file_->GetStringData(method_accessor.GetNameId());
    std::string_view method_name(utf::Mutf8AsCString(name_data.data));
    if (method_name != MAIN_FUNCTION_NAME) {
        return true;
    }
    if (!method_accessor.GetCodeId().has_value()) {
        return true;
    }
    panda_file::CodeDataAccessor code_accessor(*file_, method_accessor.GetCodeId().value());
    auto bc_ins = BytecodeInstruction(code_accessor.GetInstructions());
    const auto bc_ins_last = bc_ins.JumpTo(code_accessor.GetCodeSize());
    std::unordered_set<const uint8_t *> block_entries;
    CollectTlaBlockEntries(code_accessor, bc_ins, bc_ins_last, block_entries);
    bool falls_through = true;
    auto current_ins = bc_ins;
    while (current_ins.GetAddress() < bc_ins_last.GetAddress()) {
        bool reachable = falls_through || block_entries.count(current_ins.GetAddress()) > 0;
        if (reachable && current_ins.GetOpcode() == Opcode::RETURNUNDEFINED) {
            LOG(ERROR, VERIFIER) << "The func_main_0 of a module with top level await returns undefined!";
            return false;
        }
        bool ends_block =
            current_ins.IsReturnOrThrowInstruction() ||
            (current_ins.IsJumpInstruction() && !current_ins.HasFlag(BytecodeInstruction::Flags::CONDITIONAL));
        falls_through = reachable && !ends_block;
        current_ins = current_ins.GetNext();
    }
    return true;
}

}  // namespace panda::verifier
