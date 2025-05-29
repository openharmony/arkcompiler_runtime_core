/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

static ark::compiler::CallInst *CastToCallLaunch(ark::compiler::Inst *inst)
{
    switch (inst->GetOpcode()) {
        case compiler::Opcode::CallLaunchStatic:
            return inst->CastToCallLaunchStatic();
        case compiler::Opcode::CallLaunchVirtual:
            return inst->CastToCallLaunchVirtual();
        default:
            UNREACHABLE();
    }
}

static ark::pandasm::Opcode ChooseCallLaunchOpcode(ark::compiler::Opcode op, size_t nargs)
{
    ASSERT(op == compiler::Opcode::CallLaunchStatic || op == compiler::Opcode::CallLaunchVirtual);
    if (nargs > MAX_NUM_NON_RANGE_ARGS) {
        switch (op) {
            case compiler::Opcode::CallLaunchStatic:
                return pandasm::Opcode::ETS_LAUNCH_RANGE;
            case compiler::Opcode::CallLaunchVirtual:
                return pandasm::Opcode::ETS_LAUNCH_VIRT_RANGE;
            default:
                UNREACHABLE();
        }
    } else if (nargs > MAX_NUM_SHORT_CALL_ARGS) {
        switch (op) {
            case compiler::Opcode::CallLaunchStatic:
                return pandasm::Opcode::ETS_LAUNCH;
            case compiler::Opcode::CallLaunchVirtual:
                return pandasm::Opcode::ETS_LAUNCH_VIRT;
            default:
                UNREACHABLE();
        }
    }
    switch (op) {
        case compiler::Opcode::CallLaunchStatic:
            return pandasm::Opcode::ETS_LAUNCH_SHORT;
        case compiler::Opcode::CallLaunchVirtual:
            return pandasm::Opcode::ETS_LAUNCH_VIRT_SHORT;
        default:
            UNREACHABLE();
    }
}

static void CallLaunchHandler(ark::compiler::GraphVisitor *visitor, ark::compiler::Inst *inst)
{
    auto op = inst->GetOpcode();
    ASSERT(op == ark::compiler::Opcode::CallLaunchStatic || op == ark::compiler::Opcode::CallLaunchVirtual);
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto callInst = CastToCallLaunch(inst);
    auto sfCount = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);
    size_t start = 1;
    auto nargs = sfCount - start;
    pandasm::Ins ins;
    ins.opcode = ChooseCallLaunchOpcode(op, nargs);

    auto zeroArg = inst->GetInput(0).GetInst();
    ASSERT(zeroArg->GetOpcode() == Opcode::NewObject);
    auto newobjInst = zeroArg->CastToNewObject();

    if (nargs > MAX_NUM_NON_RANGE_ARGS) {
#ifndef NDEBUG
        auto startReg = inst->GetSrcReg(start);
        ASSERT(startReg <= MAX_8_BIT_REG);
        for (size_t i = start; i < sfCount; ++i) {
            auto reg = inst->GetSrcReg(i);
            ASSERT(reg - startReg == static_cast<int>(i - start));  // check 'range-ness' of registers
        }
#endif  // !NDEBUG
        ins.regs.emplace_back(inst->GetSrcReg(start));
    } else {
        for (size_t i = start; i < sfCount; ++i) {
            auto reg = inst->GetSrcReg(i);
            ASSERT(reg < NUM_COMPACTLY_ENCODED_REGS);
            ins.regs.emplace_back(reg);
        }
    }
    ins.ids.emplace_back(enc->irInterface_->GetMethodIdByOffset(callInst->GetCallMethodId()));
    enc->result_.emplace_back(ins);
    if (newobjInst->GetDstReg() != compiler::GetAccReg()) {
        enc->result_.emplace_back(pandasm::Create_STA_OBJ(newobjInst->GetDstReg()));
    }
}

static void VisitCallLaunchStatic(ark::compiler::GraphVisitor *visitor, ark::compiler::Inst *inst)
{
    CallLaunchHandler(visitor, inst);
}

static void VisitCallLaunchVirtual(GraphVisitor *visitor, Inst *inst)
{
    CallLaunchHandler(visitor, inst);
}

static void VisitEtsLdObjByName(BytecodeGen *enc, compiler::IntrinsicInst *inst)
{
    auto v0 = inst->GetSrcReg(0);
    auto bcId0 = enc->irInterface_->GetFieldIdByOffset(static_cast<uint32_t>(inst->GetImms()[0]));
    switch (inst->GetIntrinsicId()) {
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_I32:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_F32:
            enc->result_.emplace_back(pandasm::Create_ETS_LDOBJ_NAME(v0, bcId0));
            DoSta(inst->GetDstReg(), enc->result_);
            break;
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_I64:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_F64:
            enc->result_.emplace_back(pandasm::Create_ETS_LDOBJ_NAME_64(v0, bcId0));
            DoSta64(inst->GetDstReg(), enc->result_);
            break;
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LD_OBJ_BY_NAME_OBJ:
            enc->result_.emplace_back(pandasm::Create_ETS_LDOBJ_NAME_OBJ(v0, bcId0));
            DoStaObj(inst->GetDstReg(), enc->result_);
            break;
        default:
            UNREACHABLE();
    }
}

static void VisitEtsStObjByName(BytecodeGen *enc, compiler::IntrinsicInst *inst)
{
    auto v0 = inst->GetSrcReg(0);
    auto bcId0 = enc->irInterface_->GetFieldIdByOffset(static_cast<uint32_t>(inst->GetImms()[0]));
    switch (inst->GetIntrinsicId()) {
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I32:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I16:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I8:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_F32:
            DoLda(inst->GetSrcReg(1U), enc->result_);
            enc->result_.emplace_back(pandasm::Create_ETS_STOBJ_NAME(v0, bcId0));
            break;
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_I64:
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_F64:
            DoLda64(inst->GetSrcReg(1U), enc->result_);
            enc->result_.emplace_back(pandasm::Create_ETS_STOBJ_NAME_64(v0, bcId0));
            break;
        case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ST_OBJ_BY_NAME_OBJ:
            DoLdaObj(inst->GetSrcReg(1U), enc->result_);
            enc->result_.emplace_back(pandasm::Create_ETS_STOBJ_NAME_OBJ(v0, bcId0));
            break;
        default:
            UNREACHABLE();
    }
}

static void VisitLoadUniqueObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    if (inst->GetDstReg() == compiler::GetAccReg()) {
        enc->result_.emplace_back(pandasm::Create_ETS_LDNULLVALUE());
    } else {
        enc->result_.emplace_back(pandasm::Create_ETS_MOVNULLVALUE(inst->GetDstReg()));
    }
}

template <bool IS_STRICT>
static void VisitEtsEquals(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto v0 = inst->GetSrcReg(0);
    auto v1 = inst->GetSrcReg(1);
    if constexpr (IS_STRICT) {
        enc->result_.emplace_back(pandasm::Create_ETS_STRICTEQUALS(v0, v1));
    } else {
        enc->result_.emplace_back(pandasm::Create_ETS_EQUALS(v0, v1));
    }
    DoSta(inst->GetDstReg(), enc->result_);
}

static void VisitEtsTypeof(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto v0 = inst->GetSrcReg(0);
    enc->result_.emplace_back(pandasm::Create_ETS_TYPEOF(v0));
    DoStaObj(inst->GetDstReg(), enc->result_);
}

static void VisitEtsIstrue(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto v = inst->GetSrcReg(0);
    enc->result_.emplace_back(pandasm::Create_ETS_ISTRUE(v));
    DoSta(inst->GetDstReg(), enc->result_);
}