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

llvm::Value *GetAddressToTLS(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface, uintptr_t tlsOffset)
{
    auto threadRegValue = GetThreadRegValue(builder, arkInterface);
    auto threadRegPtr = builder->CreateIntToPtr(threadRegValue, builder->getPtrTy());
    return builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), threadRegPtr, tlsOffset);
}

llvm::Value *LoadTLSValue(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface, uintptr_t tlsOffset,
                          llvm::Type *type)
{
    auto addr = GetAddressToTLS(builder, arkInterface, tlsOffset);
    return builder->CreateLoad(type, addr);
}

llvm::CallInst *CreateEntrypointCallCommon(llvm::IRBuilder<> *builder, llvm::Value *threadRegValue,
                                           LLVMArkInterface *arkInterface, EntrypointId eid,
                                           llvm::ArrayRef<llvm::Value *> arguments)
{
    auto tlsOffset = arkInterface->GetEntrypointTlsOffset(eid);
    auto [function_proto, function_name] = arkInterface->GetEntrypointCallee(eid);

    auto threadRegPtr = builder->CreateIntToPtr(threadRegValue, builder->getPtrTy());
    auto addr = builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), threadRegPtr, tlsOffset);
    auto callee = builder->CreateLoad(builder->getPtrTy(), addr, function_name + "_addr");

    auto calleeFuncTy = llvm::cast<llvm::FunctionType>(function_proto);
    auto call = builder->CreateCall(calleeFuncTy, callee, arguments);
    return call;
}

llvm::Value *GetThreadRegValue(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface)
{
    ASSERT(!arkInterface->IsIrtocMode());
    auto func = builder->GetInsertBlock()->getParent();
    auto &ctx = func->getContext();
    auto regMd = llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, arkInterface->GetThreadRegister())});
    auto threadReg = llvm::MetadataAsValue::get(ctx, regMd);
    auto readReg =
        llvm::Intrinsic::getDeclaration(func->getParent(), llvm::Intrinsic::read_register, builder->getInt64Ty());
    return builder->CreateCall(readReg, {threadReg});
}

llvm::Value *GetRealFrameRegValue(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface)
{
    ASSERT(!arkInterface->IsIrtocMode());
    auto func = builder->GetInsertBlock()->getParent();
    auto &ctx = func->getContext();
    auto regMd = llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, arkInterface->GetFramePointerRegister())});
    auto frameReg = llvm::MetadataAsValue::get(ctx, regMd);
    auto readReg =
        llvm::Intrinsic::getDeclaration(func->getParent(), llvm::Intrinsic::read_register, builder->getInt64Ty());
    return builder->CreateCall(readReg, {frameReg});
}

}  // namespace panda::llvmbackend::runtime_calls
