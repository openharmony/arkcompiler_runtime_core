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

#ifndef LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H
#define LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H

#include <map>
#include <unordered_map>

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/Support/AtomicOrdering.h>
#include <llvm/Support/Mutex.h>

namespace llvm {
class Function;
class FunctionType;
class Instruction;
class Module;
}  // namespace llvm

namespace ark {
class Method;
}  // namespace ark

namespace ark::compiler {
class AotBuilder;
class RuntimeInterface;
class Graph;
}  // namespace ark::compiler

namespace ark::panda_file {
class File;
}  // namespace ark::panda_file

namespace ark::llvmbackend {

enum class BridgeType {
    NONE,        //
    ODD_SAVED,   //
    ENTRYPOINT,  //
    SLOW_PATH,   //
};

class LLVMArkInterface {
public:
    using MethodPtr = void *;
    using MethodId = uint32_t;
    using IntrinsicId = int;
    using EntrypointId = int;
    using RuntimeCallee = std::pair<llvm::FunctionType *, llvm::StringRef>;
    using RegMasks = std::tuple<uint32_t, uint32_t>;
    enum class RuntimeCallType { INTRINSIC, ENTRYPOINT };

    explicit LLVMArkInterface(ark::compiler::RuntimeInterface *runtime, llvm::Triple triple,
                              ark::compiler::AotBuilder *aotBuilder, llvm::sys::Mutex *lock);

    RuntimeCallee GetEntrypointCallee(EntrypointId id) const;

    void PutCalleeSavedRegistersMask(const llvm::Function *method, RegMasks masks);

    RegMasks GetCalleeSavedRegistersMask(const llvm::Function *method);

    intptr_t GetCompiledEntryPointOffset() const;

    const char *GetThreadRegister() const;
    const char *GetFramePointerRegister() const;

    uintptr_t GetEntrypointTlsOffset(EntrypointId id) const;
    size_t GetTlsFrameKindOffset() const;
    size_t GetTlsPreWrbEntrypointOffset() const;

    llvm::Function *GetFunctionByMethodPtr(MethodPtr method) const;
    void PutFunction(MethodPtr methodPtr, llvm::Function *function);

    IntrinsicId GetIntrinsicId(const llvm::Instruction *inst) const;
    llvm::Intrinsic::ID GetLLVMIntrinsicId(const llvm::Instruction *inst) const;

    const char *GetIntrinsicRuntimeFunctionName(IntrinsicId id) const;
    const char *GetEntrypointRuntimeFunctionName(EntrypointId id) const;
    BridgeType GetBridgeType(EntrypointId id) const;

    llvm::StringRef GetRuntimeFunctionName(LLVMArkInterface::RuntimeCallType callType, IntrinsicId id);
    llvm::FunctionType *GetRuntimeFunctionType(llvm::StringRef name) const;
    llvm::FunctionType *GetOrCreateRuntimeFunctionType(llvm::LLVMContext &ctx, llvm::Module *module,
                                                       LLVMArkInterface::RuntimeCallType callType, IntrinsicId id);

    void CreateRequiredIntrinsicFunctionTypes(llvm::LLVMContext &ctx);

    bool IsRememberedCall(const llvm::Function *caller, const llvm::Function *callee) const;
    void RememberFunctionOrigin(const llvm::Function *function, MethodPtr methodPtr);

    std::string GetUniqMethodName(MethodPtr methodPtr) const;
    std::string GetUniqMethodName(const Method *method) const;
    static std::string GetUniqueBasicBlockName(const std::string &bbName, const std::string &uniqueSuffix);

    MethodId GetMethodId(const llvm::Function *caller, const llvm::Function *callee) const;

    uint32_t GetClassOffset() const;
    uint32_t GetManagedThreadPostWrbOneObjectOffset() const;
    size_t GetStackOverflowCheckOffset() const;

    void RememberFunctionCall(const llvm::Function *caller, const llvm::Function *callee, MethodId methodId);

    int32_t GetPltSlotId(const llvm::Function *caller, const llvm::Function *callee) const;
    int32_t GetClassIndexInAotGot(const llvm::Function *caller, uint32_t klassId, bool initialized);
    uint64_t GetMethodStackSize(MethodPtr method) const;
    void PutMethodStackSize(const llvm::Function *method, size_t size);
    uint32_t GetVirtualRegistersCount(MethodPtr method) const;
    uint32_t GetCFrameHasFloatRegsFlagMask() const;

    bool IsIrtocMode() const;
    bool IsArm64() const
    {
        return triple_.getArch() == llvm::Triple::ArchType::aarch64;
    }

    void AppendIrtocReturnHandler(llvm::StringRef returnHandler);

    bool IsIrtocReturnHandler(const llvm::Function &function) const;

    int32_t CreateIntfInlineCacheSlotId(const llvm::Function *caller) const;

public:
    static constexpr auto NO_INTRINSIC_ID = static_cast<IntrinsicId>(-1);
    static constexpr auto GC_ADDR_SPACE = 271;
    static constexpr auto VOLATILE_ORDER = llvm::AtomicOrdering::SequentiallyConsistent;
    static constexpr auto NOT_ATOMIC_ORDER = llvm::AtomicOrdering::NotAtomic;
    static constexpr std::string_view GC_SAFEPOINT_POLL_NAME = "gc.safepoint_poll";
    static constexpr std::string_view GC_STRATEGY = "ark";
    static constexpr std::string_view FUNCTION_MD_CLASS_ID = "class_id";
    static constexpr std::string_view FUNCTION_MD_INLINE_MODULE = "inline_module";
    static constexpr std::string_view PATCH_STACK_ADJUSTMENT_COMMENT = " ${:comment} patch-stack-adjustment";

    ark::compiler::RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }

    bool DeoptsEnabled();

private:
    IntrinsicId GetPluginIntrinsicId(const llvm::Instruction *instruction) const;
    IntrinsicId GetIntrinsicIdSwitch(const llvm::IntrinsicInst *inst) const;
    IntrinsicId GetIntrinsicIdMemory(const llvm::IntrinsicInst *inst) const;
    IntrinsicId GetIntrinsicIdMath(const llvm::IntrinsicInst *inst) const;
#include "get_intrinsic_id_llvm_ark_interface_gen.h.inl"
private:
    static constexpr auto NO_INTRINSIC_ID_CONTINUE = static_cast<IntrinsicId>(-2);

    ark::compiler::RuntimeInterface *runtime_;
    llvm::Triple triple_;
    llvm::StringMap<llvm::FunctionType *> runtimeFunctionTypes_;
    llvm::ValueMap<const llvm::Function *, const panda_file::File *> functionOrigins_;
    llvm::DenseMap<std::pair<const llvm::Function *, const panda_file::File *>, MethodId> methodIds_;
    llvm::DenseMap<MethodPtr, llvm::Function *> functions_;
    llvm::DenseMap<llvm::Function *, uint8_t> sourceLanguages_;
    llvm::StringMap<uint64_t> llvmStackSizes_;
    llvm::DenseMap<const llvm::Function *, RegMasks> calleeSavedRegisters_;
    ark::compiler::AotBuilder *aotBuilder_;
    std::vector<llvm::StringRef> irtocReturnHandlers_;
    mutable llvm::sys::Mutex *lock_;
};
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H
