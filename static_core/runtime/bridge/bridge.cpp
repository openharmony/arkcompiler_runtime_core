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

#include "runtime/bridge/bridge.h"

#include "libpandafile/bytecode_instruction-inl.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/interpreter/interpreter.h"
#include "bytecode_instruction.h"
#include "bytecode_instruction-inl.h"

namespace ark {

const void *GetCompiledCodeToInterpreterBridge(const Method *method)
{
    ASSERT(method != nullptr);
    const void *bridge = nullptr;

    if (method->GetClass() == nullptr) {
        bridge = reinterpret_cast<const void *>(CompiledCodeToInterpreterBridgeDyn);
    } else {
        if (ark::panda_file::IsDynamicLanguage(method->GetClass()->GetSourceLang())) {
            bridge = reinterpret_cast<const void *>(CompiledCodeToInterpreterBridgeDyn);
        } else {
            bridge = reinterpret_cast<const void *>(CompiledCodeToInterpreterBridge);
        }
    }
    return bridge;
}

const void *GetCompiledCodeToInterpreterBridge()
{
    return reinterpret_cast<const void *>(CompiledCodeToInterpreterBridge);
}

const void *GetCompiledCodeToInterpreterBridgeDyn()
{
    return reinterpret_cast<const void *>(CompiledCodeToInterpreterBridgeDyn);
}

template <class VRegRef>
static inline int64_t GetVRegValue(VRegRef reg)
{
    return reg.HasObject() ? static_cast<int64_t>(bit_cast<uintptr_t>(reg.GetReference())) : reg.GetLong();
}

/**
 * This function supposed to be called from the deoptimization code. It aims to call interpreter for given frame from
 * specific pc. Note, that it releases input interpreter's frame at the exit.
 */
extern "C" int64_t InvokeInterpreter(ManagedThread *thread, const uint8_t *pc, Frame *frame, Frame *lastFrame)
{
    bool prevFrameKind = thread->IsCurrentFrameCompiled();
    thread->SetCurrentFrame(frame);
    thread->SetCurrentFrameIsCompiled(false);
    LOG(DEBUG, INTEROP) << "InvokeInterpreter for method: " << frame->GetMethod()->GetFullName();

    interpreter::Execute(thread, pc, frame, thread->HasPendingException());

    int64_t res;
    auto acc = frame->GetAcc();
    if (frame->IsDynamic()) {
        res = GetVRegValue(acc.template AsVRegRef<true>());
    } else {
        res = GetVRegValue(acc.AsVRegRef());
    }

    auto prevFrame = frame->GetPrevFrame();
    thread->SetCurrentFrame(prevFrame);
    FreeFrame(frame);

    // We need to execute(find catch block) in all inlined methods. For this we use number of inlined method
    // Else we can execute previus interpreter frames and we will FreeFrames in incorrect order
    while (prevFrame != nullptr && lastFrame != frame) {
        ASSERT(!StackWalker::IsBoundaryFrame<FrameKind::INTERPRETER>(prevFrame));
        frame = prevFrame;
        LOG(DEBUG, INTEROP) << "InvokeInterpreter for method: " << frame->GetMethod()->GetFullName();
        prevFrame = frame->GetPrevFrame();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pc = frame->GetMethod()->GetInstructions() + frame->GetBytecodeOffset();
        if (!thread->HasPendingException()) {
            auto bcInst = BytecodeInstruction(pc);
            auto opcode = bcInst.GetOpcode();
            // Compiler splites InitObj to NewObject + CallStatic
            // if we have an deoptimization occurred in the CallStatic, we must not copy acc from CallStatic,
            // because acc contain result of the NewObject
            if (opcode != BytecodeInstruction::Opcode::INITOBJ_SHORT_V4_V4_ID16 &&
                opcode != BytecodeInstruction::Opcode::INITOBJ_V4_V4_V4_V4_ID16 &&
                opcode != BytecodeInstruction::Opcode::INITOBJ_RANGE_V8_ID16) {
                frame->GetAcc() = acc;
            }
            pc = bcInst.GetNext().GetAddress();
        } else {
            frame->GetAcc() = acc;
        }
        interpreter::Execute(thread, pc, frame, thread->HasPendingException());

        acc = frame->GetAcc();
        if (frame->IsDynamic()) {
            res = GetVRegValue(acc.template AsVRegRef<true>());
        } else {
            res = GetVRegValue(acc.AsVRegRef());
        }

        thread->SetCurrentFrame(prevFrame);
        FreeFrame(frame);
    }
    thread->SetCurrentFrameIsCompiled(prevFrameKind);

    return res;
}

const void *GetAbstractMethodStub()
{
    return reinterpret_cast<const void *>(AbstractMethodStub);
}

const void *GetDefaultConflictMethodStub()
{
    return reinterpret_cast<const void *>(DefaultConflictMethodStub);
}
}  // namespace ark
