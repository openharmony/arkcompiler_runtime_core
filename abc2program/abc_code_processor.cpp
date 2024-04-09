/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "mangling.h"
#include "method_data_accessor-inl.h"

namespace panda::abc2program {

AbcCodeProcessor::AbcCodeProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container,
                                   panda_file::File::EntityId method_id, pandasm::Function &function)
    : AbcFileEntityProcessor(entity_id, entity_container), method_id_(method_id), function_(function)
{
    code_data_accessor_ = std::make_unique<panda_file::CodeDataAccessor>(*file_, entity_id_);
    code_converter_ = std::make_unique<AbcCodeConverter>(entity_container_);
}

void AbcCodeProcessor::FillProgramData()
{
    FillFunctionRegsNum();
    FillIns();
    FillCatchBlocks();
}

void AbcCodeProcessor::FillFunctionRegsNum()
{
    function_.regs_num = code_data_accessor_->GetNumVregs();
}

void AbcCodeProcessor::FillIns()
{
    FillInsWithoutLabels();
    if (NeedToAddDummyEndIns()) {
        AddDummyEndIns();
    }
    AddJumpLabels();
}

void AbcCodeProcessor::FillInsWithoutLabels()
{
    ins_size_ = code_data_accessor_->GetCodeSize();
    const uint8_t *ins_arr = code_data_accessor_->GetInstructions();
    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size_);
    uint32_t inst_pc = 0;
    uint32_t inst_idx = 0;
    while (bc_ins.GetAddress() != bc_ins_last.GetAddress()) {
        pandasm::Ins pa_ins = code_converter_->BytecodeInstructionToPandasmInstruction(bc_ins, method_id_);
        /*
         * Private field jump_inst_idx_vec_ store all jump inst idx in a pandasm::Function.
         * For example, a pandasm::Function has 100 pandasm::Ins with only 4 jump pandasm::Ins.
         * When we add label for jump target and add jump id for jump inst,
         * we only need to enumerate the 4 jump pandasm::Ins which stored in jump_inst_idx_vec_,
         * while no need to enumerate the whole 100 pandasm::Ins.
         * It will improve our compile performance.
        */
        if (pa_ins.IsJump()) {
            jump_inst_idx_vec_.emplace_back(inst_idx);
        }
        function_.AddInstruction(pa_ins);
        inst_pc_idx_map_.emplace(inst_pc, inst_idx);
        inst_idx_pc_map_.emplace(inst_idx, inst_pc);
        inst_idx++;
        inst_pc += bc_ins.GetSize();
        bc_ins = bc_ins.GetNext();
    }
}

bool AbcCodeProcessor::NeedToAddDummyEndIns() const
{
    bool need_to_add_dummy_end_ins = false;
    code_data_accessor_->EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            uint32_t catch_end_pc = catch_block.GetHandlerPc() + catch_block.GetCodeSize();
            if (catch_end_pc == ins_size_) {
                need_to_add_dummy_end_ins = true;
            }
            return true;
        });
        return true;
    });
    return need_to_add_dummy_end_ins;
}

void AbcCodeProcessor::AddDummyEndIns()
{
    uint32_t inst_idx = static_cast<uint32_t>(function_.ins.size());
    pandasm::Ins dummy_end_ins{};
    dummy_end_ins.opcode = pandasm::Opcode::INVALID;
    dummy_end_ins.label = AbcFileUtils::GetLabelNameByInstIdx(inst_idx);
    dummy_end_ins.set_label = true;
    inst_pc_idx_map_.emplace(ins_size_, inst_idx);
    inst_idx_pc_map_.emplace(inst_idx, ins_size_);
    function_.AddInstruction(dummy_end_ins);
}

uint32_t AbcCodeProcessor::GetInstIdxByInstPc(uint32_t inst_pc) const
{
    auto it = inst_pc_idx_map_.find(inst_pc);
    ASSERT(it != inst_pc_idx_map_.end());
    return it->second;
}

uint32_t AbcCodeProcessor::GetInstPcByInstIdx(uint32_t inst_idx) const
{
    auto it = inst_idx_pc_map_.find(inst_idx);
    ASSERT(it != inst_idx_pc_map_.end());
    return it->second;
}

void AbcCodeProcessor::AddJumpLabels() const
{
    for (uint32_t jump_inst_idx : jump_inst_idx_vec_) {
        pandasm::Ins &jump_pa_ins = function_.ins[jump_inst_idx];
        AddJumpLabel4InsAtIndex(jump_inst_idx, jump_pa_ins);
    }
}

void AbcCodeProcessor::AddJumpLabel4InsAtIndex(uint32_t curr_inst_idx, pandasm::Ins &curr_pa_ins) const
{
    uint32_t curr_inst_pc = GetInstPcByInstIdx(curr_inst_idx);
    const int32_t jmp_offset = std::get<int64_t>(curr_pa_ins.imms.at(0));
    uint32_t dst_inst_pc = curr_inst_pc + jmp_offset;
    uint32_t dst_inst_idx = GetInstIdxByInstPc(dst_inst_pc);
    pandasm::Ins &dst_pa_ins = function_.ins[dst_inst_idx];
    AddLabel4InsAtIndex(dst_inst_idx);
    curr_pa_ins.imms.clear();
    curr_pa_ins.ids.emplace_back(dst_pa_ins.label);
}

void AbcCodeProcessor::FillCatchBlocks()
{
    code_data_accessor_->EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        HandleTryBlock(try_block);
        try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            HandleCatchBlock(catch_block);
            return true;
        });
        return true;
    });
}

void AbcCodeProcessor::HandleTryBlock(panda_file::CodeDataAccessor::TryBlock &try_block)
{
    curr_try_begin_inst_pc_ = try_block.GetStartPc();
    curr_try_end_inst_pc_ = try_block.GetStartPc() + try_block.GetLength();
    AddLabel4InsAtPc(curr_try_begin_inst_pc_);
    AddLabel4InsAtPc(curr_try_end_inst_pc_);
}

void AbcCodeProcessor::HandleCatchBlock(panda_file::CodeDataAccessor::CatchBlock &catch_block)
{
    curr_catch_begin_pc_ = catch_block.GetHandlerPc();
    curr_catch_end_pc_ = catch_block.GetHandlerPc() + catch_block.GetCodeSize();
    AddLabel4InsAtPc(curr_catch_begin_pc_);
    AddLabel4InsAtPc(curr_catch_end_pc_);
    pandasm::Function::CatchBlock pa_catch_block{};
    FillCatchBlockLabels(pa_catch_block);
    FillExceptionRecord(catch_block, pa_catch_block);
    function_.catch_blocks.emplace_back(pa_catch_block);
}

void AbcCodeProcessor::FillCatchBlockLabels(pandasm::Function::CatchBlock &pa_catch_block) const
{
    pa_catch_block.try_begin_label = GetLabelNameAtPc(curr_try_begin_inst_pc_);
    pa_catch_block.try_end_label = GetLabelNameAtPc(curr_try_end_inst_pc_);
    pa_catch_block.catch_begin_label = GetLabelNameAtPc(curr_catch_begin_pc_);
    pa_catch_block.catch_end_label = GetLabelNameAtPc(curr_catch_end_pc_);
}

void AbcCodeProcessor::FillExceptionRecord(panda_file::CodeDataAccessor::CatchBlock &catch_block,
                                           pandasm::Function::CatchBlock &pa_catch_block) const
{
    uint32_t class_idx = catch_block.GetTypeIdx();
    if (class_idx == panda_file::INVALID_INDEX) {
        pa_catch_block.exception_record = "";
    } else {
        const panda_file::File::EntityId class_id = file_->ResolveClassIndex(method_id_, class_idx);
        pa_catch_block.exception_record = entity_container_.GetFullRecordNameById(class_id);
    }
}

void AbcCodeProcessor::AddLabel4InsAtIndex(uint32_t inst_idx) const
{
    pandasm::Ins &pa_ins = function_.ins[inst_idx];
    if (pa_ins.set_label) {
        return;
    }
    pa_ins.label = AbcFileUtils::GetLabelNameByInstIdx(inst_idx);
    pa_ins.set_label = true;
}

void AbcCodeProcessor::AddLabel4InsAtPc(uint32_t inst_pc) const
{
    uint32_t inst_idx = GetInstIdxByInstPc(inst_pc);
    AddLabel4InsAtIndex(inst_idx);
}

std::string AbcCodeProcessor::GetLabelNameAtPc(uint32_t inst_pc) const
{
    uint32_t inst_idx = GetInstIdxByInstPc(inst_pc);
    return AbcFileUtils::GetLabelNameByInstIdx(inst_idx);
}

} // namespace panda::abc2program
