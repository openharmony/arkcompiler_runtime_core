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
#include "class_data_accessor.h"
#include "field_data_accessor.h"
#include "libpandafile/type_helper.h"
#include "mangling.h"
#include "utils/logger.h"

#include <iomanip>

#include "get_language_specific_metadata.inc"

namespace panda::disasm {

void Disassembler::Disassemble(std::string_view filename_in, const bool quiet, const bool skip_strings)
{
    auto file = panda_file::File::Open(filename_in);
    if (file == nullptr) {
        LOG(FATAL, DISASSEMBLER) << "> unable to open specified pandafile: <" << filename_in << ">";
    }

    Disassemble(file, quiet, skip_strings);
}

void Disassembler::Disassemble(const panda_file::File &file, const bool quiet, const bool skip_strings)
{
    SetFile(file);
    DisassembleImpl(quiet, skip_strings);
}

void Disassembler::Disassemble(std::unique_ptr<const panda_file::File> &file, const bool quiet, const bool skip_strings)
{
    SetFile(file);
    DisassembleImpl(quiet, skip_strings);
}

void Disassembler::DisassembleImpl(const bool quiet, const bool skip_strings)
{
    prog_ = pandasm::Program {};

    record_name_to_id_.clear();
    method_name_to_id_.clear();

    skip_strings_ = skip_strings;
    quiet_ = quiet;

    prog_info_ = ProgInfo {};

    prog_ann_ = ProgAnnotations {};

    GetLiteralArrays();
    GetRecords();

    AddExternalFieldsToRecords();
    GetLanguageSpecificMetadata();
}

void Disassembler::SetFile(std::unique_ptr<const panda_file::File> &file)
{
    file_holder_.swap(file);
    file_ = file_holder_.get();
}

void Disassembler::SetFile(const panda_file::File &file)
{
    file_holder_.reset();
    file_ = &file;
}

void Disassembler::SetProfile(std::string_view fname)
{
    std::ifstream stm(fname.data(), std::ios::binary);
    if (!stm.is_open()) {
        LOG(FATAL, DISASSEMBLER) << "Cannot open profile file";
    }

    auto res = profiling::ReadProfile(stm, file_language_);
    if (!res) {
        LOG(FATAL, DISASSEMBLER) << "Failed to deserialize: " << res.Error();
    }
    profile_ = res.Value();
}

void Disassembler::GetInsInfo(panda_file::MethodDataAccessor &mda, const panda_file::File::EntityId &code_id,
                              MethodInfo *method_info /* out */) const
{
    const static size_t FORMAT_WIDTH = 20;
    const static size_t INSTRUCTION_WIDTH = 2;

    panda_file::CodeDataAccessor code_accessor(*file_, code_id);

    std::string method_name = mda.GetFullName();
    auto prof = profiling::INVALID_PROFILE;
    if (profile_ != profiling::INVALID_PROFILE) {
        prof = profiling::FindMethodInProfile(profile_, file_language_, method_name);
    }

    auto ins_sz = code_accessor.GetCodeSize();
    auto ins_arr = code_accessor.GetInstructions();

    auto bc_ins = BytecodeInstruction(ins_arr);
    auto bc_ins_last = bc_ins.JumpTo(ins_sz);

    while (bc_ins.GetAddress() != bc_ins_last.GetAddress()) {
        std::stringstream ss;

        uintptr_t bc = bc_ins.GetAddress() - BytecodeInstruction(ins_arr).GetAddress();
        ss << "offset: 0x" << std::setfill('0') << std::setw(4U) << std::hex << bc;
        ss << ", " << std::setfill('.');

        BytecodeInstruction::Format format = bc_ins.GetFormat();

        auto format_str = std::string("[") + BytecodeInstruction::GetFormatString(format) + ']';
        ss << std::setw(FORMAT_WIDTH) << std::left << format_str;

        ss << "[";

        const uint8_t *pc = bc_ins.GetAddress();
        const size_t sz = bc_ins.GetSize();

        for (size_t i = 0; i < sz; i++) {
            ss << "0x" << std::setw(INSTRUCTION_WIDTH) << std::setfill('0') << std::right << std::hex
               << static_cast<int>(pc[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            if (i != sz - 1) {
                ss << " ";
            }
        }

        ss << "]";

        if (profile_ != profiling::INVALID_PROFILE && prof != profiling::INVALID_PROFILE) {
            auto prof_id = bc_ins.GetProfileId();
            if (prof_id != -1) {
                ss << ", Profile: ";
                profiling::DumpProfile(prof, file_language_, &bc_ins, ss);
            }
        }

        method_info->instructions_info.push_back(ss.str());

        bc_ins = bc_ins.GetNext();
    }
}

void Disassembler::CollectInfo()
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting program info]\n";

    debug_info_extractor_ = std::make_unique<panda_file::DebugInfoExtractor>(file_);

    for (const auto &pair : record_name_to_id_) {
        GetRecordInfo(pair.second, &prog_info_.records_info[pair.first]);
    }

    for (const auto &pair : method_name_to_id_) {
        GetMethodInfo(pair.second, &prog_info_.methods_info[pair.first]);
    }
}

void Disassembler::Serialize(std::ostream &os, bool add_separators, bool print_information) const
{
    if (os.bad()) {
        LOG(DEBUG, DISASSEMBLER) << "> serialization failed. os bad\n";

        return;
    }

    SerializeFilename(os);
    SerializeLanguage(os);
    SerializeLitArrays(os, add_separators);
    SerializeRecords(os, add_separators, print_information);
    SerializeMethods(os, add_separators, print_information);
}

void Disassembler::Serialize(const pandasm::Function &method, std::ostream &os, bool print_information,
                             panda_file::LineNumberTable *line_table) const
{
    std::ostringstream header_ss;
    header_ss << ".function " << method.return_type.GetPandasmName() << " " << method.name << "(";

    if (!method.params.empty()) {
        header_ss << method.params[0].type.GetPandasmName() << " a0";

        for (size_t i = 1; i < method.params.size(); i++) {
            header_ss << ", " << method.params[i].type.GetPandasmName() << " a" << (size_t)i;
        }
    }
    header_ss << ")";

    const std::string signature = pandasm::GetFunctionSignatureFromName(method.name, method.params);

    const auto method_iter = prog_ann_.method_annotations.find(signature);
    if (method_iter != prog_ann_.method_annotations.end()) {
        Serialize(*method.metadata, method_iter->second, header_ss);
    } else {
        Serialize(*method.metadata, {}, header_ss);
    }

    if (!method.HasImplementation()) {
        header_ss << "\n\n";
        os << header_ss.str();
        return;
    }

    header_ss << " {";

    size_t width;
    const MethodInfo *method_info;
    auto method_info_it = prog_info_.methods_info.find(signature);
    bool print_method_info = print_information && method_info_it != prog_info_.methods_info.end();
    if (print_method_info) {
        method_info = &method_info_it->second;

        width = 0;
        for (const auto &i : method.ins) {
            if (i.ToString().size() > width) {
                width = i.ToString().size();
            }
        }

        header_ss << " # " << method_info->method_info << "\n#   CODE:";
    }

    header_ss << "\n";

    auto header_ss_str = header_ss.str();
    size_t line_number = std::count(header_ss_str.begin(), header_ss_str.end(), '\n') + 1;

    os << header_ss_str;

    for (size_t i = 0; i < method.ins.size(); i++) {
        std::ostringstream ins_ss;

        std::string ins = method.ins[i].ToString("", method.GetParamsNum() != 0, method.regs_num);
        if (method.ins[i].set_label) {
            std::string delim = ": ";
            size_t pos = ins.find(delim);
            std::string label = ins.substr(0, pos);
            ins.erase(0, pos + delim.length());

            ins_ss << label << ":\n";
        }

        ins_ss << "\t";
        if (print_method_info) {
            ins_ss << std::setw(width) << std::left;
        }
        ins_ss << ins;
        if (print_method_info) {
            ASSERT(method_info != nullptr);
            ins_ss << " # " << method_info->instructions_info[i];
        }
        ins_ss << "\n";

        auto ins_ss_str = ins_ss.str();
        line_number += std::count(ins_ss_str.begin(), ins_ss_str.end(), '\n');

        if (line_table != nullptr) {
            line_table->emplace_back(panda_file::LineTableEntry {
                static_cast<uint32_t>(method.ins[i].ins_debug.bound_left), line_number - 1});
        }

        os << ins_ss_str;
    }

    if (!method.catch_blocks.empty()) {
        os << "\n";

        for (const auto &catch_block : method.catch_blocks) {
            Serialize(catch_block, os);

            os << "\n";
        }
    }

    if (print_method_info) {
        ASSERT(method_info != nullptr);
        SerializeLineNumberTable(method_info->line_number_table, os);
        SerializeLocalVariableTable(method_info->local_variable_table, method, os);
    }

    os << "}\n\n";
}

inline bool Disassembler::IsSystemType(const std::string &type_name)
{
    bool is_array_type = type_name.back() == ']';
    bool is_global = type_name == "_GLOBAL";

    return is_array_type || is_global;
}

void Disassembler::GetRecord(pandasm::Record *record, const panda_file::File::EntityId &record_id)
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting record]\nid: " << record_id << " (0x" << std::hex << record_id << ")";

    if (record == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but record ptr expected!";

        return;
    }

    record->name = GetFullRecordName(record_id);

    LOG(DEBUG, DISASSEMBLER) << "name: " << record->name;

    GetMetaData(record, record_id);

    if (!file_->IsExternal(record_id)) {
        GetMethods(record_id);
        GetFields(record, record_id);
    }
}

void Disassembler::AddMethodToTables(const panda_file::File::EntityId &method_id)
{
    pandasm::Function new_method("", file_language_);
    GetMethod(&new_method, method_id);

    const auto signature = pandasm::GetFunctionSignatureFromName(new_method.name, new_method.params);
    if (prog_.function_table.find(signature) != prog_.function_table.end()) {
        return;
    }

    method_name_to_id_.emplace(signature, method_id);
    prog_.function_synonyms[new_method.name].push_back(signature);
    prog_.function_table.emplace(signature, std::move(new_method));
}

void Disassembler::GetMethod(pandasm::Function *method, const panda_file::File::EntityId &method_id)
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting method]\nid: " << method_id << " (0x" << std::hex << method_id << ")";

    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!";

        return;
    }

    panda_file::MethodDataAccessor method_accessor(*file_, method_id);

    method->name = GetFullMethodName(method_id);

    LOG(DEBUG, DISASSEMBLER) << "name: " << method->name;

    GetParams(method, method_accessor.GetProtoId());
    GetMetaData(method, method_id);

    if (!method->HasImplementation()) {
        return;
    }

    if (method_accessor.GetCodeId().has_value()) {
        const IdList id_list = GetInstructions(method, method_id, method_accessor.GetCodeId().value());

        for (const auto &id : id_list) {
            AddMethodToTables(id);
        }
    } else {
        LOG(ERROR, DISASSEMBLER) << "> error encountered at " << method_id << " (0x" << std::hex << method_id
                                 << "). implementation of method expected, but no \'CODE\' tag was found!";

        return;
    }
}

template <typename T>
void Disassembler::FillLiteralArrayData(pandasm::LiteralArray *lit_array, const panda_file::LiteralTag &tag,
                                        const panda_file::LiteralDataAccessor::LiteralValue &value) const
{
    panda_file::File::EntityId id(std::get<uint32_t>(value));
    auto sp = file_->GetSpanFromId(id);
    auto len = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        for (size_t i = 0; i < len; i++) {
            pandasm::LiteralArray::Literal lit;
            lit.tag = tag;
            lit.value = bit_cast<T>(panda_file::helpers::Read<sizeof(T)>(&sp));
            lit_array->literals.push_back(lit);
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            auto str_id = panda_file::helpers::Read<sizeof(T)>(&sp);
            pandasm::LiteralArray::Literal lit;
            lit.tag = tag;
            lit.value = StringDataToString(file_->GetStringData(panda_file::File::EntityId(str_id)));
            lit_array->literals.push_back(lit);
        }
    }
}

void Disassembler::FillLiteralData(pandasm::LiteralArray *lit_array,
                                   const panda_file::LiteralDataAccessor::LiteralValue &value,
                                   const panda_file::LiteralTag &tag) const
{
    pandasm::LiteralArray::Literal lit;
    lit.tag = tag;
    switch (tag) {
        case panda_file::LiteralTag::BOOL: {
            lit.value = std::get<bool>(value);
            break;
        }
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE: {
            lit.value = std::get<uint8_t>(value);
            break;
        }
        case panda_file::LiteralTag::METHODAFFILIATE: {
            lit.value = std::get<uint16_t>(value);
            break;
        }
        case panda_file::LiteralTag::INTEGER: {
            lit.value = std::get<uint32_t>(value);
            break;
        }
        case panda_file::LiteralTag::BIGINT: {
            lit.value = std::get<uint64_t>(value);
            break;
        }
        case panda_file::LiteralTag::FLOAT: {
            lit.value = std::get<float>(value);
            break;
        }
        case panda_file::LiteralTag::DOUBLE: {
            lit.value = std::get<double>(value);
            break;
        }
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD: {
            auto str_data = file_->GetStringData(panda_file::File::EntityId(std::get<uint32_t>(value)));
            lit.value = StringDataToString(str_data);
            break;
        }
        case panda_file::LiteralTag::TAGVALUE: {
            return;
        }
        default: {
            LOG(ERROR, DISASSEMBLER) << "Unsupported literal with tag 0x" << std::hex << static_cast<uint32_t>(tag);
            UNREACHABLE();
        }
    }
    lit_array->literals.push_back(lit);
}

void Disassembler::GetLiteralArray(pandasm::LiteralArray *lit_array, const size_t index)
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting literal array]\nindex: " << index;

    panda_file::LiteralDataAccessor lit_array_accessor(*file_, file_->GetLiteralArraysId());

    lit_array_accessor.EnumerateLiteralVals(
        index, [this, lit_array](const panda_file::LiteralDataAccessor::LiteralValue &value,
                                 const panda_file::LiteralTag &tag) {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U1: {
                    FillLiteralArrayData<bool>(lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I8:
                case panda_file::LiteralTag::ARRAY_U8: {
                    FillLiteralArrayData<uint8_t>(lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I16:
                case panda_file::LiteralTag::ARRAY_U16: {
                    FillLiteralArrayData<uint16_t>(lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I32:
                case panda_file::LiteralTag::ARRAY_U32: {
                    FillLiteralArrayData<uint32_t>(lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I64:
                case panda_file::LiteralTag::ARRAY_U64: {
                    FillLiteralArrayData<uint64_t>(lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_F32: {
                    FillLiteralArrayData<float>(lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_F64: {
                    FillLiteralArrayData<double>(lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_STRING: {
                    FillLiteralArrayData<uint32_t>(lit_array, tag, value);
                    break;
                }
                default: {
                    FillLiteralData(lit_array, value, tag);
                }
            }
        });
}

void Disassembler::GetLiteralArrays()
{
    const auto lit_arrays_id = file_->GetLiteralArraysId();

    LOG(DEBUG, DISASSEMBLER) << "\n[getting literal arrays]\nid: " << lit_arrays_id << " (0x" << std::hex
                             << lit_arrays_id << ")";

    panda_file::LiteralDataAccessor lit_array_accessor(*file_, lit_arrays_id);
    size_t num_litarrays = lit_array_accessor.GetLiteralNum();
    for (size_t index = 0; index < num_litarrays; index++) {
        panda::pandasm::LiteralArray lit_ar;
        GetLiteralArray(&lit_ar, index);
        prog_.literalarray_table.emplace(std::to_string(index), lit_ar);
    }
}

void Disassembler::GetRecords()
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting records]\n";

    const auto class_idx = file_->GetClasses();

    for (size_t i = 0; i < class_idx.size(); i++) {
        uint32_t class_id = class_idx[i];
        auto class_off = file_->GetHeader()->class_idx_off + sizeof(uint32_t) * i;

        if (class_id > file_->GetHeader()->file_size) {
            LOG(ERROR, DISASSEMBLER) << "> error encountered in record at " << class_off << " (0x" << std::hex
                                     << class_off << "). binary file corrupted. record offset (0x" << class_id
                                     << ") out of bounds (0x" << file_->GetHeader()->file_size << ")!";
            break;
        }

        const panda_file::File::EntityId record_id {class_id};
        auto language = GetRecordLanguage(record_id);

        if (language != file_language_) {
            if (file_language_ == panda_file::SourceLang::PANDA_ASSEMBLY) {
                file_language_ = language;
            } else if (language != panda_file::SourceLang::PANDA_ASSEMBLY) {
                LOG(ERROR, DISASSEMBLER) << "> possible error encountered in record at" << class_off << " (0x"
                                         << std::hex << class_off << "). record's language  ("
                                         << panda_file::LanguageToString(language)
                                         << ")  differs from file's language ("
                                         << panda_file::LanguageToString(file_language_) << ")!";
            }
        }

        pandasm::Record record("", file_language_);
        GetRecord(&record, record_id);

        if (prog_.record_table.find(record.name) == prog_.record_table.end()) {
            record_name_to_id_.emplace(record.name, record_id);
            prog_.record_table.emplace(record.name, std::move(record));
        }
    }
}

void Disassembler::GetField(pandasm::Field &field, const panda_file::FieldDataAccessor &field_accessor)
{
    panda_file::File::EntityId field_name_id = field_accessor.GetNameId();
    field.name = StringDataToString(file_->GetStringData(field_name_id));

    uint32_t field_type = field_accessor.GetType();
    field.type = FieldTypeToPandasmType(field_type);

    GetMetaData(&field, field_accessor.GetFieldId());
}

void Disassembler::GetFields(pandasm::Record *record, const panda_file::File::EntityId &record_id)
{
    panda_file::ClassDataAccessor class_accessor {*file_, record_id};

    class_accessor.EnumerateFields([&](panda_file::FieldDataAccessor &field_accessor) -> void {
        pandasm::Field field(file_language_);

        GetField(field, field_accessor);

        record->field_list.push_back(std::move(field));
    });
}

void Disassembler::AddExternalFieldsToRecords()
{
    for (auto &[record_name, record] : prog_.record_table) {
        auto &[unused, field_list] = *(external_field_table_.find(record_name));
        (void)unused;
        if (field_list.empty()) {
            continue;
        }
        for (auto &field_iter : field_list) {
            if (!field_iter.name.empty()) {
                record.field_list.push_back(std::move(field_iter));
            }
        }
        external_field_table_.erase(record_name);
    }
}

void Disassembler::GetMethods(const panda_file::File::EntityId &record_id)
{
    panda_file::ClassDataAccessor class_accessor {*file_, record_id};

    class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
        AddMethodToTables(method_accessor.GetMethodId());
    });
}

void Disassembler::GetParams(pandasm::Function *method, const panda_file::File::EntityId &proto_id) const
{
    /// frame size - 2^16 - 1
    static const uint32_t MAX_ARG_NUM = 0xFFFF;

    LOG(DEBUG, DISASSEMBLER) << "[getting params]\nproto id: " << proto_id << " (0x" << std::hex << proto_id << ")";

    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!";

        return;
    }

    panda_file::ProtoDataAccessor proto_accessor(*file_, proto_id);

    if (proto_accessor.GetNumArgs() > MAX_ARG_NUM) {
        LOG(ERROR, DISASSEMBLER) << "> error encountered at " << proto_id << " (0x" << std::hex << proto_id
                                 << "). number of function's arguments (" << std::dec << proto_accessor.GetNumArgs()
                                 << ") exceeds MAX_ARG_NUM (" << MAX_ARG_NUM << ") !";

        return;
    }

    size_t ref_idx = 0;
    method->return_type = PFTypeToPandasmType(proto_accessor.GetReturnType(), proto_accessor, ref_idx);

    for (size_t i = 0; i < proto_accessor.GetNumArgs(); i++) {
        auto arg_type = PFTypeToPandasmType(proto_accessor.GetArgType(i), proto_accessor, ref_idx);
        method->params.emplace_back(arg_type, file_language_);
    }
}

LabelTable Disassembler::GetExceptions(pandasm::Function *method, panda_file::File::EntityId method_id,
                                       panda_file::File::EntityId code_id) const
{
    LOG(DEBUG, DISASSEMBLER) << "[getting exceptions]\ncode id: " << code_id << " (0x" << std::hex << code_id << ")";

    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!\n";
        return LabelTable {};
    }

    panda_file::CodeDataAccessor code_accessor(*file_, code_id);

    const auto bc_ins = BytecodeInstruction(code_accessor.GetInstructions());
    const auto bc_ins_last = bc_ins.JumpTo(code_accessor.GetCodeSize());

    size_t try_idx = 0;
    LabelTable label_table {};
    code_accessor.EnumerateTryBlocks([&](panda_file::CodeDataAccessor::TryBlock &try_block) {
        pandasm::Function::CatchBlock catch_block_pa {};
        if (!LocateTryBlock(bc_ins, bc_ins_last, try_block, &catch_block_pa, &label_table, try_idx)) {
            return false;
        }
        size_t catch_idx = 0;
        try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            auto class_idx = catch_block.GetTypeIdx();

            if (class_idx == panda_file::INVALID_INDEX) {
                catch_block_pa.exception_record = "";
            } else {
                const auto class_id = file_->ResolveClassIndex(method_id, class_idx);
                catch_block_pa.exception_record = GetFullRecordName(class_id);
            }
            if (!LocateCatchBlock(bc_ins, bc_ins_last, catch_block, &catch_block_pa, &label_table, try_idx,
                                  catch_idx)) {
                return false;
            }

            method->catch_blocks.push_back(catch_block_pa);
            catch_block_pa.catch_begin_label = "";
            catch_block_pa.catch_end_label = "";
            catch_idx++;

            return true;
        });
        try_idx++;

        return true;
    });

    return label_table;
}

static size_t GetBytecodeInstructionNumber(BytecodeInstruction bc_ins_first, BytecodeInstruction bc_ins_cur)
{
    size_t count = 0;

    while (bc_ins_first.GetAddress() != bc_ins_cur.GetAddress()) {
        count++;
        bc_ins_first = bc_ins_first.GetNext();
        if (bc_ins_first.GetAddress() > bc_ins_cur.GetAddress()) {
            return std::numeric_limits<size_t>::max();
        }
    }

    return count;
}

bool Disassembler::LocateTryBlock(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                                  const panda_file::CodeDataAccessor::TryBlock &try_block,
                                  pandasm::Function::CatchBlock *catch_block_pa, LabelTable *label_table,
                                  size_t try_idx) const
{
    const auto try_begin_bc_ins = bc_ins.JumpTo(try_block.GetStartPc());
    const auto try_end_bc_ins = bc_ins.JumpTo(try_block.GetStartPc() + try_block.GetLength());

    const size_t try_begin_idx = GetBytecodeInstructionNumber(bc_ins, try_begin_bc_ins);
    const size_t try_end_idx = GetBytecodeInstructionNumber(bc_ins, try_end_bc_ins);

    const bool try_begin_offset_in_range = bc_ins_last.GetAddress() > try_begin_bc_ins.GetAddress();
    const bool try_end_offset_in_range = bc_ins_last.GetAddress() >= try_end_bc_ins.GetAddress();
    const bool try_begin_offset_valid = try_begin_idx != std::numeric_limits<size_t>::max();
    const bool try_end_offset_valid = try_end_idx != std::numeric_limits<size_t>::max();

    if (!try_begin_offset_in_range || !try_begin_offset_valid) {
        LOG(ERROR, DISASSEMBLER) << "> invalid try block begin offset! address is: 0x" << std::hex
                                 << try_begin_bc_ins.GetAddress();
        return false;
    }

    auto it_begin = label_table->find(try_begin_idx);
    if (it_begin == label_table->end()) {
        std::stringstream ss {};
        ss << "try_begin_label_" << try_idx;
        catch_block_pa->try_begin_label = ss.str();
        label_table->insert(std::pair<size_t, std::string>(try_begin_idx, ss.str()));
    } else {
        catch_block_pa->try_begin_label = it_begin->second;
    }

    if (!try_end_offset_in_range || !try_end_offset_valid) {
        LOG(ERROR, DISASSEMBLER) << "> invalid try block end offset! address is: 0x" << std::hex
                                 << try_end_bc_ins.GetAddress();
        return false;
    }

    auto it_end = label_table->find(try_end_idx);
    if (it_end == label_table->end()) {
        std::stringstream ss {};
        ss << "try_end_label_" << try_idx;
        catch_block_pa->try_end_label = ss.str();
        label_table->insert(std::pair<size_t, std::string>(try_end_idx, ss.str()));
    } else {
        catch_block_pa->try_end_label = it_end->second;
    }

    return true;
}

bool Disassembler::LocateCatchBlock(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                                    const panda_file::CodeDataAccessor::CatchBlock &catch_block,
                                    pandasm::Function::CatchBlock *catch_block_pa, LabelTable *label_table,
                                    size_t try_idx, size_t catch_idx) const
{
    const auto handler_begin_offset = catch_block.GetHandlerPc();
    const auto handler_end_offset = handler_begin_offset + catch_block.GetCodeSize();

    const auto handler_begin_bc_ins = bc_ins.JumpTo(handler_begin_offset);
    const auto handler_end_bc_ins = bc_ins.JumpTo(handler_end_offset);

    const size_t handler_begin_idx = GetBytecodeInstructionNumber(bc_ins, handler_begin_bc_ins);
    const size_t handler_end_idx = GetBytecodeInstructionNumber(bc_ins, handler_end_bc_ins);

    const bool handler_begin_offset_in_range = bc_ins_last.GetAddress() > handler_begin_bc_ins.GetAddress();
    const bool handler_end_offset_in_range = bc_ins_last.GetAddress() > handler_end_bc_ins.GetAddress();
    const bool handler_end_present = catch_block.GetCodeSize() != 0;
    const bool handler_begin_offset_valid = handler_begin_idx != std::numeric_limits<size_t>::max();
    const bool handler_end_offset_valid = handler_end_idx != std::numeric_limits<size_t>::max();

    if (!handler_begin_offset_in_range || !handler_begin_offset_valid) {
        LOG(ERROR, DISASSEMBLER) << "> invalid catch block begin offset! address is: 0x" << std::hex
                                 << handler_begin_bc_ins.GetAddress();
        return false;
    }

    auto it_begin = label_table->find(handler_begin_idx);
    if (it_begin == label_table->end()) {
        std::stringstream ss {};
        ss << "handler_begin_label_" << try_idx << "_" << catch_idx;
        catch_block_pa->catch_begin_label = ss.str();
        label_table->insert(std::pair<size_t, std::string>(handler_begin_idx, ss.str()));
    } else {
        catch_block_pa->catch_begin_label = it_begin->second;
    }

    if (!handler_end_offset_in_range || !handler_end_offset_valid) {
        LOG(ERROR, DISASSEMBLER) << "> invalid catch block end offset! address is: 0x" << std::hex
                                 << handler_end_bc_ins.GetAddress();
        return false;
    }

    if (handler_end_present) {
        auto it_end = label_table->find(handler_end_idx);
        if (it_end == label_table->end()) {
            std::stringstream ss {};
            ss << "handler_end_label_" << try_idx << "_" << catch_idx;
            catch_block_pa->catch_end_label = ss.str();
            label_table->insert(std::pair<size_t, std::string>(handler_end_idx, ss.str()));
        } else {
            catch_block_pa->catch_end_label = it_end->second;
        }
    }

    return true;
}

template <typename T>
static void SetEntityAttribute(T *entity, const std::function<bool()> &should_set, std::string_view attribute)
{
    if (should_set()) {
        auto err = entity->metadata->SetAttribute(attribute);
        if (err.has_value()) {
            LOG(ERROR, DISASSEMBLER) << err.value().GetMessage();
        }
    }
}

template <typename T>
static void SetEntityAttributeValue(T *entity, const std::function<bool()> &should_set, std::string_view attribute,
                                    const char *value)
{
    if (should_set()) {
        auto err = entity->metadata->SetAttributeValue(attribute, value);
        if (err.has_value()) {
            LOG(ERROR, DISASSEMBLER) << err.value().GetMessage();
        }
    }
}

void Disassembler::GetMetaData(pandasm::Function *method, const panda_file::File::EntityId &method_id) const
{
    LOG(DEBUG, DISASSEMBLER) << "[getting metadata]\nmethod id: " << method_id << " (0x" << std::hex << method_id
                             << ")";

    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!";

        return;
    }

    panda_file::MethodDataAccessor method_accessor(*file_, method_id);

    const auto method_name_raw = StringDataToString(file_->GetStringData(method_accessor.GetNameId()));

    if (!method_accessor.IsStatic()) {
        const auto class_name = StringDataToString(file_->GetStringData(method_accessor.GetClassId()));
        auto this_type = pandasm::Type::FromDescriptor(class_name);

        LOG(DEBUG, DISASSEMBLER) << "method (raw: \'" << method_name_raw
                                 << "\') is not static. emplacing self-argument of type " << this_type.GetName();

        method->params.insert(method->params.begin(), pandasm::Function::Parameter(this_type, file_language_));
    }
    SetEntityAttribute(
        method, [&method_accessor]() { return method_accessor.IsStatic(); }, "static");

    SetEntityAttribute(
        method, [this, &method_accessor]() { return file_->IsExternal(method_accessor.GetMethodId()); }, "external");

    SetEntityAttribute(
        method, [&method_accessor]() { return method_accessor.IsNative(); }, "native");

    SetEntityAttribute(
        method, [&method_accessor]() { return method_accessor.IsAbstract(); }, "noimpl");

    SetEntityAttributeValue(
        method, [&method_accessor]() { return method_accessor.IsPublic(); }, "access.function", "public");

    SetEntityAttributeValue(
        method, [&method_accessor]() { return method_accessor.IsProtected(); }, "access.function", "protected");

    SetEntityAttributeValue(
        method, [&method_accessor]() { return method_accessor.IsPrivate(); }, "access.function", "private");

    SetEntityAttribute(
        method, [&method_accessor]() { return method_accessor.IsFinal(); }, "final");

    std::string ctor_name = panda::panda_file::GetCtorName(file_language_);
    std::string cctor_name = panda::panda_file::GetCctorName(file_language_);

    const bool is_ctor = (method_name_raw == ctor_name);
    const bool is_cctor = (method_name_raw == cctor_name);

    if (is_ctor) {
        method->metadata->SetAttribute("ctor");
        method->name.replace(method->name.find(ctor_name), ctor_name.length(), "_ctor_");
    } else if (is_cctor) {
        method->metadata->SetAttribute("cctor");
        method->name.replace(method->name.find(cctor_name), cctor_name.length(), "_cctor_");
    }
}

void Disassembler::GetMetaData(pandasm::Record *record, const panda_file::File::EntityId &record_id) const
{
    LOG(DEBUG, DISASSEMBLER) << "[getting metadata]\nrecord id: " << record_id << " (0x" << std::hex << record_id
                             << ")";

    if (record == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but record ptr expected!";

        return;
    }

    SetEntityAttribute(
        record, [this, record_id]() { return file_->IsExternal(record_id); }, "external");

    auto external = file_->IsExternal(record_id);
    if (!external) {
        auto cda = panda_file::ClassDataAccessor {*file_, record_id};
        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsPublic(); }, "access.record", "public");

        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsProtected(); }, "access.record", "protected");

        SetEntityAttributeValue(
            record, [&cda]() { return cda.IsPrivate(); }, "access.record", "private");

        SetEntityAttribute(
            record, [&cda]() { return cda.IsFinal(); }, "final");
    }
}

void Disassembler::GetMetaData(pandasm::Field *field, const panda_file::File::EntityId &field_id) const
{
    LOG(DEBUG, DISASSEMBLER) << "[getting metadata]\nfield id: " << field_id << " (0x" << std::hex << field_id << ")";

    if (field == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved, but method ptr expected!";

        return;
    }

    panda_file::FieldDataAccessor field_accessor(*file_, field_id);

    SetEntityAttribute(
        field, [&field_accessor]() { return field_accessor.IsExternal(); }, "external");

    SetEntityAttribute(
        field, [&field_accessor]() { return field_accessor.IsStatic(); }, "static");

    SetEntityAttributeValue(
        field, [&field_accessor]() { return field_accessor.IsPublic(); }, "access.field", "public");

    SetEntityAttributeValue(
        field, [&field_accessor]() { return field_accessor.IsProtected(); }, "access.field", "protected");

    SetEntityAttributeValue(
        field, [&field_accessor]() { return field_accessor.IsPrivate(); }, "access.field", "private");

    SetEntityAttribute(
        field, [&field_accessor]() { return field_accessor.IsFinal(); }, "final");
}

std::string Disassembler::AnnotationTagToString(const char tag) const
{
    static const std::unordered_map<char, std::string> TAG_TO_STRING = {{'1', "u1"},
                                                                        {'2', "i8"},
                                                                        {'3', "u8"},
                                                                        {'4', "i16"},
                                                                        {'5', "u16"},
                                                                        {'6', "i32"},
                                                                        {'7', "u32"},
                                                                        {'8', "i64"},
                                                                        {'9', "u64"},
                                                                        {'A', "f32"},
                                                                        {'B', "f64"},
                                                                        {'C', "string"},
                                                                        {'D', "record"},
                                                                        {'E', "method"},
                                                                        {'F', "enum"},
                                                                        {'G', "annotation"},
                                                                        {'J', "method_handle"},
                                                                        {'H', "array"},
                                                                        {'K', "u1[]"},
                                                                        {'L', "i8[]"},
                                                                        {'M', "u8[]"},
                                                                        {'N', "i16[]"},
                                                                        {'O', "u16[]"},
                                                                        {'P', "i32[]"},
                                                                        {'Q', "u32[]"},
                                                                        {'R', "i64[]"},
                                                                        {'S', "u64[]"},
                                                                        {'T', "f32[]"},
                                                                        {'U', "f64[]"},
                                                                        {'V', "string[]"},
                                                                        {'W', "record[]"},
                                                                        {'X', "method[]"},
                                                                        {'Y', "enum[]"},
                                                                        {'Z', "annotation[]"},
                                                                        {'@', "method_handle[]"},
                                                                        {'*', "nullptr_string"}};

    return TAG_TO_STRING.at(tag);
}

std::string Disassembler::ScalarValueToString(const panda_file::ScalarValue &value, const std::string &type)
{
    std::stringstream ss;

    if (type == "i8") {
        auto res = value.Get<int8_t>();
        ss << static_cast<int>(res);
    } else if (type == "u1" || type == "u8") {
        auto res = value.Get<uint8_t>();
        ss << static_cast<unsigned int>(res);
    } else if (type == "i16") {
        ss << value.Get<int16_t>();
    } else if (type == "u16") {
        ss << value.Get<uint16_t>();
    } else if (type == "i32") {
        ss << value.Get<int32_t>();
    } else if (type == "u32") {
        ss << value.Get<uint32_t>();
    } else if (type == "i64") {
        ss << value.Get<int64_t>();
    } else if (type == "u64") {
        ss << value.Get<uint64_t>();
    } else if (type == "f32") {
        ss << value.Get<float>();
    } else if (type == "f64") {
        ss << value.Get<double>();
    } else if (type == "string") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << "\"" << StringDataToString(file_->GetStringData(id)) << "\"";
    } else if (type == "record") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << GetFullRecordName(id);
    } else if (type == "method") {
        const auto id = value.Get<panda_file::File::EntityId>();
        AddMethodToTables(id);
        ss << GetMethodSignature(id);
    } else if (type == "enum") {
        const auto id = value.Get<panda_file::File::EntityId>();
        panda_file::FieldDataAccessor field_accessor(*file_, id);
        ss << GetFullRecordName(field_accessor.GetClassId()) << "."
           << StringDataToString(file_->GetStringData(field_accessor.GetNameId()));
    } else if (type == "annotation") {
        const auto id = value.Get<panda_file::File::EntityId>();
        ss << "id_" << id;
    } else if (type == "void") {
        return std::string();
    } else if (type == "method_handle") {
    } else if (type == "nullptr_string") {
        ss << static_cast<uint32_t>(0);
    }

    return ss.str();
}

std::string Disassembler::ArrayValueToString(const panda_file::ArrayValue &value, const std::string &type,
                                             const size_t idx)
{
    std::stringstream ss;

    if (type == "i8") {
        auto res = value.Get<int8_t>(idx);
        ss << static_cast<int>(res);
    } else if (type == "u1" || type == "u8") {
        auto res = value.Get<uint8_t>(idx);
        ss << static_cast<unsigned int>(res);
    } else if (type == "i16") {
        ss << value.Get<int16_t>(idx);
    } else if (type == "u16") {
        ss << value.Get<uint16_t>(idx);
    } else if (type == "i32") {
        ss << value.Get<int32_t>(idx);
    } else if (type == "u32") {
        ss << value.Get<uint32_t>(idx);
    } else if (type == "i64") {
        ss << value.Get<int64_t>(idx);
    } else if (type == "u64") {
        ss << value.Get<uint64_t>(idx);
    } else if (type == "f32") {
        ss << value.Get<float>(idx);
    } else if (type == "f64") {
        ss << value.Get<double>(idx);
    } else if (type == "string") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << '\"' << StringDataToString(file_->GetStringData(id)) << '\"';
    } else if (type == "record") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << GetFullRecordName(id);
    } else if (type == "method") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        AddMethodToTables(id);
        ss << GetMethodSignature(id);
    } else if (type == "enum") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        panda_file::FieldDataAccessor field_accessor(*file_, id);
        ss << GetFullRecordName(field_accessor.GetClassId()) << "."
           << StringDataToString(file_->GetStringData(field_accessor.GetNameId()));
    } else if (type == "annotation") {
        const auto id = value.Get<panda_file::File::EntityId>(idx);
        ss << "id_" << id;
    } else if (type == "method_handle") {
    } else if (type == "nullptr_string") {
    }

    return ss.str();
}

std::string Disassembler::GetFullMethodName(const panda_file::File::EntityId &method_id) const
{
    panda::panda_file::MethodDataAccessor method_accessor(*file_, method_id);

    const auto method_name_raw = StringDataToString(file_->GetStringData(method_accessor.GetNameId()));

    std::string class_name = GetFullRecordName(method_accessor.GetClassId());
    if (IsSystemType(class_name)) {
        class_name = "";
    } else {
        class_name += ".";
    }

    return class_name + method_name_raw;
}

std::string Disassembler::GetMethodSignature(const panda_file::File::EntityId &method_id) const
{
    panda::panda_file::MethodDataAccessor method_accessor(*file_, method_id);

    pandasm::Function method(GetFullMethodName(method_id), file_language_);
    GetParams(&method, method_accessor.GetProtoId());
    GetMetaData(&method, method_id);

    return pandasm::GetFunctionSignatureFromName(method.name, method.params);
}

std::string Disassembler::GetFullRecordName(const panda_file::File::EntityId &class_id) const
{
    std::string name = StringDataToString(file_->GetStringData(class_id));

    auto type = pandasm::Type::FromDescriptor(name);
    type = pandasm::Type(type.GetComponentName(), type.GetRank());

    return type.GetPandasmName();
}

void Disassembler::GetRecordInfo(const panda_file::File::EntityId &record_id, RecordInfo *record_info) const
{
    constexpr size_t DEFAULT_OFFSET_WIDTH = 4;

    if (file_->IsExternal(record_id)) {
        return;
    }

    panda_file::ClassDataAccessor class_accessor {*file_, record_id};
    std::stringstream ss;

    ss << "offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex
       << class_accessor.GetClassId() << ", size: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH)
       << class_accessor.GetSize() << " (" << std::dec << class_accessor.GetSize() << ")";

    record_info->record_info = ss.str();
    ss.str(std::string());

    class_accessor.EnumerateFields([&](panda_file::FieldDataAccessor &field_accessor) -> void {
        ss << "offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex
           << field_accessor.GetFieldId() << ", type: 0x" << field_accessor.GetType();

        record_info->fields_info.push_back(ss.str());

        ss.str(std::string());
    });
}

void Disassembler::GetMethodInfo(const panda_file::File::EntityId &method_id, MethodInfo *method_info) const
{
    constexpr size_t DEFAULT_OFFSET_WIDTH = 4;

    panda_file::MethodDataAccessor method_accessor {*file_, method_id};
    std::stringstream ss;

    ss << "offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex
       << method_accessor.GetMethodId();

    if (method_accessor.GetCodeId().has_value()) {
        ss << ", code offset: 0x" << std::setfill('0') << std::setw(DEFAULT_OFFSET_WIDTH) << std::hex
           << method_accessor.GetCodeId().value();

        GetInsInfo(method_accessor, method_accessor.GetCodeId().value(), method_info);
    } else {
        ss << ", <no code>";
    }

    auto profile_size = method_accessor.GetProfileSize();
    if (profile_size) {
        ss << ", profile size: " << profile_size.value();
    }

    method_info->method_info = ss.str();

    if (method_accessor.GetCodeId()) {
        ASSERT(debug_info_extractor_ != nullptr);
        method_info->line_number_table = debug_info_extractor_->GetLineNumberTable(method_id);
        method_info->local_variable_table = debug_info_extractor_->GetLocalVariableTable(method_id);

        // Add information about parameters into the table
        panda_file::CodeDataAccessor codeda(*file_, method_accessor.GetCodeId().value());
        auto arg_idx = static_cast<int32_t>(codeda.GetNumVregs());
        uint32_t code_size = codeda.GetCodeSize();
        for (const auto &info : debug_info_extractor_->GetParameterInfo(method_id)) {
            panda_file::LocalVariableInfo arg_info {info.name, info.signature, "", arg_idx++, 0, code_size};
            method_info->local_variable_table.emplace_back(arg_info);
        }
    }
}

void Disassembler::Serialize(const std::string &name, const pandasm::LiteralArray &lit_array, std::ostream &os) const
{
    if (lit_array.literals.empty()) {
        return;
    }

    bool is_const = lit_array.literals[0].IsArray();

    std::stringstream specifiers {};

    if (is_const) {
        specifiers << LiteralTagToString(lit_array.literals[0].tag) << " " << lit_array.literals.size() << " ";
    }

    os << ".array array_" << name << " " << specifiers.str() << "{";

    SerializeValues(lit_array, is_const, os);

    os << "}\n";
}

std::string Disassembler::LiteralTagToString(const panda_file::LiteralTag &tag) const
{
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
        case panda_file::LiteralTag::ARRAY_U1:
            return "u1";
        case panda_file::LiteralTag::ARRAY_U8:
            return "u8";
        case panda_file::LiteralTag::ARRAY_I8:
            return "i8";
        case panda_file::LiteralTag::ARRAY_U16:
            return "u16";
        case panda_file::LiteralTag::ARRAY_I16:
            return "i16";
        case panda_file::LiteralTag::ARRAY_U32:
            return "u32";
        case panda_file::LiteralTag::INTEGER:
        case panda_file::LiteralTag::ARRAY_I32:
            return "i32";
        case panda_file::LiteralTag::ARRAY_U64:
            return "u64";
        case panda_file::LiteralTag::BIGINT:
        case panda_file::LiteralTag::ARRAY_I64:
            return "i64";
        case panda_file::LiteralTag::FLOAT:
        case panda_file::LiteralTag::ARRAY_F32:
            return "f32";
        case panda_file::LiteralTag::DOUBLE:
        case panda_file::LiteralTag::ARRAY_F64:
            return "f64";
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::ARRAY_STRING:
            return pandasm::Type::FromDescriptor(panda_file::GetStringClassDescriptor(file_language_)).GetPandasmName();
        case panda_file::LiteralTag::ACCESSOR:
            return "accessor";
        case panda_file::LiteralTag::NULLVALUE:
            return "nullvalue";
        case panda_file::LiteralTag::METHODAFFILIATE:
            return "method_affiliate";
        case panda_file::LiteralTag::METHOD:
            return "method";
        case panda_file::LiteralTag::GENERATORMETHOD:
            return "generator_method";
        default:
            LOG(ERROR, DISASSEMBLER) << "Unsupported literal with tag 0x" << std::hex << static_cast<uint32_t>(tag);
            UNREACHABLE();
    }
}

std::string Disassembler::LiteralValueToString(const pandasm::LiteralArray::Literal &lit) const
{
    if (lit.IsBoolValue()) {
        std::stringstream res {};
        res << (std::get<bool>(lit.value));
        return res.str();
    }

    if (lit.IsByteValue()) {
        return LiteralIntegralValueToString<uint8_t>(lit);
    }

    if (lit.IsShortValue()) {
        return LiteralIntegralValueToString<uint16_t>(lit);
    }

    if (lit.IsIntegerValue()) {
        return LiteralIntegralValueToString<uint32_t>(lit);
    }

    if (lit.IsLongValue()) {
        return LiteralIntegralValueToString<uint64_t>(lit);
    }

    if (lit.IsDoubleValue()) {
        std::stringstream res {};
        res << std::get<double>(lit.value);
        return res.str();
    }

    if (lit.IsFloatValue()) {
        std::stringstream res {};
        res << std::get<float>(lit.value);
        return res.str();
    }

    if (lit.IsStringValue()) {
        std::stringstream res {};
        res << "\"" << std::get<std::string>(lit.value) << "\"";
        return res.str();
    }

    UNREACHABLE();
}

void Disassembler::SerializeValues(const pandasm::LiteralArray &lit_array, const bool is_const, std::ostream &os) const
{
    std::string separator = (is_const) ? (" ") : ("\n");

    os << separator;

    if (is_const) {
        for (const auto &l : lit_array.literals) {
            os << LiteralValueToString(l) << separator;
        }
    } else {
        for (const auto &l : lit_array.literals) {
            os << "\t" << LiteralTagToString(l.tag) << " " << LiteralValueToString(l) << separator;
        }
    }
}

void Disassembler::Serialize(const pandasm::Record &record, std::ostream &os, bool print_information) const
{
    if (IsSystemType(record.name)) {
        return;
    }

    os << ".record " << record.name;

    const auto record_iter = prog_ann_.record_annotations.find(record.name);
    const bool record_in_table = record_iter != prog_ann_.record_annotations.end();

    if (record_in_table) {
        Serialize(*record.metadata, record_iter->second.ann_list, os);
    } else {
        Serialize(*record.metadata, {}, os);
    }

    if (record.metadata->IsForeign() && record.field_list.empty()) {
        os << "\n\n";
        return;
    }

    os << " {";

    if (print_information && prog_info_.records_info.find(record.name) != prog_info_.records_info.end()) {
        os << " # " << prog_info_.records_info.at(record.name).record_info << "\n";
        SerializeFields(record, os, true);
    } else {
        os << "\n";
        SerializeFields(record, os, false);
    }

    os << "}\n\n";
}

void Disassembler::SerializeUnionFields(const pandasm::Record &record, std::ostream &os, bool print_information) const
{
    if (print_information && prog_info_.records_info.find(record.name) != prog_info_.records_info.end()) {
        os << " # " << prog_info_.records_info.at(record.name).record_info << "\n";
        SerializeFields(record, os, true, true);
    } else {
        SerializeFields(record, os, false, true);
    }
    os << "\n";
}

void Disassembler::SerializeFields(const pandasm::Record &record, std::ostream &os, bool print_information,
                                   bool is_union) const
{
    constexpr size_t INFO_OFFSET = 80;

    const auto record_iter = prog_ann_.record_annotations.find(record.name);
    const bool record_in_table = record_iter != prog_ann_.record_annotations.end();

    const auto rec_inf = (print_information) ? (prog_info_.records_info.at(record.name)) : (RecordInfo {});

    size_t field_idx = 0;

    std::stringstream ss;
    for (const auto &f : record.field_list) {
        if (is_union) {
            ss << ".union_field ";
        } else {
            ss << "\t";
        }
        ss << f.type.GetPandasmName() << " " << f.name;
        if (!is_union) {
            if (record_in_table) {
                const auto field_iter = record_iter->second.field_annotations.find(f.name);
                if (field_iter != record_iter->second.field_annotations.end()) {
                    Serialize(*f.metadata, field_iter->second, ss);
                } else {
                    Serialize(*f.metadata, {}, ss);
                }
            } else {
                Serialize(*f.metadata, {}, ss);
            }
        }

        if (print_information && !rec_inf.fields_info.empty()) {
            os << std::setw(INFO_OFFSET) << std::left << ss.str() << " # " << rec_inf.fields_info.at(field_idx) << "\n";
        } else {
            os << ss.str() << "\n";
        }

        ss.str(std::string());
        ss.clear();

        field_idx++;
    }
}

void Disassembler::Serialize(const pandasm::Function::CatchBlock &catch_block, std::ostream &os) const
{
    if (catch_block.exception_record.empty()) {
        os << ".catchall ";
    } else {
        os << ".catch " << catch_block.exception_record << ", ";
    }

    os << catch_block.try_begin_label << ", " << catch_block.try_end_label << ", " << catch_block.catch_begin_label;

    if (!catch_block.catch_end_label.empty()) {
        os << ", " << catch_block.catch_end_label;
    }
}

void Disassembler::Serialize(const pandasm::ItemMetadata &meta, const AnnotationList &ann_list, std::ostream &os) const
{
    auto bool_attributes = meta.GetBoolAttributes();
    auto attributes = meta.GetAttributes();
    if (bool_attributes.empty() && attributes.empty() && ann_list.empty()) {
        return;
    }

    os << " <";

    size_t size = bool_attributes.size();
    size_t idx = 0;
    for (const auto &attr : bool_attributes) {
        os << attr;
        ++idx;

        if (!attributes.empty() || !ann_list.empty() || idx < size) {
            os << ", ";
        }
    }

    size = attributes.size();
    idx = 0;
    for (const auto &[key, values] : attributes) {
        for (size_t i = 0; i < values.size(); i++) {
            os << key << "=" << values[i];

            if (i < values.size() - 1) {
                os << ", ";
            }
        }

        ++idx;

        if (!ann_list.empty() || idx < size) {
            os << ", ";
        }
    }

    size = ann_list.size();
    idx = 0;
    for (const auto &[key, value] : ann_list) {
        os << key << "=" << value;

        ++idx;

        if (idx < size) {
            os << ", ";
        }
    }

    os << ">";
}

void Disassembler::SerializeLineNumberTable(const panda_file::LineNumberTable &line_number_table,
                                            std::ostream &os) const
{
    if (line_number_table.empty()) {
        return;
    }

    os << "\n#   LINE_NUMBER_TABLE:\n";
    for (const auto &line_info : line_number_table) {
        os << "#\tline " << line_info.line << ": " << line_info.offset << "\n";
    }
}

void Disassembler::SerializeLocalVariableTable(const panda_file::LocalVariableTable &local_variable_table,
                                               const pandasm::Function &method, std::ostream &os) const
{
    if (local_variable_table.empty()) {
        return;
    }

    os << "\n#   LOCAL_VARIABLE_TABLE:\n";
    os << "#\t Start   End  Register           Name   Signature\n";
    const int start_width = 5;
    const int end_width = 4;
    const int reg_width = 8;
    const int name_width = 14;
    for (const auto &variable_info : local_variable_table) {
        std::ostringstream reg_stream;
        reg_stream << variable_info.reg_number << '(';
        if (variable_info.reg_number < 0) {
            reg_stream << "acc";
        } else {
            uint32_t vreg = variable_info.reg_number;
            uint32_t first_arg_reg = method.GetTotalRegs();
            if (vreg < first_arg_reg) {
                reg_stream << 'v' << vreg;
            } else {
                reg_stream << 'a' << vreg - first_arg_reg;
            }
        }
        reg_stream << ')';

        os << "#\t " << std::setw(start_width) << std::right << variable_info.start_offset << "  ";
        os << std::setw(end_width) << std::right << variable_info.end_offset << "  ";
        os << std::setw(reg_width) << std::right << reg_stream.str() << " ";
        os << std::setw(name_width) << std::right << variable_info.name << "   " << variable_info.type;
        if (!variable_info.type_signature.empty() && variable_info.type_signature != variable_info.type) {
            os << " (" << variable_info.type_signature << ")";
        }
        os << "\n";
    }
}

void Disassembler::SerializeLanguage(std::ostream &os) const
{
    os << ".language " << panda::panda_file::LanguageToString(file_language_) << "\n\n";
}

void Disassembler::SerializeFilename(std::ostream &os) const
{
    if (file_ == nullptr || file_->GetFilename().empty()) {
        return;
    }

    os << "# source binary: " << file_->GetFilename() << "\n\n";
}

void Disassembler::SerializeLitArrays(std::ostream &os, bool add_separators) const
{
    LOG(DEBUG, DISASSEMBLER) << "[serializing literals]";

    if (prog_.literalarray_table.empty()) {
        return;
    }

    if (add_separators) {
        os << "# ====================\n"
              "# LITERALS\n\n";
    }

    for (const auto &pair : prog_.literalarray_table) {
        Serialize(pair.first, pair.second, os);
    }

    os << "\n";
}

void Disassembler::SerializeRecords(std::ostream &os, bool add_separators, bool print_information) const
{
    LOG(DEBUG, DISASSEMBLER) << "[serializing records]";

    if (prog_.record_table.empty()) {
        return;
    }

    if (add_separators) {
        os << "# ====================\n"
              "# RECORDS\n\n";
    }

    for (const auto &r : prog_.record_table) {
        if (!panda_file::IsDummyClassName(r.first)) {
            Serialize(r.second, os, print_information);
        } else {
            SerializeUnionFields(r.second, os, print_information);
        }
    }
}

void Disassembler::SerializeMethods(std::ostream &os, bool add_separators, bool print_information) const
{
    LOG(DEBUG, DISASSEMBLER) << "[serializing methods]";

    if (prog_.function_table.empty()) {
        return;
    }

    if (add_separators) {
        os << "# ====================\n"
              "# METHODS\n\n";
    }

    for (const auto &m : prog_.function_table) {
        Serialize(m.second, os, print_information);
    }
}

pandasm::Opcode Disassembler::BytecodeOpcodeToPandasmOpcode(uint8_t o) const
{
    return BytecodeOpcodeToPandasmOpcode(BytecodeInstruction::Opcode(o));
}

std::string Disassembler::IDToString(BytecodeInstruction bc_ins, panda_file::File::EntityId method_id) const
{
    std::stringstream name;

    if (bc_ins.HasFlag(BytecodeInstruction::Flags::TYPE_ID)) {
        auto idx = bc_ins.GetId().AsIndex();
        auto id = file_->ResolveClassIndex(method_id, idx);
        auto type = pandasm::Type::FromDescriptor(StringDataToString(file_->GetStringData(id)));

        name.str("");
        name << type.GetPandasmName();
    } else if (bc_ins.HasFlag(BytecodeInstruction::Flags::METHOD_ID)) {
        auto idx = bc_ins.GetId().AsIndex();
        auto id = file_->ResolveMethodIndex(method_id, idx);

        name << GetMethodSignature(id);
    } else if (bc_ins.HasFlag(BytecodeInstruction::Flags::STRING_ID)) {
        name << '\"';

        if (skip_strings_ || quiet_) {
            name << std::hex << "0x" << bc_ins.GetId().AsFileId();
        } else {
            name << StringDataToString(file_->GetStringData(bc_ins.GetId().AsFileId()));
        }

        name << '\"';
    } else if (bc_ins.HasFlag(BytecodeInstruction::Flags::FIELD_ID)) {
        auto idx = bc_ins.GetId().AsIndex();
        auto id = file_->ResolveFieldIndex(method_id, idx);
        panda_file::FieldDataAccessor field_accessor(*file_, id);

        auto record_name = GetFullRecordName(field_accessor.GetClassId());
        if (!panda_file::IsDummyClassName(record_name)) {
            name << record_name;
            name << '.';
        }
        name << StringDataToString(file_->GetStringData(field_accessor.GetNameId()));
    } else if (bc_ins.HasFlag(BytecodeInstruction::Flags::LITERALARRAY_ID)) {
        auto index = bc_ins.GetId().AsIndex();
        name << "array_" << index;
    }

    return name.str();
}

panda::panda_file::SourceLang Disassembler::GetRecordLanguage(panda_file::File::EntityId class_id) const
{
    if (file_->IsExternal(class_id)) {
        return panda::panda_file::SourceLang::PANDA_ASSEMBLY;
    }

    panda_file::ClassDataAccessor cda(*file_, class_id);
    return cda.GetSourceLang().value_or(panda_file::SourceLang::PANDA_ASSEMBLY);
}

static void TranslateImmToLabel(pandasm::Ins *pa_ins, LabelTable *label_table, const uint8_t *ins_arr,
                                BytecodeInstruction bc_ins, BytecodeInstruction bc_ins_last,
                                panda_file::File::EntityId code_id)
{
    const int32_t jmp_offset = std::get<int64_t>(pa_ins->imms.at(0));
    const auto bc_ins_dest = bc_ins.JumpTo(jmp_offset);
    if (bc_ins_last.GetAddress() > bc_ins_dest.GetAddress()) {
        size_t idx = GetBytecodeInstructionNumber(BytecodeInstruction(ins_arr), bc_ins_dest);

        if (idx != std::numeric_limits<size_t>::max()) {
            if (label_table->find(idx) == label_table->end()) {
                std::stringstream ss {};
                ss << "jump_label_" << label_table->size();
                (*label_table)[idx] = ss.str();
            }

            pa_ins->imms.clear();
            pa_ins->ids.push_back(label_table->at(idx));
        } else {
            LOG(ERROR, DISASSEMBLER) << "> error encountered at " << code_id << " (0x" << std::hex << code_id
                                     << "). incorrect instruction at offset: 0x" << (bc_ins.GetAddress() - ins_arr)
                                     << ": invalid jump offset 0x" << jmp_offset
                                     << " - jumping in the middle of another instruction!";
        }
    } else {
        LOG(ERROR, DISASSEMBLER) << "> error encountered at " << code_id << " (0x" << std::hex << code_id
                                 << "). incorrect instruction at offset: 0x" << (bc_ins.GetAddress() - ins_arr)
                                 << ": invalid jump offset 0x" << jmp_offset << " - jumping out of bounds!";
    }
}

IdList Disassembler::GetInstructions(pandasm::Function *method, panda_file::File::EntityId method_id,
                                     panda_file::File::EntityId code_id)
{
    panda_file::CodeDataAccessor code_accessor(*file_, code_id);

    const auto ins_sz = code_accessor.GetCodeSize();
    const auto ins_arr = code_accessor.GetInstructions();

    method->regs_num = code_accessor.GetNumVregs();

    auto bc_ins = BytecodeInstruction(ins_arr);
    auto from = bc_ins.GetAddress();
    const auto bc_ins_last = bc_ins.JumpTo(ins_sz);

    LabelTable label_table = GetExceptions(method, method_id, code_id);

    IdList unknown_external_methods {};

    while (bc_ins.GetAddress() != bc_ins_last.GetAddress()) {
        if (bc_ins.GetAddress() > bc_ins_last.GetAddress()) {
            LOG(ERROR, DISASSEMBLER) << "> error encountered at " << code_id << " (0x" << std::hex << code_id
                                     << "). bytecode instructions sequence corrupted for method " << method->name
                                     << "! went out of bounds";

            break;
        }

        if (bc_ins.HasFlag(BytecodeInstruction::Flags::FIELD_ID)) {
            auto idx = bc_ins.GetId().AsIndex();
            auto id = file_->ResolveFieldIndex(method_id, idx);
            panda_file::FieldDataAccessor field_accessor(*file_, id);

            if (field_accessor.IsExternal()) {
                auto record_name = GetFullRecordName(field_accessor.GetClassId());

                pandasm::Field field(file_language_);
                GetField(field, field_accessor);

                auto &field_list = external_field_table_[record_name];
                auto ret_field =
                    std::find_if(field_list.begin(), field_list.end(), [&field](pandasm::Field &field_from_list) {
                        return field.name == field_from_list.name;
                    });
                if (ret_field == field_list.end()) {
                    field_list.push_back(std::move(field));
                }
            }
        }

        auto pa_ins = BytecodeInstructionToPandasmInstruction(bc_ins, method_id);
        pa_ins.ins_debug.bound_left =
            bc_ins.GetAddress() - from;  // It is used to produce a line table during method serialization
        if (pa_ins.IsJump()) {
            TranslateImmToLabel(&pa_ins, &label_table, ins_arr, bc_ins, bc_ins_last, code_id);
        }

        // check if method id is unknown external method. if so, emplace it in table
        if (bc_ins.HasFlag(BytecodeInstruction::Flags::METHOD_ID)) {
            const auto arg_method_idx = bc_ins.GetId().AsIndex();
            const auto arg_method_id = file_->ResolveMethodIndex(method_id, arg_method_idx);

            const auto arg_method_signature = GetMethodSignature(arg_method_id);

            const bool is_present = prog_.function_table.find(arg_method_signature) != prog_.function_table.cend();
            const bool is_external = file_->IsExternal(arg_method_id);

            if (is_external && !is_present) {
                unknown_external_methods.push_back(arg_method_id);
            }
        }

        method->ins.push_back(pa_ins);
        bc_ins = bc_ins.GetNext();
    }

    for (const auto &pair : label_table) {
        method->ins[pair.first].label = pair.second;
        method->ins[pair.first].set_label = true;
    }

    return unknown_external_methods;
}

}  // namespace panda::disasm
