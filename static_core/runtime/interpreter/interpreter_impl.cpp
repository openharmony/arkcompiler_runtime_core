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

#include "runtime/interpreter/interpreter_impl.h"

#include "libpandabase/macros.h"
#include "runtime/interpreter/interpreter-inl.h"
#include "runtime/interpreter/runtime_interface.h"
#include "irtoc_interpreter_utils.h"
#include "interpreter-inl_gen.h"

extern "C" void ExecuteImplFast(void *, void *, void *, void *);
extern "C" void ExecuteImplFastEH(void *, void *, void *, void *);
#ifdef PANDA_LLVMAOT
extern "C" void ExecuteImplFast_LLVM(void *, void *, void *, void *);    // NOLINT(readability-identifier-naming)
extern "C" void ExecuteImplFastEH_LLVM(void *, void *, void *, void *);  // NOLINT(readability-identifier-naming)
#endif

namespace panda::interpreter {

enum InterpreterType { CPP = 0, IRTOC, LLVM };

void ExecuteImpl(ManagedThread *thread, const uint8_t *pc, Frame *frame, bool jump_to_eh)
{
    const uint8_t *inst = frame->GetMethod()->GetInstructions();
    frame->SetInstruction(inst);
    InterpreterType interpreter_type = InterpreterType::CPP;
#if !defined(PANDA_TARGET_ARM32)  // Arm32 sticks to cpp - irtoc is not available there
    bool was_set = Runtime::GetOptions().WasSetInterpreterType();
    // Dynamic languages default is always cpp interpreter (unless option was set)
    if (!frame->IsDynamic() || was_set) {
        auto interpreter_type_str = Runtime::GetOptions().GetInterpreterType();
        if (interpreter_type_str == "llvm") {
#ifdef PANDA_LLVMAOT
            interpreter_type = InterpreterType::LLVM;
#else
            interpreter_type = was_set ? InterpreterType::LLVM : InterpreterType::IRTOC;
#endif
        } else if (interpreter_type_str == "irtoc") {
            interpreter_type = InterpreterType::IRTOC;
        } else {
            ASSERT(interpreter_type_str == "cpp");
        }
    }
    if (interpreter_type > InterpreterType::CPP) {
        if (Runtime::GetCurrent()->IsDebugMode()) {
            LOG(FATAL, RUNTIME) << "--debug-mode=true option is supported only with --interpreter-type=cpp";
            return;
        }
        if (frame->IsDynamic()) {
            auto gc_type = thread->GetVM()->GetGC()->GetType();
            if (gc_type != mem::GCType::G1_GC) {
                LOG(FATAL, RUNTIME) << "For dynamic languages, --gc-type=" << mem::GCStringFromType(gc_type)
                                    << " option is supported only with --interpreter-type=cpp";
                return;
            }
            if (Runtime::GetCurrent()->IsJitEnabled()) {
                LOG(INFO, RUNTIME) << "Dynamic types profiling disabled, use --interpreter-type=cpp to enable";
            }
        }
    }
#endif
    if (interpreter_type == InterpreterType::LLVM) {
#ifdef PANDA_LLVMAOT
        LOG(DEBUG, RUNTIME) << "Setting up LLVM Irtoc dispatch table";
        auto dispath_table = SetupLLVMDispatchTableImpl();
        if (jump_to_eh) {
            ExecuteImplFastEH_LLVM(thread, const_cast<uint8_t *>(pc), frame, dispath_table);
        } else {
            ExecuteImplFast_LLVM(thread, const_cast<uint8_t *>(pc), frame, dispath_table);
        }
#else
        LOG(FATAL, RUNTIME) << "--interpreter-type=llvm is not supported in this configuration";
#endif
    } else if (interpreter_type == InterpreterType::IRTOC) {
        LOG(DEBUG, RUNTIME) << "Setting up Irtoc dispatch table";
        auto dispath_table = SetupDispatchTableImpl();
        if (jump_to_eh) {
            ExecuteImplFastEH(thread, const_cast<uint8_t *>(pc), frame, dispath_table);
        } else {
            ExecuteImplFast(thread, const_cast<uint8_t *>(pc), frame, dispath_table);
        }
    } else {
        if (frame->IsDynamic()) {
            if (thread->GetVM()->IsBytecodeProfilingEnabled()) {
                ExecuteImplInner<RuntimeInterface, true, true>(thread, pc, frame, jump_to_eh);
            } else {
                ExecuteImplInner<RuntimeInterface, true, false>(thread, pc, frame, jump_to_eh);
            }
        } else {
            ExecuteImplInner<RuntimeInterface, false>(thread, pc, frame, jump_to_eh);
        }
    }
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
    auto frame_handler = GetFrameHandler(frame);
    for (size_t i = 0; i < frame->GetSize(); ++i) {
        std::cerr << pad << "v" << i << "." << frame_handler.GetVReg(i).DumpVReg() << std::endl;
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

}  // namespace panda::interpreter
