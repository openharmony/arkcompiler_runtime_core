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

#include "runtime/osr.h"

#include "libpandabase/events/events.h"
#include "libpandafile/shorty_iterator.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/method.h"
#include "runtime/include/stack_walker.h"
#include "code_info/code_info.h"

namespace panda {

using compiler::CodeInfo;
using compiler::VRegInfo;

static void UnpoisonAsanStack([[maybe_unused]] void *ptr)
{
#ifdef PANDA_ASAN_ON
    uint8_t sp;
    ASAN_UNPOISON_MEMORY_REGION(&sp, reinterpret_cast<uint8_t *>(ptr) - &sp);
#endif  // PANDA_ASAN_ON
}

#if EVENT_OSR_ENTRY_ENABLED
void WriteOsrEventError(Frame *frame, FrameKind kind, uintptr_t loop_head_bc)
{
    events::OsrEntryKind osr_kind;
    switch (kind) {
        case FrameKind::INTERPRETER:
            osr_kind = events::OsrEntryKind::AFTER_IFRAME;
            break;
        case FrameKind::COMPILER:
            osr_kind = events::OsrEntryKind::AFTER_CFRAME;
            break;
        case FrameKind::NONE:
            osr_kind = events::OsrEntryKind::TOP_FRAME;
            break;
        default:
            UNREACHABLE();
    }
    EVENT_OSR_ENTRY(std::string(frame->GetMethod()->GetFullName()), loop_head_bc, osr_kind,
                    events::OsrEntryResult::ERROR);
}
#endif  // EVENT_OSR_ENTRY_ENABLED

bool OsrEntry(uintptr_t loop_head_bc, const void *osr_code)
{
    auto stack = StackWalker::Create(ManagedThread::GetCurrent());
    Frame *frame = stack.GetIFrame();
    LOG(DEBUG, INTEROP) << "OSR entry in method '" << stack.GetMethod()->GetFullName() << "': " << osr_code;
    CodeInfo code_info(CodeInfo::GetCodeOriginFromEntryPoint(osr_code));
    auto stackmap = code_info.FindOsrStackMap(loop_head_bc);
    if (!stackmap.IsValid()) {
#if EVENT_OSR_ENTRY_ENABLED
        WriteOsrEventError(frame, stack.GetPreviousFrameKind(), loop_head_bc);
#endif  // EVENT_OSR_ENTRY_ENABLED
        return false;
    }

    switch (stack.GetPreviousFrameKind()) {
        case FrameKind::INTERPRETER:
            LOG(DEBUG, INTEROP) << "OSR: after interpreter frame";
            EVENT_OSR_ENTRY(std::string(frame->GetMethod()->GetFullName()), loop_head_bc,
                            events::OsrEntryKind::AFTER_IFRAME, events::OsrEntryResult::SUCCESS);
            OsrEntryAfterIFrame(frame, loop_head_bc, osr_code, code_info.GetFrameSize());
            break;
        case FrameKind::COMPILER:
            UnpoisonAsanStack(frame->GetPrevFrame());
            LOG(DEBUG, INTEROP) << "OSR: after compiled frame";
            EVENT_OSR_ENTRY(std::string(frame->GetMethod()->GetFullName()), loop_head_bc,
                            events::OsrEntryKind::AFTER_CFRAME, events::OsrEntryResult::SUCCESS);
            OsrEntryAfterCFrame(frame, loop_head_bc, osr_code, code_info.GetFrameSize());
            UNREACHABLE();
            break;
        case FrameKind::NONE:
            LOG(DEBUG, INTEROP) << "OSR: after no frame";
            EVENT_OSR_ENTRY(std::string(frame->GetMethod()->GetFullName()), loop_head_bc,
                            events::OsrEntryKind::TOP_FRAME, events::OsrEntryResult::SUCCESS);
            OsrEntryTopFrame(frame, loop_head_bc, osr_code, code_info.GetFrameSize());
            break;
        default:
            break;
    }
    return true;
}

extern "C" void *PrepareOsrEntry(const Frame *iframe, uintptr_t bc_offset, const void *osr_code, void *cframe_ptr,
                                 uintptr_t *reg_buffer, uintptr_t *fp_reg_buffer)
{
    CodeInfo code_info(CodeInfo::GetCodeOriginFromEntryPoint(osr_code));
    CFrame cframe(cframe_ptr);
    auto stackmap = code_info.FindOsrStackMap(bc_offset);

    ASSERT(stackmap.IsValid() && osr_code != nullptr);

    cframe.SetMethod(iframe->GetMethod());
    cframe.SetFrameKind(CFrameLayout::FrameKind::OSR);
    cframe.SetHasFloatRegs(code_info.HasFloatRegs());

    for (auto vreg : code_info.GetVRegList(stackmap, mem::InternalAllocator<>::GetInternalAllocatorFromRuntime())) {
        int64_t value =
            vreg.IsAccumulator() ? iframe->GetAcc().GetValue() : iframe->GetVReg(vreg.GetIndex()).GetValue();
        if (!vreg.IsLive()) {
            continue;
        }
        switch (vreg.GetLocation()) {
            case VRegInfo::Location::SLOT:
                cframe.SetVRegValue(vreg, value, nullptr);
                break;
            case VRegInfo::Location::REGISTER:
                reg_buffer[vreg.GetValue()] = value;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            case VRegInfo::Location::FP_REGISTER:
                fp_reg_buffer[vreg.GetValue()] = value;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            // NOLINTNEXTLINE(bugprone-branch-clone)
            case VRegInfo::Location::CONSTANT:
                break;
            default:
                break;
        }
    }
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    thread->SetCurrentFrame(reinterpret_cast<Frame *>(cframe_ptr));
    thread->SetCurrentFrameIsCompiled(true);

    return bit_cast<void *>(bit_cast<uintptr_t>(osr_code) + stackmap.GetNativePcUnpacked());
}

extern "C" void SetOsrResult(Frame *frame, uint64_t uval, double fval)
{
    ASSERT(frame != nullptr);
    panda_file::ShortyIterator it(frame->GetMethod()->GetShorty());
    using panda_file::Type;
    auto &acc = frame->GetAcc();

    switch ((*it).GetId()) {
        case Type::TypeId::U1:
        case Type::TypeId::I8:
        case Type::TypeId::U8:
        case Type::TypeId::I16:
        case Type::TypeId::U16:
        case Type::TypeId::I32:
        case Type::TypeId::U32:
        case Type::TypeId::I64:
        case Type::TypeId::U64:
            acc.SetValue(uval);
            acc.SetTag(interpreter::StaticVRegisterRef::PRIMITIVE_TYPE);
            break;
        case Type::TypeId::REFERENCE:
            acc.SetValue(uval);
            acc.SetTag(interpreter::StaticVRegisterRef::GC_OBJECT_TYPE);
            break;
        case Type::TypeId::F32:
        case Type::TypeId::F64:
            acc.SetValue(bit_cast<int64_t>(fval));
            acc.SetTag(interpreter::StaticVRegisterRef::PRIMITIVE_TYPE);
            break;
        case Type::TypeId::VOID:
            // Interpreter always restores accumulator from the callee method, even if callee method is void. Thus, we
            // need to reset it here, otherwise it can hold old object, that probably isn't live already.
            acc.SetValue(0);
            acc.SetTag(interpreter::StaticVRegisterRef::PRIMITIVE_TYPE);
            break;
        case Type::TypeId::TAGGED:
        case Type::TypeId::INVALID:
            UNREACHABLE();
        default:
            UNREACHABLE();
    }
}

}  // namespace panda
