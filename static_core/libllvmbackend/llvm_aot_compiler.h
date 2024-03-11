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

#ifndef LIBLLVMBACKEND_LLVM_AOT_COMPILER_H
#define LIBLLVMBACKEND_LLVM_AOT_COMPILER_H

#include "compiler/code_info/code_info_builder.h"
#include "compiler/optimizer/ir/runtime_interface.h"

#include "llvm_compiler.h"
#include "lowering/debug_data_builder.h"
#include "lowering/wrapped_module.h"
#include "object_code/ark_aot_linker.h"
#include "object_code/code_info_producer.h"

#include <llvm/Object/StackMapParser.h>
#include <llvm/Support/Mutex.h>

namespace ark::compiler {
class LLVMAotBuilder;
class CompiledMethod;
class Graph;
}  // namespace ark::compiler

namespace ark::llvmbackend {

/// Spreads methods to compile between modules
class Spreader {
public:
    Spreader() = default;
    virtual std::shared_ptr<WrappedModule> GetModuleForMethod(compiler::RuntimeInterface::MethodPtr method) = 0;

    /// Returns all modules, that this spreader has created
    virtual std::unordered_set<std::shared_ptr<WrappedModule>> GetModules() = 0;

    virtual ~Spreader() = default;

    NO_COPY_SEMANTIC(Spreader);
    NO_MOVE_SEMANTIC(Spreader);
};

class AotBuilderOffsets {
public:
    AotBuilderOffsets(std::unordered_map<std::string, size_t> sectionAddresses,
                      std::unordered_map<std::string, size_t> methodOffsets)
        : sectionAddresses_(std::move(sectionAddresses)), methodOffsets_(std::move(methodOffsets))
    {
    }

    const std::unordered_map<std::string, size_t> &GetSectionAddresses() const
    {
        return sectionAddresses_;
    }

    const std::unordered_map<std::string, size_t> &GetMethodOffsets() const
    {
        return methodOffsets_;
    }

private:
    std::unordered_map<std::string, size_t> sectionAddresses_;
    std::unordered_map<std::string, size_t> methodOffsets_;
};

class LLVMAotCompiler final : public LLVMCompiler {
public:
    Expected<bool, std::string> CanCompile(ark::compiler::Graph *graph) override;

    explicit LLVMAotCompiler(ark::compiler::RuntimeInterface *runtime, ark::ArenaAllocator *allocator,
                             ark::compiler::LLVMAotBuilder *aotBuilder, std::string cmdline, std::string filename);

    bool AddGraph(ark::compiler::Graph *graph) override;

    void CompileAll() override;

    bool HasCompiledCode() override
    {
        return compiled_;
    }

    bool IsIrFailed() override
    {
        return irFailed_;
    }

private:
    enum class AddGraphMode {
        // Add graph to allow inlining only, the function created from this graph is discarded later
        INLINE_FUNCTION,
        // Add graph in a regular way, do not discard function created for it later
        PRIMARY_FUNCTION,
    };

    static std::vector<std::string> GetFeaturesForArch(Arch arch);

    void RunArkPasses(ark::compiler::Graph *graph);

    bool AddGraphToModule(ark::compiler::Graph *graph, WrappedModule &module, AddGraphMode addGraphMode);

    ark::compiler::CompiledMethod AdaptCode(ark::Method *method, Span<const uint8_t> machineCode);

    void PrepareAotGot(WrappedModule *wrappedModule);

    WrappedModule CreateModule(uint32_t moduleId);

    AotBuilderOffsets CollectAotBuilderOffsets(const std::unordered_set<std::shared_ptr<WrappedModule>> &modules);

    void DumpCodeInfo(compiler::CompiledMethod &method) const;

    void CompileModule(WrappedModule &module);

    ArkAotLinker::RoDataSections LinkModule(WrappedModule *wrappedModule, ArkAotLinker *linker,
                                            AotBuilderOffsets *offsets);

    void AddInlineMethodByDepth(WrappedModule &module, ark::compiler::Graph *caller,
                                compiler::RuntimeInterface::MethodPtr method, int32_t depth);

    void AddInlineFunctionsByDepth(WrappedModule &module, ark::compiler::Graph *caller, int32_t depth);

    llvm::Expected<ark::compiler::Graph *> CreateGraph(ArenaAllocator &allocator, ArenaAllocator &localAllocator,
                                                       Method &method);

private:
    llvm::ExitOnError exitOnErr_;

    ArenaVector<ark::Method *> methods_;
    ark::compiler::LLVMAotBuilder *aotBuilder_;
    std::string cmdline_;
    std::string filename_;

    bool compiled_ {false};
    bool irFailed_ {false};

    ark::compiler::RuntimeInterface *runtime_;
    std::unique_ptr<Spreader> spreader_;
    std::atomic<uint32_t> compiledModules_ {0};
    llvm::sys::Mutex lock_ {};
};
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_AOT_COMPILER_H
