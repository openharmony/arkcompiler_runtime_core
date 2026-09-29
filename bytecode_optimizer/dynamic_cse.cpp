/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "dynamic_cse.h"

#include <array>
#include <cstdint>
#include <utility>

#include "compiler/optimizer/analysis/dominators_tree.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "libpandabase/utils/arena_containers.h"

namespace panda::bytecodeopt {
namespace {

using IntrinsicId = compiler::RuntimeInterface::IntrinsicId;

struct ExpressionKey {
    IntrinsicId id;
    compiler::DataType::Type type;
    std::array<uint32_t, 2U> inputVns;
    size_t inputCount;

    bool operator==(const ExpressionKey &other) const
    {
        return id == other.id && type == other.type && inputVns == other.inputVns && inputCount == other.inputCount;
    }
};

struct ExpressionKeyHash {
    size_t operator()(const ExpressionKey &key) const
    {
        constexpr size_t HASH_SHIFT = 6U;
        size_t hash = static_cast<size_t>(key.id);
        hash ^= static_cast<size_t>(key.type) + (hash << HASH_SHIFT) + (hash >> 2U);
        for (auto vn : key.inputVns) {
            hash ^= static_cast<size_t>(vn) + (hash << HASH_SHIFT) + (hash >> 2U);
        }
        hash ^= key.inputCount + (hash << HASH_SHIFT) + (hash >> 2U);
        return hash;
    }
};

struct SuperCheckKey {
    compiler::Inst *input;
    uint32_t immediate;

    bool operator==(const SuperCheckKey &other) const
    {
        return input == other.input && immediate == other.immediate;
    }
};

struct SuperCheckKeyHash {
    size_t operator()(const SuperCheckKey &key) const
    {
        constexpr size_t HASH_SHIFT = 6U;
        auto hash = reinterpret_cast<uintptr_t>(key.input);
        hash ^= static_cast<size_t>(key.immediate) + (hash << HASH_SHIFT) + (hash >> 2U);
        return hash;
    }
};

bool IsSuperCheck(IntrinsicId id)
{
    return id == IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8 ||
           id == IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM16;
}

bool BuildSuperCheckKey(compiler::Inst *inst, SuperCheckKey *key)
{
    ASSERT(inst->IsIntrinsic());
    auto *intrinsic = inst->CastToIntrinsic();
    if (!IsSuperCheck(intrinsic->GetIntrinsicId()) || inst->GetType() != compiler::DataType::VOID ||
        !intrinsic->HasImms() || intrinsic->GetImms().size() != 1U) {
        return false;
    }

    auto immediate = intrinsic->GetImms()[0];
    if (immediate > 1U) {
        return false;
    }

    compiler::Inst *dataInput = nullptr;
    for (size_t index = 0; index < inst->GetInputsCount(); ++index) {
        if (inst->GetInputType(index) == compiler::DataType::NO_TYPE) {
            continue;
        }
        if (dataInput != nullptr) {
            return false;
        }
        dataInput = inst->GetDataFlowInput(inst->GetInput(index).GetInst());
    }
    if (dataInput == nullptr) {
        return false;
    }

    *key = {dataInput, immediate};
    return true;
}

size_t GetExpectedInputCount(IntrinsicId id)
{
    switch (id) {
        case IntrinsicId::ISTRUE:
        case IntrinsicId::ISFALSE:
            return 1U;
        case IntrinsicId::STRICTEQ_IMM8_V8:
        case IntrinsicId::STRICTNOTEQ_IMM8_V8:
            return 2U;
        default:
            return 0U;
    }
}

bool BuildKey(compiler::Inst *inst, ExpressionKey *key)
{
    ASSERT(inst->IsIntrinsic());
    auto *intrinsic = inst->CastToIntrinsic();
    auto expectedInputCount = GetExpectedInputCount(intrinsic->GetIntrinsicId());
    if (expectedInputCount == 0U) {
        return false;
    }

    std::array<uint32_t, 2U> inputVns {};
    size_t dataInputIndex = 0;
    for (size_t index = 0; index < inst->GetInputsCount(); ++index) {
        if (inst->GetInputType(index) == compiler::DataType::NO_TYPE) {
            continue;
        }
        if (dataInputIndex >= inputVns.size()) {
            return false;
        }
        auto *input = inst->GetDataFlowInput(inst->GetInput(index).GetInst());
        if (input->GetVN() == compiler::INVALID_VN) {
            return false;
        }
        inputVns[dataInputIndex++] = input->GetVN();
    }
    if (dataInputIndex != expectedInputCount) {
        return false;
    }

    *key = {intrinsic->GetIntrinsicId(), inst->GetType(), inputVns, dataInputIndex};
    return true;
}

bool IsAlreadyLiveAcross(compiler::Inst *producer, compiler::Inst *current)
{
    for (const auto &user : producer->GetUsers()) {
        auto *userInst = user.GetInst();
        if (userInst != nullptr && !userInst->IsPhi() && !userInst->IsCatchPhi() && current->IsDominate(userInst)) {
            return true;
        }
    }
    return false;
}

bool IsEhBlock(const compiler::BasicBlock *block)
{
    return block->IsTry() || block->IsTryBegin() || block->IsTryEnd() || block->IsCatch() || block->IsCatchBegin() ||
           block->IsCatchEnd();
}

bool IsSafeRegion(compiler::Inst *producer, compiler::Inst *current)
{
    auto *producerBlock = producer->GetBasicBlock();
    auto *currentBlock = current->GetBasicBlock();
    if (producerBlock == currentBlock) {
        return true;
    }
    return !IsEhBlock(producerBlock) && !IsEhBlock(currentBlock);
}

using CandidateList = ArenaVector<compiler::Inst *>;

template <typename Key, typename KeyHash>
using CandidateMap = ArenaUnorderedMap<Key, CandidateList, KeyHash>;

// Returns an already collected candidate equivalent to 'inst' when it dominates 'inst'
// within a safe EH region (and additionally stays live across 'inst' when required for
// value-producing expressions); otherwise records 'inst' as a new candidate and returns nullptr.
template <typename Key, typename KeyHash>
compiler::Inst *FindDominatedDuplicate(compiler::Inst *inst, const Key &key, ArenaAllocator *allocator,
                                       CandidateMap<Key, KeyHash> &candidates, bool requireProducerLiveAcross)
{
    auto &equivalentInsts = candidates.try_emplace(key, allocator->Adapter()).first->second;
    for (auto *producer : equivalentInsts) {
        if (!producer->IsDominate(inst) || !IsSafeRegion(producer, inst)) {
            continue;
        }
        if (requireProducerLiveAcross && !IsAlreadyLiveAcross(producer, inst)) {
            continue;
        }
        return producer;
    }
    equivalentInsts.push_back(inst);
    return nullptr;
}

bool ProcessInstruction(compiler::Inst *inst, ArenaAllocator *allocator,
                        CandidateMap<ExpressionKey, ExpressionKeyHash> &candidates,
                        CandidateMap<SuperCheckKey, SuperCheckKeyHash> &superCheckCandidates)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }

    SuperCheckKey superCheckKey {};
    if (BuildSuperCheckKey(inst, &superCheckKey)) {
        if (FindDominatedDuplicate(inst, superCheckKey, allocator, superCheckCandidates, false) != nullptr) {
            inst->ClearFlag(compiler::inst_flags::NO_DCE);
            return true;
        }
        return false;
    }

    ExpressionKey key {};
    if (!BuildKey(inst, &key)) {
        return false;
    }

    auto *producer = FindDominatedDuplicate(inst, key, allocator, candidates, true);
    if (producer == nullptr) {
        return false;
    }
    inst->ReplaceUsers(producer);
    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    return true;
}

}  // namespace

bool DynamicCse::RunImpl()
{
    GetGraph()->RunPass<compiler::DominatorsTree>();
    bool changed = false;
    const auto allocator = GetGraph()->GetLocalAllocator();
    CandidateMap<ExpressionKey, ExpressionKeyHash> candidates(allocator->Adapter());
    CandidateMap<SuperCheckKey, SuperCheckKeyHash> superCheckCandidates(allocator->Adapter());
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : block->AllInsts()) {
            changed = ProcessInstruction(inst, allocator, candidates, superCheckCandidates) || changed;
        }
    }
    return changed;
}

}  // namespace panda::bytecodeopt
