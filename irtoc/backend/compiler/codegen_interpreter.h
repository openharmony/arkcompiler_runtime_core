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

#ifndef PANDA_CODEGEN_INTERPRETER_H
#define PANDA_CODEGEN_INTERPRETER_H

#include "compiler/optimizer/code_generator/codegen_native.h"

namespace panda::compiler {

/**
 * Code generation for functions that should be called inside interpreter
 * Prologue and epilogue of such functions are empty, all arguments are placed in registers manually
 */
class CodegenInterpreter : public CodegenNative {
public:
    using CodegenNative::CodegenNative;

    /**
     *  fix number of spill slots such that RETURN handlers are able to generate epilogue correspondent to
     *  InterpreterEntry prologue independently from number of spill slots in handler itself.
     *  Should be synced with spills_count_max in interpreter.irt
     */
    static constexpr size_t SPILL_SLOTS = 12;

    void CreateFrameInfo() override
    {
        auto &fl = GetFrameLayout();
        auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
            FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(true) |
            FrameInfo::CallersRelativeFp::Encode(false) | FrameInfo::CalleesRelativeFp::Encode(true));
        frame->SetFrameSize(fl.GetFrameSize<CFrameLayout::BYTES>());
        frame->SetSpillsCount(fl.GetSpillsCount());

        frame->SetCallersOffset(
            fl.GetOffset<CFrameLayout::SP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
        frame->SetFpCallersOffset(
            fl.GetOffset<CFrameLayout::SP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
        frame->SetCalleesOffset(
            -fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
        frame->SetFpCalleesOffset(
            -fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

        frame->SetSetupFrame(false);
        frame->SetSaveFrameAndLinkRegs(true);
        frame->SetSaveUnusedCalleeRegs(true);
        frame->SetAdjustSpReg(true);

        SetFrameInfo(frame);
    }

    void GeneratePrologue() override
    {
        GetEncoder()->SetFrameLayout(panda::CFrameLayout(GetGraph()->GetArch(), SPILL_SLOTS));
        if (GetGraph()->GetMode().IsInterpreterEntry()) {
            ScopedDisasmPrinter printer(this, "Entrypoint Prologue");
            GetCallingConvention()->GenerateNativePrologue(*GetFrameInfo());
        }
    }

    void GenerateEpilogue() override {}
};

}  // namespace panda::compiler

#endif  // PANDA_CODEGEN_INTERPRETER_H
