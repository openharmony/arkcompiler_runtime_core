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

#include <mutex>
#include "codegen.h"
#include "disassembly.h"

namespace panda::compiler {

// clang-format off
static constexpr std::array INDENT_STRINGS = {
    "  ",
    "    ",
    "      ",
    "        ",
    "          ",
    "            ",
    "              ",
    "                ",
    "                  ",
};
// clang-format on

static const char *GetIndent(uint32_t depth)
{
    return INDENT_STRINGS[depth];
}

static constexpr auto StreamDeleter = [](std::ostream *stream) {
    if (!options.IsCompilerDisasmDumpStdout()) {
        delete stream;
    }
};

Disassembly::Disassembly(const Codegen *codegen)
    : codegen_(codegen),
      encoder_(codegen->GetEncoder()),
      stream_(nullptr, StreamDeleter),
      is_enabled_(options.IsCompilerDisasmDump()),
      is_code_enabled_(is_enabled_ && options.IsCompilerDisasmDumpCode())
{
}

void Disassembly::Init()
{
    if (!IsEnabled()) {
        return;
    }
    if (options.IsCompilerDisasmDumpStdout()) {
        stream_.reset(&std::cout);
    } else if (options.IsCompilerDisasmDumpSingleFile()) {
        auto stm = new std::ofstream;
        if (stm == nullptr) {
            UNREACHABLE();
        }
        static std::once_flag flag;
        auto file_name = options.GetCompilerDisasmDumpFileName();
        std::call_once(flag, [&file_name]() { std::remove(file_name.c_str()); });
        stm->open(file_name, std::ios_base::app);
        if (!stm->is_open()) {
            LOG(FATAL, COMPILER) << "Cannot open 'disasm.txt'";
        }
        stream_.reset(stm);
    } else {
        auto stm = new std::ofstream;
        if (stm == nullptr) {
            UNREACHABLE();
        }
        std::stringstream ss;
        auto graph = codegen_->GetGraph();
        auto exec_num = graph->GetPassManager()->GetExecutionCounter();
        ss << "disasm_" << exec_num << '_' << codegen_->GetRuntime()->GetClassNameFromMethod(graph->GetMethod()) << '_'
           << codegen_->GetRuntime()->GetMethodName(graph->GetMethod()) << (graph->IsOsrMode() ? "_osr" : "") << ".txt";
        stm->open(ss.str());
        if (!stm->is_open()) {
            LOG(FATAL, COMPILER) << "Cannot open '" << ss.str() << "'";
        }
        stream_.reset(stm);
    }
}

static void PrintChapter(std::ostream &stream, const char *name)
{
    stream << name << ":\n";
}

void Disassembly::IncreaseDepth()
{
    if ((depth_ + 1) < INDENT_STRINGS.size()) {
        depth_++;
    }
}

void Disassembly::PrintMethodEntry(const Codegen *codegen)
{
    static constexpr const char *indent = "  ";
    auto &stream = GetStream();
    auto graph = codegen->GetGraph();
    stream << "======================================================================\n";
    PrintChapter(stream, "METHOD_INFO");
    stream << indent << "name: " << codegen->GetRuntime()->GetMethodFullName(graph->GetMethod(), true) << std::endl;
    stream << indent << "mode: ";
    graph->GetMode().Dump(stream);
    stream << std::endl;
    stream << indent << "id: " << codegen->GetRuntime()->GetMethodId(graph->GetMethod()) << std::endl;
    if (graph->IsAotMode()) {
        stream << indent << "code_offset: " << reinterpret_cast<void *>(graph->GetAotData()->GetCodeOffset())
               << std::endl;
    }

    auto arch = codegen->GetArch();
    auto frame = codegen->GetFrameInfo();
    stream << indent << "frame_size: " << frame->GetFrameSize() << std::endl;
    stream << indent << "spills_count: " << frame->GetSpillsCount() << std::endl;
    stream << indent << "Callees:   " << (frame->GetCalleesRelativeFp() ? "fp" : "sp") << std::showpos
           << frame->GetCalleesOffset() << std::noshowpos << " (" << GetCalleeRegsCount(arch, false) << ")"
           << std::endl;
    stream << indent << "FpCallees: " << (frame->GetCalleesRelativeFp() ? "fp" : "sp") << std::showpos
           << frame->GetFpCalleesOffset() << std::noshowpos << " (" << GetCalleeRegsCount(arch, true) << ")"
           << std::endl;
    stream << indent << "Callers:   " << (frame->GetCallersRelativeFp() ? "fp" : "sp") << std::showpos
           << frame->GetCallersOffset() << std::noshowpos << " (" << GetCallerRegsCount(arch, false) << ")"
           << std::endl;
    stream << indent << "FpCallers: " << (frame->GetCallersRelativeFp() ? "fp" : "sp") << std::showpos
           << frame->GetFpCallersOffset() << std::noshowpos << " (" << GetCallerRegsCount(arch, true) << ")"
           << std::endl;
    if (IsCodeEnabled()) {
        PrintChapter(stream, "DISASSEMBLY");
    }
}

void Disassembly::PrintCodeInfo(const Codegen *codegen)
{
    auto &stream = GetStream();
    auto graph = codegen->GetGraph();

    CodeInfo code_info;
    ASSERT(!graph->GetCodeInfoData().empty());
    code_info.Decode(graph->GetCodeInfoData());
    PrintChapter(stream, "CODE_INFO");
    code_info.Dump(stream);
}

void Disassembly::PrintCodeStatistics(const Codegen *codegen)
{
    auto &stream = GetStream();
    auto graph = codegen->GetGraph();

    PrintChapter(stream, "CODE_STATS");
    stream << "  code_size: " << std::dec << graph->GetData().Size() << std::endl;
}

void Disassembly::FlushDisasm([[maybe_unused]] const Codegen *codegen)
{
    auto encoder = GetEncoder();
    auto &stream = GetStream();
    for (size_t pc = GetPosition(); pc < (encoder->GetCursorOffset());) {
        stream << GetIndent(GetDepth());
        auto new_pc = encoder->DisasmInstr(stream, pc, 0);
        stream << std::endl;
        pc = new_pc;
    }
    SetPosition(encoder->GetCursorOffset());
}

void Disassembly::PrintStackMap(const Codegen *codegen)
{
    FlushDisasm(codegen);
    auto &stream = GetStream();
    stream << GetIndent(GetDepth());
    codegen->GetCodeBuilder()->DumpCurrentStackMap(stream);
    stream << std::endl;
}

ScopedDisasmPrinter::ScopedDisasmPrinter(Codegen *codegen, const Inst *inst) : disasm_(codegen->GetDisasm())
{
    if (disasm_->IsCodeEnabled()) {
        disasm_->FlushDisasm(codegen);
        disasm_->GetStream() << GetIndent(disasm_->GetDepth()) << "# [inst] " << *inst << std::endl;
        disasm_->IncreaseDepth();
    }
}

ScopedDisasmPrinter::ScopedDisasmPrinter(Codegen *codegen, const std::string &msg) : disasm_(codegen->GetDisasm())
{
    if (disasm_->IsCodeEnabled()) {
        disasm_->FlushDisasm(codegen);
        disasm_->GetStream() << GetIndent(disasm_->GetDepth()) << "# " << msg << std::endl;
        disasm_->IncreaseDepth();
    }
}

ScopedDisasmPrinter::~ScopedDisasmPrinter()
{
    if (disasm_->IsCodeEnabled()) {
        disasm_->FlushDisasm(nullptr);
        disasm_->DecreaseDepth();
    }
}

}  // namespace panda::compiler
