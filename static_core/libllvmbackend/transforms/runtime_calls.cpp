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

#include "runtime_calls.h"
#include "llvm_ark_interface.h"

#include <llvm/IR/IRBuilder.h>

namespace panda::llvmbackend::runtime_calls {

llvm::Value *GetAddressToTLS(llvm::IRBuilder<> *builder, LLVMArkInterface *ark_interface, uintptr_t tls_offset)
{
    auto thread_reg_value = GetThreadRegValue(builder, ark_interface);
    auto thread_reg_ptr = builder->CreateIntToPtr(thread_reg_value, builder->getPtrTy());
    return builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), thread_reg_ptr, tls_offset);
}

llvm::Value *LoadTLSValue(llvm::IRBuilder<> *builder, LLVMArkInterface *ark_interface, uintptr_t tls_offset,
                          llvm::Type *type)
{
    auto addr = GetAddressToTLS(builder, ark_interface, tls_offset);
    return builder->CreateLoad(type, addr);
}

llvm::CallInst *CreateEntrypointCallCommon(llvm::IRBuilder<> *builder, llvm::Value *thread_reg_value,
                                           LLVMArkInterface *ark_interface, EntrypointId eid,
                                           llvm::ArrayRef<llvm::Value *> arguments)
{
    auto tls_offset = ark_interface->GetEntrypointTlsOffset(eid);
    auto [function_proto, function_name] = ark_interface->GetEntrypointCallee(eid);

    auto thread_reg_ptr = builder->CreateIntToPtr(thread_reg_value, builder->getPtrTy());
    auto addr = builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), thread_reg_ptr, tls_offset);
    auto callee = builder->CreateLoad(builder->getPtrTy(), addr, function_name + "_addr");

    auto callee_func_ty = llvm::cast<llvm::FunctionType>(function_proto);
    auto call = builder->CreateCall(callee_func_ty, callee, arguments);
    return call;
}

llvm::Value *GetThreadRegValue(llvm::IRBuilder<> *builder, LLVMArkInterface *ark_interface)
{
    assert(!ark_interface->IsIrtocMode());
    auto func = builder->GetInsertBlock()->getParent();
    auto &ctx = func->getContext();
    auto reg_md = llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, ark_interface->GetThreadRegister())});
    auto thread_reg = llvm::MetadataAsValue::get(ctx, reg_md);
    auto read_reg =
        llvm::Intrinsic::getDeclaration(func->getParent(), llvm::Intrinsic::read_register, builder->getInt64Ty());
    return builder->CreateCall(read_reg, {thread_reg});
}

llvm::Value *GetRealFrameRegValue(llvm::IRBuilder<> *builder, LLVMArkInterface *ark_interface)
{
    assert(!ark_interface->IsIrtocMode());
    auto func = builder->GetInsertBlock()->getParent();
    auto &ctx = func->getContext();
    auto reg_md = llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, ark_interface->GetFramePointerRegister())});
    auto frame_reg = llvm::MetadataAsValue::get(ctx, reg_md);
    auto read_reg =
        llvm::Intrinsic::getDeclaration(func->getParent(), llvm::Intrinsic::read_register, builder->getInt64Ty());
    return builder->CreateCall(read_reg, {frame_reg});
}

}  // namespace panda::llvmbackend::runtime_calls
