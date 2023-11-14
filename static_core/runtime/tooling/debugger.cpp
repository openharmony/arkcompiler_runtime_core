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

#include "debugger.h"
#include "include/stack_walker.h"
#include "include/thread_scopes.h"
#include "include/tooling/pt_location.h"
#include "include/tooling/pt_thread.h"
#include "pt_scoped_managed_code.h"
#include "interpreter/frame.h"
#include "include/mem/panda_smart_pointers.h"
#include "pt_thread_info.h"

#include "libpandabase/macros.h"
#include "libpandabase/os/mem.h"
#include "libpandabase/utils/expected.h"
#include "libpandabase/utils/span.h"
#include "libpandafile/bytecode_instruction.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/thread-inl.h"
#include "runtime/interpreter/frame.h"
#include "runtime/handle_scope-inl.h"

namespace panda::tooling {
// NOTE(maksenov): remove PtProperty class
static PtProperty FieldToPtProperty(Field *field)
{
    return PtProperty(field);
}

static Field *PtPropertyToField(PtProperty property)
{
    return reinterpret_cast<Field *>(property.GetData());
}

std::optional<Error> Debugger::SetNotification(PtThread thread, bool enable, PtHookType hook_type)
{
    if (thread == PtThread::NONE) {
        if (enable) {
            hooks_.EnableGlobalHook(hook_type);
        } else {
            hooks_.DisableGlobalHook(hook_type);
        }
    } else {
        ManagedThread *managed_thread = thread.GetManagedThread();
        if (enable) {
            managed_thread->GetPtThreadInfo()->GetHookTypeInfo().Enable(hook_type);
        } else {
            managed_thread->GetPtThreadInfo()->GetHookTypeInfo().Disable(hook_type);
        }
    }

    return {};
}

static bool CheckLocationInClass(const panda_file::File &pf, panda_file::File::EntityId class_id,
                                 const PtLocation &location, std::optional<Error> &error)
{
    panda_file::ClassDataAccessor cda(pf, class_id);
    bool found = false;
    cda.EnumerateMethods([&pf, &location, &error, &found](panda_file::MethodDataAccessor mda) {
        if (mda.GetMethodId() == location.GetMethodId()) {
            found = true;
            auto code_id = mda.GetCodeId();
            uint32_t code_size = 0;
            if (code_id.has_value()) {
                panda_file::CodeDataAccessor code_da(pf, *code_id);
                code_size = code_da.GetCodeSize();
            }
            if (location.GetBytecodeOffset() >= code_size) {
                error = Error(Error::Type::INVALID_BREAKPOINT,
                              std::string("Invalid breakpoint location: bytecode offset (") +
                                  std::to_string(location.GetBytecodeOffset()) + ") >= method code size (" +
                                  std::to_string(code_size) + ")");
            }
            return false;
        }
        return true;
    });
    return found;
}

std::optional<Error> Debugger::CheckLocation(const PtLocation &location)
{
    std::optional<Error> res;
    runtime_->GetClassLinker()->EnumeratePandaFiles([&location, &res](const panda_file::File &pf) {
        if (pf.GetFilename() != location.GetPandaFile()) {
            return true;
        }

        auto classes = pf.GetClasses();
        bool found = false;
        for (size_t i = 0; i < classes.Size(); i++) {
            panda_file::File::EntityId id(classes[i]);
            if (pf.IsExternal(id) || id.GetOffset() > location.GetMethodId().GetOffset()) {
                continue;
            }

            found = CheckLocationInClass(pf, id, location, res);
            if (found) {
                break;
            }
        }
        if (!found) {
            res =
                Error(Error::Type::METHOD_NOT_FOUND,
                      std::string("Cannot find method with id ") + std::to_string(location.GetMethodId().GetOffset()) +
                          " in panda file '" + std::string(location.GetPandaFile()) + "'");
        }
        return false;
    });
    return res;
}

std::optional<Error> Debugger::SetBreakpoint(const PtLocation &location)
{
    auto error = CheckLocation(location);
    if (error.has_value()) {
        return error;
    }

    os::memory::WriteLockHolder wholder(rwlock_);
    if (!breakpoints_.emplace(location).second) {
        return Error(Error::Type::BREAKPOINT_ALREADY_EXISTS,
                     std::string("Breakpoint already exists: bytecode offset ") +
                         std::to_string(location.GetBytecodeOffset()));
    }

    return {};
}

std::optional<Error> Debugger::RemoveBreakpoint(const PtLocation &location)
{
    if (!EraseBreakpoint(location)) {
        return Error(Error::Type::BREAKPOINT_NOT_FOUND, "Breakpoint not found");
    }

    return {};
}

static panda::Frame *GetPandaFrame(StackWalker *pstack, uint32_t frame_depth, bool *out_is_native = nullptr)
{
    ASSERT(pstack != nullptr);
    StackWalker &stack = *pstack;

    while (stack.HasFrame() && frame_depth != 0) {
        stack.NextFrame();
        --frame_depth;
    }

    bool is_native = false;
    panda::Frame *frame = nullptr;
    if (stack.HasFrame()) {
        if (stack.IsCFrame()) {
            is_native = true;
        } else {
            frame = stack.GetIFrame();
        }
    }

    if (out_is_native != nullptr) {
        *out_is_native = is_native;
    }

    return frame;
}

static panda::Frame *GetPandaFrame(ManagedThread *thread, uint32_t frame_depth = 0, bool *out_is_native = nullptr)
{
    auto stack = StackWalker::Create(thread);
    return GetPandaFrame(&stack, frame_depth, out_is_native);
}

static interpreter::StaticVRegisterRef GetThisAddrVRegByPandaFrame(panda::Frame *frame)
{
    ASSERT(!frame->IsDynamic());
    ASSERT(frame->GetMethod()->GetNumArgs() > 0);
    uint32_t this_reg_num = frame->GetSize() - frame->GetMethod()->GetNumArgs();
    return StaticFrameHandler(frame).GetVReg(this_reg_num);
}

static interpreter::DynamicVRegisterRef GetThisAddrVRegByPandaFrameDyn(panda::Frame *frame)
{
    ASSERT(frame->IsDynamic());
    ASSERT(frame->GetMethod()->GetNumArgs() > 0);
    uint32_t this_reg_num = frame->GetSize() - frame->GetMethod()->GetNumArgs();
    return DynamicFrameHandler(frame).GetVReg(this_reg_num);
}

template <typename Callback>
Expected<panda::Frame *, Error> GetPandaFrameByPtThread(PtThread thread, uint32_t frame_depth,
                                                        Callback native_frame_handler)
{
    ManagedThread *managed_thread = thread.GetManagedThread();
    ASSERT(managed_thread != nullptr);

    if (MTManagedThread::ThreadIsMTManagedThread(managed_thread)) {
        // Check if thread is suspended
        MTManagedThread *mt_managed_thread = MTManagedThread::CastFromThread(managed_thread);
        if (MTManagedThread::GetCurrent() != mt_managed_thread && !mt_managed_thread->IsUserSuspended()) {
            return Unexpected(Error(Error::Type::THREAD_NOT_SUSPENDED,
                                    std::string("Thread " + std::to_string(thread.GetId()) + " is not suspended")));
        }
    }

    auto stack = StackWalker::Create(managed_thread);
    panda::Frame *frame = GetPandaFrame(&stack, frame_depth, nullptr);
    if (frame == nullptr) {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (!std::is_same_v<decltype(native_frame_handler), std::nullptr_t>) {
            native_frame_handler(&stack);
        }
        return Unexpected(Error(Error::Type::FRAME_NOT_FOUND,
                                std::string("Frame not found or native, threadId=" + std::to_string(thread.GetId()) +
                                            " frameDepth=" + std::to_string(frame_depth))));
    }
    return frame;
}

Expected<interpreter::StaticVRegisterRef, Error> Debugger::GetVRegByPandaFrame(panda::Frame *frame,
                                                                               int32_t reg_number) const
{
    if (reg_number == -1) {
        return frame->GetAccAsVReg();
    }

    if (reg_number >= 0 && uint32_t(reg_number) < frame->GetSize()) {
        return StaticFrameHandler(frame).GetVReg(uint32_t(reg_number));
    }

    return Unexpected(
        Error(Error::Type::INVALID_REGISTER, std::string("Invalid register number: ") + std::to_string(reg_number)));
}

Expected<interpreter::DynamicVRegisterRef, Error> Debugger::GetVRegByPandaFrameDyn(panda::Frame *frame,
                                                                                   int32_t reg_number) const
{
    if (reg_number == -1) {
        return frame->template GetAccAsVReg<true>();
    }

    if (reg_number >= 0 && uint32_t(reg_number) < frame->GetSize()) {
        return DynamicFrameHandler(frame).GetVReg(uint32_t(reg_number));
    }

    return Unexpected(
        Error(Error::Type::INVALID_REGISTER, std::string("Invalid register number: ") + std::to_string(reg_number)));
}

std::optional<Error> Debugger::GetThisVariableByFrame(PtThread thread, uint32_t frame_depth, ObjectHeader **this_ptr)
{
    ASSERT_MANAGED_CODE();
    *this_ptr = nullptr;

    std::optional<Error> native_error;

    auto native_frame_handler = [thread, &native_error, this_ptr](StackWalker *stack) {
        if (!stack->GetCFrame().IsNative()) {
            return;
        }
        if (stack->GetCFrame().GetMethod()->IsStatic()) {
            native_error =
                Error(Error::Type::INVALID_VALUE, std::string("Static native method, no this address slot, threadId=" +
                                                              std::to_string(thread.GetId())));
            return;
        }
        stack->IterateObjects([this_ptr](auto &vreg) {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (std::is_same_v<decltype(vreg), interpreter::StaticVRegisterRef &>) {
                ASSERT(vreg.HasObject());
                *this_ptr = vreg.GetReference();
            }
            return false;
        });
    };
    auto ret = GetPandaFrameByPtThread(thread, frame_depth, native_frame_handler);
    if (native_error) {
        return native_error;
    }
    if (*this_ptr != nullptr) {
        // The value was set by native frame handler
        return {};
    }
    if (!ret) {
        return ret.Error();
    }
    Frame *frame = ret.Value();
    if (frame->GetMethod()->IsStatic()) {
        return Error(Error::Type::INVALID_VALUE,
                     std::string("Static method, no this address slot, threadId=" + std::to_string(thread.GetId())));
    }

    if (frame->IsDynamic()) {
        auto reg = GetThisAddrVRegByPandaFrameDyn(frame);
        *this_ptr = reg.GetReference();
    } else {
        auto reg = GetThisAddrVRegByPandaFrame(ret.Value());
        *this_ptr = reg.GetReference();
    }
    return {};
}

std::optional<Error> Debugger::GetVariable(PtThread thread, uint32_t frame_depth, int32_t reg_number,
                                           VRegValue *result) const
{
    ASSERT_MANAGED_CODE();
    auto ret = GetPandaFrameByPtThread(thread, frame_depth, nullptr);
    if (!ret) {
        return ret.Error();
    }

    Frame *frame = ret.Value();
    if (frame->IsDynamic()) {
        auto reg = GetVRegByPandaFrameDyn(frame, reg_number);
        if (!reg) {
            return reg.Error();
        }

        auto vreg = reg.Value();
        *result = VRegValue(vreg.GetValue());
        return {};
    }

    auto reg = GetVRegByPandaFrame(ret.Value(), reg_number);
    if (!reg) {
        return reg.Error();
    }

    auto vreg = reg.Value();
    *result = VRegValue(vreg.GetValue());
    return {};
}

std::optional<Error> Debugger::SetVariable(PtThread thread, uint32_t frame_depth, int32_t reg_number,
                                           const VRegValue &value) const
{
    ASSERT_MANAGED_CODE();
    auto ret = GetPandaFrameByPtThread(thread, frame_depth, nullptr);
    if (!ret) {
        return ret.Error();
    }

    if (ret.Value()->IsDynamic()) {
        auto reg = GetVRegByPandaFrameDyn(ret.Value(), reg_number);
        if (!reg) {
            return reg.Error();
        }

        auto vreg = reg.Value();
        vreg.SetValue(value.GetValue());
        return {};
    }

    auto reg = GetVRegByPandaFrame(ret.Value(), reg_number);
    if (!reg) {
        return reg.Error();
    }

    auto vreg = reg.Value();
    vreg.SetValue(value.GetValue());
    return {};
}

Expected<std::unique_ptr<PtFrame>, Error> Debugger::GetCurrentFrame(PtThread thread) const
{
    ManagedThread *managed_thread = thread.GetManagedThread();
    ASSERT(managed_thread != nullptr);

    auto stack = StackWalker::Create(managed_thread);
    Method *method = stack.GetMethod();

    Frame *interpreter_frame = nullptr;
    if (!stack.IsCFrame()) {
        interpreter_frame = stack.GetIFrame();
    }

    return {std::make_unique<PtDebugFrame>(method, interpreter_frame)};
}

std::optional<Error> Debugger::EnumerateFrames(PtThread thread, std::function<bool(const PtFrame &)> callback) const
{
    ManagedThread *managed_thread = thread.GetManagedThread();
    ASSERT(managed_thread != nullptr);

    auto stack = StackWalker::Create(managed_thread);
    while (stack.HasFrame()) {
        Method *method = stack.GetMethod();
        Frame *frame = stack.IsCFrame() ? nullptr : stack.GetIFrame();
        PtDebugFrame debug_frame(method, frame);
        if (!callback(debug_frame)) {
            break;
        }
        stack.NextFrame();
    }

    return {};
}

std::optional<Error> Debugger::SuspendThread(PtThread thread) const
{
    ManagedThread *managed_thread = thread.GetManagedThread();
    ASSERT(managed_thread != nullptr);

    if (!MTManagedThread::ThreadIsMTManagedThread(managed_thread)) {
        return Error(Error::Type::THREAD_NOT_FOUND,
                     std::string("Thread ") + std::to_string(thread.GetId()) + " is not MT Thread");
    }
    MTManagedThread *mt_managed_thread = MTManagedThread::CastFromThread(managed_thread);
    mt_managed_thread->Suspend();

    return {};
}

std::optional<Error> Debugger::ResumeThread(PtThread thread) const
{
    ManagedThread *managed_thread = thread.GetManagedThread();
    ASSERT(managed_thread != nullptr);

    if (!MTManagedThread::ThreadIsMTManagedThread(managed_thread)) {
        return Error(Error::Type::THREAD_NOT_FOUND,
                     std::string("Thread ") + std::to_string(thread.GetId()) + " is not MT Thread");
    }
    MTManagedThread *mt_managed_thread = MTManagedThread::CastFromThread(managed_thread);
    mt_managed_thread->Resume();

    return {};
}

std::optional<Error> Debugger::RestartFrame(PtThread thread, uint32_t frame_number) const
{
    ManagedThread *managed_thread = thread.GetManagedThread();
    ASSERT(managed_thread != nullptr);

    if (!managed_thread->IsUserSuspended()) {
        return Error(Error::Type::THREAD_NOT_SUSPENDED,
                     std::string("Thread ") + std::to_string(thread.GetId()) + " is not suspended");
    }

    auto stack = StackWalker::Create(managed_thread);
    panda::Frame *pop_frame = nullptr;
    panda::Frame *retry_frame = nullptr;
    uint32_t current_frame_number = 0;

    while (stack.HasFrame()) {
        if (stack.IsCFrame()) {
            return Error(Error::Type::OPAQUE_FRAME, std::string("Thread ") + std::to_string(thread.GetId()) +
                                                        ", frame at depth is executing a native method");
        }
        if (current_frame_number == frame_number) {
            pop_frame = stack.GetIFrame();
        } else if (current_frame_number == (frame_number + 1)) {
            retry_frame = stack.GetIFrame();
            break;
        }
        ++current_frame_number;
        stack.NextFrame();
    }

    if (pop_frame == nullptr) {
        return Error(Error::Type::FRAME_NOT_FOUND, std::string("Thread ") + std::to_string(thread.GetId()) +
                                                       " doesn't have managed frame with number " +
                                                       std::to_string(frame_number));
    }

    if (retry_frame == nullptr) {
        return Error(Error::Type::NO_MORE_FRAMES, std::string("Thread ") + std::to_string(thread.GetId()) +
                                                      " does not have more than one frame on the call stack");
    }

    // Set force pop frames from top to target
    stack.Reset(managed_thread);
    while (stack.HasFrame()) {
        panda::Frame *frame = stack.GetIFrame();
        frame->SetForcePop();
        if (frame == pop_frame) {
            break;
        }
        stack.NextFrame();
    }
    retry_frame->SetRetryInstruction();

    return {};
}

std::optional<Error> Debugger::NotifyFramePop(PtThread thread, uint32_t depth) const
{
    ManagedThread *managed_thread = thread.GetManagedThread();
    ASSERT(managed_thread != nullptr);

    /* NOTE: (cmd) the second NotifyFramePop is error. use one debugger instance to resolve this.
    if (!mt_managed_thread->IsUserSuspended()) {
        return Error(Error::Type::THREAD_NOT_SUSPENDED,
                     std::string("Thread ") + std::to_string(thread.GetId()) + " is not suspended");
    }
    */

    bool is_native = false;
    panda::Frame *pop_frame = GetPandaFrame(managed_thread, depth, &is_native);
    if (pop_frame == nullptr) {
        if (is_native) {
            return Error(Error::Type::OPAQUE_FRAME, std::string("Thread ") + std::to_string(thread.GetId()) +
                                                        ", frame at depth is executing a native method");
        }

        return Error(Error::Type::NO_MORE_FRAMES,
                     std::string("Thread ") + std::to_string(thread.GetId()) +
                         ", are no stack frames at the specified depth: " + std::to_string(depth));
    }

    pop_frame->SetNotifyPop();
    return {};
}

void Debugger::BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bc_offset)
{
    ASSERT(bc_offset < method->GetCodeSize() && "code size of current method less then bc_offset");
    PtLocation location(method->GetPandaFile()->GetFilename().c_str(), method->GetFileId(), bc_offset);

    // Step event is reported before breakpoint, according to the spec.
    HandleStep(thread, method, location);
    HandleBreakpoint(thread, method, location);

    if (IsPropertyWatchActive()) {
        if (!HandlePropertyAccess(thread, method, location)) {
            HandlePropertyModify(thread, method, location);
        }
    }
}

void Debugger::ObjectAlloc(BaseClass *klass, ObjectHeader *object, ManagedThread *thread, size_t size)
{
    if (!vm_started_) {
        return;
    }
    if (thread == nullptr) {
        thread = ManagedThread::GetCurrent();
        if (thread == nullptr) {
            return;
        }
    }

    hooks_.ObjectAlloc(klass, object, PtThread(thread), size);
}

void Debugger::MethodEntry(ManagedThread *managed_thread, Method *method)
{
    hooks_.MethodEntry(PtThread(managed_thread), method);
}

void Debugger::MethodExit(ManagedThread *managed_thread, Method *method)
{
    bool is_exception_triggered = managed_thread->HasPendingException();
    VRegValue ret_value(managed_thread->GetCurrentFrame()->GetAcc().GetValue());
    hooks_.MethodExit(PtThread(managed_thread), method, is_exception_triggered, ret_value);

    HandleNotifyFramePop(managed_thread, method, is_exception_triggered);
}

void Debugger::ClassLoad(Class *klass)
{
    auto *thread = Thread::GetCurrent();
    if (!vm_started_ || thread->GetThreadType() == Thread::ThreadType::THREAD_TYPE_COMPILER) {
        return;
    }

    hooks_.ClassLoad(PtThread(ManagedThread::CastFromThread(thread)), klass);
}

void Debugger::ClassPrepare(Class *klass)
{
    auto *thread = Thread::GetCurrent();
    if (!vm_started_ || thread->GetThreadType() == Thread::ThreadType::THREAD_TYPE_COMPILER) {
        return;
    }

    hooks_.ClassPrepare(PtThread(ManagedThread::CastFromThread(thread)), klass);
}

void Debugger::MonitorWait(ObjectHeader *object, int64_t timeout)
{
    hooks_.MonitorWait(PtThread(ManagedThread::GetCurrent()), object, timeout);
}

void Debugger::MonitorWaited(ObjectHeader *object, bool timed_out)
{
    hooks_.MonitorWaited(PtThread(ManagedThread::GetCurrent()), object, timed_out);
}

void Debugger::MonitorContendedEnter(ObjectHeader *object)
{
    hooks_.MonitorContendedEnter(PtThread(ManagedThread::GetCurrent()), object);
}

void Debugger::MonitorContendedEntered(ObjectHeader *object)
{
    hooks_.MonitorContendedEntered(PtThread(ManagedThread::GetCurrent()), object);
}

bool Debugger::HandleBreakpoint(ManagedThread *managed_thread, Method *method, const PtLocation &location)
{
    {
        os::memory::ReadLockHolder rholder(rwlock_);
        if (!IsBreakpoint(location)) {
            return false;
        }
    }

    hooks_.Breakpoint(PtThread(managed_thread), method, location);
    return true;
}

void Debugger::ExceptionThrow(ManagedThread *thread, Method *method, ObjectHeader *exception_object, uint32_t bc_offset)
{
    ASSERT(thread->HasPendingException());
    HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> handle(thread, exception_object);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*method);
    std::pair<Method *, uint32_t> res = ctx.GetCatchMethodAndOffset(method, thread);
    auto *catch_method_file = res.first->GetPandaFile();

    PtLocation throw_location {method->GetPandaFile()->GetFilename().c_str(), method->GetFileId(), bc_offset};
    PtLocation catch_location {catch_method_file->GetFilename().c_str(), res.first->GetFileId(), res.second};

    hooks_.Exception(PtThread(thread), method, throw_location, handle.GetPtr(), res.first, catch_location);
}

void Debugger::ExceptionCatch(ManagedThread *thread, Method *method, ObjectHeader *exception_object, uint32_t bc_offset)
{
    ASSERT(!thread->HasPendingException());

    auto *pf = method->GetPandaFile();
    PtLocation catch_location {pf->GetFilename().c_str(), method->GetFileId(), bc_offset};

    hooks_.ExceptionCatch(PtThread(thread), method, catch_location, exception_object);
}

bool Debugger::HandleStep(ManagedThread *managed_thread, Method *method, const PtLocation &location)
{
    hooks_.SingleStep(PtThread(managed_thread), method, location);
    return true;
}

void Debugger::HandleNotifyFramePop(ManagedThread *managed_thread, Method *method, bool was_popped_by_exception)
{
    panda::Frame *frame = GetPandaFrame(managed_thread);
    if (frame != nullptr && frame->IsNotifyPop()) {
        hooks_.FramePop(PtThread(managed_thread), method, was_popped_by_exception);
        frame->ClearNotifyPop();
    }
}

static Field *ResolveField(ManagedThread *thread, const Method *caller, const BytecodeInstruction &inst)
{
    auto property_index = inst.GetId().AsIndex();
    auto property_id = caller->GetClass()->ResolveFieldIndex(property_index);
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(class_linker);
    ASSERT(!thread->HasPendingException());
    auto *field = class_linker->GetField(*caller, property_id);
    if (UNLIKELY(field == nullptr)) {
        // Field might be nullptr if a class was not found
        thread->ClearException();
    }
    return field;
}

bool Debugger::HandlePropertyAccess(ManagedThread *thread, Method *method, const PtLocation &location)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstruction inst(method->GetInstructions() + location.GetBytecodeOffset());
    auto opcode = inst.GetOpcode();
    bool is_static = false;

    switch (opcode) {
        case BytecodeInstruction::Opcode::LDOBJ_V8_ID16:
        case BytecodeInstruction::Opcode::LDOBJ_64_V8_ID16:
        case BytecodeInstruction::Opcode::LDOBJ_OBJ_V8_ID16:
            break;
        case BytecodeInstruction::Opcode::LDSTATIC_ID16:
        case BytecodeInstruction::Opcode::LDSTATIC_64_ID16:
        case BytecodeInstruction::Opcode::LDSTATIC_OBJ_ID16:
            is_static = true;
            break;
        default:
            return false;
    }

    Field *field = ResolveField(thread, method, inst);
    if (field == nullptr) {
        return false;
    }
    Class *klass = field->GetClass();
    ASSERT(klass);

    {
        os::memory::ReadLockHolder rholder(rwlock_);
        if (FindPropertyWatch(klass->GetFileId(), field->GetFileId(), PropertyWatch::Type::ACCESS) == nullptr) {
            return false;
        }
    }

    PtProperty pt_property = FieldToPtProperty(field);

    if (is_static) {
        hooks_.PropertyAccess(PtThread(thread), method, location, nullptr, pt_property);
    } else {
        interpreter::VRegister &reg = thread->GetCurrentFrame()->GetVReg(inst.GetVReg());
        hooks_.PropertyAccess(PtThread(thread), method, location, reg.GetReference(), pt_property);
    }

    return true;
}

bool Debugger::HandlePropertyModify(ManagedThread *thread, Method *method, const PtLocation &location)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstruction inst(method->GetInstructions() + location.GetBytecodeOffset());
    auto opcode = inst.GetOpcode();
    bool is_static = false;

    switch (opcode) {
        case BytecodeInstruction::Opcode::STOBJ_V8_ID16:
        case BytecodeInstruction::Opcode::STOBJ_64_V8_ID16:
        case BytecodeInstruction::Opcode::STOBJ_OBJ_V8_ID16:
            break;
        case BytecodeInstruction::Opcode::STSTATIC_ID16:
        case BytecodeInstruction::Opcode::STSTATIC_64_ID16:
        case BytecodeInstruction::Opcode::STSTATIC_OBJ_ID16:
            is_static = true;
            break;
        default:
            return false;
    }

    Field *field = ResolveField(thread, method, inst);
    if (field == nullptr) {
        return false;
    }
    Class *klass = field->GetClass();
    ASSERT(klass);

    {
        os::memory::ReadLockHolder rholder(rwlock_);
        if (FindPropertyWatch(klass->GetFileId(), field->GetFileId(), PropertyWatch::Type::MODIFY) == nullptr) {
            return false;
        }
    }

    PtProperty pt_property = FieldToPtProperty(field);

    VRegValue value(thread->GetCurrentFrame()->GetAcc().GetValue());
    if (is_static) {
        hooks_.PropertyModification(PtThread(thread), method, location, nullptr, pt_property, value);
    } else {
        interpreter::VRegister &reg = thread->GetCurrentFrame()->GetVReg(inst.GetVReg());
        hooks_.PropertyModification(PtThread(thread), method, location, reg.GetReference(), pt_property, value);
    }

    return true;
}

std::optional<Error> Debugger::SetPropertyAccessWatch(BaseClass *klass, PtProperty property)
{
    os::memory::WriteLockHolder wholder(rwlock_);
    ASSERT(!klass->IsDynamicClass());
    panda_file::File::EntityId class_id = static_cast<Class *>(klass)->GetFileId();
    panda_file::File::EntityId property_id = PtPropertyToField(property)->GetFileId();
    if (FindPropertyWatch(class_id, property_id, PropertyWatch::Type::ACCESS) != nullptr) {
        return Error(Error::Type::INVALID_PROPERTY_ACCESS_WATCH,
                     std::string("Invalid property access watch, already exist, ClassID: ") +
                         std::to_string(class_id.GetOffset()) +
                         ", PropertyID: " + std::to_string(property_id.GetOffset()));
    }
    property_watches_.emplace_back(class_id, property_id, PropertyWatch::Type::ACCESS);
    return {};
}

std::optional<Error> Debugger::ClearPropertyAccessWatch(BaseClass *klass, PtProperty property)
{
    ASSERT(!klass->IsDynamicClass());
    panda_file::File::EntityId class_id = static_cast<Class *>(klass)->GetFileId();
    panda_file::File::EntityId property_id = PtPropertyToField(property)->GetFileId();
    if (!RemovePropertyWatch(class_id, property_id, PropertyWatch::Type::ACCESS)) {
        return Error(Error::Type::PROPERTY_ACCESS_WATCH_NOT_FOUND,
                     std::string("Property access watch not found, ClassID: ") + std::to_string(class_id.GetOffset()) +
                         ", PropertyID: " + std::to_string(property_id.GetOffset()));
    }
    return {};
}

std::optional<Error> Debugger::SetPropertyModificationWatch(BaseClass *klass, PtProperty property)
{
    os::memory::WriteLockHolder wholder(rwlock_);
    ASSERT(!klass->IsDynamicClass());
    panda_file::File::EntityId class_id = static_cast<Class *>(klass)->GetFileId();
    panda_file::File::EntityId property_id = PtPropertyToField(property)->GetFileId();
    if (FindPropertyWatch(class_id, property_id, PropertyWatch::Type::MODIFY) != nullptr) {
        return Error(Error::Type::INVALID_PROPERTY_MODIFY_WATCH,
                     std::string("Invalid property modification watch, already exist, ClassID: ") +
                         std::to_string(class_id.GetOffset()) + ", PropertyID" +
                         std::to_string(property_id.GetOffset()));
    }
    property_watches_.emplace_back(class_id, property_id, PropertyWatch::Type::MODIFY);
    return {};
}

std::optional<Error> Debugger::ClearPropertyModificationWatch(BaseClass *klass, PtProperty property)
{
    ASSERT(!klass->IsDynamicClass());
    panda_file::File::EntityId class_id = static_cast<Class *>(klass)->GetFileId();
    panda_file::File::EntityId property_id = PtPropertyToField(property)->GetFileId();
    if (!RemovePropertyWatch(class_id, property_id, PropertyWatch::Type::MODIFY)) {
        return Error(Error::Type::PROPERTY_MODIFY_WATCH_NOT_FOUND,
                     std::string("Property modification watch not found, ClassID: ") +
                         std::to_string(class_id.GetOffset()) + ", PropertyID" +
                         std::to_string(property_id.GetOffset()));
    }
    return {};
}

bool Debugger::IsBreakpoint(const PtLocation &location) const REQUIRES_SHARED(rwlock_)
{
    auto it = breakpoints_.find(location);
    return it != breakpoints_.end();
}

bool Debugger::EraseBreakpoint(const PtLocation &location)
{
    os::memory::WriteLockHolder wholder(rwlock_);
    auto it = breakpoints_.find(location);
    if (it != breakpoints_.end()) {
        breakpoints_.erase(it);
        return true;
    }
    return false;
}

const tooling::PropertyWatch *Debugger::FindPropertyWatch(panda_file::File::EntityId class_id,
                                                          panda_file::File::EntityId field_id,
                                                          tooling::PropertyWatch::Type type) const
    REQUIRES_SHARED(rwlock_)
{
    for (const auto &pw : property_watches_) {
        if (pw.GetClassId() == class_id && pw.GetFieldId() == field_id && pw.GetType() == type) {
            return &pw;
        }
    }

    return nullptr;
}

bool Debugger::RemovePropertyWatch(panda_file::File::EntityId class_id, panda_file::File::EntityId field_id,
                                   tooling::PropertyWatch::Type type)
{
    os::memory::WriteLockHolder wholder(rwlock_);
    auto it = property_watches_.begin();
    while (it != property_watches_.end()) {
        const auto &pw = *it;
        if (pw.GetClassId() == class_id && pw.GetFieldId() == field_id && pw.GetType() == type) {
            property_watches_.erase(it);
            return true;
        }

        it++;
    }

    return false;
}

template <class VRegRef>
static inline uint64_t GetVRegValue(VRegRef reg)
{
    return reg.HasObject() ? reinterpret_cast<uintptr_t>(reg.GetReference()) : reg.GetLong();
}

template <class VRegRef>
static inline PtFrame::RegisterKind GetVRegKind([[maybe_unused]] VRegRef reg)
{
    if constexpr (std::is_same<VRegRef, interpreter::DynamicVRegisterRef>::value) {
        return PtFrame::RegisterKind::TAGGED;
    } else {
        return reg.HasObject() ? PtFrame::RegisterKind::REFERENCE : PtFrame::RegisterKind::PRIMITIVE;
    }
}

template <class FrameHandler>
static inline void FillRegisters(Frame *interpreter_frame, PandaVector<uint64_t> &vregs,
                                 PandaVector<PtFrame::RegisterKind> &vreg_kinds, size_t nregs,
                                 PandaVector<uint64_t> &args, PandaVector<PtFrame::RegisterKind> &arg_kinds,
                                 size_t nargs, uint64_t &acc, PtFrame::RegisterKind &acc_kind)
{
    FrameHandler frame_handler(interpreter_frame);

    vregs.reserve(nregs);
    vreg_kinds.reserve(nregs);
    for (size_t i = 0; i < nregs; i++) {
        auto vreg_reg = frame_handler.GetVReg(i);
        vregs.push_back(GetVRegValue(vreg_reg));
        vreg_kinds.push_back(GetVRegKind(vreg_reg));
    }

    args.reserve(nargs);
    arg_kinds.reserve(nargs);
    for (size_t i = 0; i < nargs; i++) {
        auto arg_reg = frame_handler.GetVReg(i + nregs);
        args.push_back(GetVRegValue(arg_reg));
        arg_kinds.push_back(GetVRegKind(arg_reg));
    }

    auto acc_reg = frame_handler.GetAccAsVReg();
    acc = GetVRegValue(acc_reg);
    acc_kind = GetVRegKind(acc_reg);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
PtDebugFrame::PtDebugFrame(Method *method, Frame *interpreter_frame) : method_(method)
{
    panda_file_ = method->GetPandaFile()->GetFilename();
    method_id_ = method->GetFileId();

    is_interpreter_frame_ = interpreter_frame != nullptr;
    if (!is_interpreter_frame_) {
        return;
    }

    size_t nregs = method->GetNumVregs();
    size_t nargs = method->GetNumArgs();
    if (interpreter_frame->IsDynamic()) {
        FillRegisters<DynamicFrameHandler>(interpreter_frame, vregs_, vreg_kinds_, nregs, args_, arg_kinds_, nargs,
                                           acc_, acc_kind_);
    } else {
        FillRegisters<StaticFrameHandler>(interpreter_frame, vregs_, vreg_kinds_, nregs, args_, arg_kinds_, nargs, acc_,
                                          acc_kind_);
    }

    bc_offset_ = interpreter_frame->GetBytecodeOffset();
}

}  // namespace panda::tooling
