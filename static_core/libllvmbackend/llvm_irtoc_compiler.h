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

#ifndef LIBLLVMBACKEND_LLVM_IRTOC_COMPILER_H
#define LIBLLVMBACKEND_LLVM_IRTOC_COMPILER_H

#include "compiler/code_info/code_info_builder.h"
#include "compiler/optimizer/ir/runtime_interface.h"

#include "llvm_ark_interface.h"
#include "llvm_compiler.h"
#include "llvm_compiler_options.h"
#include "lowering/debug_data_builder.h"
#include "object_code/created_object_file.h"

#include <llvm/Support/Error.h>

namespace panda::compiler {
class CompiledMethod;
class Graph;
}  // namespace panda::compiler

namespace panda::llvmbackend {

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LLVMIrtocCompiler final : public LLVMCompiler, public IrtocCompilerInterface {
public:
    Expected<bool, std::string> CanCompile(panda::compiler::Graph *graph) override;

    explicit LLVMIrtocCompiler(panda::compiler::RuntimeInterface *runtime, panda::ArenaAllocator *allocator,
                               panda::Arch arch, std::string filename);

    bool AddGraph(panda::compiler::Graph *graph) override;

    void CompileAll() override;

    bool IsEmpty() override
    {
        return methods_.empty();
    }

    bool HasCompiledCode() override
    {
        return object_file_ != nullptr;
    }

    bool IsIrFailed() override
    {
        return ir_failed_;
    }
    void WriteObjectFile(std::string_view output) override;

    CompiledCode GetCompiledCode(std::string_view function_name) override;

private:
    static std::vector<std::string> GetFeaturesForArch(Arch arch);

    void InitializeSpecificLLVMOptions(Arch arch);

    void InitializeModule();

    size_t GetObjectFileSize() override;

private:
    llvm::ExitOnError exit_on_err_;

    ArenaVector<panda::Method *> methods_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<DebugDataBuilder> debug_data_;
    std::unique_ptr<panda::llvmbackend::CreatedObjectFile> object_file_ {nullptr};
    std::string filename_;

    LLVMArkInterface ark_interface_;
    bool ir_failed_ {false};
    std::unique_ptr<panda::llvmbackend::MIRCompiler> mir_compiler_;
    std::unique_ptr<panda::llvmbackend::LLVMOptimizer> optimizer_;
};
}  // namespace panda::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_IRTOC_COMPILER_H
