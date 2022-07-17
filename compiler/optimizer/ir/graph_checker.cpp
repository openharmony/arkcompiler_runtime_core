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

#include "compiler_options.h"
#include "graph_cloner.h"
#include "compiler_options.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/optimizations/cleanup.h"
#include "inst_checker_gen.h"
#include "graph_checker.h"

namespace panda::compiler {

GraphChecker::GraphChecker(Graph *graph)
{
    PreCloneChecks(graph);
    graph_ = GraphCloner(graph, GetAllocator(), GetLocalAllocator()).CloneGraph();
    GetGraph()->GetPassManager()->SetCheckMode(true);
}

void GraphChecker::PreCloneChecks(Graph *graph)
{
    UserInputCheck(graph);
}

void GraphChecker::UserInputCheck(Graph *graph)
{
    for (auto block : graph->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        for (auto inst : block->AllInsts()) {
            auto u = inst->GetFirstUser();
            ASSERT(u == nullptr || u->GetPrev() == nullptr);
            while (u != nullptr) {
                ASSERT(u->GetNext() == nullptr || u->GetNext()->GetPrev() == u);
                u = u->GetNext();
            }
            for (auto &user : inst->GetUsers()) {
                [[maybe_unused]] auto user_inst = user.GetInst();
                ASSERT(user_inst->GetBasicBlock() != nullptr);
                ASSERT_DO(CheckInstHasInput(user_inst, inst), std::cerr << "Instruction is not an input to its user\n"
                                                                        << "input: " << *inst << std::endl
                                                                        << "user:  " << *user_inst << std::endl);
            }
            for (auto &input : inst->GetInputs()) {
                [[maybe_unused]] auto input_inst = input.GetInst();
                ASSERT(input_inst != nullptr && input_inst->GetBasicBlock() != nullptr);
                ASSERT_DO(CheckInstHasUser(input_inst, inst), std::cerr << "Instruction is not a user to its input:\n"
                                                                        << "user: " << *inst << std::endl
                                                                        << "input:  " << *input_inst << std::endl);
            }
            // Check `require_state` flag
            auto it = std::find_if(inst->GetInputs().begin(), inst->GetInputs().end(),
                                   [](Input input) { return input.GetInst()->IsSaveState(); });
            [[maybe_unused]] bool has_save_state = (it != inst->GetInputs().end());
            ASSERT_DO(inst->RequireState() == has_save_state,
                      std::cerr << "Incorrect 'require_state' flag in the inst: " << *inst);
            if (inst->RequireState()) {
                ASSERT(it->GetInst() == inst->GetSaveState());
            }
        }
    }
}

void GraphChecker::Check()
{
    if (!GetGraph()->IsDynamicMethod()) {
        InstChecker::Run(GetGraph());
    }

#ifndef NDEBUG
    if (GetGraph()->IsAnalysisValid<DominatorsTree>()) {
        CheckDomTree();
    } else {
        GetGraph()->RunPass<DominatorsTree>();
    }
    if (GetGraph()->IsAnalysisValid<LoopAnalyzer>()) {
        CheckLoopAnalysis();
    } else {
        GetGraph()->RunPass<LoopAnalyzer>();
    }
    CheckStartBlock();
    CheckEndBlock();
    size_t blocks_count = 0;
    size_t blocks_id = -1;
    for (auto block : GetGraph()->GetVectorBlocks()) {
        ++blocks_id;
        if (block == nullptr) {
            continue;
        }
        ASSERT_PRINT(block->GetGraph() == GetGraph(), "Block linked to incorrect graph");
        ASSERT_PRINT(block->GetId() == blocks_id, "Block ID must be equal to its ID in graph vector");
        CheckBlock(block);
        blocks_count++;
    }
    ASSERT_PRINT(blocks_count == GetGraph()->GetBlocksRPO().size(), "There is disconnected block");
    CheckLoops();
    // Visit graph to check instructions types
    CheckGraph();
    // Check that call.Inlined and Return.Inlined in correct order
    // and check that savestate has correct link to call.inlined.
    CheckCallReturnInlined();
    if (NeedCheckSaveState()) {
        // Check that objects in stack.
        CheckSaveStateInputs();
        // Check that between savestate and it's runtime call user have not reference insts.
        CheckSaveStatesWithRuntimeCallUsers();
    }

#endif  // !NDEBUG
}

#ifndef NDEBUG
bool GraphChecker::NeedCheckSaveState()
{
    return !GetGraph()->IsBytecodeOptimizer() && GetGraph()->GetParentGraph() == nullptr &&
           GetGraph()->IsInliningComplete() && !GetGraph()->IsSchedulerComplete();
}
#endif  // !NDEBUG

void GraphChecker::CheckBlock([[maybe_unused]] BasicBlock *block)
{
#ifndef NDEBUG
    CheckControlFlow(block);
    CheckDataFlow(block);
    for (auto phi_inst : block->PhiInsts()) {
        CheckPhiInputs(phi_inst);
    }
    if (!GetGraph()->IsLowLevelInstructionsEnabled() && !GetGraph()->IsDynamicMethod()) {
        CheckNoLowLevel(block);
    }
    if (!block->IsEndBlock() && !block->IsStartBlock()) {
        CheckBlockEdges(*block);
    }
    if (block->IsTryBegin()) {
        CheckTryBeginBlock(*block);
    }
    if (block->NeedsJump()) {
        CheckJump(*block);
    }
#endif  // !NDEBUG
}

void GraphChecker::CheckControlFlow(BasicBlock *block)
{
    auto num_succs = block->GetSuccsBlocks().size();
    ASSERT_PRINT(block->IsEndBlock() || block->IsTryBegin() || block->IsTryEnd() ||
                     (num_succs > 0 && num_succs <= MAX_SUCCS_NUM) ||
                     block->GetLastInst()->GetOpcode() == Opcode::Throw,
                 "Non-end block and non-try-begin block should have 1 or 2 successesors");

    for ([[maybe_unused]] auto pred : block->GetPredsBlocks()) {
        ASSERT_PRINT(CheckBlockHasSuccessor(pred, block), "Block is not a successor to its predecessor");
    }
    for ([[maybe_unused]] auto succ : block->GetSuccsBlocks()) {
        ASSERT_PRINT(CheckBlockHasPredecessor(succ, block), "Block is not a predecessor to its successor");
    }

    if (num_succs == MAX_SUCCS_NUM) {
        ASSERT_PRINT(block->GetSuccessor(0) != block->GetSuccessor(1), "Wrong CFG - block with two same successors");
    }

    for ([[maybe_unused]] auto phi : block->PhiInsts()) {
        ASSERT_DO(phi->GetInputsCount() == block->GetPredsBlocks().size(),
                  std::cerr << phi->GetInputsCount() << " " << block->GetPredsBlocks().size()
                            << "Incorrect phi's inputs count" << *phi);
    }
}

void GraphChecker::CheckDataFlow(BasicBlock *block)
{
    auto graph = block->GetGraph();
    for (auto inst : block->AllInsts()) {
        ASSERT_DO(inst->GetBasicBlock() == block,
                  std::cerr << "Instruction block's pointer isn't correct" << *inst << std::endl);
        if (block != graph->GetStartBlock()) {
            ASSERT_DO(inst->GetOpcode() != Opcode::Parameter,
                      std::cerr << "Not entry block can't contain Parameter instructions" << *inst << std::endl);
        }
        if (inst->GetPrev() == nullptr) {
            ASSERT_PRINT(*block->AllInsts().begin() == inst, "First block instruction isn't correct");
        }
        if (inst->GetNext() == nullptr) {
            ASSERT_PRINT(*block->AllInstsSafeReverse().begin() == inst, "Last block instruction isn't correct");
        }
        auto opc = inst->GetOpcode();
        // Inst with reference type must have no_cse and no_hoist flags.
        if (inst->GetType() == DataType::REFERENCE && !inst->IsClassInst() && opc != Opcode::GetInstanceClass) {
            ASSERT(inst->IsNotCseApplicable());
            ASSERT(inst->IsNotHoistable());
        }
        for ([[maybe_unused]] auto &user : inst->GetUsers()) {
            auto user_inst = user.GetInst();
            ASSERT_DO(CheckInstHasInput(user_inst, inst), std::cerr << "Instruction is not an input to its user\n"
                                                                    << "input: " << *inst << std::endl
                                                                    << "user:  " << *user_inst << std::endl);
            if (!user_inst->IsPhi() && !user_inst->IsCatchPhi()) {
                ASSERT_DO(inst->IsDominate(user_inst) ||
                              (GetGraph()->IsRegAllocApplied() &&
                               IsTryCatchDomination(inst->GetBasicBlock(), user_inst->GetBasicBlock())),
                          std::cerr << "Instruction doesn't dominate its user\n"
                                    << "input: " << *inst << std::endl
                                    << "user:  " << *user_inst << std::endl);
            }
            auto arch = graph->GetArch();
            if (DataType::Is32Bits(inst->GetType(), arch)) {
                if (!user_inst->HasType()) {
                    continue;
                }
                // Unsigned Load in AARCH64 zerod all high bits
#ifndef NDEBUG
                if (inst->IsLoad() && !DataType::IsTypeSigned(inst->GetType()) && arch == Arch::AARCH64 &&
                    graph->IsLowLevelInstructionsEnabled()) {
#else
                if (inst->IsLoad() && !DataType::IsTypeSigned(inst->GetType()) && arch == Arch::AARCH64) {
#endif  // !NDEBUG
                    continue;
                }
                [[maybe_unused]] auto user_input_type = user_inst->GetInputType(user.GetIndex());
                [[maybe_unused]] bool ref_to_ptr =
                    user_input_type == DataType::POINTER && inst->GetType() == DataType::REFERENCE;
                ASSERT_DO(DataType::Is32Bits(user_input_type, arch) || ref_to_ptr ||
                              (block->GetGraph()->IsDynamicMethod() && user_input_type == DataType::ANY),
                          std::cerr << "Undefined high-part of input instruction for its user\n"
                                    << "input: " << *inst << std::endl
                                    << "user:  " << *user_inst << std::endl);
            }
        }
        for ([[maybe_unused]] auto input : inst->GetInputs()) {
            ASSERT_DO(CheckInstHasUser(input.GetInst(), inst), std::cerr << "Instruction is not a user to its input:\n"
                                                                         << "input: " << *input.GetInst() << std::endl
                                                                         << "user:  " << *inst << std::endl);
        }
    }
}

void GraphChecker::CheckCallReturnInlined()
{
    ArenaStack<Inst *> inlined_calles(GetLocalAllocator()->Adapter());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
                ASSERT_PRINT(inst->NoDest(), "Inlined call should have NO_DST flag");
                inlined_calles.push(inst);
            } else if (inst->GetOpcode() == Opcode::ReturnInlined) {
                // TODO(Sergey Chernykh) fix checker
                if (block->GetLastInst()->GetOpcode() == Opcode::Throw ||
                    block->GetLastInst()->GetOpcode() == Opcode::Deoptimize) {
                    continue;
                }
                ASSERT(!inlined_calles.empty());
                inlined_calles.pop();
            }
        }
    }
    [[maybe_unused]] bool throw_exit = false;
    if (GetGraph()->HasEndBlock()) {
        for (auto block : GetGraph()->GetEndBlock()->GetPredsBlocks()) {
            if (block->IsTryEnd()) {
                continue;
            }
            if (block->GetLastInst()->GetOpcode() == Opcode::Throw ||
                block->GetLastInst()->GetOpcode() == Opcode::Deoptimize) {
                throw_exit = true;
                break;
            }
        }
    }
    ASSERT(inlined_calles.empty() || throw_exit);
#ifndef NDEBUG
    // avoid check after ir_builder in inline pass
    if (!GetGraph()->IsInliningComplete() || GetGraph()->GetParentGraph() != nullptr) {
        return;
    }
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (inst->IsSaveState()) {
                CheckSaveStateCaller(static_cast<SaveStateInst *>(inst));
            }
        }
    }
#endif
}

void GraphChecker::CheckSaveStateCaller(SaveStateInst *savestate)
{
    ASSERT(savestate != nullptr);
    auto block = savestate->GetBasicBlock();
    ArenaStack<Inst *> inlined_calles(GetLocalAllocator()->Adapter());
    auto caller = savestate->GetCallerInst();
    if (caller == nullptr) {
        return;
    }
    ASSERT(caller->GetBasicBlock() != nullptr);
    ASSERT(caller->GetBasicBlock()->GetGraph() == block->GetGraph());
    auto dom_block = block;
    bool skip = true;
    for (auto inst : dom_block->InstsSafeReverse()) {
        if (inst == savestate) {
            skip = false;
        }
        if (skip) {
            continue;
        }
        if (inst->GetOpcode() == Opcode::ReturnInlined) {
            inlined_calles.push(inst);
        } else if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
            if (!inlined_calles.empty()) {
                inlined_calles.pop();
            } else {
                ASSERT(caller == inst);
                return;
            }
        }
    }
    dom_block = dom_block->GetDominator();
    while (dom_block != nullptr) {
        for (auto inst : dom_block->InstsSafeReverse()) {
            if (inst->GetOpcode() == Opcode::ReturnInlined) {
                inlined_calles.push(inst);
            } else if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
                if (!inlined_calles.empty()) {
                    inlined_calles.pop();
                } else {
                    ASSERT(caller == inst);
                    return;
                }
            }
        }
        dom_block = dom_block->GetDominator();
    }
    UNREACHABLE();
}

void GraphChecker::CheckStartBlock()
{
    [[maybe_unused]] Inst *has_nullptr = nullptr;
    [[maybe_unused]] int32_t last_num = -1;
    ASSERT(GetGraph()->GetStartBlock());
    ASSERT_PRINT(GetGraph()->GetStartBlock()->GetPredsBlocks().empty(), "Start block can't have predecessors");
    ASSERT_PRINT(GetGraph()->GetStartBlock()->GetSuccsBlocks().size() == 1, "Start block should have one successor");
    for (auto inst : GetGraph()->GetStartBlock()->AllInsts()) {
        [[maybe_unused]] Opcode opc = inst->GetOpcode();
        ASSERT_DO(
            opc == Opcode::Constant || opc == Opcode::Parameter || opc == Opcode::SafePoint ||
                opc == Opcode::SpillFill || opc == Opcode::NullPtr || opc == Opcode::NOP || opc == Opcode::LiveIn,
            std::cerr
                << "Entry block can contain Constant, Parameter, NullPtr, SafePoint, NOP or SpillFill instructions"
                << *inst << std::endl);
        if (opc == Opcode::Parameter) {
            auto arg_num = inst->CastToParameter()->GetArgNumber();
            ASSERT_DO(
                last_num < static_cast<int32_t>(arg_num),
                std::cerr << "The argument number in the parameter must be greater than that of the previous parameter"
                          << *inst << std::endl);
            last_num = static_cast<int32_t>(arg_num);
        }
        if (opc == Opcode::NullPtr) {
            ASSERT_PRINT(has_nullptr == nullptr, "There should be not more than one NullPtr instruction");
            has_nullptr = inst;
        }
    }
}

void GraphChecker::CheckEndBlock()
{
    if (!GetGraph()->HasEndBlock()) {
        ASSERT_PRINT(HasOuterInfiniteLoop(), "Graph without infinite loops should have end block");
        return;
    }
    ASSERT_PRINT(GetGraph()->GetEndBlock()->GetSuccsBlocks().empty(), "End block can't have successors");
    [[maybe_unused]] auto iter = GetGraph()->GetEndBlock()->Insts();
    ASSERT_PRINT(iter.begin() == iter.end(), "End block can't have instructions");
}

void GraphChecker::CheckGraph()
{
    size_t num_inst = GetGraph()->GetCurrentInstructionId();
    ArenaVector<bool> inst_vec(num_inst, GetLocalAllocator()->Adapter());
    for (auto &bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            auto id = inst->GetId();
            ASSERT_DO(id < num_inst,
                      (std::cerr << "Instruction ID must be less than graph instruction counter: " << num_inst << "\n",
                       inst->Dump(&std::cerr)));
            ASSERT_DO(!inst_vec[id],
                      (std::cerr << "Instruction with same Id already exists:\n", inst->Dump(&std::cerr)));
            inst_vec[id] = true;
            ASSERT_DO(GetGraph()->IsDynamicMethod() || inst->GetType() != DataType::ANY,
                      (std::cerr << "The type ANY is supported only for dynamic languages\n", inst->Dump(&std::cerr)));
            ASSERT_DO(inst->SupportsMode(GetGraph()->GetCompilerMode()),
                      (std::cerr << "Instruction used in wrong mode\n", inst->Dump(&std::cerr)));
            VisitInstruction(inst);
        }
    }
}

void GraphChecker::CheckPhiInputs(Inst *phi_inst)
{
    for (size_t index = 0; index < phi_inst->GetInputsCount(); ++index) {
        [[maybe_unused]] auto pred = phi_inst->CastToPhi()->GetPhiInputBb(index);
        [[maybe_unused]] auto input_bb = phi_inst->CastToPhi()->GetPhiInput(pred)->GetBasicBlock();
        ASSERT_DO(input_bb->IsDominate(pred) || IsTryCatchDomination(input_bb, pred),
                  (std::cerr
                   << "Block where phi-input is located should dominate predecessor block corresponding to this input\n"
                   << "Block " << input_bb->GetId() << " should dominate " << pred->GetId() << std::endl
                   << *phi_inst));
    }
}

bool GraphChecker::CheckInstRegUsageSaved(const Inst *inst, Register reg) const
{
    if (reg == ACC_REG_ID) {
        return true;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    // Empty vector regs mask means we are using dynamic general regs set.
    if (DataType::IsFloatType(inst->GetType()) && !graph->GetUsedRegs<DataType::FLOAT64>()->empty()) {
        return graph->GetUsedRegs<DataType::FLOAT64>()->at(reg);
    }
    return graph->GetUsedRegs<DataType::INT64>()->at(reg);
}

[[maybe_unused]] static bool checkSpillFillMultiple(const compiler::Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::Parameter:
            return false;
        case Opcode::LoadObject:
        case Opcode::NewArray:
        case Opcode::NewObject:
            // In this case for BytecodeOptimizer SpillFill will be added after instruction, not before
            // user-insturction. So this check can't find it and it is skipped.
            return !inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer();
        default:
            return true;
    }
}

void GraphChecker::CheckNoLowLevel(BasicBlock *block)
{
    for ([[maybe_unused]] auto inst : block->Insts()) {
        ASSERT_DO(!inst->IsLowLevel(), inst->Dump(&std::cerr));
    }
}

void GraphChecker::MarkBlocksInLoop(Loop *loop, Marker mrk)
{
    ASSERT(loop->IsIrreducible() || loop->IsRoot() || loop->GetHeader() != nullptr);
    ASSERT(loop->IsIrreducible() || loop->IsRoot() || loop->IsTryCatchLoop() || loop->GetPreHeader() != nullptr);
    // Mark blocks and check if marker was not set before
    for ([[maybe_unused]] auto block : loop->GetBlocks()) {
        ASSERT(!block->SetMarker(mrk));
    }

    for (auto inner : loop->GetInnerLoops()) {
        MarkBlocksInLoop(inner, mrk);
    }
}

bool GraphChecker::CheckBlockHasPredecessor(BasicBlock *block, BasicBlock *predecessor)
{
    ASSERT(block != nullptr && predecessor != nullptr);
    for (auto pred : block->GetPredsBlocks()) {
        if (pred == predecessor) {
            return true;
        }
    }
    return false;
}

bool GraphChecker::CheckBlockHasSuccessor(BasicBlock *block, BasicBlock *successor)
{
    ASSERT(block != nullptr && successor != nullptr);
    for (auto succ : block->GetSuccsBlocks()) {
        if (succ == successor) {
            return true;
        }
    }
    return false;
}

bool GraphChecker::BlockContainsInstruction(BasicBlock *block, Opcode opcode)
{
    return std::find_if(block->Insts().begin(), block->Insts().end(),
                        [opcode](Inst *inst) { return inst->GetOpcode() == opcode; }) != block->Insts().end();
}

void GraphChecker::CheckLoopHasSafePoint(Loop *loop)
{
    [[maybe_unused]] auto it =
        std::find_if(loop->GetBlocks().begin(), loop->GetBlocks().end(),
                     [this](BasicBlock *block) { return BlockContainsInstruction(block, Opcode::SafePoint); });
    // Irreducible isn't fully populated - only 'one of the headers' and back-edge,
    // SafePoint can be inserted to the another 'header' and search will be failed
    ASSERT_DO(loop->IsTryCatchLoop() || loop->IsIrreducible() || it != loop->GetBlocks().end(),
              std::cerr << "Loop " << loop->GetId() << " must have safepoint\n");
    for (auto inner : loop->GetInnerLoops()) {
        CheckLoopHasSafePoint(inner);
    }
}

void GraphChecker::CheckLoops()
{
    ASSERT(GetGraph()->GetAnalysis<LoopAnalyzer>().IsValid());
    ASSERT(GetGraph()->GetRootLoop() != nullptr);
    ASSERT(GetGraph()->GetRootLoop()->IsRoot());
    ASSERT(GetGraph()->GetRootLoop()->GetHeader() == nullptr);
    ASSERT(GetGraph()->GetRootLoop()->GetPreHeader() == nullptr);
    auto root_loop = GetGraph()->GetRootLoop();
    auto mrk = GetGraph()->NewMarker();
    MarkBlocksInLoop(root_loop, mrk);

    for ([[maybe_unused]] auto block : GetGraph()->GetBlocksRPO()) {
        [[maybe_unused]] auto loop = block->GetLoop();
        ASSERT(loop != nullptr);
        ASSERT(block->IsMarked(mrk));
        if (block->IsLoopHeader()) {
            if (block->IsOsrEntry()) {
                ASSERT(GetGraph()->IsOsrMode());
                auto ss_osr = block->GetFirstInst();
                while (ss_osr != nullptr && (ss_osr->IsCatchPhi() || ss_osr->GetOpcode() == Opcode::Try)) {
                    ss_osr = ss_osr->GetNext();
                }
                ASSERT(ss_osr != nullptr && ss_osr->GetOpcode() == Opcode::SaveStateOsr);
            }
            [[maybe_unused]] auto preds = block->GetPredsBlocks();
            for ([[maybe_unused]] auto pred : preds) {
                ASSERT(pred->GetLoop() != loop || loop->HasBackEdge(pred));
            }

            if (!loop->IsIrreducible()) {
                for ([[maybe_unused]] auto back : loop->GetBackEdges()) {
                    ASSERT(std::find(preds.begin(), preds.end(), back) != preds.end());
                }
            }
        } else {
            ASSERT(!block->IsOsrEntry());
        }
    }
    GetGraph()->EraseMarker(mrk);
    if (options.IsCompilerUseSafepoint() && GetGraph()->SupportManagedCode()) {
        for (auto inner : root_loop->GetInnerLoops()) {
            CheckLoopHasSafePoint(inner);
        }
    }
}

void GraphChecker::CheckDomTree()
{
    ASSERT(GetGraph()->GetAnalysis<DominatorsTree>().IsValid());
    ArenaVector<BasicBlock *> dominators(GetGraph()->GetVectorBlocks().size(), GetLocalAllocator()->Adapter());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        dominators[block->GetId()] = block->GetDominator();
    }
    // Rebuild dom-tree
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->RunPass<DominatorsTree>();

    for ([[maybe_unused]] auto block : GetGraph()->GetBlocksRPO()) {
        ASSERT_DO(dominators[block->GetId()] == block->GetDominator(),
                  std::cerr << "Basic block with id " << block->GetId() << " has incorrect dominator with id "
                            << dominators[block->GetId()]->GetId() << std::endl
                            << "Correct dominator must be block with id " << block->GetDominator()->GetId() << std::endl
                            << "Note: basic blocks' ids in the original graph and in the cloned graph can be different"
                            << std::endl);
    }
}

void GraphChecker::CheckLoopAnalysis()
{
    // Save current loop info
    ArenaUnorderedMap<BasicBlock *, Loop *> loops(GetLocalAllocator()->Adapter());
    [[maybe_unused]] auto root_loop = GetGraph()->GetRootLoop();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsLoopHeader()) {
            loops.emplace(block, block->GetLoop());
        }
    }
    // Build new loop info and compare with saved one
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    ASSERT_PRINT(*root_loop == *GetGraph()->GetRootLoop(), "Root loop is incorrect\n");
    for (auto &[block, loop] : loops) {
        auto expected_loop = block->GetLoop();
        // An irreducible loop can have different heads, depending on the order of traversal
        if (loop->IsIrreducible()) {
            ASSERT(expected_loop->IsIrreducible());
            continue;
        }
        ASSERT(block->IsLoopHeader());
        if (loop == nullptr || expected_loop == nullptr) {
            UNREACHABLE();
            return;
        }
        ASSERT_DO(*loop == *expected_loop, std::cerr << "Loop " << loop->GetId() << " is incorrect\n");
    }
}

/**
 * Check that there is root's inner loop without exit-points
 */
bool GraphChecker::HasOuterInfiniteLoop()
{
    const auto &loops = GetGraph()->GetRootLoop()->GetInnerLoops();
    return std::find_if(loops.begin(), loops.end(), [](const Loop *loop) { return loop->IsInfinite(); }) != loops.end();
}

bool GraphChecker::CheckInstHasInput(Inst *inst, Inst *input)
{
    ASSERT(inst != nullptr && input != nullptr);
    ASSERT(input->GetBasicBlock() != nullptr);
    ASSERT(input->GetBasicBlock()->GetGraph() != nullptr);
    for (auto node : inst->GetInputs()) {
        if (node.GetInst() == input) {
            return true;
        }
    }
    return false;
}

bool GraphChecker::CheckInstHasUser(Inst *inst, Inst *user)
{
    ASSERT(inst != nullptr && user != nullptr);
    ASSERT(user->GetBasicBlock() != nullptr);
    ASSERT(user->GetBasicBlock()->GetGraph() != nullptr);
    for (auto &node : inst->GetUsers()) {
        if (node.GetInst() == user) {
            return true;
        }
    }
    return false;
}

void GraphChecker::CheckBlockEdges(const BasicBlock &block)
{
    [[maybe_unused]] auto last_inst_in_block = block.GetLastInst();
    if (block.GetSuccsBlocks().size() > 1) {
        ASSERT_PRINT(!block.IsEmpty() || block.IsTryEnd(),
                     "Block with 2 successors have no instructions or should be try-end");
        ASSERT_PRINT(block.IsTryBegin() || block.IsTryEnd() || last_inst_in_block->IsControlFlow(),
                     "Last instruction must be control flow in block with 2 successors");
    } else if (block.GetSuccsBlocks().size() == 1) {
        if (block.GetSuccsBlocks()[0]->IsEndBlock()) {
            if (block.IsEmpty()) {
                ASSERT(block.IsTryEnd());
                return;
            }
            auto last_inst = block.GetLastInst();
            [[maybe_unused]] auto opc = last_inst->GetOpcode();
            ASSERT_PRINT(last_inst->GetFlag(inst_flags::TERMINATOR),
                         "Last instruction in block before exit-block must be Return or Throw instruction.");
        }
    }
}

void GraphChecker::CheckTryBeginBlock(const BasicBlock &block)
{
    ASSERT(block.IsTryBegin());
    auto try_inst_it = std::find_if(block.AllInsts().begin(), block.AllInsts().end(),
                                    [](Inst *inst) { return inst->GetOpcode() == Opcode::Try; });
    ASSERT_PRINT(try_inst_it != block.AllInsts().end(), "Try-begin basic block should contain try-instructions");
    [[maybe_unused]] auto try_inst = (*try_inst_it)->CastToTry();
    for ([[maybe_unused]] auto succ_index : *try_inst->GetCatchEdgeIndexes()) {
        ASSERT_PRINT(succ_index < block.GetSuccsBlocks().size(),
                     "Try instruction holds incorrect try-begin block successor number");
    }
}

void GraphChecker::CheckJump(const BasicBlock &block)
{
    ASSERT(GetGraph()->IsRegAllocApplied());
    ASSERT(GetGraph()->IsAnalysisValid<LinearOrder>());
    if (block.IsIfBlock()) {
        const auto &blocks_vector = GetGraph()->GetBlocksLinearOrder();
        auto if_block_it = std::find(blocks_vector.begin(), blocks_vector.end(), &block);
        ASSERT(if_block_it != blocks_vector.end());
        auto block_after_if = std::next(if_block_it);
        if (block_after_if != blocks_vector.end()) {
            ASSERT_PRINT(*block_after_if != (*if_block_it)->GetFalseSuccessor(),
                         "`If-block` with immediate `false`-successor shouldn't have `JumpFlag`");
            ASSERT_PRINT(*block_after_if != (*if_block_it)->GetTrueSuccessor(),
                         "`true`-successor should be replaced with `false`-successor");
        }
    }
    [[maybe_unused]] auto num_succs = block.GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == 1 || block.IsTryBegin() || block.IsTryEnd() || block.IsIfBlock(),
                 "Basic block with Jump must have 1 successor or should be try-begin or if block");
}

/**
 * Regalloc propagates catch-phi's inputs to the users and can broke user's domination. In this case:
 * - input_block should be placed inside try block;
 * - try-begin block should dominate user_block;
 *
 * [try-begin]----------\
 *     |                |
 * [input_block]        |
 *     |                |
 * [try-end]----------->|
 *                      |
 *                [catch-begin]
 *                      |
 *                [user_block]
 */
bool GraphChecker::IsTryCatchDomination(const BasicBlock *input_block, const BasicBlock *user_block) const
{
    ASSERT(GetGraph()->IsRegAllocApplied());
    if (input_block->IsTry()) {
        auto blocks = GetGraph()->GetTryBeginBlocks();
        auto it =
            std::find_if(blocks.begin(), blocks.end(), [user_block](auto &bb) { return bb->IsDominate(user_block); });
        return it != blocks.end();
    }
    return false;
}

bool IsObjectCheckDisabledForOpcode(const Inst *inst)
{
    auto opc = inst->GetOpcode();
    return inst->IsCheck() || inst->IsConst() || opc == Opcode::NullPtr || inst->IsClassInst() ||
           opc == Opcode::GetInstanceClass;
}

void GraphChecker::CheckSaveStatesWithRuntimeCallUsers()
{
#ifndef NDEBUG
    for (auto &block : GetGraph()->GetBlocksRPO()) {
        for (const auto &ss : block->AllInsts()) {
            if (ss->GetOpcode() != Opcode::SaveState) {
                continue;
            }
            for (auto &user : ss->GetUsers()) {
                auto user_inst = user.GetInst();
                if (!user_inst->IsRuntimeCall()) {
                    continue;
                }
                ASSERT(user_inst->GetBasicBlock() == ss->GetBasicBlock());
                auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, user_inst);
                for (++it; *it != ss; ++it) {
                    // Non-reference instructions, checks, nullptr and classes cannot be moved by GC
                    ASSERT((*it)->GetType() != DataType::REFERENCE || IsObjectCheckDisabledForOpcode(*it));
                }
            }
        }
    }
#endif
}

void PrepareUsers(Inst *inst, ArenaVector<User *> *users)
{
    for (auto &user : inst->GetUsers()) {
        users->push_back(&user);
    }
    auto i = std::find_if(users->begin(), users->end(), [](User *user) { return user->GetInst()->IsCheck(); });
    while (i != users->end()) {
        for (auto &u : (*i)->GetInst()->GetUsers()) {
            users->push_back(&u);
        }
        users->erase(i);
        i = std::find_if(users->begin(), users->end(), [](User *user) { return user->GetInst()->IsCheck(); });
    }
    for (auto &it : (*users)) {
        [[maybe_unused]] auto user = it->GetInst();
        ASSERT(!user->IsCheck());
    }
}

void GraphChecker::CheckSaveStateInputs()
{
#ifndef NDEBUG
    ArenaVector<User *> users(GetLocalAllocator()->Adapter());
    for (auto &block : GetGraph()->GetBlocksRPO()) {
        for (const auto &inst : block->AllInsts()) {
            if (IsObjectCheckDisabledForOpcode(inst)) {
                continue;
            }
            // skip phi which all inputs is disabled
            if (inst->GetOpcode() == Opcode::Phi) {
                bool skip_flag = true;
                for (const auto &input : inst->GetInputs()) {
                    skip_flag &= IsObjectCheckDisabledForOpcode(input.GetInst());
                }
                if (skip_flag) {
                    continue;
                }
            }

            PrepareUsers(inst, &users);

            auto object_visited = GetGraph()->NewMarker();
            auto osr_visited = GetGraph()->NewMarker();
            for (auto &it : users) {
                auto user = it->GetInst();
                // For Phi we need to check only pass between object and phi
                if (user->IsPhi() || user->IsCatchPhi()) {
                    continue;
                }
                // Virtual register can be overwrite
                if (user->IsSaveState()) {
                    continue;
                }
                if (inst->GetType() == DataType::REFERENCE) {
                    CheckObjectRec(inst, user, user->GetBasicBlock(), user->GetPrev(), object_visited);
                }
                CheckSaveStateOsrRec(inst, user, user->GetBasicBlock(), osr_visited);
            }
            GetGraph()->EraseMarker(object_visited);
            GetGraph()->EraseMarker(osr_visited);
            users.clear();
        }
    }
#endif
}

void GraphChecker::FindObjectInSaveState(const Inst *object, Inst *ss) const
{
    while (ss != nullptr && object->IsDominate(ss)) {
        auto it = std::find_if(ss->GetInputs().begin(), ss->GetInputs().end(),
                               [object, ss](Input input) { return ss->GetDataFlowInput(input.GetInst()) == object; });
        if (it != ss->GetInputs().end()) {
            return;
        }
        auto caller = static_cast<SaveStateInst *>(ss)->GetCallerInst();
        if (caller == nullptr) {
            break;
        }
        ss = caller->GetSaveState();
    }
    if (object != nullptr && ss != nullptr) {
        std::cerr << "Object not found in the SaveState: " << std::endl
                  << *object << std::endl
                  << " " << *ss << std::endl;
    }
    UNREACHABLE();
}

bool IsSaveStateForGc(Inst *inst)
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

void GraphChecker::CheckObjectRec(const Inst *object, const Inst *user, const BasicBlock *block, Inst *start_from,
                                  Marker visited) const
{
    if (start_from != nullptr) {
        auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, start_from);
        for (; it != block->AllInstsSafeReverse().end(); ++it) {
            auto inst = *it;
            if (inst == nullptr) {
                break;
            }
            if (inst->SetMarker(visited)) {
                return;
            }
            if (IsSaveStateForGc(inst)) {
                FindObjectInSaveState(object, inst);
            } else if (inst == object || inst == user) {
                return;
            }
        }
    }
    for (auto pred : block->GetPredsBlocks()) {
        // Catch-begin block has edge from try-end block, and all try-blocks should be visited from this edge.
        // `object` can be placed inside try-block - after try-begin, so that visiting try-begin is wrong
        if (block->IsCatchBegin() && pred->IsTryBegin()) {
            continue;
        }
        CheckObjectRec(object, user, pred, pred->GetLastInst(), visited);
    }
}

void GraphChecker::CheckSaveStateOsrRec(const Inst *inst, const Inst *user, BasicBlock *block, Marker visited)
{
    if (block->SetMarker(visited)) {
        return;
    }
    if (inst->GetBasicBlock() == block) {
        return;
    }
    if (block->IsOsrEntry()) {
        ASSERT(GetGraph()->IsOsrMode());
        auto ss = block->GetFirstInst();
        ASSERT(ss != nullptr && ss->GetOpcode() == Opcode::SaveStateOsr);
        [[maybe_unused]] auto it =
            std::find_if(ss->GetInputs().begin(), ss->GetInputs().end(),
                         [inst, ss](Input input) { return ss->GetDataFlowInput(input.GetInst()) == inst; });
        ASSERT(it != ss->GetInputs().end());
    }
    for (auto pred : block->GetPredsBlocks()) {
        CheckSaveStateOsrRec(inst, user, pred, visited);
    }
}

/*
 * Visitors to check instructions types
 */
void GraphChecker::VisitMov([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitAbs([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitSqrt([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(DataType::IsFloatType(inst->GetType()),
              (std::cerr << "\nSqrt must have float type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}

void GraphChecker::VisitAddI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (inst->GetType() == DataType::POINTER) {
        ASSERT_DO(inst->GetInputType(0) == DataType::POINTER,
                  (std::cerr << "\nptr AddI must have ptr input type\n", inst->Dump(&std::cerr)));
        return;
    }
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nAddI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitSubI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!static_cast<GraphChecker *>(v)->GetGraph()->SupportManagedCode() && inst->GetType() == DataType::POINTER) {
        ASSERT_DO(inst->GetInputType(0) == DataType::POINTER,
                  (std::cerr << "\nptr SubI must have ptr input type\n", inst->Dump(&std::cerr)));
        return;
    }
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nSubI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitMulI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto type = inst->GetType();
    ASSERT_DO(DataType::Is32Bits(type, static_cast<GraphChecker *>(v)->GetGraph()->GetArch()) &&
                  !DataType::IsReference(type),
              (std::cerr << "\nMulI must have Int32 type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitDivI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto type = inst->GetType();
    ASSERT_DO(DataType::Is32Bits(type, static_cast<GraphChecker *>(v)->GetGraph()->GetArch()) &&
                  !DataType::IsReference(type),
              (std::cerr << "\nDivI must have Int32 type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitModI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto type = inst->GetType();
    ASSERT_DO(DataType::Is32Bits(type, static_cast<GraphChecker *>(v)->GetGraph()->GetArch()) &&
                  !DataType::IsReference(type),
              (std::cerr << "\nModI must have Int32 type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitAndI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nAndI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitOrI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nOrI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitXorI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nXorI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}
void GraphChecker::VisitShlI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nShlI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
    [[maybe_unused]] auto imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    ASSERT_DO(imm <= DataType::GetTypeSize(inst->GetType(), static_cast<GraphChecker *>(v)->GetGraph()->GetArch()),
              (std::cerr << "\nShlI have shift more then size of type\n", inst->Dump(&std::cerr)));
}
void GraphChecker::VisitShrI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nShrI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
    [[maybe_unused]] auto imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    ASSERT_DO(imm <= DataType::GetTypeSize(inst->GetType(), static_cast<GraphChecker *>(v)->GetGraph()->GetArch()),
              (std::cerr << "\nShrI have shift more then size of type\n", inst->Dump(&std::cerr)));
}
void GraphChecker::VisitAShlI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nAShrI must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
    [[maybe_unused]] auto imm = static_cast<BinaryImmOperation *>(inst)->GetImm();
    ASSERT_DO(imm <= DataType::GetTypeSize(inst->GetType(), static_cast<GraphChecker *>(v)->GetGraph()->GetArch()),
              (std::cerr << "\nAShlI have shift more then size of type\n", inst->Dump(&std::cerr)));
}
void GraphChecker::VisitNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "\nNot must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
}

void GraphChecker::VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!static_cast<GraphChecker *>(v)->GetGraph()->SupportManagedCode() && inst->GetType() == DataType::POINTER) {
        [[maybe_unused]] auto type_1 = inst->GetInput(0).GetInst()->GetType();
        [[maybe_unused]] auto type_2 = inst->GetInput(1).GetInst()->GetType();
        ASSERT_DO(type_1 != type_2,
                  (std::cerr << "\nptr Add must have ptr and int input types\n", inst->Dump(&std::cerr)));
        ASSERT_DO((type_1 == DataType::POINTER && DataType::GetCommonType(type_2) == DataType::INT64) ||
                      (type_2 == DataType::POINTER && DataType::GetCommonType(type_1) == DataType::INT64),
                  (std::cerr << "\nptr Add must have ptr and int input types\n", inst->Dump(&std::cerr)));
        return;
    }
    CheckBinaryOperationTypes(inst);
}
void GraphChecker::VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!static_cast<GraphChecker *>(v)->GetGraph()->SupportManagedCode()) {
        [[maybe_unused]] auto type_1 = inst->GetInput(0).GetInst()->GetType();
        [[maybe_unused]] auto type_2 = inst->GetInput(1).GetInst()->GetType();
        if (inst->GetType() == DataType::POINTER) {
            ASSERT_DO(type_1 != type_2,
                      (std::cerr << "\nptr Sub must have ptr and int input types\n", inst->Dump(&std::cerr)));
            ASSERT_DO((type_1 == DataType::POINTER && DataType::GetCommonType(type_2) == DataType::INT64) ||
                          (type_2 == DataType::POINTER && DataType::GetCommonType(type_1) == DataType::INT64),
                      (std::cerr << "\nptr Sub must have ptr and int input types\n", inst->Dump(&std::cerr)));
            return;
        }
        if (type_1 == DataType::POINTER && type_2 == DataType::POINTER) {
            ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                      (std::cerr << "\n Sub with 2 ptr inputs must have int type\n", inst->Dump(&std::cerr)));
            return;
        }
    }
    CheckBinaryOperationTypes(inst);
}
void GraphChecker::VisitMul([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst);
}
void GraphChecker::VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst);
}
void GraphChecker::VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst);
}
void GraphChecker::VisitMin([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst);
}
void GraphChecker::VisitMax([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst);
}
void GraphChecker::VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}
void GraphChecker::VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}
void GraphChecker::VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}
void GraphChecker::VisitAnd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}
void GraphChecker::VisitOr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}
void GraphChecker::VisitXor([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}

void GraphChecker::VisitAddOverflow([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOverflowOperation(inst->CastToAddOverflow());
}
void GraphChecker::VisitSubOverflow([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOverflowOperation(inst->CastToSubOverflow());
}
void GraphChecker::VisitLoadArray([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
}
void GraphChecker::VisitLoadArrayI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
}
void GraphChecker::VisitLoadArrayPair([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
              (std::cerr << "Unallowed type of coalesced load\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(inst);
}
void GraphChecker::VisitLoadArrayPairI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
              (std::cerr << "Unallowed type of coalesced load\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(inst);
}

void GraphChecker::VisitLoadPairPart([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
              (std::cerr << "Unallowed type of coalesced load\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(inst);
    [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto idx = inst->CastToLoadPairPart()->GetImm();
    ASSERT_DO(op1->GetOpcode() == Opcode::LoadArrayPair || op1->GetOpcode() == Opcode::LoadArrayPairI,
              (std::cerr << "Input instruction is not a Pair\n", inst->Dump(&std::cerr)));
    if (op1->GetOpcode() == Opcode::LoadArrayPairI) {
        ASSERT_DO(idx < op1->CastToLoadArrayPairI()->GetDstCount(),
                  (std::cerr << "Pair index is out of bounds\n", inst->Dump(&std::cerr)));
    } else {
        ASSERT_DO(idx < op1->CastToLoadArrayPair()->GetDstCount(),
                  (std::cerr << "Pair index is out of bounds\n", inst->Dump(&std::cerr)));
    }
    ASSERT_DO(
        CheckCommonTypes(inst, inst->GetInputs()[0].GetInst()),
        (std::cerr << "Types of load vector element and vector input are not compatible\n", inst->Dump(&std::cerr)));

    // Strict order here
    auto prev = inst->GetPrev();
    while (prev != nullptr && prev != op1) {
        if (prev->GetOpcode() == Opcode::LoadPairPart || prev->GetOpcode() == Opcode::SpillFill) {
            prev = prev->GetPrev();
        } else {
            break;
        }
    }
    ASSERT_DO(prev != nullptr && prev == op1,
              (std::cerr << "LoadPairPart(s) instructions must follow immediately after appropriate LoadArrayPair(I)\n",
               inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreArrayPair([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
              (std::cerr << "Unallowed type of coalesced store\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(inst);
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[2U].GetInst()),
              (std::cerr << "Types of store and the first stored value are not compatible\n", inst->Dump(&std::cerr)));
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[3U].GetInst()),
              (std::cerr << "Types of store and the second stored value are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool need_barrier = inst->CastToStoreArrayPair()->GetNeedBarrier();
    ASSERT_DO(need_barrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
              (std::cerr << "StoreArrayPair has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreArrayPairI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(MemoryCoalescing::AcceptedType(inst->GetType()) || DataType::IsReference(inst->GetType()),
              (std::cerr << "Unallowed type of coalesced store\n", inst->Dump(&std::cerr)));
    CheckMemoryInstruction(inst);
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
              (std::cerr << "Types of store and the first stored value are not compatible\n", inst->Dump(&std::cerr)));
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[2U].GetInst()),
              (std::cerr << "Types of store and the second stored value are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool need_barrier = inst->CastToStoreArrayPairI()->GetNeedBarrier();
    ASSERT_DO(need_barrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
              (std::cerr << "StoreArrayPairI has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreArray([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[2U].GetInst()),
              (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool need_barrier = inst->CastToStoreArray()->GetNeedBarrier();
    ASSERT_DO(need_barrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
              (std::cerr << "StoreArray has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreArrayI([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
              (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool need_barrier = inst->CastToStoreArrayI()->GetNeedBarrier();
    ASSERT_DO(need_barrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
              (std::cerr << "StoreArrayI has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreStatic([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
              (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool need_barrier = inst->CastToStoreStatic()->GetNeedBarrier();
    ASSERT_DO(need_barrier == (inst->GetType() == DataType::REFERENCE),
              (std::cerr << "StoreStatic has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto init_inst = inst->GetInputs()[0].GetInst();
    if (init_inst->IsPhi()) {
        return;
    }
    ASSERT_DO(init_inst->GetOpcode() == Opcode::LoadAndInitClass,
              (std::cerr << "The first input for the StoreStatic should be LoadAndInitClass", inst->Dump(&std::cerr),
               init_inst->Dump(&std::cerr)));
    [[maybe_unused]] auto store_static = inst->CastToStoreStatic();
    [[maybe_unused]] auto class_id =
        graph->GetRuntime()->GetClassIdForField(store_static->GetMethod(), store_static->GetTypeId());
    // See comment in VisitNewObject about this if statement
    if (init_inst->CastToLoadAndInitClass()->GetClass() == nullptr) {
        ASSERT_DO(init_inst->CastToLoadAndInitClass()->GetTypeId() == class_id,
                  (std::cerr << "StoreStatic and LoadAndInitClass must have equal class", inst->Dump(&std::cerr),
                   init_inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitUnresolvedStoreStatic([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[0].GetInst()),
              (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool need_barrier = inst->CastToUnresolvedStoreStatic()->GetNeedBarrier();
    ASSERT_DO(need_barrier == (inst->GetType() == DataType::REFERENCE),
              (std::cerr << "UnresolvedStoreStatic has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInputs()[1].GetInst();
    ASSERT_DO(ss->GetOpcode() == Opcode::SaveState,
              (std::cerr << "UnresolvedStoreStatic instruction second operand is not a SaveState",
               inst->Dump(&std::cerr), ss->Dump(&std::cerr)));
}

void GraphChecker::VisitStoreObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
              (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool need_barrier = inst->CastToStoreObject()->GetNeedBarrier();
    ASSERT_DO(need_barrier == (inst->GetType() == DataType::REFERENCE) || inst->GetType() == DataType::ANY,
              (std::cerr << "StoreObject has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitUnresolvedStoreObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    ASSERT_DO(CheckCommonTypes(inst, inst->GetInputs()[1].GetInst()),
              (std::cerr << "Types of store and store input are not compatible\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] bool need_barrier = inst->CastToUnresolvedStoreObject()->GetNeedBarrier();
    ASSERT_DO(need_barrier == (inst->GetType() == DataType::REFERENCE),
              (std::cerr << "UnresolvedStoreObject has incorrect value NeedBarrier", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInputs()[2U].GetInst();
    ASSERT_DO(ss->GetOpcode() == Opcode::SaveState,
              (std::cerr << "UnresolvedStoreObject instruction third operand is not a SaveState",
               inst->Dump(&std::cerr), ss->Dump(&std::cerr)));
}

void GraphChecker::VisitLoadStatic([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    auto graph = static_cast<GraphChecker *>(v)->GetGraph();
    [[maybe_unused]] auto init_inst = inst->GetInputs()[0].GetInst();
    if (init_inst->IsPhi()) {
        return;
    }
    ASSERT_DO(init_inst->GetOpcode() == Opcode::LoadAndInitClass,
              (std::cerr << "The first input for the LoadStatic should be LoadAndInitClass", inst->Dump(&std::cerr),
               init_inst->Dump(&std::cerr)));
    [[maybe_unused]] auto load_static = inst->CastToLoadStatic();
    [[maybe_unused]] auto class_id =
        graph->GetRuntime()->GetClassIdForField(load_static->GetMethod(), load_static->GetTypeId());
    // See comment in VisitNewObject about this if statement
    if (init_inst->CastToLoadAndInitClass()->GetClass() == nullptr) {
        ASSERT_DO(init_inst->CastToLoadAndInitClass()->GetTypeId() == class_id,
                  (std::cerr << "LoadStatic and LoadAndInitClass must have equal class", inst->Dump(&std::cerr),
                   init_inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitUnresolvedLoadStatic([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    [[maybe_unused]] auto ss = inst->GetInputs()[0].GetInst();
    ASSERT_DO(ss->GetOpcode() == Opcode::SaveState,
              (std::cerr << "UnresolvedLoadStatic instruction first operand is not a SaveState", inst->Dump(&std::cerr),
               ss->Dump(&std::cerr)));
}

void GraphChecker::VisitLoadClass([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(inst->GetType() == DataType::REFERENCE,
              (std::cerr << "LoadClass must have Reference type", inst->Dump(&std::cerr)));
    for (auto &user : inst->GetUsers()) {
        [[maybe_unused]] auto user_inst = user.GetInst();
        ASSERT_DO(
            user_inst->GetOpcode() == Opcode::CheckCast || user_inst->GetOpcode() == Opcode::IsInstance ||
                user_inst->GetOpcode() == Opcode::Phi,
            (std::cerr << "Incorrect user of the LoadClass", inst->Dump(&std::cerr), user_inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitLoadAndInitClass([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(inst->GetType() == DataType::REFERENCE,
              (std::cerr << "LoadAndInitClass must have Reference type", inst->Dump(&std::cerr)));
    for (auto &user : inst->GetUsers()) {
        [[maybe_unused]] auto user_inst = user.GetInst();
        ASSERT_DO(user_inst->GetOpcode() == Opcode::LoadStatic || user_inst->GetOpcode() == Opcode::StoreStatic ||
                      user_inst->GetOpcode() == Opcode::NewObject || user_inst->GetOpcode() == Opcode::Phi ||
                      user_inst->GetOpcode() == Opcode::MultiArray || user_inst->GetOpcode() == Opcode::InitObject ||
                      user_inst->GetOpcode() == Opcode::UnresolvedLoadStatic ||
                      user_inst->GetOpcode() == Opcode::UnresolvedStoreStatic ||
                      user_inst->GetOpcode() == Opcode::Intrinsic || user_inst->GetOpcode() == Opcode::NewArray ||
                      user_inst->GetOpcode() == Opcode::IsInstance || user_inst->GetOpcode() == Opcode::CheckCast,
                  (std::cerr << "Incorrect user of the LoadAndInitClass", inst->Dump(&std::cerr),
                   user_inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitUnresolvedLoadAndInitClass([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(inst->GetType() == DataType::REFERENCE,
              (std::cerr << "UnresolvedLoadAndInitClass must have Reference type", inst->Dump(&std::cerr)));
    ASSERT_DO(inst->CastToUnresolvedLoadAndInitClass()->GetClass() == nullptr,
              (std::cerr << "UnresolvedLoadAndInitClass must have a null ClassPtr", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInputs()[0].GetInst();
    ASSERT_DO(ss->GetOpcode() == Opcode::SaveState,
              (std::cerr << "UnresolvedLoadAndInitClass instruction first operand is not a SaveState",
               inst->Dump(&std::cerr), ss->Dump(&std::cerr)));
    for (auto &user : inst->GetUsers()) {
        [[maybe_unused]] auto user_inst = user.GetInst();
        ASSERT_DO(user_inst->GetOpcode() == Opcode::LoadStatic || user_inst->GetOpcode() == Opcode::StoreStatic ||
                      user_inst->GetOpcode() == Opcode::NewObject || user_inst->GetOpcode() == Opcode::NewArray ||
                      user_inst->GetOpcode() == Opcode::Phi || user_inst->GetOpcode() == Opcode::MultiArray ||
                      user_inst->GetOpcode() == Opcode::UnresolvedLoadStatic ||
                      user_inst->GetOpcode() == Opcode::UnresolvedStoreStatic,
                  (std::cerr << "Incorrect user of the UnresolvedLoadAndInitClass", inst->Dump(&std::cerr),
                   user_inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitNewObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(inst->GetType() == DataType::REFERENCE,
              (std::cerr << "NewObject must be have Reference type", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto init_inst = inst->GetInputs()[0].GetInst();
    if (init_inst->IsPhi()) {
        return;
    }
    ASSERT_DO(
        init_inst->GetOpcode() == Opcode::LoadAndInitClass ||
            init_inst->GetOpcode() == Opcode::UnresolvedLoadAndInitClass,
        (std::cerr << "The first input for the NewObject should be LoadAndInitClass or UnresolvedLoadAndInitClass",
         inst->Dump(&std::cerr), init_inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss_inst = inst->GetInputs()[1].GetInst();
    ASSERT_DO(ss_inst->GetOpcode() == Opcode::SaveState,
              (std::cerr << "The second input for the NewObject should be SaveState", inst->Dump(&std::cerr),
               ss_inst->Dump(&std::cerr)));
    // If InitClass contains an already resolved class, then IDs may be different. Because VN can remove the
    // duplicated InitClass and keep only one that is located in the inlined method and has a different id
    // accordingly.
    if (init_inst->GetOpcode() == Opcode::LoadAndInitClass &&
        init_inst->CastToLoadAndInitClass()->GetClass() == nullptr) {
        ASSERT_DO(init_inst->CastToLoadAndInitClass()->GetTypeId() == inst->CastToNewObject()->GetTypeId(),
                  std::cerr << "NewObject and LoadAndInitClass must have equal class:\n"
                            << *inst << '\n'
                            << *init_inst << std::endl);
    } else if (init_inst->GetOpcode() == Opcode::UnresolvedLoadAndInitClass) {
        ASSERT_DO(init_inst->CastToUnresolvedLoadAndInitClass()->GetTypeId() == inst->CastToNewObject()->GetTypeId(),
                  std::cerr << "NewObject and UnresolvedLoadAndInitClass must have equal class:\n"
                            << *inst << '\n'
                            << *init_inst << std::endl);
    }
}

void GraphChecker::VisitInitObject([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(options.IsCompilerSupportInitObjectInst(),
              (std::cerr << "Instruction InitObject isn't supported", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitInitClass([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(inst->GetType() == DataType::NO_TYPE,
              (std::cerr << "InitClass doesn't have type", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitLoadObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
}

void GraphChecker::VisitUnresolvedLoadObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckMemoryInstruction(inst);
    [[maybe_unused]] auto ss = inst->GetInputs()[1].GetInst();
    ASSERT_DO(ss->GetOpcode() == Opcode::SaveState,
              (std::cerr << "UnresolvedLoadObject instruction second operand is not a SaveState",
               inst->Dump(&std::cerr), ss->Dump(&std::cerr)));
}

void GraphChecker::VisitConstant([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    [[maybe_unused]] auto type = inst->GetType();
    [[maybe_unused]] auto is_dynamic = static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod();
    if (static_cast<GraphChecker *>(v)->GetGraph()->IsBytecodeOptimizer()) {
        ASSERT_DO(
            type == DataType::FLOAT32 || type == DataType::FLOAT64 || type == DataType::INT64 ||
                type == DataType::INT32 || (type == DataType::ANY && is_dynamic),
            (std::cerr << "Constant inst can be only FLOAT32, FLOAT64, INT32 or INT64\n", inst->Dump(&std::cerr)));

    } else {
        ASSERT_DO(
            type == DataType::FLOAT32 || type == DataType::FLOAT64 || type == DataType::INT64 ||
                (type == DataType::ANY && is_dynamic),
            (std::cerr << "Constant instruction can be only FLOAT32, FLOAT64 or INT64\n", inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitNullPtr([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(inst->GetType() == DataType::REFERENCE,
              (std::cerr << "NullPtr instruction should have REFERENCE type only\n", inst->Dump(&std::cerr)));

    ASSERT_DO(static_cast<GraphChecker *>(v)->IncrementNullPtrInstCounterAndGet() == 1,
              (std::cerr << "There should be not more than one NullPtr instruction in graph\n",
               inst->GetBasicBlock()->Dump(&std::cerr)));
}

void GraphChecker::VisitPhi([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    for ([[maybe_unused]] auto input : inst->GetInputs()) {
        ASSERT_DO(CheckCommonTypes(inst, input.GetInst()),
                  (std::cerr << "Types of phi result and phi input are not compatible\n"
                             << *inst << std::endl
                             << *input.GetInst()));
    }
}

void GraphChecker::VisitParameter([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(inst->GetType() != DataType::NO_TYPE,
              (std::cerr << "The parametr doesn't have type:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitCompare([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto op2 = inst->GetInputs()[1].GetInst();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        ASSERT_DO(inst->GetInputType(i) != DataType::NO_TYPE,
                  std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }
    ASSERT_DO(inst->GetInputType(0) == inst->GetInputType(1),
              std::cerr << "Conditional instruction has different inputs type: " << *inst << std::endl);
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        ASSERT_DO(inst->CastToCompare()->GetCc() == ConditionCode::CC_NE ||
                      inst->CastToCompare()->GetCc() == ConditionCode::CC_EQ,
                  (std::cerr << "Reference compare must have CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op1->IsConst()) {
            ASSERT_DO(IsZeroConstant(op1), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op1->GetType() == DataType::REFERENCE,
                      (std::cerr << "Condition instruction 1st operand type is not a reference\n",
                       inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        }
        if (op2->IsConst()) {
            ASSERT_DO(IsZeroConstant(op2), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op2->GetType() == DataType::REFERENCE,
                      (std::cerr << "Condition instruction 2nd operand type is not a reference\n",
                       inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        }
    }
    ASSERT_PRINT(inst->GetType() == DataType::BOOL, "Condition instruction type is not a bool");
}

void GraphChecker::VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto dst_type = inst->GetType();
    [[maybe_unused]] auto src_type = inst->GetInputType(0);
    [[maybe_unused]] auto input_type = inst->GetInput(0).GetInst()->GetType();

    if (dst_type == DataType::ANY) {
        ASSERT_DO((DataType::IsTypeNumeric(input_type) || input_type == DataType::REFERENCE),
                  (std::cerr << "Cast instruction operand type is not a numeric or reference type\n",
                   inst->Dump(&std::cerr)));
    } else {
        ASSERT_DO(DataType::IsTypeNumeric(dst_type),
                  (std::cerr << "Cast instruction dst type is not a numeric type\n", inst->Dump(&std::cerr)));
        if (static_cast<GraphChecker *>(v)->GetGraph()->GetMode().SupportManagedCode()) {
            ASSERT_DO(DataType::IsTypeNumeric(src_type),
                      (std::cerr << "Cast instruction src type is not a numeric type\n", inst->Dump(&std::cerr)));
            ASSERT_DO(DataType::IsTypeNumeric(input_type),
                      (std::cerr << "Cast instruction operand type is not a numeric type\n", inst->Dump(&std::cerr)));
        }
        ASSERT_DO(DataType::GetCommonType(src_type) == DataType::GetCommonType(input_type),
                  (std::cerr << "Incorrect src_type and input type\n", inst->Dump(&std::cerr)));
        ASSERT_DO(!(DataType::IsFloatType(src_type) && DataType::IsLessInt32(dst_type)),
                  (std::cerr << "Cast instruction from " << DataType::internal::TYPE_NAMES.at(src_type) << " to "
                             << DataType::internal::TYPE_NAMES.at(dst_type) << " don't support\n",
                   inst->Dump(&std::cerr)));
    }
}

void GraphChecker::VisitCmp([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op1 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op2 = inst->GetInput(1).GetInst();
    ASSERT_DO(DataType::IsTypeNumeric(op1->GetType()),
              (std::cerr << "Cmp instruction 1st operand type is not a numeric type\n", inst->Dump(&std::cerr),
               op1->Dump(&std::cerr)));
    ASSERT_DO(DataType::IsTypeNumeric(op2->GetType()),
              (std::cerr << "Cmp instruction 2st operand type is not a numeric type\n", inst->Dump(&std::cerr),
               op2->Dump(&std::cerr)));
    ASSERT_DO(
        DataType::GetCommonType(op1->GetType()) == DataType::GetCommonType(inst->GetInputType(0)),
        (std::cerr << "Input type and Cmp Input Type are not equal\n", inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
    ASSERT_DO(
        DataType::GetCommonType(op2->GetType()) == DataType::GetCommonType(inst->GetInputType(1)),
        (std::cerr << "Input type and Cmp Input Type are not equal\n", inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
    ASSERT_DO(inst->GetType() == DataType::INT32,
              (std::cerr << "Cmp instruction type is not a int32\n", inst->Dump(&std::cerr)));
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        ASSERT_DO(inst->GetInputType(i) != DataType::NO_TYPE,
                  std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }
}

void GraphChecker::VisitMonitor([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    ASSERT_DO(inst->GetType() == DataType::VOID, (std::cerr << "Monitor type is not a void", inst->Dump(&std::cerr)));
    ASSERT_DO(DataType::IsReference(op->GetType()),
              (std::cerr << "Monitor instruction 1st operand type is not a reference", inst->Dump(&std::cerr),
               op->Dump(&std::cerr)));
    [[maybe_unused]] auto op1 = inst->GetInputs()[1].GetInst();
    ASSERT_DO(op1->GetOpcode() == Opcode::SaveState,
              (std::cerr << "Monitor instruction second operand is not a SaveState", inst->Dump(&std::cerr),
               op1->Dump(&std::cerr)));
}

void GraphChecker::VisitReturn([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    ASSERT_DO(CheckCommonTypes(inst, op), (std::cerr << "Types of return and its input are not compatible\n return:\n",
                                           inst->Dump(&std::cerr), std::cerr << "\n input:\n", op->Dump(&std::cerr)));
    CheckContrlFlowInst(inst);
    [[maybe_unused]] auto num_succs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == 1, "Basic block with Return must have 1 successor");
    [[maybe_unused]] auto succ = inst->GetBasicBlock()->GetSuccsBlocks()[0];
    ASSERT_DO(succ->IsEndBlock() || succ->IsTryEnd(),
              std::cerr << "Basic block with Return must have end or try end block as successor:\n"
                        << *inst << std::endl);
}

void GraphChecker::VisitReturnVoid([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckContrlFlowInst(inst);
    [[maybe_unused]] auto num_succs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == 1, "Basic block with ReturnVoid must have 1 successor");
    [[maybe_unused]] auto succ = inst->GetBasicBlock()->GetSuccsBlocks()[0];
    ASSERT_PRINT(succ->IsEndBlock() || succ->IsTryEnd(),
                 "Basic block with ReturnVoid must have end or try_end block as successor.");
}

void GraphChecker::VisitNullCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] Inst *array = inst->GetInput(0).GetInst();
    ASSERT_DO(DataType::IsReference(array->GetType()) || array->GetType() == DataType::ANY,
              (std::cerr << "\n Types of input NullCheck must be REFERENCE or ANY: \n", inst->Dump(&std::cerr),
               array->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInput(1).GetInst();
    ASSERT_DO(ss->GetOpcode() == Opcode::SaveState || ss->GetOpcode() == Opcode::SaveStateDeoptimize,
              (std::cerr << "\n Second input of NullCheck must be SaveState: \n", inst->Dump(&std::cerr),
               ss->Dump(&std::cerr)));
}

void GraphChecker::VisitBoundsCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    for (int i = 0; i < 1; i++) {
        [[maybe_unused]] auto op = inst->GetInputs()[i].GetInst();
        [[maybe_unused]] auto op_type = op->GetType();
        // TODO(pishin): actually type should be INT32, but predecessor may be Call instruction with type u16, u8
        // e.t.c
        ASSERT_DO(
            (op->IsConst() && op_type == DataType::INT64) ||
                (DataType::GetCommonType(op_type) == DataType::INT64 &&
                 Is32Bits(op_type, static_cast<GraphChecker *>(v)->GetGraph()->GetArch())),
            (std::cerr << "Types of " << i << " input BoundsCheck must be INT32 or less:\n", inst->Dump(&std::cerr)));
    }
    CheckThrows(inst, {Opcode::LoadArray, Opcode::StoreArray, Opcode::LoadArrayPair, Opcode::StoreArrayPair,
                       Opcode::Phi, Opcode::Intrinsic, Opcode::Store, Opcode::Load, Opcode::LoadCompressedStringChar});
}

void GraphChecker::VisitRefTypeCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO((inst->GetType() == DataType::REFERENCE),
              (std::cerr << "Types of RefTypeCheck must be REFERENCE\n", inst->Dump(&std::cerr)));
    for (unsigned i = 0; i < 2U; i++) {
        [[maybe_unused]] auto op = inst->GetInputs()[i].GetInst();
        [[maybe_unused]] auto op_type = op->GetType();
        ASSERT_DO((op_type == DataType::REFERENCE),
                  (std::cerr << "Types of " << i << " input RefTypeCheck must be REFERENCE\n", inst->Dump(&std::cerr),
                   op->Dump(&std::cerr)));
    }
    CheckThrows(inst, {Opcode::StoreArray, Opcode::StoreArrayPair, Opcode::StoreArrayI, Opcode::StoreArrayPairI});
}

void GraphChecker::VisitNegativeCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto op_type = op->GetType();
    ASSERT_DO(DataType::GetCommonType(op_type) == DataType::INT64,
              (std::cerr << "Type of NegativeCheck ZeroCheck must be integer\n", inst->Dump(&std::cerr)));
    CheckThrows(inst, {Opcode::NewArray, Opcode::MultiArray, Opcode::Phi});
}

void GraphChecker::VisitZeroCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto op_type = op->GetType();
    ASSERT_DO(DataType::GetCommonType(op_type) == DataType::INT64,
              (std::cerr << "Type of ZeroCheck input must be integer\n", inst->Dump(&std::cerr)));
    CheckThrows(inst, {Opcode::Div, Opcode::Mod, Opcode::Phi});
}

void GraphChecker::VisitDeoptimizeIf([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto op = inst->GetInput(0).GetInst();
    ASSERT_DO(op->GetType() == DataType::BOOL || op->IsBoolConst(),
              (std::cerr << "Type of first input DeoptimizeIf must be BOOL:\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInput(1).GetInst();
    ASSERT_DO(
        ss->GetOpcode() == Opcode::SaveStateDeoptimize || ss->GetOpcode() == Opcode::SaveState,
        (std::cerr << "Second input DeoptimizeIf must be SaveStateDeoptimize or SaveState:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitLenArray([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(inst->GetType() == DataType::INT32,
              (std::cerr << "Type of LenArray must be INT32:\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    if (op->GetOpcode() == Opcode::NullCheck) {
        op = op->GetInput(0).GetInst();
    }
    ASSERT_DO(DataType::IsReference(op->GetType()), (std::cerr << "Types of input LenArray must be REFERENCE:\n",
                                                     inst->Dump(&std::cerr), op->Dump(&std::cerr)));
}

void GraphChecker::VisitiUnresolvedCallStatic([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(inst->CastToUnresolvedCallStatic()->GetCallMethod() != nullptr,
              (std::cerr << "UnresolvedCallStatic must have non-null MethodPtr", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInputs()[inst->GetInputsCount() - 1].GetInst();
    ASSERT_DO(ss->GetOpcode() == Opcode::SaveState,
              (std::cerr << "UnresolvedCallStatic instruction last operand is not a SaveState", inst->Dump(&std::cerr),
               ss->Dump(&std::cerr)));
}

void GraphChecker::VisitCallVirtual([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(inst->GetInputs().Size() > 0,
              (std::cerr << "Virtual function must have inputs:\n", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    ASSERT_DO(DataType::IsReference(op->GetType()),
              (std::cerr << "Types of first input CallVirtual must be REFERENCE(this):\n", inst->Dump(&std::cerr),
               op->Dump(&std::cerr)));
}

void GraphChecker::VisitUnresolvedCallVirtual([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT_DO(inst->GetInputs().Size() > 0,
              (std::cerr << "Virtual function must have inputs:\n", inst->Dump(&std::cerr)));
    ASSERT_DO(inst->CastToUnresolvedCallVirtual()->GetCallMethod() != nullptr,
              (std::cerr << "UnresolvedCallVirtual must have non-null MethodPtr", inst->Dump(&std::cerr)));
    [[maybe_unused]] auto op = inst->GetInputs()[0].GetInst();
    ASSERT_DO(DataType::IsReference(op->GetType()),
              (std::cerr << "Types of first input UnresolvedCallVirtual must be REFERENCE(this):\n",
               inst->Dump(&std::cerr), op->Dump(&std::cerr)));
    [[maybe_unused]] auto ss = inst->GetInputs()[inst->GetInputsCount() - 1].GetInst();
    ASSERT_DO(ss->GetOpcode() == Opcode::SaveState,
              (std::cerr << "UnresolvedCallVirtual instruction last operand is not a SaveState", inst->Dump(&std::cerr),
               ss->Dump(&std::cerr)));
}

void GraphChecker::VisitCallDynamic([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
              (std::cerr << "CallDynamic is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitSaveState([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO((static_cast<SaveStateInst *>(inst))->Verify(), std::cerr << "Inconsistent SaveState instruction:\n"
                                                                        << *inst << std::endl);
#ifndef NDEBUG
    auto ss = inst->CastToSaveState();
    if (ss->GetInputsWereDeleted()) {
        for (auto &user : inst->GetUsers()) {
            ASSERT_DO(!user.GetInst()->RequireRegMap(),
                      std::cerr << "Some inpust from save_state were deleted, but the user requireRegMap:\n"
                                << *inst << std::endl
                                << *(user.GetInst()) << std::endl);
        }
    }
#endif
}

void GraphChecker::VisitSafePoint([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(!inst->HasUsers(), std::cerr << "SafePoint must not have users:\n" << *inst << std::endl);
    ASSERT_DO((static_cast<SaveStateInst *>(inst))->Verify(), std::cerr << "Inconsistent SafePoint instruction:\n"
                                                                        << *inst << std::endl);
}

void GraphChecker::VisitSaveStateOsr([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(!inst->HasUsers(), std::cerr << "SafeStateOsr must not have users:\n" << *inst << std::endl);
    ASSERT_DO((static_cast<SaveStateInst *>(inst))->Verify(), std::cerr << "Inconsistent SafeStateOsr instruction:\n"
                                                                        << *inst << std::endl);
    ASSERT_DO(static_cast<GraphChecker *>(v)->GetGraph()->IsOsrMode(),
              std::cerr << "SafeStateOsr must be created in the OSR mode only\n");
    ASSERT_DO(inst->GetBasicBlock()->IsOsrEntry(), std::cerr << "SafeStateOsr's basic block must be osr-entry\n");
    auto first_inst = inst->GetBasicBlock()->GetFirstInst();
    while (first_inst != nullptr && (first_inst->IsCatchPhi() || first_inst->GetOpcode() == Opcode::Try)) {
        first_inst = first_inst->GetNext();
    }
    ASSERT_DO(first_inst == inst, std::cerr << "SafeStateOsr must be the first instruction in the basic block\n");
}

void GraphChecker::VisitThrow([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(DataType::IsReference(inst->GetInput(0).GetInst()->GetType()),
              std::cerr << "Throw instruction must have input with reference type: " << *inst << std::endl);
    [[maybe_unused]] auto bb = inst->GetBasicBlock();
    ASSERT_DO(inst == bb->GetLastInst(),
              std::cerr << "Throw instruction must be last instruction in the basic block: " << *inst << std::endl);
    for ([[maybe_unused]] auto succ : bb->GetSuccsBlocks()) {
        ASSERT_DO(succ->IsEndBlock() || succ->IsTryEnd(),
                  std::cerr << "Throw block must have end block or try-end block as successor\n");
    }
}

void GraphChecker::VisitCheckCast([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(DataType::IsReference(inst->GetInput(0).GetInst()->GetType()),
              std::cerr << "CheckCast instruction must have input 0 with reference type: " << *inst << std::endl);

    ASSERT_DO(DataType::IsReference(inst->GetInput(1).GetInst()->GetType()),
              std::cerr << "CheckCast instruction must have input 1 with reference type: " << *inst << std::endl);

    [[maybe_unused]] auto save_state = inst->GetInput(2).GetInst();
    ASSERT_DO((save_state != nullptr && save_state->GetOpcode() == Opcode::SaveState),
              std::cerr << "CheckCast instruction must have SaveState as input 2: " << *inst << std::endl);
}

void GraphChecker::VisitIsInstance([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(DataType::IsReference(inst->GetInput(0).GetInst()->GetType()),
              std::cerr << "IsInstance instruction must have input 0 with reference type: " << *inst << std::endl);
    ASSERT_DO(DataType::IsReference(inst->GetInput(1).GetInst()->GetType()),
              std::cerr << "IsInstance instruction must have input 1 with reference type: " << *inst << std::endl);

    [[maybe_unused]] auto save_state = inst->GetInput(2).GetInst();
    ASSERT_DO((save_state != nullptr && save_state->GetOpcode() == Opcode::SaveState),
              std::cerr << "IsInstance instruction must have SaveState as input 2: " << *inst << std::endl);

    ASSERT_DO(inst->GetType() == DataType::BOOL,
              (std::cerr << "Types of IsInstance must be BOOL:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitSelect([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    [[maybe_unused]] auto op0 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op1 = inst->GetInput(1).GetInst();
    [[maybe_unused]] auto op2 = inst->GetInput(2U).GetInst();
    [[maybe_unused]] auto op3 = inst->GetInput(3U).GetInst();

    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        ASSERT_DO(inst->GetInputType(i) != DataType::NO_TYPE,
                  std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }

    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64 || inst->GetType() == DataType::REFERENCE ||
                  inst->GetType() == DataType::ANY,
              (std::cerr << "Select instruction type is not integer or reference or any", inst->Dump(&std::cerr)));
    ASSERT_DO(DataType::GetCommonType(op0->GetType()) == DataType::INT64 || op0->GetType() == DataType::REFERENCE ||
                  op0->GetType() == DataType::ANY,
              (std::cerr << "Select instruction 1st operand type is not integer or reference or any",
               inst->Dump(&std::cerr)));
    ASSERT_DO(DataType::GetCommonType(op1->GetType()) == DataType::INT64 || op1->GetType() == DataType::REFERENCE ||
                  op1->GetType() == DataType::ANY,
              (std::cerr << "Select instruction 2nd operand type is not integer or reference or any",
               inst->Dump(&std::cerr)));

    ASSERT_DO(CheckCommonTypes(op0, op1),
              (std::cerr << "Types of two first select instruction operands are not compatible\n",
               op0->Dump(&std::cerr), op1->Dump(&std::cerr), inst->Dump(&std::cerr)));
    ASSERT_DO(
        CheckCommonTypes(inst, op0),
        (std::cerr << "Types of instruction result and its operands are not compatible\n", inst->Dump(&std::cerr)));

    ASSERT_DO(inst->GetInputType(2U) == inst->GetInputType(3U),
              std::cerr << "Select comparison arguments has different inputs type: " << *inst << std::endl);
    if (inst->GetInputType(2U) == DataType::REFERENCE) {
        [[maybe_unused]] auto cc = inst->CastToSelect()->GetCc();
        ASSERT_DO(cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
                  (std::cerr << "Select reference comparison must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op2->IsConst()) {
            ASSERT_DO(IsZeroConstant(op2), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op2->GetType() == DataType::REFERENCE,
                      (std::cerr << "Select instruction 3rd operand type is not a reference\n", inst->Dump(&std::cerr),
                       op1->Dump(&std::cerr)));
        }
        if (op3->IsConst()) {
            ASSERT_DO(IsZeroConstant(op3), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op3->GetType() == DataType::REFERENCE,
                      (std::cerr << "Select instruction 4th operand type is not a reference\n", inst->Dump(&std::cerr),
                       op2->Dump(&std::cerr)));
        }
    }
}

void GraphChecker::VisitSelectImm([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    [[maybe_unused]] auto op0 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op1 = inst->GetInput(1).GetInst();
    [[maybe_unused]] auto op2 = inst->GetInput(2U).GetInst();
    [[maybe_unused]] auto op3 = inst->CastToSelectImm()->GetImm();
    [[maybe_unused]] bool is_dynamic = static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod();

    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        ASSERT_DO(inst->GetInputType(i) != DataType::NO_TYPE,
                  std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }

    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64 || inst->GetType() == DataType::REFERENCE ||
                  (is_dynamic && inst->GetType() == DataType::ANY),
              (std::cerr << "SelectImm instruction type is not integer or reference or any", inst->Dump(&std::cerr)));
    ASSERT_DO(DataType::GetCommonType(op0->GetType()) == DataType::INT64 || op0->GetType() == DataType::REFERENCE ||
                  (is_dynamic && op0->GetType() == DataType::ANY),
              (std::cerr << "SelectImm instruction 1st operand type is not integer or reference or any",
               inst->Dump(&std::cerr)));
    ASSERT_DO(DataType::GetCommonType(op1->GetType()) == DataType::INT64 || op1->GetType() == DataType::REFERENCE ||
                  (is_dynamic && op1->GetType() == DataType::ANY),
              (std::cerr << "SelectImm instruction 2nd operand type is not integer or reference or any",
               inst->Dump(&std::cerr)));

    ASSERT_DO(CheckCommonTypes(op0, op1),
              (std::cerr << "Types of two first SelectImm instruction operands are not compatible\n",
               op0->Dump(&std::cerr), op1->Dump(&std::cerr), inst->Dump(&std::cerr)));
    ASSERT_DO(
        CheckCommonTypes(inst, op0),
        (std::cerr << "Types of instruction result and its operands are not compatible\n", inst->Dump(&std::cerr)));

    if (inst->GetInputType(2U) == DataType::REFERENCE) {
        [[maybe_unused]] auto cc = inst->CastToSelectImm()->GetCc();
        ASSERT_DO(cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
                  (std::cerr << "SelectImm reference comparison must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op2->IsConst()) {
            ASSERT_DO(IsZeroConstant(op2), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op2->GetType() == DataType::REFERENCE,
                      (std::cerr << "Condition with immediate jump 1st operand type is not a reference\n",
                       inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        }
        ASSERT_DO(op3 == 0,
                  (std::cerr << "Reference can be compared only with 0 immediate: \n", inst->Dump(&std::cerr)));
    } else {
        ASSERT_PRINT(DataType::GetCommonType(op2->GetType()) == DataType::INT64 ||
                         (is_dynamic && DataType::GetCommonType(op2->GetType()) == DataType::ANY),
                     "SelectImm 3rd operand type is not an integer or any");

        if (DataType::GetCommonType(op2->GetType()) == DataType::ANY) {
            [[maybe_unused]] auto cc = inst->CastToSelectImm()->GetCc();
            ASSERT_DO(cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
                      (std::cerr << "SelectImm any comparison must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        }
    }
}

void GraphChecker::VisitIf([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckContrlFlowInst(inst);
    [[maybe_unused]] auto num_succs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == MAX_SUCCS_NUM, "Basic block with If must have 2 successesors");

    [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
    [[maybe_unused]] auto op2 = inst->GetInputs()[1].GetInst();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        ASSERT_DO(inst->GetInputType(i) != DataType::NO_TYPE,
                  std::cerr << "Source operand type is not set: " << *inst << std::endl);
    }
    ASSERT_DO(inst->GetInputType(0) == inst->GetInputType(1),
              std::cerr << "If has different inputs type: " << *inst << std::endl);
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        [[maybe_unused]] auto cc = inst->CastToIf()->GetCc();
        ASSERT_DO(cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
                  (std::cerr << "Reference comparison in If must be CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op1->IsConst()) {
            ASSERT_DO(IsZeroConstant(op1), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op1->GetType() == DataType::REFERENCE, (std::cerr << "If 1st operand type is not a reference\n",
                                                              inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        }
        if (op2->IsConst()) {
            ASSERT_DO(IsZeroConstant(op2), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op2->GetType() == DataType::REFERENCE, (std::cerr << "If 2nd operand type is not a reference\n",
                                                              inst->Dump(&std::cerr), op2->Dump(&std::cerr)));
        }
    }
}

void GraphChecker::VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckContrlFlowInst(inst);
    [[maybe_unused]] auto num_succs = inst->GetBasicBlock()->GetSuccsBlocks().size();
    ASSERT_PRINT(num_succs == MAX_SUCCS_NUM, "Basic block with IfImm must have 2 successesors");

    [[maybe_unused]] auto op1 = inst->GetInput(0).GetInst();
    [[maybe_unused]] auto op2 = inst->CastToIfImm()->GetImm();
    ASSERT_DO(inst->GetInputType(0) != DataType::NO_TYPE,
              std::cerr << "Source operand type is not set: " << *inst << std::endl);
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        [[maybe_unused]] auto cc = inst->CastToIfImm()->GetCc();
        ASSERT_DO(cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ,
                  (std::cerr << "Reference comparison in IfImm must have CC_NE or CC_EQ: \n", inst->Dump(&std::cerr)));
        if (op1->IsConst()) {
            ASSERT_DO(IsZeroConstant(op1), (std::cerr << "Constant reference input must be integer 0: \n",
                                            inst->Dump(&std::cerr), op1->Dump(&std::cerr)));
        } else {
            ASSERT_DO(op1->GetType() == DataType::REFERENCE,
                      (std::cerr << "IfImm operand type should be here a reference: \n", inst->Dump(&std::cerr),
                       op1->Dump(&std::cerr)));
        }
        ASSERT_DO(op2 == 0,
                  (std::cerr << "Reference can be compared only with 0 immediate: \n", inst->Dump(&std::cerr)));
    } else {
        ASSERT_PRINT(
            DataType::GetCommonType(op1->GetType()) == DataType::INT64 ||
                (static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod() && op1->GetType() == DataType::ANY),
            "IfImm operand type should be here an integer");
    }
}

void GraphChecker::VisitTry([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto bb = inst->GetBasicBlock();
    ASSERT_PRINT(bb->IsTryBegin(), "TryInst should be placed in the try-begin basic block");
}

void GraphChecker::VisitNOP([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_PRINT(inst->GetUsers().Empty(), "NOP can not have users\n");
}

void GraphChecker::VisitAndNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}
void GraphChecker::VisitOrNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}
void GraphChecker::VisitXorNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, true);
}
void GraphChecker::VisitMNeg([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckBinaryOperationTypes(inst, false);
}
void GraphChecker::VisitMAdd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckTernaryOperationTypes(inst);
}
void GraphChecker::VisitMSub([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    CheckTernaryOperationTypes(inst);
}

void GraphChecker::VisitCompareAnyType([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
              (std::cerr << "CompareAnyType is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitCastAnyTypeValue([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
              (std::cerr << "CastAnyTypeValue is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
}

void GraphChecker::VisitCastValueToAnyType([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
              (std::cerr << "CastValueToAnyType is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));

    const auto *input_inst = inst->GetInput(0).GetInst();
    auto input_type = inst->GetInputType(0);
    auto output_type = AnyBaseTypeToDataType(inst->CastToCastValueToAnyType()->GetAnyType());

    ASSERT_DO(input_type != DataType::ANY,
              (std::cerr << "CastValueToAnyType cannot accept inputs of ANY type:\n", inst->Dump(&std::cerr)));

    if (input_inst->IsConst() && (input_type == DataType::Type::INT64 || input_type == DataType::Type::INT32)) {
        if (output_type == DataType::Type::BOOL) {
            ASSERT_DO(input_inst->IsBoolConst(),
                      (std::cerr << "Integral constant input not coercible to BOOL:\n", inst->Dump(&std::cerr)));
            return;
        }

        if (output_type == DataType::INT32 && input_type == DataType::INT64) {
            [[maybe_unused]] int64_t value = input_inst->CastToConstant()->GetInt64Value();
            ASSERT_DO(value == static_cast<int32_t>(value),
                      (std::cerr << "Integral constant input not coercible to INT32:\n", inst->Dump(&std::cerr)));
            return;
        }

        if (output_type == DataType::Type::REFERENCE) {
            return;  // Always coercible
        }

        if (output_type == DataType::Type::VOID) {
            return;  // Always coercible
        }

        // Otherwise proceed with the generic check.
    }
}

void GraphChecker::VisitAnyTypeCheck([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(static_cast<GraphChecker *>(v)->GetGraph()->IsDynamicMethod(),
              (std::cerr << "AnyTypeCheck is supported only for dynamic languages:\n", inst->Dump(&std::cerr)));
    ASSERT_DO(inst->GetInput(0).GetInst()->GetType() == DataType::Type::ANY,
              (std::cerr << "First input in AnyTypeCheck must be Any type:\n", inst->Dump(&std::cerr)));
    ASSERT_DO(inst->GetInput(1).GetInst()->IsSaveState(),
              (std::cerr << "Second input in AnyTypeCheck must be SaveState:\n", inst->Dump(&std::cerr)));
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define VisitBinaryShiftedRegister(opc)                                                         \
    void GraphChecker::Visit##opc(GraphVisitor *v, Inst *inst)                                  \
    {                                                                                           \
        CheckBinaryOperationWithShiftedOperandTypes(                                            \
            v, inst, inst->GetOpcode() != Opcode::AddSR && inst->GetOpcode() != Opcode::SubSR); \
    }

VisitBinaryShiftedRegister(AddSR) VisitBinaryShiftedRegister(SubSR) VisitBinaryShiftedRegister(AndSR)
    VisitBinaryShiftedRegister(OrSR) VisitBinaryShiftedRegister(XorSR) VisitBinaryShiftedRegister(AndNotSR)
        VisitBinaryShiftedRegister(OrNotSR) VisitBinaryShiftedRegister(XorNotSR)
#undef VisitBinaryShiftedRegister

            void GraphChecker::VisitNegSR([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
              (std::cerr << "NegSR must have integer type\n", inst->Dump(&std::cerr)));
    CheckUnaryOperationTypes(inst);
    [[maybe_unused]] auto shift_type = static_cast<UnaryShiftedRegisterOperation *>(inst)->GetShiftType();
    ASSERT_DO(shift_type != ShiftType::INVALID_SHIFT && shift_type != ShiftType::ROR,
              (std::cerr << "Operation has invalid shift type\n", inst->Dump(&std::cerr)));
}

}  // namespace panda::compiler
