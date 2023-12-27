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

#ifndef LIBLLVMBACKEND_LOWERING_LLVM_IR_CONSTRUCTOR_H
#define LIBLLVMBACKEND_LOWERING_LLVM_IR_CONSTRUCTOR_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/inst.h"

#include "debug_data_builder.h"
#include "llvm_ark_interface.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>

namespace panda::compiler {

class LLVMIrConstructor : public GraphVisitor {
    using RuntimeCallId = std::variant<RuntimeInterface::EntrypointId, RuntimeInterface::IntrinsicId>;

    bool TryEmitIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId arkId);

private:
    // Specific intrinsic Emitters
    bool EmitUnreachable();
    bool EmitSlowPathEntry(Inst *inst);
    bool EmitExclusiveLoadWithAcquire(Inst *inst);
    bool EmitExclusiveStoreWithRelease(Inst *inst);
    bool EmitInterpreterReturn();
    bool EmitTailCall(Inst *inst);
    bool EmitMemoryFence(memory_order::Order order);
    template <uint32_t VECTOR_SIZE>
    bool EmitCompressUtf16ToUtf8CharsUsingSimd(Inst *inst);

public:
    llvm::Value *GetMappedValue(Inst *inst, DataType::Type type);
    llvm::Value *GetInputValue(Inst *inst, size_t index, bool skipCoerce = false);
    llvm::Value *GetInputValueFromConstant(ConstantInst *constant, DataType::Type pandaType);

    Graph *GetGraph() const
    {
        return graph_;
    }

    llvm::Function *GetFunc()
    {
        return func_;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return graph_->GetBlocksRPO();
    }

private:
    // Initializers. BuildIr calls them
    void BuildBasicBlocks();
    void BuildInstructions();
    void FillPhiInputs();

    // Creator functions for internal usage

    void CreateInterpreterReturnRestoreRegs(RegMask &regMask, size_t offset, bool fp);
    llvm::Value *CreateBinaryOp(Inst *inst, llvm::Instruction::BinaryOps opcode);
    llvm::Value *CreateBinaryImmOp(Inst *inst, llvm::Instruction::BinaryOps opcode, uint64_t c);
    llvm::Value *CreateShiftOp(Inst *inst, llvm::Instruction::BinaryOps opcode);
    llvm::Value *CreateSignDivMod(Inst *inst, llvm::Instruction::BinaryOps opcode);
    llvm::Value *CreateAArch64SignDivMod(Inst *inst, llvm::Instruction::BinaryOps opcode, llvm::Value *x,
                                         llvm::Value *y);
    llvm::Value *CreateFloatComparison(CmpInst *cmpInst, llvm::Value *x, llvm::Value *y);
    llvm::Value *CreateIntegerComparison(CmpInst *inst, llvm::Value *x, llvm::Value *y);
    llvm::Value *CreateCastToInt(Inst *inst);
    llvm::Value *CreateLoadWithOrdering(Inst *inst, llvm::Value *value, llvm::AtomicOrdering ordering,
                                        const llvm::Twine &name = "");
    llvm::Value *CreateStoreWithOrdering(llvm::Value *value, llvm::Value *ptr, llvm::AtomicOrdering ordering);
    void CreatePreWRB(Inst *inst, llvm::Value *mem);
    void CreatePostWRB(Inst *inst, llvm::Value *mem, llvm::Value *offset, llvm::Value *value);
    llvm::Value *CreateMemoryFence(memory_order::Order order);
    llvm::Value *CreateCondition(ConditionCode cc, llvm::Value *x, llvm::Value *y);
    void CreateIf(Inst *inst, llvm::Value *cond, bool likely, bool unlikely);
    llvm::CallInst *CreateTailCallFastPath(Inst *inst);
    llvm::CallInst *CreateTailCallInterpreter(Inst *inst);

    // Getters
    llvm::FunctionType *GetEntryFunctionType();

    llvm::StringRef GetRuntimeFunctionName(RuntimeCallId id);

    llvm::Value *GetThreadRegValue();
    llvm::Value *GetRealFrameRegValue();

    llvm::Argument *GetArgument(size_t index)
    {
        ASSERT(func_ != nullptr);
        auto offset = 0;
        return func_->arg_begin() + offset + index;
    }

    llvm::Type *GetType(DataType::Type pandaType);
    llvm::Type *GetExactType(DataType::Type targetType);

    llvm::Instruction::CastOps GetCastOp(DataType::Type from, DataType::Type to);

    llvm::Value *CoerceValue(llvm::Value *value, DataType::Type sourceType, DataType::Type targetType);
    llvm::Value *CoerceValue(llvm::Value *value, llvm::Type *targetType);

    void ValueMapAdd(Inst *inst, llvm::Value *value, bool setName = true);
    void FillValueMapForUsers(Inst *inst, llvm::Value *value, DataType::Type type,
                              ArenaUnorderedMap<DataType::Type, llvm::Value *> *typeMap);

    void AddBlock(BasicBlock *pb, llvm::BasicBlock *lb)
    {
        ASSERT(blockTailMap_.count(pb) == 0);
        blockTailMap_.insert({pb, lb});
        blockHeadMap_.insert({pb, lb});
    }

    void SetCurrentBasicBlock(llvm::BasicBlock *block)
    {
        builder_.SetInsertPoint(block);
    }

    llvm::BasicBlock *GetCurrentBasicBlock()
    {
        return builder_.GetInsertBlock();
    }

    void ReplaceTailBlock(BasicBlock *pandaBlock, llvm::BasicBlock *llvmBlock)
    {
        auto it = blockTailMap_.find(pandaBlock);
        ASSERT(it != blockTailMap_.end());
        it->second = llvmBlock;
    }

    llvm::BasicBlock *GetHeadBlock(BasicBlock *block)
    {
        ASSERT(blockHeadMap_.count(block) == 1);
        auto result = blockHeadMap_.at(block);
        ASSERT(result != nullptr);
        return result;
    }

    llvm::BasicBlock *GetTailBlock(BasicBlock *block)
    {
        ASSERT(blockTailMap_.count(block) == 1);
        auto result = blockTailMap_.at(block);
        ASSERT(result != nullptr);
        return result;
    }

protected:
    // Instruction Visitors

    static void VisitConstant(GraphVisitor *v, Inst *inst);
    static void VisitNullPtr(GraphVisitor *v, Inst *inst);
    static void VisitLiveIn(GraphVisitor *v, Inst *inst);
    static void VisitParameter(GraphVisitor *v, Inst *inst);
    static void VisitReturnVoid(GraphVisitor *v, Inst *inst);
    static void VisitReturn(GraphVisitor *v, Inst *inst);
    static void VisitLiveOut(GraphVisitor *v, Inst *inst);
    static void VisitLoad(GraphVisitor *v, Inst *inst);
    static void VisitStore(GraphVisitor *v, Inst *inst);
    static void VisitLoadI(GraphVisitor *v, Inst *inst);
    static void VisitStoreI(GraphVisitor *v, Inst *inst);
    static void VisitBitcast(GraphVisitor *v, Inst *inst);
    static void VisitCast(GraphVisitor *v, Inst *inst);
    static void VisitAnd(GraphVisitor *v, Inst *inst);
    static void VisitAndI(GraphVisitor *v, Inst *inst);
    static void VisitOr(GraphVisitor *v, Inst *inst);
    static void VisitOrI(GraphVisitor *v, Inst *inst);
    static void VisitXor(GraphVisitor *v, Inst *inst);
    static void VisitXorI(GraphVisitor *v, Inst *inst);
    static void VisitShl(GraphVisitor *v, Inst *inst);
    static void VisitShlI(GraphVisitor *v, Inst *inst);
    static void VisitShr(GraphVisitor *v, Inst *inst);
    static void VisitShrI(GraphVisitor *v, Inst *inst);
    static void VisitAShr(GraphVisitor *v, Inst *inst);
    static void VisitAShrI(GraphVisitor *v, Inst *inst);
    static void VisitAdd(GraphVisitor *v, Inst *inst);
    static void VisitAddI(GraphVisitor *v, Inst *inst);
    static void VisitSub(GraphVisitor *v, Inst *inst);
    static void VisitSubI(GraphVisitor *v, Inst *inst);
    static void VisitMul(GraphVisitor *v, Inst *inst);
    static void VisitMulI(GraphVisitor *v, Inst *inst);
    static void VisitDiv(GraphVisitor *v, Inst *inst);
    static void VisitMod(GraphVisitor *v, Inst *inst);
    static void VisitCompare(GraphVisitor *v, Inst *inst);
    static void VisitCmp(GraphVisitor *v, Inst *inst);
    static void VisitNeg(GraphVisitor *v, Inst *inst);
    static void VisitNot(GraphVisitor *v, Inst *inst);
    static void VisitIfImm(GraphVisitor *v, Inst *inst);
    static void VisitIf(GraphVisitor *v, Inst *inst);
    static void VisitCallIndirect(GraphVisitor *v, Inst *inst);
    static void VisitCall(GraphVisitor *v, Inst *inst);
    static void VisitPhi(GraphVisitor *v, Inst *inst);
    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);

    void VisitDefault(Inst *inst) override;

public:
    explicit LLVMIrConstructor(Graph *graph, llvm::Module *module, llvm::LLVMContext *context,
                               llvmbackend::LLVMArkInterface *arkInterface,
                               const std::unique_ptr<llvmbackend::DebugDataBuilder> &debugData);

    bool BuildIr();

    static Expected<bool, std::string> CanCompile(Graph *graph);
    static bool CanCompile(Inst *inst);

#include "optimizer/ir/visitor.inc"

private:
    Graph *graph_ {nullptr};
    llvm::Function *func_;
    llvm::IRBuilder<> builder_;
    ArenaDoubleUnorderedMap<Inst *, DataType::Type, llvm::Value *> inputMap_;
    ArenaUnorderedMap<BasicBlock *, llvm::BasicBlock *> blockTailMap_;
    ArenaUnorderedMap<BasicBlock *, llvm::BasicBlock *> blockHeadMap_;
    llvmbackend::LLVMArkInterface *arkInterface_;
    const std::unique_ptr<llvmbackend::DebugDataBuilder> &debugData_;
    ArenaVector<uint8_t> cc_;
    ArenaVector<llvm::Value *> ccValues_;
};

}  // namespace panda::compiler

#endif  // LIBLLVMBACKEND_LOWERING_LLVM_IR_CONSTRUCTOR_H
