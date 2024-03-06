/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_INST_BUILDER_H
#define PANDA_INST_BUILDER_H

#include "compiler_options.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "code_info/vreg_info.h"
#include "code_data_accessor.h"
#include "file_items.h"
#include "compiler_logger.h"

#include "bytecode_instruction.h"

namespace ark::compiler {
constexpr int64_t INVALID_OFFSET = std::numeric_limits<int64_t>::max();

class InstBuilder {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENV_IDX(ENV_TYPE) \
    static constexpr uint8_t ENV_TYPE##_IDX = VRegInfo::VRegType::ENV_TYPE - VRegInfo::VRegType::ENV_BEGIN;
    VREGS_ENV_TYPE_DEFS(ENV_IDX)
#undef ENV_IDX
public:
    InstBuilder(Graph *graph, RuntimeInterface::MethodPtr method, CallInst *callerInst, uint32_t inliningDepth)
        : graph_(graph),
          runtime_(graph->GetRuntime()),
          defs_(graph->GetLocalAllocator()->Adapter()),
          method_(method),
          vregsAndArgsCount_(graph->GetRuntime()->GetMethodRegistersCount(method) +
                             graph->GetRuntime()->GetMethodTotalArgumentsCount(method)),
          instructionsBuf_(GetGraph()->GetRuntime()->GetMethodCode(GetGraph()->GetMethod())),
          callerInst_(callerInst),
          inliningDepth_(inliningDepth),
          classId_ {runtime_->GetClassIdForMethod(method_)}
    {
        noTypeMarker_ = GetGraph()->NewMarker();
        visitedBlockMarker_ = GetGraph()->NewMarker();

        defs_.resize(graph_->GetVectorBlocks().size(), InstVector(graph->GetLocalAllocator()->Adapter()));
        for (auto &v : defs_) {
            v.resize(GetVRegsCount());
        }

        for (auto bb : graph->GetBlocksRPO()) {
            if (bb->IsCatchBegin()) {
                for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
                    auto catchPhi = GetGraph()->CreateInstCatchPhi(DataType::NO_TYPE, bb->GetGuestPc());
                    catchPhi->SetMarker(GetNoTypeMarker());
                    bb->AppendInst(catchPhi);
                    COMPILER_LOG(DEBUG, IR_BUILDER)
                        << "Creat catchphi " << catchPhi->GetId() << " for bb(" << bb->GetId() << ")";
                    if (vreg == vregsAndArgsCount_) {
                        catchPhi->SetIsAcc();
                    } else if (vreg > vregsAndArgsCount_) {
                        catchPhi->SetType(DataType::ANY);
                    }
                }
            }
        }
    }

    NO_COPY_SEMANTIC(InstBuilder);
    NO_MOVE_SEMANTIC(InstBuilder);
    ~InstBuilder()
    {
        GetGraph()->EraseMarker(noTypeMarker_);
        GetGraph()->EraseMarker(visitedBlockMarker_);
    }

    /**
     * Content of this function is auto generated from inst_builder.erb and is located in inst_builder_gen.cpp file
     * @param instruction Pointer to bytecode instruction
     */
    void BuildInstruction(const BytecodeInstruction *instruction);

    void InitEnv(BasicBlock *bb)
    {
        auto thisFunc = GetGraph()->FindParameter(0);
        auto cp = GetGraph()->CreateInstLoadConstantPool(DataType::ANY, INVALID_PC, thisFunc);
        bb->AppendInst(cp);

        auto lexEnv = GetGraph()->CreateInstLoadLexicalEnv(DataType::ANY, INVALID_PC, thisFunc);
        bb->AppendInst(lexEnv);

        defs_[bb->GetId()][vregsAndArgsCount_ + 1 + THIS_FUNC_IDX] = thisFunc;
        defs_[bb->GetId()][vregsAndArgsCount_ + 1 + CONST_POOL_IDX] = cp;
        defs_[bb->GetId()][vregsAndArgsCount_ + 1 + LEX_ENV_IDX] = lexEnv;
        COMPILER_LOG(DEBUG, IR_BUILDER) << "Init environment this_func = " << thisFunc->GetId()
                                        << ", const_pool = " << cp->GetId() << ", lex_env = " << lexEnv->GetId();
    }

    bool IsFailed() const
    {
        return failed_;
    }

    /// Return jump offset for instruction `inst`, 0 if it is not jump instruction.
    static int64_t GetInstructionJumpOffset(const BytecodeInstruction *inst);

    void SetCurrentBlock(BasicBlock *bb)
    {
        if (GetGraph()->IsDynamicMethod() && !GetGraph()->IsBytecodeOptimizer() &&
            currentBb_ != GetGraph()->GetStartBlock() && currentDefs_ != nullptr) {
            ASSERT((*currentDefs_)[vregsAndArgsCount_ + 1 + THIS_FUNC_IDX] != nullptr);
            ASSERT((*currentDefs_)[vregsAndArgsCount_ + 1 + CONST_POOL_IDX] != nullptr);
            ASSERT((*currentDefs_)[vregsAndArgsCount_ + 1 + LEX_ENV_IDX] != nullptr);
        }
        currentBb_ = bb;
        currentDefs_ = &defs_[bb->GetId()];
    }

    BasicBlock *GetCurrentBlock() const
    {
        return currentBb_;
    }

    void Prepare(bool isInlinedGraph);

    void FixInstructions();
    void ResolveConstants();
    void SplitConstant(ConstantInst *constInst);

    static void RemoveNotDominateInputs(SaveStateInst *saveState);

    size_t GetPc(const uint8_t *instPtr) const;

    auto CreateSafePoint(BasicBlock *bb)
    {
        return CreateSaveState(Opcode::SafePoint, bb->GetGuestPc());
    }

    auto CreateSaveStateOsr(BasicBlock *bb)
    {
        return CreateSaveState(Opcode::SaveStateOsr, bb->GetGuestPc());
    }

    auto CreateSaveStateDeoptimize(uint32_t pc)
    {
        return CreateSaveState(Opcode::SaveStateDeoptimize, pc);
    }

    void UpdateDefs();
    bool UpdateDefsForPreds(size_t vreg, std::optional<Inst *> &value);

    const auto &GetCurrentDefs()
    {
        ASSERT(currentDefs_ != nullptr);
        return *currentDefs_;
    }

    void AddCatchPhiInputs(const ArenaUnorderedSet<BasicBlock *> &catchHandlers, const InstVector &defs,
                           Inst *throwableInst);

    SaveStateInst *CreateSaveState(Opcode opc, size_t pc);

    static void SetParamSpillFill(Graph *graph, ParameterInst *paramInst, size_t numArgs, size_t i,
                                  DataType::Type type);

private:
    void SyncWithGraph();

    void UpdateDefsForCatch();
    void UpdateDefsForLoopHead();

    size_t GetVRegsCount() const
    {
        return vregsAndArgsCount_ + 1 + GetGraph()->GetEnvCount();
    }

    template <typename T>
    void AddInstruction(T inst)
    {
        ASSERT(currentBb_);
        currentBb_->AppendInst(inst);

#ifdef PANDA_COMPILER_DEBUG_INFO
        if (inst->GetPc() != INVALID_PC) {
            inst->SetCurrentMethod(method_);
        }
#endif
        COMPILER_LOG(DEBUG, IR_BUILDER) << *inst;
    }

    template <typename T, typename... Ts>
    void AddInstruction(T inst, Ts... insts)
    {
        AddInstruction(inst);
        AddInstruction(insts...);
    }

    void UpdateDefinition(size_t vreg, Inst *inst)
    {
        ASSERT(vreg < currentDefs_->size());
        COMPILER_LOG(DEBUG, IR_BUILDER) << "update def for r" << vreg << " from "
                                        << ((*currentDefs_)[vreg] != nullptr
                                                ? std::to_string((*currentDefs_)[vreg]->GetId())
                                                : "null")
                                        << " to " << inst->GetId();
        (*currentDefs_)[vreg] = inst;
    }

    void UpdateDefinitionAcc(Inst *inst)
    {
        if (inst == nullptr) {
            COMPILER_LOG(DEBUG, IR_BUILDER) << "reset accumulator definition";
        } else {
            COMPILER_LOG(DEBUG, IR_BUILDER) << "update accumulator from "
                                            << ((*currentDefs_)[vregsAndArgsCount_] != nullptr
                                                    ? std::to_string((*currentDefs_)[vregsAndArgsCount_]->GetId())
                                                    : "null")
                                            << " to " << inst->GetId();
        }
        (*currentDefs_)[vregsAndArgsCount_] = inst;
    }

    void UpdateDefinitionLexEnv(Inst *inst)
    {
        ASSERT(inst != nullptr);
        ASSERT((*currentDefs_)[vregsAndArgsCount_ + 1 + LEX_ENV_IDX] != nullptr);
        COMPILER_LOG(DEBUG, IR_BUILDER) << "update lexical environment from "
                                        << std::to_string(
                                               (*currentDefs_)[vregsAndArgsCount_ + 1 + LEX_ENV_IDX]->GetId())
                                        << " to " << inst->GetId();
        (*currentDefs_)[vregsAndArgsCount_ + 1 + LEX_ENV_IDX] = inst;
    }

    Inst *GetDefinition(size_t vreg)
    {
        ASSERT(vreg < currentDefs_->size());
        ASSERT((*currentDefs_)[vreg] != nullptr);

        if (vreg >= currentDefs_->size() || (*currentDefs_)[vreg] == nullptr) {
            failed_ = true;
            COMPILER_LOG(ERROR, IR_BUILDER) << "GetDefinition failed for verg " << vreg;
            return nullptr;
        }
        return (*currentDefs_)[vreg];
    }

    Inst *GetDefinitionAcc()
    {
        auto *accInst = (*currentDefs_)[vregsAndArgsCount_];
        ASSERT(accInst != nullptr);

        if (accInst == nullptr) {
            failed_ = true;
            COMPILER_LOG(ERROR, IR_BUILDER) << "GetDefinitionAcc failed";
        }
        return accInst;
    }

    Inst *GetEnvDefinition(uint8_t envIdx)
    {
        auto *inst = (*currentDefs_)[vregsAndArgsCount_ + 1 + envIdx];
        ASSERT(inst != nullptr);

        if (inst == nullptr) {
            failed_ = true;
        }
        return inst;
    }

    auto FindOrCreate32BitConstant(uint32_t value)
    {
        auto inst = GetGraph()->FindOrCreateConstant<uint32_t>(value);
        if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value << ", inst=" << inst->GetId();
        }
        return inst;
    }

    auto FindOrCreateConstant(uint64_t value)
    {
        auto inst = GetGraph()->FindOrCreateConstant<uint64_t>(value);
        if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value << ", inst=" << inst->GetId();
        }
        return inst;
    }

    auto FindOrCreateAnyConstant(DataType::Any value)
    {
        auto inst = GetGraph()->FindOrCreateConstant(value);
        if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
            COMPILER_LOG(DEBUG, IR_BUILDER)
                << "create new constant: value=" << value.Raw() << ", inst=" << inst->GetId();
        }
        return inst;
    }

    auto FindOrCreateDoubleConstant(double value)
    {
        auto inst = GetGraph()->FindOrCreateConstant<double>(value);
        if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value << ", inst=" << inst->GetId();
        }
        return inst;
    }

    auto FindOrCreateFloatConstant(float value)
    {
        auto inst = GetGraph()->FindOrCreateConstant<float>(value);
        if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create new constant: value=" << value << ", inst=" << inst->GetId();
        }
        return inst;
    }

    enum SaveStateType {
        CHECK = 0,  // side_exit = true,  move_to_side_exit = true
        CALL,       // side_exit = false,  move_to_side_exit = false
        VIRT_CALL   // side_exit = true,  move_to_side_exit = false
    };

    ClassInst *CreateLoadAndInitClassGeneric(uint32_t classId, size_t pc);

    Inst *CreateCast(Inst *input, DataType::Type type, DataType::Type operandsType, size_t pc)
    {
        auto cast = GetGraph()->CreateInstCast(type, pc, input, operandsType);
        if (!input->HasType()) {
            input->SetType(operandsType);
        }
        return cast;
    }

    NewObjectInst *CreateNewObjectInst(size_t pc, uint32_t typeId, SaveStateInst *saveState, Inst *initClass)
    {
        auto newObj =
            graph_->CreateInstNewObject(DataType::REFERENCE, pc, initClass, saveState, typeId, graph_->GetMethod());
        return newObj;
    }

    template <Opcode OPCODE>
    void BuildCall(const BytecodeInstruction *bcInst, bool isRange, bool accRead, Inst *additionalInput = nullptr);
    template <Opcode OPCODE>
    CallInst *BuildCallInst(RuntimeInterface::MethodPtr method, uint32_t methodId, size_t pc, Inst **resolver,
                            uint32_t classId);
    template <Opcode OPCODE>
    CallInst *BuildCallStaticInst(RuntimeInterface::MethodPtr method, uint32_t methodId, size_t pc, Inst **resolver,
                                  [[maybe_unused]] uint32_t classId);
    template <Opcode OPCODE>
    CallInst *BuildCallVirtualInst(RuntimeInterface::MethodPtr method, uint32_t methodId, size_t pc, Inst **resolver);
    void BuildInitClassInstForCallStatic(RuntimeInterface::MethodPtr method, uint32_t classId, size_t pc,
                                         Inst *saveState);
    template <typename T>
    void SetCallArgs(const BytecodeInstruction *bcInst, bool isRange, bool accRead, Inst *resolver, T *call,
                     Inst *nullCheck, SaveStateInst *saveState, bool hasImplicitArg, uint32_t methodId,
                     Inst *additionalInput = nullptr);
    Inst *GetArgDefinition(const BytecodeInstruction *bcInst, size_t idx, bool accRead, bool isRange = false);
    Inst *GetArgDefinitionRange(const BytecodeInstruction *bcInst, size_t idx);
    template <bool IS_VIRTUAL>
    void AddArgNullcheckIfNeeded(RuntimeInterface::IntrinsicId intrinsic, Inst *inst, Inst *saveState, size_t bcAddr);
    void BuildMonitor(const BytecodeInstruction *bcInst, Inst *def, bool isEnter);
    Inst *BuildFloatInst(const BytecodeInstruction *bcInst);
    void BuildIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead);
    void BuildDefaultIntrinsic(bool isVirtual, const BytecodeInstruction *bcInst, bool isRange, bool accRead);
    void BuildStaticCallIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead);
    void BuildAbsIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    template <Opcode OPCODE>
    void BuildBinaryOperationIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildSqrtIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildIsNanIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildStringLengthIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildStringIsEmptyIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildCharIsUpperCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildCharToUpperCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildCharIsLowerCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildCharToLowerCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildMonitorIntrinsic(const BytecodeInstruction *bcInst, bool isEnter, bool accRead);
    void BuildDefaultStaticIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead);
    void BuildDefaultVirtualCallIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead);
    void BuildVirtualCallIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead);
    void BuildThrow(const BytecodeInstruction *bcInst);
    void BuildLenArray(const BytecodeInstruction *bcInst);
    void BuildNewArray(const BytecodeInstruction *bcInst);
    void BuildNewObject(const BytecodeInstruction *bcInst);
    void BuildLoadConstArray(const BytecodeInstruction *bcInst);
    void BuildLoadConstStringArray(const BytecodeInstruction *bcInst);
    template <typename T>
    void BuildUnfoldLoadConstArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                   const pandasm::LiteralArray &litArray);
    template <typename T>
    void BuildUnfoldLoadConstStringArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                         const pandasm::LiteralArray &litArray, NewArrayInst *arrayInst);
    void BuildInitString(const BytecodeInstruction *bcInst);
    void BuildInitObject(const BytecodeInstruction *bcInst, bool isRange);
    CallInst *BuildCallStaticForInitObject(const BytecodeInstruction *bcInst, uint32_t methodId, Inst **resolver);
    void BuildMultiDimensionalArrayObject(const BytecodeInstruction *bcInst, bool isRange);
    void BuildInitObjectMultiDimensionalArray(const BytecodeInstruction *bcInst, bool isRange);
    template <bool IS_ACC_WRITE>
    void BuildLoadObject(const BytecodeInstruction *bcInst, DataType::Type type);
    template <bool IS_ACC_READ>
    void BuildStoreObject(const BytecodeInstruction *bcInst, DataType::Type type);
    Inst *BuildStoreObjectInst(const BytecodeInstruction *bcInst, DataType::Type type, RuntimeInterface::FieldPtr field,
                               size_t fieldId, Inst **resolveInst);
    void BuildLoadStatic(const BytecodeInstruction *bcInst, DataType::Type type);
    Inst *BuildLoadStaticInst(size_t pc, DataType::Type type, size_t typeId, Inst *saveState);
    void BuildStoreStatic(const BytecodeInstruction *bcInst, DataType::Type type);
    Inst *BuildStoreStaticInst(const BytecodeInstruction *bcInst, DataType::Type type, size_t typeId, Inst *storeInput,
                               Inst *saveState);
    void BuildCheckCast(const BytecodeInstruction *bcInst);
    void BuildIsInstance(const BytecodeInstruction *bcInst);
    Inst *BuildLoadClass(RuntimeInterface::IdType typeId, size_t pc, Inst *saveState);
    void BuildLoadArray(const BytecodeInstruction *bcInst, DataType::Type type);
    void BuildStoreArray(const BytecodeInstruction *bcInst, DataType::Type type);
    template <bool CREATE_REF_CHECK>
    void BuildStoreArrayInst(const BytecodeInstruction *bcInst, DataType::Type type, Inst *arrayRef, Inst *index,
                             Inst *value);
    void BuildChecksBeforeArray(size_t pc, Inst *arrayRef, Inst **ss, Inst **nc, Inst **al, Inst **bc,
                                bool withNullcheck = true);
    template <Opcode OPCODE>
    void BuildLoadFromPool(const BytecodeInstruction *bcInst);
    void BuildCastToAnyString(const BytecodeInstruction *bcInst);
    void BuildCastToAnyNumber(const BytecodeInstruction *bcInst);
    Inst *BuildAnyTypeCheckInst(size_t bcAddr, Inst *input, Inst *saveState,
                                AnyBaseType type = AnyBaseType::UNDEFINED_TYPE, bool typeWasProfiled = false,
                                profiling::AnyInputType allowedInputType = {});
    bool TryBuildStringCharAtIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
#include "inst_builder_extensions.inl.h"

    Graph *GetGraph()
    {
        return graph_;
    }

    const Graph *GetGraph() const
    {
        return graph_;
    }

    const RuntimeInterface *GetRuntime() const
    {
        return runtime_;
    }

    RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }

    auto GetMethod() const
    {
        return method_;
    }

    auto GetClassId() const
    {
        return classId_;
    }

    Marker GetNoTypeMarker() const
    {
        return noTypeMarker_;
    }

    Marker GetVisitedBlockMarker() const
    {
        return visitedBlockMarker_;
    }

    bool ForceUnresolved() const
    {
#ifndef NDEBUG
        return g_options.IsCompilerForceUnresolved() && !graph_->IsBytecodeOptimizer();
#else
        return false;
#endif
    }

    void SetTypeRec(Inst *inst, DataType::Type type);

    /// Convert Panda bytecode type to COMPILER IR type
    static DataType::Type ConvertPbcType(panda_file::Type type);

    /// Get return type of the method specified by id
    DataType::Type GetMethodReturnType(uintptr_t id) const;
    /// Get type of argument of the method specified by id
    DataType::Type GetMethodArgumentType(uintptr_t id, size_t index) const;
    /// Get count of arguments for the method specified by id
    size_t GetMethodArgumentsCount(uintptr_t id) const;
    /// Get return type of currently compiling method
    DataType::Type GetCurrentMethodReturnType() const;
    /// Get type of argument of currently compiling method
    DataType::Type GetCurrentMethodArgumentType(size_t index) const;
    /// Get count of arguments of currently compiling method
    size_t GetCurrentMethodArgumentsCount() const;

    template <bool IS_STATIC>
    bool IsInConstructor() const;

#ifndef PANDA_ETS_INTEROP_JS
    bool TryBuildInteropCall([[maybe_unused]] const BytecodeInstruction *bcInst, [[maybe_unused]] bool isRange,
                             [[maybe_unused]] bool accRead)
    {
        return false;
    }
#endif

private:
    static constexpr size_t ONE_FOR_OBJECT = 1;
    static constexpr size_t ONE_FOR_SSTATE = 1;

    Graph *graph_ {nullptr};
    RuntimeInterface *runtime_ {nullptr};
    BasicBlock *currentBb_ {nullptr};

    RuntimeInterface::MethodProfile methodProfile_ {};

    // Definitions vector of currently processed basic block
    InstVector *currentDefs_ {nullptr};
    // Result of LoadFromConstantPool which will be added to SaveState inputs
    Inst *additionalDef_ {nullptr};
    // Contains definitions of the virtual registers in all basic blocks
    ArenaVector<InstVector> defs_;

    RuntimeInterface::MethodPtr method_ {nullptr};
    // Set to true if builder failed to build IR
    bool failed_ {false};
    // Number of virtual registers and method arguments
    const size_t vregsAndArgsCount_;
    // Marker for instructions with undefined type in the building phase
    Marker noTypeMarker_;
    Marker visitedBlockMarker_;

    // Pointer to start position of bytecode instructions buffer
    const uint8_t *instructionsBuf_ {nullptr};

    CallInst *callerInst_ {nullptr};
    uint32_t inliningDepth_ {0};
    size_t classId_;
#include "intrinsics_ir_build.inl.h"
};
}  // namespace ark::compiler

#endif  // PANDA_INST_BUILDER_H
