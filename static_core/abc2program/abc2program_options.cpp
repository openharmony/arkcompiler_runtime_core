/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "abc2program_options.h"
#include <sstream>

namespace ark::abc2program {

bool Abc2ProgramOptions::Parse(int argc, const char **argv)
{
    ark::PandArg<bool> help("help", false, "Print this message and exit");
    ark::PandArg<bool> debug(
        "debug", false, "enable debug messages (will be printed to standard output if no --debug-file was specified) ");
    ark::PandArg<std::string> debugFile("debug-file", "",
                                        "(--debug-file FILENAME) set debug file name. default is std::cout");
    ark::PandArg<std::string> inputFile("inputFile", "", "Path to the source binary code");
    ark::PandArg<std::string> outputFile("outputFile", "", "Path to the generated assembly code");
    helpArg_ = &help;
    debugArg_ = &debug;
    debugFileArg_ = &debugFile;
    inputFileArg_ = &inputFile;
    outputFileArg_ = &outputFile;
    paParser_.Add(&help);
    paParser_.Add(&debug);
    paParser_.Add(&debugFile);
    paParser_.PushBackTail(&inputFile);
    paParser_.PushBackTail(&outputFile);
    paParser_.EnableTail();
    if (!ProcessArgs(argc, argv)) {
        PrintErrorMsg();
        paParser_.DisableTail();
        return false;
    }
    paParser_.DisableTail();
    return true;
}

bool Abc2ProgramOptions::ProcessArgs(int argc, const char **argv)
{
    if (!paParser_.Parse(argc, argv)) {
        ConstructErrorMsg();
        return false;
    }
    if (debugArg_->GetValue()) {
        if (debugFileArg_->GetValue().empty()) {
            ark::Logger::InitializeStdLogging(ark::Logger::Level::DEBUG,
                                              ark::Logger::ComponentMask().set(ark::Logger::Component::ABC2PROGRAM));
        } else {
            ark::Logger::InitializeFileLogging(debugFileArg_->GetValue(), ark::Logger::Level::DEBUG,
                                               ark::Logger::ComponentMask().set(ark::Logger::Component::ABC2PROGRAM));
        }
    } else {
        ark::Logger::InitializeStdLogging(ark::Logger::Level::ERROR,
                                          ark::Logger::ComponentMask().set(ark::Logger::Component::ABC2PROGRAM));
    }
    inputFilePath_ = inputFileArg_->GetValue();
    outputFilePath_ = outputFileArg_->GetValue();
    if (inputFilePath_.empty() || outputFilePath_.empty()) {
        ConstructErrorMsg();
        return false;
    }
    return true;
}

void Abc2ProgramOptions::ConstructErrorMsg()
{
    std::stringstream ss;
    ss << "Usage:" << std::endl;
    ss << "abc2prog [options] inputFile outputFile" << std::endl;
    ss << "Supported options:" << std::endl;
    ss << paParser_.GetHelpString() << std::endl;
    errorMsg_ = ss.str();
}

const std::string &Abc2ProgramOptions::GetInputFilePath() const
{
    return inputFilePath_;
}

const std::string &Abc2ProgramOptions::GetOutputFilePath() const
{
    return outputFilePath_;
}

void Abc2ProgramOptions::PrintErrorMsg() const
{
    std::cerr << errorMsg_;
}

}  // namespace ark::abc2program
