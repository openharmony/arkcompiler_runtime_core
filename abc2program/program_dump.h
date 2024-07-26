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
#include "dump_utils.h"

namespace panda::abc2program {

class PandasmProgramDumper {
public:
    PandasmProgramDumper() {}
    PandasmProgramDumper(bool is_normalized, bool is_debug) : is_normalized_(is_normalized), is_debug_(is_debug) {}
    void Dump(std::ostream &os, const pandasm::Program &program);
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
    void DumpFunctionKind(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionAnnotations(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionHead(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionReturnType(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionName(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionParams(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionParamAtIndex(std::ostream &os, const pandasm::Function::Parameter &param, size_t idx) const;
    void DumpFunctionAttributes(std::ostream &os, const pandasm::Function &function) const;
    void DumpFunctionBody(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionIns(std::ostream &os, const pandasm::Function &function);
    void DumpOriginalFunctionIns(std::ostream &os, const pandasm::Function &function);
    void DumpNormalizedFunctionIns(std::ostream &os, const pandasm::Function &function);
    void DumpFunctionDebugInfo(std::ostream &os, const pandasm::Function &function);
    void UpdateLocalVarMap(const pandasm::Function &function,
        std::map<int32_t, panda::pandasm::debuginfo::LocalVariable>& local_variable_table);
    void DumpAnnotationData(std::ostream &os, const pandasm::AnnotationData &anno) const;
    void DumpArrayValue(std::ostream &os, const pandasm::ArrayValue &array) const;
    void DumpScalarValue(std::ostream &os, const pandasm::ScalarValue &scalar) const;
    void GetOriginalDumpIns(const pandasm::Function &function);
    void GetFinalDumpIns();
    void GetInvalidOpLabelMap();
    void HandleInvalidopInsLabel(size_t invalid_op_idx, pandasm::Ins &invalid_op_ins);
    pandasm::Ins *GetNearestValidopIns4InvalidopIns(size_t invalid_op_ins_idx);
    void GetFinalLabelMap();
    void UpdateLabels4DumpIns(std::vector<pandasm::Ins*> &dump_ins, const LabelMap &label_map) const;
    void UpdateLabels4DumpInsAtIndex(size_t idx, std::vector<pandasm::Ins*> &dump_ins,
                                     const LabelMap &label_map) const;
    std::string GetMappedLabel(const std::string &label, const LabelMap &label_map) const;
    void HandleFinalLabelAtIndex(size_t idx);
    void DumpFinalIns(std::ostream &os);
    void DumpFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const;
    void DumpOriginalFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const;
    void DumpNormalizedFunctionCatchBlocks(std::ostream &os, const pandasm::Function &function) const;
    void DumpCatchBlock(std::ostream &os, const pandasm::Function::CatchBlock &catch_block) const;
    void UpdateCatchBlock(pandasm::Function::CatchBlock &catch_block) const;
    std::string GetUpdatedCatchBlockLabel(const std::string &orignal_label) const;
    void ReplaceLiteralId4Ins(pandasm::Ins &pa_ins) const;
    void DumpStrings(std::ostream &os) const;
    std::string SerializeLiteralArray(const pandasm::LiteralArray &lit_array, uint32_t id) const;
    void SerializeLiterals(const pandasm::LiteralArray &lit_array, std::stringstream &os) const;
    void SerializeLiteralsAtIndex(const pandasm::LiteralArray &lit_array, std::stringstream &os, size_t i) const;
    void SerializeNestedLiteralArrayById(std::stringstream &os, const std::string &literal_array_id_name) const;
    // True when the option 'dump-normalized-asm-program' is enabled. See option description for details
    bool is_normalized_ = false;
    // True when the option 'debug-info' is enabled. When both it and is_normalized_ are true, skip dump
    // of function annotation and the record with name '_ESSlotNumberAnnotation'
    bool is_debug_ = false;
    std::string abc_file_path_;
    std::vector<pandasm::Ins> original_dump_ins_;
    std::vector<pandasm::Ins*> original_dump_ins_ptrs_;
    std::vector<pandasm::Ins*> final_dump_ins_ptrs_;
    LabelMap invalid_op_label_map_;
    LabelMap final_label_map_;
    const pandasm::Program *program_ = nullptr;
    size_t regs_num_ = 0;
    std::unordered_map<pandasm::Ins*, uint32_t> original_ins_index_map_;
    std::unordered_map<pandasm::Ins*, uint32_t> final_ins_index_map_;
    mutable std::unordered_set<uint32_t> processing_literal_array_id_set_;
};

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H
