/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "transforms/passes/panda_runtime_lowering.h"

#include "llvm_ark_interface.h"
#include "lowering/metadata.h"
#include "transforms/runtime_calls.h"
#include "transforms/builtins.h"
#include "transforms/transform_utils.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Pass.h>

using llvm::BasicBlock;
using llvm::CallInst;
using llvm::cast;
using llvm::Function;
using llvm::FunctionAnalysisManager;
using llvm::Instruction;
using llvm::Value;

namespace ark::llvmbackend::passes {

PandaRuntimeLowering PandaRuntimeLowering::Create(llvmbackend::LLVMArkInterface *arkInterface,
                                                  [[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return PandaRuntimeLowering(arkInterface);
}

PandaRuntimeLowering::PandaRuntimeLowering(LLVMArkInterface *arkInterface) : arkInterface_ {arkInterface} {}

llvm::PreservedAnalyses PandaRuntimeLowering::run(Function &function, FunctionAnalysisManager & /*analysisManager*/)
{
    ASSERT(arkInterface_ != nullptr);
    bool changed = false;
    for (BasicBlock &block : function) {
        llvm::SmallVector<llvm::CallInst *> calls;
        for (Instruction &inst : block) {
            if (IsRememberedCall(inst) || IsBuiltinCall(inst)) {
                calls.push_back(cast<CallInst>(&inst));
            }
        }
        for (auto call : calls) {
            if (call->getCalledFunction() == call->getCaller()) {
                // Do not touch pure recursive calls as LLVM is able to
                // make correct call by itself
                continue;
            }
            if (IsRememberedCall(*call)) {
                LowerCallStatic(call);
            } else if (IsBuiltinCall(*call)) {
                LowerBuiltin(call);
            } else {
                llvm_unreachable("Unexpected call to be lowered");
            }
            changed = true;
        }
    }
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

bool PandaRuntimeLowering::IsBuiltinCall(const llvm::Instruction &inst)
{
    if (!llvm::isa<CallInst>(inst)) {
        return false;
    }
    const auto &callInst = llvm::cast<CallInst>(inst);
    auto function = callInst.getCalledFunction();
    if (function == nullptr) {
        // callInst is a call through a function pointer
        return false;
    }
    auto prefix = function->getSectionPrefix();
    return prefix && prefix->equals(builtins::BUILTIN_SECTION);
}

void PandaRuntimeLowering::LowerCallStatic(CallInst *inst)
{
    auto epOffset = llvm::ConstantInt::get(llvm::Type::getInt64Ty(inst->getContext()),
                                           arkInterface_->GetCompiledEntryPointOffset());

    auto builder = llvm::IRBuilder<>(inst);
    auto calleePtr = GetMethodOrResolverPtr(&builder, inst);
    // Calculate address of entry point
    auto calleeAddr = builder.CreateInBoundsGEP(builder.getInt8Ty(), calleePtr, epOffset, "ep_addr");
    // Cast entry point address to a pointer to callee function pointer
    auto ftype = inst->getFunctionType();
    // Load function pointer
    ASSERT(inst->getCalledFunction() != nullptr);
    auto calleeExec = builder.CreateLoad(builder.getPtrTy(), calleeAddr, {inst->getCalledFunction()->getName(), "_p"});
    // Update call inst
    inst->setCalledFunction(ftype, calleeExec);
    inst->setArgOperand(0, calleePtr);
    if (!arkInterface_->IsArm64()) {
        inst->setCallingConv(llvm::CallingConv::ArkPlt);
    }
    inst->addFnAttr(llvm::Attribute::get(inst->getContext(), "use-ark-spills"));
}

Value *PandaRuntimeLowering::GetMethodOrResolverPtr(llvm::IRBuilder<> *builder, CallInst *inst)
{
    auto slot = arkInterface_->GetPltSlotId(inst->getFunction(), inst->getCalledFunction());
    auto block = builder->GetInsertBlock();
    auto aotGot = block->getModule()->getGlobalVariable("__aot_got");
    auto arrayType = llvm::ArrayType::get(builder->getInt64Ty(), 0);
    auto methodPtr = builder->CreateConstInBoundsGEP2_64(arrayType, aotGot, 0, slot);
    auto cachedMethodAddr = builder->CreateLoad(builder->getInt64Ty(), methodPtr);
    return builder->CreateIntToPtr(cachedMethodAddr, builder->getPtrTy(), "method_ptr");
}

void PandaRuntimeLowering::LowerBuiltin(CallInst *inst)
{
    auto builder = llvm::IRBuilder<>(inst);
    auto lowered = builtins::LowerBuiltin(&builder, inst, arkInterface_);
    if (lowered != nullptr) {
        BasicBlock::iterator ii(inst);
        ReplaceInstWithValue(inst->getParent()->getInstList(), ii, lowered);
    } else {
        ASSERT(inst->use_empty());
        ASSERT(inst->getFunctionType()->getReturnType()->isVoidTy());
        inst->eraseFromParent();
    }
}

bool PandaRuntimeLowering::IsRememberedCall(const llvm::Instruction &inst)
{
    if (!llvm::isa<CallInst>(inst)) {
        return false;
    }
    const auto &callInst = llvm::cast<CallInst>(inst);
    if (callInst.getCalledFunction() == nullptr) {
        // callInst is a call through a function pointer
        return false;
    }
    if (callInst.getCalledFunction()->isIntrinsic()) {
        return false;
    }
    return arkInterface_->IsRememberedCall(callInst.getFunction(), callInst.getCalledFunction());
}

}  // namespace ark::llvmbackend::passes
