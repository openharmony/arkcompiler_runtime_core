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

#include "codegen_native.h"
#include "optimizer/code_generator/method_properties.h"

namespace panda::compiler {

void CodegenNative::CreateFrameInfo()
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

    ASSERT(!GetGraph()->GetMethodProperties().GetRequireFrameSetup());
    // we don't need to setup frame in native mode
    frame->SetSetupFrame(false);
    // we don't need to save FP and LR registers only for leaf methods
    frame->SetSaveFrameAndLinkRegs(!GetGraph()->GetMethodProperties().IsLeaf());
    // we never need to save unused registers in native mode
    frame->SetSaveUnusedCalleeRegs(false);
    // we have to sub/add SP in prologue/epilogue in the following cases:
    // - non-leaf method
    // - leaf method and there are spills or parameters on stack
    frame->SetAdjustSpReg(!GetGraph()->GetMethodProperties().IsLeaf() || GetGraph()->GetStackSlotsCount() != 0 ||
                          GetGraph()->GetMethodProperties().GetHasParamsOnStack());
    SetFrameInfo(frame);
}

void CodegenNative::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "Method Prologue");

    GetCallingConvention()->GenerateNativePrologue(*GetFrameInfo());

    if (GetGraph()->IsDynamicMethod()) {
        GenerateExtensionsForPrologue();
    }

#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
    if (GetGraph()->IsAotMode()) {
        SCOPED_DISASM_STR(this, "LoadMethod for trace");
        ScopedTmpReg method_reg(GetEncoder());
        LoadMethod(method_reg);
        InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)), method_reg,
                     Imm(static_cast<size_t>(events::MethodEnterKind::COMPILED))});
    } else {
        InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                     Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())),
                     Imm(static_cast<size_t>(events::MethodEnterKind::COMPILED))});
    }
#endif
}

void CodegenNative::GenerateEpilogue()
{
    ASSERT(GetGraph()->GetMethodProperties().IsLeaf());
    SCOPED_DISASM_STR(this, "Method Epilogue");

    if (GetGraph()->IsDynamicMethod()) {
        GenerateExtensionsForEpilogue();
    }

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
    GetCallingConvention()->GenerateNativeEpilogue(*GetFrameInfo(), [this]() {
        if (GetGraph()->IsAotMode()) {
            ScopedTmpReg method_reg(GetEncoder());
            LoadMethod(method_reg);
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_EXIT)), method_reg,
                         Imm(static_cast<size_t>(events::MethodExitKind::COMPILED))});
        } else {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_EXIT)),
                         Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())),
                         Imm(static_cast<size_t>(events::MethodExitKind::COMPILED))});
        }
    });
#else
    GetCallingConvention()->GenerateNativeEpilogue(*GetFrameInfo(), []() {});
#endif
}
}  // namespace panda::compiler
