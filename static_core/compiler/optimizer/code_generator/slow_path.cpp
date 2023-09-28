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

#include "slow_path.h"
#include "codegen.h"

namespace panda::compiler {

void SlowPathBase::Generate(Codegen *codegen)
{
    ASSERT(!generated_);

    SCOPED_DISASM_STR(codegen, std::string("SlowPath for inst ") + std::to_string(GetInst()->GetId()) + ". " +
                                   GetInst()->GetOpcodeStr());
    Encoder *encoder = codegen->GetEncoder();
    ASSERT(encoder->IsValid());
    encoder->BindLabel(GetLabel());

    GenerateImpl(codegen);

    if (encoder->IsLabelValid(label_back_)) {
        codegen->GetEncoder()->EncodeJump(GetBackLabel());
    }
#ifndef NDEBUG
    generated_ = true;
#endif
}

// ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION, STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION
bool SlowPathEntrypoint::GenerateThrowOutOfBoundsException(Codegen *codegen)
{
    auto len_reg = codegen->ConvertRegister(GetInst()->GetSrcReg(0), GetInst()->GetInputType(0));
    if (GetInst()->GetOpcode() == Opcode::BoundsCheckI) {
        ScopedTmpReg index_reg(codegen->GetEncoder());
        codegen->GetEncoder()->EncodeMov(index_reg, Imm(GetInst()->CastToBoundsCheckI()->GetImm()));
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), index_reg, len_reg);
    } else {
        ASSERT(GetInst()->GetOpcode() == Opcode::BoundsCheck);
        auto index_reg = codegen->ConvertRegister(GetInst()->GetSrcReg(1), GetInst()->GetInputType(1));
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), index_reg, len_reg);
    }
    return true;
}

// INITIALIZE_CLASS
bool SlowPathEntrypoint::GenerateInitializeClass(Codegen *codegen)
{
    auto inst = GetInst();
    if (GetInst()->GetDstReg() != INVALID_REG) {
        ASSERT(inst->GetOpcode() == Opcode::LoadAndInitClass);
        Reg klass_reg {codegen->ConvertRegister(GetInst()->GetDstReg(), DataType::REFERENCE)};
        RegMask preserved_regs;
        codegen->GetEncoder()->SetRegister(&preserved_regs, nullptr, klass_reg);
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, preserved_regs, klass_reg);
    } else {
        ASSERT(inst->GetOpcode() == Opcode::InitClass);
        ASSERT(!codegen->GetGraph()->IsAotMode());
        // check uintptr_t for cross:
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToInitClass()->GetClass());
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), TypedImm(klass));
    }
    return true;
}

// IS_INSTANCE
bool SlowPathEntrypoint::GenerateIsInstance(Codegen *codegen)
{
    auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto klass = codegen->ConvertRegister(GetInst()->GetSrcReg(1), DataType::REFERENCE);
    auto dst = codegen->ConvertRegister(GetInst()->GetDstReg(), GetInst()->GetType());
    codegen->CallRuntime(GetInst(), EntrypointId::IS_INSTANCE, dst, RegMask::GetZeroMask(), src, klass);
    return true;
}

// CHECK_CAST
bool SlowPathEntrypoint::GenerateCheckCast(Codegen *codegen)
{
    auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto klass = codegen->ConvertRegister(GetInst()->GetSrcReg(1), DataType::REFERENCE);
    codegen->CallRuntime(GetInst(), EntrypointId::CHECK_CAST, INVALID_REGISTER, RegMask::GetZeroMask(), src, klass);
    return true;
}

// CREATE_OBJECT
bool SlowPathEntrypoint::GenerateCreateObject(Codegen *codegen)
{
    auto inst = GetInst();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src = codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    codegen->CallRuntime(inst, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);

    return true;
}

bool SlowPathEntrypoint::GenerateByEntry(Codegen *codegen)
{
    switch (GetEntrypoint()) {
        case EntrypointId::THROW_EXCEPTION: {
            auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::Type::REFERENCE);
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), src);
            return true;
        }
        case EntrypointId::NULL_POINTER_EXCEPTION:
        case EntrypointId::ARITHMETIC_EXCEPTION:
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {});
            return true;
        case EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION:
        case EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION:
            return GenerateThrowOutOfBoundsException(codegen);
        case EntrypointId::NEGATIVE_ARRAY_SIZE_EXCEPTION: {
            auto size = codegen->ConvertRegister(GetInst()->GetSrcReg(0), GetInst()->GetInputType(0));
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), size);
            return true;
        }
        case EntrypointId::INITIALIZE_CLASS:
            return GenerateInitializeClass(codegen);
        case EntrypointId::IS_INSTANCE:
            return GenerateIsInstance(codegen);
        case EntrypointId::CHECK_CAST:
            return GenerateCheckCast(codegen);
        case EntrypointId::CREATE_OBJECT_BY_CLASS:
            return GenerateCreateObject(codegen);
        case EntrypointId::SAFEPOINT:
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {});
            return true;
        default:
            return false;
    }
}

void SlowPathEntrypoint::GenerateImpl(Codegen *codegen)
{
    if (!GenerateByEntry(codegen)) {
        switch (GetEntrypoint()) {
            case EntrypointId::GET_UNKNOWN_CALLEE_METHOD:
            case EntrypointId::RESOLVE_UNKNOWN_VIRTUAL_CALL:
            case EntrypointId::GET_FIELD_OFFSET:
            case EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS:
            case EntrypointId::RESOLVE_CLASS_OBJECT:
            case EntrypointId::RESOLVE_CLASS:
            case EntrypointId::ABSTRACT_METHOD_ERROR:
            case EntrypointId::INITIALIZE_CLASS_BY_ID:
            case EntrypointId::CHECK_STORE_ARRAY_REFERENCE:
            case EntrypointId::RESOLVE_STRING_AOT:
            case EntrypointId::CLASS_CAST_EXCEPTION:
                break;
            default:
                LOG(FATAL, COMPILER) << "Unsupported entrypoint!";
                UNREACHABLE();
                break;
        }
    }
}

void SlowPathDeoptimize::GenerateImpl(Codegen *codegen)
{
    uintptr_t value =
        helpers::ToUnderlying(deoptimize_type_) | (GetInst()->GetId() << MinimumBitsToStore(DeoptimizeType::COUNT));
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), TypedImm(value));
}

void SlowPathIntrinsic::GenerateImpl(Codegen *codegen)
{
    codegen->CreateCallIntrinsic(GetInst()->CastToIntrinsic());
}

void SlowPathImplicitNullCheck::GenerateImpl(Codegen *codegen)
{
    ASSERT(!GetInst()->CastToNullCheck()->IsImplicit());
    SlowPathEntrypoint::GenerateImpl(codegen);
}

void SlowPathShared::GenerateImpl(Codegen *codegen)
{
    ASSERT(tmp_reg_ != INVALID_REGISTER);
    [[maybe_unused]] ScopedTmpReg tmp_reg(codegen->GetEncoder(), tmp_reg_);
    ASSERT(tmp_reg.GetReg().GetId() == tmp_reg_.GetId());
    auto graph = codegen->GetGraph();
    ASSERT(graph->IsAotMode());
    auto aot_data = graph->GetAotData();
    aot_data->SetSharedSlowPathOffset(GetEntrypoint(), codegen->GetEncoder()->GetCursorOffset());
    MemRef entry(codegen->ThreadReg(), graph->GetRuntime()->GetEntrypointTlsOffset(graph->GetArch(), GetEntrypoint()));
    ScopedTmpReg tmp1_reg(codegen->GetEncoder());
    codegen->GetEncoder()->EncodeLdr(tmp1_reg, false, entry);
    codegen->GetEncoder()->EncodeJump(tmp1_reg);
}

void SlowPathResolveStringAot::GenerateImpl(Codegen *codegen)
{
    ScopedTmpRegU64 tmp_addr_reg(codegen->GetEncoder());
    // Slot address was loaded into temporary register before we jumped into slow path, but it is already released
    // because temporary registers are scoped. Try to allocate a new one and check that it is the same register
    // as was allocated in codegen. If it is a different register then copy the slot address into it.
    if (tmp_addr_reg.GetReg() != addr_reg_) {
        codegen->GetEncoder()->EncodeMov(tmp_addr_reg, addr_reg_);
    }
    codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), dst_reg_, TypedImm(string_id_), tmp_addr_reg);
}

void SlowPathRefCheck::GenerateImpl(Codegen *codegen)
{
    ASSERT(array_reg_ != INVALID_REGISTER);
    ASSERT(ref_reg_ != INVALID_REGISTER);
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), array_reg_, ref_reg_);
}

void SlowPathAbstract::GenerateImpl(Codegen *codegen)
{
    SCOPED_DISASM_STR(codegen, std::string("SlowPath for Abstract method ") + std::to_string(GetInst()->GetId()));
    ASSERT(method_reg_ != INVALID_REGISTER);
    ScopedTmpReg method_reg(codegen->GetEncoder(), method_reg_);
    ASSERT(method_reg.GetReg().GetId() == method_reg_.GetId());
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), method_reg.GetReg());
}

void SlowPathCheckCast::GenerateImpl(Codegen *codegen)
{
    SCOPED_DISASM_STR(codegen, std::string("SlowPath for CheckCast exception") + std::to_string(GetInst()->GetId()));
    auto inst = GetInst();
    auto src = codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), class_reg_, src);
}

void SlowPathUnresolved::GenerateImpl(Codegen *codegen)
{
    SlowPathEntrypoint::GenerateImpl(codegen);

    ASSERT(method_ != nullptr);
    ASSERT(type_id_ != 0);
    ASSERT(slot_addr_ != 0);
    auto type_imm = TypedImm(type_id_);
    auto arch = codegen->GetGraph()->GetArch();
    // On 32-bit architecture slot address requires additional down_cast,
    // similar to `method` address processing in `CallRuntimeWithMethod`
    auto slot_imm = Is64BitsArch(arch) ? TypedImm(slot_addr_) : TypedImm(down_cast<uint32_t>(slot_addr_));

    ScopedTmpReg value_reg(codegen->GetEncoder());
    if (GetInst()->GetOpcode() == Opcode::ResolveVirtual) {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), value_reg, arg_reg_, type_imm, slot_imm);
    } else if (GetEntrypoint() == EntrypointId::GET_UNKNOWN_CALLEE_METHOD ||
               GetEntrypoint() == EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS) {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), value_reg, type_imm, slot_imm);
    } else {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), value_reg, type_imm);

        ScopedTmpReg addr_reg(codegen->GetEncoder());
        codegen->GetEncoder()->EncodeMov(addr_reg, Imm(slot_addr_));
        codegen->GetEncoder()->EncodeStr(value_reg, MemRef(addr_reg));
    }

    if (dst_reg_.IsValid()) {
        codegen->GetEncoder()->EncodeMov(dst_reg_, value_reg);
    }
}

void SlowPathJsCastDoubleToInt32::GenerateImpl(Codegen *codegen)
{
    ASSERT(dst_reg_.IsValid());
    ASSERT(src_reg_.IsValid());

    auto enc {codegen->GetEncoder()};
    if (codegen->GetGraph()->GetMode().SupportManagedCode()) {
        ScopedTmpRegU64 tmp(enc);
        enc->EncodeMov(tmp, src_reg_);
        codegen->CallRuntime(GetInst(), EntrypointId::JS_CAST_DOUBLE_TO_INT32, dst_reg_, RegMask::GetZeroMask(), tmp);
        return;
    }

    auto [live_regs, live_vregs] {codegen->GetLiveRegisters<true>(GetInst())};
    live_regs.Reset(dst_reg_.GetId());

    codegen->SaveCallerRegisters(live_regs, live_vregs, true);
    codegen->FillCallParams(src_reg_);
    codegen->EmitCallRuntimeCode(nullptr, EntrypointId::JS_CAST_DOUBLE_TO_INT32_NO_BRIDGE);

    auto ret_reg {codegen->GetTarget().GetReturnReg(dst_reg_.GetType())};
    if (dst_reg_.GetId() != ret_reg.GetId()) {
        enc->EncodeMov(dst_reg_, ret_reg);
    }
    codegen->LoadCallerRegisters(live_regs, live_vregs, true);
}

}  // namespace panda::compiler
