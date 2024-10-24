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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_METADATA_INSPECT_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_METADATA_INSPECT_STATIC_H

#include "libabckit/include/c/metadata_core.h"

#include <iostream>

namespace libabckit {

// ========================================
// Module
// ========================================

void ModuleEnumerateAnonymousFunctionsStatic(AbckitCoreModule *m, void *data,
                                             bool (*cb)(AbckitCoreFunction *function, void *data));

// ========================================
// Namespace
// ========================================

// ========================================
// Class
// ========================================

AbckitString *ClassGetNameStatic(AbckitCoreClass *klass);

// ========================================
// Function
// ========================================

AbckitString *FunctionGetNameStatic(AbckitCoreFunction *function);
AbckitGraph *CreateGraphFromFunctionStatic(AbckitCoreFunction *function);
bool FunctionIsStaticStatic(AbckitCoreFunction *function);
bool FunctionIsCtorStatic(AbckitCoreFunction *function);
bool FunctionIsAnonymousStatic(AbckitCoreFunction *function);
bool FunctionIsNativeStatic(AbckitCoreFunction *function);

// ========================================
// Annotation
// ========================================

// ========================================
// String
// ========================================

void StringToStringStatic(AbckitFile *file, AbckitString *value, char *out, size_t *len);

// ========================================
// ImportDescriptor
// ========================================

// ========================================
// ImportDescriptor
// ========================================

// ========================================
// Literal
// ========================================

bool LiteralGetBoolStatic(AbckitLiteral *lit);
uint8_t LiteralGetU8Static(AbckitLiteral *lit);
uint16_t LiteralGetU16Static(AbckitLiteral *lit);
uint16_t LiteralGetMethodAffiliateStatic(AbckitLiteral *lit);
uint32_t LiteralGetU32Static(AbckitLiteral *lit);
uint64_t LiteralGetU64Static(AbckitLiteral *lit);
float LiteralGetFloatStatic(AbckitLiteral *lit);
double LiteralGetDoubleStatic(AbckitLiteral *lit);
AbckitString *LiteralGetStringStatic(AbckitLiteral *lit);
AbckitLiteralTag LiteralGetTagStatic(AbckitLiteral *lit);

// ========================================
// Value
// ========================================

AbckitType *ValueGetTypeStatic(AbckitValue *value);
bool ValueGetU1Static(AbckitValue *value);
double ValueGetDoubleStatic(AbckitValue *value);
AbckitString *ValueGetStringStatic(AbckitValue *value);
AbckitLiteralArray *ArrayValueGetLiteralArrayStatic(AbckitValue *value);

}  // namespace libabckit

#endif
