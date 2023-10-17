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
#include "runtime/include/panda_vm.h"
#include "compiler/optimizer/ir/runtime_interface.h"

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

static size_t GetStackParamsSize(const Frame *frame);

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
            OsrEntryAfterIFrame(frame, loop_head_bc, osr_code, code_info.GetFrameSize(), GetStackParamsSize(frame));
            break;
        case FrameKind::COMPILER:
            if (frame->IsDynamic() && frame->GetNumActualArgs() < frame->GetMethod()->GetNumArgs()) {
                frame->DisableOsr();
                // We need to adjust slot arguments space from previous compiled frame.
#if EVENT_OSR_ENTRY_ENABLED
                WriteOsrEventError(frame, stack.GetPreviousFrameKind(), loop_head_bc);
#endif  // EVENT_OSR_ENTRY_ENABLED
                LOG(DEBUG, INTEROP) << "OSR: after compiled frame, fail: num_actual_args < num_args";
                return false;
            }
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
            OsrEntryTopFrame(frame, loop_head_bc, osr_code, code_info.GetFrameSize(), GetStackParamsSize(frame));
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

    auto *thread {ManagedThread::GetCurrent()};
    ASSERT(thread != nullptr);

    auto *vm {thread->GetVM()};

    auto num_slots {[](size_t bytes) -> size_t {
        ASSERT(IsAligned(bytes, ArchTraits<RUNTIME_ARCH>::POINTER_SIZE));
        return bytes / ArchTraits<RUNTIME_ARCH>::POINTER_SIZE;
    }};

    Span param_slots(reinterpret_cast<uintptr_t *>(cframe.GetStackArgsStart()), num_slots(GetStackParamsSize(iframe)));

    auto ctx {vm->GetLanguageContext()};
    ctx.InitializeOsrCframeSlots(param_slots);

    for (auto vreg : code_info.GetVRegList(stackmap, mem::InternalAllocator<>::GetInternalAllocatorFromRuntime())) {
        if (!vreg.IsLive()) {
            continue;
        }
        int64_t value = 0;
        if (vreg.IsAccumulator()) {
            value = iframe->GetAcc().GetValue();
        } else if (!vreg.IsSpecialVReg()) {
            ASSERT(vreg.GetVRegType() == VRegInfo::VRegType::VREG);
            value = iframe->GetVReg(vreg.GetIndex()).GetValue();
        } else {
            value = ctx.GetOsrEnv(iframe, vreg);
        }
#ifdef PANDA_USE_32_BIT_POINTER
        if (vreg.IsObject()) {
            value = static_cast<int32_t>(value);
        }
#endif
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
            acc.SetValue(uval);
            break;
        case Type::TypeId::INVALID:
            UNREACHABLE();
        default:
            UNREACHABLE();
    }
}

static size_t GetStackParamsSize(const Frame *frame)
{
    constexpr auto SLOT_SIZE {ArchTraits<RUNTIME_ARCH>::POINTER_SIZE};
    auto *method {frame->GetMethod()};
    if (frame->IsDynamic()) {
        auto num_args {std::max(method->GetNumArgs(), frame->GetNumActualArgs())};
        auto arg_size {std::max(sizeof(coretypes::TaggedType), SLOT_SIZE)};
        return RoundUp(num_args * arg_size, 2U * SLOT_SIZE);
    }

    arch::ArgCounter<RUNTIME_ARCH> counter;
    counter.Count<Method *>();
    if (!method->IsStatic()) {
        counter.Count<ObjectHeader *>();
    }
    panda_file::ShortyIterator it(method->GetShorty());
    ++it;  // skip return type
    while (it != panda_file::ShortyIterator()) {
        switch ((*it).GetId()) {
            case panda_file::Type::TypeId::U1:
            case panda_file::Type::TypeId::U8:
            case panda_file::Type::TypeId::I8:
            case panda_file::Type::TypeId::I16:
            case panda_file::Type::TypeId::U16:
            case panda_file::Type::TypeId::I32:
            case panda_file::Type::TypeId::U32:
                counter.Count<int32_t>();
                break;
            case panda_file::Type::TypeId::F32:
                counter.Count<float>();
                break;
            case panda_file::Type::TypeId::F64:
                counter.Count<double>();
                break;
            case panda_file::Type::TypeId::I64:
            case panda_file::Type::TypeId::U64:
                counter.Count<int64_t>();
                break;
            case panda_file::Type::TypeId::REFERENCE:
                counter.Count<ObjectHeader *>();
                break;
            case panda_file::Type::TypeId::TAGGED:
                counter.Count<coretypes::TaggedType>();
                break;
            default:
                UNREACHABLE();
        }
        ++it;
    }
    return RoundUp(counter.GetOnlyStackSize(), 2U * SLOT_SIZE);
}

}  // namespace panda
