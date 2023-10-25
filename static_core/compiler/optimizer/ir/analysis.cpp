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
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "compiler_logger.h"
namespace panda::compiler {

class BasicBlock;

class IsOsrEntryBlock {
public:
    bool operator()(const BasicBlock *bb) const
    {
        return bb->IsOsrEntry();
    }
};

class IsTryBlock {
public:
    bool operator()(const BasicBlock *bb) const
    {
        return bb->IsTry();
    }
};

class IsSaveState {
public:
    bool operator()(const Inst *inst) const
    {
        return inst->IsSaveState() && inst->IsNotRemovable();
    }
};

template <typename T>
bool FindBlockBetween(BasicBlock *dominate_bb, BasicBlock *current_bb, Marker marker)
{
    if (dominate_bb == current_bb) {
        return false;
    }
    if (current_bb->SetMarker(marker)) {
        return false;
    }
    if (T()(current_bb)) {
        return true;
    }
    for (auto pred : current_bb->GetPredsBlocks()) {
        if (FindBlockBetween<T>(dominate_bb, pred, marker)) {
            return true;
        }
    }
    return false;
}

RuntimeInterface::ClassPtr GetClassPtrForObject(Inst *inst, size_t input_num)
{
    auto obj_inst = inst->GetDataFlowInput(input_num);
    if (obj_inst->GetOpcode() != Opcode::NewObject) {
        return nullptr;
    }
    auto init_class = obj_inst->GetInput(0).GetInst();
    if (init_class->GetOpcode() == Opcode::LoadAndInitClass) {
        return init_class->CastToLoadAndInitClass()->GetClass();
    }
    ASSERT(init_class->GetOpcode() == Opcode::LoadImmediate);
    return init_class->CastToLoadImmediate()->GetClass();
}

bool HasOsrEntryBetween(Inst *dominate_inst, Inst *inst)
{
    ASSERT(dominate_inst->IsDominate(inst));
    auto bb = inst->GetBasicBlock();
    auto graph = bb->GetGraph();
    if (!graph->IsOsrMode()) {
        return false;
    }
    MarkerHolder marker(graph);
    return FindBlockBetween<IsOsrEntryBlock>(dominate_inst->GetBasicBlock(), bb, marker.GetMarker());
}

bool HasOsrEntryBetween(BasicBlock *dominate_bb, BasicBlock *bb)
{
    ASSERT(dominate_bb->IsDominate(bb));
    auto graph = bb->GetGraph();
    if (!graph->IsOsrMode()) {
        return false;
    }
    MarkerHolder marker(graph);
    return FindBlockBetween<IsOsrEntryBlock>(dominate_bb, bb, marker.GetMarker());
}

bool HasTryBlockBetween(Inst *dominate_inst, Inst *inst)
{
    ASSERT(dominate_inst->IsDominate(inst));
    auto bb = inst->GetBasicBlock();
    MarkerHolder marker(bb->GetGraph());
    return FindBlockBetween<IsTryBlock>(dominate_inst->GetBasicBlock(), bb, marker.GetMarker());
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
        case Opcode::StoreObjectDynamic:
            // Last input is a stored value
            val = inst->GetInput(inst->GetInputsCount() - 1).GetInst();
            break;
        case Opcode::StoreResolvedObjectField:
        case Opcode::StoreResolvedObjectFieldStatic:
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
        case Opcode::FillConstArray: {
            return nullptr;
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

SaveStateInst *CopySaveState(Graph *graph, SaveStateInst *inst)
{
    auto copy = static_cast<SaveStateInst *>(inst->Clone(graph));
    ASSERT(copy->GetCallerInst() == inst->GetCallerInst());
    for (size_t input_idx = 0; input_idx < inst->GetInputsCount(); input_idx++) {
        copy->AppendInput(inst->GetInput(input_idx));
        copy->SetVirtualRegister(input_idx, inst->GetVirtualRegister(input_idx));
    }
    copy->SetLinearNumber(inst->GetLinearNumber());
    return copy;
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
        case Opcode::StoreObject:
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

static bool FindObjectInSaveState(Inst *object, Inst *ss)
{
    if (!object->IsMovableObject()) {
        return true;
    }
    while (ss != nullptr && object->IsDominate(ss)) {
        auto it = std::find_if(ss->GetInputs().begin(), ss->GetInputs().end(),
                               [object, ss](Input input) { return ss->GetDataFlowInput(input.GetInst()) == object; });
        if (it != ss->GetInputs().end()) {
            return true;
        }
        auto caller = static_cast<SaveStateInst *>(ss)->GetCallerInst();
        if (caller == nullptr) {
            break;
        }
        ss = caller->GetSaveState();
    }
    return false;
}

// Returns true if GC can be triggered at this point
static bool IsSaveStateForGc(Inst *inst)
{
    if (inst->GetOpcode() == Opcode::SafePoint) {
        return true;
    }
    if (inst->GetOpcode() == Opcode::SaveState) {
        for (auto &user : inst->GetUsers()) {
            if (user.GetInst()->IsRuntimeCall()) {
                return true;
            }
        }
    }
    return false;
}

// Checks if object is correctly used in SaveStates between it and user
bool CheckObjectRec(Inst *object, const Inst *user, const BasicBlock *block, Inst *start_from, Marker visited,
                    Inst **failed_ss)
{
    if (start_from != nullptr) {
        auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, start_from);
        for (; it != block->AllInstsSafeReverse().end(); ++it) {
            auto inst = *it;
            if (inst == nullptr) {
                break;
            }
            if (inst->SetMarker(visited) || inst == object || inst == user) {
                return true;
            }
            if (IsSaveStateForGc(inst) && !FindObjectInSaveState(object, inst)) {
                if (failed_ss != nullptr) {
                    *failed_ss = inst;
                }
                return false;
            }
        }
    }
    for (auto pred : block->GetPredsBlocks()) {
        // Catch-begin block has edge from try-end block, and all try-blocks should be visited from this edge.
        // `object` can be placed inside try-block - after try-begin, so that visiting try-begin is wrong
        if (block->IsCatchBegin() && pred->IsTryBegin()) {
            continue;
        }
        if (!CheckObjectRec(object, user, pred, pred->GetLastInst(), visited, failed_ss)) {
            return false;
        }
    }
    return true;
}

// Checks if input edges of phi_block come from different branches of dominating if_imm instruction
// Returns true if the first input is in true branch, false if it is in false branch, and std::nullopt
// if branches intersect
std::optional<bool> IsIfInverted(BasicBlock *phi_block, IfImmInst *if_imm)
{
    auto if_block = if_imm->GetBasicBlock();
    ASSERT(if_block == phi_block->GetDominator());
    ASSERT(phi_block->GetPredsBlocks().size() == MAX_SUCCS_NUM);
    auto true_bb = if_imm->GetEdgeIfInputTrue();
    auto false_bb = if_imm->GetEdgeIfInputFalse();
    auto pred0 = phi_block->GetPredecessor(0);
    auto pred1 = phi_block->GetPredecessor(1);

    // Triangle case: phi block is the first in true branch
    if (true_bb == phi_block && false_bb->GetPredsBlocks().size() == 1) {
        return pred0 != if_block;
    }
    // Triangle case: phi block is the first in false branch
    if (false_bb == phi_block && true_bb->GetPredsBlocks().size() == 1) {
        return pred0 == if_block;
    }
    // If true_bb has more than one predecessor, there can be a path from false_bb
    // to true_bb avoiding if_imm
    if (true_bb->GetPredsBlocks().size() > 1 || false_bb->GetPredsBlocks().size() > 1) {
        return std::nullopt;
    }
    // Every path through first input edge to phi_block comes from true branch
    // Every path through second input edge to phi_block comes from false branch
    if (true_bb->IsDominate(pred0) && false_bb->IsDominate(pred1)) {
        return false;
    }
    // Every path through first input edge to phi_block comes from false branch
    // Every path through second input edge to phi_block comes from true branch
    if (false_bb->IsDominate(pred0) && true_bb->IsDominate(pred1)) {
        return true;
    }
    // True and false branches intersect
    return std::nullopt;
}
ArenaVector<Inst *> *SaveStateBridgesBuilder::SearchMissingObjInSaveStates(Graph *graph, Inst *source, Inst *target,
                                                                           Inst *stop_search, BasicBlock *target_block)
{
    ASSERT(graph != nullptr);
    ASSERT(source != nullptr);
    ASSERT(target_block != nullptr);
    ASSERT(source->IsMovableObject());

    if (bridges_ == nullptr) {
        auto adapter = graph->GetLocalAllocator();
        bridges_ = adapter->New<ArenaVector<Inst *>>(adapter->Adapter());
    } else {
        bridges_->clear();
    }
    auto visited = graph->NewMarker();
    SearchSSOnWay(target_block, target, source, visited, bridges_, stop_search);
    graph->EraseMarker(visited);
    return bridges_;
}

void SaveStateBridgesBuilder::SearchSSOnWay(BasicBlock *block, Inst *start_from, Inst *source_inst, Marker visited,
                                            ArenaVector<Inst *> *bridges, Inst *stop_search)
{
    ASSERT(block != nullptr);
    ASSERT(source_inst != nullptr);
    ASSERT(bridges != nullptr);

    if (start_from != nullptr) {
        auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, start_from);
        for (; it != block->AllInstsSafeReverse().end(); ++it) {
            auto inst = *it;
            if (inst == nullptr) {
                break;
            }
            COMPILER_LOG(DEBUG, BRIDGES_SS) << " See inst" << *inst;

            if (inst->SetMarker(visited)) {
                return;
            }
            if (IsSaveStateForGc(inst)) {
                COMPILER_LOG(DEBUG, BRIDGES_SS) << "\tSearch in SS";
                SearchInSaveStateAndFillBridgeVector(inst, source_inst, bridges);
            }
            // When "stop_search" is nullptr second clause never causes early exit here
            if (inst == source_inst || inst == stop_search) {
                return;
            }
        }
    }
    for (auto pred : block->GetPredsBlocks()) {
        SearchSSOnWay(pred, pred->GetLastInst(), source_inst, visited, bridges, stop_search);
    }
}

void SaveStateBridgesBuilder::SearchInSaveStateAndFillBridgeVector(Inst *inst, Inst *searched_inst,
                                                                   ArenaVector<Inst *> *bridges)
{
    ASSERT(inst != nullptr);
    ASSERT(searched_inst != nullptr);
    ASSERT(bridges != nullptr);
    auto user = std::find_if(inst->GetInputs().begin(), inst->GetInputs().end(), [searched_inst, inst](Input input) {
        return inst->GetDataFlowInput(input.GetInst()) == searched_inst;
    });

    if (user == inst->GetInputs().end()) {
        COMPILER_LOG(DEBUG, BRIDGES_SS) << "\tNot found";
        bridges->push_back(inst);
    }
}

void SaveStateBridgesBuilder::FixUsagePhiInBB(BasicBlock *block, Inst *inst)
{
    ASSERT(block != nullptr);
    ASSERT(inst != nullptr);
    if (inst->IsMovableObject()) {
        for (auto &user : inst->GetUsers()) {
            auto target_inst = user.GetInst();
            COMPILER_LOG(DEBUG, BRIDGES_SS) << " Check usage: Try to do SSB for inst: " << inst->GetId() << "\t"
                                            << " For target inst: " << target_inst->GetId() << "\n";
            // If inst usage in other BB than in all case object must exist until the end of the BB
            if (target_inst->IsPhi() || target_inst->GetBasicBlock() != block) {
                target_inst = block->GetLastInst();
            }
            SearchAndCreateMissingObjInSaveState(block->GetGraph(), inst, target_inst, block->GetFirstInst());
        }
    }
}

void SaveStateBridgesBuilder::FixUsageInstInOtherBB(BasicBlock *block, Inst *inst)
{
    ASSERT(block != nullptr);
    ASSERT(inst != nullptr);
    if (inst->IsMovableObject()) {
        for (auto &user : inst->GetUsers()) {
            auto target_inst = user.GetInst();
            // This way "in same block" checked when we saw inputs of instructions
            if (target_inst->GetBasicBlock() == block) {
                continue;
            }
            COMPILER_LOG(DEBUG, BRIDGES_SS) << " Check inputs: Try to do SSB for real source inst: " << *inst << "\n"
                                            << "  For target inst: " << *target_inst << "\n";
            // If inst usage in other BB than in all case object must must exist until the end of the BB
            target_inst = block->GetLastInst();
            SearchAndCreateMissingObjInSaveState(block->GetGraph(), inst, target_inst, block->GetFirstInst());
        }
    }
}

void SaveStateBridgesBuilder::DeleteUnrealObjInSaveState(Inst *ss)
{
    ASSERT(ss != nullptr);
    size_t index_input = 0;
    for (auto &input : ss->GetInputs()) {
        // If the user of SS before inst
        auto input_inst = input.GetInst();
        if (ss->GetBasicBlock() == input_inst->GetBasicBlock() && ss->IsDominate(input_inst)) {
            ss->RemoveInput(index_input);
            COMPILER_LOG(DEBUG, BRIDGES_SS) << " Fixed incorrect user in ss: " << ss->GetId() << "  "
                                            << " deleted input: " << input_inst->GetId() << "\n";
        }
        index_input++;
    }
}

void SaveStateBridgesBuilder::FixSaveStatesInBB(BasicBlock *block)
{
    ASSERT(block != nullptr);
    bool block_in_loop = !(block->GetLoop()->IsRoot());
    // Check usage ".ref" PHI inst
    for (auto phi : block->PhiInsts()) {
        FixUsagePhiInBB(block, phi);
    }
    // Check all insts
    for (auto inst : block->Insts()) {
        if (IsSaveStateForGc(inst)) {
            DeleteUnrealObjInSaveState(inst);
        }
        // Check reference inputs of instructions
        for (auto &input : inst->GetInputs()) {
            // We record the original object in SaveState without checks
            auto real_source_inst = inst->GetDataFlowInput(input.GetInst());
            if (!real_source_inst->IsMovableObject()) {
                continue;
            }
            // In case, when usege of object in loop and defenition is not in loop or usage's loop inside defenition's
            // loop, we should check SaveStates till the end of BasicBlock
            if (block_in_loop && (block->GetLoop()->IsInside(real_source_inst->GetBasicBlock()->GetLoop()))) {
                COMPILER_LOG(DEBUG, BRIDGES_SS)
                    << " Check inputs: Try to do SSB for real source inst: " << *real_source_inst << "\n"
                    << "  Block in loop:  " << block->GetLoop() << " So target is end of BB:" << *(block->GetLastInst())
                    << "\n";
                SearchAndCreateMissingObjInSaveState(block->GetGraph(), real_source_inst, block->GetLastInst(),
                                                     block->GetFirstInst());
            } else {
                COMPILER_LOG(DEBUG, BRIDGES_SS)
                    << " Check inputs: Try to do SSB for real source inst: " << *real_source_inst << "\n"
                    << "  For target inst: " << *inst << "\n";
                SearchAndCreateMissingObjInSaveState(block->GetGraph(), real_source_inst, inst, block->GetFirstInst());
            }
        }
        // Check usage reference instruction
        FixUsageInstInOtherBB(block, inst);
    }
}

bool SaveStateBridgesBuilder::IsSaveStateForGc(Inst *inst)
{
    return inst->GetOpcode() == Opcode::SafePoint || inst->GetOpcode() == Opcode::SaveState;
}

void SaveStateBridgesBuilder::CreateBridgeInSS(Inst *source, ArenaVector<Inst *> *bridges)
{
    ASSERT(bridges != nullptr);
    ASSERT(source != nullptr);
    ASSERT(source->IsMovableObject());

    for (Inst *ss : *bridges) {
        static_cast<SaveStateInst *>(ss)->AppendBridge(source);
    }
}

void SaveStateBridgesBuilder::SearchAndCreateMissingObjInSaveState(Graph *graph, Inst *source, Inst *target,
                                                                   Inst *stop_search_inst, BasicBlock *target_block)
{
    ASSERT(graph != nullptr);
    ASSERT(source != nullptr);
    ASSERT(source->IsMovableObject());

    if (graph->IsBytecodeOptimizer()) {
        return;  // SaveState bridges useless when bytecode optimizer enabled.
    }

    if (target_block == nullptr) {
        ASSERT(target != nullptr);
        target_block = target->GetBasicBlock();
    } else {
        ASSERT(target == target_block->GetLastInst());
    }
    auto bridges = SearchMissingObjInSaveStates(graph, source, target, stop_search_inst, target_block);
    if (!bridges->empty()) {
        CreateBridgeInSS(source, bridges);
        COMPILER_LOG(DEBUG, BRIDGES_SS) << " Created bridge(s)";
    }
}

void SaveStateBridgesBuilder::FixInstUsageInSS(Graph *graph, Inst *inst)
{
    if (!inst->IsMovableObject()) {
        return;
    }
    for (auto &user : inst->GetUsers()) {
        auto target_inst = user.GetInst();
        COMPILER_LOG(DEBUG, BRIDGES_SS) << " Check usage: Try to do SSB for real source inst: " << *inst << "\n"
                                        << "  For target inst: " << *target_inst << "\n";
        if (target_inst->IsPhi() && !(graph->IsAnalysisValid<DominatorsTree>() && inst->IsDominate(target_inst))) {
            for (auto pred_block : target_inst->GetBasicBlock()->GetPredsBlocks()) {
                if (target_inst->CastToPhi()->GetPhiInput(pred_block) == inst) {
                    SearchAndCreateMissingObjInSaveState(graph, inst, pred_block->GetLastInst(), nullptr, pred_block);
                }
            }
        } else {
            SearchAndCreateMissingObjInSaveState(graph, inst, target_inst);
        }
    }
}

// Check instructions don't have their own vregs and thus are not added in SaveStates,
// but newly added Phi instructions with check inputs should be added
void SaveStateBridgesBuilder::FixPhisWithCheckInputs(BasicBlock *block)
{
    if (block == nullptr) {
        return;
    }
    auto graph = block->GetGraph();
    for (auto phi : block->PhiInsts()) {
        if (!phi->IsMovableObject()) {
            continue;
        }
        for (auto &input : phi->GetInputs()) {
            if (input.GetInst()->IsCheck()) {
                FixInstUsageInSS(graph, phi);
                break;
            }
        }
    }
}

void SaveStateBridgesBuilder::DumpBridges(std::ostream &out, Inst *source, ArenaVector<Inst *> *bridges)
{
    ASSERT(source != nullptr);
    ASSERT(bridges != nullptr);
    out << "Inst id " << source->GetId() << " with type ";
    source->DumpOpcode(&out);
    out << "need bridge in SS id: ";
    for (auto ss : *bridges) {
        out << ss->GetId() << " ";
    }
    out << '\n';
}

bool StoreValueCanBeObject(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::CastValueToAnyType: {
            auto type = AnyBaseTypeToDataType(inst->CastToCastValueToAnyType()->GetAnyType());
            return (type == DataType::ANY || type == DataType::REFERENCE);
        }
        case Opcode::Constant:
            return false;
        default:
            return true;
    }
}

bool IsConditionEqual(const Inst *inst0, const Inst *inst1, bool inverted)
{
    if (inst0->GetOpcode() != inst1->GetOpcode()) {
        return false;
    }
    if (inst0->GetOpcode() != Opcode::IfImm) {
        // investigate why Opcode::If cannot be lowered to Opcode::IfImm and support it if needed
        return false;
    }
    auto if_imm0 = inst0->CastToIfImm();
    auto if_imm1 = inst1->CastToIfImm();
    auto opcode = if_imm0->GetInput(0).GetInst()->GetOpcode();
    if (opcode != if_imm1->GetInput(0).GetInst()->GetOpcode()) {
        return false;
    }
    if (if_imm0->GetImm() != 0 && if_imm0->GetImm() != 1) {
        return false;
    }
    if (if_imm1->GetImm() != 0 && if_imm1->GetImm() != 1) {
        return false;
    }
    if (if_imm0->GetImm() != if_imm1->GetImm()) {
        inverted = !inverted;
    }
    if (opcode != Opcode::Compare) {
        if (if_imm0->GetInput(0).GetInst() != if_imm1->GetInput(0).GetInst()) {
            return false;
        }
        auto cc = inverted ? GetInverseConditionCode(if_imm0->GetCc()) : if_imm0->GetCc();
        return cc == if_imm1->GetCc();
    }
    auto cmp0 = if_imm0->GetInput(0).GetInst()->CastToCompare();
    auto cmp1 = if_imm1->GetInput(0).GetInst()->CastToCompare();
    if (cmp0->GetInput(0).GetInst() == cmp1->GetInput(0).GetInst() &&
        cmp0->GetInput(1).GetInst() == cmp1->GetInput(1).GetInst()) {
        if (GetInverseConditionCode(if_imm0->GetCc()) == if_imm1->GetCc()) {
            inverted = !inverted;
        } else if (if_imm0->GetCc() != if_imm1->GetCc()) {
            return false;
        }
        auto cc = inverted ? GetInverseConditionCode(cmp0->GetCc()) : cmp0->GetCc();
        return cc == cmp1->GetCc();
    }
    return false;
}

void CleanupGraphSaveStateOSR(Graph *graph)
{
    ASSERT(graph != nullptr);
    ASSERT(graph->IsOsrMode());
    graph->InvalidateAnalysis<LoopAnalyzer>();
    graph->RunPass<LoopAnalyzer>();
    for (auto block : graph->GetBlocksRPO()) {
        if (block->IsOsrEntry() && !block->IsLoopHeader()) {
            auto first_inst = block->GetFirstInst();
            if (first_inst == nullptr) {
                continue;
            }
            if (first_inst->GetOpcode() == Opcode::SaveStateOsr) {
                block->RemoveInst(first_inst);
                block->SetOsrEntry(false);
            }
        }
    }
}

template <typename T>
bool FindInstBetween(Inst *dom_inst, BasicBlock *current_bb, Marker marker)
{
    if (current_bb->SetMarker(marker)) {
        return false;
    }
    bool is_same_block = dom_inst->GetBasicBlock() == current_bb;
    auto curr_inst = current_bb->GetLastInst();
    Inst *finish = is_same_block ? dom_inst : nullptr;
    while (curr_inst != finish) {
        if (T()(curr_inst)) {
            return true;
        }
        curr_inst = curr_inst->GetPrev();
    }
    if (is_same_block) {
        return false;
    }
    for (auto pred : current_bb->GetPredsBlocks()) {
        if (FindInstBetween<T>(dom_inst, pred, marker)) {
            return true;
        }
    }
    return false;
}

bool HasSaveStateBetween(Inst *dom_inst, Inst *inst)
{
    ASSERT(dom_inst->IsDominate(inst));
    if (dom_inst == inst) {
        return false;
    }
    auto bb = inst->GetBasicBlock();
    bool is_same_block = dom_inst->GetBasicBlock() == bb;
    auto curr_inst = inst->GetPrev();
    Inst *finish = is_same_block ? dom_inst : nullptr;
    while (curr_inst != finish) {
        if (curr_inst->IsSaveState()) {
            return true;
        }
        curr_inst = curr_inst->GetPrev();
    }
    if (is_same_block) {
        return false;
    }
    MarkerHolder marker(bb->GetGraph());
    for (auto pred : bb->GetPredsBlocks()) {
        if (FindInstBetween<IsSaveState>(dom_inst, pred, marker.GetMarker())) {
            return true;
        }
    }
    return false;
}

void InstAppender::Append(Inst *inst)
{
    if (prev_ == nullptr) {
        block_->AppendInst(inst);
    } else {
        block_->InsertAfter(inst, prev_);
    }
    prev_ = inst;
}

void InstAppender::Append(std::initializer_list<Inst *> instructions)
{
    for (auto *inst : instructions) {
        Append(inst);
    }
}

}  // namespace panda::compiler
