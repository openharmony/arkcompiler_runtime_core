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

#include "aot_manager.h"
#include "aotdump_options.h"
#include "class_data_accessor.h"
#include "file.h"
#include "file-inl.h"
#include "mem/arena_allocator.h"
#include "mem/gc/gc_types.h"
#include "mem/pool_manager.h"
#include "method_data_accessor.h"
#include "method_data_accessor-inl.h"
#include "proto_data_accessor.h"
#include "proto_data_accessor-inl.h"
#include "runtime/include/class_helper.h"
#include "utils/arch.h"
#include "utils/bit_memory_region-inl.h"

#ifdef PANDA_COMPILER_TARGET_AARCH64
#include "aarch64/disasm-aarch64.h"
using vixl::aarch64::Decoder;
using vixl::aarch64::Disassembler;
using vixl::aarch64::Instruction;
#endif  // PANDA_COMPILER_TARGET_AARCH64
#ifdef PANDA_COMPILER_TARGET_X86_64
#include "Zydis/Zydis.h"
#endif  // PANDA_COMPILER_TARGET_X86_64

#include <fstream>
#include <iomanip>
#include <elf.h>
#include <regex>

using namespace panda::compiler;

namespace panda::aoutdump {

class TypePrinter {
public:
    explicit TypePrinter(panda_file::ProtoDataAccessor &pda, std::ostream &out) : pda_(pda), out_(out) {}
    NO_COPY_SEMANTIC(TypePrinter);
    NO_MOVE_SEMANTIC(TypePrinter);
    ~TypePrinter() = default;

    void Dump(panda_file::Type type)
    {
        if (!type.IsReference()) {
            out_ << type;
        } else {
            out_ << ClassHelper::GetName(pda_.GetPandaFile().GetStringData(pda_.GetReferenceType(ref_idx_++)).data);
        }
    }

private:
    panda_file::ProtoDataAccessor &pda_;
    std::ostream &out_;
    uint32_t ref_idx_ {0};
};

class PandaFileHelper {
public:
    explicit PandaFileHelper(const char *file_name) : file_(panda_file::OpenPandaFile(file_name)) {}
    NO_COPY_SEMANTIC(PandaFileHelper);
    NO_MOVE_SEMANTIC(PandaFileHelper);
    ~PandaFileHelper() = default;

    std::string GetMethodName(uint32_t id) const
    {
        if (!file_) {
            return "-";
        }
        auto file_id = panda_file::File::EntityId(id);
        panda_file::MethodDataAccessor mda(*file_, file_id);
        panda_file::ProtoDataAccessor pda(*file_, panda_file::MethodDataAccessor::GetProtoId(*file_, file_id));
        std::ostringstream ss;
        TypePrinter type_printer(pda, ss);

        type_printer.Dump(pda.GetReturnType());
        ss << ' ';

        auto class_name = ClassHelper::GetName(file_->GetStringData(mda.GetClassId()).data);
        ss << class_name << "::" << file_->GetStringData(mda.GetNameId()).data;

        ss << '(';
        bool first_arg = true;
        // inject class name as the first argument of non static method for consitency with panda::Method::GetFullName
        if (!mda.IsStatic()) {
            first_arg = false;
            ss << class_name;
        }
        for (uint32_t arg_idx = 0; arg_idx < pda.GetNumArgs(); ++arg_idx) {
            if (!first_arg) {
                ss << ", ";
            }
            first_arg = false;
            type_printer.Dump(pda.GetArgType(arg_idx));
        }
        ss << ')';
        return ss.str();
    }

    std::string GetClassName(uint32_t id) const
    {
        if (!file_) {
            return "-";
        }
        return ClassHelper::GetName(file_->GetStringData(panda_file::File::EntityId(id)).data);
    }

private:
    std::unique_ptr<const panda_file::File> file_;
};

class AotDump {
public:
    explicit AotDump(panda::ArenaAllocator *allocator) : allocator_(allocator) {}
    NO_COPY_SEMANTIC(AotDump);
    NO_MOVE_SEMANTIC(AotDump);
    ~AotDump() = default;

    int Run(int argc, const char *argv[])
    {
        panda::PandArgParser pa_parser;
        PandArg<std::string> input_file {"input_file", "", "Input file path"};
        pa_parser.EnableTail();
        pa_parser.PushBackTail(&input_file);
        std::array<char, NAME_MAX> tmpfile_buf {"/tmp/fixed_aot_XXXXXX"};
        // Remove temporary file at the function exit
        auto finalizer = [](const char *file_name) {
            if (file_name != nullptr) {
                remove(file_name);
            }
        };
        std::unique_ptr<const char, decltype(finalizer)> temp_file_remover(nullptr, finalizer);

        panda::Span<const char *> sp(argv, argc);
        panda::aoutdump::Options options(sp[0]);
        options.AddOptions(&pa_parser);
        if (!pa_parser.Parse(argc, argv)) {
            std::cerr << "Parse options failed: " << pa_parser.GetErrorString() << std::endl;
            return -1;
        }
        if (input_file.GetValue().empty()) {
            std::cerr << "Please specify input file\n";
            return -1;
        }
        Expected<std::unique_ptr<AotFile>, std::string> aot_res;
        // Fix elf header for cross platform files. Cross opening is available only in X86_64 arch.
        if (RUNTIME_ARCH == Arch::X86_64) {
            if (!FixElfHeader(tmpfile_buf, input_file)) {
                return -1;
            }
            temp_file_remover.reset(tmpfile_buf.data());
            aot_res = AotFile::Open(tmpfile_buf.data(), 0, true);
        } else {
            aot_res = AotFile::Open(input_file.GetValue(), 0, true);
        }
        if (!aot_res) {
            std::cerr << "Open AOT file failed: " << aot_res.Error() << std::endl;
            return -1;
        }
        auto aot_file = std::move(aot_res.Value());
        std::ostream *output_stream;
        std::ofstream out_fstream;
        if (options.WasSetOutputFile()) {
            out_fstream.open(options.GetOutputFile());
            output_stream = &out_fstream;
        } else {
            output_stream = &std::cerr;
        }
        auto &stream = *output_stream;
        DumpHeader(stream, aot_file);
        DumpFiles(stream, aot_file, options);
        return 0;
    }

    bool FixElfHeader(std::array<char, NAME_MAX> &tmpfile_buf, PandArg<std::string> &input_file)
    {
        int fd = mkstemp(tmpfile_buf.data());
        if (fd == -1) {
            std::cerr << "Failed to open temporary file\n";
            return false;
        }
        close(fd);
        std::ofstream ostm(tmpfile_buf.data(), std::ios::binary);
        std::ifstream istm(input_file.GetValue(), std::ios::binary);
        if (!ostm.is_open() || !istm.is_open()) {
            std::cerr << "Cannot open tmpfile or input file\n";
            return false;
        }
        std::vector<char> buffer(std::istreambuf_iterator<char>(istm), {});
        Elf64_Ehdr *header = reinterpret_cast<Elf64_Ehdr *>(buffer.data());
        header->e_machine = EM_X86_64;
        ostm.write(buffer.data(), buffer.size());
        return true;
    }

    void DumpHeader(std::ostream &stream, std::unique_ptr<panda::compiler::AotFile> &aot_file)
    {
        auto aot_header = aot_file->GetAotHeader();
        stream << "header:" << std::endl;
        stream << "  magic: " << aot_header->magic.data() << std::endl;
        stream << "  version: " << aot_header->version.data() << std::endl;
        stream << "  filename: " << aot_file->GetFileName() << std::endl;
        stream << "  cmdline: " << aot_file->GetCommandLine() << std::endl;
        stream << "  checksum: " << aot_header->checksum << std::endl;
        stream << "  env checksum: " << aot_header->environment_checksum << std::endl;
        stream << "  arch: " << GetArchString(static_cast<Arch>(aot_header->arch)) << std::endl;
        stream << "  gc_type: " << mem::GCStringFromType(static_cast<mem::GCType>(aot_header->gc_type)) << std::endl;
        stream << "  files_count: " << aot_header->files_count << std::endl;
        stream << "  files_offset: " << aot_header->files_offset << std::endl;
        stream << "  classes_offset: " << aot_header->classes_offset << std::endl;
        stream << "  methods_offset: " << aot_header->methods_offset << std::endl;
        stream << "  bitmap_offset: " << aot_header->bitmap_offset << std::endl;
        stream << "  strtab_offset: " << aot_header->strtab_offset << std::endl;
    }

    void DumpClassHashTable(std::ostream &stream, panda::compiler::AotPandaFile &aot_panda_file, PandaFileHelper &pfile)
    {
        stream << "  class_hash_table:" << std::endl;
        constexpr int ALIGN_SIZE = 32;
        stream << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << "i" << std::left << std::setfill(' ')
               << std::setw(ALIGN_SIZE) << "next_pos";
        stream << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << "entity_id_offset" << std::left
               << std::setfill(' ') << std::setw(ALIGN_SIZE) << "descriptor" << std::endl;
        auto class_hash_table = aot_panda_file.GetClassHashTable();
        auto hash_table_size = class_hash_table.size();
        for (size_t i = 0; i < hash_table_size; i++) {
            auto entity_pair = class_hash_table[i];
            if (entity_pair.descriptor_hash != 0) {
                auto descriptor = pfile.GetClassName(entity_pair.entity_id_offset);
                stream << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << (i + 1);
                stream << std::left << std::setfill(' ') << std::dec << std::setw(ALIGN_SIZE) << entity_pair.next_pos;
                stream << std::left << std::setfill(' ') << std::dec << std::setw(ALIGN_SIZE)
                       << entity_pair.entity_id_offset;
                stream << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << descriptor << std::endl;
            } else {
                stream << std::left << std::setfill(' ') << std::setw(ALIGN_SIZE) << (i + 1) << std::endl;
            }
        }
    }

    void DumpFiles(std::ostream &stream, std::unique_ptr<panda::compiler::AotFile> &aot_file,
                   panda::aoutdump::Options &options)
    {
        stream << "files:" << std::endl;
        for (decltype(auto) file_header : aot_file->FileHeaders()) {
            AotPandaFile aot_panda_file(aot_file.get(), &file_header);
            auto file_name = aot_file->GetString(file_header.file_name_str);
            PandaFileHelper pfile(file_name);
            stream << "- name: " << file_name << std::endl;
            stream << "  classes:\n";
            for (decltype(auto) class_header : aot_panda_file.GetClassHeaders()) {
                AotClass klass(aot_file.get(), &class_header);
                stream << "  - class_id: " << class_header.class_id << std::endl;
                stream << "    name: " << pfile.GetClassName(class_header.class_id) << std::endl;
                auto methods_bitmap = klass.GetBitmap();
                BitMemoryRegion rgn(methods_bitmap.data(), methods_bitmap.size());
                stream << "    methods_bitmap: " << rgn << std::endl;
                stream << "    methods:\n";
                for (decltype(auto) method_header : klass.GetMethodHeaders()) {
                    auto method_name = pfile.GetMethodName(method_header.method_id);
                    if (options.WasSetMethodRegex() && !method_name.empty()) {
                        static std::regex rgx(options.GetMethodRegex());
                        if (!std::regex_match(method_name, rgx)) {
                            continue;
                        }
                    }
                    stream << "    - id: " << std::dec << method_header.method_id << std::endl;
                    stream << "      name: " << method_name << std::endl;
                    stream << "      code_offset: 0x" << std::hex << method_header.code_offset << std::dec << std::endl;
                    stream << "      code_size: " << method_header.code_size << std::endl;
                    auto code_info = aot_panda_file.GetMethodCodeInfo(&method_header);
                    if (options.GetShowCode() == "disasm") {
                        stream << "      code: |\n";
                        PrintCode("        ", *aot_file, code_info, stream, pfile);
                    }
                }
            }
            DumpClassHashTable(stream, aot_panda_file, pfile);
        }
    }

    void PrintCode(const char *prefix, const AotFile &aot_file, const CodeInfo &code_info, std::ostream &stream,
                   const PandaFileHelper &pfile) const
    {
        Arch arch = static_cast<Arch>(aot_file.GetAotHeader()->arch);
        switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH64
            case Arch::AARCH64:
                return PrintCodeArm64(prefix, code_info, stream, pfile);
#endif  // PANDA_COMPILER_TARGET_AARCH64
#ifdef PANDA_COMPILER_TARGET_X86_64
            case Arch::X86_64:
                return PrintCodeX86_64(prefix, code_info, stream, pfile);
#endif  // PANDA_COMPILER_TARGET_X86_64
            default:
                stream << prefix << "Unsupported target arch: " << GetArchString(arch) << std::endl;
                break;
        }
    }

#ifdef PANDA_COMPILER_TARGET_AARCH64
    void PrintCodeArm64(const char *prefix, const CodeInfo &code_info, std::ostream &stream,
                        const PandaFileHelper &pfile) const
    {
        Span<const uint8_t> code = code_info.GetCodeSpan();

        Decoder decoder(allocator_);
        Disassembler disasm(allocator_);
        decoder.AppendVisitor(&disasm);
        auto start_instr = reinterpret_cast<const Instruction *>(code.data());
        auto end_instr = reinterpret_cast<const Instruction *>(code.end());
        uint32_t pc = 0;

        for (auto instr = start_instr; instr < end_instr; instr += vixl::aarch64::kInstructionSize) {
            auto stackmap = code_info.FindStackMapForNativePc(pc, Arch::AARCH64);
            if (stackmap.IsValid()) {
                PrintStackmap(stream, prefix, code_info, stackmap, Arch::AARCH64, pfile);
            }
            decoder.Decode(instr);
            stream << prefix << std::hex << std::setw(8U) << std::setfill('0') << instr - start_instr << ": "
                   << disasm.GetOutput() << std::endl;
            pc += vixl::aarch64::kInstructionSize;
        }
        stream << std::dec;
    }
#endif  // PANDA_COMPILER_TARGET_AARCH64
#ifdef PANDA_COMPILER_TARGET_X86_64
    void PrintCodeX86_64(const char *prefix, const CodeInfo &code_info, std::ostream &stream,
                         const PandaFileHelper &pfile) const
    {
        Span<const uint8_t> code = code_info.GetCodeSpan();
        constexpr size_t LENGTH = ZYDIS_MAX_INSTRUCTION_LENGTH;  // 15 bytes is max inst length in amd64
        size_t code_size = code.size();

        // Initialize decoder context
        ZydisDecoder decoder;
        if (!ZYAN_SUCCESS(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64))) {
            LOG(FATAL, AOT) << "ZydisDecoderInit failed";
        }

        // Initialize formatter
        ZydisFormatter formatter;
        if (!ZYAN_SUCCESS(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL))) {
            LOG(FATAL, AOT) << "ZydisFormatterInit failed";
        }

        for (size_t pos = 0; pos < code_size;) {
            ZydisDecodedInstruction instruction;
            constexpr auto BUF_SIZE = 256;
            std::array<char, BUF_SIZE> buffer;
            auto len = std::min(LENGTH, code_size - pos);
            if (!ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, &code[pos], len, &instruction))) {
                LOG(FATAL, AOT) << "ZydisDecoderDecodeBuffer failed";
            }
            if (!ZYAN_SUCCESS(ZydisFormatterFormatInstruction(&formatter, &instruction, buffer.data(), buffer.size(),
                                                              uintptr_t(&code[pos])))) {
                LOG(FATAL, AOT) << "ZydisFormatterFormatInstruction failed";
            }
            auto stackmap = code_info.FindStackMapForNativePc(pos, Arch::X86_64);
            if (stackmap.IsValid()) {
                PrintStackmap(stream, prefix, code_info, stackmap, Arch::X86_64, pfile);
            }
            stream << prefix << std::hex << std::setw(8U) << std::setfill('0') << pos << ": " << buffer.data()
                   << std::endl;
            pos += instruction.length;
        }
    }
#endif  // PANDA_COMPILER_TARGET_X86_64

    void PrintStackmap(std::ostream &stream, const char *prefix, const CodeInfo &code_info, const StackMap &stackmap,
                       Arch arch, const PandaFileHelper &pfile) const
    {
        stream << prefix << "          ";
        code_info.Dump(stream, stackmap, arch);
        stream << std::endl;
        if (stackmap.HasInlineInfoIndex()) {
            for (auto ii : const_cast<CodeInfo &>(code_info).GetInlineInfos(stackmap)) {
                stream << prefix << "          ";
                code_info.DumpInlineInfo(stream, stackmap, ii.GetRow() - stackmap.GetInlineInfoIndex());
                auto id =
                    const_cast<CodeInfo &>(code_info).GetMethod(stackmap, ii.GetRow() - stackmap.GetInlineInfoIndex());
                stream << ", method: " << pfile.GetMethodName(std::get<uint32_t>(id));
                stream << std::endl;
            }
        }
    }

private:
    [[maybe_unused]] panda::ArenaAllocator *allocator_;
};

}  // namespace panda::aoutdump

int main(int argc, const char *argv[])
{
    panda::mem::MemConfig::Initialize(panda::operator""_MB(64ULL), panda::operator""_MB(64ULL),
                                      panda::operator""_MB(64ULL), panda::operator""_MB(32ULL));
    panda::PoolManager::Initialize();
    auto allocator = new panda::ArenaAllocator(panda::SpaceType::SPACE_TYPE_COMPILER);
    panda::aoutdump::AotDump aotdump(allocator);

    auto result = aotdump.Run(argc, argv);

    delete allocator;
    panda::PoolManager::Finalize();
    panda::mem::MemConfig::Finalize();
    return result;
}
