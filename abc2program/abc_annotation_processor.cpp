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

#include "abc_annotation_processor.h"
#include "abc2program_log.h"

namespace panda::abc2program {

AbcAnnotationProcessor::AbcAnnotationProcessor(panda_file::File::EntityId entity_id,
                                               Abc2ProgramEntityContainer &entity_container,
                                               pandasm::Function &function)
    : AbcFileEntityProcessor(entity_id, entity_container), function_(function)
{
    annotation_data_accessor_ = std::make_unique<panda_file::AnnotationDataAccessor>(*file_, entity_id_);
    auto typeDescriptorName = string_table_->GetStringById(annotation_data_accessor_->GetClassId());
    annotation_name_ = pandasm::Type::FromDescriptor(typeDescriptorName).GetName();
}

void AbcAnnotationProcessor::FillProgramData()
{
    FillAnnotation();
}

void AbcAnnotationProcessor::FillAnnotation()
{
    std::vector<pandasm::AnnotationElement> elements;
    FillAnnotationElements(elements);
    pandasm::AnnotationData annotation_data(annotation_name_, elements);
    std::vector<pandasm::AnnotationData> annotations;
    annotations.emplace_back(std::move(annotation_data));
    function_.metadata->AddAnnotations(annotations);
}

void AbcAnnotationProcessor::FillAnnotationElements(std::vector<pandasm::AnnotationElement> &elements)
{
    for (uint32_t i = 0; i < annotation_data_accessor_->GetCount(); ++i) {
        auto annotation_data_accessor_elem = annotation_data_accessor_->GetElement(i);
        auto annotation_elem_name = string_table_->GetStringById(annotation_data_accessor_elem.GetNameId());
        auto value = annotation_data_accessor_elem.GetScalarValue().GetValue();
        auto value_type = pandasm::Value::GetCharAsType(annotation_data_accessor_->GetTag(i).GetItem());
        switch (value_type) {
            case pandasm::Value::Type::U32: {
                pandasm::AnnotationElement annotation_element(
                    annotation_elem_name, std::make_unique<pandasm::ScalarValue>(
                    pandasm::ScalarValue::Create<pandasm::Value::Type::U32>(value)));
                elements.emplace_back(annotation_element);
                break;
            }
            default:
                UNREACHABLE();
        }
    }
}

} // namespace panda::abc2program
