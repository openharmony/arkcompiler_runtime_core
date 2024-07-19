/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark {

static bool IsValidStack([[maybe_unused]] const ManagedThread *thread)
{
    // #3649 CFrame::Initialize fires an ASSERT fail.
    // The issue is that ManagedStack is not always in a consistent state.
    // NOTE(bulasevich): implement ManagedStack state check before the printout. For now, disable the output.
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
    ark::Logger::Sync();
    if (InOatCode(info, context, true)) {
        for (const auto &handler : oatCodeHandler_) {
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
                              [[maybe_unused]] bool checkBytecodePc) const
{
    // NOTE(00510180) leak judge GetMethodAndReturnPcAndSp
    return true;
}

bool SignalManager::InOtherCode([[maybe_unused]] int sig, [[maybe_unused]] const siginfo_t *info,
                                [[maybe_unused]] const void *context) const
{
    return false;
}

void SignalManager::AddHandler(SignalHandler *handler, bool oatCode)
{
    if (oatCode) {
        oatCodeHandler_.push_back(handler);
    } else {
        otherHandlers_.push_back(handler);
    }
}

void SignalManager::RemoveHandler(SignalHandler *handler)
{
    auto itOat = std::find(oatCodeHandler_.begin(), oatCodeHandler_.end(), handler);
    if (itOat != oatCodeHandler_.end()) {
        oatCodeHandler_.erase(itOat);
        return;
    }
    auto itOther = std::find(otherHandlers_.begin(), otherHandlers_.end(), handler);
    if (itOther != otherHandlers_.end()) {
        otherHandlers_.erase(itOther);
        return;
    }
    LOG(FATAL, RUNTIME) << "handler doesn't exist: " << handler;
}

void SignalManager::InitSignals()
{
    if (isInit_) {
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
    SigchainAction sigchainAction = {
        CallSignalActionHandler,
        mask,
        SA_SIGINFO,
    };
    AddSpecialSignalHandlerFn(SIGSEGV, &sigchainAction);

    isInit_ = true;

    for (auto tmp : oatCodeHandler_) {
        allocator_->Delete(tmp);
    }
    oatCodeHandler_.clear();
    for (auto tmp : otherHandlers_) {
        allocator_->Delete(tmp);
    }
    otherHandlers_.clear();
}

void SignalManager::GetMethodAndReturnPcAndSp([[maybe_unused]] const siginfo_t *siginfo,
                                              [[maybe_unused]] const void *context,
                                              [[maybe_unused]] const Method **outMethod,
                                              [[maybe_unused]] const uintptr_t *outReturnPc,
                                              [[maybe_unused]] const uintptr_t *outSp)
{
    // just stub now
}

void SignalManager::DeleteHandlersArray()
{
    if (isInit_) {
        for (auto tmp : oatCodeHandler_) {
            allocator_->Delete(tmp);
        }
        oatCodeHandler_.clear();
        for (auto tmp : otherHandlers_) {
            allocator_->Delete(tmp);
        }
        otherHandlers_.clear();
        RemoveSpecialSignalHandlerFn(SIGSEGV, CallSignalActionHandler);
        isInit_ = false;
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

    auto heapManager = thread->GetVM()->GetHeapManager();
    if (heapManager == nullptr) {
        return false;
    }
    auto codeAllocator = heapManager->GetCodeAllocator();
    if (codeAllocator == nullptr) {
        return false;
    }
    return codeAllocator->InAllocatedCodeRange(ToVoidPtr(pc));
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
    // | ark::Method* |
    // +----------------+
    const int compiledFrameMethodOffset = BoundaryFrame<FrameKind::COMPILER>::METHOD_OFFSET;

    if (IsInvalidPointer(reinterpret_cast<uintptr_t>(fp))) {
        return 0;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uintptr_t pmethod = fp[compiledFrameMethodOffset];
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
    SignalContext signalContext(context);
    uintptr_t pc = signalContext.GetPC();
    LOG(DEBUG, RUNTIME) << "Handling SIGSEGV signal. PC:" << std::hex << pc;
    if (!InAllocatedCodeRange(pc)) {
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    return false;
}

bool DetectSEGVFromHandler(int sig, siginfo_t *siginfo, void *context)
{
    SignalContext signalContext(context);
    uintptr_t pc = signalContext.GetPC();
    auto func = ToUintPtr(&FindCompilerEntrypoint);
    const unsigned thisMethodSizeEstimation = 0x1000;  // there is no way to find exact compiled method size
    if (func < pc && pc < func + thisMethodSizeEstimation) {
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
    auto memFaultLocation = ToUintPtr(siginfo->si_addr);
    const uintptr_t maxObjectSize = 1U << 30U;
    // the expected fault address is nullptr + offset within the object
    if (memFaultLocation > maxObjectSize) {
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    return false;
}

bool DetectSEGVFromCode(int sig, siginfo_t *siginfo, void *context)
{
    SignalContext signalContext(context);
    uintptr_t pc = signalContext.GetPC();
    uintptr_t entrypoint = FindCompilerEntrypoint(signalContext.GetFP());
    if (entrypoint == 0) {
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    compiler::CodeInfo codeinfo(compiler::CodeInfo::GetCodeOriginFromEntryPoint(ToVoidPtr(entrypoint)));

    auto codeSize = codeinfo.GetCodeSize();
    if ((pc < entrypoint) || (pc > entrypoint + codeSize)) {
        // we are not in a compiled method
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    return false;
}

void UpdateReturnAddress(SignalContext &signalContext, uintptr_t newAddress)
{
#if (defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_ARM32))
    signalContext.SetLR(newAddress);
#elif defined(PANDA_TARGET_AMD64)
    auto *sp = reinterpret_cast<uintptr_t *>(signalContext.GetSP() - sizeof(uintptr_t));
    *sp = newAddress;
    signalContext.SetSP(reinterpret_cast<uintptr_t>(sp));
#endif
}

bool DetectSEGVFromNullCheck(int sig, siginfo_t *siginfo, void *context)
{
    SignalContext signalContext(context);
    uintptr_t pc = signalContext.GetPC();
    uintptr_t entrypoint = FindCompilerEntrypoint(signalContext.GetFP());
    compiler::CodeInfo codeinfo(compiler::CodeInfo::GetCodeOriginFromEntryPoint(ToVoidPtr(entrypoint)));
    uintptr_t newPc = 0;
    static constexpr uint32_t LLVM_HANDLER_TAG = 1U << 31U;
    auto handlerPc = reinterpret_cast<uintptr_t>(NullPointerExceptionBridge);
    bool llvm = false;
    for (auto icheck : codeinfo.GetImplicitNullChecksTable()) {
        uintptr_t nullCheckAddr = entrypoint + icheck.GetInstNativePc();
        auto offset = icheck.GetOffset();
        if (pc == nullCheckAddr && (LLVM_HANDLER_TAG & offset) != 0) {
            // Code was compiled by LLVM.
            // We jump to the handler pc, which will call NullPointerException on its own.
            // We do not jump to NullPointerExceptionBridge directly here because LLVM code does not have stackmap on
            // memory instructions
            handlerPc = (~LLVM_HANDLER_TAG & offset) + entrypoint;
            newPc = nullCheckAddr;
            llvm = true;
            break;
        }
        // We insert information about implicit nullcheck after mem instruction,
        // because encoder can insert memory calculation before the instruction, and we don't know real address:
        //   addr |               |
        //    |   +---------------+  <--- nullCheckAddr - offset
        //    |   | address calc  |
        //    |   | memory inst   |  <--- pc
        //    V   +---------------+  <--- nullCheckAddr
        //        |               |
        if (pc < nullCheckAddr && pc + offset >= nullCheckAddr) {
            newPc = nullCheckAddr;
            break;
        }
    }

    if (newPc == 0) {
        LOG(INFO, RUNTIME) << "SEGV can't be handled. No matching entry found in the NullCheck table.\n"
                           << "PC: " << std::hex << pc;
        for (auto icheck : codeinfo.GetImplicitNullChecksTable()) {
            LOG(INFO, RUNTIME) << "nullcheck: " << std::hex << (entrypoint + icheck.GetInstNativePc());
        }
        DumpStackTrace(sig, siginfo, context);
        return true;
    }
    LOG(DEBUG, RUNTIME) << "PC fixup: " << std::hex << newPc;

    if (!llvm) {
        UpdateReturnAddress(signalContext, newPc);
    }
    signalContext.SetPC(handlerPc);
    EVENT_IMPLICIT_NULLCHECK(newPc);

    return false;
}

void SamplerSigSegvHandler([[maybe_unused]] int sig, [[maybe_unused]] siginfo_t *siginfo,
                           [[maybe_unused]] void *context)
{
    auto mthread = ManagedThread::GetCurrent();
    ASSERT(mthread != nullptr);

    int numToReturn = 1;
    // NOLINTNEXTLINE(cert-err52-cpp)
    longjmp(mthread->GetPtThreadInfo()->GetSamplingInfo()->GetSigSegvJmpEnv(), numToReturn);
}

bool DetectSEGVFromSamplingProfilerHandler([[maybe_unused]] int sig, [[maybe_unused]] siginfo_t *siginfo,
                                           [[maybe_unused]] void *context)
{
    auto *mthread = ManagedThread::GetCurrent();
    ASSERT(mthread != nullptr);

    auto *sampler = Runtime::GetCurrent()->GetTools().GetSamplingProfiler();
    auto *samplingInfo = mthread->GetPtThreadInfo()->GetSamplingInfo();
    if (sampler == nullptr || samplingInfo == nullptr) {
        return false;
    }

    if (sampler->IsSegvHandlerEnable() && samplingInfo->IsThreadSampling()) {
        SignalContext signalContext(context);
        signalContext.SetPC(reinterpret_cast<uintptr_t>(&SamplerSigSegvHandler));
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
     * ark::ThrowNullPointerException();
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
    SignalContext signalContext(context);
    auto memCheckLocation = signalContext.GetSP() - ManagedThread::GetStackOverflowCheckOffset();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    auto memFaultLocation = ToUintPtr(siginfo->si_addr);
    if (memCheckLocation != memFaultLocation) {
        return false;
    }

    LOG(DEBUG, RUNTIME) << "Stack overflow occurred";

    // StackOverflow stackmap has zero address
    thread->SetNativePc(0);
    // Set compiler Frame in Thread
    thread->SetCurrentFrame(reinterpret_cast<Frame *>(signalContext.GetFP()));
#ifdef PANDA_TARGET_AMD64
    signalContext.SetPC(reinterpret_cast<uintptr_t>(StackOverflowExceptionEntrypointTrampoline));
#else
    /* To save/restore callee-saved regs we get into StackOverflowExceptionEntrypoint
     * by means of StackOverflowExceptionBridge.
     * The bridge stores LR to ManagedThread.npc, which is used by StackWalker::CreateCFrame,
     * and it must be 0 in case of StackOverflow.
     */
    signalContext.SetLR(0);
    signalContext.SetPC(reinterpret_cast<uintptr_t>(StackOverflowExceptionBridge));
#endif

    return true;
}
}  // namespace ark
