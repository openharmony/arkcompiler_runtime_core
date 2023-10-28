/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_PAOC_H
#define PANDA_PAOC_H

#include "paoc_options.h"
#include "aot_builder/aot_builder.h"
#include "paoc_clusters.h"
#include "runtime/compiler.h"
#include "utils/expected.h"
#include "utils/span.h"

namespace panda::compiler {
class Graph;
class SharedSlowPathData;
}  // namespace panda::compiler

namespace panda::paoc {

struct SkipInfo {
    bool is_first_compiled;
    bool is_last_compiled;
};

enum class PaocMode : uint8_t { AOT, JIT, OSR, LLVM };

class Paoc {
public:
    int Run(const panda::Span<const char *> &args);

protected:
    enum class LLVMCompilerStatus { FALLBACK = 0, COMPILED = 1, SKIP = 2, ERROR = 3 };

    // Wrapper for CompileJit(), CompileOsr() and CompileAot() arguments:
    struct CompilingContext {
        NO_COPY_SEMANTIC(CompilingContext);
        NO_MOVE_SEMANTIC(CompilingContext);
        CompilingContext(Method *method_ptr, size_t method_index, std::ofstream *statistics_dump);
        ~CompilingContext();
        void DumpStatistics() const;

    public:
        Method *method {nullptr};                     // NOLINT(misc-non-private-member-variables-in-classes)
        panda::ArenaAllocator allocator;              // NOLINT(misc-non-private-member-variables-in-classes)
        panda::ArenaAllocator graph_local_allocator;  // NOLINT(misc-non-private-member-variables-in-classes)
        panda::compiler::Graph *graph {nullptr};      // NOLINT(misc-non-private-member-variables-in-classes)
        size_t index;                                 // NOLINT(misc-non-private-member-variables-in-classes)
        std::ofstream *stats {nullptr};               // NOLINT(misc-non-private-member-variables-in-classes)
        bool compilation_status {true};               // NOLINT(misc-non-private-member-variables-in-classes)
    };

    virtual std::unique_ptr<compiler::AotBuilder> CreateAotBuilder()
    {
        return std::make_unique<compiler::AotBuilder>();
    }

private:
    void RunAotMode(const panda::Span<const char *> &args);
    void StartAotFile(const panda_file::File &pfile_ref);
    bool CompileFiles();
    bool CompilePandaFile(const panda_file::File &pfile_ref);
    panda::Class *ResolveClass(const panda_file::File &pfile_ref, panda_file::File::EntityId class_id);
    bool PossibleToCompile(const panda_file::File &pfile_ref, const panda::Class *klass,
                           panda_file::File::EntityId class_id);
    bool Compile(Class *klass, const panda_file::File &pfile_ref);

    bool Compile(Method *method, size_t method_index);
    bool CompileInGraph(CompilingContext *ctx, std::string method_name, bool is_osr);
    bool RunOptimizations(CompilingContext *ctx);
    bool CompileJit(CompilingContext *ctx);
    bool CompileOsr(CompilingContext *ctx);
    bool CompileAot(CompilingContext *ctx);
    bool FinalizeCompileAot(CompilingContext *ctx, [[maybe_unused]] uintptr_t code_address);
    void PrintError(const std::string &error);
    void PrintUsage(const panda::PandArgParser &pa_parser);
    bool IsMethodInList(const std::string &method_full_name);
    bool Skip(Method *method);
    static std::string GetFileLocation(const panda_file::File &pfile_ref, std::string location);
    static bool CompareBootFiles(std::string filename, std::string paoc_location);
    bool LoadPandaFiles();
    bool TryCreateGraph(CompilingContext *ctx);
    void BuildClassHashTable(const panda_file::File &pfile_ref);
    std::string GetFilePath(std::string file_name);

    bool IsAotMode()
    {
        return mode_ == PaocMode::AOT || mode_ == PaocMode::LLVM;
    }

    class ErrorHandler : public ClassLinkerErrorHandler {
        void OnError([[maybe_unused]] ClassLinker::Error error, [[maybe_unused]] const PandaString &message) override {}
    };

    class PaocInitializer;

protected:
    virtual void AddExtraOptions([[maybe_unused]] PandArgParser *parser) {}
    virtual void ValidateExtraOptions() {}

    panda::paoc::Options *GetPaocOptions()
    {
        return paoc_options_.get();
    }

    compiler::RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }
    ArenaAllocator *GetCodeAllocator()
    {
        return code_allocator_;
    }

    bool ShouldIgnoreFailures();
    bool IsLLVMAotMode()
    {
        return mode_ == PaocMode::LLVM;
    }

    virtual void Clear(panda::mem::InternalAllocatorPtr allocator);
    virtual void PrepareLLVM([[maybe_unused]] const panda::Span<const char *> &args)
    {
        LOG(FATAL, COMPILER) << "--paoc-mode=llvm is temporarily not available";
    }
    virtual LLVMCompilerStatus TryLLVM([[maybe_unused]] CompilingContext *ctx)
    {
        UNREACHABLE();
        return LLVMCompilerStatus::FALLBACK;
    }
    virtual bool EndLLVM()
    {
        UNREACHABLE();
    }

protected:
    std::unique_ptr<compiler::AotBuilder> aot_builder_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    std::unique_ptr<panda::RuntimeOptions> runtime_options_ {nullptr};
    std::unique_ptr<panda::paoc::Options> paoc_options_ {nullptr};

    compiler::RuntimeInterface *runtime_ {nullptr};

    PaocMode mode_ {PaocMode::AOT};
    ClassLinker *loader_ {nullptr};
    ArenaAllocator *code_allocator_ {nullptr};
    std::set<std::string> methods_list_;
    std::unordered_map<std::string, std::string> location_mapping_;
    std::unordered_map<std::string, const panda_file::File *> preloaded_files_;
    size_t compilation_index_ {0};
    SkipInfo skip_info_ {false, false};

    PaocClusters clusters_info_;
    compiler::SharedSlowPathData *slow_path_data_ {nullptr};

    unsigned success_methods_ {0};
    unsigned failed_methods_ {0};

    std::ofstream statistics_dump_;
};

}  // namespace panda::paoc
#endif  // PANDA_PAOC_H
