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

#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "vn.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "compiler_logger.h"

namespace panda::compiler {
class BasicBlock;

inline bool IsNotCommutativeInst(Inst *inst)
{
    return !inst->IsCommutative() || DataType::IsFloatType(inst->GetType());
}

static bool AddClassInst(Inst *inst, VnObject *obj)
{
    if (inst->GetOpcode() != Opcode::LoadAndInitClass && inst->GetOpcode() != Opcode::InitClass &&
        inst->GetOpcode() != Opcode::LoadClass) {
        return false;
    }

    obj->Add(static_cast<uint32_t>(Opcode::InitClass));
    auto class_inst = static_cast<ClassInst *>(inst);
    auto klass = class_inst->GetClass();
    if (klass == nullptr) {
        obj->Add(class_inst->GetTypeId());
    } else {
        obj->Add(reinterpret_cast<uint64_t>(klass));
    }
    inst->SetVnObject(obj);
    return true;
}

static bool AddGlobalVarInst(Inst *inst, VnObject *obj)
{
    if (inst->GetOpcode() != Opcode::GetGlobalVarAddress) {
        return false;
    }

    obj->Add(static_cast<uint32_t>(Opcode::GetGlobalVarAddress));
    obj->Add(inst->CastToGetGlobalVarAddress()->GetTypeId());
    inst->SetVnObject(obj);
    return true;
}

static bool AddCommutativeInst(Inst *inst, VnObject *obj)
{
    if (IsNotCommutativeInst(inst)) {
        return false;
    }

    [[maybe_unused]] constexpr auto INPUTS_NUM = 2;
    ASSERT(inst->GetInputsCount() == INPUTS_NUM);
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    ASSERT(input0->GetVN() != INVALID_VN);
    ASSERT(input1->GetVN() != INVALID_VN);
    if (input0->GetId() > input1->GetId()) {
        obj->Add(input0->GetVN());
        obj->Add(input1->GetVN());
    } else {
        obj->Add(input1->GetVN());
        obj->Add(input0->GetVN());
    }

    inst->SetVnObject(obj);
    return true;
}

static void AddSpecialTraits(Inst *inst, VnObject *obj)
{
    switch (inst->GetOpcode()) {
        case Opcode::Intrinsic:
            if (inst->GetFlagsMask() == 0) {
                /* Add only those intrinsics that have no flags set */
                obj->Add(static_cast<uint32_t>(inst->CastToIntrinsic()->GetIntrinsicId()));
            }
            break;
        case Opcode::CompareAnyType:
            obj->Add(static_cast<uint32_t>(inst->CastToCompareAnyType()->GetAnyType()));
            break;
        case Opcode::CastAnyTypeValue:
            obj->Add(static_cast<uint32_t>(inst->CastToCastAnyTypeValue()->GetAnyType()));
            break;
        case Opcode::CastValueToAnyType:
            obj->Add(static_cast<uint32_t>(inst->CastToCastValueToAnyType()->GetAnyType()));
            break;
        default:
            break;
    }
}

static bool IsIrreducibleClassInst(Inst *inst, Inst *equiv_inst)
{
    return equiv_inst->GetOpcode() == Opcode::LoadClass &&
           (inst->GetOpcode() == Opcode::InitClass || inst->GetOpcode() == Opcode::LoadAndInitClass);
}

static bool AddResolver(Inst *inst, VnObject *obj)
{
    if (!inst->IsResolver()) {
        return false;
    }

    for (auto input : inst->GetInputs()) {
        auto input_inst = inst->GetDataFlowInput(input.GetInst());
        if (!input_inst->IsSaveState()) {
            auto vn = input_inst->GetVN();
            ASSERT(vn != INVALID_VN);
            obj->Add(vn);
        }
    }

    if (inst->GetOpcode() == Opcode::ResolveVirtual) {
        obj->Add(inst->CastToResolveVirtual()->GetCallMethodId());
    } else if (inst->GetOpcode() == Opcode::ResolveStatic) {
        obj->Add(inst->CastToResolveStatic()->GetCallMethodId());
    } else if (inst->GetOpcode() == Opcode::ResolveObjectField) {
        obj->Add(inst->CastToResolveObjectField()->GetTypeId());
        obj->Add(reinterpret_cast<uint64_t>(inst->CastToResolveObjectField()->GetMethod()));
    } else if (inst->GetOpcode() == Opcode::ResolveObjectFieldStatic) {
        obj->Add(inst->CastToResolveObjectFieldStatic()->GetTypeId());
        obj->Add(reinterpret_cast<uint64_t>(inst->CastToResolveObjectFieldStatic()->GetMethod()));
    } else {
        UNREACHABLE();
    }

    inst->SetVnObject(obj);
    return true;
}

void VnObject::Add(Inst *inst)
{
    if (AddClassInst(inst, this)) {
        return;
    }
    if (AddGlobalVarInst(inst, this)) {
        return;
    }

    Add(static_cast<uint32_t>(inst->GetOpcode()));
    Add(static_cast<uint32_t>(inst->GetType()));

    AddSpecialTraits(inst, this);

    if (AddResolver(inst, this)) {
        return;
    }

    if (AddCommutativeInst(inst, this)) {
        return;
    }

    for (auto input : inst->GetInputs()) {
        auto input_inst = inst->GetDataFlowInput(input.GetInst());
        if (!input_inst->IsSaveState()) {
            auto vn = input_inst->GetVN();
            ASSERT(vn != INVALID_VN);
            Add(vn);
        }
    }

    inst->SetVnObject(this);
}

void VnObject::Add(uint32_t obj)
{
    ASSERT(size_objs_ < MAX_ARRAY_SIZE);
    objs_[size_objs_++] = obj;
}

void VnObject::Add(uint64_t obj)
{
    ASSERT(size_objs_ < MAX_ARRAY_SIZE);
    static constexpr uint64_t MASK32 = std::numeric_limits<uint32_t>::max();
    static constexpr uint64_t SHIFT32 = 32;
    objs_[size_objs_++] = static_cast<uint32_t>(obj & MASK32);
    objs_[size_objs_++] = static_cast<uint32_t>(obj >> SHIFT32);
}

bool VnObject::Compare(VnObject *obj)
{
    uint32_t size = GetSize();
    if (size != obj->GetSize()) {
        return false;
    }
    for (uint32_t i = 0; i < size; ++i) {
        if (GetElement(i) != obj->GetElement(i)) {
            return false;
        }
    }
    return true;
}

ValNum::ValNum(Graph *graph) : Optimization(graph), map_insts_(GetGraph()->GetLocalAllocator()->Adapter()) {}

inline void ValNum::SetInstValNum(Inst *inst)
{
    COMPILER_LOG(DEBUG, VN_OPT) << " Set VN " << curr_vn_ << " for inst " << inst->GetId();
    inst->SetVN(curr_vn_++);
}

void ValNum::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool ValNum::TryToApplyCse(Inst *inst, InstVector *equiv_insts)
{
    ASSERT(!equiv_insts->empty());
    inst->SetVN((*equiv_insts)[0]->GetVN());
    COMPILER_LOG(DEBUG, VN_OPT) << " Set VN " << inst->GetVN() << " for inst " << inst->GetId();
    for (auto equiv_inst : *equiv_insts) {
        COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions are found, id " << equiv_inst->GetId();
        if (IsIrreducibleClassInst(inst, equiv_inst)) {
            continue;
        }
        if (IsInstInDifferentBlocks(inst, equiv_inst)) {
            if (!equiv_inst->IsDominate(inst) || (inst->IsResolver() && HasTryBlockBetween(equiv_inst, inst)) ||
                HasOsrEntryBetween(equiv_inst, inst)) {
                continue;
            }
        }
        // Check bridges
        if (inst->IsMovableObject()) {
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), equiv_inst, inst);
        } else {
            // check result can be moved by GC, but checks have no_cse flag
            ASSERT(!inst->IsCheck());
            COMPILER_LOG(DEBUG, VN_OPT) << " Don't need to do bridge for opcode " << *inst;
        }

        COMPILER_LOG(DEBUG, VN_OPT) << " CSE is applied for inst with id " << inst->GetId();
        GetGraph()->GetEventWriter().EventGvn(inst->GetId(), inst->GetPc(), equiv_inst->GetId(), equiv_inst->GetPc());
        inst->ReplaceUsers(equiv_inst);
        if (equiv_inst->GetOpcode() == Opcode::InitClass &&
            (inst->GetOpcode() == Opcode::LoadClass || inst->GetOpcode() == Opcode::LoadAndInitClass)) {
            equiv_inst->SetOpcode(Opcode::LoadAndInitClass);
            equiv_inst->SetType(DataType::REFERENCE);
        }

        // IsInstance, InitClass, LoadClass and LoadAndInitClass have attribute NO_DCE,
        // so they can't be removed by DCE pass. But we can remove the instructions after VN
        // because there is dominate equal instruction.
        inst->ClearFlag(compiler::inst_flags::NO_DCE);
        cse_is_applied_ = true;
        return true;
    }

    return false;
}

void ValNum::FindEqualVnOrCreateNew(Inst *inst)
{
    // create new vn for instruction with the property NO_CSE
    if (inst->IsNotCseApplicable()) {
        COMPILER_LOG(DEBUG, VN_OPT) << " The inst with id " << inst->GetId() << " has the property NO_CSE";
        SetInstValNum(inst);
        return;
    }
    auto obj = GetGraph()->GetLocalAllocator()->New<VnObject>();
    obj->Add(inst);
    COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions are searched for inst with id " << inst->GetId();
    auto it = map_insts_.find(obj);
    if (it == map_insts_.cend()) {
        COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions aren't found";
        SetInstValNum(inst);
        InstVector equiv_insts(GetGraph()->GetLocalAllocator()->Adapter());
        equiv_insts.push_back(inst);
        map_insts_.insert({obj, std::move(equiv_insts)});
        return;
    }

    auto &equiv_insts = it->second;
    if (!TryToApplyCse(inst, &equiv_insts)) {
        equiv_insts.push_back(inst);
    }
}

bool ValNum::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            inst->SetVN(INVALID_VN);
        }
    }
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            FindEqualVnOrCreateNew(inst);
        }
    }
    return cse_is_applied_;
}
}  // namespace panda::compiler
