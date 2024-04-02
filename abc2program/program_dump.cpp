/*
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

#include "program_dump.h"
#include "method_data_accessor-inl.h"
#include "abc2program_log.h"
#include "common/abc_file_utils.h"
#include "os/file.h"

namespace panda::abc2program {

void PandasmProgramDumper::Dump(std::ostream &os, const pandasm::Program &program)
{
    program_ = &program;
    os << std::flush;
    DumpAbcFilePath(os);
    DumpProgramLanguage(os);
    DumpLiteralArrayTable(os);
    DumpRecordTable(os);
    DumpFunctionTable(os);
    DumpStrings(os);
}

void PandasmProgramDumper::SetDumperSource(ProgramDumperSource dumper_source)
{
    dumper_source_ = dumper_source;
}

void PandasmProgramDumper::SetAbcFilePath(const std::string &abc_file_path)
{
    abc_file_path_ = abc_file_path;
}

void PandasmProgramDumper::DumpAbcFilePath(std::ostream &os) const
{
    if (dumper_source_ == ProgramDumperSource::ECMASCRIPT) {
        return;
    }
    std::string file_abs_path = os::file::File::GetAbsolutePath(abc_file_path_).Value();
    os << DUMP_TITLE_SOURCE_BINARY << file_abs_path << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpProgramLanguage(std::ostream &os) const
{
    os << DUMP_TITLE_LANGUAGE;
    if (program_->lang == panda::panda_file::SourceLang::ECMASCRIPT) {
        os << DUMP_CONTENT_ECMASCRIPT;
    } else {
        os << DUMP_CONTENT_PANDA_ASSEMBLY;
    }
    os << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpLiteralArrayTable(std::ostream &os) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_LITERALS;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    std::vector<std::string> literal_array_contents;
    auto it = program_->literalarray_table.begin();
    auto end = program_->literalarray_table.end();
    for (; it != end; ++it) {
        literal_array_contents.emplace_back(SerializeLiteralArray(it->second));
    }
    for (const std::string &str : literal_array_contents) {
        os << str << DUMP_CONTENT_SINGLE_ENDL;
    }
    os << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpRecordTable(std::ostream &os) const
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_RECORDS;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    for (const auto &it : program_->record_table) {
        DumpRecord(os, it.second);
    }
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpRecord(std::ostream &os, const pandasm::Record &record) const
{
    if (dumper_source_ == ProgramDumperSource::ECMASCRIPT) {
        if (AbcFileUtils::IsGlobalTypeName(record.name)) {
            return;
        }
        if (AbcFileUtils::IsESTypeAnnotationName(record.name)) {
            return;
        }
    }
    os << DUMP_TITLE_RECORD << record.name;
    if (DumpRecordMetaData(os, record)) {
        DumpFieldList(os, record);
    }
    DumpRecordSourceFile(os, record);
}

bool PandasmProgramDumper::DumpRecordMetaData(std::ostream &os, const pandasm::Record &record) const
{
    if (record.metadata->IsForeign()) {
        os << DUMP_CONTENT_SPACE << DUMP_CONTENT_ATTR_EXTERNAL;
        os << DUMP_CONTENT_SINGLE_ENDL;
        return false;
    }
    return true;
}

void PandasmProgramDumper::DumpFieldList(std::ostream &os, const pandasm::Record &record) const
{
    os << " {" << DUMP_CONTENT_SINGLE_ENDL;
    for (const auto &it : record.field_list) {
        DumpField(os, it);
    }
    os << DUMP_CONTENT_SINGLE_ENDL << "}" << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpField(std::ostream &os, const pandasm::Field &field) const
{
    os << DUMP_CONTENT_TAB << field.type.GetPandasmName() << DUMP_CONTENT_SPACE << field.name;
    DumpFieldMetaData(os, field);
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpFieldMetaData(std::ostream &os, const pandasm::Field &field) const
{
    if (field.metadata->GetValue()) {
        DumpScalarValue(os, *(field.metadata->GetValue()));
    }
    os << std::endl;
}

void PandasmProgramDumper::DumpAnnotationData(std::ostream &os, const pandasm::AnnotationData &anno) const
{
    os << DUMP_CONTENT_SPACE << anno.GetName() << DUMP_CONTENT_SINGLE_ENDL;
    for (const auto &element : anno.GetElements()) {
        os << DUMP_CONTENT_SPACE << element.GetName();
        if (element.GetValue()->IsArray()) {
            DumpArrayValue(os, *(element.GetValue()->GetAsArray()));
        } else {
            DumpScalarValue(os, *(element.GetValue()->GetAsScalar()));
        }
    }
}

void PandasmProgramDumper::DumpArrayValue(std::ostream &os, const pandasm::ArrayValue &array) const
{
    for (const auto &val : array.GetValues()) {
        DumpScalarValue(os, val);
    }
}
void PandasmProgramDumper::DumpScalarValue(std::ostream &os, const pandasm::ScalarValue &scalar) const
{
    switch (scalar.GetType()) {
        case pandasm::Value::Type::U1:
        case pandasm::Value::Type::I8:
        case pandasm::Value::Type::U8:
        case pandasm::Value::Type::I16:
        case pandasm::Value::Type::U16:
        case pandasm::Value::Type::I32:
        case pandasm::Value::Type::U32:
        case pandasm::Value::Type::I64:
        case pandasm::Value::Type::U64:
        case pandasm::Value::Type::STRING_NULLPTR: {
            os << DUMP_CONTENT_SPACE << scalar.GetValue<uint64_t>();
            break;
        }
        case pandasm::Value::Type::F32:
            os << DUMP_CONTENT_SPACE << scalar.GetValue<float>();
            break;
        case pandasm::Value::Type::F64: {
            os << DUMP_CONTENT_SPACE << scalar.GetValue<double>();
            break;
        }
        case pandasm::Value::Type::STRING:
        case pandasm::Value::Type::METHOD:
        case pandasm::Value::Type::ENUM:
        case pandasm::Value::Type::LITERALARRAY: {
            os << DUMP_CONTENT_SPACE << scalar.GetValue<std::string>();
            break;
        }
        case pandasm::Value::Type::RECORD: {
            pandasm::Type type = scalar.GetValue<pandasm::Type>();
            os << DUMP_CONTENT_SPACE << type.GetComponentName() << DUMP_CONTENT_SPACE << type.GetName();
            break;
        }
        case pandasm::Value::Type::ANNOTATION: {
            DumpAnnotationData(os, scalar.GetValue<pandasm::AnnotationData>());
            break;
        }
        default :
            break;
    }
}

void PandasmProgramDumper::DumpRecordSourceFile(std::ostream &os, const pandasm::Record &record) const
{
    os << DUMP_TITLE_RECORD_SOURCE_FILE << record.source_file << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionTable(std::ostream &os)
{
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_METHODS;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    for (const auto &it : program_->function_table) {
        DumpFunction(os, it.second);
    }
}

void PandasmProgramDumper::DumpFunction(std::ostream &os, const pandasm::Function &function)
{
    regs_num_ = function.regs_num;
    DumpFunctionAnnotations(os, function);
    DumpFunctionHead(os, function);
    DumpFunctionBody(os, function);
}

void PandasmProgramDumper::DumpFunctionAnnotations(std::ostream &os, const pandasm::Function &function) const
{
    for (auto &annotation : function.metadata->GetAnnotations()) {
        DumpAnnotationData(os, annotation);
    }
    os << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionHead(std::ostream &os, const pandasm::Function &function) const
{
    os << DUMP_TITLE_FUNCTION;
    DumpFunctionName(os, function);
    os << " {" << DUMP_CONTENT_SINGLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionReturnType(std::ostream &os, const pandasm::Function &function) const
{
    os << function.return_type.GetPandasmName() << DUMP_CONTENT_SPACE;
}

void PandasmProgramDumper::DumpFunctionName(std::ostream &os, const pandasm::Function &function) const
{
    os << function.name;
}

void PandasmProgramDumper::DumpFunctionParams(std::ostream &os, const pandasm::Function &function) const
{
    os << "(";
    if (function.params.size() > 0) {
        DumpFunctionParamAtIndex(os, function.params[0], 0);
        for (size_t i = 1; i < function.params.size(); ++i) {
            os << ", ";
            DumpFunctionParamAtIndex(os, function.params[i], i);
        }
    }
    os << ")";
}

void PandasmProgramDumper::DumpFunctionParamAtIndex(std::ostream &os,
                                                    const pandasm::Function::Parameter &param,
                                                    size_t idx) const
{
    os << param.type.GetPandasmName() << DUMP_CONTENT_SPACE << DUMP_CONTENT_FUNCTION_PARAM_NAME_PREFIX << idx;
}

void PandasmProgramDumper::DumpFunctionAttributes(std::ostream &os, const pandasm::Function &function) const
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

void PandasmProgramDumper::DumpFunctionBody(std::ostream &os, const pandasm::Function &function)
{
    DumpFunctionIns(os, function);
    DumpFunctionCatchBlocks(os, function);
    DumpFunctionDebugInfo(os, function);
    os << "}" << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::DumpFunctionIns(std::ostream &os, const pandasm::Function &function)
{
    if (dumper_source_ == ProgramDumperSource::ECMASCRIPT) {
        DumpFunctionIns4EcmaScript(os, function);
    } else {
        DumpFunctionIns4PandaAssembly(os, function);
    }
}

void PandasmProgramDumper::DumpFunctionIns4PandaAssembly(std::ostream &os, const pandasm::Function &function)
{
    for (const pandasm::Ins &pa_ins : function.ins) {
        std::string insStr = pa_ins.ToString("", true, regs_num_);
        os << DUMP_CONTENT_TAB << std::setw(LINE_WIDTH)
           << std::left << insStr
           << DUMP_CONTENT_LINE_NUMBER << pa_ins.ins_debug.line_number;
        os << std::setw(COLUMN_WIDTH) << std::left << DUMP_CONTENT_SPACE
           << DUMP_CONTENT_COLUMN_NUMBER << pa_ins.ins_debug.column_number
           << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::DumpFunctionIns4EcmaScript(std::ostream &os, const pandasm::Function &function)
{
    GetOriginalDumpIns(function);
    GetInvalidOpLabelMap();
    UpdateLabels4DumpIns(original_dump_ins_ptrs_, invalid_op_label_to_nearest_valid_op_label_map_);
    GetFinalDumpIns();
    GetFinallyLabelMap();
    UpdateLabels4DumpIns(final_dump_ins_ptrs_, finally_label_map_);
    DumpFinallyIns(os);
}

void PandasmProgramDumper::GetOriginalDumpIns(const pandasm::Function &function)
{
    original_dump_ins_.clear();
    original_dump_ins_ptrs_.clear();
    original_ins_index_map_.clear();
    for (const pandasm::Ins &pa_ins : function.ins) {
        original_dump_ins_.emplace_back(DeepCopyIns(pa_ins));
    }
    uint32_t idx = 0;
    for (pandasm::Ins &pa_ins : original_dump_ins_) {
        original_dump_ins_ptrs_.emplace_back(&pa_ins);
        original_ins_index_map_[&pa_ins] = idx;
        idx++;
    }
}

pandasm::Ins PandasmProgramDumper::DeepCopyIns(const pandasm::Ins &input) const
{
    std::string insStr = input.ToString("", true, regs_num_);
    pandasm::Ins res{};
    res.opcode = input.opcode;
    for (size_t i = 0; i < input.regs.size(); ++i) {
        res.regs.emplace_back(input.regs[i]);
    }
    for (size_t i = 0; i < input.ids.size(); ++i) {
        std::string new_id(input.ids[i]);
        res.ids.emplace_back(new_id);
    }
    for (size_t i = 0; i < input.imms.size(); ++i) {
        pandasm::Ins::IType new_imm = input.imms[i];
        res.imms.emplace_back(new_imm);
    }
    res.label = input.label;
    res.set_label = input.set_label;
    pandasm::debuginfo::Ins debug_ins{};
    debug_ins.line_number = input.ins_debug.line_number;
    debug_ins.column_number = input.ins_debug.column_number;
    debug_ins.whole_line = input.ins_debug.whole_line;
    debug_ins.bound_left = input.ins_debug.bound_left;
    debug_ins.bound_right = input.ins_debug.bound_right;
    res.ins_debug = debug_ins;
    return res;
}

void PandasmProgramDumper::GetFinalDumpIns()
{
    final_dump_ins_ptrs_.clear();
    final_ins_index_map_.clear();
    uint32_t idx = 0;
    for (pandasm::Ins *pa_ins : original_dump_ins_ptrs_) {
        final_ins_index_map_[pa_ins] = idx;
        if (pa_ins->opcode != pandasm::Opcode::INVALID) {
            final_dump_ins_ptrs_.emplace_back(pa_ins);
            idx++;
        }
    }
}

void PandasmProgramDumper::DumpFinallyIns(std::ostream &os)
{
    for (pandasm::Ins *pa_ins : final_dump_ins_ptrs_) {
        if (IsMatchLiteralId(*pa_ins)) {
            ReplaceLiteralId4Ins(*pa_ins);
        }
        std::string insStr = pa_ins->ToString("", true, regs_num_);
        os << DUMP_CONTENT_TAB << std::setw(LINE_WIDTH)
           << std::left << insStr
           << DUMP_CONTENT_LINE_NUMBER << pa_ins->ins_debug.line_number;
        os << std::setw(COLUMN_WIDTH) << std::left << DUMP_CONTENT_SPACE
           << DUMP_CONTENT_COLUMN_NUMBER << pa_ins->ins_debug.column_number
           << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::GetInvalidOpLabelMap()
{
    invalid_op_label_to_nearest_valid_op_label_map_.clear();
    size_t dump_ins_size = original_dump_ins_.size();
    for (size_t i = 0; i < dump_ins_size; ++i) {
        pandasm::Ins &curr_ins = original_dump_ins_[i];
        if (curr_ins.opcode == pandasm::Opcode::INVALID) {
            HandleInvalidopInsLabel(i, curr_ins);
        }
    }
}

void PandasmProgramDumper::HandleInvalidopInsLabel(size_t invalid_op_idx, pandasm::Ins &invalid_op_ins)
{
    if (!invalid_op_ins.set_label) {
        return;
    }
    pandasm::Ins *nearest_valid_op_ins = GetNearestValidopIns4InvalidopIns(invalid_op_idx);
    if (nearest_valid_op_ins == nullptr) {
        return;
    }
    if (!nearest_valid_op_ins->set_label) {
        // here, the invalid op ins and its nearest valid op ins has the same label
        // the invalid op will be removed, so the label is still unique for each inst
        nearest_valid_op_ins->label = invalid_op_ins.label;
        nearest_valid_op_ins->set_label = true;
    }
    invalid_op_label_to_nearest_valid_op_label_map_.emplace(invalid_op_ins.label, nearest_valid_op_ins->label);
}

pandasm::Ins *PandasmProgramDumper::GetNearestValidopIns4InvalidopIns(size_t invalid_op_ins_idx)
{
    size_t dump_ins_size = original_dump_ins_.size();
    // search downwards
    for (size_t i = invalid_op_ins_idx + 1; i < dump_ins_size; ++i) {
        pandasm::Ins &curr_ins = original_dump_ins_[i];
        if (curr_ins.opcode != pandasm::Opcode::INVALID) {
            return &curr_ins;
        }
    }
    // search upwards
    for (size_t i = invalid_op_ins_idx - 1; i >= 0; --i) {
        pandasm::Ins &curr_ins = original_dump_ins_[i];
        if (curr_ins.opcode != pandasm::Opcode::INVALID) {
            return &curr_ins;
        }
    }
    return nullptr;
}

void PandasmProgramDumper::UpdateLabels4DumpIns(std::vector<pandasm::Ins*> &dump_ins, const LabelMap &label_map) const
{
    size_t dump_ins_size = dump_ins.size();
    for (size_t i = 0; i < dump_ins_size; ++i) {
        UpdateLabels4DumpInsAtIndex(i, dump_ins, label_map);
    }
}

void PandasmProgramDumper::UpdateLabels4DumpInsAtIndex(size_t idx, std::vector<pandasm::Ins*> &dump_ins,
                                                       const LabelMap &label_map) const
{
    pandasm::Ins *curr_ins = dump_ins[idx];
    std::string mapped_label = GetMappedLabel(curr_ins->label, label_map);
    if (mapped_label != "") {
        curr_ins->label = mapped_label;
    }
    if (curr_ins->IsJump()) {
        mapped_label = GetMappedLabel(curr_ins->ids[0], label_map);
        if (mapped_label != "") {
            curr_ins->ids.clear();
            curr_ins->ids.emplace_back(mapped_label);
        }
    }
}

std::string PandasmProgramDumper::GetMappedLabel(const std::string &label, const LabelMap &label_map) const
{
    auto it = label_map.find(label);
    if (it != label_map.end()) {
        return it->second;
    } else {
        return "";
    }
}

void PandasmProgramDumper::GetFinallyLabelMap()
{
    finally_label_map_.clear();
    size_t dump_ins_size = final_dump_ins_ptrs_.size();
    for (size_t i = 0; i < dump_ins_size; ++i) {
        SetFinallyLabelAtIndex(i);
    }
}

void PandasmProgramDumper::SetFinallyLabelAtIndex(size_t idx)
{
    pandasm::Ins *curr_ins = final_dump_ins_ptrs_[idx];
    std::string new_label_name = AbcFileUtils::GetLabelNameByInstIdx(idx);
    if (curr_ins->set_label) {
        finally_label_map_.emplace(curr_ins->label, new_label_name);
    } else {
        curr_ins->label = new_label_name;
        curr_ins->set_label = true;
    }
}

void PandasmProgramDumper::DumpFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const
{
    if (dumper_source_ == ProgramDumperSource::ECMASCRIPT) {
        DumpFunctionCatchBlocks4EcmaScript(os, function);
    } else {
        DumpFunctionCatchBlocks4PandaAssembly(os, function);
    }
}

void PandasmProgramDumper::DumpFunctionCatchBlocks4PandaAssembly(std::ostream &os,
                                                                 const pandasm::Function &function) const
{
    for (const pandasm::Function::CatchBlock &catch_block : function.catch_blocks) {
        DumpCatchBlock(os, catch_block);
    }
}

void PandasmProgramDumper::DumpFunctionCatchBlocks4EcmaScript(std::ostream &os,
                                                              const pandasm::Function &function) const
{
    std::vector<pandasm::Function::CatchBlock> catch_blocks;
    for (const pandasm::Function::CatchBlock &catch_block : function.catch_blocks) {
        catch_blocks.emplace_back(DeepCopyCatchBlock(catch_block));
    }
    for (pandasm::Function::CatchBlock &catch_block : catch_blocks) {
        UpdateCatchBlock(catch_block);
    }
    for (const pandasm::Function::CatchBlock &catch_block : catch_blocks) {
        DumpCatchBlock(os, catch_block);
    }
}

void PandasmProgramDumper::UpdateCatchBlock(pandasm::Function::CatchBlock &catch_block) const
{
    catch_block.try_begin_label = GetUpdatedCatchBlockLabel(catch_block.try_begin_label);
    catch_block.try_end_label = GetUpdatedCatchBlockLabel(catch_block.try_end_label);
    catch_block.catch_begin_label = GetUpdatedCatchBlockLabel(catch_block.catch_begin_label);
    catch_block.catch_end_label = GetUpdatedCatchBlockLabel(catch_block.catch_end_label);
}

std::string PandasmProgramDumper::GetUpdatedCatchBlockLabel(const std::string &orignal_label) const
{
    std::string updated_label = orignal_label;
    std::string mapped_label1 = GetMappedLabel(orignal_label, invalid_op_label_to_nearest_valid_op_label_map_);
    if (mapped_label1 == "") {
        return orignal_label;
    }
    std::string mapped_label2 = GetMappedLabel(mapped_label1, finally_label_map_);
    if (mapped_label2 == "") {
        return mapped_label1;
    }
    return mapped_label2;
}

pandasm::Function::CatchBlock PandasmProgramDumper::DeepCopyCatchBlock(
    const pandasm::Function::CatchBlock &catch_block) const
{
    pandasm::Function::CatchBlock res{};
    res.whole_line = catch_block.whole_line;
    res.exception_record = catch_block.exception_record;
    res.try_begin_label = catch_block.try_begin_label;
    res.try_end_label = catch_block.try_end_label;
    res.catch_begin_label = catch_block.catch_begin_label;
    res.catch_end_label = catch_block.catch_end_label;
    return res;
}

void PandasmProgramDumper::DumpCatchBlock(std::ostream &os, const pandasm::Function::CatchBlock &catch_block) const
{
    if (catch_block.exception_record.empty()) {
        os << DUMP_TITLE_CATCH_ALL << DUMP_CONTENT_SINGLE_ENDL;
    } else {
        os << DUMP_TITLE_CATCH << DUMP_CONTENT_SINGLE_ENDL;
    }
    os << DUMP_CONTENT_TAB << DUMP_CONTENT_TRY_BEGIN_LABEL
       << catch_block.try_begin_label << DUMP_CONTENT_SINGLE_ENDL;
    os << DUMP_CONTENT_TAB << DUMP_CONTENT_TRY_END_LABEL
       << catch_block.try_end_label << DUMP_CONTENT_SINGLE_ENDL;
    os << DUMP_CONTENT_TAB << DUMP_CONTENT_CATCH_BEGIN_LABEL
       << catch_block.catch_begin_label << DUMP_CONTENT_SINGLE_ENDL;
    if (dumper_source_ == ProgramDumperSource::PANDA_ASSEMBLY) {
        os << DUMP_CONTENT_TAB << DUMP_CONTENT_CATCH_END_LABEL
           << catch_block.catch_end_label << DUMP_CONTENT_SINGLE_ENDL;
    }
}

void PandasmProgramDumper::DumpFunctionDebugInfo(std::ostream &os, const pandasm::Function &function)
{
    if (function.local_variable_debug.empty()) {
        return;
    }
    std::map<int32_t, panda::pandasm::debuginfo::LocalVariable> local_variable_table;
    if (dumper_source_ == ProgramDumperSource::ECMASCRIPT) {
        UpdateLocalVarMap(function, local_variable_table);
    } else {
        for (const auto &variable_info : function.local_variable_debug) {
            local_variable_table[variable_info.reg] = variable_info;
        }
    }

    os << DUMP_CONTENT_SINGLE_ENDL;
    os << DUMP_TITLE_LOCAL_VAR_TABLE;
    os << DUMP_CONTENT_DOUBLE_ENDL;
    os << DUMP_CONTENT_LOCAL_VAR_TABLE;
    for (const auto &iter : local_variable_table) {
        const auto &variable_info = iter.second;
        os << DUMP_CONTENT_TAB
           << std::setw(START_WIDTH) << std::right << variable_info.start << DUMP_CONTENT_TRIPLE_SPACES;
        os << std::setw(END_WIDTH) << std::right << variable_info.length << DUMP_CONTENT_DOUBLE_SPACES;
        os << std::setw(REG_WIDTH) << std::right << variable_info.reg << DUMP_CONTENT_DOUBLE_SPACES;
        os << std::setw(NAME_WIDTH)
           << std::right << variable_info.name << DUMP_CONTENT_NONUPLE_SPACES << variable_info.signature;
        if (!variable_info.signature_type.empty() && variable_info.signature_type != variable_info.signature) {
            os << " (" << variable_info.signature_type << ")";
        }
        os << DUMP_CONTENT_SINGLE_ENDL;
    }
    os << DUMP_CONTENT_DOUBLE_ENDL;
}

void PandasmProgramDumper::UpdateLocalVarMap(const pandasm::Function &function,
    std::map<int32_t, panda::pandasm::debuginfo::LocalVariable>& local_variable_table)
{
    std::unordered_map<uint32_t, uint32_t> original_to_final_index_map_;
    uint32_t max_original_idx = 0;
    uint32_t max_final_idx = 0;
    for (const auto &[key, value] : original_ins_index_map_) {
        uint32_t final_idx = final_ins_index_map_[key];
        original_to_final_index_map_[value] = final_idx;
        if (value > max_original_idx) {
            max_original_idx = value;
            max_final_idx = final_idx;
        }
    }
    original_to_final_index_map_[max_original_idx + 1] = max_final_idx;

    for (const auto &variable_info : function.local_variable_debug) {
        uint32_t original_start = variable_info.start;
        uint32_t original_end = variable_info.length + variable_info.start;
        uint32_t new_start = original_to_final_index_map_[original_start];
        uint32_t new_length = original_to_final_index_map_[original_end] - new_start;
        panda::pandasm::debuginfo::LocalVariable local_var = {variable_info.name,
                                                              variable_info.signature,
                                                              variable_info.signature_type,
                                                              variable_info.reg,
                                                              new_start,
                                                              new_length};
        local_variable_table[variable_info.reg] = local_var;
    }
}

void PandasmProgramDumper::DumpStrings(std::ostream &os) const
{
    if (dumper_source_ == ProgramDumperSource::ECMASCRIPT) {
        return;
    }
    os << DUMP_TITLE_SEPARATOR;
    os << DUMP_TITLE_STRING;
    for (const std::string &str : program_->strings) {
        os << str << DUMP_CONTENT_SINGLE_ENDL;
    }
}

bool PandasmProgramDumper::IsMatchLiteralId(const pandasm::Ins &pa_ins) const
{
    return opcode_literal_id_index_map_.count(pa_ins.opcode);
}

void PandasmProgramDumper::ReplaceLiteralId4Ins(pandasm::Ins &pa_ins) const
{
    size_t idx = GetLiteralIdIndexInIns(pa_ins);
    std::string id_str = pa_ins.ids[idx];
    auto it = program_->literalarray_table.find(id_str);
    ASSERT(it != program_->literalarray_table.end());
    const pandasm::LiteralArray &literal_array = it->second;
    std::string replaced_value = SerializeLiteralArray(literal_array);
    pa_ins.ids[idx] = replaced_value;
}

std::string PandasmProgramDumper::SerializeLiteralArray(const pandasm::LiteralArray &lit_array) const
{
    if (lit_array.literals_.empty()) {
        return "";
    }
    std::stringstream ss;
    ss << "{ ";
    const auto &tag = lit_array.literals_[0].tag_;
    if (AbcFileUtils::IsLiteralTagArray(tag)) {
        ss << LiteralTagToString(tag);
    }
    ss << lit_array.literals_.size();
    ss << " [ ";
    SerializeValues(lit_array, ss);
    ss << "]}";
    return ss.str();
}

LiteralTagToStringMap PandasmProgramDumper::literal_tag_to_string_map_ = {
    {panda_file::LiteralTag::BOOL, "u1"},
    {panda_file::LiteralTag::ARRAY_U1, "u1"},
    {panda_file::LiteralTag::ARRAY_U8, "u8"},
    {panda_file::LiteralTag::ARRAY_I8, "i8"},
    {panda_file::LiteralTag::ARRAY_U16, "u16"},
    {panda_file::LiteralTag::ARRAY_I16, "i16"},
    {panda_file::LiteralTag::ARRAY_U32, "u32"},
    {panda_file::LiteralTag::INTEGER, "i32"},
    {panda_file::LiteralTag::ARRAY_I32, "i32"},
    {panda_file::LiteralTag::ARRAY_U64, "u64"},
    {panda_file::LiteralTag::ARRAY_I64, "i64"},
    {panda_file::LiteralTag::ARRAY_F32, "f32"},
    {panda_file::LiteralTag::DOUBLE, "f64"},
    {panda_file::LiteralTag::ARRAY_F64, "f64"},
    {panda_file::LiteralTag::STRING, "string"},
    {panda_file::LiteralTag::ARRAY_STRING, "string"},
    {panda_file::LiteralTag::METHOD, "method"},
    {panda_file::LiteralTag::GETTER, "getter"},
    {panda_file::LiteralTag::SETTER, "setter"},
    {panda_file::LiteralTag::GENERATORMETHOD, "generator_method"},
    {panda_file::LiteralTag::ACCESSOR, "accessor"},
    {panda_file::LiteralTag::METHODAFFILIATE, "method_affiliate"},
    {panda_file::LiteralTag::NULLVALUE, "null_value"},
    {panda_file::LiteralTag::TAGVALUE, "tagvalue"},
    {panda_file::LiteralTag::LITERALBUFFERINDEX, "lit_index"},
    {panda_file::LiteralTag::LITERALARRAY, "lit_offset"},
    {panda_file::LiteralTag::BUILTINTYPEINDEX, "builtin_type"},
};

std::string PandasmProgramDumper::LiteralTagToString(const panda_file::LiteralTag &tag) const
{
    auto it = literal_tag_to_string_map_.find(tag);
    ASSERT(it != literal_tag_to_string_map_.end());
    return it->second;
}

std::unordered_map<pandasm::Opcode, size_t> PandasmProgramDumper::opcode_literal_id_index_map_ = {
    {pandasm::Opcode::CREATEARRAYWITHBUFFER, 0},
    {pandasm::Opcode::CREATEOBJECTWITHBUFFER, 0},
    {pandasm::Opcode::DEFINECLASSWITHBUFFER, 1},
    {pandasm::Opcode::NEWLEXENVWITHNAME, 0},
    {pandasm::Opcode::WIDE_NEWLEXENVWITHNAME, 0},
    {pandasm::Opcode::CALLRUNTIME_CREATEPRIVATEPROPERTY, 0},
    {pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS, 1},
};

size_t PandasmProgramDumper::GetLiteralIdIndexInIns(const pandasm::Ins &pa_ins) const
{
    auto it = opcode_literal_id_index_map_.find(pa_ins.opcode);
    ASSERT(it != opcode_literal_id_index_map_.end());
    return it->second;
}


template <typename T>
void PandasmProgramDumper::SerializeValues(const pandasm::LiteralArray &lit_array, T &os) const
{
    const panda_file::LiteralTag &tag0 = lit_array.literals_[0].tag_;
    switch (tag0) {
        case panda_file::LiteralTag::ARRAY_U1:
            SerializeValues4ArrayU1(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_U8:
            SerializeValues4ArrayU8(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_I8:
            SerializeValues4ArrayI8(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_U16:
            SerializeValues4ArrayU16(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_I16:
            SerializeValues4ArrayI16(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_U32:
            SerializeValues4ArrayU32(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_I32:
            SerializeValues4ArrayI32(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_U64:
            SerializeValues4ArrayU64(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_I64:
            SerializeValues4ArrayI64(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_F32:
            SerializeValues4ArrayF32(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_F64:
            SerializeValues4ArrayF64(lit_array, os);
            break;
        case panda_file::LiteralTag::ARRAY_STRING:
            SerializeValues4ArrayString(lit_array, os);
            break;
        default:
            SerializeLiterals(lit_array, os);
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayU1(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (std::get<bool>(lit_array.literals_[i].value_)) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayU8(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (static_cast<uint16_t>(std::get<uint8_t>(lit_array.literals_[i].value_))) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayI8(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (static_cast<int16_t>(bit_cast<int8_t>(std::get<uint8_t>(lit_array.literals_[i].value_)))) <<
            DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayU16(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (std::get<uint16_t>(lit_array.literals_[i].value_)) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayI16(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (bit_cast<int16_t>(std::get<uint16_t>(lit_array.literals_[i].value_))) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayU32(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (std::get<uint32_t>(lit_array.literals_[i].value_)) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayI32(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (bit_cast<int32_t>(std::get<uint32_t>(lit_array.literals_[i].value_))) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayU64(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (std::get<uint64_t>(lit_array.literals_[i].value_)) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayI64(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (bit_cast<int64_t>(std::get<uint64_t>(lit_array.literals_[i].value_))) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayF32(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << (std::get<float>(lit_array.literals_[i].value_)) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayF64(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << std::get<double>(lit_array.literals_[i].value_) << DUMP_CONTENT_SPACE;
    }
}

template <typename T>
void PandasmProgramDumper::SerializeValues4ArrayString(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        os << "\"" << std::get<std::string>(lit_array.literals_[i].value_) << "\" ";
    }
}

template <typename T>
void PandasmProgramDumper::SerializeLiterals(const pandasm::LiteralArray &lit_array, T &os) const
{
    for (size_t i = 0; i < lit_array.literals_.size(); i++) {
        SerializeLiteralsAtIndex(lit_array, os, i);
        os << ", ";
    }
}

template <typename T>
void PandasmProgramDumper::SerializeLiteralsAtIndex(const pandasm::LiteralArray &lit_array, T &os, size_t i) const
{
    const panda_file::LiteralTag &tag = lit_array.literals_[i].tag_;
    os << LiteralTagToString(tag) << ":";
    const auto &val = lit_array.literals_[i].value_;
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
            os << (std::get<bool>(val));
            break;
        case panda_file::LiteralTag::LITERALBUFFERINDEX:
        case panda_file::LiteralTag::INTEGER:
            os << (bit_cast<int32_t>(std::get<uint32_t>(val)));
            break;
        case panda_file::LiteralTag::DOUBLE:
            os << (std::get<double>(val));
            break;
        case panda_file::LiteralTag::STRING:
            os << "\"" << (std::get<std::string>(val)) << "\"";
            break;
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GETTER:
        case panda_file::LiteralTag::SETTER:
        case panda_file::LiteralTag::GENERATORMETHOD:
            os << (std::get<std::string>(val));
            break;
        case panda_file::LiteralTag::NULLVALUE:
        case panda_file::LiteralTag::ACCESSOR:
            os << (static_cast<int16_t>(bit_cast<int8_t>(std::get<uint8_t>(val))));
            break;
        case panda_file::LiteralTag::METHODAFFILIATE:
            os << (std::get<uint16_t>(val));
            break;
        case panda_file::LiteralTag::LITERALARRAY:
            os << DUMP_CONTENT_NESTED_LITERALARRAY;  // we use "$$NESTED-LITERALARRAY$$" as a place holder
            break;
        case panda_file::LiteralTag::BUILTINTYPEINDEX:
            os << (static_cast<int16_t>(std::get<uint8_t>(val)));
            break;
        case panda_file::LiteralTag::TAGVALUE:
            os << (static_cast<int16_t>(std::get<uint8_t>(val)));
            break;
        default:
            LOG(FATAL, ABC2PROGRAM) << __PRETTY_FUNCTION__ << LiteralTagToString(tag) << " is unreachable!";
            break;
    }
}

} // namespace panda::abc2program
