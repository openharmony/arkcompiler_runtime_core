/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_METADATA_MODIFY_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_METADATA_MODIFY_STATIC_H

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/extensions/js/metadata_js.h"
#include <vector>

namespace libabckit {

// ========================================
// Create / Update
// ========================================
AbckitArktsModule *FileAddExternalModuleArktsV2Static(AbckitFile *file, const char *moduleName);
AbckitArktsClass *ModuleImportClassFromArktsV2ToArktsV2Static(AbckitArktsModule *externalModule, const char *className);

AbckitArktsFunction *ModuleImportStaticFunctionStatic(AbckitArktsModule *externalModule, const char *functionName,
                                                      const char *returnType, const std::vector<const char *> &params);

AbckitArktsFunction *ModuleImportClassMethodStatic(AbckitArktsModule *externalModule, const char *className,
                                                   const char *methodName, const char *returnType,
                                                   const std::vector<const char *> &params);

AbckitString *CreateStringStatic(AbckitFile *file, const char *value, size_t len);

bool ModuleSetNameStatic(AbckitCoreModule *m, const char *newName);
bool ModuleFieldSetNameStatic(AbckitCoreModuleField *field, const char *newName);

bool NamespaceSetNameStatic(AbckitCoreNamespace *ns, const char *newName);
bool NamespaceFieldSetNameStatic(AbckitCoreNamespaceField *field, const char *newName);

bool AnnotationInterfaceSetNameStatic(AbckitCoreAnnotationInterface *ai, const char *newName);
bool AnnotationSetNameStatic(AbckitCoreAnnotation *anno, const char *newName);

bool ClassSetNameStatic(AbckitCoreClass *klass, const char *newName);
bool ClassFieldSetNameStatic(AbckitCoreClassField *field, const char *newName);

bool InterfaceSetNameStatic(AbckitCoreInterface *iface, const char *newName);
bool InterfaceFieldSetNameStatic(AbckitCoreInterfaceField *field, const char *newName);

bool EnumSetNameStatic(AbckitCoreEnum *enm, const char *newName);
bool EnumFieldSetNameStatic(AbckitCoreEnumField *field, const char *newName);

void FunctionSetGraphStatic(AbckitCoreFunction *function, AbckitGraph *graph);
void FunctionSetGraphStaticSync(AbckitCoreFunction *function, AbckitGraph *graph);

AbckitLiteral *FindOrCreateLiteralBoolStatic(AbckitFile *file, bool value);
AbckitLiteral *FindOrCreateLiteralU8Static(AbckitFile *file, uint8_t value);
AbckitLiteral *FindOrCreateLiteralU16Static(AbckitFile *file, uint16_t value);
AbckitLiteral *FindOrCreateLiteralMethodAffiliateStatic(AbckitFile *file, uint16_t value);
AbckitLiteral *FindOrCreateLiteralU32Static(AbckitFile *file, uint32_t value);
AbckitLiteral *FindOrCreateLiteralU64Static(AbckitFile *file, uint64_t value);
AbckitLiteral *FindOrCreateLiteralFloatStatic(AbckitFile *file, float value);
AbckitLiteral *FindOrCreateLiteralDoubleStatic(AbckitFile *file, double value);
AbckitLiteral *FindOrCreateLiteralStringStatic(AbckitFile *file, const char *value, size_t len);
AbckitLiteral *FindOrCreateLiteralMethodStatic(AbckitFile *file, AbckitCoreFunction *function);
AbckitLiteralArray *CreateLiteralArrayStatic(AbckitFile *file, AbckitLiteral **value, size_t size);

AbckitValue *FindOrCreateValueU1Static(AbckitFile *file, bool value);
AbckitValue *FindOrCreateValueIntStatic(AbckitFile *file, int value);
AbckitValue *FindOrCreateValueDoubleStatic(AbckitFile *file, double value);
AbckitValue *FindOrCreateValueStringStatic(AbckitFile *file, const char *value, size_t len);

// ========================================
// ModuleField
// ========================================
bool ModuleFieldSetTypeStatic(AbckitArktsModuleField *field, AbckitType *type);
bool ModuleFieldSetValueStatic(AbckitArktsModuleField *field, AbckitValue *value);

// ========================================
// ClassField
// ========================================
bool ClassFieldAddAnnotationStatic(AbckitArktsClassField *field, const AbckitArktsAnnotationCreateParams *params);
bool ClassFieldRemoveAnnotationStatic(AbckitArktsClassField *field, AbckitArktsAnnotation *anno);
bool ClassFieldSetTypeStatic(AbckitArktsClassField *field, AbckitType *type);
bool ClassFieldSetValueStatic(AbckitArktsClassField *field, AbckitValue *value);

// ========================================
// Module
// ========================================
AbckitArktsAnnotationInterface *ModuleAddAnnotationInterfaceStatic(
    AbckitCoreModule *m, const struct AbckitArktsAnnotationInterfaceCreateParams *params);

// ========================================
// AnnotationInterface
// ========================================
AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddFieldStatic(
    AbckitCoreAnnotationInterface *ai, const AbckitArktsAnnotationInterfaceFieldCreateParams *params);
void AnnotationInterfaceRemoveFieldStatic(AbckitCoreAnnotationInterface *ai,
                                          AbckitCoreAnnotationInterfaceField *aiField);

// ========================================
// InterfaceField
// ========================================
bool InterfaceFieldSetTypeStatic(AbckitArktsInterfaceField *field, AbckitType *type);
bool InterfaceFieldAddAnnotationStatic(AbckitArktsInterfaceField *field,
                                       const AbckitArktsAnnotationCreateParams *params);
bool InterfaceFieldRemoveAnnotationStatic(AbckitArktsInterfaceField *field, AbckitArktsAnnotation *anno);

// ========================================
// Function
// ========================================
bool FunctionSetNameStatic(AbckitArktsFunction *function, const char *name);
bool FunctionAddParameterStatic(AbckitArktsFunction *func, AbckitArktsFunctionParam *param);
bool FunctionSetReturnTypeStatic(AbckitArktsFunction *func, AbckitType *type);
bool FunctionRemoveParameterStatic(AbckitArktsFunction *func, size_t index);

AbckitArktsFunction *ModuleAddFunctionStatic(AbckitArktsModule *module, struct AbckitArktsFunctionCreateParams *params);
AbckitArktsFunction *ClassAddMethodStatic(AbckitArktsClass *klass, struct ArktsMethodCreateParams *params);
// ========================================
// Class
// ========================================
AbckitArktsClass *CreateClassStatic(AbckitCoreModule *m, const char *name);
bool ClassRemoveMethodStatic(AbckitCoreClass *klass, AbckitCoreFunction *method);
bool ClassAddInterfaceStatic(AbckitArktsClass *klass, AbckitArktsInterface *iface);
bool ClassRemoveInterfaceStatic(AbckitArktsClass *klass, AbckitArktsInterface *iface);
bool ClassSetSuperClassStatic(AbckitArktsClass *klass, AbckitArktsClass *superClass);
bool ClassAddMethod(AbckitArktsClass *klass, AbckitArktsFunction *method);
bool ClassRemoveFieldStatic(AbckitArktsClass *klass, AbckitCoreClassField *field);
bool ClassAddFieldStatic(AbckitCoreClass *klass, AbckitCoreClassField *coreClassField);
bool ClassSetOwningModuleStatic(AbckitCoreClass *klass, AbckitCoreModule *module);

// ========================================
// Interface
// ========================================
bool InterfaceRemoveFieldStatic(AbckitArktsInterface *iface, AbckitCoreInterfaceField *field);
AbckitArktsInterface *CreateInterfaceStatic(AbckitArktsModule *m, const char *name);
bool InterfaceAddFieldStatic(AbckitArktsInterface *iface, AbckitCoreInterfaceField *field);
bool InterfaceAddMethodStatic(AbckitArktsInterface *iface, AbckitArktsFunction *method);
bool InterfaceAddSuperInterfaceStatic(AbckitCoreInterface *iface, AbckitCoreInterface *superIface);
bool InterfaceRemoveSuperInterfaceStatic(AbckitCoreInterface *iface, AbckitCoreInterface *superIface);
bool InterfaceRemoveMethodStatic(AbckitCoreInterface *iface, AbckitCoreFunction *method);
bool InterfaceSetOwningModuleStatic(AbckitCoreInterface *iface, AbckitCoreModule *module);

// ========================================
// Annotation
// ========================================
AbckitArktsAnnotationElement *AnnotationAddAnnotationElementStatic(AbckitCoreAnnotation *anno,
                                                                   AbckitArktsAnnotationElementCreateParams *params);
void AnnotationRemoveAnnotationElementStatic(AbckitCoreAnnotation *anno, AbckitCoreAnnotationElement *elem);
AbckitArktsAnnotation *FunctionAddAnnotationStatic(AbckitCoreFunction *function,
                                                   const AbckitArktsAnnotationCreateParams *params);
void FunctionRemoveAnnotationStatic(AbckitCoreFunction *function, AbckitCoreAnnotation *anno);
AbckitArktsAnnotation *ClassAddAnnotationStatic(AbckitCoreClass *klass,
                                                const AbckitArktsAnnotationCreateParams *params);
void ClassRemoveAnnotationStatic(AbckitCoreClass *klass, AbckitCoreAnnotation *anno);
}  // namespace libabckit

#endif
