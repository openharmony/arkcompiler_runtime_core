/**
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

#include "abc_field_processor.h"
#include "abc2program_log.h"

namespace ark::abc2program {

AbcFieldProcessor::AbcFieldProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                                     pandasm::Record &record, bool isExternal)
    : AbcFileEntityProcessor(entityId, keyData), record_(record), isExternal_(isExternal)
{
    fieldDataAccessor_ = std::make_unique<panda_file::FieldDataAccessor>(*file_, entityId_);
    typeConverter_ = std::make_unique<AbcTypeConverter>(*stringTable_);
    FillProgramData();
}

void AbcFieldProcessor::FillProgramData()
{
    pandasm::Field field(keyData_.GetFileLanguage());
    FillFieldData(field);
    if (isExternal_) {
        auto &fieldList =
            keyData_.GetExternalFieldTable()[keyData_.GetFullRecordNameById(fieldDataAccessor_->GetClassId())];
        auto retField = std::find_if(fieldList.begin(), fieldList.end(), [&field](pandasm::Field &fieldFromList) {
            return field.name == fieldFromList.name;
        });
        if (retField == fieldList.end()) {
            fieldList.push_back(std::move(field));
        }
        return;
    }
    record_.fieldList.emplace_back(std::move(field));
}

void AbcFieldProcessor::FillFieldData(pandasm::Field &field)
{
    FillFieldName(field);
    FillFieldType(field);
    FillFieldMetaData(field);
}

void AbcFieldProcessor::FillFieldName(pandasm::Field &field)
{
    panda_file::File::EntityId fieldNameId = fieldDataAccessor_->GetNameId();
    field.name = stringTable_->GetStringById(fieldNameId);
}

void AbcFieldProcessor::FillFieldType(pandasm::Field &field)
{
    uint32_t fieldType = fieldDataAccessor_->GetType();
    field.type = typeConverter_->FieldTypeToPandasmType(fieldType);
}

void AbcFieldProcessor::FillFieldMetaData(pandasm::Field &field)
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting metadata]\nfield id: " << entityId_ << " (0x" << std::hex << entityId_ << ")";

    panda_file::FieldDataAccessor fieldAccessor(*file_, entityId_);

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsExternal(); }, "external");

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsStatic(); }, "static");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsPublic(); }, "access.field", "public");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsProtected(); }, "access.field", "protected");

    SetEntityAttributeValue(
        field, [&fieldAccessor]() { return fieldAccessor.IsPrivate(); }, "access.field", "private");

    SetEntityAttribute(
        field, [&fieldAccessor]() { return fieldAccessor.IsFinal(); }, "final");
}

}  // namespace ark::abc2program
