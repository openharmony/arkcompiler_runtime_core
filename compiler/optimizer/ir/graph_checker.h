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

#ifndef COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_H
#define COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_H

#include <iostream>
#include "compiler_options.h"
#include "graph.h"
#include "graph_visitor.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/memory_coalescing.h"

namespace panda::compiler {
inline std::ostream &operator<<(std::ostream &os, const std::initializer_list<Opcode> &opcs)
{
    os << "[ ";
    for (auto opc : opcs) {
        os << GetOpcodeString(opc) << " ";
    }
    os << "]";
    return os;
}

class GraphChecker : public GraphVisitor {
public:
    explicit GraphChecker(Graph *graph);
    ~GraphChecker() override
    {
        GetGraph()->GetPassManager()->SetCheckMode(false);
    }

    NO_COPY_SEMANTIC(GraphChecker);
    NO_MOVE_SEMANTIC(GraphChecker);

    void Check();

private:
    void PreCloneChecks(Graph *graph);
    void UserInputCheck(Graph *graph);
    void CheckBlock(BasicBlock *block);
    void CheckDomTree();
    void CheckLoopAnalysis();
    void CheckStartBlock();
    void CheckEndBlock();
    void CheckControlFlow(BasicBlock *block);
    void CheckDataFlow(BasicBlock *block);
    void CheckPhiInputs(Inst *phi_inst);
    void CheckInstsRegisters(BasicBlock *block);
    void CheckPhisRegisters(BasicBlock *block);
    void CheckNoLowLevel(BasicBlock *block);
    void CheckLoops();
    void CheckGraph();
    bool HasOuterInfiniteLoop();
    bool CheckInstHasInput(Inst *inst, Inst *input);
    bool CheckInstHasUser(Inst *inst, Inst *user);
    void CheckCallReturnInlined();
    void CheckSaveStateCaller(SaveStateInst *savestate);
    void CheckSpillFillHolder(Inst *inst);
    bool CheckInstRegUsageSaved(const Inst *inst, Register reg) const;
    void MarkBlocksInLoop(Loop *loop, Marker mrk);
    bool CheckBlockHasPredecessor(BasicBlock *block, BasicBlock *predecessor);
    bool CheckBlockHasSuccessor(BasicBlock *block, BasicBlock *successor);
    bool BlockContainsInstruction(BasicBlock *block, Opcode opcode);
    void CheckLoopHasSafePoint(Loop *loop);
    void CheckBlockEdges(const BasicBlock &block);
    void CheckTryBeginBlock(const BasicBlock &block);
    void CheckJump(const BasicBlock &block);
    bool IsTryCatchDomination(const BasicBlock *input_block, const BasicBlock *user_block) const;
#ifndef NDEBUG
    bool NeedCheckSaveState();
#endif  // !NDEBUG
    void CheckSaveStateInputs();
    void CheckObjectRec(const Inst *object, const Inst *user, const BasicBlock *block, Inst *start_from,
                        Marker visited) const;
    void FindObjectInSaveState(const Inst *object, Inst *ss) const;
    void CheckSaveStatesWithRuntimeCallUsers();
    void CheckSaveStateOsrRec(const Inst *inst, const Inst *user, BasicBlock *block, Marker visited);

    Graph *GetGraph() const
    {
        return graph_;
    }

    ArenaAllocator *GetAllocator()
    {
        return &allocator_;
    }

    ArenaAllocator *GetLocalAllocator()
    {
        return &local_allocator_;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    /*
     * Visitors to check instructions types
     */
    static void VisitMov([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAbs([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSqrt([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAddI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSubI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMulI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitDivI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitModI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAndI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitOrI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitXorI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShlI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShrI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAShlI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMul([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMin([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMax([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAnd([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitOr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitXor([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadArray([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayPair([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayPairI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadPairPart([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayPair([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayPairI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreArray([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayI([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreStatic([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedStoreStatic([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreObject([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedStoreObject([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadStatic([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadStatic([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadClass([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadAndInitClass([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadAndInitClass([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNewObject([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitInitObject([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitInitClass([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitLoadObject([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadObject([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitConstant([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitNullPtr([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitPhi([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitParameter([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitCompare([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCmp([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMonitor([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitReturn([[maybe_unused]] GraphVisitor *v, Inst *inst);

    static void VisitReturnVoid([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNullCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitBoundsCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitRefTypeCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNegativeCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitZeroCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitDeoptimizeIf([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLenArray([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitiUnresolvedCallStatic([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitCallVirtual([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedCallVirtual([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCallDynamic([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitSaveState([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitSafePoint([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitSaveStateOsr([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitThrow([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitCheckCast([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitIsInstance([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitSelect([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitSelectImm([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitIf([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitTry([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNOP([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitAndNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitOrNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitXorNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMNeg([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMAdd([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMSub([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAddSR(GraphVisitor *v, Inst *inst);
    static void VisitSubSR(GraphVisitor *v, Inst *inst);
    static void VisitAndSR(GraphVisitor *v, Inst *inst);
    static void VisitOrSR(GraphVisitor *v, Inst *inst);
    static void VisitXorSR(GraphVisitor *v, Inst *inst);
    static void VisitAndNotSR(GraphVisitor *v, Inst *inst);
    static void VisitOrNotSR(GraphVisitor *v, Inst *inst);
    static void VisitXorNotSR(GraphVisitor *v, Inst *inst);
    static void VisitNegSR([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitCompareAnyType(GraphVisitor *v, Inst *inst);
    static void VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst);
    static void VisitAnyTypeCheck(GraphVisitor *v, Inst *inst);

    static void VisitAddOverflow([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSubOverflow([[maybe_unused]] GraphVisitor *v, Inst *inst);

#include "visitor.inc"

    static bool CheckCommonTypes(Inst *inst1, Inst *inst2)
    {
        if (inst1->GetBasicBlock()->GetGraph()->IsDynamicMethod() &&
            (inst1->GetType() == DataType::ANY || inst2->GetType() == DataType::ANY)) {
            return true;
        }
        DataType::Type type1 = inst1->GetType();
        DataType::Type type2 = inst2->GetType();
        return DataType::GetCommonType(type1) == DataType::GetCommonType(type2);
    }

    static void CheckBinaryOperationTypes(Inst *inst, bool is_int = false)
    {
        [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
        [[maybe_unused]] auto op2 = inst->GetInputs()[1].GetInst();

        if (inst->GetOpcode() == Opcode::Div || inst->GetOpcode() == Opcode::Mod) {
            if (op2->GetOpcode() == Opcode::ZeroCheck) {
                op2 = op2->GetInput(0).GetInst();
            }
        }

        if (is_int) {
            ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                      (std::cerr << "Binary instruction type is not a integer", inst->Dump(&std::cerr)));
        }

        ASSERT_DO(DataType::IsTypeNumeric(op1->GetType()),
                  (std::cerr << "Binary instruction 1st operand type is not a numeric", inst->Dump(&std::cerr)));
        ASSERT_DO(DataType::IsTypeNumeric(op2->GetType()),
                  (std::cerr << "Binary instruction 2nd operand type is not a numeric", inst->Dump(&std::cerr)));
        ASSERT_DO(DataType::IsTypeNumeric(inst->GetType()),
                  (std::cerr << "Binary instruction type is not a numeric", inst->Dump(&std::cerr)));

        ASSERT_DO(CheckCommonTypes(op1, op2), (std::cerr << "Types of binary instruction operands are not compatible\n",
                                               op1->Dump(&std::cerr), op2->Dump(&std::cerr), inst->Dump(&std::cerr)));
        ASSERT_DO(CheckCommonTypes(inst, op1),
                  (std::cerr << "Types of binary instruction result and its operands are not compatible\n",
                   inst->Dump(&std::cerr)));
    }

    static void CheckBinaryOverflowOperation(IfInst *inst)
    {
        // Overflow instruction are used only in dynamic methods.
        // But ASSERT wasn't added because the instructions checks in codegen_test.cpp with default method
        // TODO(pishin) add an ASSERT after add assembly tests tests and remove the test from codegen_test.cpp
        [[maybe_unused]] auto cc = inst->GetCc();
        ASSERT_DO((cc == CC_EQ || cc == CC_NE), (std::cerr << "overflow instruction are used only CC_EQ or CC_NE"));
        CheckBinaryOperationTypes(inst, true);
        ASSERT_DO(!DataType::IsLessInt32(inst->GetType()),
                  (std::cerr << "overflow instruction have INT32 or INT64 types"));
    }

    static void CheckBinaryOperationWithShiftedOperandTypes([[maybe_unused]] GraphVisitor *v, Inst *inst,
                                                            [[maybe_unused]] bool ror_supported)
    {
        CheckBinaryOperationTypes(inst, true);
        [[maybe_unused]] auto inst_w_shift = static_cast<BinaryShiftedRegisterOperation *>(inst);
        ASSERT_DO(inst_w_shift->GetShiftType() != ShiftType::INVALID_SHIFT &&
                      (ror_supported || inst_w_shift->GetShiftType() != ShiftType::ROR),
                  (std::cerr << "Operation has invalid shift type\n", inst->Dump(&std::cerr)));
    }

    static void CheckUnaryOperationTypes(Inst *inst)
    {
        [[maybe_unused]] auto op = inst->GetInput(0).GetInst();
        ASSERT_DO(CheckCommonTypes(inst, op),
                  (std::cerr << "Types of unary instruction result and its operand are not compatible\n",
                   inst->Dump(&std::cerr), op->Dump(&std::cerr)));
    }

    static void CheckTernaryOperationTypes(Inst *inst, bool is_int = false)
    {
        [[maybe_unused]] auto op1 = inst->GetInputs()[0].GetInst();
        [[maybe_unused]] auto op2 = inst->GetInputs()[1].GetInst();
        [[maybe_unused]] auto op3 = inst->GetInputs()[1].GetInst();

        if (is_int) {
            ASSERT_DO(DataType::GetCommonType(inst->GetType()) == DataType::INT64,
                      (std::cerr << "Ternary instruction type is not a integer", inst->Dump(&std::cerr)));
        }

        ASSERT_DO(DataType::IsTypeNumeric(op1->GetType()),
                  (std::cerr << "Ternary instruction 1st operand type is not a numeric", inst->Dump(&std::cerr)));
        ASSERT_DO(DataType::IsTypeNumeric(op2->GetType()),
                  (std::cerr << "Ternary instruction 2nd operand type is not a numeric", inst->Dump(&std::cerr)));
        ASSERT_DO(DataType::IsTypeNumeric(op3->GetType()),
                  (std::cerr << "Ternary instruction 2nd operand type is not a numeric", inst->Dump(&std::cerr)));
        ASSERT_DO(DataType::IsTypeNumeric(inst->GetType()),
                  (std::cerr << "Ternary instruction type is not a numeric", inst->Dump(&std::cerr)));

        ASSERT_DO(CheckCommonTypes(op1, op2) && CheckCommonTypes(op2, op3),
                  (std::cerr << "Types of ternary instruction operands are not compatible\n", op1->Dump(&std::cerr),
                   op2->Dump(&std::cerr), op3->Dump(&std::cerr), inst->Dump(&std::cerr)));
        ASSERT_DO(CheckCommonTypes(inst, op1),
                  (std::cerr << "Types of ternary instruction result and its operands are not compatible\n",
                   inst->Dump(&std::cerr)));
    }

    static void CheckMemoryInstruction([[maybe_unused]] Inst *inst)
    {
        ASSERT_DO(DataType::IsTypeNumeric(inst->GetType()) || inst->GetType() == DataType::REFERENCE ||
                      inst->GetType() == DataType::ANY,
                  (std::cerr << "Memory instruction has wrong type\n", inst->Dump(&std::cerr)));
    }

    static void CheckContrlFlowInst(Inst *inst)
    {
        auto block = inst->GetBasicBlock();
        [[maybe_unused]] auto last_inst = *block->AllInstsSafeReverse().begin();
        ASSERT_DO((last_inst == inst),
                  (std::cerr << "Control flow instruction must be last instruction in block\n CF instruction:\n",
                   inst->Dump(&std::cerr), std::cerr << "\n last instruction:\n", last_inst->Dump(&std::cerr)));
        ASSERT_DO(inst->GetUsers().Empty(),
                  (std::cerr << "Control flow instruction has users\n", inst->Dump(&std::cerr)));
    }

    static void CheckThrows([[maybe_unused]] Inst *inst, [[maybe_unused]] std::initializer_list<Opcode> opcs)
    {
#ifndef NDEBUG
        const auto &inputs = inst->GetInputs();
        auto ss_input = [](const auto &input) { return input.GetInst()->GetOpcode() == Opcode::SaveState; };
        bool has_save_state = std::find_if(inputs.begin(), inputs.end(), ss_input) != inputs.end();

        bool has_opc = true;
        for (auto &node : inst->GetUsers()) {
            auto opc = node.GetInst()->GetOpcode();
            has_opc &= std::find(opcs.begin(), opcs.end(), opc) != opcs.end();
        }

        ASSERT_DO(!inst->HasUsers() || has_opc,
                  (inst->Dump(&std::cerr),
                   std::cerr << "Throw inst doesn't have any users from the list:" << opcs << std::endl));
        ASSERT_DO(has_save_state, (inst->Dump(&std::cerr), std::cerr << "Throw inst without SaveState" << std::endl));
#endif
    }

    static void CheckSaveStateInput([[maybe_unused]] Inst *inst)
    {
        ASSERT_DO(inst->GetInputsCount() != 0 &&
                      inst->GetInput(inst->GetInputsCount() - 1).GetInst()->GetOpcode() == Opcode::SaveState,
                  (std::cerr << "Instruction must have SaveState as last input:\n", inst->Dump(&std::cerr)));
    }

    int IncrementNullPtrInstCounterAndGet()
    {
        return ++null_ptr_inst_counter_;
    }

private:
    Graph *graph_;
    ArenaAllocator allocator_ {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};
    ArenaAllocator local_allocator_ {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};
    int null_ptr_inst_counter_ = 0;
};
}  // namespace panda::compiler
#endif  // COMPILER_OPTIMIZER_IR_GRAPH_CHECKER_H
