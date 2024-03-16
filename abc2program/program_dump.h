/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "abc_string_table.h"

namespace panda::abc2program {

using LiteralTagToStringMap = std::unordered_map<panda_file::LiteralTag, std::string>;

class PandasmProgramDumper {
public:
    PandasmProgramDumper(const panda_file::File &file, const AbcStringTable &string_table)
        : file_(&file), string_table_(&string_table) {}
    PandasmProgramDumper() {}
    void Dump(std::ostream &os, const pandasm::Program &program) const;

private:
    bool HasNoAbcInput() const;
    void DumpAbcFilePath(std::ostream &os) const;
    void DumpProgramLanguage(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArrayTable(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArrayTableWithKey(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArrayTableWithoutKey(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArrayWithKey(std::ostream &os, const std::string &key,
                                 const pandasm::LiteralArray &lit_array) const;
    void DumpLiteralArrayContents(std::ostream &os) const;
    void DumpRecordTable(std::ostream &os, const pandasm::Program &program) const;
    void DumpRecord(std::ostream &os, const pandasm::Record &record) const;
    bool DumpRecordMetaData(std::ostream &os, const pandasm::Record &record) const;
    void DumpFieldList(std::ostream &os, const pandasm::Record &record) const;
    void DumpField(std::ostream &os, const pandasm::Field &field) const;
    void DumpFieldMetaData(std::ostream &os, const pandasm::Field &field) const;
    void DumpRecordSourceFile(std::ostream &os, const pandasm::Record &record) const;
    void DumpFunctionTable(std::ostream &os, const pandasm::Program &program) const;
    void DumpFunction(std::ostream &os, const pandasm::Function &function) const;
    void DumpStrings(std::ostream &os, const pandasm::Program &program) const;
    void DumpStringsByStringTable(std::ostream &os, const AbcStringTable &string_table) const;
    void DumpStringsByProgram(std::ostream &os, const pandasm::Program &program) const;
    std::string LiteralTagToString(const panda_file::LiteralTag &tag) const;
    std::string SerializeLiteralArray(const pandasm::LiteralArray &lit_array) const;
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
    void GetLiteralArrayByOffset(pandasm::LiteralArray *lit_array, panda_file::File::EntityId offset) const;
    template <typename T>
    void FillLiteralArrayData(pandasm::LiteralArray *lit_array, const panda_file::LiteralTag &tag,
                              const panda_file::LiteralDataAccessor::LiteralValue &value) const;
    void FillLiteralData(pandasm::LiteralArray *lit_array, const panda_file::LiteralDataAccessor::LiteralValue &value,
                         const panda_file::LiteralTag &tag) const;
    void FillLiteralData4Method(const panda_file::LiteralDataAccessor::LiteralValue &value,
                                pandasm::LiteralArray::Literal &lit) const;
    void FillLiteralData4LiteralArray(const panda_file::LiteralDataAccessor::LiteralValue &value,
                                      pandasm::LiteralArray::Literal &lit) const;
    static bool IsArray(const panda_file::LiteralTag &tag);
    static LiteralTagToStringMap literal_tag_to_string_map_;
    const panda_file::File *file_ = nullptr;
    const AbcStringTable *string_table_ = nullptr;
};

} // namespace panda::abc2program

#endif // ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H
