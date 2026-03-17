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
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "compiler/tools/aptool/common/abc_metadata.h"
#include "compiler/tools/aptool/common/profile_loader.h"
#include "compiler/tools/aptool/dump/dump_command.h"
#include "compiler/tools/aptool/dump/filter.h"
#include "compiler/tools/aptool/dump/yaml_writer.h"
#include "generated/dump_options.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/pandargs.h"

namespace ark::aptool::dump {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_APTOOL(level) LOG(level, PROFILER) << "APTOOL: "

int DumpCommand::Run(int argc, const char **argv)
{
    PandArgParser parser;
    PandArg<bool> help("help", false, "Print this message and exit");

    Options options(argv[0]);
    options.AddOptions(&parser);
    parser.Add(&help);

    if (!parser.Parse(argc, argv)) {
        LOG_APTOOL(ERROR) << parser.GetErrorString();
        std::cout << std::endl;
        std::cout << parser.GetHelpString() << std::endl;
        return EXIT_FAILURE;
    }

    if (help.GetValue()) {
        std::cout << parser.GetHelpString() << std::endl;
        return EXIT_SUCCESS;
    }

    // Validate required options
    if (options.GetApPath().empty()) {
        LOG_APTOOL(ERROR) << "--ap-path is required";
        return EXIT_FAILURE;
    }

    if (options.GetOutput().empty()) {
        LOG_APTOOL(ERROR) << "--output is required";
        return EXIT_FAILURE;
    }

    // Load profile
    common::ProfileData profile;
    std::string error;
    if (!profile.Load(options.GetApPath(), error)) {
        LOG_APTOOL(ERROR) << "failed to load profile '" << options.GetApPath() << "': " << error;
        return EXIT_FAILURE;
    }

    // Load ABC metadata if paths provided
    common::AbcMetadataProvider metadata;
    bool hasMetadata = false;
    if (!options.GetAbcPath().empty() || !options.GetAbcDir().empty()) {
        arg_list_t files;
        if (!options.GetAbcPath().empty()) {
            files.push_back(options.GetAbcPath());
        }
        arg_list_t dirs;
        if (!options.GetAbcDir().empty()) {
            dirs.push_back(options.GetAbcDir());
        }
        if (!metadata.Load(files, dirs)) {
            LOG_APTOOL(ERROR) << "failed to load ABC metadata";
            return EXIT_FAILURE;
        }
        hasMetadata = true;
    }

    // Validate that class/method filters require ABC metadata
    bool hasClassFilter = !options.GetKeepClassName().empty();
    bool hasMethodFilter = !options.GetKeepMethodName().empty();
    if ((hasClassFilter || hasMethodFilter) && !hasMetadata) {
        LOG_APTOOL(ERROR) << "--keep-class-name and --keep-method-name require --abc-path or --abc-dir";
        return EXIT_FAILURE;
    }

    // Build checksum index and verify class context when metadata is available
    if (hasMetadata) {
        metadata.BuildChecksumIndex(profile.GetClassContext());
        std::string verifyError;
        if (!metadata.VerifyClassContext(profile.GetClassContext(), verifyError)) {
            LOG_APTOOL(ERROR) << "class context verification failed: " << verifyError;
            return EXIT_FAILURE;
        }
    }

    // Build filter options
    DumpFilterOptions filters;
    for (const auto &v : options.GetKeepPandaFileName()) {
        filters.keepPandaFiles.push_back(v);
    }
    for (const auto &v : options.GetKeepClassName()) {
        filters.keepClassDescriptors.push_back(v);
    }
    for (const auto &v : options.GetKeepMethodName()) {
        filters.keepMethodNames.push_back(v);
    }

    // Open output file
    std::ofstream outFile(options.GetOutput());
    if (!outFile.is_open()) {
        LOG_APTOOL(ERROR) << "cannot open output file '" << options.GetOutput() << "'";
        return EXIT_FAILURE;
    }

    // Dump profile to YAML
    DumpProfileToYaml(outFile, profile, hasMetadata ? &metadata : nullptr, filters);

    outFile.close();

    LOG_APTOOL(INFO) << "Profile dumped to " << options.GetOutput();
    LOG_APTOOL(INFO) << "  Methods: " << profile.GetMethodCount();
    LOG_APTOOL(INFO) << "  Panda files: " << profile.GetPandaFiles().size();

    return EXIT_SUCCESS;
}

#undef LOG_APTOOL

}  // namespace ark::aptool::dump
