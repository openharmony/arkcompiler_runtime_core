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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"

/**
 * See  "Efficient Field-sensitive pointer analysis for C" by "David J. Pearce
 * and Paul H. J. Kelly and Chris Hankin
 *
 * We treat each IR Inst as a constraint that may be applied to a set of
 * aliases of some virtual register.  Virtual registers are used as constraint
 * variables as well.
 *
 * In order to solve the system of set constraints, the following is done:
 *
 * 1. Each constraint variable x has a solution set associated with it, Sol(x).
 * Implemented through AliasAnalysis::points_to_ that contains mapping of
 * virtual register to possible aliases.
 *
 * 2. Constraints are separated into direct, copy.
 *
 * - Direct constraints are constraints that require no extra processing, such
 * as P = &Q.
 *
 * - Copy constraints are those of the form P = Q.  Such semantic can be
 * obtained through NullCheck, Mov, and Phi instructions.
 *
 * 3. All direct constraints of the form P = &Q are processed, such that Q is
 * added to Sol(P).
 *
 * 4. A directed graph is built out of the copy constraints.  Each constraint
 * variable is a node in the graph, and an edge from Q to P is added for each
 * copy constraint of the form P = Q.
 *
 * 5. The graph is then walked, and solution sets are propagated along the copy
 * edges, such that an edge from Q to P causes Sol(P) <- Sol(P) union Sol(Q).
 *
 * 6. The process of walking the graph is iterated until no solution sets
 * change.
 *
 * To add new instructions to alias analysis please consider following:
 *     - AliasAnalysis class: to add a visitor for a new instruction that should be analyzed
 *
 * TODO(Evgenii Kudriashov): Prior to walking the graph in steps 5 and 6, We
 * need to perform static cycle elimination on the constraint graph, as well as
 * off-line variable substitution.
 *
 * TODO(Evgenii Kudriashov): To add flow-sensitivity the "Flow-sensitive
 * pointer analysis for millions of lines of code" by Ben Hardekopf and Calvin
 * Lin may be considered.
 *
 * TODO(Evgenii Kudriashov): After implementing VRP and SCEV the "Loop-Oriented
 * Array- and Field-Sensitive Pointer Analysis for Automatic SIMD
 * Vectorization" by Yulei Sui, Xiaokang Fan, Hao Zhou, and Jingling Xue may be
 * considered to add advanced analysis of array indices.
 */

namespace panda::compiler {

AliasAnalysis::AliasAnalysis(Graph *graph) : Analysis(graph), points_to_(graph->GetAllocator()->Adapter()) {}

const ArenaVector<BasicBlock *> &AliasAnalysis::GetBlocksToVisit() const
{
    return GetGraph()->GetBlocksRPO();
}

bool AliasAnalysis::RunImpl()
{
    Init();

    VisitGraph();

    // Initialize solution sets
    for (auto pair : *direct_) {
        auto it = points_to_.try_emplace(pair.first, GetGraph()->GetAllocator()->Adapter());
        ASSERT(pair.first.GetBase() == nullptr || pair.first.GetBase()->GetOpcode() != Opcode::NullCheck);
        ASSERT(pair.second.GetBase() == nullptr || pair.second.GetBase()->GetOpcode() != Opcode::NullCheck);
        it.first->second.insert(pair.second);
    }

    SolveConstraints();

#ifndef NDEBUG
    if (CompilerLogger::IsComponentEnabled(CompilerLoggerComponents::ALIAS_ANALYSIS)) {
        std::ostringstream out;
        DumpChains(&out);
        Dump(&out);
        COMPILER_LOG(DEBUG, ALIAS_ANALYSIS) << out.str();
    }
#endif
    return true;
}

void AliasAnalysis::Init()
{
    auto allocator = GetGraph()->GetLocalAllocator();
    chains_ = allocator->New<PointerMap<ArenaVector<Pointer>>>(allocator->Adapter());
    direct_ = allocator->New<PointerPairVector>(allocator->Adapter());
    inputs_set_ = allocator->New<ArenaSet<Inst *>>(allocator->Adapter());
    ASSERT(chains_ != nullptr);
    ASSERT(direct_ != nullptr);
    ASSERT(inputs_set_ != nullptr);
    points_to_.clear();
}

void Pointer::Dump(std::ostream *out) const
{
    switch (type_) {
        case OBJECT:
            (*out) << "v" << base_->GetId();
            break;
        case STATIC_FIELD:
            (*out) << "SF #" << imm_;
            break;
        case POOL_CONSTANT:
            (*out) << "PC #" << imm_;
            break;
        case OBJECT_FIELD:
            (*out) << "v" << base_->GetId() << " #" << imm_;
            break;
        case ARRAY_ELEMENT:
            (*out) << "v" << base_->GetId() << "[";
            if (idx_ != nullptr) {
                (*out) << "v" << idx_->GetId();
                if (imm_ != 0) {
                    (*out) << "+" << imm_;
                }
            } else {
                (*out) << imm_;
            }
            (*out) << "]";
            break;
        default:
            UNREACHABLE();
    }
    if (local_) {
        (*out) << "(local)";
    }
    if (volatile_) {
        (*out) << "(v)";
    }
}

static bool PointerLess(const Pointer &lhs, const Pointer &rhs)
{
    if (lhs.GetBase() == rhs.GetBase()) {
        return lhs.GetImm() < rhs.GetImm();
    }
    if (lhs.GetBase() == nullptr) {
        return true;
    }
    if (rhs.GetBase() == nullptr) {
        return false;
    }
    return lhs.GetBase()->GetId() < rhs.GetBase()->GetId();
}

void AliasAnalysis::DumpChains(std::ostream *out) const
{
    ArenaVector<Pointer> sorted_keys(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto &pair : *chains_) {
        sorted_keys.push_back(pair.first);
    }
    std::sort(sorted_keys.begin(), sorted_keys.end(), PointerLess);

    (*out) << "The chains are the following:" << std::endl;
    for (auto &p : sorted_keys) {
        (*out) << "\t";
        p.Dump(out);
        (*out) << ": {";

        // Sort by instruction ID to add more readability to logs
        ArenaVector<Pointer> sorted(chains_->at(p), GetGraph()->GetLocalAllocator()->Adapter());
        std::sort(sorted.begin(), sorted.end(), PointerLess);
        auto edge = sorted.begin();
        if (edge != sorted.end()) {
            edge->Dump(out);
            while (++edge != sorted.end()) {
                (*out) << ", ";
                edge->Dump(out);
            }
        }
        (*out) << "}" << std::endl;
    }
}

void AliasAnalysis::Dump(std::ostream *out) const
{
    ArenaVector<Pointer> sorted_keys(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto &pair : points_to_) {
        sorted_keys.push_back(pair.first);
    }
    std::sort(sorted_keys.begin(), sorted_keys.end(), PointerLess);

    (*out) << "The solution set is the following:" << std::endl;
    for (auto &p : sorted_keys) {
        (*out) << "\t";
        p.Dump(out);
        (*out) << ": {";

        // Sort by instruction ID to add more readability to logs
        auto values = points_to_.at(p);
        ArenaVector<Pointer> sorted(values.begin(), values.end(), GetGraph()->GetLocalAllocator()->Adapter());
        std::sort(sorted.begin(), sorted.end(), PointerLess);
        auto iter = sorted.begin();
        if (iter != sorted.end()) {
            iter->Dump(out);
            while (++iter != sorted.end()) {
                (*out) << ", ";
                iter->Dump(out);
            }
        }
        (*out) << "}" << std::endl;
    }
}

AliasType AliasAnalysis::CheckInstAlias(Inst *mem1, Inst *mem2) const
{
    return MAY_ALIAS;
}

/**
 * Here we propagate solutions obtained from direct constraints through copy
 * constraints e.g: we have a node A with solution {a} and the node A was
 * copied to B and C (this->chains_ maintains these links), and C was copied to
 * D.
 *
 *    A{a} -> B
 *        \-> C -> D
 *
 * After first iteration (iterating A node) we will obtain
 *
 *     A{a} -> B{a}
 *         \-> C{a} -> D
 *
 * After second iteration (iterating B node) nothing changes
 *
 * After third iteration (iterating C node):
 *
 * A{a} -> B{a}
 *     \-> C{a} -> D{a}
 *
 * For complex nodes (OBJECT_FIELD, ARRAY_ELEMENT) we create auxiliary nodes e.g.
 * if a field F was accessed from object A then we have two nodes:
 *
 * A{a} -> A.F
 *
 * And solutions from A would be propagated as following:
 *
 * A{a} -> A.F{a.F}
 *
 * The function works using worklist to process only updated nodes.
 */
void AliasAnalysis::SolveConstraints()
{
    ArenaQueue<Pointer> worklist(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto &pair : *direct_) {
        if (chains_->find(pair.first) != chains_->end()) {
            worklist.push(pair.first);
        }
    }

    while (!worklist.empty()) {
        Pointer &ref = worklist.front();
        ASSERT(ref.GetBase() == nullptr || ref.GetBase()->GetOpcode() != Opcode::NullCheck);
        for (auto &edge : chains_->at(ref)) {
            // POOL_CONSTANT cannot be assignee
            ASSERT(edge.GetType() != POOL_CONSTANT);
            auto &sols = points_to_.try_emplace(edge, GetGraph()->GetAllocator()->Adapter()).first->second;
            bool added = false;
            for (auto &alias : points_to_.at(ref)) {
                ASSERT(alias.GetBase() == nullptr || alias.GetBase()->GetOpcode() != Opcode::NullCheck);
                if (edge.GetType() == OBJECT_FIELD && ref.GetBase() == edge.GetBase()) {
                    // Propagating from object to fields: A{a} -> A.F{a.f}
                    if (alias.GetType() == OBJECT) {
                        Pointer p = Pointer::CreateObjectField(alias.GetBase(), edge.GetImm(), edge.GetTypePtr());
                        p.SetLocalVolatile(alias.IsLocal(), edge.IsVolatile());

                        added |= sols.insert(p).second;
                        continue;
                    }
                    // In case A{a.g} -> A.F we propagate symbolic name: A{a.g} -> A.F{A.F}
                    Pointer p = Pointer::CreateObjectField(ref.GetBase(), edge.GetImm(), edge.GetTypePtr());
                    p.SetLocalVolatile(alias.IsLocal(), edge.IsVolatile());

                    added |= sols.insert(p).second;
                    continue;
                }
                if (edge.GetType() == ARRAY_ELEMENT && ref.GetBase() == edge.GetBase()) {
                    // Propagating from object to elements: A{a} -> A[i]{a[i]}
                    if (alias.GetType() == OBJECT) {
                        Pointer p = Pointer::CreateArrayElement(alias.GetBase(), edge.GetIdx(), edge.GetImm());
                        p.SetLocalVolatile(alias.IsLocal(), edge.IsVolatile());

                        added |= sols.insert(p).second;
                        continue;
                    }
                    // In case A{a[j]} -> A[i] we propagate symbolic name: A{a[j]} -> A[i]{A[i]}
                    Pointer p = Pointer::CreateArrayElement(ref.GetBase(), edge.GetIdx(), edge.GetImm());
                    p.SetLocalVolatile(alias.IsLocal(), edge.IsVolatile());

                    added |= sols.insert(p).second;
                    continue;
                }
                added |= sols.insert(alias).second;
            }
            if (added && chains_->find(edge) != chains_->end()) {
                worklist.push(edge);
            }
            ASSERT(!sols.empty());
        }
        worklist.pop();
    }
}

/**
 * Instructions that introduce aliases.
 */

void AliasAnalysis::VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst)
{
    if (inst->GetType() == DataType::REFERENCE) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

}  // namespace panda::compiler
