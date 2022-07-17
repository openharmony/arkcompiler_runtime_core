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
#ifndef PANDA_RUNTIME_COMPILER_INTERFACE_H_
#define PANDA_RUNTIME_COMPILER_INTERFACE_H_

#include <cstdint>
#include <atomic>

#include "libpandabase/macros.h"
#include "libpandafile/file.h"
#include "runtime/interpreter/frame.h"
#include "runtime/thread_pool_queue.h"

namespace panda {

namespace coretypes {
class Array;
}  // namespace coretypes

class PandaVM;
class CompilerTask : public TaskInterface {
public:
    explicit CompilerTask(Method *method = nullptr, bool is_osr = false, PandaVM *vm = nullptr)
        : method_(method), is_osr_(is_osr), vm_(vm)
    {
    }
    Method *GetMethod() const
    {
        return method_;
    }
    bool IsOsr() const
    {
        return is_osr_;
    }

    bool IsEmpty() const
    {
        return method_ == nullptr;
    }

    PandaVM *GetVM() const
    {
        return vm_;
    }

private:
    Method *method_ {nullptr};
    bool is_osr_ {false};
    PandaVM *vm_ {nullptr};
};

class Method;
class CompilerInterface {
public:
    enum class ReturnReason { RET_OK = 0, RET_DEOPTIMIZATION = 1 };

    class ExecState {
    public:
        ExecState(const uint8_t *pc, Frame *frame, Method *callee, size_t num_args, const bool *sp_flag)
            : pc_(pc), frame_(frame), callee_method_(callee), num_args_(num_args), sp_flag_(sp_flag)
        {
        }

        const uint8_t *GetPc() const
        {
            return pc_;
        }

        void SetPc(const uint8_t *pc)
        {
            pc_ = pc;
        }

        Frame *GetFrame() const
        {
            return frame_;
        }

        void SetFrame(Frame *frame)
        {
            frame_ = frame;
        }

        size_t GetNumArgs() const
        {
            return num_args_;
        }

        const interpreter::VRegister &GetAcc() const
        {
            return acc_;
        }

        interpreter::VRegister &GetAcc()
        {
            return acc_;
        }

        void SetAcc(const interpreter::VRegister &acc)
        {
            acc_ = acc;
        }

        interpreter::VRegister &GetArg(size_t i)
        {
            return args_[i];
        }

        void SetArg(size_t i, const interpreter::VRegister &reg)
        {
            args_[i] = reg;
        }

        const uint8_t *GetMethodInst()
        {
            return frame_->GetInstrOffset();
        }

        const bool *GetSPFlag() const
        {
            return sp_flag_;
        }

        Method *GetCalleeMethod()
        {
            return callee_method_;
        }

        static size_t GetSize(size_t nargs)
        {
            return sizeof(ExecState) + sizeof(interpreter::VRegister) * nargs;
        }

        static constexpr uint32_t GetExecStateAccOffset()
        {
            return MEMBER_OFFSET(ExecState, acc_);
        }

        static constexpr uint32_t GetExecStateArgsOffset()
        {
            return MEMBER_OFFSET(ExecState, args_);
        }

        static constexpr uint32_t GetExecStatePcOffset()
        {
            return MEMBER_OFFSET(ExecState, pc_);
        }

        static constexpr uint32_t GetExecStateFrameOffset()
        {
            return MEMBER_OFFSET(ExecState, frame_);
        }

        static constexpr uint32_t GetExecStateSPFlagAddrOffset()
        {
            return MEMBER_OFFSET(ExecState, sp_flag_);
        }

        static constexpr uint32_t GetCalleeMethodOffset()
        {
            return MEMBER_OFFSET(ExecState, callee_method_);
        }

    private:
        const uint8_t *pc_;
        Frame *frame_;
        Method *callee_method_;
        size_t num_args_;
        const bool *sp_flag_;
        interpreter::VRegister acc_;
        __extension__ interpreter::VRegister args_[0];  // NOLINT(modernize-avoid-c-arrays)
    };

    CompilerInterface() = default;

    using CompiledEntryPoint = ReturnReason (*)(ExecState *);

    virtual bool CompileMethod(Method *method, uintptr_t bytecode_offset, bool osr) = 0;

    virtual void Destroy() = 0;

    virtual void PreZygoteFork() = 0;

    virtual void PostZygoteFork() = 0;

    virtual void *GetOsrCode(const Method *method) = 0;

    virtual void SetOsrCode(const Method *method, void *ptr) = 0;

    virtual void RemoveOsrCode(const Method *method) = 0;

    virtual ~CompilerInterface() = default;

    NO_COPY_SEMANTIC(CompilerInterface);
    NO_MOVE_SEMANTIC(CompilerInterface);
};

}  // namespace panda

#endif  // PANDA_RUNTIME_COMPILER_INTERFACE_H_
