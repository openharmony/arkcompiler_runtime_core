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

#include "runtime/deoptimization.h"

#include "libpandabase/events/events.h"
#include "libpandafile/file_items.h"
#include "runtime/include/locks.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/rendezvous.h"

namespace panda {

/**
 * \brief Deoptimize CFrame that lies after another CFrame.
 * @param thread       Pointer to thread
 * @param pc           PC from which interpreter starts execution
 * @param frame        Pointer to the first interpreter frame to that CFrame will be converted. It will be released via
 *                     FreeFrame inside this function
 * @param cframe_fp    Pointer to Cframe to be deoptimized
 * @param last_frame   Pointer to the last interpreter frame to that CFrame will be converted. It will be released via
 *                     FreeFrame inside this function
 * @param callee_regs  Pointer to a callee-saved registers buffer from StackWalker
 */
extern "C" [[noreturn]] void DeoptimizeAfterCFrame(ManagedThread *thread, const uint8_t *pc, Frame *frame,
                                                   void *cframe_fp, Frame *last_frame, void *callee_regs);
/**
 * \brief Deoptimize CFrame that lies after interpreter frame.
 * @param thread       Pointer to thread
 * @param pc           PC from which interpreter starts execution
 * @param frame        Pointer to the first interpreter frame to that CFrame will be converted. It will be released via
 *                     FreeFrame inside this function
 * @param cframe_fp    Pointer to Cframe to be deoptimized
 * @param last_frame   Pointer to the last interpreter frame to that CFrame will be converted. It will be released via
 *                     FreeFrame inside this function
 * @param callee_regs  Pointer to a callee-saved registers buffer from StackWalker
 */
extern "C" [[noreturn]] void DeoptimizeAfterIFrame(ManagedThread *thread, const uint8_t *pc, Frame *frame,
                                                   void *cframe_fp, Frame *last_frame, void *callee_regs);
/**
 * \brief Drop given CFrame and return to its caller.
 * Drop means that we set stack pointer to the top of given CFrame, restores its return address and invoke `return`
 * instruction.
 * @param cframe_fp    Pointer to Cframe to be dropped
 * @param callee_regs  Pointer to a callee-saved registers buffer from StackWalker
 */
extern "C" [[noreturn]] void DropCompiledFrameAndReturn(void *cframe_fp, void *callee_vregs);

static void UnpoisonAsanStack([[maybe_unused]] void *ptr)
{
#ifdef PANDA_ASAN_ON
    uint8_t sp;
    ASAN_UNPOISON_MEMORY_REGION(&sp, reinterpret_cast<uint8_t *>(ptr) - &sp);
#endif  // PANDA_ASAN_ON
}

// NO_THREAD_SAFETY_ANALYSIS because it doesn't know about mutator_lock status in this scope
void InvalidateCompiledEntryPoint(const PandaSet<Method *> &methods, bool is_cha) NO_THREAD_SAFETY_ANALYSIS
{
    PandaVM *vm = Thread::GetCurrent()->GetVM();
    ScopedSuspendAllThreadsRunning ssat(vm->GetRendezvous());
    // TODO(msherstennikov): remove this loop and check `methods` contains frame's method in stack traversing
    for (const auto &method : methods) {
#ifdef PANDA_EVENTS_ENABLED
        size_t in_stack_count = 0;
        vm->GetThreadManager()->EnumerateThreads([method, &in_stack_count, is_cha](ManagedThread *thread) {
#else
        vm->GetThreadManager()->EnumerateThreads([method, is_cha](ManagedThread *thread) {
#endif
            ASSERT(thread != nullptr);
            // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
            for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
                if (stack.IsCFrame() && stack.GetMethod() == method) {
                    auto &cframe = stack.GetCFrame();
                    cframe.SetShouldDeoptimize(true);
                    cframe.SetDeoptCodeEntry(stack.GetCompiledCodeEntry());
                    if (is_cha) {
                        LOG(DEBUG, CLASS_LINKER)
                            << "[CHA]   Set ShouldDeoptimize for method: " << cframe.GetMethod()->GetFullName();
                    } else {
                        LOG(DEBUG, CLASS_LINKER)
                            << "[IC]   Set ShouldDeoptimize for method: " << cframe.GetMethod()->GetFullName();
                    }
#ifdef PANDA_EVENTS_ENABLED
                    in_stack_count++;
#endif
                }
            }
            return true;
        });
        if (is_cha) {
            EVENT_CHA_DEOPTIMIZE(std::string(method->GetFullName()), in_stack_count);
        }
        // TODO (Trubenkov)  clean up compiled code(See issue 1706)
        method->SetInterpreterEntryPoint();
        Thread::GetCurrent()->GetVM()->GetCompiler()->RemoveOsrCode(method);
        method->SetCompilationStatus(Method::NOT_COMPILED);
    }
}

[[noreturn]] NO_ADDRESS_SANITIZE void Deoptimize(StackWalker *stack, const uint8_t *pc, bool has_exception,
                                                 bool destroy_compiled_code)
{
    ASSERT(stack != nullptr);
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    ASSERT(stack->IsCFrame());
    auto &cframe = stack->GetCFrame();
    UnpoisonAsanStack(cframe.GetFrameOrigin());

    LOG(DEBUG, INTEROP) << "Deoptimize frame: " << stack->GetMethod()->GetFullName();

    if (pc == nullptr) {
        ASSERT(cframe.GetMethod() != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pc = stack->GetMethod()->GetInstructions() + stack->GetBytecodePc();
    }

    // We must run InvalidateCompiledEntryPoint before we convert the frame, because GC is already may be in the
    // collecting phase and it can move some object in the deoptimized frame.
    if (destroy_compiled_code) {
        PandaSet<Method *> destroy_method;
        destroy_method.insert(cframe.GetMethod());
        InvalidateCompiledEntryPoint(destroy_method, false);
    }

    FrameKind prev_frame_kind;
    // We need to execute(find catch block) in all inlined methods. For this we calculate the number of inlined method
    // Else we can execute previus interpreter frames and we will FreeFrames in incorrect order
    uint32_t num_inlined_methods = 0;
    Frame *iframe = stack->ConvertToIFrame(&prev_frame_kind, &num_inlined_methods);
    ASSERT(iframe != nullptr);

    Frame *last_iframe = iframe;
    while (num_inlined_methods-- != 0) {
        EVENT_METHOD_EXIT(last_iframe->GetMethod()->GetFullName() + "(deopt)", events::MethodExitKind::INLINED,
                          thread->RecordMethodExit());
        last_iframe = last_iframe->GetPrevFrame();
        ASSERT(!StackWalker::IsBoundaryFrame<FrameKind::INTERPRETER>(last_iframe));
    }

    EVENT_METHOD_EXIT(last_iframe->GetMethod()->GetFullName() + "(deopt)", events::MethodExitKind::COMPILED,
                      thread->RecordMethodExit());

    if (thread->HasPendingException()) {
        LOG(DEBUG, INTEROP) << "Deoptimization has pending exception: "
                            << thread->GetException()->ClassAddr<Class>()->GetName();
        auto context = thread->GetVM()->GetLanguageContext();
        context.SetExceptionToVReg(iframe->GetAcc(), thread->GetException());
    }

    if (!has_exception) {
        thread->ClearException();
    } else {
        ASSERT(thread->HasPendingException());
    }

    switch (prev_frame_kind) {
        case FrameKind::COMPILER:
            LOG(DEBUG, INTEROP) << "Deoptimize after cframe";
            EVENT_DEOPTIMIZATION(std::string(cframe.GetMethod()->GetFullName()),
                                 pc - stack->GetMethod()->GetInstructions(), events::DeoptimizationAfter::CFRAME);
            // We need to set current frame kind to `compiled` as it's possible that we came here from other Deoptimize
            // call and in this case frame kind will be `non-compiled`:
            //
            //     compiled code
            //          |
            //      Deoptimize
            //          |
            // DeoptimizeAfterCFrame
            //          |
            // (change frame kind to `non-compiled`) interpreter::Execute -- we don't return after this call
            //          |
            // FindCatchBlockInCallStack
            //          |
            //      Deoptimize
            thread->SetCurrentFrameIsCompiled(true);
            DeoptimizeAfterCFrame(thread, pc, iframe, cframe.GetFrameOrigin(), last_iframe,
                                  stack->GetCalleeRegsForDeoptimize().end());
        case FrameKind::NONE:
        case FrameKind::INTERPRETER:
            EVENT_DEOPTIMIZATION(std::string(cframe.GetMethod()->GetFullName()),
                                 pc - stack->GetMethod()->GetInstructions(),
                                 prev_frame_kind == FrameKind::NONE ? events::DeoptimizationAfter::TOP
                                                                    : events::DeoptimizationAfter::IFRAME);
            LOG(DEBUG, INTEROP) << "Deoptimize after iframe";
            DeoptimizeAfterIFrame(thread, pc, iframe, cframe.GetFrameOrigin(), last_iframe,
                                  stack->GetCalleeRegsForDeoptimize().end());
    }
    UNREACHABLE();
}

[[noreturn]] void DropCompiledFrame(StackWalker *stack)
{
    LOG(DEBUG, INTEROP) << "Drop compiled frame: " << stack->GetMethod()->GetFullName();
    auto cframe_fp = stack->GetCFrame().GetFrameOrigin();
    EVENT_METHOD_EXIT(stack->GetMethod()->GetFullName() + "(drop)", events::MethodExitKind::COMPILED,
                      ManagedThread::GetCurrent()->RecordMethodExit());
    UnpoisonAsanStack(cframe_fp);
    DropCompiledFrameAndReturn(cframe_fp, stack->GetCalleeRegsForDeoptimize().end());
    UNREACHABLE();
}

}  // namespace panda
