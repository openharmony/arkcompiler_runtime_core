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
    SetNumVRegsArgs(0, 0);
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
            graph = GenerateOperation(inst, -1);
            break;
        case Opcode::ReturnVoid:
        case Opcode::NullPtr:
        case Opcode::Constant:
        case Opcode::Parameter:
        case Opcode::SpillFill:
        case Opcode::ReturnI:
            graph = GenerateOperation(inst, 0);
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
            graph = GenerateOperation(inst, 1);
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
            graph = GenerateOperation(inst, 2);
            break;
        case Opcode::MAdd:
        case Opcode::MSub:
            graph = GenerateOperation(inst, 3);
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
            ASSERT_DO(0, inst->Dump(&std::cerr));
            graph = nullptr;
            break;
    }
    if (graph != nullptr) {
        auto id = graph->GetCurrentInstructionId();
        inst->SetId(id);
        graph->SetCurrentInstructionId(++id);
        graph->ResetParameterInfo();
        for (auto param : graph->GetStartBlock()->Insts()) {
            if (param->GetOpcode() == Opcode::Parameter) {
                param->CastToParameter()->SetLocationData(graph->GetDataForNativeParam(param->GetType()));
                runtime_.arg_types->push_back(param->GetType());
            }
        }
    }
    if (inst->GetType() == DataType::NO_TYPE || inst->IsStore()) {
        runtime_.return_type = DataType::VOID;
    } else {
        runtime_.return_type = inst->GetType();
    }
#ifndef NDEBUG
    // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
    graph->SetLowLevelInstructionsEnabled();
#endif
    return graph;
}

Graph *GraphCreator::CreateGraph()
{
    Graph *graph = allocator_.New<Graph>(&allocator_, &local_allocator_, arch_);
    runtime_.arg_types = allocator_.New<ArenaVector<DataType::Type>>(allocator_.Adapter());
    graph->SetRuntime(&runtime_);
    graph->SetStackSlotsCount(3U);
    return graph;
}

Graph *GraphCreator::GenerateOperation(Inst *inst, int32_t n)
{
    Graph *graph;
    auto opc = inst->GetOpcode();
    if (opc == Opcode::If || opc == Opcode::IfImm) {
        graph = CreateGraphWithThreeBasicBlock();
    } else {
        graph = CreateGraphWithOneBasicBlock();
    }
    ASSERT(graph->GetVectorBlocks().size() > 2);
    auto block = graph->GetVectorBlocks()[2];
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
        auto array = CreateParamInst(graph, DataType::REFERENCE, 0);
        Inst *index = nullptr;
        if (opc == Opcode::LoadArrayPair) {
            index = CreateParamInst(graph, DataType::INT32, 1);
        }
        inst->SetInput(0, array);
        if (opc == Opcode::LoadArrayPair) {
            inst->SetInput(1, index);
        }
        block->AppendInst(inst);
        auto load_pair_part0 = graph->CreateInstLoadPairPart()->CastToLoadPairPart();
        auto load_pair_part1 = graph->CreateInstLoadPairPart()->CastToLoadPairPart();
        load_pair_part0->SetType(inst->GetType());
        load_pair_part0->SetImm(0);
        load_pair_part0->SetInput(0, inst);
        load_pair_part1->SetType(inst->GetType());
        load_pair_part1->SetImm(1);
        load_pair_part1->SetInput(0, inst);
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
        ASSERT(n == -1);
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        block->AppendInst(saveState);

        auto call_inst = static_cast<CallInst *>(graph->CreateInstCallStatic());
        call_inst->SetType(DataType::VOID);
        call_inst->SetInlined(true);
        call_inst->AllocateInputTypes(&allocator_, 0);
        call_inst->AppendInput(saveState);
        call_inst->AddInputType(DataType::NO_TYPE);
        block->AppendInst(call_inst);

        inst->SetInput(0, saveState);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    } else if (opc == Opcode::CallStatic || opc == Opcode::CallVirtual) {
        ASSERT(n >= 0);
        auto call_inst = static_cast<CallInst *>(inst);
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        block->PrependInst(saveState);
        call_inst->AllocateInputTypes(&allocator_, n);
        for (int32_t i = 0; i < n; ++i) {
            auto param = CreateParamInst(graph, type, i);
            call_inst->AppendInput(param);
            call_inst->AddInputType(type);
            saveState->AppendInput(param);
        }
        for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
            saveState->SetVirtualRegister(i, VirtualRegister(i, false));
        }
        call_inst->AppendInput(saveState);
        call_inst->AddInputType(DataType::NO_TYPE);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    } else if (opc == Opcode::LoadArray || opc == Opcode::StoreArray) {
        ASSERT(n == -1);
        auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0);  // array
        auto param2 = CreateParamInst(graph, DataType::INT32, 1);      // index
        inst->SetInput(0, param1);
        inst->SetInput(1, param2);
        if (inst->GetOpcode() == Opcode::StoreArray) {
            auto param3 = CreateParamInst(graph, type, 2);
            inst->SetInput(2, param3);
        }
    } else if (opc == Opcode::LoadArrayI || opc == Opcode::StoreArrayI || opc == Opcode::StoreObject) {
        ASSERT(n == -1);
        auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0);  // array/object
        inst->SetInput(0, param1);
        if (inst->GetOpcode() != Opcode::LoadArrayI) {
            auto param2 = CreateParamInst(graph, type, 1);
            inst->SetInput(1, param2);
        }
    } else if (opc == Opcode::Select) {
        ASSERT(n == -1);
        auto cmp_type = inst->CastToSelect()->GetOperandsType();
        auto param0 = CreateParamInst(graph, type, 0);
        auto param1 = CreateParamInst(graph, type, 1);
        auto param2 = CreateParamInst(graph, cmp_type, 2);
        auto param3 = CreateParamInst(graph, cmp_type, 3);
        inst->SetInput(0, param0);
        inst->SetInput(1, param1);
        inst->SetInput(2, param2);
        inst->SetInput(3, param3);
    } else if (opc == Opcode::SelectImm) {
        ASSERT(n == -1);
        auto cmp_type = inst->CastToSelectImm()->GetOperandsType();
        auto param0 = CreateParamInst(graph, type, 0);
        auto param1 = CreateParamInst(graph, type, 1);
        auto param2 = CreateParamInst(graph, cmp_type, 2);
        inst->SetInput(0, param0);
        inst->SetInput(1, param1);
        inst->SetInput(2, param2);
    } else if (opc == Opcode::StoreStatic) {
        auto param0 = CreateParamInst(graph, type, 0);
        inst->SetInput(1, param0);
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        saveState->AppendInput(param0);
        saveState->SetVirtualRegister(0, VirtualRegister(0, false));
        auto init_inst = graph->CreateInstLoadAndInitClass();
        init_inst->SetTypeId(inst->CastToStoreStatic()->GetTypeId());
        init_inst->SetType(DataType::REFERENCE);
        init_inst->SetInput(0, saveState);
        inst->SetInput(0, init_inst);
        block->PrependInst(init_inst);
        block->PrependInst(saveState);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    } else if (opc == Opcode::LoadStatic) {
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        auto init_inst = graph->CreateInstLoadAndInitClass();
        inst->SetInput(0, init_inst);
        init_inst->SetTypeId(inst->CastToLoadStatic()->GetTypeId());
        init_inst->SetType(DataType::REFERENCE);
        init_inst->SetInput(0, saveState);
        block->PrependInst(init_inst);
        block->PrependInst(saveState);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    } else if (opc == Opcode::Monitor) {
        auto param0 = CreateParamInst(graph, DataType::REFERENCE, 0);
        inst->SetInput(0, param0);
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        saveState->AppendInput(param0);
        saveState->SetVirtualRegister(0, VirtualRegister(0, false));
        inst->SetInput(1, saveState);
        block->PrependInst(saveState);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    } else if (opc == Opcode::LoadType || opc == Opcode::LoadString) {
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        inst->SetInput(0, saveState);
        block->PrependInst(saveState);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    } else if (opc == Opcode::IsInstance) {
        auto param0 = CreateParamInst(graph, DataType::REFERENCE, 0);
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        saveState->AppendInput(param0);
        saveState->SetVirtualRegister(0, VirtualRegister(0, false));
        auto loadClass = graph->CreateInstLoadClass();
        loadClass->SetType(DataType::REFERENCE);
        loadClass->SetInput(0, saveState);
        loadClass->SetClass(reinterpret_cast<RuntimeInterface::ClassPtr>(1));
        inst->SetInput(0, param0);
        inst->SetInput(1, loadClass);
        inst->SetSaveState(saveState);
        block->PrependInst(loadClass);
        block->PrependInst(saveState);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    } else if (opc == Opcode::NewArray) {
        ASSERT(n == -1);
        auto init_inst = graph->CreateInstLoadAndInitClass();
        inst->SetInput(NewArrayInst::INDEX_CLASS, init_inst);

        auto param0 = CreateParamInst(graph, DataType::INT32, 0);
        inst->SetInput(NewArrayInst::INDEX_SIZE, param0);
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        saveState->AppendInput(param0);
        saveState->SetVirtualRegister(0, VirtualRegister(0, false));

        init_inst->SetTypeId(inst->CastToNewArray()->GetTypeId());
        init_inst->SetType(DataType::REFERENCE);
        init_inst->SetInput(0, saveState);

        inst->SetInput(NewArrayInst::INDEX_SAVE_STATE, saveState);
        block->PrependInst(init_inst);
        block->PrependInst(saveState);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    } else if (opc == Opcode::NewObject) {
        ASSERT(n == -1);
        auto saveState = graph->CreateInstSaveState()->CastToSaveState();
        auto init_inst = graph->CreateInstLoadAndInitClass();
        inst->SetInput(0, init_inst);
        init_inst->SetTypeId(inst->CastToNewObject()->GetTypeId());
        init_inst->SetType(DataType::REFERENCE);
        init_inst->SetInput(0, saveState);
        inst->SetInput(0, init_inst);
        inst->SetInput(1, saveState);
        block->PrependInst(init_inst);
        block->PrependInst(saveState);
        SetNumVRegsArgs(0, saveState->GetInputsCount());
        graph->SetVRegsCount(saveState->GetInputsCount() + 1);
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
            auto ret = graph->CreateInstReturn();
            ret->SetInput(0, inst);
            block->AppendInst(ret);
            ret->SetType(inst->GetType());
        } else {
            auto ret = graph->CreateInstReturnVoid();
            block->AppendInst(ret);
        }
    }

    if (opc == Opcode::SaveState || opc == Opcode::SafePoint) {
        auto *save_state = static_cast<SaveStateInst *>(inst);
        for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
            save_state->SetVirtualRegister(i, VirtualRegister(i, false));
        }
        SetNumVRegsArgs(0, save_state->GetInputsCount());
        graph->SetVRegsCount(save_state->GetInputsCount() + 1);
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
    ASSERT(graph->GetVectorBlocks().size() > 2);
    auto block = graph->GetVectorBlocks()[2];
    auto param1 = CreateParamInst(graph, type, 0);
    auto saveState = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    saveState->AppendInput(param1);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, false));
    }
    SetNumVRegsArgs(0, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    block->AppendInst(saveState);
    inst->SetInput(0, param1);
    inst->SetSaveState(saveState);
    inst->SetType(type);
    if (inst->GetOpcode() == Opcode::CheckCast) {
        auto loadClass = graph->CreateInstLoadClass();
        loadClass->SetType(DataType::REFERENCE);
        loadClass->SetSaveState(saveState);
        loadClass->SetClass(reinterpret_cast<RuntimeInterface::ClassPtr>(1));
        block->AppendInst(loadClass);
        inst->SetInput(1, loadClass);
    }
    block->AppendInst(inst);

    if (inst->GetOpcode() == Opcode::CheckCast) {
        auto ret = graph->CreateInstReturnVoid();
        block->AppendInst(ret);
    } else {
        auto new_inst = graph->CreateInst(opcode);
        if (opcode == Opcode::NewArray || opcode == Opcode::NewObject) {
            auto init_inst = graph->CreateInstLoadAndInitClass();
            init_inst->SetType(DataType::REFERENCE);
            init_inst->SetSaveState(saveState);
            block->AppendInst(init_inst);
            if (opcode == Opcode::NewArray) {
                new_inst->SetInput(NewArrayInst::INDEX_CLASS, init_inst);
                new_inst->SetInput(NewArrayInst::INDEX_SIZE, inst);
            } else {
                new_inst->SetInput(0, init_inst);
            }
            new_inst->SetSaveState(saveState);
            type = DataType::REFERENCE;
        } else {
            new_inst->SetInput(0, param1);
            new_inst->SetInput(1, inst);
            type = DataType::UINT64;
        }
        new_inst->SetType(type);
        block->AppendInst(new_inst);

        auto ret = graph->CreateInstReturn();
        ret->SetType(type);
        ret->SetInput(0, new_inst);
        block->AppendInst(ret);
    }
    return graph;
}

Graph *GraphCreator::GenerateSSOperation(Inst *inst)
{
    DataType::Type type = DataType::UINT64;

    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2);
    auto block = graph->GetVectorBlocks()[2];
    auto param1 = CreateParamInst(graph, type, 0);
    auto saveState = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    saveState->AppendInput(param1);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, false));
    }
    SetNumVRegsArgs(0, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    block->AppendInst(saveState);
    if (!inst->IsOperandsDynamic()) {
        inst->SetInput(0, saveState);
    } else {
        (static_cast<DynamicInputsInst *>(inst))->AppendInput(saveState);
    }
    inst->SetType(type);
    block->AppendInst(inst);

    auto ret = graph->CreateInstReturn();
    ret->SetType(type);
    ret->SetInput(0, inst);
    block->AppendInst(ret);
    return graph;
}

Graph *GraphCreator::GenerateBoundaryCheckOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2);
    auto block = graph->GetVectorBlocks()[2];
    auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0);
    auto param2 = CreateParamInst(graph, DataType::UINT64, 1);

    auto saveState = static_cast<SaveStateInst *>(graph->CreateInstSaveState());
    saveState->AppendInput(param1);
    saveState->AppendInput(param2);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, false));
    }
    block->AppendInst(saveState);

    auto lenArr = graph->CreateInstLenArray();
    lenArr->SetInput(0, param1);
    lenArr->SetType(DataType::INT32);
    block->AppendInst(lenArr);
    auto boundsCheck = static_cast<FixedInputsInst3 *>(inst);
    boundsCheck->SetInput(0, lenArr);
    boundsCheck->SetType(DataType::INT32);
    if (inst->GetOpcode() == Opcode::BoundsCheck) {
        boundsCheck->SetInput(1, param2);
        boundsCheck->SetInput(2, saveState);
    } else {
        boundsCheck->SetInput(1, saveState);
    }
    block->AppendInst(boundsCheck);

    Inst *ldArr = nullptr;
    if (inst->GetOpcode() == Opcode::BoundsCheck) {
        ldArr = graph->CreateInstLoadArray();
        ldArr->SetInput(1, boundsCheck);
    } else {
        auto ldArrI = graph->CreateInstLoadArrayI();
        ldArrI->SetImm(1);
        ldArr = ldArrI;
    }
    ldArr->SetInput(0, param1);
    ldArr->SetType(DataType::UINT64);
    block->AppendInst(ldArr);

    auto ret = graph->CreateInstReturn();
    ret->SetType(DataType::UINT64);
    ret->SetInput(0, ldArr);
    block->AppendInst(ret);
    SetNumVRegsArgs(0, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    return graph;
}

Graph *GraphCreator::GenerateMultiArrayOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2);
    auto block = graph->GetVectorBlocks()[2];
    auto param1 = CreateParamInst(graph, DataType::INT32, 0);
    auto param2 = CreateParamInst(graph, DataType::INT32, 1);

    auto saveState = graph->CreateInstSaveState();
    block->AppendInst(saveState);

    auto init_inst = graph->CreateInstLoadAndInitClass();
    init_inst->SetType(DataType::REFERENCE);
    init_inst->SetInput(0, saveState);
    auto arrays_inst = inst->CastToMultiArray();
    arrays_inst->AllocateInputTypes(&allocator_, 4);
    inst->AppendInput(init_inst);
    arrays_inst->AddInputType(DataType::REFERENCE);
    inst->AppendInput(param1);
    arrays_inst->AddInputType(DataType::INT32);
    inst->AppendInput(param2);
    arrays_inst->AddInputType(DataType::INT32);
    inst->AppendInput(saveState);
    arrays_inst->AddInputType(DataType::NO_TYPE);

    block->AppendInst(init_inst);
    block->AppendInst(inst);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, false));
    }
    SetNumVRegsArgs(0, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    return graph;
}

Graph *GraphCreator::GenerateThrowOperation(Inst *inst)
{
    auto graph = CreateGraphWithOneBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() > 2);
    auto block = graph->GetVectorBlocks()[2];
    auto param1 = CreateParamInst(graph, DataType::REFERENCE, 0);

    auto saveState = graph->CreateInstSaveState();
    saveState->AppendInput(param1);
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        saveState->SetVirtualRegister(i, VirtualRegister(i, false));
    }
    SetNumVRegsArgs(0, saveState->GetInputsCount());
    graph->SetVRegsCount(saveState->GetInputsCount() + 1);
    block->AppendInst(saveState);

    inst->SetInput(0, param1);
    inst->SetInput(1, saveState);
    block->AppendInst(inst);
    return graph;
}

Graph *GraphCreator::GeneratePhiOperation(Inst *inst)
{
    PhiInst *phi = static_cast<PhiInst *>(inst);
    auto graph = CreateGraphWithFourBasicBlock();
    ASSERT(graph->GetVectorBlocks().size() == 6);
    auto param1 = CreateParamInst(graph, inst->GetType(), 0);
    auto param2 = CreateParamInst(graph, inst->GetType(), 1);
    auto param3 = CreateParamInst(graph, DataType::BOOL, 2);
    auto add = graph->CreateInstAdd();
    auto sub = graph->CreateInstSub();
    auto if_inst = graph->CreateInstIfImm();
    if_inst->SetOperandsType(DataType::BOOL);
    if_inst->SetCc(CC_NE);
    if_inst->SetImm(0);
    if_inst->SetInput(0, param3);
    graph->GetVectorBlocks()[2]->AppendInst(if_inst);
    if (inst->GetType() != DataType::REFERENCE) {
        add->SetInput(0, param1);
        add->SetInput(1, param2);
        add->SetType(inst->GetType());
        graph->GetVectorBlocks()[3]->AppendInst(add);

        sub->SetInput(0, param1);
        sub->SetInput(1, param2);
        sub->SetType(inst->GetType());
        graph->GetVectorBlocks()[4]->AppendInst(sub);

        phi->AppendInput(add);
        phi->AppendInput(sub);
    } else {
        phi->AppendInput(param1);
        phi->AppendInput(param2);
    }
    graph->GetVectorBlocks()[5]->AppendPhi(phi);
    auto ret = graph->CreateInstReturn();
    ret->SetType(phi->GetType());
    ret->SetInput(0, phi);
    graph->GetVectorBlocks()[5]->AppendInst(ret);
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
    auto param = graph->CreateInstParameter(slot);
    param->SetType(type);
    graph->GetStartBlock()->AppendInst(param);
    return param;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperations(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto inst = Inst::New<T>(&allocator_, OpCode);
        inst->SetType(opcode_x_possible_types_[OpCode][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperationsImm(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto inst = Inst::New<T>(&allocator_, OpCode);
        auto type = opcode_x_possible_types_[OpCode][i];
        inst->SetType(type);
        inst->SetImm(type == DataType::REFERENCE ? 0 : 1);
        insts_.push_back(inst);
    }
    return insts_;
}

template <class T>
std::vector<Inst *> &InstGenerator::GenerateOperationsShiftedRegister(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        for (auto &shift_type : opcode_x_possible_shift_types_[OpCode]) {
            auto inst = Inst::New<T>(&allocator_, OpCode);
            auto type = opcode_x_possible_types_[OpCode][i];
            inst->SetType(type);
            inst->SetShiftType(shift_type);
            inst->SetImm(type == DataType::REFERENCE ? 0 : 1);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CallInst>(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto inst = Inst::New<CallInst>(&allocator_, OpCode);
        inst->SetType(opcode_x_possible_types_[OpCode][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CastInst>(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto inst = Inst::New<CastInst>(&allocator_, OpCode);
        inst->SetType(opcode_x_possible_types_[OpCode][i]);
        inst->CastToCast()->SetOperandsType(opcode_x_possible_types_[OpCode][i]);
        insts_.push_back(inst);
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CompareInst>(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto type = opcode_x_possible_types_[OpCode][i];
        for (int CCInt = ConditionCode::CC_FIRST; CCInt != ConditionCode::CC_LAST; CCInt++) {
            auto cc = static_cast<ConditionCode>(CCInt);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE) {
                continue;
            }
            if (IsFloatType(type) && (cc == ConditionCode::CC_TST_EQ || cc == ConditionCode::CC_TST_NE)) {
                continue;
            }
            auto inst = Inst::New<CompareInst>(&allocator_, OpCode);
            inst->SetType(DataType::BOOL);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<CmpInst>(Opcode OpCode)
{
    auto inst = Inst::New<CmpInst>(&allocator_, OpCode);
    inst->SetType(opcode_x_possible_types_[OpCode][0]);
    inst->SetOperandsType(DataType::FLOAT64);
    inst->SetFcmpg();
    insts_.push_back(inst);
    inst = Inst::New<CmpInst>(&allocator_, OpCode);
    inst->SetType(opcode_x_possible_types_[OpCode][0]);
    inst->SetOperandsType(DataType::FLOAT64);
    inst->SetFcmpl();
    insts_.push_back(inst);
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<IfInst>(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto type = opcode_x_possible_types_[OpCode][i];
        for (int CCInt = ConditionCode::CC_FIRST; CCInt != ConditionCode::CC_LAST; CCInt++) {
            auto cc = static_cast<ConditionCode>(CCInt);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE) {
                continue;
            }
            auto inst = Inst::New<IfInst>(&allocator_, OpCode);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperationsImm<IfImmInst>(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto type = opcode_x_possible_types_[OpCode][i];
        for (int CCInt = ConditionCode::CC_FIRST; CCInt != ConditionCode::CC_LAST; CCInt++) {
            auto cc = static_cast<ConditionCode>(CCInt);
            if (type == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            auto inst = Inst::New<IfImmInst>(&allocator_, OpCode);
            inst->SetCc(cc);
            inst->SetOperandsType(type);
            inst->SetImm(type == DataType::REFERENCE ? 0 : 1);
            insts_.push_back(inst);
        }
    }
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<SelectInst>(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto cmp_type = opcode_x_possible_types_[OpCode][i];
        for (int CCInt = ConditionCode::CC_FIRST; CCInt != ConditionCode::CC_LAST; CCInt++) {
            auto cc = static_cast<ConditionCode>(CCInt);
            if (cmp_type == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            for (size_t j = 0; j < opcode_x_possible_types_[OpCode].size(); ++j) {
                auto dst_type = opcode_x_possible_types_[OpCode][j];
                auto inst = Inst::New<SelectInst>(&allocator_, OpCode);
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
std::vector<Inst *> &InstGenerator::GenerateOperationsImm<SelectImmInst>(Opcode OpCode)
{
    for (size_t i = 0; i < opcode_x_possible_types_[OpCode].size(); ++i) {
        auto cmp_type = opcode_x_possible_types_[OpCode][i];
        for (int CCInt = ConditionCode::CC_FIRST; CCInt != ConditionCode::CC_LAST; CCInt++) {
            auto cc = static_cast<ConditionCode>(CCInt);
            if (cmp_type == DataType::REFERENCE && cc != ConditionCode::CC_NE && cc != ConditionCode::CC_EQ) {
                continue;
            }
            for (size_t j = 0; j < opcode_x_possible_types_[OpCode].size(); ++j) {
                auto dst_type = opcode_x_possible_types_[OpCode][j];
                auto inst = Inst::New<SelectImmInst>(&allocator_, OpCode);
                inst->SetOperandsType(cmp_type);
                inst->SetType(dst_type);
                inst->SetCc(cc);
                inst->SetImm(cmp_type == DataType::REFERENCE ? 0 : 1);
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
std::vector<Inst *> &InstGenerator::GenerateOperations<SpillFillInst>(Opcode OpCode)
{
    auto inst = Inst::New<SpillFillInst>(&allocator_, OpCode);
    inst->SetType(opcode_x_possible_types_[OpCode][0]);
    inst->AddSpill(0, 2, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, OpCode);
    inst->SetType(opcode_x_possible_types_[OpCode][0]);
    inst->AddFill(0, 2, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, OpCode);
    inst->SetType(opcode_x_possible_types_[OpCode][0]);
    inst->AddMove(0, 2, DataType::UINT64);
    insts_.push_back(inst);

    inst = Inst::New<SpillFillInst>(&allocator_, OpCode);
    inst->SetType(opcode_x_possible_types_[OpCode][0]);
    inst->AddMemCopy(0, 2, DataType::UINT64);
    insts_.push_back(inst);
    return insts_;
}

template <>
std::vector<Inst *> &InstGenerator::GenerateOperations<MonitorInst>(Opcode OpCode)
{
    auto inst = Inst::New<MonitorInst>(&allocator_, OpCode);
    inst->SetType(opcode_x_possible_types_[OpCode][0]);
    inst->SetEntry();
    insts_.push_back(inst);

    inst = Inst::New<MonitorInst>(&allocator_, OpCode);
    inst->SetType(opcode_x_possible_types_[OpCode][0]);
    inst->SetExit();
    insts_.push_back(inst);

    return insts_;
}

#include "generate_operations_intrinsic_inst.inl"

std::vector<Inst *> &InstGenerator::Generate(Opcode OpCode)
{
    insts_.clear();
    switch (OpCode) {
        case Opcode::Neg:
        case Opcode::Abs:
        case Opcode::Not:
            return GenerateOperations<UnaryOperation>(OpCode);
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
            return GenerateOperations<BinaryOperation>(OpCode);
        case Opcode::Compare:
            return GenerateOperations<CompareInst>(OpCode);
        case Opcode::Constant:
            return GenerateOperations<ConstantInst>(OpCode);
        case Opcode::NewObject:
            return GenerateOperations<NewObjectInst>(OpCode);
        case Opcode::If:
            return GenerateOperations<IfInst>(OpCode);
        case Opcode::IfImm:
            return GenerateOperationsImm<IfImmInst>(OpCode);
        case Opcode::IsInstance:
            return GenerateOperations<IsInstanceInst>(OpCode);
        case Opcode::LenArray:
            return GenerateOperations<LengthMethodInst>(OpCode);
        case Opcode::Return:
        case Opcode::ReturnInlined:
            return GenerateOperations<FixedInputsInst1>(OpCode);
        case Opcode::NewArray:
            return GenerateOperations<NewArrayInst>(OpCode);
        case Opcode::Cmp:
            return GenerateOperations<CmpInst>(OpCode);
        case Opcode::CheckCast:
            return GenerateOperations<CheckCastInst>(OpCode);
        case Opcode::NullCheck:
        case Opcode::ZeroCheck:
        case Opcode::NegativeCheck:
            return GenerateOperations<FixedInputsInst2>(OpCode);
        case Opcode::Throw:
            return GenerateOperations<ThrowInst>(OpCode);
        case Opcode::BoundsCheck:
        case Opcode::MAdd:
        case Opcode::MSub:
            return GenerateOperations<FixedInputsInst3>(OpCode);
        case Opcode::Parameter:
            return GenerateOperations<ParameterInst>(OpCode);
        case Opcode::LoadArray:
            return GenerateOperations<LoadInst>(OpCode);
        case Opcode::StoreArray:
            return GenerateOperations<StoreInst>(OpCode);
        case Opcode::LoadArrayI:
            return GenerateOperationsImm<LoadInstI>(OpCode);
        case Opcode::StoreArrayI:
            return GenerateOperationsImm<StoreInstI>(OpCode);
        case Opcode::NullPtr:
        case Opcode::ReturnVoid:
            return GenerateOperations<FixedInputsInst0>(OpCode);
        case Opcode::SaveState:
        case Opcode::SafePoint:
            return GenerateOperations<SaveStateInst>(OpCode);
        case Opcode::Phi:
            return GenerateOperations<PhiInst>(OpCode);
        case Opcode::CallStatic:
        case Opcode::CallVirtual:
            return GenerateOperations<CallInst>(OpCode);
        case Opcode::Monitor:
            return GenerateOperations<MonitorInst>(OpCode);
        case Opcode::AddI:
        case Opcode::SubI:
        case Opcode::ShlI:
        case Opcode::ShrI:
        case Opcode::AShrI:
        case Opcode::AndI:
        case Opcode::OrI:
        case Opcode::XorI:
            return GenerateOperationsImm<BinaryImmOperation>(OpCode);
        case Opcode::AndSR:
        case Opcode::SubSR:
        case Opcode::AddSR:
        case Opcode::OrSR:
        case Opcode::XorSR:
        case Opcode::AndNotSR:
        case Opcode::OrNotSR:
        case Opcode::XorNotSR:
            return GenerateOperationsShiftedRegister<BinaryShiftedRegisterOperation>(OpCode);
        case Opcode::NegSR:
            return GenerateOperationsShiftedRegister<UnaryShiftedRegisterOperation>(OpCode);
        case Opcode::SpillFill:
            return GenerateOperations<SpillFillInst>(OpCode);
        case Opcode::LoadObject:
            return GenerateOperations<LoadObjectInst>(OpCode);
        case Opcode::StoreObject:
            return GenerateOperations<StoreObjectInst>(OpCode);
        case Opcode::LoadStatic:
            return GenerateOperations<LoadStaticInst>(OpCode);
        case Opcode::StoreStatic:
            return GenerateOperations<StoreStaticInst>(OpCode);
        case Opcode::LoadString:
        case Opcode::LoadType:
            return GenerateOperations<LoadFromPool>(OpCode);
        case Opcode::BoundsCheckI:
            return GenerateOperationsImm<BoundsCheckInstI>(OpCode);
        case Opcode::ReturnI:
            return GenerateOperationsImm<ReturnInstI>(OpCode);
        case Opcode::Intrinsic:
            return GenerateOperations<IntrinsicInst>(OpCode);
        case Opcode::Select:
            return GenerateOperations<SelectInst>(OpCode);
        case Opcode::SelectImm:
            return GenerateOperationsImm<SelectImmInst>(OpCode);
        case Opcode::LoadArrayPair:
            return GenerateOperations<LoadArrayPairInst>(OpCode);
        case Opcode::LoadArrayPairI:
            return GenerateOperationsImm<LoadArrayPairInstI>(OpCode);
        case Opcode::StoreArrayPair:
            return GenerateOperations<StoreArrayPairInst>(OpCode);
        case Opcode::StoreArrayPairI:
            return GenerateOperationsImm<StoreArrayPairInstI>(OpCode);
        case Opcode::Cast:
            return GenerateOperations<CastInst>(OpCode);
        case Opcode::Builtin:
            ASSERT_DO(0, std::cerr << "Unexpected Opcode Builtin\n");
            return insts_;
        default:
            ASSERT_DO(0, std::cerr << GetOpcodeString(OpCode) << "\n");
            return insts_;
    }
}

constexpr std::array<const char *, 15> LABELS = {"NO_TYPE", "REF",     "BOOL",    "UINT8", "INT8",
                                                 "UINT16",  "INT16",   "UINT32",  "INT32", "UINT64",
                                                 "INT64",   "FLOAT32", "FLOAT64", "ANY",   "VOID"};

void StatisticGenerator::GenerateHTMLPage(std::string file_name)
{
    std::ofstream HTMLPage;
    HTMLPage.open(file_name);
    HTMLPage << "<!DOCTYPE html>\n"
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
             << "\t\t<tr><th>Positive tests</th><td>" << positive_inst_number << "</td></tr>\n"
             << "\t\t<tr><th>All generated tests</th><td>" << all_inst_number_ << "</td></tr>\n"
             << "\t\t<tr><th></th><td>" << positive_inst_number * 100.0 / all_inst_number_ << "%</td></tr>\n"
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
        HTMLPage << "<th style=\"width:90px\">" << label << "</th>";
    }
    HTMLPage << "<th>%</th>";
    HTMLPage << "<tr>\n";
    for (auto i = 0; i != static_cast<int>(Opcode::NUM_OPCODES); ++i) {
        auto opc = static_cast<Opcode>(i);
        if (opc == Opcode::NOP || opc == Opcode::Intrinsic || opc == Opcode::LoadPairPart) {
            continue;
        }
        HTMLPage << "\t\t<tr>";
        HTMLPage << "<th>" << GetOpcodeString(opc) << "</th>";
        if (statistic_.first.find(opc) != statistic_.first.end()) {
            auto item = statistic_.first[opc];
            int positiv_count = 0;
            int negativ_count = 0;
            for (auto j = item.begin(); j != item.end(); ++j) {
                std::string flag = "";
                std::string color = "";
                switch ((*j).second) {
                    case 0:
                        flag = "-";
                        color = "bgcolor=\"#fd0000\"";
                        negativ_count++;
                        break;
                    case 1:
                        flag = "+";
                        color = "bgcolor=\"#00fd00\"";
                        positiv_count++;
                        break;
                    default:
                        break;
                }
                HTMLPage << "<td align=\"center\" " << color << ">" << flag << "</td>";
            }
            if (positiv_count + negativ_count != 0) {
                HTMLPage << "<td align=\"right\">" << positiv_count * 100.0 / (positiv_count + negativ_count)
                         << "</td>";
            }
        } else {
            for (auto j = tmplt_.begin(); j != tmplt_.end(); ++j) {
                HTMLPage << "<td align=\"center\" bgcolor=\"#808080\"></td>";
            }
            HTMLPage << "<td align=\"right\">0</td>";
        }
        HTMLPage << "</tr>\n";
    }
    HTMLPage << "\t</table>\n";

    HTMLPage << "\t<h3>Intrinsics</h3>\n";
    HTMLPage << "\t<table>\n";
    HTMLPage << "\t\t<tr><th>IntrinsicId</th><th>Status</th></tr>";
    for (auto i = 0; i != static_cast<int>(RuntimeInterface::IntrinsicId::COUNT); ++i) {
        auto IntrinsicId = static_cast<RuntimeInterface::IntrinsicId>(i);
        auto IntrinsicName = GetIntrinsicName(IntrinsicId);
        if (IntrinsicName == "") {
            continue;
        }
        HTMLPage << "<tr><th>" << IntrinsicName << "</th>";
        if (statistic_.second.find(IntrinsicId) != statistic_.second.end()) {
            std::string flag = "";
            std::string color = "";
            if (statistic_.second[IntrinsicId]) {
                flag = "+";
                color = "bgcolor=\"#00fd00\"";
            } else {
                flag = "-";
                color = "bgcolor=\"#fd0000\"";
            }
            HTMLPage << "<td align=\"center\" " << color << ">" << flag << "</td></tr>";
        } else {
            HTMLPage << "<td align=\"center\" bgcolor=\"#808080\"></td></tr>";
        }
        HTMLPage << "\n";
    }
    HTMLPage << "</table>\n";
    HTMLPage << "</body>\n"
             << "</html>";
    HTMLPage.close();
}
}  // namespace panda::compiler
