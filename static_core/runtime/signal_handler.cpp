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

#include "signal_handler.h"
#include "utils/logger.h"
#include <algorithm>
#include <cstdlib>
#include "include/method.h"
#include "include/runtime.h"
#include "include/panda_vm.h"
#include <sys/ucontext.h>
#include "compiler_options.h"
#include "code_info/code_info.h"
#include "include/stack_walker.h"
#include "tooling/pt_thread_info.h"
#include "tooling/sampler/sampling_profiler.h"

#ifdef PANDA_TARGET_AMD64
extern "C" void StackOverflowExceptionEntrypointTrampoline();
#endif

namespace panda {

static bool IsValidStack([[maybe_unused]] const ManagedThread *thread)
{
    // #3649 CFrame::Initialize fires an ASSERT fail.
    // The issue is that ManagedStack is not always in a consistent state.
    // TODO(bulasevich): implement ManagedStack state check before the printout. For now, disable the output.
    return false;
}

// Something goes really wrong. Dump info and exit.
static void DumpStackTrace([[maybe_unused]] int signo, [[maybe_unused]] const siginfo_t *info,
                           [[maybe_unused]] const void *context)
{
    auto thread = ManagedThread::GetCurrent();
    if (thread == nullptr) {
        LOG(ERROR, RUNTIME) << "Native thread segmentation fault";
        return;
    }
    if (!IsValidStack(thread)) {
        return;
    }

    LOG(ERROR, RUNTIME) << "Managed thread segmentation fault";
    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        Method *method = stack.GetMethod();
        LOG(ERROR, RUNTIME) << method->GetClass()->GetName() << "." << method->GetName().data << " at "
                            << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
    }
}

static void UseDebuggerdSignalHandler(int sig)
{
    LOG(WARNING, RUNTIME) << "panda vm can not handle sig " << sig << ", call next handler";
}

static bool CallSignalActionHandler(int sig, siginfo_t *info, void *context)
{  // NOLINT
    return Runtime::GetCurrent()->GetSignalManager()->SignalActionHandler(sig, info, context);
}

bool SignalManager::SignalActionHandler(int sig, siginfo_t *info, void *context)
{
    panda::Logger::Sync();
    if (InOatCode(info, context, true)) {
        for (const auto &handler : oat_code_handler_) {
            if (handler->Action(sig, info, context)) {
                return true;
            }
        }
    }

    // a signal can not handle in oat
    if (InOtherCode(sig, info, context)) {
        return true;
    }

    // Use the default exception handler function.
    UseDebuggerdSignalHandler(sig);
    return false;
}

bool SignalManager::InOatCode([[maybe_unused]] const siginfo_t *siginfo, [[maybe_unused]] const void *context,
                              [[maybe_unused]] bool check_bytecode_pc) const
{
    // TODO(00510180) leak judge GetMethodAndReturnPcAndSp
    return true;
}

bool SignalManager::InOtherCode([[maybe_unused]] int sig, [[maybe_unused]] const siginfo_t *info,
                                [[maybe_unused]] const void *context) const
{
    return false;
}

void SignalManager::AddHandler(SignalHandler *handler, bool oat_code)
{
    if (oat_code) {
        oat_code_handler_.push_back(handler);
    } else {
        other_handlers_.push_back(handler);
    }
}

void SignalManager::RemoveHandler(SignalHandler *handler)
{
    auto it_oat = std::find(oat_code_handler_.begin(), oat_code_handler_.end(), handler);
    if (it_oat != oat_code_handler_.end()) {
        oat_code_handler_.erase(it_oat);
        return;
    }
    auto it_other = std::find(other_handlers_.begin(), other_handlers_.end(), handler);
    if (it_other != other_handlers_.end()) {
        other_handlers_.erase(it_other);
        return;
    }
    LOG(FATAL, RUNTIME) << "handler doesn't exist: " << handler;
}

void SignalManager::InitSignals()
{
    if (is_init_) {
        return;
    }

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGABRT);
    sigdelset(&mask, SIGBUS);
    sigdelset(&mask, SIGFPE);
    sigdelset(&mask, SIGILL);
    sigdelset(&mask, SIGSEGV);

    ClearSignalHooksHandlersArray();

    // if running in phone,Sigchain will work,AddSpecialSignalHandlerFn in sighook will not be used
    SigchainAction sigchain_action = {
        CallSignalActionHandler,
        mask,
        SA_SIGINFO,
    };
    AddSpecialSignalHandlerFn(SIGSEGV, &sigchain_action);

    is_init_ = true;

    for (auto tmp : oat_code_handler_) {
        allocator_->Delete(tmp);
    }
    oat_code_handler_.clear();
    for (auto tmp : other_handlers_) {
        allocator_->Delete(tmp);
    }
    other_handlers_.clear();
}

void SignalManager::GetMethodAndReturnPcAndSp([[maybe_unused]] const siginfo_t *siginfo,
                                              [[maybe_unused]] const void *context,
                                              [[maybe_unused]] const Method **out_method,
                                              [[maybe_unused]] const uintptr_t *out_return_pc,
                                              [[maybe_unused]] const uintptr_t *out_sp)
{
    // just stub now
}

void SignalManager::DeleteHandlersArray()
{
    if (is_init_) {
        for (auto tmp : oat_code_handler_) {
            allocator_->Delete(tmp);
        }
        oat_code_handler_.clear();
        for (auto tmp : other_handlers_) {
            allocator_->Delete(tmp);
        }
        other_handlers_.clear();
        RemoveSpecialSignalHandlerFn(SIGSEGV, CallSignalActionHandler);
        is_init_ = false;
    }
}

bool InAllocatedCodeRange(uintptr_t pc)
{
    Thread *thread = Thread::GetCurrent();
    if (thread == nullptr) {
        // Current thread is not attatched to any of the VMs
        return false;
    }

    if (Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->InAotFileRange(pc)) {
        return true;
    }

    auto heap_manager = thread->GetVM()->GetHeapManager();
    if (heap_manager == nullptr) {
        return false;
    }
    auto code_allocator = heap_manager->GetCodeAllocator();
    if (code_allocator == nullptr) {
        return false;
    }
    return code_allocator->InAllocatedCodeRange(ToVoidPtr(pc));
}

bool IsInvalidPointer(uintptr_t addr)
{
    if (addr == 0) {
        return true;
    }
    // address is at least 8-byte aligned
    uintptr_t mask = 0x7;
    return ((addr & mask) != 0);
}

// This is the way to get compiled method entry point
// FP regsiter -> stack -> method -> compilerEntryPoint
static uintptr_t FindCompilerEntrypoint(const uintptr_t *fp)
{
    // Compiled code stack frame:
    // +----------------+
    // | Return address |
    // +----------------+ <- Frame pointer
    // | Frame pointer  |
    // +----------------+
    // | panda::Method* |
    // +----------------+
    const int compiled_frame_method_offset = BoundaryFrame<FrameKind::COMPILER>::METHOD_OFFSET;

    if (IsInvalidPointer(reinterpret_cast<uintptr_t>(fp))) {
        return 0;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uintptr_t pmethod = fp[compiled_frame_method_offset];
    if (IsInvalidPointer(pmethod)) {
        return 0;
    }

    // it is important for GetCompiledEntryPoint method to have a lock-free implementation
    auto entrypoint = reinterpret_cast<uintptr_t>((reinterpret_cast<Method *>(pmethod))->GetCompiledEntryPoint());
    if (IsInvalidPointer(entrypoint)) {
        return 0;
    }

    if (!InAllocatedCodeRange(entrypoint)) {
        LOG(INFO, RUNTIME) << "Runtime SEGV handler: the entrypoint is not from JIT code";
        return 0;
    }

    if (!compiler::CodeInfo::VerifyCompiledEntry(entrypoint)) {
        // what we have found is not a compiled method
        return 0;
    }

    return entrypoint;
}

bool DetectSEGVFromCompiledCode(int sig, siginfo_t *siginfo, void *context)
{
    SignalContext signal_context(context);
    uintptr_t pc = signal_context.GetPC();
    LOG(DEBUG, RUNTIME) << "Handling SIGSEGV signal. PC:" << std::hex << pc;
    if (!InAllocatedCodeRange(pc)) {
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    return false;
}

bool DetectSEGVFromHandler(int sig, siginfo_t *siginfo, void *context)
{
    SignalContext signal_context(context);
    uintptr_t pc = signal_context.GetPC();
    auto func = ToUintPtr(&FindCompilerEntrypoint);
    const unsigned this_method_size_estimation = 0x1000;  // there is no way to find exact compiled method size
    if (func < pc && pc < func + this_method_size_estimation) {
        // We have got SEGV from the signal handler itself!
        // The links must have led us to wrong memory: FP regsiter -> stack -> method -> compilerEntryPoint
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    return false;
}

bool DetectSEGVFromMemory(int sig, siginfo_t *siginfo, void *context)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    auto mem_fault_location = ToUintPtr(siginfo->si_addr);
    const uintptr_t max_object_size = 1U << 30U;
    // the expected fault address is nullptr + offset within the object
    if (mem_fault_location > max_object_size) {
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    return false;
}

bool DetectSEGVFromCode(int sig, siginfo_t *siginfo, void *context)
{
    SignalContext signal_context(context);
    uintptr_t pc = signal_context.GetPC();
    uintptr_t entrypoint = FindCompilerEntrypoint(signal_context.GetFP());
    if (entrypoint == 0) {
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    compiler::CodeInfo codeinfo(compiler::CodeInfo::GetCodeOriginFromEntryPoint(ToVoidPtr(entrypoint)));

    auto code_size = codeinfo.GetCodeSize();
    if ((pc < entrypoint) || (pc > entrypoint + code_size)) {
        // we are not in a compiled method
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    return false;
}

void UpdateReturnAddress(SignalContext &signal_context, uintptr_t new_address)
{
#if (defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_ARM32))
    signal_context.SetLR(new_address);
#elif defined(PANDA_TARGET_AMD64)
    auto *sp = reinterpret_cast<uintptr_t *>(signal_context.GetSP() - sizeof(uintptr_t));
    *sp = new_address;
    signal_context.SetSP(reinterpret_cast<uintptr_t>(sp));
#endif
}

bool DetectSEGVFromNullCheck(int sig, siginfo_t *siginfo, void *context)
{
    SignalContext signal_context(context);
    uintptr_t pc = signal_context.GetPC();
    uintptr_t entrypoint = FindCompilerEntrypoint(signal_context.GetFP());
    compiler::CodeInfo codeinfo(compiler::CodeInfo::GetCodeOriginFromEntryPoint(ToVoidPtr(entrypoint)));
    uintptr_t new_pc = 0;
    for (auto icheck : codeinfo.GetImplicitNullChecksTable()) {
        uintptr_t null_check_addr = entrypoint + icheck.GetInstNativePc();
        auto offset = icheck.GetOffset();
        // We inserts information about implicit nullcheck after mem instruction,
        // because encoder can insert emory calculation before the instruction and we don't know real address:
        //   addr |               |
        //    |   +---------------+  <--- null_check_addr - offset
        //    |   | address calc  |
        //    |   | memory inst   |  <--- pc
        //    V   +---------------+  <--- null_check_addr
        //        |               |
        if (pc < null_check_addr && pc + offset >= null_check_addr) {
            new_pc = null_check_addr;
            break;
        }
    }

    if (new_pc == 0) {
        LOG(INFO, RUNTIME) << "SEGV can't be handled. No matching entry found in the NullCheck table.\n"
                           << "PC: " << std::hex << pc;
        for (auto icheck : codeinfo.GetImplicitNullChecksTable()) {
            LOG(INFO, RUNTIME) << "nullcheck: " << std::hex << (entrypoint + icheck.GetInstNativePc());
        }
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    LOG(DEBUG, RUNTIME) << "PC fixup: " << std::hex << new_pc;

    UpdateReturnAddress(signal_context, new_pc);
    signal_context.SetPC(reinterpret_cast<uintptr_t>(NullPointerExceptionBridge));
    EVENT_IMPLICIT_NULLCHECK(new_pc);

    return false;
}

void SamplerSigSegvHandler([[maybe_unused]] int sig, [[maybe_unused]] siginfo_t *siginfo,
                           [[maybe_unused]] void *context)
{
    auto mthread = ManagedThread::GetCurrent();
    ASSERT(mthread != nullptr);

    int num_to_return = 1;
    // NOLINTNEXTLINE(cert-err52-cpp)
    longjmp(mthread->GetPtThreadInfo()->GetSamplingInfo()->GetSigSegvJmpEnv(), num_to_return);
}

bool DetectSEGVFromSamplingProfilerHandler([[maybe_unused]] int sig, [[maybe_unused]] siginfo_t *siginfo,
                                           [[maybe_unused]] void *context)
{
    auto *mthread = ManagedThread::GetCurrent();
    ASSERT(mthread != nullptr);

    auto *sampler = Runtime::GetCurrent()->GetTools().GetSamplingProfiler();
    auto *sampling_info = mthread->GetPtThreadInfo()->GetSamplingInfo();
    if (sampler == nullptr || sampling_info == nullptr) {
        return false;
    }

    if (sampler->IsSegvHandlerEnable() && sampling_info->IsThreadSampling()) {
        SignalContext signal_context(context);
        signal_context.SetPC(reinterpret_cast<uintptr_t>(&SamplerSigSegvHandler));
        return true;
    }

    return false;
}

bool RuntimeSEGVHandler(int sig, siginfo_t *siginfo, void *context)
{
    if (DetectSEGVFromSamplingProfilerHandler(sig, siginfo, context)) {
        return true;
    }

    if (DetectSEGVFromCompiledCode(sig, siginfo, context)) {
        return false;
    }

    if (DetectSEGVFromHandler(sig, siginfo, context)) {
        return false;
    }

    if (DetectSEGVFromMemory(sig, siginfo, context)) {
        return false;
    }

    if (DetectSEGVFromCode(sig, siginfo, context)) {
        return false;
    }

    if (DetectSEGVFromNullCheck(sig, siginfo, context)) {
        return false;
    }

    return true;
}

bool NullPointerHandler::Action(int sig, siginfo_t *siginfo, void *context)
{
    if (sig != SIGSEGV) {
        return false;
    }
    if (!RuntimeSEGVHandler(sig, siginfo, context)) {
        return false;
    }
    LOG(DEBUG, RUNTIME) << "NullPointerHandler happen,Throw NullPointerHandler Exception, signal:" << sig;
    /* NullPointer has been check in aot or here now,then return to interpreter, so exception not build here
     * issue #1437
     * panda::ThrowNullPointerException();
     */
    return true;
}

NullPointerHandler::~NullPointerHandler() = default;

bool StackOverflowHandler::Action(int sig, [[maybe_unused]] siginfo_t *siginfo, [[maybe_unused]] void *context)
{
    if (sig != SIGSEGV) {
        return false;
    }
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    SignalContext signal_context(context);
    auto mem_check_location = signal_context.GetSP() - ManagedThread::GetStackOverflowCheckOffset();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    auto mem_fault_location = ToUintPtr(siginfo->si_addr);

    if (mem_check_location != mem_fault_location) {
        return false;
    }

    LOG(DEBUG, RUNTIME) << "Stack overflow occurred";

    // StackOverflow stackmap has zero address
    thread->SetNativePc(0);
    // Set compiler Frame in Thread
    thread->SetCurrentFrame(reinterpret_cast<Frame *>(signal_context.GetFP()));
#ifdef PANDA_TARGET_AMD64
    signal_context.SetPC(reinterpret_cast<uintptr_t>(StackOverflowExceptionEntrypointTrampoline));
#else
    /* To save/restore callee-saved regs we get into StackOverflowExceptionEntrypoint
     * by means of StackOverflowExceptionBridge.
     * The bridge stores LR to ManagedThread.npc, which is used by StackWalker::CreateCFrame,
     * and it must be 0 in case of StackOverflow.
     */
    signal_context.SetLR(0);
    signal_context.SetPC(reinterpret_cast<uintptr_t>(StackOverflowExceptionBridge));
#endif

    return true;
}
}  // namespace panda
