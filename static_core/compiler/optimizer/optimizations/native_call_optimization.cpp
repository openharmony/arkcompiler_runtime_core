/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "optimizer/optimizations/native_call_optimization.h"

namespace ark::compiler {

void NativeCallOptimization::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool NativeCallOptimization::RunImpl()
{
    if (!GetGraph()->CanOptimizeNativeMethods()) {
        COMPILER_LOG(DEBUG, NATIVE_CALL_OPT)
            << "Graph " << GetGraph()->GetRuntime()->GetMethodFullName(GetGraph()->GetMethod(), true)
            << " cannot optimize native methods, skip";
        return false;
    }

    VisitGraph();
    return IsApplied();
}

void NativeCallOptimization::VisitCallStatic(GraphVisitor *v, Inst *inst)
{
    OptimizeCallStatic(v, inst->CastToCallStatic());
}

void NativeCallOptimization::VisitCallResolvedStatic(GraphVisitor *v, Inst *inst)
{
    OptimizeCallStatic(v, inst->CastToCallResolvedStatic());
}

void NativeCallOptimization::OptimizeCallStatic(GraphVisitor *v, CallInst *callInst)
{
    if (!callInst->GetIsNative()) {
        COMPILER_LOG(DEBUG, NATIVE_CALL_OPT) << "CallStatic with id=" << callInst->GetId() << " is not native, skip";
        // NOTE: add event here!
        return;
    }

    if (callInst->GetCanNativeException()) {
        // NOTE: this case will be optimized in the next patch
        COMPILER_LOG(DEBUG, NATIVE_CALL_OPT)
            << "CallStatic with id=" << callInst->GetId() << " is native, but has exception, skip as not implemented";
        return;
    }

    if (callInst->GetOpcode() == Opcode::CallStatic) {
        callInst->SetOpcode(Opcode::CallNative);
    } else {
        callInst->SetOpcode(Opcode::CallResolvedNative);
    }

    callInst->ClearFlag(inst_flags::Flags::CAN_THROW);
    callInst->ClearFlag(inst_flags::Flags::HEAP_INV);
    callInst->ClearFlag(inst_flags::Flags::REQUIRE_STATE);
    callInst->ClearFlag(inst_flags::Flags::RUNTIME_CALL);

    // remove save state from managed call
    callInst->RemoveInput(callInst->GetInputsCount() - 1U);

    static_cast<NativeCallOptimization *>(v)->SetIsApplied();
    COMPILER_LOG(DEBUG, NATIVE_CALL_OPT) << "CallStatic with id=" << callInst->GetId()
                                         << " is native and was replaced with CallNative";
    // NOTE: add event here!
}

}  // namespace ark::compiler
