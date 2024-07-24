/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef DISASM_ETS_PLUGIN_INCLUDED
#define DISASM_ETS_PLUGIN_INCLUDED

void GeteTSMetadata();
void GetETSMetadata(pandasm::Record *record, const panda_file::File::EntityId &recordId);
void SetETSAttributes(pandasm::Record *record, const panda_file::File::EntityId &recordId) const;
void GetETSMetadata(pandasm::Function *method, const panda_file::File::EntityId &methodId);
void SetETSAttributes(pandasm::Function *method, const panda_file::File::EntityId &methodId) const;
void GetETSMetadata(pandasm::Field *field, const panda_file::File::EntityId &fieldId);
void SetETSAttributes(pandasm::Field *field, const panda_file::File::EntityId &fieldId) const;
AnnotationList GetETSAnnotation(const panda_file::File::EntityId &id, const std::string &type = "");
void GetETSAnnotationIteration(const panda_file::File::EntityId &annotationId, const std::string &type,
                               panda_file::AnnotationDataAccessor &annotationAccessor, AnnotationList &annList,
                               const std::string &className);
void GetETSAnnotationIteration(const panda_file::ArrayValue &values, AnnotationList &annList);
#endif
