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

#ifndef LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H
#define LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H

#include <map>
#include <unordered_map>

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/Support/AtomicOrdering.h>

#include "llvm/IR/Instructions.h"

namespace llvm {
class Function;
class FunctionType;
class Instruction;
class Module;
}  // namespace llvm

namespace panda {
class Method;
}  // namespace panda

namespace panda::compiler {
class RuntimeInterface;
class Graph;
}  // namespace panda::compiler

namespace panda::panda_file {
class File;
}  // namespace panda::panda_file

namespace panda::llvmbackend {

class LLVMArkInterface {
public:
    using MethodPtr = void *;
    using MethodId = uint32_t;
    using IntrinsicId = int;
    using EntrypointId = int;
    using RuntimeCallee = std::pair<llvm::FunctionType *, llvm::StringRef>;
    using RegMasks = std::tuple<uint32_t, uint32_t>;
    enum class RuntimeCallType { INTRINSIC, ENTRYPOINT };

    explicit LLVMArkInterface(panda::compiler::RuntimeInterface *runtime, llvm::Triple triple);

    RuntimeCallee GetEntrypointCallee(EntrypointId id) const;

    const char *GetThreadRegister() const;

    const char *GetFramePointerRegister() const;

    uintptr_t GetEntrypointTlsOffset(EntrypointId id) const;

    size_t GetTlsPreWrbEntrypointOffset() const;

    llvm::Function *GetFunctionByMethodPtr(MethodPtr method) const;

    void PutFunction(MethodPtr method_ptr, llvm::Function *function);

    const char *GetIntrinsicRuntimeFunctionName(IntrinsicId id) const;

    const char *GetEntrypointRuntimeFunctionName(EntrypointId id) const;

    llvm::StringRef GetRuntimeFunctionName(LLVMArkInterface::RuntimeCallType call_type, IntrinsicId id);
    llvm::FunctionType *GetRuntimeFunctionType(llvm::StringRef name) const;
    llvm::FunctionType *GetOrCreateRuntimeFunctionType(llvm::LLVMContext &ctx, llvm::Module *module,
                                                       LLVMArkInterface::RuntimeCallType call_type, IntrinsicId id);

    void RememberFunctionOrigin(const llvm::Function *function, MethodPtr method_ptr);

    std::string GetUniqMethodName(MethodPtr method_ptr) const;

    std::string GetUniqMethodName(const Method *method) const;

    static std::string GetUniqueBasicBlockName(const std::string &bb_name, const std::string &unique_suffix);

    uint32_t GetManagedThreadPostWrbOneObjectOffset() const;

    bool IsIrtocMode() const;

    bool IsArm64() const
    {
        return triple_.getArch() == llvm::Triple::ArchType::aarch64;
    }

    void AppendIrtocReturnHandler(llvm::StringRef return_handler);

    bool IsIrtocReturnHandler(const llvm::Function &function) const;

public:
    static constexpr auto NO_INTRINSIC_ID = static_cast<IntrinsicId>(-1);
    static constexpr auto GC_ADDR_SPACE = 271;

    static constexpr auto VOLATILE_ORDER = llvm::AtomicOrdering::SequentiallyConsistent;
    static constexpr auto NOT_ATOMIC_ORDER = llvm::AtomicOrdering::NotAtomic;

    static constexpr std::string_view FUNCTION_MD_CLASS_ID = "class_id";
    static constexpr std::string_view PATCH_STACK_ADJUSTMENT_COMMENT = " ${:comment} patch-stack-adjustment";

    panda::compiler::RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }

private:
    static constexpr auto NO_INTRINSIC_ID_CONTINUE = static_cast<IntrinsicId>(-2);

    panda::compiler::RuntimeInterface *runtime_;
    llvm::Triple triple_;
    llvm::StringMap<llvm::FunctionType *> runtime_function_types_;

    llvm::ValueMap<const llvm::Function *, const panda_file::File *> function_origins_;
    llvm::DenseMap<MethodPtr, llvm::Function *> functions_;
    llvm::DenseMap<llvm::Function *, uint8_t> source_languages_;
    std::vector<llvm::StringRef> irtoc_return_handlers_;
};
}  // namespace panda::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H
