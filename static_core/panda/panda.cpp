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

#include "include/runtime.h"
#include "include/thread.h"
#include "include/thread_scopes.h"
#include "runtime/include/locks.h"
#include "runtime/include/method-inl.h"
#include "runtime/include/class.h"
#include "utils/pandargs.h"
#include "compiler/compiler_options.h"
#include "compiler/compiler_logger.h"
#include "compiler_events_gen.h"
#include "mem/mem_stats.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/os/native_stack.h"
#include "generated/base_options.h"

#include "ark_version.h"

#include "utils/span.h"

#include "utils/logger.h"

#include <limits>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <csignal>

namespace panda {
const panda_file::File *GetPandaFile(const ClassLinker &class_linker, std::string_view file_name)
{
    const panda_file::File *res = nullptr;
    class_linker.EnumerateBootPandaFiles([&res, file_name](const panda_file::File &pf) {
        if (pf.GetFilename() == file_name) {
            res = &pf;
            return false;
        }
        return true;
    });
    return res;
}

void PrintHelp(const panda::PandArgParser &pa_parser)
{
    std::cerr << pa_parser.GetErrorString() << std::endl;
    std::cerr << "Usage: "
              << "panda"
              << " [OPTIONS] [file] [entrypoint] -- [arguments]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "optional arguments:" << std::endl;
    std::cerr << pa_parser.GetHelpString() << std::endl;
}

bool PrepareArguments(panda::PandArgParser *pa_parser, const RuntimeOptions &runtime_options,
                      const panda::PandArg<std::string> &file, const panda::PandArg<std::string> &entrypoint,
                      const panda::PandArg<bool> &help, int argc, const char **argv)
{
    auto start_time =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    if (!pa_parser->Parse(argc, argv)) {
        PrintHelp(*pa_parser);
        return false;
    }

    if (runtime_options.IsVersion()) {
        PrintPandaVersion();
        return false;
    }

    if (file.GetValue().empty() || entrypoint.GetValue().empty() || help.GetValue()) {
        PrintHelp(*pa_parser);
        return false;
    }

    if (runtime_options.IsStartupTime()) {
        std::cout << "\n"
                  << "Startup start time: " << start_time << std::endl;
    }

    auto runtime_options_err = runtime_options.Validate();
    if (runtime_options_err) {
        std::cerr << "Error: " << runtime_options_err.value().GetMessage() << std::endl;
        return false;
    }

    auto compiler_options_err = compiler::OPTIONS.Validate();
    if (compiler_options_err) {
        std::cerr << "Error: " << compiler_options_err.value().GetMessage() << std::endl;
        return false;
    }

    return true;
}

int Main(int argc, const char **argv)
{
    Span<const char *> sp(argv, argc);
    RuntimeOptions runtime_options(sp[0]);
    base_options::Options base_options(sp[0]);
    panda::PandArgParser pa_parser;

    panda::PandArg<bool> help("help", false, "Print this message and exit");
    panda::PandArg<bool> options("options", false, "Print compiler and runtime options");
    // tail arguments
    panda::PandArg<std::string> file("file", "", "path to pandafile");
    panda::PandArg<std::string> entrypoint("entrypoint", "", "full name of entrypoint function or method");

    runtime_options.AddOptions(&pa_parser);
    base_options.AddOptions(&pa_parser);
    compiler::OPTIONS.AddOptions(&pa_parser);

    pa_parser.Add(&help);
    pa_parser.Add(&options);
    pa_parser.PushBackTail(&file);
    pa_parser.PushBackTail(&entrypoint);
    pa_parser.EnableTail();
    pa_parser.EnableRemainder();

    if (!panda::PrepareArguments(&pa_parser, runtime_options, file, entrypoint, help, argc, argv)) {
        return 1;
    }

    compiler::OPTIONS.AdjustCpuFeatures(false);

    Logger::Initialize(base_options);

    runtime_options.SetVerificationMode(VerificationModeFromString(
        static_cast<Options>(runtime_options).GetVerificationMode()));  // NOLINT(cppcoreguidelines-slicing)
    if (runtime_options.IsVerificationEnabled()) {
        if (!runtime_options.WasSetVerificationMode()) {
            runtime_options.SetVerificationMode(VerificationMode::AHEAD_OF_TIME);
        }
    }

    arg_list_t arguments = pa_parser.GetRemainder();

    panda::compiler::CompilerLogger::SetComponents(panda::compiler::OPTIONS.GetCompilerLog());
    if (compiler::OPTIONS.IsCompilerEnableEvents()) {
        panda::compiler::EventWriter::Init(panda::compiler::OPTIONS.GetCompilerEventsPath());
    }

    auto boot_panda_files = runtime_options.GetBootPandaFiles();

    if (runtime_options.GetPandaFiles().empty()) {
        boot_panda_files.push_back(file.GetValue());
    } else {
        auto panda_files = runtime_options.GetPandaFiles();
        auto found_iter = std::find_if(panda_files.begin(), panda_files.end(),
                                       [&](auto &file_name) { return file_name == file.GetValue(); });
        if (found_iter == panda_files.end()) {
            panda_files.push_back(file.GetValue());
            runtime_options.SetPandaFiles(panda_files);
        }
    }

    runtime_options.SetBootPandaFiles(boot_panda_files);

    if (!Runtime::Create(runtime_options)) {
        std::cerr << "Error: cannot create runtime" << std::endl;
        return -1;
    }

    int ret = 0;

    if (options.GetValue()) {
        std::cout << pa_parser.GetRegularArgs() << std::endl;
    }

    std::string file_name = file.GetValue();
    std::string entry = entrypoint.GetValue();

    auto &runtime = *Runtime::GetCurrent();

    auto res = runtime.ExecutePandaFile(file_name, entry, arguments);
    if (!res) {
        std::cerr << "Cannot execute panda file '" << file_name << "' with entry '" << entry << "'" << std::endl;
        ret = -1;
    } else {
        ret = res.Value();
    }

    if (runtime_options.IsPrintMemoryStatistics()) {
        std::cout << runtime.GetMemoryStatistics();
    }
    if (runtime_options.IsPrintGcStatistics()) {
        std::cout << runtime.GetFinalStatistics();
    }
    if (!Runtime::Destroy()) {
        std::cerr << "Error: cannot destroy runtime" << std::endl;
        return -1;
    }
    pa_parser.DisableTail();
    return ret;
}
}  // namespace panda

int main(int argc, const char **argv)
{
    return panda::Main(argc, argv);
}
