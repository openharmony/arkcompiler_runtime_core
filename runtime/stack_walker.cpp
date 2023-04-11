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

#include "compiler/code_info/code_info.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/panda_vm.h"
#include "libpandabase/mem/mem.h"
#include "runtime/interpreter/runtime_interface.h"

#include <iomanip>

namespace panda {

StackWalker StackWalker::Create(const ManagedThread *thread, UnwindPolicy policy)
{
#ifndef NDEBUG
    ASSERT(thread->IsRuntimeCallEnabled());
    if (Runtime::GetOptions().IsVerifyCallStack()) {
        StackWalker(thread->GetCurrentFrame(), thread->IsCurrentFrameCompiled(),
                    thread->GetNativePc(), policy).Verify();
    }
#endif
    return StackWalker(thread->GetCurrentFrame(), thread->IsCurrentFrameCompiled(), thread->GetNativePc(), policy);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
StackWalker::StackWalker(void *fp, bool is_frame_compiled, uintptr_t npc, UnwindPolicy policy)
{
    frame_ = GetTopFrameFromFp(fp, is_frame_compiled, npc);
    if (policy == UnwindPolicy::SKIP_INLINED) {
        inline_depth_ = -1;
    }
}

void StackWalker::Reset(const ManagedThread *thread)
{
    frame_ = GetTopFrameFromFp(thread->GetCurrentFrame(), thread->IsCurrentFrameCompiled(), thread->GetNativePc());
}

/* static */
typename StackWalker::FrameVariant StackWalker::GetTopFrameFromFp(void *ptr, bool is_frame_compiled, uintptr_t npc)
{
    if (is_frame_compiled) {
        if (IsBoundaryFrame<FrameKind::INTERPRETER>(ptr)) {
            auto bp = GetPrevFromBoundary<FrameKind::INTERPRETER>(ptr);
            if (GetBoundaryFrameMethod<FrameKind::COMPILER>(bp) == BYPASS) {
                return CreateCFrame(GetPrevFromBoundary<FrameKind::COMPILER>(bp),
                                    GetReturnAddressFromBoundary<FrameKind::COMPILER>(bp),
                                    GetCalleeStackFromBoundary<FrameKind::COMPILER>(bp));
            }
            return CreateCFrame(GetPrevFromBoundary<FrameKind::INTERPRETER>(ptr),
                                GetReturnAddressFromBoundary<FrameKind::INTERPRETER>(ptr),
                                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                reinterpret_cast<SlotType *>(ptr) +
                                    BoundaryFrame<FrameKind::INTERPRETER>::CALLEES_OFFSET);  // NOLINT
        }
        return CreateCFrame(reinterpret_cast<SlotType *>(ptr), npc, nullptr);
    }
    return reinterpret_cast<Frame *>(ptr);
}

Method *StackWalker::GetMethod()
{
    ASSERT(HasFrame());
    if (!IsCFrame()) {
        return GetIFrame()->GetMethod();
    }
    auto &cframe = GetCFrame();
    if (!cframe.IsNative()) {
        ASSERT(stackmap_.IsValid());
        if (IsInlined()) {
            auto method_variant = code_info_.GetMethod(stackmap_, inline_depth_);
            if (std::holds_alternative<uint32_t>(method_variant)) {
                return Runtime::GetCurrent()->GetClassLinker()->GetMethod(
                    *cframe.GetMethod(), panda_file::File::EntityId(std::get<uint32_t>(method_variant)));
            }
            return reinterpret_cast<Method *>(std::get<void *>(method_variant));
        }
    }
    return cframe.GetMethod();
}

template <bool create>
StackWalker::CFrameType StackWalker::CreateCFrameForC2IBridge(Frame *frame)
{
    auto prev = GetPrevFromBoundary<FrameKind::INTERPRETER>(frame);
    ASSERT(GetBoundaryFrameMethod<FrameKind::COMPILER>(prev) != FrameBridgeKind::BYPASS);
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (create) {
        return CreateCFrame(reinterpret_cast<SlotType *>(prev),
                            GetReturnAddressFromBoundary<FrameKind::INTERPRETER>(frame),
                            GetCalleeStackFromBoundary<FrameKind::INTERPRETER>(frame));
    }
    return CFrameType(prev);
}

StackWalker::CFrameType StackWalker::CreateCFrame(SlotType *ptr, uintptr_t npc, SlotType *callee_slots,
                                                  CalleeStorage *prev_callees)
{
    CFrameType cframe(ptr);
    if (cframe.IsNativeMethod()) {
        return cframe;
    }
    const void *code_entry;
    if (cframe.IsOsr()) {
        code_entry = Thread::GetCurrent()->GetVM()->GetCompiler()->GetOsrCode(cframe.GetMethod());
    } else if (cframe.ShouldDeoptimize()) {
        // When method was deoptimized due to speculation failure, regular code entry become invalid,
        // so we read entry from special backup field in the frame.
        code_entry = cframe.GetDeoptCodeEntry();
    } else {
        code_entry = cframe.GetMethod()->GetCompiledEntryPoint();
    }
    new (&code_info_) CodeInfo(CodeInfo::GetCodeOriginFromEntryPoint(code_entry));
    // StackOverflow stackmap has zero address
    if (npc == 0) {
        stackmap_ = code_info_.FindStackMapForNativePc(npc);
    } else {
        auto code = reinterpret_cast<uintptr_t>(code_info_.GetCode());
        CHECK_GT(npc, code);
        CHECK_LT(npc - code, std::numeric_limits<uint32_t>::max());
        stackmap_ = code_info_.FindStackMapForNativePc(npc - code);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT_PRINT(
        stackmap_.IsValid(), "Stackmap not found "
                                 << cframe.GetMethod()->GetFullName() << ": npc=0x" << std::hex << npc << ", code=["
                                 << reinterpret_cast<const void *>(code_info_.GetCode()) << ".."
                                 << reinterpret_cast<const void *>(code_info_.GetCode() + code_info_.GetCodeSize())
                                 << "]" << std::dec);
    callee_stack_.int_regs_mask = code_info_.GetHeader().GetCalleeRegMask();
    callee_stack_.fp_regs_mask = code_info_.GetHeader().GetCalleeFpRegMask();
    inline_depth_ = code_info_.GetInlineDepth(stackmap_);

    InitCalleeBuffer(callee_slots, prev_callees);

    return cframe;
}

/**
 * If all callees are saved then callee-saved regs are placed on the stack as follows:
 *
 * ---------------------  <-- callee_slots
 * LastCalleeReg   (x28)
 * ---------------------  <-- callee_slots - 1
 *                  ...
 * ---------------------
 * FirstCalleeReg  (x19)
 * ---------------------  <-- callee_slots - CalleeRegsCount()
 * LastCalleeFpReg (d15)
 * ---------------------
 *                  ...
 * ---------------------
 * FirstCalleeFpReg (d8)
 * ---------------------  <-- callee_slots - CalleeRegsCount() - CalleeFpRegsCount()
 *
 * If only used callees are saved, then the space is reserved for all callee registers,
 * but only umasked regs are saved and there are no gaps between them.
 *
 * Suppose that regs masks are as follows:
 *
 * int_regs_mask = 0x00980000 (1001 1000 0000 0000 0000 0000, i.e. x19, x20 and x23 must be saved)
 * fp_regs_mask  = 0x0,
 *
 * then we have the following layout:
 *
 * --------------------  <-- callee_slots
 *                (x23)
 * --------------------  <-- callee_slots - 1
 *                (x20)
 * --------------------  <-- callee_slots - 2
 *                (x19)
 * --------------------  <-- callee_slots - 3
 *                 ...
 * --------------------
 *                (---)
 * --------------------  <-- callee_slots - CalleeIntRegsCount()
 *                 ...
 * --------------------
 *                (---)
 * --------------------  <-- callee_slots - CalleeIntRegsCount() - CalleeFpRegsCount()
 */
void StackWalker::InitCalleeBuffer(SlotType *callee_slots, CalleeStorage *prev_callees)
{
    constexpr RegMask ArchIntRegsMask(panda::GetCalleeRegsMask(RUNTIME_ARCH, false));
    constexpr RegMask ArchFpRegsMask(panda::GetCalleeRegsMask(RUNTIME_ARCH, true));

    bool prev_is_native = IsCFrame() ? GetCFrame().IsNative() : false;
    if (callee_slots != nullptr || prev_callees != nullptr) {
        // Process scalar integer callee registers
        for (size_t reg = FirstCalleeIntReg(); reg <= LastCalleeIntReg(); reg++) {
            size_t offset = reg - FirstCalleeIntReg();
            if (prev_callees == nullptr || prev_is_native) {
                size_t slot = ArchIntRegsMask.GetDistanceFromHead(reg);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                callee_stack_.stack[offset] = callee_slots - slot - 1;
            } else if (prev_callees->int_regs_mask.Test(reg)) {
                size_t slot = prev_callees->int_regs_mask.GetDistanceFromHead(reg);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                callee_stack_.stack[offset] = callee_slots - slot - 1;
            } else {
                ASSERT(prev_callees->stack[offset] != nullptr);
                callee_stack_.stack[offset] = prev_callees->stack[offset];
            }
        }
        // Process SIMD and Floating-Point callee registers
        for (size_t reg = FirstCalleeFpReg(); reg <= LastCalleeFpReg(); reg++) {
            size_t offset = CalleeIntRegsCount() + reg - FirstCalleeFpReg();
            if (prev_callees == nullptr || prev_is_native) {
                size_t slot = ArchFpRegsMask.GetDistanceFromHead(reg);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                callee_stack_.stack[offset] = callee_slots - CalleeIntRegsCount() - slot - 1;
            } else if (prev_callees->fp_regs_mask.Test(reg)) {
                size_t slot = prev_callees->fp_regs_mask.GetDistanceFromHead(reg);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                callee_stack_.stack[offset] = callee_slots - CalleeIntRegsCount() - slot - 1;
            } else {
                ASSERT(prev_callees->stack[offset] != nullptr);
                callee_stack_.stack[offset] = prev_callees->stack[offset];
            }
        }
    }
}

StackWalker::CalleeRegsBuffer &StackWalker::GetCalleeRegsForDeoptimize()
{
    // Process scalar integer callee registers
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    SlotType *callee_src_slots = GetCFrame().GetCalleeSaveStack() - 1;
    SlotType *callee_dst_slots = &deopt_callee_regs_[CalleeFpRegsCount()];
    for (size_t reg = FirstCalleeIntReg(); reg <= LastCalleeIntReg(); reg++) {
        size_t offset = reg - FirstCalleeIntReg();
        if (callee_stack_.int_regs_mask.Test(reg)) {
            size_t slot = callee_stack_.int_regs_mask.GetDistanceFromHead(reg);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            callee_dst_slots[offset] = *(callee_src_slots - slot);
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            callee_dst_slots[offset] = *callee_stack_.stack[offset];
        }
    }
    // Process SIMD and Floating-Point callee registers
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    callee_src_slots = GetCFrame().GetCalleeSaveStack() - CalleeIntRegsCount() - 1;
    callee_dst_slots = deopt_callee_regs_.begin();
    for (size_t reg = FirstCalleeFpReg(); reg <= LastCalleeFpReg(); reg++) {
        size_t offset = reg - FirstCalleeFpReg();
        if (callee_stack_.fp_regs_mask.Test(reg)) {
            size_t slot = callee_stack_.fp_regs_mask.GetDistanceFromHead(reg);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            callee_dst_slots[offset] = *(callee_src_slots - slot);
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            callee_dst_slots[offset] = *callee_stack_.stack[CalleeIntRegsCount() + offset];
        }
    }

    return deopt_callee_regs_;
}

interpreter::VRegister StackWalker::GetVRegValue(size_t vreg_num)
{
    if (IsCFrame()) {
        // TODO(msherstennikov): we need to cache vregs_list within single cframe
        auto vregs_list = code_info_.GetVRegList(stackmap_, inline_depth_,
                                                 mem::InternalAllocator<>::GetInternalAllocatorFromRuntime());
        ASSERT(vregs_list[vreg_num].GetIndex() == vreg_num);
        interpreter::VRegister vreg0;
        [[maybe_unused]] interpreter::VRegister vreg1;
        GetCFrame().GetVRegValue(vregs_list[vreg_num], code_info_, callee_stack_.stack.data(),
                                 interpreter::StaticVRegisterRef(&vreg0, &vreg1));
        return vreg0;
    }
    ASSERT(vreg_num < GetIFrame()->GetSize());
    return GetIFrame()->GetVReg(vreg_num);
}

template <bool is_dynamic, typename T>
void StackWalker::SetVRegValue(VRegInfo reg_info, T value)
{
    if (IsCFrame()) {
        auto &cframe = GetCFrame();
        if (IsDynamicMethod()) {
            if constexpr (sizeof(T) == sizeof(uint64_t)) {  // NOLINT
                cframe.SetVRegValue<true>(reg_info, bit_cast<uint64_t>(value), callee_stack_.stack.data());
            } else {  // NOLINT
                static_assert(sizeof(T) == sizeof(uint32_t));
                cframe.SetVRegValue<true>(reg_info, static_cast<uint64_t>(bit_cast<uint32_t>(value)),
                                          callee_stack_.stack.data());
            }
        } else {
            if constexpr (sizeof(T) == sizeof(uint64_t)) {  // NOLINT
                cframe.SetVRegValue(reg_info, bit_cast<uint64_t>(value), callee_stack_.stack.data());
            } else {  // NOLINT
                static_assert(sizeof(T) == sizeof(uint32_t));
                cframe.SetVRegValue(reg_info, static_cast<uint64_t>(bit_cast<uint32_t>(value)),
                                    callee_stack_.stack.data());
            }
        }
    } else {
        auto vreg = GetFrameHandler<is_dynamic>(GetIFrame()).GetVReg(reg_info.GetIndex());
        if constexpr (std::is_same_v<T, ObjectHeader *>) {  // NOLINT
            ASSERT(vreg.HasObject() && "Trying to change object variable by scalar value");
            vreg.SetReference(value);
        } else {  // NOLINT
            ASSERT(!vreg.HasObject() && "Trying to change object variable by scalar value");
            vreg.Set(value);
        }
    }
}

template void StackWalker::SetVRegValue(VRegInfo reg_info, uint32_t value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, int32_t value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, uint64_t value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, int64_t value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, float value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, double value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, ObjectHeader *value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, uint32_t value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, int32_t value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, uint64_t value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, int64_t value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, float value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, double value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, ObjectHeader *value);

void StackWalker::NextFrame()
{
    if (IsCFrame()) {
        NextFromCFrame();
    } else {
        NextFromIFrame();
    }
}

void StackWalker::NextFromCFrame()
{
    if (IsInlined()) {
        if (policy_ != UnwindPolicy::SKIP_INLINED) {
            inline_depth_--;
            return;
        }
        inline_depth_ = -1;
    }
    if (policy_ == UnwindPolicy::ONLY_INLINED) {
        frame_ = nullptr;
        return;
    }
    auto prev = GetCFrame().GetPrevFrame();
    if (prev == nullptr) {
        frame_ = nullptr;
        return;
    }
    auto frame_method = GetBoundaryFrameMethod<FrameKind::COMPILER>(prev);
    switch (frame_method) {
        case FrameBridgeKind::INTERPRETER_TO_COMPILED_CODE: {
            auto prev_frame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
            if (prev_frame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prev_frame)) {
                frame_ = CreateCFrameForC2IBridge<true>(prev_frame);
                break;
            }

            frame_ = reinterpret_cast<Frame *>(prev_frame);
            break;
        }
        case FrameBridgeKind::BYPASS: {
            auto prev_frame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
            if (prev_frame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prev_frame)) {
                frame_ = CreateCFrameForC2IBridge<true>(prev_frame);
                break;
            }
            frame_ = CreateCFrame(reinterpret_cast<SlotType *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev)),
                                  GetReturnAddressFromBoundary<FrameKind::COMPILER>(prev),
                                  GetCalleeStackFromBoundary<FrameKind::COMPILER>(prev));
            break;
        }
        default:
            prev_callee_stack_ = callee_stack_;
            frame_ = CreateCFrame(reinterpret_cast<SlotType *>(prev), GetCFrame().GetLr(),
                                  GetCFrame().GetCalleeSaveStack(), &prev_callee_stack_);
            break;
    }
}

void StackWalker::NextFromIFrame()
{
    if (policy_ == UnwindPolicy::ONLY_INLINED) {
        frame_ = nullptr;
        return;
    }
    auto prev = GetIFrame()->GetPrevFrame();
    if (prev == nullptr) {
        frame_ = nullptr;
        return;
    }
    if (IsBoundaryFrame<FrameKind::INTERPRETER>(prev)) {
        auto bp = GetPrevFromBoundary<FrameKind::INTERPRETER>(prev);
        if (GetBoundaryFrameMethod<FrameKind::COMPILER>(bp) == BYPASS) {
            frame_ = CreateCFrame(GetPrevFromBoundary<FrameKind::COMPILER>(bp),
                                  GetReturnAddressFromBoundary<FrameKind::COMPILER>(bp),
                                  GetCalleeStackFromBoundary<FrameKind::COMPILER>(bp));
        } else {
            frame_ = CreateCFrameForC2IBridge<true>(prev);
        }
    } else {
        frame_ = reinterpret_cast<Frame *>(prev);
    }
}

FrameAccessor StackWalker::GetNextFrame()
{
    if (IsCFrame()) {
        if (IsInlined()) {
            return FrameAccessor(frame_);
        }
        auto prev = GetCFrame().GetPrevFrame();
        if (prev == nullptr) {
            return FrameAccessor(nullptr);
        }
        auto frame_method = GetBoundaryFrameMethod<FrameKind::COMPILER>(prev);
        switch (frame_method) {
            case FrameBridgeKind::INTERPRETER_TO_COMPILED_CODE: {
                auto prev_frame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
                if (prev_frame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prev_frame)) {
                    return FrameAccessor(CreateCFrameForC2IBridge<false>(prev_frame));
                }
                return FrameAccessor(prev_frame);
            }
            case FrameBridgeKind::BYPASS: {
                auto prev_frame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
                if (prev_frame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prev_frame)) {
                    return FrameAccessor(CreateCFrameForC2IBridge<false>(prev_frame));
                }
                return FrameAccessor(
                    CFrameType(reinterpret_cast<SlotType *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev))));
            }
            default:
                return FrameAccessor(CFrameType(reinterpret_cast<SlotType *>(prev)));
        }
    } else {
        auto prev = GetIFrame()->GetPrevFrame();
        if (prev == nullptr) {
            return FrameAccessor(nullptr);
        }
        if (IsBoundaryFrame<FrameKind::INTERPRETER>(prev)) {
            auto bp = GetPrevFromBoundary<FrameKind::INTERPRETER>(prev);
            if (GetBoundaryFrameMethod<FrameKind::COMPILER>(bp) == BYPASS) {
                return FrameAccessor(CreateCFrame(GetPrevFromBoundary<FrameKind::COMPILER>(bp),
                                                  GetReturnAddressFromBoundary<FrameKind::COMPILER>(bp),
                                                  GetCalleeStackFromBoundary<FrameKind::COMPILER>(bp)));
            }
            return FrameAccessor(CreateCFrameForC2IBridge<false>(prev));
        }
        return FrameAccessor(reinterpret_cast<Frame *>(prev));
    }
}

FrameKind StackWalker::GetPreviousFrameKind() const
{
    if (IsCFrame()) {
        auto prev = GetCFrame().GetPrevFrame();
        if (prev == nullptr) {
            return FrameKind::NONE;
        }
        if (IsBoundaryFrame<FrameKind::COMPILER>(prev)) {
            return FrameKind::INTERPRETER;
        }
        return FrameKind::COMPILER;
    }
    auto prev = GetIFrame()->GetPrevFrame();
    if (prev == nullptr) {
        return FrameKind::NONE;
    }
    if (IsBoundaryFrame<FrameKind::INTERPRETER>(prev)) {
        return FrameKind::COMPILER;
    }
    return FrameKind::INTERPRETER;
}

bool StackWalker::IsCompilerBoundFrame(SlotType *prev)
{
    if (IsBoundaryFrame<FrameKind::COMPILER>(prev)) {
        return true;
    }
    if (GetBoundaryFrameMethod<FrameKind::COMPILER>(prev) == FrameBridgeKind::BYPASS) {
        auto prev_frame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
        // Case for clinit:
        // Compiled code -> C2I -> InitializeClass -> call clinit -> I2C -> compiled code for clinit
        if (prev_frame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prev_frame)) {
            return true;
        }
    }

    return false;
}

Frame *StackWalker::ConvertToIFrame(FrameKind *prev_frame_kind, uint32_t *num_inlined_methods)
{
    if (!IsCFrame()) {
        return GetIFrame();
    }
    auto &cframe = GetCFrame();

    auto inline_depth = inline_depth_;
    bool is_invoke = false;

    void *prev_frame;
    if (IsInlined()) {
        inline_depth_--;
        *num_inlined_methods = *num_inlined_methods + 1;
        prev_frame = ConvertToIFrame(prev_frame_kind, num_inlined_methods);
    } else {
        auto prev = cframe.GetPrevFrame();
        if (prev == nullptr) {
            *prev_frame_kind = FrameKind::NONE;
            prev_frame = nullptr;
        } else {
            if (IsCompilerBoundFrame(prev)) {
                is_invoke = true;
                prev_frame = reinterpret_cast<Frame *>(
                    StackWalker::GetPrevFromBoundary<FrameKind::COMPILER>(cframe.GetPrevFrame()));
                if (prev_frame_kind != nullptr) {
                    *prev_frame_kind = FrameKind::INTERPRETER;
                }
            } else {
                prev_frame = cframe.GetPrevFrame();
                if (prev_frame_kind != nullptr) {
                    *prev_frame_kind = FrameKind::COMPILER;
                }
            }
        }
    }
    inline_depth_ = inline_depth;
    auto vreg_list =
        code_info_.GetVRegList(stackmap_, inline_depth_, mem::InternalAllocator<>::GetInternalAllocatorFromRuntime());

    auto method = GetMethod();
    Frame *frame;

    if (IsDynamicMethod()) {
        /* If there is a usage of rest arguments in dynamic function, then a managed object to contain actual arguments
         * is constructed in prologue. Thus there is no need to reconstruct rest arguments here
         */
        auto num_actual_args = method->GetNumArgs();
        /* If there are no arguments-keeping object construction in execution path, the number of actual args may be
         * retreived from cframe
         */

        size_t frame_num_vregs = method->GetNumVregs() + num_actual_args;
        frame = interpreter::RuntimeInterface::CreateFrameWithActualArgs<true>(frame_num_vregs, num_actual_args, method,
                                                                               reinterpret_cast<Frame *>(prev_frame));
        frame->SetDynamic();
        DynamicFrameHandler frame_handler(frame);
        for (size_t i = 0; i < vreg_list.size(); i++) {
            auto vreg = vreg_list[i];
            if (!vreg.IsLive()) {
                continue;
            }

            bool is_acc = i == (vreg_list.size() - 1);
            auto reg_ref = is_acc ? frame->GetAccAsVReg<true>() : frame_handler.GetVReg(i);
            GetCFrame().GetPackVRegValue(vreg, code_info_, callee_stack_.stack.data(), reg_ref);
        }
    } else {
        auto frame_num_vregs = method->GetNumVregs() + method->GetNumArgs();
        ASSERT((frame_num_vregs + 1) >= vreg_list.size());
        frame =
            interpreter::RuntimeInterface::CreateFrame(frame_num_vregs, method, reinterpret_cast<Frame *>(prev_frame));
        StaticFrameHandler frame_handler(frame);
        for (size_t i = 0; i < vreg_list.size(); i++) {
            auto vreg = vreg_list[i];
            if (!vreg.IsLive()) {
                continue;
            }

            bool is_acc = i == (vreg_list.size() - 1);
            auto reg_ref = is_acc ? frame->GetAccAsVReg() : frame_handler.GetVReg(i);
            GetCFrame().GetVRegValue(vreg, code_info_, callee_stack_.stack.data(), reg_ref);
        }
    }

    frame->SetDeoptimized();
    frame->SetBytecodeOffset(GetBytecodePc());
    if (is_invoke) {
        frame->SetInvoke();
    }

    auto context = ManagedThread::GetCurrent()->GetVM()->GetLanguageContext();
    context.DeoptimizeBegin(frame, inline_depth);

    return frame;
}

bool StackWalker::IsDynamicMethod() const
{
    // Dynamic method may have no class
    return GetMethod()->GetClass() == nullptr ||
           panda::panda_file::IsDynamicLanguage(Runtime::GetCurrent()->GetLanguageContext(*GetMethod()).GetLanguage());
}

void StackWalker::Verify()
{
    for (; HasFrame(); NextFrame()) {
#ifndef NDEBUG
        ASSERT(GetMethod() != nullptr);
        IterateVRegsWithInfo([this]([[maybe_unused]] const auto &reg_info, const auto &vreg) {
            if (reg_info.GetType() == compiler::VRegInfo::Type::ANY) {
                ASSERT(IsDynamicMethod());
                return true;
            }

            if (vreg.HasObject()) {
                // Use Frame::VRegister::HasObject() to detect objects
                ASSERT(reg_info.IsObject());
                if (ObjectHeader *object = vreg.GetReference(); object != nullptr) {
                    auto cls = object->ClassAddr<Class>();
                    if (!IsInObjectsAddressSpace(reinterpret_cast<uintptr_t>(cls)) || cls == nullptr) {
                        StackWalker::Create(ManagedThread::GetCurrent()).Dump(std::cerr, true);
                        LOG(FATAL, INTEROP) << "Wrong class " << cls << " for object " << object << "\n";
                    } else {
                        cls->GetName();
                    }
                }
            } else {
                ASSERT(!reg_info.IsObject());
                vreg.GetLong();
            }
            return true;
        });

        if (IsCFrame()) {
            IterateObjects([this](const auto &vreg) {
                if (IsDynamicMethod()) {
                    ASSERT(vreg.HasObject());
                    return true;
                }

                ASSERT(vreg.HasObject());
                if (ObjectHeader *object = vreg.GetReference(); object != nullptr) {
                    ASSERT(IsInObjectsAddressSpace(reinterpret_cast<uintptr_t>(object)));
                    auto cls = object->ClassAddr<Class>();
                    if (!IsInObjectsAddressSpace(reinterpret_cast<uintptr_t>(cls)) || cls == nullptr) {
                        StackWalker::Create(ManagedThread::GetCurrent()).Dump(std::cerr, true);
                        LOG(FATAL, INTEROP) << "Wrong class " << cls << " for object " << object << "\n";
                    } else {
                        cls->GetName();
                    }
                }
                return true;
            });
        }
#endif  // ifndef NDEBUG
    }
}

// Dump function change StackWalker object-state, that's why it may be called only
// with rvalue reference.
void StackWalker::Dump(std::ostream &os, bool print_vregs /* = false */) &&
{
    [[maybe_unused]] static constexpr size_t WIDTH_INDEX = 4;
    [[maybe_unused]] static constexpr size_t WIDTH_REG = 4;
    [[maybe_unused]] static constexpr size_t WIDTH_FRAME = 8;
    [[maybe_unused]] static constexpr size_t WIDTH_LOCATION = 12;
    [[maybe_unused]] static constexpr size_t WIDTH_TYPE = 20;

    size_t frame_index = 0;
    os << "Panda call stack:\n";
    for (; HasFrame(); NextFrame()) {
        os << std::setw(WIDTH_INDEX) << std::setfill(' ') << std::right << std::dec << frame_index << ": "
           << std::setfill('0');
        os << std::setw(WIDTH_FRAME) << std::hex;
        os << (IsCFrame() ? reinterpret_cast<Frame *>(GetCFrame().GetFrameOrigin()) : GetIFrame()) << " in ";
        DumpFrame(os);
        os << std::endl;
        if (print_vregs) {
            IterateVRegsWithInfo([this, &os](auto reg_info, const auto &vreg) {
                os << "     " << std::setw(WIDTH_REG) << std::setfill(' ') << std::right
                   << (reg_info.IsAccumulator() ? "acc" : (std::string("v") + std::to_string(reg_info.GetIndex())));
                os << " = ";
                if (reg_info.GetType() == compiler::VRegInfo::Type::ANY) {
                    os << "0x";
                }
                os << std::left;
                os << std::setw(WIDTH_TYPE) << std::setfill(' ');
                switch (reg_info.GetType()) {
                    case compiler::VRegInfo::Type::INT64:
                    case compiler::VRegInfo::Type::INT32:
                        os << std::dec << vreg.GetLong();
                        break;
                    case compiler::VRegInfo::Type::FLOAT64:
                        os << vreg.GetDouble();
                        break;
                    case compiler::VRegInfo::Type::FLOAT32:
                        os << vreg.GetFloat();
                        break;
                    case compiler::VRegInfo::Type::BOOL:
                        os << (vreg.Get() ? "true" : "false");
                        break;
                    case compiler::VRegInfo::Type::OBJECT:
                        os << vreg.GetReference();
                        break;
                    case compiler::VRegInfo::Type::ANY: {
                        os << std::hex << static_cast<uint64_t>(vreg.GetValue());
                        break;
                    }
                    case compiler::VRegInfo::Type::UNDEFINED:
                        os << "undfined";
                        break;
                    default:
                        os << "unknown";
                        break;
                }
                os << std::setw(WIDTH_LOCATION) << std::setfill(' ') << reg_info.GetTypeString();  // NOLINT
                if (IsCFrame()) {
                    os << reg_info.GetLocationString() << ":" << std::dec << helpers::ToSigned(reg_info.GetValue());
                } else {
                    os << '-';
                }
                os << std::endl;
                return true;
            });
        }
        if (IsCFrame() && print_vregs) {
            os << "roots:";
            IterateObjectsWithInfo([&os](auto &reg_info, const auto &vreg) {
                ASSERT(vreg.HasObject());
                os << " " << reg_info.GetLocationString() << "[" << std::dec << reg_info.GetValue() << "]=" << std::hex
                   << vreg.GetReference();
                return true;
            });
            os << std::endl;
        }
        frame_index++;
    }
}

void StackWalker::DumpFrame(std::ostream &os)
{
    os << (IsDynamicMethod() ? "[dynamic function]" : GetMethod()->GetFullName());
    if (IsCFrame()) {
        if (GetCFrame().IsNative()) {
            os << " (native)";
        } else {
            os << " (compiled" << (GetCFrame().IsOsr() ? "/osr" : "") << ": npc=" << GetNativePc()
               << (IsInlined() ? ", inlined) " : ") ");
            if (IsInlined()) {
                code_info_.DumpInlineInfo(os, stackmap_, inline_depth_);
            } else {
                code_info_.Dump(os, stackmap_);
            }
        }

    } else {
        os << " (managed)";
    }
}

}  // namespace panda
