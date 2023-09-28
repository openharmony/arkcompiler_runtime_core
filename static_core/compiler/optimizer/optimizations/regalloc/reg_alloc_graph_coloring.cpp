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

#include "reg_alloc_graph_coloring.h"
#include <cmath>
#include "compiler_logger.h"
#include "interference_graph.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/code_generator/callconv.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"
#include "reg_type.h"

namespace panda::compiler {
RegAllocGraphColoring::RegAllocGraphColoring(Graph *graph) : RegAllocBase(graph) {}
RegAllocGraphColoring::RegAllocGraphColoring(Graph *graph, size_t regs_count) : RegAllocBase(graph, regs_count) {}

void RegAllocGraphColoring::BuildIG(InterferenceGraph *ig, WorkingRanges *ranges, bool remat_constants)
{
    ig->Reserve(ranges->regular.size() + ranges->physical.size());
    ArenaDeque<ColorNode *> active_nodes(GetGraph()->GetLocalAllocator()->Adapter());
    ArenaVector<ColorNode *> physical_nodes(GetGraph()->GetLocalAllocator()->Adapter());
    const auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();

    for (auto physical_interval : ranges->physical) {
        ColorNode *node = ig->AllocNode();
        node->Assign(physical_interval);
        node->SetPhysical();
        physical_nodes.push_back(node);
    }

    for (auto current_interval : ranges->regular) {
        auto range_start = current_interval->GetBegin();

        if (remat_constants && TryToSpillConstant(current_interval, GetGraph())) {
            continue;
        }

        // Expire active_ranges
        while (!active_nodes.empty() && active_nodes.front()->GetLifeIntervals()->GetEnd() <= range_start) {
            active_nodes.pop_front();
        }

        ColorNode *node = ig->AllocNode();
        node->Assign(current_interval);

        if (ig->IsUsedSpillWeight()) {
            node->SetSpillWeight(CalcSpillWeight(la, current_interval));
        }

        // Interfer node
        for (auto active_node : active_nodes) {
            auto active_interval = active_node->GetLifeIntervals();
            if (current_interval->IntersectsWith(active_interval)) {
                ig->AddEdge(node->GetNumber(), active_node->GetNumber());
            }
        }

        for (auto physical_node : physical_nodes) {
            auto physical_interval = physical_node->GetLifeIntervals();
            if (current_interval->IntersectsWith<true>(physical_interval)) {
                ig->AddEdge(node->GetNumber(), physical_node->GetNumber());
                node->AddCallsite(range_start);
            }
        }

        if (!current_interval->HasInst()) {
            // current_interval - is additional life interval for an instruction required temp, add edges to the fixed
            // inputs' nodes of that instruction
            la.EnumerateFixedLocationsOverlappingTemp(current_interval, [ig, node](Location location) {
                ASSERT(location.IsFixedRegister());
                auto physical_node = ig->FindPhysicalNode(location);
                if (physical_node == nullptr) {
                    return;
                }
                ig->AddEdge(node->GetNumber(), physical_node->GetNumber());
            });
        }

        // Add node to active_nodes sorted by End time
        auto ranges_iter =
            std::upper_bound(active_nodes.begin(), active_nodes.end(), node, [](const auto &lhs, const auto &rhs) {
                return lhs->GetLifeIntervals()->GetEnd() <= rhs->GetLifeIntervals()->GetEnd();
            });
        active_nodes.insert(ranges_iter, node);
    }
}

RegAllocGraphColoring::IndexVector RegAllocGraphColoring::PrecolorIG(InterferenceGraph *ig)
{
    // Walk nodes and propagate properties
    IndexVector affinity_nodes;
    for (const auto &node : ig->GetNodes()) {
        AddAffinityEdgesToSiblings(ig, node, &affinity_nodes);
    }
    return affinity_nodes;
}

// Find precolorings and set registers to intervals in advance
RegAllocGraphColoring::IndexVector RegAllocGraphColoring::PrecolorIG(InterferenceGraph *ig, const RegisterMap &map)
{
    // Walk nodes and propagate properties
    IndexVector affinity_nodes;
    for (auto &node : ig->GetNodes()) {
        const auto *interv = node.GetLifeIntervals();
        // Take in account preassigned registers in intervals
        if (interv->IsPhysical() || interv->IsPreassigned()) {
            // Translate preassigned register from interval to color graph
            auto color = map.CodegenToRegallocReg(interv->GetReg());
            node.SetFixedColor(color);
        }

        if (!interv->HasInst() || interv->IsSplitSibling()) {
            continue;
        }

        AddAffinityEdgesToSiblings(ig, node, &affinity_nodes);

        const auto *inst = interv->GetInst();
        ASSERT(inst != nullptr);
        if (inst->IsPhi()) {
            AddAffinityEdgesToPhi(ig, node, &affinity_nodes);
        }
    }
    AddAffinityEdgesToPhysicalNodes(ig, &affinity_nodes);
    return affinity_nodes;
}

void RegAllocGraphColoring::BuildBias(InterferenceGraph *ig, const IndexVector &affinity_nodes)
{
    auto &nodes = ig->GetNodes();

    // Build affinity connected-components UCC(Unilaterally Connected Components) for coalescing (assign bias number to
    // nodes of same component)
    SmallVector<unsigned, DEFAULT_VECTOR_SIZE> walked;
    SmallVector<unsigned, DEFAULT_VECTOR_SIZE> biased;
    for (auto index : affinity_nodes) {
        auto &node = nodes[index];

        // Skip already biased
        if (node.HasBias()) {
            continue;
        }

        // Find connected component of graph UCC by (DFS), and collect Call-sites intersections
        walked.clear();
        walked.push_back(node.GetNumber());
        biased.clear();
        biased.push_back(node.GetNumber());
        unsigned bias_num = ig->GetBiasCount();
        node.SetBias(bias_num);
        auto &bias = ig->AddBias();
        ig->UpdateBiasData(&bias, node);
        do {
            // Pop back
            unsigned cur_index = walked.back();
            walked.resize(walked.size() - 1);

            // Walk N affine nodes
            for (auto try_index : affinity_nodes) {
                auto &try_node = nodes[try_index];
                if (try_node.HasBias() || !ig->HasAffinityEdge(cur_index, try_index)) {
                    continue;
                }
                // Check if the `try_node` intersects one of the already biased
                auto it = std::find_if(biased.cbegin(), biased.cend(),
                                       [ig, try_index](auto id) { return ig->HasEdge(id, try_index); });
                if (it != biased.cend()) {
                    continue;
                }

                try_node.SetBias(bias_num);
                ig->UpdateBiasData(&bias, try_node);
                walked.push_back(try_index);
                biased.push_back(try_index);
            }
        } while (!walked.empty());
    }
}

void RegAllocGraphColoring::AddAffinityEdgesToPhi(InterferenceGraph *ig, const ColorNode &node,
                                                  IndexVector *affinity_nodes)
{
    // Duplicates are possible but we tolerate it
    affinity_nodes->push_back(node.GetNumber());

    auto phi = node.GetLifeIntervals()->GetInst();
    ASSERT(phi->IsPhi());
    auto la = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    // Iterate over Phi inputs
    for (size_t i = 0; i < phi->GetInputsCount(); i++) {
        // Add affinity edge
        auto input_li = la->GetInstLifeIntervals(phi->GetDataFlowInput(i));
        AddAffinityEdge(ig, affinity_nodes, node, input_li);
    }
}

void RegAllocGraphColoring::AddAffinityEdgesToSiblings(InterferenceGraph *ig, const ColorNode &node,
                                                       IndexVector *affinity_nodes)
{
    auto sibling = node.GetLifeIntervals()->GetSibling();
    if (sibling == nullptr) {
        return;
    }
    affinity_nodes->push_back(node.GetNumber());
    while (sibling != nullptr) {
        AddAffinityEdge(ig, affinity_nodes, node, sibling);
        sibling = sibling->GetSibling();
    }
}

void RegAllocGraphColoring::AddAffinityEdgesToPhysicalNodes(InterferenceGraph *ig, IndexVector *affinity_nodes)
{
    auto la = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    for (auto *interval : la->GetLifeIntervals()) {
        if (!interval->HasInst()) {
            continue;
        }
        const auto *inst = interval->GetInst();
        ASSERT(inst != nullptr);
        // Add affinity edges to fixed locations
        for (auto i = 0U; i < inst->GetInputsCount(); i++) {
            auto location = inst->GetLocation(i);
            if (location.IsFixedRegister()) {
                auto fixed_node = ig->FindPhysicalNode(location);
                // Possible when general intervals are processing, while input is fp-interval or vice versa
                if (fixed_node == nullptr) {
                    continue;
                }
                affinity_nodes->push_back(fixed_node->GetNumber());

                auto input_li = la->GetInstLifeIntervals(inst->GetDataFlowInput(i));
                auto sibling = input_li->FindSiblingAt(interval->GetBegin());
                ASSERT(sibling != nullptr);
                AddAffinityEdge(ig, affinity_nodes, *fixed_node, sibling);
            }
        }
    }
}

/**
 * Try to find node for the `li` interval in the IG;
 * If node exists, create affinity edge between it and the `node`
 */
void RegAllocGraphColoring::AddAffinityEdge(InterferenceGraph *ig, IndexVector *affinity_nodes, const ColorNode &node,
                                            LifeIntervals *li)
{
    if (auto af_node = ig->FindNode(li)) {
        COMPILER_LOG(DEBUG, REGALLOC) << "AfEdge: " << node.GetNumber() << " " << af_node->GetNumber();
        ig->AddAffinityEdge(node.GetNumber(), af_node->GetNumber());
        affinity_nodes->push_back(af_node->GetNumber());
    }
}

bool RegAllocGraphColoring::AllocateRegisters(InterferenceGraph *ig, WorkingRanges *ranges, WorkingRanges *stack_ranges,
                                              const RegisterMap &map)
{
    if (GetGraph()->IsBytecodeOptimizer()) {
        BuildIG(ig, ranges);
        BuildBias(ig, PrecolorIG(ig, map));
        return ig->AssignColors<VIRTUAL_FRAME_SIZE>(map.GetAvailableRegsCount(), map.GetBorder());
    }

    Presplit(ranges);
    ig->SetUseSpillWeight(true);
    auto rounds = 0;
    static constexpr size_t ROUNDS_LIMIT = 30;
    while (true) {
        if (++rounds == ROUNDS_LIMIT) {
            return false;
        }
        BuildIG(ig, ranges);
        BuildBias(ig, PrecolorIG(ig, map));
        if (ig->AssignColors<MAX_NUM_REGS>(map.GetAvailableRegsCount(), map.GetBorder())) {
            break;
        }
        SparseIG(ig, map.GetAvailableRegsCount(), ranges, stack_ranges);
    }
    return true;
}

bool RegAllocGraphColoring::AllocateSlots(InterferenceGraph *ig, WorkingRanges *stack_ranges)
{
    ig->SetUseSpillWeight(false);
    BuildIG(ig, stack_ranges, true);
    BuildBias(ig, PrecolorIG(ig));
    return ig->AssignColors<MAX_NUM_STACK_SLOTS>(MAX_NUM_STACK_SLOTS, 0);
}

/*
 * Coloring was unsuccessful, hence uncolored nodes should be split to sparse the interference graph
 */
void RegAllocGraphColoring::SparseIG(InterferenceGraph *ig, unsigned regs_count, WorkingRanges *ranges,
                                     WorkingRanges *stack_ranges)
{
    for (const auto &node : ig->GetNodes()) {
        if (node.GetColor() != regs_count) {
            continue;
        }
        auto interval = node.GetLifeIntervals();
        if (interval->GetUsePositions().empty()) {
            SpillInterval(interval, ranges, stack_ranges);
            continue;
        }

        interval->SplitAroundUses(GetGraph()->GetAllocator());
        bool is_const = interval->GetInst()->IsConst();
        if (is_const && interval->GetUsePositions().empty()) {
            SpillInterval(interval, ranges, stack_ranges);
        }
        for (auto sibling = interval->GetSibling(); sibling != nullptr; sibling = sibling->GetSibling()) {
            if (is_const && sibling->GetUsePositions().empty()) {
                AddRange(sibling, &stack_ranges->regular);
            } else {
                AddRange(sibling, &ranges->regular);
            }
        }
    }
}

void RegAllocGraphColoring::SpillInterval(LifeIntervals *interval, WorkingRanges *ranges, WorkingRanges *stack_ranges)
{
    ASSERT(interval->GetUsePositions().empty());
    ranges->regular.erase(std::remove(ranges->regular.begin(), ranges->regular.end(), interval), ranges->regular.end());
    AddRange(interval, &stack_ranges->regular);
}

void RegAllocGraphColoring::Remap(const InterferenceGraph &ig, const RegisterMap &map)
{
    // Map allocated colors to registers
    for (const auto &node : ig.GetNodes()) {
        auto *interval = node.GetLifeIntervals();
        if (!node.IsFixedColor()) {
            // Make interval's register
            auto color = node.GetColor();
            ASSERT(color != INVALID_REG);
            auto reg = map.RegallocToCodegenReg(color);
            interval->SetReg(reg);
        }
    }
}

bool RegAllocGraphColoring::MapSlots(const InterferenceGraph &ig)
{
    // Map allocated colors to stack slots
    for (const auto &node : ig.GetNodes()) {
        auto *interval = node.GetLifeIntervals();
        // Constant definition on the stack slot is not supported now
        if (!interval->IsSplitSibling() && interval->GetInst()->IsConst()) {
            COMPILER_LOG(DEBUG, REGALLOC) << "Stack slots RA failed";
            return false;
        }
        auto slot = node.GetColor();
        interval->SetLocation(Location::MakeStackSlot(slot));
        GetStackMask().Set(slot);
    }
    return true;
}

bool RegAllocGraphColoring::Allocate()
{
    auto *gr = GetGraph();

    ReserveTempRegisters();
    // Create intervals sequences
    WorkingRanges general_ranges(gr->GetLocalAllocator());
    WorkingRanges fp_ranges(gr->GetLocalAllocator());
    WorkingRanges stack_ranges(gr->GetLocalAllocator());
    InitWorkingRanges(&general_ranges, &fp_ranges);
    COMPILER_LOG(INFO, REGALLOC) << "Ranges reg " << general_ranges.regular.size() << " fp "
                                 << fp_ranges.regular.size();

    // Register allocation
    InterferenceGraph ig(gr->GetLocalAllocator());
    RegisterMap map(gr->GetLocalAllocator());

    if (!general_ranges.regular.empty()) {
        InitMap(&map, false);
        if (!AllocateRegisters(&ig, &general_ranges, &stack_ranges, map)) {
            COMPILER_LOG(DEBUG, REGALLOC) << "Integer RA failed";
            return false;
        }
        Remap(ig, map);
    }

    if (!fp_ranges.regular.empty()) {
        GetGraph()->SetHasFloatRegs();
        InitMap(&map, true);
        if (!AllocateRegisters(&ig, &fp_ranges, &stack_ranges, map)) {
            COMPILER_LOG(DEBUG, REGALLOC) << "Vector RA failed";
            return false;
        }
        Remap(ig, map);
    }

    if (!stack_ranges.regular.empty()) {
        if (AllocateSlots(&ig, &stack_ranges) && MapSlots(ig)) {
            return true;
        }
        COMPILER_LOG(DEBUG, REGALLOC) << "Stack slots RA failed";
        return false;
    }
    return true;
}

void RegAllocGraphColoring::InitWorkingRanges(WorkingRanges *general_ranges, WorkingRanges *fp_ranges)
{
    for (auto *interval : GetGraph()->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals()) {
        if (interval->GetReg() == ACC_REG_ID) {
            continue;
        }

        if (interval->IsPreassigned() && interval->GetReg() == GetGraph()->GetZeroReg()) {
            ASSERT(interval->GetReg() != INVALID_REG);
            continue;
        }

        // Skip instructions without destination register
        if (interval->HasInst() && interval->NoDest()) {
            ASSERT(interval->GetLocation().IsInvalid());
            continue;
        }

        bool is_fp = DataType::IsFloatType(interval->GetType());
        auto *ranges = is_fp ? fp_ranges : general_ranges;
        if (interval->IsPhysical()) {
            auto mask = is_fp ? GetVRegMask() : GetRegMask();
            if (mask.IsSet(interval->GetReg())) {
                // skip physical intervals for unavailable registers, they do not affect allocation
                continue;
            }
            AddRange(interval, &ranges->physical);
        } else {
            AddRange(interval, &ranges->regular);
        }
    }
}

void RegAllocGraphColoring::InitMap(RegisterMap *map, bool is_vector)
{
    auto arch = GetGraph()->GetArch();
    if (arch == Arch::NONE) {
        ASSERT(GetGraph()->IsBytecodeOptimizer());
        ASSERT(!is_vector);
        map->SetMask(GetRegMask(), 0);
    } else {
        size_t first_callee = GetFirstCalleeReg(arch, is_vector);
        size_t last_callee = GetLastCalleeReg(arch, is_vector);
        map->SetCallerFirstMask(is_vector ? GetVRegMask() : GetRegMask(), first_callee, last_callee);
    }
}

void RegAllocGraphColoring::Presplit(WorkingRanges *ranges)
{
    ArenaVector<LifeIntervals *> to_split(GetGraph()->GetLocalAllocator()->Adapter());

    for (auto interval : ranges->regular) {
        if (!interval->GetLocation().IsFixedRegister()) {
            continue;
        }
        for (auto next : ranges->regular) {
            if (next->GetBegin() <= interval->GetBegin()) {
                continue;
            }
            if (interval->GetLocation() == next->GetLocation() && interval->IntersectsWith(next)) {
                to_split.push_back(interval);
                break;
            }
        }

        if (!to_split.empty() && to_split.back() == interval) {
            // Already added to split
            continue;
        }
        for (auto physical : ranges->physical) {
            if (interval->GetLocation() == physical->GetLocation() && interval->IntersectsWith<true>(physical)) {
                to_split.push_back(interval);
                break;
            }
        }
    }

    for (auto interval : to_split) {
        COMPILER_LOG(DEBUG, REGALLOC) << "Split at the beginning: " << interval->ToString();
        auto split = interval->SplitAt(interval->GetBegin() + 1, GetGraph()->GetAllocator());
        AddRange(split, &ranges->regular);
    }
}
}  // namespace panda::compiler
