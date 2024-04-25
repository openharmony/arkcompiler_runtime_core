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

#include "abc_literal_array_processor.h"
#include "abc2program_log.h"
#include "method_data_accessor-inl.h"

namespace panda::abc2program {

AbcLiteralArrayProcessor::AbcLiteralArrayProcessor(panda_file::File::EntityId entity_id,
                                                   Abc2ProgramEntityContainer &entity_container,
                                                   panda_file::LiteralDataAccessor &literal_data_accessor)
    : AbcFileEntityProcessor(entity_id, entity_container), literal_data_accessor_(literal_data_accessor) {}

void AbcLiteralArrayProcessor::FillProgramData()
{
    GetLiteralArrayById(&literal_array_, entity_id_);
    program_->literalarray_table.emplace(entity_container_.GetLiteralArrayIdName(entity_id_),
                                         std::move(literal_array_));
}

void AbcLiteralArrayProcessor::GetLiteralArrayById(pandasm::LiteralArray *lit_array,
                                                   panda_file::File::EntityId lit_array_id) const
{
    literal_data_accessor_.EnumerateLiteralVals(
        lit_array_id, [this, lit_array](
                    const panda_file::LiteralDataAccessor::LiteralValue &value,
                    const panda_file::LiteralTag &tag) {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U1:
                    FillLiteralArrayData<bool>(lit_array, tag, value);
                    break;
                case panda_file::LiteralTag::ARRAY_I8:
                case panda_file::LiteralTag::ARRAY_U8:
                    FillLiteralArrayData<uint8_t>(lit_array, tag, value);
                    break;
                case panda_file::LiteralTag::ARRAY_I16:
                case panda_file::LiteralTag::ARRAY_U16:
                    FillLiteralArrayData<uint16_t>(lit_array, tag, value);
                    break;
                case panda_file::LiteralTag::ARRAY_I32:
                case panda_file::LiteralTag::ARRAY_U32:
                    FillLiteralArrayData<uint32_t>(lit_array, tag, value);
                    break;
                case panda_file::LiteralTag::ARRAY_I64:
                case panda_file::LiteralTag::ARRAY_U64:
                    FillLiteralArrayData<uint64_t>(lit_array, tag, value);
                    break;
                case panda_file::LiteralTag::ARRAY_F32:
                    FillLiteralArrayData<float>(lit_array, tag, value);
                    break;
                case panda_file::LiteralTag::ARRAY_F64:
                    FillLiteralArrayData<double>(lit_array, tag, value);
                    break;
                case panda_file::LiteralTag::ARRAY_STRING:
                    FillLiteralArrayData<uint32_t>(lit_array, tag, value);
                    break;
                default:
                    FillLiteralData(lit_array, value, tag);
                    break;
            }
        });
}

template <typename T>
void AbcLiteralArrayProcessor::FillLiteralArrayData(pandasm::LiteralArray *lit_array, const panda_file::LiteralTag &tag,
                                                    const panda_file::LiteralDataAccessor::LiteralValue &value) const
{
    panda_file::File::EntityId id(std::get<uint32_t>(value));
    auto sp = file_->GetSpanFromId(id);
    auto len = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        for (size_t i = 0; i < len; i++) {
            pandasm::LiteralArray::Literal lit;
            lit.tag_ = tag;
            lit.value_ = bit_cast<T>(panda_file::helpers::Read<sizeof(T)>(&sp));
            lit_array->literals_.emplace_back(lit);
        }
        return;
    }
    for (size_t i = 0; i < len; i++) {
        auto str_id = panda_file::helpers::Read<sizeof(T)>(&sp);
        pandasm::LiteralArray::Literal lit;
        lit.tag_ = tag;
        lit.value_ = string_table_->GetStringById(str_id);
        lit_array->literals_.emplace_back(lit);
    }
}

void AbcLiteralArrayProcessor::FillLiteralData(pandasm::LiteralArray *lit_array,
                                               const panda_file::LiteralDataAccessor::LiteralValue &value,
                                               const panda_file::LiteralTag &tag) const
{
    pandasm::LiteralArray::Literal value_lit;
    pandasm::LiteralArray::Literal tag_lit;
    value_lit.tag_ = tag;
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
            value_lit.value_ = std::get<bool>(value);
            break;
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE:
        case panda_file::LiteralTag::BUILTINTYPEINDEX:
            value_lit.value_ = std::get<uint8_t>(value);
            break;
        case panda_file::LiteralTag::METHODAFFILIATE:
            value_lit.value_ = std::get<uint16_t>(value);
            break;
        case panda_file::LiteralTag::LITERALBUFFERINDEX:
        case panda_file::LiteralTag::INTEGER:
            value_lit.value_ = std::get<uint32_t>(value);
            break;
        case panda_file::LiteralTag::DOUBLE:
            value_lit.value_ = std::get<double>(value);
            break;
        case panda_file::LiteralTag::STRING:
            value_lit.value_ = string_table_->GetStringById(std::get<uint32_t>(value));
            break;
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GETTER:
        case panda_file::LiteralTag::SETTER:
        case panda_file::LiteralTag::GENERATORMETHOD:
            value_lit.value_ = entity_container_.GetFullMethodNameById(std::get<uint32_t>(value));
            break;
        case panda_file::LiteralTag::LITERALARRAY:
            value_lit.value_ = entity_container_.GetLiteralArrayIdName(std::get<uint32_t>(value));
            entity_container_.TryAddUnprocessedNestedLiteralArrayId(std::get<uint32_t>(value));
            break;
        case panda_file::LiteralTag::TAGVALUE:
            value_lit.value_ = std::get<uint8_t>(value);
            break;
        default:
            UNREACHABLE();
    }

    tag_lit.tag_ = panda_file::LiteralTag::TAGVALUE;
    tag_lit.value_ = static_cast<uint8_t>(value_lit.tag_);

    lit_array->literals_.emplace_back(tag_lit);
    lit_array->literals_.emplace_back(value_lit);
}

} // namespace panda::abc2program
