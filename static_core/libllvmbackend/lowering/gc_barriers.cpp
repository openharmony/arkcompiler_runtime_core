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

#include "gc_barriers.h"

#include "llvm_ark_interface.h"
#include "metadata.h"

#include "compiler/optimizer/ir/basicblock.h"

#include <llvm/IR/MDBuilder.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

namespace panda::llvmbackend::gc_barriers {

void EmitPreWRB(llvm::IRBuilder<> *builder, llvm::Value *mem, bool is_volatile_mem, llvm::BasicBlock *out_bb,
                LLVMArkInterface *ark_interface, llvm::Value *thread_reg_value)
{
    auto func = builder->GetInsertBlock()->getParent();
    auto module = func->getParent();
    auto &ctx = module->getContext();
    auto initial_bb = builder->GetInsertBlock();

    auto create_uniq_basic_block_name = [&initial_bb](const std::string &suffix) {
        return panda::llvmbackend::LLVMArkInterface::GetUniqueBasicBlockName(initial_bb->getName().str(), suffix);
    };
    auto create_basic_block = [&ctx, &initial_bb, &create_uniq_basic_block_name](const std::string &suffix) {
        auto name = create_uniq_basic_block_name(suffix);
        auto func_ibb = initial_bb->getParent();
        return llvm::BasicBlock::Create(ctx, name, func_ibb);
    };

    auto load_value_bb = create_basic_block("pre_wrb_load_value");
    auto call_runtime_bb = create_basic_block("pre_wrb_call_runtime");
    auto thread_struct_ptr = builder->CreateIntToPtr(thread_reg_value, builder->getPtrTy());
    auto entrypoint_offset = ark_interface->GetTlsPreWrbEntrypointOffset();
    auto entrypoint_ptr =
        builder->CreateConstInBoundsGEP1_32(builder->getInt8Ty(), thread_struct_ptr, entrypoint_offset);

    // Check if entrypoint is null
    auto entrypoint =
        builder->CreateLoad(builder->getPtrTy(), entrypoint_ptr, "__panda_entrypoint_PreWrbFuncNoBridge_addr");
    auto has_entrypoint = builder->CreateIsNotNull(entrypoint);
    builder->CreateCondBr(has_entrypoint, load_value_bb, out_bb);

    // Load old value
    builder->SetInsertPoint(load_value_bb);

    // See LLVMEntry::EmitLoad
    auto load = builder->CreateLoad(builder->getPtrTy(LLVMArkInterface::GC_ADDR_SPACE), mem);
    if (is_volatile_mem) {
        auto alignment = module->getDataLayout().getPrefTypeAlignment(load->getType());
        load->setOrdering(LLVMArkInterface::VOLATILE_ORDER);
        load->setAlignment(llvm::Align(alignment));
    }
    auto object_is_null = builder->CreateIsNotNull(load);
    builder->CreateCondBr(object_is_null, call_runtime_bb, out_bb);

    // Call Runtime
    builder->SetInsertPoint(call_runtime_bb);
    static constexpr auto VAR_ARGS = true;
    auto function_type =
        llvm::FunctionType::get(builder->getVoidTy(), {builder->getPtrTy(LLVMArkInterface::GC_ADDR_SPACE)}, !VAR_ARGS);
    builder->CreateCall(function_type, entrypoint, {load});
    builder->CreateBr(out_bb);

    builder->SetInsertPoint(out_bb);
}

void EmitPostWRB(llvm::IRBuilder<> *builder, llvm::Value *mem, llvm::Value *offset, llvm::Value *value,
                 LLVMArkInterface *ark_interface, llvm::Value *thread_reg_value, llvm::Value *frame_reg_value)
{
    auto tls_offset = ark_interface->GetManagedThreadPostWrbOneObjectOffset();

    auto gc_ptr_ty = builder->getPtrTy(LLVMArkInterface::GC_ADDR_SPACE);
    auto ptr_ty = builder->getPtrTy();
    auto int32_ty = builder->getInt32Ty();
    auto thread_reg_ptr = builder->CreateIntToPtr(thread_reg_value, ptr_ty);
    auto addr = builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), thread_reg_ptr, tls_offset);
    auto callee = builder->CreateLoad(ptr_ty, addr, "post_wrb_one_object_addr");

    if (ark_interface->IsArm64()) {
        // Arm64 Irtoc, 4 params (add thread)
        auto func_ty = llvm::FunctionType::get(builder->getVoidTy(), {gc_ptr_ty, int32_ty, gc_ptr_ty, ptr_ty}, false);
        auto call = builder->CreateCall(func_ty, callee, {mem, offset, value, thread_reg_ptr});
        call->setCallingConv(llvm::CallingConv::ArkFast3);
        return;
    }
    // X86_64 Irtoc, 5 params (add thread, fp)
    ASSERT(frame_reg_value != nullptr);
    auto func_ty =
        llvm::FunctionType::get(builder->getVoidTy(), {gc_ptr_ty, int32_ty, gc_ptr_ty, ptr_ty, ptr_ty}, false);
    auto frame_reg_ptr = builder->CreateIntToPtr(frame_reg_value, ptr_ty);
    auto call = builder->CreateCall(func_ty, callee, {mem, offset, value, thread_reg_ptr, frame_reg_ptr});
    call->setCallingConv(llvm::CallingConv::ArkFast3);
}

}  // namespace panda::llvmbackend::gc_barriers
