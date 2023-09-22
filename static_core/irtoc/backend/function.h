/*
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

#ifndef PANDA_FUNCTION_H
#define PANDA_FUNCTION_H

#include "compiler/optimizer/ir/ir_constructor.h"
#include "compiler/optimizer/code_generator/relocations.h"
#include "utils/expected.h"
#include "source_languages.h"
#include "irtoc_options.h"

#ifdef PANDA_LLVMAOT
#include "llvm_compiler_creator.h"
#include "llvmaot_options.h"
#endif

namespace panda::irtoc {

enum class LLVMCompilationResult {
    INVALID,
    LLVM_COMPILED,
    LLVM_CAN_NOT_COMPILE,
    USE_ARK_AS_SKIP_LIST,
    USE_ARK_AS_NO_SUFFIX,
};

#if USE_VIXL_ARM64 && !PANDA_MINIMAL_VIXL && PANDA_LLVMAOT
struct UsedRegisters {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    RegMask gpr;
    VRegMask fp;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    constexpr UsedRegisters &operator|=(UsedRegisters that)
    {
        gpr |= that.gpr;
        fp |= that.fp;
        return *this;
    }
};
#endif

class Function : public compiler::RelocationHandler {
public:
    using Result = Expected<int, const char *>;

#ifdef PANDA_LLVMAOT
    Function() : llvm_aot_compiler_(nullptr) {}
#else
    Function() = default;
#endif
    NO_COPY_SEMANTIC(Function);
    NO_MOVE_SEMANTIC(Function);

    virtual ~Function() = default;

    virtual void MakeGraphImpl() = 0;
    virtual const char *GetName() const = 0;

    Result Compile(Arch arch, ArenaAllocator *allocator, ArenaAllocator *local_allocator);

    LLVMCompilationResult CompileByLLVM(panda::compiler::RuntimeInterface *runtime, ArenaAllocator *allocator);

    auto GetCode() const
    {
        return Span(code_);
    }

    compiler::Graph *GetGraph()
    {
        return graph_;
    }

    const compiler::Graph *GetGraph() const
    {
        return graph_;
    }

    size_t WordSize() const
    {
        return PointerSize(GetArch());
    }

    void AddRelocation(const compiler::RelocationInfo &info) override;

    const auto &GetRelocations() const
    {
        return relocation_entries_;
    }

    const char *GetExternalFunction(size_t index) const
    {
        CHECK_LT(index, external_functions_.size());
        return external_functions_[index].c_str();
    }

    SourceLanguage GetLanguage() const
    {
        return lang_;
    }

    uint32_t AddSourceDir(std::string_view dir)
    {
        auto it = std::find(source_dirs_.begin(), source_dirs_.end(), dir);
        if (it != source_dirs_.end()) {
            return std::distance(source_dirs_.begin(), it);
        }
        source_dirs_.emplace_back(dir);
        return source_dirs_.size() - 1;
    }

    uint32_t AddSourceFile(std::string_view filename)
    {
        auto it = std::find(source_files_.begin(), source_files_.end(), filename);
        if (it != source_files_.end()) {
            return std::distance(source_files_.begin(), it);
        }
        source_files_.emplace_back(filename);
        return source_files_.size() - 1;
    }

    uint32_t GetSourceFileIndex(const char *filename) const
    {
        auto it = std::find(source_files_.begin(), source_files_.end(), filename);
        ASSERT(it != source_files_.end());
        return std::distance(source_files_.begin(), it);
    }

    uint32_t GetSourceDirIndex(const char *dir) const
    {
        auto it = std::find(source_dirs_.begin(), source_dirs_.end(), dir);
        ASSERT(it != source_dirs_.end());
        return std::distance(source_dirs_.begin(), it);
    }

    Span<const std::string> GetSourceFiles() const
    {
        return Span<const std::string>(source_files_);
    }

    Span<const std::string> GetSourceDirs() const
    {
        return Span<const std::string>(source_dirs_);
    }

    void SetArgsCount(size_t args_count)
    {
        args_count_ = args_count;
    }

    size_t GetArgsCount() const
    {
        return GetArch() != Arch::AARCH32 ? args_count_ : 0U;
    }

#ifdef PANDA_LLVMAOT
    void ReportCompilationStatistic(std::ostream *out);

    compiler::GraphMode GetGraphMode()
    {
        ASSERT_PRINT(GetGraph() != nullptr, "Must invoke 'Compile' method before");
        return graph_mode_;
    }

    void SetLLVMCompiler(std::shared_ptr<llvmaot::AotCompilerInterface> compiler)
    {
        llvm_aot_compiler_ = std::move(compiler);
    }

#endif  // PANDA_LLVMAOT

    bool IsCompiledByLlvm() const
    {
        return llvm_compilation_result_ == LLVMCompilationResult::LLVM_COMPILED;
    }

    LLVMCompilationResult GetLLVMCompilationResult()
    {
        return llvm_compilation_result_;
    }

    size_t GetCodeSize()
    {
        return code_.size();
    }

    void SetCode(Span<uint8_t> code);

    Result RunOptimizations(compiler::RuntimeInterface *runtime, ArenaAllocator *allocator,
                            LLVMCompilationResult &compilation_result);

protected:
    Arch GetArch() const
    {
        return GetGraph()->GetArch();
    }

    compiler::RuntimeInterface *GetRuntime()
    {
        return GetGraph()->GetRuntime();
    }

    void SetExternalFunctions(std::initializer_list<std::string> funcs)
    {
        external_functions_ = funcs;
    }

    void SetLanguage(SourceLanguage lang)
    {
        lang_ = lang;
    }

#ifdef PANDA_LLVMAOT
    bool SkippedByLLVM();

    std::string_view GraphModeToString();

    std::string_view LLVMCompilationResultToString() const;
#endif  // PANDA_LLVMAOT

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::unique_ptr<compiler::IrConstructor> builder_;

private:
    /**
     * Suffix for compilation unit names to compile by llvm.
     *
     * --interpreter-type option in Ark supports interpreters produced by stock irtoc and llvm irtoc.
     * To support interpreter compiled by stock irtoc and llvm irtoc we:
     * 1. Copy a graph for a handler with "_LLVM" suffix. For example, "ClassResolver" becomes "ClassResolver_LLVM"
     * 2. Compile handlers with suffix by LLVMIrtocCompiler
     * 3. Setup dispatch table to invoke handlers with "_LLVM" suffix at runtime in SetupLLVMDispatchTableImpl
     */
    static constexpr std::string_view LLVM_SUFFIX = "_LLVM";

    compiler::Graph *graph_ {nullptr};
    SourceLanguage lang_ {SourceLanguage::PANDA_ASSEMBLY};
    std::vector<uint8_t> code_;
    std::vector<std::string> external_functions_;
    std::vector<std::string> source_dirs_;
    std::vector<std::string> source_files_;
    std::vector<compiler::RelocationInfo> relocation_entries_;
    size_t args_count_ {0U};
    LLVMCompilationResult llvm_compilation_result_ {LLVMCompilationResult::INVALID};
#ifdef PANDA_LLVMAOT
    compiler::GraphMode graph_mode_ {std::numeric_limits<uint32_t>::max()};

    std::shared_ptr<compiler::AotCompilerInterface> llvm_aot_compiler_ {nullptr};
    static constexpr std::array X86_64_SKIPPED_INTERPRETER_HANDLERS = {
        "ExecuteImplFast",
        "ExecuteImplFastEH",
    };
    static constexpr std::array AARCH64_SKIPPED_INTERPRETER_HANDLERS = {
        "ExecuteImplFast",
        "ExecuteImplFastEH",
    };
    static constexpr std::array AARCH64_SKIPPED_FASTPATHS = {
        "IntfInlineCache",
    };
#endif
};
}  // namespace panda::irtoc

#endif  // PANDA_FUNCTION_H
