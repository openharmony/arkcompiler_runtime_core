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

#ifndef COMPILER_OPTIMIZER_CODEGEN_CODEGEN_H_
#define COMPILER_OPTIMIZER_CODEGEN_CODEGEN_H_

/*
Codegen interface for compiler
! Do not use this file in runtime
*/

#include "code_info/code_info_builder.h"
#include "compiler_logger.h"
#include "disassembly.h"
#include "frame_info.h"
#include "optimizer/analysis/live_registers.h"
#include "optimizer/code_generator/callconv.h"
#include "optimizer/code_generator/encode.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/code_generator/slow_path.h"
#include "optimizer/code_generator/spill_fill_encoder.h"
#include "optimizer/code_generator/target_info.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "optimizer/pass_manager.h"
#include "utils/cframe_layout.h"

namespace panda::compiler {
// Maximum size in bytes
constexpr size_t INST_IN_SLOW_PATH = 64;

class Encoder;
class CodeBuilder;
class OsrEntryStub;

class Codegen : public Optimization {
    using EntrypointId = RuntimeInterface::EntrypointId;

public:
    explicit Codegen(Graph *graph);
    NO_MOVE_SEMANTIC(Codegen);
    NO_COPY_SEMANTIC(Codegen);

    ~Codegen() override = default;

    bool RunImpl() override;
    const char *GetPassName() const override;
    bool AbortIfFailed() const override;

    static bool Run(Graph *graph);

    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }
    ArenaAllocator *GetLocalAllocator() const
    {
        return local_allocator_;
    }
    FrameInfo *GetFrameInfo() const
    {
        return frame_info_;
    }
    void SetFrameInfo(FrameInfo *frame_info)
    {
        frame_info_ = frame_info;
    }
    virtual void CreateFrameInfo();

    RuntimeInterface *GetRuntime() const
    {
        return runtime_;
    }
    RegistersDescription *GetRegfile() const
    {
        return regfile_;
    }
    Encoder *GetEncoder() const
    {
        return enc_;
    }
    CallingConvention *GetCallingConvention() const
    {
        return callconv_;
    }

    GraphVisitor *GetGraphVisitor() const
    {
        return visitor_;
    }

    LabelHolder::LabelId GetLabelEntry() const
    {
        return label_entry_;
    }

    LabelHolder::LabelId GetLabelExit() const
    {
        return label_exit_;
    }

    RuntimeInterface::MethodId GetMethodId()
    {
        return method_id_;
    }

    void SetStartCodeOffset(size_t offset)
    {
        start_code_offset_ = offset;
    }

    size_t GetStartCodeOffset() const
    {
        return start_code_offset_;
    }

    size_t GetLanguageExtensionOffsetFromSpInBytes();

    void Convert(ArenaVector<Reg> *regs_usage, const ArenaVector<bool> *mask, TypeInfo type_info);

    Reg ConvertRegister(Register ref, DataType::Type type = DataType::Type::INT64);

    Imm ConvertImm(uint64_t imm, DataType::Type type);

    Imm ConvertImmWithExtend(uint64_t imm, DataType::Type type);

    Imm ConvertImm(ConstantInst *const_inst, DataType::Type type);

    Condition ConvertCc(ConditionCode cc);
    Condition ConvertCcOverflow(ConditionCode cc);

    static inline TypeInfo ConvertDataType(DataType::Type type, Arch arch)
    {
        return TypeInfo::FromDataType(type, arch);
    }

    Arch GetArch() const
    {
        return GetTarget().GetArch();
    }

    Target GetTarget() const
    {
        return target_;
    }

    TypeInfo GetPtrRegType() const
    {
        return target_.GetPtrRegType();
    }

    CodeInfoBuilder *GetCodeBuilder() const
    {
        return code_builder_;
    }

    void CreateStackMap(Inst *inst, Inst *user = nullptr);

    void CreateStackMapRec(SaveStateInst *save_state, bool require_vreg_map, Inst *target_site);
    void CreateVRegMap(SaveStateInst *save_state, size_t vregs_count, Inst *target_site);
    void CreateVreg(const Location &location, Inst *inst, const VirtualRegister &vreg);
    void FillVregIndices(SaveStateInst *save_state);

    void CreateOsrEntry(SaveStateInst *save_state);

    void CreateVRegForRegister(const Location &location, Inst *inst, const VirtualRegister &vreg);

    /**
     * 'live_inputs' shows that inst's source registers should be added the the mask
     */
    template <bool live_inputs = false>
    std::pair<RegMask, VRegMask> GetLiveRegisters(Inst *inst)
    {
        RegMask live_regs;
        VRegMask live_fp_regs;
        if (!options.IsCompilerSaveOnlyLiveRegisters() || inst == nullptr) {
            live_regs.set();
            live_fp_regs.set();
            return {live_regs, live_fp_regs};
        }
        // Run LiveRegisters pass only if it is actually required
        if (!GetGraph()->IsAnalysisValid<LiveRegisters>()) {
            GetGraph()->RunPass<LiveRegisters>();
        }

        // Add registers from intervals that are live at inst's definition
        auto &lr = GetGraph()->GetAnalysis<LiveRegisters>();
        lr.VisitIntervalsWithLiveRegisters<live_inputs>(inst, [&live_regs, &live_fp_regs, this](const auto &li) {
            auto reg = ConvertRegister(li->GetReg(), li->GetType());
            GetEncoder()->SetRegister(&live_regs, &live_fp_regs, reg);
        });

        // Add live temp registers
        live_regs |= GetEncoder()->GetLiveTmpRegMask();
        live_fp_regs |= GetEncoder()->GetLiveTmpFpRegMask();

        return {live_regs, live_fp_regs};
    }

    // Limits live register set to a number of registers used to pass parameters to the runtime call:
    // 1) these ones are saved/restored by caller
    // 2) the remaining ones are saved/restored by the bridge function (aarch only)
    void FillOnlyParameters(RegMask *live_regs, uint32_t num_params) const;

    template <typename T, typename... Args>
    T *CreateSlowPath(Inst *inst, Args &&... args)
    {
        static_assert(std::is_base_of_v<SlowPathBase, T>);
        auto label = GetEncoder()->CreateLabel();
        auto slow_path = GetLocalAllocator()->New<T>(label, inst, std::forward<Args>(args)...);
        slow_paths_.push_back(slow_path);
        return slow_path;
    }

    void EmitSlowPaths();

    void InsertTrace(std::initializer_list<std::variant<Reg, Imm>> params);

    void CallIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId id);

    // The function is used for calling runtime functions through special bridges.
    // !NOTE Don't use the function for calling runtime without bridges(it save only parameters on stack)
    void CallRuntime(Inst *inst, EntrypointId id, Reg dst_reg, std::initializer_list<std::variant<Reg, Imm>> params,
                     RegMask preserved_regs = {});

    template <typename... Args>
    void CallRuntimeWithMethod(Inst *inst, void *method, EntrypointId eid, Reg dst_reg, Args &&... params)
    {
        if (GetGraph()->IsAotMode()) {
            ScopedTmpReg method_reg(GetEncoder());
            LoadMethod(method_reg);
            CallRuntime(inst, eid, dst_reg, {method_reg, params...});
        } else {
            if (Is64BitsArch(GetArch())) {
                CallRuntime(inst, eid, dst_reg, {Imm(reinterpret_cast<uint64_t>(method)), params...});
            } else {
                // uintptr_t causes problems on host cross-jit compilation
                CallRuntime(inst, eid, dst_reg, {Imm(down_cast<uint32_t>(method)), params...});
            }
        }
    }

    void SaveRegistersForImplicitRuntime(Inst *inst, RegMask *params_mask, RegMask *mask);

    void VisitNewArray(Inst *inst);

    void LoadClassFromObject(Reg class_reg, Reg obj_reg);
    void CreateCall(CallInst *call_inst);
    void VisitCallIndirect(CallIndirectInst *inst);
    void VisitCall(CallInst *inst);
    void CreateUnresolvedVirtualMethodLoad(CallInst *vcall, Reg method);
    void CreateVirtualCall(CallInst *call_inst);
    void CreateDynamicCall(CallInst *call_inst);
    void CreateCallIntrinsic(IntrinsicInst *inst);
    void CreateMultiArrayCall(CallInst *call_inst);
    void CreateNewObjCall(NewObjectInst *new_obj);
    void CreateNewObjCallOld(NewObjectInst *new_obj);
    void CreateMonitorCall(MonitorInst *inst);
    void CreateMonitorCallOld(MonitorInst *inst);
    void CreateCheckCastInterfaceCall(Inst *inst);
    void CreateNonDefaultInitClass(ClassInst *init_inst);
    void CreatePreWRB(Inst *inst, MemRef mem, bool store_pair = false);
    void CreatePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2 = INVALID_REGISTER);
    void EncodePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool check_nullptr = true);
    void CreatePostInterRegionBarrier(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool check_nullptr);
    void CreatePostInterGenerationalBarrier(MemRef mem);
    void CallBarrier(RegMask live_regs, VRegMask live_vregs, EntrypointId id,
                     const std::initializer_list<std::variant<Reg, Imm>> &params);
    void CreateLoadClassFromPLT(Inst *inst, Reg tmp_reg, Reg dst, size_t class_id);
    void CreateJumpToClassResolverPltShared(Inst *inst, Reg tmp_reg, RuntimeInterface::EntrypointId id);
    void CreateLoadTLABInformation(Reg reg_tlab_start, Reg reg_tlab_size);
    void CreateCheckForTLABWithConstSize(Inst *inst, Reg reg_tlab_start, Reg reg_tlab_size, size_t size,
                                         LabelHolder::LabelId label);
    void CreateDebugRuntimeCallsForNewObject(Inst *inst, Reg reg_tlab_start, size_t alloc_size, RegMask preserved);
    void CreateDebugRuntimeCallsForCreateString(Inst *inst, Reg dst);
    void CreateReturn(const Inst *inst);

    // The function alignment up the value from alignment_reg using tmp_reg.
    void CreateAlignmentValue(Reg alignment_reg, Reg tmp_reg, size_t alignment);
    void TryInsertImplicitNullCheck(Inst *inst, size_t prevOffset);

    const CFrameLayout &GetFrameLayout() const
    {
        return frame_layout_;
    }

    bool RegisterKeepCallArgument(CallInst *call_inst, Reg reg);

    void LoadMethod(Reg dst);
    void LoadFreeSlot(Reg dst);
    void StoreFreeSlot(Reg src);

    ssize_t GetStackOffset(Location location)
    {
        if (location.GetKind() == LocationType::STACK_ARGUMENT) {
            return location.GetValue() * GetFrameLayout().GetSlotSize();
        }

        if (location.GetKind() == LocationType::STACK_PARAMETER) {
            return GetFrameLayout().GetFrameSize<CFrameLayout::BYTES>() +
                   (location.GetValue() * GetFrameLayout().GetSlotSize());
        }

        ASSERT(location.GetKind() == LocationType::STACK);
        return GetFrameLayout().GetSpillOffsetFromSpInBytes(location.GetValue());
    }

    MemRef GetMemRefForSlot(Location location)
    {
        ASSERT(location.IsAnyStack());
        return MemRef(SpReg(), GetStackOffset(location));
    }

    Reg SpReg() const
    {
        return GetTarget().GetStackReg();
    }

    Reg FpReg() const
    {
        return GetTarget().GetFrameReg();
    }

    bool HasLiveCallerSavedRegs(Inst *inst);
    void SaveCallerRegisters(RegMask live_regs, VRegMask live_vregs, bool adjust_regs);
    void LoadCallerRegisters(RegMask live_regs, VRegMask live_vregs, bool adjust_regs);

    // Initialization internal variables
    void Initialize();

    const Disassembly *GetDisasm() const
    {
        return &disasm_;
    }

    Disassembly *GetDisasm()
    {
        return &disasm_;
    }

    void AddLiveOut(const BasicBlock *bb, const Register reg)
    {
        live_outs_[bb].Set(reg);
    }

    RegMask GetLiveOut(const BasicBlock *bb) const
    {
        auto it = live_outs_.find(bb);
        return it != live_outs_.end() ? it->second : RegMask();
    }

    Reg ThreadReg() const
    {
        return Reg(GetThreadReg(GetArch()), GetTarget().GetPtrRegType());
    }

    static bool InstEncodedWithLibCall(const Inst *inst, Arch arch);

protected:
    virtual void GeneratePrologue();
    virtual void GenerateEpilogue();

    // Main logic steps
    bool BeginMethod();
    bool VisitGraph();
    void EndMethod();
    bool CopyToCodeCache();
    void DumpCode();

    RegMask GetUsedRegs() const
    {
        return used_regs_;
    }
    RegMask GetUsedVRegs() const
    {
        return used_vregs_;
    }

    void FillCallParams(const std::initializer_list<std::variant<Reg, Imm>> &params);

    void EmitJump(const BasicBlock *bb);

    bool EmitCallRuntimeCode(Inst *inst, EntrypointId id);

    void PrepareAndEmitCallVirtual(CallInst *call_inst);

    void IntfInlineCachePass(CallInst *call_inst, Reg method_reg, Reg tmp_reg, Reg obj_reg);

    void EmitCallVirtual(Reg method_reg);

    void PrepareCallVirtualAot(CallInst *call_inst, Reg method_reg);
    void PrepareCallVirtual(CallInst *call_inst, Reg method_reg);

    uint32_t GetVtableShift();

    void CalculateCardIndex(MemRef mem, ScopedTmpReg *tmp, ScopedTmpReg *tmp1);

    void EmitGetUnresolvedCalleeMethod(CallInst *call_inst);

    void EmitCreateCallCode(CallInst *call_inst);

    void EmitEpilogueForCreateCall(CallInst *call_inst);

    void CreateBuiltinIntrinsic(IntrinsicInst *inst);
    static constexpr int32_t NUM_OF_SRC_BUILTIN = 6;
    static constexpr uint8_t FIRST_OPERAND = 0;
    static constexpr uint8_t SECOND_OPERAND = 1;
    static constexpr uint8_t THIRD_OPERAND = 2;
    static constexpr uint8_t FOURTH_OPERAND = 3;
    static constexpr uint8_t FIFTH_OPERAND = 4;
    using SRCREGS = std::array<Reg, NUM_OF_SRC_BUILTIN>;
    // implementation is generated with compiler/optimizer/templates/intrinsics/intrinsics_codegen.inl.erb
    void FillBuiltin(IntrinsicInst *inst, SRCREGS src, Reg dst, RegMask *mask);
    static Reg AcquireNonLiveReg(RegMask *mask);

    void AddParamRegsInLiveMasks(RegMask *live_regs, VRegMask *live_vregs,
                                 const std::initializer_list<std::variant<Reg, Imm>> &params);

    void CreateStubCall(Inst *inst, RuntimeInterface::IntrinsicId intrinsicId, Reg dst,
                        const std::initializer_list<std::variant<Reg, Imm>> &params);

    ScopedTmpReg CalculatePreviousTLABAllocSize(Reg reg, LabelHolder::LabelId label);
    friend class IntrinsicCodegenTest;

    virtual void IntrinsicSlowPathEntry([[maybe_unused]] IntrinsicInst *inst)
    {
        GetEncoder()->SetFalseResult();
    }
    virtual void IntrinsicCallRuntimeSaveAll([[maybe_unused]] IntrinsicInst *inst)
    {
        GetEncoder()->SetFalseResult();
    }
    virtual void IntrinsicSaveRegisters([[maybe_unused]] IntrinsicInst *inst)
    {
        GetEncoder()->SetFalseResult();
    }
    virtual void IntrinsicRestoreRegisters([[maybe_unused]] IntrinsicInst *inst)
    {
        GetEncoder()->SetFalseResult();
    }
    virtual void IntrinsicTailCall([[maybe_unused]] IntrinsicInst *inst)
    {
        GetEncoder()->SetFalseResult();
    }

#include "codegen_language_extensions.h"
#include "intrinsics_codegen.inl.h"

private:
    template <typename T>
    void EncodeImms(const T &imms)
    {
        auto param_info = GetCallingConvention()->GetParameterInfo(0);
        auto imm_type = DataType::INT32;
        for (auto imm : imms) {
            auto location = param_info->GetNextLocation(imm_type);
            ASSERT(location.IsFixedRegister());
            auto dst_reg = ConvertRegister(location.GetValue(), imm_type);
            GetEncoder()->EncodeMov(dst_reg, Imm(imm));
        }
    }

private:
    ArenaAllocator *allocator_;
    ArenaAllocator *local_allocator_;
    // Register description
    RegistersDescription *regfile_;
    // Encoder implementation
    Encoder *enc_;
    // Target architecture calling convention model
    CallingConvention *callconv_;
    // Current execution model implementation
    // Visitor for instructions
    GraphVisitor *visitor_ {};

    CodeInfoBuilder *code_builder_ {nullptr};

    ArenaVector<SlowPathBase *> slow_paths_;
    ArenaUnorderedMap<RuntimeInterface::EntrypointId, SlowPathShared *> slow_paths_map_;

    const CFrameLayout frame_layout_;  // NOLINT(readability-identifier-naming)

    ArenaVector<OsrEntryStub *> osr_entries_;

    RuntimeInterface::MethodId method_id_ {INVALID_ID};

    size_t start_code_offset_ {0};

    ArenaVector<std::pair<int16_t, int16_t>> vreg_indices_;

    RuntimeInterface *runtime_ {nullptr};

    LabelHolder::LabelId label_entry_ {};
    LabelHolder::LabelId label_exit_ {};

    FrameInfo *frame_info_ {nullptr};

    const Target target_;

    /* Registers that have been allocated by regalloc */
    RegMask used_regs_ {0};
    RegMask used_vregs_ {0};

    /* Map of BasicBlock to live-out regsiters mask. It is needed in epilogue encoding to avoid overwriting of the
     * live-out registers */
    ArenaUnorderedMap<const BasicBlock *, RegMask> live_outs_;

    Disassembly disasm_;

    SpillFillsResolver spill_fills_resolver_;

    friend class EncodeVisitor;
    friend class BaselineCodegen;

    void CreateStubCall(RuntimeInterface::IntrinsicId intrinsicId, Reg dst,
                        const std::initializer_list<std::variant<Reg, Imm>> &params);
};  // Codegen

class EncodeVisitor : public GraphVisitor {
    using EntrypointId = RuntimeInterface::EntrypointId;

public:
    explicit EncodeVisitor(Codegen *cg) : cg_(cg), arch_(cg->GetArch()) {}

    EncodeVisitor() = delete;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return cg_->GetGraph()->GetBlocksRPO();
    }
    Codegen *GetCodegen() const
    {
        return cg_;
    }
    Encoder *GetEncoder()
    {
        return cg_->GetEncoder();
    }
    Arch GetArch() const
    {
        return arch_;
    }
    CallingConvention *GetCallingConvention()
    {
        return cg_->GetCallingConvention();
    }

    RegistersDescription *GetRegfile()
    {
        return cg_->GetRegfile();
    }

    bool GetResult()
    {
        return success_ && cg_->GetEncoder()->GetResult();
    }

    // For each group of SpillFillData representing spill or fill operations and
    // sharing the same source and destination types order by stack slot number in descending order.
    static void SortSpillFillData(ArenaVector<SpillFillData> *spill_fills);
    // Checks if two spill-fill operations could be coalesced into single operation over pair of arguments.
    static bool CanCombineSpillFills(SpillFillData pred, SpillFillData succ, const CFrameLayout &fl,
                                     const Graph *graph);

protected:
    // UnaryOperation
    static void VisitMov(GraphVisitor *visitor, Inst *inst);
    static void VisitNeg(GraphVisitor *visitor, Inst *inst);
    static void VisitAbs(GraphVisitor *visitor, Inst *inst);
    static void VisitNot(GraphVisitor *visitor, Inst *inst);
    static void VisitSqrt(GraphVisitor *visitor, Inst *inst);

    // BinaryOperation
    static void VisitAdd(GraphVisitor *visitor, Inst *inst);
    static void VisitSub(GraphVisitor *visitor, Inst *inst);
    static void VisitMul(GraphVisitor *visitor, Inst *inst);
    static void VisitShl(GraphVisitor *visitor, Inst *inst);
    static void VisitAShr(GraphVisitor *visitor, Inst *inst);
    static void VisitAnd(GraphVisitor *visitor, Inst *inst);
    static void VisitOr(GraphVisitor *visitor, Inst *inst);
    static void VisitXor(GraphVisitor *visitor, Inst *inst);

    // Binary Overflow Operation
    static void VisitAddOverflow(GraphVisitor *v, Inst *inst);
    static void VisitAddOverflowCheck(GraphVisitor *v, Inst *inst);
    static void VisitSubOverflow(GraphVisitor *v, Inst *inst);
    static void VisitSubOverflowCheck(GraphVisitor *v, Inst *inst);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryImmOperation(opc) static void Visit##opc##I(GraphVisitor *visitor, Inst *inst);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARRY_IMM_OPS(DEF) DEF(Add) DEF(Sub) DEF(Shl) DEF(AShr) DEF(And) DEF(Or) DEF(Xor)

    BINARRY_IMM_OPS(BinaryImmOperation)

#undef BINARRY_IMM_OPS
#undef BinaryImmOperation

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinarySignUnsignOperation(opc) static void Visit##opc(GraphVisitor *visitor, Inst *inst);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SIGN_UNSIGN_OPS(DEF) DEF(Div) DEF(Mod) DEF(Min) DEF(Max) DEF(Shr)

    SIGN_UNSIGN_OPS(BinarySignUnsignOperation)

#undef SIGN_UNSIGN_OPS
#undef BinarySignUnsignOperation

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryShiftedRegisterOperationDef(opc, ignored) static void Visit##opc##SR(GraphVisitor *visitor, Inst *inst);

    ENCODE_INST_WITH_SHIFTED_OPERAND(BinaryShiftedRegisterOperationDef)

#undef BinaryShiftedRegisterOperationDef

    static void VisitShrI(GraphVisitor *visitor, Inst *inst);

    static void VisitCast(GraphVisitor *visitor, Inst *inst);

    static void VisitPhi([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst);

    static void VisitConstant(GraphVisitor *visitor, Inst *inst);

    static void VisitNullPtr(GraphVisitor *visitor, Inst *inst);

    // Next visitors use calling convention
    static void VisitIndirectJump(GraphVisitor *visitor, Inst *inst);

    static void VisitIf(GraphVisitor *visitor, Inst *inst);

    static void VisitIfImm(GraphVisitor *visitor, Inst *inst);

    static void VisitCompare(GraphVisitor *visitor, Inst *inst);

    static void VisitCmp(GraphVisitor *visitor, Inst *inst);

    // All next visitors use execution model for implementation
    static void VisitReturnVoid(GraphVisitor *visitor, Inst * /* unused */);

    static void VisitReturn(GraphVisitor *visitor, Inst *inst);

    static void VisitReturnI(GraphVisitor *visitor, Inst *inst);

    static void VisitReturnInlined(GraphVisitor *visitor, Inst * /* unused */);

    static void VisitNewArray(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadConstArray(GraphVisitor *visitor, Inst *inst);

    static void VisitFillConstArray(GraphVisitor *visitor, Inst *inst);

    static void VisitParameter(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreArray(GraphVisitor *visitor, Inst *inst);

    static void VisitSpillFill(GraphVisitor *visitor, Inst *inst);

    static void VisitSaveState(GraphVisitor *visitor, Inst *inst);

    static void VisitSaveStateDeoptimize(GraphVisitor *visitor, Inst *inst);

    static void VisitSaveStateOsr(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadArray(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadCompressedStringChar(GraphVisitor *visitor, Inst *inst);

    static void VisitLenArray(GraphVisitor *visitor, Inst *inst);

    static void VisitNullCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitBoundsCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitZeroCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitRefTypeCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitNegativeCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadString(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadObject(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedLoadObject(GraphVisitor *visitor, Inst *inst);

    static void VisitLoad(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreObject(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedStoreObject(GraphVisitor *visitor, Inst *inst);

    static void VisitStore(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedLoadStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedStoreStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitNewObject(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadClass(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadAndInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedLoadAndInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadType(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedLoadType(GraphVisitor *visitor, Inst *inst);

    static void VisitCheckCast(GraphVisitor *visitor, Inst *inst);

    static void VisitIsInstance(GraphVisitor *visitor, Inst *inst);

    static void VisitMonitor(GraphVisitor *visitor, Inst *inst);

    static void VisitIntrinsic(GraphVisitor *visitor, Inst *inst);

    static void VisitBuiltin(GraphVisitor *visitor, Inst *inst);

    static void VisitBoundsCheckI(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreArrayI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadArrayI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadCompressedStringCharI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadI(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreI(GraphVisitor *visitor, Inst *inst);

    static void VisitMultiArray(GraphVisitor *visitor, Inst *inst);

    static void VisitCallStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedCallStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitCallVirtual(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedCallVirtual(GraphVisitor *visitor, Inst *inst);

    static void VisitCallDynamic(GraphVisitor *visitor, Inst *inst);

    static void VisitSafePoint(GraphVisitor *visitor, Inst *inst);

    static void VisitSelect(GraphVisitor *visitor, Inst *inst);

    static void VisitSelectImm(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadArrayPair(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadArrayPairI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadPairPart(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreArrayPair(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreArrayPairI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadExclusive(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreExclusive(GraphVisitor *visitor, Inst *inst);

    static void VisitNOP(GraphVisitor *visitor, Inst *inst);

    static void VisitThrow(GraphVisitor *visitor, Inst *inst);

    static void VisitDeoptimizeIf(GraphVisitor *visitor, Inst *inst);

    static void VisitDeoptimizeCompare(GraphVisitor *visitor, Inst *inst);

    static void VisitDeoptimizeCompareImm(GraphVisitor *visitor, Inst *inst);

    static void VisitDeoptimize(GraphVisitor *visitor, Inst *inst);

    static void VisitIsMustDeoptimize(GraphVisitor *visitor, Inst *inst);

    static void VisitMAdd(GraphVisitor *visitor, Inst *inst);
    static void VisitMSub(GraphVisitor *visitor, Inst *inst);
    static void VisitMNeg(GraphVisitor *visitor, Inst *inst);
    static void VisitOrNot(GraphVisitor *visitor, Inst *inst);
    static void VisitAndNot(GraphVisitor *visitor, Inst *inst);
    static void VisitXorNot(GraphVisitor *visitor, Inst *inst);
    static void VisitNegSR(GraphVisitor *visitor, Inst *inst);

    static void VisitGetInstanceClass(GraphVisitor *visitor, Inst *inst);
    static void VisitGetManagedClassObject(GraphVisitor *visito, Inst *inst);
    static void VisitClassImmediate(GraphVisitor *visitor, Inst *inst);
    static void VisitRegDef(GraphVisitor *visitor, Inst *inst);
    static void VisitLiveIn(GraphVisitor *visitor, Inst *inst);
    static void VisitLiveOut(GraphVisitor *visitor, Inst *inst);
    static void VisitCallIndirect(GraphVisitor *visitor, Inst *inst);
    static void VisitCall(GraphVisitor *visitor, Inst *inst);

    // Dyn inst.
    static void VisitCompareAnyType(GraphVisitor *visitor, Inst *inst);
    static void VisitCastAnyTypeValue(GraphVisitor *visitor, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *visitor, Inst *inst);
    static void VisitAnyTypeCheck(GraphVisitor *visitor, Inst *inst);

    void VisitDefault([[maybe_unused]] Inst *inst) override
    {
#ifndef NDEBUG
        COMPILER_LOG(DEBUG, CODEGEN) << "Can't encode instruction " << GetOpcodeString(inst->GetOpcode())
                                     << " with type " << DataType::ToString(inst->GetType());
#endif
        success_ = false;
    }

    // Helper functions
    static void FillUnresolvedClass(GraphVisitor *visitor, Inst *inst);
    static void FillObjectClass(GraphVisitor *visitor, Reg tmp_reg, LabelHolder::LabelId throw_label);
    static void FillOtherClass(GraphVisitor *visitor, Inst *inst, Reg tmp_reg, LabelHolder::LabelId throw_label);
    static void FillArrayObjectClass(GraphVisitor *visitor, Reg tmp_reg, LabelHolder::LabelId throw_label);
    static void FillArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmp_reg, LabelHolder::LabelId throw_label);
    static void FillInterfaceClass(GraphVisitor *visitor, Inst *inst);

    static void FillLoadClassUnresolved(GraphVisitor *visitor, Inst *inst);

    static void FillCheckCast(GraphVisitor *visitor, Inst *inst, Reg src, LabelHolder::LabelId end_label,
                              compiler::ClassType klass_type);

    static void FillIsInstanceUnresolved(GraphVisitor *visitor, Inst *inst);

    static void FillIsInstanceCaseObject(GraphVisitor *visitor, Inst *inst, Reg tmp_reg);

    static void FillIsInstanceCaseOther(GraphVisitor *visitor, Inst *inst, Reg tmp_reg, LabelHolder::LabelId end_label);

    static void FillIsInstanceCaseArrayObject(GraphVisitor *visitor, Inst *inst, Reg tmp_reg,
                                              LabelHolder::LabelId end_label);

    static void FillIsInstanceCaseArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmp_reg,
                                             LabelHolder::LabelId end_label);

    static void FillIsInstanceCaseInterface(GraphVisitor *visitor, Inst *inst);

    static void FillIsInstance(GraphVisitor *visitor, Inst *inst, Reg tmp_reg, LabelHolder::LabelId end_label);

#include "optimizer/ir/visitor.inc"

private:
    static void VisitDynamicMethodParameter(GraphVisitor *visitor, Inst *inst);
    static void HandleDynParamPassed(const SpillFillData &sf, EncodeVisitor *enc);
    static void HandleDynParamNotPassed(const SpillFillData &sf, EncodeVisitor *enc);
    static void CastToAny(GraphVisitor *visitor, Inst *inst);

private:
    Codegen *cg_;
    Arch arch_;
    bool success_ {true};
};  // EncodeVisitor

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_CODEGEN_H_
