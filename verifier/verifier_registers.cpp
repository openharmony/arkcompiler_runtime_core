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

namespace panda::verifier {

bool Verifier::IsRegisterWriteInstruction(const BytecodeInstruction &bc_ins)
{
    return OpcodeIsAnyOf(bc_ins.GetOpcode(), kRegisterWriteOpcodes);
}

void Verifier::CollectInsnRegInfo(const BytecodeInstruction &bc_ins, uint64_t num_regs, InsnRegInfo &info,
                                  std::vector<bool> &written_anywhere) const
{
    if (IsRegisterWriteInstruction(bc_ins)) {
        uint16_t write_reg = bc_ins.GetVReg(FIRST_INDEX);
        if (write_reg < num_regs) {
            written_anywhere[write_reg] = true;
            info.write_reg = write_reg;
        }
        if (bc_ins.GetOpcode() != Opcode::STA_V8) {
            uint16_t read_reg = bc_ins.GetVReg(SECOND_INDEX);
            if (read_reg < num_regs) {
                info.read_regs.push_back(read_reg);
            }
        }
        return;
    }
    if (bc_ins.GetOpcode() == Opcode::RESUMEGENERATOR ||
        bc_ins.GetOpcode() == Opcode::DEPRECATED_RESUMEGENERATOR_PREF_V8) {
        info.define_all_regs = true;
        return;
    }
    if (bc_ins.IsRangeInstruction()) {
        auto last_reg_idx = bc_ins.GetRangeInsLastRegIdx();
        if (last_reg_idx.has_value() && last_reg_idx.value() < num_regs) {
            for (uint64_t reg = bc_ins.GetVReg(FIRST_INDEX); reg <= last_reg_idx.value(); reg++) {
                info.read_regs.push_back(static_cast<uint16_t>(reg));
            }
        }
        return;
    }
    const size_t count = GetVRegCount(bc_ins);
    for (size_t idx = 0; idx < count; idx++) {
        uint16_t read_reg = bc_ins.GetVReg(idx);
        if (read_reg < num_regs) {
            info.read_regs.push_back(read_reg);
        }
    }
}

void Verifier::CollectInsnControlFlow(const BytecodeInstruction &bc_ins, InsnRegInfo &info) const
{
    bool is_return =
        bc_ins.HasFlag(BytecodeInstruction::Flags::RETURN) || bc_ins.IsThrow(BytecodeInstruction::Exceptions::X_THROW);
    bool is_jump = bc_ins.IsJumpInstruction();
    bool is_unconditional_jump = is_jump && !bc_ins.HasFlag(BytecodeInstruction::Flags::CONDITIONAL);
    info.has_fallthrough = !is_return && !is_unconditional_jump;
    if (is_jump) {
        std::optional<int64_t> immdata = GetFirstImmFromInstruction(bc_ins);
        if (immdata.has_value()) {
            const auto bc_ins_dest = bc_ins.JumpTo(immdata.value());
            auto iter = instruction_index_map_.find(bc_ins_dest.GetAddress());
            if (iter != instruction_index_map_.end()) {
                info.jump_target_index = iter->second;
            }
        }
    }
}

void Verifier::SeedCatchHandlerStates(panda_file::CodeDataAccessor &code_accessor, RegisterDefinitionDataflow &dataflow)
{
    code_accessor.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            const auto handler_bc_ins =
                BytecodeInstruction(code_accessor.GetInstructions()).JumpTo(catch_block.GetHandlerPc());
            auto iter = instruction_index_map_.find(handler_bc_ins.GetAddress());
            if (iter != instruction_index_map_.end()) {
                dataflow.SeedAllDefined(iter->second);
            }
            return true;
        });
        return true;
    });
}

bool Verifier::VerifyMethodRegisterInitialization(panda_file::CodeDataAccessor &code_accessor)
{
    const uint64_t num_vregs = code_accessor.GetNumVregs();
    const uint64_t num_args = code_accessor.GetNumArgs();
    const uint64_t num_regs = num_vregs + num_args;
    if (num_regs == 0 || instruction_index_map_.empty()) {
        return true;
    }
    const size_t insn_num = instruction_index_map_.size() - 1U;
    if (insn_num == 0) {
        return true;
    }
    if (static_cast<uint64_t>(insn_num) * num_regs > MAX_REGISTER_ANALYSIS_COMPLEXITY ||
        static_cast<uint64_t>(insn_num) * num_regs * num_regs > MAX_REGISTER_ANALYSIS_TIME_COMPLEXITY) {
        LOG(WARNING, VERIFIER) << "Skip register initialization analysis for a too large method!";
        return true;
    }
    std::vector<InsnRegInfo> insn_infos(insn_num);
    std::vector<bool> written_anywhere(num_regs, false);
    CollectMethodInsnRegInfos(code_accessor, num_regs, insn_infos, written_anywhere);
    RegisterDefinitionDataflow dataflow(insn_num, num_regs);
    dataflow.SeedEntry(num_vregs, num_regs, written_anywhere);
    SeedCatchHandlerStates(code_accessor, dataflow);
    dataflow.Run(insn_infos, num_vregs);
    return CheckInsnRegisterInitialization(insn_infos, dataflow);
}

void Verifier::CollectMethodInsnRegInfos(panda_file::CodeDataAccessor &code_accessor, uint64_t num_regs,
                                         std::vector<InsnRegInfo> &insn_infos, std::vector<bool> &written_anywhere)
{
    const uint8_t *ins_arr = code_accessor.GetInstructions();
    size_t insn_index = 0;
    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(code_accessor.GetCodeSize());
    while (bc_ins.GetAddress() < bc_ins_last.GetAddress() && insn_index < insn_infos.size()) {
        CollectInsnRegInfo(bc_ins, num_regs, insn_infos[insn_index], written_anywhere);
        CollectInsnControlFlow(bc_ins, insn_infos[insn_index]);
        insn_infos[insn_index].offset = static_cast<uint32_t>(bc_ins.GetAddress() - ins_arr);
        insn_index++;
        bc_ins = bc_ins.GetNext();
    }
}

bool Verifier::CheckInsnRegisterInitialization(const std::vector<InsnRegInfo> &insn_infos,
                                               const RegisterDefinitionDataflow &dataflow)
{
    for (size_t index = 0; index < insn_infos.size(); index++) {
        if (!dataflow.IsReachable(index)) {
            continue;
        }
        for (const auto &reg : insn_infos[index].read_regs) {
            if (!dataflow.IsDefined(index, reg)) {
                LOG(ERROR, VERIFIER) << "Read of uninitialized register v" << reg << " at instruction offset: 0x"
                                     << std::hex << insn_infos[index].offset << "!";
                return false;
            }
        }
    }
    return true;
}

}  // namespace panda::verifier
