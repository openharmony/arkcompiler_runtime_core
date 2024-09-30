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

#ifndef ABC2PROGRAM_ETS_PLUGIN_INCLUDED
#define ABC2PROGRAM_ETS_PLUGIN_INCLUDED

// CC-OFFNXT(G.FUD.01) project codestyle
void GeteTSMetadata();
void GetETSMetadata(pandasm::Record *record, const panda_file::File::EntityId &recordId);
void SetETSAttributes(pandasm::Record *record, const panda_file::File::EntityId &recordId) const;
void GetETSMetadata(pandasm::Function *method, const panda_file::File::EntityId &methodId);
void SetETSAttributes(pandasm::Function *method, const panda_file::File::EntityId &methodId) const;
void GetETSMetadata(pandasm::Field *field, const panda_file::File::EntityId &fieldId);
void SetETSAttributes(pandasm::Field *field, const panda_file::File::EntityId &fieldId) const;
AnnotationList GetETSAnnotation(const panda_file::File::EntityId &id, const std::string &type = "");
void GetETSAnnotationImpl(AnnotationList &annList, const std::string &elemType,
                          const panda_file::AnnotationDataAccessor::Elem &elem);

#endif
