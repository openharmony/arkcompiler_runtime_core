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

#ifndef COMPILER_OPTIMIZER_CODEGEN_CODEGEN_H
#define COMPILER_OPTIMIZER_CODEGEN_CODEGEN_H

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

inline VRegInfo::Type IrTypeToMetainfoType(DataType::Type type)
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
        return localAllocator_;
    }
    FrameInfo *GetFrameInfo() const
    {
        return frameInfo_;
    }
    void SetFrameInfo(FrameInfo *frameInfo)
    {
        frameInfo_ = frameInfo;
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
        return labelEntry_;
    }

    LabelHolder::LabelId GetLabelExit() const
    {
        return labelExit_;
    }

    RuntimeInterface::MethodId GetMethodId()
    {
        return methodId_;
    }

    void SetStartCodeOffset(size_t offset)
    {
        startCodeOffset_ = offset;
    }

    size_t GetStartCodeOffset() const
    {
        return startCodeOffset_;
    }

    void Convert(ArenaVector<Reg> *regsUsage, const ArenaVector<bool> *mask, TypeInfo typeInfo);

    Reg ConvertRegister(Register ref, DataType::Type type = DataType::Type::INT64);

    Imm ConvertImmWithExtend(uint64_t imm, DataType::Type type);

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
        return codeBuilder_;
    }

    void CreateStackMap(Inst *inst, Inst *user = nullptr);

    void CreateStackMapRec(SaveStateInst *saveState, bool requireVregMap, Inst *targetSite);
    void CreateVRegMap(SaveStateInst *saveState, size_t vregsCount, Inst *targetSite);
    void CreateVreg(const Location &location, Inst *inst, const VirtualRegister &vreg);
    void FillVregIndices(SaveStateInst *saveState);

    void CreateOsrEntry(SaveStateInst *saveState);

    void CreateVRegForRegister(const Location &location, Inst *inst, const VirtualRegister &vreg);

    /// 'live_inputs' shows that inst's source registers should be added the the mask
    template <bool LIVE_INPUTS = false>
    std::pair<RegMask, VRegMask> GetLiveRegisters(Inst *inst)
    {
        RegMask liveRegs;
        VRegMask liveFpRegs;
        if (!g_options.IsCompilerSaveOnlyLiveRegisters() || inst == nullptr) {
            liveRegs.set();
            liveFpRegs.set();
            return {liveRegs, liveFpRegs};
        }
        // Run LiveRegisters pass only if it is actually required
        if (!GetGraph()->IsAnalysisValid<LiveRegisters>()) {
            GetGraph()->RunPass<LiveRegisters>();
        }

        // Add registers from intervals that are live at inst's definition
        auto &lr = GetGraph()->GetAnalysis<LiveRegisters>();
        lr.VisitIntervalsWithLiveRegisters<LIVE_INPUTS>(inst, [&liveRegs, &liveFpRegs, this](const auto &li) {
            auto reg = ConvertRegister(li->GetReg(), li->GetType());
            GetEncoder()->SetRegister(&liveRegs, &liveFpRegs, reg);
        });

        // Add live temp registers
        liveRegs |= GetEncoder()->GetLiveTmpRegMask();
        liveFpRegs |= GetEncoder()->GetLiveTmpFpRegMask();

        return {liveRegs, liveFpRegs};
    }

    // Limits live register set to a number of registers used to pass parameters to the runtime or fastpath call:
    // 1) these ones are saved/restored by caller
    // 2) the remaining ones are saved/restored by the bridge function (aarch only) or by fastpath prologue/epilogue
    void FillOnlyParameters(RegMask *liveRegs, uint32_t numParams, bool isFastpath) const;

    template <typename T, typename... Args>
    T *CreateSlowPath(Inst *inst, Args &&...args)
    {
        static_assert(std::is_base_of_v<SlowPathBase, T>);
        auto label = GetEncoder()->CreateLabel();
        auto slowPath = GetLocalAllocator()->New<T>(label, inst, std::forward<Args>(args)...);
        slowPaths_.push_back(slowPath);
        return slowPath;
    }

    void EmitSlowPaths();

    /**
     * Insert tracing code to the generated code. See `Trace` method in the `runtime/entrypoints.cpp`.
     * NOTE(compiler): we should rework parameters assigning algorithm, that is duplicated here.
     * @param params parameters to be passed to the TRACE entrypoint, first parameter must be TraceId value.
     */
    template <typename... Args>
    void InsertTrace(Args &&...params)
    {
        SCOPED_DISASM_STR(this, "Trace");
        [[maybe_unused]] constexpr size_t MAX_PARAM_NUM = 8;
        static_assert(sizeof...(Args) <= MAX_PARAM_NUM);
        auto regfile = GetRegfile();
        auto saveRegs = regfile->GetCallerSavedRegMask();
        saveRegs.set(GetTarget().GetReturnRegId());
        auto saveVregs = regfile->GetCallerSavedVRegMask();
        saveVregs.set(GetTarget().GetReturnFpRegId());

        SaveCallerRegisters(saveRegs, saveVregs, false);
        FillCallParams(std::forward<Args>(params)...);
        EmitCallRuntimeCode(nullptr, EntrypointId::TRACE);
        LoadCallerRegisters(saveRegs, saveVregs, false);
    }

    void CallIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId id);

    template <bool IS_FASTPATH, typename... Args>
    void CallEntrypoint(Inst *inst, EntrypointId id, Reg dstReg, RegMask preservedRegs, Args &&...params)
    {
        ASSERT(inst != nullptr);
        CHECK_EQ(sizeof...(Args), GetRuntime()->GetEntrypointArgsNum(id));
        if (GetArch() == Arch::AARCH32) {
            // There is a problem with 64-bit parameters:
            // params number passed from entrypoints_gen.S.erb will be inconsistent with Aarch32 ABI.
            // Thus, runtime bridges will have wrong params number (\paramsnum macro argument).
            ASSERT(EnsureParamsFitIn32Bit({params...}));
            ASSERT(!dstReg.IsValid() || dstReg.GetSize() <= WORD_SIZE);
        }

        SCOPED_DISASM_STR(this, std::string("CallEntrypoint: ") + GetRuntime()->GetEntrypointName(id));
        RegMask liveRegs {preservedRegs | GetLiveRegisters(inst).first};
        RegMask paramsMask;
        if (inst->HasImplicitRuntimeCall() && !GetRuntime()->IsEntrypointNoreturn(id)) {
            SaveRegistersForImplicitRuntime(inst, &paramsMask, &liveRegs);
        }

        ASSERT(IS_FASTPATH == GetRuntime()->IsEntrypointFastPath(id));
        bool retRegAlive {liveRegs.Test(GetTarget().GetReturnRegId())};
        // parameter regs: their initial values must be stored by the caller
        // Other caller regs stored in bridges
        FillOnlyParameters(&liveRegs, sizeof...(Args), IS_FASTPATH);

        if (IS_FASTPATH && retRegAlive && dstReg.IsValid()) {
            Reg retReg = GetTarget().GetReturnReg(dstReg.GetType());
            if (dstReg.GetId() != retReg.GetId()) {
                GetEncoder()->SetRegister(&liveRegs, nullptr, retReg, true);
            }
        }

        GetEncoder()->SetRegister(&liveRegs, nullptr, dstReg, false);
        SaveCallerRegisters(liveRegs, VRegMask(), true);

        if (sizeof...(Args) != 0) {
            FillCallParams(std::forward<Args>(params)...);
        }

        // Call Code
        if (!EmitCallRuntimeCode(inst, id)) {
            return;
        }
        if (dstReg.IsValid()) {
            ASSERT(dstReg.IsScalar());
            Reg retReg = GetTarget().GetReturnReg(dstReg.GetType());
            if (!IS_FASTPATH && retRegAlive && dstReg.GetId() != retReg.GetId() &&
                (!GetTarget().FirstParamIsReturnReg(retReg.GetType()) || sizeof...(Args) == 0U)) {
                GetEncoder()->SetRegister(&liveRegs, nullptr, retReg, true);
            }

            // We must:
            //  sign extended INT8 and INT16 to INT32
            //  zero extended UINT8 and UINT16 to UINT32
            if (dstReg.GetSize() < WORD_SIZE) {
                bool isSigned = DataType::IsTypeSigned(inst->GetType());
                GetEncoder()->EncodeCast(dstReg.As(INT32_TYPE), isSigned, retReg, isSigned);
            } else {
                GetEncoder()->EncodeMov(dstReg, retReg);
            }
        }
        CallEntrypointFinalize(liveRegs, paramsMask, inst);
    }

    void CallEntrypointFinalize(RegMask &liveRegs, RegMask &paramsMask, Inst *inst)
    {
        LoadCallerRegisters(liveRegs, VRegMask(), true);

        if (!inst->HasImplicitRuntimeCall()) {
            return;
        }
        for (auto i = 0U; i < paramsMask.size(); i++) {
            if (paramsMask.test(i)) {
                inst->GetSaveState()->GetRootsRegsMask().reset(i);
            }
        }
    }

    // The function is used for calling runtime functions through special bridges.
    // !NOTE Don't use the function for calling runtime without bridges(it save only parameters on stack)
    template <typename... Args>
    void CallRuntime(Inst *inst, EntrypointId id, Reg dstReg, RegMask preservedRegs, Args &&...params)
    {
        CallEntrypoint<false>(inst, id, dstReg, preservedRegs, std::forward<Args>(params)...);
    }

    template <typename... Args>
    void CallFastPath(Inst *inst, EntrypointId id, Reg dstReg, RegMask preservedRegs, Args &&...params)
    {
        CallEntrypoint<true>(inst, id, dstReg, preservedRegs, std::forward<Args>(params)...);
    }

    template <typename... Args>
    void CallRuntimeWithMethod(Inst *inst, void *method, EntrypointId eid, Reg dstReg, Args &&...params)
    {
        if (GetGraph()->IsAotMode()) {
            ScopedTmpReg methodReg(GetEncoder());
            LoadMethod(methodReg);
            CallRuntime(inst, eid, dstReg, RegMask::GetZeroMask(), methodReg, std::forward<Args>(params)...);
        } else {
            if (Is64BitsArch(GetArch())) {
                CallRuntime(inst, eid, dstReg, RegMask::GetZeroMask(), TypedImm(reinterpret_cast<uint64_t>(method)),
                            std::forward<Args>(params)...);
            } else {
                // uintptr_t causes problems on host cross-jit compilation
                CallRuntime(inst, eid, dstReg, RegMask::GetZeroMask(), TypedImm(down_cast<uint32_t>(method)),
                            std::forward<Args>(params)...);
            }
        }
    }

    void SaveRegistersForImplicitRuntime(Inst *inst, RegMask *paramsMask, RegMask *mask);

    void VisitNewArray(Inst *inst);

    void LoadClassFromObject(Reg classReg, Reg objReg);
    void VisitCallIndirect(CallIndirectInst *inst);
    void VisitCall(CallInst *inst);
    void CreateCallIntrinsic(IntrinsicInst *inst);
    void CreateMultiArrayCall(CallInst *callInst);
    void CreateNewObjCall(NewObjectInst *newObj);
    void CreateNewObjCallOld(NewObjectInst *newObj);
    void CreateMonitorCall(MonitorInst *inst);
    void CreateMonitorCallOld(MonitorInst *inst);
    void CreateCheckCastInterfaceCall(Inst *inst);
    void CreateNonDefaultInitClass(ClassInst *initInst);
    void CheckObject(Reg reg, LabelHolder::LabelId label);
    template <bool IS_CLASS = false>
    void CreatePreWRB(Inst *inst, MemRef mem, RegMask preserved = {}, bool storePair = false);
    void CreatePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2 = INVALID_REGISTER);
    void CreatePostWRBForDynamic(Inst *inst, MemRef mem, Reg reg1, Reg reg2);
    void EncodePostWRB(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool checkObject = true);
    void CreatePostInterRegionBarrier(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool checkObject);
    void CreatePostInterGenerationalBarrier(Reg base);
    // Creates call to IRtoC PostWrb Entrypoint. Offline means AOT or IRtoC compilation -> type of GC is not known. So
    // Managed Thread keeps pointer to actual IRtoC GC barriers implementation at run-time.
    void CreateOfflineIrtocPostWrb(Inst *inst, MemRef mem, Reg reg1, Reg reg2);
    // Creates call to IRtoC PostWrb Entrypoint. Online means JIT compilation -> we know GC type.
    void CreateOnlineIrtocPostWrbRegionTwoRegs(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool checkObject);
    void CreateOnlineIrtocPostWrbRegionOneReg(Inst *inst, MemRef mem, Reg reg1, bool checkObject);
    void CreateOnlineIrtocPostWrb(Inst *inst, MemRef mem, Reg reg1, Reg reg2, bool checkObject);
    template <typename... Args>
    void CallBarrier(RegMask liveRegs, VRegMask liveVregs, std::variant<EntrypointId, Reg> entrypoint, Args &&...params)
    {
        SaveCallerRegisters(liveRegs, liveVregs, true);
        FillCallParams(std::forward<Args>(params)...);
        EmitCallRuntimeCode(nullptr, entrypoint);
        LoadCallerRegisters(liveRegs, liveVregs, true);
    }

    void CreateLoadClassFromPLT(Inst *inst, Reg tmpReg, Reg dst, size_t classId);
    void CreateJumpToClassResolverPltShared(Inst *inst, Reg tmpReg, RuntimeInterface::EntrypointId id);
    void CreateLoadTLABInformation(Reg regTlabStart, Reg regTlabSize);
    void CreateCheckForTLABWithConstSize(Inst *inst, Reg regTlabStart, Reg regTlabSize, size_t size,
                                         LabelHolder::LabelId label);
    void CreateDebugRuntimeCallsForNewObject(Inst *inst, Reg regTlabStart, size_t allocSize, RegMask preserved);
    void CreateDebugRuntimeCallsForObjectClone(Inst *inst, Reg dst);
    void CallFastCreateStringFromCharArrayTlab(Inst *inst, Reg dst, Reg offset, Reg count, Reg array,
                                               std::variant<Reg, TypedImm> klass);
    void CreateReturn(const Inst *inst);
    template <typename T>
    void CreateUnaryCheck(Inst *inst, RuntimeInterface::EntrypointId id, DeoptimizeType type, Condition cc)
    {
        [[maybe_unused]] auto ss = inst->GetSaveState();
        ASSERT(ss != nullptr &&
               (ss->GetOpcode() == Opcode::SaveState || ss->GetOpcode() == Opcode::SaveStateDeoptimize));

        LabelHolder::LabelId slowPath;
        if (inst->CanDeoptimize()) {
            slowPath = CreateSlowPath<SlowPathDeoptimize>(inst, type)->GetLabel();
        } else {
            slowPath = CreateSlowPath<T>(inst, id)->GetLabel();
        }
        auto srcType = inst->GetInputType(0);
        auto src = ConvertRegister(inst->GetSrcReg(0), srcType);
        GetEncoder()->EncodeJump(slowPath, src, cc);
    }

    // The function alignment up the value from alignment_reg using tmp_reg.
    void CreateAlignmentValue(Reg alignmentReg, Reg tmpReg, size_t alignment);
    void TryInsertImplicitNullCheck(Inst *inst, size_t prevOffset);

    const CFrameLayout &GetFrameLayout() const
    {
        return frame_layout_;
    }

    bool RegisterKeepCallArgument(CallInst *callInst, Reg reg);

    void LoadMethod(Reg dst);
    void LoadFreeSlot(Reg dst);
    void StoreFreeSlot(Reg src);

    ssize_t GetStackOffset(Location location)
    {
        if (location.GetKind() == LocationType::STACK_ARGUMENT) {
            return location.GetValue() * GetFrameLayout().GetSlotSize();
        }

        if (location.GetKind() == LocationType::STACK_PARAMETER) {
            return GetFrameLayout().GetFrameSize<CFrameLayout::OffsetUnit::BYTES>() +
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
    void SaveCallerRegisters(RegMask liveRegs, VRegMask liveVregs, bool adjustRegs);
    void LoadCallerRegisters(RegMask liveRegs, VRegMask liveVregs, bool adjustRegs);

    void IssueDisasm();

    // Initialization internal variables
    void Initialize();
    bool Finalize();

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
        liveOuts_[bb].Set(reg);
    }

    RegMask GetLiveOut(const BasicBlock *bb) const
    {
        auto it = liveOuts_.find(bb);
        return it != liveOuts_.end() ? it->second : RegMask();
    }

    Reg ThreadReg() const
    {
        return Reg(GetThreadReg(GetArch()), GetTarget().GetPtrRegType());
    }

    static bool InstEncodedWithLibCall(const Inst *inst, Arch arch);

    void EncodeDynamicCast(Inst *inst, Reg dst, bool dstSigned, Reg src);

    Reg ConvertInstTmpReg(const Inst *inst, DataType::Type type) const;
    Reg ConvertInstTmpReg(const Inst *inst) const;

    bool OffsetFitReferenceTypeSize(uint64_t offset) const
    {
        // -1 because some arch uses signed offset
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        uint64_t maxOffset = 1ULL << (DataType::GetTypeSize(DataType::REFERENCE, GetArch()) - 1);
        return offset < maxOffset;
    }

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
        return usedRegs_;
    }
    RegMask GetUsedVRegs() const
    {
        return usedVregs_;
    }

    template <typename... Args>
    void FillCallParams(Args &&...params);

    template <size_t IMM_ARRAY_SIZE, typename Arg, typename... Args>
    ALWAYS_INLINE inline void FillCallParamsHandleOperands(
        ParameterInfo *paramInfo, SpillFillInst *regMoves, ArenaVector<Reg> *spMoves,
        [[maybe_unused]] typename std::array<std::pair<Reg, Imm>, IMM_ARRAY_SIZE>::iterator immsIter, Arg &&arg,
        Args &&...params);

    void EmitJump(const BasicBlock *bb);

    bool EmitCallRuntimeCode(Inst *inst, std::variant<EntrypointId, Reg> entrypoint);

    void IntfInlineCachePass(ResolveVirtualInst *resolver, Reg methodReg, Reg tmpReg, Reg objReg);

    template <typename T>
    RuntimeInterface::MethodPtr GetCallerOfUnresolvedMethod(T *resolver);

    void EmitResolveVirtual(ResolveVirtualInst *resolver);
    void EmitResolveUnknownVirtual(ResolveVirtualInst *resolver, Reg methodReg);
    void EmitResolveVirtualAot(ResolveVirtualInst *resolver, Reg methodReg);
    void EmitCallVirtual(CallInst *call);
    void EmitCallResolvedVirtual(CallInst *call);
    void EmitCallStatic(CallInst *call);
    void EmitResolveStatic(ResolveStaticInst *resolver);
    void EmitCallResolvedStatic(CallInst *call);
    void EmitCallDynamic(CallInst *call);
    void FinalizeCall(CallInst *call);

    uint32_t GetVtableShift()
    {
        // The size of the VTable element is equal to the size of pointers for the architecture
        // (not the size of pointer to objects)
        constexpr uint32_t SHIFT_64_BITS = 3;
        constexpr uint32_t SHIFT_32_BITS = 2;
        return Is64BitsArch(GetGraph()->GetArch()) ? SHIFT_64_BITS : SHIFT_32_BITS;
    }

    void CalculateCardIndex(Reg baseReg, ScopedTmpReg *tmp, ScopedTmpReg *tmp1);
    void CreateBuiltinIntrinsic(IntrinsicInst *inst);
    static constexpr int32_t NUM_OF_SRC_BUILTIN = 6;
    static constexpr uint8_t FIRST_OPERAND = 0;
    static constexpr uint8_t SECOND_OPERAND = 1;
    static constexpr uint8_t THIRD_OPERAND = 2;
    static constexpr uint8_t FOURTH_OPERAND = 3;
    static constexpr uint8_t FIFTH_OPERAND = 4;
    using SRCREGS = std::array<Reg, NUM_OF_SRC_BUILTIN>;
    // implementation is generated with compiler/optimizer/templates/intrinsics/intrinsics_codegen.inl.erb
    void FillBuiltin(IntrinsicInst *inst, SRCREGS src, Reg dst);

    template <typename Arg, typename... Args>
    ALWAYS_INLINE inline void AddParamRegsInLiveMasksHandleArgs(ParameterInfo *paramInfo, RegMask *liveRegs,
                                                                VRegMask *liveVregs, Arg param, Args &&...params)
    {
        auto currDst = paramInfo->GetNativeParam(param.GetType());
        if (std::holds_alternative<Reg>(currDst)) {
            auto reg = std::get<Reg>(currDst);
            if (reg.IsScalar()) {
                liveRegs->set(reg.GetId());
            } else {
                liveVregs->set(reg.GetId());
            }
        } else {
            GetEncoder()->SetFalseResult();
            UNREACHABLE();
        }
        if constexpr (sizeof...(Args) != 0) {
            AddParamRegsInLiveMasksHandleArgs(paramInfo, liveRegs, liveVregs, std::forward<Args>(params)...);
        }
    }

    template <typename... Args>
    void AddParamRegsInLiveMasks(RegMask *liveRegs, VRegMask *liveVregs, Args &&...params)
    {
        auto callconv = GetCallingConvention();
        auto paramInfo = callconv->GetParameterInfo(0);
        AddParamRegsInLiveMasksHandleArgs(paramInfo, liveRegs, liveVregs, std::forward<Args>(params)...);
    }

    template <typename... Args>
    void CreateStubCall(Inst *inst, RuntimeInterface::IntrinsicId intrinsicId, Reg dst, Args &&...params)
    {
        VRegMask liveVregs;
        RegMask liveRegs;
        AddParamRegsInLiveMasks(&liveRegs, &liveVregs, params...);
        auto enc = GetEncoder();
        {
            SCOPED_DISASM_STR(this, "Save caller saved regs");
            SaveCallerRegisters(liveRegs, liveVregs, true);
        }

        FillCallParams(std::forward<Args>(params)...);
        CallIntrinsic(inst, intrinsicId);

        if (inst->GetSaveState() != nullptr) {
            CreateStackMap(inst);
        }

        if (dst.IsValid()) {
            Reg retVal = GetTarget().GetReturnReg(dst.GetType());
            if (dst.GetId() != retVal.GetId()) {
                enc->SetRegister(&liveRegs, &liveVregs, retVal, true);
            }
            ASSERT(dst.IsScalar());
            enc->EncodeMov(dst, retVal);
        }

        {
            SCOPED_DISASM_STR(this, "Restore caller saved regs");
            enc->SetRegister(&liveRegs, &liveVregs, dst, false);
            LoadCallerRegisters(liveRegs, liveVregs, true);
        }
    }

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
    void EncodeImms(const T &imms, bool skipFirstLocation)
    {
        auto paramInfo = GetCallingConvention()->GetParameterInfo(0);
        auto immType = DataType::INT32;
        if (skipFirstLocation) {
            paramInfo->GetNextLocation(immType);
        }
        for (auto imm : imms) {
            auto location = paramInfo->GetNextLocation(immType);
            ASSERT(location.IsFixedRegister());
            auto dstReg = ConvertRegister(location.GetValue(), immType);
            GetEncoder()->EncodeMov(dstReg, Imm(imm));
        }
    }

    [[maybe_unused]] bool EnsureParamsFitIn32Bit(std::initializer_list<std::variant<Reg, TypedImm>> params);

    template <typename... Args>
    void FillPostWrbCallParams(MemRef mem, Args &&...params);

private:
    ArenaAllocator *allocator_;
    ArenaAllocator *localAllocator_;
    // Register description
    RegistersDescription *regfile_;
    // Encoder implementation
    Encoder *enc_;
    // Target architecture calling convention model
    CallingConvention *callconv_;
    // Current execution model implementation
    // Visitor for instructions
    GraphVisitor *visitor_ {};

    CodeInfoBuilder *codeBuilder_ {nullptr};

    ArenaVector<SlowPathBase *> slowPaths_;
    ArenaUnorderedMap<RuntimeInterface::EntrypointId, SlowPathShared *> slowPathsMap_;

    const CFrameLayout frame_layout_;  // NOLINT(readability-identifier-naming)

    ArenaVector<OsrEntryStub *> osrEntries_;

    RuntimeInterface::MethodId methodId_ {INVALID_ID};

    size_t startCodeOffset_ {0};

    ArenaVector<std::pair<int16_t, int16_t>> vregIndices_;

    RuntimeInterface *runtime_ {nullptr};

    LabelHolder::LabelId labelEntry_ {};
    LabelHolder::LabelId labelExit_ {};

    FrameInfo *frameInfo_ {nullptr};

    const Target target_;

    /* Registers that have been allocated by regalloc */
    RegMask usedRegs_ {0};
    RegMask usedVregs_ {0};

    /* Map of BasicBlock to live-out regsiters mask. It is needed in epilogue encoding to avoid overwriting of the
     * live-out registers */
    ArenaUnorderedMap<const BasicBlock *, RegMask> liveOuts_;

    Disassembly disasm_;

    SpillFillsResolver spillFillsResolver_;

    friend class EncodeVisitor;
    friend class BaselineCodegen;
    friend class SlowPathJsCastDoubleToInt32;
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
    static void SortSpillFillData(ArenaVector<SpillFillData> *spillFills);
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
    static void VisitNegOverflowAndZeroCheck(GraphVisitor *v, Inst *inst);

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

    static void VisitBitcast(GraphVisitor *visitor, Inst *inst);

    static void VisitPhi([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst);

    static void VisitConstant(GraphVisitor *visitor, Inst *inst);

    static void VisitNullPtr(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadUndefined(GraphVisitor *visitor, Inst *inst);

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

    static void VisitNotPositiveCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadString(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveObjectField(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadResolvedObjectField(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadObject(GraphVisitor *visitor, Inst *inst);

    static void VisitLoad(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreObject(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreResolvedObjectField(GraphVisitor *visitor, Inst *inst);

    static void VisitStore(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveObjectFieldStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadResolvedObjectFieldStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedStoreStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreResolvedObjectFieldStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitNewObject(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadRuntimeClass(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadClass(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadAndInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitGetGlobalVarAddress(GraphVisitor *visitor, Inst *inst);

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
    static void VisitInitEmptyString(GraphVisitor *visitor, Inst *inst);
    static void VisitInitString(GraphVisitor *visitor, Inst *inst);

    static void VisitCallStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveStatic(GraphVisitor *visitor, Inst *inst);
    static void VisitCallResolvedStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitCallVirtual(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveVirtual(GraphVisitor *visitor, Inst *inst);
    static void VisitCallResolvedVirtual(GraphVisitor *visitor, Inst *inst);

    static void VisitCallLaunchStatic(GraphVisitor *visitor, Inst *inst);
    static void VisitCallResolvedLaunchStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitCallLaunchVirtual(GraphVisitor *visitor, Inst *inst);
    static void VisitCallResolvedLaunchVirtual(GraphVisitor *visitor, Inst *inst);

    static void VisitCallDynamic(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadConstantPool(GraphVisitor *visitor, Inst *inst);
    static void VisitLoadLexicalEnv(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadFromConstantPool(GraphVisitor *visitor, Inst *inst);

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
    static void VisitLoadImmediate(GraphVisitor *visitor, Inst *inst);
    static void VisitFunctionImmediate(GraphVisitor *visitor, Inst *inst);
    static void VisitLoadObjFromConst(GraphVisitor *visitor, Inst *inst);
    static void VisitRegDef(GraphVisitor *visitor, Inst *inst);
    static void VisitLiveIn(GraphVisitor *visitor, Inst *inst);
    static void VisitLiveOut(GraphVisitor *visitor, Inst *inst);
    static void VisitCallIndirect(GraphVisitor *visitor, Inst *inst);
    static void VisitCall(GraphVisitor *visitor, Inst *inst);

    // Dyn inst.
    static void VisitCompareAnyType(GraphVisitor *visitor, Inst *inst);
    static void VisitGetAnyTypeName(GraphVisitor *visitor, Inst *inst);
    static void VisitCastAnyTypeValue(GraphVisitor *visitor, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *visitor, Inst *inst);
    static void VisitAnyTypeCheck(GraphVisitor *visitor, Inst *inst);
    static void VisitHclassCheck(GraphVisitor *visitor, Inst *inst);
    static void VisitObjByIndexCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadObjectDynamic(GraphVisitor *visitor, Inst *inst);
    static void VisitStoreObjectDynamic(GraphVisitor *visitor, Inst *inst);

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
    static void FillObjectClass(GraphVisitor *visitor, Reg tmpReg, LabelHolder::LabelId throwLabel);
    static void FillOtherClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId throwLabel);
    static void FillArrayObjectClass(GraphVisitor *visitor, Reg tmpReg, LabelHolder::LabelId throwLabel);
    static void FillArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId throwLabel);
    static void FillInterfaceClass(GraphVisitor *visitor, Inst *inst);

    static void FillLoadClassUnresolved(GraphVisitor *visitor, Inst *inst);

    static void FillCheckCast(GraphVisitor *visitor, Inst *inst, Reg src, LabelHolder::LabelId endLabel,
                              compiler::ClassType klassType);

    static void FillIsInstanceUnresolved(GraphVisitor *visitor, Inst *inst);

    static void FillIsInstanceCaseObject(GraphVisitor *visitor, Inst *inst, Reg tmpReg);

    static void FillIsInstanceCaseOther(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId endLabel);

    static void FillIsInstanceCaseArrayObject(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                              LabelHolder::LabelId endLabel);

    static void FillIsInstanceCaseArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                             LabelHolder::LabelId endLabel);

    static void FillIsInstanceCaseInterface(GraphVisitor *visitor, Inst *inst);

    static void FillIsInstance(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId endLabel);

#include "optimizer/ir/visitor.inc"

private:
    Codegen *cg_;
    Arch arch_;
    bool success_ {true};
};  // EncodeVisitor

template <size_t IMM_ARRAY_SIZE, typename Arg, typename... Args>
ALWAYS_INLINE inline void Codegen::FillCallParamsHandleOperands(
    ParameterInfo *paramInfo, SpillFillInst *regMoves, ArenaVector<Reg> *spMoves,
    [[maybe_unused]] typename std::array<std::pair<Reg, Imm>, IMM_ARRAY_SIZE>::iterator immsIter, Arg &&arg,
    Args &&...params)
{
    Location dst;
    auto type = arg.GetType().ToDataType();
    dst = paramInfo->GetNextLocation(type);
    if (dst.IsStackArgument()) {
        GetEncoder()->SetFalseResult();
        UNREACHABLE();  // Move to BoundaryFrame
    }

    static_assert(std::is_same_v<std::decay_t<Arg>, TypedImm> || std::is_convertible_v<Arg, Reg>);
    if constexpr (std::is_same_v<std::decay_t<Arg>, TypedImm>) {
        auto reg = ConvertRegister(dst.GetValue(), type);
        *immsIter = {reg, arg.GetImm()};
        immsIter++;
    } else {
        Reg reg(std::forward<Arg>(arg));
        if (reg == SpReg()) {
            // SP should be handled separately, since on the ARM64 target it has ID out of range
            spMoves->emplace_back(ConvertRegister(dst.GetValue(), type));
        } else {
            regMoves->AddSpillFill(Location::MakeRegister(reg.GetId(), type), dst, type);
        }
    }
    if constexpr (sizeof...(Args) != 0) {
        FillCallParamsHandleOperands<IMM_ARRAY_SIZE>(paramInfo, regMoves, spMoves, immsIter,
                                                     std::forward<Args>(params)...);
    }
}

template <typename T, typename... Args>
constexpr std::pair<size_t, size_t> CountParameters()
{
    static_assert(std::is_same_v<std::decay_t<T>, TypedImm> != std::is_convertible_v<T, Reg>);
    if constexpr (sizeof...(Args) != 0) {
        constexpr auto IMM_REG_COUNT = CountParameters<Args...>();

        if constexpr (std::is_same_v<std::decay_t<T>, TypedImm>) {
            return {IMM_REG_COUNT.first + 1, IMM_REG_COUNT.second};
        } else if constexpr (std::is_convertible_v<T, Reg>) {
            return {IMM_REG_COUNT.first, IMM_REG_COUNT.second + 1};
        }
    }
    return {std::is_same_v<std::decay_t<T>, TypedImm>, std::is_convertible_v<T, Reg>};
}

template <typename... Args>
void Codegen::FillCallParams(Args &&...params)
{
    SCOPED_DISASM_STR(this, "FillCallParams");
    if constexpr (sizeof...(Args) != 0) {
        constexpr size_t IMMEDIATES_COUNT = CountParameters<Args...>().first;
        constexpr size_t REGS_COUNT = CountParameters<Args...>().second;
        // Native call - do not add reserve parameters
        auto paramInfo = GetCallingConvention()->GetParameterInfo(0);
        std::array<std::pair<Reg, Imm>, IMMEDIATES_COUNT> immediates {};
        ArenaVector<Reg> spMoves(GetLocalAllocator()->Adapter());
        auto regMoves = GetGraph()->CreateInstSpillFill();
        spMoves.reserve(REGS_COUNT);
        regMoves->GetSpillFills().reserve(REGS_COUNT);

        FillCallParamsHandleOperands<IMMEDIATES_COUNT>(paramInfo, regMoves, &spMoves, immediates.begin(),
                                                       std::forward<Args>(params)...);

        // Resolve registers move order and encode
        spillFillsResolver_.ResolveIfRequired(regMoves);
        SpillFillEncoder(this, regMoves).EncodeSpillFill();

        // Encode immediates moves
        for (auto &immValues : immediates) {
            GetEncoder()->EncodeMov(immValues.first, immValues.second);
        }

        // Encode moves from SP reg
        for (auto dst : spMoves) {
            GetEncoder()->EncodeMov(dst, SpReg());
        }
    }
}

template <typename... Args>
void Codegen::FillPostWrbCallParams(MemRef mem, Args &&...params)
{
    auto base {mem.GetBase().As(TypeInfo::FromDataType(DataType::REFERENCE, GetArch()))};
    if (mem.HasIndex()) {
        ASSERT(mem.GetScale() == 0 && !mem.HasDisp());
        FillCallParams(base, mem.GetIndex(), std::forward<Args>(params)...);
    } else {
        FillCallParams(base, TypedImm(mem.GetDisp()), std::forward<Args>(params)...);
    }
}

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_CODEGEN_H
