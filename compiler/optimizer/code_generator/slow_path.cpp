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
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {index_reg, len_reg});
    } else {
        ASSERT(GetInst()->GetOpcode() == Opcode::BoundsCheck);
        auto index_reg = codegen->ConvertRegister(GetInst()->GetSrcReg(1), GetInst()->GetInputType(1));
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {index_reg, len_reg});
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
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {klass_reg}, preserved_regs);
    } else {
        ASSERT(inst->GetOpcode() == Opcode::InitClass);
        ASSERT(!codegen->GetGraph()->IsAotMode());
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToInitClass()->GetClass());
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {Imm(klass)});
    }
    return true;
}

// IS_INSTANCE
bool SlowPathEntrypoint::GenerateIsInstance(Codegen *codegen)
{
    auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto klass = codegen->ConvertRegister(GetInst()->GetSrcReg(1), DataType::REFERENCE);
    auto dst = codegen->ConvertRegister(GetInst()->GetDstReg(), GetInst()->GetType());
    codegen->CallRuntime(GetInst(), EntrypointId::IS_INSTANCE, dst, {src, klass});
    return true;
}

// CHECK_CAST
bool SlowPathEntrypoint::GenerateCheckCast(Codegen *codegen)
{
    auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto klass = codegen->ConvertRegister(GetInst()->GetSrcReg(1), DataType::REFERENCE);
    codegen->CallRuntime(GetInst(), EntrypointId::CHECK_CAST, INVALID_REGISTER, {src, klass});
    return true;
}

// DEOPTIMIZE
bool SlowPathEntrypoint::GenerateDeoptimize(Codegen *codegen)
{
    DeoptimizeType type = DeoptimizeType::INVALID;
    if (GetInst()->GetOpcode() == Opcode::Deoptimize) {
        type = GetInst()->CastToDeoptimize()->GetDeoptimizeType();
    } else if (GetInst()->GetOpcode() == Opcode::DeoptimizeIf) {
        type = GetInst()->CastToDeoptimizeIf()->GetDeoptimizeType();
    } else if (GetInst()->GetOpcode() == Opcode::DeoptimizeCompare) {
        type = GetInst()->CastToDeoptimizeCompare()->GetDeoptimizeType();
    } else if (GetInst()->GetOpcode() == Opcode::DeoptimizeCompareImm) {
        type = GetInst()->CastToDeoptimizeCompareImm()->GetDeoptimizeType();
    } else if (GetInst()->GetOpcode() == Opcode::AnyTypeCheck) {
        type = DeoptimizeType::ANY_TYPE_CHECK;
    } else if (GetInst()->GetOpcode() == Opcode::AddOverflowCheck) {
        type = DeoptimizeType::DEOPT_OVERFLOW;
    } else if (GetInst()->GetOpcode() == Opcode::SubOverflowCheck) {
        type = DeoptimizeType::DEOPT_OVERFLOW;
    } else {
        UNREACHABLE();
    }
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {Imm(static_cast<uint8_t>(type))});
    return true;
}

// CREATE_OBJECT
bool SlowPathEntrypoint::GenerateCreateObject(Codegen *codegen)
{
    auto inst = GetInst();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src = codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    codegen->CallRuntime(inst, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, {src});

    return true;
}

bool SlowPathEntrypoint::GenerateByEntry(Codegen *codegen)
{
    switch (GetEntrypoint()) {
        case EntrypointId::THROW_EXCEPTION: {
            auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::Type::REFERENCE);
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {src});
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
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {size});
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
        case EntrypointId::DEOPTIMIZE: {
            return GenerateDeoptimize(codegen);
        }
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
            case EntrypointId::GET_UNKNOWN_STATIC_FIELD_PTR:
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
    codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), dst_reg_, Imm(string_id_), tmp_addr_reg);
}

void SlowPathRefCheck::GenerateImpl(Codegen *codegen)
{
    ASSERT(array_reg_ != INVALID_REGISTER);
    ASSERT(ref_reg_ != INVALID_REGISTER);
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {array_reg_, ref_reg_});
}

void SlowPathAbstract::GenerateImpl(Codegen *codegen)
{
    SCOPED_DISASM_STR(codegen, std::string("SlowPath for Abstract method ") + std::to_string(GetInst()->GetId()));
    ASSERT(method_reg_ != INVALID_REGISTER);
    ScopedTmpReg method_reg(codegen->GetEncoder(), method_reg_);
    ASSERT(method_reg.GetReg().GetId() == method_reg_.GetId());
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {method_reg.GetReg()});
}

void SlowPathCheckCast::GenerateImpl(Codegen *codegen)
{
    SCOPED_DISASM_STR(codegen, std::string("SlowPath for CheckCast exception") + std::to_string(GetInst()->GetId()));
    auto inst = GetInst();
    auto src = codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {class_reg_, src});
}

void SlowPathUnresolved::GenerateImpl(Codegen *codegen)
{
    SlowPathEntrypoint::GenerateImpl(codegen);

    ASSERT(method_ != nullptr);
    ASSERT(type_id_ != 0);
    ASSERT(slot_addr_ != 0);

    ScopedTmpReg value_reg(codegen->GetEncoder());
    if (GetInst()->GetOpcode() == Opcode::UnresolvedCallVirtual) {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), value_reg, arg_reg_, Imm(type_id_),
                                       Imm(slot_addr_));
    } else if (GetEntrypoint() == EntrypointId::GET_UNKNOWN_CALLEE_METHOD ||
               GetEntrypoint() == EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS ||
               GetEntrypoint() == EntrypointId::GET_UNKNOWN_STATIC_FIELD_PTR) {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), value_reg, Imm(type_id_), Imm(slot_addr_));
    } else {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), value_reg, Imm(type_id_));

        ScopedTmpReg addr_reg(codegen->GetEncoder());
        codegen->GetEncoder()->EncodeMov(addr_reg, Imm(slot_addr_));
        codegen->GetEncoder()->EncodeStr(value_reg, MemRef(addr_reg));
    }

    if (dst_reg_.IsValid()) {
        codegen->GetEncoder()->EncodeMov(dst_reg_, value_reg);
    }
}

}  // namespace panda::compiler
