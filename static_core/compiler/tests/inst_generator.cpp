/**
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

#include "inst_generator.h"

namespace panda::compiler {
Graph *GraphCreator::GenerateGraph(Inst *inst)
{
    Graph *graph;
    SetNumVRegsArgs(0U, 0U);
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::LoadArrayI:
        case Opcode::StoreArray:
        case Opcode::StoreArrayI:
        case Opcode::StoreObject:
        case Opcode::SelectImm:
        case Opcode::Select:
        case Opcode::ReturnInlined:
        case Opcode::LoadArrayPair:
        case Opcode::LoadArrayPairI:
        case Opcode::StoreArrayPair:
        case Opcode::StoreArrayPairI:
        case Opcode::NewArray:
        case Opcode::NewObject:
            // -1 means special processing
            graph = GenerateOperation(inst, -1L);
            break;
        case Opcode::ReturnVoid:
        case Opcode::NullPtr:
        case Opcode::Constant:
        case Opcode::Parameter:
        case Opcode::SpillFill:
        case Opcode::ReturnI:
            graph = GenerateOperation(inst, 0U);
            break;
        case Opcode::Neg:
        case Opcode::Abs:
        case Opcode::Not:
        case Opcode::LoadString:
        case Opcode::LoadType:
        case Opcode::LenArray:
        case Opcode::Return:
        case Opcode::IfImm:
        case Opcode::SaveState:
        case Opcode::SafePoint:
        case Opcode::Cast:
        case Opcode::CallStatic:
        case Opcode::CallVirtual:
        case Opcode::AddI:
        case Opcode::SubI:
        case Opcode::ShlI:
        case Opcode::ShrI:
        case Opcode::AShrI:
        case Opcode::AndI:
        case Opcode::OrI:
        case Opcode::XorI:
        case Opcode::LoadObject:
        case Opcode::LoadStatic:
        case Opcode::Monitor:
        case Opcode::NegSR:
            graph = GenerateOperation(inst, 1U);
            break;
        case Opcode::Add:
        case Opcode::Sub:
        case Opcode::Mul:
        case Opcode::Div:
        case Opcode::Mod:
        case Opcode::Min:
        case Opcode::Max:
        case Opcode::Shl:
        case Opcode::Shr:
        case Opcode::AShr:
        case Opcode::And:
        case Opcode::Or:
        case Opcode::Xor:
        case Opcode::Compare:
        case Opcode::Cmp:
        case Opcode::If:
        case Opcode::StoreStatic:
        case Opcode::AndNot:
        case Opcode::OrNot:
        case Opcode::XorNot:
        case Opcode::MNeg:
        case Opcode::AddSR:
        case Opcode::SubSR:
        case Opcode::AndSR:
        case Opcode::OrSR:
        case Opcode::XorSR:
        case Opcode::AndNotSR:
        case Opcode::OrNotSR:
        case Opcode::XorNotSR:
        case Opcode::IsInstance:
            graph = GenerateOperation(inst, 2U);
            break;
        case Opcode::MAdd:
        case Opcode::MSub:
            graph = GenerateOperation(inst, 3U);
            break;
        case Opcode::BoundsCheck:
        case Opcode::BoundsCheckI:
            graph = GenerateBoundaryCheckOperation(inst);
            break;
        case Opcode::NullCheck:
        case Opcode::CheckCast:
        case Opcode::ZeroCheck:
        case Opcode::NegativeCheck:
            graph = GenerateCheckOperation(inst);
            break;
        case Opcode::Phi:
            graph = GeneratePhiOperation(inst);
            break;
        case Opcode::Throw:
            graph = GenerateThrowOperation(inst);
            break;
        case Opcode::MultiArray:
            graph = GenerateMultiArrayOperation(inst);
            break;
        case Opcode::Intrinsic:
            graph = GenerateIntrinsicOperation(inst);
            break;
        default:
            ASSERT_DO(0U, inst->Dump(&std::cerr));
            graph = nullptr;
            break;
    }
    if (graph != nullptr) {
        auto id = graph->GetCurrentInstructionId();
        inst->SetId(id);
        graph->SetCurrentInstructionId(++id);
        graph->SetMethod(reinterpret_cast<RuntimeInterface::MethodPtr>(runtime_.METHOD));
        graph->ResetParameterInfo();
        for (auto param : graph->GetStartBlock()->Insts()) {
            if (param->GetOpcode() == Opcode::Parameter) {
                param->CastToParameter()->SetLocationData(graph->GetDataForNativeParam(param->GetType()));
                runtime_.arg_types_->push_back(param->GetType());
            }
        }
    }
    if (inst->GetType() == DataType::NO_TYPE || inst->IsStore()) {
        runtime_.return_type_ = DataType::VOID;
    } else {
        runtime_.return_type_ = inst->GetType();
    }
#ifndef NDEBUG
    if (graph != nullptr) {
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        graph->SetLowLevelInstructionsEnabled();
    }
#endif
    return graph;
}

Graph *GraphCreator::CreateGraph()
{
    Graph *graph = allocator_.New<Graph>(&allocator_, &local_allocator_, arch_);
    runtime_.arg_types_ = allocator_.New<ArenaVector<DataType::Type>>(allocator_.Adapter());
    graph->SetRuntime(&runtime_);
    graph->SetStackSlotsCount(3U);
    return graph;
}

// NOLINTNEXTLINE(readability-function-size)
Graph *GraphCreator::GenerateOperation(Inst *inst, int32_t n)
{
    Graph *graph;
    auto opc = inst->GetOpcode();
    if (opc == Opcode::If || opc == Opcode::IfImm) {
        graph = CreateGraphWithThreeBasicBlock();
    } else {
        graph = CreateGraphWithOneBasicBlock();
    }
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    DataType::Type type;
    switch (opc) {
        case Opcode::IsInstance:
        case Opcode::LenArray:
        case Opcode::SaveState:
        case Opcode::SafePoint:
        case Opcode::CallStatic:
        case Opcode::CallVirtual:
        case Opcode::NewArray:
        case Opcode::LoadObject:
        case Opcode::Monitor:
            type = DataType::REFERENCE;
            break;
        case Opcode::IfImm:
            type = inst->CastToIfImm()->GetOperandsType();
            break;
        case Opcode::If:
            type = inst->CastToIf()->GetOperandsType();
            break;
        case Opcode::Compare:
            type = inst->CastToCompare()->GetOperandsType();
            break;
        case Opcode::Cmp:
            type = inst->CastToCmp()->GetOperandsType();
            break;
        default:
            type = inst->GetType();
    }
    if (opc == Opcode::LoadArrayPair || opc == Opcode::LoadArrayPairI) {
        auto array = CreateParamInst(graph, DataType::REFERENCE, 0U);
        Inst *index = nullptr;
        if (opc == Opcode::LoadArrayPair) {
            index = CreateParamInst(graph, DataType::INT32, 1U);
        }
        inst->SetInput(0U, array);
        if (opc == Opcode::LoadArrayPair) {
            inst->SetInput(1U, index);
        }
        block->AppendInst(inst);
        auto load_pair_part0 = graph->CreateInstLoadPairPart(inst->GetType(), INVALID_PC, inst, 0U);
        auto load_pair_part1 = graph->CreateInstLoadPairPart(inst->GetType(), INVALID_PC, inst, 1U);
        block->AppendInst(load_pair_part0);
        inst = load_pair_part1;
    } else if (opc == Opcode::StoreArrayPairI || opc == Opcode::StoreArrayPair) {
        int stack_slot = 0;
        auto array = CreateParamInst(graph, DataType::REFERENCE, stack_slot++);
        Inst *index = nullptr;
        if (opc == Opcode::StoreArrayPair) {
            index = CreateParamInst(graph, DataType::INT32, stack_slot++);
        }
        auto val1 = CreateParamInst(graph, inst->GetType(), stack_slot++);
        auto val2 = CreateParamInst(graph, inst->GetType(), stack_slot++);
        int idx = 0;
        inst->SetInput(idx++, array);
        if (opc == Opcode::StoreArrayPair) {
            inst->SetInput(idx++, index);
        }
        inst->SetInput(idx++, val1);
        inst->SetInput(idx++, val2);
    } else if (opc == Opcode::ReturnInlined) {
        ASSERT(n == -1L);
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        block->AppendInst(save_state);

        auto call_inst = static_cast<CallInst *>(graph->CreateInstCallStatic());
        call_inst->SetType(DataType::VOID);
        call_inst->SetInlined(true);
        call_inst->AllocateInputTypes(&allocator_, 0U);
        call_inst->AppendInput(save_state);
        call_inst->AddInputType(DataType::NO_TYPE);
        block->AppendInst(call_inst);

        inst->SetInput(0U, save_state);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else if (opc == Opcode::CallStatic || opc == Opcode::CallVirtual) {
        ASSERT(n >= 0);
        auto call_inst = static_cast<CallInst *>(inst);
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        block->PrependInst(save_state);
        call_inst->AllocateInputTypes(&allocator_, n);
        for (int32_t i = 0; i < n; ++i) {
            auto param = CreateParamInst(graph, type, i);
            call_inst->AppendInput(param);
            call_inst->AddInputType(type);
            save_state->AppendInput(param);
        }
        for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
            save_state->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
        }
        call_inst->AppendInput(save_state);
        call_inst->AddInputType(DataType::NO_TYPE);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else if (opc == Opcode::LoadArray || opc == Opcode::StoreArray) {
        ASSERT(n == -1L);
        auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0U);  // array
        auto param2 = CreateParamInst(graph, DataType::INT32, 1U);      // index
        inst->SetInput(0U, param1);
        inst->SetInput(1U, param2);
        if (inst->GetOpcode() == Opcode::StoreArray) {
            auto param3 = CreateParamInst(graph, type, 2U);
            inst->SetInput(2U, param3);
        }
    } else if (opc == Opcode::LoadArrayI || opc == Opcode::StoreArrayI || opc == Opcode::StoreObject) {
        ASSERT(n == -1L);
        auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0U);  // array/object
        inst->SetInput(0U, param1);
        if (inst->GetOpcode() != Opcode::LoadArrayI) {
            auto param2 = CreateParamInst(graph, type, 1U);
            inst->SetInput(1U, param2);
        }
    } else if (opc == Opcode::Select) {
        ASSERT(n == -1L);
        auto cmp_type = inst->CastToSelect()->GetOperandsType();
        auto param0 = CreateParamInst(graph, type, 0U);
        auto param1 = CreateParamInst(graph, type, 1U);
        auto param2 = CreateParamInst(graph, cmp_type, 2U);
        auto param3 = CreateParamInst(graph, cmp_type, 3U);
        inst->SetInput(0U, param0);
        inst->SetInput(1U, param1);
        inst->SetInput(2U, param2);
        inst->SetInput(3U, param3);
    } else if (opc == Opcode::SelectImm) {
        ASSERT(n == -1L);
        auto cmp_type = inst->CastToSelectImm()->GetOperandsType();
        auto param0 = CreateParamInst(graph, type, 0U);
        auto param1 = CreateParamInst(graph, type, 1U);
        auto param2 = CreateParamInst(graph, cmp_type, 2U);
        inst->SetInput(0U, param0);
        inst->SetInput(1U, param1);
        inst->SetInput(2U, param2);
    } else if (opc == Opcode::StoreStatic) {
        auto param0 = CreateParamInst(graph, type, 0U);
        inst->SetInput(1U, param0);
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        save_state->AppendInput(param0);
        save_state->SetVirtualRegister(0U, VirtualRegister(0U, VRegType::VREG));
        auto init_inst = graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC, save_state,
                                                           inst->CastToStoreStatic()->GetTypeId(), nullptr, nullptr);
        inst->SetInput(0U, init_inst);
        block->PrependInst(init_inst);
        block->PrependInst(save_state);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else if (opc == Opcode::LoadStatic) {
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        auto init_inst = graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC, save_state,
                                                           inst->CastToLoadStatic()->GetTypeId(), nullptr, nullptr);
        inst->SetInput(0U, init_inst);
        block->PrependInst(init_inst);
        block->PrependInst(save_state);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else if (opc == Opcode::Monitor) {
        auto param0 = CreateParamInst(graph, DataType::REFERENCE, 0U);
        inst->SetInput(0U, param0);
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        save_state->AppendInput(param0);
        save_state->SetVirtualRegister(0U, VirtualRegister(0U, VRegType::VREG));
        inst->SetInput(1U, save_state);
        block->PrependInst(save_state);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else if (opc == Opcode::LoadType || opc == Opcode::LoadString) {
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        inst->SetInput(0U, save_state);
        block->PrependInst(save_state);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else if (opc == Opcode::IsInstance) {
        auto param0 = CreateParamInst(graph, DataType::REFERENCE, 0U);
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        save_state->AppendInput(param0);
        save_state->SetVirtualRegister(0U, VirtualRegister(0U, VRegType::VREG));
        auto load_class = graph->CreateInstLoadClass(DataType::REFERENCE, INVALID_PC, save_state, 0, nullptr,
                                                     reinterpret_cast<RuntimeInterface::ClassPtr>(1));
        inst->SetInput(0U, param0);
        inst->SetInput(1U, load_class);
        inst->SetSaveState(save_state);
        block->PrependInst(load_class);
        block->PrependInst(save_state);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else if (opc == Opcode::NewArray) {
        ASSERT(n == -1L);
        auto init_inst = graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC);
        inst->SetInput(NewArrayInst::INDEX_CLASS, init_inst);

        auto param0 = CreateParamInst(graph, DataType::INT32, 0U);
        inst->SetInput(NewArrayInst::INDEX_SIZE, param0);
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        save_state->AppendInput(param0);
        save_state->SetVirtualRegister(0U, VirtualRegister(0U, VRegType::VREG));

        init_inst->SetTypeId(inst->CastToNewArray()->GetTypeId());
        init_inst->SetInput(0U, save_state);

        inst->SetInput(NewArrayInst::INDEX_SAVE_STATE, save_state);
        block->PrependInst(init_inst);
        block->PrependInst(save_state);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else if (opc == Opcode::NewObject) {
        ASSERT(n == -1L);
        auto save_state = graph->CreateInstSaveState()->CastToSaveState();
        auto init_inst = graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC, save_state,
                                                           inst->CastToNewObject()->GetTypeId(), nullptr, nullptr);
        inst->SetInput(0U, init_inst);
        inst->SetInput(0U, init_inst);
        inst->SetInput(1U, save_state);
        block->PrependInst(init_inst);
        block->PrependInst(save_state);
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    } else {
        ASSERT(n >= 0);
        for (int32_t i = 0; i < n; ++i) {
            auto param = CreateParamInst(graph, type, i);
            if (!inst->IsOperandsDynamic()) {
                inst->SetInput(i, param);
            } else {
                inst->AppendInput(param);
            }
        }
    }
    if (opc == Opcode::Constant || opc == Opcode::Parameter || opc == Opcode::NullPtr) {
        graph->GetStartBlock()->AppendInst(inst);
    } else {
        block->AppendInst(inst);
    }
    if (!inst->IsControlFlow()) {
        if (!inst->NoDest() || IsPseudoUserOfMultiOutput(inst)) {
            auto ret = graph->CreateInstReturn(inst->GetType(), INVALID_PC, inst);
            block->AppendInst(ret);
        } else {
            auto ret = graph->CreateInstReturnVoid();
            block->AppendInst(ret);
        }
    }

    if (opc == Opcode::SaveState || opc == Opcode::SafePoint) {
        auto *save_state = static_cast<SaveStateInst *>(inst);
        for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
            save_state->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
        }
        SetNumVRegsArgs(0U, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    }
    if (inst->GetType() == DataType::REFERENCE) {
        if (inst->GetOpcode() == Opcode::StoreArray) {
            inst->CastToStoreArray()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreArrayI) {
            inst->CastToStoreArrayI()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreStatic) {
            inst->CastToStoreStatic()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreObject) {
            inst->CastToStoreObject()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreArrayPair) {
            inst->CastToStoreArrayPair()->SetNeedBarrier(true);
        }
        if (inst->GetOpcode() == Opcode::StoreArrayPairI) {
            inst->CastToStoreArrayPairI()->SetNeedBarrier(true);
        }
    }
    return graph;
}

Graph *GraphCreator::GenerateCheckOperation(Inst *inst)
{
    Opcode opcode;
    DataType::Type type;
    if (inst->GetOpcode() == Opcode::ZeroCheck) {
        opcode = Opcode::Div;
        type = DataType::UINT64;
    } else if (inst->GetOpcode() == Opcode::NegativeCheck) {
        opcode = Opcode::NewArray;
        type = DataType::INT32;
    } else if (inst->GetOpcode() == Opcode::NullCheck) {
        opcode = Opcode::NewObject;
        type = DataType::REFERENCE;
    } else {
        opcode = Opcode::LoadArray;
        type = DataType::REFERENCE;
    }
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, type, 0U);
    auto param2 = CreateParamInst(graph, DataType::UINT32, 1U);
    auto save_state = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    save_state->AppendInput(param1);
    for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
        save_state->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    SetNumVRegsArgs(0U, save_state->GetInputsCount());
    graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    block->AppendInst(save_state);
    inst->SetInput(0U, param1);
    inst->SetSaveState(save_state);
    inst->SetType(type);
    if (inst->GetOpcode() == Opcode::CheckCast) {
        auto load_class = graph->CreateInstLoadClass(DataType::REFERENCE, INVALID_PC, nullptr, 0, nullptr,
                                                     reinterpret_cast<RuntimeInterface::ClassPtr>(1));
        load_class->SetSaveState(save_state);
        block->AppendInst(load_class);
        inst->SetInput(1U, load_class);
    }
    block->AppendInst(inst);

    Inst *ret = nullptr;
    if (inst->GetOpcode() == Opcode::CheckCast) {
        ret = graph->CreateInstReturn(type, INVALID_PC, param1);
    } else {
        auto new_inst = graph->CreateInst(opcode);
        if (opcode == Opcode::NewArray || opcode == Opcode::NewObject) {
            auto init_inst = graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC);
            init_inst->SetSaveState(save_state);
            block->AppendInst(init_inst);
            if (opcode == Opcode::NewArray) {
                new_inst->SetInput(NewArrayInst::INDEX_CLASS, init_inst);
                new_inst->SetInput(NewArrayInst::INDEX_SIZE, inst);
            } else {
                new_inst->SetInput(0U, init_inst);
            }
            new_inst->SetSaveState(save_state);
            type = DataType::REFERENCE;
        } else if (opcode == Opcode::LoadArray) {
            new_inst->SetInput(0U, param1);
            new_inst->SetInput(1U, param2);
            type = DataType::REFERENCE;
        } else {
            new_inst->SetInput(0U, param1);
            new_inst->SetInput(1U, inst);
            type = DataType::UINT64;
        }
        new_inst->SetType(type);
        block->AppendInst(new_inst);

        ret = graph->CreateInstReturn();
        if (opcode == Opcode::NewArray) {
            ret->SetType(DataType::UINT32);
            ret->SetInput(0U, param2);
        } else {
            ret->SetType(type);
            ret->SetInput(0U, new_inst);
        }
    }
    block->AppendInst(ret);
    return graph;
}

Graph *GraphCreator::GenerateSSOperation(Inst *inst)
{
    DataType::Type type = DataType::UINT64;

    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, type, 0U);
    auto save_state = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    save_state->AppendInput(param1);
    for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
        save_state->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    SetNumVRegsArgs(0U, save_state->GetInputsCount());
    graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    block->AppendInst(save_state);
    if (!inst->IsOperandsDynamic()) {
        inst->SetInput(0U, save_state);
    } else {
        (static_cast<DynamicInputsInst *>(inst))->AppendInput(save_state);
    }
    inst->SetType(type);
    block->AppendInst(inst);

    auto ret = graph->CreateInstReturn(type, INVALID_PC, inst);
    block->AppendInst(ret);
    return graph;
}

Graph *GraphCreator::GenerateBoundaryCheckOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0U);
    auto param2 = CreateParamInst(graph, DataType::UINT32, 1U);

    auto save_state = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    save_state->AppendInput(param1);
    save_state->AppendInput(param2);
    for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
        save_state->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    block->AppendInst(save_state);

    auto len_arr = graph->CreateInstLenArray(DataType::INT32, INVALID_PC, param1);
    block->AppendInst(len_arr);
    auto bounds_check = static_cast<FixedInputsInst3 *>(inst);
    bounds_check->SetInput(0U, len_arr);
    bounds_check->SetType(DataType::INT32);
    if (inst->GetOpcode() == Opcode::BoundsCheck) {
        bounds_check->SetInput(1U, param2);
        bounds_check->SetInput(2U, save_state);
    } else {
        bounds_check->SetInput(1U, save_state);
    }
    block->AppendInst(bounds_check);

    Inst *ld_arr = nullptr;
    if (inst->GetOpcode() == Opcode::BoundsCheck) {
        ld_arr = graph->CreateInstLoadArray(DataType::UINT32, INVALID_PC, param1, bounds_check);
    } else {
        ld_arr = graph->CreateInstLoadArrayI(DataType::UINT32, INVALID_PC, param1, 1U);
    }
    block->AppendInst(ld_arr);

    auto ret = graph->CreateInstReturn(DataType::UINT32, INVALID_PC, ld_arr);
    block->AppendInst(ret);
    SetNumVRegsArgs(0U, save_state->GetInputsCount());
    graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    return graph;
}

Graph *GraphCreator::GenerateMultiArrayOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, DataType::INT32, 0U);
    auto param2 = CreateParamInst(graph, DataType::INT32, 1U);

    auto save_state = graph->CreateInstSaveState();
    block->AppendInst(save_state);

    auto init_inst =
        graph->CreateInstLoadAndInitClass(DataType::REFERENCE, INVALID_PC, save_state, 0U, nullptr, nullptr);
    auto arrays_inst = inst->CastToMultiArray();
    arrays_inst->AllocateInputTypes(&allocator_, 4U);
    inst->AppendInput(init_inst);
    arrays_inst->AddInputType(DataType::REFERENCE);
    inst->AppendInput(param1);
    arrays_inst->AddInputType(DataType::INT32);
    inst->AppendInput(param2);
    arrays_inst->AddInputType(DataType::INT32);
    inst->AppendInput(save_state);
    arrays_inst->AddInputType(DataType::NO_TYPE);

    block->AppendInst(init_inst);
    block->AppendInst(inst);
    for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
        save_state->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    SetNumVRegsArgs(0U, save_state->GetInputsCount());
    graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    return graph;
}

Graph *GraphCreator::GenerateThrowOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2U);
    auto block = graph->GetVectorBlocks()[2U];
    auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0U);

    auto save_state = graph->CreateInstSaveState();
    save_state->AppendInput(param1);
    for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
        save_state->SetVirtualRegister(i, VirtualRegister(i, VRegType::VREG));
    }
    SetNumVRegsArgs(0U, save_state->GetInputsCount());
    graph->SetVRegsCount(save_state->GetInputsCount() + 1U);
    block->AppendInst(save_state);

    inst->SetInput(0U, param1);
    inst->SetInput(1U, save_state);
    block->AppendInst(inst);
    return graph;
}

Graph *GraphCreator::GeneratePhiOperation(Inst *inst)
{
    auto *phi = static_cast<PhiInst *>(inst);
    auto graph = CreateGraphWithFourBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() == 6U);
    auto param1 = CreateParamInst(graph, inst->GetType(), 0U);
    auto param2 = CreateParamInst(graph, inst->GetType(), 1U);
    auto param3 = CreateParamInst(graph, DataType::BOOL, 2U);
    auto add = graph->CreateInstAdd();
    auto sub = graph->CreateInstSub();
    auto if_inst = graph->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, param3, 0U, DataType::BOOL, CC_NE);
    graph->GetVectorBlocks()[2U]->AppendInst(if_inst);
    if (inst->GetType() != DataType::REFERENCE) {
        add->SetInput(0U, param1);
        add->SetInput(1U, param2);
        add->SetType(inst->GetType());
        graph->GetVectorBlocks()[3U]->AppendInst(add);

        sub->SetInput(0U, param1);
        sub->SetInput(1U, param2);
        sub->SetType(inst->GetType());
        graph->GetVectorBlocks()[4U]->AppendInst(sub);

        phi->AppendInput(add);
        phi->AppendInput(sub);
    } else {
        phi->AppendInput(param1);
        phi->AppendInput(param2);
    }
    graph->GetVectorBlocks()[5U]->AppendPhi(phi);
    auto ret = graph->CreateInstReturn(phi->GetType(), INVALID_PC, phi);
    graph->GetVectorBlocks()[5U]->AppendInst(ret);
    return graph;
}

Graph *GraphCreator::CreateGraphWithOneBasicBlock()
{
    Graph *graph = CreateGraph();
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);
    return graph;
}

Graph *GraphCreator::CreateGraphWithTwoBasicBlock()
{
    Graph *graph = CreateGraph();
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block1 = graph->CreateEmptyBlock();
    auto block2 = graph->CreateEmptyBlock();
    entry->AddSucc(block1);
    block1->AddSucc(block2);
    block2->AddSucc(exit);
    return graph;
}

Graph *GraphCreator::CreateGraphWithThreeBasicBlock()
{
    Graph *graph = CreateGraph();
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block_main = graph->CreateEmptyBlock();
    auto block_true = graph->CreateEmptyBlock();
    auto block_false = graph->CreateEmptyBlock();
    auto ret1 = graph->CreateInstReturnVoid();
    auto ret2 = graph->CreateInstReturnVoid();
    block_true->AppendInst(ret1);
    block_false->AppendInst(ret2);
    entry->AddSucc(block_main);
    block_main->AddSucc(block_true);
    block_main->AddSucc(block_false);
    block_true->AddSucc(exit);
    block_false->AddSucc(exit);
    return graph;
}

Graph *GraphCreator::CreateGraphWithFourBasicBlock()
{
    Graph *graph = CreateGraph();
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block_main = graph->CreateEmptyBlock();
    auto block_true = graph->CreateEmptyBlock();
    auto block_false = graph->CreateEmptyBlock();
    auto block_phi = graph->CreateEmptyBlock();

    entry->AddSucc(block_main);
    block_main->AddSucc(block_true);
    block_main->AddSucc(block_false);
    block_true->AddSucc(block_phi);
    block_false->AddSucc(block_phi);
    block_phi->AddSucc(exit);
    return graph;
}

ParameterInst *GraphCreator::CreateParamInst(Graph *graph, DataType::Type type, uint8_t slot)
{
    auto param = graph->CreateInstParameter(slot, type);
    graph->GetStartBlock()->AppendInst(param);
    return param;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperations(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto inst = Inst::New<T>(&allocator_, op_code);
        inst->SetType(opcode_x_possible_types_[op_code][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperationsImm(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto inst = Inst::New<T>(&allocator_, op_code);
        auto type = opcode_x_possible_types_[op_code][i];
        inst->SetType(type);
        inst->SetImm(type == DataType::REFERENCE ? 0U : 1U);
        insts_.push_back(inst);
    }
    return insts_;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperationsShiftedRegister(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        for (auto &shift_type : opcode_x_possible_shift_types_[op_code]) {
            auto inst = Inst::New<T>(&allocator_, op_code);
            auto type = opcode_x_possible_types_[op_code][i];
            inst->SetType(type);
            inst->SetShiftType(shift_type);
            inst->SetImm(type == DataType::REFERENCE ? 0U : 1U);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CallInst>(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto inst = Inst::New<CallInst>(&allocator_, op_code);
        inst->SetType(opcode_x_possible_types_[op_code][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CastInst>(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto inst = Inst::New<CastInst>(&allocator_, op_code);
        inst->SetType(opcode_x_possible_types_[op_code][i]);
        inst->CastToCast()->SetOperandsType(opcode_x_possible_types_[op_code][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CompareInst>(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto type = opcode_x_possible_types_[op_code][i];
        for (int cc_int = ConditionCode::CC_FIRST; cc_int != ConditionCode::CC_LAST; cc_int++) {
            auto cc = static_cast<ConditionCode>(cc_int);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE) {
                continue;
            }
            if (IsFloatType(type) && (cc == ConditionCode::CC_TST_EQ || cc == ConditionCode::CC_TST_NE)) {
                continue;
            }
            auto inst = Inst::New<CompareInst>(&allocator_, op_code);
            inst->SetType(DataType::BOOL);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CmpInst>(Opcode op_code)
{
    auto inst = Inst::New<CmpInst>(&allocator_, op_code);
    inst->SetType(opcode_x_possible_types_[op_code][0U]);
    inst->SetOperandsType(DataType::FLOAT64);
    inst->SetFcmpg();
    insts_.push_back(inst);
    inst = Inst::New<CmpInst>(&allocator_, op_code);
    inst->SetType(opcode_x_possible_types_[op_code][0U]);
    inst->SetOperandsType(DataType::FLOAT64);
    inst->SetFcmpl();
    insts_.push_back(inst);
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<IfInst>(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto type = opcode_x_possible_types_[op_code][i];
        for (int cc_int = ConditionCode::CC_FIRST; cc_int != ConditionCode::CC_LAST; cc_int++) {
            auto cc = static_cast<ConditionCode>(cc_int);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE) {
                continue;
            }
            auto inst = Inst::New<IfInst>(&allocator_, op_code);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperationsImm<IfImmInst>(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto type = opcode_x_possible_types_[op_code][i];
        for (int cc_int = ConditionCode::CC_FIRST; cc_int != ConditionCode::CC_LAST; cc_int++) {
            auto cc = static_cast<ConditionCode>(cc_int);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            auto inst = Inst::New<IfImmInst>(&allocator_, op_code);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            inst->SetImm(type == DataType::REFERENCE ? 0U : 1U);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<SelectInst>(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto cmp_type = opcode_x_possible_types_[op_code][i];
        for (int cc_int = ConditionCode::CC_FIRST; cc_int != ConditionCode::CC_LAST; cc_int++) {
            auto cc = static_cast<ConditionCode>(cc_int);
            if (cmp_type == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            for (size_t j = 0; j < opcode_x_possible_types_[op_code].size(); ++j) {
                auto dst_type = opcode_x_possible_types_[op_code][j];
                auto inst = Inst::New<SelectInst>(&allocator_, op_code);
                inst->SetOperandsType(cmp_type);
                inst->SetType(dst_type);
                inst->SetCc(cc);
                if (dst_type == DataType::REFERENCE) {
                    inst->SetFlag(inst_flags::NO_CSE);
                    inst->SetFlag(inst_flags::NO_HOIST);
                }
                insts_.push_back(inst);
            }
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperationsImm<SelectImmInst>(Opcode op_code)
{
    for (size_t i = 0; i < opcode_x_possible_types_[op_code].size(); ++i) {
        auto cmp_type = opcode_x_possible_types_[op_code][i];
        for (int cc_int = ConditionCode::CC_FIRST; cc_int != ConditionCode::CC_LAST; cc_int++) {
            auto cc = static_cast<ConditionCode>(cc_int);
            if (cmp_type == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            for (size_t j = 0; j < opcode_x_possible_types_[op_code].size(); ++j) {
                auto dst_type = opcode_x_possible_types_[op_code][j];
                auto inst = Inst::New<SelectImmInst>(&allocator_, op_code);
                inst->SetOperandsType(cmp_type);
                inst->SetType(dst_type);
                inst->SetCc(cc);
                inst->SetImm(cmp_type == DataType::REFERENCE ? 0U : 1U);
                if (dst_type == DataType::REFERENCE) {
                    inst->SetFlag(inst_flags::NO_CSE);
                    inst->SetFlag(inst_flags::NO_HOIST);
                }
                insts_.push_back(inst);
            }
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<SpillFillInst>(Opcode op_code)
{
    auto inst = Inst::New<SpillFillInst>(&allocator_, op_code);
    inst->SetType(opcode_x_possible_types_[op_code][0U]);
    inst->AddSpill(0U, 2U, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, op_code);
    inst->SetType(opcode_x_possible_types_[op_code][0U]);
    inst->AddFill(0U, 2U, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, op_code);
    inst->SetType(opcode_x_possible_types_[op_code][0U]);
    inst->AddMove(0U, 2U, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, op_code);
    inst->SetType(opcode_x_possible_types_[op_code][0U]);
    inst->AddMemCopy(0U, 2U, DataType::UINT64);
    insts_.push_back(inst);
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<MonitorInst>(Opcode op_code)
{
    auto inst = Inst::New<MonitorInst>(&allocator_, op_code);
    inst->SetType(opcode_x_possible_types_[op_code][0U]);
    inst->SetEntry();
    insts_.push_back(inst);

    inst = Inst::New<MonitorInst>(&allocator_, op_code);
    inst->SetType(opcode_x_possible_types_[op_code][0U]);
    inst->SetExit();
    insts_.push_back(inst);

    return insts_;
}

#include "generate_operations_intrinsic_inst.inl"

std::vector<Inst *> &InstGenerator::Generate(Opcode op_code)
{
    insts_.clear();
    switch (op_code) {
        case Opcode::Neg:
        case Opcode::Abs:
        case Opcode::Not:
            return GenerateOperations<UnaryOperation>(op_code);
        case Opcode::Add:
        case Opcode::Sub:
        case Opcode::Mul:
        case Opcode::Div:
        case Opcode::Mod:
        case Opcode::Min:
        case Opcode::Max:
        case Opcode::Shl:
        case Opcode::Shr:
        case Opcode::AShr:
        case Opcode::And:
        case Opcode::Or:
        case Opcode::Xor:
        case Opcode::AndNot:
        case Opcode::OrNot:
        case Opcode::XorNot:
        case Opcode::MNeg:
            return GenerateOperations<BinaryOperation>(op_code);
        case Opcode::Compare:
            return GenerateOperations<CompareInst>(op_code);
        case Opcode::Constant:
            return GenerateOperations<ConstantInst>(op_code);
        case Opcode::NewObject:
            return GenerateOperations<NewObjectInst>(op_code);
        case Opcode::If:
            return GenerateOperations<IfInst>(op_code);
        case Opcode::IfImm:
            return GenerateOperationsImm<IfImmInst>(op_code);
        case Opcode::IsInstance:
            return GenerateOperations<IsInstanceInst>(op_code);
        case Opcode::LenArray:
            return GenerateOperations<LengthMethodInst>(op_code);
        case Opcode::Return:
        case Opcode::ReturnInlined:
            return GenerateOperations<FixedInputsInst1>(op_code);
        case Opcode::NewArray:
            return GenerateOperations<NewArrayInst>(op_code);
        case Opcode::Cmp:
            return GenerateOperations<CmpInst>(op_code);
        case Opcode::CheckCast:
            return GenerateOperations<CheckCastInst>(op_code);
        case Opcode::NullCheck:
        case Opcode::ZeroCheck:
        case Opcode::NegativeCheck:
            return GenerateOperations<FixedInputsInst2>(op_code);
        case Opcode::Throw:
            return GenerateOperations<ThrowInst>(op_code);
        case Opcode::BoundsCheck:
        case Opcode::MAdd:
        case Opcode::MSub:
            return GenerateOperations<FixedInputsInst3>(op_code);
        case Opcode::Parameter:
            return GenerateOperations<ParameterInst>(op_code);
        case Opcode::LoadArray:
            return GenerateOperations<LoadInst>(op_code);
        case Opcode::StoreArray:
            return GenerateOperations<StoreInst>(op_code);
        case Opcode::LoadArrayI:
            return GenerateOperationsImm<LoadInstI>(op_code);
        case Opcode::StoreArrayI:
            return GenerateOperationsImm<StoreInstI>(op_code);
        case Opcode::NullPtr:
        case Opcode::ReturnVoid:
            return GenerateOperations<FixedInputsInst0>(op_code);
        case Opcode::SaveState:
        case Opcode::SafePoint:
            return GenerateOperations<SaveStateInst>(op_code);
        case Opcode::Phi:
            return GenerateOperations<PhiInst>(op_code);
        case Opcode::CallStatic:
        case Opcode::CallVirtual:
            return GenerateOperations<CallInst>(op_code);
        case Opcode::Monitor:
            return GenerateOperations<MonitorInst>(op_code);
        case Opcode::AddI:
        case Opcode::SubI:
        case Opcode::ShlI:
        case Opcode::ShrI:
        case Opcode::AShrI:
        case Opcode::AndI:
        case Opcode::OrI:
        case Opcode::XorI:
            return GenerateOperationsImm<BinaryImmOperation>(op_code);
        case Opcode::AndSR:
        case Opcode::SubSR:
        case Opcode::AddSR:
        case Opcode::OrSR:
        case Opcode::XorSR:
        case Opcode::AndNotSR:
        case Opcode::OrNotSR:
        case Opcode::XorNotSR:
            return GenerateOperationsShiftedRegister<BinaryShiftedRegisterOperation>(op_code);
        case Opcode::NegSR:
            return GenerateOperationsShiftedRegister<UnaryShiftedRegisterOperation>(op_code);
        case Opcode::SpillFill:
            return GenerateOperations<SpillFillInst>(op_code);
        case Opcode::LoadObject:
            return GenerateOperations<LoadObjectInst>(op_code);
        case Opcode::StoreObject:
            return GenerateOperations<StoreObjectInst>(op_code);
        case Opcode::LoadStatic:
            return GenerateOperations<LoadStaticInst>(op_code);
        case Opcode::StoreStatic:
            return GenerateOperations<StoreStaticInst>(op_code);
        case Opcode::LoadString:
        case Opcode::LoadType:
            return GenerateOperations<LoadFromPool>(op_code);
        case Opcode::BoundsCheckI:
            return GenerateOperationsImm<BoundsCheckInstI>(op_code);
        case Opcode::ReturnI:
            return GenerateOperationsImm<ReturnInstI>(op_code);
        case Opcode::Intrinsic:
            return GenerateOperations<IntrinsicInst>(op_code);
        case Opcode::Select:
            return GenerateOperations<SelectInst>(op_code);
        case Opcode::SelectImm:
            return GenerateOperationsImm<SelectImmInst>(op_code);
        case Opcode::LoadArrayPair:
            return GenerateOperations<LoadArrayPairInst>(op_code);
        case Opcode::LoadArrayPairI:
            return GenerateOperationsImm<LoadArrayPairInstI>(op_code);
        case Opcode::StoreArrayPair:
            return GenerateOperations<StoreArrayPairInst>(op_code);
        case Opcode::StoreArrayPairI:
            return GenerateOperationsImm<StoreArrayPairInstI>(op_code);
        case Opcode::Cast:
            return GenerateOperations<CastInst>(op_code);
        case Opcode::Builtin:
            ASSERT_DO(0U, std::cerr << "Unexpected Opcode Builtin\n");
            return insts_;
        default:
            ASSERT_DO(0U, std::cerr << GetOpcodeString(op_code) << "\n");
            return insts_;
    }
}

constexpr std::array<const char *, 15U> LABELS = {"NO_TYPE", "REF",     "BOOL",    "UINT8", "INT8",
                                                  "UINT16",  "INT16",   "UINT32",  "INT32", "UINT64",
                                                  "INT64",   "FLOAT32", "FLOAT64", "ANY",   "VOID"};

void StatisticGenerator::GenerateHTMLPage(const std::string &file_name)
{
    std::ofstream html_page;
    html_page.open(file_name);
    html_page << "<!DOCTYPE html>\n"
              << "<html>\n"
              << "<head>\n"
              << "\t<style>table, th, td {border: 1px solid black; border-collapse: collapse;}</style>\n"
              << "\t<title>Codegen coverage statistic</title>\n"
              << "</head>\n"
              << "<body>\n"
              << "\t<header><h1>Codegen coverage statistic</h1></header>"
              << "\t<h3>Legend</h3>"
              << "\t<table style=\"width:300px%\">\n"
              << "\t\t<tr><th align=\"left\">Codegen successfully translate IR</th><td align=\"center\""
              << "bgcolor=\"#00fd00\" width=\"90px\">+</td></tr>\n"
              << "\t\t<tr><th align=\"left\">Codegen UNsuccessfully translate IR</th><td align=\"center\""
              << "bgcolor=\"#fd0000\">-</td></tr>\n"
              << "\t\t<tr><th align=\"left\">IR does't support instruction with this type </th><td></td></tr>\n"
              << "\t\t<tr><th align=\"left\">Test generator not implement for this opcode</th><td "
              << "bgcolor=\"#808080\"></td></tr>\n"
              << "\t</table>\n"
              << "\t<br>\n"
              << "\t<h3>Summary information</h3>\n"
              << "\t<table>\n"
              << "\t\t<tr><th>Positive tests</th><td>" << positive_inst_number_ << "</td></tr>\n"
              << "\t\t<tr><th>All generated tests</th><td>" << all_inst_number_ << "</td></tr>\n"
              << "\t\t<tr><th></th><td>" << positive_inst_number_ * 100.0 / all_inst_number_ << "%</td></tr>\n"
              << "\t</table>\n"
              << "\t<br>\n"
              << "\t<table>"
              << "\t\t<tr><th align=\"left\">Number of opcodes for which tests were generated</th><td>"
              << implemented_opcode_number_ << "</td></tr>"
              << "\t\t<tr><th align=\"left\">Full number of opcodes</th><td>" << all_opcode_number_ << "</td></tr>"
              << "\t\t<tr><th></th><td>" << implemented_opcode_number_ * 100.0 / all_opcode_number_ << "%</td></tr>"
              << "\t</table>\n"
              << "\t<h3>Detailed information</h3>"
              << "\t\t<table>"
              << "\t\t<tr><th>Opcode\\Type</th>";
    for (auto label : LABELS) {
        html_page << "<th style=\"width:90px\">" << label << "</th>";
    }
    html_page << "<th>%</th>";
    html_page << "<tr>\n";
    for (auto i = 0; i != static_cast<int>(Opcode::NUM_OPCODES); ++i) {
        auto opc = static_cast<Opcode>(i);
        if (opc == Opcode::NOP || opc == Opcode::Intrinsic || opc == Opcode::LoadPairPart) {
            continue;
        }
        html_page << "\t\t<tr>";
        html_page << "<th>" << GetOpcodeString(opc) << "</th>";
        if (statistic_.first.find(opc) != statistic_.first.end()) {
            auto item = statistic_.first[opc];
            int positiv_count = 0;
            int negativ_count = 0;
            for (auto &j : item) {
                std::string flag;
                std::string color;
                switch (j.second) {
                    case 0U:
                        flag = "-";
                        color = "bgcolor=\"#fd0000\"";
                        negativ_count++;
                        break;
                    case 1U:
                        flag = "+";
                        color = "bgcolor=\"#00fd00\"";
                        positiv_count++;
                        break;
                    default:
                        break;
                }
                html_page << "<td align=\"center\" " << color << ">" << flag << "</td>";
            }
            if (positiv_count + negativ_count != 0U) {
                html_page << "<td align=\"right\">" << positiv_count * 100.0 / (positiv_count + negativ_count)
                          << "</td>";
            }
        } else {
            for (auto j = tmplt_.begin(); j != tmplt_.end(); ++j) {
                html_page << R"(<td align=" center " bgcolor=" #808080"></td>)";
            }
            html_page << "<td align=\"right\">0</td>";
        }
        html_page << "</tr>\n";
    }
    html_page << "\t</table>\n";

    html_page << "\t<h3>Intrinsics</h3>\n";
    html_page << "\t<table>\n";
    html_page << "\t\t<tr><th>IntrinsicId</th><th>Status</th></tr>";
    for (auto i = 0; i != static_cast<int>(RuntimeInterface::IntrinsicId::COUNT); ++i) {
        auto intrinsic_id = static_cast<RuntimeInterface::IntrinsicId>(i);
        auto intrinsic_name = GetIntrinsicName(intrinsic_id);
        if (intrinsic_name.empty()) {
            continue;
        }
        html_page << "<tr><th>" << intrinsic_name << "</th>";
        if (statistic_.second.find(intrinsic_id) != statistic_.second.end()) {
            std::string flag;
            std::string color;
            if (statistic_.second[intrinsic_id]) {
                flag = "+";
                color = "bgcolor=\"#00fd00\"";
            } else {
                flag = "-";
                color = "bgcolor=\"#fd0000\"";
            }
            html_page << "<td align=\"center\" " << color << ">" << flag << "</td></tr>";
        } else {
            html_page << R"(<td align=" center " bgcolor=" #808080"></td></tr>)";
        }
        html_page << "\n";
    }
    html_page << "</table>\n";
    html_page << "</body>\n"
              << "</html>";
    html_page.close();
}
}  // namespace panda::compiler
