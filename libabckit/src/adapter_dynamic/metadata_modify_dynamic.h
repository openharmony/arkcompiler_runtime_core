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

#ifndef LIBABCKIT_SRC_ADAPTER_DYNAMIC_METADATA_MODIFY_DYNAMIC_H
#define LIBABCKIT_SRC_ADAPTER_DYNAMIC_METADATA_MODIFY_DYNAMIC_H

#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/include/c/extensions/js/metadata_js.h"

namespace libabckit {

AbckitString *CreateStringDynamic(AbckitFile *file, const char *value);
void FunctionSetGraphDynamic(AbckitCoreFunction *function, AbckitGraph *graph);
AbckitJsModule *FileAddExternalModuleDynamic(AbckitFile *file, const struct AbckitJsExternalModuleCreateParams *params);
AbckitArktsModule *FileAddExternalModuleDynamic(AbckitFile *file,
                                                const struct AbckitArktsExternalModuleCreateParams *params);
void ModuleRemoveImportDynamic(AbckitCoreModule *m, AbckitArktsImportDescriptor *i);
void ModuleRemoveImportDynamic(AbckitCoreModule *m, AbckitJsImportDescriptor *i);
AbckitArktsImportDescriptor *ModuleAddImportFromDynamicModuleDynamic(
    AbckitCoreModule *importing, AbckitCoreModule *imported,
    const AbckitArktsImportFromDynamicModuleCreateParams *params);
AbckitJsImportDescriptor *ModuleAddImportFromDynamicModuleDynamic(
    AbckitCoreModule *importing, AbckitCoreModule *imported, const AbckitJsImportFromDynamicModuleCreateParams *params);
AbckitArktsExportDescriptor *DynamicModuleAddExportDynamic(AbckitCoreModule *exporting, AbckitCoreModule *exported,
                                                           const AbckitArktsDynamicModuleExportCreateParams *params);
AbckitJsExportDescriptor *DynamicModuleAddExportDynamic(AbckitCoreModule *exporting, AbckitCoreModule *exported,
                                                        const AbckitJsDynamicModuleExportCreateParams *params);
void ModuleRemoveExportDynamic(AbckitCoreModule *m, AbckitArktsExportDescriptor *i);
void ModuleRemoveExportDynamic(AbckitCoreModule *m, AbckitJsExportDescriptor *i);
AbckitLiteral *CreateLiteralBoolDynamic(AbckitFile *file, bool value);
AbckitLiteral *CreateLiteralU8Dynamic(AbckitFile *file, uint8_t value);
AbckitLiteral *CreateLiteralU16Dynamic(AbckitFile *file, uint16_t value);
AbckitLiteral *CreateLiteralMethodAffiliateDynamic(AbckitFile *file, uint16_t value);
AbckitLiteral *CreateLiteralU32Dynamic(AbckitFile *file, uint32_t value);
AbckitLiteral *CreateLiteralU64Dynamic(AbckitFile *file, uint64_t value);
AbckitLiteral *CreateLiteralFloatDynamic(AbckitFile *file, float value);
AbckitLiteral *CreateLiteralDoubleDynamic(AbckitFile *file, double value);
AbckitLiteral *CreateLiteralStringDynamic(AbckitFile *file, const char *value);
AbckitLiteral *CreateLiteralMethodDynamic(AbckitFile *file, AbckitCoreFunction *function);
AbckitLiteral *CreateLiteralLiteralArrayDynamic(AbckitFile *file, AbckitLiteralArray *litarr);
AbckitLiteralArray *CreateLiteralArrayDynamic(AbckitFile *file, AbckitLiteral **value, size_t size);

AbckitValue *CreateValueU1Dynamic(AbckitFile *file, bool value);
AbckitValue *CreateValueDoubleDynamic(AbckitFile *file, double value);
AbckitValue *CreateValueStringDynamic(AbckitFile *file, const char *value);
AbckitValue *CreateLiteralArrayValueDynamic(AbckitFile *file, AbckitValue **value, size_t size);

AbckitArktsAnnotationInterface *ModuleAddAnnotationInterfaceDynamic(
    AbckitCoreModule *m, const AbckitArktsAnnotationInterfaceCreateParams *params);

AbckitArktsAnnotation *ClassAddAnnotationDynamic(AbckitCoreClass *klass,
                                                 const AbckitArktsAnnotationCreateParams *params);
void ClassRemoveAnnotationDynamic(AbckitCoreClass *klass, AbckitCoreAnnotation *anno);
AbckitArktsAnnotation *FunctionAddAnnotationDynamic(AbckitCoreFunction *function,
                                                    const AbckitArktsAnnotationCreateParams *params);
void FunctionRemoveAnnotationDynamic(AbckitCoreFunction *function, AbckitCoreAnnotation *anno);
AbckitArktsAnnotationElement *AnnotationAddAnnotationElementDynamic(AbckitCoreAnnotation *anno,
                                                                    AbckitArktsAnnotationElementCreateParams *params);
void AnnotationRemoveAnnotationElementDynamic(AbckitCoreAnnotation *anno, AbckitCoreAnnotationElement *elem);
AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddFieldDynamic(
    AbckitCoreAnnotationInterface *ai, const AbckitArktsAnnotationInterfaceFieldCreateParams *params);
void AnnotationInterfaceRemoveFieldDynamic(AbckitCoreAnnotationInterface *ai,
                                           AbckitCoreAnnotationInterfaceField *field);
}  // namespace libabckit

#endif
