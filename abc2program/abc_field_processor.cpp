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

namespace panda::abc2program {

AbcFieldProcessor::AbcFieldProcessor(panda_file::File::EntityId entity_id, Abc2ProgramEntityContainer &entity_container,
                                     pandasm::Record &record)
    : AbcFileEntityProcessor(entity_id, entity_container), record_(record),
      type_converter_(AbcTypeConverter(*string_table_))
{
    field_data_accessor_ = std::make_unique<panda_file::FieldDataAccessor>(*file_, entity_id_);
}

void AbcFieldProcessor::FillProgramData()
{
    const pandasm::Field *field_ptr = GetFieldById(entity_id_);
    if (field_ptr != nullptr) {
        return;
    }
    pandasm::Field field(panda_file::SourceLang::ECMASCRIPT);
    FillFieldData(field);
    record_.field_list.emplace_back(std::move(field));
    const pandasm::Field &field_ref = record_.field_list[record_.field_list.size() - 1];
    AddField(entity_id_, field_ref);
}

void AbcFieldProcessor::FillFieldData(pandasm::Field &field)
{
    FillFieldName(field);
    FillFieldType(field);
    FillFieldMetaData(field);
}

void AbcFieldProcessor::FillFieldName(pandasm::Field &field)
{
    panda_file::File::EntityId field_name_id = field_data_accessor_->GetNameId();
    field.name = string_table_->GetStringById(field_name_id);
}

void AbcFieldProcessor::FillFieldType(pandasm::Field &field)
{
    uint32_t field_type = field_data_accessor_->GetType();
    field.type = type_converter_.FieldTypeToPandasmType(field_type);
}

void AbcFieldProcessor::FillFieldMetaData(pandasm::Field &field)
{
    field.metadata->SetFieldType(field.type);
    FillFieldAttributes(field);
    FillFieldAnnotations(field);
    FillMetaDataValue(field);
}

void AbcFieldProcessor::FillFieldAttributes(pandasm::Field &field)
{
    if (field_data_accessor_->IsExternal()) {
        field.metadata->SetAttribute(ABC_ATTR_EXTERNAL);
    }
    if (field_data_accessor_->IsStatic()) {
        field.metadata->SetAttribute(ABC_ATTR_STATIC);
    }
}

void AbcFieldProcessor::FillMetaDataValue(pandasm::Field &field)
{
    switch (field.type.GetId()) {
        case panda_file::Type::TypeId::U32: {
            uint32_t module_literal_array_id = field_data_accessor_->GetValue<uint32_t>().value();
            if (record_.name == ES_MODULE_RECORD || field.name == MODULE_RECORD_IDX) {
                entity_container_.AddModuleLiteralArrayId(module_literal_array_id);
                auto module_literal_array_id_name = entity_container_.GetLiteralArrayIdName(module_literal_array_id);
                field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(
                    module_literal_array_id_name));
            } else {
                const auto val = field_data_accessor_->GetValue<uint32_t>().value();
                field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U32>(val));
            }
            break;
        }
        case panda_file::Type::TypeId::U8: {
            const auto val = field_data_accessor_->GetValue<uint8_t>().value();
            field.metadata->SetValue(pandasm::ScalarValue::Create<pandasm::Value::Type::U8>(val));
            break;
        }
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::I8:
        case panda_file::Type::TypeId::I16:
        case panda_file::Type::TypeId::U16:
        case panda_file::Type::TypeId::I32:
        case panda_file::Type::TypeId::F32:
        case panda_file::Type::TypeId::F64:
        case panda_file::Type::TypeId::I64:
        case panda_file::Type::TypeId::U64:
        case panda_file::Type::TypeId::REFERENCE:
        case panda_file::Type::TypeId::TAGGED:
        case panda_file::Type::TypeId::VOID:
        case panda_file::Type::TypeId::INVALID: {
            UNREACHABLE();
        }
        default:
            UNREACHABLE();
    }
}

void AbcFieldProcessor::FillFieldAnnotations(pandasm::Field &field)
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

} // namespace panda::abc2program
