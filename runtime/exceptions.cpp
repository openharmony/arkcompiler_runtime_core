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

#include "runtime/include/exceptions.h"

#include <libpandabase/utils/cframe_layout.h>

#include "runtime/bridge/bridge.h"
#include "runtime/deoptimization.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker.h"
#include "runtime/mem/vm_handle.h"
#include "libpandabase/utils/asan_interface.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "events/events.h"
#include "runtime/handle_base-inl.h"

namespace panda {

void ThrowException(const LanguageContext &ctx, ManagedThread *thread, const uint8_t *mutf8_name,
                    const uint8_t *mutf8_msg)
{
    ctx.ThrowException(thread, mutf8_name, mutf8_msg);
}

static LanguageContext GetLanguageContext(ManagedThread *thread)
{
    ASSERT(thread != nullptr);
    return thread->GetVM()->GetLanguageContext();
}

void ThrowNullPointerException()
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowNullPointerException(ctx, thread);
}

void ThrowNullPointerException(const LanguageContext &ctx, ManagedThread *thread)
{
    ThrowException(ctx, thread, ctx.GetNullPointerExceptionClassDescriptor(), nullptr);
}

void ThrowStackOverflowException(ManagedThread *thread)
{
    auto ctx = GetLanguageContext(thread);
    ctx.ThrowStackOverflowException(thread);
}

void ThrowArrayIndexOutOfBoundsException(coretypes::array_ssize_t idx, coretypes::array_size_t length)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowArrayIndexOutOfBoundsException(idx, length, ctx, thread);
}

void ThrowArrayIndexOutOfBoundsException(coretypes::array_ssize_t idx, coretypes::array_size_t length,
                                         const LanguageContext &ctx, ManagedThread *thread)
{
    PandaString msg;
    msg = "idx = " + ToPandaString(idx) + "; length = " + ToPandaString(length);

    ThrowException(ctx, thread, ctx.GetArrayIndexOutOfBoundsExceptionClassDescriptor(),
                   utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIndexOutOfBoundsException(coretypes::array_ssize_t idx, coretypes::array_ssize_t length)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    PandaString msg;
    msg = "idx = " + ToPandaString(idx) + "; length = " + ToPandaString(length);

    ThrowException(ctx, thread, ctx.GetIndexOutOfBoundsExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIllegalStateException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetIllegalStateExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowStringIndexOutOfBoundsException(coretypes::array_ssize_t idx, coretypes::array_size_t length)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    PandaString msg;
    msg = "idx = " + ToPandaString(idx) + "; length = " + ToPandaString(length);

    ThrowException(ctx, thread, ctx.GetStringIndexOutOfBoundsExceptionClassDescriptor(),
                   utf::CStringAsMutf8(msg.c_str()));
}

void ThrowNegativeArraySizeException(coretypes::array_ssize_t size)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    PandaString msg;
    msg = "size = " + ToPandaString(size);

    ThrowException(ctx, thread, ctx.GetNegativeArraySizeExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowNegativeArraySizeException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetNegativeArraySizeExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowArithmeticException()
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetArithmeticExceptionClassDescriptor(), utf::CStringAsMutf8("/ by zero"));
}

void ThrowClassCastException(const Class *dst_type, const Class *src_type)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    PandaString msg;
    msg = src_type->GetName() + " cannot be cast to " + dst_type->GetName();

    ThrowException(ctx, thread, ctx.GetClassCastExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowAbstractMethodError(const Method *method)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    PandaString msg;
    msg = "abstract method \"" + method->GetClass()->GetName() + ".";
    msg += utf::Mutf8AsCString(method->GetName().data);
    msg += "\"";

    ThrowException(ctx, thread, ctx.GetAbstractMethodErrorClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIncompatibleClassChangeErrorForMethodConflict(const Method *method)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    PandaString msg;
    msg = "Conflicting default method implementations \"" + method->GetClass()->GetName() + ".";
    msg += utf::Mutf8AsCString(method->GetName().data);
    msg += "\"";

    ThrowException(ctx, thread, ctx.GetIncompatibleClassChangeErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowArrayStoreException(const Class *array_class, const Class *element_class)
{
    PandaStringStream ss;
    ss << element_class->GetName() << " cannot be stored in an array of type " << array_class->GetName();
    ThrowArrayStoreException(ss.str());
}

void ThrowArrayStoreException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetArrayStoreExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowRuntimeException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetRuntimeExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIllegalArgumentException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetIllegalArgumentExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowClassCircularityError(const PandaString &class_name, const LanguageContext &ctx)
{
    auto *thread = ManagedThread::GetCurrent();
    PandaString msg = "Class or interface \"" + class_name + "\" is its own superclass or superinterface";
    ThrowException(ctx, thread, ctx.GetClassCircularityErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

/**
 * The function finds the corresponding catch block for the exception in the thread.
 * The function uses thread as an exception storage because:
 *  1. thread's exception is a GC root
 *  2. we cannot use Handl;eScope her ebecause the function is [[noreturn]]
 */
// NOLINTNEXTLINE(google-runtime-references)
NO_ADDRESS_SANITIZE void FindCatchBlockInCFrames(ManagedThread *thread, StackWalker *stack, Frame *orig_frame)
{
    auto next_frame = stack->GetNextFrame();
    for (; stack->HasFrame(); stack->NextFrame(), next_frame = stack->GetNextFrame()) {
        LOG(DEBUG, INTEROP) << "Search for the catch block in " << stack->GetMethod()->GetFullName();

        auto pc = stack->GetBytecodePc();
        auto *method = stack->GetMethod();
        ASSERT(method != nullptr);
        uint32_t pc_offset = method->FindCatchBlock(thread->GetException()->ClassAddr<Class>(), pc);

        if (pc_offset != panda_file::INVALID_OFFSET) {
            if (orig_frame != nullptr) {
                FreeFrame(orig_frame);
            }

            LOG(DEBUG, INTEROP) << "Catch block is found in " << stack->GetMethod()->GetFullName();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            Deoptimize(stack, method->GetInstructions() + pc_offset);
            UNREACHABLE();
        }

        thread->GetVM()->HandleReturnFrame();

        if (!next_frame.IsCFrame()) {
            if (orig_frame != nullptr) {
                FreeFrame(orig_frame);
            }
            thread->SetCurrentFrame(next_frame.GetIFrame());
            LOG(DEBUG, INTEROP) << "DropCompiledFrameAndReturn. Next frame isn't CFrame";
            DropCompiledFrame(stack);
            UNREACHABLE();
        }

        if (next_frame.GetCFrame().IsNative()) {
            if (orig_frame != nullptr) {
                FreeFrame(orig_frame);
            }
            LOG(DEBUG, INTEROP) << "DropCompiledFrameAndReturn. Next frame is NATIVE";
            DropCompiledFrame(stack);
            UNREACHABLE();
        }

        if (method->IsStaticConstructor()) {
            if (orig_frame != nullptr) {
                FreeFrame(orig_frame);
            }
            LOG(DEBUG, INTEROP) << "DropCompiledFrameAndReturn. Next frame is StaticConstructor";
            DropCompiledFrame(stack);
            UNREACHABLE();
        }

        if (!stack->IsInlined()) {
            auto prev = stack->GetCFrame().GetPrevFrame();
            if (stack->GetBoundaryFrameMethod<FrameKind::COMPILER>(prev) == FrameBridgeKind::BYPASS) {
                /**
                 * There is bypass bridge and current frame is not inlined, hence we are going to exit compiled
                 * function. Dynamic languages may do c2c call through runtime, so it's necessary to return to exit
                 * active function properly.
                 */
                if (orig_frame != nullptr) {
                    FreeFrame(orig_frame);
                }
                LOG(DEBUG, INTEROP) << "DropCompiledFrameAndReturn. Next frame is caller's cframe";
                DropCompiledFrame(stack);
                UNREACHABLE();
            }
        }
    }

    if (next_frame.IsValid()) {
        LOG(DEBUG, INTEROP) << "Exception " << thread->GetException()->ClassAddr<Class>()->GetName() << " isn't found";
        EVENT_METHOD_EXIT(stack->GetMethod()->GetFullName(), events::MethodExitKind::COMPILED,
                          thread->RecordMethodExit());
        thread->GetVM()->HandleReturnFrame();
        DropCompiledFrame(stack);
    }
    UNREACHABLE();
}

/**
 * The function finds the corresponding catch block for the exception in the thread.
 * The function uses thread as an exception storage because:
 *  1. thread's exception is a GC root
 *  2. we cannot use Handl;eScope her ebecause the function is [[noreturn]]
 */
NO_ADDRESS_SANITIZE void FindCatchBlockInCallStack(ManagedThread *thread)
{
    auto stack = StackWalker::Create(thread);
    auto orig_frame = stack.GetIFrame();
    ASSERT(!stack.IsCFrame());
    LOG(DEBUG, INTEROP) << "Enter in FindCatchBlockInCallStack for " << orig_frame->GetMethod()->GetFullName();
    // Exception will be handled in the Method::Invoke's caller
    if (orig_frame->IsInvoke()) {
        return;
    }

    stack.NextFrame();

    // NATIVE frames can handle exceptions as well
    if (!stack.HasFrame() || !stack.IsCFrame() || stack.GetCFrame().IsNative()) {
        return;
    }
    FindCatchBlockInCFrames(thread, &stack, orig_frame);
}

void ThrowFileNotFoundException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetFileNotFoundExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIOException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetIOExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIllegalAccessException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetIllegalAccessExceptionClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowOutOfMemoryError(ManagedThread *thread, const PandaString &msg)
{
    auto ctx = GetLanguageContext(thread);

    if (thread->IsThrowingOOM()) {
        thread->SetUsePreAllocObj(true);
    }

    thread->SetThrowingOOM(true);
    ThrowException(ctx, thread, ctx.GetOutOfMemoryErrorClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
    thread->SetThrowingOOM(false);
}

void ThrowOutOfMemoryError(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    ThrowOutOfMemoryError(thread, msg);
}

void ThrowUnsupportedOperationException()
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetUnsupportedOperationExceptionClassDescriptor(), nullptr);
}

void ThrowVerificationException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetVerifyErrorClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowVerificationException(const LanguageContext &ctx, const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();

    ThrowException(ctx, thread, ctx.GetVerifyErrorClassDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowInstantiationError(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetInstantiationErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowNoClassDefFoundError(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetNoClassDefFoundErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowTypedErrorDyn(const std::string &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetTypedErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowReferenceErrorDyn(const std::string &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    ThrowException(ctx, thread, ctx.GetReferenceErrorDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowIllegalMonitorStateException(const PandaString &msg)
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);

    ThrowException(ctx, thread, ctx.GetIllegalMonitorStateExceptionDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowCloneNotSupportedException()
{
    auto *thread = ManagedThread::GetCurrent();
    auto ctx = GetLanguageContext(thread);
    PandaString msg = "Class doesn't implement Cloneable";
    ThrowException(ctx, thread, ctx.GetCloneNotSupportedExceptionDescriptor(), utf::CStringAsMutf8(msg.c_str()));
}

}  // namespace panda
