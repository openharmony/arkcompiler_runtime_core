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

#include "abc_field_processor.h"
#include "abc2program_log.h"

namespace panda::abc2program {

AbcFieldProcessor::AbcFieldProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container,
                                     pandasm::Record &record)
    : AbcFileEntityProcessor(entity_id, entity_container), record_(record),
      type_converter_(AbcTypeConverter(*string_table_)),
      field_(pandasm::Field(LANG_ECMA))
{
    field_data_accessor_ = std::make_unique<panda_file::FieldDataAccessor>(*file_, entity_id_);
}

void AbcFieldProcessor::FillProgramData()
{
    FillFieldData();
    record_.field_list.emplace_back(std::move(field_));
}

void AbcFieldProcessor::FillFieldData()
{
    FillFieldName();
    FillFieldType();
    FillFieldMetaData();
}

void AbcFieldProcessor::FillFieldName()
{
    panda_file::File::EntityId field_name_id = field_data_accessor_->GetNameId();
    field_.name = string_table_->GetStringById(field_name_id);
}

void AbcFieldProcessor::FillFieldType()
{
    uint32_t field_type = field_data_accessor_->GetType();
    field_.type = type_converter_.FieldTypeToPandasmType(field_type);
}

void AbcFieldProcessor::FillFieldMetaData()
{
    field_.metadata->SetFieldType(field_.type);
    FillFieldAttributes();
    FillMetaDataValue();
}

void AbcFieldProcessor::FillFieldAttributes()
{
    if (field_data_accessor_->IsExternal()) {
        field_.metadata->SetAttribute(ABC_ATTR_EXTERNAL);
    }
    if (field_data_accessor_->IsStatic()) {
        field_.metadata->SetAttribute(ABC_ATTR_STATIC);
    }
}

void AbcFieldProcessor::FillMetaDataValue()
{
    switch (field_.type.GetId()) {
        case panda_file::Type::TypeId::U32: {
            uint32_t module_literal_array_id = field_data_accessor_->GetValue<uint32_t>().value();
            if (record_.name == ES_MODULE_RECORD || field_.name == MODULE_RECORD_IDX) {
                entity_container_.AddModuleLiteralArrayId(module_literal_array_id);
                auto module_literal_array_id_name = entity_container_.GetLiteralArrayIdName(module_literal_array_id);
                field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(
                    module_literal_array_id_name));
            } else {
                const uint32_t val = field_data_accessor_->GetValue<uint32_t>().value();
                field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U32>(val));
            }
            break;
        }
        case panda_file::Type::TypeId::U8: {
            const uint8_t val = field_data_accessor_->GetValue<uint8_t>().value();
            field_.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U8>(val));
            break;
        }
        default:
            UNREACHABLE();
    }
}

void AbcFieldProcessor::FillFieldAnnotations()
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

}  // namespace panda::abc2program
