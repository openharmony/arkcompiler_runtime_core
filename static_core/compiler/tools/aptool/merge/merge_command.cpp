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

#include "merge_command.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/tools/aptool/common/profile_loader.h"
#include "generated/merge_options.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/pandargs.h"
#include "runtime/jit/libprofile/pgo_class_context_utils.h"
#include "runtime/jit/libprofile/pgo_file_builder.h"
#include "runtime/jit/libprofile/profile_merger.h"

namespace ark::aptool::merge {

// CC-OFFNXT(G.PRE.02-CPP) stream-style logging helper macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_APTOOL(level) LOG(level, PROFILER) << "APTOOL: "

namespace {

void CollectInputPaths(const arg_list_t &src, std::vector<std::string> &dst)
{
    // Deduplicate by linear scan: merge input count is typically small
    dst.reserve(src.size());
    for (const auto &item : src) {
        if (!item.empty() && std::find(dst.begin(), dst.end(), item) == dst.end()) {
            dst.emplace_back(item);
        }
    }
}

bool LoadInputProfile(const std::string &path, common::ProfileData &profile, std::string &error)
{
    if (!profile.Load(path, error)) {
        error = "failed to load input '" + path + "': " + error;
        return false;
    }
    return true;
}

size_t CountMergedMethods(const ark::pgo::MergedProfile &mergedProfile)
{
    size_t methodCount = 0;
    for (const auto &entry : mergedProfile.data.GetAllMethods()) {
        methodCount += entry.second.size();
    }
    return methodCount;
}

}  // namespace

// NOLINTNEXTLINE(readability-function-size)
MergeExitCode MergeCommand::Run(const char *progName, int argc, const char **argv)
{
    PandArgParser parser;

    Options options(argv[0]);
    options.AddOptions(&parser);

    if (!parser.Parse(argc, argv)) {
        auto error = parser.GetErrorString();
        while (!error.empty() && (error.back() == '\n' || error.back() == '\r')) {
            error.pop_back();
        }
        LOG_APTOOL(ERROR) << error;
        LOG_APTOOL(ERROR) << "Run '" << progName << " " << argv[0] << " --help' for usage.";
        return MergeExitCode::USAGE_ERROR;
    }

    if (options.IsHelp()) {
        std::cout << "Usage: " << progName << " " << argv[0]
                  << " --ap-path <input-profile> --ap-path <input-profile> [--ap-path <input-profile> ...] --output"
                  << " <merged-profile> [options]" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << parser.GetHelpString() << std::endl;
        return MergeExitCode::SUCCESS;
    }

    // Normalize user input list: remove empty items and duplicate paths
    std::vector<std::string> inputPaths;
    CollectInputPaths(options.GetApPath(), inputPaths);
    if (inputPaths.size() < 2U) {
        LOG_APTOOL(ERROR) << "--ap-path is required; provide at least two distinct profiles";
        return MergeExitCode::USAGE_ERROR;
    }

    const auto &outputPath = options.GetOutput();
    if (outputPath.empty()) {
        LOG_APTOOL(ERROR) << "--output is required";
        return MergeExitCode::USAGE_ERROR;
    }

    std::vector<common::ProfileData> loadedProfiles;
    std::vector<std::string_view> classContexts;
    PandaVector<const ark::pgo::AotProfilingData *> mergerInputs;
    loadedProfiles.reserve(inputPaths.size());
    classContexts.reserve(inputPaths.size());
    mergerInputs.reserve(inputPaths.size());
    std::string error;
    // Load all inputs
    for (const auto &path : inputPaths) {
        common::ProfileData profile;
        if (!LoadInputProfile(path, profile, error)) {
            LOG_APTOOL(ERROR) << error;
            return MergeExitCode::PROFILE_LOAD_ERROR;
        }
        loadedProfiles.emplace_back(std::move(profile));
        auto &storedProfile = loadedProfiles.back();
        const auto &classCtx = storedProfile.GetClassContext();
        classContexts.emplace_back(classCtx.c_str(), classCtx.size());
        mergerInputs.push_back(&storedProfile.GetProfilingData());
    }

    // Merge class context first: checksum mismatch should fail fast.
    std::string mergedClassCtx;
    if (!ark::pgo::PgoClassContextUtils::Merge(classContexts, mergedClassCtx, &error)) {
        LOG_APTOOL(ERROR) << "failed to merge class context: " << error;
        return MergeExitCode::PROFILE_COMPATIBILITY_ERROR;
    }

    ark::pgo::ProfileMerger merger;
    ark::pgo::MergedProfile mergedProfile;
    // Merge all profile payloads.
    if (!merger.Merge(mergerInputs, mergedProfile, &error)) {
        LOG_APTOOL(ERROR) << "failed to merge profile data: " << error;
        return MergeExitCode::PROFILE_COMPATIBILITY_ERROR;
    }

    // Save merged profile with merged class context.
    ark::pgo::AotPgoFile writer;
    uint32_t writtenBytes =
        writer.Save(PandaString(outputPath.c_str()), &mergedProfile.data, PandaString(mergedClassCtx.c_str()));
    if (writtenBytes == 0U) {
        LOG_APTOOL(ERROR) << "failed to write output profile '" << outputPath << "'";
        return MergeExitCode::PROFILE_WRITE_ERROR;
    }

    if (options.IsVerbose()) {
        LOG_APTOOL(INFO) << "Profile merged to " << outputPath;
        LOG_APTOOL(INFO) << "  Input profiles: " << loadedProfiles.size();
        LOG_APTOOL(INFO) << "  Methods: " << CountMergedMethods(mergedProfile);
        LOG_APTOOL(INFO) << "  Panda files: " << mergedProfile.pandaFilesStorage.size();
    }
    return MergeExitCode::SUCCESS;
}

#undef LOG_APTOOL

}  // namespace ark::aptool::merge
