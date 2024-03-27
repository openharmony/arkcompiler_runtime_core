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

#ifndef VERIFIER_VERIFIER_H
#define VERIFIER_VERIFIER_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "bytecode_instruction_enum_gen.h"
#include "bytecode_instruction-inl.h"
#include "code_data_accessor-inl.h"
#include "file.h"
#include "file_items.h"
#include "literal_data_accessor.h"
#include "method_data_accessor-inl.h"
#include "utils/utf.h"

namespace panda::verifier {

using Opcode = BytecodeInstruction::Opcode;

enum class ActionType {
    CHECKCONSTPOOLCONTENT,
    COLLECTINFOS,
};

class Verifier {
public:
    explicit Verifier(const std::string &filename);
    ~Verifier() = default;

    bool Verify();
    void CollectIdInfos();
    bool VerifyChecksum();
    bool VerifyConstantPool();
    bool VerifyRegisterIndex();
    bool VerifyConstantPoolIndex();
    bool VerifyConstantPoolContent();

    std::vector<uint32_t> literal_ids_;
    std::unordered_map<uint32_t, uint32_t> inner_literal_map_;
    std::unordered_map<uint32_t, uint32_t> inner_method_map_;

private:
    void GetLiteralIds();
    void GetConstantPoolIds();
    bool CollectIdInInstructions(const panda_file::File::EntityId &method_id);
    void CollectModuleLiteralId(const panda_file::File::EntityId &field_id);
    bool CheckConstantPool(const verifier::ActionType type);
    size_t GetVRegCount(const BytecodeInstruction &bc_ins);
    bool CheckConstantPoolActions(const verifier::ActionType type, panda_file::File::EntityId method_id);
    bool VerifyMethodId(const uint32_t &method_id) const;
    bool VerifyLiteralId(const uint32_t &literal_id) const;
    bool VerifyStringId(const uint32_t &string_id) const;
    bool CheckVRegIdx(const BytecodeInstruction &bc_ins, const size_t count, const uint32_t max_reg_idx);
    std::optional<int64_t> GetFirstImmFromInstruction(const BytecodeInstruction &bc_ins);
    std::optional<uint64_t> GetSlotNumberFromAnnotation(panda_file::MethodDataAccessor &method_accessor);
    bool VerifyMethodIdInLiteralArray(const uint32_t &id);
    bool VerifyStringIdInLiteralArray(const uint32_t &id);
    bool VerifyLiteralIdInLiteralArray(const uint32_t &id);
    bool IsModuleLiteralId(const panda_file::File::EntityId &id) const;
    bool VerifySingleLiteralArray(const panda_file::File::EntityId &literal_id);
    bool VerifyLiteralArrays();
    bool IsJumpInstruction(const Opcode &ins_opcode);
    bool VerifyJumpInstruction(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                               const BytecodeInstruction &bc_ins_first);
    bool GetIcSlotFromInstruction(const BytecodeInstruction &bc_ins, uint32_t &first_slot_index, bool &has_slot,
                                  bool &is_two_slot);
    bool VerifySlotNumber(panda_file::MethodDataAccessor &method_accessor, const uint32_t &slot_number,
                          const panda_file::File::EntityId &method_id);
    bool CheckConstantPoolMethodContent(const panda_file::File::EntityId &method_id);
    bool CheckConstantPoolIndex() const;

    std::unique_ptr<const panda_file::File> file_;
    std::vector<uint32_t> constant_pool_ids_;
    std::vector<uint32_t> all_method_ids_;
    std::unordered_set<uint32_t> ins_method_ids_;
    std::unordered_set<uint32_t> ins_literal_ids_;
    std::unordered_set<uint32_t> ins_string_ids_;
    std::unordered_set<uint32_t> module_literals_;
    static constexpr size_t DEFAULT_ARGUMENT_NUMBER = 3;
    static constexpr uint32_t FILE_CONTENT_OFFSET = 12U;
};
} // namespace panda::verifier
#endif
