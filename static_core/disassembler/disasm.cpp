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
    panda::PandArg<bool> skipStrings {
        "skip-string-literals", false,
        "replaces string literals with their respectie id's, thus shortening emitted code size"};
    panda::PandArg<bool> withSeparators {"with_separators", false,
                                         "adds comments that separate sections in the output file"};
    panda::PandArg<bool> debug {
        "debug", false, "enable debug messages (will be printed to standard output if no --debug-file was specified) "};
    panda::PandArg<std::string> debugFile {"debug-file", "",
                                           "(--debug-file FILENAME) set debug file name. default is std::cout"};
    panda::PandArg<std::string> inputFile {"input_file", "", "Path to the source binary code"};
    panda::PandArg<std::string> outputFile {"output_file", "", "Path to the generated assembly code"};
    panda::PandArg<std::string> profile {"profile", "", "Path to the profile"};
    panda::PandArg<bool> version {"version", false,
                                  "Ark version, file format version and minimum supported file format version"};

    explicit Options(panda::PandArgParser &paParser)
    {
        paParser.Add(&help);
        paParser.Add(&verbose);
        paParser.Add(&quiet);
        paParser.Add(&skipStrings);
        paParser.Add(&debug);
        paParser.Add(&debugFile);
        paParser.Add(&version);
        paParser.PushBackTail(&inputFile);
        paParser.PushBackTail(&outputFile);
        paParser.Add(&profile);
        paParser.EnableTail();
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

void PrintHelp(panda::PandArgParser &paParser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "ark_disasm [options] input_file output_file" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << paParser.GetHelpString() << std::endl;
}

void Disassemble(const Options &options)
{
    auto inputFile = options.inputFile.GetValue();
    LOG(DEBUG, DISASSEMBLER) << "[initializing disassembler]\nfile: " << inputFile << "\n";

    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(inputFile, options.quiet.GetValue(), options.skipStrings.GetValue());
    auto verbose = options.verbose.GetValue();
    if (verbose) {
        auto profile = options.profile.GetValue();
        if (!profile.empty()) {
            disasm.SetProfile(profile);
        }
        disasm.CollectInfo();
    }

    LOG(DEBUG, DISASSEMBLER) << "[serializing results]\n";

    std::ofstream resPa;
    resPa.open(options.outputFile.GetValue(), std::ios::trunc | std::ios::out);
    disasm.Serialize(resPa, options.withSeparators.GetValue(), verbose);
    resPa.close();
}

bool ProcessArgs(panda::PandArgParser &paParser, const Options &options, int argc, const char **argv)
{
    if (!paParser.Parse(argc, argv)) {
        PrintHelp(paParser);
        return false;
    }

    if (options.version.GetValue()) {
        panda::PrintPandaVersion();
        panda::panda_file::PrintBytecodeVersion();
        return false;
    }

    if (options.inputFile.GetValue().empty() || options.outputFile.GetValue().empty() || options.help.GetValue()) {
        PrintHelp(paParser);
        return false;
    }

    if (options.debug.GetValue()) {
        auto debugFile = options.debugFile.GetValue();
        if (debugFile.empty()) {
            panda::Logger::InitializeStdLogging(
                panda::Logger::Level::DEBUG,
                panda::Logger::ComponentMask().set(panda::Logger::Component::DISASSEMBLER));
        } else {
            panda::Logger::InitializeFileLogging(
                debugFile, panda::Logger::Level::DEBUG,
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
    panda::PandArgParser paParser;
    Options options {paParser};

    if (!ProcessArgs(paParser, options, argc, argv)) {
        return 1;
    }

    Disassemble(options);

    paParser.DisableTail();

    return 0;
}
