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

#ifndef PANDA_INTERPRETER_INSTRUCTION_HANDLER_BASE_H_
#define PANDA_INTERPRETER_INSTRUCTION_HANDLER_BASE_H_

#include <isa_constants_gen.h>
#include "runtime/include/method.h"
#include "runtime/interpreter/instruction_handler_state.h"

namespace panda::interpreter {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INST()                                                                           \
    LOG(DEBUG, INTERPRETER) << std::hex << std::setw(sizeof(uintptr_t)) << std::setfill('0') \
                            << reinterpret_cast<uintptr_t>(this->GetInst().GetAddress()) << std::dec << ": "

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
#include "arch/global_regs.h"

class StaticFrameHandlerT : public StaticFrameHandler {
public:
    ALWAYS_INLINE inline explicit StaticFrameHandlerT(Frame *frame) : StaticFrameHandler(frame) {}

private:
    ALWAYS_INLINE inline interpreter::VRegister *GetVRegisters()
    {
        return reinterpret_cast<interpreter::VRegister *>(arch::regs::GetFp());
    }

    ALWAYS_INLINE inline interpreter::VRegister *GetMirrorVRegisters()
    {
        return reinterpret_cast<interpreter::VRegister *>(arch::regs::GetMirrorFp());
    }
};

class DynamicFrameHandlerT : public DynamicFrameHandler {
public:
    ALWAYS_INLINE inline explicit DynamicFrameHandlerT(Frame *frame) : DynamicFrameHandler(frame) {}

private:
    ALWAYS_INLINE inline interpreter::VRegister *GetVRegisters()
    {
        return reinterpret_cast<interpreter::VRegister *>(arch::regs::GetFp());
    }
};

#else

using StaticFrameHandlerT = StaticFrameHandler;
using DynamicFrameHandlerT = DynamicFrameHandler;

#endif  // PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES

template <class RuntimeIfaceT, bool is_dynamic>
class InstructionHandlerBase {
public:
    ALWAYS_INLINE explicit InstructionHandlerBase(InstructionHandlerState *state) : state_(state) {}

    ALWAYS_INLINE uint16_t GetExceptionOpcode() const
    {
        // Need to call GetInst().GetOpcode() in this case too, otherwise compiler can generate non optimal code
        return static_cast<unsigned>(GetInst().GetPrimaryOpcode()) + state_->GetOpcodeExtension();
    }

    ALWAYS_INLINE uint8_t GetPrimaryOpcode() const
    {
        return static_cast<unsigned>(GetInst().GetPrimaryOpcode());
    }

    ALWAYS_INLINE uint8_t GetSecondaryOpcode() const
    {
        return static_cast<unsigned>(GetInst().GetSecondaryOpcode());
    }

    ALWAYS_INLINE bool IsPrimaryOpcodeValid() const
    {
        return GetInst().IsPrimaryOpcodeValid();
    }

    void DumpVRegs()
    {
#if PANDA_ENABLE_SLOW_DEBUG
        // Skip dump if logger is disable. This allows us to speed up interpretation in the 'Debug' build.
        if (!Logger::IsLoggingOn(Logger::Level::DEBUG, Logger::Component::INTERPRETER)) {
            return;
        }

        static constexpr uint64_t STANDARD_DEBUG_INDENT = 5;
        LOG(DEBUG, INTERPRETER) << PandaString(STANDARD_DEBUG_INDENT, ' ') << "acc."
                                << GetAccAsVReg<is_dynamic>().DumpVReg();
        auto frame_handler = GetFrameHandler();
        for (size_t i = 0; i < GetFrame()->GetSize(); ++i) {
            LOG(DEBUG, INTERPRETER) << PandaString(STANDARD_DEBUG_INDENT, ' ') << "v" << i << "."
                                    << frame_handler.GetVReg(i).DumpVReg();
        }
#endif
    }

    ALWAYS_INLINE uint32_t UpdateBytecodeOffset()
    {
        auto pc = GetBytecodeOffset();
        GetFrame()->SetBytecodeOffset(pc);
        return pc;
    }

    void InstrumentInstruction()
    {
        // Should set ACC to Frame, so that ACC will be marked when GC
        GetFrame()->SetAcc(GetAcc());

        auto pc = UpdateBytecodeOffset();
        RuntimeIfaceT::GetNotificationManager()->BytecodePcChangedEvent(GetThread(), GetFrame()->GetMethod(), pc);

        // BytecodePcChangedEvent hook can call the GC, so we need to update the ACC
        GetAcc() = GetFrame()->GetAcc();
    }

    void InstrumentForceReturn()
    {
        interpreter::AccVRegister result;  // empty result, because force exit
        GetAcc() = result;
        GetFrame()->GetAcc() = result;
    }

    ALWAYS_INLINE const AccVRegisterT &GetAcc() const
    {
        return state_->GetAcc();
    }

    ALWAYS_INLINE AccVRegisterT &GetAcc()
    {
        return state_->GetAcc();
    }

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
    template <bool is_dynamic_t = is_dynamic>
    ALWAYS_INLINE AccVRegisterTRef<is_dynamic_t> GetAccAsVReg()
    {
        return AccVRegisterTRef<is_dynamic_t>(&state_->GetAcc());
    }
#else
    template <bool is_dynamic_t = is_dynamic>
    ALWAYS_INLINE typename std::enable_if<is_dynamic_t, DynamicVRegisterRef>::type GetAccAsVReg()
    {
        return state_->GetAcc().template AsVRegRef<is_dynamic_t>();
    }

    template <bool is_dynamic_t = is_dynamic>
    ALWAYS_INLINE typename std::enable_if<!is_dynamic_t, StaticVRegisterRef>::type GetAccAsVReg()
    {
        return state_->GetAcc().template AsVRegRef<is_dynamic_t>();
    }
#endif  // PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES

    ALWAYS_INLINE BytecodeInstruction GetInst() const
    {
        return state_->GetInst();
    }

    void DebugDump();

    template <bool is_dynamic_t = is_dynamic>
    ALWAYS_INLINE typename std::enable_if<is_dynamic_t, DynamicFrameHandlerT>::type GetFrameHandler()
    {
        return DynamicFrameHandlerT(state_->GetFrame());
    }

    template <bool is_dynamic_t = is_dynamic>
    ALWAYS_INLINE typename std::enable_if<!is_dynamic_t, StaticFrameHandlerT>::type GetFrameHandler()
    {
        return StaticFrameHandlerT(state_->GetFrame());
    }

    template <bool is_dynamic_t = is_dynamic>
    ALWAYS_INLINE typename std::enable_if<is_dynamic_t, DynamicFrameHandler>::type GetFrameHandler(Frame *frame)
    {
        return DynamicFrameHandler(frame);
    }

    template <bool is_dynamic_t = is_dynamic>
    ALWAYS_INLINE typename std::enable_if<!is_dynamic_t, StaticFrameHandler>::type GetFrameHandler(Frame *frame)
    {
        return StaticFrameHandler(frame);
    }

    ALWAYS_INLINE Frame *GetFrame() const
    {
        return state_->GetFrame();
    }

    ALWAYS_INLINE void SetFrame(Frame *frame)
    {
        state_->SetFrame(frame);
    }

    ALWAYS_INLINE ManagedThread *GetThread() const
    {
        return state_->GetThread();
    }

protected:
    template <BytecodeInstruction::Format format, bool can_throw>
    ALWAYS_INLINE void MoveToNextInst()
    {
        SetInst(GetInst().template GetNext<format>());

        if (can_throw) {
            SetOpcodeExtension(0);
        }
    }

    template <bool can_throw>
    ALWAYS_INLINE void JumpToInst(int32_t offset)
    {
        SetInst(GetInst().JumpTo(offset));

        if (can_throw) {
            SetOpcodeExtension(0);
        }
    }

    template <bool can_throw>
    ALWAYS_INLINE void JumpTo(const uint8_t *pc)
    {
        SetInst(BytecodeInstruction(pc));

        if (can_throw) {
            SetOpcodeExtension(0);
        }
    }

    ALWAYS_INLINE void MoveToExceptionHandler()
    {
        SetOpcodeExtension(UINT8_MAX + NUM_PREFIXED + 1);
        SetOpcodeExtension(GetOpcodeExtension() - GetPrimaryOpcode());
    }

    ALWAYS_INLINE void SetThread(ManagedThread *thread)
    {
        state_->SetThread(thread);
    }

    ALWAYS_INLINE void SetInst(BytecodeInstruction inst)
    {
        state_->SetInst(inst);
    }

    ALWAYS_INLINE const void *const *GetDispatchTable() const
    {
        return state_->GetDispatchTable();
    }

    ALWAYS_INLINE void SetDispatchTable(const void *const *dispatch_table)
    {
        return state_->SetDispatchTable(dispatch_table);
    }

    ALWAYS_INLINE void SaveState()
    {
        state_->SaveState();
    }

    ALWAYS_INLINE void RestoreState()
    {
        state_->RestoreState();
    }

    ALWAYS_INLINE uint16_t GetOpcodeExtension() const
    {
        return state_->GetOpcodeExtension();
    }

    ALWAYS_INLINE void SetOpcodeExtension(uint16_t opcode_extension)
    {
        state_->SetOpcodeExtension(opcode_extension);
    }

    ALWAYS_INLINE auto &GetFakeInstBuf()
    {
        return state_->GetFakeInstBuf();
    }

    ALWAYS_INLINE void UpdateHotness(Method *method)
    {
        method->IncrementHotnessCounter(0, nullptr);
    }

    ALWAYS_INLINE uint32_t GetBytecodeOffset() const
    {
        return state_->GetBytecodeOffset();
    }

    ALWAYS_INLINE InstructionHandlerState *GetInstructionHandlerState()
    {
        return state_;
    }

    template <bool taken>
    ALWAYS_INLINE void UpdateBranchStatistics()
    {
        ProfilingData *prof_data = this->GetFrame()->GetMethod()->GetProfilingDataWithoutCheck();
        if (prof_data != nullptr) {
            auto pc = this->GetBytecodeOffset();
            if constexpr (taken) {
                prof_data->UpdateBranchTaken(pc);
            } else {
                prof_data->UpdateBranchNotTaken(pc);
            }
        }
    }

private:
    InstructionHandlerState *state_;
};

}  // namespace panda::interpreter

#endif  // PANDA_INTERPRETER_INSTRUCTION_HANDLER_BASE_H_
