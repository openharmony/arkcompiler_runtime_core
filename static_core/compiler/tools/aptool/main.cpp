/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <functional>
#include <iostream>
#include <string_view>

#include "compiler/tools/aptool/common/utils.h"
#include "compiler/tools/aptool/dump/dump_command.h"
#include "libarkbase/utils/logger.h"
#include "runtime/mem/internal_allocator-inl.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/mem/mem_stats_additional_info.h"
#include "runtime/mem/mem_stats_default.h"

// Explicit template instantiation to avoid linking errors in Release mode
// These are needed because some inline functions in Allocator base classes
// may call these template methods.
namespace ark::mem {
template void InternalAllocator<InternalAllocatorConfig::MALLOC_ALLOCATOR>::VisitAndRemoveAllPools(
    std::function<void(void *, size_t)>);
template void InternalAllocator<InternalAllocatorConfig::MALLOC_ALLOCATOR>::VisitAndRemoveFreePools(
    std::function<void(void *, size_t)>);
}  // namespace ark::mem

namespace ark::aptool {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_APTOOL(level) LOG(level, PROFILER) << "APTOOL: "

void PrintGlobalUsage(std::ostream &out, const char *progName)
{
    out << "Usage: " << progName << " <command> [options]" << std::endl;
    out << std::endl;
    out << "Ark Profile Tool (aptool) - A tool for managing and analyzing .ap profile files" << std::endl;
    out << std::endl;
    out << "Commands:" << std::endl;
    out << "  dump      Dump profile content to human-readable YAML format" << std::endl;
    out << std::endl;
    out << "Global options:" << std::endl;
    out << "  --help     Show this help message" << std::endl;
    out << "  --version  Show version information" << std::endl;
    out << std::endl;
    out << "Run '" << progName << " <command> --help' for more information on a command." << std::endl;
}

void PrintVersion(std::ostream &out)
{
    out << "aptool version " << common::APTOOL_VERSION << std::endl;
}

int Main(int argc, const char **argv)
{
    if (argc < 2) {  // NOLINT
        PrintGlobalUsage(std::cout, argv[0]);
        return EXIT_FAILURE;
    }

    std::string_view firstArg(argv[1]);

    // Handle global options
    if (firstArg == "--help" || firstArg == "-h") {
        PrintGlobalUsage(std::cout, argv[0]);
        return EXIT_SUCCESS;
    }

    if (firstArg == "--version" || firstArg == "-v") {
        PrintVersion(std::cout);
        return EXIT_SUCCESS;
    }

    // Route to subcommands
    if (firstArg == "dump") {
        dump::DumpCommand cmd;
        return cmd.Run(argc - 1, argv + 1);
    }

    // Unknown command or option
    LOG_APTOOL(ERROR) << "unknown command or option '" << firstArg << "'";
    LOG_APTOOL(ERROR) << "Run '" << argv[0] << " --help' for usage.";
    return EXIT_FAILURE;
}

#undef LOG_APTOOL

}  // namespace ark::aptool

int main(int argc, const char **argv)
{
    // Initialize memory allocators required by arkruntime libraries
    auto *memStats = new (std::nothrow) ark::mem::MemStatsType();
    ASSERT(memStats != nullptr);
    auto *internalAllocator =
        new (std::nothrow) ark::mem::InternalAllocatorT<ark::mem::InternalAllocatorConfig::MALLOC_ALLOCATOR>(memStats);
    ASSERT(internalAllocator != nullptr);
    ark::mem::InternalAllocator<>::InitInternalAllocatorFromRuntime(
        static_cast<ark::mem::Allocator *>(internalAllocator));

    // Initialize logging
    ark::Logger::InitializeStdLogging(ark::Logger::Level::ERROR,
                                      ark::Logger::ComponentMask().set(ark::Logger::Component::PROFILER));

    // Run main logic
    auto result = ark::aptool::Main(argc, argv);

    // Cleanup logger and allocators
    ark::Logger::Destroy();
    ark::mem::InternalAllocator<>::ClearInternalAllocatorFromRuntime();
    delete internalAllocator;
    delete memStats;

    return result;
}
