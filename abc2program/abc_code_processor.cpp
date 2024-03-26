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
    AddJumpLabels();
}

void AbcCodeProcessor::FillInsWithoutLabels()
{
    const auto ins_size = code_data_accessor_->GetCodeSize();
    const uint8_t *ins_arr = code_data_accessor_->GetInstructions();
    auto bc_ins = BytecodeInstruction(ins_arr);
    const auto bc_ins_last = bc_ins.JumpTo(ins_size);
    size_t inst_pc = 0;
    size_t inst_idx = 0;
    while (bc_ins.GetAddress() != bc_ins_last.GetAddress()) {
        pandasm::Ins pa_ins = code_converter_->BytecodeInstructionToPandasmInstruction(bc_ins, method_id_);
        function_.ins.emplace_back(pa_ins);
        inst_pc_idx_map_.emplace(inst_pc, inst_idx);
        inst_idx_pc_map_.emplace(inst_idx, inst_pc);
        inst_idx++;
        inst_pc += bc_ins.GetSize();
        bc_ins = bc_ins.GetNext();
    }
}

size_t AbcCodeProcessor::GetInstIdxByInstPc(size_t inst_pc) const
{
    auto it = inst_pc_idx_map_.find(inst_pc);
    ASSERT(it != inst_pc_idx_map_.end());
    return it->second;
}

size_t AbcCodeProcessor::GetInstPcByInstIdx(size_t inst_idx) const
{
    auto it = inst_idx_pc_map_.find(inst_idx);
    ASSERT(it != inst_idx_pc_map_.end());
    return it->second;
}

void AbcCodeProcessor::AddJumpLabels()
{
    size_t inst_count = function_.ins.size();
    for (size_t curr_inst_idx = 0; curr_inst_idx < inst_count; ++curr_inst_idx) {
        pandasm::Ins &curr_pa_ins = function_.ins[curr_inst_idx];
        if (curr_pa_ins.IsJump()) {
            AddJumpLabel4InsAtIndex(curr_inst_idx, curr_pa_ins);
        }
    }
}

void AbcCodeProcessor::AddJumpLabel4InsAtIndex(size_t curr_inst_idx, pandasm::Ins &curr_pa_ins) const
{
    size_t curr_inst_pc = GetInstPcByInstIdx(curr_inst_idx);
    const int32_t jmp_offset = std::get<int64_t>(curr_pa_ins.imms.at(0));
    size_t dst_inst_pc = curr_inst_pc + jmp_offset;
    size_t dst_inst_idx = GetInstIdxByInstPc(dst_inst_pc);
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

void AbcCodeProcessor::AddLabel4InsAtIndex(size_t inst_idx) const
{
    pandasm::Ins &pa_ins = function_.ins[inst_idx];
    if (pa_ins.set_label) {
        return;
    }
    pa_ins.label = AbcFileUtils::GetLabelNameByInstIdx(inst_idx);
    pa_ins.set_label = true;
}

void AbcCodeProcessor::AddLabel4InsAtPc(size_t inst_pc) const
{
    size_t inst_idx = GetInstIdxByInstPc(inst_pc);
    AddLabel4InsAtIndex(inst_idx);
}

std::string AbcCodeProcessor::GetLabelNameAtPc(size_t inst_pc) const
{
    size_t inst_idx = GetInstIdxByInstPc(inst_pc);
    return AbcFileUtils::GetLabelNameByInstIdx(inst_idx);
}

} // namespace panda::abc2program
