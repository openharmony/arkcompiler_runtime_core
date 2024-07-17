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
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/alias_analysis.h"
#include "compiler_logger.h"

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
 *     - IsLocalAlias method: to add instructions that create new object
 *     - ParseInstruction method: to add a new instruction alias analysis works for
 *     - AliasAnalysis class: to add a visitor for a new instruction that should be analyzed
 *
 * NOTE(Evgenii Kudriashov): Prior to walking the graph in steps 5 and 6, We
 * need to perform static cycle elimination on the constraint graph, as well as
 * off-line variable substitution.
 *
 * NOTE(Evgenii Kudriashov): To add flow-sensitivity the "Flow-sensitive
 * pointer analysis for millions of lines of code" by Ben Hardekopf and Calvin
 * Lin may be considered.
 *
 * NOTE(Evgenii Kudriashov): After implementing VRP and SCEV the "Loop-Oriented
 * Array- and Field-Sensitive Pointer Analysis for Automatic SIMD
 * Vectorization" by Yulei Sui, Xiaokang Fan, Hao Zhou, and Jingling Xue may be
 * considered to add advanced analysis of array indices.
 */

namespace ark::compiler {

AliasAnalysis::AliasAnalysis(Graph *graph) : Analysis(graph), pointsTo_(graph->GetAllocator()->Adapter()) {}

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
        auto it = pointsTo_.try_emplace(pair.first, GetGraph()->GetAllocator()->Adapter());
        ASSERT(pair.first.GetBase() == nullptr || pair.first.GetBase()->GetOpcode() != Opcode::NullCheck);
        ASSERT(pair.second.GetBase() == nullptr || pair.second.GetBase()->GetOpcode() != Opcode::NullCheck);
        it.first->second.insert(pair.second);
    }

    // Build graph
    for (auto pair : *copy_) {
        auto it = chains_->try_emplace(pair.first, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(pair.first.GetBase() == nullptr || pair.first.GetBase()->GetOpcode() != Opcode::NullCheck);
        ASSERT(pair.second.GetBase() == nullptr || pair.second.GetBase()->GetOpcode() != Opcode::NullCheck);
        it.first->second.push_back(pair.second);
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
    copy_ = allocator->New<PointerPairVector>(allocator->Adapter());
    inputsSet_ = allocator->New<ArenaSet<Inst *>>(allocator->Adapter());
    ASSERT(chains_ != nullptr);
    ASSERT(direct_ != nullptr);
    ASSERT(copy_ != nullptr);
    ASSERT(inputsSet_ != nullptr);
    pointsTo_.clear();
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
        case DICTIONARY_ELEMENT:
            (*out) << "v" << base_->GetId() << "[";
            (*out) << "v" << idx_->GetId();
            if (imm_ != 0) {
                (*out) << "#NAME";
            } else {
                (*out) << "#INDEX";
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

static Pointer GetDynamicAccessPointer(Inst *inst, Inst *base, DynObjectAccessType type, DynObjectAccessMode mode)
{
    if (type == DynObjectAccessType::UNKNOWN || mode == DynObjectAccessMode::UNKNOWN) {
        return Pointer::CreateObject(base);
    }

    Inst *idx = inst->GetDataFlowInput(1);
    uint64_t imm = 0;
    if (type == DynObjectAccessType::BY_NAME) {
        ASSERT_DO(mode != DynObjectAccessMode::ARRAY,
                  (std::cerr << "Unexpected access type BY_NAME for access mode ARRAY: ", inst->Dump(&std::cerr)));
        imm = UINT64_MAX;
    } else {
        ASSERT_DO(type == DynObjectAccessType::BY_INDEX,
                  (std::cerr << "Unsupported dynamic access type in alias analysis: ", inst->Dump(&std::cerr)));
    }
    if (mode == DynObjectAccessMode::ARRAY) {
        return Pointer::CreateArrayElement(base, idx, imm);
    }
    ASSERT_DO(mode == DynObjectAccessMode::DICTIONARY,
              (std::cerr << "Unsupported dynamic access mode in alias analysis: ", inst->Dump(&std::cerr)));
    return Pointer::CreateDictionaryElement(base, idx, imm);
}

void AliasAnalysis::DumpChains(std::ostream *out) const
{
    ArenaVector<Pointer> sortedKeys(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto &pair : *chains_) {
        sortedKeys.push_back(pair.first);
    }
    if (sortedKeys.empty()) {
        (*out) << "\nThe chains is empty" << std::endl;
        return;
    }
    std::sort(sortedKeys.begin(), sortedKeys.end(), PointerLess);

    (*out) << "\nThe chains are the following: {" << std::endl;
    for (auto &p : sortedKeys) {
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
    (*out) << "}" << std::endl;
}

void AliasAnalysis::DumpDirect(std::ostream *out) const
{
    (*out) << "\nThe direct are the following:" << std::endl;
    for (auto &pair : *direct_) {
        (*out) << "{ ";
        pair.first.Dump(out);
        (*out) << " , ";
        pair.second.Dump(out);
        (*out) << " }" << std::endl;
    }
}

void AliasAnalysis::DumpCopy(std::ostream *out) const
{
    (*out) << "\nThe copy are the following:" << std::endl;
    for (auto &pair : *copy_) {
        (*out) << "{ ";
        pair.first.Dump(out);
        (*out) << " , ";
        pair.second.Dump(out);
        (*out) << " }" << std::endl;
    }
}

void AliasAnalysis::Dump(std::ostream *out) const
{
    ArenaVector<Pointer> sortedKeys(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto &pair : pointsTo_) {
        sortedKeys.push_back(pair.first);
    }
    if (sortedKeys.empty()) {
        (*out) << "\nThe solution set is empty" << std::endl;
        return;
    }
    std::sort(sortedKeys.begin(), sortedKeys.end(), PointerLess);

    (*out) << "\nThe solution set is the following:" << std::endl;
    for (auto &p : sortedKeys) {
        (*out) << "\t";
        p.Dump(out);
        (*out) << ": {";

        // Sort by instruction ID to add more readability to logs
        auto values = pointsTo_.at(p);
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
    ASSERT(mem1->IsMemory() && mem2->IsMemory());
    Pointer p1 = {};
    Pointer p2 = {};

    if (!ParseInstruction(mem1, &p1) || !ParseInstruction(mem2, &p2)) {
        return MAY_ALIAS;
    }

    // Instructions with different types cannot alias each other. Handle
    // difference only in numeric types for now.
    if (IsTypeNumeric(mem1->GetType()) && IsTypeNumeric(mem2->GetType()) &&
        GetTypeSize(mem1->GetType(), GetGraph()->GetArch()) != GetTypeSize(mem2->GetType(), GetGraph()->GetArch())) {
        return NO_ALIAS;
    }
    // Instructions with a primitive type and the reference type cannot alias each other.
    if ((IsTypeNumeric(mem1->GetType()) && mem2->IsReferenceOrAny()) ||
        (IsTypeNumeric(mem2->GetType()) && mem1->IsReferenceOrAny())) {
        return NO_ALIAS;
    }

    return CheckMemAddress(p1, p2);
}

AliasType AliasAnalysis::CheckRefAlias(Inst *ref1, Inst *ref2) const
{
    ASSERT(ref1->IsReferenceOrAny());
    ASSERT(ref2->IsReferenceOrAny());
    return CheckMemAddress(Pointer::CreateObject(ref1), Pointer::CreateObject(ref2));
}

AliasType AliasAnalysis::CheckMemAddressEmptyIntersectionCase(const PointerSet &aliases1, const PointerSet &aliases2,
                                                              const Pointer &p1, const Pointer &p2) const
{
    // If at least one set of aliases consists of only local aliases then there is NO_ALIAS
    auto isOuter = [](Pointer const &p) { return !p.IsLocal(); };
    if (std::find_if(aliases1.begin(), aliases1.end(), isOuter) == aliases1.end() ||
        std::find_if(aliases2.begin(), aliases2.end(), isOuter) == aliases2.end()) {
        return NO_ALIAS;
    }
    // Different fields cannot alias each other even if they are not created locally
    if (p1.GetType() == OBJECT_FIELD && !p1.HasSameOffset(p2)) {
        return NO_ALIAS;
    }
    if (p1.GetType() == ARRAY_ELEMENT) {
        auto equal = IsSameOffsets(p1.GetIdx(), p2.GetIdx());
        // If it is known that indices are different OR Imm indices are different then there is
        // no alias.  If they are both different we can't certainly say so.
        if ((equal == Trilean::FALSE && p1.GetImm() == p2.GetImm()) ||
            (equal == Trilean::TRUE && p1.GetImm() != p2.GetImm())) {
            return NO_ALIAS;
        }
    }
    if (p1.GetType() == DICTIONARY_ELEMENT) {
        auto equal = IsSameOffsets(p1.GetIdx(), p2.GetIdx());
        if (equal == Trilean::FALSE && p1.GetImm() == p2.GetImm()) {
            return NO_ALIAS;
        }
    }
    return MAY_ALIAS;
}

/**
 * We have 5 types of pointers: OBJECT, OBJECT_FIELD, POOL_CONSTANT,
 * STATIC_FIELD and ARRAY_ELEMENT.  They correspond to groups of memory storing
 * and loading instructions.  Assuming that these groups cannot load the same
 * memory address (at IR level), it is true that different types of pointers
 * cannot alias each other.
 *
 * Global pointers such as STATIC_FIELD and POOL_CONSTANT are checked through
 * their unique TypeId.
 *
 * For OBJECT_FIELDs we look at objects they referenced to.  If they refer to
 * the same objects then depending on TypeId they MUST_ALIAS or NO_ALIAS.  If
 * the situation of the referenced objects is unclear then they MAY_ALIAS.
 *
 * The same is for ARRAY_ELEMENT.  Instead of TypeId we look at index and compare them.
 *
 * All function's arguments MAY_ALIAS each other.  Created objects in the
 * function may not alias arguments.
 */
AliasType AliasAnalysis::CheckMemAddress(const Pointer &p1, const Pointer &p2) const
{
    if (p1.GetType() != p2.GetType()) {
        return NO_ALIAS;
    }

    if (p1.GetType() == STATIC_FIELD || p2.GetType() == STATIC_FIELD || p1.GetType() == POOL_CONSTANT ||
        p2.GetType() == POOL_CONSTANT) {
        if (p1.HasSameOffset(p2)) {
            return MUST_ALIAS;
        }
        return NO_ALIAS;
    }

    auto baseObj1 = Pointer::CreateObject(p1.GetBase());
    auto baseObj2 = Pointer::CreateObject(p2.GetBase());
    ASSERT_DO(pointsTo_.find(baseObj1) != pointsTo_.end(),
              (std::cerr << "Undefined inst in AliasAnalysis: ", p1.GetBase()->Dump(&std::cerr)));
    ASSERT_DO(pointsTo_.find(baseObj2) != pointsTo_.end(),
              (std::cerr << "Undefined inst in AliasAnalysis: ", p2.GetBase()->Dump(&std::cerr)));
    auto &aliases1 = pointsTo_.at(baseObj1);
    auto &aliases2 = pointsTo_.at(baseObj2);
    // Find the intersection
    const Pointer *intersection = nullptr;
    for (auto &alias : aliases1) {
        if (aliases2.find(alias) != aliases2.end()) {
            intersection = &alias;
            break;
        }
    }

    // The only common intersection
    if (intersection != nullptr && aliases1.size() == 1 && aliases2.size() == 1) {
        return SingleIntersectionAliasing(p1, p2, intersection);
    }
    // Empty intersection: check that both addresses are not parameters
    if (intersection == nullptr) {
        return CheckMemAddressEmptyIntersectionCase(aliases1, aliases2, p1, p2);
    }
    return MAY_ALIAS;
}

static uint64_t CombineIdxAndImm(const Pointer *p)
{
    uint64_t sum = 0;
    if (p->GetIdx() != nullptr) {
        auto idx = p->GetIdx();
        ASSERT(idx->IsConst());
        ASSERT(!DataType::IsFloatType(idx->GetType()));
        sum = idx->CastToConstant()->GetRawValue();
    }
    sum += p->GetImm();
    return sum;
}

AliasType AliasAnalysis::AliasingTwoArrayPointers(const Pointer *p1, const Pointer *p2)
{
    // Structure of Pointer: base, idx, imm
    // Base is same. We should compare fields in order: idx, combine {idx + imm}.
    // Is necessary compare sum, because LoadArrayI (..., nullptr, 2) and StoreArray(..., Const{2}, 0) will alias,
    // but in separate idx and imm are different.

    // 1) Compare idx. If they same, compare Imm part -> MUST_ALIAS, NO_ALIAS
    if (AliasAnalysis::IsSameOffsets(p1->GetIdx(), p2->GetIdx()) == Trilean::TRUE) {
        return p1->GetImm() == p2->GetImm() ? MUST_ALIAS : NO_ALIAS;
    }
    // 2) If still one of indexes is not constant or nullptr -> MAY_ALIAS
    for (auto *pointer : {p1, p2}) {
        if (pointer->GetIdx() != nullptr && !pointer->GetIdx()->IsConst()) {
            return MAY_ALIAS;
        }
    }
    // 3) Combine idx(is constant) and imm for compare
    if (CombineIdxAndImm(p1) == CombineIdxAndImm(p2)) {
        return MUST_ALIAS;
    }
    return NO_ALIAS;
}

/// Checks aliasing if P1 and P2 point to the one single object.
/* static */
AliasType AliasAnalysis::SingleIntersectionAliasing(const Pointer &p1, const Pointer &p2, const Pointer *intersection)
{
    ASSERT(p1.GetType() == p2.GetType());
    switch (p1.GetType()) {
        case ARRAY_ELEMENT: {
            return AliasingTwoArrayPointers(&p1, &p2);
        }
        case DICTIONARY_ELEMENT:
            // It is dinamic case, there are less guarantees
            if (IsSameOffsets(p1.GetIdx(), p2.GetIdx()) != Trilean::TRUE) {
                return MAY_ALIAS;
            }
            if (p1.GetImm() != p2.GetImm()) {
                return MAY_ALIAS;
            }
            break;
        case OBJECT_FIELD:
            if (!p1.HasSameOffset(p2)) {
                return NO_ALIAS;
            }
            break;
        case OBJECT:
            break;
        default:
            UNREACHABLE();
    }
    if (intersection->IsVolatile()) {
        return MAY_ALIAS;
    }
    return MUST_ALIAS;
}

void AliasAnalysis::SolveConstraintsMainLoop(Pointer &ref, Pointer &edge, bool &added, PointerSet &sols)
{
    for (auto &alias : pointsTo_.at(ref)) {
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
        if (edge.GetType() == DICTIONARY_ELEMENT && ref.GetBase() == edge.GetBase()) {
            // Propagating from object to elements: A{a} -> A[i]{a[i]}
            if (alias.GetType() == OBJECT) {
                Pointer p = Pointer::CreateDictionaryElement(alias.GetBase(), edge.GetIdx());
                p.SetLocalVolatile(alias.IsLocal(), edge.IsVolatile());

                added |= sols.insert(p).second;
                continue;
            }
            // In case A{a[j]} -> A[i] we propagate symbolic name: A{a[j]} -> A[i]{A[i]}
            Pointer p = Pointer::CreateDictionaryElement(ref.GetBase(), edge.GetIdx());
            p.SetLocalVolatile(alias.IsLocal(), edge.IsVolatile());

            added |= sols.insert(p).second;
            continue;
        }
        added |= sols.insert(alias).second;
    }
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
            auto &sols = pointsTo_.try_emplace(edge, GetGraph()->GetAllocator()->Adapter()).first->second;
            bool added = false;

            SolveConstraintsMainLoop(ref, edge, added, sols);

            if (added && chains_->find(edge) != chains_->end()) {
                worklist.push(edge);
            }
            ASSERT(!sols.empty());
        }
        worklist.pop();
    }
}

/// Selects the address from instruction that should be checked on alias
bool AliasAnalysis::ParseInstruction(Inst *inst, Pointer *pointer)
{
    Pointer p {};
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::LoadArrayI:
        case Opcode::StoreArray:
        case Opcode::StoreArrayI:
            p = ParseArrayElement(inst);
            break;
        case Opcode::LoadString:
        case Opcode::LoadType:
        case Opcode::UnresolvedLoadType:
            p = ParsePoolConstant(inst);
            break;
        case Opcode::LoadStatic:
        case Opcode::StoreStatic:
        case Opcode::UnresolvedStoreStatic:
        case Opcode::LoadResolvedObjectFieldStatic:
        case Opcode::StoreResolvedObjectFieldStatic:
            p = ParseStaticField(inst);
            break;
        case Opcode::LoadObject:
        case Opcode::StoreObject:
        case Opcode::LoadResolvedObjectField:
        case Opcode::StoreResolvedObjectField:
            p = ParseObjectField(inst);
            break;
        case Opcode::LoadObjectDynamic:
        case Opcode::StoreObjectDynamic:
            p = ParseDynamicField(inst);
            break;
        default:
            return false;
    }

    auto base = p.GetBase();
    if (base != nullptr) {
        // Currently unhandled and return always MAY_ALIAS
        if (base->GetOpcode() == Opcode::LoadArrayPair || base->GetOpcode() == Opcode::LoadArrayPairI ||
            base->GetOpcode() == Opcode::LoadPairPart || base->GetOpcode() == Opcode::CatchPhi ||
            base->GetOpcode() == Opcode::Load || base->GetOpcode() == Opcode::LoadI ||
            base->GetOpcode() == Opcode::Store || base->GetOpcode() == Opcode::StoreI) {
            return false;
        }
    }

    *pointer = p;
    return true;
}

Pointer AliasAnalysis::ParseArrayElement(Inst *inst)
{
    uint32_t imm = 0;
    Inst *offset = nullptr;
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::StoreArray:
            offset = inst->GetDataFlowInput(1);
            break;
        case Opcode::LoadArrayI:
            imm = inst->CastToLoadArrayI()->GetImm();
            break;
        case Opcode::StoreArrayI:
            imm = inst->CastToStoreArrayI()->GetImm();
            break;
        default:
            UNREACHABLE();
    }
    auto base = inst->GetDataFlowInput(0);
    return Pointer::CreateArrayElement(base, offset, imm);
}

Pointer AliasAnalysis::ParsePoolConstant(Inst *inst)
{
    uint32_t typeId = 0;
    switch (inst->GetOpcode()) {
        case Opcode::LoadString:
            typeId = inst->CastToLoadString()->GetTypeId();
            break;
        case Opcode::LoadType:
            typeId = inst->CastToLoadType()->GetTypeId();
            break;
        case Opcode::UnresolvedLoadType:
            typeId = inst->CastToUnresolvedLoadType()->GetTypeId();
            break;
        default:
            UNREACHABLE();
    }
    return Pointer::CreatePoolConstant(typeId);
}

Pointer AliasAnalysis::ParseStaticField(Inst *inst)
{
    uint32_t typeId = 0;
    void *typePtr = nullptr;
    switch (inst->GetOpcode()) {
        case Opcode::LoadStatic:
            typeId = inst->CastToLoadStatic()->GetTypeId();
            typePtr = inst->CastToLoadStatic()->GetObjField();
            break;
        case Opcode::LoadResolvedObjectFieldStatic:
            typeId = inst->CastToLoadResolvedObjectFieldStatic()->GetTypeId();
            break;
        case Opcode::StoreStatic:
            typeId = inst->CastToStoreStatic()->GetTypeId();
            typePtr = inst->CastToStoreStatic()->GetObjField();
            break;
        case Opcode::UnresolvedStoreStatic:
            typeId = inst->CastToUnresolvedStoreStatic()->GetTypeId();
            break;
        case Opcode::StoreResolvedObjectFieldStatic:
            typeId = inst->CastToStoreResolvedObjectFieldStatic()->GetTypeId();
            break;
        default:
            UNREACHABLE();
    }
    return Pointer::CreateStaticField(typeId, typePtr);
}

Pointer AliasAnalysis::ParseObjectField(Inst *inst)
{
    uint32_t typeId = 0;
    void *typePtr = nullptr;
    bool isStatic = false;
    switch (inst->GetOpcode()) {
        case Opcode::LoadObject:
            isStatic = inst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_STATIC;
            typeId = inst->CastToLoadObject()->GetTypeId();
            typePtr = inst->CastToLoadObject()->GetObjField();
            break;
        case Opcode::LoadResolvedObjectField:
            typeId = inst->CastToLoadResolvedObjectField()->GetTypeId();
            break;
        case Opcode::StoreObject:
            isStatic = inst->CastToStoreObject()->GetObjectType() == ObjectType::MEM_STATIC;
            typeId = inst->CastToStoreObject()->GetTypeId();
            typePtr = inst->CastToStoreObject()->GetObjField();
            break;
        case Opcode::StoreResolvedObjectField:
            typeId = inst->CastToStoreResolvedObjectField()->GetTypeId();
            break;
        default:
            UNREACHABLE();
    }
    auto base = inst->GetDataFlowInput(0);
    return isStatic ? Pointer::CreateStaticField(typeId, typePtr) : Pointer::CreateObjectField(base, typeId, typePtr);
}

Pointer AliasAnalysis::ParseDynamicField(Inst *inst)
{
    auto base = inst->GetDataFlowInput(0);

    DynObjectAccessType type;
    DynObjectAccessMode mode;
    switch (inst->GetOpcode()) {
        case Opcode::LoadObjectDynamic:
            type = inst->CastToLoadObjectDynamic()->GetAccessType();
            mode = inst->CastToLoadObjectDynamic()->GetAccessMode();
            break;
        case Opcode::StoreObjectDynamic:
            type = inst->CastToStoreObjectDynamic()->GetAccessType();
            mode = inst->CastToStoreObjectDynamic()->GetAccessMode();
            break;
        default:
            UNREACHABLE();
    }

    return GetDynamicAccessPointer(inst, base, type, mode);
}

/**
 * Compares offsets. Since we have a situation when we cannot determine either equal offsets or not,
 * we use three valued logic. It is handy when we want to know whether offsets are definitely
 * different or definitely equal.
 */
/* static */
AliasAnalysis::Trilean AliasAnalysis::IsSameOffsets(const Inst *off1, const Inst *off2)
{
    if (off1 == off2) {
        return Trilean::TRUE;
    }
    if (off1 == nullptr || off2 == nullptr) {
        return Trilean::FALSE;
    }

    if (off1->GetVN() != INVALID_VN && off2->GetVN() != INVALID_VN && off1->GetVN() == off2->GetVN()) {
        return Trilean::TRUE;
    }

    if (off1->IsConst() && off2->IsConst() &&
        off1->CastToConstant()->GetRawValue() != off2->CastToConstant()->GetRawValue()) {
        return Trilean::FALSE;
    }

    return Trilean::UNKNOWN;
}

/// Instructions that definitely are not an alias of anything.
void AliasAnalysis::VisitNullPtr(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasAnalysis::VisitLoadUndefined(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasAnalysis::VisitInitObject(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasAnalysis::VisitNewObject(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasAnalysis::VisitNewArray(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasAnalysis::VisitMultiArray(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasAnalysis::VisitInitEmptyString(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasAnalysis::VisitInitString(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}

/**
 * Instructions that can introduce references that are an alias of
 * something already existed.
 */
void AliasAnalysis::VisitConstant(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitParameter(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitLoadImmediate(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitIntrinsic(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitBuiltin(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitCallStatic(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitCallResolvedStatic(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitCallVirtual(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitCallResolvedVirtual(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitCallDynamic(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitGetManagedClassObject(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasAnalysis::VisitResolveObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

void AliasAnalysis::VisitBitcast(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

/// Instructions that introduce static fields (global variables).
void AliasAnalysis::VisitLoadStatic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToLoadStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId, typedInst->GetObjField());

    sfield.SetVolatile(typedInst->GetVolatile());

    visitor->AddDirectEdge(sfield);
    visitor->AddCopyEdge(sfield, Pointer::CreateObject(inst));
}

void AliasAnalysis::VisitLoadResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToLoadResolvedObjectFieldStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId);

    sfield.SetVolatile(IsVolatileMemInst(typedInst));

    visitor->AddDirectEdge(sfield);
    visitor->AddCopyEdge(sfield, Pointer::CreateObject(inst));
}

void AliasAnalysis::VisitStoreStatic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToStoreStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId, typedInst->GetObjField());

    sfield.SetVolatile(typedInst->GetVolatile());

    visitor->AddDirectEdge(sfield);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), sfield);
}

void AliasAnalysis::VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    if (inst->GetType() != DataType::REFERENCE) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToStoreResolvedObjectFieldStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId);

    sfield.SetVolatile(IsVolatileMemInst(typedInst));

    visitor->AddDirectEdge(sfield);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), sfield);
}

void AliasAnalysis::VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToUnresolvedStoreStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId);

    sfield.SetVolatile(IsVolatileMemInst(typedInst));

    visitor->AddDirectEdge(sfield);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), sfield);
}

/// Instructions that introduce unique constant references (global constants).
void AliasAnalysis::VisitLoadRuntimeClass(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadRuntimeClass()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}

void AliasAnalysis::VisitLoadClass(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadClass()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasAnalysis::VisitLoadAndInitClass(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadAndInitClass()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasAnalysis::VisitUnresolvedLoadAndInitClass(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToUnresolvedLoadAndInitClass()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasAnalysis::VisitGetGlobalVarAddress(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToGetGlobalVarAddress()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasAnalysis::VisitLoadString(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadString()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasAnalysis::VisitLoadConstArray(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadConstArray()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasAnalysis::VisitLoadType(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadType()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasAnalysis::VisitUnresolvedLoadType(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToUnresolvedLoadType()->GetTypeId();
        static_cast<AliasAnalysis *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}

void AliasAnalysis::VisitLoadObjFromConst(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

/// Instructions that introduce aliases.
void AliasAnalysis::VisitLoadArray(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    Inst *arr = inst->GetDataFlowInput(0);
    Inst *idx = inst->GetDataFlowInput(1);
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elem = Pointer::CreateArrayElement(arr, idx);
    Pointer val = Pointer::CreateObject(inst);

    visitor->AddCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasAnalysis::VisitStoreArray(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    Inst *arr = inst->GetDataFlowInput(0);
    Inst *idx = inst->GetDataFlowInput(1);
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elem = Pointer::CreateArrayElement(arr, idx);
    Pointer val = Pointer::CreateObject(inst->GetDataFlowInput(2U));

    visitor->AddCopyEdge(obj, elem);
    visitor->AddCopyEdge(val, elem);
}

void AliasAnalysis::VisitLoadArrayI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    Inst *arr = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elem = Pointer::CreateArrayElement(arr, nullptr, inst->CastToLoadArrayI()->GetImm());
    Pointer val = Pointer::CreateObject(inst);

    visitor->AddCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasAnalysis::VisitStoreArrayI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    Inst *arr = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elem = Pointer::CreateArrayElement(arr, nullptr, inst->CastToStoreArrayI()->GetImm());
    Pointer val = Pointer::CreateObject(inst->GetDataFlowInput(1));

    visitor->AddCopyEdge(obj, elem);
    visitor->AddCopyEdge(val, elem);
}

void AliasAnalysis::VisitLoadArrayPair(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto *load = inst->CastToLoadArrayPair();
    Inst *arr = load->GetDataFlowInput(load->GetArray());
    Pointer obj = Pointer::CreateObject(arr);
    for (auto &user : load->GetUsers()) {
        ASSERT(user.GetInst()->GetOpcode() == Opcode::LoadPairPart);
        auto uinst = user.GetInst()->CastToLoadPairPart();

        Pointer elem = Pointer::CreateArrayElement(arr, load->GetIndex(), uinst->GetImm());
        Pointer val = Pointer::CreateObject(uinst);
        visitor->AddCopyEdge(obj, elem);
        visitor->AddCopyEdge(elem, val);
        visitor->AddCopyEdge(elem, Pointer::CreateObject(load));
    }
}

void AliasAnalysis::VisitStoreArrayPair(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto *store = inst->CastToStoreArrayPair();
    Inst *arr = store->GetDataFlowInput(store->GetArray());
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elFst = Pointer::CreateArrayElement(arr, store->GetIndex());
    Pointer elSnd = Pointer::CreateArrayElement(arr, store->GetIndex(), 1);
    Pointer valFst = Pointer::CreateObject(store->GetDataFlowInput(store->GetStoredValue(0)));
    Pointer valSnd = Pointer::CreateObject(store->GetDataFlowInput(store->GetStoredValue(1)));

    visitor->AddCopyEdge(obj, elFst);
    visitor->AddCopyEdge(obj, elSnd);
    visitor->AddCopyEdge(valFst, elFst);
    visitor->AddCopyEdge(valSnd, elSnd);
}

void AliasAnalysis::VisitLoadObjectPair(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToLoadObjectPair();
    uint32_t typeId0 = typedInst->GetTypeId0();
    uint32_t typeId1 = typedInst->GetTypeId1();
    ASSERT(typedInst->GetObjectType() != ObjectType::MEM_STATIC);
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer field0 = Pointer::CreateObjectField(dfobj, typeId0, typedInst->GetObjField0());
    Pointer field1 = Pointer::CreateObjectField(dfobj, typeId1, typedInst->GetObjField1());

    Pointer to = Pointer::CreateObject(inst);

    field0.SetVolatile(typedInst->GetVolatile());
    field1.SetVolatile(typedInst->GetVolatile());

    visitor->AddCopyEdge(obj, field0);
    visitor->AddCopyEdge(field0, to);
    visitor->AddCopyEdge(obj, field1);
    visitor->AddCopyEdge(field1, to);

    for (auto &user : inst->GetUsers()) {
        visitor->AddCopyEdge(obj, Pointer::CreateObject(user.GetInst()));
    }
}

void AliasAnalysis::VisitStoreObjectPair(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToStoreObjectPair();
    uint32_t typeId0 = typedInst->GetTypeId0();
    uint32_t typeId1 = typedInst->GetTypeId1();
    ASSERT(typedInst->GetObjectType() != ObjectType::MEM_STATIC);
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer field0 = Pointer::CreateObjectField(dfobj, typeId0, typedInst->GetObjField0());
    Pointer val0 = Pointer::CreateObject(inst->GetDataFlowInput(1));
    Pointer field1 = Pointer::CreateObjectField(dfobj, typeId1, typedInst->GetObjField1());
    Pointer val1 = Pointer::CreateObject(inst->GetDataFlowInput(2));

    field0.SetVolatile(typedInst->GetVolatile());
    field1.SetVolatile(typedInst->GetVolatile());

    visitor->AddCopyEdge(obj, field0);
    visitor->AddCopyEdge(val0, field0);
    visitor->AddCopyEdge(obj, field1);
    visitor->AddCopyEdge(val1, field1);
}

void AliasAnalysis::VisitLoadArrayPairI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto *load = inst->CastToLoadArrayPairI();
    Inst *arr = load->GetDataFlowInput(load->GetArray());
    Pointer obj = Pointer::CreateObject(arr);
    for (auto &user : load->GetUsers()) {
        ASSERT(user.GetInst()->GetOpcode() == Opcode::LoadPairPart);
        auto uinst = user.GetInst()->CastToLoadPairPart();

        Pointer elem = Pointer::CreateArrayElement(arr, nullptr, load->GetImm() + uinst->GetImm());
        Pointer val = Pointer::CreateObject(uinst);
        visitor->AddCopyEdge(obj, elem);
        visitor->AddCopyEdge(elem, val);
        visitor->AddCopyEdge(elem, Pointer::CreateObject(load));
    }
}

void AliasAnalysis::VisitStoreArrayPairI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto *store = inst->CastToStoreArrayPairI();
    Inst *arr = store->GetDataFlowInput(store->GetArray());
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elFst = Pointer::CreateArrayElement(arr, nullptr, store->GetImm());
    Pointer elSnd = Pointer::CreateArrayElement(arr, nullptr, store->GetImm() + 1);
    Pointer valFst = Pointer::CreateObject(store->GetDataFlowInput(store->GetFirstValue()));
    Pointer valSnd = Pointer::CreateObject(store->GetDataFlowInput(store->GetSecondValue()));

    visitor->AddCopyEdge(obj, elFst);
    visitor->AddCopyEdge(obj, elSnd);
    visitor->AddCopyEdge(valFst, elFst);
    visitor->AddCopyEdge(valSnd, elSnd);
}

void AliasAnalysis::VisitLoadObject(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToLoadObject();
    uint32_t typeId = typedInst->GetTypeId();
    if (inst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_STATIC) {
        Pointer sfield = Pointer::CreateStaticField(typeId, typedInst->GetObjField());

        sfield.SetVolatile(typedInst->GetVolatile());

        visitor->AddDirectEdge(sfield);
        visitor->AddCopyEdge(sfield, Pointer::CreateObject(inst));
    } else {
        Inst *dfobj = inst->GetDataFlowInput(0);
        Pointer obj = Pointer::CreateObject(dfobj);
        Pointer ifield = Pointer::CreateObjectField(dfobj, typeId, typedInst->GetObjField());
        Pointer to = Pointer::CreateObject(inst);

        ifield.SetVolatile(typedInst->GetVolatile());

        visitor->AddCopyEdge(obj, ifield);
        visitor->AddCopyEdge(ifield, to);
    }
}

void AliasAnalysis::VisitLoadResolvedObjectField(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToLoadResolvedObjectField();
    uint32_t typeId = typedInst->GetTypeId();
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer ifield = Pointer::CreateObjectField(dfobj, typeId);
    Pointer to = Pointer::CreateObject(inst);

    ifield.SetVolatile(IsVolatileMemInst(typedInst));

    visitor->AddCopyEdge(obj, ifield);
    visitor->AddCopyEdge(ifield, to);
}

void AliasAnalysis::VisitStoreObject(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToStoreObject();
    uint32_t typeId = typedInst->GetTypeId();
    if (inst->CastToStoreObject()->GetObjectType() == ObjectType::MEM_STATIC) {
        Pointer sfield = Pointer::CreateStaticField(typeId, typedInst->GetObjField());

        sfield.SetVolatile(typedInst->GetVolatile());

        visitor->AddDirectEdge(sfield);
        visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), sfield);
    } else {
        Inst *dfobj = inst->GetDataFlowInput(0);
        Pointer obj = Pointer::CreateObject(dfobj);
        Pointer ifield = Pointer::CreateObjectField(dfobj, typeId, typedInst->GetObjField());
        Pointer val = Pointer::CreateObject(inst->GetDataFlowInput(1));

        ifield.SetVolatile(typedInst->GetVolatile());

        visitor->AddCopyEdge(obj, ifield);
        visitor->AddCopyEdge(val, ifield);
    }
}

void AliasAnalysis::VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto typedInst = inst->CastToStoreResolvedObjectField();
    uint32_t typeId = typedInst->GetTypeId();
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer ifield = Pointer::CreateObjectField(dfobj, typeId);
    Pointer val = Pointer::CreateObject(inst->GetDataFlowInput(1));

    ifield.SetVolatile(IsVolatileMemInst(inst));

    visitor->AddCopyEdge(obj, ifield);
    visitor->AddCopyEdge(val, ifield);
}

void AliasAnalysis::VisitCatchPhi(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto inputsSet = visitor->GetClearInputsSet();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        inputsSet->insert(inst->GetDataFlowInput(i));
    }

    for (auto inputInst : *inputsSet) {
        visitor->AddCopyEdge(Pointer::CreateObject(inputInst), Pointer::CreateObject(inst));
    }
}

void AliasAnalysis::VisitPhi(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(i)), Pointer::CreateObject(inst));
    }
}

void AliasAnalysis::VisitSelect(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    ASSERT(inst->GetInputsCount() == 4U);
    auto visitor = static_cast<AliasAnalysis *>(v);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), Pointer::CreateObject(inst));
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(1)), Pointer::CreateObject(inst));
}

void AliasAnalysis::VisitSelectImm(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    ASSERT(inst->GetInputsCount() == 3U);
    auto visitor = static_cast<AliasAnalysis *>(v);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), Pointer::CreateObject(inst));
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(1)), Pointer::CreateObject(inst));
}

void AliasAnalysis::VisitMov(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), Pointer::CreateObject(inst));
}

void AliasAnalysis::VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst)
{
    if (inst->GetType() == DataType::REFERENCE) {
        static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

void AliasAnalysis::VisitCastValueToAnyType(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToCastValueToAnyType()->GetInputType(0) == DataType::REFERENCE) {
        static_cast<AliasAnalysis *>(v)->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)),
                                                     Pointer::CreateObject(inst));
    }
}

void AliasAnalysis::VisitGetAnyTypeName(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasAnalysis *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}

void AliasAnalysis::VisitLoadConstantPool(GraphVisitor *v, Inst *inst)
{
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer to = Pointer::CreateObject(inst);
    static_cast<AliasAnalysis *>(v)->AddCopyEdge(obj, to);
}

void AliasAnalysis::VisitLoadLexicalEnv(GraphVisitor *v, Inst *inst)
{
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer to = Pointer::CreateObject(inst);
    static_cast<AliasAnalysis *>(v)->AddCopyEdge(obj, to);
}

void AliasAnalysis::VisitLoadObjectDynamic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto type = inst->CastToLoadObjectDynamic()->GetAccessType();
    auto mode = inst->CastToLoadObjectDynamic()->GetAccessMode();

    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer val = Pointer::CreateObject(inst);
    Pointer elem = GetDynamicAccessPointer(inst, dfobj, type, mode);

    visitor->AddCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasAnalysis::VisitStoreObjectDynamic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasAnalysis *>(v);
    auto type = inst->CastToStoreObjectDynamic()->GetAccessType();
    auto mode = inst->CastToStoreObjectDynamic()->GetAccessMode();

    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer val = Pointer::CreateObject(inst);
    Pointer elem = GetDynamicAccessPointer(inst, dfobj, type, mode);

    visitor->AddCopyEdge(obj, elem);
    visitor->AddCopyEdge(val, elem);
}

void AliasAnalysis::VisitLoadFromConstantPool(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    uint32_t typeId = inst->CastToLoadFromConstantPool()->GetTypeId();
    auto visitor = static_cast<AliasAnalysis *>(v);
    Inst *constpool = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(constpool);
    Pointer elem = Pointer::CreateArrayElement(constpool, nullptr, typeId);
    Pointer val = Pointer::CreateObject(inst);

    visitor->AddCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasAnalysis::VisitDefault([[maybe_unused]] Inst *inst)
{
    /* Ignore the following instructions with REFERENCE type intentionally */
    switch (inst->GetOpcode()) {
        // Handled on its input
        case Opcode::LoadPairPart:
        // No passes that check class references aliasing
        case Opcode::GetInstanceClass:
        case Opcode::LoadImmediate:
        // NOTE(ekudriashov): Probably should be added
        case Opcode::Monitor:
        // Mitigated by using GetDataFlowInput
        case Opcode::NullCheck:
        case Opcode::RefTypeCheck:
        case Opcode::AnyTypeCheck:
        case Opcode::ObjByIndexCheck:
        case Opcode::HclassCheck:
        // Irrelevant for analysis
        case Opcode::Return:
        case Opcode::ReturnI:
        // NOTE(compiler team): support Load, Store
        case Opcode::Load:
        case Opcode::LoadI:
        case Opcode::Store:
        case Opcode::StoreI:
        // No need to analyze
        case Opcode::LiveOut:
        case Opcode::FunctionImmediate:
            return;
        default:
            ASSERT_DO(!inst->IsReferenceOrAny(),
                      (std::cerr << "Unsupported instruction in alias analysis: ", inst->Dump(&std::cerr)));
            return;
    }
}
}  // namespace ark::compiler
