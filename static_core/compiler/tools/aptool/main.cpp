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

#ifdef PANDA_OHOS_USE_INNER_HILOG
#include <hilog/log.h>
#endif

#include "compiler/tools/aptool/common/utils.h"
#ifdef APTOOL_INCLUDE_DUMP
#include "compiler/tools/aptool/dump/dump_command.h"
#endif
#include "compiler/tools/aptool/merge/merge_command.h"
#include "libarkbase/utils/logger.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/mem/internal_allocator-inl.h"
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

#ifdef PANDA_OHOS_USE_INNER_HILOG
int AptoolLogPrint([[maybe_unused]] int id, int level, const char *component, [[maybe_unused]] const char *fmt,
                   const char *message)
{
    constexpr unsigned int ARK_DOMAIN = 0xD003F00;
    constexpr auto TAG = "ArkAptool";
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, ARK_DOMAIN, TAG};
    switch (level) {
        case ark::Logger::PandaLog2MobileLog::DEBUG:
            OHOS::HiviewDFX::HiLog::Debug(LABEL, "%{public}s: %{public}s", component, message);
            return 0;
        case ark::Logger::PandaLog2MobileLog::INFO:
            OHOS::HiviewDFX::HiLog::Info(LABEL, "%{public}s: %{public}s", component, message);
            return 0;
        case ark::Logger::PandaLog2MobileLog::ERROR:
            OHOS::HiviewDFX::HiLog::Error(LABEL, "%{public}s: %{public}s", component, message);
            return 0;
        case ark::Logger::PandaLog2MobileLog::FATAL:
            OHOS::HiviewDFX::HiLog::Fatal(LABEL, "%{public}s: %{public}s", component, message);
            return 0;
        case ark::Logger::PandaLog2MobileLog::WARN:
            OHOS::HiviewDFX::HiLog::Warn(LABEL, "%{public}s: %{public}s", component, message);
            return 0;
        default:
            OHOS::HiviewDFX::HiLog::Info(LABEL, "%{public}s: %{public}s", component, message);
            return 0;
    }
}
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_APTOOL(level) LOG(level, PROFILER) << "APTOOL: "

void PrintGlobalUsage(std::ostream &out, const char *progName)
{
    out << "Usage: " << progName << " <command> [options]" << std::endl;
    out << std::endl;
    out << "Ark Profile Tool (aptool) - A tool for managing and analyzing .ap profile files" << std::endl;
    out << std::endl;
    out << "Commands:" << std::endl;
#ifdef APTOOL_INCLUDE_DUMP
    out << "  dump      Dump profile content to human-readable YAML format" << std::endl;
#endif
    out << "  merge     Merge multiple profiles into one" << std::endl;
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
    constexpr int minArgc = 2;
    if (argc < minArgc) {
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
#ifdef APTOOL_INCLUDE_DUMP
    if (firstArg == "dump") {
        dump::DumpCommand cmd;
        return cmd.Run(argv[0], argc - 1, argv + 1);
    }
#endif
    if (firstArg == "merge") {
        merge::MergeCommand cmd;
        return static_cast<int>(cmd.Run(argv[0], argc - 1, argv + 1));
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
    ark::Logger::InitializeStdLogging(ark::Logger::Level::INFO,
                                      ark::Logger::ComponentMask().set(ark::Logger::Component::PROFILER));
#ifdef PANDA_OHOS_USE_INNER_HILOG
    ark::Logger::SetMobileLogPrintEntryPointByPtr(reinterpret_cast<void *>(ark::aptool::AptoolLogPrint));
#endif

    // Run main logic
    auto result = ark::aptool::Main(argc, argv);

    // Cleanup logger and allocators
    ark::Logger::Destroy();
    ark::mem::InternalAllocator<>::ClearInternalAllocatorFromRuntime();
    delete internalAllocator;
    delete memStats;

    return result;
}
