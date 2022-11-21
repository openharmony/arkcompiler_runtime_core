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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H_
#define COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H_

#include <unordered_map>
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/pass.h"
#include "utils/arena_containers.h"
#include "utils/hash.h"

namespace panda::compiler {
class BasicBlock;
class Graph;

enum AliasType : uint8_t {
    // Proved that references are not aliases
    NO_ALIAS,
    // References may or may not alias each other (cannot be proven statically)
    MAY_ALIAS,
    // References are proven aliases
    MUST_ALIAS
};

enum PointerType {
    // Reference to unknown object.
    // Valid fields: base
    OBJECT,
    // Constant from pool
    // Valid fields: imm
    POOL_CONSTANT,
    // Object's field
    // Valid fields: base, imm, type_ptr
    OBJECT_FIELD,
    // Static field of the object
    // Valid fields: imm, type_ptr
    STATIC_FIELD,
    // Array pointer
    // Valid fields: base, idx
    ARRAY_ELEMENT
};

class Pointer {
public:
    Pointer() = default;
    Pointer(PointerType type, const Inst *base, const Inst *idx, uint64_t imm, const void *type_ptr)
        : type_(type), base_(base), idx_(idx), imm_(imm), type_ptr_(type_ptr), volatile_(false)
    {
        local_ = false;
    };

    static Pointer CreateObject(const Inst *base)
    {
        return Pointer(OBJECT, base, nullptr, 0, nullptr);
    }

    static Pointer CreateObjectField(const Inst *base, uint32_t type_id, const void *type_ptr = nullptr)
    {
        return Pointer(OBJECT_FIELD, base, nullptr, type_id, type_ptr);
    }

    static Pointer CreateArrayElement(const Inst *array, const Inst *idx, uint64_t imm = 0)
    {
        return Pointer(ARRAY_ELEMENT, array, idx, imm, nullptr);
    }

    PointerType GetType() const
    {
        return type_;
    }

    const Inst *GetBase() const
    {
        return base_;
    }

    const Inst *GetIdx() const
    {
        return idx_;
    }

    uint64_t GetImm() const
    {
        return imm_;
    }

    const void *GetTypePtr() const
    {
        return type_ptr_;
    }

    bool IsLocal() const
    {
        return local_;
    }

    void SetLocalVolatile(bool local, bool is_volatile)
    {
        local_ = local;
        volatile_ = is_volatile;
    }

    bool IsVolatile() const
    {
        return volatile_;
    }

    void Dump(std::ostream *out) const;

    bool HasSameOffset(const Pointer &p) const
    {
        if (type_ptr_ == nullptr && p.type_ptr_ == nullptr) {
            return imm_ == p.imm_;
        }
        return type_ptr_ == p.type_ptr_;
    }

private:
    PointerType type_;
    const Inst *base_;
    const Inst *idx_;
    uint64_t imm_;
    const void *type_ptr_;
    bool local_;
    bool volatile_;
};

struct PointerEqual {
    bool operator()(Pointer const &p1, Pointer const &p2) const
    {
        return p1.GetType() == p2.GetType() && p1.GetBase() == p2.GetBase() && p1.GetIdx() == p2.GetIdx() &&
               p1.HasSameOffset(p2);
    }
};

struct PointerHash {
    uint32_t operator()(Pointer const &p) const
    {
        auto inst_hasher = std::hash<const Inst *> {};
        uint32_t hash = inst_hasher(p.GetBase());
        hash += inst_hasher(p.GetIdx());
        if (p.GetTypePtr() == nullptr) {
            hash += std::hash<uint64_t> {}(p.GetImm());
        } else {
            hash += std::hash<const void *> {}(p.GetTypePtr());
        }
        return hash;
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class AliasAnalysis : public Analysis, public GraphVisitor {
public:
    enum class Trilean {
        TRUE,
        UNKNOWN,
        FALSE,
    };

    using PointerPairVector = ArenaVector<std::pair<Pointer, Pointer>>;

    explicit AliasAnalysis(Graph *graph);
    NO_MOVE_SEMANTIC(AliasAnalysis);
    NO_COPY_SEMANTIC(AliasAnalysis);
    ~AliasAnalysis() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "AliasAnalysis";
    }

    AliasType CheckInstAlias(Inst *mem1, Inst *mem2) const;
    void Dump(std::ostream *out) const;

    /**
     * Sort IR instructions into two constraint groups:
     *     Direct: introduce the alias
     *     Copy: copy one alias to another
     */
    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override;

    /**
     * Instructions that introduce aliases.
     */
    static void VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst);

    void AddDirectEdge(const Pointer &p)
    {
        direct_->push_back({p, p});
    }

    ArenaSet<Inst *> *GetClearInputsSet()
    {
        inputs_set_->clear();
        return inputs_set_;
    }

#include "optimizer/ir/visitor.inc"

private:
    void Init();
    using PointerSet = ArenaUnorderedSet<Pointer, PointerHash, PointerEqual>;
    template <class T>
    using PointerMap = ArenaUnorderedMap<Pointer, T, PointerHash, PointerEqual>;

    void SolveConstraints();

    void DumpChains(std::ostream *out) const;

private:
    PointerMap<PointerSet> points_to_;

    // Local containers:
    PointerMap<ArenaVector<Pointer>> *chains_ {nullptr};
    PointerPairVector *direct_ {nullptr};
    ArenaSet<Inst *> *inputs_set_ {nullptr};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H_
