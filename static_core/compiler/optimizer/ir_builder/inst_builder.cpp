/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "inst_builder.h"
#include "phi_resolver.h"
#include "optimizer/code_generator/encode.h"
#include "compiler_logger.h"
#ifndef PANDA_TARGET_WINDOWS
#include "callconv.h"
#endif

namespace panda::compiler {

/* static */
DataType::Type InstBuilder::ConvertPbcType(panda_file::Type type)
{
    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return DataType::VOID;
        case panda_file::Type::TypeId::U1:
            return DataType::BOOL;
        case panda_file::Type::TypeId::I8:
            return DataType::INT8;
        case panda_file::Type::TypeId::U8:
            return DataType::UINT8;
        case panda_file::Type::TypeId::I16:
            return DataType::INT16;
        case panda_file::Type::TypeId::U16:
            return DataType::UINT16;
        case panda_file::Type::TypeId::I32:
            return DataType::INT32;
        case panda_file::Type::TypeId::U32:
            return DataType::UINT32;
        case panda_file::Type::TypeId::I64:
            return DataType::INT64;
        case panda_file::Type::TypeId::U64:
            return DataType::UINT64;
        case panda_file::Type::TypeId::F32:
            return DataType::FLOAT32;
        case panda_file::Type::TypeId::F64:
            return DataType::FLOAT64;
        case panda_file::Type::TypeId::REFERENCE:
            return DataType::REFERENCE;
        case panda_file::Type::TypeId::TAGGED:
        case panda_file::Type::TypeId::INVALID:
        default:
            UNREACHABLE();
    }
    UNREACHABLE();
}

void InstBuilder::Prepare(bool is_inlined_graph)
{
    SetCurrentBlock(GetGraph()->GetStartBlock());
#ifndef PANDA_TARGET_WINDOWS
    GetGraph()->ResetParameterInfo();
#endif
    // Create parameter for actual num args
    if (!GetGraph()->IsBytecodeOptimizer() && GetGraph()->IsDynamicMethod() && !GetGraph()->GetMode().IsDynamicStub()) {
        auto param_inst = GetGraph()->AddNewParameter(ParameterInst::DYNAMIC_NUM_ARGS);
        param_inst->SetType(DataType::UINT32);
        param_inst->SetLocationData(GetGraph()->GetDataForNativeParam(DataType::UINT32));
    }
    size_t arg_ref_num = 0;
    if (GetRuntime()->GetMethodReturnType(GetMethod()) == DataType::REFERENCE) {
        arg_ref_num = 1;
    }
    auto num_args = GetRuntime()->GetMethodTotalArgumentsCount(GetMethod());
    bool is_static = GetRuntime()->IsMethodStatic(GetMethod());
    // Create Parameter instructions for all arguments
    for (size_t i = 0; i < num_args; i++) {
        auto param_inst = GetGraph()->AddNewParameter(i);
        auto type = GetCurrentMethodArgumentType(i);
        auto reg_num = GetRuntime()->GetMethodRegistersCount(GetMethod()) + i;
        ASSERT(!GetGraph()->IsBytecodeOptimizer() || reg_num != INVALID_REG);

        param_inst->SetType(type);
        // This parameter in virtaul method is implicit, so skipped
        if (type == DataType::REFERENCE && (is_static || i > 0)) {
            param_inst->SetArgRefNumber(arg_ref_num++);
        }
        SetParamSpillFill(GetGraph(), param_inst, num_args, i, type);

        UpdateDefinition(reg_num, param_inst);
    }

    // We don't need to create SafePoint at the beginning of the callee graph
    if (OPTIONS.IsCompilerUseSafepoint() && !is_inlined_graph) {
        GetGraph()->GetStartBlock()->AppendInst(CreateSafePoint(GetGraph()->GetStartBlock()));
    }
    method_profile_ = GetRuntime()->GetMethodProfile(GetMethod(), !GetGraph()->IsAotMode());
}

void InstBuilder::UpdateDefsForCatch()
{
    Inst *catch_phi = current_bb_->GetFirstInst();
    ASSERT(catch_phi != nullptr);
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        ASSERT(catch_phi->IsCatchPhi());
        defs_[current_bb_->GetId()][vreg] = catch_phi;
        catch_phi = catch_phi->GetNext();
    }
}

void InstBuilder::UpdateDefsForLoopHead()
{
    // If current block is a loop header, then propagate all definitions from preheader's predecessors to
    // current block.
    ASSERT(current_bb_->GetLoop()->GetPreHeader());
    auto pred_defs = defs_[current_bb_->GetLoop()->GetPreHeader()->GetId()];
    COMPILER_LOG(DEBUG, IR_BUILDER) << "basic block is loop header";
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        auto def_inst = pred_defs[vreg];
        if (def_inst != nullptr) {
            auto phi = GetGraph()->CreateInstPhi();
            if (vreg > vregs_and_args_count_) {
                phi->SetType(DataType::ANY);
            }
            phi->SetMarker(GetNoTypeMarker());
            phi->SetLinearNumber(vreg);
            current_bb_->AppendPhi(phi);
            (*current_defs_)[vreg] = phi;
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create Phi(id=" << phi->GetId() << ") for r" << vreg
                                            << "(def id=" << pred_defs[vreg]->GetId() << ")";
        }
    }
}

bool InstBuilder::UpdateDefsForPreds(size_t vreg, std::optional<Inst *> &value)
{
    for (auto pred_bb : current_bb_->GetPredsBlocks()) {
        // When irreducible loop header is visited before it's back-edge, phi should be created,
        // since we do not know if definitions are different at this point
        if (!pred_bb->IsMarked(visited_block_marker_)) {
            ASSERT(current_bb_->GetLoop()->IsIrreducible());
            return true;
        }
        if (!value.has_value()) {
            value = defs_[pred_bb->GetId()][vreg];
        } else if (value.value() != defs_[pred_bb->GetId()][vreg]) {
            return true;
        }
    }
    return false;
}

void InstBuilder::UpdateDefs()
{
    current_bb_->SetMarker(visited_block_marker_);
    if (current_bb_->IsCatchBegin()) {
        UpdateDefsForCatch();
    } else if (current_bb_->IsLoopHeader() && !current_bb_->GetLoop()->IsIrreducible()) {
        UpdateDefsForLoopHead();
    } else if (current_bb_->GetPredsBlocks().size() == 1) {
        // Only one predecessor - simply copy all its definitions
        auto &pred_defs = defs_[current_bb_->GetPredsBlocks()[0]->GetId()];
        std::copy(pred_defs.begin(), pred_defs.end(), current_defs_->begin());
    } else if (current_bb_->GetPredsBlocks().size() > 1) {
        // If there are multiple predecessors, then add phi for each register that has different definitions
        for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
            std::optional<Inst *> value;
            bool different = UpdateDefsForPreds(vreg, value);
            if (different) {
                auto phi = GetGraph()->CreateInstPhi();
                phi->SetMarker(GetNoTypeMarker());
                phi->SetLinearNumber(vreg);
                current_bb_->AppendPhi(phi);
                (*current_defs_)[vreg] = phi;
                COMPILER_LOG(DEBUG, IR_BUILDER) << "create Phi(id=" << phi->GetId() << ") for r" << vreg;
            } else {
                (*current_defs_)[vreg] = value.value();
            }
        }
    }
}

void InstBuilder::AddCatchPhiInputs(const ArenaUnorderedSet<BasicBlock *> &catch_handlers, const InstVector &defs,
                                    Inst *throwable_inst)
{
    ASSERT(!catch_handlers.empty());
    for (auto catch_bb : catch_handlers) {
        auto inst = catch_bb->GetFirstInst();
        while (!inst->IsCatchPhi()) {
            inst = inst->GetNext();
        }
        ASSERT(inst != nullptr);
        GetGraph()->AppendThrowableInst(throwable_inst, catch_bb);
        for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++, inst = inst->GetNext()) {
            ASSERT(inst->GetOpcode() == Opcode::CatchPhi);
            auto catch_phi = inst->CastToCatchPhi();
            if (catch_phi->IsAcc()) {
                ASSERT(vreg == vregs_and_args_count_);
                continue;
            }
            auto input_inst = defs[vreg];
            if (input_inst != nullptr && input_inst != catch_phi) {
                catch_phi->AppendInput(input_inst);
                catch_phi->AppendThrowableInst(throwable_inst);
            }
        }
    }
}

void InstBuilder::SetParamSpillFill(Graph *graph, ParameterInst *param_inst, size_t num_args, size_t i,
                                    DataType::Type type)
{
    if (graph->IsBytecodeOptimizer()) {
        auto reg_src = static_cast<Register>(VIRTUAL_FRAME_SIZE - num_args + i);
        DataType::Type reg_type;
        if (DataType::IsReference(type)) {
            reg_type = DataType::REFERENCE;
        } else if (DataType::Is64Bits(type, graph->GetArch())) {
            reg_type = DataType::UINT64;
        } else {
            reg_type = DataType::UINT32;
        }

        param_inst->SetLocationData({LocationType::REGISTER, LocationType::REGISTER, reg_src, reg_src, reg_type});
    } else {
#ifndef PANDA_TARGET_WINDOWS
        if (graph->IsDynamicMethod() && !graph->GetMode().IsDynamicStub()) {
            ASSERT(type == DataType::ANY);
            uint16_t slot = i + CallConvDynInfo::FIXED_SLOT_COUNT;
            ASSERT(slot <= UINT8_MAX);
            param_inst->SetLocationData(
                {LocationType::STACK_PARAMETER, LocationType::INVALID, slot, INVALID_REG, DataType::UINT64});
        } else {
            param_inst->SetLocationData(graph->GetDataForNativeParam(type));
        }
#endif
    }
}

/// Set type of instruction, then recursively set type to its inputs.
void InstBuilder::SetTypeRec(Inst *inst, DataType::Type type)
{
    inst->SetType(type);
    inst->ResetMarker(GetNoTypeMarker());
    for (auto input : inst->GetInputs()) {
        if (input.GetInst()->IsMarked(GetNoTypeMarker())) {
            SetTypeRec(input.GetInst(), type);
        }
    }
}

/**
 * Remove vreg from SaveState for the case
 * BB 1
 *   ....
 * succs: [bb 2, bb 3]
 *
 * BB 2: preds: [bb 1]
 *   89.i64  Sub                        v85, v88 -> (v119, v90)
 *   90.f64  Cast                       v89 -> (v96, v92)
 * succs: [bb 3]
 *
 * BB 3: preds: [bb 1, bb 2]
 *   .....
 *   119.     SaveState                  v105(vr0), v106(vr1), v94(vr4), v89(vr8), v0(vr10), v1(vr11) -> (v120)
 *
 * v89(vr8) used only in BB 2, so we need to remove its from "119.     SaveState"
 */
/* static */
void InstBuilder::RemoveNotDominateInputs(SaveStateInst *save_state)
{
    size_t idx = 0;
    size_t inputs_count = save_state->GetInputsCount();
    while (idx < inputs_count) {
        auto input_inst = save_state->GetInput(idx).GetInst();
        // We can don't call IsDominate, if save_state and input_inst in one basic block.
        // It's reduce number of IsDominate calls.
        if (!input_inst->InSameBlockOrDominate(save_state)) {
            save_state->RemoveInput(idx);
            inputs_count--;
        } else {
            ASSERT(input_inst->GetBasicBlock() != save_state->GetBasicBlock() || input_inst->IsDominate(save_state));
            idx++;
        }
    }
}

/// Fix instructions that can't be fully completed in building process.
void InstBuilder::FixInstructions()
{
    // Remove dead Phi and set types to phi which have not type.
    // Phi may not have type if all it users are pseudo instructions, like SaveState
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->PhiInstsSafe()) {
            inst->ReserveInputs(bb->GetPredsBlocks().size());
            for (auto &pred_bb : bb->GetPredsBlocks()) {
                if (inst->GetLinearNumber() == INVALID_LINEAR_NUM) {
                    continue;
                }
                auto pred = defs_[pred_bb->GetId()][inst->GetLinearNumber()];
                if (pred == nullptr) {
                    // If any input of phi instruction is not defined then we assume that phi is dead. DCE should
                    // remove it.
                    continue;
                }
                inst->AppendInput(pred);
            }
        }
    }

    // Check all instructions that have no type and fix it. Type is got from instructions with known input types.
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            if (inst->IsSaveState()) {
                RemoveNotDominateInputs(static_cast<SaveStateInst *>(inst));
                continue;
            }
            auto input_idx = 0;
            for (auto input : inst->GetInputs()) {
                if (input.GetInst()->IsMarked(GetNoTypeMarker())) {
                    auto input_type = inst->GetInputType(input_idx);
                    if (input_type != DataType::NO_TYPE) {
                        SetTypeRec(input.GetInst(), input_type);
                    }
                }
                input_idx++;
            }
        }
    }
    // Resolve dead and inconsistent phi instructions
    PhiResolver phi_resolver(GetGraph());
    phi_resolver.Run();
    ResolveConstants();
}

SaveStateInst *InstBuilder::CreateSaveState(Opcode opc, size_t pc)
{
    ASSERT(opc == Opcode::SaveState || opc == Opcode::SafePoint || opc == Opcode::SaveStateOsr ||
           opc == Opcode::SaveStateDeoptimize);
    SaveStateInst *inst;
    bool without_numeric_inputs = false;
    auto live_vergs_count =
        std::count_if(current_defs_->begin(), current_defs_->end(), [](Inst *p) { return p != nullptr; });
    if (opc == Opcode::SaveState) {
        inst = GetGraph()->CreateInstSaveState(pc, GetMethod(), caller_inst_, inlining_depth_);
    } else if (opc == Opcode::SaveStateOsr) {
        inst = GetGraph()->CreateInstSaveStateOsr(pc, GetMethod(), caller_inst_, inlining_depth_);
    } else if (opc == Opcode::SafePoint) {
        inst = GetGraph()->CreateInstSafePoint(pc, GetMethod(), caller_inst_, inlining_depth_);
        without_numeric_inputs = true;
    } else {
        inst = GetGraph()->CreateInstSaveStateDeoptimize(pc, GetMethod(), caller_inst_, inlining_depth_);
    }
    if (GetGraph()->IsBytecodeOptimizer()) {
        inst->ReserveInputs(0);
        return inst;
    }
    inst->ReserveInputs(live_vergs_count);

    for (VirtualRegister::ValueType reg_idx = 0; reg_idx < vregs_and_args_count_; ++reg_idx) {
        auto def_inst = (*current_defs_)[reg_idx];
        if (def_inst != nullptr && (!without_numeric_inputs || !DataType::IsTypeNumeric(def_inst->GetType()))) {
            auto input_idx = inst->AppendInput(def_inst);
            inst->SetVirtualRegister(input_idx, VirtualRegister(reg_idx, VRegType::VREG));
        }
    }
    VirtualRegister::ValueType reg_idx = vregs_and_args_count_;
    auto def_inst = (*current_defs_)[reg_idx];
    if (def_inst != nullptr && (!without_numeric_inputs || !DataType::IsTypeNumeric(def_inst->GetType()))) {
        auto input_idx = inst->AppendInput(def_inst);
        inst->SetVirtualRegister(input_idx, VirtualRegister(reg_idx, VRegType::ACC));
    }
    reg_idx++;
    if (GetGraph()->IsDynamicMethod() && !GetGraph()->IsBytecodeOptimizer() && (*current_defs_)[reg_idx] != nullptr) {
        for (uint8_t env_idx = 0; env_idx < VRegInfo::ENV_COUNT; ++env_idx) {
            auto input_idx = inst->AppendInput(GetEnvDefinition(env_idx));
            inst->SetVirtualRegister(input_idx, VirtualRegister(reg_idx++, VRegType(VRegType::ENV_BEGIN + env_idx)));
        }
        if (additional_def_ != nullptr) {
            inst->AppendBridge(additional_def_);
        }
    }
    return inst;
}

ClassInst *InstBuilder::CreateLoadAndInitClassGeneric(uint32_t class_id, size_t pc)
{
    auto class_ptr = GetRuntime()->ResolveType(GetGraph()->GetMethod(), class_id);
    ClassInst *inst = nullptr;
    if (class_ptr == nullptr) {
        ASSERT(!graph_->IsBytecodeOptimizer());
        inst = graph_->CreateInstUnresolvedLoadAndInitClass(DataType::REFERENCE, pc, nullptr, class_id,
                                                            GetGraph()->GetMethod(), class_ptr);
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), class_id,
                                                             UnresolvedTypesInterface::SlotKind::CLASS);
        }
    } else {
        inst = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, nullptr, class_id, GetGraph()->GetMethod(),
                                                  class_ptr);
    }
    return inst;
}

DataType::Type InstBuilder::GetCurrentMethodReturnType() const
{
    return GetRuntime()->GetMethodReturnType(GetMethod());
}

DataType::Type InstBuilder::GetCurrentMethodArgumentType(size_t index) const
{
    return GetRuntime()->GetMethodTotalArgumentType(GetMethod(), index);
}

size_t InstBuilder::GetCurrentMethodArgumentsCount() const
{
    return GetRuntime()->GetMethodTotalArgumentsCount(GetMethod());
}

DataType::Type InstBuilder::GetMethodReturnType(uintptr_t id) const
{
    return GetRuntime()->GetMethodReturnType(GetMethod(), id);
}

DataType::Type InstBuilder::GetMethodArgumentType(uintptr_t id, size_t index) const
{
    return GetRuntime()->GetMethodArgumentType(GetMethod(), id, index);
}

size_t InstBuilder::GetMethodArgumentsCount(uintptr_t id) const
{
    return GetRuntime()->GetMethodArgumentsCount(GetMethod(), id);
}

size_t InstBuilder::GetPc(const uint8_t *inst_ptr) const
{
    return inst_ptr - instructions_buf_;
}

void InstBuilder::ResolveConstants()
{
    ConstantInst *curr_const = GetGraph()->GetFirstConstInst();
    while (curr_const != nullptr) {
        SplitConstant(curr_const);
        curr_const = curr_const->GetNextConst();
    }
}

void InstBuilder::SplitConstant(ConstantInst *const_inst)
{
    if (const_inst->GetType() != DataType::INT64 || !const_inst->HasUsers()) {
        return;
    }
    auto users = const_inst->GetUsers();
    auto curr_it = users.begin();
    while (curr_it != users.end()) {
        auto user = (*curr_it).GetInst();
        DataType::Type type = user->GetInputType(curr_it->GetIndex());
        ++curr_it;
        if (type != DataType::FLOAT32 && type != DataType::FLOAT64) {
            continue;
        }
        ConstantInst *new_const = nullptr;
        if (type == DataType::FLOAT32) {
            auto val = bit_cast<float>(static_cast<uint32_t>(const_inst->GetIntValue()));
            new_const = GetGraph()->FindOrCreateConstant(val);
        } else {
            auto val = bit_cast<double, uint64_t>(const_inst->GetIntValue());
            new_const = GetGraph()->FindOrCreateConstant(val);
        }
        user->ReplaceInput(const_inst, new_const);
    }
}

void InstBuilder::SyncWithGraph()
{
    size_t idx = current_defs_ - &defs_[0];
    size_t size = defs_.size();
    defs_.resize(graph_->GetVectorBlocks().size(), InstVector(graph_->GetLocalAllocator()->Adapter()));
    for (size_t i = size; i < defs_.size(); i++) {
        defs_[i].resize(vregs_and_args_count_ + 1 + graph_->GetEnvCount());
        std::copy(defs_[idx].cbegin(), defs_[idx].cend(), defs_[i].begin());
    }
    current_defs_ = &defs_[current_bb_->GetId()];
}

}  // namespace panda::compiler
