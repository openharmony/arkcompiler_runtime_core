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

#include "inst.h"
#include "graph.h"
#include "basicblock.h"
#include "graph_visitor.h"
#include "optimizer/optimizations/vn.h"
#include "profiling/profiling.h"

namespace panda::compiler {

const ObjectTypeInfo ObjectTypeInfo::INVALID {};
const ObjectTypeInfo ObjectTypeInfo::UNKNOWN {1};

void Inst::ReserveInputs(size_t capacity)
{
    ASSERT(IsOperandsDynamic());
    GetDynamicOperands()->Reallocate(capacity);
}

Inst *User::GetInst()
{
    if (UNLIKELY(IsDynamic())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *reinterpret_cast<Inst **>(this + GetIndex() + 1);
    }
    auto p = reinterpret_cast<uintptr_t>(this);
    p += (GetIndex() + 1) * sizeof(User);

    auto inputs_count {SizeField::Decode(properties_)};
    p += (inputs_count + Input::GetPadding(RUNTIME_ARCH, inputs_count)) * sizeof(Input);
    return reinterpret_cast<Inst *>(p);
}

void Inst::InsertBefore(Inst *inst)
{
    ASSERT(bb_ != nullptr);
    bb_->InsertBefore(inst, this);
}

void Inst::InsertAfter(Inst *inst)
{
    ASSERT(bb_ != nullptr);
    bb_->InsertAfter(inst, this);
}

uint32_t Inst::GetInliningDepth() const
{
    auto ss = GetSaveState();
    return ss == nullptr ? 0 : ss->GetInliningDepth();
}

void DynamicOperands::Reallocate([[maybe_unused]] size_t new_capacity /* =0 */)
{
    if (new_capacity == 0) {
        constexpr auto IMM_2 = 2;
        new_capacity = (((capacity_ != 0U) ? capacity_ : 1U) << 1U) + IMM_2;
    } else if (new_capacity <= capacity_) {
        return;
    }
    auto size = new_capacity * (sizeof(User) + sizeof(Inst *)) + sizeof(Inst *);
    auto new_stor = reinterpret_cast<uintptr_t>(allocator_->Alloc(size));

    auto owner_inst {GetOwnerInst()};
    // Set pointer to owned instruction into new storage NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *reinterpret_cast<Inst **>(reinterpret_cast<User *>(new_stor) + new_capacity) = owner_inst;

    if (users_ == nullptr) {
        users_ = reinterpret_cast<User *>(new_stor);
        capacity_ = new_capacity;
        return;
    }
    Input *old_inputs = Inputs();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *new_inputs = reinterpret_cast<Input *>(new_stor + sizeof(User) * new_capacity) + 1;

    for (size_t i = 0; i < size_; i++) {
        Inst *old_input = old_inputs[i].GetInst();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT(old_input);
        // Initialize new User in container. Since users are placed from end of array, i.e. zero index element
        // will be at the end of array, we need to add capacity and substitute index.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        User *new_user = new (reinterpret_cast<User *>(new_stor) + new_capacity - i - 1) User(false, i, new_capacity);
        auto old_user {GetUser(i)};
        if (owner_inst->IsSaveState()) {
            new_user->SetVirtualRegister(old_user->GetVirtualRegister());
        } else if (owner_inst->IsPhi()) {
            new_user->SetBbNum(old_user->GetBbNum());
        }
        old_input->RemoveUser(old_user);
        old_input->AddUser(new_user);
        new_inputs[i] = Input(old_input);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    capacity_ = new_capacity;
    users_ = reinterpret_cast<User *>(new_stor);
}

unsigned DynamicOperands::Append(Inst *inst)
{
    ASSERT(capacity_ >= size_);
    if (capacity_ == size_) {
        Reallocate();
    }
    SetInput(size_, Input(inst));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    new (users_ + capacity_ - size_ - 1) User(false, size_, capacity_);
    auto user {GetUser(size_)};
    if (GetOwnerInst()->IsPhi()) {
        user->SetBbNum(size_);
    }
    inst->AddUser(user);
    return size_++;
}

void DynamicOperands::Remove(unsigned index)
{
    size_--;
    auto *curr_input = GetInput(index)->GetInst();
    if (curr_input->GetBasicBlock() != nullptr && curr_input->HasUsers()) {
        curr_input->RemoveUser(GetUser(index));
    }

    auto bb_num {GetUser(index)->GetBbNum()};
    auto owner_inst {GetOwnerInst()};

    if (index != size_) {
        auto *last_input = GetInput(size_)->GetInst();
        if (last_input->HasUsers()) {
            last_input->RemoveUser(GetUser(size_));
            last_input->AddUser(GetUser(index));
        }
        SetInput(index, *GetInput(size_));
        if (owner_inst->IsSaveState()) {
            GetUser(index)->SetVirtualRegister(GetUser(size_)->GetVirtualRegister());
        } else if (owner_inst->IsPhi()) {
            GetUser(index)->SetBbNum(GetUser(size_)->GetBbNum());
        }
    }

    if (owner_inst->IsPhi()) {
        for (size_t i {0}; i < size_; ++i) {
            if (GetUser(i)->GetBbNum() == size_) {
                GetUser(i)->SetBbNum(bb_num);
                break;
            }
        }
    }
}

void GetAnyTypeNameInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetAnyType()));
}

void CompareAnyTypeInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetAnyType()));
}

void BinaryImmOperation::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(GetImm());
}

void BinaryShiftedRegisterOperation::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(GetImm());
    vn_obj->Add(static_cast<uint32_t>(GetShiftType()));
}

void UnaryShiftedRegisterOperation::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(GetImm());
    vn_obj->Add(static_cast<uint32_t>(GetShiftType()));
}

void CompareInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetCc()));
}

void SelectInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetCc()));
}

void IfInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetCc()));
}

void IfImmInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetCc()));
}

void UnaryOperation::SetVnObject(VnObject *vn_obj)
{
    if (GetOpcode() == Opcode::Cast) {
        vn_obj->Add(static_cast<uint32_t>(GetInput(0).GetInst()->GetType()));
    }
}

void CmpInst::SetVnObject(VnObject *vn_obj)
{
    if (DataType::IsFloatType(GetOperandsType())) {
        vn_obj->Add(static_cast<uint32_t>(IsFcmpg()));
    }
    vn_obj->Add(static_cast<uint32_t>(GetInputType(0)));
}

void LoadFromPoolDynamic::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(GetTypeId());
}

void CastInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetInputType(0)));
}

void LoadImmediateInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(reinterpret_cast<uint64_t>(GetObject()));
}

void RuntimeClassInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(reinterpret_cast<uint64_t>(GetClass()));
}

void LoadObjFromConstInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint64_t>(GetObjPtr()));
}

void FunctionImmediateInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint64_t>(GetFunctionPtr()));
}

bool CastInst::IsDynamicCast() const
{
    return DataType::IsFloatType(GetInputType(0U)) && DataType::GetCommonType(GetType()) == DataType::INT64 &&
           GetBasicBlock()->GetGraph()->IsDynamicMethod();
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_INST(TYPE)              \
    void TYPE::Accept(GraphVisitor *v) \
    {                                  \
        v->VisitInst(this);            \
    }
// NOTE(msherstennikov): There must be another way to generate this list
OPCODE_CLASS_LIST(DEFINE_INST)
#undef DEFINE_INST

BasicBlock *PhiInst::GetPhiInputBb(unsigned index)
{
    ASSERT(index < GetInputsCount());

    auto bb_num {GetPhiInputBbNum(index)};
    ASSERT(bb_num < GetBasicBlock()->GetPredsBlocks().size());
    return GetBasicBlock()->GetPredsBlocks()[bb_num];
}

Inst *PhiInst::GetPhiInput(BasicBlock *bb)
{
    auto index = GetPredBlockIndex(bb);
    ASSERT(index < GetInputs().size());
    return GetInput(index).GetInst();
}

Inst *PhiInst::GetPhiDataflowInput(BasicBlock *bb)
{
    auto index = GetPredBlockIndex(bb);
    ASSERT(index < GetInputs().size());
    return GetDataFlowInput(index);
}

size_t PhiInst::GetPredBlockIndex(const BasicBlock *block) const
{
    for (size_t i {0}; i < GetInputsCount(); ++i) {
        if (GetPhiInputBb(i) == block) {
            return i;
        }
    }
    UNREACHABLE();
}

template <Opcode OPC, size_t INPUT_IDX>
Inst *SkipInstructions(Inst *input_inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (Opcode opcode = input_inst->GetOpcode(); opcode == OPC; opcode = input_inst->GetOpcode()) {
        input_inst = input_inst->GetInput(INPUT_IDX).GetInst();
    }
    return input_inst;
}
/*
 * For instructions LoadArray, StoreArray, LoadArrayPair, StoreArrayPair, LoadArrayI, StoreArrayI, LoadArrayPairI,
 * StoreArrayPairI, LenArray, LoadObject, StoreObject, CallVirtual, Monitor with NullCheck input the dataflow user
 * is object, which is the first input of NullCheck instruction.
 * For instructions LoadArray, StoreArray, LoadArrayPair, StoreArrayPair with BoundsCheck input the dataflow user is
 * array index, which is the second input of BoundsCheck instruction
 * For instructions Div and Mod with ZeroCheck input the dataflow user is the first input of ZeroCheck
 */
Inst *Inst::GetDataFlowInput(Inst *input_inst)
{
    auto opcode = input_inst->GetOpcode();
    if (opcode == Opcode::NullCheck) {
        return SkipInstructions<Opcode::NullCheck, 0>(input_inst);
    }
    if (opcode == Opcode::BoundsCheck) {
        return SkipInstructions<Opcode::BoundsCheck, 1>(input_inst);
    }
    if (opcode == Opcode::BoundsCheckI) {
        return SkipInstructions<Opcode::BoundsCheckI, 0>(input_inst);
    }
    if (opcode == Opcode::ZeroCheck) {
        return SkipInstructions<Opcode::ZeroCheck, 0>(input_inst);
    }
    if (opcode == Opcode::NegativeCheck) {
        return SkipInstructions<Opcode::NegativeCheck, 0>(input_inst);
    }
    if (opcode == Opcode::NotPositiveCheck) {
        return SkipInstructions<Opcode::NotPositiveCheck, 0>(input_inst);
    }
    if (opcode == Opcode::AnyTypeCheck) {
        return SkipInstructions<Opcode::AnyTypeCheck, 0>(input_inst);
    }
    if (opcode == Opcode::ObjByIndexCheck) {
        return SkipInstructions<Opcode::ObjByIndexCheck, 0>(input_inst);
    }
    if (opcode == Opcode::HclassCheck) {
        input_inst = SkipInstructions<Opcode::HclassCheck, 0>(input_inst);
        return SkipInstructions<Opcode::LoadObject, 0>(input_inst);
    }
    if (opcode == Opcode::RefTypeCheck) {
        input_inst = SkipInstructions<Opcode::RefTypeCheck, 1>(input_inst);
        if (input_inst->GetOpcode() == Opcode::NullCheck) {
            return SkipInstructions<Opcode::NullCheck, 0>(input_inst);
        }
        return input_inst;
    }
    return input_inst;
}

bool Inst::IsPrecedingInSameBlock(const Inst *other) const
{
    ASSERT(other != nullptr && GetBasicBlock() == other->GetBasicBlock());
    if (this == other) {
        return true;
    }
    auto next = GetNext();
    while (next != nullptr) {
        if (next == other) {
            return true;
        }
        next = next->GetNext();
    }
    return false;
}

bool Inst::IsDominate(const Inst *other) const
{
    ASSERT(other != nullptr);
    if (this == other) {
        return true;
    }
    auto this_bb = GetBasicBlock();
    auto other_bb = other->GetBasicBlock();
    return this_bb == other_bb ? IsPrecedingInSameBlock(other) : this_bb->IsDominate(other_bb);
}

bool Inst::InSameBlockOrDominate(const Inst *other) const
{
    return GetBasicBlock() == other->GetBasicBlock() || IsDominate(other);
}

Inst *Inst::Clone(const Graph *target_graph) const
{
    ASSERT(target_graph != nullptr);
    auto clone = target_graph->CreateInst(GetOpcode());
    clone->bit_fields_ = GetAllFields();
    clone->pc_ = GetPc();
#ifndef NDEBUG
    clone->SetDstReg(GetDstReg());
#endif
    if (IsOperandsDynamic()) {
        clone->ReserveInputs(GetInputsCount());
    }
#ifdef PANDA_COMPILER_DEBUG_INFO
    clone->SetCurrentMethod(GetCurrentMethod());
#endif
    return clone;
}

template <size_t N>
Inst *FixedInputsInst<N>::Clone(const Graph *target_graph) const
{
    auto clone = static_cast<FixedInputsInst *>(Inst::Clone(target_graph));
#ifndef NDEBUG
    for (size_t i = 0; i < INPUT_COUNT; ++i) {
        clone->SetSrcReg(i, GetSrcReg(i));
    }
#endif
    return clone;
}

#if PANDA_TARGET_MACOS
template class FixedInputsInst<0>;
template class FixedInputsInst<1>;
template class FixedInputsInst<2U>;
template class FixedInputsInst<3U>;
template class FixedInputsInst<4U>;
#endif

Inst *CallInst::Clone(const Graph *target_graph) const
{
    ASSERT(target_graph != nullptr);
    auto inst_clone = Inst::Clone(target_graph);
    auto call_clone = static_cast<CallInst *>(inst_clone);
    call_clone->SetCallMethodId(GetCallMethodId());
    call_clone->SetCallMethod(GetCallMethod());
    call_clone->SetCanNativeException(GetCanNativeException());
    CloneTypes(target_graph->GetAllocator(), call_clone);
    return inst_clone;
}

Inst *CallIndirectInst::Clone(const Graph *target_graph) const
{
    auto clone = Inst::Clone(target_graph)->CastToCallIndirect();
    CloneTypes(target_graph->GetAllocator(), clone);
    return clone;
}

Inst *IntrinsicInst::Clone(const Graph *target_graph) const
{
    ASSERT(target_graph != nullptr);
    auto intrinsic_clone = (GetOpcode() == Opcode::Intrinsic ? Inst::Clone(target_graph)->CastToIntrinsic()
                                                             : Inst::Clone(target_graph)->CastToBuiltin());
    intrinsic_clone->SetIntrinsicId(GetIntrinsicId());
    CloneTypes(target_graph->GetAllocator(), intrinsic_clone);
    if (HasImms()) {
        for (auto imm : GetImms()) {
            intrinsic_clone->AddImm(target_graph->GetAllocator(), imm);
        }
    }
    intrinsic_clone->SetMethod(GetMethod());
    return intrinsic_clone;
}

Inst *ConstantInst::Clone(const Graph *target_graph) const
{
    Inst *new_cnst = nullptr;
    bool is_support_int32 = GetBasicBlock()->GetGraph()->IsBytecodeOptimizer();
    switch (GetType()) {
        case DataType::INT32:
            new_cnst = target_graph->CreateInstConstant(static_cast<int32_t>(GetIntValue()), is_support_int32);
            break;
        case DataType::INT64:
            new_cnst = target_graph->CreateInstConstant(GetIntValue(), is_support_int32);
            break;
        case DataType::FLOAT32:
            new_cnst = target_graph->CreateInstConstant(GetFloatValue(), is_support_int32);
            break;
        case DataType::FLOAT64:
            new_cnst = target_graph->CreateInstConstant(GetDoubleValue(), is_support_int32);
            break;
        case DataType::ANY:
            new_cnst = target_graph->CreateInstConstant(GetRawValue(), is_support_int32);
            new_cnst->SetType(DataType::ANY);
            break;
        default:
            UNREACHABLE();
    }
#ifndef NDEBUG
    new_cnst->SetDstReg(GetDstReg());
#endif
    return new_cnst;
}

Inst *ParameterInst::Clone(const Graph *target_graph) const
{
    auto clone = Inst::Clone(target_graph)->CastToParameter();
    clone->SetArgNumber(GetArgNumber());
    clone->SetLocationData(GetLocationData());
    return clone;
}

Inst *SaveStateInst::Clone(const Graph *target_graph) const
{
    auto clone = static_cast<SaveStateInst *>(Inst::Clone(target_graph));
    if (GetImmediatesCount() > 0) {
        clone->AllocateImmediates(target_graph->GetAllocator(), GetImmediatesCount());
        std::copy(immediates_->begin(), immediates_->end(), clone->immediates_->begin());
    }
    clone->method_ = method_;
    clone->caller_inst_ = caller_inst_;
    clone->inlining_depth_ = inlining_depth_;
    return clone;
}

Inst *BinaryShiftedRegisterOperation::Clone(const Graph *target_graph) const
{
    auto clone = static_cast<BinaryShiftedRegisterOperation *>(FixedInputsInst::Clone(target_graph));
    clone->SetImm(GetImm());
    clone->SetShiftType(GetShiftType());
    return clone;
}

Inst *UnaryShiftedRegisterOperation::Clone(const Graph *target_graph) const
{
    auto clone = static_cast<UnaryShiftedRegisterOperation *>(FixedInputsInst::Clone(target_graph));
    clone->SetImm(GetImm());
    clone->SetShiftType(GetShiftType());
    return clone;
}

void SaveStateInst::AppendImmediate(uint64_t imm, uint16_t vreg, DataType::Type type, VRegType vreg_type)
{
    if (immediates_ == nullptr) {
        ASSERT(GetBasicBlock() != nullptr);
        AllocateImmediates(GetBasicBlock()->GetGraph()->GetAllocator(), 0);
    }
    immediates_->emplace_back(SaveStateImm {imm, vreg, type, vreg_type});
}

void SaveStateInst::AllocateImmediates(ArenaAllocator *allocator, size_t size)
{
    immediates_ = allocator->New<ArenaVector<SaveStateImm>>(allocator->Adapter());
    immediates_->resize(size);
}

void TryInst::AppendCatchTypeId(uint32_t id, uint32_t catch_edge_index)
{
    if (catch_type_ids_ == nullptr) {
        ASSERT(catch_edge_indexes_ == nullptr);
        ASSERT(GetBasicBlock() != nullptr);
        auto allocator = GetBasicBlock()->GetGraph()->GetAllocator();
        catch_type_ids_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
        catch_edge_indexes_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
    }
    catch_type_ids_->push_back(id);
    catch_edge_indexes_->push_back(catch_edge_index);
}

void CatchPhiInst::AppendThrowableInst(const Inst *inst)
{
    if (throw_insts_ == nullptr) {
        ASSERT(GetBasicBlock() != nullptr);
        auto allocator = GetBasicBlock()->GetGraph()->GetAllocator();
        throw_insts_ = allocator->New<ArenaVector<const Inst *>>(allocator->Adapter());
    }
    throw_insts_->push_back(inst);
}

void CatchPhiInst::ReplaceThrowableInst(const Inst *old_inst, const Inst *new_inst)
{
    auto index = GetThrowableInstIndex(old_inst);
    throw_insts_->at(index) = new_inst;
}

void CatchPhiInst::RemoveInput(unsigned index)
{
    Inst::RemoveInput(index);
    if (throw_insts_ != nullptr) {
        throw_insts_->at(index) = throw_insts_->back();
        throw_insts_->pop_back();
    }
}

Inst *TryInst::Clone(const Graph *target_graph) const
{
    auto clone = FixedInputsInst::Clone(target_graph)->CastToTry();
    if (auto ids_count = this->GetCatchTypeIdsCount(); ids_count > 0) {
        if (clone->catch_type_ids_ == nullptr) {
            auto allocator = target_graph->GetAllocator();
            clone->catch_type_ids_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
            clone->catch_edge_indexes_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
        }
        clone->catch_type_ids_->resize(ids_count);
        clone->catch_edge_indexes_->resize(ids_count);
        std::copy(this->catch_type_ids_->begin(), this->catch_type_ids_->end(), clone->catch_type_ids_->begin());
        std::copy(this->catch_edge_indexes_->begin(), this->catch_edge_indexes_->end(),
                  clone->catch_edge_indexes_->begin());
    }
    return clone;
}

BasicBlock *IfImmInst::GetEdgeIfInputTrue()
{
    return GetBasicBlock()->GetSuccessor(GetTrueInputEdgeIdx());
}

BasicBlock *IfImmInst::GetEdgeIfInputFalse()
{
    return GetBasicBlock()->GetSuccessor(1 - GetTrueInputEdgeIdx());
}

/**
 * NB! Can be called before Lowering pass only
 * Return if_imm's block successor index when input is true
 */
size_t IfImmInst::GetTrueInputEdgeIdx()
{
    ASSERT(GetBasicBlock() != nullptr);
    ASSERT(GetBasicBlock()->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    ASSERT(GetCc() == ConditionCode::CC_NE || GetCc() == ConditionCode::CC_EQ);
    ASSERT(GetImm() == 0);
    return GetCc() == CC_NE ? 0 : 1;
}

bool Inst::IsPropagateLiveness() const
{
    return (CanThrow() && GetBasicBlock()->IsTry()) || CanDeoptimize();
}

bool Inst::RequireRegMap() const
{
    if (GetOpcode() == Opcode::SafePoint) {
        return OPTIONS.IsCompilerSafePointsRequireRegMap();
    }
    return GetOpcode() == Opcode::SaveStateOsr || IsPropagateLiveness();
}

bool Inst::IsZeroRegInst() const
{
    ASSERT(GetBasicBlock() != nullptr);
    ASSERT(GetBasicBlock()->GetGraph() != nullptr);
    return GetBasicBlock()->GetGraph()->GetZeroReg() != INVALID_REG && IsZeroConstantOrNullPtr(this);
}

bool Inst::IsAccRead() const
{
    return GetFlag(inst_flags::ACC_READ);
}

bool Inst::IsAccWrite() const
{
    if (GetBasicBlock()->GetGraph()->IsDynamicMethod() && IsConst()) {
        return true;
    }
    return GetFlag(inst_flags::ACC_WRITE);
}

// Returns true if instruction result can be object
bool Inst::IsReferenceOrAny() const
{
    if (GetType() == DataType::ANY) {
        switch (opcode_) {
            // GetAnyTypeName always return movable string
            case Opcode::GetAnyTypeName:
            // We conservative decide that phi with ANY type is always reference,
            // because for phi we can speculate incorrect any_type
            case Opcode::Phi:
                return true;
            default:
                break;
        }
        auto any_type = GetAnyType();
        if (any_type == AnyBaseType::UNDEFINED_TYPE) {
            return true;
        }
        auto data_type = AnyBaseTypeToDataType(any_type);
        return data_type == DataType::REFERENCE;
    }
    return GetType() == DataType::REFERENCE;
}

// Returns true if instruction result can be moved by GC
// Returns false for checks because their result is equal to input
bool Inst::IsMovableObject() const
{
    if (IsCheck() || !IsReferenceOrAny()) {
        return false;
    }
    switch (opcode_) {
        case Opcode::NullPtr:
        case Opcode::LoadClass:
        case Opcode::InitClass:
        case Opcode::LoadAndInitClass:
        case Opcode::UnresolvedLoadAndInitClass:
        case Opcode::LoadImmediate:
        case Opcode::GetInstanceClass:
        case Opcode::GetGlobalVarAddress:
        case Opcode::ResolveObjectFieldStatic:
        case Opcode::Constant:
        case Opcode::LoadConstantPool:
        case Opcode::LoadRuntimeClass:
        case Opcode::LoadUndefined:
            // The result of these instructions can't be moved by GC.
            return false;
        case Opcode::LoadObject:
            // Classes in non moveble space.
            return this->CastToLoadObject()->GetObjectType() != ObjectType::MEM_DYN_CLASS &&
                   this->CastToLoadObject()->GetObjectType() != ObjectType::MEM_DYN_HCLASS;

        default:
            return true;
    }
}

TryInst *GetTryBeginInst(const BasicBlock *try_begin_bb)
{
    ASSERT(try_begin_bb != nullptr && try_begin_bb->IsTryBegin());
    for (auto inst : try_begin_bb->AllInsts()) {
        if (inst->GetOpcode() == Opcode::Try) {
            return inst->CastToTry();
        }
    }
    UNREACHABLE();
    return nullptr;
}

/**
 * Regalloc's helper to checks if intrinsic's arguments should be located on the registers according to
 * calling-convention
 */
bool IntrinsicInst::IsNativeCall() const
{
    ASSERT(GetBasicBlock() != nullptr);
    ASSERT(GetBasicBlock()->GetGraph() != nullptr);
    if (IsFastpathIntrinsic(intrinsic_id_)) {
        return false;
    }
#ifdef PANDA_WITH_IRTOC
    if (IsIrtocIntrinsic(intrinsic_id_)) {
        return intrinsic_id_ == RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY;
    }
#endif
    auto graph = GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();
    auto runtime = graph->GetRuntime();
    return !EncodesBuiltin(runtime, intrinsic_id_, arch) || IsRuntimeCall();
}

DeoptimizeType AnyTypeCheckInst::GetDeoptimizeType() const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto custom_deoptimize = graph->IsAotMode() || graph->GetRuntime()->GetMethodProfile(graph->GetMethod(), true) !=
                                                       profiling::INVALID_PROFILE;
    if (!custom_deoptimize) {
        return DeoptimizeType::ANY_TYPE_CHECK;
    }
    switch (AnyBaseTypeToDataType(GetAnyType())) {
        case DataType::Type::INT32:
            return DeoptimizeType::NOT_SMALL_INT;
        case DataType::Type::FLOAT64:
            if (IsIntegerWasSeen()) {
                return DeoptimizeType::NOT_NUMBER;
            }
            return DeoptimizeType::DOUBLE_WITH_INT;
        default:
            return DeoptimizeType::ANY_TYPE_CHECK;
    }
}

void HclassCheckInst::ExtendFlags(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::HclassCheck);
    auto check = inst->CastToHclassCheck();
    if (check->GetCheckFunctionIsNotClassConstructor()) {
        SetCheckFunctionIsNotClassConstructor(true);
    }
    if (check->GetCheckIsFunction()) {
        SetCheckIsFunction(true);
    }
}

}  // namespace panda::compiler
