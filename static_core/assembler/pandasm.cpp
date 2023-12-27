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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "ark_version.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"
#ifdef PANDA_WITH_BYTECODE_OPTIMIZER
#include "bytecode_optimizer/optimize_bytecode.h"
#endif
#include "file_format_version.h"
#include "error.h"
#include "lexer.h"
#include "pandasm.h"
#include "utils/expected.h"
#include "utils/logger.h"
#include "utils/pandargs.h"

namespace panda::pandasm {

void PrintError(const panda::pandasm::Error &e, const std::string &msg)
{
    std::stringstream sos;
    std::cerr << msg << ": " << e.message << std::endl;
    sos << "      Line " << e.lineNumber << ", Column " << (e.pos + 1) << ": ";
    std::cerr << sos.str() << e.wholeLine << std::endl;
    std::cerr << std::setw(static_cast<int>(e.pos + sos.str().size()) + 1) << "^" << std::endl;
}

void PrintErrors(const panda::pandasm::ErrorList &warnings, const std::string &msg)
{
    for (const auto &iter : warnings) {
        PrintError(iter, msg);
    }
}

void PrintHelp(const panda::PandArgParser &paParser)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "pandasm [OPTIONS] INPUT_FILE OUTPUT_FILE" << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << paParser.GetHelpString() << std::endl;
}

bool PrepareArgs(panda::PandArgParser &paParser, const panda::PandArg<std::string> &inputFile,
                 const panda::PandArg<std::string> &outputFile, const panda::PandArg<std::string> &logFile,
                 const panda::PandArg<bool> &help, const panda::PandArg<bool> &verbose,
                 const panda::PandArg<bool> &version, std::ifstream &inputfile, int argc, const char **argv)
{
    if (!paParser.Parse(argc, argv)) {
        PrintHelp(paParser);
        return false;
    }

    if (version.GetValue()) {
        panda::PrintPandaVersion();
        panda_file::PrintBytecodeVersion();
        return false;
    }

    if (inputFile.GetValue().empty() || outputFile.GetValue().empty() || help.GetValue()) {
        PrintHelp(paParser);
        return false;
    }

    if (verbose.GetValue()) {
        if (logFile.GetValue().empty()) {
            panda::Logger::ComponentMask componentMask;
            componentMask.set(panda::Logger::Component::ASSEMBLER);
            componentMask.set(panda::Logger::Component::BYTECODE_OPTIMIZER);
            panda::Logger::InitializeStdLogging(panda::Logger::Level::DEBUG, componentMask);
        } else {
            panda::Logger::ComponentMask componentMask;
            componentMask.set(panda::Logger::Component::ASSEMBLER);
            componentMask.set(panda::Logger::Component::BYTECODE_OPTIMIZER);
            panda::Logger::InitializeFileLogging(logFile.GetValue(), panda::Logger::Level::DEBUG, componentMask);
        }
    }

    inputfile.open(inputFile.GetValue(), std::ifstream::in);

    if (!inputfile) {
        std::cerr << "The input file does not exist." << std::endl;
        return false;
    }

    return true;
}

bool Tokenize(panda::pandasm::Lexer &lexer, std::vector<std::vector<panda::pandasm::Token>> &tokens,
              std::ifstream &inputfile)
{
    std::string s;

    while (getline(inputfile, s)) {
        panda::pandasm::Tokens q = lexer.TokenizeString(s);

        auto e = q.second;

        if (e.err != panda::pandasm::Error::ErrorType::ERR_NONE) {
            e.lineNumber = tokens.size() + 1;
            PrintError(e, "ERROR");
            return false;
        }

        tokens.push_back(q.first);
    }

    return true;
}

bool ParseProgram(panda::pandasm::Parser &parser, std::vector<std::vector<panda::pandasm::Token>> &tokens,
                  const panda::PandArg<std::string> &inputFile,
                  panda::Expected<panda::pandasm::Program, panda::pandasm::Error> &res)
{
    res = parser.Parse(tokens, inputFile.GetValue());
    if (!res) {
        PrintError(res.Error(), "ERROR");
        return false;
    }

    return true;
}

bool DumpProgramInJson(panda::pandasm::Program &program, const panda::PandArg<std::string> &scopesFile)
{
    if (!scopesFile.GetValue().empty()) {
        std::ofstream dumpFile;
        dumpFile.open(scopesFile.GetValue());

        if (!dumpFile) {
            std::cerr << "Cannot write scopes into the given file." << std::endl;
            return false;
        }
        dumpFile << program.JsonDump();
    }

    return true;
}

bool EmitProgramInBinary(panda::pandasm::Program &program, panda::PandArgParser &paParser,
                         const panda::PandArg<std::string> &outputFile, panda::PandArg<bool> &optimize,
                         panda::PandArg<bool> &sizeStat)
{
    auto emitDebugInfo = !optimize.GetValue();
    std::map<std::string, size_t> stat;
    std::map<std::string, size_t> *statp = sizeStat.GetValue() ? &stat : nullptr;
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps {};
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = optimize.GetValue() ? &maps : nullptr;

    if (!panda::pandasm::AsmEmitter::Emit(outputFile.GetValue(), program, statp, mapsp, emitDebugInfo)) {
        std::cerr << "Failed to emit binary data: " << panda::pandasm::AsmEmitter::GetLastError() << std::endl;
        return false;
    }

#ifdef PANDA_WITH_BYTECODE_OPTIMIZER
    if (optimize.GetValue()) {
        bool isOptimized = panda::bytecodeopt::OptimizeBytecode(&program, mapsp, outputFile.GetValue());
        if (!panda::pandasm::AsmEmitter::Emit(outputFile.GetValue(), program, statp, mapsp, emitDebugInfo)) {
            std::cerr << "Failed to emit binary data: " << panda::pandasm::AsmEmitter::GetLastError() << std::endl;
            return false;
        }
        if (!isOptimized) {
            std::cerr << "Bytecode optimizer reported internal errors" << std::endl;
            return false;
        }
    }
#endif

    if (sizeStat.GetValue()) {
        size_t totalSize = 0;
        std::cout << "Panda file size statistic:" << std::endl;

        for (auto [name, size] : stat) {
            std::cout << name << " section: " << size << std::endl;
            totalSize += size;
        }

        std::cout << "total: " << totalSize << std::endl;
    }

    paParser.DisableTail();

    return true;
}

bool BuildFiles(panda::pandasm::Program &program, panda::PandArgParser &paParser,
                const panda::PandArg<std::string> &outputFile, panda::PandArg<bool> &optimize,
                panda::PandArg<bool> &sizeStat, panda::PandArg<std::string> &scopesFile)
{
    if (!DumpProgramInJson(program, scopesFile)) {
        return false;
    }

    if (!EmitProgramInBinary(program, paParser, outputFile, optimize, sizeStat)) {
        return false;
    }

    return true;
}

}  // namespace panda::pandasm

int main(int argc, const char *argv[])
{
    panda::PandArg<bool> verbose("verbose", false, "Enable verbose output (will be printed to standard output)");
    panda::PandArg<std::string> logFile("log-file", "", "(--log-file FILENAME) Set log file name");
    panda::PandArg<std::string> scopesFile("dump-scopes", "", "(--dump-scopes FILENAME) Enable dump of scopes to file");
    panda::PandArg<bool> help("help", false, "Print this message and exit");
    panda::PandArg<bool> sizeStat("size-stat", false, "Print panda file size statistic");
    panda::PandArg<bool> optimize("optimize", false, "Run the bytecode optimization");
    panda::PandArg<bool> version {"version", false,
                                  "Ark version, file format version and minimum supported file format version"};
    // tail arguments
    panda::PandArg<std::string> inputFile("INPUT_FILE", "", "Path to the source assembly code");
    panda::PandArg<std::string> outputFile("OUTPUT_FILE", "", "Path to the generated binary code");
    panda::PandArgParser paParser;
    paParser.Add(&verbose);
    paParser.Add(&help);
    paParser.Add(&logFile);
    paParser.Add(&scopesFile);
    paParser.Add(&sizeStat);
    paParser.Add(&optimize);
    paParser.Add(&version);
    paParser.PushBackTail(&inputFile);
    paParser.PushBackTail(&outputFile);
    paParser.EnableTail();

    std::ifstream inputfile;

    if (!panda::pandasm::PrepareArgs(paParser, inputFile, outputFile, logFile, help, verbose, version, inputfile, argc,
                                     argv)) {
        return 1;
    }

    LOG(DEBUG, ASSEMBLER) << "Lexical analysis:";

    panda::pandasm::Lexer lexer;

    std::vector<std::vector<panda::pandasm::Token>> tokens;

    if (!Tokenize(lexer, tokens, inputfile)) {
        return 1;
    }

    LOG(DEBUG, ASSEMBLER) << "parsing:";

    panda::pandasm::Parser parser;

    panda::Expected<panda::pandasm::Program, panda::pandasm::Error> res;
    if (!panda::pandasm::ParseProgram(parser, tokens, inputFile, res)) {
        return 1;
    }

    auto &program = res.Value();

    auto w = parser.ShowWarnings();
    if (!w.empty()) {
        panda::pandasm::PrintErrors(w, "WARNING");
    }

    if (!panda::pandasm::BuildFiles(program, paParser, outputFile, optimize, sizeStat, scopesFile)) {
        return 1;
    }

    return 0;
}
