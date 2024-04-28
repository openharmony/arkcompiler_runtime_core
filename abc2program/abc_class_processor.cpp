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

#include "abc_class_processor.h"
#include <iostream>
#include "abc_method_processor.h"
#include "abc_field_processor.h"
#include "mangling.h"

namespace panda::abc2program {

AbcClassProcessor::AbcClassProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container)
    : AbcFileEntityProcessor(entity_id, entity_container)
{
    class_data_accessor_ = std::make_unique<panda_file::ClassDataAccessor>(*file_, entity_id_);
}

void AbcClassProcessor::FillProgramData()
{
    LOG(DEBUG, ABC2PROGRAM) << __PRETTY_FUNCTION__ << " started...";
    FillRecord();
    FillFunctions();
    LOG(DEBUG, ABC2PROGRAM) << __PRETTY_FUNCTION__ << " ended...";
}

void AbcClassProcessor::FillRecord()
{
    const pandasm::Record *record_ptr = GetRecordById(entity_id_);
    if (record_ptr != nullptr) {
        return;
    }
    pandasm::Record record("", panda_file::SourceLang::ECMASCRIPT);
    record.name = entity_container_.GetFullRecordNameById(entity_id_);
    if (record.name == SLOT_NUMBER_RECORD_NAME || record.name == CONCURRENT_MODULE_REQUEST_RECORD_NAME) {
        record.metadata->SetAccessFlags(class_data_accessor_->GetAccessFlags());
    }
    ASSERT(program_->record_table.find(record.name) == program_->record_table.end());
    FillRecordData(record);
    program_->record_table.emplace(record.name, std::move(record));
    const auto it = program_->record_table.find(record.name);
    const pandasm::Record &recordRef = it->second;
    AddRecord(entity_id_, recordRef);
}

void AbcClassProcessor::FillRecordData(pandasm::Record &record)
{
    FillRecordMetaData(record);
    FillFields(record);
    FillRecordSourceFile(record);
}

void AbcClassProcessor::FillRecordMetaData(pandasm::Record &record)
{
    FillRecordAttributes(record);
    FillRecordAnnotations(record);
}

void AbcClassProcessor::FillRecordAttributes(pandasm::Record &record)
{
    if (file_->IsExternal(entity_id_)) {
        record.metadata->SetAttribute(ABC_ATTR_EXTERNAL);
    }
}

void AbcClassProcessor::FillRecordAnnotations(pandasm::Record &record)
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

void AbcClassProcessor::FillRecordSourceFile(pandasm::Record &record)
{
    std::optional<panda_file::File::EntityId> source_file_id = class_data_accessor_->GetSourceFileId();
    if (source_file_id.has_value()) {
        record.source_file = string_table_->GetStringById(source_file_id.value());
    } else {
        record.source_file = "";
    }
}

void AbcClassProcessor::FillFields(pandasm::Record &record)
{
    if (file_->IsExternal(entity_id_)) {
        return;
    }
    class_data_accessor_->EnumerateFields([&](panda_file::FieldDataAccessor &fda) -> void {
        panda_file::File::EntityId field_id = fda.GetFieldId();
        AbcFieldProcessor field_processor(field_id, entity_container_, record);
        field_processor.FillProgramData();
    });
}

void AbcClassProcessor::FillFunctions()
{
    if (file_->IsExternal(entity_id_)) {
        return;
    }
    class_data_accessor_->EnumerateMethods([&](panda_file::MethodDataAccessor &mda) -> void {
        panda_file::File::EntityId method_id = mda.GetMethodId();
        AbcMethodProcessor method_processor(method_id, entity_container_);
        method_processor.FillProgramData();
    });
}

} // namespace panda::abc2program
