/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <iostream>

#include "libpandafile/file.h"
#include "libpandafile/file_reader.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/pandargs.h"
#include "libpandafile/file_writer.h"
#include "libpandabase/utils/utf.h"
#include "runtime/include/mem/panda_string.h"
#include "header_writer.h"

static void PrintHelp(ark::PandArgParser &paParser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "arkts_header [options] INPUT_FILE OUTPUT_FILE" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << paParser.GetHelpString() << std::endl;
}

static bool ProcessArgs(ark::PandArgParser &paParser, const ark::PandArg<std::string> &input,
                        ark::PandArg<std::string> &output, const ark::PandArg<bool> &help)
{
    if (input.GetValue().empty() || help.GetValue()) {
        PrintHelp(paParser);
        return false;
    }

    if (output.GetValue().empty()) {
        std::string outputFilename = input.GetValue().substr(0, input.GetValue().find_last_of('.')) + ".h";
        output.SetValue(outputFilename);
    }

    ark::Logger::InitializeStdLogging(ark::Logger::Level::ERROR,
                                      ark::Logger::ComponentMask().set(ark::Logger::Component::ETS_NAPI));

    return true;
}

int main(int argc, const char **argv)
{
    ark::PandArg<bool> help("help", false, "Print this message and exit");
    ark::PandArg<std::string> input("INPUT", "", "Input binary file");
    ark::PandArg<std::string> output("OUTPUT", "", "Output header file");

    ark::PandArgParser paParser;

    paParser.Add(&help);
    paParser.PushBackTail(&input);
    paParser.PushBackTail(&output);
    paParser.EnableTail();

    if (!paParser.Parse(argc, argv)) {
        PrintHelp(paParser);
        return 1;
    }

    if (!ProcessArgs(paParser, input, output, help)) {
        return 1;
    }

    auto inputFile = ark::panda_file::File::Open(input.GetValue());
    if (!inputFile) {
        LOG(ERROR, ETS_NAPI) << "Cannot open file '" << input.GetValue() << "'";
        return 1;
    }

    ark::ets::header_writer::HeaderWriter writer(std::move(inputFile), output.GetValue());

    auto createdHeader = writer.PrintFunction();
    if (!createdHeader) {
        std::cout << "No native functions found in file '" << input.GetValue() << "', header not created" << std::endl;
    }

    return 0;
}
