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

#ifndef ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H
#define ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H

#include <iostream>
#include <assembly-program.h>
#include "common/abc_string_table.h"

namespace panda::abc2program {

enum class ProgramDumperSource {
    ECMASCRIPT = 0,
    PANDA_ASSEMBLY = 1,
};

using LiteralTagToStringMap = std::unordered_map<panda_file::LiteralTag, std::string>;
using LabelMap = std::unordered_map<std::string, std::string>;

class PandasmProgramDumper {
public:
    PandasmProgramDumper() {}
    void Dump(std::ostream &os, const pandasm::Program &program);
    void SetDumperSource(ProgramDumperSource dumper_source);
    void SetAbcFilePath(const std::string &abc_file_path);

private:
    void DumpAbcFilePath(std::ostream &os) const;
    void DumpProgramLanguage(std::ostream &os) const;
    void DumpLiteralArrayTable(std::ostream &os) const;
    void DumpRecordTable(std::ostream &os) const;
    void DumpRecord(std::ostream &os, const pandasm::Record &record) const;
    bool DumpRecordMetaData(std::ostream &os, const pandasm::Record &record) const;
    void DumpFieldList(std::ostream &os, const pandasm::Record &record) const;
    void DumpField(std::ostream &os, const pandasm::Field &field) const;
    void DumpFieldMetaData(std::ostream &os, const pandasm::Field &field) const;
    void DumpRecordSourceFile(std::ostream &os, const pandasm::Record &record) const;
    void DumpFunctionTable(std::ostream &os);
    void DumpFunction(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionAnnotations(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionHead(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionReturnType(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionName(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionParams(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionParamAtIndex(std::ostream &os, const pandasm::Function::Parameter &param, size_t idx) const;
    void DumpFunctionAttributes(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionBody(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionIns(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionIns4PandaAssembly(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionIns4EcmaScript(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionDebugInfo(std::ostream &os, const pandasm::Function &function);
    void UpdateLocalVarMap(const pandasm::Function &function,
        std::map<int32_t, panda::pandasm::debuginfo::LocalVariable>& local_variable_table);
    void DumpAnnotationData(std::ostream &os, const pandasm::AnnotationData &anno) const;
    void DumpArrayValue(std::ostream &os, const pandasm::ArrayValue &array) const;
    void DumpScalarValue(std::ostream &os, const pandasm::ScalarValue &scalar) const;
    void GetOriginalDumpIns(const pandasm::Function &function);
    pandasm::Ins DeepCopyIns(const pandasm::Ins &input) const;
    pandasm::Function::CatchBlock DeepCopyCatchBlock(const pandasm::Function::CatchBlock &catch_block) const;
    void GetFinalDumpIns();
    void GetInvalidOpLabelMap();
    void HandleInvalidopInsLabel(size_t invalid_op_idx, pandasm::Ins &invalid_op_ins);
    pandasm::Ins *GetNearestValidopIns4InvalidopIns(size_t invalid_op_ins_idx);
    void GetFinallyLabelMap();
    void UpdateLabels4DumpIns(std::vector<pandasm::Ins*> &dump_ins, const LabelMap &label_map) const;
    void UpdateLabels4DumpInsAtIndex(size_t idx, std::vector<pandasm::Ins*> &dump_ins,
                                     const LabelMap &label_map) const;
    std::string GetMappedLabel(const std::string &label, const LabelMap &label_map) const;
    void SetFinallyLabelAtIndex(size_t idx);
    void DumpFinallyIns(std::ostream &os);
    void DumpFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionCatchBlocks4PandaAssembly(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionCatchBlocks4EcmaScript(std::ostream &os, const pandasm::Function &function) const;
    void DumpCatchBlock(std::ostream &os, const pandasm::Function::CatchBlock &catch_block) const;
    void UpdateCatchBlock(pandasm::Function::CatchBlock &catch_block) const;
    std::string GetUpdatedCatchBlockLabel(const std::string &orignal_label) const;
    bool IsMatchLiteralId(const pandasm::Ins &pa_ins) const;
    size_t GetLiteralIdIndexInIns(const pandasm::Ins &pa_ins) const;
    void ReplaceLiteralId4Ins(pandasm::Ins &pa_ins) const;
    void DumpStrings(std::ostream &os) const;
    std::string SerializeLiteralArray(const pandasm::LiteralArray &lit_array) const;
    std::string LiteralTagToString(const panda_file::LiteralTag &tag) const;
    template <typename T>
    void SerializeValues(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayU1(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayU8(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayI8(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayU16(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayI16(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayU32(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayI32(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayU64(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayI64(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayF32(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayF64(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeValues4ArrayString(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeLiterals(const pandasm::LiteralArray &lit_array, T &os) const;
    template <typename T>
    void SerializeLiteralsAtIndex(const pandasm::LiteralArray &lit_array, T &os, size_t i) const;
    static LiteralTagToStringMap literal_tag_to_string_map_;
    static std::unordered_map<pandasm::Opcode, size_t> opcode_literal_id_index_map_;
    ProgramDumperSource dumper_source_ = ProgramDumperSource::ECMASCRIPT;
    std::string abc_file_path_;
    std::vector<pandasm::Ins> original_dump_ins_;
    std::vector<pandasm::Ins*> original_dump_ins_ptrs_;
    std::vector<pandasm::Ins*> final_dump_ins_ptrs_;
    LabelMap invalid_op_label_to_nearest_valid_op_label_map_;
    LabelMap finally_label_map_;
    const pandasm::Program *program_ = nullptr;
    size_t regs_num_ = 0;
    std::unordered_map<pandasm::Ins*, uint32_t> original_ins_index_map_;
    std::unordered_map<pandasm::Ins*, uint32_t> final_ins_index_map_;
};

} // namespace panda::abc2program

#endif // ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H
