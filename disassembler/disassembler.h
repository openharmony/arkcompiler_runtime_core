/*
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

#ifndef DISASSEMBLER_DISASSEMBLER_H
#define DISASSEMBLER_DISASSEMBLER_H

#include "macros.h"
#include "utils/type_helpers.h"
#include "utils/span.h"

#include "class_data_accessor-inl.h"
#include "code_data_accessor-inl.h"
#include "debug_data_accessor-inl.h"
#include "debug_info_extractor.h"
#include "field_data_accessor-inl.h"
#include "method_data_accessor-inl.h"
#include "literal_data_accessor-inl.h"
#include "param_annotations_data_accessor.h"
#include "annotation_data_accessor.h"
#include "proto_data_accessor-inl.h"
#include "file-inl.h"
#include "file.h"
#include "os/file.h"

#include "assembly-program.h"
#include "assembly-ins.h"

#include "bytecode_instruction-inl.h"
#include "libpandabase/macros.h"

#include <map>
#include <memory>
#include <string>

#include "accumulators.h"

namespace panda::disasm {
class Disassembler {
public:
    NO_COPY_SEMANTIC(Disassembler);
    DEFAULT_MOVE_SEMANTIC(Disassembler);

    Disassembler() = default;
    ~Disassembler() = default;

    void Disassemble(const std::string &filename_in, const bool quiet = false, const bool skip_strings = false);
    void CollectInfo();
    void Serialize(std::ostream &os, bool add_separators = false, bool print_information = false) const;

    static inline bool IsPandasmFriendly(const char c);
    static inline bool IsSystemType(const std::string &type_name);

    void GetRecord(pandasm::Record *record, const panda_file::File::EntityId &record_id);
    void AddMethodToTables(const panda_file::File::EntityId &method_id);
    void GetMethod(pandasm::Function *method, const panda_file::File::EntityId &method_id);
    void GetLiteralArray(pandasm::LiteralArray *lit_array, size_t index) const;
    template <typename T>
    void FillLiteralArrayData(pandasm::LiteralArray *lit_array, const panda_file::LiteralTag &tag,
                              const panda_file::LiteralDataAccessor::LiteralValue &value) const;

    const ProgInfo &GetProgInfo() const
    {
        return prog_info_;
    }

private:
    void GetLiteralArrays();
    void FillLiteralData(pandasm::LiteralArray *lit_array, const panda_file::LiteralDataAccessor::LiteralValue &value,
                         const panda_file::LiteralTag &tag) const;
    void GetRecords();
    void GetFields(pandasm::Record *record, const panda_file::File::EntityId &record_id);

    void GetMethods(const panda_file::File::EntityId &record_id);
    void GetParams(pandasm::Function *method, const panda_file::File::EntityId &proto_id) const;
    IdList GetInstructions(pandasm::Function *method, panda_file::File::EntityId method_id,
                           panda_file::File::EntityId code_id) const;
    LabelTable GetExceptions(pandasm::Function *method, panda_file::File::EntityId method_id,
                             panda_file::File::EntityId code_id) const;
    bool LocateTryBlock(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                        const panda_file::CodeDataAccessor::TryBlock &try_block,
                        pandasm::Function::CatchBlock *catch_block_pa, LabelTable *label_table, size_t try_idx) const;
    bool LocateCatchBlock(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                          const panda_file::CodeDataAccessor::CatchBlock &catch_block,
                          pandasm::Function::CatchBlock *catch_block_pa, LabelTable *label_table, size_t try_idx,
                          size_t catch_idx) const;

    void GetMetaData(pandasm::Record *record, const panda_file::File::EntityId &record_id) const;
    void GetMetaData(pandasm::Function *method, const panda_file::File::EntityId &method_id) const;
    void GetMetaData(pandasm::Field *field, const panda_file::File::EntityId &field_id);

    void GetLanguageSpecificMetadata();

    std::string AnnotationTagToString(const char tag) const;

    std::string ScalarValueToString(const panda_file::ScalarValue &value, const std::string &type);
    std::string ArrayValueToString(const panda_file::ArrayValue &value, const std::string &type, const size_t idx);

    std::string GetFullMethodName(const panda_file::File::EntityId &method_id) const;
    std::string GetMethodSignature(const panda_file::File::EntityId &method_id) const;
    std::string GetFullRecordName(const panda_file::File::EntityId &class_id) const;

    void GetRecordInfo(const panda_file::File::EntityId &record_id, RecordInfo *record_info) const;
    void GetMethodInfo(const panda_file::File::EntityId &method_id, MethodInfo *method_info) const;
    void GetInsInfo(const panda_file::File::EntityId &code_id, MethodInfo *method_info) const;

    template <typename T>
    void SerializeValues(const pandasm::LiteralArray &lit_array, T &os) const;
    std::string SerializeLiteralArray(const pandasm::LiteralArray &lit_array) const;
    void Serialize(const std::string &key, const pandasm::LiteralArray &lit_array, std::ostream &os) const;
    template <typename T>
    void SerializeLiterals(const pandasm::LiteralArray &lit_array, T &os) const;
    std::string LiteralTagToString(const panda_file::LiteralTag &tag) const;
    void Serialize(const pandasm::Record &record, std::ostream &os, bool print_information = false) const;
    void SerializeFields(const pandasm::Record &record, std::ostream &os, bool print_information) const;
    void Serialize(const pandasm::Function &method, std::ostream &os, bool print_information = false) const;
    void Serialize(const pandasm::Function::CatchBlock &catch_block, std::ostream &os) const;
    void Serialize(const pandasm::ItemMetadata &meta, const AnnotationList &ann_list, std::ostream &os) const;
    void SerializeLineNumberTable(const panda_file::LineNumberTable &line_number_table, std::ostream &os) const;
    void SerializeLocalVariableTable(const panda_file::LocalVariableTable &local_variable_table,
                                     const pandasm::Function &method, std::ostream &os) const;
    bool IsModuleLiteralOffset(const panda_file::File::EntityId &id) const;
    inline void SerializeLanguage(std::ostream &os) const
    {
        os << ".language " << panda::panda_file::LanguageToString(file_language_) << "\n\n";
    }

    pandasm::Type PFTypeToPandasmType(const panda_file::Type &type, panda_file::ProtoDataAccessor &pda,
                                      size_t &ref_idx) const;

    pandasm::Type FieldTypeToPandasmType(const uint32_t &type) const;

    static inline std::string StringDataToString(panda_file::File::StringData sd)
    {
        return std::string(utf::Mutf8AsCString(sd.data));
    }

    pandasm::Opcode BytecodeOpcodeToPandasmOpcode(BytecodeInstruction::Opcode o) const;
    pandasm::Opcode BytecodeOpcodeToPandasmOpcode(uint8_t o) const;

    pandasm::Ins BytecodeInstructionToPandasmInstruction(BytecodeInstruction bc_ins,
                                                         panda_file::File::EntityId method_id) const;

    std::string IDToString(BytecodeInstruction bc_ins, panda_file::File::EntityId method_id, size_t idx) const;

    panda::panda_file::SourceLang GetRecordLanguage(panda_file::File::EntityId class_id) const;

    void GetLiteralArrayByOffset(pandasm::LiteralArray *lit_array, panda_file::File::EntityId offset) const;

    std::unique_ptr<const panda_file::File> file_;
    pandasm::Program prog_;

    panda::panda_file::SourceLang file_language_ = panda::panda_file::SourceLang::PANDA_ASSEMBLY;

    std::map<std::string, panda_file::File::EntityId> record_name_to_id_;
    std::map<std::string, panda_file::File::EntityId> method_name_to_id_;

    ProgAnnotations prog_ann_;

    ProgInfo prog_info_;

    std::unique_ptr<panda_file::DebugInfoExtractor> debug_info_extractor_;

    bool quiet_;
    bool skip_strings_;
    std::unordered_set<uint32_t> module_literals_;
#include "disasm_plugins.inc"
};
}  // namespace panda::disasm

#endif  // DISASSEMBLER_DISASSEMBLER_H
