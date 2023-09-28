/**
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

#include "disassembler.h"

namespace panda::disasm {
void Disassembler::GeteTSMetadata()
{
    LOG(DEBUG, DISASSEMBLER) << "\n[getting ETS-specific metadata]\n";

    for (auto &pair : prog_.record_table) {
        if (pair.second.language == panda::panda_file::SourceLang::ETS) {
            const auto record_id = record_name_to_id_[pair.first];

            if (file_->IsExternal(record_id)) {
                continue;
            }

            GetETSMetadata(&pair.second, record_id);

            panda_file::ClassDataAccessor class_accessor(*file_, record_id);

            size_t field_idx = 0;
            class_accessor.EnumerateFields([&](panda_file::FieldDataAccessor &field_accessor) {
                GetETSMetadata(&pair.second.field_list[field_idx++], field_accessor.GetFieldId());
            });
        }
    }

    for (auto &pair : prog_.function_table) {
        if (pair.second.language == panda::panda_file::SourceLang::ETS) {
            const auto method_id = method_name_to_id_[pair.first];

            GetETSMetadata(&pair.second, method_id);
        }
    }
}

void Disassembler::GetETSMetadata(pandasm::Record *record, const panda_file::File::EntityId &record_id)
{
    LOG(DEBUG, DISASSEMBLER) << "[getting ETS metadata]\nrecord id: " << record_id;
    if (record == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved!";
        return;
    }

    panda_file::ClassDataAccessor class_accessor(*file_, record_id);
    const auto record_name = GetFullRecordName(class_accessor.GetClassId());
    SetETSAttributes(record, record_id);
    AnnotationList ann_list {};
    LOG(DEBUG, DISASSEMBLER) << "[getting ets annotations]\nrecord id: " << record_id;

    class_accessor.EnumerateAnnotations([&](panda_file::File::EntityId id) {
        const auto ann_list_part = GetETSAnnotation(id, "class");
        ann_list.reserve(ann_list.size() + ann_list_part.size());
        ann_list.insert(ann_list.end(), ann_list_part.begin(), ann_list_part.end());
    });

    class_accessor.EnumerateRuntimeAnnotations([&](panda_file::File::EntityId id) {
        const auto ann_list_part = GetETSAnnotation(id, "runtime");
        ann_list.reserve(ann_list.size() + ann_list_part.size());
        ann_list.insert(ann_list.end(), ann_list_part.begin(), ann_list_part.end());
    });

    if (!ann_list.empty()) {
        RecordAnnotations record_ets_ann {};
        record_ets_ann.ann_list = std::move(ann_list);
        prog_ann_.record_annotations.emplace(record_name, std::move(record_ets_ann));
    }
}

void Disassembler::SetETSAttributes(pandasm::Record *record, const panda_file::File::EntityId &record_id) const
{
    // id of std.core.Object
    [[maybe_unused]] static const size_t OBJ_ENTITY_ID = 0;

    panda_file::ClassDataAccessor class_accessor(*file_, record_id);
    uint32_t acc_flags = class_accessor.GetAccessFlags();

    if ((acc_flags & ACC_INTERFACE) != 0) {
        record->metadata->SetAttribute("ets.interface");
    }
    if ((acc_flags & ACC_ABSTRACT) != 0) {
        record->metadata->SetAttribute("ets.abstract");
    }
    if ((acc_flags & ACC_ANNOTATION) != 0) {
        record->metadata->SetAttribute("ets.annotation");
    }
    if ((acc_flags & ACC_ENUM) != 0) {
        record->metadata->SetAttribute("ets.enum");
    }
    if ((acc_flags & ACC_SYNTHETIC) != 0) {
        record->metadata->SetAttribute("ets.synthetic");
    }

    if (class_accessor.GetSuperClassId().GetOffset() != OBJ_ENTITY_ID) {
        const auto super_class_name = GetFullRecordName(class_accessor.GetSuperClassId());
        record->metadata->SetAttributeValue("ets.extends", super_class_name);
    }

    class_accessor.EnumerateInterfaces([&](panda_file::File::EntityId id) {
        const auto iface_name = GetFullRecordName(id);
        record->metadata->SetAttributeValue("ets.implements", iface_name);
    });
}

void Disassembler::GetETSMetadata(pandasm::Function *method, const panda_file::File::EntityId &method_id)
{
    LOG(DEBUG, DISASSEMBLER) << "[getting ETS metadata]\nmethod id: " << method_id;
    if (method == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved!";
        return;
    }

    panda_file::MethodDataAccessor method_accessor(*file_, method_id);
    SetETSAttributes(method, method_id);
    AnnotationList ann_list {};
    LOG(DEBUG, DISASSEMBLER) << "[getting ETS annotations]\nmethod id: " << method_id;

    method_accessor.EnumerateAnnotations([&](panda_file::File::EntityId id) {
        const auto ann_list_part = GetETSAnnotation(id, "class");
        ann_list.reserve(ann_list.size() + ann_list_part.size());
        ann_list.insert(ann_list.end(), ann_list_part.begin(), ann_list_part.end());
    });

    method_accessor.EnumerateRuntimeAnnotations([&](panda_file::File::EntityId id) {
        const auto ann_list_part = GetETSAnnotation(id, "runtime");
        ann_list.reserve(ann_list.size() + ann_list_part.size());
        ann_list.insert(ann_list.end(), ann_list_part.begin(), ann_list_part.end());
    });

    if (!ann_list.empty()) {
        const auto method_name = GetMethodSignature(method_id);
        prog_ann_.method_annotations.emplace(method_name, std::move(ann_list));
    }
}

void Disassembler::SetETSAttributes(pandasm::Function *method, const panda_file::File::EntityId &method_id) const
{
    panda_file::MethodDataAccessor method_accessor(*file_, method_id);
    uint32_t acc_flags = method_accessor.GetAccessFlags();

    if ((acc_flags & ACC_ABSTRACT) != 0) {
        method->metadata->SetAttribute("ets.abstract");
    }
}

void Disassembler::GetETSMetadata(pandasm::Field *field, const panda_file::File::EntityId &field_id)
{
    LOG(DEBUG, DISASSEMBLER) << "[getting ETS metadata]\nfield id: " << field_id;
    if (field == nullptr) {
        LOG(ERROR, DISASSEMBLER) << "> nullptr recieved!";
        return;
    }

    panda_file::FieldDataAccessor field_accessor(*file_, field_id);
    SetETSAttributes(field, field_id);
    AnnotationList ann_list {};

    LOG(DEBUG, DISASSEMBLER) << "[getting ETS annotations]\nfield id: " << field_id;

    field_accessor.EnumerateAnnotations([&](panda_file::File::EntityId id) {
        const auto ann_list_part = GetETSAnnotation(id, "class");
        ann_list.reserve(ann_list.size() + ann_list_part.size());
        ann_list.insert(ann_list.end(), ann_list_part.begin(), ann_list_part.end());
    });

    field_accessor.EnumerateRuntimeAnnotations([&](panda_file::File::EntityId id) {
        const auto ann_list_part = GetETSAnnotation(id, "runtime");
        ann_list.reserve(ann_list.size() + ann_list_part.size());
        ann_list.insert(ann_list.end(), ann_list_part.begin(), ann_list_part.end());
    });

    if (!ann_list.empty()) {
        const auto record_name = GetFullRecordName(field_accessor.GetClassId());
        const auto field_name = StringDataToString(file_->GetStringData(field_accessor.GetNameId()));
        prog_ann_.record_annotations[record_name].field_annotations.emplace(field_name, std::move(ann_list));
    }
}

void Disassembler::SetETSAttributes(pandasm::Field *field, const panda_file::File::EntityId &field_id) const
{
    panda_file::FieldDataAccessor field_accessor(*file_, field_id);
    uint32_t acc_flags = field_accessor.GetAccessFlags();

    if ((acc_flags & ACC_VOLATILE) != 0) {
        field->metadata->SetAttribute("ets.volatile");
    }
    if ((acc_flags & ACC_ENUM) != 0) {
        field->metadata->SetAttribute("ets.enum");
    }
    if ((acc_flags & ACC_TRANSIENT) != 0) {
        field->metadata->SetAttribute("ets.transient");
    }
    if ((acc_flags & ACC_SYNTHETIC) != 0) {
        field->metadata->SetAttribute("ets.synthetic");
    }
}

// CODECHECK-NOLINTNEXTLINE(C_RULE_ID_FUNCTION_SIZE)
AnnotationList Disassembler::GetETSAnnotation(const panda_file::File::EntityId &annotation_id, const std::string &type)
{
    LOG(DEBUG, DISASSEMBLER) << "[getting ets annotation]\nid: " << annotation_id;
    panda_file::AnnotationDataAccessor annotation_accessor(*file_, annotation_id);
    AnnotationList ann_list {};
    // annotation

    const auto class_name = GetFullRecordName(annotation_accessor.GetClassId());

    if (annotation_accessor.GetCount() == 0) {
        if (!type.empty()) {
            ann_list.push_back({"ets.annotation.type", type});
        }
        ann_list.push_back({"ets.annotation.class", class_name});
        ann_list.push_back({"ets.annotation.id", "id_" + std::to_string(annotation_id.GetOffset())});
    }

    for (size_t i = 0; i < annotation_accessor.GetCount(); ++i) {
        // element
        const auto elem_name_id = annotation_accessor.GetElement(i).GetNameId();
        auto elem_type = AnnotationTagToString(annotation_accessor.GetTag(i).GetItem());
        const bool is_array = elem_type.back() == ']';
        const auto elem_comp_type =
            elem_type.substr(0, elem_type.length() - 2);  // 2 last characters are '[' & ']' if annotation is an array
        // adding necessary annotations
        if (elem_type == "annotations") {
            const auto val = annotation_accessor.GetElement(i).GetScalarValue().Get<panda_file::File::EntityId>();
            const auto ann_definition = GetETSAnnotation(val, "");
            for (const auto &elem : ann_definition) {
                ann_list.push_back(elem);
            }
        }
        if (is_array && elem_comp_type == "annotation") {
            const auto values = annotation_accessor.GetElement(i).GetArrayValue();
            for (size_t idx = 0; idx < values.GetCount(); ++idx) {
                const auto val = values.Get<panda_file::File::EntityId>(idx);
                const auto ann_definition = GetETSAnnotation(val, "");
                for (const auto &elem : ann_definition) {
                    ann_list.push_back(elem);
                }
            }
        }
        if (!type.empty()) {
            ann_list.push_back({"ets.annotation.type", type});
        }

        ann_list.push_back({"ets.annotation.class", class_name});

        ann_list.push_back({"ets.annotation.id", "id_" + std::to_string(annotation_id.GetOffset())});
        ann_list.push_back({"ets.annotation.element.name", StringDataToString(file_->GetStringData(elem_name_id))});
        // type
        if (is_array) {
            elem_type = "array";
            ann_list.push_back({"ets.annotation.element.type", elem_type});
            ann_list.push_back({"ets.annotation.element.array.component.type", elem_comp_type});
            // values
            const auto values = annotation_accessor.GetElement(i).GetArrayValue();
            for (size_t idx = 0; idx < values.GetCount(); ++idx) {
                ann_list.push_back({"ets.annotation.element.value", ArrayValueToString(values, elem_comp_type, idx)});
            }
        } else {
            ann_list.push_back({"ets.annotation.element.type", elem_type});
            // value
            if (elem_type != "void") {
                const auto value = annotation_accessor.GetElement(i).GetScalarValue();
                ann_list.push_back({"ets.annotation.element.value", ScalarValueToString(value, elem_type)});
            }
        }
    }
    return ann_list;
}

}  // namespace panda::disasm