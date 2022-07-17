/*
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

#ifndef PANDA_RUNTIME_METHOD_INL_H_
#define PANDA_RUNTIME_METHOD_INL_H_

#include "entrypoints/entrypoints.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/code_data_accessor.h"
#include "libpandafile/file.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "libpandafile/method_data_accessor.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "libpandafile/proto_data_accessor.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread-inl.h"
#include "runtime/interpreter/interpreter.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/osr.h"

namespace panda {

inline void FrameDeleter::operator()(Frame *frame) const
{
    interpreter::RuntimeInterface::FreeFrame(thread_, frame);
}

class InvokeHelperStatic {
public:
    static constexpr bool is_dynamic = false;

    ALWAYS_INLINE inline static uint32_t GetFrameSize(uint32_t num_vregs, uint32_t num_declared_args,
                                                      [[maybe_unused]] uint32_t num_actual_args)
    {
        return num_vregs + num_declared_args;
    }

    ALWAYS_INLINE inline static void InitActualArgs(Frame *frame, Span<Value> args_span, uint32_t num_vregs,
                                                    [[maybe_unused]] uint32_t num_declared_args)
    {
        StaticFrameHandler static_frame_helper(frame);
        uint32_t num_actual_args = args_span.Size();
        for (size_t i = 0; i < num_actual_args; ++i) {
            if (args_span[i].IsDecodedTaggedValue()) {
                DecodedTaggedValue decoded = args_span[i].GetDecodedTaggedValue();
                static_frame_helper.GetVReg(num_vregs + i).SetValue(decoded.value);
                static_frame_helper.GetVReg(num_vregs + i).SetTag(decoded.tag);
            } else if (args_span[i].IsReference()) {
                static_frame_helper.GetVReg(num_vregs + i).SetReference(args_span[i].GetAs<ObjectHeader *>());
            } else {
                static_frame_helper.GetVReg(num_vregs + i).SetPrimitive(args_span[i].GetAs<int64_t>());
            }
        }
    }

    ALWAYS_INLINE inline static void InterpreterExecute(ManagedThread *thread, const uint8_t *pc, Frame *frame)
    {
        interpreter::Execute(thread, pc, frame);
    }

    ALWAYS_INLINE static Frame *CreateFrame([[maybe_unused]] ManagedThread *thread, uint32_t nregs_size, Method *method,
                                            Frame *prev, uint32_t nregs, uint32_t num_actual_args)
    {
        return interpreter::RuntimeInterface::CreateFrameWithActualArgsAndSize(nregs_size, nregs, num_actual_args,
                                                                               method, prev);
    }
};

class InvokeHelperDynamic {
public:
    static constexpr bool is_dynamic = true;

    ALWAYS_INLINE inline static uint32_t GetFrameSize(uint32_t num_vregs, uint32_t num_declared_args,
                                                      uint32_t num_actual_args)
    {
        return num_vregs + std::max(num_declared_args, num_actual_args);
    }

    ALWAYS_INLINE inline static void InitActualArgs(Frame *frame, Span<coretypes::TaggedValue> args_span,
                                                    uint32_t num_vregs, [[maybe_unused]] uint32_t num_declared_args)
    {
        frame->SetDynamic();

        DynamicFrameHandler dynamic_frame_helper(frame);
        uint32_t num_actual_args = args_span.Size();
        for (size_t i = 0; i < num_actual_args; ++i) {
            dynamic_frame_helper.GetVReg(num_vregs + i).SetValue(args_span[i].GetRawData());
        }

        for (size_t i = num_actual_args; i < num_declared_args; i++) {
            dynamic_frame_helper.GetVReg(num_vregs + i).SetValue(TaggedValue::VALUE_UNDEFINED);
        }
    }

    ALWAYS_INLINE inline static void InterpreterExecute(ManagedThread *thread, const uint8_t *pc, Frame *frame)
    {
        interpreter::Execute(thread, pc, frame);
    }

    ALWAYS_INLINE inline static coretypes::TaggedValue CompiledCodeExecute(ManagedThread *thread, Method *method,
                                                                           uint32_t num_args,
                                                                           coretypes::TaggedValue *args)
    {
        Frame *current_frame = thread->GetCurrentFrame();
        bool is_compiled = thread->IsCurrentFrameCompiled();

        ASSERT(num_args >= 2U);  // NOTE(asoldatov): Adjust this check
        uint64_t ret = InvokeCompiledCodeWithArgArrayDyn(reinterpret_cast<uint64_t *>(args), num_args, current_frame,
                                                         method, thread);
        thread->SetCurrentFrameIsCompiled(is_compiled);
        thread->SetCurrentFrame(current_frame);
        if (UNLIKELY(thread->HasPendingException())) {
            return coretypes::TaggedValue::Exception();
        }
        return coretypes::TaggedValue(ret);
    }

    ALWAYS_INLINE static Frame *CreateFrame([[maybe_unused]] ManagedThread *thread, uint32_t nregs_size, Method *method,
                                            Frame *prev, uint32_t nregs, uint32_t num_actual_args)
    {
        return interpreter::RuntimeInterface::CreateFrameWithActualArgsAndSize(nregs_size, nregs, num_actual_args,
                                                                               method, prev);
    }
};

template <class InvokeHelper, class ValueT>
ValueT Method::GetReturnValueFromException()
{
    if constexpr (InvokeHelper::is_dynamic) {  // NOLINT(readability-braces-around-statements)
        return TaggedValue::Undefined();
    } else {  // NOLINT(readability-misleading-indentation)
        if (GetReturnType().IsReference()) {
            return Value(nullptr);
        }
        return Value(static_cast<int64_t>(0));
    }
}

template <class InvokeHelper, class ValueT>
ValueT Method::GetReturnValueFromAcc(interpreter::AccVRegister &aac_vreg)
{
    if constexpr (InvokeHelper::is_dynamic) {  // NOLINT(readability-braces-around-statements)
        return TaggedValue(aac_vreg.GetAs<uint64_t>());
    } else {  // NOLINT(readability-misleading-indentation)
        ASSERT(GetReturnType().GetId() != panda_file::Type::TypeId::TAGGED);
        if (GetReturnType().GetId() != panda_file::Type::TypeId::VOID) {
            interpreter::StaticVRegisterRef acc = aac_vreg.AsVRegRef<false>();
            if (acc.HasObject()) {
                return Value(aac_vreg.GetReference());
            }
            return Value(aac_vreg.GetLong());
        }
        return Value(static_cast<int64_t>(0));
    }
}

inline Value Method::InvokeCompiledCode(ManagedThread *thread, uint32_t num_args, Value *args)
{
    Frame *current_frame = thread->GetCurrentFrame();
    Span<Value> args_span(args, num_args);
    DecodedTaggedValue ret_value {};
    bool is_compiled = thread->IsCurrentFrameCompiled();
    // Use frame allocator to alloc memory for parameters as thread can be terminated and
    // InvokeCompiledCodeWithArgArray will not return in this case we will get memory leak with internal
    // allocator
    mem::StackFrameAllocator *allocator = thread->GetStackFrameAllocator();
    auto values_deleter = [allocator](int64_t *values) {
        if (values != nullptr) {
            allocator->Free(values);
        }
    };
    auto values = PandaUniquePtr<int64_t, decltype(values_deleter)>(nullptr, values_deleter);
    if (num_args > 0) {
        // In the worse case we are calling a dynamic method in which all arguments are pairs ot int64_t
        // That is why we allocate 2 x num_actual_args
        size_t capacity = num_args * sizeof(int64_t);
        // All allocations though FrameAllocator must be aligned
        capacity = AlignUp(capacity, GetAlignmentInBytes(DEFAULT_FRAME_ALIGNMENT));
        values.reset(reinterpret_cast<int64_t *>(allocator->Alloc(capacity)));
        Span<int64_t> values_span(values.get(), capacity);
        for (uint32_t i = 0; i < num_args; ++i) {
            if (args_span[i].IsReference()) {
                values_span[i] = reinterpret_cast<int64_t>(args_span[i].GetAs<ObjectHeader *>());
            } else {
                values_span[i] = args_span[i].GetAs<int64_t>();
            }
        }
    }

    ret_value = InvokeCompiledCodeWithArgArray(values.get(), current_frame, this, thread);

    thread->SetCurrentFrameIsCompiled(is_compiled);
    thread->SetCurrentFrame(current_frame);
    if (UNLIKELY(thread->HasPendingException())) {
        ret_value = DecodedTaggedValue(0, 0);
    }
    return GetReturnValueFromTaggedValue(ret_value);
}

template <class InvokeHelper, class ValueT>
ValueT Method::InvokeInterpretedCode(ManagedThread *thread, uint32_t num_actual_args, ValueT *args)
{
    if (!Verify()) {
        auto ctx = Runtime::GetCurrent()->GetLanguageContext(*this);
        panda::ThrowVerificationException(ctx, GetFullName());
        return GetReturnValueFromException<InvokeHelper, ValueT>();
    }

    Frame *current_frame = thread->GetCurrentFrame();
    PandaUniquePtr<Frame, FrameDeleter> frame = InitFrame<InvokeHelper>(thread, num_actual_args, args, current_frame);
    if (UNLIKELY(frame.get() == nullptr)) {
        panda::ThrowOutOfMemoryError("CreateFrame failed: " + GetFullName());
        return GetReturnValueFromException<InvokeHelper, ValueT>();
    }

    LOG(DEBUG, INTERPRETER) << "Invoke entry: " << GetFullName();
    auto is_compiled = thread->IsCurrentFrameCompiled();
    thread->SetCurrentFrameIsCompiled(false);
    thread->SetCurrentFrame(frame.get());
    if (is_compiled && current_frame != nullptr) {
        // Create C2I bridge frame in case of previous frame is a native frame or other compiler frame.
        // But create only if the previous frame is not a C2I bridge already.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        C2IBridge bridge;
        if (!StackWalker::IsBoundaryFrame<FrameKind::INTERPRETER>(current_frame)) {
            bridge = {0, reinterpret_cast<uintptr_t>(current_frame), COMPILED_CODE_TO_INTERPRETER,
                      thread->GetNativePc()};
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            frame->SetPrevFrame(reinterpret_cast<Frame *>(&bridge.v_[1]));
        }
        // Workaround for
        // issues #2888 and #2925
        // We cannot make OSR on the methods called from here, because:
        // 1. If caller is native method, then C2I bridge, created above, is not complete. It can be fixed by
        //    allocating full size boundary frame.
        // 2. If caller is compiled method, then we got here from entrypoint. But currently compiler creates
        //    boundary frame with pseudo LR value, that doesn't point to the instruction after call, thereby
        //    OSR will fail. It can be fixed by addresses patching, currently codegen hasn't such machinery.
        // TODO(msherstennikov): fix issue
        frame->DisableOsr();
        Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, this);
        // interpreter::Execute(thread, GetInstructions(), frame.get());
        InvokeHelper::InterpreterExecute(thread, GetInstructions(), frame.get());
        Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, this);
        thread->SetCurrentFrameIsCompiled(true);
    } else {
        Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, this);
        InvokeHelper::InterpreterExecute(thread, GetInstructions(), frame.get());
        // interpreter::Execute(thread, GetInstructions(), frame.get());
        Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, this);
    }
    thread->SetCurrentFrame(current_frame);

    ValueT res = (UNLIKELY(thread->HasPendingException()))
                     ? GetReturnValueFromException<InvokeHelper, ValueT>()
                     : GetReturnValueFromAcc<InvokeHelper, ValueT>(frame->GetAcc());
    LOG(DEBUG, INTERPRETER) << "Invoke exit: " << GetFullName();
    return res;
}

inline coretypes::TaggedValue Method::InvokeDyn(ManagedThread *thread, uint32_t num_args, coretypes::TaggedValue *args)
{
    return InvokeDyn<InvokeHelperDynamic>(thread, num_args, args);
}

template <class InvokeHelper>
inline coretypes::TaggedValue Method::InvokeDyn(ManagedThread *thread, uint32_t num_args, coretypes::TaggedValue *args)
{
    return InvokeImpl<InvokeHelper>(thread, num_args, args, false);
}

inline coretypes::TaggedValue Method::InvokeContext(ManagedThread *thread, const uint8_t *pc,
                                                    coretypes::TaggedValue acc, uint32_t nregs,
                                                    coretypes::TaggedValue *regs)
{
    return InvokeContext<InvokeHelperDynamic>(thread, pc, acc, nregs, regs);
}

template <class InvokeHelper>
inline coretypes::TaggedValue Method::InvokeContext(ManagedThread *thread, const uint8_t *pc,
                                                    coretypes::TaggedValue acc, uint32_t nregs,
                                                    coretypes::TaggedValue *regs)
{
    static_assert(InvokeHelper::is_dynamic == true);
    ASSERT(GetReturnType().GetId() == panda_file::Type::TypeId::VOID ||
           GetReturnType().GetId() == panda_file::Type::TypeId::TAGGED);

    TaggedValue res(TaggedValue::VALUE_UNDEFINED);

    if (!Verify()) {
        auto ctx = Runtime::GetCurrent()->GetLanguageContext(*this);
        panda::ThrowVerificationException(ctx, GetFullName());
        return res;
    }

    Frame *current_frame = thread->GetCurrentFrame();
    PandaUniquePtr<Frame, FrameDeleter> frame(
        interpreter::RuntimeInterface::CreateFrameWithActualArgs<true>(nregs, nregs, this,
                                                                                      current_frame),
        FrameDeleter(thread));
    if (UNLIKELY(frame.get() == nullptr)) {
        panda::ThrowOutOfMemoryError("CreateFrame failed: " + GetFullName());
        return res;
    }

    frame->SetDynamic();

    DynamicFrameHandler dynamic_frame_helper(frame.get());
    Span<TaggedValue> args_span(regs, nregs);
    for (size_t i = 0; i < nregs; ++i) {
        dynamic_frame_helper.GetVReg(i).SetValue(args_span[i].GetRawData());
    }

    LOG(DEBUG, INTERPRETER) << "Invoke entry: " << GetFullName();

    dynamic_frame_helper.GetAcc().SetValue(acc.GetRawData());
    thread->SetCurrentFrame(frame.get());

    Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, this);
    InvokeHelper::InterpreterExecute(thread, pc, frame.get());
    Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, this);

    thread->SetCurrentFrame(current_frame);
    res = TaggedValue(dynamic_frame_helper.GetAcc().GetAs<uint64_t>());

    LOG(DEBUG, INTERPRETER) << "Invoke exit: " << GetFullName();
    return res;
}

template <class InvokeHelper, class ValueT>
Frame *Method::EnterNativeMethodFrame(ManagedThread *thread, uint32_t num_vregs, uint32_t num_args, ValueT *args)
{
    Frame *current_frame = thread->GetCurrentFrame();
    PandaUniquePtr<Frame, FrameDeleter> frame =
        InitFrameWithNumVRegs<InvokeHelper, ValueT, true>(thread, num_vregs, num_args, args, current_frame);
    if (UNLIKELY(frame.get() == nullptr)) {
        panda::ThrowOutOfMemoryError("CreateFrame failed: " + GetFullName());
        return nullptr;
    }

    LOG(DEBUG, INTERPRETER) << "Enter native frame";

    thread->SetCurrentFrame(frame.get());
    return frame.release();
}

inline void Method::ExitNativeMethodFrame(ManagedThread *thread)
{
    Frame *current_frame = thread->GetCurrentFrame();
    ASSERT(current_frame != nullptr);

    LOG(DEBUG, INTERPRETER) << "Exit native frame";

    thread->SetCurrentFrame(current_frame->GetPrevFrame());
    FreeFrame(current_frame);
}

template <class InvokeHelper, class ValueT>
PandaUniquePtr<Frame, FrameDeleter> Method::InitFrame(ManagedThread *thread, uint32_t num_actual_args, ValueT *args,
                                                      Frame *current_frame)
{
    ASSERT(code_id_.IsValid());
    auto num_vregs = panda_file::CodeDataAccessor::GetNumVregs(*(panda_file_), code_id_);
    return InitFrameWithNumVRegs<InvokeHelper, ValueT, false>(thread, num_vregs, num_actual_args, args, current_frame);
}

template <class InvokeHelper, class ValueT, bool is_native_method>
PandaUniquePtr<Frame, FrameDeleter> Method::InitFrameWithNumVRegs(ManagedThread *thread, uint32_t num_vregs,
                                                                  uint32_t num_actual_args, ValueT *args,
                                                                  Frame *current_frame)
{
    Span<ValueT> args_span(args, num_actual_args);

    uint32_t num_declared_args = GetNumArgs();
    uint32_t frame_size = InvokeHelper::GetFrameSize(num_vregs, num_declared_args, num_actual_args);

    Frame *frame_ptr;
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (is_native_method) {
        frame_ptr = interpreter::RuntimeInterface::CreateNativeFrameWithActualArgs<InvokeHelper::is_dynamic>(
            frame_size, num_actual_args, this, current_frame);
    } else {  // NOLINTNEXTLINE(readability-braces-around-statements)
        frame_ptr = InvokeHelper::CreateFrame(thread, Frame::GetActualSize<InvokeHelper::is_dynamic>(frame_size), this,
                                              current_frame, frame_size, num_actual_args);
    }
    PandaUniquePtr<Frame, FrameDeleter> frame(frame_ptr, FrameDeleter(thread));
    if (UNLIKELY(frame.get() == nullptr)) {
        return frame;
    }

    InvokeHelper::InitActualArgs(frame.get(), args_span, num_vregs, num_declared_args);

    frame->SetInvoke();
    return frame;
}

template <class InvokeHelper, class ValueT>
ValueT Method::InvokeImpl(ManagedThread *thread, uint32_t num_actual_args, ValueT *args, bool proxy_call)
{
    IncrementHotnessCounter(0, nullptr);

    // Currently, proxy methods should always be invoked in the interpreter. This constraint should be relaxed once
    // we support same frame layout for interpreter and compiled methods.
    // TODO(msherstennikov): remove `proxy_call`
    bool run_interpreter = !HasCompiledCode() || proxy_call;
    ASSERT(!(proxy_call && IsNative()));
    if (!run_interpreter) {
        if constexpr (InvokeHelper::is_dynamic) {  // NOLINT(readability-braces-around-statements)
            return InvokeHelper::CompiledCodeExecute(thread, this, num_actual_args, args);
        } else {  // NOLINT(readability-misleading-indentation)
            return InvokeCompiledCode(thread, num_actual_args, args);
        }
    }
    if (!thread->template StackOverflowCheck<true, false>()) {
        return GetReturnValueFromException<InvokeHelper, ValueT>();
    }

    return InvokeInterpretedCode<InvokeHelper>(thread, num_actual_args, args);
}

template <class AccVRegisterPtrT>
inline void Method::SetAcc([[maybe_unused]] AccVRegisterPtrT acc)
{
    if constexpr (!std::is_same_v<AccVRegisterPtrT, std::nullptr_t>) {  // NOLINT
        if (acc != nullptr) {
            ManagedThread::GetCurrent()->GetCurrentFrame()->SetAcc(*acc);
        }
    }
}

/**
 * Increment method's hotness counter.
 * @param bytecode_offset Offset of the target bytecode instruction. Used only for OSR.
 * @param acc Pointer to the accumulator, it is needed because interpreter uses own Frame, not the one in the method.
 *            Used only for OSR.
 * @return true if OSR has been occurred
 */
template <class AccVRegisterPtrT>
inline bool Method::IncrementHotnessCounter(uintptr_t bytecode_offset, [[maybe_unused]] AccVRegisterPtrT acc, bool osr)
{
    bool is_jit_enabled = Runtime::GetCurrent()->IsJitEnabled();

    if (!is_jit_enabled) {
        // hotness_counter_ may be used as profiling information so it should be incremented even with disabled JIT:
        stor_16_pair_.hotness_counter_++;
        return false;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (!ArchTraits<RUNTIME_ARCH>::SUPPORT_OSR) {
        ASSERT(!osr);
    }

    auto runtime = Runtime::GetCurrent();
    auto &options = Runtime::GetOptions();
    uint32_t threshold = options.GetCompilerHotnessThreshold();
    if (UNLIKELY(GetHotnessCounter() >= threshold)) {
        CompilationStage status = GetCompilationStatus();
        if (!(status == FAILED || status == WAITING || status == COMPILATION)) {
            ASSERT((!osr) == (acc == nullptr));
            SetAcc<AccVRegisterPtrT>(acc);

            return runtime->GetPandaVM()->GetCompiler()->CompileMethod(this, bytecode_offset, osr);
        }
        if (status == WAITING) {
            IncrementHotnessCounter();
        }
    } else {
        if (GetHotnessCounter() == options.GetCompilerProfilingThreshold() && !HasCompiledCode() && !IsProfiling()) {
            StartProfiling();
        }
        IncrementHotnessCounter();
    }
    return false;
}

template <typename Callback>
void Method::EnumerateTypes(Callback handler) const
{
    panda_file::MethodDataAccessor mda(*(panda_file_), file_id_);
    mda.EnumerateTypesInProto(handler);
}

template <typename Callback>
void Method::EnumerateTryBlocks(Callback callback) const
{
    ASSERT(!IsAbstract());

    panda_file::MethodDataAccessor mda(*(panda_file_), file_id_);
    panda_file::CodeDataAccessor cda(*(panda_file_), mda.GetCodeId().value());

    cda.EnumerateTryBlocks(callback);
}

template <typename Callback>
void Method::EnumerateCatchBlocks(Callback callback) const
{
    ASSERT(!IsAbstract());

    using TryBlock = panda_file::CodeDataAccessor::TryBlock;
    using CatchBlock = panda_file::CodeDataAccessor::CatchBlock;

    EnumerateTryBlocks([&callback, code = GetInstructions()](const TryBlock &try_block) {
        bool next = true;
        const uint8_t *try_start_pc = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(code) +
                                                                  static_cast<uintptr_t>(try_block.GetStartPc()));
        const uint8_t *try_end_pc = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(try_start_pc) +
                                                                static_cast<uintptr_t>(try_block.GetLength()));
        // ugly, but API of TryBlock is bad designed: enumaration is paired with mutation & updating
        const_cast<TryBlock &>(try_block).EnumerateCatchBlocks(
            [&callback, &next, try_start_pc, try_end_pc](const CatchBlock &catch_block) {
                return next = callback(try_start_pc, try_end_pc, catch_block);
            });
        return next;
    });
}

template <typename Callback>
void Method::EnumerateExceptionHandlers(Callback callback) const
{
    ASSERT(!IsAbstract());

    using CatchBlock = panda_file::CodeDataAccessor::CatchBlock;

    EnumerateCatchBlocks([this, callback = std::move(callback)](const uint8_t *try_start_pc, const uint8_t *try_end_pc,
                                                                const CatchBlock &catch_block) {
        auto type_idx = catch_block.GetTypeIdx();
        const uint8_t *pc =
            &GetInstructions()[catch_block.GetHandlerPc()];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        size_t size = catch_block.GetCodeSize();
        const Class *cls = nullptr;
        if (type_idx != panda_file::INVALID_INDEX) {
            Runtime *runtime = Runtime::GetCurrent();
            auto type_id = GetClass()->ResolveClassIndex(type_idx);
            // todo: remove next code, after solving #1220 '[Runtime] Proposal for class descriptors in panda files'
            //       and clean up of ClassLinker API
            // cut
            LanguageContext ctx = runtime->GetLanguageContext(*this);
            cls = runtime->GetClassLinker()->GetExtension(ctx)->GetClass(*(panda_file_), type_id);
            // end cut
        }
        return callback(try_start_pc, try_end_pc, cls, pc, size);
    });
}

}  // namespace panda

#endif  // !PANDA_RUNTIME_METHOD_INL_H_
