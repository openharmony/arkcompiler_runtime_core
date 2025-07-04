/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "abc_literal_array_processor.h"
#include "file-inl.h"

namespace ark::abc2program {

AbcAnnotationProcessor::AbcAnnotationProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData,
                                               pandasm::Record &record)
    : AbcFileEntityProcessor(entityId, keyData), record_(record)
{
    annotationDataAccessor_ = std::make_unique<panda_file::AnnotationDataAccessor>(*file_, entityId);
    std::string stringId = keyData.GetAbcStringTable().GetStringById(annotationDataAccessor_->GetClassId());
    annotationName_ = pandasm::Type::FromDescriptor(stringId).GetName();
    std::replace(annotationName_.begin(), annotationName_.end(), '/', '.');
}

void AbcAnnotationProcessor::FillProgramData()
{
    if (annotationName_.empty()) {
        return;
    }
    FillAnnotation();
}

void AbcAnnotationProcessor::FillAnnotation()
{
    std::vector<pandasm::AnnotationElement> elements;
    FillAnnotationElements(elements);
    pandasm::AnnotationData annotationData(annotationName_, elements);
    std::vector<pandasm::AnnotationData> annotations;
    annotations.emplace_back(std::move(annotationData));
    record_.metadata->AddAnnotations(annotations);
}

void AbcAnnotationProcessor::FillLiteralArrayAnnotation(std::vector<pandasm::AnnotationElement> &elements,
                                                        const std::string &annotationElemName, uint32_t value)
{
    AbcLiteralArrayProcessor litArrayProc(panda_file::File::EntityId {value}, keyData_);
    litArrayProc.FillProgramData();
    std::string name = keyData_.GetLiteralArrayIdName(value);
    pandasm::AnnotationElement annotationElement(
        annotationElemName,
        std::make_unique<pandasm::ScalarValue>(pandasm::ScalarValue::Create<pandasm::Value::Type::LITERALARRAY>(name)));
    elements.emplace_back(annotationElement);
}

template <pandasm::Value::Type P_TYPE, typename VType>
static pandasm::AnnotationElement CreateAnnoElem(std::string &annotationElemName, VType value)
{
    return pandasm::AnnotationElement(
        annotationElemName, std::make_unique<pandasm::ScalarValue>(pandasm::ScalarValue::Create<P_TYPE>(value)));
}

std::optional<pandasm::AnnotationElement> AbcAnnotationProcessor::CreateAnnotationElement(
    const panda_file::AnnotationDataAccessor::Elem &annotationDataAccessorElem, std::string &annotationElemName,
    pandasm::Value::Type valueType)
{
    switch (valueType) {
        case pandasm::Value::Type::U1: {
            auto value = static_cast<uint8_t>(annotationDataAccessorElem.GetScalarValue().Get<bool>());
            return CreateAnnoElem<pandasm::Value::Type::U1, uint8_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::U32: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint32_t>();
            return CreateAnnoElem<pandasm::Value::Type::U32, uint32_t>(annotationElemName, value);
        }
        case pandasm::Value::Type::F64: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<double>();
            return CreateAnnoElem<pandasm::Value::Type::F64, double>(annotationElemName, value);
        }
        case pandasm::Value::Type::STRING: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint32_t>();
            std::string_view stringValue {
                reinterpret_cast<const char *>(file_->GetStringData(panda_file::File::EntityId(value)).data)};
            return CreateAnnoElem<pandasm::Value::Type::STRING, std::string_view>(annotationElemName, stringValue);
        }
        case pandasm::Value::Type::LITERALARRAY: {
            auto value = annotationDataAccessorElem.GetScalarValue().Get<uint32_t>();
            AbcLiteralArrayProcessor litArrayProc(panda_file::File::EntityId {value}, keyData_);
            litArrayProc.FillProgramData();
            std::string name = keyData_.GetLiteralArrayIdName(value);
            return CreateAnnoElem<pandasm::Value::Type::LITERALARRAY, std::string>(annotationElemName, name);
        }
        case pandasm::Value::Type::RECORD: {
            // NOTE: Annotations are not fully supported #25313
            return std::nullopt;
        }
        case pandasm::Value::Type::ARRAY: {
            // NOTE: Annotations are not fully supported #25313
            return std::nullopt;
        }
        default:
            UNREACHABLE();
    }
    return std::nullopt;
}

void AbcAnnotationProcessor::FillAnnotationElements(std::vector<pandasm::AnnotationElement> &elements)
{
    for (uint32_t i = 0; i < annotationDataAccessor_->GetCount(); ++i) {
        auto annotationDataAccessorElem = annotationDataAccessor_->GetElement(i);
        auto annotationElemName = keyData_.GetAbcStringTable().GetStringById(annotationDataAccessorElem.GetNameId());
        auto valueType = pandasm::Value::GetCharAsType(annotationDataAccessor_->GetTag(i).GetItem());
        // NOTE: Module annotation, which contains arrays of records
        if (valueType == pandasm::Value::Type::UNKNOWN) {
            valueType = pandasm::Value::GetCharAsArrayType(annotationDataAccessor_->GetTag(i).GetItem());
        }
        auto annoElem = CreateAnnotationElement(annotationDataAccessorElem, annotationElemName, valueType);
        if (annoElem) {
            elements.emplace_back(annoElem.value());
        }
    }
}

}  // namespace ark::abc2program
