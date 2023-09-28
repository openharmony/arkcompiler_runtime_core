/*
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

#ifndef DISASM_ETS_PLUGIN_INCLUDED
#define DISASM_ETS_PLUGIN_INCLUDED

void GeteTSMetadata();
void GetETSMetadata(pandasm::Record *record, const panda_file::File::EntityId &record_id);
void SetETSAttributes(pandasm::Record *record, const panda_file::File::EntityId &record_id) const;
void GetETSMetadata(pandasm::Function *method, const panda_file::File::EntityId &method_id);
void SetETSAttributes(pandasm::Function *method, const panda_file::File::EntityId &method_id) const;
void GetETSMetadata(pandasm::Field *field, const panda_file::File::EntityId &field_id);
void SetETSAttributes(pandasm::Field *field, const panda_file::File::EntityId &field_id) const;
AnnotationList GetETSAnnotation(const panda_file::File::EntityId &id, const std::string &type = "");

#endif
