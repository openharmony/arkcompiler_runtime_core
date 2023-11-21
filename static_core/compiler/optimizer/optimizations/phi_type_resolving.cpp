/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/phi_type_resolving.h"

namespace panda::compiler {
PhiTypeResolving::PhiTypeResolving(Graph *graph) : Optimization(graph), phis_ {graph->GetLocalAllocator()->Adapter()} {}

void PhiTypeResolving::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool PhiTypeResolving::RunImpl()
{
    bool is_applied = false;
    GetGraph()->RunPass<DominatorsTree>();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        // We use reverse iter because new instrucion is inserted after last phi and forward iteratoris broken.
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

bool PhiTypeResolving::CheckInputsAnyTypesRec(Inst *phi)
{
    ASSERT(phi->IsPhi());
    if (std::find(phis_.begin(), phis_.end(), phi) != phis_.end()) {
        return true;
    }

    if (GetGraph()->IsOsrMode()) {
        for (auto &user : phi->GetUsers()) {
            if (user.GetInst()->GetOpcode() == Opcode::SaveStateOsr) {
                // Find way to enable it in OSR mode.
                return false;
            }
        }
    }

    phis_.push_back(phi);
    int32_t input_num = -1;
    for (auto &input : phi->GetInputs()) {
        ++input_num;
        auto input_inst = phi->GetDataFlowInput(input.GetInst());
        if (GetGraph()->IsOsrMode()) {
            if (HasOsrEntryBetween(input_inst->GetBasicBlock(), phi->CastToPhi()->GetPhiInputBb(input_num))) {
                return false;
            }
        }
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
    return AnyBaseTypeToDataType(any_type_) != DataType::ANY;
}

void PhiTypeResolving::PropagateTypeToPhi()
{
    auto new_type = AnyBaseTypeToDataType(any_type_);
    for (auto phi : phis_) {
        phi->SetType(new_type);
        size_t inputs_count = phi->GetInputsCount();
        for (size_t idx = 0; idx < inputs_count; ++idx) {
            auto input_inst = phi->GetDataFlowInput(idx);
            if (input_inst->GetOpcode() == Opcode::CastValueToAnyType) {
                phi->SetInput(idx, input_inst->GetInput(0).GetInst());
            } else if (phi->GetInput(idx).GetInst() != input_inst) {
                ASSERT(std::find(phis_.begin(), phis_.end(), phi) != phis_.end());
                // case:
                // 2.any Phi v1(bb1), v3(bb3) -> v3
                // 3.any AnyTypeCheck v2 - > v2
                ASSERT(phi->GetInput(idx).GetInst()->GetOpcode() == Opcode::AnyTypeCheck);
                phi->SetInput(idx, input_inst);
            } else {
                ASSERT(std::find(phis_.begin(), phis_.end(), phi) != phis_.end());
            }
        }
        auto *cast_to_any_inst = GetGraph()->CreateInstCastValueToAnyType(phi->GetPc(), any_type_, nullptr);
        auto *bb = phi->GetBasicBlock();
        auto *first = bb->GetFirstInst();
        if (first == nullptr) {
            bb->AppendInst(cast_to_any_inst);
        } else if (first->IsSaveState()) {
            bb->InsertAfter(cast_to_any_inst, first);
        } else {
            bb->InsertBefore(cast_to_any_inst, first);
        }

        for (auto it = phi->GetUsers().begin(); it != phi->GetUsers().end();) {
            auto user_inst = it->GetInst();
            if ((user_inst->IsPhi() && user_inst->GetType() != DataType::ANY) ||
                (user_inst == first && user_inst->IsSaveState())) {
                ++it;
                continue;
            }
            user_inst->SetInput(it->GetIndex(), cast_to_any_inst);
            it = phi->GetUsers().begin();
        }
        cast_to_any_inst->SetInput(0, phi);
    }
}
}  // namespace panda::compiler
