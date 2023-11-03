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

#ifndef COMPILER_OPTIMIZER_IR_GRAPH_CLONER_H_
#define COMPILER_OPTIMIZER_IR_GRAPH_CLONER_H_

#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "utils/arena_containers.h"
#include "utils/hash.h"

namespace panda::compiler {
class BasicBlock;
class Graph;
class Inst;

// NOLINTNEXTLINE(readability-redundant-declaration)
bool IsLoopSingleBackEdgeExitPoint(Loop *loop);

enum UnrollType : uint8_t {
    UNROLL_WITH_SIDE_EXITS = 0,
    UNROLL_WITHOUT_SIDE_EXITS = 1,
    UNROLL_REMOVE_BACK_EDGE = 2,
    UNROLL_POST_INCREMENT = UNROLL_REMOVE_BACK_EDGE,
    UNROLL_CONSTANT_ITERATIONS = UNROLL_WITHOUT_SIDE_EXITS | UNROLL_REMOVE_BACK_EDGE
};

enum class InstCloneType : uint8_t { CLONE_ALL, CLONE_INSTS };

enum class CloneEdgeType : uint8_t { EDGE_PRED, EDGE_SUCC };

/**
 * Helper-class, provides methods to:
 * - Clone the whole graph;
 * - Clone loop;
 * - Unroll loop;
 * - Peel loop;
 */
class GraphCloner {
    using PhiInputsMap = ArenaUnorderedMap<Inst *, Inst *>;

    struct LoopUnrollData {
        BasicBlock *header {nullptr};
        BasicBlock *backedge {nullptr};
        BasicBlock *exit_block {nullptr};
        BasicBlock *outer {nullptr};
        ArenaVector<BasicBlock *> *blocks {nullptr};
        InstVector *phi_update_inputs {nullptr};
        PhiInputsMap *phi_replaced_inputs {nullptr};
    };

protected:
    struct LoopClonerData {
        BasicBlock *outer {nullptr};
        BasicBlock *header {nullptr};
        BasicBlock *pre_header {nullptr};
        ArenaVector<BasicBlock *> *blocks {nullptr};
    };

public:
    explicit GraphCloner(Graph *graph, ArenaAllocator *allocator, ArenaAllocator *local_allocator);

    Graph *CloneGraph();
    BasicBlock *CloneLoopHeader(BasicBlock *block, BasicBlock *outer, BasicBlock *replaceable_pred);
    Loop *CloneLoop(Loop *loop);
    bool IsLoopClonable(Loop *loop, size_t inst_limit);

    /**
     * Make equal to the `factor` number of clones of loop body and insert them into the graph
     *
     *      /----[pre-loop]
     *      |        |
     *      |        v
     *      |    [loop-body]<----\
     *      |        |   |       |
     *      |        |   \-------/
     *      |        |
     *      |        v
     *      \--->[outside-block]
     *               |
     *               v
     *              ...
     *
     * Cloning with side-exits:
     *
     *      /----[pre-loop]
     *      |        |
     *      |        v
     *      |   [loop-body]---------\
     *      |        |              |
     *      |        v              |
     *      |   [loop-body']------->|
     *      |        |              |
     *      |        v              |
     *      |   [loop-body'']------>|
     *      |        |              |
     *      |        v              |
     *      |  [resolver-block]<----/
     *      |        |
     *      |        v
     *      \--->[outside-block]
     *              ...
     *
     *  Cloning without side-exits:
     *
     *      /----[pre-loop]
     *      |        |
     *      |        v
     *      |    [loop-body]
     *      |        |
     *      |        v
     *      |    [loop-body']
     *      |        |
     *      |        v
     *      |    [loop-body'']
     *      |        |
     *      |        v
     *      \--->[outside-block]
     */
    template <UnrollType TYPE>
    void UnrollLoopBody(Loop *loop, size_t factor)
    {
        ASSERT_PRINT(IsLoopSingleBackEdgeExitPoint(loop), "Cloning blocks doesn't have single entry/exit point");
        auto marker_holder = MarkerHolder(GetGraph());
        clone_marker_ = marker_holder.GetMarker();
        auto unroll_data = PrepareLoopToUnroll(loop, (TYPE & UnrollType::UNROLL_WITHOUT_SIDE_EXITS) == 0);

        auto clone_count = factor - 1;
        for (size_t i = 0; i < clone_count; i++) {
            CloneBlocksAndInstructions<InstCloneType::CLONE_ALL, true>(*unroll_data->blocks, GetGraph());
            BuildLoopUnrollControlFlow(unroll_data);
            // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
            if constexpr ((TYPE & UnrollType::UNROLL_WITHOUT_SIDE_EXITS) != 0) {
                // Users update should be done on the last no-side-exits unroll iteration
                // before building loop data-flow
                if (i + 1 == clone_count) {  // last_iteration
                    UpdateUsersAfterNoSideExitsUnroll(unroll_data);
                }
            }
            BuildLoopUnrollDataFlow(unroll_data);
        }

        // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
        if constexpr ((TYPE & UnrollType::UNROLL_REMOVE_BACK_EDGE) != 0) {
            RemoveLoopBackEdge(unroll_data);
        }
        // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
        if constexpr (TYPE == UnrollType::UNROLL_CONSTANT_ITERATIONS) {
            RemoveLoopPreHeader(unroll_data);
        }
        ssb_.FixPhisWithCheckInputs(unroll_data->outer);
    }

protected:
    Graph *GetGraph()
    {
        return graph_;
    }

    BasicBlock *GetClone(const BasicBlock *block)
    {
        ASSERT(block != nullptr);
        ASSERT_PRINT(block->GetGraph() == GetGraph(), "GraphCloner probably caught disconnected block");
        ASSERT_DO(HasClone(block), block->Dump(&std::cerr));
        return clone_blocks_[block->GetId()];
    }

    template <CloneEdgeType EDGE_TYPE>
    void CloneEdges(BasicBlock *block)
    {
        auto clone = GetClone(block);
        auto block_edges = &block->GetPredsBlocks();
        auto clone_edges = &clone->GetPredsBlocks();
        // NOLINTNEXTLINE(bugprone-suspicious-semicolon, readability-braces-around-statements)
        if constexpr (EDGE_TYPE == CloneEdgeType::EDGE_SUCC) {
            block_edges = &block->GetSuccsBlocks();
            clone_edges = &clone->GetSuccsBlocks();
        }
        ASSERT(clone_edges->empty());
        clone_edges->reserve(block_edges->size());
        for (auto edge : *block_edges) {
            clone_edges->push_back(GetClone(edge));
        }
    }

    /**
     * For each block of input vector create a new one empty block and populate it with the instructions,
     * cloned form the original block
     */
    template <InstCloneType TYPE, bool SKIP_SAFEPOINTS>
    void CloneBlocksAndInstructions(const ArenaVector<BasicBlock *> &blocks, Graph *target_graph)
    {
        clone_blocks_.clear();
        clone_blocks_.resize(GetGraph()->GetVectorBlocks().size(), nullptr);
        clone_instructions_.clear();
        size_t inst_count = 0;
        for (const auto &block : blocks) {
            if (block != nullptr) {
                auto clone = block->Clone(target_graph);
                clone_blocks_[block->GetId()] = clone;
                CloneInstructions<TYPE, SKIP_SAFEPOINTS>(block, clone, &inst_count);
                if (block->IsTryBegin()) {
                    target_graph->AppendTryBeginBlock(clone);
                }
            }
        }
    }

    void MakeLoopCloneInfo(LoopClonerData *unroll_data);
    BasicBlock *CreateNewOutsideSucc(BasicBlock *outside_succ, BasicBlock *back_edge, BasicBlock *pre_header);

    /**
     * Use the following rules cloning the inputs:
     * - if input of the original instruction has clone - insert this clone as input
     * - otherwise - use original input as clone instruction input
     */
    template <bool REPLACE_HEADER_PHI>
    void SetCloneInputs(Inst *inst, BasicBlock *block = nullptr)
    {
        auto clone = GetClone(inst);
        for (size_t i = 0; i < inst->GetInputsCount(); i++) {
            auto input = inst->GetInput(i).GetInst();
            if (REPLACE_HEADER_PHI && IsInstLoopHeaderPhi(input, inst->GetBasicBlock()->GetLoop())) {
                ASSERT(block != nullptr);
                input = input->CastToPhi()->GetPhiInput(block);
            } else if (HasClone(input)) {
                input = GetClone(input);
            }

            if (clone->IsOperandsDynamic()) {
                clone->AppendInput(input);
                if (clone->IsSaveState()) {
                    static_cast<SaveStateInst *>(clone)->SetVirtualRegister(
                        i, static_cast<SaveStateInst *>(inst)->GetVirtualRegister(i));
                } else if (clone->IsPhi()) {
                    clone->CastToPhi()->SetPhiInputBbNum(i, inst->CastToPhi()->GetPhiInputBbNum(i));
                }
            } else {
                clone->SetInput(i, input);
            }
        }
    }

    void UpdateCaller(Inst *inst);

    Inst *GetClone(Inst *inst)
    {
        ASSERT(inst != nullptr);
        ASSERT(HasClone(inst));

        // We don't use clone_marker_ when cloning the whole graph, so lets at least check the basic block here
        ASSERT(inst->GetBasicBlock() != nullptr);
        ASSERT_PRINT(inst->GetBasicBlock()->GetGraph() == GetGraph(),
                     "GraphCloner probably caught an instruction from disconnected block");
        // Empty clone_blocks_ means we are cloning only one basic block
        ASSERT(clone_blocks_.empty() || HasClone(inst->GetBasicBlock()));

        return clone_instructions_[inst->GetCloneNumber()];
    }

private:
    // Whole graph cloning
    void CopyLoop(Loop *loop, Loop *cloned_loop);
    void CloneLinearOrder(Graph *new_graph);
    void BuildControlFlow();
    void BuildDataFlow();
    void CloneAnalyses(Graph *new_graph);
    // Loop cloning
    LoopClonerData *PrepareLoopToClone(Loop *loop);
    void BuildLoopCloneControlFlow(LoopClonerData *unroll_data);
    void BuildLoopCloneDataFlow(LoopClonerData *unroll_data);
    // Unroll cloning
    LoopUnrollData *PrepareLoopToUnroll(Loop *loop, bool clone_side_exits);
    BasicBlock *CreateResolverBlock(Loop *loop, BasicBlock *back_edge);
    BasicBlock *SplitBackEdge(LoopUnrollData *unroll_data, Loop *loop, BasicBlock *back_edge);
    void UpdateUsersAfterNoSideExitsUnroll(const LoopUnrollData *unroll_data);
    void BuildLoopUnrollControlFlow(LoopUnrollData *unroll_data);
    void BuildLoopUnrollDataFlow(LoopUnrollData *unroll_data);
    void RemoveLoopBackEdge(const LoopUnrollData *unroll_data);
    void RemoveLoopPreHeader(const LoopUnrollData *unroll_data);
    // Loop header cloning
    void BuildClonedLoopHeaderDataFlow(const BasicBlock &block, BasicBlock *resolver, BasicBlock *clone);
    void UpdateUsersForClonedLoopHeader(Inst *inst, BasicBlock *outer_block);
    // Cloned blocks and instructions getters
    bool HasClone(const BasicBlock *block)
    {
        return (block->GetId() < clone_blocks_.size()) && (clone_blocks_[block->GetId()] != nullptr);
    }

    bool HasClone(Inst *inst)
    {
        return inst->IsMarked(clone_marker_) && (inst->GetCloneNumber() < clone_instructions_.size());
    }

    /// Clone block's instructions and append to the block's clone
    template <InstCloneType TYPE, bool SKIP_SAFEPOINTS>
    void CloneInstructions(const BasicBlock *block, BasicBlock *clone, size_t *inst_count)
    {
        for (auto inst : block->Insts()) {
            if constexpr (SKIP_SAFEPOINTS) {  // NOLINT
                if (inst->GetOpcode() == Opcode::SafePoint) {
                    continue;
                }
            }
            if constexpr (TYPE != InstCloneType::CLONE_ALL) {  // NOLINT
                if (inst->GetOpcode() == Opcode::NOP) {
                    continue;
                }
            }

            clone->AppendInst(CloneInstruction(inst, inst_count, clone->GetGraph()));
        }

        if constexpr (TYPE == InstCloneType::CLONE_ALL) {  // NOLINT
            for (auto phi : block->PhiInsts()) {
                auto phi_clone = CloneInstruction(phi, inst_count, clone->GetGraph());
                clone->AppendPhi(phi_clone->CastToPhi());
            }
        }
    }

    /// Clone instruction and mark both: clone and cloned
    Inst *CloneInstruction(Inst *inst, size_t *inst_count, Graph *target_graph)
    {
        inst->SetCloneNumber((*inst_count)++);
        auto inst_clone = inst->Clone(target_graph);
        clone_instructions_.push_back(inst_clone);
        inst->SetMarker(clone_marker_);
        inst_clone->SetMarker(clone_marker_);
        if (inst->GetBasicBlock()->GetGraph() != target_graph) {
            inst_clone->SetId(inst->GetId());
        }
        return inst_clone;
    }

    bool IsInstLoopHeaderPhi(Inst *inst, Loop *loop);

protected:
    Marker clone_marker_ {UNDEF_MARKER};  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Graph *graph_;
    ArenaAllocator *allocator_;
    ArenaAllocator *local_allocator_;
    ArenaVector<BasicBlock *> clone_blocks_;
    InstVector clone_instructions_;
    SaveStateBridgesBuilder ssb_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_GRAPH_CLONER_H_
