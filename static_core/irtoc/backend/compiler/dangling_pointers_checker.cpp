/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "dangling_pointers_checker.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "runtime/interpreter/frame.h"
#include "runtime/include/managed_thread.h"

namespace panda::compiler {
DanglingPointersChecker::DanglingPointersChecker(Graph *graph)
    : Analysis {graph},
      objects_users_ {graph->GetLocalAllocator()->Adapter()},
      checked_blocks_ {graph->GetLocalAllocator()->Adapter()},
      phi_insts_ {graph->GetLocalAllocator()->Adapter()},
      object_insts_ {graph->GetLocalAllocator()->Adapter()}
{
}

bool DanglingPointersChecker::RunImpl()
{
    return CheckAccSyncCallRuntime();
}

static Inst *MoveToPrevInst(Inst *inst, BasicBlock *bb)
{
    if (inst == bb->GetFirstInst()) {
        return nullptr;
    }
    return inst->GetPrev();
}

static bool IsRefType(Inst *inst)
{
    return inst->GetType() == DataType::REFERENCE;
}

static bool IsPointerType(Inst *inst)
{
    return inst->GetType() == DataType::POINTER;
}

static bool IsObjectDef(Inst *inst)
{
    if (!IsRefType(inst) && !IsPointerType(inst)) {
        return false;
    }
    if (inst->IsPhi()) {
        return false;
    }
    if (inst->GetOpcode() != Opcode::AddI) {
        return true;
    }
    auto imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    return imm != static_cast<uint64_t>(cross_values::GetFrameAccOffset(inst->GetBasicBlock()->GetGraph()->GetArch()));
}

void DanglingPointersChecker::InitLiveIns()
{
    auto arch = GetGraph()->GetArch();
    for (auto inst : GetGraph()->GetStartBlock()->Insts()) {
        if (inst->GetOpcode() != Opcode::LiveIn) {
            continue;
        }
        if (inst->GetDstReg() == regmap_[arch]["acc"]) {
            acc_livein_ = inst;
        }
        if (inst->GetDstReg() == regmap_[arch]["acc_tag"]) {
            acc_tag_livein_ = inst;
        }
        if (inst->GetDstReg() == regmap_[arch]["frame"]) {
            frame_livein_ = inst;
        }
        if (inst->GetDstReg() == regmap_[arch]["thread"]) {
            thread_livein_ = inst;
        }
    }
}

// Frame can be defined in three ways:
// 1. LiveIn(frame)
// 2. LoadI(LiveIn(thread)).Imm(ManagedThread::GetFrameOffset())
// 3. LoadI(<another_frame_def>).Imm(Frame::GetPrevFrameOffset())

bool DanglingPointersChecker::IsFrameDef(Inst *inst)
{
    //    inst := LiveIn(frame)
    if (inst == frame_livein_) {
        return true;
    }
    // or
    //    inst := LoadI(inst_input).Imm(imm)
    if (inst->GetOpcode() == Opcode::LoadI) {
        // where
        //       inst_input := LiveIn(thread)
        //       imm := ManagedThread::GetFrameOffset()
        auto inst_input = inst->GetInput(0).GetInst();
        if (inst_input == thread_livein_ &&
            static_cast<LoadInstI *>(inst)->GetImm() == panda::ManagedThread::GetFrameOffset()) {
            return true;
        }
        // or
        //       inst_input := <frame_def>
        //       imm := Frame::GetPrevFrameOffset()
        if (static_cast<LoadInstI *>(inst)->GetImm() == static_cast<uint64_t>(panda::Frame::GetPrevFrameOffset()) &&
            IsFrameDef(inst_input)) {
            return true;
        }
    }
    return false;
}

bool DanglingPointersChecker::CheckSuccessors(BasicBlock *bb, bool prev_res)
{
    if (checked_blocks_.find(bb) != checked_blocks_.end()) {
        return prev_res;
    }
    for (auto succ_bb : bb->GetSuccsBlocks()) {
        if (!prev_res) {
            return false;
        }
        for (auto inst : succ_bb->AllInsts()) {
            auto user = std::find(objects_users_.begin(), objects_users_.end(), inst);
            if (user == objects_users_.end() || (*user)->IsPhi()) {
                continue;
            }
            return false;
        }
        checked_blocks_.insert(bb);

        prev_res &= CheckSuccessors(succ_bb, prev_res);
    }

    return prev_res;
}

// Accumulator can be defined in three ways:
// 1. acc_def := LiveIn(acc)
// 2. acc_def := LoadI(last_frame_def).Imm(frame_acc_offset)
// 3. acc_ptr := AddI(last_frame_def).Imm(frame_acc_offset)
//    acc_def := LoadI(acc_ptr).Imm(0)

std::tuple<Inst *, Inst *> DanglingPointersChecker::GetAccAndFrameDefs(Inst *inst)
{
    auto arch = GetGraph()->GetArch();
    if (inst == acc_livein_) {
        return std::make_pair(acc_livein_, nullptr);
    }
    if (inst->GetOpcode() != Opcode::LoadI) {
        return std::make_pair(nullptr, nullptr);
    }

    auto inst_input = inst->GetInput(0).GetInst();
    auto frame_acc_offset = static_cast<uint64_t>(cross_values::GetFrameAccOffset(arch));
    auto load_imm = static_cast<LoadInstI *>(inst)->GetImm();
    if (load_imm == frame_acc_offset && IsFrameDef(inst_input)) {
        return std::make_pair(inst, inst_input);
    }

    if (load_imm == 0 && IsAccPtr(inst_input)) {
        return std::make_pair(inst, inst_input->GetInput(0).GetInst());
    }

    return std::make_pair(nullptr, nullptr);
}

// Accumulator tag can be defined in three ways:
// 1. acc_tag_def := LiveIn(acc_tag)
// 2. acc_ptr     := AddI(last_frame_def).Imm(frame_acc_offset)
//    acc_tag_def := LoadI(acc_ptr).Imm(acc_tag_offset)
// 3. acc_ptr     := AddI(last_frame_def).Imm(frame_acc_offset)
//    acc_tag_ptr := AddI(acc_ptr).Imm(acc_tag_offset)
//    acc_tag_def := LoadI(acc_tag_ptr).Imm(0)

bool DanglingPointersChecker::IsAccTagDef(Inst *inst)
{
    if (inst == acc_tag_livein_) {
        return true;
    }
    if (inst->GetOpcode() != Opcode::LoadI) {
        return false;
    }

    auto inst_input = inst->GetInput(0).GetInst();
    auto arch = GetGraph()->GetArch();
    auto acc_tag_offset = static_cast<uint64_t>(cross_values::GetFrameAccMirrorOffset(arch));
    auto load_imm = static_cast<LoadInstI *>(inst)->GetImm();
    if (load_imm == acc_tag_offset && IsAccPtr(inst_input)) {
        return true;
    }

    if (load_imm == 0 && IsAccTagPtr(inst_input)) {
        return true;
    }

    return false;
}

bool DanglingPointersChecker::IsAccTagPtr(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::AddI) {
        return false;
    }
    auto arch = GetGraph()->GetArch();
    auto inst_imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    auto acc_tag_offset = static_cast<uint64_t>(cross_values::GetFrameAccMirrorOffset(arch));
    if (inst_imm != acc_tag_offset) {
        return false;
    }
    auto acc_ptr_inst = inst->GetInput(0).GetInst();
    return IsAccPtr(acc_ptr_inst);
}

bool DanglingPointersChecker::IsAccPtr(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::AddI) {
        return false;
    }
    auto arch = GetGraph()->GetArch();
    auto inst_imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    auto frame_acc_offset = static_cast<uint64_t>(cross_values::GetFrameAccOffset(arch));
    if (inst_imm != frame_acc_offset) {
        return false;
    }
    auto frame_inst = inst->GetInput(0).GetInst();
    if (!IsFrameDef(frame_inst)) {
        return false;
    }
    if (last_frame_def_ == nullptr) {
        return true;
    }
    return last_frame_def_ == frame_inst;
}

void DanglingPointersChecker::UpdateLastAccAndFrameDef(Inst *inst)
{
    auto [acc_def, frame_def] = GetAccAndFrameDefs(inst);
    if (acc_def != nullptr) {
        // inst is acc definition
        if (last_acc_def_ == nullptr) {
            // don't have acc definition before
            last_acc_def_ = acc_def;
            last_frame_def_ = frame_def;
        }
    } else {
        // inst isn't acc definition
        if (IsObjectDef(inst) && !IsPointerType(inst)) {
            // objects defs should be only ref type
            object_insts_.insert(inst);
        }
    }
}

void DanglingPointersChecker::GetLastAccDefinition(CallInst *runtime_call_inst)
{
    auto block = runtime_call_inst->GetBasicBlock();
    auto prev_inst = runtime_call_inst->GetPrev();

    phi_insts_.clear();
    object_insts_.clear();
    while (block != GetGraph()->GetStartBlock()) {
        while (prev_inst != nullptr) {
            UpdateLastAccAndFrameDef(prev_inst);

            if (last_acc_tag_def_ == nullptr && IsAccTagDef(prev_inst)) {
                last_acc_tag_def_ = prev_inst;
            }

            prev_inst = MoveToPrevInst(prev_inst, block);
        }

        object_insts_.insert(acc_livein_);

        for (auto *phi_inst : block->PhiInsts()) {
            phi_insts_.push_back(phi_inst);
        }
        block = block->GetDominator();
        prev_inst = block->GetLastInst();
    }

    // Check that accumulator has not been overwritten in any execution branch except restored acc
    auto [phi_def_acc, phi_def_frame] = GetPhiAccDef();
    if (phi_def_acc != nullptr) {
        last_acc_def_ = phi_def_acc;
        last_frame_def_ = phi_def_frame;
    }
    if (last_acc_tag_def_ == nullptr) {
        last_acc_tag_def_ = GetPhiAccTagDef();
    }

    if (last_acc_def_ == nullptr) {
        last_acc_def_ = acc_livein_;
    }

    if (last_acc_tag_def_ == nullptr) {
        last_acc_tag_def_ = acc_tag_livein_;
    }
}

std::tuple<Inst *, Inst *> DanglingPointersChecker::GetPhiAccDef()
{
    // If any input isn't a definition (or there are no definitions among its inputs),
    // then the phi is not a definition.
    // Otherwise, if we have reached the last input and it is a definition (or there is a definition in among its
    // inputs), then the phi is a definition.
    for (auto *phi_inst : phi_insts_) {
        bool is_acc_def_phi = true;
        auto inputs_count = phi_inst->GetInputsCount();
        Inst *acc_def {nullptr};
        Inst *frame_def {nullptr};
        for (uint32_t input_idx = 0; input_idx < inputs_count; input_idx++) {
            auto input_inst = phi_inst->GetInput(input_idx).GetInst();
            std::tie(acc_def, frame_def) = GetAccAndFrameDefs(input_inst);
            if (acc_def != nullptr || input_inst == nullptr) {
                continue;
            }
            if (input_inst->IsConst() ||
                (input_inst->GetOpcode() == Opcode::Bitcast && input_inst->GetInput(0).GetInst()->IsConst())) {
                acc_def = input_inst;
                continue;
            }
            std::tie(acc_def, frame_def) = GetAccDefFromInputs(input_inst);
            if (acc_def == nullptr) {
                is_acc_def_phi = false;
                break;
            }
        }
        if (!is_acc_def_phi) {
            continue;
        }
        if (acc_def != nullptr) {
            return std::make_pair(phi_inst, frame_def);
        }
    }
    return std::make_pair(nullptr, nullptr);
}

std::tuple<Inst *, Inst *> DanglingPointersChecker::GetAccDefFromInputs(Inst *inst)
{
    auto inputs_count = inst->GetInputsCount();
    Inst *acc_def {nullptr};
    Inst *frame_def {nullptr};
    for (uint32_t input_idx = 0; input_idx < inputs_count; input_idx++) {
        auto input_inst = inst->GetInput(input_idx).GetInst();

        std::tie(acc_def, frame_def) = GetAccAndFrameDefs(input_inst);
        if (acc_def != nullptr || input_inst == nullptr) {
            continue;
        }
        if (input_inst->IsConst() ||
            (input_inst->GetOpcode() == Opcode::Bitcast && input_inst->GetInput(0).GetInst()->IsConst())) {
            acc_def = input_inst;
            continue;
        }
        std::tie(acc_def, frame_def) = GetAccDefFromInputs(input_inst);
        if (acc_def == nullptr) {
            return std::make_pair(nullptr, nullptr);
        }
    }
    return std::make_pair(acc_def, frame_def);
}

Inst *DanglingPointersChecker::GetPhiAccTagDef()
{
    for (auto *phi_inst : phi_insts_) {
        if (IsRefType(phi_inst) || IsPointerType(phi_inst)) {
            continue;
        }
        auto inputs_count = phi_inst->GetInputsCount();
        for (uint32_t input_idx = 0; input_idx < inputs_count; input_idx++) {
            auto input_inst = phi_inst->GetInput(input_idx).GetInst();
            auto is_acc_tag_def = IsAccTagDef(input_inst);
            if (is_acc_tag_def || input_inst->IsConst()) {
                if (input_idx == inputs_count - 1) {
                    return phi_inst;
                }
                continue;
            }

            if (!IsAccTagDefInInputs(input_inst)) {
                break;
            }
            return phi_inst;
        }
    }
    return nullptr;
}

bool DanglingPointersChecker::IsAccTagDefInInputs(Inst *inst)
{
    auto inputs_count = inst->GetInputsCount();
    for (uint32_t input_idx = 0; input_idx < inputs_count; input_idx++) {
        auto input_inst = inst->GetInput(input_idx).GetInst();
        if (IsAccTagDef(input_inst)) {
            return true;
        }

        if ((input_idx == inputs_count - 1) && input_inst->IsConst()) {
            return true;
        }

        if (IsAccTagDefInInputs(input_inst)) {
            return true;
        }
    }
    return false;
}

bool DanglingPointersChecker::IsSaveAcc(const Inst *inst)
{
    if (inst->GetOpcode() != Opcode::StoreI) {
        return false;
    }

    auto arch = GetGraph()->GetArch();
    auto frame_acc_offset = static_cast<uint64_t>(cross_values::GetFrameAccOffset(arch));
    if (static_cast<const StoreInstI *>(inst)->GetImm() != frame_acc_offset) {
        return false;
    }
    auto store_input_1 = inst->GetInput(1).GetInst();
    if (store_input_1 != last_acc_def_) {
        return false;
    }
    auto store_input_0 = inst->GetInput(0).GetInst();
    if (last_frame_def_ == nullptr) {
        if (IsFrameDef(store_input_0)) {
            return true;
        }
    } else if (store_input_0 == last_frame_def_) {
        return true;
    }
    return false;
}

// Accumulator is saved using the StoreI instruction:
// StoreI(last_frame_def, last_acc_def).Imm(cross_values::GetFrameAccOffset(GetArch()))

bool DanglingPointersChecker::CheckStoreAcc(CallInst *runtime_call_inst)
{
    auto prev_inst = runtime_call_inst->GetPrev();
    auto block = runtime_call_inst->GetBasicBlock();
    while (block != GetGraph()->GetStartBlock()) {
        while (prev_inst != nullptr && prev_inst != last_acc_def_) {
            if (IsSaveAcc(prev_inst)) {
                return true;
            }
            prev_inst = MoveToPrevInst(prev_inst, block);
        }
        block = block->GetDominator();
        prev_inst = block->GetLastInst();
    }
    return false;
}

// Accumulator tag is saved using the StoreI instruction:
// StoreI(acc_ptr, last_acc_tag_def).Imm(cross_values::GetFrameAccMirrorOffset(GetArch()))

bool DanglingPointersChecker::CheckStoreAccTag(CallInst *runtime_call_inst)
{
    bool is_save_acc_tag = false;
    auto arch = GetGraph()->GetArch();
    auto prev_inst = runtime_call_inst->GetPrev();
    auto block = runtime_call_inst->GetBasicBlock();
    auto acc_tag_offset = static_cast<uint64_t>(cross_values::GetFrameAccMirrorOffset(arch));
    while (block != GetGraph()->GetStartBlock()) {
        while (prev_inst != nullptr && prev_inst != last_acc_def_) {
            if (prev_inst->GetOpcode() != Opcode::StoreI) {
                prev_inst = MoveToPrevInst(prev_inst, block);
                continue;
            }
            if (static_cast<StoreInstI *>(prev_inst)->GetImm() != acc_tag_offset) {
                prev_inst = MoveToPrevInst(prev_inst, block);
                continue;
            }
            auto store_input_1 = prev_inst->GetInput(1).GetInst();
            if (last_acc_tag_def_ == nullptr) {
                last_acc_tag_def_ = store_input_1;
            }
            if (store_input_1 != last_acc_tag_def_ && !store_input_1->IsConst()) {
                prev_inst = MoveToPrevInst(prev_inst, block);
                continue;
            }
            auto store_input_0 = prev_inst->GetInput(0).GetInst();
            if (IsAccPtr(store_input_0)) {
                is_save_acc_tag = true;
                break;
            }

            prev_inst = MoveToPrevInst(prev_inst, block);
        }
        if (is_save_acc_tag) {
            break;
        }
        block = block->GetDominator();
        prev_inst = block->GetLastInst();
    }
    return is_save_acc_tag;
}

bool DanglingPointersChecker::CheckAccUsers(CallInst *runtime_call_inst)
{
    objects_users_.clear();
    for (const auto &user : last_acc_def_->GetUsers()) {
        objects_users_.push_back(user.GetInst());
    }

    return CheckUsers(runtime_call_inst);
}

bool DanglingPointersChecker::CheckObjectsUsers(CallInst *runtime_call_inst)
{
    objects_users_.clear();
    for (auto *object_inst : object_insts_) {
        for (const auto &user : object_inst->GetUsers()) {
            objects_users_.push_back(user.GetInst());
        }
    }

    return CheckUsers(runtime_call_inst);
}

bool DanglingPointersChecker::CheckUsers(CallInst *runtime_call_inst)
{
    bool check_object_users = true;
    auto runtime_call_block = runtime_call_inst->GetBasicBlock();

    auto next_inst = runtime_call_inst->GetNext();
    while (next_inst != nullptr) {
        auto user = std::find(objects_users_.begin(), objects_users_.end(), next_inst);
        if (user == objects_users_.end() || (*user)->IsPhi()) {
            next_inst = next_inst->GetNext();
            continue;
        }
        return false;
    }

    checked_blocks_.clear();
    return CheckSuccessors(runtime_call_block, check_object_users);
}

bool DanglingPointersChecker::CheckAccSyncCallRuntime()
{
    if (regmap_.find(GetGraph()->GetArch()) == regmap_.end()) {
        return true;
    }

    if (GetGraph()->GetRelocationHandler() == nullptr) {
        return true;
    }

    // collect runtime calls
    ArenaVector<CallInst *> runtime_calls(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (!inst->IsCall()) {
                continue;
            }
            auto call_inst = static_cast<CallInst *>(inst);
            auto call_func_name =
                GetGraph()->GetRuntime()->GetExternalMethodName(GetGraph()->GetMethod(), call_inst->GetCallMethodId());
            if (target_funcs_.find(call_func_name) == target_funcs_.end()) {
                continue;
            }
            runtime_calls.push_back(call_inst);
        }
    }
    if (runtime_calls.empty()) {
        return true;
    }

    // find LiveIns for acc and frame
    InitLiveIns();

    for (auto runtime_call_inst : runtime_calls) {
        last_acc_def_ = nullptr;
        last_acc_tag_def_ = nullptr;
        GetLastAccDefinition(runtime_call_inst);

        if (!IsRefType(last_acc_def_) && !IsPointerType(last_acc_def_)) {
            continue;
        }

        // check that acc has been stored in the frame before call
        if (!CheckStoreAcc(runtime_call_inst)) {
            return false;
        }

        if (!GetGraph()->IsDynamicMethod() && !CheckStoreAccTag(runtime_call_inst)) {
            return false;
        }

        // check that acc isn't used after call
        if (!CheckAccUsers(runtime_call_inst)) {
            return false;
        }

        // check that other objects aren't used after call
        if (!CheckObjectsUsers(runtime_call_inst)) {
            return false;
        }
    }
    return true;
}
}  // namespace panda::compiler
