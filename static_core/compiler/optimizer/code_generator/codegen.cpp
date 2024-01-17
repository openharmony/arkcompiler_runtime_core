/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

/*
Codegen Hi-Level implementation
*/
#include "operands.h"
#include "codegen.h"
#include "compiler_options.h"
#include "lib_call_inst.h"
#include "relocations.h"
#include "include/compiler_interface.h"
#include "ir-dyn-base-types.h"
#include "runtime/include/coretypes/string.h"
#include "compiler/optimizer/ir/analysis.h"
#include "compiler/optimizer/ir/locations.h"
#include "compiler/optimizer/analysis/liveness_analyzer.h"
#include "optimizer/code_generator/method_properties.h"
#include "events/events.h"
#include "libpandabase/utils/tsan_interface.h"
#include "libpandabase/utils/utils.h"
#include <iomanip>

namespace panda::compiler {

class OsrEntryStub {
    void FixIntervals(Codegen *codegen, Encoder *encoder)
    {
        auto &la = codegen->GetGraph()->GetAnalysis<LivenessAnalyzer>();
        la.EnumerateLiveIntervalsForInst(saveState_, [this, codegen, encoder](const auto &li) {
            auto inst = li->GetInst();
            auto location = li->GetLocation();
            // Skip live registers that are already in the input list of the OsrSaveState
            const auto &ssInputs = saveState_->GetInputs();
            if (std::find_if(ssInputs.begin(), ssInputs.end(),
                             [inst](auto &input) { return input.GetInst() == inst; }) != ssInputs.end()) {
                return;
            }
            // Only constants allowed
            switch (inst->GetOpcode()) {
                case Opcode::LoadAndInitClass: {
                    auto klass = reinterpret_cast<uintptr_t>(inst->CastToLoadAndInitClass()->GetClass());
                    encoder->EncodeMov(codegen->ConvertRegister(inst->GetDstReg(), inst->GetType()),
                                       Imm(reinterpret_cast<uintptr_t>(klass)));
                    break;
                }
                case Opcode::Constant: {
                    if (location.IsFixedRegister()) {
                        EncodeConstantMove(li, encoder);
                    } else if (location.IsStack()) {
                        auto slot = location.GetValue();
                        encoder->EncodeSti(
                            bit_cast<int64_t>(inst->CastToConstant()->GetRawValue()), DOUBLE_WORD_SIZE_BYTES,
                            MemRef(codegen->SpReg(), codegen->GetFrameLayout().GetSpillOffsetFromSpInBytes(slot)));
                    } else {
                        ASSERT(location.IsConstant());
                    }
                    break;
                }
                // NOTE (ekudriashov): UnresolvedLoadAndInitClass
                default:
                    break;
            }
        });
    }

    static void EncodeConstantMove(const LifeIntervals *li, Encoder *encoder)
    {
        auto inst = li->GetInst();
        switch (li->GetType()) {
            case DataType::FLOAT64:
                encoder->EncodeMov(Reg(li->GetReg(), FLOAT64_TYPE),
                                   Imm(bit_cast<double>(inst->CastToConstant()->GetDoubleValue())));
                break;
            case DataType::FLOAT32:
                encoder->EncodeMov(Reg(li->GetReg(), FLOAT32_TYPE),
                                   Imm(bit_cast<float>(inst->CastToConstant()->GetFloatValue())));
                break;
            case DataType::UINT32:
                encoder->EncodeMov(Reg(li->GetReg(), INT32_TYPE), Imm(inst->CastToConstant()->GetRawValue()));
                break;
            case DataType::UINT64:
                encoder->EncodeMov(Reg(li->GetReg(), INT64_TYPE), Imm(inst->CastToConstant()->GetRawValue()));
                break;
            default:
                UNREACHABLE();
        }
    }

public:
    OsrEntryStub(Codegen *codegen, SaveStateInst *inst) : label_(codegen->GetEncoder()->CreateLabel()), saveState_(inst)
    {
    }

    DEFAULT_MOVE_SEMANTIC(OsrEntryStub);
    DEFAULT_COPY_SEMANTIC(OsrEntryStub);
    ~OsrEntryStub() = default;

    void Generate(Codegen *codegen)
    {
        auto encoder = codegen->GetEncoder();
        auto lr = codegen->GetTarget().GetLinkReg();
        auto fl = codegen->GetFrameLayout();
        codegen->CreateStackMap(saveState_->CastToSaveStateOsr());
        ssize_t slot = CFrameLayout::LOCALS_START_SLOT + CFrameLayout::GetLocalsCount() - 1U;
        encoder->EncodeStp(
            codegen->FpReg(), lr,
            MemRef(codegen->FpReg(),
                   -fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::BYTES>(slot)));

        FixIntervals(codegen, encoder);
        encoder->EncodeJump(label_);
    }

    SaveStateInst *GetInst()
    {
        return saveState_;
    }

    auto &GetLabel()
    {
        return label_;
    }

private:
    LabelHolder::LabelId label_;
    SaveStateInst *saveState_ {nullptr};
};

Codegen::Codegen(Graph *graph)
    : Optimization(graph),
      allocator_(graph->GetAllocator()),
      localAllocator_(graph->GetLocalAllocator()),
      codeBuilder_(allocator_->New<CodeInfoBuilder>(graph->GetArch(), allocator_)),
      slowPaths_(graph->GetLocalAllocator()->Adapter()),
      slowPathsMap_(graph->GetLocalAllocator()->Adapter()),
      frame_layout_(CFrameLayout(graph->GetArch(), graph->GetStackSlotsCount())),
      osrEntries_(graph->GetLocalAllocator()->Adapter()),
      vregIndices_(GetAllocator()->Adapter()),
      runtime_(graph->GetRuntime()),
      target_(graph->GetArch()),
      liveOuts_(graph->GetLocalAllocator()->Adapter()),
      disasm_(this),
      spillFillsResolver_(graph)
{
    graph->SetCodeBuilder(codeBuilder_);
    regfile_ = graph->GetRegisters();
    if (regfile_ != nullptr) {
        ASSERT(regfile_->IsValid());
        ArenaVector<Reg> regsUsage(allocator_->Adapter());
        Convert(&regsUsage, graph->GetUsedRegs<DataType::INT64>(), INT64_TYPE);
        Convert(&regsUsage, graph->GetUsedRegs<DataType::FLOAT64>(), FLOAT64_TYPE);
        regfile_->SetUsedRegs(regsUsage);
#ifndef NDEBUG
        COMPILER_LOG(DEBUG, CODEGEN) << "Regalloc used registers scalar " << graph->GetUsedRegs<DataType::INT64>();
        COMPILER_LOG(DEBUG, CODEGEN) << "Regalloc used registers vector " << graph->GetUsedRegs<DataType::FLOAT64>();
#endif
    }

    enc_ = graph->GetEncoder();
    ASSERT(enc_ != nullptr && enc_->IsValid());
    enc_->SetRegfile(regfile_);
    if (enc_->InitMasm()) {
        enc_->SetFrameLayout(GetFrameLayout());
    }

    callconv_ = graph->GetCallingConvention();
    if (callconv_ != nullptr) {
        ASSERT(callconv_->IsValid());
        if (callconv_->GetEncoder() == nullptr) {
            callconv_->SetEncoder(enc_);
        }
    }

    auto method = graph->GetMethod();
    // workaround for test
    if (method != nullptr) {
        methodId_ = graph->GetRuntime()->GetMethodId(method);
    }
    GetDisasm()->Init();
    GetDisasm()->SetEncoder(GetEncoder());
}

const char *Codegen::GetPassName() const
{
    return "Codegen";
}

bool Codegen::AbortIfFailed() const
{
    return true;
}

void Codegen::CreateFrameInfo()
{
    // Create FrameInfo for CFrame
    auto &fl = GetFrameLayout();
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(true) |
        FrameInfo::CallersRelativeFp::Encode(false) | FrameInfo::CalleesRelativeFp::Encode(true));
    frame->SetFrameSize(fl.GetFrameSize<CFrameLayout::OffsetUnit::BYTES>());
    frame->SetSpillsCount(fl.GetSpillsCount());

    frame->SetCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
    frame->SetFpCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
    frame->SetCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
    frame->SetFpCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

    frame->SetSetupFrame(true);
    frame->SetSaveFrameAndLinkRegs(true);
    frame->SetSaveUnusedCalleeRegs(!GetGraph()->GetMethodProperties().GetCompactPrologueAllowed());
    frame->SetAdjustSpReg(true);
    frame->SetHasFloatRegs(GetGraph()->HasFloatRegs());

    GetCodeBuilder()->SetHasFloatRegs(GetGraph()->HasFloatRegs());

    SetFrameInfo(frame);
}

void Codegen::FillOnlyParameters(RegMask *liveRegs, uint32_t numParams, bool isFastpath) const
{
    ASSERT(numParams <= 6U);
    if (GetArch() == Arch::AARCH64 && !isFastpath) {
        numParams = AlignUp(numParams, 2U);
    }
    *liveRegs &= GetTarget().GetParamRegsMask(numParams);
}

void Codegen::Convert(ArenaVector<Reg> *regsUsage, const ArenaVector<bool> *mask, TypeInfo typeInfo)
{
    ASSERT(regsUsage != nullptr);
    // There are no used registers
    if (mask == nullptr) {
        return;
    }
    ASSERT(mask->size() == MAX_NUM_REGS);
    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if ((*mask)[i]) {
            regsUsage->emplace_back(i, typeInfo);
        }
    }
}

#ifdef INTRINSIC_SLOW_PATH_ENTRY_ENABLED
void Codegen::CreateIrtocIntrinsic(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS src)
{
    switch (inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY:
            IntrinsicSlowPathEntry(inst);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_UNREACHABLE:
            GetEncoder()->EncodeAbort();
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_SAVE_REGISTERS_EP:
            IntrinsicSaveRegisters(inst);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_RESTORE_REGISTERS_EP:
            IntrinsicRestoreRegisters(inst);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_TAIL_CALL:
            IntrinsicTailCall(inst);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_INTERPRETER_RETURN:
            GetCallingConvention()->GenerateNativeEpilogue(*GetFrameInfo(), []() {});
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_LOAD_ACQUIRE_MARK_WORD_EXCLUSIVE:
            ASSERT(GetRuntime()->GetObjMarkWordOffset(GetArch()) == 0);
            GetEncoder()->EncodeLdrExclusive(dst, src[0], true);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STORE_RELEASE_MARK_WORD_EXCLUSIVE:
            ASSERT(GetRuntime()->GetObjMarkWordOffset(GetArch()) == 0);
            GetEncoder()->EncodeStrExclusive(dst, src[SECOND_OPERAND], src[0], true);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPARE_AND_SET_MARK_WORD:
            ASSERT(GetRuntime()->GetObjMarkWordOffset(GetArch()) == 0);
            GetEncoder()->EncodeCompareAndSwap(dst, src[0], src[SECOND_OPERAND], src[THIRD_OPERAND]);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_DATA_MEMORY_BARRIER_FULL:
            GetEncoder()->EncodeMemoryBarrier(memory_order::FULL);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPRESS_EIGHT_UTF16_TO_UTF8_CHARS_USING_SIMD:
            GetEncoder()->EncodeCompressEightUtf16ToUtf8CharsUsingSimd(src[FIRST_OPERAND], src[SECOND_OPERAND]);
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPRESS_SIXTEEN_UTF16_TO_UTF8_CHARS_USING_SIMD:
            GetEncoder()->EncodeCompressSixteenUtf16ToUtf8CharsUsingSimd(src[FIRST_OPERAND], src[SECOND_OPERAND]);
            break;
        default:
            UNREACHABLE();
            break;
    }
}
#endif

bool Codegen::BeginMethod()
{
    // Do not try to encode too large graph
    auto instSize = GetGraph()->GetCurrentInstructionId();
    auto instsPerByte = GetEncoder()->MaxArchInstPerEncoded();
    auto maxBitsInInst = GetInstructionSizeBits(GetArch());
    instSize += slowPaths_.size() * INST_IN_SLOW_PATH;
    if ((instSize * instsPerByte * maxBitsInInst) > g_options.GetCompilerMaxGenCodeSize()) {
        return false;
    }
    // After this - encoder aborted, if allocated too much size.
    GetEncoder()->SetMaxAllocatedBytes(g_options.GetCompilerMaxGenCodeSize());

    if (GetGraph()->IsAotMode()) {
        GetEncoder()->SetCodeOffset(GetGraph()->GetAotData()->GetCodeOffset() + CodeInfo::GetCodeOffset(GetArch()));
    } else {
        GetEncoder()->SetCodeOffset(0);
    }

    codeBuilder_->BeginMethod(GetFrameLayout().GetFrameSize<CFrameLayout::OffsetUnit::BYTES>(),
                              GetGraph()->GetVRegsCount() + GetGraph()->GetEnvCount());

    GetEncoder()->BindLabel(GetLabelEntry());
    SetStartCodeOffset(GetEncoder()->GetCursorOffset());

    GeneratePrologue();

    return GetEncoder()->GetResult();
}

void Codegen::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "Method Prologue");

    GetCallingConvention()->GeneratePrologue(*frameInfo_);

    if (!GetGraph()->GetMode().IsNative()) {
        GetEncoder()->EncodeSti(1, 1, MemRef(ThreadReg(), GetRuntime()->GetTlsFrameKindOffset(GetArch())));
    }
    if (!GetGraph()->GetMode().IsNative()) {
        // Create stack overflow check
        GetEncoder()->EncodeStackOverflowCheck(-GetRuntime()->GetStackOverflowCheckOffset());
        // Create empty stackmap for the stack overflow check
        GetCodeBuilder()->BeginStackMap(0, 0, nullptr, 0, false, false);
        GetCodeBuilder()->EndStackMap();
    }

#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
    if (GetGraph()->IsAotMode()) {
        SCOPED_DISASM_STR(this, "LoadMethod for trace");
        ScopedTmpReg method_reg(GetEncoder());
        LoadMethod(method_reg);
        InsertTrace(Imm(static_cast<size_t>(TraceId::METHOD_ENTER)), method_reg,
                    Imm(static_cast<size_t>(events::MethodEnterKind::COMPILED)));
    } else {
        InsertTrace(Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                    Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())),
                    Imm(static_cast<size_t>(events::MethodEnterKind::COMPILED)));
    }
#endif
}

void Codegen::GenerateEpilogue()
{
    SCOPED_DISASM_STR(this, "Method Epilogue");

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
    GetCallingConvention()->GenerateEpilogue(*frame_info_, [this]() {
        if (GetGraph()->IsAotMode()) {
            ScopedTmpReg method_reg(GetEncoder());
            LoadMethod(method_reg);
            InsertTrace(Imm(static_cast<size_t>(TraceId::METHOD_EXIT)), method_reg,
                        Imm(static_cast<size_t>(events::MethodExitKind::COMPILED)));
        } else {
            InsertTrace(Imm(static_cast<size_t>(TraceId::METHOD_EXIT)),
                        Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())),
                        Imm(static_cast<size_t>(events::MethodExitKind::COMPILED)));
        }
    });
#else
    GetCallingConvention()->GenerateEpilogue(*frameInfo_, []() {});
#endif
}

bool Codegen::VisitGraph()
{
    EncodeVisitor visitor(this);
    visitor_ = &visitor;

    const auto &blocks = GetGraph()->GetBlocksLinearOrder();

    for (auto bb : blocks) {
        GetEncoder()->BindLabel(bb->GetId());
        for (auto inst : bb->AllInsts()) {
            SCOPED_DISASM_INST(this, inst);

#ifdef PANDA_COMPILER_DEBUG_INFO
            auto debugInfo = inst->GetDebugInfo();
            if (GetGraph()->IsLineDebugInfoEnabled() && debugInfo != nullptr) {
                debugInfo->SetOffset(GetEncoder()->GetCursorOffset());
            }
#endif
            visitor.VisitInstruction(inst);
            if (!visitor.GetResult()) {
                COMPILER_LOG(DEBUG, CODEGEN)
                    << "Can't encode instruction: " << GetOpcodeString(inst->GetOpcode()) << *inst;
                break;
            }
        }

        if (bb->NeedsJump()) {
            EmitJump(bb);
        }

        if (!visitor.GetResult()) {
            return false;
        }
    }

    auto instsPerByte = GetEncoder()->MaxArchInstPerEncoded();
    auto maxBitsInInst = GetInstructionSizeBits(GetArch());
    auto instSize = GetGraph()->GetCurrentInstructionId() + slowPaths_.size() * INST_IN_SLOW_PATH;
    if ((instSize * instsPerByte * maxBitsInInst) > g_options.GetCompilerMaxGenCodeSize()) {
        return false;
    }

    EmitSlowPaths();
    visitor_ = nullptr;

    return true;
}

void Codegen::EmitJump(const BasicBlock *bb)
{
    BasicBlock *sucBb = nullptr;

    if (bb->GetLastInst() == nullptr) {
        ASSERT(bb->IsEmpty());
        sucBb = bb->GetSuccsBlocks()[0];
    } else {
        switch (bb->GetLastInst()->GetOpcode()) {
            case Opcode::If:
            case Opcode::IfImm:
                ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
                sucBb = bb->GetFalseSuccessor();
                break;
            default:
                sucBb = bb->GetSuccsBlocks()[0];
                break;
        }
    }
    SCOPED_DISASM_STR(this, std::string("Jump from  BB_") + std::to_string(bb->GetId()) + " to BB_" +
                                std::to_string(sucBb->GetId()));

    auto label = sucBb->GetId();
    GetEncoder()->EncodeJump(label);
}

void Codegen::EndMethod()
{
    for (auto &osrStub : osrEntries_) {
        SCOPED_DISASM_STR(this,
                          std::string("Osr stub for OsrStateStump ") + std::to_string(osrStub->GetInst()->GetId()));
        osrStub->Generate(this);
    }

    GetEncoder()->Finalize();
}

// Allocates memory, copies generated code to it, sets the code to the graph's codegen data. Also this function
// encodes the code info and sets it to the graph.
bool Codegen::CopyToCodeCache()
{
    auto codeEntry = reinterpret_cast<void *>(GetEncoder()->GetLabelAddress(GetLabelEntry()));
    auto codeSize = GetEncoder()->GetCursorOffset();
    bool saveAllCalleeRegisters = !GetGraph()->GetMethodProperties().GetCompactPrologueAllowed();

    auto code = reinterpret_cast<uint8_t *>(GetAllocator()->Alloc(codeSize));
    if (code == nullptr) {
        return false;
    }
    memcpy_s(code, codeSize, codeEntry, codeSize);
    GetGraph()->SetCode(EncodeDataType(code, codeSize));

    RegMask calleeRegs;
    VRegMask calleeVregs;
    GetRegfile()->FillUsedCalleeSavedRegisters(&calleeRegs, &calleeVregs, saveAllCalleeRegisters);
    constexpr size_t MAX_NUM_REGISTERS = 32;
    static_assert(MAX_NUM_REGS <= MAX_NUM_REGISTERS && MAX_NUM_VREGS <= MAX_NUM_REGISTERS);
    codeBuilder_->SetSavedCalleeRegsMask(static_cast<uint32_t>(calleeRegs.to_ulong()),
                                         static_cast<uint32_t>(calleeVregs.to_ulong()));

    ArenaVector<uint8_t> codeInfoData(GetGraph()->GetAllocator()->Adapter());
    codeBuilder_->Encode(&codeInfoData);
    GetGraph()->SetCodeInfo(Span(codeInfoData));

    return true;
}

void Codegen::IssueDisasm()
{
    if (GetGraph()->GetMode().SupportManagedCode() && g_options.IsCompilerDisasmDumpCodeInfo()) {
        GetDisasm()->PrintCodeInfo(this);
    }
    GetDisasm()->PrintCodeStatistics(this);
    auto &stream = GetDisasm()->GetStream();
    for (auto &item : GetDisasm()->GetItems()) {
        if (std::holds_alternative<CodeItem>(item)) {
            auto &code = std::get<CodeItem>(item);
            for (size_t pc = code.GetPosition(); pc < code.GetCursorOffset();) {
                stream << GetDisasm()->GetIndent(code.GetDepth());
                auto newPc = GetEncoder()->DisasmInstr(stream, pc, 0);
                stream << "\n";
                pc = newPc;
            }
        } else if (std::holds_alternative<std::string>(item)) {
            stream << (std::get<std::string>(item));
        }
    }
}

bool Codegen::RunImpl()
{
    Initialize();

    auto encoder = GetEncoder();
    encoder->GetLabels()->CreateLabels(GetGraph()->GetVectorBlocks().size());
    labelEntry_ = encoder->CreateLabel();
    labelExit_ = encoder->CreateLabel();

#ifndef NDEBUG
    if (g_options.IsCompilerNonOptimizing()) {
        // In case of non-optimizing compiler lowering pass is not run but low-level instructions may
        // also appear on codegen so, to satisfy GraphChecker, the flag should be raised.
        GetGraph()->SetLowLevelInstructionsEnabled();
    }
#endif  // NDEBUG

    if ((GetCallingConvention() == nullptr) || (GetEncoder() == nullptr)) {
        return false;
    }

    if (GetCallingConvention()->IsDynCallMode()) {
        auto numExpectedArgs = GetRuntime()->GetMethodTotalArgumentsCount(GetGraph()->GetMethod());
        if (numExpectedArgs > GetRuntime()->GetDynamicNumFixedArgs()) {
            CallConvDynInfo dynInfo(numExpectedArgs,
                                    GetRuntime()->GetEntrypointTlsOffset(
                                        GetArch(), RuntimeInterface::EntrypointId::EXPAND_COMPILED_CODE_ARGS_DYN));
            GetCallingConvention()->SetDynInfo(dynInfo);
            frameInfo_->SetSaveFrameAndLinkRegs(true);
        } else {
            GetCallingConvention()->SetDynInfo(CallConvDynInfo());
        }
    }

    if (!GetEncoder()->GetResult()) {
        return false;
    }

    // Remove registers from the temp registers, if they are in the regalloc mask, i.e. available for regalloc.
    auto usedRegs = ~GetGraph()->GetArchUsedRegs();
    auto forbiddenTemps = usedRegs & GetTarget().GetTempRegsMask();
    if (forbiddenTemps.Any()) {
        for (size_t i = forbiddenTemps.GetMinRegister(); i <= forbiddenTemps.GetMaxRegister(); i++) {
            if (forbiddenTemps[i] && encoder->IsScratchRegisterReleased(Reg(i, INT64_TYPE))) {
                encoder->AcquireScratchRegister(Reg(i, INT64_TYPE));
            }
        }
    }

    return Finalize();
}

void Codegen::Initialize()
{
    CreateFrameInfo();

    GetRegfile()->SetCalleeSaved(GetRegfile()->GetCalleeSaved());

    if (!GetGraph()->SupportManagedCode()) {
        for (auto inst : GetGraph()->GetStartBlock()->AllInsts()) {
            if (inst->GetOpcode() == Opcode::LiveIn && GetTarget().GetTempRegsMask().Test(inst->GetDstReg())) {
                GetEncoder()->AcquireScratchRegister(Reg(inst->GetDstReg(), INT64_TYPE));
            }
        }
    }

    bool hasCalls = false;

    for (auto bb : GetGraph()->GetBlocksLinearOrder()) {
        // Calls may be in the middle of method
        for (auto inst : bb->Insts()) {
            // For throw instruction need jump2runtime same way
            if (inst->IsCall() || inst->GetOpcode() == Opcode::Throw) {
                hasCalls = true;
                break;
            }
        }
        if (hasCalls) {
            break;
        }
    }

    /* Convert Graph::GetUsedRegs(), which is std::vector<bool>, to simple
     * RegMask and save it in the Codegen. These masks are used to determine
     * which registers we need to save in prologue.
     *
     * NB! It's related to IRTOC specific prologue generation (see CodegenFastPath etc.).
     * Arch specific CallingConvention::GenerateProlog() relies on reg usage information
     * prepared in the Codegen constructor (before Initialize() is called).
     */
    auto fillMask = [](RegMask *mask, auto *vector) {
        if (vector == nullptr) {
            return;
        }
        ASSERT(mask->size() >= vector->size());
        mask->reset();
        for (size_t i = 0; i < mask->size(); i++) {
            if ((*vector)[i]) {
                mask->set(i);
            }
        }
    };
    fillMask(&usedRegs_, GetGraph()->GetUsedRegs<DataType::INT64>());
    fillMask(&usedVregs_, GetGraph()->GetUsedRegs<DataType::FLOAT64>());
    usedVregs_ &= GetTarget().GetAvailableVRegsMask();
    usedRegs_ &= GetTarget().GetAvailableRegsMask();
    usedRegs_ &= ~GetGraph()->GetArchUsedRegs();
    usedVregs_ &= ~GetGraph()->GetArchUsedVRegs();

    /* Remove used registers from Encoder's scratch registers */
    RegMask usedTemps = usedRegs_ & GetTarget().GetTempRegsMask();
    if (usedTemps.any()) {
        for (size_t i = 0; i < usedTemps.size(); i++) {
            if (usedTemps[i]) {
                GetEncoder()->AcquireScratchRegister(Reg(i, INT64_TYPE));
            }
        }
    }
}

bool Codegen::Finalize()
{
    if (GetDisasm()->IsEnabled()) {
        GetDisasm()->PrintMethodEntry(this);
    }

    if (!BeginMethod()) {
        return false;
    }

    if (!VisitGraph()) {
        return false;
    }
    EndMethod();

    if (!CopyToCodeCache()) {
        return false;
    }

    if (GetDisasm()->IsEnabled()) {
        IssueDisasm();
    }

    return true;
}

Reg Codegen::ConvertRegister(Register r, DataType::Type type)
{
    switch (type) {
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8: {
            return Reg(r, INT8_TYPE);
        }
        case DataType::UINT16:
        case DataType::INT16: {
            return Reg(r, INT16_TYPE);
        }
        case DataType::UINT32:
        case DataType::INT32: {
            return Reg(r, INT32_TYPE);
        }
        case DataType::UINT64:
        case DataType::INT64:
        case DataType::ANY: {
            return Reg(r, INT64_TYPE);
        }
        case DataType::FLOAT32: {
            return Reg(r, FLOAT32_TYPE);
        }
        case DataType::FLOAT64: {
            return Reg(r, FLOAT64_TYPE);
        }
        case DataType::REFERENCE: {
            return ConvertRegister(r, DataType::GetIntTypeForReference(GetArch()));
        }
        case DataType::POINTER: {
            return Reg(r, ConvertDataType(DataType::POINTER, GetArch()));
        }
        default:
            // Invalid converted register
            return INVALID_REGISTER;
    }
}

// Panda don't support types less then 32, so we need sign or zero extended to 32
Imm Codegen::ConvertImmWithExtend(uint64_t imm, DataType::Type type)
{
    switch (type) {
        case DataType::BOOL:
        case DataType::UINT8:
            return Imm(static_cast<uint32_t>(static_cast<uint8_t>(imm)));
        case DataType::INT8:
            return Imm(static_cast<int32_t>(bit_cast<int8_t, uint8_t>(imm)));
        case DataType::UINT16:
            return Imm(static_cast<uint32_t>(static_cast<uint16_t>(imm)));
        case DataType::INT16:
            return Imm(static_cast<int32_t>(bit_cast<int16_t, uint16_t>(imm)));
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::UINT32:
            return Imm(bit_cast<int32_t, uint32_t>(imm));
        case DataType::INT32:
            return Imm(bit_cast<int32_t, uint32_t>(imm));
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::UINT64:
            return Imm(bit_cast<int64_t, uint64_t>(imm));
        case DataType::INT64:
            return Imm(bit_cast<int64_t, uint64_t>(imm));
        case DataType::FLOAT32:
            return Imm(bit_cast<float, uint32_t>(static_cast<uint32_t>(imm)));
        case DataType::FLOAT64:
            return Imm(bit_cast<double, uint64_t>(imm));
        case DataType::ANY:
            return Imm(bit_cast<uint64_t, uint64_t>(imm));
        case DataType::REFERENCE:
            if (imm == 0) {
                return Imm(0);
            }
            [[fallthrough]]; /* fall-through */
        default:
            // Invalid converted immediate
            UNREACHABLE();
    }
    return Imm(0);
}

Condition Codegen::ConvertCc(ConditionCode cc)
{
    switch (cc) {
        case CC_EQ:
            return Condition::EQ;
        case CC_NE:
            return Condition::NE;
        case CC_LT:
            return Condition::LT;
        case CC_LE:
            return Condition::LE;
        case CC_GT:
            return Condition::GT;
        case CC_GE:
            return Condition::GE;
        case CC_B:
            return Condition::LO;
        case CC_BE:
            return Condition::LS;
        case CC_A:
            return Condition::HI;
        case CC_AE:
            return Condition::HS;
        case CC_TST_EQ:
            return Condition::TST_EQ;
        case CC_TST_NE:
            return Condition::TST_NE;
        default:
            UNREACHABLE();
            return Condition::EQ;
    }
    return Condition::EQ;
}

Condition Codegen::ConvertCcOverflow(ConditionCode cc)
{
    switch (cc) {
        case CC_EQ:
            return Condition::VS;
        case CC_NE:
            return Condition::VC;
        default:
            UNREACHABLE();
            return Condition::VS;
    }
    return Condition::VS;
}

void Codegen::EmitSlowPaths()
{
    for (auto slowPath : slowPaths_) {
        slowPath->Generate(this);
    }
}

void Codegen::CreateStackMap(Inst *inst, Inst *user)
{
    SaveStateInst *saveState = nullptr;
    if (inst->IsSaveState()) {
        saveState = static_cast<SaveStateInst *>(inst);
    } else {
        saveState = inst->GetSaveState();
    }
    ASSERT(saveState != nullptr);

    bool requireVregMap = inst->RequireRegMap();
    uint32_t outerBpc = inst->GetPc();
    for (auto callInst = saveState->GetCallerInst(); callInst != nullptr;
         callInst = callInst->GetSaveState()->GetCallerInst()) {
        outerBpc = callInst->GetPc();
    }
    codeBuilder_->BeginStackMap(outerBpc, GetEncoder()->GetCursorOffset(), saveState->GetRootsStackMask(),
                                saveState->GetRootsRegsMask().to_ulong(), requireVregMap,
                                saveState->GetOpcode() == Opcode::SaveStateOsr);
    if (user == nullptr) {
        user = inst;
        if (inst == saveState && inst->HasUsers()) {
            auto users = inst->GetUsers();
            auto it = std::find_if(users.begin(), users.end(),
                                   [](auto &u) { return u.GetInst()->GetOpcode() != Opcode::ReturnInlined; });
            user = it->GetInst();
        }
    }
    CreateStackMapRec(saveState, requireVregMap, user);

    codeBuilder_->EndStackMap();
    if (GetDisasm()->IsEnabled()) {
        GetDisasm()->PrintStackMap(this);
    }
}

void Codegen::CreateStackMapRec(SaveStateInst *saveState, bool requireVregMap, Inst *targetSite)
{
    bool hasInlineInfo = saveState->GetCallerInst() != nullptr;
    size_t vregsCount = 0;
    if (requireVregMap) {
        auto runtime = GetRuntime();
        if (auto caller = saveState->GetCallerInst()) {
            vregsCount = runtime->GetMethodRegistersCount(caller->GetCallMethod()) +
                         runtime->GetMethodArgumentsCount(caller->GetCallMethod());
        } else {
            vregsCount = runtime->GetMethodRegistersCount(saveState->GetMethod()) +
                         runtime->GetMethodArgumentsCount(saveState->GetMethod());
        }
        ASSERT(!GetGraph()->IsBytecodeOptimizer());
        // 1 for acc, number of env regs for dynamic method
        vregsCount += 1U + GetGraph()->GetEnvCount();
#ifndef NDEBUG
        ASSERT_PRINT(!saveState->GetInputsWereDeleted(), "Some vregs were deleted from the save state");
#endif
    }

    if (auto callInst = saveState->GetCallerInst()) {
        CreateStackMapRec(callInst->GetSaveState(), requireVregMap, targetSite);
        auto method = GetGraph()->IsAotMode() ? nullptr : callInst->GetCallMethod();
        codeBuilder_->BeginInlineInfo(method, GetRuntime()->GetMethodId(callInst->GetCallMethod()), saveState->GetPc(),
                                      vregsCount);
    }

    if (requireVregMap) {
        CreateVRegMap(saveState, vregsCount, targetSite);
    }

    if (hasInlineInfo) {
        codeBuilder_->EndInlineInfo();
    }
}

void Codegen::CreateVRegMap(SaveStateInst *saveState, size_t vregsCount, Inst *targetSite)
{
    vregIndices_.clear();
    vregIndices_.resize(vregsCount, {-1, -1});
    FillVregIndices(saveState);

    ASSERT(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    auto targetLifeNumber = la.GetInstLifeIntervals(targetSite)->GetBegin();

    for (auto &inputIndex : vregIndices_) {
        if (inputIndex.first == -1 && inputIndex.second == -1) {
            codeBuilder_->AddVReg(VRegInfo());
            continue;
        }
        if (inputIndex.second != -1) {
            auto imm = saveState->GetImmediate(inputIndex.second);
            codeBuilder_->AddConstant(imm.value, IrTypeToMetainfoType(imm.type), imm.vregType);
            continue;
        }
        ASSERT(inputIndex.first != -1);
        auto vreg = saveState->GetVirtualRegister(inputIndex.first);
        auto inputInst = saveState->GetDataFlowInput(inputIndex.first);
        auto interval = la.GetInstLifeIntervals(inputInst)->FindSiblingAt(targetLifeNumber);
        ASSERT(interval != nullptr);
        CreateVreg(interval->GetLocation(), inputInst, vreg);
    }
}

void Codegen::CreateVreg(const Location &location, Inst *inst, const VirtualRegister &vreg)
{
    switch (location.GetKind()) {
        case LocationType::FP_REGISTER:
        case LocationType::REGISTER: {
            CreateVRegForRegister(location, inst, vreg);
            break;
        }
        case LocationType::STACK_PARAMETER: {
            auto slot = location.GetValue();
            codeBuilder_->AddVReg(VRegInfo(GetFrameLayout().GetStackArgsStartSlot() - slot - CFrameSlots::Start(),
                                           VRegInfo::Location::SLOT, IrTypeToMetainfoType(inst->GetType()),
                                           vreg.GetVRegType()));
            break;
        }
        case LocationType::STACK: {
            auto slot = location.GetValue();
            if (!Is64BitsArch(GetArch())) {
                slot = ((location.GetValue() << 1U) + 1);
            }
            codeBuilder_->AddVReg(VRegInfo(GetFrameLayout().GetFirstSpillSlot() + slot, VRegInfo::Location::SLOT,
                                           IrTypeToMetainfoType(inst->GetType()), vreg.GetVRegType()));
            break;
        }
        case LocationType::IMMEDIATE: {
            ASSERT(inst->IsConst());
            auto type = inst->GetType();
            // There are no int64 type for dynamic methods.
            if (GetGraph()->IsDynamicMethod() && type == DataType::INT64) {
                type = DataType::INT32;
            }
            codeBuilder_->AddConstant(inst->CastToConstant()->GetRawValue(), IrTypeToMetainfoType(type),
                                      vreg.GetVRegType());
            break;
        }
        default:
            // Reg-to-reg spill fill must not occurs within SaveState
            UNREACHABLE();
    }
}

void Codegen::FillVregIndices(SaveStateInst *saveState)
{
    for (size_t i = 0; i < saveState->GetInputsCount(); ++i) {
        size_t vregIndex = saveState->GetVirtualRegister(i).Value();
        if (vregIndex == VirtualRegister::BRIDGE) {
            continue;
        }
        ASSERT(vregIndex < vregIndices_.size());
        vregIndices_[vregIndex].first = i;
    }
    for (size_t i = 0; i < saveState->GetImmediatesCount(); i++) {
        auto vregImm = saveState->GetImmediate(i);
        if (vregImm.vreg == VirtualRegister::BRIDGE) {
            continue;
        }
        ASSERT(vregImm.vreg < vregIndices_.size());
        ASSERT(vregIndices_[vregImm.vreg].first == -1);
        vregIndices_[vregImm.vreg].second = i;
    }
}

void Codegen::CreateVRegForRegister(const Location &location, Inst *inst, const VirtualRegister &vreg)
{
    bool isOsr = GetGraph()->IsOsrMode();
    bool isFp = (location.GetKind() == LocationType::FP_REGISTER);
    auto regNum = location.GetValue();
    auto reg = Reg(regNum, isFp ? FLOAT64_TYPE : INT64_TYPE);
    if (!isOsr && GetRegfile()->GetZeroReg() == reg) {
        codeBuilder_->AddConstant(0, IrTypeToMetainfoType(inst->GetType()), vreg.GetVRegType());
    } else if (isOsr || GetRegfile()->IsCalleeRegister(reg)) {
        if (isFp) {
            ASSERT(inst->GetType() != DataType::REFERENCE);
            ASSERT(isOsr || regNum >= GetFirstCalleeReg(GetArch(), true));
            codeBuilder_->AddVReg(VRegInfo(regNum, VRegInfo::Location::FP_REGISTER,
                                           IrTypeToMetainfoType(inst->GetType()), vreg.GetVRegType()));
        } else {
            ASSERT(isOsr || regNum >= GetFirstCalleeReg(GetArch(), false));
            codeBuilder_->AddVReg(VRegInfo(regNum, VRegInfo::Location::REGISTER, IrTypeToMetainfoType(inst->GetType()),
                                           vreg.GetVRegType()));
        }
    } else {
        ASSERT(regNum >= GetFirstCallerReg(GetArch(), isFp));
        auto lastSlot = GetFrameLayout().GetCallerLastSlot(isFp);
        regNum -= GetFirstCallerReg(GetArch(), isFp);
        codeBuilder_->AddVReg(VRegInfo(lastSlot - regNum, VRegInfo::Location::SLOT,
                                       IrTypeToMetainfoType(inst->GetType()), vreg.GetVRegType()));
    }
}

void Codegen::CreateOsrEntry(SaveStateInst *saveState)
{
    auto &stub = osrEntries_.emplace_back(GetAllocator()->New<OsrEntryStub>(this, saveState));
    GetEncoder()->BindLabel(stub->GetLabel());
}

void Codegen::CallIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId id)
{
    SCOPED_DISASM_STR(this, "CallIntrinsic");
    // Call intrinsics isn't supported for IrToc, because we don't know address of intrinsics during IRToc compilation.
    // We store adresses of intrinsics in aot file in AOT mode.
    ASSERT(GetGraph()->SupportManagedCode());
    if (GetGraph()->IsAotMode()) {
        auto aotData = GetGraph()->GetAotData();
        intptr_t offset = aotData->GetEntrypointOffset(GetEncoder()->GetCursorOffset(), static_cast<int32_t>(id));
        GetEncoder()->MakeCallAot(offset);
    } else {
        GetEncoder()->MakeCall(reinterpret_cast<const void *>(GetRuntime()->GetIntrinsicAddress(
            inst->IsRuntimeCall(), GetRuntime()->GetMethodSourceLanguage(GetGraph()->GetMethod()), id)));
    }
}

bool Codegen::EmitCallRuntimeCode(Inst *inst, std::variant<EntrypointId, Reg> entrypoint)
{
    auto encoder = GetEncoder();
    if (std::holds_alternative<Reg>(entrypoint)) {
        auto reg = std::get<Reg>(entrypoint);
        ASSERT(reg.IsValid());
        encoder->MakeCall(reg);
    } else {
        auto id = std::get<EntrypointId>(entrypoint);
        MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), id));
        encoder->MakeCall(entry);
    }

    SaveStateInst *saveState =
        (inst == nullptr || inst->IsSaveState()) ? static_cast<SaveStateInst *>(inst) : inst->GetSaveState();
    // StackMap should follow the call as the bridge function expects retaddr points to the stackmap
    if (saveState != nullptr) {
        CreateStackMap(inst);
    }

    if (std::holds_alternative<EntrypointId>(entrypoint) &&
        GetRuntime()->IsEntrypointNoreturn(std::get<EntrypointId>(entrypoint))) {
        if constexpr (DEBUG_BUILD) {  // NOLINT
            encoder->EncodeAbort();
        }
        return false;
    }
    ASSERT(saveState == nullptr || inst->IsRuntimeCall());

    return true;
}

void Codegen::SaveRegistersForImplicitRuntime(Inst *inst, RegMask *paramsMask, RegMask *mask)
{
    auto &rootsMask = inst->GetSaveState()->GetRootsRegsMask();
    for (auto i = 0U; i < inst->GetInputsCount(); i++) {
        auto input = inst->GetDataFlowInput(i);
        if (!input->IsMovableObject()) {
            continue;
        }
        auto location = inst->GetLocation(i);
        if (location.IsRegister() && location.IsRegisterValid()) {
            auto val = location.GetValue();
            auto reg = Reg(val, INT64_TYPE);
            GetEncoder()->SetRegister(mask, nullptr, reg, true);
            if (DataType::IsReference(inst->GetInputType(i)) && !rootsMask.test(val)) {
                paramsMask->set(val);
                rootsMask.set(val);
            }
        }
    }
}

void Codegen::CreateCheckForTLABWithConstSize([[maybe_unused]] Inst *inst, Reg regTlabStart, Reg regTlabSize,
                                              size_t size, LabelHolder::LabelId label)
{
    SCOPED_DISASM_STR(this, "CreateCheckForTLABWithConstSize");
    auto encoder = GetEncoder();
    if (encoder->CanEncodeImmAddSubCmp(size, WORD_SIZE, false)) {
        encoder->EncodeJump(label, regTlabSize, Imm(size), Condition::LO);
        // Change pointer to start free memory
        encoder->EncodeAdd(regTlabSize, regTlabStart, Imm(size));
    } else {
        ScopedTmpReg sizeReg(encoder);
        encoder->EncodeMov(sizeReg, Imm(size));
        encoder->EncodeJump(label, regTlabSize, sizeReg, Condition::LO);
        encoder->EncodeAdd(regTlabSize, regTlabStart, sizeReg);
    }
}

void Codegen::CreateDebugRuntimeCallsForNewObject(Inst *inst, [[maybe_unused]] Reg regTlabStart, size_t allocSize,
                                                  RegMask preserved)
{
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON)
    CallRuntime(inst, EntrypointId::ANNOTATE_SANITIZERS, INVALID_REGISTER, preserved, regTlabStart,
                TypedImm(allocSize));
#endif
    if (GetRuntime()->IsTrackTlabAlloc()) {
        CallRuntime(inst, EntrypointId::WRITE_TLAB_STATS, INVALID_REGISTER, preserved, regTlabStart,
                    TypedImm(allocSize));
    }
}

void Codegen::CreateNewObjCall(NewObjectInst *newObj)
{
    auto dst = ConvertRegister(newObj->GetDstReg(), newObj->GetType());
    auto src = ConvertRegister(newObj->GetSrcReg(0), newObj->GetInputType(0));
    auto initClass = newObj->GetInput(0).GetInst();
    auto srcClass = ConvertRegister(newObj->GetSrcReg(0), DataType::POINTER);
    auto runtime = GetRuntime();

    auto maxTlabSize = runtime->GetTLABMaxSize();
    if (maxTlabSize == 0 ||
        (initClass->GetOpcode() != Opcode::LoadAndInitClass && initClass->GetOpcode() != Opcode::LoadImmediate)) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    RuntimeInterface::ClassPtr klass;
    if (initClass->GetOpcode() == Opcode::LoadAndInitClass) {
        klass = initClass->CastToLoadAndInitClass()->GetClass();
    } else {
        ASSERT(initClass->GetOpcode() == Opcode::LoadImmediate);
        klass = initClass->CastToLoadImmediate()->GetClass();
    }
    if (klass == nullptr || !runtime->CanUseTlabForClass(klass)) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    auto classSize = runtime->GetClassSize(klass);
    auto alignment = runtime->GetTLABAlignment();

    classSize = (classSize & ~(alignment - 1U)) + ((classSize % alignment) != 0U ? alignment : 0U);
    if (classSize > maxTlabSize) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    CallFastPath(newObj, EntrypointId::ALLOCATE_OBJECT_TLAB, dst, RegMask::GetZeroMask(), srcClass,
                 TypedImm(classSize));
}

void Codegen::CreateNewObjCallOld(NewObjectInst *newObj)
{
    auto dst = ConvertRegister(newObj->GetDstReg(), newObj->GetType());
    auto src = ConvertRegister(newObj->GetSrcReg(0), newObj->GetInputType(0));
    auto initClass = newObj->GetInput(0).GetInst();
    auto runtime = GetRuntime();
    auto maxTlabSize = runtime->GetTLABMaxSize();
    auto encoder = GetEncoder();

    if (maxTlabSize == 0 ||
        (initClass->GetOpcode() != Opcode::LoadAndInitClass && initClass->GetOpcode() != Opcode::LoadImmediate)) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    RuntimeInterface::ClassPtr klass;
    if (initClass->GetOpcode() == Opcode::LoadAndInitClass) {
        klass = initClass->CastToLoadAndInitClass()->GetClass();
    } else {
        ASSERT(initClass->GetOpcode() == Opcode::LoadImmediate);
        klass = initClass->CastToLoadImmediate()->GetClass();
    }
    if (klass == nullptr || !runtime->CanUseTlabForClass(klass)) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    auto classSize = runtime->GetClassSize(klass);
    auto alignment = runtime->GetTLABAlignment();

    classSize = (classSize & ~(alignment - 1U)) + ((classSize % alignment) != 0U ? alignment : 0U);
    if (classSize > maxTlabSize) {
        CallRuntime(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);
        return;
    }
    ScopedLiveTmpReg regTlabStart(encoder);
    ScopedLiveTmpReg regTlabSize(encoder);

    auto slowPath = CreateSlowPath<SlowPathEntrypoint>(newObj, EntrypointId::CREATE_OBJECT_BY_CLASS);
    CreateLoadTLABInformation(regTlabStart, regTlabSize);
    CreateCheckForTLABWithConstSize(newObj, regTlabStart, regTlabSize, classSize, slowPath->GetLabel());

    RegMask preservedRegs;
    encoder->SetRegister(&preservedRegs, nullptr, src);
    CreateDebugRuntimeCallsForNewObject(newObj, regTlabStart, reinterpret_cast<size_t>(classSize), preservedRegs);

    ScopedTmpReg tlabReg(encoder);
    // Load pointer to tlab
    encoder->EncodeLdr(tlabReg, false, MemRef(ThreadReg(), runtime->GetCurrentTLABOffset(GetArch())));

    // Store pointer to the class
    encoder->EncodeStr(src, MemRef(regTlabStart, runtime->GetObjClassOffset(GetArch())));
    encoder->EncodeMov(dst, regTlabStart);
    regTlabStart.Release();
    // Store new pointer to start free memory in TLAB
    encoder->EncodeStrRelease(regTlabSize, MemRef(tlabReg, runtime->GetTLABFreePointerOffset(GetArch())));
    slowPath->BindBackLabel(encoder);
}

void Codegen::LoadClassFromObject(Reg classReg, Reg objReg)
{
    Reg reg = ConvertRegister(classReg.GetId(), DataType::REFERENCE);
    GetEncoder()->EncodeLdr(reg, false, MemRef(objReg, GetRuntime()->GetObjClassOffset(GetArch())));
}

void Codegen::CreateMultiArrayCall(CallInst *callInst)
{
    SCOPED_DISASM_STR(this, "Create Call for MultiArray");

    auto dstReg = ConvertRegister(callInst->GetDstReg(), callInst->GetType());
    auto numArgs = callInst->GetInputsCount() - 2U;  // first is class, last is save_state

    ScopedTmpReg classReg(GetEncoder());
    auto classType = ConvertDataType(DataType::REFERENCE, GetArch());
    Reg classOrig = classReg.GetReg().As(classType);
    auto location = callInst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

    GetEncoder()->EncodeMov(classOrig, ConvertRegister(location.GetValue(), DataType::INT32));
    CallRuntime(callInst, EntrypointId::CREATE_MULTI_ARRAY, dstReg, RegMask::GetZeroMask(), classReg, TypedImm(numArgs),
                SpReg());
    if (callInst->GetFlag(inst_flags::MEM_BARRIER)) {
        GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
}

void Codegen::CreateJumpToClassResolverPltShared(Inst *inst, Reg tmpReg, RuntimeInterface::EntrypointId id)
{
    auto encoder = GetEncoder();
    auto graph = GetGraph();
    auto aotData = graph->GetAotData();
    auto offset = aotData->GetSharedSlowPathOffset(id, encoder->GetCursorOffset());
    if (offset == 0 || !encoder->CanMakeCallByOffset(offset)) {
        SlowPathShared *slowPath;
        auto search = slowPathsMap_.find(id);
        if (search != slowPathsMap_.end()) {
            slowPath = search->second;
            ASSERT(slowPath->GetTmpReg().GetId() == tmpReg.GetId());
        } else {
            slowPath = CreateSlowPath<SlowPathShared>(inst, id);
            slowPath->SetTmpReg(tmpReg);
            slowPathsMap_[id] = slowPath;
        }
        encoder->MakeCall(slowPath->GetLabel());
    } else {
        encoder->MakeCallByOffset(offset);
    }
    CreateStackMap(inst);
}

void Codegen::CreateLoadClassFromPLT(Inst *inst, Reg tmpReg, Reg dst, size_t classId)
{
    auto encoder = GetEncoder();
    auto graph = GetGraph();
    auto aotData = graph->GetAotData();
    intptr_t offset = aotData->GetClassSlotOffset(encoder->GetCursorOffset(), classId, false);
    auto label = encoder->CreateLabel();
    ASSERT(tmpReg.GetId() != dst.GetId());
    ASSERT(inst->IsRuntimeCall());
    encoder->MakeLoadAotTableAddr(offset, tmpReg, dst);
    encoder->EncodeJump(label, dst, Condition::NE);

    // PLT Class Resolver has special calling convention:
    // First encoder temporary (tmp_reg) works as parameter and return value
    CHECK_EQ(tmpReg.GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());

    CreateJumpToClassResolverPltShared(inst, tmpReg, EntrypointId::CLASS_RESOLVER);

    encoder->EncodeMov(dst, tmpReg);
    encoder->BindLabel(label);
}

void Codegen::CreateLoadTLABInformation(Reg regTlabStart, Reg regTlabSize)
{
    SCOPED_DISASM_STR(this, "LoadTLABInformation");
    auto runtime = GetRuntime();
    // Load pointer to tlab
    GetEncoder()->EncodeLdr(regTlabSize, false, MemRef(ThreadReg(), runtime->GetCurrentTLABOffset(GetArch())));
    // Load pointer to start free memory in TLAB
    GetEncoder()->EncodeLdr(regTlabStart, false, MemRef(regTlabSize, runtime->GetTLABFreePointerOffset(GetArch())));
    // Load pointer to end free memory in TLAB
    GetEncoder()->EncodeLdr(regTlabSize, false, MemRef(regTlabSize, runtime->GetTLABEndPointerOffset(GetArch())));
    // Calculate size of the free memory
    GetEncoder()->EncodeSub(regTlabSize, regTlabSize, regTlabStart);
}

// The function alignment up the value from alignment_reg using tmp_reg.
void Codegen::CreateAlignmentValue(Reg alignmentReg, Reg tmpReg, size_t alignment)
{
    auto andVal = ~(alignment - 1U);
    // zeroed lower bits
    GetEncoder()->EncodeAnd(tmpReg, alignmentReg, Imm(andVal));
    GetEncoder()->EncodeSub(alignmentReg, alignmentReg, tmpReg);

    auto endLabel = GetEncoder()->CreateLabel();

    // if zeroed value is different, add alignment
    GetEncoder()->EncodeJump(endLabel, alignmentReg, Condition::EQ);
    GetEncoder()->EncodeAdd(tmpReg, tmpReg, Imm(alignment));

    GetEncoder()->BindLabel(endLabel);
    GetEncoder()->EncodeMov(alignmentReg, tmpReg);
}

inline Reg GetObjectReg(Codegen *codegen, Inst *inst)
{
    ASSERT(inst->IsCall() || inst->GetOpcode() == Opcode::ResolveVirtual);
    auto location = inst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());
    auto objReg = codegen->ConvertRegister(location.GetValue(), inst->GetInputType(0));
    ASSERT(objReg != INVALID_REGISTER);
    return objReg;
}

void Codegen::CreateCallIntrinsic(IntrinsicInst *inst)
{
    if (inst->HasArgumentsOnStack()) {
        // Since for some intrinsics(f.e. TimedWaitNanos) we need SaveState instruction and
        // more than two arguments, we need to place arguments on the stack, but in same time we need to
        // create boundary frame
        LOG(WARNING, COMPILER) << "Intrinsics with arguments on stack are not supported";
        GetEncoder()->SetFalseResult();
        return;
    }

    ASSERT(!HasLiveCallerSavedRegs(inst));
    if (inst->HasImms() && GetGraph()->SupportManagedCode()) {
        EncodeImms(inst->GetImms(), (inst->HasIdInput() || inst->IsMethodFirstInput()));
    }

    size_t explicitArgs;
    if (IsStackRangeIntrinsic(inst->GetIntrinsicId(), &explicitArgs)) {
        auto paramInfo = GetCallingConvention()->GetParameterInfo(explicitArgs);
        auto rangePtrReg =
            ConvertRegister(paramInfo->GetNextLocation(DataType::POINTER).GetRegister(), DataType::POINTER);
        if (inst->GetInputsCount() > explicitArgs + (inst->RequireState() ? 1U : 0U)) {
            auto rangeSpOffs = GetStackOffset(inst->GetLocation(explicitArgs));
            GetEncoder()->EncodeAdd(rangePtrReg, GetTarget().GetStackReg(), Imm(rangeSpOffs));
        }
    }
    if (inst->IsMethodFirstInput()) {
        Reg param0 = GetTarget().GetParamReg(0);
        if (GetGraph()->IsAotMode()) {
            LoadMethod(param0);
        } else {
            GetEncoder()->EncodeMov(param0, Imm(reinterpret_cast<uintptr_t>(inst->GetMethod())));
        }
    }
    CallIntrinsic(inst, inst->GetIntrinsicId());

    if (inst->GetSaveState() != nullptr) {
        // StackMap should follow the call as the bridge function expects retaddr points to the stackmap
        CreateStackMap(inst);
    }

    if (inst->GetType() != DataType::VOID) {
        auto arch = GetArch();
        auto returnType = inst->GetType();
        auto dstReg = ConvertRegister(inst->GetDstReg(), inst->GetType());
        auto returnReg = GetTarget().GetReturnReg(dstReg.GetType());
        // We must:
        //  sign extended INT8 and INT16 to INT32
        //  zero extended UINT8 and UINT16 to UINT32
        if (DataType::ShiftByType(returnType, arch) < DataType::ShiftByType(DataType::INT32, arch)) {
            bool isSigned = DataType::IsTypeSigned(returnType);
            GetEncoder()->EncodeCast(dstReg, isSigned, Reg(returnReg.GetId(), INT32_TYPE), isSigned);
        } else {
            GetEncoder()->EncodeMov(dstReg, returnReg);
        }
    }
}

void Codegen::IntfInlineCachePass(ResolveVirtualInst *resolver, Reg methodReg, Reg tmpReg, Reg objReg)
{
    // Cache structure:（offset addr)/(class addr) 32bit/32bit
    // -----------------------------------------------
    // (.aot_got)
    //     ...
    //     cache:offset/class   <----------|
    //     ...                             |
    // (.text)                             |
    // interface call start                |
    // call runtime irtoc function
    // if call class == cache.class <------|
    //    use cache.offset(method)  <------|
    // else                                |
    //    call RESOLVE_VIRTUAL_CALL_AOT    |
    //    save method‘s offset to cache >--|
    // return to (.text)
    // call method
    // -----------------------------------------------
    auto aotData = GetGraph()->GetAotData();
    uint64_t intfInlineCacheIndex = aotData->GetIntfInlineCacheIndex();
    // NOTE(liyiming): do LoadMethod in irtoc to reduce use tmp reg
    if (objReg.GetId() != tmpReg.GetId()) {
        auto regTmp64 = tmpReg.As(INT64_TYPE);
        uint64_t offset = aotData->GetInfInlineCacheSlotOffset(GetEncoder()->GetCursorOffset(), intfInlineCacheIndex);
        GetEncoder()->MakeLoadAotTableAddr(offset, regTmp64, INVALID_REGISTER);
        LoadMethod(methodReg);
        CallFastPath(resolver, EntrypointId::INTF_INLINE_CACHE, methodReg, {}, methodReg, objReg,
                     TypedImm(resolver->GetCallMethodId()), regTmp64);
    } else {
        // we don't have tmp reg here, so use x3 directly
        Reg reg3 = Reg(3U, INT64_TYPE);
        ScopedTmpRegF64 vtmp(GetEncoder());
        GetEncoder()->EncodeMov(vtmp, reg3);
        uint64_t offset = aotData->GetInfInlineCacheSlotOffset(GetEncoder()->GetCursorOffset(), intfInlineCacheIndex);
        GetEncoder()->MakeLoadAotTableAddr(offset, reg3, INVALID_REGISTER);
        LoadMethod(methodReg);
        CallFastPath(resolver, EntrypointId::INTF_INLINE_CACHE, methodReg, {}, methodReg, objReg,
                     TypedImm(resolver->GetCallMethodId()), reg3);
        GetEncoder()->EncodeMov(reg3, vtmp);
    }

    intfInlineCacheIndex++;
    aotData->SetIntfInlineCacheIndex(intfInlineCacheIndex);
}

/**
 * Returns a pointer to a caller of a virtual/static resolver.
 *
 * InstBuilder uses UnresolvedTypesInterface::AddTableSlot(method_ptr, ...) to allocate cache slots
 * for unresolved methods. The codegen uses UnresolvedTypesInterface::GetTableSlot(method_ptr, ...)
 * to get corresponding slots for unresolved methods.
 *
 * Since there is no valid method_ptr for an unresolved method at the point of IR construction
 * InstBuilder::BuildCallInst uses the pointer to the caller (GetGraph()->GetMethod()) as an argument
 * of UnresolvedTypesInterface::AddTableSlot. So the codegen has to deduce the pointer to the caller
 * and pass it to UnresolvedTypesInterface::GetTableSlot. If the caller was not inlined then
 * GetGraph()->GetMethod() works just fine, otherwise it is done by means of SaveState,
 * which is capable of remebering the corresponding caller instruction.
 *
 * To clarify the details let's consider the following sequence of calls:
 *
 * main() -> foo() -> bar()
 *
 * Suppose that foo() is a static method and bar() is an unresolved virtual method.
 *
 * 1) foo() is going to be inlined
 *
 * To inline foo() the inlining pass creates its IR by means of IRBuilder(..., caller_inst),
 * where caller_inst (the third argument) represents "CallStatic foo" instruction.
 * The caller_inst gets remembered in the corresponding SaveState created for bar's resolver
 * (see InstBuilder::CreateSaveState for the details).
 * At the same time "CallStatic foo" instruction contains a valid method pointer to foo()
 * (it can not be nullptr because we do not inline Unresolved).
 *
 * So we go through the following chain: bar's resolver->SaveState->caller_inst->pointer to the method.
 *
 * 2) foo() is not inlined
 *
 * In this case SaveState.caller_inst_ == nullptr (because foo() is not inlined).
 * Thus we just use plain GetGraph()->GetMethod().
 */
template <typename T>
RuntimeInterface::MethodPtr Codegen::GetCallerOfUnresolvedMethod(T *resolver)
{
    ASSERT(resolver->GetCallMethod() == nullptr);
    auto saveState = resolver->GetSaveState();
    ASSERT(saveState != nullptr);
    auto caller = saveState->GetCallerInst();
    auto method = (caller == nullptr ? GetGraph()->GetMethod() : caller->GetCallMethod());
    ASSERT(method != nullptr);
    return method;
}

void Codegen::EmitResolveUnknownVirtual(ResolveVirtualInst *resolver, Reg methodReg)
{
    SCOPED_DISASM_STR(this, "Create runtime call to resolve an unknown virtual method");
    ASSERT(resolver->GetOpcode() == Opcode::ResolveVirtual);
    auto method = GetCallerOfUnresolvedMethod(resolver);
    ScopedTmpReg tmpReg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
    Reg objReg = GetObjectReg(this, resolver);
    if (GetGraph()->IsAotMode()) {
        LoadMethod(methodReg);
        CallRuntime(resolver, EntrypointId::RESOLVE_VIRTUAL_CALL_AOT, methodReg, {}, methodReg, objReg,
                    TypedImm(resolver->GetCallMethodId()), TypedImm(0));
    } else {
        auto runtime = GetRuntime();
        auto utypes = runtime->GetUnresolvedTypes();
        auto skind = UnresolvedTypesInterface::SlotKind::VIRTUAL_METHOD;
        // Try to load vtable index
        auto methodSlotAddr = utypes->GetTableSlot(method, resolver->GetCallMethodId(), skind);
        GetEncoder()->EncodeMov(methodReg, Imm(methodSlotAddr));
        GetEncoder()->EncodeLdr(methodReg, false, MemRef(methodReg));
        // 0 means the virtual call is uninitialized or it is an interface call
        auto entrypoint = EntrypointId::RESOLVE_UNKNOWN_VIRTUAL_CALL;
        auto slowPath = CreateSlowPath<SlowPathUnresolved>(resolver, entrypoint);
        slowPath->SetUnresolvedType(method, resolver->GetCallMethodId());
        slowPath->SetDstReg(methodReg);
        slowPath->SetArgReg(objReg);
        slowPath->SetSlotAddr(methodSlotAddr);
        GetEncoder()->EncodeJump(slowPath->GetLabel(), methodReg, Condition::EQ);
        // Load klass into tmp_reg
        LoadClassFromObject(tmpReg, objReg);
        // Load from VTable, address = (klass + (index << shift)) + vtable_offset
        auto tmpReg64 = Reg(tmpReg.GetReg().GetId(), INT64_TYPE);
        GetEncoder()->EncodeAdd(tmpReg64, tmpReg, Shift(methodReg, GetVtableShift()));
        GetEncoder()->EncodeLdr(methodReg, false,
                                MemRef(tmpReg64, runtime->GetVTableOffset(GetArch()) - (1U << GetVtableShift())));
        slowPath->BindBackLabel(GetEncoder());
    }
}

void Codegen::EmitResolveVirtualAot(ResolveVirtualInst *resolver, Reg methodReg)
{
    SCOPED_DISASM_STR(this, "AOT ResolveVirtual using PLT-GOT");
    ASSERT(resolver->IsRuntimeCall());
    ScopedTmpReg tmpReg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
    auto methodReg64 = Reg(methodReg.GetId(), INT64_TYPE);
    auto tmpReg64 = Reg(tmpReg.GetReg().GetId(), INT64_TYPE);
    auto aotData = GetGraph()->GetAotData();
    intptr_t offset = aotData->GetVirtIndexSlotOffset(GetEncoder()->GetCursorOffset(), resolver->GetCallMethodId());
    GetEncoder()->MakeLoadAotTableAddr(offset, tmpReg64, methodReg64);
    auto label = GetEncoder()->CreateLabel();
    GetEncoder()->EncodeJump(label, methodReg, Condition::NE);
    GetEncoder()->EncodeMov(methodReg64, tmpReg64);
    // PLT CallVirtual Resolver has a very special calling convention:
    //   First encoder temporary (method_reg) works as a parameter and return value
    CHECK_EQ(methodReg64.GetId(), GetTarget().GetTempRegsMask().GetMinRegister());
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), EntrypointId::CALL_VIRTUAL_RESOLVER));
    GetEncoder()->MakeCall(entry);
    // Need a stackmap to build correct boundary frame
    CreateStackMap(resolver);
    GetEncoder()->BindLabel(label);
    // Load class into method_reg
    Reg objReg = GetObjectReg(this, resolver);
    LoadClassFromObject(tmpReg, objReg);
    // Load from VTable, address = (klass + (index << shift)) + vtable_offset
    GetEncoder()->EncodeAdd(methodReg, tmpReg64, Shift(methodReg, GetVtableShift()));
    GetEncoder()->EncodeLdr(methodReg, false,
                            MemRef(methodReg, GetRuntime()->GetVTableOffset(GetArch()) - (1U << GetVtableShift())));
}

void Codegen::EmitResolveVirtual(ResolveVirtualInst *resolver)
{
    auto methodReg = ConvertRegister(resolver->GetDstReg(), resolver->GetType());
    auto objectReg = ConvertRegister(resolver->GetSrcReg(0), DataType::REFERENCE);
    ScopedTmpReg tmpMethodReg(GetEncoder());
    if (resolver->GetCallMethod() == nullptr) {
        EmitResolveUnknownVirtual(resolver, tmpMethodReg);
        GetEncoder()->EncodeMov(methodReg, tmpMethodReg);
    } else if (GetRuntime()->IsInterfaceMethod(resolver->GetCallMethod())) {
        SCOPED_DISASM_STR(this, "Create runtime call to resolve a known virtual call");
        if (GetGraph()->IsAotMode()) {
            if (GetArch() == Arch::AARCH64) {
                ScopedTmpReg tmpReg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
                IntfInlineCachePass(resolver, tmpMethodReg, tmpReg, objectReg);
            } else {
                LoadMethod(tmpMethodReg);
                CallRuntime(resolver, EntrypointId::RESOLVE_VIRTUAL_CALL_AOT, tmpMethodReg, {}, tmpMethodReg, objectReg,
                            TypedImm(resolver->GetCallMethodId()), TypedImm(0));
            }
        } else {
            CallRuntime(resolver, EntrypointId::RESOLVE_VIRTUAL_CALL, tmpMethodReg, {},
                        TypedImm(reinterpret_cast<size_t>(resolver->GetCallMethod())), objectReg);
        }
        GetEncoder()->EncodeMov(methodReg, tmpMethodReg);
    } else if (GetGraph()->IsAotNoChaMode()) {
        // ResolveVirtualAot requires method_reg to be the first tmp register.
        EmitResolveVirtualAot(resolver, tmpMethodReg);
        GetEncoder()->EncodeMov(methodReg, tmpMethodReg);
    } else {
        UNREACHABLE();
    }
}

void Codegen::EmitCallResolvedVirtual(CallInst *call)
{
    SCOPED_DISASM_STR(this, "Create a call to resolved virtual method");
    ASSERT(call->GetOpcode() == Opcode::CallResolvedVirtual);
    if (call->GetSaveState() != nullptr && call->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                         Imm(reinterpret_cast<size_t>(call->GetCallMethod())),
                         Imm(static_cast<size_t>(events::MethodEnterKind::INLINED))});
        }
#endif
        return;
    }
    ASSERT(!HasLiveCallerSavedRegs(call));
    ASSERT(call->GetCallMethod() == nullptr || GetGraph()->GetRuntime()->IsInterfaceMethod(call->GetCallMethod()) ||
           GetGraph()->IsAotNoChaMode());
    Reg methodReg = ConvertRegister(call->GetSrcReg(0), DataType::POINTER);
    Reg param0 = GetTarget().GetParamReg(0);
    // Set method
    GetEncoder()->EncodeMov(param0, methodReg);
    GetEncoder()->MakeCall(MemRef(param0, GetRuntime()->GetCompiledEntryPointOffset(GetArch())));
    FinalizeCall(call);
}

void Codegen::EmitCallVirtual(CallInst *call)
{
    SCOPED_DISASM_STR(this, "Create a call to virtual method");
    ASSERT(call->GetOpcode() == Opcode::CallVirtual);
    if (call->GetSaveState() != nullptr && call->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                         Imm(reinterpret_cast<size_t>(call->GetCallMethod())),
                         Imm(static_cast<size_t>(events::MethodEnterKind::INLINED))});
        }
#endif
        return;
    }
    auto runtime = GetRuntime();
    auto method = call->GetCallMethod();
    ASSERT(!HasLiveCallerSavedRegs(call));
    ASSERT(!call->IsUnresolved() && !runtime->IsInterfaceMethod(method) && !GetGraph()->IsAotNoChaMode());
    Reg methodReg = GetTarget().GetParamReg(0);
    ASSERT(!RegisterKeepCallArgument(call, methodReg));
    LoadClassFromObject(methodReg, GetObjectReg(this, call));
    // Get index
    auto vtableIndex = runtime->GetVTableIndex(method);
    // Load from VTable, address = klass + ((index << shift) + vtable_offset)
    auto totalOffset = runtime->GetVTableOffset(GetArch()) + (vtableIndex << GetVtableShift());
    // Class ref was loaded to method_reg
    GetEncoder()->EncodeLdr(methodReg, false, MemRef(methodReg, totalOffset));
    // Set method
    GetEncoder()->MakeCall(MemRef(methodReg, runtime->GetCompiledEntryPointOffset(GetArch())));
    FinalizeCall(call);
}

[[maybe_unused]] bool Codegen::EnsureParamsFitIn32Bit(std::initializer_list<std::variant<Reg, TypedImm>> params)
{
    for (auto &param : params) {
        if (std::holds_alternative<Reg>(param)) {
            auto reg = std::get<Reg>(param);
            if (reg.GetSize() > WORD_SIZE) {
                return false;
            }
        } else {
            auto immType = std::get<TypedImm>(param).GetType();
            if (immType.GetSize() > WORD_SIZE) {
                return false;
            }
        }
    }
    return true;
}

void Codegen::EmitResolveStatic(ResolveStaticInst *resolver)
{
    auto methodReg = ConvertRegister(resolver->GetDstReg(), resolver->GetType());
    if (GetGraph()->IsAotMode()) {
        LoadMethod(methodReg);
        CallRuntime(resolver, EntrypointId::GET_UNKNOWN_CALLEE_METHOD, methodReg, RegMask::GetZeroMask(), methodReg,
                    TypedImm(resolver->GetCallMethodId()), TypedImm(0));
        return;
    }
    auto method = GetCallerOfUnresolvedMethod(resolver);
    ScopedTmpReg tmp(GetEncoder());
    auto utypes = GetRuntime()->GetUnresolvedTypes();
    auto skind = UnresolvedTypesInterface::SlotKind::METHOD;
    auto methodAddr = utypes->GetTableSlot(method, resolver->GetCallMethodId(), skind);
    GetEncoder()->EncodeMov(tmp.GetReg(), Imm(methodAddr));
    GetEncoder()->EncodeLdr(tmp.GetReg(), false, MemRef(tmp.GetReg()));
    auto slowPath = CreateSlowPath<SlowPathUnresolved>(resolver, EntrypointId::GET_UNKNOWN_CALLEE_METHOD);
    slowPath->SetUnresolvedType(method, resolver->GetCallMethodId());
    slowPath->SetDstReg(tmp.GetReg());
    slowPath->SetSlotAddr(methodAddr);
    GetEncoder()->EncodeJump(slowPath->GetLabel(), tmp.GetReg(), Condition::EQ);
    slowPath->BindBackLabel(GetEncoder());
    GetEncoder()->EncodeMov(methodReg, tmp.GetReg());
}

void Codegen::EmitCallResolvedStatic(CallInst *call)
{
    ASSERT(call->GetOpcode() == Opcode::CallResolvedStatic && call->GetCallMethod() == nullptr);
    Reg methodReg = ConvertRegister(call->GetSrcReg(0), DataType::POINTER);
    Reg param0 = GetTarget().GetParamReg(0);
    GetEncoder()->EncodeMov(param0, methodReg);
    GetEncoder()->MakeCall(MemRef(param0, GetRuntime()->GetCompiledEntryPointOffset(GetArch())));
    FinalizeCall(call);
}

void Codegen::EmitCallStatic(CallInst *call)
{
    ASSERT(call->IsStaticCall() && !call->IsUnresolved());
    if (call->GetSaveState() != nullptr && call->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                         Imm(reinterpret_cast<size_t>(call->GetCallMethod())),
                         Imm(static_cast<size_t>(events::MethodEnterKind::INLINED))});
        }
#endif
        return;
    }
    SCOPED_DISASM_STR(this, "Create a call to static");
    ASSERT(!HasLiveCallerSavedRegs(call));
    // Now MakeCallByOffset is not supported in Arch32Encoder (support ADR instruction)
    Reg param0 = GetTarget().GetParamReg(0);
    if (call->GetCallMethod() == GetGraph()->GetMethod() && GetArch() != Arch::AARCH32 && !GetGraph()->IsOsrMode() &&
        !GetGraph()->GetMethodProperties().GetHasDeopt()) {
        if (GetGraph()->IsAotMode()) {
            LoadMethod(param0);
        } else {
            GetEncoder()->EncodeMov(param0, Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())));
        }
        GetEncoder()->MakeCallByOffset(GetStartCodeOffset() - GetEncoder()->GetCursorOffset());
    } else {
        if (GetGraph()->IsAotMode()) {
            auto aotData = GetGraph()->GetAotData();
            intptr_t offset = aotData->GetPltSlotOffset(GetEncoder()->GetCursorOffset(), call->GetCallMethodId());
            // PLT CallStatic Resolver transparently uses param_0 (Method) register
            GetEncoder()->MakeLoadAotTable(offset, param0);
        } else {  // usual JIT case
            auto method = call->GetCallMethod();
            GetEncoder()->EncodeMov(param0, Imm(reinterpret_cast<size_t>(method)));
        }
        GetEncoder()->MakeCall(MemRef(param0, GetRuntime()->GetCompiledEntryPointOffset(GetArch())));
    }
    FinalizeCall(call);
}

void Codegen::EmitCallDynamic(CallInst *call)
{
    SCOPED_DISASM_STR(this, "Create a dynamic call");
    if (call->GetSaveState() != nullptr && call->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace(Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                        Imm(reinterpret_cast<size_t>(call->GetCallMethod())),
                        Imm(static_cast<size_t>(events::MethodEnterKind::INLINED)));
        }
#endif
        return;
    }
    RuntimeInterface *runtime = GetRuntime();
    Encoder *encoder = GetEncoder();

    auto dstReg = ConvertRegister(call->GetDstReg(), call->GetType());
    Reg methodParamReg = GetTarget().GetParamReg(CallConvDynInfo::REG_METHOD).As(GetPtrRegType());
    Reg numArgsParamReg = GetTarget().GetParamReg(CallConvDynInfo::REG_NUM_ARGS);
    auto paramFuncObjLoc = Location::MakeStackArgument(CallConvDynInfo::SLOT_CALLEE);

    ASSERT(!HasLiveCallerSavedRegs(call));

    // Load method from callee object
    static_assert(coretypes::TaggedValue::TAG_OBJECT == 0);
    encoder->EncodeLdr(methodParamReg, false, MemRef(SpReg(), GetStackOffset(paramFuncObjLoc)));
    encoder->EncodeLdr(methodParamReg, false, MemRef(methodParamReg, runtime->GetFunctionTargetOffset(GetArch())));

    ASSERT(call->GetInputsCount() > 1);
    auto numArgs = static_cast<uint32_t>(call->GetInputsCount() - 1);  // '-1' means 1 for spill fill input
    encoder->EncodeMov(numArgsParamReg, Imm(numArgs));

    size_t entryPointOffset = runtime->GetCompiledEntryPointOffset(GetArch());
    encoder->MakeCall(MemRef(methodParamReg, entryPointOffset));

    CreateStackMap(call);
    // Dynamic callee may have moved sp if there was insufficient num_actual_args
    encoder->EncodeSub(
        SpReg(), FpReg(),
        Imm(GetFrameLayout().GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::BYTES>(0)));

    if (dstReg.IsValid()) {
        Reg retReg = GetTarget().GetReturnReg(dstReg.GetType());
        encoder->EncodeMov(dstReg, retReg);
    }
}

void Codegen::FinalizeCall(CallInst *call)
{
    ASSERT(!call->IsDynamicCall());
    CreateStackMap(call);
    auto returnType = call->GetType();
    auto dstReg = ConvertRegister(call->GetDstReg(), returnType);
    // Restore frame pointer in the TLS
    GetEncoder()->EncodeStr(FpReg(), MemRef(ThreadReg(), GetRuntime()->GetTlsFrameOffset(GetArch())));
    // Sign/Zero extend return_reg if necessary
    if (dstReg.IsValid()) {
        auto arch = GetArch();
        auto returnReg = GetTarget().GetReturnReg(dstReg.GetType());
        //  INT8  and INT16  must be sign extended to INT32
        //  UINT8 and UINT16 must be zero extended to UINT32
        if (DataType::ShiftByType(returnType, arch) < DataType::ShiftByType(DataType::INT32, arch)) {
            bool isSigned = DataType::IsTypeSigned(returnType);
            GetEncoder()->EncodeCast(dstReg, isSigned, Reg(returnReg.GetId(), INT32_TYPE), isSigned);
        } else {
            GetEncoder()->EncodeMov(dstReg, returnReg);
        }
    }
}

template <typename T>
static T GetBarrierOperandValue(RuntimeInterface *runtime, panda::mem::BarrierPosition position, std::string_view name)
{
    auto operand = runtime->GetBarrierOperand(position, name);
    return std::get<T>(operand.GetValue());
}

template <bool IS_CLASS>
void Codegen::CreatePreWRB(Inst *inst, MemRef mem, RegMask preserved, bool storePair)
{
    auto runtime = GetRuntime();
    auto *enc = GetEncoder();

    SCOPED_DISASM_STR(this, "Pre WRB");
    ScopedTmpReg entrypointReg(enc, enc->IsLrAsTempRegEnabledAndReleased());
    GetEncoder()->EncodeLdr(entrypointReg, false,
                            MemRef(ThreadReg(), runtime->GetTlsPreWrbEntrypointOffset(GetArch())));

    // Check entrypoint address
    auto label = GetEncoder()->CreateLabel();
    enc->EncodeJump(label, entrypointReg, Condition::EQ);
    auto refType =
        inst->GetType() == DataType::REFERENCE ? DataType::GetIntTypeForReference(enc->GetArch()) : DataType::INT64;
    ScopedTmpReg tmpRef(enc, ConvertDataType(refType, GetArch()));
    auto prevOffset = enc->GetCursorOffset();
    // Load old value
    if (IsVolatileMemInst(inst)) {
        enc->EncodeLdrAcquire(tmpRef, false, mem);
    } else {
        enc->EncodeLdr(tmpRef, false, mem);
    }
    TryInsertImplicitNullCheck(inst, prevOffset);
    if constexpr (IS_CLASS) {
        enc->EncodeLdr(tmpRef, false, MemRef(tmpRef, runtime->GetManagedClassOffset(GetArch())));
    } else {
        CheckObject(tmpRef, label);
    }
    auto [live_regs, live_vregs] = GetLiveRegisters<true>(inst);
    live_regs |= preserved;
    CallBarrier(live_regs, live_vregs, entrypointReg.GetReg(), tmpRef);

    if (storePair) {
        // store pair doesn't support index and scalar
        ASSERT(!mem.HasIndex() && !mem.HasScale());
        // calculate offset for second store
        // NOLINTNEXTLINE(bugprone-misplaced-widening-cast)
        auto secondOffset = static_cast<ssize_t>(1U << DataType::ShiftByType(DataType::REFERENCE, enc->GetArch()));
        if (mem.HasDisp()) {
            secondOffset += mem.GetDisp();
        }
        // Load old value
        if (IsVolatileMemInst(inst)) {
            enc->EncodeLdrAcquire(tmpRef, false, MemRef(mem.GetBase(), secondOffset));
        } else {
            enc->EncodeLdr(tmpRef, false, MemRef(mem.GetBase(), secondOffset));
        }
        CheckObject(tmpRef, label);
        CallBarrier(live_regs, live_vregs, entrypointReg.GetReg(), tmpRef);
    }
    enc->BindLabel(label);
}

void Codegen::EncodePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool checkObject)
{
    auto refType {TypeInfo::FromDataType(DataType::REFERENCE, GetArch())};
    ASSERT(reg1.IsValid());
    reg1 = reg1.As(refType);
    if (reg2.IsValid()) {
        reg2 = reg2.As(refType);
    }

    if (GetGraph()->SupportsIrtocBarriers()) {
        if (GetGraph()->IsOfflineCompilationMode()) {
            CreateOfflineIrtocPostWrb(inst, mem, reg1, reg2);
        } else {
            CreateOnlineIrtocPostWrb(inst, mem, reg1, reg2, checkObject);
        }
        return;
    }

    auto barrierType {GetRuntime()->GetPostType()};
    ASSERT(barrierType == panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER ||
           barrierType == panda::mem::BarrierType::POST_INTERREGION_BARRIER);

    if (barrierType == panda::mem::BarrierType::POST_INTERREGION_BARRIER) {
        CreatePostInterRegionBarrier(inst, mem, reg1, reg2, checkObject);
    } else {
        auto base {mem.GetBase().As(refType)};
        CreatePostInterGenerationalBarrier(base);
    }
}

void Codegen::CreatePostWRBForDynamic(Inst *inst, MemRef mem, Reg reg1, Reg reg2)
{
    int storeIndex;
    if (reg2 == INVALID_REGISTER) {
        if (inst->GetOpcode() == Opcode::StoreObject || inst->GetOpcode() == Opcode::StoreI ||
            inst->GetOpcode() == Opcode::StoreArrayI) {
            storeIndex = 1_I;
        } else {
            ASSERT(inst->GetOpcode() == Opcode::StoreArray || inst->GetOpcode() == Opcode::Store);
            storeIndex = 2_I;
        }
        if (StoreValueCanBeObject(inst->GetDataFlowInput(storeIndex))) {
            EncodePostWRB(inst, mem, reg1, reg2, true);
        }
    } else {
        if (inst->GetOpcode() == Opcode::StoreArrayPairI) {
            storeIndex = 1_I;
        } else {
            ASSERT(inst->GetOpcode() == Opcode::StoreArrayPair);
            storeIndex = 2_I;
        }
        bool firstIsObject = StoreValueCanBeObject(inst->GetDataFlowInput(storeIndex));
        bool secondIsObject = StoreValueCanBeObject(inst->GetDataFlowInput(storeIndex + 1));
        if (firstIsObject || secondIsObject) {
            if (firstIsObject && !secondIsObject) {
                reg2 = INVALID_REGISTER;
            } else if (!firstIsObject && secondIsObject) {
                reg1 = reg2;
                reg2 = INVALID_REGISTER;
            }
            EncodePostWRB(inst, mem, reg1, reg2, true);
        }
    }
}

void Codegen::CreatePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2)
{
    ASSERT(reg1 != INVALID_REGISTER);

    if (!GetGraph()->SupportsIrtocBarriers() || !GetGraph()->IsOfflineCompilationMode()) {
        auto barrierType = GetRuntime()->GetPostType();
        if (barrierType == panda::mem::BarrierType::POST_WRB_NONE) {
            return;
        }
        ASSERT(barrierType == panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER ||
               barrierType == panda::mem::BarrierType::POST_INTERREGION_BARRIER);
    }

    // For dynamic methods, another check
    if (GetGraph()->IsDynamicMethod()) {
        CreatePostWRBForDynamic(inst, mem, reg1, reg2);
        return;
    }
    Inst *secondValue;
    Inst *val = InstStoredValue(inst, &secondValue);
    ASSERT(secondValue == nullptr || reg2 != INVALID_REGISTER);
    if (val->GetOpcode() == Opcode::NullPtr) {
        // We can don't encode Post barrier for nullptr
        if (secondValue == nullptr || secondValue->GetOpcode() == Opcode::NullPtr) {
            return;
        }
        // CallPostWRB only for second reg
        EncodePostWRB(inst, mem, reg2, INVALID_REGISTER, !IsInstNotNull(secondValue));
        return;
    }
    // Create PostWRB only for first value
    if (secondValue != nullptr && secondValue->GetOpcode() == Opcode::NullPtr) {
        reg2 = INVALID_REGISTER;
    }
    bool checkObject = true;
    if (reg2 == INVALID_REGISTER) {
        if (IsInstNotNull(val)) {
            checkObject = false;
        }
    } else {
        if (IsInstNotNull(val) && IsInstNotNull(secondValue)) {
            checkObject = false;
        }
    }
    EncodePostWRB(inst, mem, reg1, reg2, checkObject);
}

void Codegen::CheckObject(Reg reg, LabelHolder::LabelId label)
{
    auto graph = GetGraph();
    auto *enc = GetEncoder();

    // interpreter use x20 in IrToc and we don't have enough temporary registers
    // remove after PR 98 or PR 47
    if (graph->IsDynamicMethod()) {
        ASSERT(reg.IsScalar());
        reg = reg.As(INT64_TYPE);
        auto tagMask = graph->GetRuntime()->GetTaggedTagMask();
        // Check that the value is object(not int and not double)
        enc->EncodeJumpTest(label, reg, Imm(tagMask), Condition::TST_NE);
        auto specialMask = graph->GetRuntime()->GetTaggedSpecialMask();
        // Check that the value is not special value
        enc->EncodeJumpTest(label, reg, Imm(~specialMask), Condition::TST_EQ);
    } else {
        enc->EncodeJump(label, reg, Condition::EQ);
    }
}

void Codegen::CreateOfflineIrtocPostWrb(Inst *inst, MemRef mem, Reg reg1, Reg reg2)
{
    ASSERT(reg1.IsValid());

    bool hasObj2 {reg2.IsValid() && reg1 != reg2};

    auto paramRegs {GetTarget().GetParamRegsMask(hasObj2 ? 4U : 3U) & GetLiveRegisters(inst).first};
    SaveCallerRegisters(paramRegs, VRegMask(), false);

    if (hasObj2) {
        FillPostWrbCallParams(mem, reg1, reg2);
    } else {
        FillPostWrbCallParams(mem, reg1);
    }

    // load function pointer from tls field
    auto offset {hasObj2 ? cross_values::GetManagedThreadPostWrbTwoObjectsOffset(GetArch())
                         : cross_values::GetManagedThreadPostWrbOneObjectOffset(GetArch())};
    MemRef entry(ThreadReg(), offset);
    GetEncoder()->MakeCall(entry);

    LoadCallerRegisters(paramRegs, VRegMask(), false);
}

void CheckObj(Encoder *enc, Codegen *cg, Reg base, Reg reg1, LabelHolder::LabelId skipLabel, bool checkNull)
{
    if (checkNull) {
        // Fast null check in-place for one register
        enc->EncodeJump(skipLabel, reg1, Condition::EQ);
    }

    ScopedTmpReg tmp(enc, cg->ConvertDataType(DataType::REFERENCE, cg->GetArch()));
    auto regionSizeBit = GetBarrierOperandValue<uint8_t>(
        cg->GetRuntime(), panda::mem::BarrierPosition::BARRIER_POSITION_POST, "REGION_SIZE_BITS");
    enc->EncodeXor(tmp, base, reg1);
    enc->EncodeShr(tmp, tmp, Imm(regionSizeBit));
    enc->EncodeJump(skipLabel, tmp, Condition::EQ);
}

void WrapOneArg(Encoder *enc, Reg param, Reg base, MemRef mem, size_t additionalOffset = 0)
{
    if (mem.HasIndex()) {
        ASSERT(mem.GetScale() == 0 && !mem.HasDisp());
        enc->EncodeAdd(param, base, mem.GetIndex());
        if (additionalOffset != 0) {
            enc->EncodeAdd(param, param, Imm(additionalOffset));
        }
    } else if (mem.HasDisp()) {
        ASSERT(!mem.HasIndex());
        enc->EncodeAdd(param, base, Imm(mem.GetDisp() + additionalOffset));
    } else {
        enc->EncodeAdd(param, base, Imm(additionalOffset));
    }
}

/**
 * Post-write barrier for StorePair case.
 * The code below may mark 0, 1 or 2 cards. The idea is following:
 *  if (the second object isn't null and is allocated in other from base object's region)
 *      MarkOneCard(CardOfSecondField(mem))
 *      if (address of the second field isn't aligned at the size of a card)
 *          # i.e. each of store address (fields of the base objects) are related to the same card
 *          goto Done
 *
 *  if (the first object isn't null and is allocated in other from base object's region)
 *      MarkOneCard(CardOfFirstField(mem))
 *
 *  label: Done
 */
void Codegen::CreateOnlineIrtocPostWrbRegionTwoRegs(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool checkObject)
{
    auto entrypointId {EntrypointId::POST_INTER_REGION_BARRIER_SLOW};
    auto enc {GetEncoder()};
    auto base {mem.GetBase().As(TypeInfo::FromDataType(DataType::REFERENCE, GetArch()))};
    auto paramReg0 = enc->GetTarget().GetParamReg(0);
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), entrypointId));
    constexpr size_t PARAMS_NUM = 1U;
    auto paramRegs {GetTarget().GetParamRegsMask(PARAMS_NUM) & GetLiveRegisters(inst).first};

    auto lblMarkCardAndExit = enc->CreateLabel();
    auto lblCheck1Obj = enc->CreateLabel();
    auto done = enc->CreateLabel();

    CheckObj(enc, this, base, reg2, lblCheck1Obj, checkObject);
    SaveCallerRegisters(paramRegs, VRegMask(), false);
    WrapOneArg(enc, paramReg0, base, mem, reg1.GetSize() / BITS_PER_BYTE);
    {
        ScopedTmpReg tmp(enc, ConvertDataType(DataType::REFERENCE, GetArch()));
        enc->EncodeAnd(tmp, paramReg0, Imm(cross_values::GetCardAlignmentMask(GetArch())));
        enc->EncodeJump(lblMarkCardAndExit, tmp, Condition::NE);
    }

    enc->MakeCall(entry);
    LoadCallerRegisters(paramRegs, VRegMask(), false);

    enc->BindLabel(lblCheck1Obj);
    CheckObj(enc, this, base, reg1, done, checkObject);
    SaveCallerRegisters(paramRegs, VRegMask(), false);
    WrapOneArg(enc, paramReg0, base, mem);
    enc->BindLabel(lblMarkCardAndExit);
    enc->MakeCall(entry);
    LoadCallerRegisters(paramRegs, VRegMask(), false);
    enc->BindLabel(done);
}

void Codegen::CreateOnlineIrtocPostWrbRegionOneReg(Inst *inst, MemRef mem, Reg reg1, bool checkObject)
{
    auto entrypointId {EntrypointId::POST_INTER_REGION_BARRIER_SLOW};
    auto enc {GetEncoder()};
    auto base {mem.GetBase().As(TypeInfo::FromDataType(DataType::REFERENCE, GetArch()))};
    auto paramReg0 = enc->GetTarget().GetParamReg(0);
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), entrypointId));
    constexpr size_t PARAMS_NUM = 1;
    auto paramRegs {GetTarget().GetParamRegsMask(PARAMS_NUM) & GetLiveRegisters(inst).first};

    auto skip = enc->CreateLabel();

    CheckObj(enc, this, base, reg1, skip, checkObject);
    SaveCallerRegisters(paramRegs, VRegMask(), false);
    WrapOneArg(enc, paramReg0, base, mem);
    enc->MakeCall(entry);
    LoadCallerRegisters(paramRegs, VRegMask(), false);
    enc->BindLabel(skip);
}

void Codegen::CreateOnlineIrtocPostWrb(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool checkObject)
{
    SCOPED_DISASM_STR(this, "Post Online Irtoc-WRB");
    ASSERT(reg1.IsValid());

    auto enc {GetEncoder()};

    bool hasObj2 {reg2.IsValid() && reg1 != reg2};
    if (GetRuntime()->GetPostType() == panda::mem::BarrierType::POST_INTERREGION_BARRIER) {
        if (hasObj2) {
            CreateOnlineIrtocPostWrbRegionTwoRegs(inst, mem, reg1, reg2, checkObject);
        } else {
            CreateOnlineIrtocPostWrbRegionOneReg(inst, mem, reg1, checkObject);
        }
    } else {
        auto entrypointId {EntrypointId::POST_INTER_GENERATIONAL_BARRIER0};
        auto base {mem.GetBase().As(TypeInfo::FromDataType(DataType::REFERENCE, GetArch()))};
        constexpr size_t PARAMS_NUM = 1;
        auto paramRegs {GetTarget().GetParamRegsMask(PARAMS_NUM) & GetLiveRegisters(inst).first};

        SaveCallerRegisters(paramRegs, VRegMask(), false);
        auto paramReg0 = enc->GetTarget().GetParamReg(0);
        enc->EncodeMov(paramReg0, base);
        MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), entrypointId));
        enc->MakeCall(entry);
        LoadCallerRegisters(paramRegs, VRegMask(), false);
    }
}

void Codegen::CreatePostInterRegionBarrier(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool checkObject)
{
    SCOPED_DISASM_STR(this, "Post IR-WRB");
    auto *enc = GetEncoder();
    ASSERT(GetRuntime()->GetPostType() == panda::mem::BarrierType::POST_INTERREGION_BARRIER);
    ASSERT(reg1 != INVALID_REGISTER);

    auto label = GetEncoder()->CreateLabel();

    if (checkObject) {
        CheckObject(reg1, label);
    }

    auto regionSizeBit = GetBarrierOperandValue<uint8_t>(
        GetRuntime(), panda::mem::BarrierPosition::BARRIER_POSITION_POST, "REGION_SIZE_BITS");

    auto base {mem.GetBase().As(TypeInfo::FromDataType(DataType::REFERENCE, GetArch()))};
    ScopedTmpReg tmp(enc, ConvertDataType(DataType::REFERENCE, GetArch()));
    // Compare first store value with mem
    enc->EncodeXor(tmp, base, reg1);
    enc->EncodeShr(tmp, tmp, Imm(regionSizeBit));

    enc->EncodeJump(label, tmp, Condition::EQ);
    auto [live_regs, live_vregs] = GetLiveRegisters<true>(inst);

    if (mem.HasIndex()) {
        ASSERT(mem.GetScale() == 0 && !mem.HasDisp());
        enc->EncodeAdd(tmp, base, mem.GetIndex());
        CallBarrier(live_regs, live_vregs, EntrypointId::POST_WRB_UPDATE_CARD_FUNC_NO_BRIDGE, tmp, reg1);
    } else if (mem.HasDisp()) {
        ASSERT(!mem.HasIndex());
        enc->EncodeAdd(tmp, base, Imm(mem.GetDisp()));
        CallBarrier(live_regs, live_vregs, EntrypointId::POST_WRB_UPDATE_CARD_FUNC_NO_BRIDGE, tmp, reg1);
    } else {
        CallBarrier(live_regs, live_vregs, EntrypointId::POST_WRB_UPDATE_CARD_FUNC_NO_BRIDGE, base, reg1);
    }
    enc->BindLabel(label);

    if (reg2.IsValid() && reg1 != reg2) {
        auto label1 = GetEncoder()->CreateLabel();
        if (checkObject) {
            CheckObject(reg2, label1);
        }

        // Compare second store value with mem
        enc->EncodeXor(tmp, base, reg2);
        enc->EncodeShr(tmp, tmp, Imm(regionSizeBit));
        enc->EncodeJump(label1, tmp, Condition::EQ);

        enc->EncodeAdd(tmp, base, Imm(reg1.GetSize() / BITS_PER_BYTE));
        if (mem.HasIndex()) {
            ASSERT(mem.GetScale() == 0 && !mem.HasDisp());
            enc->EncodeAdd(tmp, tmp, mem.GetIndex());
        } else if (mem.HasDisp()) {
            ASSERT(!mem.HasIndex());
            enc->EncodeAdd(tmp, tmp, Imm(mem.GetDisp()));
        }
        CallBarrier(live_regs, live_vregs, EntrypointId::POST_WRB_UPDATE_CARD_FUNC_NO_BRIDGE, tmp, reg2);
        enc->BindLabel(label1);
    }
}

void Codegen::CalculateCardIndex(Reg baseReg, ScopedTmpReg *tmp, ScopedTmpReg *tmp1)
{
    auto tmpType = tmp->GetReg().GetType();
    auto tmp1Type = tmp1->GetReg().GetType();
    auto *enc = GetEncoder();
    auto cardBits =
        GetBarrierOperandValue<uint8_t>(GetRuntime(), panda::mem::BarrierPosition::BARRIER_POSITION_POST, "CARD_BITS");

    ASSERT(baseReg != INVALID_REGISTER);
    if (baseReg.GetSize() < Reg(*tmp).GetSize()) {
        tmp->ChangeType(baseReg.GetType());
        tmp1->ChangeType(baseReg.GetType());
    }
    enc->EncodeSub(*tmp, baseReg, *tmp);
    enc->EncodeShr(*tmp, *tmp, Imm(cardBits));
    tmp->ChangeType(tmpType);
    tmp1->ChangeType(tmp1Type);
}

void Codegen::CreatePostInterGenerationalBarrier(Reg base)
{
    SCOPED_DISASM_STR(this, "Post IG-WRB");
    auto runtime = GetRuntime();
    auto *enc = GetEncoder();
    ASSERT(runtime->GetPostType() == panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER);
    ScopedTmpReg tmp(enc);
    ScopedTmpReg tmp1(enc);
    // * load AddressOf(MIN_ADDR) -> min_addr
    if (GetGraph()->IsOfflineCompilationMode()) {
        GetEncoder()->EncodeLdr(tmp, false, MemRef(ThreadReg(), runtime->GetTlsCardTableMinAddrOffset(GetArch())));
    } else {
        auto minAddress = reinterpret_cast<uintptr_t>(
            GetBarrierOperandValue<void *>(runtime, panda::mem::BarrierPosition::BARRIER_POSITION_POST, "MIN_ADDR"));
        enc->EncodeMov(tmp, Imm(minAddress));
    }
    // * card_index = (AddressOf(obj.field) - min_addr) >> CARD_BITS   // shift right
    CalculateCardIndex(base, &tmp, &tmp1);
    // * load AddressOf(CARD_TABLE_ADDR) -> card_table_addr
    if (GetGraph()->IsOfflineCompilationMode()) {
        GetEncoder()->EncodeLdr(tmp1.GetReg().As(INT64_TYPE), false,
                                MemRef(ThreadReg(), runtime->GetTlsCardTableAddrOffset(GetArch())));
    } else {
        auto cardTableAddr = reinterpret_cast<uintptr_t>(GetBarrierOperandValue<uint8_t *>(
            runtime, panda::mem::BarrierPosition::BARRIER_POSITION_POST, "CARD_TABLE_ADDR"));
        enc->EncodeMov(tmp1, Imm(cardTableAddr));
    }
    // * card_addr = card_table_addr + card_index
    enc->EncodeAdd(tmp, tmp1, tmp);
    // * store card_addr <- DIRTY_VAL
    auto dirtyVal =
        GetBarrierOperandValue<uint8_t>(runtime, panda::mem::BarrierPosition::BARRIER_POSITION_POST, "DIRTY_VAL");

    auto tmp1B = ConvertRegister(tmp1.GetReg().GetId(), DataType::INT8);
    enc->EncodeMov(tmp1B, Imm(dirtyVal));
    enc->EncodeStr(tmp1B, MemRef(tmp));
}

bool Codegen::HasLiveCallerSavedRegs(Inst *inst)
{
    auto [live_regs, live_fp_regs] = GetLiveRegisters<false>(inst);
    live_regs &= GetCallerRegsMask(GetArch(), false);
    live_fp_regs &= GetCallerRegsMask(GetArch(), true);
    return live_regs.Any() || live_fp_regs.Any();
}

void Codegen::SaveCallerRegisters(RegMask liveRegs, VRegMask liveVregs, bool adjustRegs)
{
    SCOPED_DISASM_STR(this, "Save caller saved regs");
    auto base = GetFrameInfo()->GetCallersRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
    liveRegs &= ~GetEncoder()->GetAvailableScratchRegisters();
    liveVregs &= ~GetEncoder()->GetAvailableScratchFpRegisters();
    if (adjustRegs) {
        liveRegs &= GetRegfile()->GetCallerSavedRegMask();
        liveVregs &= GetRegfile()->GetCallerSavedVRegMask();
    } else {
        liveRegs &= GetCallerRegsMask(GetArch(), false);
        liveVregs &= GetCallerRegsMask(GetArch(), true);
    }
    if (GetFrameInfo()->GetPushCallers()) {
        GetEncoder()->PushRegisters(liveRegs, liveVregs);
    } else {
        GetEncoder()->SaveRegisters(liveRegs, false, GetFrameInfo()->GetCallersOffset(), base,
                                    GetCallerRegsMask(GetArch(), false));
        GetEncoder()->SaveRegisters(liveVregs, true, GetFrameInfo()->GetFpCallersOffset(), base,
                                    GetFrameInfo()->GetPositionedCallers() ? GetCallerRegsMask(GetArch(), true)
                                                                           : RegMask());
    }
}

void Codegen::LoadCallerRegisters(RegMask liveRegs, VRegMask liveVregs, bool adjustRegs)
{
    SCOPED_DISASM_STR(this, "Restore caller saved regs");
    auto base = GetFrameInfo()->GetCallersRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
    liveRegs &= ~GetEncoder()->GetAvailableScratchRegisters();
    liveVregs &= ~GetEncoder()->GetAvailableScratchFpRegisters();
    if (adjustRegs) {
        liveRegs &= GetRegfile()->GetCallerSavedRegMask();
        liveVregs &= GetRegfile()->GetCallerSavedVRegMask();
    } else {
        liveRegs &= GetCallerRegsMask(GetArch(), false);
        liveVregs &= GetCallerRegsMask(GetArch(), true);
    }
    if (GetFrameInfo()->GetPushCallers()) {
        GetEncoder()->PopRegisters(liveRegs, liveVregs);
    } else {
        GetEncoder()->LoadRegisters(liveRegs, false, GetFrameInfo()->GetCallersOffset(), base,
                                    GetCallerRegsMask(GetArch(), false));
        GetEncoder()->LoadRegisters(liveVregs, true, GetFrameInfo()->GetFpCallersOffset(), base,
                                    GetCallerRegsMask(GetArch(), true));
    }
}

bool Codegen::RegisterKeepCallArgument(CallInst *callInst, Reg reg)
{
    for (auto i = 0U; i < callInst->GetInputsCount(); i++) {
        auto location = callInst->GetLocation(i);
        if (location.IsRegister() && location.GetValue() == reg.GetId()) {
            return true;
        }
    }
    return false;
}

void Codegen::LoadMethod(Reg dst)
{
    ASSERT((CFrameMethod::GetOffsetFromSpInBytes(GetFrameLayout()) -
            (GetFrameLayout().GetMethodOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::BYTES>())) ==
           0);
    auto spOffset = CFrameMethod::GetOffsetFromSpInBytes(GetFrameLayout());
    auto mem = MemRef(SpReg(), spOffset);
    GetEncoder()->EncodeLdr(dst, false, mem);
}

void Codegen::StoreFreeSlot(Reg src)
{
    ASSERT(src.GetSize() <= (GetFrameLayout().GetSlotSize() << 3U));
    auto spOffset =
        GetFrameLayout().GetFreeSlotOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::BYTES>();
    auto mem = MemRef(SpReg(), spOffset);
    GetEncoder()->EncodeStr(src, mem);
}

void Codegen::LoadFreeSlot(Reg dst)
{
    ASSERT(dst.GetSize() <= (GetFrameLayout().GetSlotSize() << 3U));
    auto spOffset =
        GetFrameLayout().GetFreeSlotOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::BYTES>();
    auto mem = MemRef(SpReg(), spOffset);
    GetEncoder()->EncodeLdr(dst, false, mem);
}

void Codegen::CreateReturn(const Inst *inst)
{
    if (GetGraph()->GetMethodProperties().GetLastReturn() == inst) {
        GetEncoder()->BindLabel(GetLabelExit());
        GenerateEpilogue();
    } else {
        GetEncoder()->EncodeJump(GetLabelExit());
    }
}

void Codegen::EncodeDynamicCast(Inst *inst, Reg dst, bool dstSigned, Reg src)
{
    CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
    CHECK_GE(dst.GetSize(), BITS_PER_UINT32);

    bool isDst64 {dst.GetSize() == BITS_PER_UINT64};
    dst = dst.As(INT32_TYPE);

    auto enc {GetEncoder()};
    if (g_options.IsCpuFeatureEnabled(CpuFeature::JSCVT)) {
        // no slow path intended
        enc->EncodeFastPathDynamicCast(dst, src, LabelHolder::INVALID_LABEL);
    } else {
        auto slowPath {CreateSlowPath<SlowPathJsCastDoubleToInt32>(inst)};
        slowPath->SetDstReg(dst);
        slowPath->SetSrcReg(src);

        enc->EncodeFastPathDynamicCast(dst, src, slowPath->GetLabel());
        slowPath->BindBackLabel(enc);
    }

    if (isDst64) {
        enc->EncodeCast(dst.As(INT64_TYPE), dstSigned, dst, dstSigned);
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UnaryOperation(opc)                                                       \
    void EncodeVisitor::Visit##opc(GraphVisitor *visitor, Inst *inst)             \
    {                                                                             \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);               \
        auto type = inst->GetType();                                              \
        auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);   \
        auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type); \
        enc->GetEncoder()->Encode##opc(dst, src0);                                \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperation(opc)                                                      \
    void EncodeVisitor::Visit##opc(GraphVisitor *visitor, Inst *inst)             \
    {                                                                             \
        auto type = inst->GetType();                                              \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);               \
        auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);   \
        auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type); \
        auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type); \
        enc->GetEncoder()->Encode##opc(dst, src0, src1);                          \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryShiftedRegisterOperation(opc)                                                     \
    void EncodeVisitor::Visit##opc##SR(GraphVisitor *visitor, Inst *inst)                       \
    {                                                                                           \
        auto type = inst->GetType();                                                            \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);                             \
        auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                 \
        auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);               \
        auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);               \
        auto imm_shift_inst = static_cast<BinaryShiftedRegisterOperation *>(inst);              \
        auto imm_value = static_cast<uint32_t>(imm_shift_inst->GetImm()) & (dst.GetSize() - 1); \
        auto shift = Shift(src1, imm_shift_inst->GetShiftType(), imm_value);                    \
        enc->GetEncoder()->Encode##opc(dst, src0, shift);                                       \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, TYPE) TYPE(OPCODE)

ENCODE_MATH_LIST(INST_DEF)
ENCODE_INST_WITH_SHIFTED_OPERAND(INST_DEF)

#undef UnaryOperation
#undef BinaryOperation
#undef BinaryShiftedRegisterOperation
#undef INST_DEF

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryImmOperation(opc)                                                   \
    void EncodeVisitor::Visit##opc##I(GraphVisitor *visitor, Inst *inst)          \
    {                                                                             \
        auto binop = inst->CastTo##opc##I();                                      \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);               \
        auto type = inst->GetType();                                              \
        auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);   \
        auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type); \
        enc->GetEncoder()->Encode##opc(dst, src0, Imm(binop->GetImm()));          \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARRY_IMM_OPS(DEF) DEF(Add) DEF(Sub) DEF(Shl) DEF(AShr) DEF(And) DEF(Or) DEF(Xor)

BINARRY_IMM_OPS(BinaryImmOperation)

#undef BINARRY_IMM_OPS
#undef BinaryImmOperation

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinarySignUnsignOperation(opc)                                            \
    void EncodeVisitor::Visit##opc(GraphVisitor *visitor, Inst *inst)             \
    {                                                                             \
        auto type = inst->GetType();                                              \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);               \
        auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);   \
        auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type); \
        auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type); \
        auto arch = enc->GetCodegen()->GetArch();                                 \
        if (!Codegen::InstEncodedWithLibCall(inst, arch)) {                       \
            enc->GetEncoder()->Encode##opc(dst, IsTypeSigned(type), src0, src1);  \
            return;                                                               \
        }                                                                         \
        ASSERT(arch == Arch::AARCH32);                                            \
        if (enc->cg_->GetGraph()->IsAotMode()) {                                  \
            enc->GetEncoder()->SetFalseResult();                                  \
            return;                                                               \
        }                                                                         \
        auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst); \
        enc->GetEncoder()->SetRegister(&live_regs, &live_vregs, dst, false);      \
        enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);      \
        enc->GetEncoder()->Encode##opc(dst, IsTypeSigned(type), src0, src1);      \
        enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);      \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SIGN_UNSIGN_OPS(DEF) DEF(Div) DEF(Min) DEF(Max)

SIGN_UNSIGN_OPS(BinarySignUnsignOperation)

#undef SIGN_UNSIGN_OPS
#undef BINARY_SIGN_UNSIGN_OPERATION

void EncodeVisitor::VisitMod(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto arch = enc->GetCodegen()->GetArch();
    if (!Codegen::InstEncodedWithLibCall(inst, arch)) {
        enc->GetEncoder()->EncodeMod(dst, IsTypeSigned(type), src0, src1);
        return;
    }

    if (DataType::IsFloatType(type)) {
        RuntimeInterface::IntrinsicId entry =
            ((type == DataType::FLOAT32) ? RuntimeInterface::IntrinsicId::LIB_CALL_FMODF
                                         : RuntimeInterface::IntrinsicId::LIB_CALL_FMOD);
        auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst);
        enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);

        enc->GetCodegen()->FillCallParams(src0, src1);
        enc->GetCodegen()->CallIntrinsic(inst, entry);

        auto retVal = enc->GetCodegen()->GetTarget().GetReturnFpReg();
        if (retVal.GetType().IsFloat()) {
            // ret_val is FLOAT64 for Arm64, AMD64 and ARM32 HardFP, but dst can be FLOAT32
            // so we convert ret_val to FLOAT32
            enc->GetEncoder()->EncodeMov(dst, Reg(retVal.GetId(), dst.GetType()));
        } else {
            // case for ARM32 SoftFP
            enc->GetEncoder()->EncodeMov(dst,
                                         Reg(retVal.GetId(), dst.GetSize() == WORD_SIZE ? INT32_TYPE : INT64_TYPE));
        }

        enc->GetEncoder()->SetRegister(&live_regs, &live_vregs, dst, false);
        enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
        return;
    }

    ASSERT(arch == Arch::AARCH32);
    // Fix after supporting AOT mode for arm32
    if (enc->cg_->GetGraph()->IsAotMode()) {
        enc->GetEncoder()->SetFalseResult();
        return;
    }
    auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst);
    enc->GetEncoder()->SetRegister(&live_regs, &live_vregs, dst, false);
    enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);
    enc->GetEncoder()->EncodeMod(dst, IsTypeSigned(type), src0, src1);
    enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
}

void EncodeVisitor::VisitShrI(GraphVisitor *visitor, Inst *inst)
{
    auto binop = inst->CastToShrI();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    enc->GetEncoder()->EncodeShr(dst, src0, Imm(binop->GetImm()));
}

void EncodeVisitor::VisitMAdd(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    constexpr int64_t IMM_2 = 2;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), type);
    enc->GetEncoder()->EncodeMAdd(dst, src0, src1, src2);
}

void EncodeVisitor::VisitMSub(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    constexpr int64_t IMM_2 = 2;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), type);
    enc->GetEncoder()->EncodeMSub(dst, src0, src1, src2);
}

void EncodeVisitor::VisitMNeg(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    enc->GetEncoder()->EncodeMNeg(dst, src0, src1);
}

void EncodeVisitor::VisitOrNot(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    enc->GetEncoder()->EncodeOrNot(dst, src0, src1);
}

void EncodeVisitor::VisitAndNot(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    enc->GetEncoder()->EncodeAndNot(dst, src0, src1);
}

void EncodeVisitor::VisitXorNot(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    enc->GetEncoder()->EncodeXorNot(dst, src0, src1);
}

void EncodeVisitor::VisitNegSR(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto immShiftInst = static_cast<UnaryShiftedRegisterOperation *>(inst);
    enc->GetEncoder()->EncodeNeg(dst, Shift(src, immShiftInst->GetShiftType(), immShiftInst->GetImm()));
}

void EncodeVisitor::VisitCast(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto srcType = inst->GetInputType(0);
    auto dstType = inst->GetType();

    ASSERT(dstType != DataType::ANY);

    bool srcSigned = IsTypeSigned(srcType);
    bool dstSigned = IsTypeSigned(dstType);

    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), srcType);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), dstType);
    if (dstType == DataType::BOOL) {
        enc->GetEncoder()->EncodeCastToBool(dst, src);
        return;
    }

    if (inst->CastToCast()->IsDynamicCast()) {
        enc->GetCodegen()->EncodeDynamicCast(inst, dst, dstSigned, src);
        return;
    }

    auto arch = enc->GetCodegen()->GetArch();
    if (!Codegen::InstEncodedWithLibCall(inst, arch)) {
        enc->GetEncoder()->EncodeCast(dst, dstSigned, src, srcSigned);
        return;
    }
    ASSERT(arch == Arch::AARCH32);
    // Fix after supporting AOT mode for arm32
    if (enc->cg_->GetGraph()->IsAotMode()) {
        enc->GetEncoder()->SetFalseResult();
        return;
    }
    auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst);
    enc->GetEncoder()->SetRegister(&live_regs, &live_vregs, dst, false);
    enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);
    enc->GetEncoder()->EncodeCast(dst, dstSigned, src, srcSigned);
    enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
}

void EncodeVisitor::VisitBitcast(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto srcType = inst->GetInputType(0);
    auto dstType = inst->GetType();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), dstType);
    auto src = codegen->ConvertRegister(inst->GetSrcReg(0), srcType);
    enc->GetEncoder()->EncodeMov(dst, src);
}

void EncodeVisitor::VisitPhi([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst) {}

void EncodeVisitor::VisitConstant(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetDstReg() == INVALID_REG) {
        return;
    }
    if (inst->GetDstReg() == enc->cg_->GetGraph()->GetZeroReg()) {
        ASSERT(IsZeroConstant(inst));
        ASSERT(enc->GetRegfile()->GetZeroReg() != INVALID_REGISTER);
        return;
    }
    auto *constInst = inst->CastToConstant();
    auto type = inst->GetType();
    if (enc->cg_->GetGraph()->IsDynamicMethod() && type == DataType::INT64) {
        type = DataType::INT32;
    }

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
#ifndef NDEBUG
    switch (type) {
        case DataType::FLOAT32:
            enc->GetEncoder()->EncodeMov(dst, Imm(constInst->GetFloatValue()));
            break;
        case DataType::FLOAT64:
            enc->GetEncoder()->EncodeMov(dst, Imm(constInst->GetDoubleValue()));
            break;
        default:
            enc->GetEncoder()->EncodeMov(dst, Imm(constInst->GetRawValue()));
    }
#else
    enc->GetEncoder()->EncodeMov(dst, Imm(constInst->GetRawValue()));
#endif
}

void EncodeVisitor::VisitNullPtr(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetDstReg() == enc->cg_->GetGraph()->GetZeroReg()) {
        ASSERT_PRINT(enc->GetRegfile()->GetZeroReg() != INVALID_REGISTER,
                     "NullPtr doesn't have correct destination register");
        return;
    }
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitLoadUndefined(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto runtime = enc->GetCodegen()->GetGraph()->GetRuntime();
    auto graph = enc->GetCodegen()->GetGraph();
    if (graph->IsJitOrOsrMode()) {
        enc->GetEncoder()->EncodeMov(dst, Imm(runtime->GetUndefinedObject()));
    } else {
        auto ref = MemRef(enc->GetCodegen()->ThreadReg(), runtime->GetTlsUndefinedObjectOffset(graph->GetArch()));
        enc->GetEncoder()->EncodeLdr(dst, false, ref);
    }
}

// Next visitors use calling convention
void Codegen::VisitCallIndirect(CallIndirectInst *inst)
{
    auto location = inst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());
    auto src = Reg(location.GetValue(), GetTarget().GetPtrRegType());
    auto dst = ConvertRegister(inst->GetDstReg(), inst->GetType());

    GetEncoder()->MakeCall(src);
    if (inst->HasUsers()) {
        GetEncoder()->EncodeMov(dst, GetTarget().GetReturnReg(dst.GetType()));
    }
}

void Codegen::VisitCall(CallInst *inst)
{
    ASSERT(GetGraph()->GetRelocationHandler() != nullptr);
    ASSERT(!HasLiveCallerSavedRegs(inst));

    RelocationInfo relocation;
    relocation.data = inst->GetCallMethodId();
    GetEncoder()->MakeCall(&relocation);
    GetGraph()->GetRelocationHandler()->AddRelocation(relocation);

    if (inst->HasUsers()) {
        auto dstReg = ConvertRegister(inst->GetDstReg(), inst->GetType());
        ASSERT(dstReg.IsValid());
        GetEncoder()->EncodeMov(dstReg, GetTarget().GetReturnReg(dstReg.GetType()));
    }
}

void EncodeVisitor::VisitCallIndirect(GraphVisitor *visitor, Inst *inst)
{
    static_cast<EncodeVisitor *>(visitor)->GetCodegen()->VisitCallIndirect(inst->CastToCallIndirect());
}

void EncodeVisitor::VisitCall(GraphVisitor *visitor, Inst *inst)
{
    static_cast<EncodeVisitor *>(visitor)->GetCodegen()->VisitCall(inst->CastToCall());
}

void EncodeVisitor::VisitCompare(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);

    auto type = inst->CastToCompare()->GetOperandsType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto cc = enc->GetCodegen()->ConvertCc(inst->CastToCompare()->GetCc());
    if (IsTestCc(cc)) {
        enc->GetEncoder()->EncodeCompareTest(dst, src0, src1, cc);
    } else {
        enc->GetEncoder()->EncodeCompare(dst, src0, src1, cc);
    }
}

void EncodeVisitor::VisitCmp(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);

    auto cmpInst = inst->CastToCmp();
    auto type = cmpInst->GetOperandsType();

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);

    Condition cc;
    if (DataType::IsFloatType(type)) {
        // check whether MI is fully correct here
        cc = cmpInst->IsFcmpg() ? (Condition::MI) : (Condition::LT);
    } else if (IsTypeSigned(type)) {
        cc = Condition::LT;
    } else {
        cc = Condition::LO;
    }
    enc->GetEncoder()->EncodeCmp(dst, src0, src1, cc);
}

void EncodeVisitor::VisitReturnVoid(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }

    enc->GetCodegen()->CreateReturn(inst);
}

void EncodeVisitor::VisitReturn(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), inst->GetType());
    enc->GetEncoder()->EncodeMov(enc->GetCodegen()->GetTarget().GetReturnReg(src.GetType()), src);

    enc->GetCodegen()->CreateReturn(inst);
}

void EncodeVisitor::VisitReturnI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto rzero = enc->GetRegfile()->GetZeroReg();
    auto immVal = inst->CastToReturnI()->GetImm();
    Imm imm = codegen->ConvertImmWithExtend(immVal, inst->GetType());

    auto returnReg = codegen->GetTarget().GetReturnReg(codegen->ConvertDataType(inst->GetType(), codegen->GetArch()));

    if (immVal == 0 && codegen->GetTarget().SupportZeroReg() && !DataType::IsFloatType(inst->GetType())) {
        enc->GetEncoder()->EncodeMov(returnReg, rzero);
    } else {
        enc->GetEncoder()->EncodeMov(returnReg, imm);
    }

    enc->GetCodegen()->CreateReturn(inst);
}

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
static CallInst *GetCallInstFromReturnInlined(Inst *return_inlined)
{
    auto ss = return_inlined->GetSaveState();
    for (auto &user : ss->GetUsers()) {
        auto inst = user.GetInst();
        if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
            return static_cast<CallInst *>(inst);
        }
    }
    return nullptr;
}
#endif

void EncodeVisitor::VisitReturnInlined(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
    if (!enc->cg_->GetGraph()->IsAotMode()) {
        auto callInst = GetCallInstFromReturnInlined(inst->CastToReturnInlined());
        ASSERT(callInst != nullptr);
        static_cast<EncodeVisitor *>(visitor)->GetCodegen()->InsertTrace(
            {Imm(static_cast<size_t>(TraceId::METHOD_EXIT)), Imm(reinterpret_cast<size_t>(callInst->GetCallMethod())),
             Imm(static_cast<size_t>(events::MethodExitKind::INLINED))});
    }
#endif
}

void EncodeVisitor::VisitLoadConstArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto method = inst->CastToLoadConstArray()->GetMethod();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto arrayType = inst->CastToLoadConstArray()->GetTypeId();

    enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::RESOLVE_LITERAL_ARRAY, dst,
                                             TypedImm(arrayType));
}

void EncodeVisitor::VisitFillConstArray(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    ASSERT(type != DataType::REFERENCE);
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto arrayType = inst->CastToFillConstArray()->GetTypeId();
    auto arch = enc->cg_->GetGraph()->GetArch();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto method = inst->CastToFillConstArray()->GetMethod();
    auto offset = runtime->GetArrayDataOffset(enc->GetCodegen()->GetArch());
    auto arraySize = inst->CastToFillConstArray()->GetImm() << DataType::ShiftByType(type, arch);

    auto arrayReg = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::GetIntTypeForReference(arch));
    encoder->EncodeAdd(arrayReg, src, Imm(offset));

    ASSERT(arraySize != 0);

    if (enc->cg_->GetGraph()->IsAotMode()) {
        auto arrOffset = runtime->GetOffsetToConstArrayData(method, arrayType);
        auto pfOffset = runtime->GetPandaFileOffset(arch);
        ScopedTmpReg methodReg(encoder);
        enc->GetCodegen()->LoadMethod(methodReg);
        // load pointer to panda file
        encoder->EncodeLdr(methodReg, false, MemRef(methodReg, pfOffset));
        // load pointer to binary file
        encoder->EncodeLdr(methodReg, false, MemRef(methodReg, runtime->GetBinaryFileBaseOffset(enc->GetArch())));
        // Get pointer to array data
        encoder->EncodeAdd(methodReg, methodReg, Imm(arrOffset));
        // call memcpy
        RuntimeInterface::IntrinsicId entry = RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY;
        auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst);
        enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);

        enc->GetCodegen()->FillCallParams(arrayReg, methodReg, TypedImm(arraySize));
        enc->GetCodegen()->CallIntrinsic(inst, entry);
        enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
    } else {
        auto data = runtime->GetPointerToConstArrayData(method, arrayType);
        // call memcpy
        RuntimeInterface::IntrinsicId entry = RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY;
        auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst);
        enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);

        enc->GetCodegen()->FillCallParams(arrayReg, TypedImm(data), TypedImm(arraySize));
        enc->GetCodegen()->CallIntrinsic(inst, entry);
        enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
    }
}

static void GetEntryPointId(uint64_t elementSize, RuntimeInterface::EntrypointId &eid)
{
    switch (elementSize) {
        case sizeof(uint8_t):
            eid = RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB8;
            break;
        case sizeof(uint16_t):
            eid = RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB16;
            break;
        case sizeof(uint32_t):
            eid = RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB32;
            break;
        case sizeof(uint64_t):
            eid = RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB64;
            break;
        default:
            UNREACHABLE();
    }
}

void Codegen::VisitNewArray(Inst *inst)
{
    auto method = inst->CastToNewArray()->GetMethod();
    auto dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto srcClass = ConvertRegister(inst->GetSrcReg(NewArrayInst::INDEX_CLASS), DataType::POINTER);
    auto srcSize = ConvertRegister(inst->GetSrcReg(NewArrayInst::INDEX_SIZE), DataType::Type::INT32);
    auto arrayType = inst->CastToNewArray()->GetTypeId();
    auto runtime = GetGraph()->GetRuntime();
    auto encoder = GetEncoder();

    auto maxTlabSize = runtime->GetTLABMaxSize();
    // NOTE(msherstennikov): support NewArray fast path for arm32
    if (maxTlabSize == 0 || GetArch() == Arch::AARCH32) {
        CallRuntime(inst, EntrypointId::CREATE_ARRAY, dst, RegMask::GetZeroMask(), srcClass, srcSize);
        if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
            encoder->EncodeMemoryBarrier(memory_order::RELEASE);
        }
        return;
    }

    auto lenInst = inst->GetDataFlowInput(0);
    auto classArraySize = runtime->GetClassArraySize(GetArch());
    uint64_t arraySize = 0;
    uint64_t elementSize = runtime->GetArrayElementSize(method, arrayType);
    uint64_t alignment = runtime->GetTLABAlignment();
    if (lenInst->GetOpcode() == Opcode::Constant) {
        ASSERT(lenInst->GetType() == DataType::INT64);
        arraySize = lenInst->CastToConstant()->GetIntValue() * elementSize + classArraySize;
        arraySize = (arraySize & ~(alignment - 1U)) + ((arraySize % alignment) != 0U ? alignment : 0U);
        if (arraySize > maxTlabSize) {
            CallRuntime(inst, EntrypointId::CREATE_ARRAY, dst, RegMask::GetZeroMask(), srcClass, srcSize);
            if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
                encoder->EncodeMemoryBarrier(memory_order::RELEASE);
            }
            return;
        }
    }

    EntrypointId eid;
    GetEntryPointId(elementSize, eid);
    CallFastPath(inst, eid, dst, RegMask::GetZeroMask(), srcClass, srcSize);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        encoder->EncodeMemoryBarrier(memory_order::RELEASE);
    }
}

void EncodeVisitor::VisitNewArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    return enc->GetCodegen()->VisitNewArray(inst);
}

void EncodeVisitor::VisitParameter(GraphVisitor *visitor, Inst *inst)
{
    /*
        Default register parameters pushed in ir_builder
        In regalloc filled spill/fill parameters part.
    */
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto paramInst = inst->CastToParameter();
    auto sf = paramInst->GetLocationData();
    if (sf.GetSrc() == sf.GetDst()) {
        return;
    }

    auto tmpSf = codegen->GetGraph()->CreateInstSpillFill();
    tmpSf->AddSpillFill(sf);
    SpillFillEncoder(codegen, tmpSf).EncodeSpillFill();
}

void EncodeVisitor::VisitStoreArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto array = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto index = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);
    constexpr int64_t IMM_2 = 2;
    auto storedValue = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), inst->GetType());
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch());
    auto scale = DataType::ShiftByType(inst->GetType(), enc->GetCodegen()->GetArch());

    auto memRef = [enc, inst, array, index, offset, scale]() {
        if (inst->CastToStoreArray()->GetNeedBarrier()) {
            auto tmpOffset =
                enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::GetIntTypeForReference(enc->GetArch()));
            enc->GetEncoder()->EncodeShl(tmpOffset, index, Imm(scale));
            enc->GetEncoder()->EncodeAdd(tmpOffset, tmpOffset, Imm(offset));
            return MemRef(array, tmpOffset, 0);
        }

        auto tmp = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::REFERENCE);
        enc->GetEncoder()->EncodeAdd(tmp, array, Imm(offset));
        return MemRef(tmp, index, scale);
    };

    auto mem = memRef();
    if (inst->CastToStoreArray()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(array.GetId(), storedValue.GetId()));
    }
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStr(storedValue, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreArray()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, storedValue, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitSpillFill(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    SpillFillEncoder(codegen, inst).EncodeSpillFill();
}

void EncodeVisitor::VisitSaveState([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    // Nothing to do, SaveState is processed in its users.
}

void EncodeVisitor::VisitSaveStateDeoptimize([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    // Nothing to do, SaveStateDeoptimize is processed in its users.
}

void EncodeVisitor::VisitSaveStateOsr(GraphVisitor *visitor, Inst *inst)
{
    static_cast<EncodeVisitor *>(visitor)->GetCodegen()->CreateOsrEntry(inst->CastToSaveStateOsr());
}

void EncodeVisitor::VisitLoadArray(GraphVisitor *visitor, Inst *inst)
{
    auto instLoadArray = inst->CastToLoadArray();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    ASSERT(instLoadArray->IsArray() || !runtime->IsCompressedStringsEnabled());
    if (static_cast<LoadInst *>(inst)->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);      // index
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // load value
    auto offset = instLoadArray->IsArray() ? runtime->GetArrayDataOffset(enc->GetCodegen()->GetArch())
                                           : runtime->GetStringDataOffset(enc->GetArch());
    auto encoder = enc->GetEncoder();
    auto arch = encoder->GetArch();
    auto shift = DataType::ShiftByType(type, arch);
    ScopedTmpReg scopedTmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto tmp = scopedTmp.GetReg();
    encoder->EncodeAdd(tmp, src0, Imm(offset));
    auto mem = MemRef(tmp, src1, shift);
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    encoder->EncodeLdr(dst, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitLoadCompressedStringChar(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);      // index
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(2), DataType::INT32);      // length
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // load value
    auto offset = runtime->GetStringDataOffset(enc->GetArch());
    auto encoder = enc->GetEncoder();
    auto arch = encoder->GetArch();
    auto shift = DataType::ShiftByType(type, arch);
    ScopedTmpReg scopedTmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto tmp = scopedTmp.GetReg();

    ASSERT(encoder->CanEncodeCompressedStringCharAt());
    auto mask = runtime->GetStringCompressionMask();
    if (mask != 1) {
        UNREACHABLE();  // mask is hardcoded in JCL, but verify it just in case it's changed
    }
    enc->GetEncoder()->EncodeCompressedStringCharAt(dst, src0, src1, src2, tmp, offset, shift);
}

void EncodeVisitor::VisitLenArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // len array

    auto lenArrayInst = inst->CastToLenArray();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    int64_t offset = lenArrayInst->IsArray() ? runtime->GetArrayLengthOffset(enc->GetCodegen()->GetArch())
                                             : runtime->GetStringLengthOffset(enc->GetArch());
    auto mem = MemRef(src0, offset);

    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitBuiltin([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    UNREACHABLE();
}

void EncodeVisitor::VisitNullCheck(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToNullCheck()->IsImplicit()) {
        return;
    }
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->template CreateUnaryCheck<SlowPathImplicitNullCheck>(inst, EntrypointId::NULL_POINTER_EXCEPTION,
                                                                            DeoptimizeType::NULL_CHECK, Condition::EQ);
}

void EncodeVisitor::VisitBoundsCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto lenType = inst->GetInputType(0);
    auto len = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), lenType);
    auto indexType = inst->GetInputType(1);
    auto index = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), indexType);
    [[maybe_unused]] constexpr int64_t IMM_2 = 2;
    ASSERT(inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveState ||
           inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveStateDeoptimize);

    EntrypointId entrypoint = inst->CastToBoundsCheck()->IsArray() ? EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION
                                                                   : EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;

    LabelHolder::LabelId label;
    if (inst->CanDeoptimize()) {
        DeoptimizeType type = DeoptimizeType::BOUNDS_CHECK_WITH_DEOPT;
        label = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, type)->GetLabel();
    } else {
        label = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, entrypoint)->GetLabel();
    }
    enc->GetEncoder()->EncodeJump(label, index, len, Condition::HS);
}

void EncodeVisitor::VisitRefTypeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto arrayReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto refReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    [[maybe_unused]] constexpr int64_t IMM_2 = 2;
    ASSERT(inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveState);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();

    auto slowPath =
        enc->GetCodegen()->CreateSlowPath<SlowPathRefCheck>(inst, EntrypointId::CHECK_STORE_ARRAY_REFERENCE);

    slowPath->SetRegs(arrayReg, refReg);
    slowPath->CreateBackLabel(encoder);

    // We don't check if stored object is nullptr
    encoder->EncodeJump(slowPath->GetBackLabel(), refReg, Condition::EQ);

    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, enc->GetCodegen()->GetArch()));
    ScopedTmpReg tmpReg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, enc->GetCodegen()->GetArch()));

    // Get Class from array
    enc->GetCodegen()->LoadClassFromObject(tmpReg, arrayReg);

    // Get element type Class from array class
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));

    // Get Class from stored object
    enc->GetCodegen()->LoadClassFromObject(tmpReg1, refReg);

    // If the object's and array element's types match we do not check further
    encoder->EncodeJump(slowPath->GetBackLabel(), tmpReg, tmpReg1, Condition::EQ);

    // If the array's element class is not Object (baseclass == null)
    // we call CheckStoreArrayReference, otherwise we fall through
    encoder->EncodeLdr(tmpReg1, false, MemRef(tmpReg, runtime->GetClassBaseOffset(enc->GetArch())));
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg1, Condition::NE);

    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::VisitZeroCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->template CreateUnaryCheck<SlowPathCheck>(inst, EntrypointId::ARITHMETIC_EXCEPTION,
                                                                DeoptimizeType::ZERO_CHECK, Condition::EQ);
}

void EncodeVisitor::VisitNegativeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->template CreateUnaryCheck<SlowPathCheck>(inst, EntrypointId::NEGATIVE_ARRAY_SIZE_EXCEPTION,
                                                                DeoptimizeType::NEGATIVE_CHECK, Condition::LT);
}

void EncodeVisitor::VisitNotPositiveCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto srcType = inst->GetInputType(0);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), srcType);
    ASSERT(inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveState ||
           inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveStateDeoptimize);

    ASSERT(inst->CanDeoptimize());
    DeoptimizeType type = DeoptimizeType::NEGATIVE_CHECK;
    auto label = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, type)->GetLabel();
    enc->GetEncoder()->EncodeJump(label, src, Condition::LE);
}

void EncodeVisitor::VisitDeoptimizeIf(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto srcType = inst->GetInput(0).GetInst()->GetType();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), srcType);

    auto slowPath =
        enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, inst->CastToDeoptimizeIf()->GetDeoptimizeType());

    // create jump to slow path if src is true
    enc->GetEncoder()->EncodeJump(slowPath->GetLabel(), src, Condition::NE);
}

void EncodeVisitor::VisitDeoptimizeCompare(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto deopt = inst->CastToDeoptimizeCompare();
    ASSERT(deopt->GetOperandsType() != DataType::NO_TYPE);
    auto src0 = enc->GetCodegen()->ConvertRegister(deopt->GetSrcReg(0), deopt->GetOperandsType());
    auto src1 = enc->GetCodegen()->ConvertRegister(deopt->GetSrcReg(1), deopt->GetOperandsType());
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(
        inst, inst->CastToDeoptimizeCompare()->GetDeoptimizeType());
    enc->GetEncoder()->EncodeJump(slowPath->GetLabel(), src0, src1, enc->GetCodegen()->ConvertCc(deopt->GetCc()));
}

void EncodeVisitor::VisitDeoptimizeCompareImm(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto deopt = inst->CastToDeoptimizeCompareImm();
    ASSERT(deopt->GetOperandsType() != DataType::NO_TYPE);
    auto cc = deopt->GetCc();
    auto src0 = enc->GetCodegen()->ConvertRegister(deopt->GetSrcReg(0), deopt->GetOperandsType());
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(
        inst, inst->CastToDeoptimizeCompareImm()->GetDeoptimizeType());

    if (deopt->GetImm() == 0) {
        Arch arch = enc->GetCodegen()->GetArch();
        DataType::Type type = deopt->GetInput(0).GetInst()->GetType();
        ASSERT(!IsFloatType(type));
        if (IsTypeSigned(type) && (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_GE)) {
            auto signBit = GetTypeSize(type, arch) - 1;
            if (cc == ConditionCode::CC_LT) {
                // x < 0
                encoder->EncodeBitTestAndBranch(slowPath->GetLabel(), src0, signBit, true);
                return;
            }
            if (cc == ConditionCode::CC_GE) {
                // x >= 0
                encoder->EncodeBitTestAndBranch(slowPath->GetLabel(), src0, signBit, false);
                return;
            }
        }
        if (enc->GetCodegen()->GetTarget().SupportZeroReg()) {
            auto zreg = enc->GetRegfile()->GetZeroReg();
            encoder->EncodeJump(slowPath->GetLabel(), src0, zreg, enc->GetCodegen()->ConvertCc(cc));
        } else {
            encoder->EncodeJump(slowPath->GetLabel(), src0, Imm(0), enc->GetCodegen()->ConvertCc(cc));
        }
        return;
    }
    encoder->EncodeJump(slowPath->GetLabel(), src0, Imm(deopt->GetImm()), enc->GetCodegen()->ConvertCc(cc));
}

void EncodeVisitor::VisitLoadString(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto method = inst->CastToLoadString()->GetMethod();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto stringType = inst->CastToLoadString()->GetTypeId();
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    ASSERT(inst->IsRuntimeCall());

    // Static constructor invoked only once, so there is no sense in replacing regular
    // ResolveString runtime call with optimized version that will only slow down constructor's execution.
    auto isCctor = graph->GetRuntime()->IsMethodStaticConstructor(method);

    if (graph->IsAotMode() && g_options.IsCompilerAotLoadStringPlt() && !isCctor) {
        auto aotData = graph->GetAotData();
        intptr_t slotOffset = aotData->GetStringSlotOffset(encoder->GetCursorOffset(), stringType);
        ScopedTmpRegU64 addrReg(encoder);
        ScopedTmpRegU64 tmpDst(encoder);
        encoder->MakeLoadAotTableAddr(slotOffset, addrReg, tmpDst);

        auto slowPath =
            enc->GetCodegen()->CreateSlowPath<SlowPathResolveStringAot>(inst, EntrypointId::RESOLVE_STRING_AOT);
        slowPath->SetDstReg(dst);
        slowPath->SetAddrReg(addrReg);
        slowPath->SetStringId(stringType);
        slowPath->SetMethod(method);
        encoder->EncodeJump(slowPath->GetLabel(), tmpDst, Imm(RuntimeInterface::RESOLVE_STRING_AOT_COUNTER_LIMIT),
                            Condition::LT);
        encoder->EncodeMov(dst, Reg(tmpDst.GetReg().GetId(), dst.GetType()));
        slowPath->BindBackLabel(encoder);
        return;
    }
    if (!graph->IsAotMode()) {
        ObjectPointerType stringPtr = graph->GetRuntime()->GetNonMovableString(method, stringType);
        if (stringPtr != 0) {
            encoder->EncodeMov(dst, Imm(stringPtr));
            EVENT_JIT_USE_RESOLVED_STRING(graph->GetRuntime()->GetMethodName(method), stringType);
            return;
        }
    }
    enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::RESOLVE_STRING, dst, TypedImm(stringType));
}

void EncodeVisitor::VisitLoadObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadObj = inst->CastToLoadObject();
    if (loadObj->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto type = inst->GetType();
    auto typeInput = inst->GetInput(0).GetInst()->GetType();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), typeInput);  // obj
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);        // load value

    auto graph = enc->cg_->GetGraph();
    auto field = loadObj->GetObjField();
    size_t offset = GetObjectOffset(graph, loadObj->GetObjectType(), field, loadObj->GetTypeId());
    auto mem = MemRef(src, offset);
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    if (loadObj->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitResolveObjectField(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto resolver = inst->CastToResolveObjectField();
    if (resolver->GetNeedBarrier()) {
        // Inserts barriers for GC
    }
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto typeId = resolver->GetTypeId();
    auto method = resolver->GetMethod();
    if (graph->IsAotMode()) {
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::GET_FIELD_OFFSET, dst, TypedImm(typeId));
    } else {
        auto skind = UnresolvedTypesInterface::SlotKind::FIELD;
        auto fieldOffsetAddr = graph->GetRuntime()->GetUnresolvedTypes()->GetTableSlot(method, typeId, skind);
        ScopedTmpReg tmpReg(enc->GetEncoder());
        // load field offset and if it's 0 then call runtime EntrypointId::GET_FIELD_OFFSET
        enc->GetEncoder()->EncodeMov(tmpReg, Imm(fieldOffsetAddr));
        enc->GetEncoder()->EncodeLdr(tmpReg, false, MemRef(tmpReg));
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::GET_FIELD_OFFSET);
        slowPath->SetUnresolvedType(method, typeId);
        slowPath->SetDstReg(tmpReg);
        slowPath->SetSlotAddr(fieldOffsetAddr);
        enc->GetEncoder()->EncodeJump(slowPath->GetLabel(), tmpReg, Condition::EQ);
        slowPath->BindBackLabel(enc->GetEncoder());
        enc->GetEncoder()->EncodeMov(dst, tmpReg);
    }
}

void EncodeVisitor::VisitLoadResolvedObjectField([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto load = inst->CastToLoadResolvedObjectField();
    if (load->GetNeedBarrier()) {
        // Inserts barriers for GC
    }
    auto type = inst->GetType();
    auto obj = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto ofs = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::UINT32);     // field offset
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value
    ScopedTmpReg tmpReg(enc->GetEncoder());
    enc->GetEncoder()->EncodeAdd(tmpReg, obj, ofs);
    enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(tmpReg));
}

void EncodeVisitor::VisitLoad(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadByOffset = inst->CastToLoad();

    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), DataType::UINT32);   // offset
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value

    auto mem = MemRef(src0, src1, loadByOffset->GetScale());

    if (loadByOffset->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
}

void EncodeVisitor::VisitLoadI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadByOffset = inst->CastToLoadI();

    auto type = inst->GetType();
    auto base = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value

    if (loadByOffset->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(base, loadByOffset->GetImm()));
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), MemRef(base, loadByOffset->GetImm()));
    }
}

void EncodeVisitor::VisitStoreI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeInst = inst->CastToStoreI();

    auto base = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), inst->GetType());

    auto mem = MemRef(base, storeInst->GetImm());
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(base.GetId(), src.GetId()));
    }

    if (storeInst->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src, mem);
    }

    if (inst->CastToStoreI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, src, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitStoreObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeObj = inst->CastToStoreObject();
    auto codegen = enc->GetCodegen();
    auto src0 = codegen->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto src1 = codegen->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value

    auto graph = enc->cg_->GetGraph();
    auto field = storeObj->GetObjField();
    size_t offset = GetObjectOffset(graph, storeObj->GetObjectType(), field, storeObj->GetTypeId());
    if (!enc->GetCodegen()->OffsetFitReferenceTypeSize(offset)) {
        // such code should not be executed
        enc->GetEncoder()->EncodeAbort();
        return;
    }
    auto mem = MemRef(src0, offset);
    auto encoder = enc->GetEncoder();
    if (inst->CastToStoreObject()->GetNeedBarrier()) {
        if (storeObj->GetObjectType() == ObjectType::MEM_DYN_CLASS) {
            codegen->CreatePreWRB<true>(inst, mem, MakeMask(src0.GetId(), src1.GetId()));
        } else {
            codegen->CreatePreWRB(inst, mem, MakeMask(src0.GetId(), src1.GetId()));
        }
    }
    auto prevOffset = encoder->GetCursorOffset();
    if (storeObj->GetVolatile()) {
        encoder->EncodeStrRelease(src1, mem);
    } else {
        encoder->EncodeStr(src1, mem);
    }
    codegen->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreObject()->GetNeedBarrier()) {
        ScopedTmpRegLazy tmp(encoder);
        if (storeObj->GetObjectType() == ObjectType::MEM_DYN_CLASS) {
            tmp.AcquireIfInvalid();
            encoder->EncodeLdr(tmp, false, MemRef(src1, graph->GetRuntime()->GetManagedClassOffset(graph->GetArch())));
            src1 = tmp;
        }
        codegen->CreatePostWRB(inst, mem, src1, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitStoreResolvedObjectField(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto store = inst->CastToStoreResolvedObjectField();
    auto obj = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // object
    auto val = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value
    auto ofs = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(2), DataType::UINT32);     // field offset
    auto mem = MemRef(obj, ofs, 0);
    if (store->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }
    // Unknown store, assume it can be volatile
    enc->GetEncoder()->EncodeStrRelease(val, mem);
    if (store->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, val, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitStore(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeByOffset = inst->CastToStore();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), DataType::UINT32);   // offset
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(2U), type);               // store value

    auto mem = MemRef(src0, src1, storeByOffset->GetScale());
    if (inst->CastToStore()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(src0.GetId(), src2.GetId()));
    }

    if (storeByOffset->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src2, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src2, mem);
    }

    if (inst->CastToStore()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, src2, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto classId = inst->CastToInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    ASSERT(inst->IsRuntimeCall());

    if (graph->IsAotMode()) {
        ScopedTmpReg tmpReg(encoder);
        ScopedTmpReg classReg(encoder);

        auto aotData = graph->GetAotData();
        intptr_t offset = aotData->GetClassSlotOffset(encoder->GetCursorOffset(), classId, true);
        encoder->MakeLoadAotTableAddr(offset, tmpReg, classReg);

        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, classReg, Condition::NE);

        // PLT Class Init Resolver has special calling convention:
        // First encoder temporary (tmp_reg) works as parameter and return value (which is unnecessary here)
        CHECK_EQ(tmpReg.GetReg().GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());
        enc->GetCodegen()->CreateJumpToClassResolverPltShared(inst, tmpReg.GetReg(), EntrypointId::CLASS_INIT_RESOLVER);
        encoder->BindLabel(label);
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToInitClass()->GetClass());
        ASSERT(klass != 0);
        if (!runtime->IsClassInitialized(klass)) {
            auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::INITIALIZE_CLASS);

            auto stateOffset = runtime->GetClassStateOffset(enc->GetArch());
            int64_t initValue = runtime->GetClassInitializedValue();
            ScopedTmpReg tmpReg(encoder);
            encoder->EncodeMov(tmpReg, Imm(klass + stateOffset));
            auto tmpI8 = enc->GetCodegen()->ConvertRegister(tmpReg.GetReg().GetId(), DataType::INT8);
            encoder->EncodeLdr(tmpI8, false, MemRef(tmpReg));

            encoder->EncodeJump(slowPath->GetLabel(), tmpI8, Imm(initValue), Condition::NE);

            slowPath->BindBackLabel(encoder);
        }
    }
}

void EncodeVisitor::VisitLoadClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto loadClass = inst->CastToLoadClass();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto typeId = loadClass->GetTypeId();
    ASSERT(inst->IsRuntimeCall());

    if (graph->IsAotMode()) {
        auto methodClassId = graph->GetRuntime()->GetClassIdForMethod(graph->GetMethod());
        if (methodClassId == typeId) {
            auto dstPtr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dstPtr);
            auto mem = MemRef(dstPtr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
            return;
        }
        ScopedTmpReg tmpReg(encoder);
        enc->GetCodegen()->CreateLoadClassFromPLT(inst, tmpReg, dst, typeId);
    } else {  // JIT mode
        auto klass = loadClass->GetClass();
        if (klass == nullptr) {
            FillLoadClassUnresolved(visitor, inst);
        } else {
            encoder->EncodeMov(dst, Imm(reinterpret_cast<uintptr_t>(klass)));
        }
    }
}

void EncodeVisitor::FillLoadClassUnresolved(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto loadClass = inst->CastToLoadClass();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto typeId = loadClass->GetTypeId();
    auto method = loadClass->GetMethod();
    auto utypes = graph->GetRuntime()->GetUnresolvedTypes();
    auto klassAddr = utypes->GetTableSlot(method, typeId, UnresolvedTypesInterface::SlotKind::CLASS);
    Reg dstPtr(dst.GetId(), enc->GetCodegen()->GetPtrRegType());
    encoder->EncodeMov(dstPtr, Imm(klassAddr));
    encoder->EncodeLdr(dst, false, MemRef(dstPtr));
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::RESOLVE_CLASS);
    slowPath->SetUnresolvedType(method, typeId);
    slowPath->SetDstReg(dst);
    slowPath->SetSlotAddr(klassAddr);

    encoder->EncodeJump(slowPath->GetLabel(), dst, Condition::EQ);
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::VisitGetGlobalVarAddress(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto id = inst->CastToGetGlobalVarAddress()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::ANY);
    auto entrypointId = runtime->GetGlobalVarEntrypointId();
    ASSERT(inst->IsRuntimeCall());
    if (graph->IsAotMode()) {
        ScopedTmpReg addr(encoder);
        auto aotData = graph->GetAotData();
        Reg constPool = src;
        ScopedTmpRegLazy tmpReg(encoder);
        if (dst.GetId() == src.GetId()) {
            tmpReg.AcquireIfInvalid();
            constPool = tmpReg.GetReg().As(src.GetType());
            encoder->EncodeMov(constPool, src);
        }
        intptr_t offset = aotData->GetCommonSlotOffset(encoder->GetCursorOffset(), id);
        encoder->MakeLoadAotTableAddr(offset, addr, dst);

        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, dst, Condition::NE);

        enc->GetCodegen()->CallRuntime(inst, entrypointId, dst, RegMask::GetZeroMask(), constPool, TypedImm(id));
        encoder->EncodeStr(dst, MemRef(addr));
        encoder->BindLabel(label);
    } else {
        auto address = inst->CastToGetGlobalVarAddress()->GetAddress();
        if (address == 0) {
            address = runtime->GetGlobalVarAddress(graph->GetMethod(), id);
        }
        if (address == 0) {
            enc->GetCodegen()->CallRuntime(inst, entrypointId, dst, RegMask::GetZeroMask(), src, TypedImm(id));
        } else {
            encoder->EncodeMov(dst, Imm(address));
        }
    }
}

void EncodeVisitor::VisitLoadRuntimeClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    size_t classAddrOffset = codegen->GetGraph()->GetRuntime()->GetTlsPromiseClassPointerOffset(codegen->GetArch());

    auto mem = MemRef(codegen->ThreadReg(), classAddrOffset);
    enc->GetEncoder()->EncodeLdr(dst, false, mem);
}

void EncodeVisitor::VisitLoadAndInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto classId = inst->CastToLoadAndInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    ASSERT(inst->IsRuntimeCall());

    if (graph->IsAotMode()) {
        auto methodClassId = runtime->GetClassIdForMethod(graph->GetMethod());
        if (methodClassId == classId) {
            auto dstPtr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dstPtr);
            auto mem = MemRef(dstPtr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
            return;
        }

        ScopedTmpReg tmpReg(encoder);

        auto aotData = graph->GetAotData();
        intptr_t offset = aotData->GetClassSlotOffset(encoder->GetCursorOffset(), classId, true);
        encoder->MakeLoadAotTableAddr(offset, tmpReg, dst);

        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, dst, Condition::NE);

        // PLT Class Init Resolver has special calling convention:
        // First encoder temporary (tmp_reg) works as parameter and return value
        CHECK_EQ(tmpReg.GetReg().GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());
        enc->GetCodegen()->CreateJumpToClassResolverPltShared(inst, tmpReg.GetReg(), EntrypointId::CLASS_INIT_RESOLVER);

        encoder->EncodeMov(dst, tmpReg);
        encoder->BindLabel(label);
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToLoadAndInitClass()->GetClass());
        encoder->EncodeMov(dst, Imm(klass));

        if (runtime->IsClassInitialized(klass)) {
            return;
        }

        auto methodClass = runtime->GetClass(graph->GetMethod());
        if (methodClass == inst->CastToLoadAndInitClass()->GetClass()) {
            return;
        }

        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::INITIALIZE_CLASS);

        auto stateOffset = runtime->GetClassStateOffset(enc->GetArch());
        int64_t initValue = runtime->GetClassInitializedValue();

        ScopedTmpReg stateReg(encoder, INT8_TYPE);
        encoder->EncodeLdr(stateReg, false, MemRef(dst, stateOffset));

        encoder->EncodeJump(slowPath->GetLabel(), stateReg, Imm(initValue), Condition::NE);

        slowPath->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitUnresolvedLoadAndInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto classId = inst->CastToUnresolvedLoadAndInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    ASSERT(inst->IsRuntimeCall());

    if (graph->IsAotMode()) {
        ScopedTmpReg tmpReg(encoder);

        auto aotData = graph->GetAotData();
        intptr_t offset = aotData->GetClassSlotOffset(encoder->GetCursorOffset(), classId, true);
        encoder->MakeLoadAotTableAddr(offset, tmpReg, dst);

        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, dst, Condition::NE);

        // PLT Class Init Resolver has special calling convention:
        // First encoder temporary (tmp_reg) works as parameter and return value
        CHECK_EQ(tmpReg.GetReg().GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());
        enc->GetCodegen()->CreateJumpToClassResolverPltShared(inst, tmpReg.GetReg(), EntrypointId::CLASS_INIT_RESOLVER);

        encoder->EncodeMov(dst, tmpReg);
        encoder->BindLabel(label);
    } else {  // JIT mode
        auto method = inst->CastToUnresolvedLoadAndInitClass()->GetMethod();
        auto utypes = graph->GetRuntime()->GetUnresolvedTypes();
        auto klassAddr = utypes->GetTableSlot(method, classId, UnresolvedTypesInterface::SlotKind::CLASS);
        Reg dstPtr(dst.GetId(), enc->GetCodegen()->GetPtrRegType());
        encoder->EncodeMov(dstPtr, Imm(klassAddr));
        encoder->EncodeLdr(dst, false, MemRef(dstPtr));

        auto slowPath =
            enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::INITIALIZE_CLASS_BY_ID);
        slowPath->SetUnresolvedType(method, classId);
        slowPath->SetDstReg(dst);
        slowPath->SetSlotAddr(klassAddr);

        encoder->EncodeJump(slowPath->GetLabel(), dst, Condition::EQ);
        slowPath->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitLoadStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadStatic = inst->CastToLoadStatic();
    if (loadStatic->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // load value
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // class

    auto graph = enc->cg_->GetGraph();
    auto field = loadStatic->GetObjField();
    auto offset = graph->GetRuntime()->GetFieldOffset(field);
    auto mem = MemRef(src0, offset);
    if (loadStatic->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
}

void EncodeVisitor::VisitResolveObjectFieldStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto resolver = inst->CastToResolveObjectFieldStatic();
    ASSERT(resolver->GetType() == DataType::REFERENCE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto typeId = resolver->GetTypeId();
    auto method = resolver->GetMethod();
    EntrypointId entrypoint = EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS;  // REFERENCE
    UnresolvedTypesInterface::SlotKind slotKind = UnresolvedTypesInterface::SlotKind::FIELD;

    if (graph->IsAotMode()) {
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, entrypoint, dst, TypedImm(typeId), TypedImm(0));
    } else {
        ScopedTmpReg tmpReg(enc->GetEncoder());
        auto fieldAddr = graph->GetRuntime()->GetUnresolvedTypes()->GetTableSlot(method, typeId, slotKind);
        enc->GetEncoder()->EncodeMov(tmpReg, Imm(fieldAddr));
        enc->GetEncoder()->EncodeLdr(tmpReg, false, MemRef(tmpReg));
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, entrypoint);
        slowPath->SetUnresolvedType(method, typeId);
        slowPath->SetDstReg(tmpReg);
        slowPath->SetSlotAddr(fieldAddr);
        enc->GetEncoder()->EncodeJump(slowPath->GetLabel(), tmpReg, Condition::EQ);
        slowPath->BindBackLabel(enc->GetEncoder());
        enc->GetEncoder()->EncodeMov(dst, tmpReg);
    }
}

void EncodeVisitor::VisitLoadResolvedObjectFieldStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto load = inst->CastToLoadResolvedObjectFieldStatic();
    if (load->GetNeedBarrier()) {
        // Insert barriers for GC
    }
    auto type = inst->GetType();
    auto fieldAddr = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    // Unknown load, assume it can be volatile
    enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(fieldAddr));
}

void EncodeVisitor::VisitStoreStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeStatic = inst->CastToStoreStatic();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // class
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value

    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto field = storeStatic->GetObjField();
    auto offset = runtime->GetFieldOffset(field);
    auto mem = MemRef(src0, offset);

    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(src1.GetId()));
    }
    if (storeStatic->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src1, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src1, mem);
    }
    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        auto arch = enc->GetEncoder()->GetArch();
        auto tmpReg = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::GetIntTypeForReference(arch));
        enc->GetEncoder()->EncodeLdr(tmpReg, false, MemRef(src0, runtime->GetManagedClassOffset(enc->GetArch())));
        auto classHeaderMem = MemRef(tmpReg);
        enc->GetCodegen()->CreatePostWRB(inst, classHeaderMem, src1, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitLoadObjectDynamic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto *codegen = enc->GetCodegen();
    codegen->CreateLoadObjectDynamic(inst);
}

void EncodeVisitor::VisitStoreObjectDynamic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto *codegen = enc->GetCodegen();
    codegen->CreateStoreObjectDynamic(inst);
}

void EncodeVisitor::VisitUnresolvedStoreStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeStatic = inst->CastToUnresolvedStoreStatic();
    ASSERT(storeStatic->GetType() == DataType::REFERENCE);
    ASSERT(storeStatic->GetNeedBarrier());
    auto typeId = storeStatic->GetTypeId();
    auto value = enc->GetCodegen()->ConvertRegister(storeStatic->GetSrcReg(0), storeStatic->GetType());
    auto entrypoint = RuntimeInterface::EntrypointId::UNRESOLVED_STORE_STATIC_BARRIERED;
    auto method = storeStatic->GetMethod();
    ASSERT(method != nullptr);
    enc->GetCodegen()->CallRuntimeWithMethod(storeStatic, method, entrypoint, Reg(), TypedImm(typeId), value);
}

void EncodeVisitor::VisitStoreResolvedObjectFieldStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto store = inst->CastToStoreResolvedObjectFieldStatic();
    ASSERT(store->GetType() != DataType::REFERENCE);
    ASSERT(!store->GetNeedBarrier());
    auto val = enc->GetCodegen()->ConvertRegister(store->GetSrcReg(1), store->GetType());
    auto reg = enc->GetCodegen()->ConvertRegister(store->GetSrcReg(0), DataType::REFERENCE);
    // Non-barriered case. Unknown store, assume it can be volatile
    enc->GetEncoder()->EncodeStrRelease(val, MemRef(reg));
}

void EncodeVisitor::VisitNewObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    // NOTE(msherstennikov): use irtoced entrypoint once spill-fills will be supported for entrypoints mode.
    if (enc->cg_->GetArch() == Arch::AARCH32) {
        enc->GetCodegen()->CreateNewObjCallOld(inst->CastToNewObject());
    } else {
        enc->GetCodegen()->CreateNewObjCall(inst->CastToNewObject());
    }
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
}

void EncodeVisitor::VisitUnresolvedLoadType(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto codegen = enc->GetCodegen();
    auto loadType = inst->CastToUnresolvedLoadType();
    if (loadType->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto typeId = loadType->GetTypeId();

    auto runtime = graph->GetRuntime();
    auto method = loadType->GetMethod();

    if (graph->IsAotMode()) {
        ScopedTmpReg tmpReg(encoder);
        // Load pointer to klass from PLT
        codegen->CreateLoadClassFromPLT(inst, tmpReg, dst, typeId);
        // Finally load Object
        encoder->EncodeLdr(dst, false, MemRef(dst, runtime->GetManagedClassOffset(enc->GetArch())));
    } else {
        auto utypes = runtime->GetUnresolvedTypes();
        auto clsAddr = utypes->GetTableSlot(method, typeId, UnresolvedTypesInterface::SlotKind::MANAGED_CLASS);
        Reg dstPtr(dst.GetId(), codegen->GetPtrRegType());
        encoder->EncodeMov(dstPtr, Imm(clsAddr));
        encoder->EncodeLdr(dst, false, MemRef(dstPtr));

        auto slowPath = codegen->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::RESOLVE_CLASS_OBJECT);
        slowPath->SetUnresolvedType(method, typeId);
        slowPath->SetDstReg(dst);
        slowPath->SetSlotAddr(clsAddr);
        encoder->EncodeJump(slowPath->GetLabel(), dst, Condition::EQ);
        slowPath->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitLoadType(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto loadType = inst->CastToLoadType();
    if (loadType->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto typeId = loadType->GetTypeId();

    auto runtime = graph->GetRuntime();
    auto method = loadType->GetMethod();

    if (graph->IsAotMode()) {
        auto methodClassId = runtime->GetClassIdForMethod(graph->GetMethod());
        if (methodClassId == typeId) {
            auto dstPtr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dstPtr);
            auto mem = MemRef(dstPtr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
        } else {
            ScopedTmpReg tmpReg(encoder);
            // Load pointer to klass from PLT
            enc->GetCodegen()->CreateLoadClassFromPLT(inst, tmpReg, dst, typeId);
        }
        // Finally load ManagedClass object
        encoder->EncodeLdr(dst, false, MemRef(dst, runtime->GetManagedClassOffset(enc->GetArch())));
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(runtime->ResolveType(method, typeId));
        auto managedKlass = runtime->GetManagedType(klass);
        encoder->EncodeMov(dst, Imm(managedKlass));
    }
}

void EncodeVisitor::FillUnresolvedClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::CHECK_CAST);
    encoder->EncodeJump(slowPath->GetLabel(), classReg, Condition::EQ);
    slowPath->CreateBackLabel(encoder);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    encoder->EncodeJump(slowPath->GetBackLabel(), src, Condition::EQ);
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmpReg, src);
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg, classReg, Condition::NE);
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillObjectClass(GraphVisitor *visitor, Reg tmpReg, LabelHolder::LabelId throwLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();

    Reg typeReg(tmpReg.GetId(), INT8_TYPE);
    // Load type class
    encoder->EncodeLdr(typeReg, false, MemRef(tmpReg, runtime->GetClassTypeOffset(enc->GetArch())));
    // Jump to EH if type not reference
    encoder->EncodeJump(throwLabel, typeReg, Imm(runtime->GetReferenceTypeMask()), Condition::NE);
}

/* The CheckCast class should be a subclass of input class:
    ......................
    bool Class::IsSubClassOf(const Class *klass) const {
        const Class *current = this;
        do {
            if (current == klass) {
                return true;
            }
            current = current->GetBase();
        } while (current != nullptr);
        return false;
    }
*/

void EncodeVisitor::FillOtherClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId throwLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto loopLabel = encoder->CreateLabel();

    // First compare `current == klass` we make before switch
    encoder->BindLabel(loopLabel);
    // Load base klass
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, graph->GetRuntime()->GetClassBaseOffset(enc->GetArch())));
    encoder->EncodeJump(throwLabel, tmpReg, Condition::EQ);
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    encoder->EncodeJump(loopLabel, tmpReg, classReg, Condition::NE);
}

void EncodeVisitor::FillArrayObjectClass(GraphVisitor *visitor, Reg tmpReg, LabelHolder::LabelId throwLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();

    Reg typeReg(tmpReg.GetId(), INT8_TYPE);
    // Load Component class
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Jump to EH if src is not array class
    encoder->EncodeJump(throwLabel, tmpReg, Condition::EQ);
    // Load type of the component class
    encoder->EncodeLdr(typeReg, false, MemRef(tmpReg, runtime->GetClassTypeOffset(enc->GetArch())));
    // Jump to EH if type not reference
    encoder->EncodeJump(throwLabel, typeReg, Imm(runtime->GetReferenceTypeMask()), Condition::NE);
}

void EncodeVisitor::FillArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId throwLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();

    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::CHECK_CAST);

    // Load Component type of Input
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(throwLabel, tmpReg, Condition::EQ);
    // Load Component type of the instance
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmpReg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    encoder->EncodeLdr(tmpReg1, false, MemRef(classReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Compare component types
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg, tmpReg1, Condition::NE);

    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillInterfaceClass(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto codegen = enc->GetCodegen();
    if (codegen->GetArch() == Arch::AARCH32) {
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::CHECK_CAST);
        encoder->EncodeJump(slowPath->GetLabel());
        slowPath->BindBackLabel(encoder);
    } else {
        codegen->CreateCheckCastInterfaceCall(inst);
    }
}

void EncodeVisitor::FillCheckCast(GraphVisitor *visitor, Inst *inst, Reg src, LabelHolder::LabelId endLabel,
                                  compiler::ClassType klassType)
{
    if (klassType == ClassType::INTERFACE_CLASS) {
        FillInterfaceClass(visitor, inst);
        return;
    }
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    // class_reg - CheckCast class
    // tmp_reg - input class
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmpReg, src);
    // There is no exception if the classes are equal
    encoder->EncodeJump(endLabel, classReg, tmpReg, Condition::EQ);

    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathCheckCast>(inst, EntrypointId::CLASS_CAST_EXCEPTION);
    slowPath->SetClassReg(classReg);
    auto throwLabel = slowPath->GetLabel();
    switch (klassType) {
        // The input class should be not primitive type
        case ClassType::OBJECT_CLASS: {
            FillObjectClass(visitor, tmpReg, throwLabel);
            break;
        }
        case ClassType::OTHER_CLASS: {
            FillOtherClass(visitor, inst, tmpReg, throwLabel);
            break;
        }
        // The input class should be array class and component type should be not primitive type
        case ClassType::ARRAY_OBJECT_CLASS: {
            FillArrayObjectClass(visitor, tmpReg, throwLabel);
            break;
        }
        // Check that components types are equals, else call slow path
        case ClassType::ARRAY_CLASS: {
            FillArrayClass(visitor, inst, tmpReg, throwLabel);
            break;
        }
        case ClassType::FINAL_CLASS: {
            EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::CHECKCAST,
                                         events::CodegenSimplificationReason::FINAL_CLASS);
            encoder->EncodeJump(throwLabel);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

void EncodeVisitor::VisitCheckCast(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto method = inst->CastToCheckCast()->GetMethod();
    auto typeId = inst->CastToCheckCast()->GetTypeId();
    auto encoder = enc->GetEncoder();

    auto klassType = inst->CastToCheckCast()->GetClassType();
    if (klassType == ClassType::UNRESOLVED_CLASS) {
        FillUnresolvedClass(visitor, inst);
        return;
    }
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto endLabel = encoder->CreateLabel();

    if (inst->CastToCheckCast()->GetOmitNullCheck()) {
        EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::CHECKCAST,
                                     events::CodegenSimplificationReason::SKIP_NULLCHECK);
    } else {
        // Compare with nullptr
        encoder->EncodeJump(endLabel, src, Condition::EQ);
    }

    [[maybe_unused]] auto klass = enc->cg_->GetGraph()->GetRuntime()->GetClass(method, typeId);
    ASSERT(klass != nullptr);

    FillCheckCast(visitor, inst, src, endLabel, klassType);
    encoder->BindLabel(endLabel);
}

void EncodeVisitor::FillIsInstanceUnresolved(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);
    encoder->EncodeJump(slowPath->GetLabel(), classReg, Condition::EQ);
    slowPath->CreateBackLabel(encoder);
    auto endLabel = slowPath->GetBackLabel();

    // Compare with nullptr
    auto nextLabel = encoder->CreateLabel();
    encoder->EncodeJump(nextLabel, src, Condition::NE);
    encoder->EncodeMov(dst, Imm(0));
    encoder->EncodeJump(endLabel);
    encoder->BindLabel(nextLabel);

    // Get instance class
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmpReg, src);

    // Sets true if the classes are equal
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg, classReg, Condition::NE);
    encoder->EncodeMov(dst, Imm(1));

    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstanceCaseObject(GraphVisitor *visitor, Inst *inst, Reg tmpReg)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    // ClassType::OBJECT_CLASS
    Reg typeReg(tmpReg.GetId(), INT8_TYPE);
    // Load type class
    encoder->EncodeLdr(typeReg, false, MemRef(tmpReg, runtime->GetClassTypeOffset(enc->GetArch())));
    ScopedTmpReg typeMaskReg(encoder, INT8_TYPE);
    encoder->EncodeMov(typeMaskReg, Imm(runtime->GetReferenceTypeMask()));
    encoder->EncodeCompare(dst, typeMaskReg, typeReg, Condition::EQ);
}

/* Sets true if IsInstance class is a subclass of input class:
    ......................
    bool Class::IsSubClassOf(const Class *klass) const {
        const Class *current = this;
        do {
            if (current == klass) {
                return true;
            }
            current = current->GetBase();
        } while (current != nullptr);
        return false;
    }
*/

void EncodeVisitor::FillIsInstanceCaseOther(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                            LabelHolder::LabelId endLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);

    // ClassType::OTHER_CLASS
    auto loopLabel = encoder->CreateLabel();
    auto falseLabel = encoder->CreateLabel();

    // First compare `current == klass` we make before switch
    encoder->BindLabel(loopLabel);
    // Load base klass
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassBaseOffset(enc->GetArch())));
    encoder->EncodeJump(falseLabel, tmpReg, Condition::EQ);
    encoder->EncodeJump(loopLabel, tmpReg, classReg, Condition::NE);

    // Set true result and jump to exit
    encoder->EncodeMov(dst, Imm(1));
    encoder->EncodeJump(endLabel);

    // Set false result and jump to exit
    encoder->BindLabel(falseLabel);
    encoder->EncodeMov(dst, Imm(0));
}

// Sets true if the Input class is array class and component type is not primitive type
void EncodeVisitor::FillIsInstanceCaseArrayObject(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                                  LabelHolder::LabelId endLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    // ClassType::ARRAY_OBJECT_CLASS
    Reg dstRef(dst.GetId(), Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    Reg typeReg(tmpReg.GetId(), INT8_TYPE);
    // Load Component class
    encoder->EncodeLdr(dstRef, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(endLabel, dstRef, Condition::EQ);
    // Load type of the component class
    encoder->EncodeLdr(typeReg, false, MemRef(dstRef, runtime->GetClassTypeOffset(enc->GetArch())));
    ScopedTmpReg typeMaskReg(encoder, INT8_TYPE);
    encoder->EncodeMov(typeMaskReg, Imm(runtime->GetReferenceTypeMask()));
    encoder->EncodeCompare(dst, typeMaskReg, typeReg, Condition::EQ);
}

// Check that components types are equals, else call slow path
void EncodeVisitor::FillIsInstanceCaseArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                                 LabelHolder::LabelId endLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    // ClassType::ARRAY_CLASS
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);

    auto nextLabel1 = encoder->CreateLabel();
    // Load Component type of Input
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(nextLabel1, tmpReg, Condition::NE);
    encoder->EncodeMov(dst, Imm(0));
    encoder->EncodeJump(endLabel);
    encoder->BindLabel(nextLabel1);
    // Load Component type of the instance
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmpReg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    encoder->EncodeLdr(tmpReg1, false, MemRef(classReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Compare component types
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg, tmpReg1, Condition::NE);
    encoder->EncodeMov(dst, Imm(1));

    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstanceCaseInterface(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    // ClassType::INTERFACE_CLASS
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);

    encoder->EncodeJump(slowPath->GetLabel());

    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstance(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId endLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    if (inst->CastToIsInstance()->GetOmitNullCheck()) {
        EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::ISINSTANCE,
                                     events::CodegenSimplificationReason::SKIP_NULLCHECK);
    } else {
        // Compare with nullptr
        auto nextLabel = encoder->CreateLabel();
        encoder->EncodeJump(nextLabel, src, Condition::NE);
        encoder->EncodeMov(dst, Imm(0));
        encoder->EncodeJump(endLabel);
        encoder->BindLabel(nextLabel);
    }

    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    enc->GetCodegen()->LoadClassFromObject(tmpReg, src);

    // Sets true if the classes are equals
    if (inst->CastToIsInstance()->GetClassType() == ClassType::FINAL_CLASS) {
        encoder->EncodeCompare(dst, classReg, tmpReg, Condition::EQ);
    } else if (dst.GetId() != src.GetId() && dst.GetId() != classReg.GetId()) {
        encoder->EncodeCompare(dst, classReg, tmpReg, Condition::EQ);
        encoder->EncodeJump(endLabel, dst, Condition::NE);
    } else {
        auto nextLabel1 = encoder->CreateLabel();
        encoder->EncodeJump(nextLabel1, classReg, tmpReg, Condition::NE);
        encoder->EncodeMov(dst, Imm(1));
        encoder->EncodeJump(endLabel);
        encoder->BindLabel(nextLabel1);
    }
}

void EncodeVisitor::VisitIsInstance(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto klassType = inst->CastToIsInstance()->GetClassType();
    if (klassType == ClassType::UNRESOLVED_CLASS) {
        FillIsInstanceUnresolved(visitor, inst);
        return;
    }
    // tmp_reg - input class
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    auto endLabel = encoder->CreateLabel();

    FillIsInstance(visitor, inst, tmpReg, endLabel);
    switch (klassType) {
        // Sets true if the Input class is not primitive type
        case ClassType::OBJECT_CLASS: {
            FillIsInstanceCaseObject(visitor, inst, tmpReg);
            break;
        }
        case ClassType::OTHER_CLASS: {
            FillIsInstanceCaseOther(visitor, inst, tmpReg, endLabel);
            break;
        }
        // Sets true if the Input class is array class and component type is not primitive type
        case ClassType::ARRAY_OBJECT_CLASS: {
            FillIsInstanceCaseArrayObject(visitor, inst, tmpReg, endLabel);
            break;
        }
        // Check that components types are equals, else call slow path
        case ClassType::ARRAY_CLASS: {
            FillIsInstanceCaseArrayClass(visitor, inst, tmpReg, endLabel);
            break;
        }
        case ClassType::INTERFACE_CLASS: {
            FillIsInstanceCaseInterface(visitor, inst);
            break;
        }
        case ClassType::FINAL_CLASS: {
            EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::ISINSTANCE,
                                         events::CodegenSimplificationReason::FINAL_CLASS);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
    encoder->BindLabel(endLabel);
}

void Codegen::CreateMonitorCall(MonitorInst *inst)
{
    auto src = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto id = inst->IsExit() ? EntrypointId::MONITOR_EXIT_FAST_PATH : EntrypointId::MONITOR_ENTER_FAST_PATH;
    CallFastPath(inst, id, INVALID_REGISTER, RegMask::GetZeroMask(), src);
}

void Codegen::CreateMonitorCallOld(MonitorInst *inst)
{
    auto src = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto id = inst->IsExit() ? EntrypointId::UNLOCK_OBJECT : EntrypointId::LOCK_OBJECT;
    CallRuntime(inst, id, dst, RegMask::GetZeroMask(), src);
}

void Codegen::CreateCheckCastInterfaceCall(Inst *inst)
{
    auto obj = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto interface = ConvertRegister(inst->GetSrcReg(SECOND_OPERAND), DataType::REFERENCE);
    CallFastPath(inst, EntrypointId::CHECK_CAST_INTERFACE, INVALID_REGISTER, RegMask::GetZeroMask(), obj, interface);
}

void EncodeVisitor::VisitMonitor(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (enc->GetCodegen()->GetArch() == Arch::AARCH32) {
        enc->GetCodegen()->CreateMonitorCallOld(inst->CastToMonitor());
    } else {
        enc->GetCodegen()->CreateMonitorCall(inst->CastToMonitor());
    }
}

void Codegen::TryInsertImplicitNullCheck(Inst *inst, size_t prevOffset)
{
    if (!IsSuitableForImplicitNullCheck(inst)) {
        return;
    }
    if (!inst->CanThrow()) {
        return;
    }

    auto nullcheck = inst->GetInput(0).GetInst();
    ASSERT(nullcheck->GetOpcode() == Opcode::NullCheck && nullcheck->CastToNullCheck()->IsImplicit());
    auto currOffset = GetEncoder()->GetCursorOffset();
    ASSERT(currOffset > prevOffset);
    GetCodeBuilder()->AddImplicitNullCheck(currOffset, currOffset - prevOffset);
    CreateStackMap(nullcheck, inst);
}

void Codegen::CreateFloatIsInf([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeIsInf(dst, src[0]);
}

void Codegen::CreateFloatIsInteger([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeIsInteger(dst, src[0]);
}

void Codegen::CreateFloatIsSafeInteger([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeIsSafeInteger(dst, src[0]);
}

void Codegen::CreateStringEquals([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entrypointId = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::STRING_EQUALS_COMPRESSED
                                                                   : EntrypointId::STRING_EQUALS;
    CallFastPath(inst, entrypointId, dst, {}, src[0], src[1U]);
}

void Codegen::CreateMathCeil([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeCeil(dst, src[0]);
}

void Codegen::CreateMathFloor([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeFloor(dst, src[0]);
}

void Codegen::CreateCountLeadingZeroBits([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeCountLeadingZeroBits(Reg(dst.GetId(), src[0].GetType()), src[0]);
}

void Codegen::CreateStringBuilderChar(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    CallFastPath(inst, EntrypointId::STRING_BUILDER_CHAR, dst, RegMask::GetZeroMask(), src[0], src[1U]);
}

void Codegen::CreateStringBuilderBool(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    CallFastPath(inst, EntrypointId::STRING_BUILDER_BOOL, dst, RegMask::GetZeroMask(), src[0], src[1U]);
}

void Codegen::CreateStringBuilderString(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    auto entrypointId = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::STRING_BUILDER_STRING_COMPRESSED
                                                                   : EntrypointId::STRING_BUILDER_STRING;

    CallFastPath(inst, entrypointId, dst, RegMask::GetZeroMask(), src[0], src[1U]);
}

void Codegen::CallFastCreateStringFromCharArrayTlab(Inst *inst, Reg dst, Reg offset, Reg count, Reg array,
                                                    std::variant<Reg, TypedImm> klass)
{
    if (GetRegfile()->GetZeroReg().GetId() == offset.GetId()) {
        auto entryId = GetRuntime()->IsCompressedStringsEnabled()
                           ? EntrypointId::CREATE_STRING_FROM_ZERO_BASED_CHAR_ARRAY_TLAB_COMPRESSED
                           : EntrypointId::CREATE_STRING_FROM_ZERO_BASED_CHAR_ARRAY_TLAB;
        if (std::holds_alternative<TypedImm>(klass)) {
            CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), count, array, std::get<TypedImm>(klass));
        } else {
            CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), count, array, std::get<Reg>(klass));
        }
    } else {
        auto entryId = GetRuntime()->IsCompressedStringsEnabled()
                           ? EntrypointId::CREATE_STRING_FROM_CHAR_ARRAY_TLAB_COMPRESSED
                           : EntrypointId::CREATE_STRING_FROM_CHAR_ARRAY_TLAB;
        if (std::holds_alternative<TypedImm>(klass)) {
            CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), offset, count, array, std::get<TypedImm>(klass));
        } else {
            CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), offset, count, array, std::get<Reg>(klass));
        }
    }
}

void Codegen::CreateStringFromCharArrayTlab(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto runtime = GetGraph()->GetRuntime();
    auto offset = src[FIRST_OPERAND];
    auto count = src[SECOND_OPERAND];
    auto array = src[THIRD_OPERAND];
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klassReg(GetEncoder());
        GetEncoder()->EncodeLdr(klassReg, false,
                                MemRef(ThreadReg(), runtime->GetStringClassPointerTlsOffset(GetArch())));
        CallFastCreateStringFromCharArrayTlab(inst, dst, offset, count, array, klassReg);
    } else {
        auto klassImm = TypedImm(reinterpret_cast<uintptr_t>(runtime->GetStringClass(GetGraph()->GetMethod())));
        CallFastCreateStringFromCharArrayTlab(inst, dst, offset, count, array, klassImm);
    }
}

void Codegen::CreateStringFromStringTlab(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entryId = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::CREATE_STRING_FROM_STRING_TLAB_COMPRESSED
                                                              : EntrypointId::CREATE_STRING_FROM_STRING_TLAB;
    auto srcStr = src[FIRST_OPERAND];
    CallFastPath(inst, entryId, dst, RegMask::GetZeroMask(), srcStr);
}

void Codegen::CreateStringSubstringTlab([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entrypointId = GetRuntime()->IsCompressedStringsEnabled()
                            ? EntrypointId::SUB_STRING_FROM_STRING_TLAB_COMPRESSED
                            : EntrypointId::SUB_STRING_FROM_STRING_TLAB;
    CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND]);
}

void Codegen::CreateStringGetCharsTlab([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entrypointId = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::STRING_GET_CHARS_TLAB_COMPRESSED
                                                                   : EntrypointId::STRING_GET_CHARS_TLAB;
    auto runtime = GetGraph()->GetRuntime();
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klassReg(GetEncoder());
        GetEncoder()->EncodeLdr(klassReg, false,
                                MemRef(ThreadReg(), runtime->GetArrayU16ClassPointerTlsOffset(GetArch())));
        CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                     klassReg);
    } else {
        auto klassImm = TypedImm(reinterpret_cast<uintptr_t>(runtime->GetArrayU16Class(GetGraph()->GetMethod())));
        CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                     klassImm);
    }
}

void Codegen::CreateStringHashCode([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entrypoint = GetRuntime()->IsCompressedStringsEnabled() ? EntrypointId::STRING_HASH_CODE_COMPRESSED
                                                                 : EntrypointId::STRING_HASH_CODE;
    auto strReg = src[FIRST_OPERAND];
    auto mref = MemRef(strReg, panda::coretypes::String::GetHashcodeOffset());
    auto slowPath = CreateSlowPath<SlowPathStringHashCode>(inst, entrypoint);
    slowPath->SetDstReg(dst);
    slowPath->SetSrcReg(strReg);
    if (dst.GetId() != strReg.GetId()) {
        GetEncoder()->EncodeLdr(ConvertRegister(dst.GetId(), DataType::INT32), false, mref);
        GetEncoder()->EncodeJump(slowPath->GetLabel(), dst, Condition::EQ);
    } else {
        ScopedTmpReg hashReg(GetEncoder(), INT32_TYPE);
        GetEncoder()->EncodeLdr(hashReg, false, mref);
        GetEncoder()->EncodeJump(slowPath->GetLabel(), hashReg, Condition::EQ);
        GetEncoder()->EncodeMov(dst, hashReg);
    }
    slowPath->BindBackLabel(GetEncoder());
}

#include "intrinsics_codegen.inl"

void Codegen::CreateBuiltinIntrinsic(IntrinsicInst *inst)
{
    Reg dst = INVALID_REGISTER;
    SRCREGS src;

    if (!inst->NoDest()) {
        dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    }

    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        if (inst->GetInput(i).GetInst()->IsSaveState()) {
            continue;
        }
        auto location = inst->GetLocation(i);
        auto type = inst->GetInputType(i);
        src[i] = ConvertRegister(location.GetValue(), type);
    }
    FillBuiltin(inst, src, dst);
}

void EncodeVisitor::VisitIntrinsic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto intrinsic = inst->CastToIntrinsic();
    auto id = intrinsic->GetIntrinsicId();
    auto arch = codegen->GetGraph()->GetArch();
    auto runtime = codegen->GetGraph()->GetRuntime();
    if (EncodesBuiltin(runtime, id, arch) || IsIrtocIntrinsic(id)) {
        codegen->CreateBuiltinIntrinsic(intrinsic);
        return;
    }
    codegen->CreateCallIntrinsic(intrinsic);
}

void EncodeVisitor::VisitBoundsCheckI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto lenReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    ASSERT(inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveState ||
           inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveStateDeoptimize);
    EntrypointId entrypoint = inst->CastToBoundsCheckI()->IsArray()
                                  ? EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION
                                  : EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;
    LabelHolder::LabelId label;
    if (inst->CanDeoptimize()) {
        label = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::BOUNDS_CHECK)->GetLabel();
    } else {
        label = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, entrypoint)->GetLabel();
    }

    auto value = inst->CastToBoundsCheckI()->GetImm();
    if (enc->GetEncoder()->CanEncodeImmAddSubCmp(value, WORD_SIZE, false)) {
        enc->GetEncoder()->EncodeJump(label, lenReg, Imm(value), Condition::LS);
    } else {
        ScopedTmpReg tmp(enc->GetEncoder(), lenReg.GetType());
        enc->GetEncoder()->EncodeMov(tmp, Imm(value));
        enc->GetEncoder()->EncodeJump(label, lenReg, tmp, Condition::LS);
    }
}

void EncodeVisitor::VisitStoreArrayI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto arrayReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto type = inst->GetType();
    auto value = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);

    auto index = inst->CastToStoreArrayI()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    if (!enc->GetCodegen()->OffsetFitReferenceTypeSize(offset)) {
        // such code should not be executed
        enc->GetEncoder()->EncodeAbort();
        return;
    }
    auto mem = MemRef(arrayReg, offset);
    if (inst->CastToStoreArrayI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(arrayReg.GetId(), value.GetId()));
    }
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStr(value, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreArrayI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, value, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitLoadArrayI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto instLoadArrayI = inst->CastToLoadArrayI();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    ASSERT(instLoadArrayI->IsArray() || !runtime->IsCompressedStringsEnabled());
    if (instLoadArrayI->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto arrayReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0));
    uint32_t index = instLoadArrayI->GetImm();
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto dataOffset = instLoadArrayI->IsArray() ? runtime->GetArrayDataOffset(enc->GetArch())
                                                : runtime->GetStringDataOffset(enc->GetArch());
    uint32_t shift = DataType::ShiftByType(type, enc->GetArch());
    auto offset = dataOffset + (index << shift);
    auto mem = MemRef(arrayReg, offset);
    auto encoder = enc->GetEncoder();
    auto arch = enc->GetArch();
    ScopedTmpReg scopedTmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    encoder->EncodeLdr(dst, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitLoadCompressedStringCharI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0));                   // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);  // length
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);               // load value
    auto offset = runtime->GetStringDataOffset(enc->GetArch());
    auto encoder = enc->GetEncoder();
    auto arch = encoder->GetArch();
    int32_t shift = DataType::ShiftByType(type, arch);
    auto index = inst->CastToLoadCompressedStringCharI()->GetImm();
    ASSERT(encoder->CanEncodeCompressedStringCharAt());
    auto mask = runtime->GetStringCompressionMask();
    if (mask != 1) {
        UNREACHABLE();  // mask is hardcoded in JCL, but verify it just in case it's changed
    }
    enc->GetEncoder()->EncodeCompressedStringCharAtI(dst, src0, src1, offset, index, shift);
}

void EncodeVisitor::VisitMultiArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();

    auto arrayInst = inst->CastToMultiArray();
    codegen->CreateMultiArrayCall(arrayInst);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
}

void EncodeVisitor::VisitInitEmptyString(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    codegen->CallRuntime(inst, EntrypointId::CREATE_EMPTY_STRING, dst, RegMask::GetZeroMask());
}

void EncodeVisitor::VisitInitString(GraphVisitor *visitor, Inst *inst)
{
    auto cg = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto initStr = inst->CastToInitString();

    auto dst = cg->ConvertRegister(initStr->GetDstReg(), initStr->GetType());
    auto ctorArg = cg->ConvertRegister(initStr->GetSrcReg(0), initStr->GetInputType(0));

    if (cg->GetArch() == Arch::AARCH32) {
        auto entryId =
            initStr->IsFromString() ? EntrypointId::CREATE_STRING_FROM_STRING : EntrypointId::CREATE_STRING_FROM_CHARS;
        cg->CallRuntime(initStr, entryId, dst, RegMask::GetZeroMask(), ctorArg);
        return;
    }

    if (initStr->IsFromString()) {
        auto entryId = cg->GetRuntime()->IsCompressedStringsEnabled()
                           ? compiler::RuntimeInterface::EntrypointId::CREATE_STRING_FROM_STRING_TLAB_COMPRESSED
                           : compiler::RuntimeInterface::EntrypointId::CREATE_STRING_FROM_STRING_TLAB;
        cg->CallFastPath(initStr, entryId, dst, RegMask::GetZeroMask(), ctorArg);
    } else {
        auto enc = cg->GetEncoder();
        auto runtime = cg->GetGraph()->GetRuntime();
        auto mem = MemRef(ctorArg, static_cast<int64_t>(runtime->GetArrayLengthOffset(cg->GetArch())));
        ScopedTmpReg lengthReg(enc);
        enc->EncodeLdr(lengthReg, IsTypeSigned(initStr->GetType()), mem);
        if (cg->GetGraph()->IsAotMode()) {
            auto klassOffs = runtime->GetStringClassPointerTlsOffset(cg->GetArch());
            ScopedTmpReg klassReg(enc);
            enc->EncodeLdr(klassReg, false, MemRef(cg->ThreadReg(), klassOffs));
            cg->CallFastCreateStringFromCharArrayTlab(initStr, dst, cg->GetRegfile()->GetZeroReg(), lengthReg, ctorArg,
                                                      klassReg);
        } else {
            auto klassPtr = runtime->GetStringClass(cg->GetGraph()->GetMethod());
            auto klassImm = TypedImm(reinterpret_cast<uintptr_t>(klassPtr));
            cg->CallFastCreateStringFromCharArrayTlab(initStr, dst, cg->GetRegfile()->GetZeroReg(), lengthReg, ctorArg,
                                                      klassImm);
        }
    }
}

void EncodeVisitor::VisitCallLaunchStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->CreateLaunchCall(inst->CastToCallLaunchStatic());
}

void EncodeVisitor::VisitCallLaunchVirtual(GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->CreateLaunchCall(inst->CastToCallLaunchVirtual());
}

void EncodeVisitor::VisitCallResolvedLaunchStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->CreateLaunchCall(inst->CastToCallResolvedLaunchStatic());
}

void EncodeVisitor::VisitCallResolvedLaunchVirtual(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->CreateLaunchCall(inst->CastToCallResolvedLaunchVirtual());
}

void EncodeVisitor::VisitResolveStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitResolveStatic(inst->CastToResolveStatic());
}

void EncodeVisitor::VisitCallResolvedStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallResolvedStatic(inst->CastToCallResolvedStatic());
}

void EncodeVisitor::VisitCallStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallStatic(inst->CastToCallStatic());
}

void EncodeVisitor::VisitCallVirtual(GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallVirtual(inst->CastToCallVirtual());
}

void EncodeVisitor::VisitResolveVirtual(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitResolveVirtual(inst->CastToResolveVirtual());
}

void EncodeVisitor::VisitCallResolvedVirtual(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallResolvedVirtual(inst->CastToCallResolvedVirtual());
}

void EncodeVisitor::VisitCallDynamic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallDynamic(inst->CastToCallDynamic());
}

void EncodeVisitor::VisitLoadConstantPool(GraphVisitor *visitor, Inst *inst)
{
    if (TryLoadConstantPoolGen(inst, static_cast<EncodeVisitor *>(visitor))) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitLoadLexicalEnv(GraphVisitor *visitor, Inst *inst)
{
    if (TryLoadLexicalEnvGen(inst, static_cast<EncodeVisitor *>(visitor))) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitLoadFromConstantPool([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    UNREACHABLE();
}

void EncodeVisitor::VisitSafePoint(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto graph = codegen->GetGraph();
    auto encoder = enc->GetEncoder();
    int64_t flagAddrOffset = graph->GetRuntime()->GetFlagAddrOffset(codegen->GetArch());
    ScopedTmpRegU16 tmp(encoder);

    // TMP <= Flag
    auto mem = MemRef(codegen->ThreadReg(), flagAddrOffset);
    encoder->EncodeLdr(tmp, false, mem);

    // check value and jump to call GC
    auto slowPath = codegen->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::SAFEPOINT);

    encoder->EncodeJump(slowPath->GetLabel(), tmp, Condition::NE);

    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::VisitSelect(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto dstType = inst->GetType();
    auto cmpType = inst->CastToSelect()->GetOperandsType();

    constexpr int32_t IMM_2 = 2;
    constexpr int32_t IMM_3 = 3;
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), dstType);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), dstType);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), dstType);
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), cmpType);
    auto src3 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_3), cmpType);
    auto cc = enc->GetCodegen()->ConvertCc(inst->CastToSelect()->GetCc());
    if (IsTestCc(cc)) {
        enc->GetEncoder()->EncodeSelectTest(dst, src0, src1, src2, src3, cc);
    } else {
        enc->GetEncoder()->EncodeSelect(dst, src0, src1, src2, src3, cc);
    }
}

void EncodeVisitor::VisitSelectImm(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto dstType = inst->GetType();
    auto cmpType = inst->CastToSelectImm()->GetOperandsType();

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), dstType);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), dstType);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), dstType);
    constexpr int32_t IMM_2 = 2;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), cmpType);
    auto imm = Imm(inst->CastToSelectImm()->GetImm());
    auto cc = enc->GetCodegen()->ConvertCc(inst->CastToSelectImm()->GetCc());
    if (IsTestCc(cc)) {
        enc->GetEncoder()->EncodeSelectTest(dst, src0, src1, src2, imm, cc);
    } else {
        enc->GetEncoder()->EncodeSelect(dst, src0, src1, src2, imm, cc);
    }
}

void EncodeVisitor::VisitIf(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);

    auto bb = inst->GetBasicBlock();
    auto label = bb->GetTrueSuccessor()->GetId();

    auto type = inst->CastToIf()->GetOperandsType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto cc = enc->GetCodegen()->ConvertCc(inst->CastToIf()->GetCc());
    if (IsTestCc(cc)) {
        enc->GetEncoder()->EncodeJumpTest(label, src0, src1, cc);
    } else {
        enc->GetEncoder()->EncodeJump(label, src0, src1, cc);
    }
}

void EncodeVisitor::VisitIfImm(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);

    auto bb = inst->GetBasicBlock();
    auto label = bb->GetTrueSuccessor()->GetId();

    auto type = inst->CastToIfImm()->GetOperandsType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto imm = Imm(inst->CastToIfImm()->GetImm());
    auto cc = enc->GetCodegen()->ConvertCc(inst->CastToIfImm()->GetCc());
    if (IsTestCc(cc)) {
        enc->GetEncoder()->EncodeJumpTest(label, src0, imm, cc);
    } else {
        enc->GetEncoder()->EncodeJump(label, src0, imm, cc);
    }
}

void EncodeVisitor::VisitAddOverflow(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);

    auto bb = inst->GetBasicBlock();
    auto label = bb->GetTrueSuccessor()->GetId();

    auto type = inst->CastToAddOverflow()->GetOperandsType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto cc = enc->GetCodegen()->ConvertCcOverflow(inst->CastToAddOverflow()->GetCc());
    enc->GetEncoder()->EncodeAddOverflow(label, dst, src0, src1, cc);
}

void EncodeVisitor::VisitAddOverflowCheck(GraphVisitor *visitor, Inst *inst)
{
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(0).GetInst()->GetType()));
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(1).GetInst()->GetType()));
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::OVERFLOW);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = Reg(inst->GetSrcReg(0), INT32_TYPE);
    auto src1 = Reg(inst->GetSrcReg(1), INT32_TYPE);
    enc->GetEncoder()->EncodeAddOverflow(slowPath->GetLabel(), dst, src0, src1, Condition::VS);
}

void EncodeVisitor::VisitSubOverflow(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);

    auto bb = inst->GetBasicBlock();
    auto label = bb->GetTrueSuccessor()->GetId();

    auto type = inst->CastToSubOverflow()->GetOperandsType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto cc = enc->GetCodegen()->ConvertCcOverflow(inst->CastToSubOverflow()->GetCc());
    enc->GetEncoder()->EncodeSubOverflow(label, dst, src0, src1, cc);
}

void EncodeVisitor::VisitSubOverflowCheck(GraphVisitor *visitor, Inst *inst)
{
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(0).GetInst()->GetType()));
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(1).GetInst()->GetType()));
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::OVERFLOW);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = Reg(inst->GetSrcReg(0), INT32_TYPE);
    auto src1 = Reg(inst->GetSrcReg(1), INT32_TYPE);
    enc->GetEncoder()->EncodeSubOverflow(slowPath->GetLabel(), dst, src0, src1, Condition::VS);
}

void EncodeVisitor::VisitNegOverflowAndZeroCheck(GraphVisitor *visitor, Inst *inst)
{
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(0).GetInst()->GetType()));
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::OVERFLOW);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src = Reg(inst->GetSrcReg(0), INT32_TYPE);
    enc->GetEncoder()->EncodeNegOverflowAndZero(slowPath->GetLabel(), dst, src);
}

void EncodeVisitor::VisitLoadArrayPair(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->CastToLoadArrayPair()->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);      // index
    auto dst0 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(0), type);                 // first value
    auto dst1 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(1), type);                 // second value
    uint64_t index = inst->CastToLoadArrayPair()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    ScopedTmpReg tmp(enc->GetEncoder());

    int32_t scale = DataType::ShiftByType(type, enc->GetCodegen()->GetArch());
    enc->GetEncoder()->EncodeAdd(tmp, src0, Shift(src1, scale));
    auto mem = MemRef(tmp, offset);
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeLdp(dst0, dst1, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitLoadArrayPairI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->CastToLoadArrayPairI()->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto dst0 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(0), type);                 // first value
    auto dst1 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(1), type);                 // second value
    uint64_t index = inst->CastToLoadArrayPairI()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    auto mem = MemRef(src0, offset);

    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeLdp(dst0, dst1, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

/**
 * It is a pseudo instruction that is needed to separate multiple outputs from a single instruction in SSA such as
 * LoadArrayPair and LoadArrayPairI.  No codegeneration is required.
 */
void EncodeVisitor::VisitLoadPairPart([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst) {}

void EncodeVisitor::VisitStoreArrayPair(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);      // index
    constexpr auto IMM_2 = 2U;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), type);  // first value
    constexpr auto IMM_3 = 3U;
    auto src3 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_3), type);  // second value
    uint64_t index = inst->CastToStoreArrayPair()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    auto scale = DataType::ShiftByType(type, enc->GetCodegen()->GetArch());

    auto tmp = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::REFERENCE);
    enc->GetEncoder()->EncodeAdd(tmp, src0, Shift(src1, scale));
    auto mem = MemRef(tmp, offset);
    if (inst->CastToStoreArrayPair()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(src0.GetId(), src2.GetId(), src3.GetId()), true);
    }
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStp(src2, src3, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);

    if (inst->CastToStoreArrayPair()->GetNeedBarrier()) {
        auto tmpOffset = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::GetIntTypeForReference(enc->GetArch()));
        enc->GetEncoder()->EncodeShl(tmpOffset, src1, Imm(scale));
        enc->GetEncoder()->EncodeAdd(tmpOffset, tmpOffset, Imm(offset));
        auto mem1 = MemRef(src0, tmpOffset, 0);
        enc->GetCodegen()->CreatePostWRB(inst, mem1, src2, src3);
    }
}

void EncodeVisitor::VisitStoreArrayPairI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);                 // first value
    constexpr int32_t IMM_2 = 2;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), type);  // second value

    auto index = inst->CastToStoreArrayPairI()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    if (!enc->GetCodegen()->OffsetFitReferenceTypeSize(offset)) {
        // such code should not be executed
        enc->GetEncoder()->EncodeAbort();
        return;
    }
    auto mem = MemRef(src0, offset);
    if (inst->CastToStoreArrayPairI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(src0.GetId(), src1.GetId(), src2.GetId()), true);
    }
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStp(src1, src2, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreArrayPairI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, src1, src2);
    }
}

void EncodeVisitor::VisitNOP([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
#ifndef NDEBUG
    COMPILER_LOG(DEBUG, CODEGEN) << "The NOP wasn't removed before " << *inst;
#endif
}

void EncodeVisitor::VisitThrow(GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    SlowPathCheck slowPath(codegen->GetEncoder()->CreateLabel(), inst, EntrypointId::THROW_EXCEPTION);
    slowPath.Generate(codegen);
}

void EncodeVisitor::VisitDeoptimize(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    ASSERT(inst->GetSaveState() != nullptr);

    SlowPathDeoptimize slowPath(codegen->GetEncoder()->CreateLabel(), inst,
                                inst->CastToDeoptimize()->GetDeoptimizeType());
    slowPath.Generate(codegen);
}

void EncodeVisitor::VisitIsMustDeoptimize(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto *enc = static_cast<EncodeVisitor *>(visitor)->GetEncoder();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto offset = CFrameFlags::GetOffsetFromSpInBytes(codegen->GetFrameLayout());

    enc->EncodeLdr(dst, false, MemRef(codegen->SpReg(), offset));
    enc->EncodeAnd(dst, dst, Imm(1));
}

void EncodeVisitor::VisitGetInstanceClass(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto objReg = codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetType());
    ASSERT(objReg.IsValid());
    codegen->LoadClassFromObject(dst, objReg);
}

void EncodeVisitor::VisitLoadImmediate(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto loadImm = inst->CastToLoadImmediate();
    if (loadImm->GetObjectType() != LoadImmediateInst::ObjectType::PANDA_FILE_OFFSET) {
        codegen->GetEncoder()->EncodeMov(dst, Imm(reinterpret_cast<uintptr_t>(loadImm->GetObject())));
    } else {
        auto runtime = codegen->GetGraph()->GetRuntime();
        auto pfOffset = runtime->GetPandaFileOffset(codegen->GetGraph()->GetArch());
        codegen->LoadMethod(dst);
        // load pointer to panda file
        codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(dst, pfOffset));
        codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(dst, cross_values::GetFileBaseOffset(codegen->GetArch())));
        // add pointer to pf with offset to str
        codegen->GetEncoder()->EncodeAdd(dst, dst, Imm(loadImm->GetPandaFileOffset()));
    }
}

void EncodeVisitor::VisitFunctionImmediate(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());

    codegen->GetEncoder()->EncodeMov(dst, Imm(inst->CastToFunctionImmediate()->GetFunctionPtr()));
    codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(dst, 0U));
}

void EncodeVisitor::VisitLoadObjFromConst(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());

    Reg dstPointer(dst.As(TypeInfo::FromDataType(DataType::POINTER, codegen->GetArch())));
    codegen->GetEncoder()->EncodeMov(dstPointer, Imm(inst->CastToLoadObjFromConst()->GetObjPtr()));
    codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(dstPointer, 0U));
}

void EncodeVisitor::VisitRegDef([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst) {}

void EncodeVisitor::VisitLiveIn([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst) {}

void EncodeVisitor::VisitLiveOut([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();

    codegen->AddLiveOut(inst->GetBasicBlock(), inst->GetDstReg());

    auto dstReg = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    if (codegen->GetTarget().GetTempRegsMask().Test(dstReg.GetId()) &&
        enc->GetEncoder()->IsScratchRegisterReleased(dstReg)) {
        enc->GetEncoder()->AcquireScratchRegister(dstReg);
    }

    if (inst->GetLocation(0) != inst->GetDstLocation()) {
        auto *src = inst->GetInput(0).GetInst();
        enc->GetEncoder()->EncodeMov(dstReg, codegen->ConvertRegister(inst->GetSrcReg(0), src->GetType()));
    }
}

void EncodeVisitor::VisitCompareAnyType(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *cati = inst->CastToCompareAnyType();

    if (cati->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
        return;
    }

    if (TryCompareAnyTypePluginGen(cati, enc)) {
        return;
    }

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    if (cati->GetAnyType() == AnyBaseType::UNDEFINED_TYPE) {
        enc->GetEncoder()->EncodeMov(dst, Imm(true));
    } else {
        enc->GetEncoder()->EncodeMov(dst, Imm(false));
    }
}

void EncodeVisitor::VisitGetAnyTypeName(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *cati = inst->CastToGetAnyTypeName();

    if (TryGetAnyTypeNamePluginGen(cati, enc)) {
        return;
    }

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), DataType::Type::BOOL);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitCastAnyTypeValue(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *cati = inst->CastToCastAnyTypeValue();

    if (cati->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
        return;
    }

    if (TryCastAnyTypeValuePluginGen(cati, enc)) {
        return;
    }

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), DataType::Type::BOOL);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitCastValueToAnyType(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *cvai = inst->CastToCastValueToAnyType();
    if (TryCastValueToAnyTypePluginGen(cvai, enc)) {
        return;
    }

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), DataType::Type::BOOL);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitAnyTypeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto *checkInst = inst->CastToAnyTypeCheck();

    if (checkInst->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
    }
    // Empty check
    if (checkInst->GetAnyType() == AnyBaseType::UNDEFINED_TYPE) {
        return;
    }

    if (TryAnyTypeCheckPluginGen(checkInst, enc)) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitHclassCheck(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto *checkInst = inst->CastToHclassCheck();

    if (checkInst->GetInputType(0) != DataType::Type::REFERENCE) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
    }

    if (TryHclassCheckPluginGen(checkInst, enc)) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitObjByIndexCheck(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *checkInst = inst->CastToObjByIndexCheck();

    if (checkInst->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
    }
    auto id = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::ANY_TYPE_CHECK)->GetLabel();
    if (TryObjByIndexCheckPluginGen(checkInst, enc, id)) {
        return;
    }
    UNREACHABLE();
}

static bool GetNeedBarrierProperty(const Inst *inst)
{
    Opcode op = inst->GetOpcode();
    if (op == Opcode::LoadObject) {
        return inst->CastToLoadObject()->GetNeedBarrier();
    }
    if (op == Opcode::StoreObject) {
        return inst->CastToStoreObject()->GetNeedBarrier();
    }
    if (op == Opcode::LoadArray) {
        return inst->CastToLoadArray()->GetNeedBarrier();
    }
    if (op == Opcode::StoreArray) {
        return inst->CastToStoreArray()->GetNeedBarrier();
    }
    if (op == Opcode::LoadArrayI) {
        return inst->CastToLoadArrayI()->GetNeedBarrier();
    }
    if (op == Opcode::StoreArrayI) {
        return inst->CastToStoreArrayI()->GetNeedBarrier();
    }
    if (op == Opcode::LoadArrayPair) {
        return inst->CastToLoadArrayPair()->GetNeedBarrier();
    }
    if (op == Opcode::StoreArrayPair) {
        return inst->CastToStoreArrayPair()->GetNeedBarrier();
    }
    if (op == Opcode::LoadArrayPairI) {
        return inst->CastToLoadArrayPairI()->GetNeedBarrier();
    }
    if (op == Opcode::StoreArrayPairI) {
        return inst->CastToStoreArrayPairI()->GetNeedBarrier();
    }
    if (op == Opcode::LoadStatic) {
        return inst->CastToLoadStatic()->GetNeedBarrier();
    }
    if (op == Opcode::StoreStatic) {
        return inst->CastToStoreStatic()->GetNeedBarrier();
    }
    return false;
}

/**
 * Returns true if codegen emits call(s) to some library function(s)
 * while processing the instruction.
 */
bool Codegen::InstEncodedWithLibCall(const Inst *inst, Arch arch)
{
    ASSERT(inst != nullptr);
    Opcode op = inst->GetOpcode();
    if (op == Opcode::Mod) {
        auto dstType = inst->GetType();
        if (arch == Arch::AARCH64 || arch == Arch::X86_64) {
            return dstType == DataType::FLOAT32 || dstType == DataType::FLOAT64;
        }
        return arch == Arch::AARCH32;
    }
    if (op == Opcode::Div && arch == Arch::AARCH32) {
        auto dstType = inst->GetType();
        return dstType == DataType::INT64 || dstType == DataType::UINT64;
    }
    if (op == Opcode::Cast && arch == Arch::AARCH32) {
        auto dstType = inst->GetType();
        auto srcType = inst->GetInputType(0);
        if (dstType == DataType::FLOAT32 || dstType == DataType::FLOAT64) {
            return srcType == DataType::INT64 || srcType == DataType::UINT64;
        }
        if (srcType == DataType::FLOAT32 || srcType == DataType::FLOAT64) {
            return dstType == DataType::INT64 || dstType == DataType::UINT64;
        }
        return false;
    }

    return GetNeedBarrierProperty(inst);
}

Reg Codegen::ConvertInstTmpReg(const Inst *inst, DataType::Type type) const
{
    ASSERT(inst->GetTmpLocation().IsFixedRegister());
    return Reg(inst->GetTmpLocation().GetValue(), ConvertDataType(type, GetArch()));
}

Reg Codegen::ConvertInstTmpReg(const Inst *inst) const
{
    return ConvertInstTmpReg(inst, Is64BitsArch(GetArch()) ? DataType::INT64 : DataType::INT32);
}

}  // namespace panda::compiler
