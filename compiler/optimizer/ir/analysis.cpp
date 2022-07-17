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

#include "analysis.h"

#include "optimizer/ir/basicblock.h"
namespace panda::compiler {

class BasicBlock;
bool FindOsrEntryRec(BasicBlock *dominate_bb, BasicBlock *current_bb, Marker mrk)
{
    if (dominate_bb == current_bb) {
        return false;
    }
    if (current_bb->SetMarker(mrk)) {
        return false;
    }
    if (current_bb->IsOsrEntry()) {
        return true;
    }

    for (auto pred : current_bb->GetPredsBlocks()) {
        if (FindOsrEntryRec(dominate_bb, pred, mrk)) {
            return true;
        }
    }
    return false;
}

bool HasOsrEntryBetween(Inst *dominate_inst, Inst *inst)
{
    auto bb = inst->GetBasicBlock();
    auto graph = bb->GetGraph();
    if (!graph->IsOsrMode()) {
        return false;
    }
    auto dominate_bb = dominate_inst->GetBasicBlock();

    auto mrk = graph->NewMarker();

    auto has_osr_entry = FindOsrEntryRec(dominate_bb, bb, mrk);

    graph->EraseMarker(mrk);
    return has_osr_entry;
}

Inst *InstStoredValue(Inst *inst, Inst **second_value)
{
    ASSERT_PRINT(inst->IsStore(), "Attempt to take a stored value on non-store instruction");
    Inst *val = nullptr;
    *second_value = nullptr;
    switch (inst->GetOpcode()) {
        case Opcode::StoreArray:
        case Opcode::StoreObject:
        case Opcode::StoreStatic:
        case Opcode::StoreArrayI:
        case Opcode::Store:
        case Opcode::StoreI:
            // Last input is a stored value
            val = inst->GetInput(inst->GetInputsCount() - 1).GetInst();
            break;
        case Opcode::UnresolvedStoreObject:
            val = inst->GetInput(1).GetInst();
            break;
        case Opcode::UnresolvedStoreStatic:
            val = inst->GetInput(0).GetInst();
            break;
        case Opcode::StoreArrayPair:
        case Opcode::StoreArrayPairI: {
            val = inst->GetInput(inst->GetInputsCount() - 2U).GetInst();
            auto second_inst = inst->GetInput(inst->GetInputsCount() - 1U).GetInst();
            *second_value = inst->GetDataFlowInput(second_inst);
            break;
        }
        // Unhandled store instructions has been met
        default:
            UNREACHABLE();
    }
    return inst->GetDataFlowInput(val);
}

Inst *InstStoredValue(Inst *inst)
{
    Inst *second_value = nullptr;
    Inst *val = InstStoredValue(inst, &second_value);
    ASSERT(second_value == nullptr);
    return val;
}

bool IsSuitableForImplicitNullCheck(const Inst *inst)
{
    auto is_compressed_enabled = inst->GetBasicBlock()->GetGraph()->GetRuntime()->IsCompressedStringsEnabled();
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
            return inst->CastToLoadArray()->IsArray() || !is_compressed_enabled;
        case Opcode::LoadArrayI:
            return inst->CastToLoadArrayI()->IsArray() || !is_compressed_enabled;
        case Opcode::LoadObject:
        // case Opcode::UnresolvedLoadObject:
        case Opcode::StoreObject:
        // case Opcode::UnresolvedStoreObject:
        case Opcode::LenArray:
        case Opcode::StoreArray:
        case Opcode::StoreArrayI:
        case Opcode::LoadArrayPair:
        case Opcode::StoreArrayPair:
        case Opcode::LoadArrayPairI:
        case Opcode::StoreArrayPairI:
            // These instructions access nullptr and produce a signal in place
            // Note that CallVirtual is not in the list
            return true;
        default:
            return false;
    }
}

bool IsInstNotNull(const Inst *inst)
{
    // Allocations cannot return null pointer
    if (inst->IsAllocation() || inst->IsNullCheck()) {
        return true;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    // The object is not null if the method is virtual and the object is first parameter.
    return !runtime->IsMethodStatic(graph->GetMethod()) && inst->GetOpcode() == Opcode::Parameter &&
           inst->CastToParameter()->GetArgNumber() == 0;
}

}  // namespace panda::compiler
