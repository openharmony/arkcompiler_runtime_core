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

static constexpr unsigned VECTOR_SIZE_8 = 8;
static constexpr unsigned VECTOR_SIZE_16 = 16;

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
inline llvm::Function *CreateFunctionDeclaration(llvm::FunctionType *functionType, const std::string &name,
                                                 llvm::Module *module)
{
    ASSERT(functionType != nullptr);
    ASSERT(!name.empty());
    ASSERT(module != nullptr);

    auto function = module->getFunction(name);
    if (function != nullptr) {
        ASSERT(function->getVisibility() == llvm::GlobalValue::ProtectedVisibility);
        ASSERT(function->doesNotThrow());
        return function;
    }

    function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, module);

    // Prevens emitting `.eh_frame` section
    function->setDoesNotThrow();

    function->setVisibility(llvm::GlobalValue::ProtectedVisibility);

    function->setSectionPrefix(name);

    return function;
}

inline void CreateBlackBoxAsm(llvm::IRBuilder<> *builder, const std::string &inlineAsm)
{
    auto iasmType = llvm::FunctionType::get(builder->getVoidTy(), {}, false);
    builder->CreateCall(iasmType, llvm::InlineAsm::get(iasmType, inlineAsm, "", true), {});
}

inline void CreateInt32ImmAsm(llvm::IRBuilder<> *builder, const std::string &inlineAsm, uint32_t imm)
{
    auto oneInt = llvm::FunctionType::get(builder->getVoidTy(), {builder->getInt32Ty()}, false);
    builder->CreateCall(oneInt, llvm::InlineAsm::get(oneInt, inlineAsm, "i", true), {builder->getInt32(imm)});
}

inline llvm::AtomicOrdering ToAtomicOrdering(bool isVolatile)
{
    return isVolatile ? LLVMArkInterface::VOLATILE_ORDER : LLVMArkInterface::NOT_ATOMIC_ORDER;
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
static inline std::string CreateBasicBlockName(Inst *inst, const std::string &bbName)
{
    std::stringstream name;
    name << "bb" << std::to_string(inst->GetBasicBlock()->GetId()) << "_i" << std::to_string(inst->GetId()) << ".."
         << bbName << "..";
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

static inline llvm::FCmpInst::Predicate FCmpCodeConvert(ConditionCode conditionCode)
{
    switch (conditionCode) {
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
            ASSERT_DO(false, (std::cerr << "Unexpected condition_code = " << conditionCode << std::endl));
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

bool LLVMIrConstructor::TryEmitIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId arkId)
{
    switch (arkId) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_UNREACHABLE:
            return EmitUnreachable();
        case RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY:
            return EmitSlowPathEntry(inst);
        case RuntimeInterface::IntrinsicId::INTRINSIC_LOAD_ACQUIRE_MARK_WORD_EXCLUSIVE:
            return EmitExclusiveLoadWithAcquire(inst);
        case RuntimeInterface::IntrinsicId::INTRINSIC_STORE_RELEASE_MARK_WORD_EXCLUSIVE:
            return EmitExclusiveStoreWithRelease(inst);
        // LLVM encodes them using calling conventions
        case RuntimeInterface::IntrinsicId::INTRINSIC_SAVE_REGISTERS_EP:
        case RuntimeInterface::IntrinsicId::INTRINSIC_RESTORE_REGISTERS_EP:
            return true;
        case RuntimeInterface::IntrinsicId::INTRINSIC_INTERPRETER_RETURN:
            return EmitInterpreterReturn();
        case RuntimeInterface::IntrinsicId::INTRINSIC_TAIL_CALL:
            return EmitTailCall(inst);
        case RuntimeInterface::IntrinsicId::INTRINSIC_DATA_MEMORY_BARRIER_FULL:
            return EmitMemoryFence(memory_order::FULL);
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPRESS_EIGHT_UTF16_TO_UTF8_CHARS_USING_SIMD:
            return EmitCompressUtf16ToUtf8CharsUsingSimd<VECTOR_SIZE_8>(inst);
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPRESS_SIXTEEN_UTF16_TO_UTF8_CHARS_USING_SIMD:
            return EmitCompressUtf16ToUtf8CharsUsingSimd<VECTOR_SIZE_16>(inst);
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

bool LLVMIrConstructor::EmitSlowPathEntry(Inst *inst)
{
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(func_->getCallingConv() == llvm::CallingConv::ArkFast0 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast1 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast2 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast3 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast4 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast5);

    // Arguments
    ArenaVector<llvm::Value *> args(GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 0; i < inst->GetInputs().Size(); ++i) {
        args.push_back(GetInputValue(inst, i));
    }
    auto threadRegPtr = builder_.CreateIntToPtr(GetThreadRegValue(), builder_.getPtrTy());
    auto frameRegPtr = builder_.CreateIntToPtr(GetRealFrameRegValue(), builder_.getPtrTy());
    args.push_back(threadRegPtr);
    args.push_back(frameRegPtr);

    // Types
    ArenaVector<llvm::Type *> argTypes(GetGraph()->GetLocalAllocator()->Adapter());
    for (const auto &input : inst->GetInputs()) {
        argTypes.push_back(GetExactType(input.GetInst()->GetType()));
    }
    argTypes.push_back(builder_.getPtrTy());
    argTypes.push_back(builder_.getPtrTy());
    auto ftype = llvm::FunctionType::get(GetType(inst->GetType()), argTypes, false);

    ASSERT(inst->CastToIntrinsic()->GetImms().size() == 2U);
    uint32_t offset = inst->CastToIntrinsic()->GetImms()[1];

    auto addr = builder_.CreateConstInBoundsGEP1_64(builder_.getInt8Ty(), threadRegPtr, offset);
    auto callee = builder_.CreateLoad(builder_.getPtrTy(), addr);

    auto call = builder_.CreateCall(ftype, callee, args, "");
    call->setCallingConv(func_->getCallingConv());
    call->setTailCallKind(llvm::CallInst::TailCallKind::TCK_MustTail);
    if (call->getType()->isVoidTy()) {
        builder_.CreateRetVoid();
    } else {
        builder_.CreateRet(call);
    }
    return true;
}

bool LLVMIrConstructor::EmitExclusiveLoadWithAcquire(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(inst->GetInputType(0) == DataType::POINTER);
    auto &ctx = func_->getContext();
    auto addr = GetInputValue(inst, 0);
    auto dstType = GetExactType(inst->GetType());
    auto intrinsicId = llvm::Intrinsic::AARCH64Intrinsics::aarch64_ldaxr;
    auto load = builder_.CreateUnaryIntrinsic(intrinsicId, addr);
    load->addParamAttr(0, llvm::Attribute::get(ctx, llvm::Attribute::ElementType, dstType));
    ValueMapAdd(inst, load);
    return true;
}

bool LLVMIrConstructor::EmitExclusiveStoreWithRelease(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(inst->GetInputType(0) == DataType::POINTER);
    auto &ctx = func_->getContext();
    auto addr = GetInputValue(inst, 0);
    auto value = GetInputValue(inst, 1);
    auto type = value->getType();
    auto intrinsicId = llvm::Intrinsic::AARCH64Intrinsics::aarch64_stlxr;
    auto stlxr = llvm::Intrinsic::getDeclaration(func_->getParent(), intrinsicId, builder_.getPtrTy());
    value = builder_.CreateZExtOrBitCast(value, stlxr->getFunctionType()->getParamType(0));
    auto store = builder_.CreateCall(stlxr, {value, addr});
    store->addParamAttr(1, llvm::Attribute::get(ctx, llvm::Attribute::ElementType, type));
    ValueMapAdd(inst, store);
    return true;
}

bool LLVMIrConstructor::EmitInterpreterReturn()
{
    ASSERT(GetGraph()->GetMode().IsInterpreter());

    // This constant is hardcoded in codegen_interpreter.h and in interpreter.irt
    constexpr size_t SPILL_SLOTS = 32;
    CFrameLayout fl(GetGraph()->GetArch(), SPILL_SLOTS);
    constexpr bool SAVE_UNUSED_CALLEE_REGS = true;

    // Restore callee-registers
    auto calleeRegsMask = GetCalleeRegsMask(GetGraph()->GetArch(), false, SAVE_UNUSED_CALLEE_REGS);
    auto calleeVregsMask = GetCalleeRegsMask(GetGraph()->GetArch(), true, SAVE_UNUSED_CALLEE_REGS);
    if (GetGraph()->GetArch() == Arch::AARCH64) {
        constexpr bool SAVE_FRAME_AND_LINK_REGS = true;

        size_t slotSize = fl.GetSlotSize();
        size_t dslotSize = slotSize * 2U;

        auto lastCalleeReg = fl.GetRegsSlotsCount() - calleeRegsMask.Count();
        auto lastCalleeVreg = fl.GetRegsSlotsCount() - fl.GetCalleeRegistersCount(false) - calleeVregsMask.Count();
        CreateInterpreterReturnRestoreRegs(calleeRegsMask, lastCalleeReg, false);
        CreateInterpreterReturnRestoreRegs(calleeVregsMask, lastCalleeVreg, true);

        // Adjust SP
        auto spToFrameTopSlots = fl.GetRegsSlotsCount() + CFrameRegs::Start() - CFrameReturnAddr::Start();
        if (SAVE_FRAME_AND_LINK_REGS) {
            spToFrameTopSlots -= CFrameLayout::GetFpLrSlotsCount();
        }

        CreateInt32ImmAsm(&builder_,
                          std::string("add  sp, sp, $0").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT),
                          spToFrameTopSlots * slotSize);
        CreateInt32ImmAsm(&builder_, "ldp  x29, x30, [sp], $0", dslotSize);
        CreateBlackBoxAsm(&builder_, "ret");
    } else {
        // Currently there is no vector regs usage in x86_64 handlers
        ASSERT(calleeVregsMask.count() == 0);
        auto regShift = DOUBLE_WORD_SIZE_BYTES *
                        (fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true));
        auto fpShift = DOUBLE_WORD_SIZE_BYTES * (2 + CFrameSlots::Start() - CFrameData::Start());

        std::string iasmStr =
            std::string("leaq  ${0:c}(%rsp), %rsp").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT);
        CreateInt32ImmAsm(&builder_, iasmStr, regShift);
        Target target {GetGraph()->GetArch()};
        while (calleeRegsMask.count() > 0) {
            auto reg = calleeRegsMask.GetMinRegister();
            calleeRegsMask ^= 1U << reg;
            iasmStr = "pop  %" + target.GetRegName(reg, false);
            CreateBlackBoxAsm(&builder_, iasmStr);
        }
        iasmStr = "leaq  " + std::to_string(fpShift) + "(%rsp), %rsp";
        CreateBlackBoxAsm(&builder_, iasmStr);
        CreateBlackBoxAsm(&builder_, "pop  %rbp");
        CreateBlackBoxAsm(&builder_, "retq");
    }
    builder_.CreateUnreachable();

    return true;
}

bool LLVMIrConstructor::EmitTailCall(Inst *inst)
{
    ASSERT(func_->getCallingConv() == llvm::CallingConv::ArkFast0 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast1 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast2 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast3 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast4 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast5 ||
           func_->getCallingConv() == llvm::CallingConv::ArkInt);
    llvm::CallInst *call;

    if (GetGraph()->GetMode().IsFastPath()) {
        call = CreateTailCallFastPath(inst);
    } else if (GetGraph()->GetMode().IsInterpreter()) {
        call = CreateTailCallInterpreter(inst);
    } else {
        UNREACHABLE();
    }
    call->setCallingConv(func_->getCallingConv());
    call->setTailCallKind(llvm::CallInst::TailCallKind::TCK_Tail);
    if (func_->getReturnType()->isVoidTy()) {
        builder_.CreateRetVoid();
    } else {
        builder_.CreateRet(call);
    }
    std::fill(ccValues_.begin(), ccValues_.end(), nullptr);
    return true;
}

bool LLVMIrConstructor::EmitMemoryFence(memory_order::Order order)
{
    CreateMemoryFence(order);
    return true;
}

template <uint32_t VECTOR_SIZE>
bool LLVMIrConstructor::EmitCompressUtf16ToUtf8CharsUsingSimd(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(inst->GetInputType(0) == DataType::POINTER);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);
    static_assert(VECTOR_SIZE == VECTOR_SIZE_8 || VECTOR_SIZE == VECTOR_SIZE_16, "Unexpected vector size");
    auto intrinsicId = llvm::Intrinsic::AARCH64Intrinsics::aarch64_neon_ld2;
    auto vecTy = llvm::VectorType::get(builder_.getInt8Ty(), VECTOR_SIZE, false);

    auto u16Ptr = GetInputValue(inst, 0);  // ptr to src array of utf16 chars
    auto u8Ptr = GetInputValue(inst, 1);   // ptr to dst array of utf8 chars
    auto ld2 = llvm::Intrinsic::getDeclaration(func_->getParent(), intrinsicId, {vecTy, u16Ptr->getType()});
    auto vld2 = builder_.CreateCall(ld2, {u16Ptr});
    auto u8Vec = builder_.CreateExtractValue(vld2, {0});
    builder_.CreateStore(u8Vec, u8Ptr);
    return true;
}

llvm::Value *LLVMIrConstructor::GetMappedValue(Inst *inst, DataType::Type type)
{
    ASSERT(inputMap_.count(inst) == 1);
    auto &typeMap = inputMap_.at(inst);
    ASSERT(typeMap.count(type) == 1);
    auto result = typeMap.at(type);
    ASSERT(result != nullptr);
    return result;
}

llvm::Value *LLVMIrConstructor::GetInputValue(Inst *inst, size_t index, bool skipCoerce)
{
    auto input = inst->GetInput(index).GetInst();
    auto type = inst->GetInputType(index);
    ASSERT(type != DataType::NO_TYPE);

    if (skipCoerce) {
        ASSERT(input->GetType() == DataType::UINT64 || input->GetType() == DataType::INT64);
        type = input->GetType();
    }

    if (input->IsConst()) {
        return GetInputValueFromConstant(input->CastToConstant(), type);
    }
    if (input->GetOpcode() == Opcode::NullPtr) {
        auto llvmType = GetExactType(DataType::REFERENCE);
        ASSERT(llvmType == builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
        return llvm::Constant::getNullValue(llvmType);
    }
    return GetMappedValue(input, type);
}

llvm::Value *LLVMIrConstructor::GetInputValueFromConstant(ConstantInst *constant, DataType::Type pandaType)
{
    auto llvmType = GetExactType(pandaType);
    if (pandaType == DataType::FLOAT64) {
        double value = constant->GetDoubleValue();
        return llvm::ConstantFP::get(llvmType, value);
    }
    if (pandaType == DataType::FLOAT32) {
        float value = constant->GetFloatValue();
        return llvm::ConstantFP::get(llvmType, value);
    }
    if (pandaType == DataType::POINTER) {
        auto cval = static_cast<int64_t>(constant->GetIntValue());
        auto integer = builder_.getInt64(cval);
        return builder_.CreateIntToPtr(integer, builder_.getPtrTy());
    }
    if (DataType::IsTypeNumeric(pandaType)) {
        auto isSigned = DataType::IsTypeSigned(pandaType);
        auto cval = static_cast<int64_t>(constant->GetIntValue());
        return llvm::ConstantInt::get(llvmType, cval, isSigned);
    }
    if (DataType::IsReference(pandaType) && constant->GetRawValue() == 0) {
        return llvm::Constant::getNullValue(llvmType);
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
        if (!graph_->GetMode().IsInterpreter()) {
            continue;
        }
        for (auto inst : block->AllInsts()) {
            if (inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() ==
                                           RuntimeInterface::IntrinsicId::INTRINSIC_INTERPRETER_RETURN) {
                arkInterface_->AppendIrtocReturnHandler(func_->getName());
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
                auto inputBlock = block->GetPredBlockByIndex(i);
                auto input = GetInputValue(inst, i);
                phi->addIncoming(input, GetTailBlock(inputBlock));
            }
        }
    }
}

// Creator functions for internal usage

void LLVMIrConstructor::CreateInterpreterReturnRestoreRegs(RegMask &regMask, size_t offset, bool fp)
{
    int32_t slotSize = PointerSize(GetGraph()->GetArch());
    int32_t dslotSize = slotSize * 2U;
    int32_t totalSize = regMask.count() * slotSize;
    auto startRegOffset = offset * DOUBLE_WORD_SIZE_BYTES;
    auto endRegOffset = startRegOffset + std::max(0, totalSize - dslotSize);

    constexpr uint32_t MAX_REPR_VAL = 504U;
    bool representable = startRegOffset <= MAX_REPR_VAL && (startRegOffset & 0x7U) == 0 &&
                         endRegOffset <= MAX_REPR_VAL && (endRegOffset & 0x7U) == 0;

    std::string baseReg = representable ? "sp" : "x16";
    if (!representable) {
        CreateInt32ImmAsm(&builder_,
                          std::string("add  x16, sp, $0").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT),
                          startRegOffset);
        startRegOffset = 0;
    }

    while (regMask.count() > 0) {
        std::string asmString = regMask.count() / 2U > 0 ? "ldp " : "ldr ";
        auto first = regMask.GetMinRegister();
        asmString += (fp ? "d" : "x") + std::to_string(first);
        regMask ^= 1U << first;
        if (regMask.count() > 0) {
            asmString += ", ";
            auto second = regMask.GetMinRegister();
            asmString += (fp ? "d" : "x") + std::to_string(second);
            regMask ^= 1U << second;
        }
        asmString += ", [";
        asmString += baseReg;
        asmString += ", $0]";
        if (representable) {
            asmString += LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT;
        }
        CreateInt32ImmAsm(&builder_, asmString, startRegOffset);
        startRegOffset += dslotSize;
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
    auto targetType = inst->GetType();
    bool target64 = (targetType == DataType::UINT64) || (targetType == DataType::INT64);
    auto constexpr SHIFT32_RANGE = 0x1f;
    auto constexpr SHIFT64_RANGE = 0x3f;

    y = builder_.CreateBinOp(llvm::Instruction::And, y,
                             llvm::ConstantInt::get(y->getType(), target64 ? SHIFT64_RANGE : SHIFT32_RANGE));

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
    auto eqM1 = builder_.CreateICmpEQ(y, llvm::ConstantInt::get(type, -1));
    auto m1Bb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "divmod_minus1"), func_);
    auto notM1Bb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "divmod_normal"), func_);
    auto contBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "divmod_cont"), func_);
    builder_.CreateCondBr(eqM1, m1Bb, notM1Bb);

    SetCurrentBasicBlock(m1Bb);
    llvm::Value *m1Result;
    if (opcode == llvm::Instruction::SDiv) {
        m1Result = builder_.CreateNeg(x);
    } else {
        m1Result = llvm::ConstantInt::get(type, 0);
    }
    builder_.CreateBr(contBb);

    SetCurrentBasicBlock(notM1Bb);
    auto result = builder_.CreateBinOp(opcode, x, y);
    builder_.CreateBr(contBb);

    SetCurrentBasicBlock(contBb);
    auto resultPhi = builder_.CreatePHI(y->getType(), 2U);
    resultPhi->addIncoming(m1Result, m1Bb);
    resultPhi->addIncoming(result, notM1Bb);
    return resultPhi;
}

llvm::Value *LLVMIrConstructor::CreateAArch64SignDivMod(Inst *inst, llvm::Instruction::BinaryOps opcode, llvm::Value *x,
                                                        llvm::Value *y)
{  // LLVM opcodes SDiv/SRem don't fully suit our needs (int_min/-1 case is UB),
    // but for now we inline assembly only for arm64 sdiv
    auto targetType = inst->GetType();
    bool target64 = (targetType == DataType::INT64);
    llvm::Value *quotient {nullptr};
    llvm::Value *result {nullptr};
    llvm::Type *type {nullptr};
    {
        std::string itext;
        if (targetType == DataType::INT8) {
            type = builder_.getInt8Ty();
            itext += "sxtb ${1:w}, ${1:w}\nsxtb ${2:w}, ${2:w}\n";
        } else if (targetType == DataType::INT16) {
            type = builder_.getInt16Ty();
            itext += "sxth ${1:w}, ${1:w}\nsxth ${2:w}, ${2:w}\n";
        } else {
            ASSERT(target64 || targetType == DataType::INT32);
            type = target64 ? builder_.getInt64Ty() : builder_.getInt32Ty();
        }
        itext += target64 ? "sdiv ${0:x}, ${1:x}, ${2:x}" : "sdiv ${0:w}, ${1:w}, ${2:w}";
        auto itype = llvm::FunctionType::get(type, {type, type}, false);  // no var args
        quotient = builder_.CreateCall(itype, llvm::InlineAsm::get(itype, itext, "=r,r,r", false), {x, y});
        result = quotient;
    }

    if (opcode == llvm::Instruction::SRem) {
        auto modAsmType = llvm::FunctionType::get(type, {type, type, type}, false);  // no var args
        // Inline asm "sdiv q, x, y" yields q = x / y
        // Inline asm "msub r, x, y, q" yields r = x - y * q
        std::string_view modAsm =
            target64 ? "msub ${0:x}, ${3:x}, ${2:x}, ${1:x}" : "msub ${0:w}, ${3:w}, ${2:w}, ${1:w}";
        auto remainder = builder_.CreateCall(modAsmType, llvm::InlineAsm::get(modAsmType, modAsm, "=r,r,r,r", false),
                                             {x, y, result});
        result = remainder;
    }
    return result;
}

llvm::Value *LLVMIrConstructor::CreateFloatComparison(CmpInst *cmpInst, llvm::Value *x, llvm::Value *y)
{
    // if x is less than y then return -1
    // else return zero extend of (x > y)
    llvm::CmpInst::Predicate greaterThanPredicate;
    llvm::CmpInst::Predicate lessThanPredicate;
    if (cmpInst->IsFcmpg()) {
        // if x or y is nan then greater_than_predicate yields true
        greaterThanPredicate = llvm::CmpInst::FCMP_UGT;
        lessThanPredicate = llvm::CmpInst::FCMP_OLT;
    } else if (cmpInst->IsFcmpl()) {
        greaterThanPredicate = llvm::CmpInst::FCMP_OGT;
        // if x or y is nan then less_than_predicate yields true
        lessThanPredicate = llvm::CmpInst::FCMP_ULT;
    } else {
        ASSERT_PRINT(false, "cmp_inst must be either Fcmpg, or Fcmpl");
        UNREACHABLE();
    }
    // x > y || (inst == Fcmpg && (x == NaN || y == NaN))
    auto greaterThan = builder_.CreateFCmp(greaterThanPredicate, x, y);
    // x < y || (inst == Fcmpl && (x == NaN || y == NaN))
    auto lessThan = builder_.CreateFCmp(lessThanPredicate, x, y);
    auto comparison = builder_.CreateZExt(greaterThan, builder_.getInt32Ty());
    auto negativeOne = builder_.getInt32(-1);
    return builder_.CreateSelect(lessThan, negativeOne, comparison);
}

llvm::Value *LLVMIrConstructor::CreateIntegerComparison(CmpInst *inst, llvm::Value *x, llvm::Value *y)
{
    ASSERT(x->getType() == y->getType());
    llvm::Value *greaterThan;
    llvm::Value *lessThan;

    if (DataType::IsTypeSigned(inst->GetOperandsType())) {
        greaterThan = builder_.CreateICmpSGT(x, y);
        lessThan = builder_.CreateICmpSLT(x, y);
    } else {
        greaterThan = builder_.CreateICmpUGT(x, y);
        lessThan = builder_.CreateICmpULT(x, y);
    }
    auto castComparisonResult = builder_.CreateZExt(greaterThan, builder_.getInt32Ty());
    auto negativeOne = builder_.getInt32(-1);
    return builder_.CreateSelect(lessThan, negativeOne, castComparisonResult);
}

llvm::Value *LLVMIrConstructor::CreateCastToInt(Inst *inst)
{
    ASSERT(inst->GetInputsCount() == 1);

    llvm::Value *input = GetInputValue(inst, 0);
    auto sourceType = input->getType();
    auto targetType = inst->GetType();

    ASSERT_DO(sourceType->isFloatTy() || sourceType->isDoubleTy(),
              std::cerr << "Unexpected source type: " << GetTypeName(sourceType) << ". Should be a float or double."
                        << std::endl);

    auto llvmId = DataType::IsTypeSigned(targetType) ? llvm::Intrinsic::fptosi_sat : llvm::Intrinsic::fptoui_sat;
    ArenaVector<llvm::Type *> intrinsicTypes(GetGraph()->GetLocalAllocator()->Adapter());
    intrinsicTypes.assign({GetExactType(targetType), sourceType});
    return builder_.CreateIntrinsic(llvmId, intrinsicTypes, {input}, nullptr);
}

llvm::Value *LLVMIrConstructor::CreateLoadWithOrdering(Inst *inst, llvm::Value *value, llvm::AtomicOrdering ordering,
                                                       const llvm::Twine &name)
{
    auto pandaType = inst->GetType();
    llvm::Type *type = GetExactType(pandaType);

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
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "pre_wrb_out"), func_);
    EmitPreWRB(&builder_, mem, IsVolatileMemInst(inst), outBb, arkInterface_, GetThreadRegValue());
}

void LLVMIrConstructor::CreatePostWRB(Inst *inst, llvm::Value *mem, llvm::Value *offset, llvm::Value *value)
{
    ASSERT(GetGraph()->GetRuntime()->GetPostType() == panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER ||
           GetGraph()->GetRuntime()->GetPostType() == panda::mem::BarrierType::POST_INTERREGION_BARRIER);

    Inst *secondValue;
    Inst *val = InstStoredValue(inst, &secondValue);
    ASSERT(secondValue == nullptr);

    if (val->GetOpcode() == Opcode::NullPtr) {
        return;
    }

    ASSERT(GetGraph()->GetMode().IsInterpreter());
    llvm::Value *frameRegValue = nullptr;
    if (GetGraph()->GetArch() == Arch::X86_64) {
        frameRegValue = GetRealFrameRegValue();
    }
    EmitPostWRB(&builder_, mem, offset, value, arkInterface_, GetThreadRegValue(), frameRegValue);
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

llvm::CallInst *LLVMIrConstructor::CreateTailCallFastPath(Inst *inst)
{
    ASSERT(inst->GetInputs().Size() == 0);
    // Arguments
    ArenaVector<llvm::Value *> args(GetGraph()->GetLocalAllocator()->Adapter());
    ArenaVector<llvm::Type *> argTypes(GetGraph()->GetLocalAllocator()->Adapter());
    ASSERT(ccValues_.size() == func_->arg_size());
    for (size_t i = 0; i < func_->arg_size(); ++i) {
        args.push_back(i < ccValues_.size() && ccValues_.at(i) != nullptr ? ccValues_.at(i) : func_->getArg(i));
        argTypes.push_back(args.at(i)->getType());
    }
    ASSERT(inst->CastToIntrinsic()->HasImms() && inst->CastToIntrinsic()->GetImms().size() == 1U);
    uint32_t offset = inst->CastToIntrinsic()->GetImms()[0];

    auto ftype = llvm::FunctionType::get(func_->getReturnType(), argTypes, false);

    auto threadRegPtr = builder_.CreateIntToPtr(GetThreadRegValue(), builder_.getPtrTy());
    auto addr = builder_.CreateConstInBoundsGEP1_64(builder_.getInt8Ty(), threadRegPtr, offset);
    auto callee = builder_.CreateLoad(builder_.getPtrTy(), addr);

    return builder_.CreateCall(ftype, callee, args);
}

llvm::CallInst *LLVMIrConstructor::CreateTailCallInterpreter(Inst *inst)
{
    auto ptr = GetInputValue(inst, 0);
    ASSERT_TYPE(ptr, builder_.getPtrTy());
    ASSERT(ccValues_.size() == (GetGraph()->GetArch() == Arch::AARCH64 ? 8U : 7U));
    ASSERT(ccValues_.at(0) != nullptr);  // pc
    static constexpr unsigned ACC = 1U;
    static constexpr unsigned ACC_TAG = 2U;
    ArenaVector<llvm::Type *> argTypes(GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 0; i < cc_.size(); i++) {
        if (ccValues_.at(i) != nullptr) {
            argTypes.push_back(ccValues_.at(i)->getType());
        } else {
            argTypes.push_back(func_->getFunctionType()->getParamType(i));
        }
    }
    if (ccValues_.at(ACC) == nullptr) {
        ccValues_[ACC] = llvm::Constant::getNullValue(argTypes[ACC]);
    }
    if (ccValues_.at(ACC_TAG) == nullptr) {
        ccValues_[ACC_TAG] = llvm::Constant::getNullValue(argTypes[ACC_TAG]);
    }
    ASSERT(ccValues_.at(3U) != nullptr);  // frame
    ASSERT(ccValues_.at(4U) != nullptr);  // dispatch
    if (GetGraph()->GetArch() == Arch::AARCH64) {
        ASSERT(ccValues_.at(5U) != nullptr);  // moffset
        ASSERT(ccValues_.at(6U) != nullptr);  // method_ptr
        ASSERT(ccValues_.at(7U) != nullptr);  // thread
    } else {
        static constexpr unsigned REAL_FRAME_POINER = 6U;
        ASSERT(ccValues_.at(5U) != nullptr);                 // thread
        ASSERT(ccValues_.at(REAL_FRAME_POINER) == nullptr);  // real frame pointer
        ccValues_[REAL_FRAME_POINER] = func_->getArg(REAL_FRAME_POINER);
    }

    auto functionType = llvm::FunctionType::get(func_->getReturnType(), argTypes, false);
    return builder_.CreateCall(functionType, ptr, ccValues_);
}

// Getters

llvm::FunctionType *LLVMIrConstructor::GetEntryFunctionType()
{
    ArenaVector<llvm::Type *> argTypes(graph_->GetLocalAllocator()->Adapter());

    // ArkInt have fake parameters
    if (graph_->GetMode().IsInterpreter()) {
        for (size_t i = 0; i < cc_.size(); ++i) {
            argTypes.push_back(builder_.getPtrTy());
        }
    }

    // Actual function arguments
    auto method = graph_->GetMethod();
    for (size_t i = 0; i < graph_->GetRuntime()->GetMethodTotalArgumentsCount(method); i++) {
        ASSERT(!graph_->GetMode().IsInterpreter());
        auto type = graph_->GetRuntime()->GetMethodTotalArgumentType(method, i);
        if (graph_->GetMode().IsFastPath()) {
            argTypes.push_back(GetExactType(type));
        } else {
            argTypes.push_back(GetType(type));
        }
    }

    // ThreadReg and RealFP for FastPaths
    if (graph_->GetMode().IsFastPath()) {
        argTypes.push_back(builder_.getPtrTy());
        argTypes.push_back(builder_.getPtrTy());
    }

    auto retType = graph_->GetRuntime()->GetMethodReturnType(method);
    ASSERT(graph_->GetMode().IsInterpreter() || retType != DataType::NO_TYPE);
    retType = retType == DataType::NO_TYPE ? DataType::VOID : retType;
    return llvm::FunctionType::get(GetType(retType), makeArrayRef(argTypes.data(), argTypes.size()), false);
}

llvm::Value *LLVMIrConstructor::GetThreadRegValue()
{
    auto regInput = std::find(cc_.begin(), cc_.end(), GetThreadReg(GetGraph()->GetArch()));
    ASSERT(regInput != cc_.end());
    auto threadRegValue = func_->arg_begin() + std::distance(cc_.begin(), regInput);
    return threadRegValue;
}

llvm::Value *LLVMIrConstructor::GetRealFrameRegValue()
{
    ASSERT(GetGraph()->GetMode().IsFastPath() || GetGraph()->GetArch() == Arch::X86_64);
    auto regInput = std::find(cc_.begin(), cc_.end(), GetRealFrameReg(GetGraph()->GetArch()));
    ASSERT(regInput != cc_.end());
    auto frameRegValue = func_->arg_begin() + std::distance(cc_.begin(), regInput);
    return frameRegValue;
}

llvm::Type *LLVMIrConstructor::GetType(DataType::Type pandaType)
{
    switch (pandaType) {
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
            ASSERT_DO(false, (std::cerr << "No handler for panda type = '" << DataType::ToString(pandaType)
                                        << "' to llvm type conversion."));
            UNREACHABLE();
    }
}

llvm::Type *LLVMIrConstructor::GetExactType(DataType::Type targetType)
{
    switch (targetType) {
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
            ASSERT_DO(false, (std::cerr << "No handler for panda type = '" << DataType::ToString(targetType)
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

llvm::Value *LLVMIrConstructor::CoerceValue(llvm::Value *value, DataType::Type sourceType, DataType::Type targetType)
{
    ASSERT(value != nullptr);
    // Other non-integer mistyping prohibited
    ASSERT_DO(!IsInteger(targetType) || value->getType()->isIntegerTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be an integer."
                        << std::endl);
    ASSERT_DO(!DataType::IsReference(targetType) || value->getType()->isPointerTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a pointer."
                        << std::endl);
    ASSERT_DO(targetType != DataType::FLOAT64 || value->getType()->isDoubleTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a double."
                        << std::endl);
    ASSERT_DO(targetType != DataType::FLOAT32 || value->getType()->isFloatTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a float."
                        << std::endl);

    if (!IsInteger(targetType)) {
        return value;
    }
    ASSERT(value->getType()->isIntegerTy());

    auto targetLlvmType = llvm::cast<llvm::IntegerType>(GetExactType(targetType));
    auto originalLlvmType = llvm::cast<llvm::IntegerType>(value->getType());
    ASSERT(originalLlvmType->getBitWidth() == DataType::GetTypeSize(sourceType, GetGraph()->GetArch()));

    llvm::CastInst::CastOps castOp;
    if (originalLlvmType->getBitWidth() > targetLlvmType->getBitWidth()) {
        castOp = llvm::Instruction::Trunc;
    } else if (originalLlvmType->getBitWidth() < targetLlvmType->getBitWidth()) {
        if (IsSignedInteger(sourceType)) {
            castOp = llvm::Instruction::SExt;
        } else {
            castOp = llvm::Instruction::ZExt;
        }
    } else {
        return value;
    }
    return builder_.CreateCast(castOp, value, targetLlvmType);
}

llvm::Value *LLVMIrConstructor::CoerceValue(llvm::Value *value, llvm::Type *targetType)
{
    auto valueType = value->getType();
    if (valueType == targetType) {
        return value;
    }

    if (!valueType->isPointerTy() && targetType->isPointerTy()) {
        // DataType::POINTER to target_type.
        // Example: i64 -> %"class.panda::Frame"*
        return builder_.CreateIntToPtr(value, targetType);
    }
    if (valueType->isPointerTy() && !targetType->isPointerTy()) {
        // value_type to DataType::POINTER
        // Example: %"class.panda::coretypes::String"* -> i64
        return builder_.CreatePtrToInt(value, targetType);
    }

    if (valueType->isIntegerTy() && targetType->isIntegerTy()) {
        auto valueWidth = llvm::cast<llvm::IntegerType>(valueType)->getBitWidth();
        auto targetWidth = llvm::cast<llvm::IntegerType>(targetType)->getBitWidth();
        if (valueWidth > targetWidth) {
            return builder_.CreateTrunc(value, targetType);
        }
        if (valueWidth < targetWidth) {
            return builder_.CreateZExt(value, targetType);
        }
    }
    if (valueType->isPointerTy() && targetType->isPointerTy()) {
        return builder_.CreateAddrSpaceCast(value, targetType);
    }
    UNREACHABLE();
}

void LLVMIrConstructor::ValueMapAdd(Inst *inst, llvm::Value *value, bool setName)
{
    auto type = inst->GetType();
    auto ltype = GetExactType(type);
    ASSERT(inputMap_.count(inst) == 0);
    auto it = inputMap_.emplace(inst, GetGraph()->GetLocalAllocator()->Adapter());
    ASSERT(it.second);
    ArenaUnorderedMap<DataType::Type, llvm::Value *> &typeMap = it.first->second;

    if (value != nullptr && setName) {
        value->setName(CreateNameForInst(inst));
    }
    if (value == nullptr || inst->GetOpcode() == Opcode::LiveOut || !ltype->isIntegerTy()) {
        typeMap.insert({type, value});
        if (type == DataType::POINTER) {
            ASSERT(value != nullptr);
            /*
             * Ark compiler implicitly converts:
             * 1. POINTER to REFERENCE
             * 2. POINTER to UINT64
             * 3. POINTER to INT64
             * Add value against those types, so we get pointer when requesting it in GetMappedValue.
             */
            typeMap.insert({DataType::REFERENCE, value});
            typeMap.insert({DataType::UINT64, value});
            if (inst->GetOpcode() == Opcode::LiveOut) {
                typeMap.insert({DataType::INT64, value});
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
    typeMap.insert({type, value});
    FillValueMapForUsers(inst, value, type, &typeMap);
}

void LLVMIrConstructor::FillValueMapForUsers(Inst *inst, llvm::Value *value, DataType::Type type,
                                             ArenaUnorderedMap<DataType::Type, llvm::Value *> *typeMap)
{
    auto liveIn = inst->GetOpcode() == Opcode::LiveIn ? value : nullptr;
    for (auto &userItem : inst->GetUsers()) {
        auto user = userItem.GetInst();
        for (unsigned i = 0; i < user->GetInputsCount(); i++) {
            auto itype = user->GetInputType(i);
            auto input = user->GetInput(i).GetInst();
            if (input != inst || itype == type || typeMap->count(itype) != 0) {
                continue;
            }
            llvm::Value *cvalue;
            if (liveIn != nullptr && itype == DataType::REFERENCE) {
                cvalue = liveIn;
            } else if ((type == DataType::INT64 || type == DataType::UINT64) && itype == DataType::REFERENCE) {
                ASSERT(user->GetOpcode() == Opcode::LiveOut);
                cvalue = builder_.CreateIntToPtr(value, builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
            } else {
                cvalue = CoerceValue(value, type, itype);
            }
            typeMap->insert({itype, cvalue});
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

    auto regInput = std::find(ctor->cc_.begin(), ctor->cc_.end(), inst->CastToLiveIn()->GetDstReg());
    ASSERT(regInput != ctor->cc_.end());
    auto idx = std::distance(ctor->cc_.begin(), regInput);
    auto n = ctor->func_->arg_begin() + idx;
    ctor->ValueMapAdd(inst, ctor->CoerceValue(n, ctor->GetExactType(inst->GetType())));
}

void LLVMIrConstructor::VisitParameter(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetBasicBlock()->IsStartBlock());
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    ASSERT(ctor->GetGraph()->GetMode().IsFastPath());
    auto n = ctor->GetArgument(inst->CastToParameter()->GetArgNumber());
    ctor->ValueMapAdd(inst, n, false);
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

    auto regInput = std::find(ctor->cc_.begin(), ctor->cc_.end(), inst->GetDstReg());
    ASSERT(regInput != ctor->cc_.end());
    size_t idx = std::distance(ctor->cc_.begin(), regInput);
    ASSERT(ctor->ccValues_[idx] == nullptr);

    // LiveOut not allowed for real frame register
    ASSERT(ctor->GetGraph()->GetArch() == Arch::AARCH64 || idx + 1 != ctor->cc_.size());
    auto value = ctor->CoerceValue(input, ctor->GetExactType(inst->GetType()));
    ctor->ccValues_[idx] = value;
    ctor->ValueMapAdd(inst, value, false);
}

void LLVMIrConstructor::VisitLoad(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto srcPtr = ctor->GetInputValue(inst, 0);
    ASSERT(srcPtr->getType()->isPointerTy());

    llvm::Value *offset;
    auto offsetInput = inst->GetInput(1).GetInst();
    auto offsetItype = offsetInput->GetType();
    if (offsetItype == DataType::UINT64 || offsetItype == DataType::INT64) {
        ASSERT(offsetInput->GetOpcode() != Opcode::Load && offsetInput->GetOpcode() != Opcode::LoadI);
        offset = ctor->GetInputValue(inst, 1, true);
    } else {
        offset = ctor->GetInputValue(inst, 1);
    }

    ASSERT(srcPtr->getType()->isPointerTy());
    auto ptr = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), srcPtr, offset);

    auto n = ctor->CreateLoadWithOrdering(inst, ptr, ToAtomicOrdering(inst->CastToLoad()->GetVolatile()));
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStore(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto srcPtr = ctor->GetInputValue(inst, 0);
    auto value = ctor->GetInputValue(inst, 2U);

    llvm::Value *offset;
    auto offsetInput = inst->GetInput(1).GetInst();
    auto offsetItype = offsetInput->GetType();
    if (offsetItype == DataType::UINT64 || offsetItype == DataType::INT64) {
        ASSERT(offsetInput->GetOpcode() != Opcode::Load && offsetInput->GetOpcode() != Opcode::LoadI);
        offset = ctor->GetInputValue(inst, 1, true);
    } else {
        offset = ctor->GetInputValue(inst, 1);
    }

    auto ptrPlus = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), srcPtr, offset);

    // Pre
    if (inst->CastToStore()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptrPlus);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, ptrPlus, ToAtomicOrdering(inst->CastToStore()->GetVolatile()));
    // Post
    if (inst->CastToStore()->GetNeedBarrier()) {
        ctor->CreatePostWRB(inst, srcPtr, offset, value);
    }
}

void LLVMIrConstructor::VisitLoadI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto srcPtr = ctor->GetInputValue(inst, 0);
    auto index = inst->CastToLoadI()->GetImm();

    ASSERT(srcPtr->getType()->isPointerTy());
    auto ptrPlus = ctor->builder_.CreateConstInBoundsGEP1_64(ctor->builder_.getInt8Ty(), srcPtr, index);

    auto n = ctor->CreateLoadWithOrdering(inst, ptrPlus, ToAtomicOrdering(inst->CastToLoadI()->GetVolatile()));
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStoreI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto srcPtr = ctor->GetInputValue(inst, 0);
    auto value = ctor->GetInputValue(inst, 1);

    auto index = inst->CastToStoreI()->GetImm();
    ASSERT(srcPtr->getType()->isPointerTy());
    auto ptrPlus = ctor->builder_.CreateConstInBoundsGEP1_64(ctor->builder_.getInt8Ty(), srcPtr, index);

    // Pre
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptrPlus);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, ptrPlus, ToAtomicOrdering(inst->CastToStoreI()->GetVolatile()));
    // Post
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        ctor->CreatePostWRB(inst, srcPtr, ctor->builder_.getInt32(index), value);
    }
}

void LLVMIrConstructor::VisitBitcast(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto type = inst->GetType();
    auto llvmTargetType = ctor->GetExactType(type);
    auto input = ctor->GetInputValue(inst, 0);
    auto itype = inst->GetInputType(0);

    llvm::Value *n;
    if (itype == DataType::POINTER) {
        ASSERT(!llvmTargetType->isPointerTy());
        n = ctor->builder_.CreatePtrToInt(input, llvmTargetType);
    } else {
        if (type == DataType::REFERENCE) {
            n = ctor->builder_.CreateIntToPtr(input, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
        } else if (type == DataType::POINTER) {
            n = ctor->builder_.CreateIntToPtr(input, ctor->builder_.getPtrTy());
        } else {
            n = ctor->builder_.CreateBitCast(input, llvmTargetType);
        }
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitCast(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto x = ctor->GetInputValue(inst, 0);

    auto type = inst->GetInputType(0);
    auto targetType = inst->GetType();
    auto llvmTargetType = ctor->GetExactType(targetType);
    // Do not cast if either Ark or LLVM types are the same
    if (type == targetType || x->getType() == llvmTargetType) {
        ctor->ValueMapAdd(inst, x, false);
        return;
    }

    if (DataType::IsFloatType(type) && IsInteger(targetType)) {
        // float to int, e.g. F64TOI32, F32TOI64, F64TOU32, F32TOU64
        auto n = ctor->CreateCastToInt(inst);
        ctor->ValueMapAdd(inst, n);
        return;
    }
    auto op = ctor->GetCastOp(type, targetType);
    if (targetType == DataType::BOOL) {
        ASSERT(op == llvm::Instruction::Trunc);
        auto u1 = ctor->builder_.CreateIsNotNull(x, CreateNameForInst(inst));
        auto n = ctor->builder_.CreateZExt(u1, ctor->builder_.getInt8Ty());
        ctor->ValueMapAdd(inst, n, false);
        return;
    }
    auto n = ctor->builder_.CreateCast(op, x, llvmTargetType);
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
    auto compareInst = inst->CastToCompare();
    auto operandsType = compareInst->GetOperandsType();

    llvm::Value *x = ctor->GetInputValue(inst, 0);
    llvm::Value *y = ctor->GetInputValue(inst, 1);

    llvm::Value *n = nullptr;
    if (IsInteger(operandsType) || DataType::IsReference(operandsType)) {
        n = ctor->CreateCondition(compareInst->GetCc(), x, y);
    } else {
        n = ctor->builder_.CreateFCmp(FCmpCodeConvert(compareInst->GetCc()), x, y);
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitCmp(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    CmpInst *cmpInst = inst->CastToCmp();
    DataType::Type operandsType = cmpInst->GetOperandsType();

    auto x = ctor->GetInputValue(inst, 0);
    auto y = ctor->GetInputValue(inst, 1);
    llvm::Value *n;
    if (DataType::IsFloatType(operandsType)) {
        n = ctor->CreateFloatComparison(cmpInst, x, y);
    } else if (IsInteger(operandsType)) {
        n = ctor->CreateIntegerComparison(cmpInst, x, y);
    } else {
        ASSERT_DO(false, (std::cerr << "Unsupported comparison for operands of type = "
                                    << DataType::ToString(operandsType) << std::endl));
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitNeg(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto inputType = inst->GetInputType(0);
    auto toNegate = ctor->GetInputValue(inst, 0);
    llvm::Value *n;
    if (inputType == DataType::Type::FLOAT64 || inputType == DataType::Type::FLOAT32) {
        n = ctor->builder_.CreateFNeg(toNegate);
    } else if (IsInteger(inputType)) {
        n = ctor->builder_.CreateNeg(toNegate);
    } else {
        ASSERT_DO(false, (std::cerr << "Negation is not supported for" << DataType::ToString(inputType) << std::endl));
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

    auto notOperator = ctor->builder_.CreateNot(input);
    ctor->ValueMapAdd(inst, notOperator);
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
        llvm::Constant *immCst;
        if (x->getType()->isPointerTy()) {
            if (ifimm->GetImm() == 0) {
                immCst = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(x->getType()));
            } else {
                immCst = llvm::ConstantInt::getSigned(x->getType(), ifimm->GetImm());
                immCst = llvm::ConstantExpr::getPointerCast(immCst, x->getType());
            }
        } else {
            immCst = llvm::ConstantInt::getSigned(x->getType(), ifimm->GetImm());
        }
        cond = ctor->CreateCondition(ifimm->GetCc(), x, immCst);
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
    ArenaVector<llvm::Type *> argTypes(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    ArenaVector<llvm::Value *> args(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 1; i < inst->GetInputs().Size(); ++i) {
        argTypes.push_back(ctor->GetType(inst->GetInput(i).GetInst()->GetType()));
        args.push_back(ctor->GetInputValue(inst, i));
    }
    auto retType = ctor->GetType(inst->GetType());
    auto funcType = llvm::FunctionType::get(retType, argTypes, false);
    auto call = ctor->builder_.CreateCall(funcType, ptr, args);
    if (!retType->isVoidTy()) {
        ctor->ValueMapAdd(inst, call);
    }
}

void LLVMIrConstructor::VisitCall(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    ASSERT(!ctor->GetGraph()->SupportManagedCode());

    // Prepare external call if needed
    auto externalId = inst->CastToCall()->GetCallMethodId();
    auto runtime = ctor->GetGraph()->GetRuntime();
    auto externalName = runtime->GetExternalMethodName(ctor->GetGraph()->GetMethod(), externalId);
    auto function = ctor->func_->getParent()->getFunction(externalName);
    if (function == nullptr) {
        ArenaVector<llvm::Type *> argTypes(ctor->GetGraph()->GetLocalAllocator()->Adapter());
        for (size_t i = 0; i < inst->GetInputs().Size(); ++i) {
            argTypes.push_back(ctor->GetType(inst->GetInputType(i)));
        }
        auto ftype = llvm::FunctionType::get(ctor->GetType(inst->GetType()), argTypes, false);
        function =
            llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, externalName, ctor->func_->getParent());
    }
    // Arguments
    ArenaVector<llvm::Value *> args(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 0; i < inst->GetInputs().Size(); ++i) {
        args.push_back(ctor->CoerceValue(ctor->GetInputValue(inst, i), function->getArg(i)->getType()));
    }
    // Call
    auto call = ctor->builder_.CreateCall(function->getFunctionType(), function, args);

    if (IsNoAliasIrtocFunction(externalName)) {
        ASSERT(call->getType()->isPointerTy());
        call->addRetAttr(llvm::Attribute::NoAlias);
    } else {
        ASSERT(call->getType()->isPointerTy() ^ !IsPtrIgnIrtocFunction(externalName));
    }

    // Check if function has debug info
    if (function->getSubprogram() != nullptr) {
        ctor->debugData_->SetLocation(call, inst->GetPc());
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
    auto nonPhi = block->getFirstNonPHI();
    if (nonPhi != nullptr) {
        ctor->builder_.SetInsertPoint(nonPhi);
    }

    auto phi = ctor->builder_.CreatePHI(ltype, inst->GetInputsCount());
    ctor->SetCurrentBasicBlock(block);
    ctor->ValueMapAdd(inst, phi);
}

void LLVMIrConstructor::VisitIntrinsic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto entryId = inst->CastToIntrinsic()->GetIntrinsicId();

    // Some intrinsics are lowered into some code or LLVM intrinsics. For LLVM intrinsics final decision about
    // lowering into code or call is made later in IntrinsicsLowering
    if (g_options.IsCompilerEncodeIntrinsics()) {
        bool lowered = ctor->TryEmitIntrinsic(inst, entryId);
        if (lowered) {
            return;
        }
        // Every intrinsic that panda can encode should be lowered
        ASSERT(!EncodesBuiltin(ctor->GetGraph()->GetRuntime(), entryId, ctor->GetGraph()->GetArch()));
    }

    UNREACHABLE();
}

void LLVMIrConstructor::VisitDefault([[maybe_unused]] Inst *inst)
{
    ASSERT_DO(false, (std::cerr << "Unsupported llvm lowering for \n", inst->Dump(&std::cerr, true)));
    UNREACHABLE();
}

LLVMIrConstructor::LLVMIrConstructor(Graph *graph, llvm::Module *module, llvm::LLVMContext *context,
                                     LLVMArkInterface *arkInterface, const std::unique_ptr<DebugDataBuilder> &debugData)
    : graph_(graph),
      builder_(llvm::IRBuilder<>(*context)),
      inputMap_(graph->GetLocalAllocator()->Adapter()),
      blockTailMap_(graph->GetLocalAllocator()->Adapter()),
      blockHeadMap_(graph->GetLocalAllocator()->Adapter()),
      arkInterface_(arkInterface),
      debugData_(debugData),
      cc_(graph->GetLocalAllocator()->Adapter()),
      ccValues_(graph->GetLocalAllocator()->Adapter())
{
    // Assign regmaps
    if (graph->GetMode().IsInterpreter()) {
        if (graph->GetArch() == Arch::AARCH64) {
            cc_.assign({AARCH64_PC, AARCH64_ACC, AARCH64_ACC_TAG, AARCH64_FP, AARCH64_DISPATCH, AARCH64_MOFFSET,
                        AARCH64_METHOD_PTR, GetThreadReg(Arch::AARCH64)});
        } else if (graph->GetArch() == Arch::X86_64) {
            cc_.assign({X86_64_PC, X86_64_ACC, X86_64_ACC_TAG, X86_64_FP, X86_64_DISPATCH, GetThreadReg(Arch::X86_64),
                        X86_64_REAL_FP});
        } else {
            LLVM_LOG(FATAL, ENTRY) << "Unsupported architecture for arkintcc";
        }
    } else if (graph->GetMode().IsFastPath()) {
        ASSERT(graph->GetArch() == Arch::AARCH64);
        for (size_t i = 0; i < graph->GetRuntime()->GetMethodTotalArgumentsCount(graph->GetMethod()); i++) {
            cc_.push_back(i);
        }
        cc_.push_back(GetThreadReg(Arch::AARCH64));
        cc_.push_back(AARCH64_REAL_FP);
    }
    ccValues_.assign(cc_.size(), nullptr);

    // Create function
    auto funcProto = GetEntryFunctionType();
    auto methodName = arkInterface_->GetUniqMethodName(graph_->GetMethod());
    func_ = CreateFunctionDeclaration(funcProto, methodName, module);
    arkInterface_->PutFunction(graph_->GetMethod(), func_);

    auto klassId = graph_->GetRuntime()->GetClassIdForMethod(graph_->GetMethod());
    auto klassIdMd = llvm::ConstantAsMetadata::get(builder_.getInt32(klassId));
    func_->addMetadata(llvmbackend::LLVMArkInterface::FUNCTION_MD_CLASS_ID, *llvm::MDNode::get(*context, {klassIdMd}));
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
    arkInterface_->RememberFunctionOrigin(func_, method);
    debugData_->BeginSubprogram(func_, runtime->GetFullFileName(method), runtime->GetMethodId(method));

    // First step - create blocks, leaving LLVM EntryBlock untouched
    BuildBasicBlocks();
    // Second step - visit all instructions, including StartBlock, but not filling PHI inputs
    BuildInstructions();
    // Third step is to fill the PHIs inputs
    FillPhiInputs();

    debugData_->EndSubprogram(func_);

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
    for (auto basicBlock : graph->GetBlocksRPO()) {
        for (auto inst : basicBlock->AllInsts()) {
            bool canCompile = LLVMIrConstructor::CanCompile(inst);
            if (!canCompile) {
                LLVM_LOG(ERROR, ENTRY) << GetOpcodeString(inst->GetOpcode())
                                       << " unexpected in LLVM lowering. Method = "
                                       << graph->GetRuntime()->GetMethodFullName(graph->GetMethod());
                return Unexpected(std::string("Unexpected opcode in CanCompile"));
            }
            if (graph->GetMode().IsInterpreter()) {
                continue;
            }

            auto opcode = inst->GetOpcode();
            if (opcode == Opcode::SaveState || opcode == Opcode::SaveStateDeoptimize) {
                can = false;  // no immediate return here - to check CanCompile for other instructions
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
