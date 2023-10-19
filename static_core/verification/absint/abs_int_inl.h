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

#ifndef PANDA_VERIFICATION_ABSINT_ABS_INT_INL_H
#define PANDA_VERIFICATION_ABSINT_ABS_INT_INL_H

#include "abs_int_inl_compat_checks.h"
#include "bytecode_instruction-inl.h"
#include "file_items.h"
#include "include/mem/panda_containers.h"
#include "include/method.h"
#include "include/runtime.h"
#include "macros.h"
#include "runtime/include/class.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/interpreter/runtime_interface.h"
#include "type/type_system.h"
#include "util/lazy.h"
#include "utils/logger.h"
#include "util/str.h"
#include "verification/config/debug_breakpoint/breakpoint.h"
#include "verification_context.h"
#include "verification/public_internal.h"
#include "verification/type/type_system.h"
#include "verification_status.h"
#include "verifier_messages.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <type_traits>
#include <unordered_map>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INST()                                             \
    do {                                                       \
        if (!GetInst().IsValid()) {                            \
            SHOW_MSG(InvalidInstruction)                       \
            LOG_VERIFIER_INVALID_INSTRUCTION();                \
            END_SHOW_MSG();                                    \
            SET_STATUS_FOR_MSG(InvalidInstruction, WARNING);   \
            return false;                                      \
        }                                                      \
        if (job_->Options().ShowContext()) {                   \
            DumpRegs(ExecCtx().CurrentRegContext());           \
        }                                                      \
        SHOW_MSG(DebugAbsIntLogInstruction)                    \
        LOG_VERIFIER_DEBUG_ABS_INT_LOG_INSTRUCTION(GetInst()); \
        END_SHOW_MSG();                                        \
    } while (0)

#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DBGBRK()                                                                       \
    if (debug_) {                                                                      \
        DBG_MANAGED_BRK(debug_ctx, job_->JobMethod()->GetUniqId(), inst_.GetOffset()); \
    }
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DBGBRK()
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SHOW_MSG(Message) if (!job_->Options().IsHidden(VerifierMessage::Message)) {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define END_SHOW_MSG() }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_STATUS_FOR_MSG(Message, AtLeast)                                                       \
    do {                                                                                           \
        SetStatusAtLeast(VerificationStatus::AtLeast);                                             \
        SetStatusAtLeast(MsgClassToStatus(job_->Options().MsgClassFor(VerifierMessage::Message))); \
    } while (0)

/*
TODO(vdyadov): add AddrMap to verification context where put marks on all checked bytes.
after absint process ends, check this AddrMap for holes.
holes are either dead byte-code or issues with absint cflow handling.
*/

// TODO(vdyadov): refactor this file, all utility functions put in separate/other files

/*
TODO(vdyadov): IMPORTANT!!! (Done)
Current definition of context incompatibility is NOT CORRECT one!
There are situations when verifier will rule out fully correct programs.
For instance:
....
movi v0,0
ldai 0
jmp label1
....
lda.str ""
sta v0
ldai 0
jmp label1
.....
label1:
  return

Here we have context incompatibility on label1, but it does not harm, because conflicting reg is
not used anymore.

Solutions:
1(current). Conflicts are reported as warnings, conflicting regs are removed fro resulting context.
So, on attempt of usage of conflicting reg, absint will fail with message of undefined reg.
May be mark them as conflicting? (done)
2. On each label/checkpoint compute set of registers that will be used in next conputations and process
conflicting contexts modulo used registers set. It is complex solution, but very preciese.
*/

// TODO(vdyadov): regain laziness, strict evaluation is very expensive!

namespace panda::verifier {

class AbsIntInstructionHandler {
public:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    static constexpr int ACC = -1;
    static constexpr int INVALID_REG = -2;  // TODO(vdyadov): may be use Index<..> here?

    using TypeId = panda_file::Type::TypeId;
    using Builtin = Type::Builtin;

    Type const top_ {Builtin::TOP};
    Type const bot_ {Builtin::BOT};
    Type const u1_ {Builtin::U1};
    Type const i8_ {Builtin::I8};
    Type const u8_ {Builtin::U8};
    Type const i16_ {Builtin::I16};
    Type const u16_ {Builtin::U16};
    Type const i32_ {Builtin::I32};
    Type const u32_ {Builtin::U32};
    Type const i64_ {Builtin::I64};
    Type const u64_ {Builtin::U64};
    Type const f32_ {Builtin::F32};
    Type const f64_ {Builtin::F64};
    Type const integral32_ {Builtin::INTEGRAL32};
    Type const integral64_ {Builtin::INTEGRAL64};
    Type const float32_ {Builtin::FLOAT32};
    Type const float64_ {Builtin::FLOAT64};
    Type const bits32_ {Builtin::BITS32};
    Type const bits64_ {Builtin::BITS64};
    Type const primitive_ {Builtin::PRIMITIVE};
    Type const ref_type_ {Builtin::REFERENCE};
    Type const null_ref_type_ {Builtin::NULL_REFERENCE};
    Type const object_type_ {Builtin::OBJECT};
    Type const array_type_ {Builtin::ARRAY};

    Job const *job_;
    debug::DebugContext const *debug_ctx;
    Config const *config;

    // NOLINTEND(misc-non-private-member-variables-in-classes)

    AbsIntInstructionHandler(VerificationContext &verif_ctx, const uint8_t *pc, EntryPointType code_type)
        : job_ {verif_ctx.GetJob()},
          debug_ctx {&job_->GetService()->debug_ctx},
          config {GetServiceConfig(job_->GetService())},
          inst_(pc, verif_ctx.CflowInfo().GetAddrStart(), verif_ctx.CflowInfo().GetAddrEnd()),
          context_ {verif_ctx},
          code_type_ {code_type}
    {
#ifndef NDEBUG
        if (config->opts.mode == VerificationMode::DEBUG) {
            const auto &method = job_->JobMethod();
            debug_ = debug::ManagedBreakpointPresent(debug_ctx, method->GetUniqId());
            if (debug_) {
                LOG(DEBUG, VERIFIER) << "Debug mode for method " << method->GetFullName() << " is on";
            }
        }
#endif
    }

    ~AbsIntInstructionHandler() = default;
    NO_MOVE_SEMANTIC(AbsIntInstructionHandler);
    NO_COPY_SEMANTIC(AbsIntInstructionHandler);

    VerificationStatus GetStatus()
    {
        return status_;
    }

    uint8_t GetPrimaryOpcode()
    {
        return inst_.GetPrimaryOpcode();
    }

    uint8_t GetSecondaryOpcode()
    {
        return inst_.GetSecondaryOpcode();
    }

    bool IsPrimaryOpcodeValid() const
    {
        return inst_.IsPrimaryOpcodeValid();
    }

    bool IsRegDefined(int reg);

    PandaString ToString(Type tp) const
    {
        return tp.ToString(GetTypeSystem());
    }

    PandaString ToString(PandaVector<Type> const &types) const
    {
        PandaString result {"[ "};
        bool comma = false;
        for (const auto &type : types) {
            if (comma) {
                result += ", ";
            }
            result += ToString(type);
            comma = true;
        }
        result += " ]";
        return result;
    }

    PandaString ToString(AbstractTypedValue const *atv) const
    {
        return atv->ToString(GetTypeSystem());
    }

    static PandaString StringDataToString(panda_file::File::StringData sd)
    {
        PandaString res {reinterpret_cast<char *>(const_cast<uint8_t *>(sd.data))};
        return res;
    }

    bool CheckType(Type type, Type tgt_type)
    {
        return IsSubtype(type, tgt_type, GetTypeSystem());
    }

    bool CheckRegType(int reg, Type tgt_type);

    const AbstractTypedValue &GetReg(int reg_idx);

    Type GetRegType(int reg_idx);

    void SetReg(int reg_idx, const AbstractTypedValue &val);
    void SetReg(int reg_idx, Type type);

    void SetRegAndOthersOfSameOrigin(int reg_idx, const AbstractTypedValue &val);
    void SetRegAndOthersOfSameOrigin(int reg_idx, Type type);

    const AbstractTypedValue &GetAcc();

    Type GetAccType();

    void SetAcc(const AbstractTypedValue &val);
    void SetAcc(Type type);

    void SetAccAndOthersOfSameOrigin(const AbstractTypedValue &val);
    void SetAccAndOthersOfSameOrigin(Type type);

    AbstractTypedValue MkVal(Type t);

    TypeSystem *GetTypeSystem() const
    {
        return context_.GetTypeSystem();
    }

    Type TypeOfClass(Class const *klass);

    Type ReturnType();

    ExecContext &ExecCtx();

    void DumpRegs(const RegContext &ctx);

    bool CheckCtxCompatibility(const RegContext &src, const RegContext &dst);

    void Sync();

    void AssignRegToReg(int dst, int src)
    {
        auto atv = GetReg(src);
        if (!atv.GetOrigin().IsValid()) {
            // generate new origin and set all values to be originated at it
            AbstractTypedValue new_atv {atv, inst_};
            SetReg(src, new_atv);
            SetReg(dst, new_atv);
        } else {
            SetReg(dst, atv);
        }
    }

    void AssignAccToReg(int src)
    {
        AssignRegToReg(ACC, src);
    }

    void AssignRegToAcc(int dst)
    {
        AssignRegToReg(dst, ACC);
    }

    // Names meanings: vs - v_source, vd - v_destination
    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMov()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegType(vs, bits32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToReg(vd, vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegType(vs, bits64_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToReg(vd, vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        if (!CheckRegType(vs, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToReg(vd, vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovDyn()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        LOG(ERROR, VERIFIER) << "Verifier error: instruction is not implemented";
        status_ = VerificationStatus::ERROR;
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNop()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovi()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMoviWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, i64_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmovi()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, f32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmoviWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, f64_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovNull()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, null_ref_type_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLda()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vs, bits32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignAccToReg(vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaiDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vs, bits64_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignAccToReg(vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vs, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignAccToReg(vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdai()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaiWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(i64_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldai()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(f32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldaiWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(f64_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldaiDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaStr()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type cached_type = GetCachedType();
        if (!cached_type.IsConsistent()) {
            SET_STATUS_FOR_MSG(CannotResolveClassId, OK);
            return false;
        }
        SetAcc(cached_type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaConst()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        Sync();
        Type cached_type = GetCachedType();
        if (!cached_type.IsConsistent()) {
            // TODO(vdyadov): refactor to verifier-messages
            LOG(ERROR, VERIFIER) << "Verifier error: HandleLdaConst cache error";
            status_ = VerificationStatus::ERROR;
            return false;
        }
        SetReg(vd, cached_type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaType()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type cached_type = GetCachedType();
        if (!cached_type.IsConsistent()) {
            SET_STATUS_FOR_MSG(CannotResolveClassId, OK);
            return false;
        }
        if (cached_type != GetTypeSystem()->ClassClass()) {
            LOG(ERROR, VERIFIER) << "LDA_TYPE type must be Class.";
            return false;
        }
        SetAcc(GetTypeSystem()->ClassClass());
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaNull()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(null_ref_type_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSta()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(ACC, bits32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToAcc(vd);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStaDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStaWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(ACC, bits64_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToAcc(vd);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStaObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(ACC, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToAcc(vd);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJmp()
    {
        LOG_INST();
        DBGBRK();
        int32_t imm = inst_.GetImm<FORMAT>();
        Sync();
        ProcessBranching(imm);
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCmpWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleUcmp();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleUcmpWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFcmpl();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFcmplWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFcmpg();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFcmpgWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJeqz();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJeqzObj()
    {
        LOG_INST();
        DBGBRK();
        auto imm = inst_.GetImm<FORMAT>();

        Sync();

        if (!CheckRegType(ACC, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        // TODO(vdyadov): think of two-pass absint, where we can catch const-null cases

        auto type = GetRegType(ACC);
        SetAccAndOthersOfSameOrigin(null_ref_type_);

        if (!ProcessBranching(imm)) {
            return false;
        }

        SetAccAndOthersOfSameOrigin(type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJnez();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJnezObj()
    {
        LOG_INST();
        DBGBRK();
        auto imm = inst_.GetImm<FORMAT>();
        Sync();

        if (!CheckRegType(ACC, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        SetAccAndOthersOfSameOrigin(null_ref_type_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJltz();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJgtz();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJlez();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJgez();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJeq();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJeqObj()
    {
        LOG_INST();
        DBGBRK();
        auto imm = inst_.GetImm<FORMAT>();
        uint16_t vs = inst_.GetVReg<FORMAT>();

        Sync();

        if (!CheckRegType(ACC, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckRegType(vs, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJne();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJneObj()
    {
        LOG_INST();
        DBGBRK();
        auto imm = inst_.GetImm<FORMAT>();
        uint16_t vs = inst_.GetVReg<FORMAT>();

        Sync();

        if (!CheckRegType(ACC, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckRegType(vs, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJlt();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJgt();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJle();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJge();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFadd2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFadd2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFsub2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFsub2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmul2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmul2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFdiv2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFdiv2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmod2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmod2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivu2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivu2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModu2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModu2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFadd2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFadd2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFsub2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFsub2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmul2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmul2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFdiv2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFdiv2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmod2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmod2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivu2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivu2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModu2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModu2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAddv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSubv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMulv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAndv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOrv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXorv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShlv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShrv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshrv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAddi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSubi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMuli();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAndi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOri();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXori();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShli();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShri();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshri();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAddiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSubiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMuliv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAndiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOriv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXoriv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShliv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShriv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshriv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiviv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNeg()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(integral32_, i32_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNegWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(integral64_, i64_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFneg()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(f32_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFnegWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(f64_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNot()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(integral32_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNotWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(integral64_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleInci()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vx = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vx, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32toi64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32toi16();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tou16();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32toi8();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tou8();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tou1();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32toi64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32toi16();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tou16();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32toi8();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tou8();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tou1();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI64toi32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI64tou1();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI64tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI64tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64toi32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64tou32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64tou1();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32toi32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32toi64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32tou32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32tou64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64toi64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64toi32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64tou64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64tou32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallPolymorphicShort()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallPolymorphic()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallPolymorphicRange()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        // It is a runtime (not verifier) responibility to check the MethodHandle.invoke()
        // parameter types and throw the WrongMethodTypeException if need
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallePolymorphicShort()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallePolymorphic()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallePolymorphicRange()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        // It is a runtime (not verifier) responibility to check the  MethodHandle.invokeExact()
        // parameter types and throw the WrongMethodTypeException if need
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarr8()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {u1_, i8_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarr16()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {i16_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarr()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {i32_, u32_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarrWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {i64_, u64_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarru8()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {u1_, u8_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarru16()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {u16_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldarr32()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {f32_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldarrWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {f64_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarrObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(ACC, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckRegType(vs, array_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        auto reg_type = GetRegType(vs);
        if (reg_type == null_ref_type_) {
            // TODO(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(vs);
            END_SHOW_MSG();
            SetAcc(top_);
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }

        auto arr_elt_type = reg_type.GetArrayElementType(GetTypeSystem());
        if (!IsSubtype(arr_elt_type, ref_type_, GetTypeSystem())) {
            SHOW_MSG(BadArrayElementType)
            LOG_VERIFIER_BAD_ARRAY_ELEMENT_TYPE(ToString(arr_elt_type), ToString(ref_type_));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadArrayElementType, WARNING);
            return false;
        }
        SetAcc(arr_elt_type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarr8()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, integral32_, {u1_, i8_, u8_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarr16()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, integral32_, {i16_, u16_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarr()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, integral32_, {i32_, u32_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarrWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, integral64_, {i64_, u64_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFstarr32()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, float32_, {f32_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFstarrWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, float64_, {f64_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarrObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStore<FORMAT>(v1, v2, ref_type_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLenarr()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vs, array_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNewarr()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        if (!CheckRegType(vs, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        Type type = GetCachedType();
        if (!type.IsConsistent()) {
            SET_STATUS_FOR_MSG(CannotResolveClassId, OK);
            return false;
        }
        SHOW_MSG(DebugType)
        LOG_VERIFIER_DEBUG_TYPE(ToString(type));
        END_SHOW_MSG();
        if (!IsSubtype(type, array_type_, GetTypeSystem())) {
            // TODO(vdyadov): implement StrictSubtypes function to not include array_type_ in output
            SHOW_MSG(ArrayOfNonArrayType)
            LOG_VERIFIER_ARRAY_OF_NON_ARRAY_TYPE(ToString(type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ArrayOfNonArrayType, WARNING);
            return false;
        }
        SetReg(vd, type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNewobj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        Sync();
        Type cached_type = GetCachedType();
        if (!cached_type.IsConsistent()) {
            LOG(ERROR, VERIFIER) << "Verifier error: HandleNewobj cache error";
            status_ = VerificationStatus::ERROR;
            return false;
        }
        SHOW_MSG(DebugType)
        LOG_VERIFIER_DEBUG_TYPE(ToString(cached_type));
        END_SHOW_MSG();
        if (!IsSubtype(cached_type, object_type_, GetTypeSystem())) {
            SHOW_MSG(ObjectOfNonObjectType)
            LOG_VERIFIER_OBJECT_OF_NON_OBJECT_TYPE(ToString(cached_type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ObjectOfNonObjectType, WARNING);
            return false;
        }
        SetReg(vd, cached_type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckCallCtor(Method const *ctor, Span<int> regs)
    {
        Type obj_type = TypeOfClass(ctor->GetClass());

        // TODO(vdyadov): put under NDEBUG?
        {
            if (debug_ctx->SkipVerificationOfCall(ctor->GetUniqId())) {
                SetAcc(obj_type);
                MoveToNextInst<FORMAT>();
                return true;
            }
        }

        auto ctor_name_getter = [&ctor]() { return ctor->GetFullName(); };

        bool check = CheckMethodArgs(ctor_name_getter, ctor, regs, obj_type);
        if (check) {
            SetAcc(obj_type);
            MoveToNextInst<FORMAT>();
        }
        return check;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckCtor(Span<int> regs)
    {
        Type type = GetCachedType();

        if (UNLIKELY(type.IsClass() && type.GetClass()->IsArrayClass())) {
            if (job_->IsMethodPresentForOffset(inst_.GetOffset())) {
                // Array constructors are synthetic methods; ClassLinker does not provide them.
                LOG(ERROR, VERIFIER) << "Verifier internal error: ArrayCtor should not be instantiated as method";
                return false;
            }
            SHOW_MSG(DebugArrayConstructor)
            LOG_VERIFIER_DEBUG_ARRAY_CONSTRUCTOR();
            END_SHOW_MSG();
            return CheckArrayCtor<FORMAT>(type, regs);
        }

        Method const *ctor = GetCachedMethod();

        if (!type.IsConsistent() || ctor == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveMethodId, OK);
            SET_STATUS_FOR_MSG(CannotResolveClassId, OK);
            return false;
        }

        PandaString expected_name = panda::panda_file::GetCtorName(ctor->GetClass()->GetSourceLang());
        if (!ctor->IsConstructor() || ctor->IsStatic() || expected_name != StringDataToString(ctor->GetName())) {
            SHOW_MSG(InitobjCallsNotConstructor)
            LOG_VERIFIER_INITOBJ_CALLS_NOT_CONSTRUCTOR(ctor->GetFullName());
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(InitobjCallsNotConstructor, WARNING);
            return false;
        }

        SHOW_MSG(DebugConstructor)
        LOG_VERIFIER_DEBUG_CONSTRUCTOR(ctor->GetFullName());
        END_SHOW_MSG();

        return CheckCallCtor<FORMAT>(ctor, regs);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleInitobj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        uint16_t vs3 = inst_.GetVReg<FORMAT, 0x02>();
        uint16_t vs4 = inst_.GetVReg<FORMAT, 0x03>();
        Sync();
        std::array<int, 4> regs {vs1, vs2, vs3, vs4};
        return CheckCtor<FORMAT>(Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleInitobjShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        std::array<int, 2> regs {vs1, vs2};
        return CheckCtor<FORMAT>(Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleInitobjRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Sync();
        std::vector<int> regs;
        for (auto reg_idx = vs; ExecCtx().CurrentRegContext().IsRegDefined(reg_idx); reg_idx++) {
            regs.push_back(reg_idx);
        }
        return CheckCtor<FORMAT>(Span {regs});
    }

    Type GetFieldType()
    {
        const Field *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return {};
        }

        ScopedChangeThreadStatus st {ManagedThread::GetCurrent(), ThreadStatus::RUNNING};
        Job::ErrorHandler handler;
        auto type_cls = field->ResolveTypeClass(&handler);
        if (type_cls == nullptr) {
            return Type {};
        }
        return Type {type_cls};
    }

    Type GetFieldObject()
    {
        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return {};
        }
        return TypeOfClass(field->GetClass());
    }

    bool CheckFieldAccess(int reg_idx, Type expected_field_type, bool is_static, bool is_volatile)
    {
        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        if (is_static != field->IsStatic()) {
            SHOW_MSG(ExpectedStaticOrInstanceField)
            LOG_VERIFIER_EXPECTED_STATIC_OR_INSTANCE_FIELD(is_static);
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ExpectedStaticOrInstanceField, WARNING);
            return false;
        }

        if (is_volatile != field->IsVolatile()) {
            // if the inst is volatile but the field is not
            if (is_volatile) {
                SHOW_MSG(ExpectedVolatileField)
                LOG_VERIFIER_EXPECTED_VOLATILE_FIELD();
                END_SHOW_MSG();
                SET_STATUS_FOR_MSG(ExpectedVolatileField, WARNING);
                return false;
            }
            // if the instruction is not volatile but the field is
            SHOW_MSG(ExpectedInstanceField)
            LOG_VERIFIER_EXPECTED_INSTANCE_FIELD();
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ExpectedInstanceField, ERROR);
            return false;
        }

        Type field_obj_type = GetFieldObject();
        Type field_type = GetFieldType();
        if (!field_type.IsConsistent()) {
            LOG_VERIFIER_CANNOT_RESOLVE_FIELD_TYPE(GetFieldName(field));
            return false;
        }

        if (!is_static) {
            if (!IsRegDefined(reg_idx)) {
                SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
                return false;
            }
            Type obj_type = GetRegType(reg_idx);
            if (obj_type == null_ref_type_) {
                // TODO(vdyadov): redesign next code, after support exception handlers,
                //                treat it as always throw NPE
                SHOW_MSG(AlwaysNpe)
                LOG_VERIFIER_ALWAYS_NPE(reg_idx);
                END_SHOW_MSG();
                SET_STATUS_FOR_MSG(AlwaysNpe, OK);
                return false;
            }
            if (!IsSubtype(obj_type, field_obj_type, GetTypeSystem())) {
                SHOW_MSG(InconsistentRegisterAndFieldTypes)
                LOG_VERIFIER_INCONSISTENT_REGISTER_AND_FIELD_TYPES(GetFieldName(field), reg_idx, ToString(obj_type),
                                                                   ToString(field_obj_type));
                END_SHOW_MSG();
                SET_STATUS_FOR_MSG(InconsistentRegisterAndFieldTypes, WARNING);
            }
        }

        if (!IsSubtype(field_type, expected_field_type, GetTypeSystem())) {
            SHOW_MSG(UnexpectedFieldType)
            LOG_VERIFIER_UNEXPECTED_FIELD_TYPE(GetFieldName(field), ToString(field_type),
                                               ToString(expected_field_type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(UnexpectedFieldType, WARNING);
            return false;
        }

        auto *plugin = job_->JobPlugin();
        auto const *job_method = job_->JobMethod();
        auto result = plugin->CheckFieldAccessViolation(field, job_method, GetTypeSystem());
        if (!result.IsOk()) {
            const auto &verif_opts = config->opts;
            if (verif_opts.debug.allow.field_access_violation && result.IsError()) {
                result.status = VerificationStatus::WARNING;
            }
            LogInnerMessage(result);
            LOG_VERIFIER_DEBUG_FIELD2(GetFieldName(field));
            status_ = result.status;
            return status_ != VerificationStatus::ERROR;
        }

        return !result.IsError();
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoad(int reg_dest, int reg_src, Type expected_field_type, bool is_static, bool is_volatile = false)
    {
        if (!CheckFieldAccess(reg_src, expected_field_type, is_static, is_volatile)) {
            return false;
        }
        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        auto type = GetFieldType();
        if (!type.IsConsistent()) {
            return false;
        }
        SetReg(reg_dest, type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoad(int reg_idx, Type expected_field_type, bool is_static)
    {
        return ProcessFieldLoad<FORMAT>(ACC, reg_idx, expected_field_type, is_static);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoadVolatile(int reg_dest, int reg_src, Type expected_field_type, bool is_static)
    {
        return ProcessFieldLoad<FORMAT>(reg_dest, reg_src, expected_field_type, is_static, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoadVolatile(int reg_idx, Type expected_field_type, bool is_static)
    {
        return ProcessFieldLoadVolatile<FORMAT>(ACC, reg_idx, expected_field_type, is_static);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vs, bits32_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vs, bits64_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vs, ref_type_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vd, vs, bits32_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vd, vs, bits64_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vd, vs, ref_type_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatile()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vs, bits32_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vs, bits64_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vs, ref_type_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vd, vs, bits32_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileVWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vd, vs, bits64_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileVObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vd, vs, ref_type_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT, typename Check>
    bool ProcessStoreField(int vs, int vd, Type expected_field_type, bool is_static, Check check,
                           bool is_volatile = false)
    {
        if (!CheckRegType(vs, expected_field_type)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckFieldAccess(vd, expected_field_type, is_static, is_volatile)) {
            return false;
        }

        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }
        Type field_type = GetFieldType();
        if (!field_type.IsConsistent()) {
            return false;
        }

        Type vs_type = GetRegType(vs);

        CheckResult const &result = check(field_type.ToTypeId(), vs_type.ToTypeId());
        if (result.status != VerificationStatus::OK) {
            LOG_VERIFIER_DEBUG_STORE_FIELD(GetFieldName(field), ToString(field_type), ToString(vs_type));
            status_ = result.status;
            if (result.IsError()) {
                return false;
            }
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobj(int vs, int vd, bool is_static)
    {
        return ProcessStoreField<FORMAT>(vs, vd, bits32_, is_static, CheckStobj);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobj(int vd, bool is_static)
    {
        return ProcessStobj<FORMAT>(ACC, vd, is_static);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatile(int vs, int vd, bool is_static)
    {
        return ProcessStoreField<FORMAT>(vs, vd, bits32_, is_static, CheckStobj, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatile(int vd, bool is_static)
    {
        return ProcessStobjVolatile<FORMAT>(ACC, vd, is_static);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStobj<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();

        return ProcessStobj<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatile()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStobjVolatile<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();

        return ProcessStobjVolatile<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjWide(int vs, int vd, bool is_static)
    {
        return ProcessStoreField<FORMAT>(vs, vd, bits64_, is_static, CheckStobjWide);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjWide(int vd, bool is_static)
    {
        return ProcessStobjWide<FORMAT>(ACC, vd, is_static);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatileWide(int vs, int vd, bool is_static)
    {
        return ProcessStoreField<FORMAT>(vs, vd, bits64_, is_static, CheckStobjWide, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatileWide(int vd, bool is_static)
    {
        return ProcessStobjVolatileWide<FORMAT>(ACC, vd, is_static);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStobjWide<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();

        return ProcessStobjWide<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStobjVolatileWide<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileVWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();

        return ProcessStobjVolatileWide<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjObj(int vs, int vd, bool is_static, bool is_volatile = false)
    {
        if (!CheckFieldAccess(vd, ref_type_, is_static, is_volatile)) {
            return false;
        }

        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        Type field_type = GetFieldType();
        if (!field_type.IsConsistent()) {
            return false;
        }

        if (!CheckRegType(vs, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        Type vs_type = GetRegType(vs);

        if (!IsSubtype(vs_type, field_type, GetTypeSystem())) {
            SHOW_MSG(BadAccumulatorType)
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE(ToString(vs_type), ToString(field_type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadAccumulatorType, WARNING);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjObj(int vd, bool is_static)
    {
        return ProcessStobjObj<FORMAT>(ACC, vd, is_static);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatileObj(int vs, int vd, bool is_static)
    {
        return ProcessStobjObj<FORMAT>(vs, vd, is_static, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatileObj(int vd, bool is_static)
    {
        return ProcessStobjVolatileObj<FORMAT>(ACC, vd, is_static);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessStobjObj<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessStobjObj<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessStobjVolatileObj<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileVObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessStobjVolatileObj<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstatic()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoad<FORMAT>(INVALID_REG, bits32_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoad<FORMAT>(INVALID_REG, bits64_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoad<FORMAT>(INVALID_REG, ref_type_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticVolatile()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(INVALID_REG, bits32_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticVolatileWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(INVALID_REG, bits64_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticVolatileObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(INVALID_REG, ref_type_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstatic()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobj<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjWide<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjObj<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticVolatile()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjVolatile<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticVolatileWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjVolatileWide<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticVolatileObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjVolatileObj<FORMAT>(INVALID_REG, true);
    }

    template <typename Check>
    bool CheckReturn(Type ret_type, Type acc_type, Check check)
    {
        TypeId ret_type_id = ret_type.ToTypeId();

        PandaVector<Type> compatible_acc_types;
        // TODO (gogabr): why recompute each time?
        for (size_t acc_idx = 0; acc_idx < static_cast<size_t>(TypeId::REFERENCE) + 1; ++acc_idx) {
            auto acc_type_id = static_cast<TypeId>(acc_idx);
            const CheckResult &info = check(ret_type_id, acc_type_id);
            if (!info.IsError()) {
                compatible_acc_types.push_back(Type::FromTypeId(acc_type_id));
            }
        }

        if (!CheckType(acc_type, primitive_) || acc_type == primitive_) {
            LOG_VERIFIER_BAD_ACCUMULATOR_RETURN_VALUE_TYPE(ToString(acc_type));
            SET_STATUS_FOR_MSG(BadAccumulatorReturnValueType, WARNING);
            return false;
        }

        TypeId acc_type_id = acc_type.ToTypeId();

        const auto &result = check(ret_type_id, acc_type_id);

        if (!result.IsOk()) {
            LogInnerMessage(result);
            if (result.IsError()) {
                LOG_VERIFIER_DEBUG_FUNCTION_RETURN_AND_ACCUMULATOR_TYPES_WITH_COMPATIBLE_TYPES(
                    ToString(ReturnType()), ToString(acc_type), ToString(compatible_acc_types));
            } else {
                LOG_VERIFIER_DEBUG_FUNCTION_RETURN_AND_ACCUMULATOR_TYPES(ToString(ReturnType()), ToString(acc_type));
            }
        }

        status_ = result.status;
        return status_ != VerificationStatus::ERROR;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturn()
    {
        LOG_INST();
        DBGBRK();
        Sync();

        if (!CheckType(ReturnType(), bits32_)) {
            LOG_VERIFIER_BAD_RETURN_INSTRUCTION_TYPE("", ToString(ReturnType()), ToString(bits32_));
            SET_STATUS_FOR_MSG(BadReturnInstructionType, WARNING);
            return false;
        }

        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        // TODO(vdyadov): handle LUB of compatible primitive types
        if (!CheckType(GetAccType(), bits32_)) {
            LOG_VERIFIER_BAD_ACCUMULATOR_RETURN_VALUE_TYPE(ToString(GetAccType()));
            SET_STATUS_FOR_MSG(BadAccumulatorReturnValueType, WARNING);
        }
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturnDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLaunchShort();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLaunch();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLaunchRange();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLaunchVirtShort();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLaunchVirt();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLaunchVirtRange();

    template <bool IS_LOAD>
    bool CheckFieldAccessByName(int reg_idx, Type expected_field_type)
    {
        Field const *raw_field = GetCachedField();

        if (raw_field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        if (raw_field->IsStatic()) {
            SHOW_MSG(ExpectedStaticOrInstanceField)
            LOG_VERIFIER_EXPECTED_STATIC_OR_INSTANCE_FIELD(false);
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ExpectedStaticOrInstanceField, WARNING);
            return false;
        }

        Type raw_field_type = GetFieldType();
        if (!raw_field_type.IsConsistent()) {
            LOG_VERIFIER_CANNOT_RESOLVE_FIELD_TYPE(GetFieldName(raw_field));
            return false;
        }

        if (!IsRegDefined(reg_idx)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        Type obj_type = GetRegType(reg_idx);
        if (obj_type == null_ref_type_) {
            // TODO(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(reg_idx);
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }

        if (!obj_type.IsClass()) {
            SHOW_MSG(BadRegisterType)
            LOG_VERIFIER_BAD_REGISTER_CLASS_TYPE(RegisterName(reg_idx, true), ToString(obj_type));
            END_SHOW_MSG();
            return false;
        }
        auto obj_class = obj_type.GetClass();
        auto field = obj_class->LookupFieldByName(raw_field->GetName());
        Type field_type;
        if (field != nullptr) {
            field_type = Type::FromTypeId(field->GetTypeId());
        } else {
            Method *method = nullptr;
            if constexpr (IS_LOAD) {
                switch (expected_field_type.GetTypeWidth()) {
                    case coretypes::INT32_BITS:
                        method = obj_class->LookupGetterByName<panda_file::Type::TypeId::I32>(raw_field->GetName());
                        break;
                    case coretypes::INT64_BITS:
                        method = obj_class->LookupGetterByName<panda_file::Type::TypeId::I64>(raw_field->GetName());
                        break;
                    case 0:
                        method =
                            obj_class->LookupGetterByName<panda_file::Type::TypeId::REFERENCE>(raw_field->GetName());
                        break;
                    default:
                        UNREACHABLE();
                }
            } else {
                switch (expected_field_type.GetTypeWidth()) {
                    case coretypes::INT32_BITS:
                        method = obj_class->LookupSetterByName<panda_file::Type::TypeId::I32>(raw_field->GetName());
                        break;
                    case coretypes::INT64_BITS:
                        method = obj_class->LookupSetterByName<panda_file::Type::TypeId::I64>(raw_field->GetName());
                        break;
                    case 0:
                        method =
                            obj_class->LookupSetterByName<panda_file::Type::TypeId::REFERENCE>(raw_field->GetName());
                        break;
                    default:
                        UNREACHABLE();
                }
            }
            if (method == nullptr) {
                SHOW_MSG(BadFieldNameOrBitWidth)
                LOG_VERIFIER_BAD_FIELD_NAME_OR_BIT_WIDTH(GetFieldName(field), ToString(obj_type),
                                                         ToString(expected_field_type));
                END_SHOW_MSG();
                return false;
            }
            if constexpr (IS_LOAD) {
                field_type = Type::FromTypeId(method->GetReturnType().GetId());
            } else {
                field_type = Type::FromTypeId(method->GetArgType(1).GetId());
            }
        }

        if (!IsSubtype(field_type, expected_field_type, GetTypeSystem())) {
            SHOW_MSG(UnexpectedFieldType)
            LOG_VERIFIER_UNEXPECTED_FIELD_TYPE(GetFieldName(field), ToString(field_type),
                                               ToString(expected_field_type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(UnexpectedFieldType, WARNING);
            return false;
        }

        auto *plugin = job_->JobPlugin();
        auto const *job_method = job_->JobMethod();
        auto result = plugin->CheckFieldAccessViolation(field, job_method, GetTypeSystem());
        if (!result.IsOk()) {
            const auto &verif_opts = config->opts;
            if (verif_opts.debug.allow.field_access_violation && result.IsError()) {
                result.status = VerificationStatus::WARNING;
            }
            LogInnerMessage(result);
            LOG_VERIFIER_DEBUG_FIELD2(GetFieldName(field));
            status_ = result.status;
            return status_ != VerificationStatus::ERROR;
        }

        return !result.IsError();
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoadByName(int reg_src, Type expected_field_type)
    {
        if (!CheckFieldAccessByName<true>(reg_src, expected_field_type)) {
            return false;
        }
        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        auto type = GetFieldType();
        if (!type.IsConsistent()) {
            return false;
        }
        SetReg(ACC, type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLdobjName()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadByName<FORMAT>(vs, bits32_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLdobjNameWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadByName<FORMAT>(vs, bits64_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLdobjNameObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadByName<FORMAT>(vs, ref_type_);
    }

    template <BytecodeInstructionSafe::Format FORMAT, typename Check>
    bool ProcessStoreFieldByName(int vd, Type expected_field_type, Check check)
    {
        if (!CheckRegType(ACC, expected_field_type)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckFieldAccessByName<false>(vd, expected_field_type)) {
            return false;
        }

        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }
        Type field_type = GetFieldType();
        if (!field_type.IsConsistent()) {
            return false;
        }

        Type vs_type = GetRegType(ACC);

        CheckResult const &result = check(field_type.ToTypeId(), vs_type.ToTypeId());
        if (result.status != VerificationStatus::OK) {
            LOG_VERIFIER_DEBUG_STORE_FIELD(GetFieldName(field), ToString(field_type), ToString(vs_type));
            status_ = result.status;
            if (result.IsError()) {
                return false;
            }
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjObjByName(int vd)
    {
        if (!CheckFieldAccessByName<false>(vd, ref_type_)) {
            return false;
        }

        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        Type field_type = GetFieldType();
        if (!field_type.IsConsistent()) {
            return false;
        }

        if (!CheckRegType(ACC, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        Type vs_type = GetRegType(ACC);

        if (!IsSubtype(vs_type, field_type, GetTypeSystem())) {
            SHOW_MSG(BadAccumulatorType)
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE(ToString(vs_type), ToString(field_type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadAccumulatorType, WARNING);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsStobjName()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStoreFieldByName<FORMAT>(vd, bits32_, CheckStobj);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsStobjNameWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStoreFieldByName<FORMAT>(vd, bits64_, CheckStobjWide);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsStobjNameObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessStobjObjByName<FORMAT>(vd);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturnWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();

        if (!CheckType(ReturnType(), bits64_)) {
            LOG_VERIFIER_BAD_RETURN_INSTRUCTION_TYPE(".64", ToString(ReturnType()), ToString(bits64_));
            status_ = VerificationStatus::ERROR;
            return false;
        }

        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckType(GetAccType(), bits64_)) {
            LOG_VERIFIER_BAD_ACCUMULATOR_RETURN_VALUE_TYPE(ToString(GetAccType()));
            status_ = VerificationStatus::ERROR;
        }
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturnObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();

        if (!CheckType(ReturnType(), ref_type_)) {
            LOG_VERIFIER_BAD_RETURN_INSTRUCTION_TYPE(".obj", ToString(ReturnType()), ToString(ref_type_));
            status_ = VerificationStatus::ERROR;
            return false;
        }

        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        auto acc_type = GetAccType();

        if (!CheckType(acc_type, ReturnType())) {
            LOG_VERIFIER_BAD_ACCUMULATOR_RETURN_VALUE_TYPE_WITH_SUBTYPE(ToString(acc_type), ToString(ReturnType()));
            // TODO(vdyadov) : after solving issues with set of types in LUB, uncomment next line
            status_ = VerificationStatus::WARNING;
        }

        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturnVoid()
    {
        LOG_INST();
        DBGBRK();
        // TODO(vdyadov): think of introducing void as of separate type, like null
        Sync();

        if (ReturnType() != Type::Top()) {
            LOG_VERIFIER_BAD_RETURN_VOID_INSTRUCTION_TYPE(ToString(ReturnType()));
            status_ = VerificationStatus::ERROR;
        }

        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCheckcast()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type cached_type = GetCachedType();
        if (!cached_type.IsConsistent()) {
            return false;
        }
        LOG_VERIFIER_DEBUG_TYPE(ToString(cached_type));
        if (!IsSubtype(cached_type, object_type_, GetTypeSystem()) &&
            !IsSubtype(cached_type, array_type_, GetTypeSystem())) {
            LOG_VERIFIER_CHECK_CAST_TO_NON_OBJECT_TYPE(ToString(cached_type));
            SET_STATUS_FOR_MSG(CheckCastToNonObjectType, WARNING);
            return false;
        }
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        auto acc_type = GetAccType();
        // TODO(vdyadov): remove this check after #2365
        auto res =
            !IsSubtype(acc_type, ref_type_, GetTypeSystem()) && !IsSubtype(acc_type, array_type_, GetTypeSystem());
        if (res) {
            LOG_VERIFIER_NON_OBJECT_ACCUMULATOR_TYPE();
            SET_STATUS_FOR_MSG(NonObjectAccumulatorType, WARNING);
            return false;
        }

        if (IsSubtype(acc_type, null_ref_type_, GetTypeSystem())) {
            LOG_VERIFIER_ACCUMULATOR_ALWAYS_NULL();
            SET_STATUS_FOR_MSG(AccumulatorAlwaysNull, OK);
            // Don't set types for "others of the same origin" when origin is null: n = null, a = n, b = n, a =
            // (NewType)x
            SetAcc(cached_type);
            MoveToNextInst<FORMAT>();
            return true;
        }

        if (IsSubtype(acc_type, cached_type, GetTypeSystem())) {
            LOG_VERIFIER_REDUNDANT_CHECK_CAST(ToString(acc_type), ToString(cached_type));
            SET_STATUS_FOR_MSG(RedundantCheckCast, OK);
            // Do not update register type to parent type as we loose details and can get errors on further flow
            MoveToNextInst<FORMAT>();
            return true;
        }

        if (IsSubtype(cached_type, array_type_, GetTypeSystem())) {
            auto elt_type = cached_type.GetArrayElementType(GetTypeSystem());
            res = !IsSubtype(acc_type, array_type_, GetTypeSystem()) &&
                  !IsSubtype(cached_type, acc_type, GetTypeSystem());
            if (res) {
                LOG_VERIFIER_IMPOSSIBLE_CHECK_CAST(ToString(acc_type));
                status_ = VerificationStatus::WARNING;
            } else if (IsSubtype(acc_type, array_type_, GetTypeSystem())) {
                auto acc_elt_type = acc_type.GetArrayElementType(GetTypeSystem());
                if (acc_elt_type.IsConsistent() && !IsSubtype(acc_elt_type, elt_type, GetTypeSystem()) &&
                    !IsSubtype(elt_type, acc_elt_type, GetTypeSystem())) {
                    LOG_VERIFIER_IMPOSSIBLE_ARRAY_CHECK_CAST(ToString(acc_elt_type));
                    SET_STATUS_FOR_MSG(ImpossibleArrayCheckCast, OK);
                }
            }
        } else if (TpIntersection(cached_type, acc_type, GetTypeSystem()) == bot_) {
            LOG_VERIFIER_INCOMPATIBLE_ACCUMULATOR_TYPE(ToString(acc_type));
            SET_STATUS_FOR_MSG(IncompatibleAccumulatorType, OK);
        }

        if (status_ == VerificationStatus::ERROR) {
            SetAcc(top_);
            return false;
        }

        SetAccAndOthersOfSameOrigin(TpIntersection(cached_type, acc_type, GetTypeSystem()));

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleIsinstance()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type cached_type = GetCachedType();
        if (!cached_type.IsConsistent()) {
            return false;
        }
        LOG_VERIFIER_DEBUG_TYPE(ToString(cached_type));
        if (!IsSubtype(cached_type, object_type_, GetTypeSystem()) &&
            !IsSubtype(cached_type, array_type_, GetTypeSystem())) {
            // !(type <= Types().ArrayType()) is redundant, because all arrays
            // are subtypes of either panda.Object <: ObjectType or java.lang.Object <: ObjectType
            // depending on selected language context
            LOG_VERIFIER_BAD_IS_INSTANCE_INSTRUCTION(ToString(cached_type));
            SET_STATUS_FOR_MSG(BadIsInstanceInstruction, WARNING);
            return false;
        }
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        auto *plugin = job_->JobPlugin();
        auto const *job_method = job_->JobMethod();
        auto result = CheckResult::ok;
        if (cached_type.IsClass()) {
            result = plugin->CheckClassAccessViolation(cached_type.GetClass(), job_method, GetTypeSystem());
        }
        if (!result.IsOk()) {
            LogInnerMessage(CheckResult::protected_class);
            LOG_VERIFIER_DEBUG_CALL_FROM_TO(job_->JobMethod()->GetClass()->GetName(), ToString(cached_type));
            status_ = VerificationStatus::ERROR;
            return false;
        }

        auto acc_type = GetAccType();
        // TODO(vdyadov): remove this check after #2365
        auto res =
            !IsSubtype(acc_type, ref_type_, GetTypeSystem()) && !IsSubtype(acc_type, array_type_, GetTypeSystem());
        if (res) {
            LOG_VERIFIER_NON_OBJECT_ACCUMULATOR_TYPE();
            status_ = VerificationStatus::ERROR;
            return false;
        }

        if (IsSubtype(acc_type, null_ref_type_, GetTypeSystem())) {
            LOG_VERIFIER_ACCUMULATOR_ALWAYS_NULL();
            SET_STATUS_FOR_MSG(AccumulatorAlwaysNull, OK);
        } else if (IsSubtype(acc_type, cached_type, GetTypeSystem())) {
            LOG_VERIFIER_REDUNDANT_IS_INSTANCE(ToString(acc_type), ToString(cached_type));
            SET_STATUS_FOR_MSG(RedundantIsInstance, OK);
        } else if (IsSubtype(cached_type, array_type_, GetTypeSystem())) {
            auto elt_type = cached_type.GetArrayElementType(GetTypeSystem());
            auto acc_elt_type = acc_type.GetArrayElementType(GetTypeSystem());
            bool acc_elt_type_is_empty = acc_elt_type.IsConsistent();
            res = !IsSubtype(acc_elt_type, elt_type, GetTypeSystem()) &&
                  !IsSubtype(elt_type, acc_elt_type, GetTypeSystem());
            if (res) {
                if (acc_elt_type_is_empty) {
                    LOG_VERIFIER_IMPOSSIBLE_IS_INSTANCE(ToString(acc_type));
                    SET_STATUS_FOR_MSG(ImpossibleIsInstance, OK);
                } else {
                    LOG_VERIFIER_IMPOSSIBLE_ARRAY_IS_INSTANCE(ToString(acc_elt_type));
                    SET_STATUS_FOR_MSG(ImpossibleArrayIsInstance, OK);
                }
            }
        } else if (TpIntersection(cached_type, acc_type, GetTypeSystem()) == bot_) {
            LOG_VERIFIER_IMPOSSIBLE_IS_INSTANCE(ToString(acc_type));
            SET_STATUS_FOR_MSG(ImpossibleIsInstance, OK);
        }  // else {
        // TODO(vdyadov): here we may increase precision to concrete values in some cases
        SetAcc(i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <typename NameGetter>
    bool CheckMethodArgs(NameGetter name_getter, Method const *method, Span<int> regs, Type constructed_type = Type {})
    {
        bool checking_constructor = !constructed_type.IsNone();
        auto const *sig = GetTypeSystem()->GetMethodSignature(method);
        auto const &formal_args = sig->args;
        bool result = true;
        if (formal_args.empty()) {
            return true;
        }

        size_t regs_needed = checking_constructor ? formal_args.size() - 1 : formal_args.size();
        if (regs.size() < regs_needed) {
            SHOW_MSG(BadCallTooFewParameters)
            LOG_VERIFIER_BAD_CALL_TOO_FEW_PARAMETERS(name_getter());
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadCallTooFewParameters, WARNING);
            return false;
        }
        auto sig_iter = formal_args.cbegin();
        auto regs_iter = regs.cbegin();
        for (size_t argnum = 0; argnum < formal_args.size(); argnum++) {
            auto reg_num = (checking_constructor && sig_iter == formal_args.cbegin()) ? INVALID_REG : *(regs_iter++);
            auto formal_type = *(sig_iter++);
            auto const norm_type = GetTypeSystem()->NormalizedTypeOf(formal_type);

            if (reg_num != INVALID_REG && !IsRegDefined(reg_num)) {
                LOG_VERIFIER_BAD_CALL_UNDEFINED_REGISTER(name_getter(), reg_num);
                SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
                result = false;
                break;
            }
            Type actual_type = reg_num == INVALID_REG ? constructed_type : GetRegType(reg_num);
            Type norm_actual_type = GetTypeSystem()->NormalizedTypeOf(actual_type);
            // arg: NormalizedTypeOf(actual_type) <= norm_type
            // check of physical compatibility
            bool incompatible_types = false;
            auto actual_is_ref = IsSubtype(actual_type, ref_type_, GetTypeSystem());
            if (reg_num != INVALID_REG && IsSubtype(formal_type, ref_type_, GetTypeSystem()) &&
                formal_type != Type::Bot() && actual_is_ref) {
                if (IsSubtype(actual_type, formal_type, GetTypeSystem())) {
                    continue;
                }
                if (!config->opts.debug.allow.wrong_subclassing_in_method_args) {
                    incompatible_types = true;
                }
            } else if (formal_type != Type::Bot() && formal_type != Type::Top() &&
                       !IsSubtype(norm_actual_type, norm_type, GetTypeSystem())) {
                incompatible_types = true;
            }
            if (incompatible_types) {
                PandaString reg_or_param = reg_num == INVALID_REG ? "Actual parameter" : RegisterName(reg_num, true);
                SHOW_MSG(BadCallIncompatibleParameter)
                LOG_VERIFIER_BAD_CALL_INCOMPATIBLE_PARAMETER(name_getter(), reg_or_param, ToString(norm_actual_type),
                                                             ToString(norm_type));
                END_SHOW_MSG();
                SET_STATUS_FOR_MSG(BadCallIncompatibleParameter, WARNING);
                return result = false;
            }
            if (formal_type == Type::Bot()) {
                if (actual_type == Type::Bot()) {
                    LOG_VERIFIER_CALL_FORMAL_ACTUAL_BOTH_BOT_OR_TOP("Bot");
                    break;
                }

                SHOW_MSG(BadCallFormalIsBot)
                LOG_VERIFIER_BAD_CALL_FORMAL_IS_BOT(name_getter(), ToString(actual_type));
                END_SHOW_MSG();
                SET_STATUS_FOR_MSG(BadCallFormalIsBot, WARNING);
                return result = false;
            }
            if (formal_type == Type::Top()) {
                if (actual_type == Type::Top()) {
                    LOG_VERIFIER_CALL_FORMAL_ACTUAL_BOTH_BOT_OR_TOP("Top");
                    break;
                }
                SHOW_MSG(CallFormalTop)
                LOG_VERIFIER_CALL_FORMAL_TOP();
                END_SHOW_MSG();
                break;
            }
            if (IsSubtype(formal_type, primitive_, GetTypeSystem())) {
                // check implicit conversion of primitive types
                TypeId formal_id = formal_type.ToTypeId();
                CheckResult check_result = CheckResult::ok;

                if (!IsSubtype(actual_type, primitive_, GetTypeSystem())) {
                    result = false;
                    break;
                }
                // !!!!!! FIXME: need to check all possible TypeId-s against formal_id
                TypeId actual_id = actual_type.ToTypeId();
                if (actual_id != TypeId::INVALID) {
                    check_result = panda::verifier::CheckMethodArgs(formal_id, actual_id);
                } else {
                    // special case, where type after contexts LUB operation is inexact one, like
                    // integral32_Type()
                    if ((IsSubtype(formal_type, integral32_, GetTypeSystem()) &&
                         IsSubtype(actual_type, integral32_, GetTypeSystem())) ||
                        (IsSubtype(formal_type, integral64_, GetTypeSystem()) &&
                         IsSubtype(actual_type, integral64_, GetTypeSystem())) ||
                        (IsSubtype(formal_type, float64_, GetTypeSystem()) &&
                         IsSubtype(actual_type, float64_, GetTypeSystem()))) {
                        SHOW_MSG(CallFormalActualDifferent)
                        LOG_VERIFIER_CALL_FORMAL_ACTUAL_DIFFERENT(ToString(formal_type), ToString(actual_type));
                        END_SHOW_MSG();
                    } else {
                        check_result = panda::verifier::CheckMethodArgs(formal_id, actual_id);
                    }
                }
                if (!check_result.IsOk()) {
                    SHOW_MSG(DebugCallParameterTypes)
                    LogInnerMessage(check_result);
                    LOG_VERIFIER_DEBUG_CALL_PARAMETER_TYPES(
                        name_getter(),
                        (reg_num == INVALID_REG ? ""
                                                : PandaString {"Actual parameter in "} + RegisterName(reg_num) + ". "),
                        ToString(actual_type), ToString(formal_type));
                    END_SHOW_MSG();
                    status_ = check_result.status;
                    if (status_ == VerificationStatus::ERROR) {
                        result = false;
                        break;
                    }
                }
                continue;
            }
            if (!CheckType(actual_type, formal_type)) {
                if (reg_num == INVALID_REG) {
                    SHOW_MSG(BadCallWrongParameter)
                    LOG_VERIFIER_BAD_CALL_WRONG_PARAMETER(name_getter(), ToString(actual_type), ToString(formal_type));
                    END_SHOW_MSG();
                    SET_STATUS_FOR_MSG(BadCallWrongParameter, WARNING);
                } else {
                    SHOW_MSG(BadCallWrongRegister)
                    LOG_VERIFIER_BAD_CALL_WRONG_REGISTER(name_getter(), reg_num);
                    END_SHOW_MSG();
                    SET_STATUS_FOR_MSG(BadCallWrongRegister, WARNING);
                }
                if (!config->opts.debug.allow.wrong_subclassing_in_method_args) {
                    status_ = VerificationStatus::ERROR;
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckCall(Method const *method, Span<int> regs)
    {
        if (method == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveMethodId, OK);
            return false;
        }

        auto *plugin = job_->JobPlugin();
        auto const *job_method = job_->JobMethod();
        auto result = plugin->CheckMethodAccessViolation(method, job_method, GetTypeSystem());
        if (!result.IsOk()) {
            const auto &verif_opts = config->opts;
            if (verif_opts.debug.allow.method_access_violation && result.IsError()) {
                result.status = VerificationStatus::WARNING;
            }
            LogInnerMessage(result);
            LOG_VERIFIER_DEBUG_CALL_FROM_TO(job_->JobMethod()->GetFullName(), method->GetFullName());
            status_ = result.status;
            if (status_ == VerificationStatus::ERROR) {
                return false;
            }
        }

        const auto *method_sig = GetTypeSystem()->GetMethodSignature(method);
        auto method_name_getter = [method]() { return method->GetFullName(); };
        Type result_type = method_sig->result;

        if (!debug_ctx->SkipVerificationOfCall(method->GetUniqId()) &&
            !CheckMethodArgs(method_name_getter, method, regs)) {
            return false;
        }
        SetAcc(result_type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::array<int, 2> regs {vs1, vs2};
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallAccShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        auto acc_pos = static_cast<unsigned>(inst_.GetImm<FORMAT, 0x00>());
        static constexpr auto NUM_ARGS = 2;
        if (acc_pos >= NUM_ARGS) {
            LOG_VERIFIER_ACCUMULATOR_POSITION_IS_OUT_OF_RANGE();
            SET_STATUS_FOR_MSG(AccumulatorPositionIsOutOfRange, WARNING);
            return status_ != VerificationStatus::ERROR;
        }
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::array<int, NUM_ARGS> regs {};
        if (acc_pos == 0) {
            regs = {ACC, vs1};
        } else {
            regs = {vs1, ACC};
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCalliDynShort();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCalliDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCalliDynRange();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCall()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        uint16_t vs3 = inst_.GetVReg<FORMAT, 0x02>();
        uint16_t vs4 = inst_.GetVReg<FORMAT, 0x03>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::array<int, 4> regs {vs1, vs2, vs3, vs4};
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallAcc()
    {
        LOG_INST();
        DBGBRK();
        auto acc_pos = static_cast<unsigned>(inst_.GetImm<FORMAT, 0x0>());
        static constexpr auto NUM_ARGS = 4;
        if (acc_pos >= NUM_ARGS) {
            LOG_VERIFIER_ACCUMULATOR_POSITION_IS_OUT_OF_RANGE();
            SET_STATUS_FOR_MSG(AccumulatorPositionIsOutOfRange, WARNING);
            return status_ != VerificationStatus::ERROR;
        }
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::array<int, NUM_ARGS> regs {};
        auto reg_idx = 0;
        for (unsigned i = 0; i < NUM_ARGS; ++i) {
            if (i == acc_pos) {
                regs[i] = ACC;
            } else {
                regs[i] = static_cast<int>(inst_.GetVReg(reg_idx++));
            }
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::vector<int> regs;
        for (auto reg_idx = vs; ExecCtx().CurrentRegContext().IsRegDefined(reg_idx); reg_idx++) {
            regs.push_back(reg_idx);
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirtShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }

        Sync();
        std::array<int, 2> regs {vs1, vs2};
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirtAccShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        auto acc_pos = static_cast<unsigned>(inst_.GetImm<FORMAT, 0x00>());
        static constexpr auto NUM_ARGS = 2;
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }
        // if (method != nullptr && method->ptr == nullptr) {
        Sync();
        std::array<int, NUM_ARGS> regs {};
        if (acc_pos == 0) {
            regs = {ACC, vs1};
        } else {
            regs = {vs1, ACC};
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirt()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        uint16_t vs3 = inst_.GetVReg<FORMAT, 0x02>();
        uint16_t vs4 = inst_.GetVReg<FORMAT, 0x03>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }

        Sync();
        std::array<int, 4> regs {vs1, vs2, vs3, vs4};
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirtAcc()
    {
        LOG_INST();
        DBGBRK();
        auto acc_pos = static_cast<unsigned>(inst_.GetImm<FORMAT, 0x0>());
        static constexpr auto NUM_ARGS = 4;
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }
        // if (method != nullptr && method->ptr == nullptr) {
        Sync();
        std::array<int, NUM_ARGS> regs {};
        auto reg_idx = 0;
        for (unsigned i = 0; i < NUM_ARGS; ++i) {
            if (i == acc_pos) {
                regs[i] = ACC;
            } else {
                regs[i] = static_cast<int>(inst_.GetVReg(reg_idx++));
            }
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirtRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();

        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }

        Sync();
        std::vector<int> regs;
        for (auto reg_idx = vs; ExecCtx().CurrentRegContext().IsRegDefined(reg_idx); reg_idx++) {
            regs.push_back(reg_idx);
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleThrow()
    {
        LOG_INST();
        DBGBRK();
        ExecCtx().SetCheckPoint(inst_.GetAddress());
        Sync();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        if (!CheckRegType(vs, GetTypeSystem()->Throwable())) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        // possible implementation:
        // on stage of building checkpoints:
        // - add all catch block starts as checkpoints/entries
        // on absint stage:
        // - find corresponding catch block/checkpoint/entry
        // - add context to checkpoint/entry
        // - add entry to entry list
        // - stop absint
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMonitorenter()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type acc_type = GetAccType();
        if (acc_type == null_ref_type_) {
            // TODO(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpeAccumulator)
            LOG_VERIFIER_ALWAYS_NPE_ACCUMULATOR();
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(AlwaysNpeAccumulator, OK);
            return false;
        }
        if (!CheckType(acc_type, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMonitorexit()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type acc_type = GetAccType();
        if (acc_type == null_ref_type_) {
            // TODO(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpeAccumulator)
            LOG_VERIFIER_ALWAYS_NPE_ACCUMULATOR();
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(AlwaysNpeAccumulator, OK);
            return false;
        }
        if (!CheckType(acc_type, ref_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    BytecodeInstructionSafe GetInst()
    {
        return inst_;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallspecRange()
    {
        // CallspecRange has the same semantics as CallRange in terms of
        // input and output for verification
        return HandleCallRange<FORMAT>();
    }

    static PandaString RegisterName(int reg_idx, bool capitalize = false)
    {
        if (reg_idx == ACC) {
            return capitalize ? "Accumulator" : "accumulator";
        }
        return PandaString {capitalize ? "Register v" : "register v"} + NumToStr(reg_idx);
    }

private:
    Type GetCachedType() const
    {
        auto offset = inst_.GetOffset();
        if (!job_->IsTypePresentForOffset(offset)) {
            SHOW_MSG(CacheMissForClassAtOffset)
            LOG_VERIFIER_CACHE_MISS_FOR_CLASS_AT_OFFSET(offset);
            END_SHOW_MSG();

            SHOW_MSG(CannotResolveClassId)
            LOG_VERIFIER_CANNOT_RESOLVE_CLASS_ID(inst_.GetId().AsFileId().GetOffset());
            END_SHOW_MSG();
            return Type {};
        }
        return job_->GetCachedType(offset);
    }

    Method const *GetCachedMethod() const
    {
        auto offset = inst_.GetOffset();
        if (!job_->IsMethodPresentForOffset(offset)) {
            SHOW_MSG(CacheMissForMethodAtOffset)
            LOG_VERIFIER_CACHE_MISS_FOR_METHOD_AT_OFFSET(offset);
            END_SHOW_MSG();

            SHOW_MSG(CannotResolveMethodId)
            LOG_VERIFIER_CANNOT_RESOLVE_METHOD_ID(inst_.GetId().AsFileId().GetOffset());
            END_SHOW_MSG();
            return nullptr;
        }
        return job_->GetCachedMethod(offset);
    }

    Field const *GetCachedField() const
    {
        auto offset = inst_.GetOffset();
        if (!job_->IsFieldPresentForOffset(offset)) {
            SHOW_MSG(CacheMissForFieldAtOffset)
            LOG_VERIFIER_CACHE_MISS_FOR_FIELD_AT_OFFSET(offset);
            END_SHOW_MSG();

            SHOW_MSG(CannotResolveFieldId)
            LOG_VERIFIER_CANNOT_RESOLVE_FIELD_ID(inst_.GetId().AsFileId().GetOffset());
            END_SHOW_MSG();
            return nullptr;
        }
        return job_->GetCachedField(offset);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    void MoveToNextInst()
    {
        inst_ = inst_.GetNext<FORMAT>();
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckArrayStore(int v1, int v2, Type expected_elt_type)
    {
        /*
        main rules:
        1. instruction should be strict in array element size, so for primitive types type equality is used
        2. accumulator may be subtype of array element type (under question)
        */
        if (!CheckRegType(v2, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(v1, array_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        Type reg_type = GetRegType(v1);

        if (reg_type == null_ref_type_) {
            // TODO(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(v1);
            END_SHOW_MSG();
            SetAcc(top_);
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }

        auto arr_elt_type = reg_type.GetArrayElementType(GetTypeSystem());

        if (!IsSubtype(arr_elt_type, expected_elt_type, GetTypeSystem())) {
            SHOW_MSG(BadArrayElementType2)
            LOG_VERIFIER_BAD_ARRAY_ELEMENT_TYPE2(ToString(arr_elt_type), ToString(expected_elt_type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadArrayElementType2, WARNING);
            return false;
        }

        Type acc_type = GetAccType();

        // TODO(dvyadov): think of subtyping here. Can we really write more precise type into array?
        // since there is no problems with storage (all refs are of the same size)
        // and no problems with information losses, it seems fine at first sight.
        bool res = !IsSubtype(acc_type, arr_elt_type, GetTypeSystem());
        if (res) {
            // accumulator is of wrong type
            SHOW_MSG(BadAccumulatorType)
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE(ToString(acc_type), ToString(arr_elt_type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadAccumulatorType, WARNING);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckArrayStoreExact(int v1, int v2, Type acc_supertype, std::initializer_list<Type> const &expected_elt_types)
    {
        if (!CheckRegType(v2, integral32_) || !CheckRegType(v1, array_type_) || !IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        Type reg_type = GetRegType(v1);

        if (reg_type == null_ref_type_) {
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(v1);
            END_SHOW_MSG();
            SetAcc(top_);
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }

        auto arr_elt_type = reg_type.GetArrayElementType(GetTypeSystem());

        auto find = [&expected_elt_types](auto type) {
            for (Type t : expected_elt_types) {
                if (type == t) {
                    return true;
                }
            }
            return false;
        };

        if (!find(arr_elt_type)) {
            // array elt type is not expected one
            PandaVector<Type> expected_types_vec;
            for (auto et : expected_elt_types) {
                expected_types_vec.push_back(et);
            }
            LOG_VERIFIER_BAD_ARRAY_ELEMENT_TYPE3(ToString(arr_elt_type), ToString(expected_types_vec));
            SET_STATUS_FOR_MSG(BadArrayElementType, WARNING);
            return false;
        }

        Type acc_type = GetAccType();

        if (!IsSubtype(acc_type, acc_supertype, GetTypeSystem())) {
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE2(ToString(acc_type));
            SET_STATUS_FOR_MSG(BadArrayElementType, WARNING);
            return false;
        }

        if (!find(acc_type)) {
            // array elt type is not expected one
            PandaVector<Type> expected_types_vec;
            for (auto et : expected_elt_types) {
                expected_types_vec.push_back(et);
            }
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE3(ToString(acc_type), ToString(expected_types_vec));
            if (status_ != VerificationStatus::ERROR) {
                status_ = VerificationStatus::WARNING;
            }
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT, bool REG_DST = false>
    bool CheckBinaryOp2(Type acc_in, Type reg_in, Type out)
    {
        uint16_t vs;
        if constexpr (REG_DST) {
            vs = inst_.GetVReg<FORMAT, 0x01>();
        } else {
            vs = inst_.GetVReg<FORMAT>();
        }
        if (!CheckRegType(ACC, acc_in)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(vs, reg_in)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if constexpr (REG_DST) {
            SetReg(inst_.GetVReg<FORMAT, 0x00>(), out);
        } else {
            SetAcc(out);
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT, bool REG_DST = false>
    bool CheckBinaryOp2(Type acc_in, Type reg_in)
    {
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        return CheckBinaryOp2<FORMAT, REG_DST>(acc_in, reg_in, GetAccType());
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckBinaryOpImm(Type in, Type out)
    {
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        if (!CheckRegType(vs, in)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetReg(vd, out);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckBinaryOp2Imm(Type acc_in, Type acc_out)
    {
        if (!CheckRegType(ACC, acc_in)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(acc_out);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckBinaryOp2Imm(Type acc_in)
    {
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        return CheckBinaryOp2Imm<FORMAT>(acc_in, GetAccType());
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckUnaryOp(Type acc_in, Type acc_out)
    {
        if (!CheckRegType(ACC, acc_in)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(acc_out);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckUnaryOp(Type acc_in)
    {
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        return CheckUnaryOp<FORMAT>(acc_in, GetAccType());
    }

    template <BytecodeInstructionSafe::Format FORMAT, bool REG_DST = false>
    bool CheckBinaryOp(Type v1_in, Type v2_in, Type out)
    {
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        if (!CheckRegType(v1, v1_in)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(v2, v2_in)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if constexpr (REG_DST) {
            SetReg(v1, out);
        } else {
            SetAcc(out);
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckBinaryOp(Type vs1_in, Type vs2_in)
    {
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        return CheckBinaryOp<FORMAT>(vs1_in, vs2_in, GetAccType());
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleConversion(Type from, Type to)
    {
        if (!CheckRegType(ACC, from)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(to);
        MoveToNextInst<FORMAT>();
        return true;
    }

    bool IsConcreteArrayType(Type type)
    {
        return IsSubtype(type, array_type_, GetTypeSystem()) && type != array_type_;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckArrayLoad(int vs, std::initializer_list<Type> const &expected_elt_types)
    {
        if (!CheckRegType(ACC, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(vs, array_type_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        Type reg_type = GetRegType(vs);
        if (reg_type == null_ref_type_) {
            // TODO(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(vs);
            END_SHOW_MSG();
            SetAcc(top_);
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }
        auto &&arr_elt_type = reg_type.GetArrayElementType(GetTypeSystem());
        auto find = [&expected_elt_types](auto type) {
            for (Type t : expected_elt_types) {
                if (type == t) {
                    return true;
                }
            }
            return false;
        };
        auto res = find(arr_elt_type);
        if (!res) {
            PandaVector<Type> expected_types_vec;
            for (auto et : expected_elt_types) {
                expected_types_vec.push_back(et);
            }
            LOG_VERIFIER_BAD_ARRAY_ELEMENT_TYPE3(ToString(arr_elt_type), ToString(expected_types_vec));
            SET_STATUS_FOR_MSG(BadArrayElementType, WARNING);
            return false;
        }
        SetAcc(arr_elt_type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    bool ProcessBranching(int32_t offset)
    {
        auto new_inst = inst_.JumpTo(offset);
        const uint8_t *target = new_inst.GetAddress();
        if (!context_.CflowInfo().IsAddrValid(target) ||
            !context_.CflowInfo().IsFlagSet(target, CflowMethodInfo::INSTRUCTION)) {
            LOG_VERIFIER_INCORRECT_JUMP();
            status_ = VerificationStatus::ERROR;
            return false;
        }

#ifndef NDEBUG
        ExecCtx().ProcessJump(
            inst_.GetAddress(), target,
            [this, print_hdr = true](int reg_idx, const auto &src_reg, const auto &dst_reg) mutable {
                if (print_hdr) {
                    LOG_VERIFIER_REGISTER_CONFLICT_HEADER();
                    print_hdr = false;
                }
                LOG_VERIFIER_REGISTER_CONFLICT(RegisterName(reg_idx), ToString(src_reg.GetAbstractType()),
                                               ToString(dst_reg.GetAbstractType()));
                return true;
            },
            code_type_);
#else
        ExecCtx().ProcessJump(inst_.GetAddress(), target, code_type_);
#endif
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT, template <typename OpT> class Op>
    bool HandleCondJmpz()
    {
        auto imm = inst_.GetImm<FORMAT>();

        if (!CheckRegType(ACC, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT, template <typename OpT> class Op>
    bool HandleCondJmp()
    {
        auto imm = inst_.GetImm<FORMAT>();
        uint16_t vs = inst_.GetVReg<FORMAT>();

        if (!CheckRegType(ACC, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckRegType(vs, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckArrayCtor(Type klass, Span<int> reg_nums)
    {
        if (!klass.IsConsistent() || !klass.IsClass() || !klass.GetClass()->IsArrayClass()) {
            return false;
        }
        auto args_num = GetArrayNumDimensions(klass.GetClass());
        bool result = false;
        for (auto reg : reg_nums) {
            if (!IsRegDefined(reg)) {
                SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
                result = false;
                break;
            }
            result = CheckRegType(reg, i32_);
            if (!result) {
                LOG(ERROR, VERIFIER) << "Verifier error: ArrayCtor type error";
                status_ = VerificationStatus::ERROR;
                break;
            }
            --args_num;
            if (args_num == 0) {
                break;
            }
        };
        if (result && args_num > 0) {
            SHOW_MSG(TooFewArrayConstructorArgs)
            LOG_VERIFIER_TOO_FEW_ARRAY_CONSTRUCTOR_ARGS(args_num);
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(TooFewArrayConstructorArgs, WARNING);
            result = false;
        }
        if (result) {
            SetAcc(klass);
            MoveToNextInst<FORMAT>();
        }
        return result;
    }

    void LogInnerMessage(const CheckResult &elt)
    {
        if (elt.IsError()) {
            LOG(ERROR, VERIFIER) << "Error: " << elt.msg << ". ";
        } else if (elt.IsWarning()) {
            LOG(WARNING, VERIFIER) << "Warning: " << elt.msg << ". ";
        }
    }

    size_t GetArrayNumDimensions(Class const *klass)
    {
        size_t res = 0;
        while (klass->IsArrayClass()) {
            res++;
            klass = klass->GetComponentType();
        }
        return res;
    }

private:
    BytecodeInstructionSafe inst_;
    VerificationContext &context_;
    VerificationStatus status_ {VerificationStatus::OK};
    // #ifndef NDEBUG
    bool debug_ {false};
    uint32_t debug_offset_ {0};
    // #endif
    EntryPointType code_type_;

    void SetStatusAtLeast(VerificationStatus new_status)
    {
        status_ = std::max(status_, new_status);
    }

    static inline VerificationStatus MsgClassToStatus(MethodOption::MsgClass msg_class)
    {
        switch (msg_class) {
            case MethodOption::MsgClass::HIDDEN:
                return VerificationStatus::OK;
            case MethodOption::MsgClass::WARNING:
                return VerificationStatus::WARNING;
            case MethodOption::MsgClass::ERROR:
                return VerificationStatus::ERROR;
            default:
                UNREACHABLE();
        }
    }

    static PandaString GetFieldName(Field const *field)
    {
        return PandaString {field->GetClass()->GetName()} + "." + utf::Mutf8AsCString(field->GetName().data);
    }
};
}  // namespace panda::verifier

#endif  // PANDA_VERIFICATION_ABSINT_ABS_INT_INL_H
