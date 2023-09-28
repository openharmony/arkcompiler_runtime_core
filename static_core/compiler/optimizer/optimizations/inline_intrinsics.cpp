/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "compiler_options.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/types_analysis.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/inline_intrinsics.h"
#ifndef __clang_analyzer__
#include "irtoc_ir_inline.h"
#endif

namespace panda::compiler {
InlineIntrinsics::InlineIntrinsics(Graph *graph)
    : Optimization(graph),
      types_ {graph->GetLocalAllocator()->Adapter()},
      saved_inputs_ {graph->GetLocalAllocator()->Adapter()},
      named_access_profile_ {graph->GetLocalAllocator()->Adapter()}
{
}

void InlineIntrinsics::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

bool InlineIntrinsics::RunImpl()
{
    bool is_applied = false;
    GetGraph()->RunPass<TypesAnalysis>();
    bool clear_bbs = false;
    // We can't replace intrinsics on Deoptimize in OSR mode, because we can get incorrect value
    bool is_replace_on_deopt = !GetGraph()->IsAotMode() && !GetGraph()->IsOsrMode();
    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (bb == nullptr || bb->IsEmpty()) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            if (GetGraph()->GetVectorBlocks()[inst->GetBasicBlock()->GetId()] == nullptr) {
                break;
            }
            if (inst->GetOpcode() == Opcode::CallDynamic) {
                is_applied |= TryInline(inst->CastToCallDynamic());
                continue;
            }
            if (inst->GetOpcode() != Opcode::Intrinsic) {
                continue;
            }
            auto intrinsics_inst = inst->CastToIntrinsic();
            if (OPTIONS.IsCompilerInlineFullIntrinsics() && !intrinsics_inst->CanBeInlined()) {
                // skip intrinsics built inside inlined irtoc handler
                continue;
            }
            if (TryInline(intrinsics_inst)) {
                is_applied = true;
                continue;
            }
            if (is_replace_on_deopt && intrinsics_inst->IsReplaceOnDeoptimize()) {
                inst->GetBasicBlock()->ReplaceInstByDeoptimize(inst);
                clear_bbs = true;
                break;
            }
        }
    }
    if (clear_bbs) {
        GetGraph()->RemoveUnreachableBlocks();
        if (GetGraph()->IsOsrMode()) {
            CleanupGraphSaveStateOSR(GetGraph());
        }
        is_applied = true;
    }
    return is_applied;
}

AnyBaseType InlineIntrinsics::GetAssumedAnyType(const Inst *inst)
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
        case Opcode::Constant: {
            if (inst->GetType() != DataType::ANY) {
                // TODO(dkofanov): Probably, we should resolve const's type based on `inst->GetType()`:
                return AnyBaseType::UNDEFINED_TYPE;
            }
            coretypes::TaggedValue any_const(inst->CastToConstant()->GetRawValue());
            auto type = GetGraph()->GetRuntime()->ResolveSpecialAnyTypeByConstant(any_const);
            ASSERT(type != AnyBaseType::UNDEFINED_TYPE);
            return type;
        }
        case Opcode::LoadFromConstantPool: {
            if (inst->CastToLoadFromConstantPool()->IsString()) {
                auto language = GetGraph()->GetRuntime()->GetMethodSourceLanguage(GetGraph()->GetMethod());
                return GetAnyStringType(language);
            }
            return AnyBaseType::UNDEFINED_TYPE;
        }
        default:
            return AnyBaseType::UNDEFINED_TYPE;
    }
}

bool InlineIntrinsics::DoInline(IntrinsicInst *intrinsic)
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

bool InlineIntrinsics::TryInline(CallInst *call_inst)
{
    if ((call_inst->GetCallMethod() == nullptr)) {
        return false;
    }
    switch (GetGraph()->GetRuntime()->GetMethodSourceLanguage(call_inst->GetCallMethod())) {
#include "intrinsics_inline_native_method.inl"
        default: {
            return false;
        }
    }
}

bool InlineIntrinsics::TryInline(IntrinsicInst *intrinsic)
{
    if (!IsIntrinsicInlinedByInputTypes(intrinsic->GetIntrinsicId())) {
        return DoInline(intrinsic);
    }
    types_.clear();
    saved_inputs_.clear();
    AnyBaseType type = AnyBaseType::UNDEFINED_TYPE;
    for (auto &input : intrinsic->GetInputs()) {
        auto input_inst = input.GetInst();
        if (input_inst->IsSaveState()) {
            continue;
        }
        auto input_type = GetAssumedAnyType(input_inst);
        if (input_type != AnyBaseType::UNDEFINED_TYPE) {
            type = input_type;
        } else if (input_inst->GetOpcode() == Opcode::AnyTypeCheck &&
                   input_inst->CastToAnyTypeCheck()->IsTypeWasProfiled()) {
            // Type is mixed and compiler cannot make optimization for that case.
            // Any deduced type will also cause deoptimization. So avoid intrinsic inline here.
            // Revise it in the future optimizations.
            return false;
        }
        types_.emplace_back(input_type);
        saved_inputs_.emplace_back(input_inst);
    }
    // last input is SaveState
    ASSERT(types_.size() + 1 == intrinsic->GetInputsCount());

    if (!GetGraph()->GetRuntime()->IsDestroyed(GetGraph()->GetMethod()) && type != AnyBaseType::UNDEFINED_TYPE &&
        AnyBaseTypeToDataType(type) != DataType::ANY) {
        // Set known type to undefined input types.
        // Do not set type based on special input types like Undefined or Null
        for (auto &curr_type : types_) {
            if (curr_type == AnyBaseType::UNDEFINED_TYPE) {
                curr_type = type;
            }
        }
    }
    if (DoInline(intrinsic)) {
        for (size_t i = 0; i < saved_inputs_.size(); i++) {
            ASSERT(!saved_inputs_[i]->IsSaveState());
            if (saved_inputs_[i]->GetOpcode() == Opcode::AnyTypeCheck) {
                saved_inputs_[i]->CastToAnyTypeCheck()->SetAnyType(types_[i]);
            }
        }
        return true;
    }
    return false;
}

}  // namespace panda::compiler
