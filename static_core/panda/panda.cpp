/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

namespace ark {
const panda_file::File *GetPandaFile(const ClassLinker &classLinker, std::string_view fileName)
{
    const panda_file::File *res = nullptr;
    classLinker.EnumerateBootPandaFiles([&res, fileName](const panda_file::File &pf) {
        if (pf.GetFilename() == fileName) {
            res = &pf;
            return false;
        }
        return true;
    });
    return res;
}

static void PrintHelp(const ark::PandArgParser &paParser)
{
    std::cerr << paParser.GetErrorString() << std::endl;
    std::cerr << "Usage: "
              << "panda"
              << " [OPTIONS] [file] [entrypoint] -- [arguments]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "optional arguments:" << std::endl;
    std::cerr << paParser.GetHelpString() << std::endl;
}

static bool PrepareArguments(ark::PandArgParser *paParser, const RuntimeOptions &runtimeOptions,
                             const ark::PandArg<std::string> &file, const ark::PandArg<std::string> &entrypoint,
                             const ark::PandArg<bool> &help)
{
    if (runtimeOptions.IsVersion()) {
        PrintPandaVersion();
        return false;
    }

    if (file.GetValue().empty() || entrypoint.GetValue().empty() || help.GetValue()) {
        PrintHelp(*paParser);
        return false;
    }

    auto runtimeOptionsErr = runtimeOptions.Validate();
    if (runtimeOptionsErr) {
        std::cerr << "Error: " << runtimeOptionsErr.value().GetMessage() << std::endl;
        return false;
    }

    auto compilerOptionsErr = compiler::g_options.Validate();
    if (compilerOptionsErr) {
        std::cerr << "Error: " << compilerOptionsErr.value().GetMessage() << std::endl;
        return false;
    }

    return true;
}

static void SetPandaFiles(RuntimeOptions &runtimeOptions, ark::PandArg<std::string> &file)
{
    const std::string &fileName = file.GetValue();
    auto bootPandaFiles = runtimeOptions.GetBootPandaFiles();
    auto pandaFiles = runtimeOptions.GetPandaFiles();
    auto bootFoundIter = std::find(bootPandaFiles.begin(), bootPandaFiles.end(), fileName);
    if (runtimeOptions.IsLoadInBoot() && bootFoundIter == bootPandaFiles.end()) {
        bootPandaFiles.push_back(fileName);
        runtimeOptions.SetBootPandaFiles(bootPandaFiles);
        return;
    }

    if (pandaFiles.empty() && bootFoundIter == bootPandaFiles.end()) {
        pandaFiles.push_back(fileName);
        runtimeOptions.SetPandaFiles(pandaFiles);
        return;
    }

    auto pandaFoundIter = std::find(pandaFiles.begin(), pandaFiles.end(), fileName);
    if (pandaFoundIter == pandaFiles.end()) {
        pandaFiles.push_back(fileName);
        runtimeOptions.SetPandaFiles(pandaFiles);
    }
}

static ark::PandArgParser GetPandArgParser(ark::PandArg<bool> &help, ark::PandArg<bool> &options,
                                           ark::PandArg<std::string> &file, ark::PandArg<std::string> &entrypoint)
{
    ark::PandArgParser paParser;

    paParser.Add(&help);
    paParser.Add(&options);
    paParser.PushBackTail(&file);
    paParser.PushBackTail(&entrypoint);
    paParser.EnableTail();
    paParser.EnableRemainder();

    return paParser;
}

static int ExecutePandaFile(Runtime &runtime, const std::string &fileName, const std::string &entry,
                            const arg_list_t &arguments)
{
    auto res = runtime.ExecutePandaFile(fileName, entry, arguments);
    if (!res) {
        std::cerr << "Cannot execute panda file '" << fileName << "' with entry '" << entry << "'" << std::endl;
        return -1;
    }

    return res.Value();
}

static void PrintStatistics(RuntimeOptions &runtimeOptions, Runtime &runtime)
{
    if (runtimeOptions.IsPrintMemoryStatistics()) {
        std::cout << runtime.GetMemoryStatistics();
    }
    if (runtimeOptions.IsPrintGcStatistics()) {
        std::cout << runtime.GetFinalStatistics();
    }
}

int Main(int argc, const char **argv)
{
    Span<const char *> sp(argv, argc);
    RuntimeOptions runtimeOptions(sp[0]);
    base_options::Options baseOptions(sp[0]);

    ark::PandArg<bool> help("help", false, "Print this message and exit");
    ark::PandArg<bool> options("options", false, "Print compiler and runtime options");
    // tail arguments
    ark::PandArg<std::string> file("file", "", "path to pandafile");
    ark::PandArg<std::string> entrypoint("entrypoint", "", "full name of entrypoint function or method");

    ark::PandArgParser paParser = GetPandArgParser(help, options, file, entrypoint);

    runtimeOptions.AddOptions(&paParser);
    baseOptions.AddOptions(&paParser);
    compiler::g_options.AddOptions(&paParser);

    auto startTime =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    if (!paParser.Parse(argc, argv)) {
        PrintHelp(paParser);
        return 1;
    }

    if (runtimeOptions.IsStartupTime()) {
        // clang-format off
        std::cout << "\n" << "Startup start time: " << startTime << std::endl;
        // clang-format on
    }

    if (!ark::PrepareArguments(&paParser, runtimeOptions, file, entrypoint, help)) {
        return 1;
    }

    compiler::g_options.AdjustCpuFeatures(false);

    Logger::Initialize(baseOptions);

    ark::compiler::CompilerLogger::SetComponents(ark::compiler::g_options.GetCompilerLog());
    if (compiler::g_options.IsCompilerEnableEvents()) {
        ark::compiler::EventWriter::Init(ark::compiler::g_options.GetCompilerEventsPath());
    }

    SetPandaFiles(runtimeOptions, file);

    if (!Runtime::Create(runtimeOptions)) {
        std::cerr << "Error: cannot create runtime" << std::endl;
        return -1;
    }

    if (options.GetValue()) {
        std::cout << paParser.GetRegularArgs() << std::endl;
    }

    auto &runtime = *Runtime::GetCurrent();

    int ret = ExecutePandaFile(runtime, file.GetValue(), entrypoint.GetValue(), paParser.GetRemainder());
    PrintStatistics(runtimeOptions, runtime);

    if (!Runtime::Destroy()) {
        std::cerr << "Error: cannot destroy runtime" << std::endl;
        return -1;
    }
    paParser.DisableTail();
    return ret;
}
}  // namespace ark

int main(int argc, const char **argv)
{
    return ark::Main(argc, argv);
}
