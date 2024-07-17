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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H
#define COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H

#include <unordered_map>
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/pass.h"
#include "utils/arena_containers.h"
#include "utils/hash.h"

namespace ark::compiler {
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
    // Valid fields
    // * in common: base, idx
    // * in case LoadArrayPair/StoreArrayPair: base, idx, imm
    // * in case LoadArrayPairI/StoreArrayPairI, LoadArrayI/StoreArrayI: base, imm
    ARRAY_ELEMENT,
    // Dictionary pointer
    // Valid fields: base, idx
    DICTIONARY_ELEMENT
};

class Pointer {
public:
    Pointer() = default;
    Pointer(PointerType type, const Inst *base, const Inst *idx, uint64_t imm, const void *typePtr)
        : type_(type), base_(base), idx_(idx), imm_(imm), typePtr_(typePtr), volatile_(false)
    {
        local_ = base == nullptr ? false : IsLocalAlias(base);
    };

    static Pointer CreateObject(const Inst *base)
    {
        return Pointer(OBJECT, base, nullptr, 0, nullptr);
    }

    static Pointer CreatePoolConstant(uint32_t typeId)
    {
        return Pointer(POOL_CONSTANT, nullptr, nullptr, typeId, nullptr);
    }

    static Pointer CreateStaticField(uint32_t typeId, const void *typePtr = nullptr)
    {
        return Pointer(STATIC_FIELD, nullptr, nullptr, typeId, typePtr);
    }

    static Pointer CreateObjectField(const Inst *base, uint32_t typeId, const void *typePtr = nullptr)
    {
        return Pointer(OBJECT_FIELD, base, nullptr, typeId, typePtr);
    }

    static Pointer CreateArrayElement(const Inst *array, const Inst *idx, uint64_t imm = 0)
    {
        return Pointer(ARRAY_ELEMENT, array, idx, imm, nullptr);
    }

    static Pointer CreateDictionaryElement(const Inst *array, const Inst *idx, uint64_t imm = 0)
    {
        return Pointer(DICTIONARY_ELEMENT, array, idx, imm, nullptr);
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
        return typePtr_;
    }

    bool IsLocal() const
    {
        return local_;
    }

    void SetLocalVolatile(bool local, bool isVolatile)
    {
        local_ = local;
        volatile_ = isVolatile;
    }

    bool IsVolatile() const
    {
        return volatile_;
    }

    void SetVolatile(bool isVolatile)
    {
        volatile_ = isVolatile;
    }

    void Dump(std::ostream *out) const;

    bool HasSameOffset(const Pointer &p) const
    {
        if (typePtr_ == nullptr && p.typePtr_ == nullptr) {
            return imm_ == p.imm_;
        }
        return typePtr_ == p.typePtr_;
    }

private:
    /**
     * Returns true if reference is used only in scope of current function: it
     * must be created in the function and must not escape its scope
     */
    static bool IsLocalAlias(const Inst *inst)
    {
        switch (inst->GetOpcode()) {
            case Opcode::NullPtr:
                return true;
            case Opcode::NewArray:
            case Opcode::MultiArray:
            case Opcode::NewObject:
            case Opcode::InitObject:
            case Opcode::InitEmptyString:
            case Opcode::InitString:
                return !IsEscapingAlias(inst);
            case Opcode::NullCheck:
            case Opcode::RefTypeCheck:
            case Opcode::ObjByIndexCheck:
                UNREACHABLE();
                /* fall-through */
            default:
                return false;
        }
    }

    /**
     * Returns true if a reference escapes the scope of current function:
     * Various function calls, constructors and stores to another objects' fields, arrays
     */
    static bool IsEscapingAlias(const Inst *inst)
    {
        for (auto &user : inst->GetUsers()) {
            switch (user.GetInst()->GetOpcode()) {
                case Opcode::NullCheck:
                case Opcode::RefTypeCheck:
                case Opcode::ObjByIndexCheck:
                    if (IsEscapingAlias(user.GetInst())) {
                        return true;
                    }
                    break;
                case Opcode::StoreObject:
                case Opcode::StoreObjectPair:
                case Opcode::StoreArray:
                case Opcode::StoreArrayI:
                case Opcode::StoreArrayPair:
                case Opcode::StoreArrayPairI:
                    if (user.GetIndex() != 0) {
                        return true;
                    }
                    break;
                case Opcode::StoreStatic:
                case Opcode::StoreResolvedObjectField:
                case Opcode::StoreResolvedObjectFieldStatic:
                case Opcode::UnresolvedStoreStatic:
                case Opcode::InitObject:
                case Opcode::InitClass:
                case Opcode::LoadAndInitClass:
                case Opcode::UnresolvedLoadAndInitClass:
                case Opcode::GetGlobalVarAddress:
                case Opcode::CallResolvedStatic:
                case Opcode::CallResolvedVirtual:
                case Opcode::CallStatic:
                case Opcode::CallVirtual:
                case Opcode::CallDynamic:
                    return true;
                case Opcode::Intrinsic:
                    if (inst->GetFlagsMask() != 0) {
                        return true;
                    }
                    break;
                default:
                    break;
            }
        }
        return false;
    }

private:
    PointerType type_;
    const Inst *base_;
    const Inst *idx_;
    uint64_t imm_;
    const void *typePtr_;
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
        auto instHasher = std::hash<const Inst *> {};
        uint32_t hash = instHasher(p.GetBase());
        hash += instHasher(p.GetIdx());
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
    AliasType CheckRefAlias(Inst *ref1, Inst *ref2) const;
    AliasType CheckMemAddress(const Pointer &p1, const Pointer &p2) const;
    void Dump(std::ostream *out) const;

    /**
     * Sort IR instructions into two constraint groups:
     *     Direct: introduce the alias
     *     Copy: copy one alias to another
     */
    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override;

    /// Instructions that definitely are not an alias of anything.
    static void VisitNullPtr(GraphVisitor *v, Inst *inst);
    static void VisitLoadUndefined(GraphVisitor *v, Inst *inst);
    static void VisitInitObject(GraphVisitor *v, Inst *inst);
    static void VisitNewObject(GraphVisitor *v, Inst *inst);
    static void VisitNewArray(GraphVisitor *v, Inst *inst);
    static void VisitMultiArray(GraphVisitor *v, Inst *inst);
    static void VisitInitEmptyString(GraphVisitor *v, Inst *inst);
    static void VisitInitString(GraphVisitor *v, Inst *inst);
    /**
     * Instructions that can introduce references that are an alias of
     * something already existed.
     */
    static void VisitConstant(GraphVisitor *v, Inst *inst);
    static void VisitParameter(GraphVisitor *v, Inst *inst);
    static void VisitLoadImmediate(GraphVisitor *v, Inst *inst);
    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);
    static void VisitBuiltin(GraphVisitor *v, Inst *inst);
    static void VisitCallStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallResolvedStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallVirtual(GraphVisitor *v, Inst *inst);
    static void VisitCallDynamic(GraphVisitor *v, Inst *inst);
    static void VisitCallResolvedVirtual(GraphVisitor *v, Inst *inst);
    static void VisitGetManagedClassObject(GraphVisitor *v, Inst *inst);
    static void VisitResolveObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitBitcast(GraphVisitor *v, Inst *inst);

    /// Instructions that introduce static fields (global variables).
    static void VisitLoadStatic(GraphVisitor *v, Inst *inst);
    static void VisitLoadResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitStoreStatic(GraphVisitor *v, Inst *inst);
    static void VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst);

    /// Instructions that introduce unique constant references (global constants).
    static void VisitLoadRuntimeClass(GraphVisitor *v, Inst *inst);
    static void VisitLoadClass(GraphVisitor *v, Inst *inst);
    static void VisitLoadAndInitClass(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadAndInitClass(GraphVisitor *v, Inst *inst);
    static void VisitGetGlobalVarAddress(GraphVisitor *v, Inst *inst);
    static void VisitLoadString(GraphVisitor *v, Inst *inst);
    static void VisitLoadConstArray(GraphVisitor *v, Inst *inst);
    static void VisitLoadType(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadType(GraphVisitor *v, Inst *inst);
    static void VisitLoadObjFromConst(GraphVisitor *v, Inst *inst);

    /// Instructions that introduce aliases.
    static void VisitLoadArray(GraphVisitor *v, Inst *inst);
    static void VisitStoreArray(GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayI(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayI(GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayPair(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayPair(GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayPairI(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayPairI(GraphVisitor *v, Inst *inst);
    static void VisitLoadObject(GraphVisitor *v, Inst *inst);
    static void VisitLoadResolvedObjectField(GraphVisitor *v, Inst *inst);
    static void VisitStoreObject(GraphVisitor *v, Inst *inst);
    static void VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst);
    static void VisitCatchPhi(GraphVisitor *v, Inst *inst);
    static void VisitPhi(GraphVisitor *v, Inst *inst);
    static void VisitSelect(GraphVisitor *v, Inst *inst);
    static void VisitSelectImm(GraphVisitor *v, Inst *inst);
    static void VisitMov(GraphVisitor *v, Inst *inst);
    static void VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst);
    static void VisitGetAnyTypeName(GraphVisitor *v, Inst *inst);
    static void VisitLoadConstantPool(GraphVisitor *v, Inst *inst);
    static void VisitLoadLexicalEnv(GraphVisitor *v, Inst *inst);
    static void VisitLoadObjectPair(GraphVisitor *v, Inst *inst);
    static void VisitStoreObjectPair(GraphVisitor *v, Inst *inst);

    /// Dynamic instructions
    static void VisitLoadObjectDynamic(GraphVisitor *v, Inst *inst);
    static void VisitStoreObjectDynamic(GraphVisitor *v, Inst *inst);
    static void VisitLoadFromConstantPool(GraphVisitor *v, Inst *inst);

    void VisitDefault([[maybe_unused]] Inst *inst) override;

    void AddDirectEdge(const Pointer &p)
    {
        direct_->push_back({p, p});
    }

    void AddConstantDirectEdge(Inst *inst, uint32_t id)
    {
        direct_->push_back({Pointer::CreateObject(inst), Pointer::CreatePoolConstant(id)});
    }

    void AddCopyEdge(const Pointer &from, const Pointer &to)
    {
        copy_->push_back({from, to});
    }

    ArenaSet<Inst *> *GetClearInputsSet()
    {
        inputsSet_->clear();
        return inputsSet_;
    }

#include "optimizer/ir/visitor.inc"

private:
    void Init();
    using PointerSet = ArenaUnorderedSet<Pointer, PointerHash, PointerEqual>;
    template <class T>
    using PointerMap = ArenaUnorderedMap<Pointer, T, PointerHash, PointerEqual>;

    void SolveConstraints();

    /* Methods to extract pointer from instruction */
    static bool ParseInstruction(Inst *inst, Pointer *pointer);
    static Pointer ParseArrayElement(Inst *inst);
    static Pointer ParsePoolConstant(Inst *inst);
    static Pointer ParseStaticField(Inst *inst);
    static Pointer ParseObjectField(Inst *inst);
    static Pointer ParseDynamicField(Inst *inst);

    static Trilean IsSameOffsets(const Inst *off1, const Inst *off2);
    static AliasType AliasingTwoArrayPointers(const Pointer *p1, const Pointer *p2);

    static AliasType SingleIntersectionAliasing(const Pointer &p1, const Pointer &p2, const Pointer *intersection);
    AliasType CheckMemAddressEmptyIntersectionCase(const PointerSet &aliases1, const PointerSet &aliases2,
                                                   const Pointer &p1, const Pointer &p2) const;
    void SolveConstraintsMainLoop(Pointer &ref, Pointer &edge, bool &added, PointerSet &sols);
    void DumpChains(std::ostream *out) const;
    void DumpDirect(std::ostream *out) const;
    void DumpCopy(std::ostream *out) const;

private:
    PointerMap<PointerSet> pointsTo_;

    // Local containers:
    PointerMap<ArenaVector<Pointer>> *chains_ {nullptr};
    PointerPairVector *direct_ {nullptr};
    PointerPairVector *copy_ {nullptr};
    ArenaSet<Inst *> *inputsSet_ {nullptr};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H
