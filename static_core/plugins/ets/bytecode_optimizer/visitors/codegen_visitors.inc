static panda::compiler::CallInst *CastToCallLaunch(panda::compiler::Inst *inst)
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

static panda::pandasm::Opcode ChooseCallLaunchOpcode(panda::compiler::Opcode op, size_t nargs)
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

static void CallLaunchHandler(panda::compiler::GraphVisitor *visitor, panda::compiler::Inst *inst)
{
    auto op = inst->GetOpcode();
    ASSERT(op == panda::compiler::Opcode::CallLaunchStatic || op == panda::compiler::Opcode::CallLaunchVirtual);
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto call_inst = CastToCallLaunch(inst);
    auto sf_count = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);
    size_t start = 1;
    auto nargs = sf_count - start;
    pandasm::Ins ins;
    ins.opcode = ChooseCallLaunchOpcode(op, nargs);

    auto zero_arg = inst->GetInput(0).GetInst();
    ASSERT(zero_arg->GetOpcode() == Opcode::NewObject);
    auto newobj_inst = zero_arg->CastToNewObject();

    if (nargs > MAX_NUM_NON_RANGE_ARGS) {
#ifndef NDEBUG
        auto start_reg = inst->GetSrcReg(start);
        ASSERT(start_reg <= MAX_8_BIT_REG);
        for (size_t i = start; i < sf_count; ++i) {
            auto reg = inst->GetSrcReg(i);
            ASSERT(reg - start_reg == static_cast<int>(i - start));  // check 'range-ness' of registers
        }
#endif  // !NDEBUG
        ins.regs.emplace_back(inst->GetSrcReg(start));
    } else {
        for (size_t i = start; i < sf_count; ++i) {
            auto reg = inst->GetSrcReg(i);
            ASSERT(reg < NUM_COMPACTLY_ENCODED_REGS);
            ins.regs.emplace_back(reg);
        }
    }
    ins.ids.emplace_back(enc->ir_interface_->GetMethodIdByOffset(call_inst->GetCallMethodId()));
    enc->result_.emplace_back(ins);
    if (newobj_inst->GetDstReg() != compiler::ACC_REG_ID) {
        enc->result_.emplace_back(pandasm::Create_STA_OBJ(newobj_inst->GetDstReg()));
    }
}

static void VisitCallLaunchStatic(panda::compiler::GraphVisitor *visitor, panda::compiler::Inst *inst)
{
    CallLaunchHandler(visitor, inst);
}

static void VisitCallLaunchVirtual(GraphVisitor *visitor, Inst *inst)
{
    CallLaunchHandler(visitor, inst);
}