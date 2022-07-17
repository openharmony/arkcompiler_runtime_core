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

#include "libpandafile/class_data_accessor.h"
#include "paoc_options.h"
#include "aot/compiled_method.h"
#include "aot_builder/aot_builder.h"
#include "paoc_clusters.h"
#include "compiler.h"
#include "compiler_options.h"
#include "compiler_events_gen.h"
#include "compiler_logger.h"
#include "events/events.h"
#include "include/runtime.h"
#include "mem/gc/gc_types.h"
#include "optimizer_run.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "os/filesystem.h"
#include "generated/base_options.h"

#include "paoc.h"

using namespace panda::compiler;  // NOLINT(google-build-using-namespace)

namespace panda::paoc {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_PAOC(level) LOG(level, COMPILER) << "PAOC: "

/**
 * A class that encloses paoc's initialization:
 */
class Paoc::PaocInitializer {
public:
    explicit PaocInitializer(Paoc *paoc) : paoc_(paoc) {}
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    int Init(const panda::Span<const char *> &args)
    {
        ASSERT(args.Data() != nullptr);
        panda::PandArgParser pa_parser;

        paoc_->runtime_options_ = std::make_unique<decltype(paoc_->runtime_options_)::element_type>(args[0]);
        paoc_->runtime_options_->AddOptions(&pa_parser);
        paoc_->paoc_options_ = std::make_unique<decltype(paoc_->paoc_options_)::element_type>(args[0]);
        paoc_->paoc_options_->AddOptions(&pa_parser);
        panda::compiler::options.AddOptions(&pa_parser);

        base_options::Options base_options("");
        base_options.AddOptions((&pa_parser));

        if (!pa_parser.Parse(args.Size(), args.Data())) {
            std::cerr << "Error: " << pa_parser.GetErrorString() << "\n";
            return -1;
        }
        Logger::Initialize(base_options);
        if (paoc_->paoc_options_->GetPaocPandaFiles().empty()) {
            paoc_->PrintUsage(pa_parser);
            return 1;
        }
        if (InitRuntime() != 0) {
            return -1;
        }
        if (InitCompiler() != 0) {
            return -1;
        }
        if (paoc_->paoc_options_->WasSetPaocClusters() && !InitPaocClusters(&pa_parser)) {
            return -1;
        }
        return 0;
    }

private:
    int InitRuntime()
    {
        auto runtime_options_err = paoc_->runtime_options_->Validate();
        if (runtime_options_err) {
            std::cerr << "Error: " << runtime_options_err.value().GetMessage() << std::endl;
            return 1;
        }
#ifndef PANDA_ASAN_ON
        if (!paoc_->runtime_options_->WasSetInternalMemorySizeLimit()) {
            // 2Gb - for emitting code
            constexpr size_t CODE_SIZE_LIMIT = 0x7f000000;
            paoc_->runtime_options_->SetInternalMemorySizeLimit(CODE_SIZE_LIMIT);
        }
#endif
        paoc_->runtime_options_->SetArkAot(true);
        if (!panda::Runtime::Create(*paoc_->runtime_options_)) {
            std::cerr << "Failed to create runtime!\n";
            return -1;
        }
        return 0;
    }

    int InitCompiler()
    {
        CompilerLogger::Init(panda::compiler::options.GetCompilerLog());

        if (panda::compiler::options.IsCompilerEnableEvents()) {
            EventWriter::Init(panda::compiler::options.GetCompilerEventsPath());
        }
        ValidateCompilerOptions();

        if (paoc_->paoc_options_->WasSetPaocMethodsFromFile()) {
            InitPaocMethodsFromFile();
        }
        paoc_->skip_info_.is_first_compiled = !paoc_->paoc_options_->WasSetPaocSkipUntil();
        paoc_->skip_info_.is_last_compiled = false;

        if (!InitAotBuilder()) {
            return 1;
        }
        if (paoc_->paoc_options_->WasSetPaocDumpStatsCsv()) {
            paoc_->statistics_dump_ = std::ofstream(paoc_->paoc_options_->GetPaocDumpStatsCsv(), std::ofstream::trunc);
        }
        InitPaocMode();
        paoc_->code_allocator_ = new ArenaAllocator(panda::SpaceType::SPACE_TYPE_INTERNAL);
        paoc_->loader_ = panda::Runtime::GetCurrent()->GetClassLinker();

        return 0;
    }

    void ValidateCompilerOptions()
    {
        auto compiler_options_err = panda::compiler::options.Validate();
        if (compiler_options_err) {
            LOG_PAOC(FATAL) << compiler_options_err.value().GetMessage();
        }
        auto paoc_options_err = paoc_->paoc_options_->Validate();
        if (paoc_options_err) {
            LOG_PAOC(FATAL) << paoc_options_err.value().GetMessage();
        }
        if (paoc_->paoc_options_->WasSetPaocOutput() && paoc_->paoc_options_->WasSetPaocBootOutput()) {
            LOG_PAOC(FATAL) << "combination of --paoc-output and --paoc-boot-output is not compatible\n";
        }
        if (paoc_->paoc_options_->WasSetPaocBootPandaLocations()) {
            if (paoc_->paoc_options_->WasSetPaocBootLocation()) {
                LOG_PAOC(FATAL) << "We can't use --paoc-boot-panda-locations with --paoc-boot-location\n";
            }
            auto locations = paoc_->paoc_options_->GetPaocBootPandaLocations();
            size_t i = 0;
            // In fact boot files are preloaded by Runtime
            for (auto bpf : Runtime::GetCurrent()->GetClassLinker()->GetBootPandaFiles()) {
                if (i >= locations.size()) {
                    EVENT_PAOC("Numbers of files in --paoc-boot-panda-locations and --boot-panda-files are different");
                    LOG_PAOC(FATAL)
                        << "number of locations in --paoc-boot-panda-locations less then files in --boot-panda-files\n";
                }
                auto filename = locations[i];
                auto bpf_name = bpf->GetFilename();
                if (!CompareBootFiles(bpf_name, filename)) {
                    EVENT_PAOC("different files in --paoc-boot-panda-locations and --boot-panda-files");
                    LOG_PAOC(ERROR) << "The file from --paoc-boot-panda-locations: " << filename;
                    LOG_PAOC(ERROR) << "isn't eqaul the file from --boot-panda-files: " << bpf->GetFilename();
                    LOG_PAOC(FATAL) << "Files posotions " << i;
                }
                paoc_->location_mapping_[bpf_name] = filename;
                paoc_->preloaded_files_[os::GetAbsolutePath(bpf_name)] = bpf;
                ++i;
            }
            if (i != locations.size()) {
                EVENT_PAOC("Numbers of files in --paoc-boot-panda-locations and --boot-panda-files are different");
                LOG_PAOC(FATAL)
                    << "number of locations in --paoc-boot-panda-locations more then files in --boot-panda-files\n";
            }
        }
        // In fact boot files are preloaded by Runtime
        for (auto bpf : Runtime::GetCurrent()->GetClassLinker()->GetBootPandaFiles()) {
            if (!paoc_->paoc_options_->GetPaocBootLocation().empty()) {
                std::string filename = GetFileLocation(*bpf, paoc_->paoc_options_->GetPaocBootLocation());
                paoc_->location_mapping_[bpf->GetFilename()] = filename;
            }
            paoc_->preloaded_files_[os::GetAbsolutePath(bpf->GetFilename())] = bpf;
        }
    }

    void InitPaocMethodsFromFile()
    {
        std::ifstream mfile(paoc_->paoc_options_->GetPaocMethodsFromFile());
        std::string line;
        if (mfile) {
            while (getline(mfile, line)) {
                paoc_->methods_list_.insert(line);
            }
        }
        LOG_PAOC(INFO) << "Method list size: " << paoc_->methods_list_.size();
    }

    void InitPaocMode()
    {
        const auto &mode_str = paoc_->paoc_options_->GetPaocMode();
        if (mode_str == "aot") {
            paoc_->mode_ = PaocMode::AOT;
        } else if (mode_str == "jit") {
            paoc_->mode_ = PaocMode::JIT;
        } else if (mode_str == "osr") {
            paoc_->mode_ = PaocMode::OSR;
        } else {
            UNREACHABLE();
        }
    }

    bool InitAotBuilder()
    {
        // Initialize Arch:
        Arch arch;
        bool cross_compilation;
        if (paoc_->paoc_options_->WasSetPaocArch()) {
            auto paoc_arch = GetArchFromString(paoc_->paoc_options_->GetPaocArch());
            cross_compilation = paoc_arch != RUNTIME_ARCH;
            arch = paoc_arch;
        } else {
            cross_compilation = false;
            arch = RUNTIME_ARCH;
        }
        panda::compiler::options.AdjustCpuFeatures(cross_compilation);

        if (arch == Arch::NONE) {
            LOG_PAOC(ERROR) << "Invalid arch option:" << paoc_->paoc_options_->GetPaocArch();
            return false;
        }
        if (!BackendSupport(arch)) {
            LOG_PAOC(ERROR) << "Backend is not supported: " << paoc_->paoc_options_->GetPaocArch();
            return false;
        }
        paoc_->aot_builder_.SetArch(arch);

        // Initialize GC:
        auto runtime_lang = paoc_->runtime_options_->GetRuntimeType();
        // Fix it in issue 8164:
        auto gc_type = panda::mem::GCTypeFromString(paoc_->runtime_options_->GetGcType(runtime_lang));
        ASSERT(gc_type != panda::mem::GCType::INVALID_GC);

        paoc_->aot_builder_.SetGcType(static_cast<uint32_t>(gc_type));

#ifndef NDEBUG
        paoc_->aot_builder_.SetGenerateSymbols(true);
#else
        paoc_->aot_builder_.SetGenerateSymbols(paoc_->paoc_options_->IsPaocGenerateSymbols());
#endif
#ifdef PANDA_COMPILER_CFI
        paoc_->aot_builder_.SetEmitDebugInfo(panda::compiler::options.IsCompilerEmitDebugInfo());
#endif
        return true;
    }

    bool InitPaocClusters(panda::PandArgParser *pa_parser)
    {
        std::ifstream fstream(paoc_->paoc_options_->GetPaocClusters());
        if (!fstream) {
            LOG_PAOC(ERROR) << "Can't open `paoc-clusters` file";
            return false;
        }
        JsonObject obj(fstream.rdbuf());
        fstream.close();
        paoc_->clusters_info_.Init(obj, pa_parser);
        return true;
    }

private:
    Paoc *paoc_ {nullptr};
};

int Paoc::Run(const panda::Span<const char *> &args)
{
    if (PaocInitializer(this).Init(args) != 0) {
        return -1;
    }
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    runtime_ = Runtime::GetCurrent()->GetPandaVM()->GetCompilerRuntimeInterface();
    aot_builder_.SetRuntime(runtime_);
    slow_path_data_ = allocator->New<compiler::SharedSlowPathData>();
    if (!LoadPandaFiles()) {
        LOG_PAOC(FATAL) << "Can not load panda files";
    }
    bool error_occurred = !CompileFiles();
    bool attempted_to_compile = (compilation_index_ != 0);
    error_occurred |= !attempted_to_compile;
    if (mode_ == PaocMode::AOT) {
        if (aot_builder_.GetCurrentCodeAddress() != 0 || !aot_builder_.GetClassHashTableSize()->empty()) {
            RunAotMode(args);
        } else {
            LOG_PAOC(ERROR) << "There are no compiled methods";
            Clear(allocator);
            return -1;
        }
    }
    if (!attempted_to_compile) {
        LOG_PAOC(WARNING) << "There are no compiled methods";
    }
    Clear(allocator);
    return ShouldIgnoreFailures() ? 0 : (error_occurred ? 1 : 0);
}

void Paoc::RunAotMode(const panda::Span<const char *> &args)
{
    std::string cmdline;
    for (auto arg : args) {
        cmdline += arg;
        cmdline += " ";
    }
    std::string class_ctx;
    loader_->EnumeratePandaFiles([this, &class_ctx](const panda_file::File &pf) {
        if (location_mapping_.find(pf.GetFilename()) == location_mapping_.end()) {
            class_ctx += os::GetAbsolutePath(pf.GetFilename());
        } else {
            class_ctx += location_mapping_[pf.GetFilename()];
        }
        class_ctx += '*';
        class_ctx += std::to_string(pf.GetHeader()->checksum);
        class_ctx += ':';
        return true;
    });
    class_ctx.pop_back();
    aot_builder_.SetClassContext(class_ctx);
    LOG(DEBUG, COMPILER) << "PAOC: ClassContext '" << class_ctx << '\'';
    auto output_file = paoc_options_->GetPaocOutput();
    if (paoc_options_->WasSetPaocBootOutput()) {
        aot_builder_.SetBootAot(true);
        output_file = paoc_options_->GetPaocBootOutput();
    }
    aot_builder_.SetWithCha(paoc_options_->IsPaocUseCha());
    aot_builder_.Write(cmdline, output_file);
    LOG_PAOC(DEBUG) << "Successfully compiled to " << output_file;
}

void Paoc::Clear(panda::mem::InternalAllocatorPtr allocator)
{
    delete code_allocator_;
    code_allocator_ = nullptr;
    allocator->Delete(slow_path_data_);
    methods_list_.clear();
    panda::Runtime::Destroy();
}

void Paoc::StartAotFile(const panda_file::File &pfile_ref)
{
    ASSERT(mode_ == PaocMode::AOT);
    std::string filename;
    if (paoc_options_->GetPaocLocation().empty()) {
        filename = os::GetAbsolutePath(pfile_ref.GetFilename());
    } else {
        filename = GetFileLocation(pfile_ref, paoc_options_->GetPaocLocation());
        location_mapping_[pfile_ref.GetFilename()] = filename;
    }
    ASSERT(!filename.empty());
    aot_builder_.StartFile(filename, pfile_ref.GetHeader()->checksum);
}

/**
 * Iterate over `--paoc-panda-files`.
 * @return `false` on error.
 */
bool Paoc::CompileFiles()
{
    auto pfiles = paoc_options_->GetPaocPandaFiles();
    bool error_occurred = false;
    auto *vm = panda::Runtime::GetCurrent()->GetPandaVM();
    for (auto &file_name : pfiles) {
        // Load panda file
        const panda_file::File *pfile;

        auto abs_path = os::GetAbsolutePath(file_name);
        if (preloaded_files_.find(abs_path) != preloaded_files_.end()) {
            pfile = preloaded_files_[abs_path];
        } else {
            auto file = vm->OpenPandaFile(file_name);
            if (!file) {
                error_occurred = true;
                if (!ShouldIgnoreFailures()) {
                    LOG_PAOC(FATAL) << "Can not open file: " << file_name;
                }
                LOG_PAOC(WARNING) << "Can not open file: " << file_name;
                continue;
            }
            pfile = file.get();
            loader_->AddPandaFile(std::move(file));
            LOG_PAOC(DEBUG) << "Added panda file: " << file_name;
        }
        auto &pfile_ref = *pfile;

        if (mode_ == PaocMode::AOT) {
            StartAotFile(pfile_ref);
        }

        if (!CompilePandaFile(pfile_ref)) {
            error_occurred = true;
        }

        if (mode_ == PaocMode::AOT) {
            aot_builder_.EndFile();
        }
        if (error_occurred && !ShouldIgnoreFailures()) {
            return false;
        }
    }
    return !error_occurred;
}

/**
 * Compile every method from loaded panda files that matches `--compiler-regex`,
 * `--paoc-skip-until`, `--paoc-compile-until` and `--paoc-methods-from-file` options:
 * @return `false` on error.
 */
bool Paoc::CompilePandaFile(const panda_file::File &pfile_ref)
{
    auto classes = pfile_ref.GetClasses();
    bool error_occurred = false;
    for (auto &class_id : classes) {
        panda_file::File::EntityId id(class_id);
        panda::Class *klass = ResolveClass(pfile_ref, id);
        std::string class_name = ClassHelper::GetName(pfile_ref.GetStringData(id).data);

        if (!PossibleToCompile(pfile_ref, klass, id)) {
            if (paoc_options_->IsPaocVerbose()) {
                LOG_PAOC(DEBUG) << "Ignoring a class `" << class_name << "` (id = " << id << ", file `"
                                << pfile_ref.GetFilename() << "`)";
            }
            continue;
        }

        ASSERT(klass != nullptr);
        if (paoc_options_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Compiling class `" << class_name << "` (id = " << id << ", file `"
                            << pfile_ref.GetFilename() << "`)";
        }

        // Check that all of the methods are compiled correctly:
        ASSERT(klass->GetPandaFile() != nullptr);
        if (!Compile(klass, *klass->GetPandaFile())) {
            error_occurred = true;
            std::string err_msg =
                "Class `" + class_name + "` from " + pfile_ref.GetFilename() + " compiled with errors";
            PrintError(err_msg);
            if (!ShouldIgnoreFailures()) {
                break;
            }
        }
    }

    BuildClassHashTable(pfile_ref);

    return !error_occurred;
}

panda::Class *Paoc::ResolveClass(const panda_file::File &pfile_ref, panda_file::File::EntityId class_id)
{
    ErrorHandler handler;
    ScopedMutatorLock lock;
    if (pfile_ref.IsExternal(class_id)) {
        return nullptr;
    }
    panda_file::ClassDataAccessor cda(pfile_ref, class_id);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(&cda);
    auto klass = loader_->GetExtension(ctx)->GetClass(pfile_ref, class_id, nullptr, &handler);
    return klass;
}

/**
 * Check if it is possible to compile a class.
 * @return true if the class isn't external, abstract, interface and isn't an array class.
 */
bool Paoc::PossibleToCompile(const panda_file::File &pfile_ref, const panda::Class *klass,
                             panda_file::File::EntityId class_id)
{
    std::string class_name = ClassHelper::GetName(pfile_ref.GetStringData(class_id).data);

    // Skip external classes
    if (pfile_ref.IsExternal(class_id)) {
        if (paoc_options_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Can't compile external class `" << class_name << "`";
        }
        return false;
    }
    if (klass == nullptr) {
        if (paoc_options_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Class is nullptr: `" << class_name << "`";
            LOG_PAOC(ERROR) << "ClassLinker can't load a class `" << class_name << "`";
        }
        return false;
    }
    // Skip object array class
    if (klass->IsArrayClass()) {
        if (paoc_options_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Can't compile array class `" << class_name << "`";
        }
        return false;
    }

    if (klass->IsAbstract()) {
        if (paoc_options_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Will compile abstract class `" << class_name << "`";
        }
    }

    return true;
}

/**
 * Compile the class.
 * @return `false` on error.
 */
bool Paoc::Compile(Class *klass, const panda_file::File &pfile_ref)
{
    ASSERT(klass != nullptr);

    if (mode_ == PaocMode::AOT) {
        aot_builder_.StartClass(*klass);
    }
    bool error_occurred = false;
    auto methods = klass->GetMethods();
    panda_file::ClassDataAccessor cda(pfile_ref, klass->GetFileId());
    size_t method_index = 0;
    size_t smethod_idx = klass->GetNumVirtualMethods();
    size_t vmethod_idx = 0;
    cda.EnumerateMethods([this, &smethod_idx, &vmethod_idx, &methods, &method_index, &pfile_ref,
                          &error_occurred](panda_file::MethodDataAccessor &method_data_accessor) {
        if (error_occurred && !ShouldIgnoreFailures()) {
            return;
        }
        // TODO(pishin, msherstennikov): revisit
        // Method (or the whole class?) may already have a definition in another file,
        // in this case it should not be added into AOT file.
        Method &method = method_data_accessor.IsStatic() ? methods[smethod_idx++] : methods[vmethod_idx++];
        auto method_name = runtime_->GetMethodFullName(&method, false);
        if (method.GetPandaFile()->GetFilename() == pfile_ref.GetFilename() && !Skip(&method) &&
            IsMethodInList(method_name) && options.MatchesRegex(method_name) && !Compile(&method, method_index)) {
            error_occurred = true;
        }
        method_index++;
    });

    if (mode_ == PaocMode::AOT) {
        aot_builder_.EndClass();
    }

    return !error_occurred;
}

// Wrapper for CompileJit(), CompileOsr() and CompileAot() arguments:
struct Paoc::CompilingContext {
public:
    NO_COPY_SEMANTIC(CompilingContext);
    NO_MOVE_SEMANTIC(CompilingContext);
    CompilingContext(Method *method_ptr, size_t method_index, std::ofstream *statistics_dump)
        : method(method_ptr),
          allocator(panda::SpaceType::SPACE_TYPE_COMPILER, PandaVM::GetCurrent()->GetMemStats()),
          graph_local_allocator(panda::SpaceType::SPACE_TYPE_COMPILER, PandaVM::GetCurrent()->GetMemStats(), true),
          index(method_index),
          stats(statistics_dump)
    {
    }
    ~CompilingContext()
    {
        if (graph != nullptr) {
            graph->~Graph();
        }
        ASSERT(stats != nullptr);
        if (*stats) {
            DumpStatistics();
        }
    }
    void DumpStatistics() const
    {
        ASSERT(stats);
        char sep = ',';
        *stats << method->GetFullName() << sep;
        *stats << "paoc-summary" << sep;
        *stats << allocator.GetAllocatedSize() << sep;
        *stats << graph_local_allocator.GetAllocatedSize() << '\n';
    }

public:
    Method *method {nullptr};                     // NOLINT(misc-non-private-member-variables-in-classes)
    panda::ArenaAllocator allocator;              // NOLINT(misc-non-private-member-variables-in-classes)
    panda::ArenaAllocator graph_local_allocator;  // NOLINT(misc-non-private-member-variables-in-classes)
    Graph *graph {nullptr};                       // NOLINT(misc-non-private-member-variables-in-classes)
    size_t index;                                 // NOLINT(misc-non-private-member-variables-in-classes)
    std::ofstream *stats {nullptr};               // NOLINT(misc-non-private-member-variables-in-classes)
};

bool Paoc::Compile(Method *method, size_t method_index)
{
    if (method == nullptr) {
        LOG_PAOC(WARNING) << "Method is nullptr";
        return false;
    }
#ifndef NDEBUG
    const auto *pfile = method->GetPandaFile();
    auto method_id = method->GetFileId();
    ASSERT(pfile != nullptr);
    ASSERT(!pfile->IsExternal(method_id));
#endif  // NDEBUG

    if (method->IsAbstract() || method->IsNative() || method->IsIntrinsic()) {
        return true;
    }
    auto method_name = runtime_->GetMethodFullName(method, false);

    ++compilation_index_;
    LOG_PAOC(INFO) << "[" << compilation_index_ << "] Compile method (id=" << method->GetFileId()
                   << "): " << method_name;

    CompilingContext ctx(method, method_index, &statistics_dump_);

    PaocClusters::ScopedApplySpecialOptions to(method_name, &clusters_info_);
    switch (mode_) {
        case PaocMode::AOT:
            if (!CompileAot(&ctx)) {
                EVENT_COMPILATION(method_name, false, ctx.method->GetCodeSize(), 0, 0, 0,
                                  events::CompilationStatus::FAILED);
                return false;
            }
            return true;
        case PaocMode::JIT:
            return CompileJit(&ctx);
        case PaocMode::OSR:
            return CompileOsr(&ctx);
        default:
            UNREACHABLE();
    }
}

/**
 * Compiles a method in JIT mode (i.e. no code generated).
 * @return `false` on error.
 */
bool Paoc::CompileJit(CompilingContext *ctx)
{
    ASSERT(ctx != nullptr);
    ASSERT(mode_ == PaocMode::JIT);
    auto name = runtime_->GetMethodFullName(ctx->method, false);
    auto arch {Arch::NONE};
    bool is_dynamic = panda::panda_file::IsDynamicLanguage(ctx->method->GetClass()->GetSourceLang());
    if (!compiler::CompileInGraph(runtime_, ctx->method, false, &ctx->allocator, &ctx->graph_local_allocator,
                                  is_dynamic, &arch, name, &ctx->graph)) {
        std::string err_msg = "Failed to JIT-compile method: " + name;
        PrintError(err_msg);
        return false;
    }
    EVENT_COMPILATION(name, false, ctx->method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::COMPILED);
    return true;
}

/**
 * Compiles a method in OSR mode.
 * @return `false` on error.
 */
bool Paoc::CompileOsr(CompilingContext *ctx)
{
    ASSERT(ctx != nullptr);
    ASSERT(mode_ == PaocMode::OSR);
    std::string name;
    auto arch {Arch::NONE};
    bool is_dynamic = panda::panda_file::IsDynamicLanguage(ctx->method->GetClass()->GetSourceLang());
    if (!compiler::CompileInGraph(runtime_, ctx->method, true, &ctx->allocator, &ctx->graph_local_allocator, is_dynamic,
                                  &arch, name, &ctx->graph)) {
        std::string err_msg = "Failed to OSR-compile method: " + runtime_->GetMethodFullName(ctx->method, false);
        PrintError(err_msg);
        return false;
    }
    EVENT_COMPILATION(name, true, ctx->method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::COMPILED);
    return true;
}

/**
 * Compiles a method in AOT mode.
 * @return `false` on error.
 */
bool Paoc::CompileAot(CompilingContext *ctx)
{
    ASSERT(ctx != nullptr);
    ASSERT(mode_ == PaocMode::AOT);
    std::string class_name = ClassHelper::GetName(ctx->method->GetClassName().data);

    bool is_dynamic = panda::panda_file::IsDynamicLanguage(ctx->method->GetClass()->GetSourceLang());

    ctx->graph = ctx->allocator.New<Graph>(&ctx->allocator, &ctx->graph_local_allocator, aot_builder_.GetArch(),
                                           ctx->method, runtime_, false, nullptr, is_dynamic);
    if (ctx->graph == nullptr) {
        PrintError("Graph creation failed!");
        return false;
    }

    auto aot_data = ctx->graph->GetAllocator()->New<AotData>(
        ctx->method->GetPandaFile(), ctx->graph, aot_builder_.GetCurrentCodeAddress(),
        aot_builder_.GetIntfInlineCacheIndex(), aot_builder_.GetGotPlt(), aot_builder_.GetGotVirtIndexes(),
        aot_builder_.GetGotClass(), aot_builder_.GetGotString(), aot_builder_.GetGotIntfInlineCache(), slow_path_data_);
    aot_data->SetUseCha(paoc_options_->IsPaocUseCha());
    ctx->graph->SetAotData(aot_data);

    if (!ctx->graph->RunPass<IrBuilder>()) {
        PrintError("IrBuilder failed!");
        return false;
    }
    if (!RunOptimizations(ctx->graph)) {
        PrintError("RunOptimizations failed!");
        return false;
    }

    CompiledMethod compiled_method(ctx->graph->GetArch(), ctx->method);
    compiled_method.SetCode(ctx->graph->GetData().ToConst());
    compiled_method.SetCodeInfo(ctx->graph->GetCodeInfoData());
#ifdef PANDA_COMPILER_CFI
    compiled_method.SetCfiInfo(ctx->graph->GetCallingConvention()->GetCfiInfo());
#endif
    if (ctx->graph->GetData().empty() || ctx->graph->GetCodeInfoData().empty()) {
        LOG(INFO, COMPILER) << "Emit code failed";
        return false;
    }
    aot_builder_.AddMethod(compiled_method, ctx->index);

    LOG(INFO, COMPILER) << "Successfully compiled method: " << runtime_->GetMethodFullName(ctx->method, false);
    ASSERT(ctx->graph != nullptr);

    EVENT_COMPILATION(runtime_->GetMethodFullName(ctx->method, false), false, ctx->method->GetCodeSize(),
                      aot_builder_.GetCurrentCodeAddress() - compiled_method.GetCode().size(),
                      compiled_method.GetCode().size(), compiled_method.GetCodeInfo().size(),
                      events::CompilationStatus::COMPILED);

    return true;
}

void Paoc::PrintError(const std::string &error)
{
    if (ShouldIgnoreFailures()) {
        LOG_PAOC(WARNING) << error;
    } else {
        LOG_PAOC(ERROR) << error;
    }
}

bool Paoc::ShouldIgnoreFailures()
{
    return compiler::options.IsCompilerIgnoreFailures();
}

void Paoc::PrintUsage(const panda::PandArgParser &pa_parser)
{
    std::cerr << "Usage: ark_aot [OPTIONS] --paoc-panda-files <list>\n";
    std::cerr << "    --paoc-panda-files          list of input panda files, it is a mandatory option\n";
    std::cerr << "    --paoc-mode                 specify compilation mode (AOT, JIT or OSR) \n";
    std::cerr << "    --paoc-output               path to output file, default is out.an\n";
    std::cerr << "    --paoc-arch                 target architecture, default is native,"
                 " other possible values see below\n";
    std::cerr << "    --paoc-location             location path of the input panda file, that will be written into"
                 " the AOT file\n";
    std::cerr << "    --paoc-skip-until           first method to compile, skip all previous\n";
    std::cerr << "    --paoc-compile-until        last method to compile, skip all following\n";
    std::cerr << "    --paoc-methods-from-file    path to file which contains methods to compile\n";
#ifndef NDEBUG
    std::cerr << "    --paoc-generate-symbols     generate symbols for compiled methods, disabled by default\n";
#endif
    std::cerr << "    --compiler-ignore-failures  ignore failures in methods/classes/files compilation\n";
    std::cerr << " You can also use other Ark compiler options\n";

    std::cerr << pa_parser.GetHelpString() << std::endl;
}

bool Paoc::IsMethodInList(const std::string &method_full_name)
{
    return !paoc_options_->WasSetPaocMethodsFromFile() || (methods_list_.find(method_full_name) != methods_list_.end());
}

/*
 * Check if needed to skip method, considering  'paoc-skip-until' and 'paoc-compile-until' options
 */
bool Paoc::Skip(Method *method)
{
    if (method == nullptr) {
        return true;
    }

    auto method_name = runtime_->GetMethodFullName(method, false);
    if (!skip_info_.is_first_compiled) {
        if (method_name == paoc_options_->GetPaocSkipUntil()) {
            skip_info_.is_first_compiled = true;
        } else {
            return true;
        }
    }
    if (skip_info_.is_last_compiled) {
        return true;
    }
    if (paoc_options_->WasSetPaocCompileUntil() && method_name == paoc_options_->GetPaocCompileUntil()) {
        skip_info_.is_last_compiled = true;
    }
    return false;
}

std::string Paoc::GetFileLocation(const panda_file::File &pfile_ref, std::string location)
{
    auto &filename = pfile_ref.GetFilename();
    if (auto pos = filename.rfind('/'); pos != std::string::npos) {
        if (location.back() == '/') {
            pos++;
        }
        location += filename.substr(pos);
    } else {
        location += '/' + filename;
    }
    return location;
}

bool Paoc::CompareBootFiles(std::string filename, std::string paoc_location)
{
    if (auto pos = filename.rfind('/'); pos != std::string::npos) {
        filename = filename.substr(++pos);
    }
    if (auto pos = paoc_location.rfind('/'); pos != std::string::npos) {
        paoc_location = paoc_location.substr(++pos);
    }

    return paoc_location == filename;
}

bool Paoc::LoadPandaFiles()
{
    bool error_occurred = false;
    auto *vm = panda::Runtime::GetCurrent()->GetPandaVM();
    auto pfiles = runtime_options_->GetPandaFiles();
    for (auto &file_name : pfiles) {
        auto pfile = vm->OpenPandaFile(file_name);
        if (!pfile) {
            error_occurred = true;
            if (!ShouldIgnoreFailures()) {
                LOG_PAOC(FATAL) << "Can not open file: " << file_name;
            }
            LOG_PAOC(WARNING) << "Can not open file: " << file_name;
            continue;
        }

        if (!paoc_options_->GetPaocLocation().empty()) {
            std::string filename = GetFileLocation(*pfile, paoc_options_->GetPaocLocation());
            location_mapping_[pfile->GetFilename()] = filename;
        }

        preloaded_files_[os::GetAbsolutePath(file_name)] = pfile.get();
        loader_->AddPandaFile(std::move(pfile));
    }
    return !error_occurred;
}

void Paoc::BuildClassHashTable(const panda_file::File &pfile_ref)
{
    if (!pfile_ref.GetClasses().empty()) {
        aot_builder_.AddClassHashTable(pfile_ref);
    }
}

#undef LOG_PAOC

}  // namespace panda::paoc

int main(int argc, const char *argv[])
{
    panda::paoc::Paoc paoc;
    panda::Span<const char *> args(argv, argc);
    return paoc.Run(args);
}
