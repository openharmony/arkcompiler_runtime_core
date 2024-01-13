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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_COLOR_GRAPH_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_COLOR_GRAPH_H

#include <algorithm>
#include <climits>
#include "compiler_logger.h"
#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/analysis/liveness_analyzer.h"
#include <iostream>
#include "libpandabase/macros.h"
#include "libpandabase/utils/bit_utils.h"
#include "optimizer/code_generator/operands.h"
#include <unordered_set>
#include "utils/arena_containers.h"
#include "utils/small_vector.h"

namespace panda::compiler {
class LifeIntervals;

constexpr size_t CALLEE_THRESHOLD = 2;

class ColorNode {
public:
    // Special color saying that color not in free registers but should be used on mapping
    static constexpr uint16_t NO_BIAS = 0xffff;

    template <typename T>
    ColorNode(unsigned number, T alloc) : csPointSet_(alloc), number_(number), physical_(), fixed_()
    {
    }

    unsigned GetNumber() const noexcept
    {
        return number_;
    }

    void SetColor(Register color) noexcept
    {
        color_ = color;
    }

    Register GetColor() const noexcept
    {
        return color_;
    }

    void SetBias(uint16_t bias) noexcept
    {
        bias_ = bias;
    }

    bool HasBias() const noexcept
    {
        return bias_ != NO_BIAS;
    }

    uint16_t GetBias() const noexcept
    {
        return bias_;
    }

    void Assign(LifeIntervals *lifeIntervals) noexcept
    {
        lifeIntervals_ = lifeIntervals;
    }

    bool IsFixedColor() const noexcept
    {
        return fixed_;
    }

    void SetPhysical() noexcept
    {
        physical_ = true;
    }

    bool IsPhysical() const noexcept
    {
        return physical_;
    }

    void SetFixedColor(Register color) noexcept
    {
        fixed_ = true;
        SetColor(color);
    }

    LifeIntervals *GetLifeIntervals() const noexcept
    {
        return lifeIntervals_;
    }

    void AddCallsite(LifeNumber point) noexcept
    {
        csPointSet_.insert(point);
    }

    unsigned GetCallsiteIntersectCount() const noexcept
    {
        return csPointSet_.size();
    }

    void SetSpillWeight(float weight)
    {
        weight_ = weight;
    }

    float GetSpillWeight() const
    {
        return weight_;
    }

private:
    LifeIntervals *lifeIntervals_ = nullptr;
    ArenaUnorderedSet<LifeNumber> csPointSet_;
    unsigned number_;
    float weight_ {};
    uint16_t bias_ = NO_BIAS;  // Bias is a group of nodes for coalescing
    Register color_ = INVALID_REG;
    uint8_t physical_ : 1;
    uint8_t fixed_ : 1;
};

class GraphMatrix {
public:
    explicit GraphMatrix(ArenaAllocator *alloc) : matrix_(alloc->Adapter()) {}
    void SetCapacity(unsigned capacity)
    {
        capacity_ = RoundUp(capacity, sizeof(uintptr_t));
        matrix_.clear();
        matrix_.resize(capacity_ * capacity_);
    }

    bool AddEdge(unsigned a, unsigned b);

    bool HasEdge(unsigned a, unsigned b) const
    {
        return matrix_[FindEdge(a, b)];
    }

    bool AddAffinityEdge(unsigned a, unsigned b);

    bool HasAffinityEdge(unsigned a, unsigned b) const
    {
        return matrix_[FindAffinityEdge(a, b)];
    }

    unsigned GetCapacity() const noexcept
    {
        return capacity_;
    }

private:
    template <typename T>
    void OrderNodes(T &a, T &b) const noexcept
    {
        ASSERT(a < capacity_);
        ASSERT(b < capacity_);
        if (a < b) {
            std::swap(a, b);
        }
    }

    unsigned FindEdge(unsigned a, unsigned b) const
    {
        // Placement in lower triangulated adjacency matrix
        OrderNodes(a, b);
        return a * capacity_ + b;
    }

    unsigned FindAffinityEdge(unsigned a, unsigned b) const
    {
        // Placement in lower triangulated adjacency matrix
        OrderNodes(a, b);
        return b * capacity_ + a;
    }

    ArenaVector<bool> matrix_;
    unsigned capacity_ = 0;
};

using NodeVector = ArenaVector<ColorNode>;

// NOTE (Evgeny.Erokhin): In Appel's book described usage of 2 structures in parallel to hold interference:
// one is for random checks (here is a matrix) and lists on adjacency for sequental access. It's worth to add!
class InterferenceGraph {
public:
    explicit InterferenceGraph(ArenaAllocator *alloc) : nodes_(alloc->Adapter()), matrix_(alloc), useSpillWeight_() {}
    ColorNode *AllocNode();

    NodeVector &GetNodes() noexcept
    {
        return nodes_;
    }

    const NodeVector &GetNodes() const noexcept
    {
        return nodes_;
    }

    ColorNode &GetNode(unsigned num)
    {
        return nodes_[num];
    }

    const ColorNode &GetNode(unsigned num) const
    {
        return nodes_[num];
    }

    const ColorNode *FindNode(const LifeIntervals *li) const
    {
        auto it = std::find_if(nodes_.begin(), nodes_.end(),
                               [li](const auto &node) { return li == node.GetLifeIntervals(); });

        return it != nodes_.end() ? &*it : nullptr;
    }

    const ColorNode *FindPhysicalNode(Location location) const
    {
        auto it = std::find_if(nodes_.begin(), nodes_.end(), [location](const auto &node) {
            return node.IsPhysical() && node.GetLifeIntervals()->GetLocation() == location;
        });

        return it != nodes_.end() ? &*it : nullptr;
    }

    unsigned Size() const noexcept
    {
        return nodes_.size();
    }

    void Reserve(size_t count);

    struct Bias {
        unsigned callsites = 0;
        Register color = INVALID_REG;
    };

    Bias &AddBias() noexcept
    {
        return biases_.emplace_back();
    }

    void UpdateBiasData(Bias *bias, const ColorNode &node)
    {
        ASSERT(bias != nullptr);
        if (node.GetColor() != INVALID_REG) {
            bias->color = node.GetColor();
        }
        bias->callsites += node.GetCallsiteIntersectCount();
    }

    unsigned GetBiasCount() const noexcept
    {
        return biases_.size();
    }

    void AddEdge(unsigned a, unsigned b);
    bool HasEdge(unsigned a, unsigned b) const;
    void AddAffinityEdge(unsigned a, unsigned b);
    bool HasAffinityEdge(unsigned a, unsigned b) const;

    // Build LexBFS, so reverse order gives minimal coloring
    ArenaVector<unsigned> LexBFS() const;

    // Get nodes ids in order they will be colored
    ArenaVector<unsigned> GetOrderedNodesIds() const;

    void SetUseSpillWeight(bool use)
    {
        useSpillWeight_ = use;
    }

    bool IsUsedSpillWeight() const
    {
        return useSpillWeight_;
    }

    bool IsChordal() const;
    void Dump(const std::string &name = "IG", bool skipPhysical = true, std::ostream &out = std::cout) const;

    template <unsigned MAX_COLORS>
    bool AssignColors(size_t colors, size_t calleeOffset)
    {
        ASSERT(colors <= MAX_COLORS);
        std::bitset<MAX_COLORS> nbrColors;
        std::bitset<MAX_COLORS> nbrBiasColors;

        bool success = true;
        for (unsigned id : GetOrderedNodesIds()) {
            auto &node = GetNode(id);
            COMPILER_LOG(DEBUG, REGALLOC)
                << "Visit Node " << node.GetNumber() << "; LI: " << node.GetLifeIntervals()->ToString()
                << "; SW: " << node.GetSpillWeight();
            // Skip colored
            if (node.GetColor() != INVALID_REG) {
                continue;
            }

            // Make busy colors maps
            MakeBusyBitmap(id, &nbrColors, &nbrBiasColors);

            // Try bias color first if free
            size_t tryColor;
            if (node.HasBias() && biases_[node.GetBias()].color != INVALID_REG &&
                !nbrColors[biases_[node.GetBias()].color]) {
                tryColor = biases_[node.GetBias()].color;
                COMPILER_LOG(DEBUG, REGALLOC) << "Bias color chosen " << tryColor;
            } else {
                // The best case is to find color that is not in neighbour colors and not in biased
                nbrBiasColors |= nbrColors;  // Make compound busy bitmap

                // For nodes that take part in bias with callsites intersection, prefer callee saved registers.
                // In cases first interval of bias isn't intersected it will be placed in callersaved register,
                // that will affect entire coalesced bias group.
                size_t off = 0;
                if (node.HasBias() && biases_[node.GetBias()].callsites > 0) {
                    off = calleeOffset;
                }

                // Find first not allocated disregard biasing
                if ((tryColor = FirstFree(nbrColors, nbrBiasColors, colors, off)) == colors) {
                    node.SetColor(tryColor);
                    success = false;
                    continue;
                }

                // Assign bias color if first observed in component
                if (node.HasBias() && biases_[node.GetBias()].color == INVALID_REG) {
                    biases_[node.GetBias()].color = tryColor;
                    COMPILER_LOG(DEBUG, REGALLOC) << "Set bias color " << tryColor;
                }
            }

            // Assign color
            node.SetColor(tryColor);
            COMPILER_LOG(DEBUG, REGALLOC) << "Node " << node.GetNumber() << ": Set color: "
                                          << " " << unsigned(node.GetColor());
        }
        return success;
    }

private:
    template <typename T>
    void MakeBusyBitmap(unsigned id, T *nbrColors, T *nbrBiasColors)
    {
        nbrColors->reset();
        nbrBiasColors->reset();
        // Collect neighbors colors
        for (unsigned nbrId = 0; nbrId < Size(); nbrId++) {
            auto &nbrNode = GetNode(nbrId);

            // Collect neighbour color
            if (nbrNode.GetColor() != INVALID_REG && HasEdge(id, nbrId)) {
                ASSERT(nbrNode.GetColor() < nbrColors->size());
                nbrColors->set(nbrNode.GetColor());
            } else if (nbrId != id && nbrNode.HasBias() && HasEdge(id, nbrId)) {
                // Collect biased neighbour color
                ASSERT(nbrNode.GetBias() < GetBiasCount());
                if (biases_[nbrNode.GetBias()].color != INVALID_REG) {
                    nbrBiasColors->set(biases_[nbrNode.GetBias()].color);
                }
            }
        }
    }

    template <typename T>
    static size_t FirstFree(const T &nbrBs, const T &nbrBsBias, size_t colors, size_t off)
    {
        // Find first free
        size_t tryColor;
        for (tryColor = off; tryColor < colors + off; tryColor++) {
            if (!nbrBs[tryColor % colors]) {
                break;
            }
        }

        // Find free regarding biasing (higher part of bitmap)
        for (auto i = tryColor; i < colors + off; i++) {
            if (!nbrBsBias[i % colors]) {
                tryColor = i;
                break;
            }
        }

        return tryColor == colors + off ? colors : tryColor % colors;
    }

    NodeVector nodes_;
    GraphMatrix matrix_;
    static const size_t DEFAULT_BIAS_SIZE = 16;
    SmallVector<Bias, DEFAULT_BIAS_SIZE> biases_;
    uint8_t useSpillWeight_ : 1;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_COLOR_GRAPH_H
