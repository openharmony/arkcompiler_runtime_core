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

bool ArkTSModuleEnumerateImports(AbckitCoreModule *m, void *data,
                                 bool (*cb)(AbckitCoreImportDescriptor *i, void *data));
bool ArkTSModuleEnumerateExports(AbckitCoreModule *m, void *data,
                                 bool (*cb)(AbckitCoreExportDescriptor *e, void *data));
bool ArkTSModuleEnumerateNamespaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *n, void *data));
bool ArkTSModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool cb(AbckitCoreClass *klass, void *data));
bool ArkTSModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                           bool (*cb)(AbckitCoreFunction *function, void *data));
bool ArkTSModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data));
bool ArkTSModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

// ========================================
// Namespace
// ========================================

bool ArkTSNamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                       bool (*cb)(AbckitCoreNamespace *klass, void *data));
bool ArkTSNamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreClass *klass, void *data));
bool ArkTSNamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                              bool (*cb)(AbckitCoreFunction *func, void *data));

// ========================================
// Class
// ========================================

bool ArkTSClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *method, void *data));
bool ArkTSClassEnumerateAnnotations(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreAnnotation *anno, void *data));

// ========================================
// Function
// ========================================

bool ArkTSFunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                           bool (*cb)(AbckitCoreFunction *nestedFunc, void *data));
bool ArkTSFunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                         bool (*cb)(AbckitCoreClass *nestedClass, void *data));
bool ArkTSFunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                       bool (*cb)(AbckitCoreAnnotation *anno, void *data));

// ========================================
// Annotation
// ========================================

bool ArkTSAnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                      bool (*cb)(AbckitCoreAnnotationElement *ae, void *data));

// ========================================
// AnnotationInterface
// ========================================

bool ArkTSAnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                             bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data));

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_METADATA_ARKTS_INSPECT_IMPL_H
