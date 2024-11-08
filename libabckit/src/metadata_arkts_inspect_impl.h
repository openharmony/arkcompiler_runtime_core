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

#ifndef LIBABCKIT_SRC_METADATA_ARKTS_INSPECT_IMPL_H
#define LIBABCKIT_SRC_METADATA_ARKTS_INSPECT_IMPL_H

#include "libabckit/include/c/metadata_core.h"

namespace libabckit {

// ========================================
// Module
// ========================================

void ArkTSModuleEnumerateImports(AbckitCoreModule *m, void *data,
                                 bool (*cb)(AbckitCoreImportDescriptor *i, void *data));
void ArkTSModuleEnumerateExports(AbckitCoreModule *m, void *data,
                                 bool (*cb)(AbckitCoreExportDescriptor *e, void *data));
void ArkTSModuleEnumerateNamespaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *n, void *data));
void ArkTSModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool cb(AbckitCoreClass *klass, void *data));
void ArkTSModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                           bool (*cb)(AbckitCoreFunction *function, void *data));
void ArkTSModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data));
void ArkTSModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

// ========================================
// Namespace
// ========================================

void ArkTSNamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                       bool (*cb)(AbckitCoreNamespace *klass, void *data));
void ArkTSNamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreClass *klass, void *data));
void ArkTSNamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                              bool (*cb)(AbckitCoreFunction *func, void *data));

// ========================================
// Class
// ========================================

void ArkTSClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *method, void *data));
void ArkTSClassEnumerateAnnotations(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreAnnotation *anno, void *data));

// ========================================
// Function
// ========================================

void ArkTSFunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                           bool (*cb)(AbckitCoreFunction *nestedFunc, void *data));
void ArkTSFunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                         bool (*cb)(AbckitCoreClass *nestedClass, void *data));
void ArkTSFunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                       bool (*cb)(AbckitCoreAnnotation *anno, void *data));

// ========================================
// Annotation
// ========================================

void ArkTSAnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                      bool (*cb)(AbckitCoreAnnotationElement *ae, void *data));

// ========================================
// AnnotationInterface
// ========================================

void ArkTSAnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                             bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data));

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_METADATA_ARKTS_INSPECT_IMPL_H
