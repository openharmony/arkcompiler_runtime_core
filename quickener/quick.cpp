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

#include <iostream>

#include "quick.h"

#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/pandargs.h"
#include "libpandafile/file_writer.h"

namespace panda {
void PrintHelp(panda::PandArgParser &pa_parser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "arkquicker [options] INPUT_FILE OUTPUT_FILE" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << pa_parser.GetHelpString() << std::endl;
}

bool ProcessArgs(panda::PandArgParser &pa_parser, const panda::PandArg<std::string> &input,
                 const panda::PandArg<std::string> &output, const panda::PandArg<bool> &help, int argc,
                 const char **argv)
{
    if (!pa_parser.Parse(argc, argv)) {
        PrintHelp(pa_parser);
        return false;
    }

    if (input.GetValue().empty() || output.GetValue().empty() || help.GetValue()) {
        PrintHelp(pa_parser);
        return false;
    }

    panda::Logger::InitializeStdLogging(
        panda::Logger::Level::ERROR, panda::Logger::ComponentMask().set(
            panda::Logger::Component::QUICKENER).set(panda::Logger::Component::PANDAFILE));

    return true;
}

int main(int argc, const char **argv)
{
    panda::PandArg<bool> help("help", false, "Print this message and exit");
    panda::PandArg<std::string> input("INPUT", "", "Path to the input binary file");
    panda::PandArg<std::string> output("OUTPUT", "", "Path to the output binary file");

    panda::PandArgParser pa_parser;

    pa_parser.Add(&help);
    pa_parser.PushBackTail(&input);
    pa_parser.PushBackTail(&output);
    pa_parser.EnableTail();

    if (!ProcessArgs(pa_parser, input, output, help, argc, argv)) {
        return 1;
    }

    auto input_file = panda::panda_file::File::Open(input.GetValue());
    if (!input_file) {
        LOG(ERROR, QUICKENER) << "Cannot open file '" << input.GetValue() << "'";
        return 1;
    }
    panda::panda_file::FileReader reader(std::move(input_file));
    if (!reader.ReadContainer()) {
        LOG(ERROR, QUICKENER) << "Cannot read container";
        return 1;
    }

    panda::panda_file::ItemContainer *container = reader.GetContainerPtr();
    auto writer = panda::panda_file::FileWriter(output.GetValue());
    if (!writer) {
        PLOG(ERROR, QUICKENER) << "Cannot create file writer with path '" << output.GetValue() << "'";
        return 1;
    }

    if (!container->Write(&writer, false)) {
        PLOG(ERROR, QUICKENER) << "Cannot write panda file '" << output.GetValue() << "'";
        return 1;
    }

    return 0;
}
}  // namespace panda
