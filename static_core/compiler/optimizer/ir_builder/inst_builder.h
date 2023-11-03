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

namespace panda::compiler {
constexpr int64_t INVALID_OFFSET = std::numeric_limits<int64_t>::max();

class InstBuilder {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENV_IDX(ENV_TYPE) \
    static constexpr uint8_t ENV_TYPE##_IDX = VRegInfo::VRegType::ENV_TYPE - VRegInfo::VRegType::ENV_BEGIN;
    VREGS_ENV_TYPE_DEFS(ENV_IDX)
#undef ENV_IDX
public:
    InstBuilder(Graph *graph, RuntimeInterface::MethodPtr method, CallInst *caller_inst, uint32_t inlining_depth)
        : graph_(graph),
          runtime_(graph->GetRuntime()),
          defs_(graph->GetLocalAllocator()->Adapter()),
          method_(method),
          vregs_and_args_count_(graph->GetRuntime()->GetMethodRegistersCount(method) +
                                graph->GetRuntime()->GetMethodTotalArgumentsCount(method)),
          instructions_buf_(GetGraph()->GetRuntime()->GetMethodCode(GetGraph()->GetMethod())),
          caller_inst_(caller_inst),
          inlining_depth_(inlining_depth),
          class_id_ {runtime_->GetClassIdForMethod(method_)}
    {
        no_type_marker_ = GetGraph()->NewMarker();
        visited_block_marker_ = GetGraph()->NewMarker();

        defs_.resize(graph_->GetVectorBlocks().size(), InstVector(graph->GetLocalAllocator()->Adapter()));
        for (auto &v : defs_) {
            v.resize(GetVRegsCount());
        }

        for (auto bb : graph->GetBlocksRPO()) {
            if (bb->IsCatchBegin()) {
                for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
                    auto catch_phi = GetGraph()->CreateInstCatchPhi(DataType::NO_TYPE, bb->GetGuestPc());
                    catch_phi->SetMarker(GetNoTypeMarker());
                    bb->AppendInst(catch_phi);
                    COMPILER_LOG(DEBUG, IR_BUILDER)
                        << "Creat catchphi " << catch_phi->GetId() << " for bb(" << bb->GetId() << ")";
                    if (vreg == vregs_and_args_count_) {
                        catch_phi->SetIsAcc();
                    } else if (vreg > vregs_and_args_count_) {
                        catch_phi->SetType(DataType::ANY);
                    }
                }
            }
        }
    }

    NO_COPY_SEMANTIC(InstBuilder);
    NO_MOVE_SEMANTIC(InstBuilder);
    ~InstBuilder()
    {
        GetGraph()->EraseMarker(no_type_marker_);
        GetGraph()->EraseMarker(visited_block_marker_);
    }

    /**
     * Content of this function is auto generated from inst_builder.erb and is located in inst_builder_gen.cpp file
     * @param instruction Pointer to bytecode instruction
     */
    void BuildInstruction(const BytecodeInstruction *instruction);

    void InitEnv(BasicBlock *bb)
    {
        auto this_func = GetGraph()->FindParameter(0);
        auto cp = GetGraph()->CreateInstLoadConstantPool(DataType::ANY, INVALID_PC, this_func);
        bb->AppendInst(cp);

        auto lex_env = GetGraph()->CreateInstLoadLexicalEnv(DataType::ANY, INVALID_PC, this_func);
        bb->AppendInst(lex_env);

        defs_[bb->GetId()][vregs_and_args_count_ + 1 + THIS_FUNC_IDX] = this_func;
        defs_[bb->GetId()][vregs_and_args_count_ + 1 + CONST_POOL_IDX] = cp;
        defs_[bb->GetId()][vregs_and_args_count_ + 1 + LEX_ENV_IDX] = lex_env;
        COMPILER_LOG(DEBUG, IR_BUILDER) << "Init environment this_func = " << this_func->GetId()
                                        << ", const_pool = " << cp->GetId() << ", lex_env = " << lex_env->GetId();
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
            current_bb_ != GetGraph()->GetStartBlock() && current_defs_ != nullptr) {
            ASSERT((*current_defs_)[vregs_and_args_count_ + 1 + THIS_FUNC_IDX] != nullptr);
            ASSERT((*current_defs_)[vregs_and_args_count_ + 1 + CONST_POOL_IDX] != nullptr);
            ASSERT((*current_defs_)[vregs_and_args_count_ + 1 + LEX_ENV_IDX] != nullptr);
        }
        current_bb_ = bb;
        current_defs_ = &defs_[bb->GetId()];
    }

    BasicBlock *GetCurrentBlock() const
    {
        return current_bb_;
    }

    void Prepare(bool is_inlined_graph);

    void FixInstructions();
    void ResolveConstants();
    void SplitConstant(ConstantInst *const_inst);

    static void RemoveNotDominateInputs(SaveStateInst *save_state);

    size_t GetPc(const uint8_t *inst_ptr) const;

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

    const auto &GetCurrentDefs()
    {
        ASSERT(current_defs_ != nullptr);
        return *current_defs_;
    }

    void AddCatchPhiInputs(const ArenaUnorderedSet<BasicBlock *> &catch_handlers, const InstVector &defs,
                           Inst *throwable_inst);

    SaveStateInst *CreateSaveState(Opcode opc, size_t pc);

    static void SetParamSpillFill(Graph *graph, ParameterInst *param_inst, size_t num_args, size_t i,
                                  DataType::Type type);

private:
    void SyncWithGraph();

    void UpdateDefsForCatch();
    void UpdateDefsForLoopHead();

    size_t GetVRegsCount() const
    {
        return vregs_and_args_count_ + 1 + GetGraph()->GetEnvCount();
    }

    template <typename T>
    void AddInstruction(T inst)
    {
        ASSERT(current_bb_);
        current_bb_->AppendInst(inst);

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
        ASSERT(vreg < current_defs_->size());
        COMPILER_LOG(DEBUG, IR_BUILDER) << "update def for r" << vreg << " from "
                                        << ((*current_defs_)[vreg] != nullptr
                                                ? std::to_string((*current_defs_)[vreg]->GetId())
                                                : "null")
                                        << " to " << inst->GetId();
        (*current_defs_)[vreg] = inst;
    }

    void UpdateDefinitionAcc(Inst *inst)
    {
        if (inst == nullptr) {
            COMPILER_LOG(DEBUG, IR_BUILDER) << "reset accumulator definition";
        } else {
            COMPILER_LOG(DEBUG, IR_BUILDER) << "update accumulator from "
                                            << ((*current_defs_)[vregs_and_args_count_] != nullptr
                                                    ? std::to_string((*current_defs_)[vregs_and_args_count_]->GetId())
                                                    : "null")
                                            << " to " << inst->GetId();
        }
        (*current_defs_)[vregs_and_args_count_] = inst;
    }

    void UpdateDefinitionLexEnv(Inst *inst)
    {
        ASSERT(inst != nullptr);
        ASSERT((*current_defs_)[vregs_and_args_count_ + 1 + LEX_ENV_IDX] != nullptr);
        COMPILER_LOG(DEBUG, IR_BUILDER) << "update lexical environment from "
                                        << std::to_string(
                                               (*current_defs_)[vregs_and_args_count_ + 1 + LEX_ENV_IDX]->GetId())
                                        << " to " << inst->GetId();
        (*current_defs_)[vregs_and_args_count_ + 1 + LEX_ENV_IDX] = inst;
    }

    Inst *GetDefinition(size_t vreg)
    {
        ASSERT(vreg < current_defs_->size());
        ASSERT((*current_defs_)[vreg] != nullptr);

        if (vreg >= current_defs_->size() || (*current_defs_)[vreg] == nullptr) {
            failed_ = true;
            COMPILER_LOG(ERROR, IR_BUILDER) << "GetDefinition failed for verg " << vreg;
            return nullptr;
        }
        return (*current_defs_)[vreg];
    }

    Inst *GetDefinitionAcc()
    {
        auto *acc_inst = (*current_defs_)[vregs_and_args_count_];
        ASSERT(acc_inst != nullptr);

        if (acc_inst == nullptr) {
            failed_ = true;
            COMPILER_LOG(ERROR, IR_BUILDER) << "GetDefinitionAcc failed";
        }
        return acc_inst;
    }

    Inst *GetEnvDefinition(uint8_t env_idx)
    {
        auto *inst = (*current_defs_)[vregs_and_args_count_ + 1 + env_idx];
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

    ClassInst *CreateLoadAndInitClassGeneric(uint32_t class_id, size_t pc);

    Inst *CreateCast(Inst *input, DataType::Type type, DataType::Type operands_type, size_t pc)
    {
        auto cast = GetGraph()->CreateInstCast(type, pc, input, operands_type);
        if (!input->HasType()) {
            input->SetType(operands_type);
        }
        return cast;
    }

    NewObjectInst *CreateNewObjectInst(size_t pc, uint32_t type_id, SaveStateInst *save_state, Inst *init_class)
    {
        auto new_obj =
            graph_->CreateInstNewObject(DataType::REFERENCE, pc, init_class, save_state, type_id, graph_->GetMethod());
        return new_obj;
    }

    template <Opcode OPCODE>
    void BuildCall(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read, Inst *additional_input = nullptr);
    template <Opcode OPCODE>
    CallInst *BuildCallInst(RuntimeInterface::MethodPtr method, uint32_t method_id, size_t pc, Inst **resolver,
                            uint32_t class_id);
    void BuildInitClassInstForCallStatic(RuntimeInterface::MethodPtr method, uint32_t class_id, size_t pc,
                                         Inst *save_state);
    template <typename T>
    void SetCallArgs(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read, Inst *resolver, T *call,
                     Inst *null_check, SaveStateInst *save_state, bool has_implicit_arg, uint32_t method_id,
                     Inst *additional_input = nullptr);
    Inst *GetArgDefinition(const BytecodeInstruction *bc_inst, size_t idx, bool acc_read, bool is_range = false);
    Inst *GetArgDefinitionRange(const BytecodeInstruction *bc_inst, size_t idx);
    template <bool IS_VIRTUAL>
    void AddArgNullcheckIfNeeded(RuntimeInterface::IntrinsicId intrinsic, Inst *inst, Inst *save_state, size_t bc_addr);
    void BuildMonitor(const BytecodeInstruction *bc_inst, Inst *def, bool is_enter);
    Inst *BuildFloatInst(const BytecodeInstruction *bc_inst);
    void BuildIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildDefaultIntrinsic(bool is_virtual, const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildStaticCallIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildAbsIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    template <Opcode OPCODE>
    void BuildBinaryOperationIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildSqrtIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildIsNanIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildStringLengthIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildStringIsEmptyIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildCharIsUpperCaseIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildCharToUpperCaseIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildCharIsLowerCaseIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildCharToLowerCaseIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildMonitorIntrinsic(const BytecodeInstruction *bc_inst, bool is_enter, bool acc_read);
    void BuildDefaultStaticIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildDefaultVirtualCallIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildVirtualCallIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildThrow(const BytecodeInstruction *bc_inst);
    void BuildLenArray(const BytecodeInstruction *bc_inst);
    void BuildNewArray(const BytecodeInstruction *bc_inst);
    void BuildNewObject(const BytecodeInstruction *bc_inst);
    void BuildLoadConstArray(const BytecodeInstruction *bc_inst);
    template <typename T>
    void BuildUnfoldLoadConstArray(const BytecodeInstruction *bc_inst, DataType::Type type,
                                   const pandasm::LiteralArray &lit_array);
    void BuildInitString(const BytecodeInstruction *bc_inst);
    void BuildInitObject(const BytecodeInstruction *bc_inst, bool is_range);
    CallInst *BuildCallStaticForInitObject(const BytecodeInstruction *bc_inst, uint32_t method_id, Inst **resolver);
    void BuildMultiDimensionalArrayObject(const BytecodeInstruction *bc_inst, bool is_range);
    void BuildInitObjectMultiDimensionalArray(const BytecodeInstruction *bc_inst, bool is_range);
    template <bool IS_ACC_WRITE>
    void BuildLoadObject(const BytecodeInstruction *bc_inst, DataType::Type type);
    template <bool IS_ACC_READ>
    void BuildStoreObject(const BytecodeInstruction *bc_inst, DataType::Type type);
    Inst *BuildStoreObjectInst(const BytecodeInstruction *bc_inst, DataType::Type type,
                               RuntimeInterface::FieldPtr field, size_t field_id, Inst **resolve_inst);
    void BuildLoadStatic(const BytecodeInstruction *bc_inst, DataType::Type type);
    Inst *BuildLoadStaticInst(const BytecodeInstruction *bc_inst, DataType::Type type, size_t type_id,
                              Inst *save_state);
    void BuildStoreStatic(const BytecodeInstruction *bc_inst, DataType::Type type);
    Inst *BuildStoreStaticInst(const BytecodeInstruction *bc_inst, DataType::Type type, size_t type_id,
                               Inst *store_input, Inst *save_state);
    void BuildCheckCast(const BytecodeInstruction *bc_inst);
    void BuildIsInstance(const BytecodeInstruction *bc_inst);
    Inst *BuildLoadClass(RuntimeInterface::IdType type_id, size_t pc, Inst *save_state);
    void BuildLoadArray(const BytecodeInstruction *bc_inst, DataType::Type type);
    void BuildStoreArray(const BytecodeInstruction *bc_inst, DataType::Type type);
    template <bool CREATE_REF_CHECK>
    void BuildStoreArrayInst(const BytecodeInstruction *bc_inst, DataType::Type type, Inst *array_ref, Inst *index,
                             Inst *value);
    void BuildChecksBeforeArray(size_t pc, Inst *array_ref, Inst **ss, Inst **nc, Inst **al, Inst **bc,
                                bool with_nullcheck = true);
    template <Opcode OPCODE>
    void BuildLoadFromPool(const BytecodeInstruction *bc_inst);
    void BuildCastToAnyString(const BytecodeInstruction *bc_inst);
    void BuildCastToAnyNumber(const BytecodeInstruction *bc_inst);
    Inst *BuildAnyTypeCheckInst(size_t bc_addr, Inst *input, Inst *save_state,
                                AnyBaseType type = AnyBaseType::UNDEFINED_TYPE, bool type_was_profiled = false,
                                profiling::AnyInputType allowed_input_type = {});
    bool TryBuildStringCharAtIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
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
        return class_id_;
    }

    Marker GetNoTypeMarker() const
    {
        return no_type_marker_;
    }

    Marker GetVisitedBlockMarker() const
    {
        return visited_block_marker_;
    }

    bool ForceUnresolved() const
    {
#ifndef NDEBUG
        return OPTIONS.IsCompilerForceUnresolved() && !graph_->IsBytecodeOptimizer();
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

#ifndef PANDA_ETS_INTEROP_JS
    bool TryBuildInteropCall([[maybe_unused]] const BytecodeInstruction *bc_inst, [[maybe_unused]] bool is_range,
                             [[maybe_unused]] bool acc_read)
    {
        return false;
    }
#endif

private:
    static constexpr size_t ONE_FOR_OBJECT = 1;
    static constexpr size_t ONE_FOR_SSTATE = 1;

    Graph *graph_ {nullptr};
    RuntimeInterface *runtime_ {nullptr};
    BasicBlock *current_bb_ {nullptr};

    RuntimeInterface::MethodProfile method_profile_ {};

    // Definitions vector of currently processed basic block
    InstVector *current_defs_ {nullptr};
    // Result of LoadFromConstantPool which will be added to SaveState inputs
    Inst *additional_def_ {nullptr};
    // Contains definitions of the virtual registers in all basic blocks
    ArenaVector<InstVector> defs_;

    RuntimeInterface::MethodPtr method_ {nullptr};
    // Set to true if builder failed to build IR
    bool failed_ {false};
    // Number of virtual registers and method arguments
    const size_t vregs_and_args_count_;
    // Marker for instructions with undefined type in the building phase
    Marker no_type_marker_;
    Marker visited_block_marker_;

    // Pointer to start position of bytecode instructions buffer
    const uint8_t *instructions_buf_ {nullptr};

    CallInst *caller_inst_ {nullptr};
    uint32_t inlining_depth_ {0};
    size_t class_id_;
#include "intrinsics_ir_build.inl.h"
};
}  // namespace panda::compiler

#endif  // PANDA_INST_BUILDER_H
