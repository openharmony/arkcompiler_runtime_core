/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

Abc2ProgramOptions::Abc2ProgramOptions()
    : helpArg_(std::string("help"), false, std::string("Print this message and exit")),
      debugArg_(
          std::string("debug"), false,
          std::string("enable debug messages (will be printed to standard output if no --debug-file was specified) ")),
      debugFileArg_(std::string("debug-file"), std::string(""),
                    std::string("(--debug-file FILENAME) set debug file name. default is std::cout")),
      inputFileArg_(std::string("inputFile"), std::string(""), std::string("Path to the source binary code")),
      outputFileArg_(std::string("outputFile"), std::string(""), std::string("Path to the generated assembly code")),
      listClassesArg_(std::string("list-classes"), false, std::string("list all classes in the abc file")),
      listMethodsArg_(std::string("list-methods"), false, std::string("list all methods in the abc file")),
      skeletonArg_(std::string("skeleton"), false, std::string("dump class and method skeleton without bytecode"))
{
    paParser_.Add(&helpArg_);
    paParser_.Add(&debugArg_);
    paParser_.Add(&debugFileArg_);
    paParser_.Add(&listClassesArg_);
    paParser_.Add(&listMethodsArg_);
    paParser_.Add(&skeletonArg_);
    paParser_.PushBackTail(&inputFileArg_);
    paParser_.PushBackTail(&outputFileArg_);
}

bool Abc2ProgramOptions::Parse(int argc, const char **argv)
{
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
        ConstructErrorMsg("failed to parse arguments");
        return false;
    }
    if (debugArg_.GetValue()) {
        if (debugFileArg_.GetValue().empty()) {
            ark::Logger::InitializeStdLogging(ark::Logger::Level::DEBUG,
                                              ark::Logger::ComponentMask().set(ark::Logger::Component::ABC2PROGRAM));
        } else {
            ark::Logger::InitializeFileLogging(debugFileArg_.GetValue(), ark::Logger::Level::DEBUG,
                                               ark::Logger::ComponentMask().set(ark::Logger::Component::ABC2PROGRAM));
        }
    } else {
        ark::Logger::InitializeStdLogging(ark::Logger::Level::ERROR,
                                          ark::Logger::ComponentMask().set(ark::Logger::Component::ABC2PROGRAM));
    }
    inputFilePath_ = inputFileArg_.GetValue();
    outputFilePath_ = outputFileArg_.GetValue();
    if (inputFilePath_.empty() || outputFilePath_.empty()) {
        ConstructErrorMsg("input file and output file must be specified");
        return false;
    }
    int modeCount =
        static_cast<int>(IsListClasses()) + static_cast<int>(IsListMethods()) + static_cast<int>(IsSkeleton());
    if (modeCount > 1) {
        ConstructErrorMsg("--skeleton, --list-classes, --list-methods are mutually exclusive");
        return false;
    }
    return true;
}

void Abc2ProgramOptions::ConstructErrorMsg(const std::string &error)
{
    std::stringstream ss;
    if (!error.empty()) {
        ss << "Error: " << error << std::endl;
    }
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

bool Abc2ProgramOptions::IsListClasses() const
{
    return listClassesArg_.GetValue();
}

bool Abc2ProgramOptions::IsListMethods() const
{
    return listMethodsArg_.GetValue();
}

bool Abc2ProgramOptions::IsSkeleton() const
{
    return skeletonArg_.GetValue();
}

void Abc2ProgramOptions::PrintErrorMsg() const
{
    std::cerr << errorMsg_;
}

}  // namespace ark::abc2program
