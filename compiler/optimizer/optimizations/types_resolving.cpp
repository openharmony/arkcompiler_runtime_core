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

#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/types_analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/types_resolving.h"
#ifndef __clang_analyzer__
#include "irtoc_ir_inline.h"
#endif

namespace panda::compiler {
TypesResolving::TypesResolving(Graph *graph)
    : Optimization(graph), types_ {graph->GetLocalAllocator()->Adapter()}, phis_ {graph->GetLocalAllocator()->Adapter()}
{
}

void TypesResolving::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

bool TypesResolving::RunImpl()
{
    bool is_applied = false;
    GetGraph()->RunPass<TypesAnalysis>();
    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (bb == nullptr || bb->IsEmpty()) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            if (GetGraph()->GetVectorBlocks()[inst->GetBasicBlock()->GetId()] == nullptr) {
                break;
            }
            if (inst->GetOpcode() != Opcode::Intrinsic) {
                continue;
            }
            auto intrinsics_inst = inst->CastToIntrinsic();
            if (!intrinsics_inst->CanBeInlined()) {
                continue;
            }
            is_applied |= TryInline(intrinsics_inst);
        }
    }
    is_applied |= TryResolvePhi();
    return is_applied;
}

AnyBaseType TypesResolving::GetAssumedAnyType(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::Phi:
            return inst->CastToPhi()->GetAssumedAnyType();
        case Opcode::CastValueToAnyType:
            return inst->CastToCastValueToAnyType()->GetAnyType();
        case Opcode::AnyTypeCheck: {
            auto type = inst->CastToAnyTypeCheck()->GetAnyType();
            if (type == AnyBaseType::UNDEFINED_TYPE) {
                return GetAssumedAnyType(inst->GetInput(0).GetInst());
            }
            return type;
        }
        default:
            return AnyBaseType::UNDEFINED_TYPE;
    }
}

bool TypesResolving::DoInline(IntrinsicInst *intrinsic)
{
    switch (intrinsic->GetIntrinsicId()) {
#ifndef __clang_analyzer__
#include "intrinsics_inline.inl"
#endif
        default: {
            return false;
        }
    }
}

bool TypesResolving::CheckInputsAnyTypesRec(Inst *phi)
{
    ASSERT(phi->IsPhi());
    if (std::find(phis_.begin(), phis_.end(), phi) != phis_.end()) {
        return true;
    }
    phis_.push_back(phi);
    for (auto &input : phi->GetInputs()) {
        auto input_inst = phi->GetDataFlowInput(input.GetInst());
        if (input_inst->GetOpcode() == Opcode::Phi) {
            if (!CheckInputsAnyTypesRec(input_inst)) {
                return false;
            }
            continue;
        }
        if (input_inst->GetOpcode() != Opcode::CastValueToAnyType) {
            return false;
        }
        auto type = input_inst->CastToCastValueToAnyType()->GetAnyType();
        ASSERT(type != AnyBaseType::UNDEFINED_TYPE);
        if (any_type_ == AnyBaseType::UNDEFINED_TYPE) {
            // We can't propogate opject, because GC can move it
            if (AnyBaseTypeToDataType(type) == DataType::REFERENCE) {
                return false;
            }
            any_type_ = type;
            continue;
        }
        if (any_type_ != type) {
            return false;
        }
    }
    return true;
}

void TypesResolving::PropagateTypeToPhi()
{
    auto new_type = AnyBaseTypeToDataType(any_type_);
    for (auto phi : phis_) {
        phi->SetType(new_type);
        size_t inputs_count = phi->GetInputsCount();
        for (size_t idx = 0; idx < inputs_count; ++idx) {
            auto input_inst = phi->GetDataFlowInput(idx);
            if (input_inst->GetOpcode() == Opcode::CastValueToAnyType) {
                phi->SetInput(idx, input_inst->GetInput(0).GetInst());
            } else {
                ASSERT(std::find(phis_.begin(), phis_.end(), phi) != phis_.end());
                // case:
                // 2.any Phi v1(bb1), v3(bb3) -> v3
                // 3.any AnyTypeCheck v2 - > v2
                if (phi->GetInput(idx).GetInst() != input_inst) {
                    ASSERT(phi->GetInput(idx).GetInst()->GetOpcode() == Opcode::AnyTypeCheck);
                    phi->SetInput(idx, input_inst);
                }
            }
        }
        auto *cast_to_any_inst = GetGraph()->CreateInstCastValueToAnyType(DataType::ANY, phi->GetPc());
        cast_to_any_inst->SetAnyType(any_type_);
        phi->GetBasicBlock()->PrependInst(cast_to_any_inst);
        for (auto it = phi->GetUsers().begin(); it != phi->GetUsers().end();) {
            auto user_inst = it->GetInst();
            if (user_inst->IsPhi() && user_inst->GetType() != DataType::ANY) {
                ++it;
                continue;
            }
            user_inst->SetInput(it->GetIndex(), cast_to_any_inst);
            it = phi->GetUsers().begin();
        }
        cast_to_any_inst->SetInput(0, phi);
    }
}

bool TypesResolving::TryResolvePhi()
{
    bool is_applied = false;
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        // We use reverse iter because new instrucion is inserted after last phi and forward iteratoris brokken.
        for (auto inst : bb->PhiInstsSafeReverse()) {
            if (inst->GetType() != DataType::ANY) {
                continue;
            }
            phis_.clear();
            any_type_ = AnyBaseType::UNDEFINED_TYPE;
            if (!CheckInputsAnyTypesRec(inst)) {
                continue;
            }
            PropagateTypeToPhi();
            ASSERT(inst->GetType() != DataType::ANY);
            is_applied = true;
        }
    }
    return is_applied;
}

bool TypesResolving::TryInline(IntrinsicInst *intrinsic)
{
    types_.clear();
    AnyBaseType type = AnyBaseType::UNDEFINED_TYPE;
    for (auto &input : intrinsic->GetInputs()) {
        auto input_inst = input.GetInst();
        if (input_inst->IsSaveState()) {
            continue;
        }
        auto input_type = GetAssumedAnyType(input_inst);
        if (input_type != AnyBaseType::UNDEFINED_TYPE) {
            type = input_type;
        }
        types_.emplace_back(input_type);
    }
    // last input is SaveSatte
    ASSERT(types_.size() + 1 == intrinsic->GetInputsCount());
    // All intrinsics inputs don't have type information.
    if (type == AnyBaseType::UNDEFINED_TYPE) {
        return false;
    }
    // Set known type to undefined input types.
    for (auto &curr_type : types_) {
        if (curr_type == AnyBaseType::UNDEFINED_TYPE) {
            curr_type = type;
        }
    }
    if (DoInline(intrinsic)) {
        size_t i = 0;
        for (auto &input : intrinsic->GetInputs()) {
            auto input_inst = input.GetInst();
            if (input_inst->IsSaveState()) {
                continue;
            }
            if (input_inst->GetOpcode() == Opcode::AnyTypeCheck) {
                input_inst->CastToAnyTypeCheck()->SetAnyType(types_[i]);
            }
            ++i;
        }
        return true;
    }
    return false;
}
}  // namespace panda::compiler
