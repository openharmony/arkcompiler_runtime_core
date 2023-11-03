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

#include "disassembler.h"

#include "utils/logger.h"
#include "utils/pandargs.h"
#include "ark_version.h"
#include "file_format_version.h"

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Options {
    panda::PandArg<bool> help {"help", false, "Print this message and exit"};
    panda::PandArg<bool> verbose {"verbose", false, "enable informative code output"};
    panda::PandArg<bool> quiet {"quiet", false, "enables all of the --skip-* flags"};
    panda::PandArg<bool> skip_strings {
        "skip-string-literals", false,
        "replaces string literals with their respectie id's, thus shortening emitted code size"};
    panda::PandArg<bool> with_separators {"with_separators", false,
                                          "adds comments that separate sections in the output file"};
    panda::PandArg<bool> debug {
        "debug", false, "enable debug messages (will be printed to standard output if no --debug-file was specified) "};
    panda::PandArg<std::string> debug_file {"debug-file", "",
                                            "(--debug-file FILENAME) set debug file name. default is std::cout"};
    panda::PandArg<std::string> input_file {"input_file", "", "Path to the source binary code"};
    panda::PandArg<std::string> output_file {"output_file", "", "Path to the generated assembly code"};
    panda::PandArg<std::string> profile {"profile", "", "Path to the profile"};
    panda::PandArg<bool> version {"version", false,
                                  "Ark version, file format version and minimum supported file format version"};

    explicit Options(panda::PandArgParser &pa_parser)
    {
        pa_parser.Add(&help);
        pa_parser.Add(&verbose);
        pa_parser.Add(&quiet);
        pa_parser.Add(&skip_strings);
        pa_parser.Add(&debug);
        pa_parser.Add(&debug_file);
        pa_parser.Add(&version);
        pa_parser.PushBackTail(&input_file);
        pa_parser.PushBackTail(&output_file);
        pa_parser.Add(&profile);
        pa_parser.EnableTail();
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

void PrintHelp(panda::PandArgParser &pa_parser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "ark_disasm [options] input_file output_file" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << pa_parser.GetHelpString() << std::endl;
}

void Disassemble(const Options &options)
{
    auto input_file = options.input_file.GetValue();
    LOG(DEBUG, DISASSEMBLER) << "[initializing disassembler]\nfile: " << input_file << "\n";

    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(input_file, options.quiet.GetValue(), options.skip_strings.GetValue());
    auto verbose = options.verbose.GetValue();
    if (verbose) {
        auto profile = options.profile.GetValue();
        if (!profile.empty()) {
            disasm.SetProfile(profile);
        }
        disasm.CollectInfo();
    }

    LOG(DEBUG, DISASSEMBLER) << "[serializing results]\n";

    std::ofstream res_pa;
    res_pa.open(options.output_file.GetValue(), std::ios::trunc | std::ios::out);
    disasm.Serialize(res_pa, options.with_separators.GetValue(), verbose);
    res_pa.close();
}

bool ProcessArgs(panda::PandArgParser &pa_parser, const Options &options, int argc, const char **argv)
{
    if (!pa_parser.Parse(argc, argv)) {
        PrintHelp(pa_parser);
        return false;
    }

    if (options.version.GetValue()) {
        panda::PrintPandaVersion();
        panda::panda_file::PrintBytecodeVersion();
        return false;
    }

    if (options.input_file.GetValue().empty() || options.output_file.GetValue().empty() || options.help.GetValue()) {
        PrintHelp(pa_parser);
        return false;
    }

    if (options.debug.GetValue()) {
        auto debug_file = options.debug_file.GetValue();
        if (debug_file.empty()) {
            panda::Logger::InitializeStdLogging(
                panda::Logger::Level::DEBUG,
                panda::Logger::ComponentMask().set(panda::Logger::Component::DISASSEMBLER));
        } else {
            panda::Logger::InitializeFileLogging(
                debug_file, panda::Logger::Level::DEBUG,
                panda::Logger::ComponentMask().set(panda::Logger::Component::DISASSEMBLER));
        }
    } else {
        panda::Logger::InitializeStdLogging(panda::Logger::Level::ERROR,
                                            panda::Logger::ComponentMask().set(panda::Logger::Component::DISASSEMBLER));
    }

    return true;
}

int main(int argc, const char **argv)
{
    panda::PandArgParser pa_parser;
    Options options {pa_parser};

    if (!ProcessArgs(pa_parser, options, argc, argv)) {
        return 1;
    }

    Disassemble(options);

    pa_parser.DisableTail();

    return 0;
}
