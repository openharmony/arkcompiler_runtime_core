/**
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

#ifndef PANDA_PAOC_H
#define PANDA_PAOC_H

#include "paoc_options.h"
#include "aot_builder/aot_builder.h"
#include "paoc_clusters.h"
#include "runtime/compiler.h"
#include "utils/span.h"

namespace panda::paoc {

struct SkipInfo {
    bool is_first_compiled;
    bool is_last_compiled;
};

enum class PaocMode : uint8_t { AOT, JIT, OSR };

class Paoc {
public:
    int Run(const panda::Span<const char *> &args);

private:
    void RunAotMode(const panda::Span<const char *> &args);
    void Clear(panda::mem::InternalAllocatorPtr allocator);
    void StartAotFile(const panda_file::File &pfile_ref);
    bool CompileFiles();
    bool CompilePandaFile(const panda_file::File &pfile_ref);
    panda::Class *ResolveClass(const panda_file::File &pfile_ref, panda_file::File::EntityId class_id);
    bool PossibleToCompile(const panda_file::File &pfile_ref, const panda::Class *klass,
                           panda_file::File::EntityId class_id);
    bool Compile(Class *klass, const panda_file::File &pfile_ref);

    struct CompilingContext;

    bool Compile(Method *method, size_t method_index);
    bool CompileJit(CompilingContext *ctx);
    bool CompileOsr(CompilingContext *ctx);
    bool CompileAot(CompilingContext *ctx);
    void PrintError(const std::string &error);
    bool ShouldIgnoreFailures();
    void PrintUsage(const panda::PandArgParser &pa_parser);
    bool IsMethodInList(const std::string &method_full_name);
    bool Skip(Method *method);
    static std::string GetFileLocation(const panda_file::File &pfile_ref, std::string location);
    static bool CompareBootFiles(std::string filename, std::string paoc_location);
    bool LoadPandaFiles();
    void BuildClassHashTable(const panda_file::File &pfile_ref);

    class ErrorHandler : public ClassLinkerErrorHandler {
        void OnError([[maybe_unused]] ClassLinker::Error error, [[maybe_unused]] const PandaString &message) override {}
    };

    class PaocInitializer;

    std::unique_ptr<panda::RuntimeOptions> runtime_options_ {nullptr};
    std::unique_ptr<panda::paoc::Options> paoc_options_ {nullptr};

    compiler::AotBuilder aot_builder_;
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

    std::ofstream statistics_dump_;
};

}  // namespace panda::paoc
#endif  // PANDA_PAOC_H
