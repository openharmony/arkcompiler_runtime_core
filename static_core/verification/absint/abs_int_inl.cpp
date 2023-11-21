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

#include "abs_int_inl.h"
#include "handle_gen.h"

namespace panda::verifier {

bool AbsIntInstructionHandler::IsRegDefined(int reg)
{
    bool is_defined = ExecCtx().CurrentRegContext().IsRegDefined(reg);
#ifndef NDEBUG
    if (!is_defined) {
        if (!ExecCtx().CurrentRegContext().WasConflictOnReg(reg)) {
            SHOW_MSG(UndefinedRegister)
            LOG_VERIFIER_UNDEFINED_REGISTER(RegisterName(reg, true));
            END_SHOW_MSG();
        } else {
            SHOW_MSG(RegisterTypeConflict)
            LOG_VERIFIER_REGISTER_TYPE_CONFLICT(RegisterName(reg, false));
            END_SHOW_MSG();
        }
    }
#endif
    return is_defined;
}

bool AbsIntInstructionHandler::CheckRegType(int reg, Type tgt_type)
{
    if (!IsRegDefined(reg)) {
        return false;
    }
    auto type = GetRegType(reg);

    if (CheckType(type, tgt_type)) {
        return true;
    }

    SHOW_MSG(BadRegisterType)
    LOG_VERIFIER_BAD_REGISTER_TYPE(RegisterName(reg, true), ToString(type), ToString(tgt_type));
    END_SHOW_MSG();
    return false;
}

const AbstractTypedValue &AbsIntInstructionHandler::GetReg(int reg_idx)
{
    return context_.ExecCtx().CurrentRegContext()[reg_idx];
}

Type AbsIntInstructionHandler::GetRegType(int reg_idx)
{
    return GetReg(reg_idx).GetAbstractType();
}

void AbsIntInstructionHandler::SetReg(int reg_idx, const AbstractTypedValue &val)
{
    if (job_->Options().ShowRegChanges()) {
        PandaString prev_atv_image {"<none>"};
        if (ExecCtx().CurrentRegContext().IsRegDefined(reg_idx)) {
            prev_atv_image = ToString(&GetReg(reg_idx));
        }
        auto new_atv_image = ToString(&val);
        LOG_VERIFIER_DEBUG_REGISTER_CHANGED(RegisterName(reg_idx), prev_atv_image, new_atv_image);
    }
    // RegContext extends flexibly for any number of registers, though on AbsInt instruction
    // handling stage it is not a time to add a new register! The regsiter set (vregs and aregs)
    // is normally initialized on PrepareVerificationContext.
    if (!context_.ExecCtx().CurrentRegContext().IsValid(reg_idx)) {
        auto const *method = job_->JobMethod();
        auto num_vregs = method->GetNumVregs();
        auto num_aregs = GetTypeSystem()->GetMethodSignature(method)->args.size();
        if (reg_idx > (int)(num_vregs + num_aregs)) {
            LOG_VERIFIER_UNDEFINED_REGISTER(RegisterName(reg_idx, true));
            SET_STATUS_FOR_MSG(UndefinedRegister, ERROR);
        }
    }
    context_.ExecCtx().CurrentRegContext()[reg_idx] = val;
}

void AbsIntInstructionHandler::SetReg(int reg_idx, Type type)
{
    SetReg(reg_idx, MkVal(type));
}

void AbsIntInstructionHandler::SetRegAndOthersOfSameOrigin(int reg_idx, const AbstractTypedValue &val)
{
    context_.ExecCtx().CurrentRegContext().ChangeValuesOfSameOrigin(reg_idx, val);
}

void AbsIntInstructionHandler::SetRegAndOthersOfSameOrigin(int reg_idx, Type type)
{
    SetRegAndOthersOfSameOrigin(reg_idx, MkVal(type));
}

const AbstractTypedValue &AbsIntInstructionHandler::GetAcc()
{
    return context_.ExecCtx().CurrentRegContext()[ACC];
}

Type AbsIntInstructionHandler::GetAccType()
{
    return GetAcc().GetAbstractType();
}

void AbsIntInstructionHandler::SetAcc(const AbstractTypedValue &val)
{
    SetReg(ACC, val);
}

void AbsIntInstructionHandler::SetAcc(Type type)
{
    SetReg(ACC, type);
}

void AbsIntInstructionHandler::SetAccAndOthersOfSameOrigin(const AbstractTypedValue &val)
{
    SetRegAndOthersOfSameOrigin(ACC, val);
}

void AbsIntInstructionHandler::SetAccAndOthersOfSameOrigin(Type type)
{
    SetRegAndOthersOfSameOrigin(ACC, type);
}

AbstractTypedValue AbsIntInstructionHandler::MkVal(Type t)
{
    return AbstractTypedValue {t, context_.NewVar(), GetInst()};
}

Type AbsIntInstructionHandler::TypeOfClass(Class const *klass)
{
    return Type {klass};
}

Type AbsIntInstructionHandler::ReturnType()
{
    return context_.ReturnType();
}

ExecContext &AbsIntInstructionHandler::ExecCtx()
{
    return context_.ExecCtx();
}

void AbsIntInstructionHandler::DumpRegs(const RegContext &ctx)
{
    LOG_VERIFIER_DEBUG_REGISTERS("registers =", ctx.DumpRegs(GetTypeSystem()));
}

void AbsIntInstructionHandler::Sync()
{
    auto addr = inst_.GetAddress();
    ExecContext &exec_ctx = ExecCtx();
    // NOTE(vdyadov): add verification options to show current context and contexts diff in case of incompatibility
#ifndef NDEBUG
    exec_ctx.StoreCurrentRegContextForAddr(
        addr, [this, print_hdr = true](int reg_idx, const auto &src, const auto &dst) mutable {
            if (print_hdr) {
                LOG_VERIFIER_REGISTER_CONFLICT_HEADER();
                print_hdr = false;
            }
            LOG_VERIFIER_REGISTER_CONFLICT(RegisterName(reg_idx), ToString(src.GetAbstractType()),
                                           ToString(dst.GetAbstractType()));
            return true;
        });
#else
    exec_ctx.StoreCurrentRegContextForAddr(addr);
#endif
}

}  // namespace panda::verifier
