/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "optimizer/code_generator/codegen.h"

#include "llvm_ir_constructor.h"

#include "gc_barriers.h"
#include "irtoc_function_utils.h"
#include "llvm_logger.h"
#include "llvm_options.h"
#include "metadata.h"
#include "transforms/runtime_calls.h"

#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/IntrinsicsAArch64.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using panda::llvmbackend::DebugDataBuilder;
using panda::llvmbackend::LLVMArkInterface;
using panda::llvmbackend::gc_barriers::EmitPostWRB;
using panda::llvmbackend::gc_barriers::EmitPreWRB;
using panda::llvmbackend::irtoc_function_utils::IsNoAliasIrtocFunction;
#ifndef NDEBUG
using panda::llvmbackend::irtoc_function_utils::IsPtrIgnIrtocFunction;
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_TYPE(input, expected_type)                                                                  \
    ASSERT_DO((input)->getType() == (expected_type),                                                       \
              std::cerr << "Unexpected data type: " << GetTypeName((input)->getType()) << ". Should be a " \
                        << GetTypeName(expected_type) << "." << std::endl)

// arm64: { dispatch: 24, pc: 20, frame: 23, acc: 21, acc_tag: 22, moffset: 25, method_ptr: 26 },
constexpr auto AARCH64_PC = 20;
constexpr auto AARCH64_ACC = 21;
constexpr auto AARCH64_ACC_TAG = 22;
constexpr auto AARCH64_FP = 23;
constexpr auto AARCH64_DISPATCH = 24;
constexpr auto AARCH64_MOFFSET = 25;
constexpr auto AARCH64_METHOD_PTR = 26;
constexpr auto AARCH64_REAL_FP = 29;

// x86_64: { dispatch: 8, pc: 4, frame: 5, acc: 11, acc_tag: 3 }
constexpr auto X86_64_PC = 4;       // renamed r10
constexpr auto X86_64_ACC = 11;     // renamed r3 (rbx)
constexpr auto X86_64_ACC_TAG = 3;  // renamed r11
constexpr auto X86_64_FP = 5;       // renamed r9
constexpr auto X86_64_DISPATCH = 8;
constexpr auto X86_64_REAL_FP = 9;  // renamed r5 (rbp)

namespace {
inline llvm::Function *CreateFunctionDeclaration(llvm::FunctionType *function_type, const std::string &name,
                                                 llvm::Module *module)
{
    ASSERT(function_type != nullptr);
    ASSERT(!name.empty());
    ASSERT(module != nullptr);

    auto function = module->getFunction(name);
    if (function != nullptr) {
        ASSERT(function->getVisibility() == llvm::GlobalValue::ProtectedVisibility);
        ASSERT(function->doesNotThrow());
        return function;
    }

    function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name, module);

    // Prevens emitting `.eh_frame` section
    function->setDoesNotThrow();

    function->setVisibility(llvm::GlobalValue::ProtectedVisibility);

    function->setSectionPrefix(name);

    return function;
}

inline void CreateBlackBoxAsm(llvm::IRBuilder<> *builder, const std::string &inline_asm)
{
    auto iasm_type = llvm::FunctionType::get(builder->getVoidTy(), {}, false);
    builder->CreateCall(iasm_type, llvm::InlineAsm::get(iasm_type, inline_asm, "", true), {});
}

inline void CreateInt32ImmAsm(llvm::IRBuilder<> *builder, const std::string &inline_asm, uint32_t imm)
{
    auto one_int = llvm::FunctionType::get(builder->getVoidTy(), {builder->getInt32Ty()}, false);
    builder->CreateCall(one_int, llvm::InlineAsm::get(one_int, inline_asm, "i", true), {builder->getInt32(imm)});
}

inline llvm::AtomicOrdering ToAtomicOrdering(bool is_volatile)
{
    return is_volatile ? LLVMArkInterface::VOLATILE_ORDER : LLVMArkInterface::NOT_ATOMIC_ORDER;
}

#ifndef NDEBUG
inline std::string GetTypeName(llvm::Type *type)
{
    std::string name;
    auto stream = llvm::raw_string_ostream(name);
    type->print(stream);
    return stream.str();
}
#endif
}  // namespace

namespace panda::compiler {

#include <can_compile_intrinsics_gen.inl>

// Use that only to pass it into method like rvalue
static inline std::string CreateBasicBlockName(Inst *inst, const std::string &bb_name)
{
    std::stringstream name;
    name << "bb" << std::to_string(inst->GetBasicBlock()->GetId()) << "_i" << std::to_string(inst->GetId()) << ".."
         << bb_name << "..";
    return name.str();
}

static inline std::string CreateNameForInst(Inst *inst)
{
    return std::string("v") + std::to_string(inst->GetId());
}

static inline bool IsInteger(DataType::Type type)
{
    return DataType::IsTypeNumeric(type) && !DataType::IsFloatType(type) && type != DataType::POINTER;
}

static inline bool IsSignedInteger(const DataType::Type &type)
{
    return IsInteger(type) && DataType::IsTypeSigned(type);
}

static inline bool IsUnsignedInteger(DataType::Type type)
{
    return IsInteger(type) && !DataType::IsTypeSigned(type);
}

static inline llvm::ICmpInst::Predicate ICmpCodeConvert(ConditionCode cc)
{
    switch (cc) {
        case ConditionCode::CC_EQ:
            return llvm::CmpInst::Predicate::ICMP_EQ;
        case ConditionCode::CC_NE:
            return llvm::CmpInst::Predicate::ICMP_NE;
        case ConditionCode::CC_LT:
            return llvm::CmpInst::Predicate::ICMP_SLT;
        case ConditionCode::CC_GT:
            return llvm::CmpInst::Predicate::ICMP_SGT;
        case ConditionCode::CC_LE:
            return llvm::CmpInst::Predicate::ICMP_SLE;
        case ConditionCode::CC_GE:
            return llvm::CmpInst::Predicate::ICMP_SGE;
        case ConditionCode::CC_B:
            return llvm::CmpInst::Predicate::ICMP_ULT;
        case ConditionCode::CC_A:
            return llvm::CmpInst::Predicate::ICMP_UGT;
        case ConditionCode::CC_BE:
            return llvm::CmpInst::Predicate::ICMP_ULE;
        case ConditionCode::CC_AE:
            return llvm::CmpInst::Predicate::ICMP_UGE;
        default:
            UNREACHABLE();
            return llvm::CmpInst::Predicate::ICMP_NE;
    }
}

static inline llvm::FCmpInst::Predicate FCmpCodeConvert(ConditionCode condition_code)
{
    switch (condition_code) {
        case ConditionCode::CC_EQ:
            return llvm::FCmpInst::Predicate::FCMP_UEQ;
        case ConditionCode::CC_NE:
            return llvm::FCmpInst::Predicate::FCMP_UNE;
        case ConditionCode::CC_LT:
            return llvm::FCmpInst::Predicate::FCMP_ULT;
        case ConditionCode::CC_GT:
            return llvm::FCmpInst::Predicate::FCMP_UGT;
        case ConditionCode::CC_LE:
            return llvm::FCmpInst::Predicate::FCMP_ULE;
        case ConditionCode::CC_GE:
            return llvm::FCmpInst::Predicate::FCMP_UGE;
        case ConditionCode::CC_B:
            return llvm::FCmpInst::Predicate::FCMP_ULT;
        case ConditionCode::CC_A:
            return llvm::FCmpInst::Predicate::FCMP_UGT;
        case ConditionCode::CC_BE:
            return llvm::FCmpInst::Predicate::FCMP_ULE;
        case ConditionCode::CC_AE:
            return llvm::FCmpInst::Predicate::FCMP_UGE;
        default:
            ASSERT_DO(false, (std::cerr << "Unexpected condition_code = " << condition_code << std::endl));
            UNREACHABLE();
    }
}

static size_t GetRealFrameReg(Arch arch)
{
    switch (arch) {
        case Arch::AARCH64:
            return AARCH64_REAL_FP;
        case Arch::X86_64:
            return X86_64_REAL_FP;
        default:
            UNREACHABLE();
    }
}

bool LLVMIrConstructor::TryEmitIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId ark_id)
{
    switch (ark_id) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_UNREACHABLE:
            return EmitUnreachable();
        case RuntimeInterface::IntrinsicId::INTRINSIC_INTERPRETER_RETURN:
            return EmitInterpreterReturn();
        case RuntimeInterface::IntrinsicId::INTRINSIC_TAIL_CALL:
            return EmitTailCall(inst);
        default:
            return false;
    }
    return false;
}

// Specific intrinsic Emitters

bool LLVMIrConstructor::EmitUnreachable()
{
    auto bb = GetCurrentBasicBlock();
    if (bb->empty() || !llvm::isa<llvm::ReturnInst>(*(bb->rbegin()))) {
        auto trap = llvm::Intrinsic::getDeclaration(func_->getParent(), llvm::Intrinsic::trap, {});
        builder_.CreateCall(trap, {});
        builder_.CreateUnreachable();
    }
    return true;
}

bool LLVMIrConstructor::EmitInterpreterReturn()
{
    // This constant is hardcoded in codegen_interpreter.h and in interpreter.irt
    constexpr size_t SPILL_SLOTS = 32;
    CFrameLayout fl(GetGraph()->GetArch(), SPILL_SLOTS);
    constexpr bool SAVE_UNUSED_CALLEE_REGS = true;

    // Restore callee-registers
    auto callee_regs_mask = GetCalleeRegsMask(GetGraph()->GetArch(), false, SAVE_UNUSED_CALLEE_REGS);
    auto callee_vregs_mask = GetCalleeRegsMask(GetGraph()->GetArch(), true, SAVE_UNUSED_CALLEE_REGS);
    if (GetGraph()->GetArch() == Arch::AARCH64) {
        constexpr bool SAVE_FRAME_AND_LINK_REGS = true;

        size_t slot_size = fl.GetSlotSize();
        size_t dslot_size = slot_size * 2U;

        auto last_callee_reg = fl.GetRegsSlotsCount() - callee_regs_mask.Count();
        auto last_callee_vreg = fl.GetRegsSlotsCount() - fl.GetCalleeRegistersCount(false) - callee_vregs_mask.Count();
        CreateInterpreterReturnRestoreRegs(callee_regs_mask, last_callee_reg, false);
        CreateInterpreterReturnRestoreRegs(callee_vregs_mask, last_callee_vreg, true);

        // Adjust SP
        auto sp_to_frame_top_slots = fl.GetRegsSlotsCount() + CFrameRegs::Start() - CFrameReturnAddr::Start();
        if (SAVE_FRAME_AND_LINK_REGS) {
            sp_to_frame_top_slots -= CFrameLayout::GetFpLrSlotsCount();
        }

        CreateInt32ImmAsm(&builder_,
                          std::string("add  sp, sp, $0").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT),
                          sp_to_frame_top_slots * slot_size);
        CreateInt32ImmAsm(&builder_, "ldp  x29, x30, [sp], $0", dslot_size);
        CreateBlackBoxAsm(&builder_, "ret");
    } else {
        // Currently there is no vector regs usage in x86_64 handlers
        ASSERT(callee_vregs_mask.count() == 0);
        auto reg_shift = DOUBLE_WORD_SIZE_BYTES *
                         (fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true));
        auto fp_shift = DOUBLE_WORD_SIZE_BYTES * (2 + CFrameSlots::Start() - CFrameData::Start());

        std::string iasm_str =
            std::string("leaq  ${0:c}(%rsp), %rsp").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT);
        CreateInt32ImmAsm(&builder_, iasm_str, reg_shift);
        Target target {GetGraph()->GetArch()};
        while (callee_regs_mask.count() > 0) {
            auto reg = callee_regs_mask.GetMinRegister();
            callee_regs_mask ^= 1U << reg;
            iasm_str = "pop  %" + target.GetRegName(reg, false);
            CreateBlackBoxAsm(&builder_, iasm_str);
        }
        iasm_str = "leaq  " + std::to_string(fp_shift) + "(%rsp), %rsp";
        CreateBlackBoxAsm(&builder_, iasm_str);
        CreateBlackBoxAsm(&builder_, "pop  %rbp");
        CreateBlackBoxAsm(&builder_, "retq");
    }
    builder_.CreateUnreachable();

    return true;
}

bool LLVMIrConstructor::EmitTailCall(Inst *inst)
{
    ASSERT(func_->getCallingConv() == llvm::CallingConv::ArkInt);
    llvm::CallInst *call;

    auto ptr = GetInputValue(inst, 0);
    ASSERT_TYPE(ptr, builder_.getPtrTy());
    ASSERT(cc_values_.size() == (GetGraph()->GetArch() == Arch::AARCH64 ? 8U : 7U));
    ASSERT(cc_values_.at(0) != nullptr);  // pc
    static constexpr unsigned ACC = 1U;
    static constexpr unsigned ACC_TAG = 2U;
    ArenaVector<llvm::Type *> arg_types(GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 0; i < cc_.size(); i++) {
        if (cc_values_.at(i) != nullptr) {
            arg_types.push_back(cc_values_.at(i)->getType());
        } else {
            arg_types.push_back(func_->getFunctionType()->getParamType(i));
        }
    }
    if (cc_values_.at(ACC) == nullptr) {
        cc_values_[ACC] = llvm::Constant::getNullValue(arg_types[ACC]);
    }
    if (cc_values_.at(ACC_TAG) == nullptr) {
        cc_values_[ACC_TAG] = llvm::Constant::getNullValue(arg_types[ACC_TAG]);
    }
    ASSERT(cc_values_.at(3U) != nullptr);  // frame
    ASSERT(cc_values_.at(4U) != nullptr);  // dispatch
    if (GetGraph()->GetArch() == Arch::AARCH64) {
        ASSERT(cc_values_.at(5U) != nullptr);  // moffset
        ASSERT(cc_values_.at(6U) != nullptr);  // method_ptr
        ASSERT(cc_values_.at(7U) != nullptr);  // thread
    } else {
        static constexpr unsigned REAL_FRAME_POINER = 6U;
        ASSERT(cc_values_.at(5U) != nullptr);                 // thread
        ASSERT(cc_values_.at(REAL_FRAME_POINER) == nullptr);  // real frame pointer
        cc_values_[REAL_FRAME_POINER] = func_->getArg(REAL_FRAME_POINER);
    }

    auto function_type = llvm::FunctionType::get(func_->getReturnType(), arg_types, false);
    call = builder_.CreateCall(function_type, ptr, cc_values_);

    call->setCallingConv(func_->getCallingConv());
    call->setTailCallKind(llvm::CallInst::TailCallKind::TCK_Tail);
    if (func_->getReturnType()->isVoidTy()) {
        builder_.CreateRetVoid();
    } else {
        builder_.CreateRet(call);
    }
    std::fill(cc_values_.begin(), cc_values_.end(), nullptr);
    return true;
}

llvm::Value *LLVMIrConstructor::GetMappedValue(Inst *inst, DataType::Type type)
{
    ASSERT(input_map_.count(inst) == 1);
    auto &type_map = input_map_.at(inst);
    ASSERT(type_map.count(type) == 1);
    auto result = type_map.at(type);
    ASSERT(result != nullptr);
    return result;
}

llvm::Value *LLVMIrConstructor::GetInputValue(Inst *inst, size_t index, bool skip_coerce)
{
    auto input = inst->GetInput(index).GetInst();
    auto type = inst->GetInputType(index);
    ASSERT(type != DataType::NO_TYPE);

    if (skip_coerce) {
        ASSERT(input->GetType() == DataType::UINT64 || input->GetType() == DataType::INT64);
        type = input->GetType();
    }

    if (input->IsConst()) {
        return GetInputValueFromConstant(input->CastToConstant(), type);
    }
    if (input->GetOpcode() == Opcode::NullPtr) {
        auto llvm_type = GetExactType(DataType::REFERENCE);
        ASSERT(llvm_type == builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
        return llvm::Constant::getNullValue(llvm_type);
    }
    return GetMappedValue(input, type);
}

llvm::Value *LLVMIrConstructor::GetInputValueFromConstant(ConstantInst *constant, DataType::Type panda_type)
{
    auto llvm_type = GetExactType(panda_type);
    if (panda_type == DataType::FLOAT64) {
        double value = constant->GetDoubleValue();
        return llvm::ConstantFP::get(llvm_type, value);
    }
    if (panda_type == DataType::FLOAT32) {
        float value = constant->GetFloatValue();
        return llvm::ConstantFP::get(llvm_type, value);
    }
    if (panda_type == DataType::POINTER) {
        auto cval = static_cast<int64_t>(constant->GetIntValue());
        auto integer = builder_.getInt64(cval);
        return builder_.CreateIntToPtr(integer, builder_.getPtrTy());
    }
    if (DataType::IsTypeNumeric(panda_type)) {
        auto is_signed = DataType::IsTypeSigned(panda_type);
        auto cval = static_cast<int64_t>(constant->GetIntValue());
        return llvm::ConstantInt::get(llvm_type, cval, is_signed);
    }
    if (DataType::IsReference(panda_type) && constant->GetRawValue() == 0) {
        return llvm::Constant::getNullValue(llvm_type);
    }
    UNREACHABLE();
}

// Initializers. BuildIr calls them

void LLVMIrConstructor::BuildBasicBlocks()
{
    auto &context = func_->getContext();
    for (auto block : graph_->GetBlocksRPO()) {
        if (block->IsEndBlock()) {
            continue;
        }
        auto bb = llvm::BasicBlock::Create(context, llvm::StringRef("bb") + llvm::Twine(block->GetId()), func_);
        AddBlock(block, bb);
        // Checking that irtoc handler contains a return instruction
        for (auto inst : block->AllInsts()) {
            if (inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() ==
                                           RuntimeInterface::IntrinsicId::INTRINSIC_INTERPRETER_RETURN) {
                ark_interface_->AppendIrtocReturnHandler(func_->getName());
            }
        }
    }
}

void LLVMIrConstructor::BuildInstructions()
{
    for (auto block : graph_->GetBlocksRPO()) {
        if (block->IsEndBlock()) {
            continue;
        }
        SetCurrentBasicBlock(GetTailBlock(block));
        for (auto inst : block->AllInsts()) {
            auto bb = GetCurrentBasicBlock();
            if (!bb->empty() && llvm::isa<llvm::UnreachableInst>(*(bb->rbegin()))) {
                break;
            }
            VisitInstruction(inst);
        }

        if ((block->GetSuccsBlocks().size() == 1 && !block->GetSuccessor(0)->IsEndBlock())) {
            builder_.CreateBr(GetHeadBlock(block->GetSuccessor(0)));
        }
        ReplaceTailBlock(block, GetCurrentBasicBlock());
    }
}

void LLVMIrConstructor::FillPhiInputs()
{
    for (auto block : graph_->GetBlocksRPO()) {
        if (block->IsStartBlock() || block->IsEndBlock()) {
            continue;
        }
        for (auto inst : block->PhiInsts()) {
            auto phi = llvm::cast<llvm::PHINode>(GetMappedValue(inst, inst->GetType()));
            for (size_t i = 0; i < inst->GetInputsCount(); i++) {
                auto input_block = block->GetPredBlockByIndex(i);
                auto input = GetInputValue(inst, i);
                phi->addIncoming(input, GetTailBlock(input_block));
            }
        }
    }
}

// Creator functions for internal usage

void LLVMIrConstructor::CreateInterpreterReturnRestoreRegs(RegMask &reg_mask, size_t offset, bool fp)
{
    int32_t slot_size = PointerSize(GetGraph()->GetArch());
    int32_t dslot_size = slot_size * 2U;
    int32_t total_size = reg_mask.count() * slot_size;
    auto start_reg_offset = offset * DOUBLE_WORD_SIZE_BYTES;
    auto end_reg_offset = start_reg_offset + std::max(0, total_size - dslot_size);

    constexpr uint32_t MAX_REPR_VAL = 504U;
    bool representable = start_reg_offset <= MAX_REPR_VAL && (start_reg_offset & 0x7U) == 0 &&
                         end_reg_offset <= MAX_REPR_VAL && (end_reg_offset & 0x7U) == 0;

    std::string base_reg = representable ? "sp" : "x16";
    if (!representable) {
        CreateInt32ImmAsm(&builder_,
                          std::string("add  x16, sp, $0").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT),
                          start_reg_offset);
        start_reg_offset = 0;
    }

    while (reg_mask.count() > 0) {
        std::string asm_string = reg_mask.count() / 2U > 0 ? "ldp " : "ldr ";
        auto first = reg_mask.GetMinRegister();
        asm_string += (fp ? "d" : "x") + std::to_string(first);
        reg_mask ^= 1U << first;
        if (reg_mask.count() > 0) {
            asm_string += ", ";
            auto second = reg_mask.GetMinRegister();
            asm_string += (fp ? "d" : "x") + std::to_string(second);
            reg_mask ^= 1U << second;
        }
        asm_string += ", [";
        asm_string += base_reg;
        asm_string += ", $0]";
        if (representable) {
            asm_string += LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT;
        }
        CreateInt32ImmAsm(&builder_, asm_string, start_reg_offset);
        start_reg_offset += dslot_size;
    }
}

llvm::Value *LLVMIrConstructor::CreateBinaryOp(Inst *inst, llvm::Instruction::BinaryOps opcode)
{
    llvm::Value *x = GetInputValue(inst, 0);
    llvm::Value *y = GetInputValue(inst, 1);

    if (x->getType()->isPointerTy()) {
        if (y->getType()->isPointerTy()) {
            ASSERT(opcode == llvm::Instruction::Sub);
            x = builder_.CreatePtrToInt(x, builder_.getInt64Ty());
            y = builder_.CreatePtrToInt(y, builder_.getInt64Ty());
            return builder_.CreateBinOp(opcode, x, y);
        }
        if (y->getType()->isIntegerTy()) {
            ASSERT(opcode == llvm::Instruction::Add);
            ASSERT(x->getType()->isPointerTy());
            return builder_.CreateInBoundsGEP(builder_.getInt8Ty(), x, y);
        }
        UNREACHABLE();
    }
    return builder_.CreateBinOp(opcode, x, y);
}

llvm::Value *LLVMIrConstructor::CreateBinaryImmOp(Inst *inst, llvm::Instruction::BinaryOps opcode, uint64_t c)
{
    ASSERT(IsTypeNumeric(inst->GetType()));
    llvm::Value *x = GetInputValue(inst, 0);
    if (x->getType()->isPointerTy()) {
        ASSERT(x->getType()->isPointerTy());
        ASSERT(opcode == llvm::Instruction::Add || opcode == llvm::Instruction::Sub);
        if (opcode == llvm::Instruction::Sub) {
            c = -c;
        }
        return builder_.CreateConstInBoundsGEP1_64(builder_.getInt8Ty(), x, c);
    }
    llvm::Value *y = CoerceValue(builder_.getInt64(c), DataType::INT64, inst->GetType());
    return builder_.CreateBinOp(opcode, x, y);
}

llvm::Value *LLVMIrConstructor::CreateShiftOp(Inst *inst, llvm::Instruction::BinaryOps opcode)
{
    llvm::Value *x = GetInputValue(inst, 0);
    llvm::Value *y = GetInputValue(inst, 1);
    auto target_type = inst->GetType();
    bool target_64 = (target_type == DataType::UINT64) || (target_type == DataType::INT64);
    auto constexpr SHIFT32_RANGE = 0x1f;
    auto constexpr SHIFT64_RANGE = 0x3f;

    y = builder_.CreateBinOp(llvm::Instruction::And, y,
                             llvm::ConstantInt::get(y->getType(), target_64 ? SHIFT64_RANGE : SHIFT32_RANGE));

    return builder_.CreateBinOp(opcode, x, y);
}

llvm::Value *LLVMIrConstructor::CreateSignDivMod(Inst *inst, llvm::Instruction::BinaryOps opcode)
{
    ASSERT(opcode == llvm::Instruction::SDiv || opcode == llvm::Instruction::SRem);
    llvm::Value *x = GetInputValue(inst, 0);
    llvm::Value *y = GetInputValue(inst, 1);
    auto arch = GetGraph()->GetArch();
    if (arch == Arch::AARCH64 && !llvm::isa<llvm::Constant>(y)) {
        return CreateAArch64SignDivMod(inst, opcode, x, y);
    }
    auto &ctx = func_->getContext();
    auto type = y->getType();
    auto eq_m1 = builder_.CreateICmpEQ(y, llvm::ConstantInt::get(type, -1));
    auto m1_bb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "divmod_minus1"), func_);
    auto not_m1_bb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "divmod_normal"), func_);
    auto cont_bb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "divmod_cont"), func_);
    builder_.CreateCondBr(eq_m1, m1_bb, not_m1_bb);

    SetCurrentBasicBlock(m1_bb);
    llvm::Value *m1_result;
    if (opcode == llvm::Instruction::SDiv) {
        m1_result = builder_.CreateNeg(x);
    } else {
        m1_result = llvm::ConstantInt::get(type, 0);
    }
    builder_.CreateBr(cont_bb);

    SetCurrentBasicBlock(not_m1_bb);
    auto result = builder_.CreateBinOp(opcode, x, y);
    builder_.CreateBr(cont_bb);

    SetCurrentBasicBlock(cont_bb);
    auto result_phi = builder_.CreatePHI(y->getType(), 2U);
    result_phi->addIncoming(m1_result, m1_bb);
    result_phi->addIncoming(result, not_m1_bb);
    return result_phi;
}

llvm::Value *LLVMIrConstructor::CreateAArch64SignDivMod(Inst *inst, llvm::Instruction::BinaryOps opcode, llvm::Value *x,
                                                        llvm::Value *y)
{  // LLVM opcodes SDiv/SRem don't fully suit our needs (int_min/-1 case is UB),
    // but for now we inline assembly only for arm64 sdiv
    auto target_type = inst->GetType();
    bool target_64 = (target_type == DataType::INT64);
    llvm::Value *quotient {nullptr};
    llvm::Value *result {nullptr};
    llvm::Type *type {nullptr};
    {
        std::string itext;
        if (target_type == DataType::INT8) {
            type = builder_.getInt8Ty();
            itext += "sxtb ${1:w}, ${1:w}\nsxtb ${2:w}, ${2:w}\n";
        } else if (target_type == DataType::INT16) {
            type = builder_.getInt16Ty();
            itext += "sxth ${1:w}, ${1:w}\nsxth ${2:w}, ${2:w}\n";
        } else {
            ASSERT(target_64 || target_type == DataType::INT32);
            type = target_64 ? builder_.getInt64Ty() : builder_.getInt32Ty();
        }
        itext += target_64 ? "sdiv ${0:x}, ${1:x}, ${2:x}" : "sdiv ${0:w}, ${1:w}, ${2:w}";
        auto itype = llvm::FunctionType::get(type, {type, type}, false);  // no var args
        quotient = builder_.CreateCall(itype, llvm::InlineAsm::get(itype, itext, "=r,r,r", false), {x, y});
        result = quotient;
    }

    if (opcode == llvm::Instruction::SRem) {
        auto mod_asm_type = llvm::FunctionType::get(type, {type, type, type}, false);  // no var args
        // Inline asm "sdiv q, x, y" yields q = x / y
        // Inline asm "msub r, x, y, q" yields r = x - y * q
        std::string_view mod_asm =
            target_64 ? "msub ${0:x}, ${3:x}, ${2:x}, ${1:x}" : "msub ${0:w}, ${3:w}, ${2:w}, ${1:w}";
        auto remainder = builder_.CreateCall(
            mod_asm_type, llvm::InlineAsm::get(mod_asm_type, mod_asm, "=r,r,r,r", false), {x, y, result});
        result = remainder;
    }
    return result;
}

llvm::Value *LLVMIrConstructor::CreateFloatComparison(CmpInst *cmp_inst, llvm::Value *x, llvm::Value *y)
{
    // if x is less than y then return -1
    // else return zero extend of (x > y)
    llvm::CmpInst::Predicate greater_than_predicate;
    llvm::CmpInst::Predicate less_than_predicate;
    if (cmp_inst->IsFcmpg()) {
        // if x or y is nan then greater_than_predicate yields true
        greater_than_predicate = llvm::CmpInst::FCMP_UGT;
        less_than_predicate = llvm::CmpInst::FCMP_OLT;
    } else if (cmp_inst->IsFcmpl()) {
        greater_than_predicate = llvm::CmpInst::FCMP_OGT;
        // if x or y is nan then less_than_predicate yields true
        less_than_predicate = llvm::CmpInst::FCMP_ULT;
    } else {
        ASSERT_PRINT(false, "cmp_inst must be either Fcmpg, or Fcmpl");
        UNREACHABLE();
    }
    // x > y || (inst == Fcmpg && (x == NaN || y == NaN))
    auto greater_than = builder_.CreateFCmp(greater_than_predicate, x, y);
    // x < y || (inst == Fcmpl && (x == NaN || y == NaN))
    auto less_than = builder_.CreateFCmp(less_than_predicate, x, y);
    auto comparison = builder_.CreateZExt(greater_than, builder_.getInt32Ty());
    auto negative_one = builder_.getInt32(-1);
    return builder_.CreateSelect(less_than, negative_one, comparison);
}

llvm::Value *LLVMIrConstructor::CreateIntegerComparison(CmpInst *inst, llvm::Value *x, llvm::Value *y)
{
    ASSERT(x->getType() == y->getType());
    llvm::Value *greater_than;
    llvm::Value *less_than;

    if (DataType::IsTypeSigned(inst->GetOperandsType())) {
        greater_than = builder_.CreateICmpSGT(x, y);
        less_than = builder_.CreateICmpSLT(x, y);
    } else {
        greater_than = builder_.CreateICmpUGT(x, y);
        less_than = builder_.CreateICmpULT(x, y);
    }
    auto cast_comparison_result = builder_.CreateZExt(greater_than, builder_.getInt32Ty());
    auto negative_one = builder_.getInt32(-1);
    return builder_.CreateSelect(less_than, negative_one, cast_comparison_result);
}

llvm::Value *LLVMIrConstructor::CreateCastToInt(Inst *inst)
{
    ASSERT(inst->GetInputsCount() == 1);

    llvm::Value *input = GetInputValue(inst, 0);
    auto source_type = input->getType();
    auto target_type = inst->GetType();

    ASSERT_DO(source_type->isFloatTy() || source_type->isDoubleTy(),
              std::cerr << "Unexpected source type: " << GetTypeName(source_type) << ". Should be a float or double."
                        << std::endl);

    auto llvm_id = DataType::IsTypeSigned(target_type) ? llvm::Intrinsic::fptosi_sat : llvm::Intrinsic::fptoui_sat;
    ArenaVector<llvm::Type *> intrinsic_types(GetGraph()->GetLocalAllocator()->Adapter());
    intrinsic_types.assign({GetExactType(target_type), source_type});
    return builder_.CreateIntrinsic(llvm_id, intrinsic_types, {input}, nullptr);
}

llvm::Value *LLVMIrConstructor::CreateLoadWithOrdering(Inst *inst, llvm::Value *value, llvm::AtomicOrdering ordering,
                                                       const llvm::Twine &name)
{
    auto panda_type = inst->GetType();
    llvm::Type *type = GetExactType(panda_type);

    auto load = builder_.CreateLoad(type, value, false, name);  // C-like volatile is not applied
    if (ordering != LLVMArkInterface::NOT_ATOMIC_ORDER) {
        auto alignment = func_->getParent()->getDataLayout().getPrefTypeAlignment(type);
        load->setOrdering(ordering);
        load->setAlignment(llvm::Align(alignment));
    }

    return load;
}

llvm::Value *LLVMIrConstructor::CreateStoreWithOrdering(llvm::Value *value, llvm::Value *ptr,
                                                        llvm::AtomicOrdering ordering)
{
    auto store = builder_.CreateStore(value, ptr, false);  // C-like volatile is not applied
    if (ordering != LLVMArkInterface::NOT_ATOMIC_ORDER) {
        auto alignment = func_->getParent()->getDataLayout().getPrefTypeAlignment(value->getType());
        store->setAlignment(llvm::Align(alignment));
        store->setOrdering(ordering);
    }
    return store;
}

void LLVMIrConstructor::CreatePreWRB(Inst *inst, llvm::Value *mem)
{
    ASSERT(GetGraph()->GetRuntime()->GetPreType() == panda::mem::BarrierType::PRE_SATB_BARRIER);

    auto &ctx = func_->getContext();
    auto out_bb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "pre_wrb_out"), func_);
    EmitPreWRB(&builder_, mem, IsVolatileMemInst(inst), out_bb, ark_interface_, GetThreadRegValue());
}

void LLVMIrConstructor::CreatePostWRB(Inst *inst, llvm::Value *mem, llvm::Value *offset, llvm::Value *value)
{
    ASSERT(GetGraph()->GetRuntime()->GetPostType() == panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER ||
           GetGraph()->GetRuntime()->GetPostType() == panda::mem::BarrierType::POST_INTERREGION_BARRIER);

    Inst *second_value;
    Inst *val = InstStoredValue(inst, &second_value);
    ASSERT(second_value == nullptr);

    if (val->GetOpcode() == Opcode::NullPtr) {
        return;
    }

    llvm::Value *frame_reg_value = nullptr;
    if (GetGraph()->GetArch() == Arch::X86_64) {
        frame_reg_value = GetRealFrameRegValue();
    }
    EmitPostWRB(&builder_, mem, offset, value, ark_interface_, GetThreadRegValue(), frame_reg_value);
}

llvm::Value *LLVMIrConstructor::CreateMemoryFence(memory_order::Order order)
{
    llvm::AtomicOrdering ordering;
    switch (order) {
        case memory_order::RELEASE:
            ordering = llvm::AtomicOrdering::Release;
            break;
        case memory_order::ACQUIRE:
            ordering = llvm::AtomicOrdering::Acquire;
            break;
        case memory_order::FULL:
            ordering = llvm::AtomicOrdering::SequentiallyConsistent;
            break;
        default:
            UNREACHABLE();
    }
    return builder_.CreateFence(ordering);
}

llvm::Value *LLVMIrConstructor::CreateCondition(ConditionCode cc, llvm::Value *x, llvm::Value *y)
{
    if (cc == CC_TST_EQ || cc == CC_TST_NE) {
        auto tst = builder_.CreateBinOp(llvm::Instruction::And, x, y);
        return (cc == CC_TST_EQ) ? builder_.CreateIsNull(tst) : builder_.CreateIsNotNull(tst);
    }
    return builder_.CreateICmp(ICmpCodeConvert(cc), x, y);
}

void LLVMIrConstructor::CreateIf(Inst *inst, llvm::Value *cond, bool likely, bool unlikely)
{
    llvm::MDNode *weights = nullptr;
    auto constexpr LIKELY = llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT;
    auto constexpr UNLIKELY = llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT;
    if (likely) {
        weights = llvm::MDBuilder(func_->getContext()).createBranchWeights(LIKELY, UNLIKELY);
    } else if (unlikely) {
        weights = llvm::MDBuilder(func_->getContext()).createBranchWeights(UNLIKELY, LIKELY);
    }
    builder_.CreateCondBr(cond, GetHeadBlock(inst->GetBasicBlock()->GetTrueSuccessor()),
                          GetHeadBlock(inst->GetBasicBlock()->GetFalseSuccessor()), weights);
}

// Getters

llvm::FunctionType *LLVMIrConstructor::GetEntryFunctionType()
{
    ArenaVector<llvm::Type *> arg_types(graph_->GetLocalAllocator()->Adapter());

    // ArkInt have fake parameters
    for (size_t i = 0; i < cc_.size(); ++i) {
        arg_types.push_back(builder_.getPtrTy());
    }

    // Actual function arguments
    auto method = graph_->GetMethod();
    ASSERT(graph_->GetRuntime()->GetMethodTotalArgumentsCount(method) == 0);

    auto ret_type = graph_->GetRuntime()->GetMethodReturnType(method);
    ret_type = ret_type == DataType::NO_TYPE ? DataType::VOID : ret_type;
    return llvm::FunctionType::get(GetType(ret_type), makeArrayRef(arg_types.data(), arg_types.size()), false);
}

llvm::Value *LLVMIrConstructor::GetThreadRegValue()
{
    auto reg_input = std::find(cc_.begin(), cc_.end(), GetThreadReg(GetGraph()->GetArch()));
    ASSERT(reg_input != cc_.end());
    auto thread_reg_value = func_->arg_begin() + std::distance(cc_.begin(), reg_input);
    return thread_reg_value;
}

llvm::Value *LLVMIrConstructor::GetRealFrameRegValue()
{
    ASSERT(GetGraph()->GetArch() == Arch::X86_64);
    auto reg_input = std::find(cc_.begin(), cc_.end(), GetRealFrameReg(GetGraph()->GetArch()));
    ASSERT(reg_input != cc_.end());
    auto frame_reg_value = func_->arg_begin() + std::distance(cc_.begin(), reg_input);
    return frame_reg_value;
}

llvm::Type *LLVMIrConstructor::GetType(DataType::Type panda_type)
{
    switch (panda_type) {
        case DataType::VOID:
            return builder_.getVoidTy();
        case DataType::POINTER:
            return builder_.getPtrTy();
        case DataType::REFERENCE:
            return builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE);
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
            return builder_.getInt32Ty();
        case DataType::UINT64:
        case DataType::INT64:
            return builder_.getInt64Ty();
        case DataType::FLOAT32:
            return builder_.getFloatTy();
        case DataType::FLOAT64:
            return builder_.getDoubleTy();
        default:
            ASSERT_DO(false, (std::cerr << "No handler for panda type = '" << DataType::ToString(panda_type)
                                        << "' to llvm type conversion."));
            UNREACHABLE();
    }
}

llvm::Type *LLVMIrConstructor::GetExactType(DataType::Type target_type)
{
    switch (target_type) {
        case DataType::VOID:
            return builder_.getVoidTy();
        case DataType::POINTER:
            return builder_.getPtrTy();
        case DataType::REFERENCE:
            return builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE);
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8:
            return builder_.getInt8Ty();
        case DataType::UINT16:
        case DataType::INT16:
            return builder_.getInt16Ty();
        case DataType::UINT32:
        case DataType::INT32:
            return builder_.getInt32Ty();
        case DataType::UINT64:
        case DataType::INT64:
            return builder_.getInt64Ty();
        case DataType::FLOAT32:
            return builder_.getFloatTy();
        case DataType::FLOAT64:
            return builder_.getDoubleTy();
        default:
            ASSERT_DO(false, (std::cerr << "No handler for panda type = '" << DataType::ToString(target_type)
                                        << "' to llvm type conversion."));
            UNREACHABLE();
    }
}

llvm::Instruction::CastOps LLVMIrConstructor::GetCastOp(DataType::Type from, DataType::Type to)
{
    Arch arch = GetGraph()->GetArch();
    if (IsInteger(from) && IsInteger(to) && DataType::GetTypeSize(from, arch) > DataType::GetTypeSize(to, arch)) {
        // narrowing, e.g. U32TOU8, I64TOI32
        return llvm::Instruction::Trunc;
    }
    if (IsSignedInteger(from) && IsInteger(to) && DataType::GetTypeSize(from, arch) < DataType::GetTypeSize(to, arch)) {
        // signed int widening, e.g. I32TOI64, I32TOU64
        return llvm::Instruction::SExt;
    }
    if (IsUnsignedInteger(from) && IsInteger(to) &&
        DataType::GetTypeSize(from, arch) < DataType::GetTypeSize(to, arch)) {
        // unsigned int widening, e.g. U32TOI64, U8TOU64
        return llvm::Instruction::ZExt;
    }
    if (IsUnsignedInteger(from) && DataType::IsFloatType(to)) {
        // unsigned int to float, e.g. U32TOF64, U64TOF64
        return llvm::Instruction::UIToFP;
    }
    if (IsSignedInteger(from) && DataType::IsFloatType(to)) {
        // signed int to float e.g. I32TOF64, I64TOF64
        return llvm::Instruction::SIToFP;
    }
    if (DataType::IsFloatType(from) && DataType::IsFloatType(to)) {
        if (DataType::GetTypeSize(from, arch) < DataType::GetTypeSize(to, arch)) {
            return llvm::Instruction::FPExt;
        }
        return llvm::Instruction::FPTrunc;
    }
    if (DataType::IsReference(from) && to == DataType::POINTER) {
        return llvm::Instruction::AddrSpaceCast;
    }
    ASSERT_DO(false, (std::cerr << "Cast from " << DataType::ToString(from) << " to " << DataType::ToString(to))
                         << " is not supported");
    UNREACHABLE();
}

// Various other helpers

llvm::Value *LLVMIrConstructor::CoerceValue(llvm::Value *value, DataType::Type source_type, DataType::Type target_type)
{
    ASSERT(value != nullptr);
    // Other non-integer mistyping prohibited
    ASSERT_DO(!IsInteger(target_type) || value->getType()->isIntegerTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be an integer."
                        << std::endl);
    ASSERT_DO(!DataType::IsReference(target_type) || value->getType()->isPointerTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a pointer."
                        << std::endl);
    ASSERT_DO(target_type != DataType::FLOAT64 || value->getType()->isDoubleTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a double."
                        << std::endl);
    ASSERT_DO(target_type != DataType::FLOAT32 || value->getType()->isFloatTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a float."
                        << std::endl);

    if (!IsInteger(target_type)) {
        return value;
    }
    ASSERT(value->getType()->isIntegerTy());

    auto target_llvm_type = llvm::cast<llvm::IntegerType>(GetExactType(target_type));
    auto original_llvm_type = llvm::cast<llvm::IntegerType>(value->getType());
    ASSERT(original_llvm_type->getBitWidth() == DataType::GetTypeSize(source_type, GetGraph()->GetArch()));

    llvm::CastInst::CastOps cast_op;
    if (original_llvm_type->getBitWidth() > target_llvm_type->getBitWidth()) {
        cast_op = llvm::Instruction::Trunc;
    } else if (original_llvm_type->getBitWidth() < target_llvm_type->getBitWidth()) {
        if (IsSignedInteger(source_type)) {
            cast_op = llvm::Instruction::SExt;
        } else {
            cast_op = llvm::Instruction::ZExt;
        }
    } else {
        return value;
    }
    return builder_.CreateCast(cast_op, value, target_llvm_type);
}

llvm::Value *LLVMIrConstructor::CoerceValue(llvm::Value *value, llvm::Type *target_type)
{
    auto value_type = value->getType();
    if (value_type == target_type) {
        return value;
    }

    if (!value_type->isPointerTy() && target_type->isPointerTy()) {
        // DataType::POINTER to target_type.
        // Example: i64 -> %"class.panda::Frame"*
        return builder_.CreateIntToPtr(value, target_type);
    }
    if (value_type->isPointerTy() && !target_type->isPointerTy()) {
        // value_type to DataType::POINTER
        // Example: %"class.panda::coretypes::String"* -> i64
        return builder_.CreatePtrToInt(value, target_type);
    }

    if (value_type->isIntegerTy() && target_type->isIntegerTy()) {
        auto value_width = llvm::cast<llvm::IntegerType>(value_type)->getBitWidth();
        auto target_width = llvm::cast<llvm::IntegerType>(target_type)->getBitWidth();
        if (value_width > target_width) {
            return builder_.CreateTrunc(value, target_type);
        }
        if (value_width < target_width) {
            return builder_.CreateZExt(value, target_type);
        }
    }
    if (value_type->isPointerTy() && target_type->isPointerTy()) {
        return builder_.CreateAddrSpaceCast(value, target_type);
    }
    UNREACHABLE();
}

void LLVMIrConstructor::ValueMapAdd(Inst *inst, llvm::Value *value, bool set_name)
{
    auto type = inst->GetType();
    auto ltype = GetExactType(type);
    ASSERT(input_map_.count(inst) == 0);
    auto it = input_map_.emplace(inst, GetGraph()->GetLocalAllocator()->Adapter());
    ASSERT(it.second);
    ArenaUnorderedMap<DataType::Type, llvm::Value *> &type_map = it.first->second;

    if (value != nullptr && set_name) {
        value->setName(CreateNameForInst(inst));
    }
    if (value == nullptr || inst->GetOpcode() == Opcode::LiveOut || !ltype->isIntegerTy()) {
        type_map.insert({type, value});
        if (type == DataType::POINTER) {
            ASSERT(value != nullptr);
            /*
             * Ark compiler implicitly converts:
             * 1. POINTER to REFERENCE
             * 2. POINTER to UINT64
             * 3. POINTER to INT64
             * Add value against those types, so we get pointer when requesting it in GetMappedValue.
             */
            type_map.insert({DataType::REFERENCE, value});
            type_map.insert({DataType::UINT64, value});
            if (inst->GetOpcode() == Opcode::LiveOut) {
                type_map.insert({DataType::INT64, value});
            }
        }
        return;
    }
    ASSERT(value->getType()->isIntegerTy());
    if (value->getType()->getIntegerBitWidth() > ltype->getIntegerBitWidth()) {
        value = builder_.CreateTrunc(value, ltype);
    } else if (value->getType()->getIntegerBitWidth() < ltype->getIntegerBitWidth()) {
        value = builder_.CreateZExt(value, ltype);
    }
    type_map.insert({type, value});
    FillValueMapForUsers(inst, value, type, &type_map);
}

void LLVMIrConstructor::FillValueMapForUsers(Inst *inst, llvm::Value *value, DataType::Type type,
                                             ArenaUnorderedMap<DataType::Type, llvm::Value *> *type_map)
{
    auto live_in = inst->GetOpcode() == Opcode::LiveIn ? value : nullptr;
    for (auto &user_item : inst->GetUsers()) {
        auto user = user_item.GetInst();
        for (unsigned i = 0; i < user->GetInputsCount(); i++) {
            auto itype = user->GetInputType(i);
            auto input = user->GetInput(i).GetInst();
            if (input != inst || itype == type || type_map->count(itype) != 0) {
                continue;
            }
            llvm::Value *cvalue;
            if (live_in != nullptr && itype == DataType::REFERENCE) {
                cvalue = live_in;
            } else if ((type == DataType::INT64 || type == DataType::UINT64) && itype == DataType::REFERENCE) {
                ASSERT(user->GetOpcode() == Opcode::LiveOut);
                cvalue = builder_.CreateIntToPtr(value, builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
            } else {
                cvalue = CoerceValue(value, type, itype);
            }
            type_map->insert({itype, cvalue});
        }
    }
}

// Instruction Visitors

// Constant and NullPtr are processed directly in GetInputValue
void LLVMIrConstructor::VisitConstant([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT(inst->GetBasicBlock()->IsStartBlock());
}

void LLVMIrConstructor::VisitNullPtr([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT(inst->GetBasicBlock()->IsStartBlock());
}

void LLVMIrConstructor::VisitLiveIn(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    ASSERT(inst->GetBasicBlock()->IsStartBlock());

    auto reg_input = std::find(ctor->cc_.begin(), ctor->cc_.end(), inst->CastToLiveIn()->GetDstReg());
    ASSERT(reg_input != ctor->cc_.end());
    auto idx = std::distance(ctor->cc_.begin(), reg_input);
    auto n = ctor->func_->arg_begin() + idx;
    ctor->ValueMapAdd(inst, ctor->CoerceValue(n, ctor->GetExactType(inst->GetType())));
}

void LLVMIrConstructor::VisitReturnVoid(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        ctor->CreateMemoryFence(memory_order::RELEASE);
    }
    ctor->builder_.CreateRetVoid();
}

void LLVMIrConstructor::VisitReturn(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto ret = ctor->GetInputValue(inst, 0);

    auto type = inst->GetType();
    if (DataType::IsLessInt32(type)) {
        ret = ctor->CoerceValue(ret, type, DataType::INT32);
    }

    ctor->builder_.CreateRet(ret);
}

void LLVMIrConstructor::VisitLiveOut(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto input = ctor->GetInputValue(inst, 0);

    auto reg_input = std::find(ctor->cc_.begin(), ctor->cc_.end(), inst->GetDstReg());
    ASSERT(reg_input != ctor->cc_.end());
    size_t idx = std::distance(ctor->cc_.begin(), reg_input);
    ASSERT(ctor->cc_values_[idx] == nullptr);

    // LiveOut not allowed for real frame register
    ASSERT(ctor->GetGraph()->GetArch() == Arch::AARCH64 || idx + 1 != ctor->cc_.size());
    auto value = ctor->CoerceValue(input, ctor->GetExactType(inst->GetType()));
    ctor->cc_values_[idx] = value;
    ctor->ValueMapAdd(inst, value, false);
}

void LLVMIrConstructor::VisitLoad(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto src_ptr = ctor->GetInputValue(inst, 0);
    ASSERT(src_ptr->getType()->isPointerTy());

    llvm::Value *offset;
    auto offset_input = inst->GetInput(1).GetInst();
    auto offset_itype = offset_input->GetType();
    if (offset_itype == DataType::UINT64 || offset_itype == DataType::INT64) {
        ASSERT(offset_input->GetOpcode() != Opcode::Load && offset_input->GetOpcode() != Opcode::LoadI);
        offset = ctor->GetInputValue(inst, 1, true);
    } else {
        offset = ctor->GetInputValue(inst, 1);
    }

    ASSERT(src_ptr->getType()->isPointerTy());
    auto ptr = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), src_ptr, offset);

    auto n = ctor->CreateLoadWithOrdering(inst, ptr, ToAtomicOrdering(inst->CastToLoad()->GetVolatile()));
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStore(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto src_ptr = ctor->GetInputValue(inst, 0);
    auto value = ctor->GetInputValue(inst, 2U);

    llvm::Value *offset;
    auto offset_input = inst->GetInput(1).GetInst();
    auto offset_itype = offset_input->GetType();
    if (offset_itype == DataType::UINT64 || offset_itype == DataType::INT64) {
        ASSERT(offset_input->GetOpcode() != Opcode::Load && offset_input->GetOpcode() != Opcode::LoadI);
        offset = ctor->GetInputValue(inst, 1, true);
    } else {
        offset = ctor->GetInputValue(inst, 1);
    }

    auto ptr_plus = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), src_ptr, offset);

    // Pre
    if (inst->CastToStore()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptr_plus);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, ptr_plus, ToAtomicOrdering(inst->CastToStore()->GetVolatile()));
    // Post
    if (inst->CastToStore()->GetNeedBarrier()) {
        ctor->CreatePostWRB(inst, src_ptr, offset, value);
    }
}

void LLVMIrConstructor::VisitLoadI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto src_ptr = ctor->GetInputValue(inst, 0);
    auto index = inst->CastToLoadI()->GetImm();

    ASSERT(src_ptr->getType()->isPointerTy());
    auto ptr_plus = ctor->builder_.CreateConstInBoundsGEP1_64(ctor->builder_.getInt8Ty(), src_ptr, index);

    auto n = ctor->CreateLoadWithOrdering(inst, ptr_plus, ToAtomicOrdering(inst->CastToLoadI()->GetVolatile()));
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStoreI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto src_ptr = ctor->GetInputValue(inst, 0);
    auto value = ctor->GetInputValue(inst, 1);

    auto index = inst->CastToStoreI()->GetImm();
    ASSERT(src_ptr->getType()->isPointerTy());
    auto ptr_plus = ctor->builder_.CreateConstInBoundsGEP1_64(ctor->builder_.getInt8Ty(), src_ptr, index);

    // Pre
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptr_plus);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, ptr_plus, ToAtomicOrdering(inst->CastToStoreI()->GetVolatile()));
    // Post
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        ctor->CreatePostWRB(inst, src_ptr, ctor->builder_.getInt32(index), value);
    }
}

void LLVMIrConstructor::VisitBitcast(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto type = inst->GetType();
    auto llvm_target_type = ctor->GetExactType(type);
    auto input = ctor->GetInputValue(inst, 0);
    auto itype = inst->GetInputType(0);

    llvm::Value *n;
    if (itype == DataType::POINTER) {
        ASSERT(!llvm_target_type->isPointerTy());
        n = ctor->builder_.CreatePtrToInt(input, llvm_target_type);
    } else {
        if (type == DataType::REFERENCE) {
            n = ctor->builder_.CreateIntToPtr(input, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
        } else if (type == DataType::POINTER) {
            n = ctor->builder_.CreateIntToPtr(input, ctor->builder_.getPtrTy());
        } else {
            n = ctor->builder_.CreateBitCast(input, llvm_target_type);
        }
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitCast(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto x = ctor->GetInputValue(inst, 0);

    auto type = inst->GetInputType(0);
    auto target_type = inst->GetType();
    auto llvm_target_type = ctor->GetExactType(target_type);
    // Do not cast if either Ark or LLVM types are the same
    if (type == target_type || x->getType() == llvm_target_type) {
        ctor->ValueMapAdd(inst, x, false);
        return;
    }

    if (DataType::IsFloatType(type) && IsInteger(target_type)) {
        // float to int, e.g. F64TOI32, F32TOI64, F64TOU32, F32TOU64
        auto n = ctor->CreateCastToInt(inst);
        ctor->ValueMapAdd(inst, n);
        return;
    }
    auto op = ctor->GetCastOp(type, target_type);
    if (target_type == DataType::BOOL) {
        ASSERT(op == llvm::Instruction::Trunc);
        auto u1 = ctor->builder_.CreateIsNotNull(x, CreateNameForInst(inst));
        auto n = ctor->builder_.CreateZExt(u1, ctor->builder_.getInt8Ty());
        ctor->ValueMapAdd(inst, n, false);
        return;
    }
    auto n = ctor->builder_.CreateCast(op, x, llvm_target_type);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAnd(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryOp(inst, llvm::Instruction::And);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAndI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::And, inst->CastToAndI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitOr(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryOp(inst, llvm::Instruction::Or);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitOrI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Or, inst->CastToOrI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitXor(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryOp(inst, llvm::Instruction::Xor);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitXorI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Xor, inst->CastToXorI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitShl(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateShiftOp(inst, llvm::Instruction::Shl);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitShlI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Shl, inst->CastToShlI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitShr(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateShiftOp(inst, llvm::Instruction::LShr);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitShrI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::LShr, inst->CastToShrI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAShr(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateShiftOp(inst, llvm::Instruction::AShr);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAShrI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::AShr, inst->CastToAShrI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAdd(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    llvm::Value *n;
    if (IsFloatType(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FAdd);
    } else if (IsTypeNumeric(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::Add);
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAddI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Add, inst->CastToAddI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitSub(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    llvm::Value *n;
    if (IsFloatType(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FSub);
    } else if (IsTypeNumeric(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::Sub);
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitSubI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Sub, inst->CastToSubI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitMul(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    llvm::Value *n;
    if (IsFloatType(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FMul);
    } else if (IsTypeNumeric(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::Mul);
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitMulI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Mul, inst->CastToMulI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitDiv(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto type = inst->GetType();
    llvm::Value *n;
    if (IsFloatType(type)) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FDiv);
    } else if (IsInteger(type)) {
        if (IsSignedInteger(type)) {
            n = ctor->CreateSignDivMod(inst, llvm::Instruction::SDiv);
        } else {
            n = ctor->CreateBinaryOp(inst, llvm::Instruction::UDiv);
        }
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitMod(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto type = inst->GetType();
    ASSERT(IsInteger(type));
    llvm::Value *n;
    if (IsSignedInteger(type)) {
        n = ctor->CreateSignDivMod(inst, llvm::Instruction::SRem);
    } else {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::URem);
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitCompare(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto compare_inst = inst->CastToCompare();
    auto operands_type = compare_inst->GetOperandsType();

    llvm::Value *x = ctor->GetInputValue(inst, 0);
    llvm::Value *y = ctor->GetInputValue(inst, 1);

    llvm::Value *n = nullptr;
    if (IsInteger(operands_type) || DataType::IsReference(operands_type)) {
        n = ctor->CreateCondition(compare_inst->GetCc(), x, y);
    } else {
        n = ctor->builder_.CreateFCmp(FCmpCodeConvert(compare_inst->GetCc()), x, y);
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitCmp(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    CmpInst *cmp_inst = inst->CastToCmp();
    DataType::Type operands_type = cmp_inst->GetOperandsType();

    auto x = ctor->GetInputValue(inst, 0);
    auto y = ctor->GetInputValue(inst, 1);
    llvm::Value *n;
    if (DataType::IsFloatType(operands_type)) {
        n = ctor->CreateFloatComparison(cmp_inst, x, y);
    } else if (IsInteger(operands_type)) {
        n = ctor->CreateIntegerComparison(cmp_inst, x, y);
    } else {
        ASSERT_DO(false, (std::cerr << "Unsupported comparison for operands of type = "
                                    << DataType::ToString(operands_type) << std::endl));
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitNeg(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto input_type = inst->GetInputType(0);
    auto to_negate = ctor->GetInputValue(inst, 0);
    llvm::Value *n;
    if (input_type == DataType::Type::FLOAT64 || input_type == DataType::Type::FLOAT32) {
        n = ctor->builder_.CreateFNeg(to_negate);
    } else if (IsInteger(input_type)) {
        n = ctor->builder_.CreateNeg(to_negate);
    } else {
        ASSERT_DO(false, (std::cerr << "Negation is not supported for" << DataType::ToString(input_type) << std::endl));
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitNot(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetInputsCount() == 1);
    ASSERT(IsInteger(inst->GetInputType(0)));

    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto input = ctor->GetInputValue(inst, 0);

    auto not_operator = ctor->builder_.CreateNot(input);
    ctor->ValueMapAdd(inst, not_operator);
}

void LLVMIrConstructor::VisitIfImm(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto x = ctor->GetInputValue(inst, 0);
    auto ifimm = inst->CastToIfImm();

    llvm::Value *cond = nullptr;
    if (ifimm->GetCc() == ConditionCode::CC_NE && ifimm->GetImm() == 0 && x->getType()->isIntegerTy()) {
        cond = ctor->builder_.CreateTrunc(x, ctor->builder_.getInt1Ty());
    } else {
        ASSERT(x->getType()->isIntOrPtrTy());
        llvm::Constant *imm_cst;
        if (x->getType()->isPointerTy()) {
            if (ifimm->GetImm() == 0) {
                imm_cst = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(x->getType()));
            } else {
                imm_cst = llvm::ConstantInt::getSigned(x->getType(), ifimm->GetImm());
                imm_cst = llvm::ConstantExpr::getPointerCast(imm_cst, x->getType());
            }
        } else {
            imm_cst = llvm::ConstantInt::getSigned(x->getType(), ifimm->GetImm());
        }
        cond = ctor->CreateCondition(ifimm->GetCc(), x, imm_cst);
    }
    ctor->CreateIf(inst, cond, ifimm->IsLikely(), ifimm->IsUnlikely());
}

void LLVMIrConstructor::VisitIf(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto x = ctor->GetInputValue(inst, 0);
    auto y = ctor->GetInputValue(inst, 1);
    ASSERT(x->getType()->isIntOrPtrTy());
    ASSERT(y->getType()->isIntOrPtrTy());
    auto ifi = inst->CastToIf();
    auto cond = ctor->CreateCondition(ifi->GetCc(), x, y);
    ctor->CreateIf(inst, cond, ifi->IsLikely(), ifi->IsUnlikely());
}

void LLVMIrConstructor::VisitCallIndirect(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto ptr = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(ptr, ctor->builder_.getPtrTy());
    // Build FunctionType
    ArenaVector<llvm::Type *> arg_types(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    ArenaVector<llvm::Value *> args(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 1; i < inst->GetInputs().Size(); ++i) {
        arg_types.push_back(ctor->GetType(inst->GetInput(i).GetInst()->GetType()));
        args.push_back(ctor->GetInputValue(inst, i));
    }
    auto ret_type = ctor->GetType(inst->GetType());
    auto func_type = llvm::FunctionType::get(ret_type, arg_types, false);
    auto call = ctor->builder_.CreateCall(func_type, ptr, args);
    if (!ret_type->isVoidTy()) {
        ctor->ValueMapAdd(inst, call);
    }
}

void LLVMIrConstructor::VisitCall(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    // Prepare external call if needed
    auto external_id = inst->CastToCall()->GetCallMethodId();
    auto runtime = ctor->GetGraph()->GetRuntime();
    auto external_name = runtime->GetExternalMethodName(ctor->GetGraph()->GetMethod(), external_id);
    auto function = ctor->func_->getParent()->getFunction(external_name);
    if (function == nullptr) {
        ArenaVector<llvm::Type *> arg_types(ctor->GetGraph()->GetLocalAllocator()->Adapter());
        for (size_t i = 0; i < inst->GetInputs().Size(); ++i) {
            arg_types.push_back(ctor->GetType(inst->GetInputType(i)));
        }
        auto ftype = llvm::FunctionType::get(ctor->GetType(inst->GetType()), arg_types, false);
        function =
            llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, external_name, ctor->func_->getParent());
    }
    // Arguments
    ArenaVector<llvm::Value *> args(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 0; i < inst->GetInputs().Size(); ++i) {
        args.push_back(ctor->CoerceValue(ctor->GetInputValue(inst, i), function->getArg(i)->getType()));
    }
    // Call
    auto call = ctor->builder_.CreateCall(function->getFunctionType(), function, args);

    if (IsNoAliasIrtocFunction(external_name)) {
        ASSERT(call->getType()->isPointerTy());
        call->addRetAttr(llvm::Attribute::NoAlias);
    } else {
        ASSERT(call->getType()->isPointerTy() ^ !IsPtrIgnIrtocFunction(external_name));
    }

    // Check if function has debug info
    if (function->getSubprogram() != nullptr) {
        ctor->debug_data_->SetLocation(call, inst->GetPc());
    }

    if (inst->GetType() != DataType::VOID) {
        ctor->ValueMapAdd(inst, ctor->CoerceValue(call, ctor->GetType(inst->GetType())));
    }
}

void LLVMIrConstructor::VisitPhi(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto ltype = ctor->GetExactType(inst->GetType());
    auto block = ctor->GetCurrentBasicBlock();

    // PHI need adjusted insert point if ValueMapAdd already created coerced values for other PHIs
    auto non_phi = block->getFirstNonPHI();
    if (non_phi != nullptr) {
        ctor->builder_.SetInsertPoint(non_phi);
    }

    auto phi = ctor->builder_.CreatePHI(ltype, inst->GetInputsCount());
    ctor->SetCurrentBasicBlock(block);
    ctor->ValueMapAdd(inst, phi);
}

void LLVMIrConstructor::VisitIntrinsic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto entry_id = inst->CastToIntrinsic()->GetIntrinsicId();

    // Some intrinsics are lowered into some code or LLVM intrinsics. For LLVM intrinsics final decision about
    // lowering into code or call is made later in IntrinsicsLowering
    if (OPTIONS.IsCompilerEncodeIntrinsics()) {
        bool lowered = ctor->TryEmitIntrinsic(inst, entry_id);
        if (lowered) {
            return;
        }
        // Every intrinsic that panda can encode should be lowered
        ASSERT(!EncodesBuiltin(ctor->GetGraph()->GetRuntime(), entry_id, ctor->GetGraph()->GetArch()));
    }

    UNREACHABLE();
}

void LLVMIrConstructor::VisitDefault([[maybe_unused]] Inst *inst)
{
    ASSERT_DO(false, (std::cerr << "Unsupported llvm lowering for \n", inst->Dump(&std::cerr, true)));
    UNREACHABLE();
}

LLVMIrConstructor::LLVMIrConstructor(Graph *graph, llvm::Module *module, llvm::LLVMContext *context,
                                     LLVMArkInterface *ark_interface,
                                     const std::unique_ptr<DebugDataBuilder> &debug_data)
    : graph_(graph),
      builder_(llvm::IRBuilder<>(*context)),
      input_map_(graph->GetLocalAllocator()->Adapter()),
      block_tail_map_(graph->GetLocalAllocator()->Adapter()),
      block_head_map_(graph->GetLocalAllocator()->Adapter()),
      ark_interface_(ark_interface),
      debug_data_(debug_data),
      cc_(graph->GetLocalAllocator()->Adapter()),
      cc_values_(graph->GetLocalAllocator()->Adapter())
{
    // Assign regmaps
    if (graph->GetArch() == Arch::AARCH64) {
        cc_.assign({AARCH64_PC, AARCH64_ACC, AARCH64_ACC_TAG, AARCH64_FP, AARCH64_DISPATCH, AARCH64_MOFFSET,
                    AARCH64_METHOD_PTR, GetThreadReg(Arch::AARCH64)});
    } else if (graph->GetArch() == Arch::X86_64) {
        cc_.assign({X86_64_PC, X86_64_ACC, X86_64_ACC_TAG, X86_64_FP, X86_64_DISPATCH, GetThreadReg(Arch::X86_64),
                    X86_64_REAL_FP});
    } else {
        LLVM_LOG(FATAL, ENTRY) << "Unsupported architecture for arkintcc";
    }

    cc_values_.assign(cc_.size(), nullptr);

    // Create function
    auto func_proto = GetEntryFunctionType();
    auto method_name = ark_interface_->GetUniqMethodName(graph_->GetMethod());
    func_ = CreateFunctionDeclaration(func_proto, method_name, module);
    ark_interface_->PutFunction(graph_->GetMethod(), func_);

    auto klass_id = graph_->GetRuntime()->GetClassIdForMethod(graph_->GetMethod());
    auto klass_id_md = llvm::ConstantAsMetadata::get(builder_.getInt32(klass_id));
    func_->addMetadata(llvmbackend::LLVMArkInterface::FUNCTION_MD_CLASS_ID,
                       *llvm::MDNode::get(*context, {klass_id_md}));
}

bool LLVMIrConstructor::BuildIr()
{
    LLVM_LOG(DEBUG, IR) << "Building IR for LLVM";

    // Set Argument Names
    auto it = func_->arg_begin();
    auto idx = 0;
    while (it != func_->arg_end()) {
        std::stringstream name;
        name << "a" << idx++;
        (it++)->setName(name.str());
    }

    auto method = graph_->GetMethod();
    auto runtime = graph_->GetRuntime();
    ark_interface_->RememberFunctionOrigin(func_, method);
    debug_data_->BeginSubprogram(func_, runtime->GetFullFileName(method), runtime->GetMethodId(method));

    // First step - create blocks, leaving LLVM EntryBlock untouched
    BuildBasicBlocks();
    // Second step - visit all instructions, including StartBlock, but not filling PHI inputs
    BuildInstructions();
    // Third step is to fill the PHIs inputs
    FillPhiInputs();

    debug_data_->EndSubprogram(func_);

    // verifyFunction returns false if there are no errors. But we return true if everything is ok.
    auto verified = !verifyFunction(*func_, &llvm::errs());
    if (!verified) {
        func_->print(llvm::errs());
    }
    return verified;
}

Expected<bool, std::string> LLVMIrConstructor::CanCompile(Graph *graph)
{
    if (graph->IsDynamicMethod()) {
        return false;
    }
    bool can = true;
    for (auto basic_block : graph->GetBlocksRPO()) {
        for (auto inst : basic_block->AllInsts()) {
            bool can_compile = LLVMIrConstructor::CanCompile(inst);
            if (!can_compile) {
                LLVM_LOG(ERROR, ENTRY) << GetOpcodeString(inst->GetOpcode())
                                       << " unexpected in LLVM lowering. Method = "
                                       << graph->GetRuntime()->GetMethodFullName(graph->GetMethod());
                return Unexpected(std::string("Unexpected opcode in CanCompile"));
            }
        }
    }
    return can;
}

bool LLVMIrConstructor::CanCompile(Inst *inst)
{
    if (inst->IsIntrinsic()) {
        auto iid = inst->CastToIntrinsic()->GetIntrinsicId();
        // We support only slowpaths where the second immediate is an external function
        if (iid == RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY) {
            return inst->CastToIntrinsic()->GetImms().size() > 1;
        }
        return CanCompileIntrinsic(iid);
    }
    // Check if we have method that can handle it
    switch (inst->GetOpcode()) {
        default:
            UNREACHABLE_CONSTEXPR();
            // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, ...)                                                     \
    case Opcode::OPCODE: {                                                        \
        return &LLVMIrConstructor::Visit##OPCODE != &GraphVisitor::Visit##OPCODE; \
    }
            OPCODE_LIST(INST_DEF)
    }
#undef INST_DEF
}

}  // namespace panda::compiler
