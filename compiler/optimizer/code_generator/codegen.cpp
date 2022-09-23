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

/*
Codegen Hi-Level implementation
*/
#include <iomanip>
#include "operands.h"
#include "compiler_options.h"
#include "lib_call_inst.h"
#include "relocations.h"
#include "include/compiler_interface.h"
#include "ir-dyn-base-types.h"
#include "compiler/optimizer/ir/analysis.h"
#include "compiler/optimizer/ir/locations.h"
#include "compiler/optimizer/analysis/liveness_analyzer.h"
#include "optimizer/code_generator/method_properties.h"
#include "events/events.h"
#include "libpandabase/utils/tsan_interface.h"
#include "codegen.h"

namespace panda::compiler {

class OsrEntryStub {
    void FixIntervals(Codegen *codegen, Encoder *encoder)
    {
        auto &la = codegen->GetGraph()->GetAnalysis<LivenessAnalyzer>();
        la.EnumerateLiveIntervalsForInst(save_state_, [this, codegen, encoder](const auto &li) {
            auto inst = li->GetInst();
            auto location = li->GetLocation();
            // Skip live registers that are already in the input list of the OsrSaveState
            const auto &ss_inputs = save_state_->GetInputs();
            if (std::find_if(ss_inputs.begin(), ss_inputs.end(),
                             [inst](auto &input) { return input.GetInst() == inst; }) != ss_inputs.end()) {
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
                            Imm(inst->CastToConstant()->GetRawValue()),
                            MemRef(codegen->SpReg(), codegen->GetFrameLayout().GetSpillOffsetFromSpInBytes(slot)));
                    } else {
                        ASSERT(location.IsConstant());
                    }
                    break;
                }
                // TODO (ekudriashov): UnresolvedLoadAndInitClass
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
    OsrEntryStub(Codegen *codegen, SaveStateInst *inst)
        : label_(codegen->GetEncoder()->CreateLabel()), save_state_(inst)
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
        codegen->CreateStackMap(save_state_->CastToSaveStateOsr());
        ssize_t slot = CFrameLayout::LOCALS_START_SLOT + CFrameLayout::GetLocalsCount() - 1;
        encoder->EncodeStp(codegen->FpReg(), lr,
                           MemRef(codegen->FpReg(), -fl.GetOffset<CFrameLayout::FP, CFrameLayout::BYTES>(slot)));

        FixIntervals(codegen, encoder);
        encoder->EncodeJump(label_);
    }

    SaveStateInst *GetInst()
    {
        return save_state_;
    }

    auto &GetLabel()
    {
        return label_;
    }

private:
    LabelHolder::LabelId label_;
    SaveStateInst *save_state_ {nullptr};
};

static VRegInfo::Type IrTypeToMetainfoType(DataType::Type type)
{
    switch (type) {
        case DataType::UINT64:
        case DataType::INT64:
            return VRegInfo::Type::INT64;
        case DataType::ANY:
            return VRegInfo::Type::ANY;
        case DataType::UINT32:
        case DataType::UINT16:
        case DataType::UINT8:
        case DataType::INT32:
        case DataType::INT16:
        case DataType::INT8:
            return VRegInfo::Type::INT32;
        case DataType::FLOAT64:
            return VRegInfo::Type::FLOAT64;
        case DataType::FLOAT32:
            return VRegInfo::Type::FLOAT32;
        case DataType::BOOL:
            return VRegInfo::Type::BOOL;
        case DataType::REFERENCE:
            return VRegInfo::Type::OBJECT;
        default:
            UNREACHABLE();
    }
}

size_t Codegen::GetLanguageExtensionOffsetFromSpInBytes()
{
    auto frame_layout = GetFrameLayout();
    size_t lang_ext_slots_count =
        GetGraph()->GetRuntime()->GetLanguageExtensionSize() / PointerSize(GetGraph()->GetArch());
    return frame_layout.GetSpillOffsetFromSpInBytes(lang_ext_slots_count + GetGraph()->GetExtSlotsStart() - 1);
}

Codegen::Codegen(Graph *graph)
    : Optimization(graph),
      allocator_(graph->GetAllocator()),
      local_allocator_(graph->GetLocalAllocator()),
      code_builder_(allocator_->New<CodeInfoBuilder>(graph->GetArch(), allocator_)),
      slow_paths_(graph->GetLocalAllocator()->Adapter()),
      slow_paths_map_(graph->GetLocalAllocator()->Adapter()),
      frame_layout_(CFrameLayout(graph->GetArch(), graph->GetStackSlotsCount())),
      osr_entries_(graph->GetLocalAllocator()->Adapter()),
      vreg_indices_(GetAllocator()->Adapter()),
      runtime_(graph->GetRuntime()),
      target_(graph->GetArch()),
      live_outs_(graph->GetLocalAllocator()->Adapter()),
      disasm_(this),
      spill_fills_resolver_(graph)
{
    graph->SetCodeBuilder(code_builder_);
    regfile_ = graph->GetRegisters();
    if (regfile_ != nullptr) {
        ASSERT(regfile_->IsValid());
        ArenaVector<Reg> regs_usage(allocator_->Adapter());
        Convert(&regs_usage, graph->GetUsedRegs<DataType::INT64>(), INT64_TYPE);
        Convert(&regs_usage, graph->GetUsedRegs<DataType::FLOAT64>(), FLOAT64_TYPE);
        regfile_->SetUsedRegs(regs_usage);
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
        method_id_ = graph->GetRuntime()->GetMethodId(method);
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
    frame->SetFrameSize(fl.GetFrameSize<CFrameLayout::BYTES>());
    frame->SetSpillsCount(fl.GetSpillsCount());

    frame->SetCallersOffset(
        fl.GetOffset<CFrameLayout::SP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
    frame->SetFpCallersOffset(
        fl.GetOffset<CFrameLayout::SP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
    frame->SetCalleesOffset(
        -fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
    frame->SetFpCalleesOffset(
        -fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

    frame->SetSetupFrame(true);
    frame->SetSaveFrameAndLinkRegs(true);
    frame->SetSaveUnusedCalleeRegs(!GetGraph()->GetMethodProperties().GetCompactPrologueAllowed());
    frame->SetAdjustSpReg(true);
    frame->SetHasFloatRegs(GetGraph()->HasFloatRegs());

    GetCodeBuilder()->SetHasFloatRegs(GetGraph()->HasFloatRegs());

    SetFrameInfo(frame);
}

void Codegen::FillOnlyParameters(RegMask *live_regs, uint32_t num_params) const
{
    ASSERT(num_params <= 6U);
    if (GetArch() == Arch::AARCH64) {
        num_params = AlignUp(num_params, 2U);
    }
    *live_regs &= GetTarget().GetParamRegsMask(num_params);
}

void Codegen::Convert(ArenaVector<Reg> *regs_usage, const ArenaVector<bool> *mask, TypeInfo type_info)
{
    ASSERT(regs_usage != nullptr);
    // There are no used registers
    if (mask == nullptr) {
        return;
    }
    ASSERT(mask->size() == MAX_NUM_REGS);
    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if ((*mask)[i]) {
            regs_usage->emplace_back(i, type_info);
        }
    }
}

#ifdef INTRINSIC_SLOW_PATH_ENTRY_ENABLED
void Codegen::CreateIrtocIntrinsic(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS src,
                                   [[maybe_unused]] RegMask *lvrmask)
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
        default:
            UNREACHABLE();
            break;
    }
}
#endif

bool Codegen::BeginMethod()
{
    // Do not try to encode too large graph
    auto inst_size = GetGraph()->GetCurrentInstructionId();
    auto insts_per_byte = GetEncoder()->MaxArchInstPerEncoded();
    auto max_bits_in_inst = GetInstructionSizeBits(GetArch());
    inst_size += slow_paths_.size() * INST_IN_SLOW_PATH;
    if ((inst_size * insts_per_byte * max_bits_in_inst) > options.GetCompilerMaxGenCodeSize()) {
        return false;
    }
    // After this - encoder aborted, if allocated too much size.
    GetEncoder()->SetMaxAllocatedBytes(options.GetCompilerMaxGenCodeSize());

    if (GetGraph()->IsAotMode()) {
        GetEncoder()->SetCodeOffset(GetGraph()->GetAotData()->GetCodeOffset() + CodeInfo::GetCodeOffset(GetArch()));
    } else {
        GetEncoder()->SetCodeOffset(0);
    }

    code_builder_->BeginMethod(GetFrameLayout().GetFrameSize<CFrameLayout::BYTES>(), GetGraph()->GetVRegsCount());

    GetEncoder()->BindLabel(GetLabelEntry());
    SetStartCodeOffset(GetEncoder()->GetCursorOffset());

    GeneratePrologue();

    return GetEncoder()->GetResult();
}

void Codegen::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "Method Prologue");

    GetCallingConvention()->GeneratePrologue(*frame_info_);

    if (!GetGraph()->GetMode().IsNative()) {
        GetEncoder()->EncodeSti(Imm(static_cast<uint8_t>(1)),
                                MemRef(ThreadReg(), GetRuntime()->GetTlsFrameKindOffset(GetArch())));
    }
    if (GetGraph()->IsDynamicMethod()) {
        GenerateExtensionsForPrologue();
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
        InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)), method_reg,
                     Imm(static_cast<size_t>(events::MethodEnterKind::COMPILED))});
    } else {
        InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                     Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())),
                     Imm(static_cast<size_t>(events::MethodEnterKind::COMPILED))});
    }
#endif
}

void Codegen::GenerateEpilogue()
{
    SCOPED_DISASM_STR(this, "Method Epilogue");

    if (GetGraph()->IsDynamicMethod()) {
        GenerateExtensionsForEpilogue();
    }

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
    GetCallingConvention()->GenerateEpilogue(*frame_info_, [this]() {
        if (GetGraph()->IsAotMode()) {
            ScopedTmpReg method_reg(GetEncoder());
            LoadMethod(method_reg);
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_EXIT)), method_reg,
                         Imm(static_cast<size_t>(events::MethodExitKind::COMPILED))});
        } else {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_EXIT)),
                         Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())),
                         Imm(static_cast<size_t>(events::MethodExitKind::COMPILED))});
        }
    });
#else
    GetCallingConvention()->GenerateEpilogue(*frame_info_, []() {});
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

    auto insts_per_byte = GetEncoder()->MaxArchInstPerEncoded();
    auto max_bits_in_inst = GetInstructionSizeBits(GetArch());
    auto inst_size = GetGraph()->GetCurrentInstructionId() + slow_paths_.size() * INST_IN_SLOW_PATH;
    if ((inst_size * insts_per_byte * max_bits_in_inst) > options.GetCompilerMaxGenCodeSize()) {
        return false;
    }

    EmitSlowPaths();
    visitor_ = nullptr;

    return true;
}

void Codegen::EmitJump(const BasicBlock *bb)
{
    BasicBlock *suc_bb = nullptr;

    if (bb->GetLastInst() == nullptr) {
        ASSERT(bb->IsEmpty());
        suc_bb = bb->GetSuccsBlocks()[0];
    } else {
        switch (bb->GetLastInst()->GetOpcode()) {
            case Opcode::If:
            case Opcode::IfImm:
                ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
                suc_bb = bb->GetFalseSuccessor();
                break;
            default:
                suc_bb = bb->GetSuccsBlocks()[0];
                break;
        }
    }
    SCOPED_DISASM_STR(this, std::string("Jump from  BB_") + std::to_string(bb->GetId()) + " to BB_" +
                                std::to_string(suc_bb->GetId()));

    auto label = suc_bb->GetId();
    GetEncoder()->EncodeJump(label);
}

void Codegen::EndMethod()
{
    for (auto &osr_stub : osr_entries_) {
        SCOPED_DISASM_STR(this,
                          std::string("Osr stub for OsrStateStump ") + std::to_string(osr_stub->GetInst()->GetId()));
        osr_stub->Generate(this);
    }

    GetEncoder()->Finalize();
}

// Allocates memory, copies generated code to it, sets the code to the graph's codegen data. Also this function
// encodes the code info and sets it to the graph.
bool Codegen::CopyToCodeCache()
{
    auto code_entry = reinterpret_cast<void *>(GetEncoder()->GetLabelAddress(GetLabelEntry()));
    auto code_size = GetEncoder()->GetCursorOffset();
    bool save_all_callee_registers = !GetGraph()->GetMethodProperties().GetCompactPrologueAllowed();

    auto code = reinterpret_cast<uint8_t *>(GetAllocator()->Alloc(code_size));
    if (code == nullptr) {
        return false;
    }
    memcpy_s(code, code_size, code_entry, code_size);
    GetGraph()->SetData(EncodeDataType(code, code_size));

    RegMask callee_regs;
    VRegMask callee_vregs;
    GetRegfile()->FillUsedCalleeSavedRegisters(&callee_regs, &callee_vregs, save_all_callee_registers);
    constexpr size_t MAX_NUM_REGISTERS = 32;
    static_assert(MAX_NUM_REGS <= MAX_NUM_REGISTERS && MAX_NUM_VREGS <= MAX_NUM_REGISTERS);
    code_builder_->SetSavedCalleeRegsMask(static_cast<uint32_t>(callee_regs.to_ulong()),
                                          static_cast<uint32_t>(callee_vregs.to_ulong()));

    ArenaVector<uint8_t> code_info_data(GetGraph()->GetAllocator()->Adapter());
    code_builder_->Encode(&code_info_data);
    GetGraph()->SetCodeInfo(Span(code_info_data));

    return true;
}

bool Codegen::RunImpl()
{
    Initialize();

    auto encoder = GetEncoder();
    encoder->GetLabels()->CreateLabels(GetGraph()->GetVectorBlocks().size());
    label_entry_ = encoder->CreateLabel();
    label_exit_ = encoder->CreateLabel();

#ifndef NDEBUG
    if (options.IsCompilerNonOptimizing()) {
        // In case of non-optimizing compiler lowering pass is not run but low-level instructions may
        // also appear on codegen so, to satisfy GraphChecker, the flag should be raised.
        GetGraph()->SetLowLevelInstructionsEnabled();
    }
#endif  // NDEBUG

    if ((GetCallingConvention() == nullptr) || (GetEncoder() == nullptr)) {
        return false;
    }

    if (!GetEncoder()->GetResult()) {
        return false;
    }

    // Remove registers from the temp registers, if they are in the regalloc mask, i.e. available for regalloc.
    auto used_regs = ~GetGraph()->GetArchUsedRegs();
    auto forbidden_temps = used_regs & GetTarget().GetTempRegsMask();
    if (forbidden_temps.Any()) {
        for (size_t i = forbidden_temps.GetMinRegister(); i <= forbidden_temps.GetMaxRegister(); i++) {
            if (forbidden_temps[i]) {
                encoder->AcquireScratchRegister(Reg(i, INT64_TYPE));
            }
        }
    }

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
        if (GetGraph()->GetMode().SupportManagedCode() && options.IsCompilerDisasmDumpCodeInfo()) {
            GetDisasm()->PrintCodeInfo(this);
        }
        GetDisasm()->PrintCodeStatistics(this);
    }

    return true;
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

    bool has_calls = false;

    for (auto bb : GetGraph()->GetBlocksLinearOrder()) {
        // Calls may be in the middle of method
        for (auto inst : bb->Insts()) {
            // For throw instruction need jump2runtime same way
            if (inst->IsCall() || inst->GetOpcode() == Opcode::Throw) {
                has_calls = true;
                break;
            }
        }
        if (has_calls) {
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
    auto fill_mask = [](RegMask *mask, auto *vector) {
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
    fill_mask(&used_regs_, GetGraph()->GetUsedRegs<DataType::INT64>());
    fill_mask(&used_vregs_, GetGraph()->GetUsedRegs<DataType::FLOAT64>());
    used_vregs_ &= GetTarget().GetAvailableVRegsMask();
    used_regs_ &= GetTarget().GetAvailableRegsMask();
    used_regs_ &= ~GetGraph()->GetArchUsedRegs();
    used_vregs_ &= ~GetGraph()->GetArchUsedVRegs();

    /* Remove used registers from Encoder's scratch registers */
    RegMask used_temps = used_regs_ & GetTarget().GetTempRegsMask();
    if (used_temps.any()) {
        for (size_t i = 0; i < used_temps.size(); i++) {
            if (used_temps[i]) {
                GetEncoder()->AcquireScratchRegister(Reg(i, INT64_TYPE));
            }
        }
    }
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

Imm Codegen::ConvertImm(uint64_t imm, DataType::Type type)
{
    switch (type) {
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::BOOL:
        case DataType::UINT8:
            return Imm(bit_cast<int8_t, uint8_t>(imm));
        case DataType::INT8:
            return Imm(bit_cast<int8_t, uint8_t>(imm));
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::UINT16:
            return Imm(bit_cast<int16_t, uint16_t>(imm));
        case DataType::INT16:
            return Imm(bit_cast<int16_t, uint16_t>(imm));
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::UINT32:
            return Imm(bit_cast<int32_t, uint32_t>(imm));
        case DataType::INT32:
            return Imm(bit_cast<int32_t, uint32_t>(imm));
        case DataType::ANY:
            return Imm(bit_cast<int64_t, uint64_t>(imm));
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case DataType::UINT64:
            return Imm(bit_cast<int64_t, uint64_t>(imm));
        case DataType::INT64:
            return Imm(bit_cast<int64_t, uint64_t>(imm));
        case DataType::FLOAT32:
            return Imm(bit_cast<float, uint32_t>(static_cast<uint32_t>(imm)));
        case DataType::FLOAT64:
            return Imm(bit_cast<double, uint64_t>(imm));
        case DataType::REFERENCE:
            if (imm == 0) {
                return Imm(0);
            }
            [[fallthrough]]; /* fall-through */
        default:
            // Invalid converted immediate
            UNREACHABLE();
    }
    return INVALID_IMM;
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
    return INVALID_IMM;
}

Imm Codegen::ConvertImm(ConstantInst *const_inst, DataType::Type type)
{
    switch (type) {
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
            return Imm(static_cast<int32_t>(const_inst->GetIntValue()));
        case DataType::UINT64:
        case DataType::INT64:
            if (const_inst->GetType() == DataType::ANY) {
                return Imm(const_inst->GetRawValue());
            }
            return Imm(static_cast<int64_t>(const_inst->GetIntValue()));
        case DataType::FLOAT32:
            return Imm(const_inst->GetFloatValue());
        case DataType::FLOAT64:
            return Imm(const_inst->GetDoubleValue());
        case DataType::ANY:
            return Imm(const_inst->GetRawValue());
        default:
            // Invalid converted immediate
            UNREACHABLE();
    }
    return INVALID_IMM;
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
    for (auto slow_path : slow_paths_) {
        slow_path->Generate(this);
    }
}

void Codegen::CreateStackMap(Inst *inst, Inst *user)
{
    SaveStateInst *save_state = nullptr;
    if (inst->IsSaveState()) {
        save_state = static_cast<SaveStateInst *>(inst);
    } else {
        save_state = inst->GetSaveState();
    }
    ASSERT(save_state != nullptr);

    bool require_vreg_map = inst->RequireRegMap();
    uint32_t outer_bpc = inst->GetPc();
    for (auto call_inst = save_state->GetCallerInst(); call_inst != nullptr;
         call_inst = call_inst->GetSaveState()->GetCallerInst()) {
        outer_bpc = call_inst->GetPc();
    }

    code_builder_->BeginStackMap(outer_bpc, GetEncoder()->GetCursorOffset(), save_state->GetRootsStackMask(),
                                 save_state->GetRootsRegsMask().to_ulong(), require_vreg_map,
                                 save_state->GetOpcode() == Opcode::SaveStateOsr);
    if (user == nullptr) {
        user = inst;
        if (inst == save_state && inst->HasUsers()) {
            for (auto &u : inst->GetUsers()) {
                if (u.GetInst()->GetOpcode() != Opcode::ReturnInlined) {
                    user = u.GetInst();
                    break;
                }
            }
        }
    }
    CreateStackMapRec(save_state, require_vreg_map, user);

    code_builder_->EndStackMap();
    if (GetDisasm()->IsEnabled()) {
        GetDisasm()->PrintStackMap(this);
    }
}

void Codegen::CreateStackMapRec(SaveStateInst *save_state, bool require_vreg_map, Inst *target_site)
{
    bool has_inline_info = save_state->GetCallerInst() != nullptr;
    size_t vregs_count = 0;
    if (require_vreg_map) {
        auto runtime = GetRuntime();
        if (auto caller = save_state->GetCallerInst()) {
            vregs_count = runtime->GetMethodRegistersCount(caller->GetCallMethod()) +
                          runtime->GetMethodArgumentsCount(caller->GetCallMethod()) + 1;  // 1 for accumulator
        } else {
            vregs_count = runtime->GetMethodRegistersCount(save_state->GetMethod()) +
                          runtime->GetMethodArgumentsCount(save_state->GetMethod()) + 1;  // 1 for accumulator
        }
#ifndef NDEBUG
        ASSERT_PRINT(!save_state->GetInputsWereDeleted(), "Some vregs were deleted from the save state");
#endif
    }

    if (auto call_inst = save_state->GetCallerInst()) {
        CreateStackMapRec(call_inst->GetSaveState(), require_vreg_map, target_site);
        auto method = GetGraph()->IsAotMode() ? nullptr : call_inst->GetCallMethod();
        code_builder_->BeginInlineInfo(method, GetRuntime()->GetMethodId(call_inst->GetCallMethod()),
                                       save_state->GetPc(), vregs_count);
    }

    if (require_vreg_map) {
        CreateVRegMap(save_state, vregs_count, target_site);
    }

    if (has_inline_info) {
        code_builder_->EndInlineInfo();
    }
}

void Codegen::CreateVRegMap(SaveStateInst *save_state, size_t vregs_count, Inst *target_site)
{
    vreg_indices_.clear();
    vreg_indices_.resize(vregs_count, {-1, -1});
    FillVregIndices(save_state);

    ASSERT(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    auto target_life_number = la.GetInstLifeIntervals(target_site)->GetBegin();

    for (auto &input_index : vreg_indices_) {
        if (input_index.first == -1 && input_index.second == -1) {
            code_builder_->AddVReg(VRegInfo());
            continue;
        }
        if (input_index.second != -1) {
            auto imm = save_state->GetImmediate(input_index.second);
            code_builder_->AddConstant(imm.value, IrTypeToMetainfoType(imm.type), imm.is_acc);
            continue;
        }
        ASSERT(input_index.first != -1);
        auto vreg = save_state->GetVirtualRegister(input_index.first);
        auto input_inst = save_state->GetDataFlowInput(input_index.first);
        auto interval = la.GetInstLifeIntervals(input_inst)->FindSiblingAt(target_life_number);
        ASSERT(interval != nullptr);
        CreateVreg(interval->GetLocation(), input_inst, vreg);
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
            code_builder_->AddVReg(VRegInfo(GetFrameLayout().GetStackArgsStartSlot() - slot - CFrameSlots::Start(),
                                            VRegInfo::Location::SLOT, IrTypeToMetainfoType(inst->GetType()),
                                            vreg.IsAccumulator()));
            break;
        }
        case LocationType::STACK: {
            auto slot = location.GetValue();
            if (!Is64BitsArch(GetArch())) {
                slot = ((location.GetValue() << 1U) + 1);
            }
            code_builder_->AddVReg(VRegInfo(GetFrameLayout().GetFirstSpillSlot() + slot, VRegInfo::Location::SLOT,
                                            IrTypeToMetainfoType(inst->GetType()), vreg.IsAccumulator()));
            break;
        }
        case LocationType::IMMEDIATE: {
            ASSERT(inst->IsConst());
            code_builder_->AddConstant(inst->CastToConstant()->GetRawValue(), IrTypeToMetainfoType(inst->GetType()),
                                       vreg.IsAccumulator());
            break;
        }
        default:
            // Reg-to-reg spill fill must not occurs within SaveState
            UNREACHABLE();
    }
}

void Codegen::FillVregIndices(SaveStateInst *save_state)
{
    for (size_t i = 0; i < save_state->GetInputsCount(); ++i) {
        size_t vreg_index = save_state->GetVirtualRegister(i).Value();
        ASSERT(vreg_index < vreg_indices_.size());
        vreg_indices_[vreg_index].first = i;
    }
    for (size_t i = 0; i < save_state->GetImmediatesCount(); i++) {
        auto vreg_imm = save_state->GetImmediate(i);
        ASSERT(vreg_imm.vreg < vreg_indices_.size());
        ASSERT(vreg_indices_[vreg_imm.vreg].first == -1);
        vreg_indices_[vreg_imm.vreg].second = i;
    }
}

void Codegen::CreateVRegForRegister(const Location &location, Inst *inst, const VirtualRegister &vreg)
{
    bool is_osr = GetGraph()->IsOsrMode();
    bool is_fp = (location.GetKind() == LocationType::FP_REGISTER);
    auto reg_num = location.GetValue();
    auto reg = Reg(reg_num, is_fp ? FLOAT64_TYPE : INT64_TYPE);
    if (!is_osr && GetRegfile()->GetZeroReg() == reg) {
        code_builder_->AddConstant(0, IrTypeToMetainfoType(inst->GetType()), vreg.IsAccumulator());
    } else if (is_osr || GetRegfile()->IsCalleeRegister(reg)) {
        if (is_fp) {
            ASSERT(inst->GetType() != DataType::REFERENCE);
            ASSERT(is_osr || reg_num >= GetFirstCalleeReg(GetArch(), true));
            code_builder_->AddVReg(VRegInfo(reg_num, VRegInfo::Location::FP_REGISTER,
                                            IrTypeToMetainfoType(inst->GetType()), vreg.IsAccumulator()));
        } else {
            ASSERT(is_osr || reg_num >= GetFirstCalleeReg(GetArch(), false));
            code_builder_->AddVReg(VRegInfo(reg_num, VRegInfo::Location::REGISTER,
                                            IrTypeToMetainfoType(inst->GetType()), vreg.IsAccumulator()));
        }
    } else {
        ASSERT(reg_num >= GetFirstCallerReg(GetArch(), is_fp));
        auto last_slot = GetFrameLayout().GetCallerLastSlot(is_fp);
        reg_num -= GetFirstCallerReg(GetArch(), is_fp);
        code_builder_->AddVReg(VRegInfo(last_slot - reg_num, VRegInfo::Location::SLOT,
                                        IrTypeToMetainfoType(inst->GetType()), vreg.IsAccumulator()));
    }
}

void Codegen::CreateOsrEntry(SaveStateInst *save_state)
{
    auto &stub = osr_entries_.emplace_back(GetAllocator()->New<OsrEntryStub>(this, save_state));
    GetEncoder()->BindLabel(stub->GetLabel());
}

/**
 * Insert tracing code to the generated code. See `Trace` method in the `runtime/entrypoints.cpp`.
 * TODO(compiler): we should rework parameters assigning algorithm, that is duplicated here.
 * @param params parameters to be passed to the TRACE entrypoint, first parameter must be TraceId value.
 */
void Codegen::InsertTrace(std::initializer_list<std::variant<Reg, Imm>> params)
{
    SCOPED_DISASM_STR(this, "Trace");
    [[maybe_unused]] constexpr size_t MAX_PARAM_NUM = 8;
    ASSERT(params.size() <= MAX_PARAM_NUM);
    auto regfile = GetRegfile();
    auto save_regs = regfile->GetCallerSavedRegMask();
    save_regs.set(GetTarget().GetReturnRegId());
    auto save_vregs = regfile->GetCallerSavedVRegMask();
    save_vregs.set(GetTarget().GetReturnFpRegId());

    SaveCallerRegisters(save_regs, save_vregs, false);
    FillCallParams(params);
    EmitCallRuntimeCode(nullptr, EntrypointId::TRACE);
    LoadCallerRegisters(save_regs, save_vregs, false);
}

void Codegen::CallIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId id)
{
    SCOPED_DISASM_STR(this, "CallIntrinsic");
    if (GetGraph()->IsAotMode()) {
        auto aot_data = GetGraph()->GetAotData();
        intptr_t offset = aot_data->GetEntrypointOffset(GetEncoder()->GetCursorOffset(), static_cast<int32_t>(id));
        GetEncoder()->MakeCallAot(offset);
    } else {
        GetEncoder()->MakeCall(
            reinterpret_cast<const void *>(GetRuntime()->GetIntrinsicAddress(inst->IsRuntimeCall(), id)));
    }
}

void Codegen::FillCallParams(const std::initializer_list<std::variant<Reg, Imm>> &params)
{
    SCOPED_DISASM_STR(this, "FillCallParams");
    // Native call - do not add reserve parameters
    auto param_info = GetCallingConvention()->GetParameterInfo(0);
    ArenaVector<std::pair<Reg, Imm>> immediates(GetLocalAllocator()->Adapter());
    ArenaVector<Reg> sp_moves(GetLocalAllocator()->Adapter());
    auto reg_moves = GetGraph()->CreateInstSpillFill();
    for (auto &param : params) {
        Location dst;
        if (std::holds_alternative<Reg>(param)) {
            auto reg = std::get<Reg>(param);
            auto type = reg.GetType().ToDataType();
            dst = param_info->GetNextLocation(type);
            if (reg == SpReg()) {
                // SP should be handled separately, since on the ARM64 target it has ID out of range
                sp_moves.emplace_back(ConvertRegister(dst.GetValue(), type));
            } else {
                reg_moves->AddSpillFill(Location::MakeRegister(reg.GetId(), type), dst, type);
            }
        } else {
            auto imm = std::get<Imm>(param);
            auto type = imm.GetType().ToDataType();
            dst = param_info->GetNextLocation(type);
            auto reg = ConvertRegister(dst.GetValue(), type);
            immediates.emplace_back(reg, imm);
        }

        if (dst.IsStackArgument()) {
            GetEncoder()->SetFalseResult();
            UNREACHABLE();  // Move to BoundaryFrame
        }
    }

    // Resolve registers move order and encode
    spill_fills_resolver_.ResolveIfRequired(reg_moves);
    SpillFillEncoder(this, reg_moves).EncodeSpillFill();

    // Encode immediates moves
    for (auto &imm_values : immediates) {
        GetEncoder()->EncodeMov(imm_values.first, imm_values.second);
    }

    // Encode moves from SP reg
    for (auto dst : sp_moves) {
        GetEncoder()->EncodeMov(dst, SpReg());
    }
}

bool Codegen::EmitCallRuntimeCode(Inst *inst, EntrypointId id)
{
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), id));
    auto encoder = GetEncoder();
    encoder->MakeCall(entry);

    SaveStateInst *save_state =
        (inst == nullptr || inst->IsSaveState()) ? static_cast<SaveStateInst *>(inst) : inst->GetSaveState();
    // StackMap should follow the call as the bridge function expects retaddr points to the stackmap
    if (save_state != nullptr) {
        CreateStackMap(inst);
    }

    if (GetRuntime()->IsEntrypointNoreturn(id)) {
        if constexpr (DEBUG_BUILD) {  // NOLINT
            encoder->EncodeAbort();
        }
        return false;
    }
    ASSERT(save_state == nullptr || inst->IsRuntimeCall());

    return true;
}

[[maybe_unused]] static bool EnsureParamsFitIn32Bit(std::initializer_list<std::variant<Reg, Imm>> params)
{
    for (auto &param : params) {
        if (std::holds_alternative<Reg>(param)) {
            auto reg = std::get<Reg>(param);
            if (reg.GetSize() > WORD_SIZE) {
                return false;
            }
        } else {
            auto imm = std::get<Imm>(param);
            if (imm.GetSize() > WORD_SIZE) {
                return false;
            }
        }
    }
    return true;
}

void Codegen::CallRuntime(Inst *inst, EntrypointId id, Reg dst_reg,
                          std::initializer_list<std::variant<Reg, Imm>> params, RegMask preserved_regs)
{
    ASSERT(inst != nullptr);
    CHECK_EQ(params.size(), GetRuntime()->GetEntrypointArgsNum(id));
    if (GetArch() == Arch::AARCH32) {
        // There is a problem with 64-bit parameters:
        // params number passed from entrypoints_gen.S.erb will be inconsistent with Aarch32 ABI.
        // Thus, runtime bridges will have wrong params number (\paramsnum macro argument).
        ASSERT(EnsureParamsFitIn32Bit(params));
        ASSERT(!dst_reg.IsValid() || dst_reg.GetSize() <= WORD_SIZE);
    }

    SCOPED_DISASM_STR(this, std::string("CallRuntime: ") + GetRuntime()->GetEntrypointName(id));
    RegMask live_regs {preserved_regs | GetLiveRegisters(inst).first};
    RegMask params_mask;
    if (inst->HasImplicitRuntimeCall() && !GetRuntime()->IsEntrypointNoreturn(id)) {
        SaveRegistersForImplicitRuntime(inst, &params_mask, &live_regs);
    }
    // parameter regs: their initial values must be stored by the caller
    // Other caller regs stored in bridges
    FillOnlyParameters(&live_regs, params.size());
    SaveCallerRegisters(live_regs, VRegMask(), true);

    if (params.size() != 0) {
        FillCallParams(params);
    }

    // Call Code
    if (!EmitCallRuntimeCode(inst, id)) {
        return;
    }
    if (dst_reg.IsValid()) {
        ASSERT(dst_reg.IsScalar());
        Reg ret_val = GetTarget().GetReturnReg(dst_reg.GetType());
        if (dst_reg.GetId() != ret_val.GetId()) {
            GetEncoder()->SetRegister(&live_regs, nullptr, ret_val, true);
        }

        // We must:
        //  sign extended INT8 and INT16 to INT32
        //  zero extended UINT8 and UINT16 to UINT32
        if (dst_reg.GetSize() < WORD_SIZE) {
            bool is_signed = DataType::IsTypeSigned(inst->GetType());
            GetEncoder()->EncodeCast(dst_reg, is_signed, ret_val, is_signed);
        } else {
            GetEncoder()->EncodeMov(dst_reg, ret_val);
        }
    }

    GetEncoder()->SetRegister(&live_regs, nullptr, dst_reg, false);
    LoadCallerRegisters(live_regs, VRegMask(), true);

    if (!inst->HasImplicitRuntimeCall()) {
        return;
    }
    ASSERT(!GetRuntime()->IsEntrypointNoreturn(id));
    for (auto i = 0U; i < params_mask.size(); i++) {
        if (params_mask.test(i)) {
            inst->GetSaveState()->GetRootsRegsMask().reset(i);
        }
    }
}

void Codegen::SaveRegistersForImplicitRuntime(Inst *inst, RegMask *params_mask, RegMask *mask)
{
    auto &roots_mask = inst->GetSaveState()->GetRootsRegsMask();
    for (auto i = 0U; i < inst->GetInputsCount(); i++) {
        auto location = inst->GetLocation(i);
        if (location.IsRegister() && location.IsRegisterValid()) {
            auto val = location.GetValue();
            auto reg = Reg(val, INT64_TYPE);
            GetEncoder()->SetRegister(mask, nullptr, reg, true);
            if (DataType::IsReference(inst->GetInputType(i)) && !roots_mask.test(val)) {
                params_mask->set(val);
                roots_mask.set(val);
            }
        }
    }
}

void Codegen::CreateCheckForTLABWithConstSize([[maybe_unused]] Inst *inst, Reg reg_tlab_start, Reg reg_tlab_size,
                                              size_t size, LabelHolder::LabelId label)
{
    SCOPED_DISASM_STR(this, "CreateCheckForTLABWithConstSize");
    auto encoder = GetEncoder();
    if (encoder->CanEncodeImmAddSubCmp(size, WORD_SIZE, false)) {
        encoder->EncodeJump(label, reg_tlab_size, Imm(size), Condition::LO);
        // Change pointer to start free memory
        encoder->EncodeAdd(reg_tlab_size, reg_tlab_start, Imm(size));
    } else {
        ScopedTmpReg size_reg(encoder);
        encoder->EncodeMov(size_reg, Imm(size));
        encoder->EncodeJump(label, reg_tlab_size, size_reg, Condition::LO);
        encoder->EncodeAdd(reg_tlab_size, reg_tlab_start, size_reg);
    }
}

void Codegen::CreateDebugRuntimeCallsForNewObject(Inst *inst, [[maybe_unused]] Reg reg_tlab_start, size_t alloc_size,
                                                  RegMask preserved)
{
#ifndef NDEBUG
    if (options.IsCompilerEnableTlabEvents()) {
        static constexpr size_t type_index = 1;

        InsertTrace({Imm(static_cast<size_t>(TraceId::TLAB_EVENT)), Imm(type_index), reg_tlab_start, Imm(alloc_size)});
    }
#endif
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON)
    CallRuntime(inst, EntrypointId::ANNOTATE_SANITIZERS, INVALID_REGISTER, {reg_tlab_start, Imm(alloc_size)},
                preserved);
#endif
    if (GetRuntime()->IsTrackTlabAlloc()) {
        CallRuntime(inst, EntrypointId::WRITE_TLAB_STATS, INVALID_REGISTER, {Imm(alloc_size)}, preserved);
    }
}

void Codegen::CreateNewObjCall(NewObjectInst *new_obj)
{
    auto dst = ConvertRegister(new_obj->GetDstReg(), new_obj->GetType());
    auto src = ConvertRegister(new_obj->GetSrcReg(0), new_obj->GetInputType(0));
    auto init_class = new_obj->GetInput(0).GetInst();
    auto src_class = ConvertRegister(new_obj->GetSrcReg(0), DataType::POINTER);
    auto runtime = GetRuntime();
    auto encoder = GetEncoder();

    auto max_tlab_size = runtime->GetTLABMaxSize();

    if (max_tlab_size == 0 || init_class->GetOpcode() != Opcode::LoadAndInitClass) {
        CallRuntime(new_obj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, {src});
        return;
    }
    auto klass = init_class->CastToLoadAndInitClass()->GetClass();
    if (klass == nullptr || !runtime->CanUseTlabForClass(klass)) {
        CallRuntime(new_obj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, {src});
        return;
    }
    auto class_size = runtime->GetClassSize(klass);
    auto alignment = runtime->GetTLABAlignment();

    class_size = (class_size & ~(alignment - 1U)) + ((class_size % alignment) != 0U ? alignment : 0U);
    if (class_size > max_tlab_size) {
        CallRuntime(new_obj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, {src});
        return;
    }

    auto param_regs {GetLiveRegisters(new_obj).first};
    param_regs &= GetTarget().GetParamRegsMask(2U);

    SaveCallerRegisters(param_regs, VRegMask(), false);

    FillCallParams({src_class, Imm(class_size)});
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), EntrypointId::ALLOCATE_OBJECT_TLAB));
    encoder->MakeCall(entry);
    CreateStackMap(new_obj);

    GetEncoder()->EncodeMov(dst, GetTarget().GetReturnReg(dst.GetType()));
    param_regs.reset(dst.GetId());
    LoadCallerRegisters(param_regs, VRegMask(), false);
}

void Codegen::CreateNewObjCallOld(NewObjectInst *new_obj)
{
    auto dst = ConvertRegister(new_obj->GetDstReg(), new_obj->GetType());
    auto src = ConvertRegister(new_obj->GetSrcReg(0), new_obj->GetInputType(0));
    auto init_class = new_obj->GetInput(0).GetInst();
    auto runtime = GetRuntime();
    auto max_tlab_size = runtime->GetTLABMaxSize();
    auto encoder = GetEncoder();

    if (max_tlab_size == 0 || init_class->GetOpcode() != Opcode::LoadAndInitClass) {
        CallRuntime(new_obj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, {src});
        return;
    }
    auto klass = init_class->CastToLoadAndInitClass()->GetClass();
    if (klass == nullptr || !runtime->CanUseTlabForClass(klass)) {
        CallRuntime(new_obj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, {src});
        return;
    }
    auto class_size = runtime->GetClassSize(klass);
    auto alignment = runtime->GetTLABAlignment();

    class_size = (class_size & ~(alignment - 1U)) + ((class_size % alignment) != 0U ? alignment : 0U);
    if (class_size > max_tlab_size) {
        CallRuntime(new_obj, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, {src});
        return;
    }
    ScopedLiveTmpReg reg_tlab_start(encoder);
    ScopedLiveTmpReg reg_tlab_size(encoder);

    auto slow_path = CreateSlowPath<SlowPathEntrypoint>(new_obj, EntrypointId::CREATE_OBJECT_BY_CLASS);
    CreateLoadTLABInformation(reg_tlab_start, reg_tlab_size);
    CreateCheckForTLABWithConstSize(new_obj, reg_tlab_start, reg_tlab_size, class_size, slow_path->GetLabel());

    RegMask preserved_regs;
    encoder->SetRegister(&preserved_regs, nullptr, src);
    CreateDebugRuntimeCallsForNewObject(new_obj, reg_tlab_start, reinterpret_cast<size_t>(class_size), preserved_regs);

    ScopedTmpReg tlab_reg(encoder);
    // Load pointer to tlab
    encoder->EncodeLdr(tlab_reg, false, MemRef(ThreadReg(), runtime->GetCurrentTLABOffset(GetArch())));

    // Store pointer to the class
    encoder->EncodeStr(src, MemRef(reg_tlab_start, runtime->GetObjClassOffset(GetArch())));
    encoder->EncodeMov(dst, reg_tlab_start);
    reg_tlab_start.Release();
    // Store new pointer to start free memory in TLAB
    encoder->EncodeStrRelease(reg_tlab_size, MemRef(tlab_reg, runtime->GetTLABFreePointerOffset(GetArch())));
    slow_path->BindBackLabel(encoder);
}

void Codegen::LoadClassFromObject(Reg class_reg, Reg obj_reg)
{
    Reg reg = ConvertRegister(class_reg.GetId(), DataType::REFERENCE);
    GetEncoder()->EncodeLdr(reg, false, MemRef(obj_reg, GetRuntime()->GetObjClassOffset(GetArch())));
}

void Codegen::CreateMultiArrayCall(CallInst *call_inst)
{
    SCOPED_DISASM_STR(this, "Create Call for MultiArray");

    auto dst_reg = ConvertRegister(call_inst->GetDstReg(), call_inst->GetType());
    auto num_args = call_inst->GetInputsCount() - 2U;  // first is class, last is save_state

    ScopedTmpReg class_reg(GetEncoder());
    auto class_type = ConvertDataType(DataType::REFERENCE, GetArch());
    Reg class_orig = class_reg.GetReg().As(class_type);
    auto location = call_inst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

    GetEncoder()->EncodeMov(class_orig, ConvertRegister(location.GetValue(), DataType::INT32));
    CallRuntime(call_inst, EntrypointId::CREATE_MULTI_ARRAY, dst_reg, {class_reg, Imm(num_args), SpReg()});
}

void Codegen::CreateJumpToClassResolverPltShared(Inst *inst, Reg tmp_reg, RuntimeInterface::EntrypointId id)
{
    auto encoder = GetEncoder();
    auto graph = GetGraph();
    auto aot_data = graph->GetAotData();
    auto offset = aot_data->GetSharedSlowPathOffset(id, encoder->GetCursorOffset());
    if (offset == 0 || !encoder->CanMakeCallByOffset(offset)) {
        SlowPathShared *slow_path;
        auto search = slow_paths_map_.find(id);
        if (search != slow_paths_map_.end()) {
            slow_path = search->second;
            ASSERT(slow_path->GetTmpReg().GetId() == tmp_reg.GetId());
        } else {
            slow_path = CreateSlowPath<SlowPathShared>(inst, id);
            slow_path->SetTmpReg(tmp_reg);
            slow_paths_map_[id] = slow_path;
        }
        encoder->MakeCall(slow_path->GetLabel());
    } else {
        encoder->MakeCallByOffset(offset);
    }
    CreateStackMap(inst);
}

void Codegen::CreateLoadClassFromPLT(Inst *inst, Reg tmp_reg, Reg dst, size_t class_id)
{
    auto encoder = GetEncoder();
    auto graph = GetGraph();
    auto aot_data = graph->GetAotData();
    intptr_t offset = aot_data->GetClassSlotOffset(encoder->GetCursorOffset(), class_id, false);
    auto label = encoder->CreateLabel();
    ASSERT(tmp_reg.GetId() != dst.GetId());
    ASSERT(inst->IsRuntimeCall());
    encoder->MakeLoadAotTableAddr(offset, tmp_reg, dst);
    encoder->EncodeJump(label, dst, Condition::NE);

    // PLT Class Resolver has special calling convention:
    // First encoder temporary (tmp_reg) works as parameter and return value
    CHECK_EQ(tmp_reg.GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());

    CreateJumpToClassResolverPltShared(inst, tmp_reg, EntrypointId::CLASS_RESOLVER);

    encoder->EncodeMov(dst, tmp_reg);
    encoder->BindLabel(label);
}

void Codegen::CreateLoadTLABInformation(Reg reg_tlab_start, Reg reg_tlab_size)
{
    SCOPED_DISASM_STR(this, "LoadTLABInformation");
    auto runtime = GetRuntime();
    // Load pointer to tlab
    GetEncoder()->EncodeLdr(reg_tlab_size, false, MemRef(ThreadReg(), runtime->GetCurrentTLABOffset(GetArch())));
    // Load pointer to start free memory in TLAB
    GetEncoder()->EncodeLdr(reg_tlab_start, false, MemRef(reg_tlab_size, runtime->GetTLABFreePointerOffset(GetArch())));
    // Load pointer to end free memory in TLAB
    GetEncoder()->EncodeLdr(reg_tlab_size, false, MemRef(reg_tlab_size, runtime->GetTLABEndPointerOffset(GetArch())));
    // Calculate size of the free memory
    GetEncoder()->EncodeSub(reg_tlab_size, reg_tlab_size, reg_tlab_start);
}

// The function alignment up the value from alignment_reg using tmp_reg.
void Codegen::CreateAlignmentValue(Reg alignment_reg, Reg tmp_reg, size_t alignment)
{
    auto and_val = ~(alignment - 1U);
    // zeroed lower bits
    GetEncoder()->EncodeAnd(tmp_reg, alignment_reg, Imm(and_val));
    GetEncoder()->EncodeSub(alignment_reg, alignment_reg, tmp_reg);

    auto end_label = GetEncoder()->CreateLabel();

    // if zeroed value is different, add alignment
    GetEncoder()->EncodeJump(end_label, alignment_reg, Condition::EQ);
    GetEncoder()->EncodeAdd(tmp_reg, tmp_reg, Imm(alignment));

    GetEncoder()->BindLabel(end_label);
    GetEncoder()->EncodeMov(alignment_reg, tmp_reg);
}

void Codegen::EmitGetUnresolvedCalleeMethod(CallInst *call_inst)
{
    Reg param_0 = GetTarget().GetParamReg(0);
    if (GetGraph()->IsAotMode()) {
        // This temporary register is needed, because the stack slot for param_0 will be overwritten in runtime call
        // by pointer to the method(LoadMethod below)
        ScopedTmpReg tmp(GetEncoder());
        LoadMethod(tmp);
        CallRuntime(call_inst, EntrypointId::GET_UNKNOWN_CALLEE_METHOD, tmp.GetReg(),
                    {tmp.GetReg(), Imm(call_inst->GetCallMethodId()), Imm(0)});
        GetEncoder()->EncodeMov(param_0, tmp.GetReg());
        return;
    }

    ScopedTmpReg tmp(GetEncoder());
    auto utypes = GetRuntime()->GetUnresolvedTypes();
    auto skind = UnresolvedTypesInterface::SlotKind::METHOD;
    auto method_addr = utypes->GetTableSlot(call_inst->GetCallMethod(), call_inst->GetCallMethodId(), skind);
    GetEncoder()->EncodeMov(tmp.GetReg(), Imm(method_addr));
    GetEncoder()->EncodeLdr(tmp.GetReg(), false, MemRef(tmp.GetReg()));
    auto slow_path = CreateSlowPath<SlowPathUnresolved>(call_inst, EntrypointId::GET_UNKNOWN_CALLEE_METHOD);
    slow_path->SetUnresolvedType(call_inst->GetCallMethod(), call_inst->GetCallMethodId());
    slow_path->SetDstReg(tmp.GetReg());
    slow_path->SetSlotAddr(method_addr);
    GetEncoder()->EncodeJump(slow_path->GetLabel(), tmp.GetReg(), Condition::EQ);
    slow_path->BindBackLabel(GetEncoder());
    GetEncoder()->EncodeMov(param_0, tmp.GetReg());
}

void Codegen::EmitCreateCallCode(CallInst *call_inst)
{
    ASSERT(call_inst->IsStaticCall());
    Reg param_0 = GetTarget().GetParamReg(0);
    if (!call_inst->IsUnresolved() && call_inst->GetCallMethod() == GetGraph()->GetMethod() &&
        GetArch() != Arch::AARCH32 && !GetGraph()->IsOsrMode() && !GetGraph()->GetMethodProperties().GetHasDeopt()) {
        if (GetGraph()->IsAotMode()) {
            LoadMethod(param_0);
        } else {
            GetEncoder()->EncodeMov(param_0, Imm(reinterpret_cast<size_t>(GetGraph()->GetMethod())));
        }
        GetEncoder()->MakeCallByOffset(GetStartCodeOffset() - GetEncoder()->GetCursorOffset());
    } else {
        if (call_inst->IsUnresolved()) {
            EmitGetUnresolvedCalleeMethod(call_inst);
        } else if (GetGraph()->IsAotMode()) {
            auto aot_data = GetGraph()->GetAotData();
            intptr_t offset = aot_data->GetPltSlotOffset(GetEncoder()->GetCursorOffset(), call_inst->GetCallMethodId());
            // PLT CallStatic Resolver transparently uses param_0 (Method) register
            GetEncoder()->MakeLoadAotTable(offset, param_0);
        } else {  // usual JIT case
            auto method = call_inst->GetCallMethod();
            GetEncoder()->EncodeMov(param_0, Imm(reinterpret_cast<size_t>(method)));
        }
        size_t entry_point_offset = GetRuntime()->GetCompiledEntryPointOffset(GetArch());
        GetEncoder()->MakeCall(MemRef(param_0, entry_point_offset));
    }
}

void Codegen::EmitEpilogueForCreateCall(CallInst *call_inst)
{
    CreateStackMap(call_inst);

    // Restore frame pointer in the TLS
    GetEncoder()->EncodeStr(FpReg(), MemRef(ThreadReg(), GetRuntime()->GetTlsFrameOffset(GetArch())));

    auto dst_reg = ConvertRegister(call_inst->GetDstReg(), call_inst->GetType());
    if (dst_reg.IsValid()) {
        auto arch = GetArch();
        auto return_type = call_inst->GetType();
        auto return_reg = GetTarget().GetReturnReg(dst_reg.GetType());
        // We must:
        //  sign extended INT8 and INT16 to INT32
        //  zero extended UINT8 and UINT16 to UINT32
        if (DataType::ShiftByType(return_type, arch) < DataType::ShiftByType(DataType::INT32, arch)) {
            bool is_signed = DataType::IsTypeSigned(return_type);
            GetEncoder()->EncodeCast(dst_reg, is_signed, Reg(return_reg.GetId(), INT32_TYPE), is_signed);
        } else {
            GetEncoder()->EncodeMov(dst_reg, return_reg);
        }
    }
}

void Codegen::CreateCall(CallInst *call_inst)
{
    if (call_inst->GetSaveState() != nullptr && call_inst->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                         Imm(reinterpret_cast<size_t>(call_inst->GetCallMethod())),
                         Imm(static_cast<size_t>(events::MethodEnterKind::INLINED))});
        }
#endif
        return;
    }
    SCOPED_DISASM_STR(this, "Create Call for method pointer");
    ASSERT(!HasLiveCallerSavedRegs(call_inst));
    // TODO(igorban) support MakeCallByOffset in Aarch32Encoder(support adr instruction)
    EmitCreateCallCode(call_inst);
    EmitEpilogueForCreateCall(call_inst);
}

inline Reg GetObjectReg(Codegen *codegen, CallInst *call_inst)
{
    auto location = call_inst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());
    auto obj_reg = codegen->ConvertRegister(location.GetValue(), call_inst->GetInputType(0));
    ASSERT(obj_reg != INVALID_REGISTER);
    return obj_reg;
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
        EncodeImms(inst->GetImms());
    }
    CallIntrinsic(inst, inst->GetIntrinsicId());

    if (inst->GetSaveState() != nullptr) {
        // StackMap should follow the call as the bridge function expects retaddr points to the stackmap
        CreateStackMap(inst);
    }

    if (inst->GetType() != DataType::VOID) {
        auto arch = GetArch();
        auto return_type = inst->GetType();
        auto dst_reg = ConvertRegister(inst->GetDstReg(), inst->GetType());
        auto return_reg = GetTarget().GetReturnReg(dst_reg.GetType());
        // We must:
        //  sign extended INT8 and INT16 to INT32
        //  zero extended UINT8 and UINT16 to UINT32
        if (DataType::ShiftByType(return_type, arch) < DataType::ShiftByType(DataType::INT32, arch)) {
            bool is_signed = DataType::IsTypeSigned(return_type);
            GetEncoder()->EncodeCast(dst_reg, is_signed, Reg(return_reg.GetId(), INT32_TYPE), is_signed);
        } else {
            GetEncoder()->EncodeMov(dst_reg, return_reg);
        }
    }
}

uint32_t Codegen::GetVtableShift()
{
    // The size of the VTable element is equal to the size of pointers for the architecture
    // (not the size of pointer to objects)
    constexpr uint32_t SHIFT_64_BITS = 3;
    constexpr uint32_t SHIFT_32_BITS = 2;
    return Is64BitsArch(GetGraph()->GetArch()) ? SHIFT_64_BITS : SHIFT_32_BITS;
}

void Codegen::CreateUnresolvedVirtualMethodLoad(CallInst *vcall, Reg method)
{
    SCOPED_DISASM_STR(this, "Create Call To ResolveUnknownVirtualCall");
    ASSERT(vcall->GetOpcode() == Opcode::UnresolvedCallVirtual);

    ScopedTmpReg tmp_reg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
    Reg obj_reg = GetObjectReg(this, vcall);

    ASSERT(vcall->GetCallMethod() != nullptr);
    if (GetGraph()->IsAotMode()) {
        // TODO(zhroma): how to make more optimal?
        LoadMethod(method);
        CallRuntime(vcall, EntrypointId::RESOLVE_VIRTUAL_CALL_AOT, method,
                    {method, obj_reg, Imm(vcall->GetCallMethodId()), Imm(0)});
    } else {
        auto runtime = GetRuntime();
        auto utypes = runtime->GetUnresolvedTypes();
        auto skind = UnresolvedTypesInterface::SlotKind::VIRTUAL_METHOD;
        // Try to load vtable index
        auto method_slot_addr = utypes->GetTableSlot(vcall->GetCallMethod(), vcall->GetCallMethodId(), skind);
        GetEncoder()->EncodeMov(method, Imm(method_slot_addr));
        GetEncoder()->EncodeLdr(method, false, MemRef(method));

        // 0 means the virtual call is uninitialized or it is an interface call
        auto entrypoint = EntrypointId::RESOLVE_UNKNOWN_VIRTUAL_CALL;
        auto slow_path = CreateSlowPath<SlowPathUnresolved>(vcall, entrypoint);
        slow_path->SetUnresolvedType(vcall->GetCallMethod(), vcall->GetCallMethodId());
        slow_path->SetDstReg(method);
        slow_path->SetArgReg(obj_reg);
        slow_path->SetSlotAddr(method_slot_addr);
        GetEncoder()->EncodeJump(slow_path->GetLabel(), method, Condition::EQ);

        // Load klass into tmp_reg
        LoadClassFromObject(tmp_reg, obj_reg);

        auto tmp_reg_64 = Reg(tmp_reg.GetReg().GetId(), INT64_TYPE);
        // Load from VTable, address = (klass + (index << shift)) + vtable_offset
        GetEncoder()->EncodeAdd(tmp_reg_64, tmp_reg, Shift(method, GetVtableShift()));
        GetEncoder()->EncodeLdr(method, false,
                                MemRef(tmp_reg_64, runtime->GetVTableOffset(GetArch()) - (1U << GetVtableShift())));
        slow_path->BindBackLabel(GetEncoder());
    }
}

void Codegen::PrepareCallVirtualAot(CallInst *call_inst, Reg method_reg)
{
    auto runtime = GetRuntime();
    SCOPED_DISASM_STR(this, "AOT CallVirtual using PLT-GOT");
    ScopedTmpReg tmp_reg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
    ASSERT(call_inst->IsRuntimeCall());

    auto method_reg_64 = Reg(method_reg.GetId(), INT64_TYPE);
    auto tmp_reg_64 = Reg(tmp_reg.GetReg().GetId(), INT64_TYPE);

    auto aot_data = GetGraph()->GetAotData();
    intptr_t offset = aot_data->GetVirtIndexSlotOffset(GetEncoder()->GetCursorOffset(), call_inst->GetCallMethodId());
    GetEncoder()->MakeLoadAotTableAddr(offset, tmp_reg_64, method_reg_64);

    auto label = GetEncoder()->CreateLabel();
    GetEncoder()->EncodeJump(label, method_reg, Condition::NE);

    GetEncoder()->EncodeMov(method_reg_64, tmp_reg_64);

    // PLT CallVirtual Resolver has very special calling convention:
    // * First encoder temporary (method_reg) works as a parameter and return value
    CHECK_EQ(method_reg_64.GetId(), GetTarget().GetTempRegsMask().GetMinRegister());
    MemRef entry(ThreadReg(), runtime->GetEntrypointTlsOffset(GetArch(), EntrypointId::CALL_VIRTUAL_RESOLVER));
    GetEncoder()->MakeCall(entry);

    // Need a stackmap to build correct boundary frame
    CreateStackMap(call_inst);
    GetEncoder()->BindLabel(label);

    // Load klass into method_reg
    Reg obj_reg = GetObjectReg(this, call_inst);
    LoadClassFromObject(tmp_reg, obj_reg);

    // Load from VTable, address = (klass + (index << shift)) + vtable_offset
    GetEncoder()->EncodeAdd(method_reg, tmp_reg_64, Shift(method_reg, GetVtableShift()));
    GetEncoder()->EncodeLdr(method_reg, false,
                            MemRef(method_reg, runtime->GetVTableOffset(GetArch()) - (1U << GetVtableShift())));
}

void Codegen::PrepareCallVirtual(CallInst *call_inst, Reg method_reg)
{
    auto obj_reg = GetObjectReg(this, call_inst);
    ASSERT(!RegisterKeepCallArgument(call_inst, method_reg));
    LoadClassFromObject(method_reg, obj_reg);
    // Get index
    auto runtime = GetRuntime();
    auto vtable_index = runtime->GetVTableIndex(call_inst->GetCallMethod());
    // Load from VTable, address = klass + ((index << shift) + vtable_offset)
    auto total_offset = runtime->GetVTableOffset(GetArch()) + (vtable_index << GetVtableShift());
    // Class ref was loaded to method_reg
    GetEncoder()->EncodeLdr(method_reg, false, MemRef(method_reg, total_offset));
}

void Codegen::PrepareAndEmitCallVirtual(CallInst *call_inst)
{
    auto runtime = GetRuntime();
    if (call_inst->IsUnresolved()) {
        ScopedTmpReg method_reg(GetEncoder());
        CreateUnresolvedVirtualMethodLoad(call_inst, method_reg);
        EmitCallVirtual(method_reg);
    } else if (runtime->IsInterfaceMethod(call_inst->GetCallMethod())) {
        SCOPED_DISASM_STR(this, "Create Call To ResolveVirtualCall");
        ScopedTmpReg tmp_reg(GetEncoder(), ConvertDataType(DataType::REFERENCE, GetArch()));
        Reg obj_reg = GetObjectReg(this, call_inst);
        ScopedTmpReg method_reg(GetEncoder());
        if (GetGraph()->IsAotMode()) {
            if (GetArch() == Arch::AARCH64) {
                IntfInlineCachePass(call_inst, method_reg, tmp_reg, obj_reg);
            } else {
                LoadMethod(method_reg);
                CallRuntime(call_inst, EntrypointId::RESOLVE_VIRTUAL_CALL_AOT, method_reg,
                            {method_reg, obj_reg, Imm(call_inst->GetCallMethodId()), Imm(0)});
            }
        } else {
            CallRuntime(call_inst, EntrypointId::RESOLVE_VIRTUAL_CALL, method_reg,
                        {Imm(reinterpret_cast<size_t>(call_inst->GetCallMethod())), obj_reg});
        }
        EmitCallVirtual(method_reg);
    } else if (GetGraph()->IsAotMode() && !GetGraph()->GetAotData()->GetUseCha()) {
        ScopedTmpReg method_reg(GetEncoder());
        PrepareCallVirtualAot(call_inst, method_reg);
        EmitCallVirtual(method_reg);
    } else {  // JIT mode
        Reg method_reg = GetTarget().GetParamReg(0);
        PrepareCallVirtual(call_inst, method_reg);
        GetEncoder()->MakeCall(MemRef(method_reg, runtime->GetCompiledEntryPointOffset(GetArch())));
    }
}

void Codegen::IntfInlineCachePass(CallInst *call_inst, Reg method_reg, Reg tmp_reg, Reg obj_reg)
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
    auto aot_data = GetGraph()->GetAotData();
    uint64_t intf_inline_cache_index = aot_data->GetIntfInlineCacheIndex();
    // TODO(liyiming): do LoadMethod in irtoc to reduce use tmp reg
    if (obj_reg.GetId() != tmp_reg.GetId()) {
        auto reg_tmp_64 = tmp_reg.As(INT64_TYPE);
        uint64_t offset =
            aot_data->GetInfInlineCacheSlotOffset(GetEncoder()->GetCursorOffset(), intf_inline_cache_index);
        GetEncoder()->MakeLoadAotTableAddr(offset, reg_tmp_64, INVALID_REGISTER);
        LoadMethod(method_reg);
        CallRuntime(call_inst, EntrypointId::INTF_INLINE_CACHE, method_reg,
                    {method_reg, obj_reg, Imm(call_inst->GetCallMethodId()), reg_tmp_64});
    } else {
        // we don't have tmp reg here, so use x3 directly
        constexpr uint32_t REG_3 = 3;
        Reg reg_3 = Reg(REG_3, INT64_TYPE);
        ScopedTmpRegF64 vtmp(GetEncoder());
        GetEncoder()->EncodeMov(vtmp, reg_3);
        uint64_t offset =
            aot_data->GetInfInlineCacheSlotOffset(GetEncoder()->GetCursorOffset(), intf_inline_cache_index);
        GetEncoder()->MakeLoadAotTableAddr(offset, reg_3, INVALID_REGISTER);
        LoadMethod(method_reg);
        CallRuntime(call_inst, EntrypointId::INTF_INLINE_CACHE, method_reg,
                    {method_reg, obj_reg, Imm(call_inst->GetCallMethodId()), reg_3});
        GetEncoder()->EncodeMov(reg_3, vtmp);
    }

    intf_inline_cache_index++;
    aot_data->SetIntfInlineCacheIndex(intf_inline_cache_index);
}

void Codegen::EmitCallVirtual(Reg method_reg)
{
    Reg param_0 = GetTarget().GetParamReg(0);
    // Set method
    GetEncoder()->EncodeMov(param_0, method_reg);

    size_t entry_point_offset = GetRuntime()->GetCompiledEntryPointOffset(GetArch());
    GetEncoder()->MakeCall(MemRef(param_0, entry_point_offset));
}

void Codegen::AddParamRegsInLiveMasks(RegMask *live_regs, VRegMask *live_vregs,
                                      const std::initializer_list<std::variant<Reg, Imm>> &params)
{
    auto enc = GetEncoder();
    auto callconv = GetCallingConvention();
    auto param_info = callconv->GetParameterInfo(0);
    for (auto &param : params) {
        if (std::holds_alternative<Reg>(param)) {
            auto reg = std::get<Reg>(param);
            auto curr_dst = param_info->GetNativeParam(reg.GetType());
            if (std::holds_alternative<Reg>(curr_dst)) {
                auto r = std::get<Reg>(curr_dst);
                if (r.IsScalar()) {
                    live_regs->set(r.GetId());
                } else {
                    live_vregs->set(r.GetId());
                }
            } else {
                enc->SetFalseResult();
                UNREACHABLE();
            }
        } else {
            auto imm = std::get<Imm>(param);
            auto curr_dst = param_info->GetNativeParam(imm.GetType());
            if (std::holds_alternative<Reg>(curr_dst)) {
                auto reg = std::get<Reg>(curr_dst);
                if (reg.IsScalar()) {
                    live_regs->set(reg.GetId());
                } else {
                    live_vregs->set(reg.GetId());
                }
            } else {
                enc->SetFalseResult();
                UNREACHABLE();
            }
        }
    }
}

void Codegen::CreateStubCall(Inst *inst, RuntimeInterface::IntrinsicId intrinsicId, Reg dst,
                             const std::initializer_list<std::variant<Reg, Imm>> &params)
{
    VRegMask live_vregs;
    RegMask live_regs;
    AddParamRegsInLiveMasks(&live_regs, &live_vregs, params);
    auto enc = GetEncoder();
    {
        SCOPED_DISASM_STR(this, "Save caller saved regs");
        SaveCallerRegisters(live_regs, live_vregs, true);
    }

    FillCallParams(params);
    CallIntrinsic(inst, intrinsicId);

    if (inst->GetSaveState() != nullptr) {
        CreateStackMap(inst);
    }

    if (dst.IsValid()) {
        Reg ret_val = GetTarget().GetReturnReg(dst.GetType());
        if (dst.GetId() != ret_val.GetId()) {
            enc->SetRegister(&live_regs, &live_vregs, ret_val, true);
        }
        ASSERT(dst.IsScalar());
        enc->EncodeMov(dst, ret_val);
    }

    {
        SCOPED_DISASM_STR(this, "Restore caller saved regs");
        enc->SetRegister(&live_regs, &live_vregs, dst, false);
        LoadCallerRegisters(live_regs, live_vregs, true);
    }
}

void Codegen::CreateVirtualCall(CallInst *call_inst)
{
    SCOPED_DISASM_STR(this, "Create Call for virtual method");
    if (call_inst->GetSaveState() != nullptr && call_inst->IsInlined()) {
#if defined(EVENT_METHOD_ENTER_ENABLED) && EVENT_METHOD_ENTER_ENABLED != 0
        if (!GetGraph()->IsAotMode()) {
            InsertTrace({Imm(static_cast<size_t>(TraceId::METHOD_ENTER)),
                         Imm(reinterpret_cast<size_t>(call_inst->GetCallMethod())),
                         Imm(static_cast<size_t>(events::MethodEnterKind::INLINED))});
        }
#endif
        return;
    }
    ASSERT(!HasLiveCallerSavedRegs(call_inst));
    PrepareAndEmitCallVirtual(call_inst);
    EmitEpilogueForCreateCall(call_inst);
}

void Codegen::CreateDynamicCall(CallInst *call_inst)
{
    SCOPED_DISASM_STR(this, "Create a dynamic call");
    RuntimeInterface *runtime = GetRuntime();
    Encoder *encoder = GetEncoder();

    auto dst_reg = ConvertRegister(call_inst->GetDstReg(), call_inst->GetType());
    Reg param_0 = GetTarget().GetParamReg(0);
    Reg native_ptr_reg = Reg(param_0.GetId(), TypeInfo(static_cast<object_pointer_type>(0)));
    Reg param_num_args = GetTarget().GetParamReg(1);
    Reg param_func_obj = GetTarget().GetParamReg(2);

    ASSERT(!HasLiveCallerSavedRegs(call_inst));
    // TODO(audovichenko): Check the following
    // * func_obj's tag is object
    // * func_obj's class is dynamic
    // * target method is not null

    // Load method into param0 as follow:
    // param_0 = param_func_obj->GetMethod()->GetExternalPointer()
    // native_ptr_reg is a (may be) smaller view of param_0 register.
    encoder->EncodeLdr(native_ptr_reg, false, MemRef(param_func_obj, runtime->GetFunctionTargetOffset(GetArch())));
    encoder->EncodeLdr(param_0, false, MemRef(param_0, runtime->GetNativePointerTargetOffset(GetArch())));

    ASSERT(call_inst->GetInputsCount() > 1);
    auto num_args = static_cast<uint32_t>(
        call_inst->GetInputsCount() -
        2);  // '-2' means 1 for spill fill input and 1 for function object which should not be counted
    encoder->EncodeMov(param_num_args, Imm(num_args));

    size_t entry_point_offset = runtime->GetCompiledEntryPointOffset(GetArch());
    encoder->MakeCall(MemRef(param_0, entry_point_offset));

    CreateStackMap(call_inst);

    if (dst_reg.IsValid()) {
        Reg ret_reg = GetTarget().GetReturnReg(dst_reg.GetType());
        encoder->EncodeMov(dst_reg, ret_reg);
    }
}

template <typename T>
static T GetBarrierOperandValue(RuntimeInterface *runtime, panda::mem::BarrierPosition position, std::string_view name)
{
    auto operand = runtime->GetBarrierOperand(position, name);
    return std::get<T>(operand.GetValue());
}

void Codegen::CallBarrier(RegMask live_regs, VRegMask live_vregs, EntrypointId id,
                          const std::initializer_list<std::variant<Reg, Imm>> &params)
{
    SaveCallerRegisters(live_regs, live_vregs, true);
    FillCallParams(params);
    EmitCallRuntimeCode(nullptr, id);
    LoadCallerRegisters(live_regs, live_vregs, true);
}

void Codegen::CreatePreWRB(Inst *inst, MemRef mem, bool store_pair)
{
    auto runtime = GetRuntime();
    auto *enc = GetEncoder();

    auto barrier_type = runtime->GetPreType();
    if (barrier_type == panda::mem::BarrierType::PRE_WRB_NONE) {
        return;
    }
    SCOPED_DISASM_STR(this, "Pre WRB");
    ASSERT(barrier_type == panda::mem::BarrierType::PRE_SATB_BARRIER);
    ScopedTmpReg tmp(enc);
    if (GetGraph()->IsOfflineCompilationMode()) {
        GetEncoder()->EncodeLdr(tmp, false, MemRef(ThreadReg(), runtime->GetTlsConcurrentMarkingAddrOffset(GetArch())));
    } else {
        auto concurrent_marker = reinterpret_cast<uintptr_t>(GetBarrierOperandValue<std::atomic<bool> *>(
            runtime, panda::mem::BarrierPosition::BARRIER_POSITION_PRE, "CONCURRENT_MARKING_ADDR"));
        enc->EncodeMov(tmp, Imm(concurrent_marker));
    }
    // Check marker
    auto marker_mem = MemRef(tmp);
    auto tmp_b = ConvertRegister(tmp.GetReg().GetId(), DataType::INT8);
    enc->EncodeLdr(tmp_b, false, marker_mem);
    auto label = GetEncoder()->CreateLabel();
    enc->EncodeJump(label, tmp_b, Condition::EQ);
    auto ref_type =
        inst->GetType() == DataType::ANY ? DataType::INT64 : DataType::GetIntTypeForReference(enc->GetArch());
    auto tmp_ref = ConvertRegister(tmp.GetReg().GetId(), ref_type);
    auto prev_offset = enc->GetCursorOffset();
    // Load old value
    if (IsVolatileMemInst(inst)) {
        enc->EncodeLdrAcquire(tmp_ref, false, mem);
    } else {
        enc->EncodeLdr(tmp_ref, false, mem);
    }
    TryInsertImplicitNullCheck(inst, prev_offset);
    enc->EncodeJump(label, tmp_ref, Condition::EQ);
    auto [live_regs, live_vregs] = GetLiveRegisters<true>(inst);
    CallBarrier(live_regs, live_vregs, EntrypointId::PRE_WRB_FUNC_NO_BRIDGE, {tmp_ref});

    if (store_pair) {
        // store pair doesn't support index and scalar
        ASSERT(!mem.HasIndex() && !mem.HasScale());
        // calculate offset for second store
        auto second_offset = 1U << DataType::ShiftByType(DataType::REFERENCE, enc->GetArch());
        if (mem.HasDisp()) {
            second_offset += mem.GetDisp();
        }
        // Load old value
        if (IsVolatileMemInst(inst)) {
            enc->EncodeLdrAcquire(tmp_ref, false, MemRef(mem.GetBase(), second_offset));
        } else {
            enc->EncodeLdr(tmp_ref, false, MemRef(mem.GetBase(), second_offset));
        }
        enc->EncodeJump(label, tmp_ref, Condition::EQ);
        CallBarrier(live_regs, live_vregs, EntrypointId::PRE_WRB_FUNC_NO_BRIDGE, {tmp_ref});
    }
    enc->BindLabel(label);
}

void Codegen::EncodePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool check_nullptr)
{
    ASSERT(reg1 != INVALID_REGISTER);
    auto barrier_type = GetRuntime()->GetPostType();
    ASSERT(barrier_type == panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER ||
           barrier_type == panda::mem::BarrierType::POST_INTERREGION_BARRIER);

    if (barrier_type == panda::mem::BarrierType::POST_INTERREGION_BARRIER) {
        CreatePostInterRegionBarrier(inst, mem, reg1, reg2, check_nullptr);
        return;
    }
    SCOPED_DISASM_STR(this, "Post WRB");
    CreatePostInterGenerationalBarrier(mem);
}

void Codegen::CreatePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2)
{
    ASSERT(reg1 != INVALID_REGISTER);
    ASSERT(!mem.HasIndex() && !mem.HasDisp() && !mem.HasScale());

    auto barrier_type = GetRuntime()->GetPostType();
    if (barrier_type == panda::mem::BarrierType::POST_WRB_NONE) {
        return;
    }
    ASSERT(barrier_type == panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER ||
           barrier_type == panda::mem::BarrierType::POST_INTERREGION_BARRIER);

    Inst *second_value;
    Inst *val = InstStoredValue(inst, &second_value);
    ASSERT(second_value == nullptr || reg2 != INVALID_REGISTER);
    if (val->GetOpcode() == Opcode::NullPtr) {
        // We can don't encode Post barrier for nullptr
        if (second_value == nullptr || second_value->GetOpcode() == Opcode::NullPtr) {
            return;
        }
        // CallPostWRB only for second reg
        EncodePostWRB(inst, mem, reg2, INVALID_REGISTER, !IsInstNotNull(second_value));
        return;
    }
    // Create PostWRB only for first value
    if (second_value != nullptr && second_value->GetOpcode() == Opcode::NullPtr) {
        reg2 = INVALID_REGISTER;
    }
    bool check_nullptr = true;
    if (reg2 == INVALID_REGISTER) {
        if (IsInstNotNull(val)) {
            check_nullptr = false;
        }
    } else {
        if (IsInstNotNull(val) && IsInstNotNull(second_value)) {
            check_nullptr = false;
        }
    }
    EncodePostWRB(inst, mem, reg1, reg2, check_nullptr);
}

void Codegen::CreatePostInterRegionBarrier(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool check_nullptr)
{
    SCOPED_DISASM_STR(this, "Post IR-WRB");
    auto *enc = GetEncoder();
    ASSERT(GetRuntime()->GetPostType() == panda::mem::BarrierType::POST_INTERREGION_BARRIER);
    ASSERT(reg1 != INVALID_REGISTER);

    auto label = GetEncoder()->CreateLabel();

    if (check_nullptr) {
        enc->EncodeJump(label, reg1, Condition::EQ);
    }

    ScopedTmpReg tmp1(enc, ConvertDataType(DataType::REFERENCE, GetArch()));
    reg1 = ConvertRegister(reg1.GetId(), DataType::REFERENCE);
    if (reg2.IsValid()) {
        reg2 = ConvertRegister(reg2.GetId(), DataType::REFERENCE);
    }
    auto mem_reg = ConvertRegister(mem.GetBase().GetId(), DataType::REFERENCE);

    auto region_size_bit = GetBarrierOperandValue<uint8_t>(
        GetRuntime(), panda::mem::BarrierPosition::BARRIER_POSITION_POST, "REGION_SIZE_BITS");
    // Compare first store value with mem
    enc->EncodeXor(tmp1, mem_reg, reg1);
    enc->EncodeShr(tmp1, tmp1, Imm(region_size_bit));

    enc->EncodeJump(label, tmp1, Condition::EQ);
    tmp1.Release();
    auto [live_regs, live_vregs] = GetLiveRegisters<true>(inst);

    CallBarrier(live_regs, live_vregs, EntrypointId::POST_WRB_UPDATE_CARD_FUNC_NO_BRIDGE, {mem_reg, reg1});
    enc->BindLabel(label);

    if (reg2.IsValid() && reg1 != reg2) {
        auto label1 = GetEncoder()->CreateLabel();
        if (check_nullptr) {
            enc->EncodeJump(label1, reg2, Condition::EQ);
        }

        ScopedTmpReg tmp2(enc, ConvertDataType(DataType::REFERENCE, GetArch()));
        // Compare second store value with mem
        enc->EncodeXor(tmp2, mem_reg, reg2);
        enc->EncodeShr(tmp2, tmp2, Imm(region_size_bit));
        enc->EncodeJump(label1, tmp2, Condition::EQ);
        tmp2.Release();
        CallBarrier(live_regs, live_vregs, EntrypointId::POST_WRB_UPDATE_CARD_FUNC_NO_BRIDGE, {mem_reg, reg2});
        enc->BindLabel(label1);
    }
}

void Codegen::CalculateCardIndex(MemRef mem, ScopedTmpReg *tmp, ScopedTmpReg *tmp1)
{
    auto tmp_type = tmp->GetReg().GetType();
    auto tmp1_type = tmp1->GetReg().GetType();
    auto *enc = GetEncoder();
    auto card_bits =
        GetBarrierOperandValue<uint8_t>(GetRuntime(), panda::mem::BarrierPosition::BARRIER_POSITION_POST, "CARD_BITS");

    auto base_reg = mem.GetBase();
    ASSERT(base_reg != INVALID_REGISTER);
    if (base_reg.GetSize() < Reg(*tmp).GetSize()) {
        tmp->ChangeType(base_reg.GetType());
        tmp1->ChangeType(base_reg.GetType());
    }
    enc->EncodeSub(*tmp, base_reg, *tmp);
    ASSERT(!mem.HasDisp() && !mem.HasIndex());
    enc->EncodeShr(*tmp, *tmp, Imm(card_bits));
    tmp->ChangeType(tmp_type);
    tmp1->ChangeType(tmp1_type);
}

void Codegen::CreatePostInterGenerationalBarrier(MemRef mem)
{
    auto runtime = GetRuntime();
    auto *enc = GetEncoder();
    ASSERT(runtime->GetPostType() == panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER);
    ScopedTmpReg tmp(enc);
    ScopedTmpReg tmp1(enc);
    // * load AddressOf(MIN_ADDR) -> min_addr
    if (GetGraph()->IsOfflineCompilationMode()) {
        GetEncoder()->EncodeLdr(tmp, false, MemRef(ThreadReg(), runtime->GetTlsCardTableMinAddrOffset(GetArch())));
    } else {
        auto min_address = reinterpret_cast<uintptr_t>(
            GetBarrierOperandValue<void *>(runtime, panda::mem::BarrierPosition::BARRIER_POSITION_POST, "MIN_ADDR"));
        enc->EncodeMov(tmp, Imm(min_address));
    }
    enc->EncodeLdr(tmp, false, MemRef(tmp));
    // * card_index = (AddressOf(obj.field) - min_addr) >> CARD_BITS   // shift right
    CalculateCardIndex(mem, &tmp, &tmp1);
    // * load AddressOf(CARD_TABLE_ADDR) -> card_table_addr
    if (GetGraph()->IsOfflineCompilationMode()) {
        GetEncoder()->EncodeLdr(tmp1.GetReg().As(INT64_TYPE), false,
                                MemRef(ThreadReg(), runtime->GetTlsCardTableAddrOffset(GetArch())));
    } else {
        auto card_table_addr = reinterpret_cast<uintptr_t>(GetBarrierOperandValue<uint8_t *>(
            runtime, panda::mem::BarrierPosition::BARRIER_POSITION_POST, "CARD_TABLE_ADDR"));
        enc->EncodeMov(tmp1, Imm(card_table_addr));
    }
    // * card_addr = card_table_addr + card_index
    enc->EncodeAdd(tmp, tmp1, tmp);
    // * store card_addr <- DIRTY_VAL
    auto dirty_val =
        GetBarrierOperandValue<uint8_t>(runtime, panda::mem::BarrierPosition::BARRIER_POSITION_POST, "DIRTY_VAL");

    auto tmp1_b = ConvertRegister(tmp1.GetReg().GetId(), DataType::INT8);
    enc->EncodeMov(tmp1_b, Imm(dirty_val));
    enc->EncodeStr(tmp1_b, MemRef(tmp));
}

bool Codegen::HasLiveCallerSavedRegs(Inst *inst)
{
    auto [live_regs, live_fp_regs] = GetLiveRegisters<false>(inst);
    live_regs &= GetCallerRegsMask(GetArch(), false);
    live_fp_regs &= GetCallerRegsMask(GetArch(), true);
    return live_regs.Any() || live_fp_regs.Any();
}

void Codegen::SaveCallerRegisters(RegMask live_regs, VRegMask live_vregs, bool adjust_regs)
{
    SCOPED_DISASM_STR(this, "Save caller saved regs");
    auto base = GetFrameInfo()->GetCallersRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
    live_regs &= ~GetEncoder()->GetAvailableScratchRegisters();
    live_vregs &= ~GetEncoder()->GetAvailableScratchFpRegisters();
    if (adjust_regs) {
        live_regs &= GetRegfile()->GetCallerSavedRegMask();
        live_vregs &= GetRegfile()->GetCallerSavedVRegMask();
    } else {
        live_regs &= GetCallerRegsMask(GetArch(), false);
        live_vregs &= GetCallerRegsMask(GetArch(), true);
    }
    GetEncoder()->SaveRegisters(live_regs, false, GetFrameInfo()->GetCallersOffset(), base,
                                GetCallerRegsMask(GetArch(), false));
    GetEncoder()->SaveRegisters(live_vregs, true, GetFrameInfo()->GetFpCallersOffset(), base,
                                GetCallerRegsMask(GetArch(), true));
}

void Codegen::LoadCallerRegisters(RegMask live_regs, VRegMask live_vregs, bool adjust_regs)
{
    SCOPED_DISASM_STR(this, "Restore caller saved regs");
    auto base = GetFrameInfo()->GetCallersRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
    live_regs &= ~GetEncoder()->GetAvailableScratchRegisters();
    live_vregs &= ~GetEncoder()->GetAvailableScratchFpRegisters();
    if (adjust_regs) {
        live_regs &= GetRegfile()->GetCallerSavedRegMask();
        live_vregs &= GetRegfile()->GetCallerSavedVRegMask();
    } else {
        live_regs &= GetCallerRegsMask(GetArch(), false);
        live_vregs &= GetCallerRegsMask(GetArch(), true);
    }
    GetEncoder()->LoadRegisters(live_regs, false, GetFrameInfo()->GetCallersOffset(), base,
                                GetCallerRegsMask(GetArch(), false));
    GetEncoder()->LoadRegisters(live_vregs, true, GetFrameInfo()->GetFpCallersOffset(), base,
                                GetCallerRegsMask(GetArch(), true));
}

bool Codegen::RegisterKeepCallArgument(CallInst *call_inst, Reg reg)
{
    for (auto i = 0U; i < call_inst->GetInputsCount(); i++) {
        auto location = call_inst->GetLocation(i);
        if (location.IsRegister() && location.GetValue() == reg.GetId()) {
            return true;
        }
    }
    return false;
}

void Codegen::LoadMethod(Reg dst)
{
    ASSERT((CFrameMethod::GetOffsetFromSpInBytes(GetFrameLayout()) -
            (GetFrameLayout().GetMethodOffset<CFrameLayout::SP, CFrameLayout::BYTES>())) == 0);
    auto sp_offset = CFrameMethod::GetOffsetFromSpInBytes(GetFrameLayout());
    auto mem = MemRef(SpReg(), sp_offset);
    GetEncoder()->EncodeLdr(dst, false, mem);
}

void Codegen::StoreFreeSlot(Reg src)
{
    ASSERT(src.GetSize() <= (GetFrameLayout().GetSlotSize() << 3U));
    auto sp_offset = GetFrameLayout().GetFreeSlotOffset<CFrameLayout::SP, CFrameLayout::BYTES>();
    auto mem = MemRef(SpReg(), sp_offset);
    GetEncoder()->EncodeStr(src, mem);
}

void Codegen::LoadFreeSlot(Reg dst)
{
    ASSERT(dst.GetSize() <= (GetFrameLayout().GetSlotSize() << 3U));
    auto sp_offset = GetFrameLayout().GetFreeSlotOffset<CFrameLayout::SP, CFrameLayout::BYTES>();
    auto mem = MemRef(SpReg(), sp_offset);
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
#define BinaryImmOperation(opc)                                                                                     \
    void EncodeVisitor::Visit##opc##I(GraphVisitor *visitor, Inst *inst)                                            \
    {                                                                                                               \
        auto binop = inst->CastTo##opc##I();                                                                        \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);                                                 \
        auto type = inst->GetType();                                                                                \
        auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                                     \
        auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);                                   \
        enc->GetEncoder()->Encode##opc(dst, src0, enc->GetCodegen()->ConvertImm(binop->GetImm(), DataType::INT64)); \
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

        enc->GetCodegen()->FillCallParams({src0, src1});
        enc->GetCodegen()->CallIntrinsic(inst, entry);

        auto ret_val = enc->GetCodegen()->GetTarget().GetReturnFpReg();
        if (ret_val.GetType().IsFloat()) {
            // ret_val is FLOAT64 for Arm64, AMD64 and ARM32 HardFP, but dst can be FLOAT32
            // so we convert ret_val to FLOAT32
            enc->GetEncoder()->EncodeMov(dst, Reg(ret_val.GetId(), dst.GetType()));
        } else {
            // case for ARM32 SoftFP
            enc->GetEncoder()->EncodeMov(dst,
                                         Reg(ret_val.GetId(), dst.GetSize() == WORD_SIZE ? INT32_TYPE : INT64_TYPE));
        }

        enc->GetEncoder()->SetRegister(&live_regs, &live_vregs, dst, false);
        enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
        return;
    }

    ASSERT(arch == Arch::AARCH32);
    // TODO(pishin) Fix after supporting AOT mode for arm32
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
    enc->GetEncoder()->EncodeShr(dst, src0, enc->GetCodegen()->ConvertImm(binop->GetImm(), DataType::INT64));
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
    auto imm_shift_inst = static_cast<UnaryShiftedRegisterOperation *>(inst);
    enc->GetEncoder()->EncodeNeg(dst, Shift(src, imm_shift_inst->GetShiftType(), imm_shift_inst->GetImm()));
}

void EncodeVisitor::VisitCast(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto src_type = inst->GetInputType(0);
    auto dst_type = inst->GetType();

    if (dst_type == DataType::ANY) {
        CastToAny(visitor, inst);
        return;
    }

    bool src_signed = IsTypeSigned(src_type);
    bool dst_signed = IsTypeSigned(dst_type);

    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), src_type);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), dst_type);
    if (dst_type == DataType::BOOL) {
        enc->GetEncoder()->EncodeCastToBool(dst, src);
        return;
    }

    auto arch = enc->GetCodegen()->GetArch();
    if (!Codegen::InstEncodedWithLibCall(inst, arch)) {
        enc->GetEncoder()->EncodeCast(dst, dst_signed, src, src_signed);
        return;
    }
    ASSERT(arch == Arch::AARCH32);
    // TODO(pishin) Fix after supporting AOT mode for arm32
    if (enc->cg_->GetGraph()->IsAotMode()) {
        enc->GetEncoder()->SetFalseResult();
        return;
    }
    auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst);
    enc->GetEncoder()->SetRegister(&live_regs, &live_vregs, dst, false);
    enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);
    enc->GetEncoder()->EncodeCast(dst, dst_signed, src, src_signed);
    enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
}

void EncodeVisitor::VisitPhi([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst) {}

void EncodeVisitor::CastToAny(GraphVisitor *visitor, Inst *inst)
{
    auto src_type = inst->GetInputType(0);
    int64_t tag;
    switch (src_type) {
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
        case DataType::UINT64:
        case DataType::INT64:
            tag = interpreter::INT;
            break;
        case DataType::FLOAT64:
            tag = interpreter::DOUBLE;
            break;
        case DataType::REFERENCE:
            if (inst->GetInput(0).GetInst()->GetOpcode() == Opcode::LoadString) {
                tag = interpreter::STRING;
            } else {
                tag = interpreter::OBJECT;
            }
            break;
        default:
            UNREACHABLE();
    }

    auto enc = static_cast<EncodeVisitor *>(visitor);
    Codegen *codegen = enc->GetCodegen();
    Encoder *encoder = enc->GetEncoder();
    Reg src_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), src_type);
    Reg dst_reg = codegen->ConvertRegister(inst->GetDstReg(), DataType::INT64);
    Reg tag_reg = codegen->ConvertRegister(inst->GetDstReg() + 1, DataType::INT64);
    if (src_reg.GetId() != dst_reg.GetId()) {
        encoder->EncodeMov(dst_reg, src_reg);
    }
    encoder->EncodeMov(tag_reg, Imm(tag));
}

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
    auto *const_inst = inst->CastToConstant();
    auto type = inst->GetType();
    if (enc->cg_->GetGraph()->IsDynamicMethod() && type == DataType::INT64) {
        type = DataType::INT32;
    }

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    Imm imm = enc->GetCodegen()->ConvertImm(const_inst, type);
    enc->GetEncoder()->EncodeMov(dst, imm);
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
    Imm imm = enc->GetCodegen()->ConvertImm(static_cast<uint64_t>(0), type);
    enc->GetEncoder()->EncodeMov(dst, imm);
}

// Next visitors use calling convention

void EncodeVisitor::VisitIndirectJump(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::POINTER);  // pointer
    enc->GetEncoder()->EncodeJump(src);
}

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
        auto dst_reg = ConvertRegister(inst->GetDstReg(), inst->GetType());
        ASSERT(dst_reg.IsValid());
        GetEncoder()->EncodeMov(dst_reg, GetTarget().GetReturnReg(dst_reg.GetType()));
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

    auto cmp_inst = inst->CastToCmp();
    auto type = cmp_inst->GetOperandsType();

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);

    Condition cc;
    if (DataType::IsFloatType(type)) {
        // TODO(igorban): check whether MI is fully correct here
        cc = cmp_inst->IsFcmpg() ? (Condition::MI) : (Condition::LT);
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
        enc->GetEncoder()->EncodeMemoryBarrier(MemoryOrder::Release);
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
    int64_t imm_val = inst->CastToReturnI()->GetImm();
    Imm imm = codegen->ConvertImmWithExtend(imm_val, inst->GetType());

    auto return_reg = codegen->GetTarget().GetReturnReg(codegen->ConvertDataType(inst->GetType(), codegen->GetArch()));

    if (imm_val == 0 && codegen->GetTarget().SupportZeroReg() && !DataType::IsFloatType(inst->GetType())) {
        enc->GetEncoder()->EncodeMov(return_reg, rzero);
    } else {
        enc->GetEncoder()->EncodeMov(return_reg, imm);
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

void EncodeVisitor::VisitReturnInlined([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(MemoryOrder::Release);
    }

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (!enc->cg_->GetGraph()->IsAotMode()) {
        auto call_inst = GetCallInstFromReturnInlined(inst->CastToReturnInlined());
        ASSERT(call_inst != nullptr);
        static_cast<EncodeVisitor *>(visitor)->GetCodegen()->InsertTrace(
            {Imm(static_cast<size_t>(TraceId::METHOD_EXIT)), Imm(reinterpret_cast<size_t>(call_inst->GetCallMethod())),
             Imm(static_cast<size_t>(events::MethodExitKind::INLINED))});
    }
#endif
}

void EncodeVisitor::VisitLoadConstArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto method = inst->CastToLoadConstArray()->GetMethod();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto array_type = inst->CastToLoadConstArray()->GetTypeId();

    enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::RESOLVE_LITERAL_ARRAY, dst, Imm(array_type));
}

void EncodeVisitor::VisitFillConstArray(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    ASSERT(type != DataType::REFERENCE);
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto array_type = inst->CastToFillConstArray()->GetTypeId();
    auto arch = enc->cg_->GetGraph()->GetArch();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto method = inst->CastToFillConstArray()->GetMethod();
    auto offset = runtime->GetArrayDataOffset(enc->GetCodegen()->GetArch());
    auto array_size = inst->CastToFillConstArray()->GetImm() << DataType::ShiftByType(type, arch);
    ScopedTmpReg array_reg(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    encoder->EncodeAdd(array_reg, src, Imm(offset));

    ASSERT(array_size != 0);

    if (enc->cg_->GetGraph()->IsAotMode()) {
        auto arr_offset = runtime->GetOffsetToConstArrayData(method, array_type);
        auto pf_offset = runtime->GetPandaFileOffset(arch);
        ScopedTmpReg method_reg(encoder);
        enc->GetCodegen()->LoadMethod(method_reg);
        // load pointer to panda file
        encoder->EncodeLdr(method_reg, false, MemRef(method_reg, pf_offset));
        // load pointer to binary file
        encoder->EncodeLdr(method_reg, false, MemRef(method_reg, runtime->GetBinaryFileBaseOffset(enc->GetArch())));
        // Get pointer to array data
        encoder->EncodeAdd(method_reg, method_reg, Imm(arr_offset));
        // call memcpy
        RuntimeInterface::IntrinsicId entry = RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY;
        auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst);
        enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);

        enc->GetCodegen()->FillCallParams({array_reg, method_reg, Imm(array_size)});
        enc->GetCodegen()->CallIntrinsic(inst, entry);
        enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
    } else {
        auto data = runtime->GetPointerToConstArrayData(method, array_type);
        // call memcpy
        RuntimeInterface::IntrinsicId entry = RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY;
        auto [live_regs, live_vregs] = enc->GetCodegen()->GetLiveRegisters(inst);
        enc->GetCodegen()->SaveCallerRegisters(live_regs, live_vregs, true);

        enc->GetCodegen()->FillCallParams({array_reg, Imm(data), Imm(array_size)});
        enc->GetCodegen()->CallIntrinsic(inst, entry);
        enc->GetCodegen()->LoadCallerRegisters(live_regs, live_vregs, true);
    }
}

void Codegen::VisitNewArray(Inst *inst)
{
    auto method = inst->CastToNewArray()->GetMethod();
    auto dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src_class = ConvertRegister(inst->GetSrcReg(NewArrayInst::INDEX_CLASS), DataType::POINTER);
    auto src_size = ConvertRegister(inst->GetSrcReg(NewArrayInst::INDEX_SIZE), DataType::Type::INT32);
    auto array_type = inst->CastToNewArray()->GetTypeId();
    auto runtime = GetGraph()->GetRuntime();
    auto encoder = GetEncoder();

    auto max_tlab_size = runtime->GetTLABMaxSize();

    // TODO(msherstennikov): support NewArray fast path for arm32
    if (max_tlab_size == 0 || GetArch() == Arch::AARCH32) {
        CallRuntime(inst, EntrypointId::CREATE_ARRAY, dst, {src_class, src_size});
        if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
            encoder->EncodeMemoryBarrier(MemoryOrder::Release);
        }
        return;
    }

    auto len_inst = inst->GetDataFlowInput(0);
    auto class_array_size = runtime->GetClassArraySize(GetArch());
    uint64_t array_size = 0;
    uint64_t element_size = runtime->GetArrayElementSize(method, array_type);
    uint64_t alignment = runtime->GetTLABAlignment();
    if (len_inst->GetOpcode() == Opcode::Constant) {
        ASSERT(len_inst->GetType() == DataType::INT64);
        array_size = len_inst->CastToConstant()->GetIntValue() * element_size + class_array_size;
        array_size = (array_size & ~(alignment - 1U)) + ((array_size % alignment) != 0U ? alignment : 0U);
        if (array_size > max_tlab_size) {
            CallRuntime(inst, EntrypointId::CREATE_ARRAY, dst, {src_class, src_size});
            if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
                encoder->EncodeMemoryBarrier(MemoryOrder::Release);
            }
            return;
        }
    }

    auto param_regs {GetLiveRegisters(inst).first};
    param_regs &= GetTarget().GetParamRegsMask(2U);

    SaveCallerRegisters(param_regs, VRegMask(), false);

    EntrypointId eid;
    switch (element_size) {
        case sizeof(uint8_t):
            eid = EntrypointId::ALLOCATE_ARRAY_TLAB8;
            break;
        case sizeof(uint16_t):
            eid = EntrypointId::ALLOCATE_ARRAY_TLAB16;
            break;
        case sizeof(uint32_t):
            eid = EntrypointId::ALLOCATE_ARRAY_TLAB32;
            break;
        case sizeof(uint64_t):
            eid = EntrypointId::ALLOCATE_ARRAY_TLAB64;
            break;
        default:
            UNREACHABLE();
    }

    FillCallParams({src_class, src_size});
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), eid));
    encoder->MakeCall(entry);
    CreateStackMap(inst);

    GetEncoder()->EncodeMov(dst, GetTarget().GetReturnReg(dst.GetType()));
    param_regs.reset(dst.GetId());
    LoadCallerRegisters(param_regs, VRegMask(), false);

    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        encoder->EncodeMemoryBarrier(MemoryOrder::Release);
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
    auto param_inst = inst->CastToParameter();
    auto sf = param_inst->GetLocationData();

    if (codegen->GetGraph()->GetMode().SupportManagedCode() && codegen->GetGraph()->IsDynamicMethod() &&
        param_inst->GetArgNumber() > 0) {
        // In dynamic methods only the first parameter is mandatory
        // The rest parameters are optional. Actual number of passed parameters is known only
        // in runtime and is located in the 'r1' register.
        // All declared parameters which are not mapped to actual parameters must have
        // special value 'undefined'. That is why we have special handling of parameters
        // for dynamic methods.
        VisitDynamicMethodParameter(visitor, inst);
        return;
    }

    if (sf.GetSrc() == sf.GetDst()) {
        return;
    }

    auto tmp_sf = codegen->GetGraph()->CreateInstSpillFill();
    tmp_sf->AddSpillFill(sf);
    SpillFillEncoder(codegen, tmp_sf).EncodeSpillFill();
}

void EncodeVisitor::VisitDynamicMethodParameter(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto param_inst = inst->CastToParameter();
    ASSERT(param_inst->GetType() == DataType::ANY);
    Codegen *codegen = enc->GetCodegen();
    Encoder *encoder = enc->GetEncoder();

    SCOPED_DISASM_STR(codegen, "VisitParameter");

    auto sf = param_inst->GetLocationData();
    ASSERT(sf.DstValue() != INVALID_REG);
    Reg num_actual_args_reg = enc->GetCodegen()->GetTarget().GetParamReg(1);

    LabelHolder::LabelId else_label = encoder->CreateLabel();
    LabelHolder::LabelId end_label = encoder->CreateLabel();
    encoder->EncodeJump(else_label, num_actual_args_reg, Imm(param_inst->GetArgNumber()), Condition::LE);
    HandleDynParamPassed(sf, enc);
    encoder->EncodeJump(end_label);
    encoder->BindLabel(else_label);
    HandleDynParamNotPassed(sf, enc);
    encoder->BindLabel(end_label);
}

void EncodeVisitor::HandleDynParamPassed(const SpillFillData &sf, EncodeVisitor *enc)
{
    if (sf.GetSrc() == sf.GetDst()) {
        return;
    }
    Codegen *codegen = enc->GetCodegen();
    Encoder *encoder = enc->GetEncoder();

    if (sf.GetSrc().IsRegister()) {
        auto src_reg = codegen->ConvertRegister(sf.SrcValue(), sf.GetType());
        if (sf.GetDst().IsRegister()) {  // param_reg -> dst_reg
            auto dst_reg = codegen->ConvertRegister(sf.DstValue(), DataType::ANY);
            encoder->EncodeMov(dst_reg, src_reg);
        } else {
            ASSERT(sf.GetDst().IsAnyStack());  // param_reg -> push to slot
            auto dst_mem = codegen->GetMemRefForSlot(sf.GetDst());
            encoder->EncodeStrz(src_reg, dst_mem);
        }
        return;
    }

    ASSERT(sf.GetSrc().IsAnyStack());
    auto src_mem = codegen->GetMemRefForSlot(sf.GetSrc());
    if (sf.GetDst().IsRegister()) {  // load from stack -> dst_reg
        auto dst_reg = codegen->ConvertRegister(sf.DstValue(), sf.GetType());
        encoder->EncodeLdr(dst_reg, false, src_mem);
    } else {
        ASSERT(sf.GetDst().IsAnyStack());  // load from stack -> push to slot
        auto dst_mem = codegen->GetMemRefForSlot(sf.GetDst());
        auto type_info = Codegen::ConvertDataType(sf.GetType(), codegen->GetArch());
        encoder->EncodeMemCopyz(src_mem, dst_mem, type_info.GetSize());
    }
}

void EncodeVisitor::HandleDynParamNotPassed(const SpillFillData &sf, EncodeVisitor *enc)
{
    Codegen *codegen = enc->GetCodegen();
    Encoder *encoder = enc->GetEncoder();
    auto value = enc->cg_->GetGraph()->GetRuntime()->GetDynamicPrimitiveUndefined();

    if (sf.GetDst().IsRegister()) {
        Reg dst_val_reg = codegen->ConvertRegister(sf.DstValue(), DataType::INT64);
        encoder->EncodeMov(dst_val_reg, Imm(value));
        return;
    }

    ASSERT(sf.GetDst().IsAnyStack());
    auto dest_val = codegen->GetMemRefForSlot(sf.GetDst());
    ScopedTmpReg reg(encoder, INT64_TYPE);
    encoder->EncodeMov(reg, Imm(value));
    encoder->EncodeStr(reg, dest_val);
}

void EncodeVisitor::VisitStoreArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);      // index
    constexpr int64_t IMM_2 = 2;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), inst->GetType());  // store value
    int32_t offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch());
    int32_t scale = DataType::ShiftByType(inst->GetType(), enc->GetCodegen()->GetArch());

    ScopedLiveTmpReg tmp(enc->GetEncoder());

    auto object_header_mem = MemRef(src0);
    enc->GetEncoder()->EncodeAdd(tmp, src0, Imm(offset));
    auto mem = MemRef(tmp, src1, scale);
    if (inst->CastToStoreArray()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStr(src2, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
    if (inst->CastToStoreArray()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, object_header_mem, src2, INVALID_REGISTER);
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
    auto inst_load_array = inst->CastToLoadArray();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    ASSERT(inst_load_array->IsArray() || !runtime->IsCompressedStringsEnabled());
    if (static_cast<LoadInst *>(inst)->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);      // index
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // load value
    int32_t offset = inst_load_array->IsArray() ? runtime->GetArrayDataOffset(enc->GetCodegen()->GetArch())
                                                : runtime->GetStringDataOffset(enc->GetArch());
    auto encoder = enc->GetEncoder();
    auto arch = encoder->GetArch();
    int32_t shift = DataType::ShiftByType(type, arch);
    ScopedTmpReg scoped_tmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto tmp = scoped_tmp.GetReg();
    encoder->EncodeAdd(tmp, src0, Imm(offset));
    auto mem = MemRef(tmp, src1, shift);
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    encoder->EncodeLdr(dst, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
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
    int32_t offset = runtime->GetStringDataOffset(enc->GetArch());
    auto encoder = enc->GetEncoder();
    auto arch = encoder->GetArch();
    int32_t shift = DataType::ShiftByType(type, arch);
    ScopedTmpReg scoped_tmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto tmp = scoped_tmp.GetReg();

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

    auto len_array_inst = inst->CastToLenArray();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    int64_t offset = len_array_inst->IsArray() ? runtime->GetArrayLengthOffset(enc->GetCodegen()->GetArch())
                                               : runtime->GetStringLengthOffset(enc->GetArch());
    auto mem = MemRef(src0, offset);

    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
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
    ASSERT(inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveState ||
           inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveStateDeoptimize);

    auto slow_path =
        enc->GetCodegen()->CreateSlowPath<SlowPathImplicitNullCheck>(inst, EntrypointId::NULL_POINTER_EXCEPTION);
    auto src_type = inst->GetInputType(0);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), src_type);
    enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), src, Condition::EQ);
}

void EncodeVisitor::VisitBoundsCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto len_type = inst->GetInputType(0);
    auto len = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), len_type);
    auto index_type = inst->GetInputType(1);
    auto index = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), index_type);
    [[maybe_unused]] constexpr int64_t IMM_2 = 2;
    ASSERT(inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveState);

    EntrypointId entrypoint = inst->CastToBoundsCheck()->IsArray() ? EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION
                                                                   : EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, entrypoint);
    enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), index, len, Condition::HS);
}

void EncodeVisitor::VisitRefTypeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto array_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto ref_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    [[maybe_unused]] constexpr int64_t IMM_2 = 2;
    ASSERT(inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveState);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();

    auto slow_path =
        enc->GetCodegen()->CreateSlowPath<SlowPathRefCheck>(inst, EntrypointId::CHECK_STORE_ARRAY_REFERENCE);

    slow_path->SetRegs(array_reg, ref_reg);
    slow_path->CreateBackLabel(encoder);

    // We don't check if stored object is nullptr
    encoder->EncodeJump(slow_path->GetBackLabel(), ref_reg, Condition::EQ);

    ScopedTmpReg tmp_reg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, enc->GetCodegen()->GetArch()));
    ScopedTmpReg tmp_reg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, enc->GetCodegen()->GetArch()));

    // Get Class from array
    enc->GetCodegen()->LoadClassFromObject(tmp_reg, array_reg);
    // Get common Class from array class
    encoder->EncodeLdr(tmp_reg, false, MemRef(tmp_reg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Get Class from stored object
    enc->GetCodegen()->LoadClassFromObject(tmp_reg1, ref_reg);

    // We calls CheckStoreArrayReference for checks Reference Type.
    // If the type is wrong, CheckStoreArrayReference doesn't return and calls Deoptimaze
    encoder->EncodeJump(slow_path->GetLabel(), tmp_reg, tmp_reg1, Condition::NE);

    slow_path->BindBackLabel(encoder);
}

void EncodeVisitor::VisitZeroCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto src_type = inst->GetInputType(0);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), src_type);
    ASSERT(inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveState);

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, EntrypointId::ARITHMETIC_EXCEPTION);
    enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), src, Condition::EQ);
}

void EncodeVisitor::VisitNegativeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto src_type = inst->GetInputType(0);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), src_type);
    ASSERT(inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveState);

    auto slow_path =
        enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, EntrypointId::NEGATIVE_ARRAY_SIZE_EXCEPTION);
    enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), src, Condition::LT);
}

void EncodeVisitor::VisitDeoptimizeIf(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto src_type = inst->GetInput(0).GetInst()->GetType();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), src_type);

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, EntrypointId::DEOPTIMIZE);

    // create jump to slow path if src is true
    enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), src, Condition::NE);
}

void EncodeVisitor::VisitDeoptimizeCompare(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto deopt = inst->CastToDeoptimizeCompare();
    ASSERT(deopt->GetOperandsType() != DataType::NO_TYPE);
    auto src0 = enc->GetCodegen()->ConvertRegister(deopt->GetSrcReg(0), deopt->GetOperandsType());
    auto src1 = enc->GetCodegen()->ConvertRegister(deopt->GetSrcReg(1), deopt->GetOperandsType());
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, EntrypointId::DEOPTIMIZE);
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
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, EntrypointId::DEOPTIMIZE);

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
            Imm imm = enc->GetCodegen()->ConvertImm(UINT64_C(0), type);
            encoder->EncodeJump(slowPath->GetLabel(), src0, imm, enc->GetCodegen()->ConvertCc(cc));
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
    auto string_type = inst->CastToLoadString()->GetTypeId();
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    ASSERT(inst->IsRuntimeCall());

    // Static constructor invoked only once, so there is no sense in replacing regular
    // ResolveString runtime call with optimized version that will only slow down constructor's execution.
    auto is_cctor = graph->GetRuntime()->IsMethodStaticConstructor(method);
    object_pointer_type string_ptr {0};

    if (graph->IsAotMode() && options.IsCompilerAotLoadStringPlt() && !is_cctor) {
        auto aot_data = graph->GetAotData();
        intptr_t slot_offset = aot_data->GetStringSlotOffset(encoder->GetCursorOffset(), string_type);
        ScopedTmpRegU64 addr_reg(encoder);
        ScopedTmpRegU64 tmp_dst(encoder);
        encoder->MakeLoadAotTableAddr(slot_offset, addr_reg, tmp_dst);

        auto slow_path =
            enc->GetCodegen()->CreateSlowPath<SlowPathResolveStringAot>(inst, EntrypointId::RESOLVE_STRING_AOT);
        slow_path->SetDstReg(dst);
        slow_path->SetAddrReg(addr_reg);
        slow_path->SetStringId(string_type);
        slow_path->SetMethod(method);
        encoder->EncodeJump(slow_path->GetLabel(), tmp_dst, Imm(RuntimeInterface::RESOLVE_STRING_AOT_COUNTER_LIMIT),
                            Condition::LT);
        encoder->EncodeMov(dst, Reg(tmp_dst.GetReg().GetId(), dst.GetType()));
        slow_path->BindBackLabel(encoder);
    } else if (!graph->IsAotMode() &&
               (string_ptr = graph->GetRuntime()->GetNonMovableString(method, string_type)) != 0) {
        encoder->EncodeMov(dst, Imm(string_ptr));
        EVENT_JIT_USE_RESOLVED_STRING(graph->GetRuntime()->GetMethodName(method), string_type);
    } else {
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::RESOLVE_STRING, dst, Imm(string_type));
    }
}

void EncodeVisitor::VisitLoadObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto load_obj = inst->CastToLoadObject();
    if (load_obj->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto type = inst->GetType();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value

    auto graph = enc->cg_->GetGraph();
    auto field = load_obj->GetObjField();
    auto offset = graph->GetRuntime()->GetFieldOffset(field);
    auto mem = MemRef(src, offset);
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    if (load_obj->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
}

void EncodeVisitor::VisitUnresolvedLoadObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto load_obj = inst->CastToUnresolvedLoadObject();
    if (load_obj->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto type = inst->GetType();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value

    auto type_id = load_obj->GetTypeId();
    auto method = load_obj->GetMethod();
    ScopedTmpReg tmp_reg(enc->GetEncoder());
    if (graph->IsAotMode()) {
        // TODO(zhroma): consider caching ?
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::GET_FIELD_OFFSET, tmp_reg, Imm(type_id));
        enc->GetEncoder()->EncodeAdd(tmp_reg, src, tmp_reg);
        // Unknown load, assume it can be volatile
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(tmp_reg));
    } else {
        auto skind = UnresolvedTypesInterface::SlotKind::FIELD;
        auto field_offset_addr = graph->GetRuntime()->GetUnresolvedTypes()->GetTableSlot(method, type_id, skind);

        auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::GET_FIELD_OFFSET);
        slow_path->SetUnresolvedType(method, type_id);
        slow_path->SetSlotAddr(field_offset_addr);
        enc->GetEncoder()->EncodeMov(tmp_reg, Imm(field_offset_addr));
        enc->GetEncoder()->EncodeLdr(tmp_reg, false, MemRef(tmp_reg));
        slow_path->SetDstReg(tmp_reg);
        enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), tmp_reg, Condition::EQ);
        slow_path->BindBackLabel(enc->GetEncoder());
        enc->GetEncoder()->EncodeAdd(tmp_reg, src, tmp_reg);
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(tmp_reg));
    }
}

void EncodeVisitor::VisitLoad(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto load_by_offset = inst->CastToLoad();

    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), DataType::UINT32);   // offset
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value

    auto mem = MemRef(src0, src1, load_by_offset->GetScale());

    if (load_by_offset->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
}

void EncodeVisitor::VisitLoadI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto load_by_offset = inst->CastToLoadI();

    auto type = inst->GetType();
    auto base = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value

    if (load_by_offset->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(base, load_by_offset->GetImm()));
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), MemRef(base, load_by_offset->GetImm()));
    }
}

void EncodeVisitor::VisitStoreI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto store_inst = inst->CastToStoreI();

    auto base = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), inst->GetType());
    auto object_header_mem = MemRef(base);
    auto mem = MemRef(base, store_inst->GetImm());
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }

    if (store_inst->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src, mem);
    }

    if (inst->CastToStoreI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, object_header_mem, src, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitStoreObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto store_obj = inst->CastToStoreObject();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value

    auto graph = enc->cg_->GetGraph();
    auto field = store_obj->GetObjField();
    auto offset = graph->GetRuntime()->GetFieldOffset(field);
    auto object_header_mem = MemRef(src0);
    auto mem = MemRef(src0, offset);
    if (inst->CastToStoreObject()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    if (store_obj->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src1, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src1, mem);
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
    if (inst->CastToStoreObject()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, object_header_mem, src1, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitUnresolvedStoreObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto store_obj = inst->CastToUnresolvedStoreObject();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value

    auto type_id = store_obj->GetTypeId();
    auto method = store_obj->GetMethod();
    ScopedLiveTmpReg tmp_reg(enc->GetEncoder());
    if (graph->IsAotMode()) {
        // TODO(zhroma): consider caching ?
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, RuntimeInterface::EntrypointId::GET_FIELD_OFFSET,
                                                 tmp_reg, Imm(type_id));
    } else {
        auto skind = UnresolvedTypesInterface::SlotKind::FIELD;
        auto field_offset_addr = graph->GetRuntime()->GetUnresolvedTypes()->GetTableSlot(method, type_id, skind);
        enc->GetEncoder()->EncodeMov(tmp_reg, Imm(field_offset_addr));
        enc->GetEncoder()->EncodeLdr(tmp_reg, false, MemRef(tmp_reg));

        auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::GET_FIELD_OFFSET);
        slow_path->SetUnresolvedType(method, type_id);
        slow_path->SetDstReg(tmp_reg);
        slow_path->SetSlotAddr(field_offset_addr);

        enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), tmp_reg, Condition::EQ);
        slow_path->BindBackLabel(enc->GetEncoder());
    }

    enc->GetEncoder()->EncodeAdd(tmp_reg, src0, tmp_reg);
    auto mem = MemRef(tmp_reg);
    auto object_header_mem = MemRef(src0);
    if (inst->CastToUnresolvedStoreObject()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }
    // Unknown store, assume it can be volatile
    enc->GetEncoder()->EncodeStrRelease(src1, mem);
    if (inst->CastToUnresolvedStoreObject()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, object_header_mem, src1, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitStore(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto store_by_offset = inst->CastToStore();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), DataType::UINT32);   // offset
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(2U), type);               // store value

    auto object_header_mem = MemRef(src0);
    auto mem = MemRef(src0, src1, store_by_offset->GetScale());
    if (inst->CastToStore()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }

    if (store_by_offset->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src2, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src2, mem);
    }

    if (inst->CastToStore()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, object_header_mem, src2, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto class_id = inst->CastToInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    ASSERT(inst->IsRuntimeCall());

    if (graph->IsAotMode()) {
        ScopedTmpReg tmp_reg(encoder);
        ScopedTmpReg class_reg(encoder);

        auto aot_data = graph->GetAotData();
        intptr_t offset = aot_data->GetClassSlotOffset(encoder->GetCursorOffset(), class_id, true);
        encoder->MakeLoadAotTableAddr(offset, tmp_reg, class_reg);

        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, class_reg, Condition::NE);

        // PLT Class Init Resolver has special calling convention:
        // First encoder temporary (tmp_reg) works as parameter and return value (which is unnecessary here)
        CHECK_EQ(tmp_reg.GetReg().GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());
        enc->GetCodegen()->CreateJumpToClassResolverPltShared(inst, tmp_reg.GetReg(),
                                                              EntrypointId::CLASS_INIT_RESOLVER);
        encoder->BindLabel(label);
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToInitClass()->GetClass());
        ASSERT(klass != 0);
        if (!runtime->IsClassInitialized(klass)) {
            auto slow_path =
                enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::INITIALIZE_CLASS);

            auto state_offset = runtime->GetClassStateOffset(enc->GetArch());
            int64_t init_value = runtime->GetClassInitializedValue();
            ScopedTmpReg tmp_reg(encoder);
            encoder->EncodeMov(tmp_reg, Imm(klass + state_offset));
            auto tmp_i8 = enc->GetCodegen()->ConvertRegister(tmp_reg.GetReg().GetId(), DataType::INT8);
            encoder->EncodeLdr(tmp_i8, false, MemRef(tmp_reg));

            encoder->EncodeJump(slow_path->GetLabel(), tmp_i8, Imm(init_value), Condition::NE);

            slow_path->BindBackLabel(encoder);
        }
    }
}

void EncodeVisitor::VisitLoadClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto load_class = inst->CastToLoadClass();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto type_id = load_class->GetTypeId();
    ASSERT(inst->IsRuntimeCall());

    if (graph->IsAotMode()) {
        auto method_class_id = graph->GetRuntime()->GetClassIdForMethod(graph->GetMethod());
        if (method_class_id == type_id) {
            auto dst_ptr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dst_ptr);
            auto mem = MemRef(dst_ptr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
            return;
        }
        ScopedTmpReg tmp_reg(encoder);
        enc->GetCodegen()->CreateLoadClassFromPLT(inst, tmp_reg, dst, type_id);
    } else {  // JIT mode
        auto klass = load_class->GetClass();
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
    auto load_class = inst->CastToLoadClass();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto type_id = load_class->GetTypeId();
    auto method = load_class->GetMethod();
    auto utypes = graph->GetRuntime()->GetUnresolvedTypes();
    auto klass_addr = utypes->GetTableSlot(method, type_id, UnresolvedTypesInterface::SlotKind::CLASS);
    Reg dst_ptr(dst.GetId(), enc->GetCodegen()->GetPtrRegType());
    encoder->EncodeMov(dst_ptr, Imm(klass_addr));
    encoder->EncodeLdr(dst, false, MemRef(dst_ptr));
    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::RESOLVE_CLASS);
    slow_path->SetUnresolvedType(method, type_id);
    slow_path->SetDstReg(dst);
    slow_path->SetSlotAddr(klass_addr);

    encoder->EncodeJump(slow_path->GetLabel(), dst, Condition::EQ);
    slow_path->BindBackLabel(encoder);
}

void EncodeVisitor::VisitLoadAndInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto class_id = inst->CastToLoadAndInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    ASSERT(inst->IsRuntimeCall());

    if (graph->IsAotMode()) {
        auto method_class_id = runtime->GetClassIdForMethod(graph->GetMethod());
        if (method_class_id == class_id) {
            auto dst_ptr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dst_ptr);
            auto mem = MemRef(dst_ptr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
            return;
        }

        ScopedTmpReg tmp_reg(encoder);

        auto aot_data = graph->GetAotData();
        intptr_t offset = aot_data->GetClassSlotOffset(encoder->GetCursorOffset(), class_id, true);
        encoder->MakeLoadAotTableAddr(offset, tmp_reg, dst);

        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, dst, Condition::NE);

        // PLT Class Init Resolver has special calling convention:
        // First encoder temporary (tmp_reg) works as parameter and return value
        CHECK_EQ(tmp_reg.GetReg().GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());
        enc->GetCodegen()->CreateJumpToClassResolverPltShared(inst, tmp_reg.GetReg(),
                                                              EntrypointId::CLASS_INIT_RESOLVER);

        encoder->EncodeMov(dst, tmp_reg);
        encoder->BindLabel(label);
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToLoadAndInitClass()->GetClass());
        encoder->EncodeMov(dst, Imm(klass));

        if (runtime->IsClassInitialized(klass)) {
            return;
        }

        auto method_class = runtime->GetClass(graph->GetMethod());
        if (method_class == inst->CastToLoadAndInitClass()->GetClass()) {
            return;
        }

        auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::INITIALIZE_CLASS);

        auto state_offset = runtime->GetClassStateOffset(enc->GetArch());
        int64_t init_value = runtime->GetClassInitializedValue();

        ScopedTmpReg state_reg(encoder, INT8_TYPE);
        encoder->EncodeLdr(state_reg, false, MemRef(dst, state_offset));

        encoder->EncodeJump(slow_path->GetLabel(), state_reg, Imm(init_value), Condition::NE);

        slow_path->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitUnresolvedLoadAndInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto class_id = inst->CastToUnresolvedLoadAndInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    ASSERT(inst->IsRuntimeCall());

    if (graph->IsAotMode()) {
        ScopedTmpReg tmp_reg(encoder);

        auto aot_data = graph->GetAotData();
        intptr_t offset = aot_data->GetClassSlotOffset(encoder->GetCursorOffset(), class_id, true);
        encoder->MakeLoadAotTableAddr(offset, tmp_reg, dst);

        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, dst, Condition::NE);

        // PLT Class Init Resolver has special calling convention:
        // First encoder temporary (tmp_reg) works as parameter and return value
        CHECK_EQ(tmp_reg.GetReg().GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());
        enc->GetCodegen()->CreateJumpToClassResolverPltShared(inst, tmp_reg.GetReg(),
                                                              EntrypointId::CLASS_INIT_RESOLVER);

        encoder->EncodeMov(dst, tmp_reg);
        encoder->BindLabel(label);
    } else {  // JIT mode
        auto method = inst->CastToUnresolvedLoadAndInitClass()->GetMethod();
        auto utypes = graph->GetRuntime()->GetUnresolvedTypes();
        auto klass_addr = utypes->GetTableSlot(method, class_id, UnresolvedTypesInterface::SlotKind::CLASS);
        Reg dst_ptr(dst.GetId(), enc->GetCodegen()->GetPtrRegType());
        encoder->EncodeMov(dst_ptr, Imm(klass_addr));
        encoder->EncodeLdr(dst, false, MemRef(dst_ptr));

        auto slow_path =
            enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::INITIALIZE_CLASS_BY_ID);
        slow_path->SetUnresolvedType(method, class_id);
        slow_path->SetDstReg(dst);
        slow_path->SetSlotAddr(klass_addr);

        encoder->EncodeJump(slow_path->GetLabel(), dst, Condition::EQ);
        slow_path->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitLoadStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto load_static = inst->CastToLoadStatic();
    if (load_static->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // load value
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // class

    auto graph = enc->cg_->GetGraph();
    auto field = load_static->GetObjField();
    auto offset = graph->GetRuntime()->GetFieldOffset(field);
    auto mem = MemRef(src0, offset);
    if (load_static->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
}

void EncodeVisitor::VisitUnresolvedLoadStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto load_static = inst->CastToUnresolvedLoadStatic();
    if (load_static->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);

    auto type_id = load_static->GetTypeId();
    auto method = load_static->GetMethod();

    auto ref_reg = enc->GetCodegen()->ConvertRegister(0, DataType::REFERENCE);
    auto entrypoint = EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS;

    if (graph->IsAotMode()) {
        // We can't use dst for Load (src - used for get Ref-type size)
        if ((dst.GetSize() < ref_reg.GetSize()) || !dst.IsScalar()) {
            ScopedTmpReg tmp_reg(enc->GetEncoder());
            enc->GetCodegen()->CallRuntimeWithMethod(inst, method, entrypoint, tmp_reg, Imm(type_id), Imm(0));
            enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(tmp_reg));
            return;
        }
        // TODO(zhroma): consider caching ?
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, entrypoint, dst, Imm(type_id), Imm(0));
    } else {
        auto skind = UnresolvedTypesInterface::SlotKind::FIELD;
        auto field_addr = graph->GetRuntime()->GetUnresolvedTypes()->GetTableSlot(method, type_id, skind);

        auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, entrypoint);
        slow_path->SetUnresolvedType(method, type_id);
        slow_path->SetSlotAddr(field_addr);

        // We can't use dst for Load (src - used for get Ref-type size)
        if ((dst.GetSize() < ref_reg.GetSize()) || !dst.IsScalar()) {
            ScopedTmpReg tmp(enc->GetEncoder());
            enc->GetEncoder()->EncodeMov(tmp, Imm(field_addr));
            enc->GetEncoder()->EncodeLdr(tmp, false, MemRef(tmp));
            slow_path->SetDstReg(tmp);
            enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), tmp, Condition::EQ);
            slow_path->BindBackLabel(enc->GetEncoder());
            // Unknown load, assume it can be volatile
            enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(tmp));
            return;
        }

        Reg dst_ptr(dst.GetId(), enc->GetCodegen()->GetPtrRegType());
        enc->GetEncoder()->EncodeMov(dst_ptr, Imm(field_addr));
        enc->GetEncoder()->EncodeLdr(dst, false, MemRef(dst_ptr));

        slow_path->SetDstReg(dst);

        enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), dst, Condition::EQ);
        slow_path->BindBackLabel(enc->GetEncoder());
    }

    // Unknown load, assume it can be volatile
    enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(dst));
}

void EncodeVisitor::VisitStoreStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto store_static = inst->CastToStoreStatic();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // class
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value

    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto field = store_static->GetObjField();
    auto offset = runtime->GetFieldOffset(field);
    auto mem = MemRef(src0, offset);

    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }
    if (store_static->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src1, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src1, mem);
    }
    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        auto arch = enc->GetEncoder()->GetArch();
        ScopedTmpReg tmp_reg(enc->GetEncoder(), Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
        enc->GetEncoder()->EncodeLdr(tmp_reg, false, MemRef(src0, runtime->GetManagedClassOffset(enc->GetArch())));
        auto class_header_mem = MemRef(tmp_reg);
        enc->GetCodegen()->CreatePostWRB(inst, class_header_mem, src1, INVALID_REGISTER);
    }
}

static void FinishUnresolvedStoreStatic(EncodeVisitor *enc, Inst *inst, Graph *graph, ScopedTmpReg *tmp1, bool pre)
{
    auto runtime = graph->GetRuntime();
    auto value = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), inst->GetType());

    {
        // tmp1 points to Field, load Class into tmp2
        ScopedLiveTmpReg tmp2(enc->GetEncoder(), Codegen::ConvertDataType(DataType::REFERENCE, enc->GetArch()));
        enc->GetEncoder()->EncodeLdr(tmp2, false, MemRef(*tmp1, runtime->GetFieldClassOffset(enc->GetArch())));
        // extract Offset
        enc->GetEncoder()->EncodeLdr(tmp1->GetReg().As(INT32_TYPE), false,
                                     MemRef(*tmp1, runtime->GetFieldOffsetOffset(enc->GetArch())));
        // address = Class + Offset
        enc->GetEncoder()->EncodeAdd(*tmp1, *tmp1, tmp2);

        // Pre-barrier if necessary
        if (pre) {
            enc->GetCodegen()->CreatePreWRB(inst, MemRef(*tmp1));
        }
        // Unknown store, assume it can be volatile
        enc->GetEncoder()->EncodeStrRelease(value, MemRef(*tmp1));

        enc->GetEncoder()->EncodeLdr(*tmp1, false, MemRef(tmp2, runtime->GetManagedClassOffset(enc->GetArch())));
    }

    auto class_header_mem = MemRef(*tmp1);
    enc->GetCodegen()->CreatePostWRB(inst, class_header_mem, value, INVALID_REGISTER);
}

// TODO(pishin): In 'slow' case extra runtime call (two in total) required because lack of registers,
// two free registers necessary for CreatePreWRB for x86_64
static void GenUnresolvedStoreStaticBarrierAot(EncodeVisitor *enc, Inst *inst, Graph *graph, bool slow)
{
    auto store_static = inst->CastToUnresolvedStoreStatic();
    auto type_id = store_static->GetTypeId();
    auto entrypoint = RuntimeInterface::EntrypointId::GET_UNKNOWN_STATIC_FIELD_PTR;

    ScopedLiveTmpReg tmp_reg(enc->GetEncoder());
    if (slow) {
        entrypoint = RuntimeInterface::EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS;
        enc->GetCodegen()->LoadMethod(tmp_reg);
        enc->GetCodegen()->CallRuntime(inst, entrypoint, tmp_reg, {tmp_reg, Imm(type_id), Imm(0)});

        auto mem = MemRef(tmp_reg);
        enc->GetCodegen()->CreatePreWRB(inst, mem);

        entrypoint = RuntimeInterface::EntrypointId::GET_UNKNOWN_STATIC_FIELD_PTR;
    }
    // TODO(zhroma): consider caching ?
    enc->GetCodegen()->LoadMethod(tmp_reg);
    enc->GetCodegen()->CallRuntime(inst, entrypoint, tmp_reg, {tmp_reg, Imm(type_id), Imm(0)});

    FinishUnresolvedStoreStatic(enc, inst, graph, &tmp_reg, !slow);
}

static void GenUnresolvedStoreStaticBarrierJitSlow(EncodeVisitor *enc, Inst *inst, Graph *graph)
{
    auto runtime = graph->GetRuntime();
    auto store_static = inst->CastToUnresolvedStoreStatic();
    auto type_id = store_static->GetTypeId();
    auto method = store_static->GetMethod();

    // Pre-barrier preparations
    auto entrypoint = RuntimeInterface::EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS;
    auto skind = UnresolvedTypesInterface::SlotKind::FIELD;
    auto field_addr = runtime->GetUnresolvedTypes()->GetTableSlot(method, type_id, skind);

    ScopedLiveTmpReg tmp_reg(enc->GetEncoder());
    enc->GetEncoder()->EncodeMov(tmp_reg, Imm(field_addr));
    enc->GetEncoder()->EncodeLdr(tmp_reg, false, MemRef(tmp_reg));

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, entrypoint);
    slow_path->SetUnresolvedType(method, type_id);
    slow_path->SetDstReg(tmp_reg);
    slow_path->SetSlotAddr(field_addr);

    enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), tmp_reg, Condition::EQ);
    slow_path->BindBackLabel(enc->GetEncoder());

    auto mem = MemRef(tmp_reg);
    enc->GetCodegen()->CreatePreWRB(inst, mem);

    // Actual store preparations
    entrypoint = RuntimeInterface::EntrypointId::GET_UNKNOWN_STATIC_FIELD_PTR;
    skind = UnresolvedTypesInterface::SlotKind::STATIC_FIELD_PTR;
    field_addr = runtime->GetUnresolvedTypes()->GetTableSlot(method, type_id, skind);

    enc->GetEncoder()->EncodeMov(tmp_reg, Imm(field_addr));
    enc->GetEncoder()->EncodeLdr(tmp_reg, false, MemRef(tmp_reg));

    slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, entrypoint);
    slow_path->SetUnresolvedType(method, type_id);
    slow_path->SetDstReg(tmp_reg);
    slow_path->SetSlotAddr(field_addr);

    enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), tmp_reg, Condition::EQ);
    slow_path->BindBackLabel(enc->GetEncoder());

    FinishUnresolvedStoreStatic(enc, inst, graph, &tmp_reg, false);
}

static void GenUnresolvedStoreStaticBarrier(EncodeVisitor *enc, Inst *inst, Graph *graph)
{
    auto arch = enc->GetCodegen()->GetArch();
    auto runtime = graph->GetRuntime();
    bool slow = (arch == Arch::AARCH32 || arch == Arch::X86_64) &&
                runtime->GetPreType() != panda::mem::BarrierType::PRE_WRB_NONE;

    if (graph->IsAotMode()) {
        ASSERT(arch == Arch::AARCH64 || arch == Arch::X86_64);
        GenUnresolvedStoreStaticBarrierAot(enc, inst, graph, slow);
        return;
    }
    if (slow) {
        GenUnresolvedStoreStaticBarrierJitSlow(enc, inst, graph);
        return;
    }

    // JIT caching case
    auto store_static = inst->CastToUnresolvedStoreStatic();
    auto type_id = store_static->GetTypeId();
    auto method = store_static->GetMethod();

    auto entrypoint = RuntimeInterface::EntrypointId::GET_UNKNOWN_STATIC_FIELD_PTR;
    auto skind = UnresolvedTypesInterface::SlotKind::STATIC_FIELD_PTR;
    auto field_addr = runtime->GetUnresolvedTypes()->GetTableSlot(method, type_id, skind);

    ScopedLiveTmpReg tmp_reg(enc->GetEncoder());
    enc->GetEncoder()->EncodeMov(tmp_reg, Imm(field_addr));
    enc->GetEncoder()->EncodeLdr(tmp_reg, false, MemRef(tmp_reg));

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, entrypoint);
    slow_path->SetUnresolvedType(method, type_id);
    slow_path->SetDstReg(tmp_reg);
    slow_path->SetSlotAddr(field_addr);

    enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), tmp_reg, Condition::EQ);
    slow_path->BindBackLabel(enc->GetEncoder());

    FinishUnresolvedStoreStatic(enc, inst, graph, &tmp_reg, true);
}

void EncodeVisitor::VisitUnresolvedStoreStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto store_static = inst->CastToUnresolvedStoreStatic();
    auto type_id = store_static->GetTypeId();
    auto method = store_static->GetMethod();

    if (store_static->GetNeedBarrier()) {
        GenUnresolvedStoreStaticBarrier(enc, inst, graph);
        return;
    }

    // Non-barriered case
    auto value = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), inst->GetType());
    auto entrypoint = EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS;

    ScopedTmpReg tmp_reg(enc->GetEncoder());
    if (graph->IsAotMode()) {
        // TODO(zhroma): consider caching ?
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, entrypoint, tmp_reg, Imm(type_id), Imm(0));
    } else {
        auto skind = UnresolvedTypesInterface::SlotKind::FIELD;
        auto field_addr = graph->GetRuntime()->GetUnresolvedTypes()->GetTableSlot(method, type_id, skind);
        enc->GetEncoder()->EncodeMov(tmp_reg, Imm(field_addr));
        enc->GetEncoder()->EncodeLdr(tmp_reg, false, MemRef(tmp_reg));

        auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, entrypoint);
        slow_path->SetUnresolvedType(method, type_id);
        slow_path->SetDstReg(tmp_reg);
        slow_path->SetSlotAddr(field_addr);

        enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), tmp_reg, Condition::EQ);
        slow_path->BindBackLabel(enc->GetEncoder());
    }

    auto mem = MemRef(tmp_reg);
    // Unknown store, assume it can be volatile
    enc->GetEncoder()->EncodeStrRelease(value, mem);
}

void EncodeVisitor::VisitNewObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    // TODO(msherstennikov): use irtoced entrypoint once spill-fills will be supported for entrypoints mode.
    if (enc->cg_->GetArch() == Arch::AARCH32) {
        enc->GetCodegen()->CreateNewObjCallOld(inst->CastToNewObject());
    } else {
        enc->GetCodegen()->CreateNewObjCall(inst->CastToNewObject());
    }
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(MemoryOrder::Release);
    }
}

void EncodeVisitor::VisitUnresolvedLoadType(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto codegen = enc->GetCodegen();
    auto load_type = inst->CastToUnresolvedLoadType();
    if (load_type->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto type_id = load_type->GetTypeId();

    auto runtime = graph->GetRuntime();
    auto method = load_type->GetMethod();

    if (graph->IsAotMode()) {
        ScopedTmpReg tmp_reg(encoder);
        // Load pointer to klass from PLT
        codegen->CreateLoadClassFromPLT(inst, tmp_reg, dst, type_id);
        // Finally load Object
        encoder->EncodeLdr(dst, false, MemRef(dst, runtime->GetManagedClassOffset(enc->GetArch())));
    } else {
        auto utypes = runtime->GetUnresolvedTypes();
        auto cls_addr = utypes->GetTableSlot(method, type_id, UnresolvedTypesInterface::SlotKind::MANAGED_CLASS);
        Reg dst_ptr(dst.GetId(), codegen->GetPtrRegType());
        encoder->EncodeMov(dst_ptr, Imm(cls_addr));
        encoder->EncodeLdr(dst, false, MemRef(dst_ptr));

        auto slow_path = codegen->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::RESOLVE_CLASS_OBJECT);
        slow_path->SetUnresolvedType(method, type_id);
        slow_path->SetDstReg(dst);
        slow_path->SetSlotAddr(cls_addr);
        encoder->EncodeJump(slow_path->GetLabel(), dst, Condition::EQ);
        slow_path->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitLoadType(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto load_type = inst->CastToLoadType();
    if (load_type->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto type_id = load_type->GetTypeId();

    auto runtime = graph->GetRuntime();
    auto method = load_type->GetMethod();

    if (graph->IsAotMode()) {
        auto method_class_id = runtime->GetClassIdForMethod(graph->GetMethod());
        if (method_class_id == type_id) {
            auto dst_ptr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dst_ptr);
            auto mem = MemRef(dst_ptr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
        } else {
            ScopedTmpReg tmp_reg(encoder);
            // Load pointer to klass from PLT
            enc->GetCodegen()->CreateLoadClassFromPLT(inst, tmp_reg, dst, type_id);
        }
        // Finally load ManagedClass object
        encoder->EncodeLdr(dst, false, MemRef(dst, runtime->GetManagedClassOffset(enc->GetArch())));
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(runtime->ResolveType(method, type_id));
        auto managed_klass = runtime->GetManagedType(klass);
        encoder->EncodeMov(dst, Imm(managed_klass));
    }
}

void EncodeVisitor::FillUnresolvedClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto class_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::CHECK_CAST);
    encoder->EncodeJump(slow_path->GetLabel(), class_reg, Condition::EQ);
    slow_path->CreateBackLabel(encoder);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    encoder->EncodeJump(slow_path->GetBackLabel(), src, Condition::EQ);
    ScopedTmpReg tmp_reg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmp_reg, src);
    encoder->EncodeJump(slow_path->GetLabel(), tmp_reg, class_reg, Condition::NE);
    slow_path->BindBackLabel(encoder);
}

void EncodeVisitor::FillObjectClass(GraphVisitor *visitor, Reg tmp_reg, LabelHolder::LabelId throw_label)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();

    Reg type_reg(tmp_reg.GetId(), INT8_TYPE);
    // Load type class
    encoder->EncodeLdr(type_reg, false, MemRef(tmp_reg, runtime->GetClassTypeOffset(enc->GetArch())));
    // Jump to EH if type not reference
    encoder->EncodeJump(throw_label, type_reg, Imm(runtime->GetReferenceTypeMask()), Condition::NE);
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

void EncodeVisitor::FillOtherClass(GraphVisitor *visitor, Inst *inst, Reg tmp_reg, LabelHolder::LabelId throw_label)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto loop_label = encoder->CreateLabel();

    // First compare `current == klass` we make before switch
    encoder->BindLabel(loop_label);
    // Load base klass
    encoder->EncodeLdr(tmp_reg, false, MemRef(tmp_reg, graph->GetRuntime()->GetClassBaseOffset(enc->GetArch())));
    encoder->EncodeJump(throw_label, tmp_reg, Condition::EQ);
    auto class_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    encoder->EncodeJump(loop_label, tmp_reg, class_reg, Condition::NE);
}

void EncodeVisitor::FillArrayObjectClass(GraphVisitor *visitor, Reg tmp_reg, LabelHolder::LabelId throw_label)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();

    Reg type_reg(tmp_reg.GetId(), INT8_TYPE);
    // Load Component class
    encoder->EncodeLdr(tmp_reg, false, MemRef(tmp_reg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Jump to EH if src is not array class
    encoder->EncodeJump(throw_label, tmp_reg, Condition::EQ);
    // Load type of the component class
    encoder->EncodeLdr(type_reg, false, MemRef(tmp_reg, runtime->GetClassTypeOffset(enc->GetArch())));
    // Jump to EH if type not reference
    encoder->EncodeJump(throw_label, type_reg, Imm(runtime->GetReferenceTypeMask()), Condition::NE);
}

void EncodeVisitor::FillArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmp_reg, LabelHolder::LabelId throw_label)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::CHECK_CAST);

    // Load Component type of Input
    encoder->EncodeLdr(tmp_reg, false, MemRef(tmp_reg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(throw_label, tmp_reg, Condition::EQ);
    // Load Component type of the instance
    auto class_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmp_reg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    encoder->EncodeLdr(tmp_reg1, false, MemRef(class_reg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Compare component types
    encoder->EncodeJump(slow_path->GetLabel(), tmp_reg, tmp_reg1, Condition::NE);

    slow_path->BindBackLabel(encoder);
}

void EncodeVisitor::FillInterfaceClass(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto codegen = enc->GetCodegen();
    if (codegen->GetArch() == Arch::AARCH32) {
        auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::CHECK_CAST);
        encoder->EncodeJump(slow_path->GetLabel());
        slow_path->BindBackLabel(encoder);
    } else {
        codegen->CreateCheckCastInterfaceCall(inst);
    }
}

void EncodeVisitor::FillCheckCast(GraphVisitor *visitor, Inst *inst, Reg src, LabelHolder::LabelId end_label,
                                  compiler::ClassType klass_type)
{
    if (klass_type == ClassType::INTERFACE_CLASS) {
        FillInterfaceClass(visitor, inst);
        return;
    }
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    // class_reg - CheckCast class
    // tmp_reg - input class
    auto class_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmp_reg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmp_reg, src);
    // There is no exception if the classes are equal
    encoder->EncodeJump(end_label, class_reg, tmp_reg, Condition::EQ);

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathCheckCast>(inst, EntrypointId::CLASS_CAST_EXCEPTION);
    slow_path->SetClassReg(class_reg);
    auto throw_label = slow_path->GetLabel();
    switch (klass_type) {
        // The input class should be not primitive type
        case ClassType::OBJECT_CLASS: {
            FillObjectClass(visitor, tmp_reg, throw_label);
            break;
        }
        case ClassType::OTHER_CLASS: {
            FillOtherClass(visitor, inst, tmp_reg, throw_label);
            break;
        }
        // The input class should be array class and component type should be not primitive type
        case ClassType::ARRAY_OBJECT_CLASS: {
            FillArrayObjectClass(visitor, tmp_reg, throw_label);
            break;
        }
        // Check that components types are equals, else call slow path
        case ClassType::ARRAY_CLASS: {
            FillArrayClass(visitor, inst, tmp_reg, throw_label);
            break;
        }
        case ClassType::FINAL_CLASS: {
            EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::CHECKCAST,
                                         events::CodegenSimplificationReason::FINAL_CLASS);
            encoder->EncodeJump(throw_label);
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
    auto type_id = inst->CastToCheckCast()->GetTypeId();
    auto encoder = enc->GetEncoder();

    auto klass_type = inst->CastToCheckCast()->GetClassType();

    if (klass_type == ClassType::UNRESOLVED_CLASS) {
        FillUnresolvedClass(visitor, inst);
        return;
    }
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto end_label = encoder->CreateLabel();

    if (inst->CastToCheckCast()->GetOmitNullCheck()) {
        EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::CHECKCAST,
                                     events::CodegenSimplificationReason::SKIP_NULLCHECK);
    } else {
        // Compare with nullptr
        encoder->EncodeJump(end_label, src, Condition::EQ);
    }

    [[maybe_unused]] auto klass = enc->cg_->GetGraph()->GetRuntime()->GetClass(method, type_id);
    ASSERT(klass != nullptr);

    FillCheckCast(visitor, inst, src, end_label, klass_type);
    encoder->BindLabel(end_label);
}

void EncodeVisitor::FillIsInstanceUnresolved(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto class_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);
    encoder->EncodeJump(slow_path->GetLabel(), class_reg, Condition::EQ);
    slow_path->CreateBackLabel(encoder);
    auto end_label = slow_path->GetBackLabel();

    // Compare with nullptr
    auto next_label = encoder->CreateLabel();
    encoder->EncodeJump(next_label, src, Condition::NE);
    encoder->EncodeMov(dst, Imm(0));
    encoder->EncodeJump(end_label);
    encoder->BindLabel(next_label);

    // Get instance class
    ScopedTmpReg tmp_reg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmp_reg, src);

    // Sets true if the classes are equal
    encoder->EncodeJump(slow_path->GetLabel(), tmp_reg, class_reg, Condition::NE);
    encoder->EncodeMov(dst, Imm(1));

    slow_path->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstanceCaseObject(GraphVisitor *visitor, Inst *inst, Reg tmp_reg)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    // ClassType::OBJECT_CLASS
    Reg type_reg(tmp_reg.GetId(), INT8_TYPE);
    // Load type class
    encoder->EncodeLdr(type_reg, false, MemRef(tmp_reg, runtime->GetClassTypeOffset(enc->GetArch())));
    ScopedTmpReg type_mask_reg(encoder, INT8_TYPE);
    encoder->EncodeMov(type_mask_reg, Imm(runtime->GetReferenceTypeMask()));
    encoder->EncodeCompare(dst, type_mask_reg, type_reg, Condition::EQ);
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

void EncodeVisitor::FillIsInstanceCaseOther(GraphVisitor *visitor, Inst *inst, Reg tmp_reg,
                                            LabelHolder::LabelId end_label)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto class_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);

    // ClassType::OTHER_CLASS
    auto loop_label = encoder->CreateLabel();
    auto false_label = encoder->CreateLabel();

    // First compare `current == klass` we make before switch
    encoder->BindLabel(loop_label);
    // Load base klass
    encoder->EncodeLdr(tmp_reg, false, MemRef(tmp_reg, runtime->GetClassBaseOffset(enc->GetArch())));
    encoder->EncodeJump(false_label, tmp_reg, Condition::EQ);
    encoder->EncodeJump(loop_label, tmp_reg, class_reg, Condition::NE);

    // Set true result and jump to exit
    encoder->EncodeMov(dst, Imm(1));
    encoder->EncodeJump(end_label);

    // Set false result and jump to exit
    encoder->BindLabel(false_label);
    encoder->EncodeMov(dst, Imm(0));
}

// Sets true if the Input class is array class and component type is not primitive type
void EncodeVisitor::FillIsInstanceCaseArrayObject(GraphVisitor *visitor, Inst *inst, Reg tmp_reg,
                                                  LabelHolder::LabelId end_label)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    // ClassType::ARRAY_OBJECT_CLASS
    Reg dst_ref(dst.GetId(), Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    Reg type_reg(tmp_reg.GetId(), INT8_TYPE);
    // Load Component class
    encoder->EncodeLdr(dst_ref, false, MemRef(tmp_reg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(end_label, dst_ref, Condition::EQ);
    // Load type of the component class
    encoder->EncodeLdr(type_reg, false, MemRef(dst_ref, runtime->GetClassTypeOffset(enc->GetArch())));
    ScopedTmpReg type_mask_reg(encoder, INT8_TYPE);
    encoder->EncodeMov(type_mask_reg, Imm(runtime->GetReferenceTypeMask()));
    encoder->EncodeCompare(dst, type_mask_reg, type_reg, Condition::EQ);
}

// Check that components types are equals, else call slow path
void EncodeVisitor::FillIsInstanceCaseArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmp_reg,
                                                 LabelHolder::LabelId end_label)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());

    // ClassType::ARRAY_CLASS
    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);

    auto next_label_1 = encoder->CreateLabel();
    // Load Component type of Input
    encoder->EncodeLdr(tmp_reg, false, MemRef(tmp_reg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(next_label_1, tmp_reg, Condition::NE);
    encoder->EncodeMov(dst, Imm(0));
    encoder->EncodeJump(end_label);
    encoder->BindLabel(next_label_1);
    // Load Component type of the instance
    auto class_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmp_reg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    encoder->EncodeLdr(tmp_reg1, false, MemRef(class_reg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Compare component types
    encoder->EncodeJump(slow_path->GetLabel(), tmp_reg, tmp_reg1, Condition::NE);
    encoder->EncodeMov(dst, Imm(1));

    slow_path->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstanceCaseInterface(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    // ClassType::INTERFACE_CLASS
    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);

    encoder->EncodeJump(slow_path->GetLabel());

    slow_path->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstance(GraphVisitor *visitor, Inst *inst, Reg tmp_reg, LabelHolder::LabelId end_label)
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
        auto next_label = encoder->CreateLabel();
        encoder->EncodeJump(next_label, src, Condition::NE);
        encoder->EncodeMov(dst, Imm(0));
        encoder->EncodeJump(end_label);
        encoder->BindLabel(next_label);
    }

    auto class_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    enc->GetCodegen()->LoadClassFromObject(tmp_reg, src);

    // Sets true if the classes are equals
    if (inst->CastToIsInstance()->GetClassType() == ClassType::FINAL_CLASS) {
        encoder->EncodeCompare(dst, class_reg, tmp_reg, Condition::EQ);
    } else if (dst.GetId() != src.GetId() && dst.GetId() != class_reg.GetId()) {
        encoder->EncodeCompare(dst, class_reg, tmp_reg, Condition::EQ);
        encoder->EncodeJump(end_label, dst, Condition::NE);
    } else {
        auto next_label_1 = encoder->CreateLabel();
        encoder->EncodeJump(next_label_1, class_reg, tmp_reg, Condition::NE);
        encoder->EncodeMov(dst, Imm(1));
        encoder->EncodeJump(end_label);
        encoder->BindLabel(next_label_1);
    }
}

void EncodeVisitor::VisitIsInstance(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto klass_type = inst->CastToIsInstance()->GetClassType();
    if (klass_type == ClassType::UNRESOLVED_CLASS) {
        FillIsInstanceUnresolved(visitor, inst);
        return;
    }
    // tmp_reg - input class
    ScopedTmpReg tmp_reg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    auto end_label = encoder->CreateLabel();

    FillIsInstance(visitor, inst, tmp_reg, end_label);
    switch (klass_type) {
        // Sets true if the Input class is not primitive type
        case ClassType::OBJECT_CLASS: {
            FillIsInstanceCaseObject(visitor, inst, tmp_reg);
            break;
        }
        case ClassType::OTHER_CLASS: {
            FillIsInstanceCaseOther(visitor, inst, tmp_reg, end_label);
            break;
        }
        // Sets true if the Input class is array class and component type is not primitive type
        case ClassType::ARRAY_OBJECT_CLASS: {
            FillIsInstanceCaseArrayObject(visitor, inst, tmp_reg, end_label);
            break;
        }
        // Check that components types are equals, else call slow path
        case ClassType::ARRAY_CLASS: {
            FillIsInstanceCaseArrayClass(visitor, inst, tmp_reg, end_label);
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
    encoder->BindLabel(end_label);
}

void Codegen::CreateMonitorCall(MonitorInst *inst)
{
    auto src = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto id = inst->IsExit() ? EntrypointId::MONITOR_EXIT_FAST_PATH : EntrypointId::MONITOR_ENTER_FAST_PATH;

    auto param_regs {GetLiveRegisters(inst).first};
    param_regs &= GetTarget().GetParamRegsMask(2U) | RegMask(MakeMask(GetTarget().GetReturnRegId()));

    SaveCallerRegisters(param_regs, VRegMask(), false);

    FillCallParams({src});
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), id));
    GetEncoder()->MakeCall(entry);
    CreateStackMap(inst);
    LoadCallerRegisters(param_regs, VRegMask(), false);
}

void Codegen::CreateMonitorCallOld(MonitorInst *inst)
{
    auto src = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto id = inst->IsExit() ? EntrypointId::UNLOCK_OBJECT : EntrypointId::LOCK_OBJECT;
    CallRuntime(inst, id, dst, {src});
}

void Codegen::CreateCheckCastInterfaceCall(Inst *inst)
{
    auto enc = GetEncoder();
    auto param_regs {GetLiveRegisters(inst).first};
    param_regs &= GetTarget().GetParamRegsMask(2U);
    SaveCallerRegisters(param_regs, VRegMask(), false);
    auto obj = ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto interface = ConvertRegister(inst->GetSrcReg(SECOND_OPERAND), DataType::REFERENCE);
    FillCallParams({obj, interface});
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), EntrypointId::CHECK_CAST_INTERFACE));
    enc->MakeCall(entry);
    CreateStackMap(inst);
    LoadCallerRegisters(param_regs, VRegMask(), false);
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

Reg Codegen::AcquireNonLiveReg(RegMask *livemask)
{
    Reg reg = INVALID_REGISTER;
    for (int k = 0; k < MAX_NUM_REGS; ++k) {
        if (!livemask->test(k)) {
            reg = Reg(k, INT32_TYPE);
            livemask->set(k);
            break;
        }
    }
    ASSERT(reg != INVALID_REGISTER);
    return reg;
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
    auto curr_offset = GetEncoder()->GetCursorOffset();
    ASSERT(curr_offset > prevOffset);
    GetCodeBuilder()->AddImplicitNullCheck(curr_offset, curr_offset - prevOffset);
    CreateStackMap(nullcheck, inst);
}

void Codegen::CreateFloatIsInf([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src,
                               [[maybe_unused]] RegMask *lvrmask)
{
    GetEncoder()->EncodeIsInf(dst, src[0]);
}

#include "intrinsics_codegen.inl"

void Codegen::CreateBuiltinIntrinsic(IntrinsicInst *inst)
{
    Reg dst = INVALID_REGISTER;
    SRCREGS src;

    if (!inst->NoDest()) {
        dst = ConvertRegister(inst->GetDstReg(), inst->GetType());
    }

    auto live_regs = GetLiveRegisters(inst).first;
    /*  regalloc doesn't set the destination register in the live
     *  registers mask as it considers it dead after the instruction
     *  is executed, set this bit for AcquireNonLiveReg() to work
     *  correctly */
    if (dst != INVALID_REGISTER && !DataType::IsFloatType(inst->GetType())) {
        live_regs.set(dst.GetId());
    }
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        if (inst->GetInput(i).GetInst()->IsSaveState()) {
            continue;
        }
        auto location = inst->GetLocation(i);
        ASSERT(location.IsFixedRegister());
        auto type = inst->GetInputType(i);
        src[i] = ConvertRegister(location.GetValue(), type);
        if (!DataType::IsFloatType(type)) {
            live_regs.set(src[i].GetId());
        }
    }
    FillBuiltin(inst, src, dst, &live_regs);
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
    auto len_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    ASSERT(inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveState);
    EntrypointId entrypoint = inst->CastToBoundsCheckI()->IsArray()
                                  ? EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION
                                  : EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;

    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, entrypoint);

    auto value = inst->CastToBoundsCheckI()->GetImm();
    if (enc->GetEncoder()->CanEncodeImmAddSubCmp(value, WORD_SIZE, false)) {
        auto imm = enc->GetCodegen()->ConvertImm(value, DataType::INT64);
        enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), len_reg, imm, Condition::LS);
    } else {
        ScopedTmpReg tmp(enc->GetEncoder(), len_reg.GetType());
        enc->GetEncoder()->EncodeMov(tmp, Imm(value));
        enc->GetEncoder()->EncodeJump(slow_path->GetLabel(), len_reg, tmp, Condition::LS);
    }
}

void EncodeVisitor::VisitStoreArrayI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto array_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto type = inst->GetType();
    auto value = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto index = inst->CastToStoreArrayI()->GetImm();
    int64_t offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                     (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    auto object_header_mem = MemRef(array_reg);
    auto mem = MemRef(array_reg, offset);
    if (inst->CastToStoreArrayI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStr(value, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
    if (inst->CastToStoreArrayI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, object_header_mem, value, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitLoadArrayI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto inst_load_array_i = inst->CastToLoadArrayI();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    ASSERT(inst_load_array_i->IsArray() || !runtime->IsCompressedStringsEnabled());
    if (inst_load_array_i->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto array_reg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0));
    uint32_t index = inst_load_array_i->GetImm();
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    int64_t data_offset = inst_load_array_i->IsArray() ? runtime->GetArrayDataOffset(enc->GetArch())
                                                       : runtime->GetStringDataOffset(enc->GetArch());
    uint32_t shift = DataType::ShiftByType(type, enc->GetArch());
    int64_t offset = data_offset + (index << shift);
    auto mem = MemRef(array_reg, offset);
    auto encoder = enc->GetEncoder();
    auto arch = enc->GetArch();
    ScopedTmpReg scoped_tmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    encoder->EncodeLdr(dst, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
}

void EncodeVisitor::VisitLoadCompressedStringCharI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0));                   // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);  // length
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);               // load value
    int32_t offset = runtime->GetStringDataOffset(enc->GetArch());
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

    auto array_inst = inst->CastToMultiArray();
    codegen->CreateMultiArrayCall(array_inst);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(MemoryOrder::Release);
    }
}

void EncodeVisitor::VisitCallStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();

    auto call_inst = inst->CastToCallStatic();
    codegen->CreateCall(call_inst);
}

void EncodeVisitor::VisitUnresolvedCallStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();

    auto call_inst = inst->CastToUnresolvedCallStatic();
    codegen->CreateCall(call_inst);
}

void EncodeVisitor::VisitCallVirtual(GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto call_inst = inst->CastToCallVirtual();

    // PC in frame must to be filled by SaveState
    enc->GetCodegen()->CreateVirtualCall(call_inst);
}

void EncodeVisitor::VisitCallDynamic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();

    auto call_inst = inst->CastToCallDynamic();
    codegen->CreateDynamicCall(call_inst);
}

void EncodeVisitor::VisitUnresolvedCallVirtual(GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto call_inst = inst->CastToUnresolvedCallVirtual();

    // PC in frame must to be filled by SaveState
    enc->GetCodegen()->CreateVirtualCall(call_inst);
}

void EncodeVisitor::VisitSafePoint(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto graph = codegen->GetGraph();
    auto encoder = enc->GetEncoder();
    int64_t flag_addr_offset = graph->GetRuntime()->GetFlagAddrOffset(codegen->GetArch());
    ScopedTmpRegU16 tmp(encoder);

    // TMP <= Flag
    auto mem = MemRef(codegen->ThreadReg(), flag_addr_offset);
    encoder->EncodeLdr(tmp, false, mem);

    // check value and jump to call GC
    auto slow_path = codegen->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::SAFEPOINT);

    encoder->EncodeJump(slow_path->GetLabel(), tmp, Condition::NE);

    slow_path->BindBackLabel(encoder);
}

void EncodeVisitor::VisitSelect(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto dst_type = inst->GetType();
    auto cmp_type = inst->CastToSelect()->GetOperandsType();

    constexpr int32_t IMM_2 = 2;
    constexpr int32_t IMM_3 = 3;
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), dst_type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), dst_type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), dst_type);
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), cmp_type);
    auto src3 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_3), cmp_type);
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
    auto dst_type = inst->GetType();
    auto cmp_type = inst->CastToSelectImm()->GetOperandsType();

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), dst_type);
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), dst_type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), dst_type);
    constexpr int32_t IMM_2 = 2;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), cmp_type);
    auto imm = enc->GetCodegen()->ConvertImm(inst->CastToSelectImm()->GetImm(), cmp_type);
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
    auto imm = enc->GetCodegen()->ConvertImm(inst->CastToIfImm()->GetImm(), DataType::INT64);
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
    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::DEOPTIMIZE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = Reg(inst->GetSrcReg(0), INT32_TYPE);
    auto src1 = Reg(inst->GetSrcReg(1), INT32_TYPE);
    enc->GetEncoder()->EncodeAddOverflow(slow_path->GetLabel(), dst, src0, src1, Condition::VS);
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
    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::DEOPTIMIZE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = Reg(inst->GetSrcReg(0), INT32_TYPE);
    auto src1 = Reg(inst->GetSrcReg(1), INT32_TYPE);
    enc->GetEncoder()->EncodeSubOverflow(slow_path->GetLabel(), dst, src0, src1, Condition::VS);
}

void EncodeVisitor::VisitLoadArrayPair(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->CastToLoadArrayPair()->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);      // index
    auto dst0 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(0), type);                 // first value
    auto dst1 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(1), type);                 // second value
    int64_t offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch());
    ScopedTmpReg tmp(enc->GetEncoder());

    int32_t scale = DataType::ShiftByType(type, enc->GetCodegen()->GetArch());
    enc->GetEncoder()->EncodeAdd(tmp, src0, Shift(src1, scale));
    auto mem = MemRef(tmp, offset);
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeLdp(dst0, dst1, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
}

void EncodeVisitor::VisitLoadArrayPairI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->CastToLoadArrayPairI()->GetNeedBarrier()) {
        // !TODO Ishin Pavel inserts barriers for GC
    }
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto dst0 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(0), type);                 // first value
    auto dst1 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(1), type);                 // second value
    uint64_t index = inst->CastToLoadArrayPairI()->GetImm();
    int64_t offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                     (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    auto mem = MemRef(src0, offset);

    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeLdp(dst0, dst1, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
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
    constexpr int32_t IMM_2 = 2;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), type);  // first value
    constexpr int32_t IMM_3 = 3;
    auto src3 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_3), type);  // second value
    int32_t offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch());
    int32_t scale = DataType::ShiftByType(type, enc->GetCodegen()->GetArch());

    ScopedLiveTmpReg tmp(enc->GetEncoder(), src0.GetType());

    auto object_header_mem = MemRef(src0);
    enc->GetEncoder()->EncodeAdd(tmp, src0, Shift(src1, scale));
    auto mem = MemRef(tmp, offset);
    if (inst->CastToStoreArrayPair()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, true);
    }
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStp(src2, src3, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
    if (inst->CastToStoreArrayPair()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, object_header_mem, src2, src3);
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
    int64_t offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                     (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    auto object_header_mem = MemRef(src0);
    auto mem = MemRef(src0, offset);
    if (inst->CastToStoreArrayPairI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, true);
    }
    auto prev_offset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStp(src1, src2, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prev_offset);
    if (inst->CastToStoreArrayPairI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, object_header_mem, src1, src2);
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
    SlowPathCheck slow_path(codegen->GetEncoder()->CreateLabel(), inst, EntrypointId::THROW_EXCEPTION);
    slow_path.Generate(codegen);
}

void EncodeVisitor::VisitDeoptimize(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    ASSERT(inst->GetSaveState() != nullptr);

    SlowPathCheck slow_path(codegen->GetEncoder()->CreateLabel(), inst, EntrypointId::DEOPTIMIZE);
    slow_path.Generate(codegen);
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
    auto obj_reg = codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetType());
    ASSERT(obj_reg.IsValid());
    codegen->LoadClassFromObject(dst, obj_reg);
}

void EncodeVisitor::VisitClassImmediate(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());

    codegen->GetEncoder()->EncodeMov(dst,
                                     Imm(reinterpret_cast<uintptr_t>(inst->CastToClassImmediate()->GetClassPtr())));
}

void EncodeVisitor::VisitRegDef([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst) {}

void EncodeVisitor::VisitLiveIn([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst) {}

void EncodeVisitor::VisitLiveOut([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();

    codegen->AddLiveOut(inst->GetBasicBlock(), inst->GetDstReg());

    auto dst_reg = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    if (codegen->GetTarget().GetTempRegsMask().Test(dst_reg.GetId()) &&
        enc->GetEncoder()->IsScratchRegisterReleased(dst_reg)) {
        enc->GetEncoder()->AcquireScratchRegister(dst_reg);
    }

    if (inst->GetSrcReg(0) != inst->GetDstReg()) {
        enc->GetEncoder()->EncodeMov(dst_reg, codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetType()));
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

    if (cvai->GetInputType(0) == DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
        return;
    }

    if (TryCastValueToAnyTypePluginGen(cvai, enc)) {
        return;
    }

    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), DataType::Type::BOOL);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitAnyTypeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *check_inst = inst->CastToAnyTypeCheck();

    if (check_inst->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
    }
    // Empty check
    if (check_inst->GetAnyType() == AnyBaseType::UNDEFINED_TYPE) {
        return;
    }
    auto slow_path = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, EntrypointId::DEOPTIMIZE);

    if (TryAnyTypeCheckPluginGen(check_inst, enc, slow_path->GetLabel())) {
        return;
    }
    UNREACHABLE();
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
        auto dst_type = inst->GetType();
        if (arch == Arch::AARCH64 || arch == Arch::X86_64) {
            return dst_type == DataType::FLOAT32 || dst_type == DataType::FLOAT64;
        }
        return arch == Arch::AARCH32;
    }
    if (op == Opcode::Div && arch == Arch::AARCH32) {
        auto dst_type = inst->GetType();
        return dst_type == DataType::INT64 || dst_type == DataType::UINT64;
    }
    if (op == Opcode::Cast && arch == Arch::AARCH32) {
        auto dst_type = inst->GetType();
        auto src_type = inst->GetInputType(0);
        if (dst_type == DataType::FLOAT32 || dst_type == DataType::FLOAT64) {
            return src_type == DataType::INT64 || src_type == DataType::UINT64;
        }
        if (src_type == DataType::FLOAT32 || src_type == DataType::FLOAT64) {
            return dst_type == DataType::INT64 || dst_type == DataType::UINT64;
        }
        return false;
    }
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
}  // namespace panda::compiler
