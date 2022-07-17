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

#ifndef PANDA_INST_BUILDER_H
#define PANDA_INST_BUILDER_H

#include "compiler_options.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "code_data_accessor.h"
#include "file_items.h"
#include "compiler_logger.h"

#include "bytecode_instruction.h"

namespace panda::compiler {
constexpr int64_t INVALID_OFFSET = std::numeric_limits<int64_t>::max();

class InstBuilder {
public:
    InstBuilder(Graph *graph, RuntimeInterface::MethodPtr method, CallInst *caller_inst)
        : graph_(graph),
          runtime_(graph->GetRuntime()),
          defs_(graph->GetLocalAllocator()->Adapter()),
          method_(method),
          VREGS_AND_ARGS_COUNT(graph->GetRuntime()->GetMethodRegistersCount(method) +
                               graph->GetRuntime()->GetMethodTotalArgumentsCount(method)),
          instructions_buf_(GetGraph()->GetRuntime()->GetMethodCode(GetGraph()->GetMethod())),
          caller_inst_(caller_inst),
          class_id_ {runtime_->GetClassIdForMethod(method_)}
    {
        no_type_marker_ = GetGraph()->NewMarker();
        visited_block_marker_ = GetGraph()->NewMarker();

        defs_.resize(graph_->GetVectorBlocks().size(), InstVector(graph->GetLocalAllocator()->Adapter()));
        for (auto &v : defs_) {
            v.resize(VREGS_AND_ARGS_COUNT + 1);
        }

        for (auto bb : graph->GetBlocksRPO()) {
            if (bb->IsCatchBegin()) {
                for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
                    auto catch_phi = GetGraph()->CreateInstCatchPhi();
                    catch_phi->SetPc(bb->GetGuestPc());
                    catch_phi->SetMarker(GetNoTypeMarker());
                    bb->AppendInst(catch_phi);
                    if (vreg == VREGS_AND_ARGS_COUNT) {
                        catch_phi->SetIsAcc();
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

    bool IsFailed() const
    {
        return failed_;
    }

    /**
     * Return jump offset for instruction `inst`, 0 if it is not jump instruction.
     */
    static int64_t GetInstructionJumpOffset(const BytecodeInstruction *inst);

    void SetCurrentBlock(BasicBlock *bb)
    {
        current_bb_ = bb;
        current_defs_ = &defs_[bb->GetId()];
    }

    void Prepare(bool is_inlined_graph);

    void FixInstructions();
    void ResolveConstants();
    void SplitConstant(ConstantInst *const_inst);
    void CleanupCatchPhis();

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
        return VREGS_AND_ARGS_COUNT + 1;
    }

    void AddInstruction(Inst *inst)
    {
        ASSERT(current_bb_);
        current_bb_->AppendInst(inst);
        COMPILER_LOG(DEBUG, IR_BUILDER)

            << *inst;
    }

    void AddInstructions(Inst *inst1, Inst *inst2, Inst *inst3, Inst *inst4)
    {
        AddInstruction(inst1);
        AddInstruction(inst2);
        AddInstruction(inst3);
        AddInstruction(inst4);
    }

    void AddInstructions(Inst *inst1, Inst *inst2, Inst *inst3, Inst *inst4, Inst *inst5)
    {
        AddInstruction(inst1);
        AddInstruction(inst2);
        AddInstruction(inst3);
        AddInstruction(inst4);
        AddInstruction(inst5);
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
                                            << ((*current_defs_)[VREGS_AND_ARGS_COUNT] != nullptr
                                                    ? std::to_string((*current_defs_)[VREGS_AND_ARGS_COUNT]->GetId())
                                                    : "null")
                                            << " to " << inst->GetId();
        }
        (*current_defs_)[VREGS_AND_ARGS_COUNT] = inst;
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
        auto *acc_inst = (*current_defs_)[VREGS_AND_ARGS_COUNT];
        ASSERT(acc_inst != nullptr);

        if (acc_inst == nullptr) {
            failed_ = true;
            COMPILER_LOG(ERROR, IR_BUILDER) << "GetDefinitionAcc failed";
        }
        return acc_inst;
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
        auto cast = GetGraph()->CreateInstCast(type, pc);
        cast->CastToCast()->SetOperandsType(operands_type);
        cast->SetInput(0, input);
        if (!input->HasType()) {
            input->SetType(operands_type);
        }
        return cast;
    }

    template <Opcode opcode>
    void BuildCall(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildDynamicCall(const BytecodeInstruction *bc_inst, bool is_range);
    template <Opcode opcode>
    CallInst *BuildCallInst(RuntimeInterface::MethodPtr method, uint32_t method_id, size_t pc);
    template <Opcode opcode>
    CallInst *BuildUnresolvedCallInst(uint32_t method_id, size_t pc);
    void BuildInitClassInstForCallStatic(RuntimeInterface::MethodPtr method, uint32_t class_id, size_t pc,
                                         Inst *save_state);
    template <typename T>
    void SetInputsForCallInst(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read, T *inst,
                              Inst *null_check, uint32_t method_id, bool has_implicit_arg, bool need_savestate);
    Inst *GetArgDefinition(const BytecodeInstruction *bc_inst, size_t idx, bool acc_read);
    template <bool is_virtual>
    void AddArgNullcheckIfNeeded(RuntimeInterface::IntrinsicId intrinsic, Inst *inst, Inst *save_state, size_t bc_addr);
    void BuildMonitor(const BytecodeInstruction *bc_inst, Inst *def, bool is_enter);
    void BuildEcma([[maybe_unused]] const BytecodeInstruction *bc_inst);
    template <bool with_speculative = false>
    void BuildEcmaAsIntrinsics([[maybe_unused]] const BytecodeInstruction *bc_inst);
    void BuildEcmaFromIrtoc([[maybe_unused]] const BytecodeInstruction *bc_inst);

    Inst *BuildFloatInst(const BytecodeInstruction *bc_inst);
    void BuildIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildDefaultIntrinsic(bool is_virtual, const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildStaticCallIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);
    void BuildAbsIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    template <Opcode opcode>
    void BuildBinaryOperationIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildSqrtIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
    void BuildIsNanIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read);
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
    void BuildInitObject(const BytecodeInstruction *bc_inst, bool is_range);
    CallInst *BuildCallStaticForInitObject(const BytecodeInstruction *bc_inst, uint32_t method_id);
    void BuildMultiDimensionalArrayObject(const BytecodeInstruction *bc_inst, bool is_range);
    void BuildInitObjectMultiDimensionalArray(const BytecodeInstruction *bc_inst, bool is_range);
    template <bool is_acc_write>
    void BuildLoadObject(const BytecodeInstruction *bc_inst, DataType::Type type);
    template <bool is_acc_read>
    void BuildStoreObject(const BytecodeInstruction *bc_inst, DataType::Type type);
    Inst *BuildStoreObjectInst(const BytecodeInstruction *bc_inst, DataType::Type type,
                               RuntimeInterface::FieldPtr field, size_t type_id);
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
    template <bool create_ref_check>
    void BuildStoreArrayInst(const BytecodeInstruction *bc_inst, DataType::Type type, Inst *array_ref, Inst *index,
                             Inst *value);
    void BuildChecksBeforeArray(const BytecodeInstruction *bc_inst, Inst *array_ref, Inst **ss, Inst **nc, Inst **al,
                                Inst **bc);
    template <Opcode opcode>
    void BuildLoadFromPool(const BytecodeInstruction *bc_inst);
    void BuildCastToAnyString(const BytecodeInstruction *bc_inst);
    void BuildCastToAnyNumber(const BytecodeInstruction *bc_inst);
    Inst *BuildAnyTypeCheckInst(size_t bc_addr, Inst *input, Inst *save_state,
                                AnyBaseType type = AnyBaseType::UNDEFINED_TYPE);

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

private:
    static constexpr size_t INPUT_2 = 2;
    static constexpr size_t INPUT_3 = 3;
    static constexpr size_t TWO_INPUTS = 2;

    Graph *graph_ {nullptr};
    RuntimeInterface *runtime_ {nullptr};
    BasicBlock *current_bb_ {nullptr};

    // Definitions vector of currently processed basic block
    InstVector *current_defs_ {nullptr};
    // Contains definitions of the virtual registers in all basic blocks
    ArenaVector<InstVector> defs_;

    RuntimeInterface::MethodPtr method_ {nullptr};
    // Set to true if builder failed to build IR
    bool failed_ {false};
    // Number of virtual registers and method arguments
    const size_t VREGS_AND_ARGS_COUNT;
    // Marker for instructions with undefined type in the building phase
    Marker no_type_marker_;
    Marker visited_block_marker_;

    // Pointer to start position of bytecode instructions buffer
    const uint8_t *instructions_buf_ {nullptr};

    CallInst *caller_inst_ {nullptr};
    size_t class_id_;
#include "intrinsics_ir_build.inl.h"
};
}  // namespace panda::compiler

#endif  // PANDA_INST_BUILDER_H
