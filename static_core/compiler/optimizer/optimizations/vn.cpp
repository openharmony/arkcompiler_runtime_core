/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::compiler {
class BasicBlock;

inline bool IsNotCommutativeInst(Inst *inst)
{
    return !inst->IsCommutative() || DataType::IsFloatType(inst->GetType());
}

inline bool IsAddressArithmeticInst(Inst *inst)
{
    return inst->GetType() == DataType::POINTER &&
           (inst->GetOpcode() == Opcode::AddI || inst->GetOpcode() == Opcode::SubI);
}

static bool AddClassInst(Inst *inst, VnObject *obj)
{
    if (inst->GetOpcode() != Opcode::LoadAndInitClass && inst->GetOpcode() != Opcode::InitClass &&
        inst->GetOpcode() != Opcode::LoadClass) {
        return false;
    }

    obj->Add(static_cast<VnObject::ObjType>(Opcode::InitClass));
    auto classInst = static_cast<ClassInst *>(inst);
    auto klass = classInst->GetClass();
    if (klass == nullptr) {
        obj->Add(classInst->GetTypeId());
    } else {
        obj->Add(reinterpret_cast<VnObject::DoubleObjType>(klass));
    }
    inst->SetVnObject(obj);
    return true;
}

static bool AddGlobalVarInst(Inst *inst, VnObject *obj)
{
    if (inst->GetOpcode() != Opcode::GetGlobalVarAddress) {
        return false;
    }

    obj->Add(static_cast<VnObject::ObjType>(Opcode::GetGlobalVarAddress));
    obj->Add(inst->CastToGetGlobalVarAddress()->GetTypeId());
    inst->SetVnObject(obj);
    return true;
}

static bool AddSelectImmInst(Inst *inst, VnObject *obj)
{
    if (inst->GetOpcode() != Opcode::SelectImm) {
        return false;
    }

    obj->Add(static_cast<VnObject::ObjType>(Opcode::SelectImm));
    obj->Add(static_cast<VnObject::HalfObjType>(inst->GetType()),
             static_cast<VnObject::HalfObjType>(inst->CastToSelectImm()->GetCc()));

    for (auto input : inst->GetInputs()) {
        auto inputInst = inst->GetDataFlowInput(input.GetInst());
        auto vn = inputInst->GetVN();
        ASSERT(vn != INVALID_VN);
        obj->Add(vn);
    }
    obj->Add(inst->CastToSelectImm()->GetImm());
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
            obj->Add(static_cast<VnObject::ObjType>(inst->CastToIntrinsic()->GetIntrinsicId()));
            break;
        case Opcode::CompareAnyType:
            obj->Add(static_cast<VnObject::ObjType>(inst->CastToCompareAnyType()->GetAnyType()));
            break;
        case Opcode::CastAnyTypeValue:
            obj->Add(static_cast<VnObject::ObjType>(inst->CastToCastAnyTypeValue()->GetAnyType()));
            break;
        case Opcode::CastValueToAnyType:
            obj->Add(static_cast<VnObject::ObjType>(inst->CastToCastValueToAnyType()->GetAnyType()));
            break;
        case Opcode::LoadStatic:
            ASSERT(inst->CastToLoadStatic()->GetObjField() != nullptr);
            obj->Add(reinterpret_cast<VnObject::DoubleObjType>(inst->CastToLoadStatic()->GetObjField()));
            break;
        case Opcode::LoadObject:
            ASSERT(inst->CastToLoadObject()->GetObjField() != nullptr);
            obj->Add(reinterpret_cast<VnObject::DoubleObjType>(inst->CastToLoadObject()->GetObjField()));
            break;
        default:
            break;
    }
}

static bool IsIrreducibleClassInst(Inst *inst, Inst *equivInst)
{
    return equivInst->GetOpcode() == Opcode::LoadClass &&
           (inst->GetOpcode() == Opcode::InitClass || inst->GetOpcode() == Opcode::LoadAndInitClass);
}

static bool AddResolver(Inst *inst, VnObject *obj)
{
    if (!inst->IsResolver()) {
        return false;
    }

    for (auto input : inst->GetInputs()) {
        auto inputInst = inst->GetDataFlowInput(input.GetInst());
        if (!inputInst->IsSaveState()) {
            auto vn = inputInst->GetVN();
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
        obj->Add(reinterpret_cast<VnObject::DoubleObjType>(inst->CastToResolveObjectField()->GetMethod()));
    } else if (inst->GetOpcode() == Opcode::ResolveObjectFieldStatic) {
        obj->Add(inst->CastToResolveObjectFieldStatic()->GetTypeId());
        obj->Add(reinterpret_cast<VnObject::DoubleObjType>(inst->CastToResolveObjectFieldStatic()->GetMethod()));
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
    if (AddSelectImmInst(inst, this)) {
        return;
    }

    Add(static_cast<ObjType>(inst->GetOpcode()));
    Add(static_cast<ObjType>(inst->GetType()));

    AddSpecialTraits(inst, this);

    if (AddResolver(inst, this)) {
        return;
    }

    if (AddCommutativeInst(inst, this)) {
        return;
    }

    for (auto input : inst->GetInputs()) {
        auto inputInst = inst->GetDataFlowInput(input.GetInst());
        if (!inputInst->IsSaveState()) {
            auto vn = inputInst->GetVN();
            ASSERT(vn != INVALID_VN);
            Add(vn);
        }
    }

    inst->SetVnObject(this);
}

void VnObject::Add(HalfObjType obj1, HalfObjType obj2)
{
    ASSERT(sizeObjs_ < MAX_ARRAY_SIZE);
    static constexpr HalfObjType SHIFT16 = 16;
    ObjType obj = static_cast<ObjType>(obj1) << SHIFT16;
    obj |= static_cast<ObjType>(obj2);
    objs_[sizeObjs_++] = obj;
}

void VnObject::Add(ObjType obj)
{
    ASSERT(sizeObjs_ < MAX_ARRAY_SIZE);
    objs_[sizeObjs_++] = obj;
}

void VnObject::Add(DoubleObjType obj)
{
    ASSERT(sizeObjs_ < MAX_ARRAY_SIZE);
    static constexpr DoubleObjType MASK32 = std::numeric_limits<uint32_t>::max();
    static constexpr DoubleObjType SHIFT32 = 32;
    objs_[sizeObjs_++] = static_cast<ObjType>(obj & MASK32);
    objs_[sizeObjs_++] = static_cast<ObjType>(obj >> SHIFT32);
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

ValNum::ValNum(Graph *graph) : Optimization(graph), mapInsts_(GetGraph()->GetLocalAllocator()->Adapter()) {}

inline void ValNum::SetInstValNum(Inst *inst)
{
    COMPILER_LOG(DEBUG, VN_OPT) << " Set VN " << currVn_ << " for inst " << inst->GetId();
    inst->SetVN(currVn_++);
}

void ValNum::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool ValNum::TryToApplyCse(Inst *inst, InstVector *equivInsts)
{
    ASSERT(!equivInsts->empty());
    inst->SetVN((*equivInsts)[0]->GetVN());
    COMPILER_LOG(DEBUG, VN_OPT) << " Set VN " << inst->GetVN() << " for inst " << inst->GetId();
    for (auto equivInst : *equivInsts) {
        COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions are found, id " << equivInst->GetId();
        if (IsIrreducibleClassInst(inst, equivInst)) {
            continue;
        }
        if (IsInstInDifferentBlocks(inst, equivInst)) {
            if (!equivInst->IsDominate(inst) || (inst->IsResolver() && HasTryBlockBetween(equivInst, inst)) ||
                HasOsrEntryBetween(equivInst, inst)) {
                continue;
            }
        }
        if (IsAddressArithmeticInst(inst) && HasSaveStateBetween<IsSaveStateCanTriggerGc>(equivInst, inst)) {
            continue;
        }
        // Check bridges
        if (inst->IsMovableObject()) {
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), equivInst, inst);
        } else {
            // check result can be moved by GC, but checks have no_cse flag
            ASSERT(!inst->IsCheck());
            COMPILER_LOG(DEBUG, VN_OPT) << " Don't need to do bridge for opcode " << *inst;
        }

        COMPILER_LOG(DEBUG, VN_OPT) << " CSE is applied for inst with id " << inst->GetId();
        GetGraph()->GetEventWriter().EventGvn(inst->GetId(), inst->GetPc(), equivInst->GetId(), equivInst->GetPc());
        inst->ReplaceUsers(equivInst);
        if (equivInst->GetOpcode() == Opcode::InitClass &&
            (inst->GetOpcode() == Opcode::LoadClass || inst->GetOpcode() == Opcode::LoadAndInitClass)) {
            equivInst->SetOpcode(Opcode::LoadAndInitClass);
            equivInst->SetType(DataType::REFERENCE);
        }

        // IsInstance, InitClass, LoadClass and LoadAndInitClass have attribute NO_DCE,
        // so they can't be removed by DCE pass. But we can remove the instructions after VN
        // because there is dominate equal instruction.
        inst->ClearFlag(compiler::inst_flags::NO_DCE);
        cseIsApplied_ = true;
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
    auto it = mapInsts_.find(obj);
    if (it == mapInsts_.cend()) {
        COMPILER_LOG(DEBUG, VN_OPT) << " Equivalent instructions aren't found";
        SetInstValNum(inst);
        InstVector equivInsts(GetGraph()->GetLocalAllocator()->Adapter());
        equivInsts.push_back(inst);
        mapInsts_.insert({obj, std::move(equivInsts)});
        return;
    }

    auto &equivInsts = it->second;
    if (!TryToApplyCse(inst, &equivInsts)) {
        equivInsts.push_back(inst);
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
    return cseIsApplied_;
}
}  // namespace ark::compiler
