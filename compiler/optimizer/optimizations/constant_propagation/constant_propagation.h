/*
 * Copyright (c) 2024 Shenzhen Kaihong Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_SCCP_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_SCCP_H

#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "compiler/optimizer/pass.h"
#include "lattice_element.h"
#include "utils/hash.h"

namespace panda::compiler {
class ConstantPropagation : public Optimization, public GraphVisitor {
public:
    explicit ConstantPropagation(Graph *graph);
    NO_MOVE_SEMANTIC(ConstantPropagation);
    NO_COPY_SEMANTIC(ConstantPropagation);
    ~ConstantPropagation() override = default;

    using Edge = std::pair<BasicBlock *, BasicBlock *>;
    struct EdgeHash {
        uint32_t operator()(Edge const &edge) const
        {
            auto block_hash = std::hash<BasicBlock *> {};
            uint32_t lhash = block_hash(edge.first);
            uint32_t rhash = block_hash(edge.second);
            return merge_hashes(lhash, rhash);
        }
    };

    struct EdgeEqual {
        bool operator()(Edge const &edge1, Edge const &edge2) const
        {
            return edge1.first == edge2.first && edge1.second == edge2.second;
        }
    };

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "SCCP";
    }

    bool IsEnable() const override
    {
        return true;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

protected:
    void VisitDefault(Inst *inst) override;
    static void VisitPhi(GraphVisitor *v, Inst *inst);
    static void VisitIfImm(GraphVisitor *v, Inst *inst);
    static void VisitCompare(GraphVisitor *v, Inst *inst);
    static void VisitConstant(GraphVisitor *v, Inst *inst);
    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst);

#include "compiler/optimizer/ir/visitor.inc"

private:
    void RunSsaEdge();
    void ReWriteInst();
    void RunFlowEdge();
    void VisitInsts(BasicBlock *bb);
    void AddUntraversedFlowEdges(BasicBlock *src, BasicBlock *dst);
    bool GetIfTargetBlock(IfImmInst *inst, uint64_t val);
    LatticeElement *GetOrCreateDefaultLattice(Inst *inst);
    LatticeElement *FoldingConstant(RuntimeInterface::IntrinsicId id);
    LatticeElement *FoldingConstant(ConstantElement *lattice, RuntimeInterface::IntrinsicId id);
    LatticeElement *FoldingConstant(ConstantElement *left, ConstantElement *right, RuntimeInterface::IntrinsicId id);
    LatticeElement *FoldingCompare(const ConstantElement *left, const ConstantElement *right, ConditionCode cc);

    LatticeElement *FoldingGreater(const ConstantElement *left, const ConstantElement *right, bool equal = false);
    LatticeElement *FoldingLess(const ConstantElement *left, const ConstantElement *right, bool equal = false);
    LatticeElement *FoldingEq(const ConstantElement *left, const ConstantElement *right);
    LatticeElement *FoldingNotEq(const ConstantElement *left, const ConstantElement *right);

    Inst *CreateReplaceInst(Inst *base_inst, ConstantElement *lattice);
    void InsertSaveState(Inst *base_inst, Inst *save_state);
    void CheckAndAddToSsaEdge(Inst *inst, LatticeElement *current, LatticeElement *new_lattice);

private:
    ArenaUnorderedMap<Inst *, LatticeElement *> lattices_;
    ArenaQueue<Edge> flow_edges_;
    ArenaQueue<Inst *> ssa_edges_;
    ArenaUnorderedSet<Edge, EdgeHash, EdgeEqual> executable_flag_;
    ArenaUnorderedSet<uint32_t> executed_node_;
};
}  // namespace panda::compiler
#endif
