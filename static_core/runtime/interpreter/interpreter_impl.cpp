/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "runtime/interpreter/interpreter_impl.h"

#include "libarkbase/macros.h"
#include "runtime/interpreter/interpreter-inl.h"
#include "runtime/interpreter/runtime_interface.h"
#ifdef PANDA_WITH_IRTOC
#include "irtoc_interpreter_utils.h"
#endif
#include "interpreter-inl_gen.h"

// Only be generated and used when irtoc is enabled
#ifdef PANDA_WITH_IRTOC
extern "C" void ExecuteImplFast(void *, void *, void *, void *);
extern "C" void ExecuteImplFastEH(void *, void *, void *, void *);
#endif
#ifdef PANDA_LLVM_INTERPRETER
extern "C" void ExecuteImplFast_LLVM(void *, void *, void *, void *);    // NOLINT(readability-identifier-naming)
extern "C" void ExecuteImplFastEH_LLVM(void *, void *, void *, void *);  // NOLINT(readability-identifier-naming)
#endif

extern "C" void DebugSwitchFromIrtocEntrypoint(ark::ManagedThread *thread, const uint8_t *pc, ark::Frame *frame)
{
    if (frame->IsDynamic()) {
        ark::interpreter::ExecuteImplDebug<ark::interpreter::RuntimeInterface, true>(thread, pc, frame, false);
    } else {
        ark::interpreter::ExecuteImplDebug<ark::interpreter::RuntimeInterface, false>(thread, pc, frame, false);
    }
}

namespace ark::interpreter {

enum InterpreterType { CPP = 0, IRTOC, LLVM };

#if !defined(PANDA_TARGET_ARM32)
namespace {
InterpreterType GetInterpreterTypeFromRuntimeOptions(Frame *frame)
{
    auto interpreterType = InterpreterType::CPP;
    bool wasSet = Runtime::GetOptions().WasSetInterpreterType();
    // Dynamic languages default is always cpp interpreter (unless option was set)
    if (!frame->IsDynamic() || wasSet) {
        auto interpreterTypeStr = Runtime::GetOptions().GetInterpreterType();
        if (interpreterTypeStr == "llvm") {
            interpreterType = InterpreterType::LLVM;
        } else if (interpreterTypeStr == "irtoc") {
            interpreterType = InterpreterType::IRTOC;
        } else {
            ASSERT(interpreterTypeStr == "cpp");
        }
        if (!wasSet) {
#ifndef PANDA_LLVM_INTERPRETER
            if (interpreterType == InterpreterType::LLVM) {
                interpreterType = InterpreterType::IRTOC;
            }
#endif
#ifndef PANDA_WITH_IRTOC
            if (interpreterType == InterpreterType::IRTOC) {
                interpreterType = InterpreterType::CPP;
            }
#endif
#if defined(USE_HWASAN)
            if (interpreterType != InterpreterType::CPP) {
                LOG(INFO, RUNTIME) << "interpreter type is downgraded into CPP due to HWASAN config";
                interpreterType = InterpreterType::CPP;
            }
#endif
#if defined(PANDA_SDK_LINUX_X64_FORCE_CPP_INTERPRETER)
            if (interpreterType != InterpreterType::CPP) {
                LOG(INFO, RUNTIME) << "interpreter type is downgraded into CPP for Linux x86_64 SDK runtime default";
                interpreterType = InterpreterType::CPP;
            }
#endif
        }
    }
    return interpreterType;
}
}  // namespace
#endif  // #if !defined(PANDA_TARGET_ARM32)

void ExecuteImplType(InterpreterType interpreterType, ManagedThread *thread, const uint8_t *pc, Frame *frame,
                     bool jumpToEh)
{
    const void *const *prevDispatchTable = thread->GetCurrentDispatchTable<false>();
    const void *const *prevDebugDispatchTable = thread->GetDebugDispatchTable();
    if (interpreterType == InterpreterType::LLVM) {
#ifdef PANDA_LLVM_INTERPRETER
        LOG(DEBUG, RUNTIME) << "Setting up LLVM Irtoc dispatch table";
        ASSERT(!Runtime::GetCurrent()->IsDebugMode() && "Debug mode execution must be done in CPP interpreter");
        auto dispatchTable = SetupLLVMDispatchTableImpl();
        thread->SetCurrentDispatchTable<false>(static_cast<const void *const *>(dispatchTable));
        auto *debugDispatchTable = SetupLLVMDebugDispatchTableImpl();
        thread->SetDebugDispatchTable(static_cast<const void *const *>(debugDispatchTable));
        if (jumpToEh) {
            ExecuteImplFastEH_LLVM(thread, const_cast<uint8_t *>(pc), frame, dispatchTable);
        } else {
            ExecuteImplFast_LLVM(thread, const_cast<uint8_t *>(pc), frame, dispatchTable);
        }
#else
        LOG(FATAL, RUNTIME) << "--interpreter-type=llvm is not supported in this configuration";
#endif
    } else if (interpreterType == InterpreterType::IRTOC) {
#ifdef PANDA_WITH_IRTOC
        LOG(DEBUG, RUNTIME) << "Setting up Irtoc dispatch table";
        ASSERT(!Runtime::GetCurrent()->IsDebugMode() && "Debug mode execution must be done in CPP interpreter");
        auto dispatchTable = SetupDispatchTableImpl();
        thread->SetCurrentDispatchTable<false>(static_cast<const void *const *>(dispatchTable));
        auto *debugDispatchTable = SetupDebugDispatchTableImpl();
        thread->SetDebugDispatchTable(static_cast<const void *const *>(debugDispatchTable));
        if (jumpToEh) {
            ExecuteImplFastEH(thread, const_cast<uint8_t *>(pc), frame, dispatchTable);
        } else {
            ExecuteImplFast(thread, const_cast<uint8_t *>(pc), frame, dispatchTable);
        }
#else
        LOG(FATAL, RUNTIME) << "--interpreter-type=irtoc is not supported in this configuration";
#endif
    } else {
        if (frame->IsDynamic()) {
            if (thread->GetVM()->IsBytecodeProfilingEnabled()) {
                ExecuteImplInner<RuntimeInterface, true, true>(thread, pc, frame, jumpToEh);
            } else {
                ExecuteImplInner<RuntimeInterface, true, false>(thread, pc, frame, jumpToEh);
            }
        } else {
            ExecuteImplInner<RuntimeInterface, false>(thread, pc, frame, jumpToEh);
        }
    }
    // Handle switch to debug mode which could happen during method invocation
    thread->SetDebugDispatchTable(prevDebugDispatchTable);
    thread->SetCurrentDispatchTable<false>(Runtime::GetCurrent()->IsDebugMode() ? prevDebugDispatchTable
                                                                                : prevDispatchTable);
}

void ExecuteImpl(ManagedThread *thread, const uint8_t *pc, Frame *frame, bool jumpToEh)
{
    const uint8_t *inst = frame->GetMethod()->GetInstructions();
    frame->SetInstruction(inst);
    InterpreterType interpreterType = InterpreterType::CPP;
#if !defined(PANDA_TARGET_ARM32)  // Arm32 sticks to cpp - irtoc is not available there
    interpreterType = GetInterpreterTypeFromRuntimeOptions(frame);
    if (interpreterType > InterpreterType::CPP) {
        if (Runtime::GetCurrent()->IsDebugMode()) {
            // Debug mode was enabled at runtime.
            // Fall back to the CPP debug interpreter which supports all debug
            // events (BytecodePcChanged, MethodEntry/Exit, ExceptionThrow/Catch).
            interpreterType = InterpreterType::CPP;
        }
        auto gcType = thread->GetVM()->GetGC()->GetType();
#if defined(ARK_USE_COMMON_RUNTIME)
        if (gcType != mem::GCType::CMC_GC) {
            LOG(FATAL, RUNTIME) << "--gc-type=" << mem::GCStringFromType(gcType) << " option is supported only with "
                                << "--interpreter-type=cpp. Use --gc-type=cmc-gc instead.";
            return;
        }
#else
        if (frame->IsDynamic()) {
            if (gcType != mem::GCType::G1_GC) {
                LOG(FATAL, RUNTIME) << "For dynamic languages, --gc-type=" << mem::GCStringFromType(gcType)
                                    << " option is supported only with --interpreter-type=cpp";
                return;
            }
            if (Runtime::GetCurrent()->IsProfilerEnabled()) {
                LOG(INFO, RUNTIME) << "Dynamic types profiling disabled, use --interpreter-type=cpp to enable";
            }
        }
#endif
    }
#if defined(ARK_USE_COMMON_RUNTIME)
    // CMC GC will be supported with LLVM interpreter with issue27125
    if (interpreterType == InterpreterType::LLVM) {
        LOG(FATAL, RUNTIME) << "CMC GC is only supported to be set with --interpreter-type=cpp or irtoc";
        return;
    }
#endif  // ARK_USE_COMMON_RUNTIME
#endif  // #if !defined(PANDA_TARGET_ARM32)
    ExecuteImplType(interpreterType, thread, pc, frame, jumpToEh);
}

// Methods for debugging

template <class RuntimeIfaceT, bool IS_DYNAMIC>
void InstructionHandlerBase<RuntimeIfaceT, IS_DYNAMIC>::DebugDump()
{
#ifndef NDEBUG
    auto frame = GetFrame();
    auto method = frame->GetMethod();
    PandaString pad = "     ";
    std::cerr << "Method " << method->GetFullName(true) << std::endl;
    std::cerr << pad << "nargs = " << method->GetNumArgs() << std::endl;
    std::cerr << pad << "nregs = " << method->GetNumVregs() << std::endl;
    std::cerr << pad << "total frame size = " << frame->GetSize() << std::endl;
    std::cerr << "Frame:" << std::endl;
    std::cerr << pad << "acc." << GetAccAsVReg<IS_DYNAMIC>().DumpVReg() << std::endl;
    auto frameHandler = GetFrameHandler(frame);
    for (size_t i = 0; i < frame->GetSize(); ++i) {
        std::cerr << pad << "v" << i << "." << frameHandler.GetVReg(i).DumpVReg() << std::endl;
    }
    std::cerr << "Bytecode:" << std::endl;
    size_t offset = 0;
    BytecodeInstruction inst(method->GetInstructions());
    while (offset < method->GetCodeSize()) {
        if (inst.GetAddress() == GetInst().GetAddress()) {
            std::cerr << "  -> ";
        } else {
            std::cerr << "     ";
        }

        std::cerr << std::hex << std::setw(sizeof(uintptr_t)) << std::setfill('0')
                  << reinterpret_cast<uintptr_t>(inst.GetAddress()) << std::dec << ": " << inst << std::endl;
        offset += inst.GetSize();
        inst = inst.GetNext();
    }
#endif  // NDEBUG
}

void EnsureDebugMethodsInstantiation(void *handler)
{
    static_cast<InstructionHandlerBase<RuntimeInterface, false> *>(handler)->DebugDump();
    static_cast<InstructionHandlerBase<RuntimeInterface, true> *>(handler)->DebugDump();
}

void SwitchToDebugMode(ManagedThread *thread)
{
    ASSERT(thread != nullptr);
    ASSERT(thread->IsSuspended());

    auto *debugDT = thread->GetDebugDispatchTable();
    if (debugDT != nullptr) {
        thread->SetCurrentDispatchTable<false>(debugDT);
    }
}

}  // namespace ark::interpreter
