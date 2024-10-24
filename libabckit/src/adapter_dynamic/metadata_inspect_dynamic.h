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

#ifndef LIBABCKIT_SRC_ADAPTER_DYNAMIC_METADATA_INSPECT_DYNAMIC_H
#define LIBABCKIT_SRC_ADAPTER_DYNAMIC_METADATA_INSPECT_DYNAMIC_H

#include "libabckit/include/c/metadata_core.h"

namespace libabckit {

// ========================================
// Module
// ========================================

void ModuleEnumerateAnonymousFunctionsDynamic(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreFunction *function, void *data));

// ========================================
// Namespace
// ========================================

AbckitString *NamespaceGetNameDynamic(AbckitCoreNamespace *n);

// ========================================
// Class
// ========================================

AbckitString *ClassGetNameDynamic(AbckitCoreClass *klass);

// ========================================
// Function
// ========================================

AbckitString *FunctionGetNameDynamic(AbckitCoreFunction *function);
void FileEnumerateTopLevelMethodsDynamic(AbckitFile *file, void *data,
                                         bool (*cb)(AbckitCoreFunction *function, void *data));
AbckitGraph *CreateGraphFromFunctionDynamic(AbckitCoreFunction *function);
bool FunctionIsStaticDynamic(AbckitCoreFunction *function);
bool FunctionIsCtorDynamic(AbckitCoreFunction *function);
bool FunctionIsAnonymousDynamic(AbckitCoreFunction *function);
bool FunctionIsNativeDynamic(AbckitCoreFunction *function);

// ========================================
// Annotation
// ========================================

AbckitString *AnnotationInterfaceGetNameDynamic(AbckitCoreAnnotationInterface *ai);

// ========================================
// String
// ========================================

void StringToStringDynamic(AbckitFile *file, AbckitString *value, char *out, size_t *len);

// ========================================
// ImportDescriptor
// ========================================

AbckitString *ImportDescriptorGetNameDynamic(AbckitCoreImportDescriptor *i);
AbckitString *ImportDescriptorGetAliasDynamic(AbckitCoreImportDescriptor *i);

// ========================================
// ExportDescriptor
// ========================================

AbckitString *ExportDescriptorGetNameDynamic(AbckitCoreExportDescriptor *i);
AbckitString *ExportDescriptorGetAliasDynamic(AbckitCoreExportDescriptor *i);

// ========================================
// Literal
// ========================================

bool LiteralGetBoolDynamic(AbckitLiteral *lit);
uint8_t LiteralGetU8Dynamic(AbckitLiteral *lit);
uint16_t LiteralGetU16Dynamic(AbckitLiteral *lit);
uint16_t LiteralGetMethodAffiliateDynamic(AbckitLiteral *lit);
uint32_t LiteralGetU32Dynamic(AbckitLiteral *lit);
uint64_t LiteralGetU64Dynamic(AbckitLiteral *lit);
float LiteralGetFloatDynamic(AbckitLiteral *lit);
double LiteralGetDoubleDynamic(AbckitLiteral *lit);
AbckitLiteralArray *LiteralGetLiteralArrayDynamic(AbckitLiteral *lit);
AbckitString *LiteralGetStringDynamic(AbckitLiteral *lit);
AbckitLiteralTag LiteralGetTagDynamic(AbckitLiteral *lit);
void LiteralArrayEnumerateElementsDynamic(AbckitLiteralArray *litArr, void *data,
                                          bool (*cb)(AbckitFile *file, AbckitLiteral *lit, void *data));

// ========================================
// Value
// ========================================

AbckitType *ValueGetTypeDynamic(AbckitValue *value);
bool ValueGetU1Dynamic(AbckitValue *value);
double ValueGetDoubleDynamic(AbckitValue *value);
AbckitString *ValueGetStringDynamic(AbckitValue *value);
AbckitLiteralArray *ArrayValueGetLiteralArrayDynamic(AbckitValue *value);

}  // namespace libabckit

#endif
